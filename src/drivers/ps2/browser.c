#include <stdio.h>
#include <fileio.h>
#include <io_common.h>
#include <sys/stat.h>
#include <libpad.h>
#include <dmaKit.h>
#include <gsKit.h>

#include "ps2fceu.h"

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

int oldselect = -1;
int selected = 0;
int selected_dir = 0;
char path[4096] = "path";

void Browser_Menu(void);

static inline char* strzncpy(char *d, char *s, int l) { d[0] = 0; return strncat(d, s, l); }

char* browseup(char *path)
{
    char *temp;

    if((temp = strrchr(path,'/')) != NULL) {//temp = address of first / path equals mc0:/folder/folder/
        printf("There is a /\n");
        *temp = 0;//stores (\0) at address of first / and path equals mc0:/folder/folder
        if((temp = strrchr(path,'/')) != NULL) {//temp = address of second /
            printf("There is a /\n");
            temp++; //adjusts address to after second /
            *temp = 0;//stores 0 at address after second / and path now equals mc0:/folder/ or mc0:
        }
        else {
            path[0] = 0;//sets the first char in path to \0
            printf("There is no /\n");
            strcpy(path,"path");
        }
    } else { path[0] = 0; strcpy(path,"path"); }

    return path;
}

int RomBrowserInput(int files_too, int inside_menu)
{
    //was easier just to c&p and replace port with 0
    int ret[2];
    u32 paddata[2];
    u32 new_pad[2];
    u16 slot = 0;

    //check to see if pads are disconnected
    ret[0]=padGetState(0, slot);
    if((ret[0] != PAD_STATE_STABLE) && (ret[0] != PAD_STATE_FINDCTP1)) {
        if(ret[0]==PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", 0, slot);
        }
        ret[0]=padGetState(0, slot);
    }
    ret[0] = padRead(0, slot, &buttons[0]); // port, slot, buttons
    if (ret[0] != 0) {
        paddata[0]= 0xffff ^ buttons[0].btns;
        new_pad[0] = paddata[0] & ~old_pad[0]; // buttons pressed AND NOT buttons previously pressed
        old_pad[0] = paddata[0];

        ret[1]=0;
        if(new_pad[0] & PAD_LEFT) {

        }
        if(new_pad[0] & PAD_DOWN) {
            ret[1] = 1;
        }
        if(new_pad[0] & PAD_RIGHT) {

        }
        if(new_pad[0] & PAD_UP) {
                ret[1] = -1;
        }
        if((paddata[0] & PAD_SELECT) && !inside_menu) {
            Browser_Menu();
            oldselect = -1;
        }
        if(new_pad[0] & PAD_CIRCLE) {
            selected = 1;
        }
        if((new_pad[0] & PAD_START) && inside_menu && !files_too) {
            selected_dir = 1;
        }
    }
  return ret[1];
}

int listdir(char *path, entries *FileEntry, int files_too)
{
    int dd;
    int n = 0;
    fio_dirent_t buf;

    if(!(strchr(path,'/'))) { //if path is not valid then load default device menu
        strcpy(FileEntry[0].displayname,"mc0:");
        strcpy(FileEntry[1].displayname,"mc1:");
        strcpy(FileEntry[2].displayname,"mass:");
        //strcpy(FileEntry[3].displayname,"hdd0:FCEUMM");
        strcpy(FileEntry[0].filename,"mc0:/");
        strcpy(FileEntry[1].filename,"mc1:/");
        strcpy(FileEntry[2].filename,"mass:/");
        //strcpy(FileEntry[3].filename,"hdd0:FCEUMM/");
        FileEntry[0].dircheck = 1;
        FileEntry[1].dircheck = 1;
        FileEntry[2].dircheck = 1;
        //FileEntry[3].dircheck = 1;
        n = 3;
    }
    else { //it has a /
        dd = fioDopen(path);
        if( dd < 0) {
            printf("Didn't open!\n");
            return 0;
        }
        else {
            printf("Directory opened!\n");
            //adds pseudo folder .. to every folder opened as mass: reported none but mc0: did
            strcpy(FileEntry[0].filename,"..");
            strcpy(FileEntry[0].displayname,"..");
            FileEntry[0].dircheck = 1;
            n=1;
            while(fioDread(dd, &buf) > 0) {
		        if((FIO_SO_ISDIR(buf.stat.mode)) && (!strcmp(buf.name,".") || !strcmp(buf.name,"..")))
		            continue; //makes sure no .. or .'s are listed since it's already there
                if(FIO_SO_ISDIR(buf.stat.mode)) {
                    FileEntry[n].dircheck = 1;
                    strcpy(FileEntry[n].filename,buf.name);
                    strzncpy(FileEntry[n].displayname,FileEntry[n].filename,31);
                    n++;
                }

                if ( n == 2046) { break; }
            }
            if(dd >= 0) {
                fioDclose(dd);
                printf("Directory closed!\n");
            }
            if(files_too) {
                dd = 0;
                dd = fioDopen(path);
                //n = n;
                while(fioDread(dd, &buf) > 0) {
                    if(FIO_SO_ISREG(buf.stat.mode)) {
                        FileEntry[n].dircheck = 0;
                        strcpy(FileEntry[n].filename,buf.name);
                        strzncpy(FileEntry[n].displayname,FileEntry[n].filename,31);
                        n++;
                    }
                    if ( n == 2046) { break; }
                }
                if(dd >= 0) {
                    fioDclose(dd);
                    printf("Directory closed!\n");
                }
            }

        }
        printf("listdir path = %s\n",path);
    }
    printf("listdir path = %s\n",path);
    return n;
}

char* Browser(int files_too, int inside_menu)
{
    int i;
    int selection = 0;
    int n = 0;

    oldselect = -1;
    entries FileEntry[2048];

    float fontheight = 10.0f;

    u64 WhiteFont = GS_SETREG_RGBAQ(0x80,0x80,0x80,0xFF,0x00);
    u64 DarkYellowFont = GS_SETREG_RGBAQ(0x80,0x80,0x40,0xFF,0x00);
    u64 Black = GS_SETREG_RGBAQ(0x00,0x00,0x00,0x00,0x00);

    //switch to one shot drawing queue
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsKit_queue_reset(gsGlobal->Os_Queue);
    gsGlobal->DrawOrder = GS_PER_OS; //draw one shot objects last

    strcpy(path,"path");
    char oldpath[4096] = "oldpath";

    while(1) {
        selected = 0; //clear selected flag
        selection += RomBrowserInput(files_too,inside_menu);

        if(strcmp(path,oldpath) != 0) {
            n = listdir(path,FileEntry,files_too); //n == max number of items + empty entry
            if(n == 0) {
                path[0] = 0;
                strcpy(path,"path");
                n = listdir(path,FileEntry,files_too);
            }
            strcpy(oldpath,path);//needed so listdir isn't called every loop
            selection = 0; //set default selection
            oldselect = -1; //so the screen draws on load
            printf("n = %d\n",n);
        }

        if(selection > (n-1)) { selection = 0; }
        if(selection < 0) { selection = n-1; }
        if(selection != oldselect) {
            if(selection > oldselect) {//if selection moves down
                if(selection*FONT_HEIGHT >= (gsGlobal->Height - (int)FONT_HEIGHT)) { //if selection height is > than screen height - a row
                    fontheight -= FONT_HEIGHT; //scroll list up
                    if((oldselect == 0) && (selection == (n-1))) { //take care of browsing from top to bottom
                        fontheight = fontheight - (FONT_HEIGHT * (selection - 27)); //reset list offset to number of max list objects
                    }

                }
            }
            if(selection < oldselect) { //if selection moves up
                fontheight += FONT_HEIGHT; //scroll down
                if((fontheight > 10.0f) || (selection == 0)) { //if offset moves past default offset or selection is 0
                    fontheight = 10.0f;
                }
            }
            if(selection == 0) { //takes care of empty directories
                fontheight = 10.0f;
            }
            gsKit_clear(gsGlobal,Black);

            menu_primitive("Browser", 0, 0, gsGlobal->Width, gsGlobal->Height);
            //menu_primitive("Browser", gsGlobal->Width*0.25,gsGlobal->Height*0.25,gsGlobal->Width*0.75, gsGlobal->Height*0.75);


            for(i=0;i<n;i++) {
                if(selection == i) {
                    gsKit_font_print_scaled(gsGlobal, gsFont, 10.0f, i*FONT_HEIGHT+fontheight, 2, 0.5f, DarkYellowFont, FileEntry[i].displayname);
                    //gsKit_font_print(gsGlobal, gsFont, 10.0f, i*FONT_HEIGHT+fontheight, 1, YellowFont, FileEntry[i].displayname);
                }
                else {
                    if((i*FONT_HEIGHT+fontheight) <= (gsGlobal->Height - (int)FONT_HEIGHT)) //takes care of halfdrawn rows
                        gsKit_font_print_scaled(gsGlobal, gsFont, 10.0f, i*FONT_HEIGHT+fontheight, 2, 0.5f, WhiteFont, FileEntry[i].displayname);
                        //gsKit_font_print(gsGlobal, gsFont, 10.0f, i*FONT_HEIGHT+fontheight, 1, WhiteFont, FileEntry[i].displayname);
                }
            }

            gsKit_sync_flip(gsGlobal);


            gsKit_queue_exec(gsGlobal);
        }

        oldselect = selection;

        if(selected) {
            if(FileEntry[selection].dircheck) {
                if(!strcmp(FileEntry[selection].filename,"..") || !strcmp(FileEntry[selection].filename,".")) {
                    if(!strcmp(FileEntry[selection].filename,"..")) {
                        strcpy(path,browseup(path));
                        printf("browseup path: %s\n",path);
                    }
                }
                else {
                    if(strchr(path,'/') == NULL) { //device selected
                        strcpy(path,FileEntry[selection].filename);
                    }
                    else {
                        //strcat(path,FileEntry[selection].filename);
                        //strcat(path,"/");
                        sprintf(path,"%s%s/",path,FileEntry[selection].filename);
                        printf("path is %s\n",path);
                    }
                }
            }
            if(!FileEntry[selection].dircheck) {
                //strcat(path,FileEntry[selection].filename);
                sprintf(path,"%s%s",path,FileEntry[selection].filename);
                printf("rompath = %s\n", path);
                //free(FileEntry);
                return (char *)path;
            }
        }
        if(selected_dir) {
            return (char *)path;
        }
    }
}
