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

u32 old_pad[2] = {0,0};

int exitgame = 0;
int NESButtons; //scope saves status of buttons pressed more than one loop
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
        new_pad[port] = paddata[port] & ~old_pad[port]; // used so savestate controls aren't executed every loop
        old_pad[port] = paddata[port];

        //JOY_Button is NES
        //P |= JOY_Button
        if(new_pad[port] == Settings.PlayerInput[port][0]) {
            Ingame_Menu();
        }
        if(paddata[port] & Settings.PlayerInput[port][1]) {
            FCEUI_SaveState(NULL);
        }
        if(paddata[port] & Settings.PlayerInput[port][2]) {
            FCEUI_LoadState(NULL);
        }
        if(paddata[port] & Settings.PlayerInput[port][3]) {
            P |= JOY_A;
        }
        if(paddata[port] & Settings.PlayerInput[port][4]) {
            P |= JOY_B;
        }
        if(paddata[port] & Settings.PlayerInput[port][5]) {
            P |= JOY_SELECT;
        }
        if(paddata[port] & Settings.PlayerInput[port][6]) {
            P |= JOY_START;
        }
        if(paddata[port] & Settings.PlayerInput[port][7]) {
            P |= JOY_UP;
        }
        if(paddata[port] & Settings.PlayerInput[port][8]) {
            P |= JOY_DOWN;
        }
        if(paddata[port] & Settings.PlayerInput[port][9]) {
            P |= JOY_LEFT;
        }
        if(paddata[port] & Settings.PlayerInput[port][10]) {
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
    int NESPress = 0;

    if(exitgame) {
        exitgame = 0;
        return 1;
    }

    NESPress = ( Get_PS2Input(0) ); //first player
    NESPress |= ( Get_PS2Input(1) << 8); //second player

    NESButtons = NESPress;

    return 0;
}
