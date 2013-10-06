#include <stdio.h>
#include <fileio.h>
#include <io_common.h>
#include <sys/stat.h>
#include <libpad.h>
#include <dmaKit.h>
#include <gsKit.h>

#ifdef SOUND_ON
#include <audsrv.h>
#endif

#include "../../driver.h"
#include "ps2fceu.h"

extern vars Settings;
extern entries *FileEntry;
int statenum=0;

/************************************/
/* gsKit Variables                  */
/************************************/
extern GSGLOBAL *gsGlobal;
extern GSFONT *gsFont;

/************************************/
/* Pad Variables                    */
/************************************/
extern u32 old_pad[2];
struct padButtonStatus buttons[2];
extern int selected;
extern int oldselect;
extern int selected_dir;
extern int exitgame;

static inline char* strzncpy(char *d, char *s, int l) { d[0] = 0; return strncat(d, s, l); }

void menu_background( float x1, float y1, float x2, float y2)
{
    int thickness = 3;
    u64 White = GS_SETREG_RGBAQ(0xFF,0xFF,0xFF,0x00,0x00);
    u64 Black = GS_SETREG_RGBAQ(0x00,0x00,0x00,0x00,0x00);
    u64 Blue = GS_SETREG_RGBAQ(0x00,0x00,0xFF,0x00,0x00);
    u64 DarkBlue = GS_SETREG_RGBAQ(0x00,0x00,0x80,0x00,0x00);

    //border
    gsKit_prim_sprite(gsGlobal, x1, y1, x2, y1+thickness, 1, White); //top
    gsKit_prim_sprite(gsGlobal, x1, y1, x1+thickness, y2, 1, White); //left
    gsKit_prim_sprite(gsGlobal, x2-thickness, y1, x2, y2, 1, White); //right
    gsKit_prim_sprite(gsGlobal, x1, y2-thickness, x2, y2, 1, White); //bottom

    //background
    gsKit_prim_quad_gouraud(gsGlobal, x1+thickness, y1+thickness,
                                      x2-thickness,  y1+thickness,
                                       x1+thickness, y2-thickness,
                                      x2-thickness, y2-thickness,
                                       1, Blue, DarkBlue, DarkBlue, Black);

    //title

}

void menu_primitive( char *title, float x1, float y1, float x2, float y2)
{
    menu_background(x1,y1,x2,y2);
    menu_background(x2-(strlen(title)*15)/*0.78*/, y1, x2, y1+FONT_HEIGHT*2);
    gsFont->FontM_Align = GSKIT_FALIGN_RIGHT;
    gsKit_font_print_scaled(gsGlobal, gsFont, x2-3, y1+FONT_HEIGHT/2, 1, 0.5f, GS_SETREG_RGBAQ(0x80,0x80,0x80,0xFF,0x00), title);
    gsFont->FontM_Align = GSKIT_FALIGN_LEFT;
}

int menu_input(int port, int center_screen)
{
    int ret[2];
    u32 paddata[2];
    u32 new_pad[2];
    u16 slot = 0;

    int change = 0;

    //check to see if pads are disconnected
    ret[port]=padGetState(0, slot);
    if((ret[port] != PAD_STATE_STABLE) && (ret[port] != PAD_STATE_FINDCTP1)) {
        if(ret[port]==PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", 0, slot);
        }
        ret[port]=padGetState(0, slot);
    }
    ret[port] = padRead(0, slot, &buttons[port]); // port, slot, buttons
    if (ret[port] != 0) {
        paddata[port]= 0xffff ^ buttons[port].btns;
        new_pad[port] = paddata[port] & ~old_pad[port]; // buttons pressed AND NOT buttons previously pressed
        old_pad[port] = paddata[port];

        if(paddata[port] & PAD_LEFT && center_screen) {
            Settings.offset_x--;
            change = 1;
        }
        if(new_pad[port] & PAD_DOWN && !center_screen) {
            change = 1;
        }
        if(paddata[port] & PAD_DOWN && center_screen) {
            Settings.offset_y++;
            change = 1;
        }
        if(paddata[port] & PAD_RIGHT && center_screen) {
            Settings.offset_x++;
            change = 1;
        }
        if(new_pad[port] & PAD_UP && !center_screen) {
            change = -1;
        }
        if(paddata[port] & PAD_UP && center_screen) {
            Settings.offset_y--;
            change = 1;
        }
        if(new_pad[port] & PAD_START && center_screen) {
            change = 2;
        }
        if(new_pad[port] & PAD_SELECT && center_screen) {
            if(Settings.display) { //PAL
                Settings.offset_x = 680;
                Settings.offset_y = 72;
            }
            else { //NTSC
                Settings.offset_x = 652;
                Settings.offset_y = 50;
            }
            change = 1;
        }
        if(new_pad[port] & PAD_CIRCLE) {
            selected = 1;
        }
        if(new_pad[port] & PAD_CROSS) {
        }
        /*if(paddata[0] & PAD_TRIANGLE) {
            //exit(0);// for code profiling
            exitgame = 1;
        }
        if(new_pad[0] & PAD_L1) { //restrict savestate controls for first player
            statenum--;
            if(statenum < 0) {statenum = 9;}
            FCEUI_SelectState(statenum);
        }
        if(new_pad[0] & PAD_R1) {
            statenum++;
            if(statenum > 9) {statenum = 0;}
            FCEUI_SelectState(statenum);
        }
        if(new_pad[0] & PAD_L2) {
            FCEUI_LoadState(NULL);
        }
        if(new_pad[0] & PAD_R2) {
            FCEUI_SaveState(NULL);
        }*/
    }
    if((center_screen && change) || (center_screen == 2)) {
        gsGlobal->StartX = Settings.offset_x;
        gsGlobal->StartY = Settings.offset_y;

        GS_SET_DISPLAY1(gsGlobal->StartX,		// X position in the display area (in VCK unit
			gsGlobal->StartY,		// Y position in the display area (in Raster u
			gsGlobal->MagX,			// Horizontal Magnification
			gsGlobal->MagY,			// Vertical Magnification
			(gsGlobal->Width * 4) -1,	// Display area width
			(gsGlobal->Height-1));		// Display area height

	    GS_SET_DISPLAY2(gsGlobal->StartX,		// X position in the display area (in VCK units)
			gsGlobal->StartY,		// Y position in the display area (in Raster units)
			gsGlobal->MagX,			// Horizontal Magnification
			gsGlobal->MagY,			// Vertical Magnification
			(gsGlobal->Width * 4) -1,	// Display area width
			(gsGlobal->Height-1));		// Display area height

        gsKit_clear(gsGlobal,GS_SETREG_RGBAQ(0x00,0x00,0x00,0x00,0x00));

        menu_primitive("Centering", 0, 0, gsGlobal->Width, gsGlobal->Height);

        gsKit_sync_flip(gsGlobal);

        gsKit_queue_exec(gsGlobal);

        center_screen = 1;
    }
  return change;
}

/** Browser Menu
        Display:         PAL/NTSC
        Emulated System: PAL/NTSC
        Center Screen
        Configure Save Path: (browse to path)
        Configure Elf Path: (browse to path)
        Exit to Elf Path
        Exit to PS2Browser
**/
int Browser_Menu(void)
{
    char *temp;
    char cnfpath[2048];
    int i,selection = 0;
    oldselect = -1;
    int option_changed = 0;

    float menu_x1 = gsGlobal->Width*0.25;
    float menu_y1 = gsGlobal->Height*0.25;
    float menu_x2 = gsGlobal->Width*0.75;
    float menu_y2 = gsGlobal->Height*0.75;

    float text_line = menu_y1 + FONT_HEIGHT*3;

    u64 WhiteFont = GS_SETREG_RGBAQ(0x80,0x80,0x80,0xFF,0x00);
    u64 DarkYellowFont = GS_SETREG_RGBAQ(0x80,0x80,0x40,0xFF,0x00);
    u64 Black = GS_SETREG_RGBAQ(0x00,0x00,0x00,0x00,0x00);

    char options[11][22] = {
        { "Display: " },
        { "Interlacing: " },
        { "Emulated System: " },
        { "Center Screen" },
        { "Configure Save Path: " },
        { "" },
        { "Configure ELF Path:  " },
        { "" },
        { "Save FCEUltra.cnf"},
        { "Exit to ELF" },
        { "Exit Options Menu" }
    };

    //fill lines with values
    for(i=0;i<11;i++) {
        switch(i) {
            case 0:
                if(!Settings.display) {
                    sprintf(options[0],"%s%s",options[0],"NTSC");
                }
                else {
                    sprintf(options[0],"%s%s",options[0],"PAL");
                }
                break;
            case 1:
                if(!Settings.interlace) {
                    sprintf(options[1],"%s%s",options[1],"Off");
                }
                else {
                    sprintf(options[1],"%s%s",options[1],"On");
                }
                break;
            case 2:
                if(!Settings.emulation) {
                    sprintf(options[2],"%s%s",options[2],"NTSC");
                }
                else {
                    sprintf(options[2],"%s%s",options[2],"PAL");
                }
                break;
            case 5:
                strzncpy(options[5],Settings.savepath,21);
                break;
            case 7:
                strzncpy(options[7],Settings.elfpath,21);
                break;
        }
    }



    while(1) {
        selected = 0; //clear selected flag
        selection += menu_input(0,0);

        if(selection > 10) { selection = 0; }
        if(selection < 0) { selection = 10; }


        gsKit_queue_reset(gsGlobal->Os_Queue);
        gsKit_clear(gsGlobal,Black);

        //menu_primitive("Browser", 0, 0, gsGlobal->Width, gsGlobal->Height);


        if((oldselect != selection) || option_changed) {
            menu_primitive("Options", menu_x1, menu_y1, menu_x2, menu_y2);

            for(i=0;i<11;i++) {
                if(selection == i) {
                    //if(i != 4 || 5)
                        gsKit_font_print_scaled(gsGlobal, gsFont, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, 0.5f, DarkYellowFont, options[i]);
                    //gsKit_font_print(gsGlobal, gsFont, 10.0f, i*FONT_HEIGHT, 2, YellowFont, FileEntry[i].displayname);
                }
                else {
                    gsKit_font_print_scaled(gsGlobal, gsFont, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, 0.5f, WhiteFont, options[i]);
                    //gsKit_font_print(gsGlobal, gsFont, 10.0f, i*FONT_HEIGHT, 1, WhiteFont, FileEntry[i].displayname);
                }
            }
            gsKit_sync_flip(gsGlobal);

            gsKit_queue_exec(gsGlobal);
        }

        oldselect = selection;
        option_changed = 0;

        if(selected) {
            switch(selection) {
                case 0: //Display PAL/NTSC
                    Settings.display ^= 1;
                    if(Settings.display) {
                        gsGlobal->Mode = GS_MODE_PAL;
                        temp = strstr(options[0],"NTSC");
                        *temp = 0;
                        strcat(options[0],"PAL");
                    }
                    else {
                        gsGlobal->Mode = GS_MODE_NTSC;
                        temp = strstr(options[0],"PAL");
                        *temp = 0;
                        strcat(options[0],"NTSC");
                    }
                    option_changed = 1;
                    SetGsCrt(gsGlobal->Interlace,gsGlobal->Mode,gsGlobal->Field);
                    break;
                case 1: //Interlacing Off/On
                    Settings.interlace ^= 1;
                    if(Settings.interlace) {
                        gsGlobal->Field = GS_FRAME;
                        gsGlobal->DoSubOffset = GS_SETTING_OFF;
                        temp = strstr(options[1],"Off");
                        *temp = 0;
                        strcat(options[1],"On");
                    }
                    else {
                        gsGlobal->Field = GS_FIELD;
                        gsGlobal->DoSubOffset = GS_SETTING_ON;
                        temp = strstr(options[1],"On");
                        *temp = 0;
                        strcat(options[1],"Off");
                    }
                    option_changed = 1;
                    SetGsCrt(gsGlobal->Interlace,gsGlobal->Mode,gsGlobal->Field);
                    break;
                case 2: //Emulated System
                    Settings.emulation ^= 1;
                    if(Settings.emulation) {
                        temp = strstr(options[2],"NTSC");
                        *temp = 0;
                        strcat(options[2],"PAL");
                    }
                    else {
                        temp = strstr(options[2],"PAL");
                        *temp = 0;
                        strcat(options[2],"NTSC");
                    }
                    FCEUI_SetVidSystem(Settings.emulation);
                    option_changed = 1;
                    break;
                case 3: //Center Screen
                    while(menu_input(0,2) != 2) {}
                    option_changed = 1;
                    break;
                case 4: //Configure Save Path
                    selected = 0;
                    strcpy(Settings.savepath,Browser(0,1));
                    strzncpy(options[5],Settings.savepath,21);
                    selected_dir = 0;
                    selection = 0;
                    oldselect = -1;
                    break;
                case 6: //Configure ELF Path
                    selected = 0;
                    strcpy(Settings.elfpath,Browser(1,1));
                    strzncpy(options[7],Settings.elfpath,21);
                    selection = 0;
                    oldselect = -1;
                    break;
                case 8: //Save CNF
                    sprintf(cnfpath,"%s%s",Settings.savepath,"FCEUltra.cnf");
                    fioMkdir("mc0:FCEUMM");
                    Save_Global_CNF("mc0:/FCEUMM/FCEUltra.cnf");
                    break;
                case 9: //Exit to ELF
                    return 2;
                    break;
                case 10: //Exit Options Menu
                    selected = 0;
                    return 1;
            }
        }
    }

    /*__asm__ __volatile__(
        "	li $3, 0x04;"
        "	syscall;"
        "	nop;"
    );*/
    //

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
void SetupNESGS(void);

void Ingame_Menu(void)
{
    int i,selection = 0;
    oldselect = -1;

    float menu_x1 = gsGlobal->Width*0.25;
    float menu_y1 = gsGlobal->Height*0.25;
    float menu_x2 = gsGlobal->Width*0.75;
    float menu_y2 = gsGlobal->Height*0.75;

    float text_line = menu_y1 + FONT_HEIGHT*3;

    u64 WhiteFont = GS_SETREG_RGBAQ(0x80,0x80,0x80,0xFF,0x00);
    u64 DarkYellowFont = GS_SETREG_RGBAQ(0x80,0x80,0x40,0xFF,0x00);
    u64 Black = GS_SETREG_RGBAQ(0x00,0x00,0x00,0x00,0x00);

    char options[9][17] = {
        { "State number: " },
        { "Save State" },
        { "Load State" },
        { "Filtering: " },
        { "Interlacing: " },
        { "Player 1 Input" },
        { "Player 2 Input" },
        { "Exit Game" },
        { "Exit Menu" },
    };

#ifdef SOUND_ON
    audsrv_stop_audio();
#endif
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsGlobal->DrawOrder = GS_PER_OS;
    gsKit_queue_reset(gsGlobal->Os_Queue);

    while(1) {
        selected = 0; //clear selected flag
        selection += menu_input(0,0);

        if(selection > 8) { selection = 0; }
        if(selection < 0) { selection = 8; }


        //gsKit_clear(gsGlobal,Black);

        //menu_primitive("Browser", 0, 0, gsGlobal->Width, gsGlobal->Height);


        if(oldselect != selection) {
            menu_primitive("Options", menu_x1, menu_y1, menu_x2, menu_y2);

            for(i=0;i<9;i++) {
                if(selection == i) {
                    gsKit_font_print_scaled(gsGlobal, gsFont, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, 0.5f, DarkYellowFont, options[i]);
                    //gsKit_font_print(gsGlobal, gsFont, 10.0f, i*FONT_HEIGHT, 2, YellowFont, FileEntry[i].displayname);
                }
                else {
                    gsKit_font_print_scaled(gsGlobal, gsFont, menu_x1+10.0f, text_line + i*FONT_HEIGHT, 2, 0.5f, WhiteFont, options[i]);
                    //gsKit_font_print(gsGlobal, gsFont, 10.0f, i*FONT_HEIGHT, 1, WhiteFont, FileEntry[i].displayname);
                }
            }
            gsKit_sync_flip(gsGlobal);

            gsKit_queue_exec(gsGlobal);
        }

        if(selected) {
            switch(selection) {
                case 7:
                    exitgame = 1;
                    return;
                case 8:
                    SetupNESGS();
                    return;
            }
        }

        oldselect = selection;
    }
}

