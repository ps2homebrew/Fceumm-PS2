#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>

#include "libpad.h"

#include "../../driver.h"
#include "../../fceu.h"

#include "ps2fceu.h"

extern vars Settings;

static char padBuf[2][256] __attribute__((aligned(64)));

u32 old_pad[2];

extern int rapidfire_a[2];
extern int rapidfire_b[2];
int rapid_a[2] = {0,0};
int rapid_b[2] = {0,0};

u8 exitgame = 0;
u8 fdsswap = 0;
int NESButtons;
struct padButtonStatus buttons[2];

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
    int port;
    int slot = 0;
    int ret;

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

unsigned char Get_PS2TurboInput(int port)
{
    int ret[2];
    u32 paddata[2];
    u32 new_pad[2];
    unsigned char P = 0;
    u16 slot = 0;


//check to see if pads are disconnected
    ret[port]=padGetState(0, slot);
    while((ret[port] != PAD_STATE_STABLE) && (ret[port] != PAD_STATE_FINDCTP1)) {
        if(ret[port]==PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", port, slot);
        }
        ret[port]=padGetState(port, slot);
        if(port == 1)
            break;
    }
    ret[port] = padRead(port, slot, &buttons[port]); // port, slot, buttons
    if (ret[port] != 0) {
        paddata[port] = 0xffff ^ buttons[port].btns;
        new_pad[port] = paddata[port] & ~old_pad[port]; //for each press
        old_pad[port] = paddata[port];

        //JOY_Button is NES
        //P |= JOY_Button
        if(new_pad[port] == Settings.PlayerInput[port][0]) {
            Ingame_Menu();
        }
        if(new_pad[port] == Settings.PlayerInput[port][1]) {
            FCEUI_SaveState(NULL);
        }
        if(new_pad[port] == Settings.PlayerInput[port][2]) {
            FCEUI_LoadState(NULL);
        }
        if(new_pad[port] == Settings.PlayerInput[port][3]) {//FDS_Disk_Swap
            fdsswap ^= 1;
            if(fdsswap) {
                FCEUI_FDSEject();
            }
            else {
                FCEUI_FDSInsert(0);
            }
        }
        if(new_pad[port] == Settings.PlayerInput[port][4]) {//FDS_Side_Swap
            FCEUI_FDSSelect();
        }
        if(paddata[port] & Settings.PlayerInput[port][5]) {
            if(rapidfire_a[port]) {
                rapid_a[port] ^= 1;
            }
            else {
                P |= JOY_A;
            }
        }
        if(!(paddata[port] & Settings.PlayerInput[port][5]) && rapid_a[port]) {
            rapid_a[port] = 0;
        }
        if(paddata[port] & Settings.PlayerInput[port][6]) {
            if(rapidfire_b[port]) {
                rapid_b[port] ^= 1;
            }
            else {
                P |= JOY_B;
            }

        }
        if(!(paddata[port] & Settings.PlayerInput[port][6]) && rapid_b[port]) {
            rapid_b[port] = 0;
        }
        if(paddata[port] & Settings.PlayerInput[port][7]) {
            P |= JOY_SELECT;
        }
        if(paddata[port] & Settings.PlayerInput[port][8]) {
            P |= JOY_START;
        }
        //Analog
        if ((buttons[port].mode >> 4) == 0x07) {
            if (buttons[port].ljoy_h < 64)
                P |= JOY_LEFT;
			else if (buttons[port].ljoy_h > 192)
				P |= JOY_RIGHT;
			if (buttons[port].ljoy_v < 64)
				P |= JOY_UP;
			else if (buttons[port].ljoy_v > 192)
				P |= JOY_DOWN;
		}
		//Digital
        if(paddata[port] & Settings.PlayerInput[port][9]) {
            P |= JOY_UP;
        }
        if(paddata[port] & Settings.PlayerInput[port][10]) {
            P |= JOY_DOWN;
        }
        if(paddata[port] & Settings.PlayerInput[port][11]) {
            P |= JOY_LEFT;
        }
        if(paddata[port] & Settings.PlayerInput[port][12]) {
            P |= JOY_RIGHT;
        }
    }

    return P;

}
unsigned char Get_PS2Input(int port)
{
    int ret[2];
    u32 paddata[2];
    u32 new_pad[2];
    unsigned char P = 0;
    u16 slot = 0;


//check to see if pads are disconnected
    ret[port]=padGetState(0, slot);
    while((ret[port] != PAD_STATE_STABLE) && (ret[port] != PAD_STATE_FINDCTP1)) {
        if(ret[port]==PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", port, slot);
        }
        ret[port]=padGetState(port, slot);
        if(port == 1)
            break;
    }
    ret[port] = padRead(port, slot, &buttons[port]); // port, slot, buttons
    if (ret[port] != 0) {
        paddata[port]= 0xffff ^ buttons[port].btns;
        new_pad[port] = paddata[port] & ~old_pad[port]; //for each press
        old_pad[port] = paddata[port];

        //JOY_Button is NES
        //P |= JOY_Button
        if(new_pad[port] == Settings.PlayerInput[port][0]) {
            Ingame_Menu();
        }
        if(new_pad[port] == Settings.PlayerInput[port][1]) {
            FCEUI_SaveState(NULL);
        }
        if(new_pad[port] == Settings.PlayerInput[port][2]) {
            FCEUI_LoadState(NULL);
        }
        if(new_pad[port] == Settings.PlayerInput[port][3]) {//FDS_Disk_Swap
            fdsswap ^= 1;
            if(fdsswap) {
                FCEUI_FDSEject();
            }
            else {
                FCEUI_FDSInsert(0);
            }
        }
        if(new_pad[port] == Settings.PlayerInput[port][4]) {//FDS_Side_Swap
            FCEUI_FDSSelect();
        }
        if(paddata[port] & Settings.PlayerInput[port][5]) {
            P |= JOY_A;
        }
        if(paddata[port] & Settings.PlayerInput[port][6]) {
            P |= JOY_B;
        }
        if(paddata[port] & Settings.PlayerInput[port][7]) {
            P |= JOY_SELECT;
        }
        if(paddata[port] & Settings.PlayerInput[port][8]) {
            P |= JOY_START;
        }
        //Analog
        if ((buttons[port].mode >> 4) == 0x07) {
            if (buttons[port].ljoy_h < 64)
                P |= JOY_LEFT;
			else if (buttons[port].ljoy_h > 192)
				P |= JOY_RIGHT;
			if (buttons[port].ljoy_v < 64)
				P |= JOY_UP;
			else if (buttons[port].ljoy_v > 192)
				P |= JOY_DOWN;
		}
		//Digital
        if(paddata[port] & Settings.PlayerInput[port][9]) {
            P |= JOY_UP;
        }
        if(paddata[port] & Settings.PlayerInput[port][10]) {
            P |= JOY_DOWN;
        }
        if(paddata[port] & Settings.PlayerInput[port][11]) {
            P |= JOY_LEFT;
        }
        if(paddata[port] & Settings.PlayerInput[port][12]) {
            P |= JOY_RIGHT;
        }
    }

    return P;

}

void Set_NESInput()
{
    void *NESPads;
	int attrib = 0;

	FCEUI_DisableFourScore(1);

	NESPads = &NESButtons;
	FCEUI_SetInput(0, SI_GAMEPAD, NESPads, attrib);
	FCEUI_SetInput(1, SI_GAMEPAD, NESPads, attrib);
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
    }

    return 0;
}
