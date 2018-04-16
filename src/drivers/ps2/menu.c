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
extern int defaultx;
extern int defaulty;
// Skin
extern skin FCEUSkin;
extern u8 menutex;
extern u8 bgtex;
// Input
extern char control_name[256];
extern char path[4096];
extern char partitions[4][256];
extern u8 h;

// Palette for FCEU
#define MAXPAL 29
struct st_palettes {
    char name[32];
    char desc[32];
    unsigned int data[64];
};
extern struct st_palettes palettes[];

int statenum = 0;
u8 power_off = 0;

/************************************/
/* gsKit Variables                  */
/************************************/
extern GSGLOBAL *gsGlobal;
extern GSFONTM *gsFontM;
extern GSTEXTURE BG_TEX;
extern GSTEXTURE MENU_TEX;

/************************************/
/* Pad Variables                    */
/************************************/
extern u32 old_pad[2];
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
        GS_SETREG_RGBA(0x80, 0x80, 0x80, 0x80) /* RGBA */
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

int menu_input(int port, int center_screen)
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
    if ((center_screen && change) || (center_screen == 2)) {
        gsGlobal->StartX = defaultx + Settings.offset_x;
        gsGlobal->StartY = defaulty + Settings.offset_y;

        normalize_screen();

        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x00));

        menu_primitive("Centering", &BG_TEX, 0, 0, gsGlobal->Width, gsGlobal->Height);

        DrawScreen(gsGlobal);

        center_screen = 1;
    }
    return change;
}

/** Browser Menu
        Display:         PAL/NTSC
        Interlace:       On/Off
        Emulated System: PAL/NTSC
        Center Screen
        Configure Save Path: (browse to path)
        Configure Elf Path: (browse to path)
        Exit to Elf Path
        Exit to PS2Browser
**/
int Browser_Menu()
{
    char cnfpath[2048];
    int i, selection = 0;
    oldselect = -1;
    int option_changed = 0;

    int menu_x1 = gsGlobal->Width*0.25;
    int menu_y1 = gsGlobal->Height*0.15;
    int menu_x2 = gsGlobal->Width*0.75;
    int menu_y2 = gsGlobal->Height*0.85 + FONT_HEIGHT;
    int text_line = menu_y1 + 4;

    char options[12][32] = {
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

    char options_state[12][64] = { { 0 } };

    // Fill lines with values
    for (i = 0; i < 12; i++) {
        switch (i) {
            case 0:
                if (!Settings.display) {
                    strcpy(options_state[i], "NTSC");
                }
                else {
                    strcpy(options_state[i], "PAL");
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

        if (selection > 11) { selection = 0; }
        if (selection < 0) { selection = 11; }
        if (selection == 5 && oldselect == 4) { selection++; } // 5 is savepath
        if (selection == 5 && oldselect == 6) { selection--; }
        if (selection == 7 && oldselect == 6) { selection++; } // 7 is elfpath
        if (selection == 7 && oldselect == 8) { selection--; }


        if ((oldselect != selection) || option_changed) {

            gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00));

            menu_primitive("Options", &MENU_TEX, menu_x1, menu_y1, menu_x2, menu_y2);

            for (i = 0; i < 12; i++) {
                char buffer[32+64];
                strcpy(buffer, options[i]);
                strcat(buffer, options_state[i]);
                if (selection == i) {
                    //font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, DarkYellowFont, options[i]);
                    printXY(buffer, menu_x1+10, text_line+i*FONT_HEIGHT, 4, FCEUSkin.highlight, 1, 0);
                }
                else {
                    //font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, WhiteFont, options[i]);
                    printXY(buffer, menu_x1+10, text_line + i*FONT_HEIGHT, 4, FCEUSkin.textcolor, 1, 0);
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
                selection = 11;
            }
            i = selection;
            switch (i) {
                case 0: // Display PAL/NTSC
                    Settings.display ^= 1;
                    if (Settings.display) {
                        gsGlobal->Mode = GS_MODE_PAL;
                        gsGlobal->Height = 512;
                        defaulty = 72;
                        strcpy(options_state[i], "PAL");
                    }
                    else {
                        gsGlobal->Mode = GS_MODE_NTSC;
                        gsGlobal->Height = 448;
                        defaulty = 50;
                        strcpy(options_state[i], "NTSC");
                    }
                    gsGlobal->Width = 640;
                    gsGlobal->Field = GS_FIELD;
                    if (gsGlobal->Interlace == GS_NONINTERLACED) {
                        gsGlobal->Height = gsGlobal->Height/2;
                        gsGlobal->StartY = gsGlobal->StartY/2 -1 ;
                    }

                    gsGlobal->StartY = gsGlobal->StartY + Settings.offset_y;
                    //if (Settings.interlace && (gsGlobal->Mode == GS_MODE_NTSC))
                        //gsGlobal->StartY = gsGlobal->StartY + 22;
                    //else
                        //gsGlobal->StartY = gsGlobal->StartY + 11;
                    //normalize_screen();
                    gsKit_init_screen(gsGlobal);    /* Apply settings. */
                    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

                    menu_x1 = gsGlobal->Width*0.25;
                    menu_y1 = gsGlobal->Height*0.15;
                    menu_x2 = gsGlobal->Width*0.75;
                    menu_y2 = gsGlobal->Height*0.85 + FONT_HEIGHT;
                    text_line = menu_y1 + 4;

                    option_changed = 1;
                    //SetGsCrt(gsGlobal->Interlace, gsGlobal->Mode, gsGlobal->Field);
                    break;
                case 1: // Interlacing Off/On
                    Settings.interlace ^= 1;
                    if (gsGlobal->Mode == GS_MODE_PAL)
                        gsGlobal->Height = 512;
                    else
                        gsGlobal->Height = 448;
                    if (Settings.interlace) {
                        gsGlobal->Interlace = GS_INTERLACED;
                        //gsGlobal->StartY = (gsGlobal->StartY-1)*2;
                        strcpy(options_state[i], "On");
                    }
                    else {
                        gsGlobal->Interlace = GS_NONINTERLACED;
                        gsGlobal->StartY = gsGlobal->StartY/2 + 1;
                        gsGlobal->Height = gsGlobal->Height/2;
                        strcpy(options_state[i], "Off");
                    }
                    gsGlobal->Width = 640;
                    gsGlobal->Field = GS_FIELD;
                    //normalize_screen();
                    gsKit_init_screen(gsGlobal);    /* Apply settings. */
                    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

                    menu_x1 = gsGlobal->Width*0.25;
                    menu_y1 = gsGlobal->Height*0.15;
                    menu_x2 = gsGlobal->Width*0.75;
                    menu_y2 = gsGlobal->Height*0.85 + FONT_HEIGHT;
                    text_line = menu_y1 + 4;
                    option_changed = 1;
                    //SetGsCrt(gsGlobal->Interlace, gsGlobal->Mode, gsGlobal->Field);
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
                    if (Settings.display)
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

/** Ingame_Menu
        State number: 0
        Save State
        Load State
        Display Settings -> Center Screen
                            Filtering (Bilinear/Nearest)
                            Interlace Off (can be saved to config.cnf)
        Set Input "DisplayName" (pushing left or right here cycles though 0-9, which parses the control(number).cnf)
        Exit Game (Saves Sram)
**/
extern void SetupNESGS();
extern void SetupNESClut();

extern void SND_SetNextSampleRate();
extern  int SND_GetCurrSampleRate();

void Ingame_Menu()
{
    int i, selection = 0;
    oldselect = -1;

    int option_changed = 0;

    int menu_x1 = gsGlobal->Width*0.25;
    int menu_y1 = gsGlobal->Height*0.15;
    int menu_x2 = gsGlobal->Width*0.75;
    int menu_y2 = gsGlobal->Height*0.85 + FONT_HEIGHT;

    int text_line = menu_y1 + 4;

    char options[14][32] = {
        { "State number: " },
        { "Save State" },
        { "Load State" },
        { "Filtering: "},
        { "Aspect Ratio: "},
        { "Sound: " },
        { "4-Players Adaptor: " },
        { "---" },
        { "---" },
        { "Reset Game" },
        { "Exit Game" },
        { "Exit Menu" },
        { "Palette:" },
        { "" }
    };

    char options_state[14][64] = { { 0 } };

    for (i = 0; i < 14; i++) {
        switch (i) {
            case 0:
                sprintf(options_state[i], "%d", statenum);
                break;
            case 3:
                if (!Settings.filter)
                    strcpy(options_state[i], "Off");
                else
                    strcpy(options_state[i], "On");
                break;
            case 4:
                if (Settings.aspect_ratio == 0)
                    strcpy(options_state[i], "Full Screen");
                else if (Settings.aspect_ratio == 1)
                    strcpy(options_state[i], "Best Fit (4:3 NTSC)");
                break;
            case 5:
                if (!Settings.sound)
                    strcpy(options_state[i], "Off");
                else
                    sprintf(options_state[i], "%dHz", SND_GetCurrSampleRate());
                break;
            case 6:
                if (!Settings.input_4p_adaptor)
                    strcpy(options_state[i], "Off");
                else
                    strcpy(options_state[i], "On");
                break;
            case 7:
                break;
            case 8:
                break;
            case 13:
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

        if (selection == 12 && oldselect == 11) { selection++; } // 12 is palette
        if (selection == 12 && oldselect == 13) { selection--; }
        if (selection > 13) { selection = 0; }
        if (selection < 0) { selection = 13; }

        if (oldselect != selection || option_changed) {
            i = 0x10000;
            while (i--) asm("nop\nnop\nnop\nnop");
            gsKit_queue_reset(gsGlobal->Os_Queue);

            option_changed = 0;

            menu_primitive("Options", &MENU_TEX, menu_x1, menu_y1, menu_x2, menu_y2);

            for (i = 0; i < 14; i++) {
                char buffer[32+64];
                strcpy(buffer, options[i]);
                strcat(buffer, options_state[i]);
                if (selection == i) {
                    //font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, DarkYellowFont, options[i]);
                    printXY(buffer, menu_x1+10, text_line + i*FONT_HEIGHT, 4, FCEUSkin.highlight, 1, 0);
                }
                else {
                    //font_print(gsGlobal, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, WhiteFont, options[i]);
                    printXY(buffer, menu_x1+10, text_line + i*FONT_HEIGHT, 4, FCEUSkin.textcolor, 1, 0);
                }
            }

            DrawScreen(gsGlobal);
        }

        oldselect = selection;

        if (selected) {
            if (selected == 2) { // Menu combo pressed again
                selection = 11;
            }
            i = selection;
            switch (i) {
                case 0: // State Number
                    statenum++;
                    if (statenum > 9) { statenum = 0; }
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
                    if (Settings.aspect_ratio > 1) {
                        Settings.aspect_ratio = 0;
                    }
                    if (Settings.aspect_ratio == 0)
                        strcpy(options_state[i], "Full Screen");
                    else if (Settings.aspect_ratio == 1)
                        strcpy(options_state[i], "Best Fit (4:3 NTSC)");
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
                    break;
                case 7:
                    break;
                case 8:
                    break;
                case 9:
                    FCEUI_ResetNES();
                    SetupNESGS();
                    return;
                case 10:
                    fdsswap = 0;
                    statenum = 0;
                    exitgame = 1;
                    selected = 0;
                    return;
                case 11:
                    SetupNESGS();
                    return;
                case 13:
                    Settings.current_palette++;
                    if (Settings.current_palette > MAXPAL) { Settings.current_palette = 1; }
                    strcpy(options_state[i], palettes[Settings.current_palette - 1].desc);
                    SetupNESClut();
                    option_changed = 1;
                    break;
            }
        }
    }
}
