#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>

#include "libpad.h"
#include "libmtap.h"

#include "../../driver.h"
#include "../../fceu.h"

#include "ps2fceu.h"

extern vars Settings;

static char padBuf[4][256] __attribute__((aligned(64)));

u32 old_pad[4];

extern int rapidfire_a[4];
extern int rapidfire_b[4];
int rapid_a[4] = {0,0};
int rapid_b[4] = {0,0};

u8 exitgame = 0;
u8 fdsswap = 0;
int NESButtons;
struct padButtonStatus buttons[4];

void Ingame_Menu(void);

void waitPadReady(int port, int slot)
{
    int state;
    int lastState;
    char stateString[16];

    state = padGetState(port, slot);
    lastState = -1;
    while((state != PAD_STATE_DISCONN) && (state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
        if (state != lastState) {
            padStateInt2String(state, stateString);
            printf("Please wait, pad(%d,%d) is in state %s\n",
                       port, slot, stateString);
        }
        lastState = state;
        state=padGetState(port, slot);
        if(port == 1)
            break;
    }
    // Were the pad ever 'out of sync'?
    if (lastState != -1) {
        printf("Pad OK!\n");
    }
}

int initializePad(int port, int slot)
{

    int ret;
    int modes;
    int i;

    waitPadReady(port, slot);

    // How many different modes can this device operate in?
    // i.e. get # entrys in the modetable
    modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
    printf("The device has %d modes\n", modes);

    if (modes > 0) {
        printf("( ");
        for (i = 0; i < modes; i++) {
            printf("%d ", padInfoMode(port, slot, PAD_MODETABLE, i));
        }
        printf(")");
    }

    printf("It is currently using mode %d\n",
               padInfoMode(port, slot, PAD_MODECURID, 0));

    // If modes == 0, this is not a Dual shock controller
    // (it has no actuator engines)
    if (modes == 0) {
        printf("This is a digital controller?\n");
        return 1;
    }

    // Verify that the controller has a DUAL SHOCK mode
    i = 0;
    do {
        if (padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK)
            break;
        i++;
    } while (i < modes);
    if (i >= modes) {
        printf("This is no Dual Shock controller\n");
        return 1;
    }

    // If ExId != 0x0 => This controller has actuator engines
    // This check should always pass if the Dual Shock test above passed
    ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);
    if (ret == 0) {
        printf("This is no Dual Shock controller??\n");
        return 1;
    }

    waitPadReady(port, slot);

    padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_UNLOCK);

    return 1;
}

//FCEUltra related functions
void setupPS2Pad()
{
    int port = 0;
    int slot = 0;
    int ret;

	if((ret = mtapGetConnection(0)) != 1) {
		for(port=0; port<2; port++) {
			if((ret = padPortOpen(port, slot, padBuf[port])) == 0) {
				printf("padOpenPort failed: %d\n", ret);
			}
			else
				printf("padOpenPort success: %d\n", port);
			if(!initializePad(port, slot)) {
				printf("pad initalization failed!\n");
			}
			else
				printf("pad initialization success!\n");
		}
	}
	else {
		printf("Multitap 1 connected\n");
		for(slot=0; slot<4; slot++) {
			if((ret = padPortOpen(port, slot, padBuf[slot])) == 0) {
				printf("padOpenPort failed: %d\n", ret);
			}
			else
				printf("padOpenPort success: %d\n", slot);
			if(!initializePad(port, slot)) {
				printf("pad initalization failed!\n");
			}
			else
				printf("pad initialization success!\n");
		}
	}
}

unsigned char Get_PS2TurboInput(int mport)
{
    int ret[4];
    u32 paddata[4];
    u32 new_pad[4];
    unsigned char P = 0;
    u16 port = 0;
    u16 slot = 0;

	if(mtapGetConnection(0) == 1)
		slot = mport;//using first port connected multitap, port=0
	else
		port = mport;//using first and second gamepad, slot=0

//check to see if pads are disconnected
    ret[mport]=padGetState(port, slot);
    while((ret[mport] != PAD_STATE_STABLE) && (ret[mport] != PAD_STATE_FINDCTP1)) {
        if(ret[mport]==PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", port, slot);
        }
        ret[mport]=padGetState(port, slot);
        if(port == 1)
            break;
    }
    ret[mport] = padRead(port, slot, &buttons[mport]); // port, slot, buttons
    if (ret[mport] != 0) {
        paddata[mport] = 0xffff ^ buttons[mport].btns;
        new_pad[mport] = paddata[mport] & ~old_pad[mport]; //for each press
        old_pad[mport] = paddata[mport];

        //JOY_Button is NES
        //P |= JOY_Button
        if(new_pad[mport] == Settings.PlayerInput[mport][0]) {
            Ingame_Menu();
        }
        if(new_pad[mport] == Settings.PlayerInput[mport][1]) {
            FCEUI_SaveState(NULL);
        }
        if(new_pad[mport] == Settings.PlayerInput[mport][2]) {
            FCEUI_LoadState(NULL);
        }
        if(new_pad[mport] == Settings.PlayerInput[mport][3]) {//FDS_Disk_Swap
            fdsswap ^= 1;
            if(fdsswap) {
                FCEUI_FDSEject();
            }
            else {
                FCEUI_FDSInsert(0);
            }
        }
        if(new_pad[mport] == Settings.PlayerInput[mport][4]) {//FDS_Side_Swap
            FCEUI_FDSSelect();
        }
        if(paddata[mport] & Settings.PlayerInput[mport][5]) {
            if(rapidfire_a[mport]) {
                rapid_a[mport] ^= 1;
            }
            else {
                P |= JOY_A;
            }
        }
        if(!(paddata[mport] & Settings.PlayerInput[mport][5]) && rapid_a[mport]) {
            rapid_a[mport] = 0;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][6]) {
            if(rapidfire_b[mport]) {
                rapid_b[mport] ^= 1;
            }
            else {
                P |= JOY_B;
            }

        }
        if(!(paddata[mport] & Settings.PlayerInput[mport][6]) && rapid_b[mport]) {
            rapid_b[mport] = 0;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][7]) {
            P |= JOY_SELECT;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][8]) {
            P |= JOY_START;
        }
        //Analog
        if ((buttons[mport].mode >> 4) == 0x07) {
            if (buttons[mport].ljoy_h < 64)
                P |= JOY_LEFT;
			else if (buttons[mport].ljoy_h > 192)
				P |= JOY_RIGHT;
			if (buttons[mport].ljoy_v < 64)
				P |= JOY_UP;
			else if (buttons[mport].ljoy_v > 192)
				P |= JOY_DOWN;
		}
		//Digital
        if(paddata[mport] & Settings.PlayerInput[mport][9]) {
            P |= JOY_UP;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][10]) {
            P |= JOY_DOWN;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][11]) {
            P |= JOY_LEFT;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][12]) {
            P |= JOY_RIGHT;
        }
    }

    return P;

}
unsigned char Get_PS2Input(int mport)
{
    int ret[4];
    u32 paddata[4];
    u32 new_pad[4];
    unsigned char P = 0;
    u16 slot = 0;
    u16 port = 0;
	if(mtapGetConnection(0) == 1)
		slot = mport;//using first port connected multitap, port=0
	else
		port = mport;//using first and second gamepad, slot=0

//check to see if pads are disconnected
    ret[mport]=padGetState(port, slot);
    while((ret[mport] != PAD_STATE_STABLE) && (ret[mport] != PAD_STATE_FINDCTP1)) {
        if(ret[mport]==PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", port, slot);
        }
        ret[mport]=padGetState(port, slot);
        if(port == 1)
            break;
    }
    ret[mport] = padRead(port, slot, &buttons[mport]); // port, slot, buttons
    if (ret[mport] != 0) {
        paddata[mport]= 0xffff ^ buttons[mport].btns;
        new_pad[mport] = paddata[mport] & ~old_pad[mport]; //for each press
        old_pad[mport] = paddata[mport];

        //JOY_Button is NES
        //P |= JOY_Button
        if(new_pad[mport] == Settings.PlayerInput[mport][0]) {
            Ingame_Menu();
        }
        if(new_pad[mport] == Settings.PlayerInput[mport][1]) {
            FCEUI_SaveState(NULL);
        }
        if(new_pad[mport] == Settings.PlayerInput[mport][2]) {
            FCEUI_LoadState(NULL);
        }
        if(new_pad[mport] == Settings.PlayerInput[mport][3]) {//FDS_Disk_Swap
            fdsswap ^= 1;
            if(fdsswap) {
                FCEUI_FDSEject();
            }
            else {
                FCEUI_FDSInsert(0);
            }
        }
        if(new_pad[mport] == Settings.PlayerInput[mport][4]) {//FDS_Side_Swap
            FCEUI_FDSSelect();
        }
        if(paddata[mport] & Settings.PlayerInput[mport][5]) {
            P |= JOY_A;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][6]) {
            P |= JOY_B;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][7]) {
            P |= JOY_SELECT;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][8]) {
            P |= JOY_START;
        }
        //Analog
        if ((buttons[mport].mode >> 4) == 0x07) {
            if (buttons[mport].ljoy_h < 64)
                P |= JOY_LEFT;
			else if (buttons[mport].ljoy_h > 192)
				P |= JOY_RIGHT;
			if (buttons[mport].ljoy_v < 64)
				P |= JOY_UP;
			else if (buttons[mport].ljoy_v > 192)
				P |= JOY_DOWN;
		}
		//Digital
        if(paddata[mport] & Settings.PlayerInput[mport][9]) {
            P |= JOY_UP;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][10]) {
            P |= JOY_DOWN;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][11]) {
            P |= JOY_LEFT;
        }
        if(paddata[mport] & Settings.PlayerInput[mport][12]) {
            P |= JOY_RIGHT;
        }
    }

    return P;

}

void Set_NESInput()
{
    void *NESPads;
	int attrib = 0;

	if(mtapGetConnection(0) != 1) {
		FCEUI_DisableFourScore(1);
		NESPads = &NESButtons;
		FCEUI_SetInput(0, SI_GAMEPAD, NESPads, attrib);
		FCEUI_SetInput(1, SI_GAMEPAD, NESPads, attrib);
	}
	else {
		FCEUI_DisableFourScore(0);
		NESPads = &NESButtons;
		FCEUI_SetInputFC(SIFC_4PLAYER, NESPads, attrib);
		FCEUI_SetInput(0, SI_GAMEPAD, NESPads, attrib);
		FCEUI_SetInput(1, SI_GAMEPAD, NESPads, attrib);
//		FCEUI_SetInput(2, SI_GAMEPAD, NESPads, attrib);
//		FCEUI_SetInput(3, SI_GAMEPAD, NESPads, attrib);
	}
}

int Get_NESInput()
{
    //int NESPress = 0;

    if(exitgame) {
        exitgame = 0;
        return 1;
    }

    if(Settings.turbo) {
        NESButtons = ( Get_PS2TurboInput(0) ); //first player
        NESButtons |= ( Get_PS2TurboInput(1) << 8); //second player

        if(rapid_a[0])
            NESButtons |= JOY_A;

        if(rapid_b[0])
            NESButtons |= JOY_B;

        if(rapid_a[1])
            NESButtons |= 0x100;

        if(rapid_b[1])
            NESButtons |= 0x200;
    }
    else {
        NESButtons = ( Get_PS2Input(0) ); //first player
        NESButtons |= ( Get_PS2Input(1) << 8); //second player
//        NESButtons |= ( Get_PS2Input(2) << 16); //third player
//        NESButtons |= ( Get_PS2Input(3) << 24); //4th player
    }

    return 0;
}
