#include <stdio.h>
#include <fileio.h>
#include <fileXio.h>
#include <fileXio_rpc.h>
#include <libhdd.h>
#include <io_common.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <libpad.h>
#include <dmaKit.h>
#include <gsKit.h>
#ifdef CDSUPPORT
#include <cdvd_rpc.h>
//#include <SMS_CDVD.h>
//#include "../smscdvd/iop/SMSCDVD/SMSCDVD.h"
#include "cd/cd.h"
#endif

#include "ps2fceu.h"


extern vars Settings;
extern skin FCEUSkin;

/************************************/
/* gsKit Variables                  */
/************************************/
extern GSGLOBAL *gsGlobal;
extern GSTEXTURE BG_TEX;
extern GSFONTM *gsFontM;

/************************************/
/* Pad Variables                    */
/************************************/
extern u32 old_pad[2];
static struct padButtonStatus buttons[2];

extern int defaultx, defaulty;
int oldselect = -1;
s8 selected = 0;
u8 selected_dir = 0;
extern int FONT_HEIGHT;
char path[4096] = "path";
int needed_path[2] = { -1, -1 };
char mpartitions[4][256];
u16 history[20]; // 20 levels should be enough
u8 h = 0;

extern int Browser_Menu();

static inline char* strzncpy(char *d, const char *s, size_t l) { d[0] = 0; return strncat(d, s, l); }

char* browseup(char *path)
{
    char *temp;

    if ((temp = strrchr(path, '/')) != NULL) { // temp = address of first / path equals mc0:/folder/folder/
        printf("There is a /\n");
        *temp = 0; // Stores (\0) at address of first / and path equals mc0:/folder/folder
        if ((temp = strrchr(path, '/')) != NULL) { // temp = address of second /
            printf("There is a /\n");
            temp++; // Adjusts address to after second /
            *temp = 0; // Stores 0 at address after second / and path now equals mc0:/folder/ or mc0:
        }
        else {
            path[0] = 0; // Sets the first char in path to \0
            printf("There is no /\n");
            strcpy(path, "path");
        }
    }
    else {
        path[0] = 0;
        strcpy(path, "path");
    }

    return path;
}

int RomBrowserInput(int files_too, int inside_menu)
{
    // Was easier just to c&p and replace port with 0
    int ret[2];
    u32 paddata[2];
    u32 new_pad[2];
    u16 slot = 0;
    ret[1] = 0;

    // Check to see if pads are disconnected
    ret[0] = padGetState(0, slot);
    if ((ret[0] != PAD_STATE_STABLE) && (ret[0] != PAD_STATE_FINDCTP1)) {
        if (ret[0] == PAD_STATE_DISCONN) {
            printf("Pad(%d, %d) is disconnected\n", 0, slot);
        }
        ret[0] = padGetState(0, slot);
    }
    ret[0] = padRead(0, slot, &buttons[0]); // port, slot, buttons
    if (ret[0] != 0) {
        paddata[0]= 0xffff ^ buttons[0].btns;
        new_pad[0] = paddata[0] & ~old_pad[0]; // Buttons pressed AND NOT buttons previously pressed
        old_pad[0] = paddata[0];


        if (new_pad[0] & PAD_LEFT) {

        }
        if (new_pad[0] & PAD_DOWN) {
            ret[1] = 1;
        }
        if (new_pad[0] & PAD_RIGHT) {

        }
        if (new_pad[0] & PAD_UP) {
                ret[1] = -1;
        }
        if ((new_pad[0] & PAD_SELECT) && !inside_menu) {
            ret[0] = Browser_Menu();
            if (ret[0] == 1) {
                oldselect = -3;
            }
            else if (ret[0] == 2) {
                oldselect = -2;
            }
            else {
                oldselect = -1;
            }
        }
        if (new_pad[0] & PAD_TRIANGLE) {
            oldselect = -4;
        }
        if (new_pad[0] & PAD_CIRCLE) {
            selected = 1;
        }
        if ((new_pad[0] & PAD_START) && inside_menu && !files_too) {
            selected_dir = 1;
        }
    }
  return ret[1];
}

#ifdef CDSUPPORT
int listcdvd(const char *path, entries *FileEntry) {
    static struct TocEntry TocEntryList[2048];
    char dir[1025];
    int i, n, t;

    strcpy(dir, &path[5]);
    // Directories first...

    CDVD_FlushCache();
    n = CDVD_GetDir(dir, NULL, CDVD_GET_DIRS_ONLY, TocEntryList, 2048, dir);

    strcpy(FileEntry[0].filename, "..");
    strcpy(FileEntry[0].displayname, "..");
    FileEntry[0].dircheck = 1;
    t = 1;

    for (i = 0; i < n; i++) {
        if (TocEntryList[i].fileProperties & 0x02 && (!strcmp(
                TocEntryList[i].filename, ".") || !strcmp(
                        TocEntryList[i].filename, "..")))
            continue; // Skip pseudopaths "." and ".."

        FileEntry[t].dircheck = 1;
        strcpy(FileEntry[t].filename, TocEntryList[i].filename);
        strzncpy(FileEntry[t].displayname, FileEntry[t].filename, 63);
        t++;

        if (t >= 2046) {
            break;
        }
     }

    // Now files only

    CDVD_FlushCache();
    n = CDVD_GetDir(dir, NULL, CDVD_GET_FILES_ONLY, TocEntryList, 2048, dir);

    for (i = 0; i < n; i++) {
        if (TocEntryList[i].fileProperties & 0x02 && (!strcmp(
                TocEntryList[i].filename, ".") || !strcmp(
                        TocEntryList[i].filename, "..")))
            continue; // Skip pseudopaths "." and ".."

        FileEntry[t].dircheck = 0;
        strcpy(FileEntry[t].filename, TocEntryList[i].filename);
        strzncpy(FileEntry[t].displayname, FileEntry[t].filename, 63);
        t++;

        if (t >= 2046) {
            break;
        }
    }

    return t;
}
#endif

int listdir(char *path, entries *FileEntry, int files_too)
{
    int dd, n = 0;
    fio_dirent_t buf;

    if (!(strchr(path, '/'))) { // If path is not valid then load default device menu
        strcpy(FileEntry[0].displayname, "mc0:");
        strcpy(FileEntry[1].displayname, "mc1:");
        strcpy(FileEntry[2].displayname, "mass:");
        strcpy(FileEntry[3].displayname, "hdd0:");
        strcpy(FileEntry[4].displayname, "cdfs:");
        strcpy(FileEntry[0].filename, "mc0:/");
        strcpy(FileEntry[1].filename, "mc1:/");
        strcpy(FileEntry[2].filename, "mass:/");
        strcpy(FileEntry[3].filename, "hdd0:/");
        strcpy(FileEntry[4].filename, "cdfs:/");
        FileEntry[0].dircheck = 1;
        FileEntry[1].dircheck = 1;
        FileEntry[2].dircheck = 1;
        FileEntry[3].dircheck = 1;
        FileEntry[4].dircheck = 1;
        n = 5;
    }
#ifdef CDSUPPORT
    else if (!strncmp(path, "cdfs", 4)) {
        n = listcdvd(path, FileEntry);
    }
#endif
    else { // It has a /
        dd = fioDopen(path);
        if (dd < 0) {
            printf("Didn't open!\n");
            return 0;
        }
        else {
            printf("Directory opened!\n");
            // Adds pseudo folder .. to every folder opened as mass: reported none but mc0: did
            strcpy(FileEntry[0].filename, "..");
            strcpy(FileEntry[0].displayname, "..");
            FileEntry[0].dircheck = 1;
            n = 1;
            while (fioDread(dd, &buf) > 0) {
                if (n > 2046) { break; }
                if ((FIO_SO_ISDIR(buf.stat.mode)) && (!strcmp(buf.name, ".") || !strcmp(buf.name, "..")))
                    continue; // Makes sure no .. or .'s are listed since it's already there
                if (FIO_SO_ISDIR(buf.stat.mode)) {
                    FileEntry[n].dircheck = 1;
                    strcpy(FileEntry[n].filename, buf.name);
                    strzncpy(FileEntry[n].displayname, FileEntry[n].filename, 63);
                    n++;
                    if (n >= 2046)
                        break;
                }
            }
            if (dd >= 0) {
                fioDclose(dd);
                printf("Directory closed!\n");
            }
            if (files_too) {
                dd = 0;
                dd = fioDopen(path);
                while (fioDread(dd, &buf) > 0) {
                    if (n > 2046) { break; }
                    if (FIO_SO_ISREG(buf.stat.mode)) {
                        FileEntry[n].dircheck = 0;
                        strcpy(FileEntry[n].filename, buf.name);
                        strzncpy(FileEntry[n].displayname, FileEntry[n].filename, 63);
                        n++;
                    }
                }
                if (dd >= 0) {
                    fioDclose(dd);
                    printf("Directory closed!\n");
                }
            }

        }
        printf("listdir path = %s\n", path);
    }
    printf("listdir path = %s\n", path);
    return n;
}

int listpfs(char *path, entries *FileEntry, int files_too)
{
    int dd, n = 0;
    iox_dirent_t buf;

    if (!(strchr(path, '/'))) { // If path is not valid then load default device menu
        strcpy(FileEntry[0].displayname, "mc0:");
        strcpy(FileEntry[1].displayname, "mc1:");
        strcpy(FileEntry[2].displayname, "mass:");
        strcpy(FileEntry[3].displayname, "hdd0:");
        strcpy(FileEntry[4].displayname, "cdfs:");
        strcpy(FileEntry[0].filename, "mc0:/");
        strcpy(FileEntry[1].filename, "mc1:/");
        strcpy(FileEntry[2].filename, "mass:/");
        strcpy(FileEntry[3].filename, "hdd0:/");
        strcpy(FileEntry[4].filename, "cdfs:/");
        FileEntry[0].dircheck = 1;
        FileEntry[1].dircheck = 1;
        FileEntry[2].dircheck = 1;
        FileEntry[3].dircheck = 1;
        n = 4;
    }
#ifdef CDSUPPORT
    else if (!strncmp(path, "cdfs", 4)) {
        n = listcdvd(path, FileEntry);
    }
#endif
    else {
        if ((dd = fileXioDopen(path)) < 0) {
            printf("Didn't open!\n");
            return 0;
        }
        else {
            printf("Directory opened!\n");
            // Adds pseudo folder .. to every folder opened as mass: reported none but mc0: did
            strcpy(FileEntry[0].filename, "..");
            strcpy(FileEntry[0].displayname, "..");
            FileEntry[0].dircheck = 1;
            n = 1;
            while (fileXioDread(dd, &buf) > 0) {
                if (buf.stat.mode & FIO_S_IFDIR && (!strcmp(buf.name, ".") || !strcmp(buf.name, "..")))
                    continue;
                if (buf.stat.mode & FIO_S_IFDIR) {
                    FileEntry[n].dircheck = 1;
                    strcpy(FileEntry[n].filename, buf.name);
                    strzncpy(FileEntry[n].displayname, FileEntry[n].filename, 63);
                    n++;
                }

                if (n > 2046) { break; }
            }
            if (dd > 0) {
                fileXioDclose(dd);
                printf("Directory closed!\n");
            }
            if (files_too) {
                dd = 0;
                dd = fileXioDopen(path);
                while (fileXioDread(dd, &buf) > 0) {
                    if (buf.stat.mode & FIO_S_IFREG) {
                        FileEntry[n].dircheck = 0;
                        strcpy(FileEntry[n].filename, buf.name);
                        strzncpy(FileEntry[n].displayname, FileEntry[n].filename, 63);
                        n++;
                    }
                    if (n > 2046) { break; }
                }
                if (dd >= 0) {
                    fileXioDclose(dd);
                    printf("Directory closed!\n");
                }
            }
        }
    }
    return n;
}

int listpartitions(entries *FileEntry)
{
    printf("List Partitions\n");

    iox_dirent_t hddEnt;
    int hddFd;
    int n = 0;

    strcpy(FileEntry[n].filename, "..");
    strcpy(FileEntry[n].displayname, "..");
    FileEntry[n].dircheck = 1;
    n = 1;

    if ((hddFd = fileXioDopen("hdd0:")) < 0)
        return 0;

    while (fileXioDread(hddFd, &hddEnt) > 0) {
        if (n >= 500)
            break;
        if ((hddEnt.stat.attr != ATTR_MAIN_PARTITION) || (hddEnt.stat.mode != FS_TYPE_PFS))
            continue;

        // Patch this to see if new CB versions use valid PFS format
        // NB: All CodeBreaker versions up to v9.3 use invalid formats
        if (!strncmp(hddEnt.name, "PP.", 3)) {
        int len = strlen(hddEnt.name);
        if (!strcmp(hddEnt.name + len - 4, ".PCB"))
            continue;
        }

        if (!strncmp(hddEnt.name, "__", 2)   &&
            strcmp(hddEnt.name, "__boot")    &&
            strcmp(hddEnt.name, "__net")     &&
            strcmp(hddEnt.name, "__system")  &&
            strcmp(hddEnt.name, "__sysconf") &&
            strcmp(hddEnt.name, "__common"))
            continue;

        strcpy(FileEntry[n].filename, hddEnt.name);
        strzncpy(FileEntry[n].displayname, hddEnt.name, 63);
        FileEntry[n].dircheck = 1;
        n++;
    }
    fileXioDclose(hddFd);

    return n;
}

char *partname(char *d, const char *hdd_path)
{
    char *temp1; // First '/'
    char *temp2; // Second '/'

    temp1 = strchr(hdd_path, '/');
    temp1++;
    temp2 = temp1;

    while (*temp2 != '/') { temp2++; }

    memcpy(d, temp1, temp2 - temp1);

    d[temp2 - temp1] = 0;

    return d;
}

void unmountPartition(int pfs_number)
{
    char pfs_str[6];

    sprintf(pfs_str, "pfs%d:", pfs_number);

    fileXioUmount(pfs_str);

}

int mountPartition(char *name)
{
    int i;
    char pfs_str[6];
    char partition[256];

    partname(partition, name);

    for (i = 0; i < 3; i++) {
        printf("Mounted Partition %d: %s\n", i, mpartitions[i]);
        if (!strcmp(mpartitions[i], partition)) {
            printf("%s already mounted at pfs%d:!\n", partition, i);
            return i;
        }
    }

    for (i = 0; i < 3; i++) { // Check for empty entry and mount there
        if (mpartitions[i][0] == 0) {
            strcpy(mpartitions[i], partition); // Fill entry
            sprintf(pfs_str, "pfs%d:", i);
            sprintf(name, "hdd0:%s", partition);
            printf("mount %s to %s\n", name, pfs_str);
            if (fileXioMount(pfs_str, name, FIO_MT_RDWR) >= 0) {
                printf("mount success\n");
                return i;
            }
            else {
                mpartitions[i][0] = 0;
                printf("mount error\n");
                return -1;
            }
        }
    }
    // Control reached past for-loop so manually mount at pfs3:
    unmountPartition(3);
    strcpy(mpartitions[3], partition); // Fill entry
    sprintf(pfs_str, "pfs%d:", 3);
    sprintf(name, "hdd0:%s", partition);
    if (fileXioMount(pfs_str, name, FIO_MT_RDWR) >= 0) {
        printf("mount success\n");
        return 3;
    }
    else {
        printf("mount error\n");
        return -1;
    }
}

char* Browser(int files_too, int menu_id)
{
    int i;
    history[0] = 0;
    int drawn_screen = 0;
    int selection = history[h];
    int button_held = 0;
    int n = 0;
    int part_num = -1;
    int max_item = 21;

    if (menu_id == 1) {
        if (!strncmp(Settings.savepath, "pfs", 3))
            part_num = needed_path[1];
    }
    else if (menu_id == 2) {
        if (!strncmp(Settings.elfpath, "pfs", 3))
            part_num = needed_path[0];
    }

    int menu_x1 = gsGlobal->Width  * 0.05;
    int menu_y1 = gsGlobal->Height * 0.05;
    int menu_x2 = gsGlobal->Width  * 0.95;
    int menu_y2 = gsGlobal->Height * 0.95;
    int text_line = menu_y1 + 40;

    oldselect = -1;
    //entries FileEntry[2048]; // = malloc(sizeof(entries)*2048);
    entries *FileEntry = calloc(sizeof(entries), 2048);

    int list_offset = text_line;

    // Switch to one shot drawing queue
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsKit_queue_reset(gsGlobal->Os_Queue);
    gsGlobal->DrawOrder = GS_PER_OS; // Draw one shot objects last

    char oldpath[2048];
    strcpy(oldpath, "oldpath");

    while (1) { // List loop
        selection += RomBrowserInput(files_too, menu_id);

        if (drawn_screen == 1) { // Workaround for screenbuffer limitation in PAL mode
            init_custom_screen();
            oldselect = -3;
        }

        if (old_pad[0] & PAD_DOWN) {
            button_held++;
            if (button_held >= 32767) {
                button_held = 32767;
                selection++;
            }
        }

        if (old_pad[0] & PAD_UP) {
            button_held++;
            if (button_held >= 32767) {
                button_held = 32767;
                selection--;
            }
        }
        if (!(old_pad[0] & PAD_UP) && !(old_pad[0] & PAD_DOWN)) {
            button_held = 0;
        }

        if (Settings.display) {
            max_item = 25;
        }
        else {
            max_item = 21;
        }
        if (gsGlobal->Interlace == GS_NONINTERLACED)
            max_item = 9;


        // Scan for direct commands from input function
        if (oldselect == -4) { //  Just pushed triangle so go up a dir
            // If path is in partition root AND current partition number is not needed
            if (!strcmp(path, "pfs0:/") || !strcmp(path, "pfs1:/") || !strcmp(path, "pfs2:/") || !strcmp(path, "pfs3:/")) {
                if (part_num != needed_path[0] || needed_path[1]) {
                    unmountPartition(part_num);
                    mpartitions[part_num][0] = 0;
                    strcpy(path, "hdd0:/");
                }
                else {
                    strcpy(path, "hdd0:/");
                }
            }
            else {
                strcpy(path, browseup(path));
            }
            if (h != 0)
                h = h - 1;
            selection = history[h];
            oldselect = selection + 1;
        }
        if (oldselect == -3) { // Reset browser dimensions
            menu_x1 = gsGlobal->Width  * 0.05;
            menu_y1 = gsGlobal->Height * 0.05;
            menu_x2 = gsGlobal->Width  * 0.95;
            menu_y2 = gsGlobal->Height * 0.95;
            text_line = menu_y1 + 40;
        }

        // List files below
        if (strcmp(path, oldpath) != 0) {
            if (!strncmp(path, "hdd0:/", 6)) {
                if (!strcmp(path, "hdd0:/")) { // hdd0: selected so list partitions
                    n = listpartitions(FileEntry);
                }
                else if (strcmp(path, "hdd0:/") > 0) { // hdd0:/partition, get partition name and mount
                    if ((part_num = mountPartition(path)) != -1) {
                        sprintf(path, "pfs%d:/", part_num); // Path becomes pfs0:/
                        n = listpfs(path, FileEntry, files_too);
                        //n = listdir(path, FileEntry, files_too);
                    }
                    else
                        n = 0;
                }
            }
            //else if (!strncmp(path, "pfs", 3)) {
                //n = listpfs(path, FileEntry, files_too);
            //}
            else {
                n = listpfs(path, FileEntry, files_too);
                //n = listdir(path, FileEntry, files_too); // n == max number of items + empty entry
            }
            if (n == 0) {
                path[0] = 0;
                strcpy(path, "path");
                n = listpfs(path, FileEntry, files_too);
                //n = listdir(path, FileEntry, files_too);
            }
            strcpy(oldpath, path); // Needed so listdir isn't called every loop
            oldselect = -1; // So the screen draws on load
            printf("n = %d\n", n);
        }

        // Display list
        if (selection > (n-1)) { selection = 0; } //selection-(n-1); }
        if (selection < 0) { selection = (n-1); }
        if (selection != oldselect) {

            gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00));
            browser_primitive("FCEUltra PS2 B0.93 [x.2.4]", "Browser", &BG_TEX, menu_x1, menu_y1, menu_x2, menu_y2);

            if (selection > max_item) {
                list_offset = text_line - (selection - max_item) * FONT_HEIGHT;
            }
            if (selection <= max_item) {
                list_offset = text_line;
            }
            for (i = 0; i < n; i++) { // Display list
                if (i*16+list_offset >= menu_y2 - FONT_HEIGHT) {
                    continue;
                }
                if (i*16+list_offset < text_line) {
                    continue;
                }
                if (i == selection) {
                    printXY(FileEntry[i].displayname, menu_x1+10, i*16+list_offset, 2, FCEUSkin.highlight, 1, 0);
                }
                else {
                    printXY(FileEntry[i].displayname, menu_x1+10, i*16+list_offset, 2, FCEUSkin.textcolor, 1, 0);
                }
            }

            DrawScreen(gsGlobal);

        }

        // Post list directory options
        drawn_screen++;
        if (drawn_screen > 2) {
            drawn_screen = 2;
        }

        if (oldselect == -2) { // Clean up my memory usage and exit to elf
            free(FileEntry);
            RunLoaderElf(Settings.elfpath, ""); // Modify function to support hdd0:/partition/etc. paths
        }

        oldselect = selection;

        if (selected) {
            if (!strcmp(FileEntry[selection].filename, "..")) {
                // If path is in partition root AND current partition number is not needed
                if (!strcmp(path, "pfs0:/") || !strcmp(path, "pfs1:/") || !strcmp(path, "pfs2:/")) {
                    if (part_num != needed_path[0] || needed_path[1]) {
                        unmountPartition(part_num);
                        mpartitions[part_num][0] = 0;
                        strcpy(path, "hdd0:/");
                    }
                    else {
                        strcpy(path, "hdd0:/");
                    }
                }
                else {
                    strcpy(path, browseup(path));
                }
                if (h != 0)
                    h = h - 1;
                selection = history[h];
                oldselect = selection + 1;
                selected = 0;
            }
            else if (FileEntry[selection].dircheck) { // If directory
                if (strchr(path, '/') == NULL) { // "path" is the value
                    strcpy(path, FileEntry[selection].filename); // Copy device:/ to path
                    history[h] = selection;
                    h = h + 1;
                    selection = 0;
                    selected = 0;
                }
                else {
                    sprintf(path, "%s%s/", path, FileEntry[selection].filename);
                    history[h] = selection;
                    h = h + 1;
                    selection = 0;
                    printf("path is %s\n", path);
                    selected = 0;
                }
            }
            else if (!FileEntry[selection].dircheck) { // If file
                sprintf(path, "%s%s", path, FileEntry[selection].filename);
                printf("rompath = %s\n", path);
                history[h] = selection;
                selected = 0;
                //free(FileEntry);
                if (!strncmp(path, "pfs", 3) && menu_id)
                    needed_path[0] = part_num; // part_num is -1 if not having browsed hdd
                return (char *)path;
            }
        } // End if file selected
        if (selected_dir) {
            if (!strncmp(path, "pfs", 3) && menu_id)
                needed_path[1] = part_num;
            if (strcmp(path, "hdd0:/")) {
                //free(FileEntry);
                return (char *)path;
            }
        }
    } // End list loop
}
