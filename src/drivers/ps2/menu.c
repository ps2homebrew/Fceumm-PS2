#include <stdio.h>
#include <fileio.h>
#include <io_common.h>
#include <sys/stat.h>
#include <libpad.h>
#include <dmaKit.h>
#include <gsKit.h>
#include <loadfile.h>
#include <libpwroff.h>
#include <kernel.h>

#ifdef SOUND_ON
#include <audsrv.h>
#endif

#include "../../driver.h"
#include "ps2fceu.h"

// Settings
extern vars Settings;
// Skin
extern skin FCEUSkin;
extern u8 menutex;
extern u8 bgtex;
// Input
extern char path[4096];
extern u8 h;

// Palette for FCEU
#define MAXPAL (29 + 1)
struct st_palettes {
    char name[32];
    char desc[32];
    unsigned int data[64];
};
extern struct st_palettes palettes[];

static int statenum = 0;
static u8 power_off = 0;

/************************************/
/* gsKit Variables                  */
/************************************/
extern GSGLOBAL *gsGlobal;
extern GSTEXTURE NES_TEX;
extern GSTEXTURE BG_TEX;
extern GSTEXTURE MENU_TEX;

/************************************/
/* Pad Variables                    */
/************************************/
extern u32 old_pad[4];
static struct padButtonStatus buttons[2];
extern u8 fdsswap;

/************************************/
/* Browser and Emulator Variables   */
/************************************/
extern s8 selected;
extern int oldselect;
extern u8 selected_dir;
extern u8 exitgame;
extern int FONT_HEIGHT;

//void font_print(GSGLOBAL *gsGlobal, float X, float Y, int Z, unsigned long color, char *String);

static inline char* strzncpy(char *d, const char *s, size_t l) { d[0] = 0; return strncat(d, s, l); }

void menu_background(float x1, float y1, float x2, float y2, int z)
{
    int thickness = 3;

    // Border
    gsKit_prim_sprite(gsGlobal, x1, y1, x2, y1+thickness, z, FCEUSkin.frame); // Top
    gsKit_prim_sprite(gsGlobal, x1, y1, x1+thickness, y2, z, FCEUSkin.frame); // Left
    gsKit_prim_sprite(gsGlobal, x2-thickness, y1, x2, y2, z, FCEUSkin.frame); // Right
    gsKit_prim_sprite(gsGlobal, x1, y2-thickness, x2, y2, z, FCEUSkin.frame); // Bottom

    // Background
    gsKit_prim_quad_gouraud(gsGlobal, x1+thickness, y1+thickness,
                                      x2-thickness, y1+thickness,
                                      x1+thickness, y2-thickness,
                                      x2-thickness, y2-thickness,
                                      z+1,
                                      FCEUSkin.bgColor1, FCEUSkin.bgColor2,
                                      FCEUSkin.bgColor3, FCEUSkin.bgColor4);
}

void menu_bgtexture(GSTEXTURE *gsTexture, float x1, float y1, float x2, float y2, int z)
{
    int thickness = 3;

    // Border
    gsKit_prim_sprite(gsGlobal, x1, y1, x2, y1+thickness, z, FCEUSkin.frame); // Top
    gsKit_prim_sprite(gsGlobal, x1, y1, x1+thickness, y2, z, FCEUSkin.frame); // Left
    gsKit_prim_sprite(gsGlobal, x2-thickness, y1, x2, y2, z, FCEUSkin.frame); // Right
    gsKit_prim_sprite(gsGlobal, x1, y2-thickness, x2, y2, z, FCEUSkin.frame); // Bottom

    gsKit_prim_sprite_texture(gsGlobal, gsTexture,
        x1 + thickness,                        /* X1 */
        y1 + thickness,                        /* Y1 */
        0.0f,                                  /* U1 */
        0.0f,                                  /* V1 */
        x2 - thickness,                        /* X2 */
        y2 - thickness,                        /* Y2 */
        gsTexture->Width,                      /* U2 */
        gsTexture->Height,                     /* V2 */
        z + 1,                                 /* Z  */
        GS_SETREG_RGBA(0x80, 0x80, 0x80, 0x00) /* RGBA */
    );
}

void menu_primitive(char *title, GSTEXTURE *gsTexture, float x1, float y1, float x2, float y2)
{

    if (!menutex || !bgtex) {
        menu_bgtexture(gsTexture, x1, y1, x2, y2, 1);
    }
    else {
        menu_background(x1, y1, x2, y2, 1);
    }
    menu_background(x2-(strlen(title)*12), y1, x2, y1+FONT_HEIGHT*2, 2);

    printXY(title, x2-(strlen(title)*10), y1+FONT_HEIGHT/2, 3, FCEUSkin.textcolor, 2, 0);
}

void browser_primitive(char *title1, char *title2, GSTEXTURE *gsTexture, float x1, float y1, float x2, float y2)
{

    if (!menutex || !bgtex) {
        menu_bgtexture(gsTexture, x1, y1, x2, y2, 1);
    }
    else {
        menu_background(x1, y1, x2, y2, 1);
    }
    menu_background(x1, y1, x1+(strlen(title1)*9), y1+FONT_HEIGHT*2, 2);
    menu_background(x2-(strlen(title2)*12), y1, x2, y1+FONT_HEIGHT*2, 2);

    printXY(title1, x1+(strlen(title2)+4), y1+FONT_HEIGHT/2, 3, FCEUSkin.textcolor, 2, 0);
    printXY(title2, x2-(strlen(title2)*10), y1+FONT_HEIGHT/2, 3, FCEUSkin.textcolor, 2, 0);
}

static int menu_input(int port, int center_screen)
{
    int ret[2];
    u32 paddata[2];
    u32 new_pad[2];
    u16 slot = 0;

    int change = 0;

    // Check to see if pads are disconnected
    ret[port] = padGetState(0, slot);
    if ((ret[port] != PAD_STATE_STABLE) && (ret[port] != PAD_STATE_FINDCTP1)) {
        if (ret[port] == PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", 0, slot);
        }
        ret[port] = padGetState(0, slot);
    }
    ret[port] = padRead(0, slot, &buttons[port]); // port, slot, buttons
    if (ret[port] != 0) {
        paddata[port]= 0xffff ^ buttons[port].btns;
        new_pad[port] = paddata[port] & ~old_pad[port]; // Buttons pressed AND NOT buttons previously pressed
        old_pad[port] = paddata[port];

        if (paddata[port] & PAD_LEFT && center_screen) {
            Settings.offset_x--;
            change = 1;
        }
        if (new_pad[port] & PAD_DOWN && !center_screen) {
            change = 1;
        }
        if (paddata[port] & PAD_DOWN && center_screen) {
            Settings.offset_y++;
            change = 1;
        }
        if (paddata[port] & PAD_RIGHT && center_screen) {
            Settings.offset_x++;
            change = 1;
        }
        if (new_pad[port] & PAD_UP && !center_screen) {
            change = -1;
        }
        if (paddata[port] & PAD_UP && center_screen) {
            Settings.offset_y--;
            change = 1;
        }
        if (new_pad[port] & PAD_START && center_screen) {
            change = 2;
        }
        if (new_pad[port] & PAD_SELECT && center_screen) {
            Settings.offset_x = 0;
            Settings.offset_y = 0;
            change = 1;
        }
        if (new_pad[port] & PAD_CIRCLE) {
            selected = 1;
        }
        if (new_pad[port] & PAD_CROSS) {
        }
        if ((new_pad[port] == Settings.PlayerInput[port][0]
          || new_pad[port] == PAD_TRIANGLE) && !center_screen) {
            selected = 2;
        }
    }
    // FIXME: Screen always updates 
    if ((center_screen && change) || (center_screen == 2)) {
        SetDisplayOffset();

        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x00));

        menu_primitive("Centering", &BG_TEX, 0, 0, gsGlobal->Width, gsGlobal->Height);

        DrawScreen(gsGlobal);

        center_screen = 1;
    }
    return change;
}

#define BROWSER_MENU_N 12
#define BROWSER_MENU_EXIT_I 11

int Browser_Menu()
{
    char cnfpath[2048];
    int i, selection = 0;
    oldselect = -1;
    int option_changed = 0;

    int menu_x1 = gsGlobal->Width  * 0.25;
    int menu_y1 = gsGlobal->Height * 0.15;
    int menu_x2 = gsGlobal->Width  * 0.75;
    int menu_y2 = gsGlobal->Height * 0.85 + FONT_HEIGHT;
    int text_line = menu_y1 + 4;

    char options[BROWSER_MENU_N][32] = {
        { "Display: " },
        { "Interlacing: " },
        { "Emulated System: " },
        { "Center Screen" },
        { "Configure Save Path: " },
        { "" },
        { "Configure ELF Path:  " },
        { "" },
        { "Save FCEUltra.cnf"},
        { "Power Off" },
        { "Exit to ELF" },
        { "Exit Options Menu" }
    };
    char options_state[BROWSER_MENU_N][64] = { { 0 } };
    char options_buffer[32+64] = { 0 };

    // Fill lines with values
    for (i = 0; i < BROWSER_MENU_N; i++) {
        switch (i) {
            case 0:
                if (Settings.display == 0) {
                    strcpy(options_state[i], "NTSC");
                }
                else if (Settings.display == 1) {
                    strcpy(options_state[i], "PAL");
                }
                else if (Settings.display == 2) {
                    strcpy(options_state[i], "DTV 720x480");
                }
                break;
            case 1:
                if (Settings.interlace) {
                    strcpy(options_state[i], "On");
                }
                else {
                    strcpy(options_state[i], "Off");
                }
                break;
            case 2:
                if (!Settings.emulation) {
                    strcpy(options_state[i], "NTSC");
                }
                else {
                    strcpy(options_state[i], "PAL");
                }
                break;
            case 5:
                strzncpy(options_state[5], Settings.savepath, 38);
                break;
            case 7:
                strzncpy(options_state[7], Settings.elfpath, 38);
                break;
        }
    }

    while (1) {
        selected = 0; // Clear selected flag
        selection += menu_input(0, 0);

        if (selection >= BROWSER_MENU_N) { selection = 0; }
        if (selection < 0) { selection = BROWSER_MENU_N - 1; }
        if (selection == 5 && oldselect == 4) { selection++; } // 5 is savepath
        if (selection == 5 && oldselect == 6) { selection--; }
        if (selection == 7 && oldselect == 6) { selection++; } // 7 is elfpath
        if (selection == 7 && oldselect == 8) { selection--; }

        if ((oldselect != selection) || option_changed) {

            gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x00));

            menu_primitive("Options", &MENU_TEX, menu_x1, menu_y1, menu_x2, menu_y2);

            for (i = 0; i < BROWSER_MENU_N; i++) {
                strcpy(options_buffer, options[i]);
                strcat(options_buffer, options_state[i]);
                if (selection == i) {
                    //font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, DarkYellowFont, options[i]);
                    printXY(options_buffer, menu_x1+10, text_line+i*FONT_HEIGHT, 4, FCEUSkin.highlight, 1, 0);
                }
                else {
                    //font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, WhiteFont, options[i]);
                    printXY(options_buffer, menu_x1+10, text_line + i*FONT_HEIGHT, 4, FCEUSkin.textcolor, 1, 0);
                }
            }

            DrawScreen(gsGlobal);

            if (power_off)
                option_changed = 1;
                power_off--;
                if (!power_off) {
                    strcpy(cnfpath, "xyz:/imaginary/hypothetical/doesn't.exist");
                    FILE *File;
                    File = fopen(cnfpath, "r");
                    if (File != NULL)
                        fclose(File);
                }
        }

        oldselect = selection;
        option_changed = 0;

        if (selected) {
            if (selected == 2) {
                selection = BROWSER_MENU_EXIT_I;
            }
            i = selection;
            switch (i) {
                case 0: // Display PAL/NTSC
                    Settings.display++;
                    if (Settings.display >= 3) {
                        Settings.display = 0;
                    }
                    if (Settings.display == 0) {
                        strcpy(options_state[i], "NTSC");
                    }
                    else if (Settings.display == 1) {
                        strcpy(options_state[i], "PAL");
                    }
                    else if (Settings.display == 2) {
                        strcpy(options_state[i], "DTV 720x480");
                    }

                    init_custom_screen();

                    menu_x1 = gsGlobal->Width  * 0.25;
                    menu_y1 = gsGlobal->Height * 0.15;
                    menu_x2 = gsGlobal->Width  * 0.75;
                    menu_y2 = gsGlobal->Height * 0.85 + FONT_HEIGHT;
                    text_line = menu_y1 + 4;
                    option_changed = 1;
                    break;
                case 1: // Interlacing Off/On
                    Settings.interlace ^= 1;
                    if (Settings.interlace) {
                        strcpy(options_state[i], "On");
                    }
                    else {
                        strcpy(options_state[i], "Off");
                    }

                    init_custom_screen();

                    menu_x1 = gsGlobal->Width  * 0.25;
                    menu_y1 = gsGlobal->Height * 0.15;
                    menu_x2 = gsGlobal->Width  * 0.75;
                    menu_y2 = gsGlobal->Height * 0.85 + FONT_HEIGHT;
                    text_line = menu_y1 + 4;
                    option_changed = 1;
                    break;
                case 2: // Emulated System
                    Settings.emulation ^= 1;
                    if (Settings.emulation) {
                        strcpy(options_state[i], "PAL");
                    }
                    else {
                        strcpy(options_state[i], "NTSC");
                    }
                    FCEUI_SetVidSystem(Settings.emulation);
                    option_changed = 1;
                    break;
                case 3: // Center Screen
                    while (menu_input(0, 2) != 2) {}
                    i = 0x10000;
                    while (i--) asm("nop\nnop\nnop\nnop");
                    option_changed = 1;
                    break;
                case 4: // Configure Save Path
                    h = 0; // Reset browser
                    selection = 0;
                    oldselect = -1;
                    selected = 0;
                    strcpy(path, "path"); // End reset browser
                    strcpy(Settings.savepath, Browser(0, 1));
                    printf("%s", Settings.savepath);
                    strzncpy(options_state[5], Settings.savepath, 38);
                    selected_dir = 0;
                    h = 0;
                    selection = 0;
                    oldselect = -1;
                    strcpy(path, "path");
                    option_changed = 1;
                    selected = 0;
                    break;
                case 6: // Configure ELF Path
                    h = 0;
                    selection = 0;
                    oldselect = -1;
                    selected = 0;
                    strcpy(path, "path");
                    strcpy(Settings.elfpath, Browser(1, 2));
                    strzncpy(options_state[7], Settings.elfpath, 38);
                    h = 0;
                    selection = 0;
                    oldselect = -1;
                    strcpy(path, "path");
                    option_changed = 1;
                    selected = 0;
                    break;
                case 8: // Save CNF
                    fioMkdir("mc0:FCEUMM");
                    Save_Global_CNF("mc0:/FCEUMM/FCEUltra.cnf");
                    break;
                case 9: // Power Off
                    poweroffShutdown();
                    if (Settings.display == 1)
                        power_off = 50/4;
                    else
                        power_off = 60/4;
                    option_changed = 1;
                    break;
                case 10: // Exit to ELF
                    return 2;
                case 11: // Exit Options Menu
                    selected = 0;
                    return 1;

            }
        }
    }
}

extern void SetupNESGS();
extern void SetupNESClut();

extern void SND_SetNextSampleRate();
extern  int SND_GetCurrSampleRate();

static void Ingame_Menu_Controls();

#define INGAME_MENU_N 11
#define INGAME_MENU_EXIT_I 9
#define INGAME_MENU_PALETTE_I 7

void Ingame_Menu()
{
    int i, selection = 0;
    oldselect = -1;

    int option_changed = 0;

    int menu_x1 = gsGlobal->Width  * 0.25;
    int menu_y1 = gsGlobal->Height * 0.15;
    int menu_x2 = gsGlobal->Width  * 0.75;
    int menu_y2 = gsGlobal->Height * 0.85 + FONT_HEIGHT;

    int text_line = menu_y1 + 4;

    char options[INGAME_MENU_N][32] = {
        { "State number: " },
        { "Save State" },
        { "Load State" },
        { "Filtering: "},
        { "Aspect Ratio: "},
        { "Sound: " },
        { "Configure Input >" },
        { "Pal: " },
        { "Reset Game" },
        { "Exit Menu" },
        { "Exit Game" },
    };
    char options_state[INGAME_MENU_N][64] = { { 0 } };
    char options_buffer[32+64] = { 0 };

    for (i = 0; i < INGAME_MENU_N; i++) {
        switch (i) {
            case 0:
                sprintf(options_state[i], "%d", statenum);
                break;
            case 3:
                if (Settings.filter)
                    strcpy(options_state[i], "On");
                else
                    strcpy(options_state[i], "Off");
                break;
            case 4:
                if (Settings.aspect_ratio == 0)
                    strcpy(options_state[i], "Full Screen");
                else if (Settings.aspect_ratio == 1)
                    strcpy(options_state[i], "Best Fit (4:3)");
                break;
            case 5:
                if (!Settings.sound)
                    strcpy(options_state[i], "Off");
                else
                    sprintf(options_state[i], "%dHz", SND_GetCurrSampleRate());
                break;
            case 7:
                if (Settings.current_palette == 0)
                    strcpy(options_state[i], "Default (FCEU)");
                else
                    strcpy(options_state[i], palettes[Settings.current_palette - 1].desc);
                break;
        }
    }

#ifdef SOUND_ON
    audsrv_stop_audio();
#endif
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsGlobal->DrawOrder = GS_PER_OS;

    while (1) {
        selected = 0; // Clear selected flag
        selection += menu_input(0, 0);

        if (selection >= INGAME_MENU_N) { selection = 0; }
        if (selection < 0) { selection = INGAME_MENU_N - 1; }

        if (oldselect != selection || option_changed) {
            i = 0x10000;
            while (i--) asm("nop\nnop\nnop\nnop");
            gsKit_queue_reset(gsGlobal->Os_Queue);

            option_changed = 0;

            if (selection == INGAME_MENU_PALETTE_I) {
                int y1 = text_line + INGAME_MENU_PALETTE_I*FONT_HEIGHT;
                int y2 = text_line + (INGAME_MENU_PALETTE_I+1)*FONT_HEIGHT;
                gsKit_prim_quad_gouraud(gsGlobal,
                    menu_x1, y1, menu_x2, y1, menu_x1, y2, menu_x2, y2,
                    2,
                    FCEUSkin.bgColor1, FCEUSkin.bgColor2, FCEUSkin.bgColor3, FCEUSkin.bgColor4);
                strcpy(options_buffer, options[INGAME_MENU_PALETTE_I]);
                strcat(options_buffer, options_state[INGAME_MENU_PALETTE_I]);
                printXY(options_buffer, menu_x1+10, y1, 4, FCEUSkin.highlight, 1, 0);
            }
            else {
                menu_primitive("Options", &MENU_TEX, menu_x1, menu_y1, menu_x2, menu_y2);

                for (i = 0; i < INGAME_MENU_N; i++) {
                    strcpy(options_buffer, options[i]);
                    strcat(options_buffer, options_state[i]);
                    if (selection == i) {
                        //font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, DarkYellowFont, options[i]);
                        printXY(options_buffer, menu_x1+10, text_line + i*FONT_HEIGHT, 4, FCEUSkin.highlight, 1, 0);
                    }
                    else {
                        //font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, WhiteFont, options[i]);
                        printXY(options_buffer, menu_x1+10, text_line + i*FONT_HEIGHT, 4, FCEUSkin.textcolor, 1, 0);
                    }
                }
            }

            DrawScreen(gsGlobal);
        }

        oldselect = selection;

        if (selected) {
            if (selected == 2) { // Menu combo pressed again
                selection = INGAME_MENU_EXIT_I;
            }
            i = selection;
            switch (i) {
                case 0: // State Number
                    statenum++;
                    if (statenum >= 10) { statenum = 0; }
                    sprintf(options_state[i], "%d", statenum);
                    FCEUI_SelectState(statenum);
                    option_changed = 1;
                    break;
                case 1:
                    FCEUI_SaveState(NULL);
                    SetupNESGS();
                    return;
                case 2:
                    FCEUI_LoadState(NULL);
                    SetupNESGS();
                    return;
                case 3:
                    Settings.filter ^= 1;
                    if (Settings.filter) {
                        strcpy(options_state[i], "On");
                    }
                    else {
                        strcpy(options_state[i], "Off");
                    }
                    option_changed = 1;
                    break;
                case 4:
                    Settings.aspect_ratio++;
                    if (Settings.aspect_ratio >= 2) {
                        Settings.aspect_ratio = 0;
                    }
                    if (Settings.aspect_ratio == 0)
                        strcpy(options_state[i], "Full Screen");
                    else if (Settings.aspect_ratio == 1)
                        strcpy(options_state[i], "Best Fit (4:3)");
                    option_changed = 1;
                    break;
                case 5:
                    SND_SetNextSampleRate();
                    if (!Settings.sound)
                        strcpy(options_state[i], "Off");
                    else
                        sprintf(options_state[i], "%dHz", SND_GetCurrSampleRate());
                    option_changed = 1;
                    break;
                case 6:
                    Ingame_Menu_Controls();
                    break;
                case 7:
                    Settings.current_palette++;
                    if (Settings.current_palette >= MAXPAL) { Settings.current_palette = 0; }
                    if (Settings.current_palette == 0)
                        strcpy(options_state[i], "Default (FCEU)");
                    else
                        strcpy(options_state[i], palettes[Settings.current_palette - 1].desc);
                    SetupNESClut();
                    gsKit_texture_upload(gsGlobal, &NES_TEX);
                    option_changed = 1;
                    break;
                case 8:
                    FCEUI_ResetNES();
                    SetupNESGS();
                    return;
                case 9:
                    SetupNESGS();
                    return;
                case 10:
                    fdsswap = 0;
                    statenum = 0;
                    exitgame = 1;
                    selected = 0;
                    return;
            }
        }
    }
}

static int menu_input_controls(int port, int is_changing_button, u32 *new_button)
{
    int ret[2];
    u32 paddata[2];
    u32 new_pad[2];
    u16 slot = 0;

    int change = 0;

    // Check to see if pads are disconnected
    ret[port] = padGetState(0, slot);
    if ((ret[port] != PAD_STATE_STABLE) && (ret[port] != PAD_STATE_FINDCTP1)) {
        if (ret[port] == PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", 0, slot);
        }
        ret[port] = padGetState(0, slot);
    }
    ret[port] = padRead(0, slot, &buttons[port]); // port, slot, buttons
    if (ret[port] != 0) {
        paddata[port]= 0xffff ^ buttons[port].btns;
        new_pad[port] = paddata[port] & ~old_pad[port]; // Buttons pressed AND NOT buttons previously pressed
        old_pad[port] = paddata[port];

        if (new_pad[port]) {
            if (is_changing_button) {
                *new_button = new_pad[port];
                selected = 1;
            }
            else {
                if (new_pad[port] & PAD_DOWN) {
                    change = 1;
                }
                if (new_pad[port] & PAD_UP) {
                    change = -1;
                }
                if (new_pad[port] & PAD_CIRCLE) {
                    selected = 1;
                }
                if ((new_pad[port] == Settings.PlayerInput[port][0]
                || new_pad[port] == PAD_TRIANGLE)) {
                    selected = 2;
                }
            }
        }
    }
    return change;
}

static void padbuttonToStr(u16 button, char button_name[9])
{
    if (button == 0) {
        strcpy(button_name, "---");
        return;
    }
    int i;
    for (i = 0; i < 16; i++) {
        if (button & (1 << i)) {
            break;
        }
    }
    char *buttons[16] = {
        "Select", "L3"   , "R3"  , "Start",
        "Up       \xFF""=",
        "Right    \xFF"":",
        "Down     \xFF"";",
        "Left     \xFF""<" ,
        "L2"    , "R2"   , "L1"  , "R1"   ,
        "Triangle \xFF""3",
        "Circle   \xFF""0",
        "Cross    \xFF""1",
        "Square   \xFF""2"
    };
    strcpy(button_name, buttons[i]);
}

#define CONTROLS_N 14
#define CONTROLS_OFFSET (CONTROLS_N - 10)

static void Ingame_Menu_Controls()
{
    int i, b, selection = 0;
    oldselect = -1;

    int option_changed = 0;

    int menu_x1 = gsGlobal->Width  * 0.25;
    int menu_y1 = gsGlobal->Height * 0.15;
    int menu_x2 = gsGlobal->Width  * 0.75;
    int menu_y2 = gsGlobal->Height * 0.85 + FONT_HEIGHT;

    int text_line = menu_y1 + 4;

    char options[CONTROLS_N][32] = {
        { "< Back" },
        { "Autofire Pattern: "},
        { "4-Players Adaptor: " },
        { "Player: "},
        { "Joy A       | " },
        { "Joy B       | " },
        { "Joy Select  | " },
        { "Joy Start   | " },
        { "Joy Up      | " },
        { "Joy Down    | " },
        { "Joy Left    | " },
        { "Joy Right   | " },
        { "Joy Turbo A | " },
        { "Joy Turbo B | " }
    };
    char options_state[CONTROLS_N][16] = { { 0 } };
    char options_buffer[32+16] = { 0 };

    int player = 0;
    int is_changing_button = 0;
    u32 new_button = 0;

    sprintf(options_state[1], "1 on, %d off", Settings.autofire_pattern + 1);
    if (Settings.input_4p_adaptor)
        strcpy(options_state[2], "On");
    else
        strcpy(options_state[2], "Off");
    strcpy(options_state[3], "1");
    for (i = 0; i < 10; i++) {
        padbuttonToStr(Settings.PlayerInput[player][i + 5], options_state[i + CONTROLS_OFFSET]);
    }

    while (1) {
        selected = 0; // Clear selected flag
        selection += menu_input_controls(0, is_changing_button, &new_button);

        if (selection >= CONTROLS_N) { selection =  0; }
        if (selection < 0) { selection = CONTROLS_N - 1; }

        if (oldselect != selection || option_changed) {
            i = 0x10000;
            while (i--) asm("nop\nnop\nnop\nnop");
            gsKit_queue_reset(gsGlobal->Os_Queue);

            option_changed = 0;

            menu_primitive("Controls", &MENU_TEX, menu_x1, menu_y1, menu_x2, menu_y2);

            for (i = 0; i < CONTROLS_N; i++) {
                strcpy(options_buffer, options[i]);
                strcat(options_buffer, options_state[i]);
                if (selection == i) {
                    printXY(options_buffer, menu_x1+10, FONT_HEIGHT + text_line + i*FONT_HEIGHT, 4, FCEUSkin.highlight, 1, 0);
                }
                else {
                    printXY(options_buffer, menu_x1+10, FONT_HEIGHT + text_line + i*FONT_HEIGHT, 4, FCEUSkin.textcolor, 1, 0);
                }
            }

            DrawScreen(gsGlobal);
        }

        oldselect = selection;

        if (selected) {
            if (selected == 2) {
                return;
            }
            i = selection;

            if (i == 0) {
                return;
            }
            else if (i == 1) {
                Settings.autofire_pattern++;
                if (Settings.autofire_pattern >= 5) {
                    Settings.autofire_pattern = 0;
                }
                sprintf(options_state[1], "1 on, %d off", Settings.autofire_pattern + 1);
                option_changed = 1;
            }
            else if (i == 2) {
                Settings.input_4p_adaptor ^= 1;
                if (Settings.input_4p_adaptor) {
                    FCEUI_SetInputFC(SIFC_4PLAYER, NULL, 0);
                    strcpy(options_state[i], "On");
                }
                else {
                    FCEUI_SetInputFC(SIFC_NONE, NULL, 0);
                    strcpy(options_state[i], "Off");
                }
                option_changed = 1;
            }
            else if (i == 3) {
                player++;
                if (player >= 4) {
                    player = 0;
                }
                sprintf(options_state[i], "%d", player + 1);
                for (b = 0; b < 10; b++) {
                    padbuttonToStr(Settings.PlayerInput[player][b + 5], options_state[b + CONTROLS_OFFSET]);
                }
                option_changed = 1;
            }
            else {
                if (!is_changing_button) {
                    strcpy(options_state[i], "<Press Button>");
                }
                else {
                    i -= CONTROLS_OFFSET;

                    // Skip special buttons
                    int is_safe = 1;
                    for (b = 0; b < 5 && player == 0; b++) {
                        if (Settings.PlayerInput[0][b] == (u16)new_button) {
                            is_safe = 0;
                            break;
                        }
                    }
                    if (is_safe) {
                        Settings.PlayerInput[player][i + 5] = (u16)new_button;
                        // Resolve conflict
                        for (b = 0; b < 10; b++) {
                            if (b != i && Settings.PlayerInput[player][i + 5] == Settings.PlayerInput[player][b + 5]) {
                                Settings.PlayerInput[player][b + 5] = 0;
                                padbuttonToStr(0, options_state[b + CONTROLS_OFFSET]);
                                break;
                            }
                        }
                    }
                    padbuttonToStr(Settings.PlayerInput[player][i + 5], options_state[i + CONTROLS_OFFSET]);
                }
                is_changing_button ^= 1;
                option_changed = 1;
            }
        }
    }
}
