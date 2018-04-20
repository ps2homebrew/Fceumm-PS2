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
static int mtapGetConnectionCached;

static int rapid_a[4] = { 0 };
static int rapid_b[4] = { 0 };

u8 exitgame = 0;
u8 fdsswap = 0;
static int NESButtons;
static struct padButtonStatus buttons[4];

extern void Ingame_Menu();

static void waitPadReady(int port, int slot)
{
    int state;
    int lastState;
    char stateString[16];

    state = padGetState(port, slot);
    lastState = -1;
    while ((state != PAD_STATE_DISCONN) && (state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
        if (state != lastState) {
            padStateInt2String(state, stateString);
            printf("Please wait, pad(%d, %d) is in state %s\n", port, slot, stateString);
        }
        lastState = state;
        state = padGetState(port, slot);
        if (port == 1)
            break;
    }
    // Were the pad ever 'out of sync'?
    if (lastState != -1) {
        printf("Pad OK!\n");
    }
}

static int initializePad(int port, int slot)
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

    printf("It is currently using mode %d\n", padInfoMode(port, slot, PAD_MODECURID, 0));

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

// FCEUltra related functions
void setupPS2Pad()
{
    int port = 0;
    int slot = 0;
    int ret;

    if ((ret = mtapGetConnection(0)) != 1) {
        for (port = 0; port < 2; port++) {
            if ((ret = padPortOpen(port, slot, padBuf[port])) == 0) {
                printf("padOpenPort failed: %d\n", ret);
            }
            else
                printf("padOpenPort success: %d\n", port);
            if (!initializePad(port, slot)) {
                printf("pad initalization failed!\n");
            }
            else
                printf("pad initialization success!\n");
        }
    }
    else {
        printf("Multitap 1 connected\n");
        for (slot = 0; slot < 4; slot++) {
            if ((ret = padPortOpen(port, slot, padBuf[slot])) == 0) {
                printf("padOpenPort failed: %d\n", ret);
            }
            else
                printf("padOpenPort success: %d\n", slot);
            if (!initializePad(port, slot)) {
                printf("pad initalization failed!\n");
            }
            else
                printf("pad initialization success!\n");
        }
    }
}

static unsigned char Get_PS2Input(int mport)
{
    int ret[4];
    u32 paddata[4];
    u32 new_pad[4];
    unsigned char P = 0;
    u16 port = 0;
    u16 slot = 0;

    if (mtapGetConnectionCached == 1)
        slot = mport; // Using first port connected multitap, port = 0
    else
        port = mport; // Using first and second gamepad, slot = 0

    // Check to see if pads are disconnected
    ret[mport] = padGetState(port, slot);
    while ((ret[mport] != PAD_STATE_STABLE) && (ret[mport] != PAD_STATE_FINDCTP1)) {
        if (ret[mport] == PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", port, slot);
        }
        ret[mport] = padGetState(port, slot);
        if (port == 1)
            break;
    }
    ret[mport] = padRead(port, slot, &buttons[mport]); // port, slot, buttons
    if (ret[mport] != 0) {
        paddata[mport] = 0xffff ^ buttons[mport].btns;
        new_pad[mport] = paddata[mport] & ~old_pad[mport]; // For each press
        old_pad[mport] = paddata[mport];

        // JOY_Button is NES
        // P |= JOY_Button
        if (new_pad[mport] == Settings.PlayerInput[mport][0]) {
            Ingame_Menu();
        }
        if (new_pad[mport] == Settings.PlayerInput[mport][1]) {
            FCEUI_SaveState(NULL);
        }
        if (new_pad[mport] == Settings.PlayerInput[mport][2]) {
            FCEUI_LoadState(NULL);
        }
        if (new_pad[mport] == Settings.PlayerInput[mport][3]) { // FDS_Disk_Swap
            if (GameInfo->type == GIT_FDS) {
                fdsswap ^= 1;
                if (fdsswap) {
                    FCEUI_FDSEject();
                }
                else {
                    FCEUI_FDSInsert(0);
                }
            }
            else if (GameInfo->type == GIT_VSUNI) {
                FCEUI_VSUniCoin();
            }
        }
        if (new_pad[mport] == Settings.PlayerInput[mport][4]) { // FDS_Side_Swap
            if (GameInfo->type == GIT_FDS) {
                FCEUI_FDSSelect();
            }
        }
        if (paddata[mport] & Settings.PlayerInput[mport][5]) {
            P |= JOY_A;
        }
        if (paddata[mport] & Settings.PlayerInput[mport][6]) {
            P |= JOY_B;
        }
        if (paddata[mport] & Settings.PlayerInput[mport][7]) {
            P |= JOY_SELECT;
        }
        if (paddata[mport] & Settings.PlayerInput[mport][8]) {
            P |= JOY_START;
        }
        // Analog
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
        // Digital
        if (paddata[mport] & Settings.PlayerInput[mport][9]) {
            P |= JOY_UP;
        }
        if (paddata[mport] & Settings.PlayerInput[mport][10]) {
            P |= JOY_DOWN;
        }
        if (paddata[mport] & Settings.PlayerInput[mport][11]) {
            P |= JOY_LEFT;
        }
        if (paddata[mport] & Settings.PlayerInput[mport][12]) {
            P |= JOY_RIGHT;
        }
        // Turbo A
        if (paddata[mport] & Settings.PlayerInput[mport][13]) {
            if (!rapid_a[mport]) {
                P |= JOY_A;
            }
            rapid_a[mport] = (rapid_a[mport] + 1) % (Settings.autofire_pattern + 2);
        }
        if (!(paddata[mport] & Settings.PlayerInput[mport][13]) && rapid_a[mport]) {
            rapid_a[mport] = 0;
        }
        // Turbo B
        if (paddata[mport] & Settings.PlayerInput[mport][14]) {
            if (!rapid_b[mport]) {
                P |= JOY_B;
            }
            rapid_b[mport] = (rapid_b[mport] + 1) % (Settings.autofire_pattern + 2);
        }
        if (!(paddata[mport] & Settings.PlayerInput[mport][14]) && rapid_b[mport]) {
            rapid_b[mport] = 0;
        }
    }

    return P;
}

void Set_NESInput()
{
    int attrib = 0;
    
    mtapGetConnectionCached = mtapGetConnection(0);
    if (mtapGetConnectionCached != 1) {
        FCEUI_DisableFourScore(1);
    }
    else {
        FCEUI_DisableFourScore(0);
    }
    FCEUI_SetInput(0, SI_GAMEPAD, &NESButtons, attrib);
    FCEUI_SetInput(1, SI_GAMEPAD, &NESButtons, attrib);
}

int Get_NESInput()
{
    if (exitgame) {
        exitgame = 0;
        return 1;
    }
    
    NESButtons  = ( Get_PS2Input(0) << 0); // First player
    NESButtons |= ( Get_PS2Input(1) << 8); // Second player
    if (mtapGetConnectionCached == 1) {
        NESButtons |= ( Get_PS2Input(2) << 16); // Third player
        NESButtons |= ( Get_PS2Input(3) << 24); // 4th player
    }

    return 0;
}
