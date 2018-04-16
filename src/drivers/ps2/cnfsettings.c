#include <stdio.h>
#include <string.h>
#include <libpad.h>
#include <fileXio_rpc.h>

#include "ps2fceu.h"
#include "../../driver.h"

extern GSGLOBAL *gsGlobal;

vars Settings;
skin FCEUSkin;
char control_name[256];
extern int needed_path[2];
extern char mpartitions[4][256];

extern int mountPartition(char *name);

/**
//---------------------------------------------------------------------------
// The following code is copyrighted by me, (c)2006 Ronald Andersson.
// I release it for fully free use by anyone, anywhere, for any purpose.
// But I still retain the copyright, so no one else can limit this release.
//---------------------------------------------------------------------------
// get_CNF_string analyzes a config file held in a single string, parsing it
// to find one config variable definition per call. The return value is a
// true/false flag, set true if a variable was found, but set false if not.
// This means it also returns false for an empty file.
//
// The config file is passed as the address of a string pointer, which will
// be set to point beyond the variable definition retrieved, but on failure
// it will remain at the point where failure was detected. Thus it will not
// pass beyond the terminating NUL of the string. The original string data
// will be partly slaughtered by the analysis, as new string terminators are
// inserted at end of each variable name, and also at the end of each value
// string. Those new strings are then passed back to the caller through the
// other arguments, these being pointers to string pointers of the calling
// procedure.
//
// So with a variable definition like this: "SomeVar = some bloody string",
// the results are the string pair: "SomeVar" and "some bloody string".
//
// A name must begin with a letter (here ascii > 0x40), so lines that begin
// with other non-whitespace characters will be considered comment lines by
// this function. (Simply ignored.) Whitespace is permitted in the variable
// definitions, and will be ignored if occurring before the value string,
// but once a value begins any whitespace used is considered a part of the
// value string. It will remain intact in the returned results.
//
// Note that the name part can only contain non-whitespace characters, but
// the value part can contain non-leading whitespace different from CR/LF.
// So a value starts with the first non-whitespace character after the '='
// and ends at the end of the line.
//
// Intended usage is to repeatedly call get_CNF_string to retrieve each of
// the variables in the config file, until the function returns false, which
// signals either the end of the file, or a syntax error. Analysis of the
// variables found, and usage of their values, is not dealt with at all.
//
// Such matters are left entirely up to the calling procedures, Which also
// means that caller may decide to allow comments terminating lines with a
// variable definition. That's just one of the many value analysis choices.
//---------------------------------------------------------------------------
**/

int true = 1;
int false = 0;
int CNF_edited = 0;

int get_CNF_string(char **CNF_p_p, char **name_p_p, char **value_p_p)
{
    char *np, *vp, *tp = *CNF_p_p;

start_line:
    while ((*tp<=' ') && (*tp>'\0')) tp+=1; // Skip leading whitespace, if any
    if (*tp=='\0') return false;            // but exit at EOF
    np = tp;                                // Current pos is potential name
    if (*tp<'A') {                          // but may be a comment line
        while ((*tp!='\r')&&(*tp!='\n')&&(tp!='\0'))
            tp+=1;                          // Seek line end to skip comment

        goto start_line;                    // Go back to try next line
    }

    while ((*tp>='A')||((*tp>='0')&&(*tp<='9')))
        tp+=1;                              // Seek name end

    if (*tp=='\0') return false;            // but exit at EOF
    *tp++ = '\0';                           // terminate name string (passing)
    while ((*tp<=' ') && (*tp>'\0')) tp+=1; // Skip post-name whitespace, if any
    if (*tp!='=') return false;             // exit (syntax error) if '=' missing
    tp += 1;                                // skip '='
    while ((*tp<=' ') && (*tp>'\0')         // Skip pre-value whitespace, if any
    && (*tp!='\r') && (*tp!='\n'))tp+=1;    // but do not pass the end of the line
    if (*tp=='\0') return false;            // but exit at EOF
    vp = tp;                                // Current pos is potential value

    while ((*tp!='\r')&&(*tp!='\n')&&(tp!='\0'))
        tp+=1;                              // Seek line end

    if (*tp!='\0') *tp++ = '\0';            // terminate value (passing if not EOF)
    while ((*tp<=' ') && (*tp>'\0')) tp+=1; // Skip following whitespace, if any

    *CNF_p_p = tp;                          // return new CNF file position
    *name_p_p = np;                         // return found variable name
    *value_p_p = vp;                        // return found variable value
    return true;                            // return control to caller
}  // Ends get_CNF_string

//---------------------------------------------------------------------------
void Load_Global_CNF(char *CNF_path_p)
{
    int fd, var_cnt = 0;
    size_t CNF_size;
    char  *RAM_p, *CNF_p, *name, *value;
    char *p;
    char *temp1;
    char partpath[1024];
    sprintf(CNF_path_p, "%sFCEUltra.cnf", CNF_path_p);

    if (!strncmp(CNF_path_p, "hdd0:/", 6)) {

        temp1 = strchr(CNF_path_p, '/');
        temp1++;

        // All my paths have two /'s
        while (*temp1 != '/') { temp1++; }

        fd = mountPartition(CNF_path_p);
        needed_path[1] = fd;
        if (needed_path[1] == -1) {
            strcpy(CNF_path_p, "mc0:/FCEUMM/FCEUltra.cnf");
        }
        else {
            sprintf(partpath, "pfs%d:", needed_path[1]);

            sprintf(partpath, "%s%s", partpath, temp1);
            strcpy(CNF_path_p, partpath);

            printf("partpath: %s\n", CNF_path_p);
            fd = fileXioOpen(CNF_path_p, O_RDONLY, 0);
        }
    }
    else {
        fd = fioOpen(CNF_path_p, O_RDONLY);
    }
    if (fd < 0) {
        printf("Load_CNF %s Open failed %d.\r\n", CNF_path_p, fd);
        strcpy(CNF_path_p, "mc0:/FCEUMM/FCEUltra.cnf");
        fd = fioOpen(CNF_path_p, O_RDONLY);
        if (fd < 0) {
            printf("Load_CNF %s Open failed %d.\r\n", CNF_path_p, fd);
            return;
        }
    }
        
    if (((p = strrchr(CNF_path_p, '/')) == NULL) && ((p = strrchr(CNF_path_p, '\\')) == NULL))
        p = strrchr(CNF_path_p, ':');
    if (p != NULL)
        *p = 0;
    // The above cuts away the cnf filename from CNF_path_p and last '/', leaving a pure path
    FCEUI_SetBaseDirectory(CNF_path_p);

    CNF_size = fioLseek(fd, 0, SEEK_END);
    fioLseek(fd, 0, SEEK_SET);
    CNF_p = (RAM_p = malloc(CNF_size + 1));
    if (CNF_p == NULL) {
        printf("Load_CNF failed malloc(%d).\r\n", CNF_size);
        return;
    }
    fioRead(fd, CNF_p, CNF_size);
    fioClose(fd);
    CNF_p[CNF_size] = '\0';

    for (var_cnt = 0; get_CNF_string(&CNF_p, &name, &value); var_cnt++) {
        // A variable was found, now we dispose of its value.
        printf("Found variable \"%s\" with value \"%s\"\r\n", name, value);
        if (!strcmp(name, "OffsetX"))               { Settings.offset_x  = atoi(value); }
        else if (!strcmp(name, "OffsetY"))          { Settings.offset_y  = atoi(value); }
        else if (!strcmp(name, "Display"))          { Settings.display   = atoi(value); }
        else if (!strcmp(name, "Emulation"))        { Settings.emulation = atoi(value); }
        else if (!strcmp(name, "Interlace"))        { Settings.interlace = atoi(value); }
        else if (!strcmp(name, "Filter"))           { Settings.filter    = atoi(value); }
        else if (!strcmp(name, "Sound"))            { Settings.sound     = atoi(value); }
        else if (!strcmp(name, "Elfpath"))          { strcpy(Settings.elfpath,  value); }
        else if (!strcmp(name, "Savepath"))         { strcpy(Settings.savepath, value); }
        else if (!strcmp(name, "Skinpath"))         { strcpy(Settings.skinpath, value); }
        // Player 1 Settings
        else if (!strcmp(name, "JOY1_Menu"))        { Settings.PlayerInput[0][0]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_SaveState"))   { Settings.PlayerInput[0][1]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_LoadState"))   { Settings.PlayerInput[0][2]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_FDS_DiskSwap")){ Settings.PlayerInput[0][3]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_FDS_SideSwap")){ Settings.PlayerInput[0][4]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_A"))           { Settings.PlayerInput[0][5]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_B"))           { Settings.PlayerInput[0][6]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_Select"))      { Settings.PlayerInput[0][7]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_Start"))       { Settings.PlayerInput[0][8]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_Up"))          { Settings.PlayerInput[0][9]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_Down"))        { Settings.PlayerInput[0][10] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_Left"))        { Settings.PlayerInput[0][11] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_Right"))       { Settings.PlayerInput[0][12] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_Turbo_A"))     { Settings.PlayerInput[0][13] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY1_Turbo_B"))     { Settings.PlayerInput[0][14] = (u16)strtoul(value, NULL, 16); }
        // Player 2 Settings
        else if (!strcmp(name, "JOY2_A"))           { Settings.PlayerInput[1][5]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_B"))           { Settings.PlayerInput[1][6]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_Select"))      { Settings.PlayerInput[1][7]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_Start"))       { Settings.PlayerInput[1][8]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_Up"))          { Settings.PlayerInput[1][9]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_Down"))        { Settings.PlayerInput[1][10] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_Left"))        { Settings.PlayerInput[1][11] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_Right"))       { Settings.PlayerInput[1][12] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_Turbo_A"))     { Settings.PlayerInput[1][13] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY2_Turbo_B"))     { Settings.PlayerInput[1][14] = (u16)strtoul(value, NULL, 16); }
        // Player 3 Settings
        else if (!strcmp(name, "JOY3_A"))           { Settings.PlayerInput[2][5]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_B"))           { Settings.PlayerInput[2][6]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_Select"))      { Settings.PlayerInput[2][7]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_Start"))       { Settings.PlayerInput[2][8]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_Up"))          { Settings.PlayerInput[2][9]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_Down"))        { Settings.PlayerInput[2][10] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_Left"))        { Settings.PlayerInput[2][11] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_Right"))       { Settings.PlayerInput[2][12] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_Turbo_A"))     { Settings.PlayerInput[2][13] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY3_Turbo_B"))     { Settings.PlayerInput[2][14] = (u16)strtoul(value, NULL, 16); }
        // Player 4 Settings
        else if (!strcmp(name, "JOY4_A"))           { Settings.PlayerInput[3][5]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_B"))           { Settings.PlayerInput[3][6]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_Select"))      { Settings.PlayerInput[3][7]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_Start"))       { Settings.PlayerInput[3][8]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_Up"))          { Settings.PlayerInput[3][9]  = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_Down"))        { Settings.PlayerInput[3][10] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_Left"))        { Settings.PlayerInput[3][11] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_Right"))       { Settings.PlayerInput[3][12] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_Turbo_A"))     { Settings.PlayerInput[3][13] = (u16)strtoul(value, NULL, 16); }
        else if (!strcmp(name, "JOY4_Turbo_B"))     { Settings.PlayerInput[3][14] = (u16)strtoul(value, NULL, 16); }
    }

    // Set so only first player controls emulator controls
    int player;
    for (player = 1; player < 4; player++) {
        int i;
        for (i = 0; i < 5; i++) {
            Settings.PlayerInput[player][i] = 0xFFFF;
        }
    }

    // Begin hdd path mounting

    if (!strncmp(Settings.elfpath, "hdd0:/", 6)) {

        temp1 = strchr(Settings.elfpath, '/');
        temp1++;

        while (*temp1 != '/') { temp1++; }

        needed_path[0] = mountPartition(Settings.elfpath);

        if (needed_path[0] == -1) {
            strcpy(Settings.elfpath, "mc0:/BOOT/BOOT.ELF");
        }
        else {

            sprintf(partpath, "pfs%d:", needed_path[0]);

            sprintf(partpath, "%s%s", partpath, temp1);
            strcpy(Settings.elfpath, partpath);
        }

        printf("partpath: %s\n", Settings.elfpath);
    }
    if (!strncmp(Settings.savepath, "hdd0:/", 6)) {

        temp1 = strchr(Settings.savepath, '/');
        temp1++;

        // All my paths have two /'s
        while (*temp1 != '/') { temp1++; }

        needed_path[1] = mountPartition(Settings.savepath);

        if (needed_path[1] == -1) {
            strcpy(Settings.savepath, "mc0:/FCEUMM/");
        }
        else {
            sprintf(partpath, "pfs%d:", needed_path[1]);

            sprintf(partpath, "%s%s", partpath, temp1);
            strcpy(Settings.savepath, partpath);

            printf("partpath: %s\n", Settings.savepath);
        }
    }

    // End hdd path mounting

    if (strlen(CNF_p))  //Was there any unprocessed CNF remainder ?
        CNF_edited = false;  //false == current settings match CNF file
    else
        printf("Syntax error in CNF file at position %d.\r\n", (CNF_p - RAM_p));

    free(RAM_p);

}  // Ends Load_Global_CNF

//---------------------------------------------------------------------------
void Load_Skin_CNF(char *CNF_path_p)
{
    int fd, var_cnt = 0;
    size_t CNF_size;
    char  *RAM_p, *CNF_p, *name, *value;

    fd = fioOpen(CNF_path_p, O_RDONLY);
    if (fd < 0) {
        printf("Load_CNF %s Open failed %d.\r\n", CNF_path_p, fd);
        return;
    }
    CNF_size = fioLseek(fd, 0, SEEK_END);
    fioLseek(fd, 0, SEEK_SET);
    CNF_p = (RAM_p = malloc(CNF_size + 1));
    if (CNF_p == NULL) {
        printf("Load_CNF failed malloc(%d).\r\n", CNF_size);
        return;
    }
    fioRead(fd, CNF_p, CNF_size);
    fioClose(fd);
    CNF_p[CNF_size] = '\0';

    for (var_cnt = 0; get_CNF_string(&CNF_p, &name, &value); var_cnt++) {
        // A variable was found, now we dispose of its value.
        printf("Found variable \"%s\" with value \"%s\"\r\n", name, value);
        if (!strcmp(name, "FrameColor"))          { FCEUSkin.frame      = strtoul(value, NULL, 16); }
        else if (!strcmp(name, "TextColor"))      { FCEUSkin.textcolor  = strtoul(value, NULL, 16); }
        else if (!strcmp(name, "Highlight"))      { FCEUSkin.highlight  = strtoul(value, NULL, 16); }
        else if (!strcmp(name, "BGColor1"))       { FCEUSkin.bgColor1   = strtoul(value, NULL, 16); }
        else if (!strcmp(name, "BGColor2"))       { FCEUSkin.bgColor2   = strtoul(value, NULL, 16); }
        else if (!strcmp(name, "BGColor3"))       { FCEUSkin.bgColor3   = strtoul(value, NULL, 16); }
        else if (!strcmp(name, "BGColor4"))       { FCEUSkin.bgColor4   = strtoul(value, NULL, 16); }
        else if (!strcmp(name, "BGTexture"))      { strcpy(FCEUSkin.bgTexture, value);   }
        else if (!strcmp(name, "BGMenu"))         { strcpy(FCEUSkin.bgMenu, value);     }
    }

    /*if (strlen(CNF_p))  // Was there any unprocessed CNF remainder ?
        CNF_edited = false;  // false == current settings match CNF file
    else
        printf("Syntax error in CNF file at position %d.\r\n", (CNF_p - RAM_p));*/

    free(RAM_p);

}  // Ends Load_Skin_CNF

//---------------------------------------------------------------------------
void Save_Skin_CNF(char *CNF_path_p)
{
    int fd;
    size_t CNF_size = 4096; // Safe preliminary value
    char  *CNF_p;

    CNF_p = malloc(CNF_size);
    if (CNF_p == NULL) return;
    sprintf(CNF_p,
        "# SKIN.CNF == Skin configuration file for the emulator FCEUltra\r\n"
        "# CNF Handling Code (c)2006 Ronald Andersson aka dlanor        \r\n"
        "# -------------------------------------------------------------\r\n"
        "FrameColor  = 0x%08llu\r\n"
        "TextColor   = 0x%08llu\r\n"
        "Highlight   = 0x%08llu\r\n"
        "BGColor1    = 0x%08llu\r\n"
        "BGColor2    = 0x%08llu\r\n"
        "BGColor3    = 0x%08llu\r\n"
        "BGColor4    = 0x%08llu\r\n"
        "BGTexture   = %s\r\n"
        "BGMenu      = %s\r\n"
        "# -------------------------------------------------------------\r\n"
        "# End-Of-File for SKIN.CNF\r\n"
        "%n", // NB: The %n specifier causes NO output, but only a measurement
        FCEUSkin.frame,
        FCEUSkin.textcolor,
        FCEUSkin.highlight,
        FCEUSkin.bgColor1,
        FCEUSkin.bgColor2,
        FCEUSkin.bgColor3,
        FCEUSkin.bgColor4,
        FCEUSkin.bgTexture,
        FCEUSkin.bgMenu,
        &CNF_size);
    // Note that the final argument above measures accumulated string size,
    // used for fioWrite below, so it's not one of the config variables.

    fd = fioOpen(CNF_path_p, O_CREAT | O_WRONLY | O_TRUNC);
    if (fd >= 0) {
        if (CNF_size == fioWrite(fd, CNF_p, CNF_size))
            CNF_edited = false;

        fioClose(fd);
    }

    free(CNF_p);
}  // Ends Save_Skin_CNF

//---------------------------------------------------------------------------
void Save_Global_CNF(char *CNF_path_p)
{
    // Begin hdd path conversion
    char temp1[1024];
    char temp2[1024];
    char temp4[1024];
    char *temp3;

    strcpy(temp1, Settings.elfpath);
    strcpy(temp2, Settings.savepath);

    if (needed_path[0] > -1) {
        if (!strncmp(temp1, "pfs", 3)) {
            temp3 = strchr(temp1, '/');
            memcpy(temp4, temp3, strlen(temp3));
            temp3[strlen(temp3)] = 0;
            sprintf(temp1, "hdd0:/%s%s", mpartitions[needed_path[0]], temp4);
            printf("Savepath: %s\n", temp1);
        }
    }
    if (needed_path[1] > -1) {
        if (!strncmp(temp2, "pfs", 3)) {
            temp3 = strchr(temp2, '/');
            memcpy(temp4, temp3, strlen(temp3));
            temp3[strlen(temp3)] = 0;
            sprintf(temp2, "hdd0:/%s%s", mpartitions[needed_path[1]], temp4);
            printf("Elfpath: %s\n", temp2);
        }
    }
    // End hdd path conversion

    int fd;
    size_t CNF_size = 4096; // Safe preliminary value
    char  *CNF_p;

    CNF_p = malloc(CNF_size);
    if (CNF_p == NULL) return;
    sprintf(CNF_p,
        "# FCEULTRA.CNF == Configuration file for the emulator FCEUltra\r\n"
        "# CNF Handling Code (c)2006 Ronald Andersson aka dlanor       \r\n"
        "# ------------------------------------------------------------\r\n"
        "OffsetX     = %d\r\n"
        "OffsetY     = %d\r\n"
        "Display     = %d\r\n"
        "Emulation   = %d\r\n"
        "Interlace   = %d\r\n"
        "Filter      = %d\r\n"
        "Sound       = %d\r\n"
        "Elfpath     = %s\r\n"
        "Savepath    = %s\r\n"
        "Skinpath    = %s\r\n"
        ";Player 1 Controls\r\n"
        "JOY1_Menu         = 0x%04x\r\n"
        "JOY1_SaveState    = 0x%04x\r\n"
        "JOY1_LoadState    = 0x%04x\r\n"
        "JOY1_FDS_DiskSwap = 0x%04x\r\n"
        "JOY1_FDS_SideSwap = 0x%04x\r\n"
        "JOY1_A            = 0x%04x\r\n"
        "JOY1_B            = 0x%04x\r\n"
        "JOY1_Select       = 0x%04x\r\n"
        "JOY1_Start        = 0x%04x\r\n"
        "JOY1_Up           = 0x%04x\r\n"
        "JOY1_Down         = 0x%04x\r\n"
        "JOY1_Left         = 0x%04x\r\n"
        "JOY1_Right        = 0x%04x\r\n"
        "JOY1_Turbo_A      = 0x%04x\r\n"
        "JOY1_Turbo_B      = 0x%04x\r\n"
        ";Player 2 Controls\r\n"
        "JOY2_A            = 0x%04x\r\n"
        "JOY2_B            = 0x%04x\r\n"
        "JOY2_Select       = 0x%04x\r\n"
        "JOY2_Start        = 0x%04x\r\n"
        "JOY2_Up           = 0x%04x\r\n"
        "JOY2_Down         = 0x%04x\r\n"
        "JOY2_Left         = 0x%04x\r\n"
        "JOY2_Right        = 0x%04x\r\n"
        "JOY2_Turbo_A      = 0x%04x\r\n"
        "JOY2_Turbo_B      = 0x%04x\r\n"
        ";Player 3 Controls\r\n"
        "JOY3_A            = 0x%04x\r\n"
        "JOY3_B            = 0x%04x\r\n"
        "JOY3_Select       = 0x%04x\r\n"
        "JOY3_Start        = 0x%04x\r\n"
        "JOY3_Up           = 0x%04x\r\n"
        "JOY3_Down         = 0x%04x\r\n"
        "JOY3_Left         = 0x%04x\r\n"
        "JOY3_Right        = 0x%04x\r\n"
        "JOY3_Turbo_A      = 0x%04x\r\n"
        "JOY3_Turbo_B      = 0x%04x\r\n"
        ";Player 4 Controls\r\n"
        "JOY4_A            = 0x%04x\r\n"
        "JOY4_B            = 0x%04x\r\n"
        "JOY4_Select       = 0x%04x\r\n"
        "JOY4_Start        = 0x%04x\r\n"
        "JOY4_Up           = 0x%04x\r\n"
        "JOY4_Down         = 0x%04x\r\n"
        "JOY4_Left         = 0x%04x\r\n"
        "JOY4_Right        = 0x%04x\r\n"
        "JOY4_Turbo_A      = 0x%04x\r\n"
        "JOY4_Turbo_B      = 0x%04x\r\n"
        "# ------------------------------------------------------------\r\n"
        "# End-Of-File for FCEUltra.CNF\r\n"
        "%n", // NB: The %n specifier causes NO output, but only a measurement
        Settings.offset_x,
        Settings.offset_y,
        Settings.display,
        Settings.emulation,
        Settings.interlace,
        Settings.filter,
        Settings.sound,
        temp1,
        temp2,
        Settings.skinpath,
        Settings.PlayerInput[0][0],
        Settings.PlayerInput[0][1],
        Settings.PlayerInput[0][2],
        Settings.PlayerInput[0][3],
        Settings.PlayerInput[0][4],
        Settings.PlayerInput[0][5],
        Settings.PlayerInput[0][6],
        Settings.PlayerInput[0][7],
        Settings.PlayerInput[0][8],
        Settings.PlayerInput[0][9],
        Settings.PlayerInput[0][10],
        Settings.PlayerInput[0][11],
        Settings.PlayerInput[0][12],
        Settings.PlayerInput[0][13],
        Settings.PlayerInput[0][14],

        Settings.PlayerInput[1][5],
        Settings.PlayerInput[1][6],
        Settings.PlayerInput[1][7],
        Settings.PlayerInput[1][8],
        Settings.PlayerInput[1][9],
        Settings.PlayerInput[1][10],
        Settings.PlayerInput[1][11],
        Settings.PlayerInput[1][12],
        Settings.PlayerInput[1][13],
        Settings.PlayerInput[1][14],

        Settings.PlayerInput[2][5],
        Settings.PlayerInput[2][6],
        Settings.PlayerInput[2][7],
        Settings.PlayerInput[2][8],
        Settings.PlayerInput[2][9],
        Settings.PlayerInput[2][10],
        Settings.PlayerInput[2][11],
        Settings.PlayerInput[2][12],
        Settings.PlayerInput[2][13],
        Settings.PlayerInput[2][14],

        Settings.PlayerInput[3][5],
        Settings.PlayerInput[3][6],
        Settings.PlayerInput[3][7],
        Settings.PlayerInput[3][8],
        Settings.PlayerInput[3][9],
        Settings.PlayerInput[3][10],
        Settings.PlayerInput[3][11],
        Settings.PlayerInput[3][12],
        Settings.PlayerInput[3][13],
        Settings.PlayerInput[3][14],
        &CNF_size);
    // Note that the final argument above measures accumulated string size,
    // used for fioWrite below, so it's not one of the config variables.

    fd = fioOpen(CNF_path_p, O_CREAT | O_WRONLY | O_TRUNC);
    if (fd >= 0) {
        if (CNF_size == fioWrite(fd, CNF_p, CNF_size))
            CNF_edited = false;

        fioClose(fd);
    }
    
    free(CNF_p);

    if (*temp1)
        strcpy(Settings.savepath, temp1);

    if (*temp2)
        strcpy(Settings.elfpath, temp2);


}  // Ends Save_Global_CNF

void Default_Global_CNF()
{
    Settings.aspect_ratio     = 0; // Full Screen
    Settings.input_4p_adaptor = 0; // False
    Settings.current_palette  = 1; // 1 - First
    Settings.offset_x         = 0;
    Settings.offset_y         = 0;
    Settings.interlace        = 1; // True
    Settings.filter           = 0; // False
    Settings.sound            = 2; // 22050Hz
    Settings.display          = 0; // NTSC
    Settings.emulation        = 0; // NTSC
    strcpy(Settings.elfpath,  "mc0:/BOOT/BOOT.ELF");
    strcpy(Settings.savepath, "mc0:/FCEUMM/");
    strcpy(Settings.skinpath, "mc0:/FCEUMM/skin.cnf");

    int player;
    for (player = 0; player < 4; player++) {
        Settings.PlayerInput[player][0]  = 0xFFFF;
        Settings.PlayerInput[player][1]  = 0xFFFF;
        Settings.PlayerInput[player][2]  = 0xFFFF;
        Settings.PlayerInput[player][3]  = 0xFFFF;
        Settings.PlayerInput[player][4]  = 0xFFFF;
        Settings.PlayerInput[player][5]  = PAD_CROSS;
        Settings.PlayerInput[player][6]  = PAD_SQUARE;
        Settings.PlayerInput[player][7]  = PAD_SELECT;
        Settings.PlayerInput[player][8]  = PAD_START;
        Settings.PlayerInput[player][9]  = PAD_UP;
        Settings.PlayerInput[player][10] = PAD_DOWN;
        Settings.PlayerInput[player][11] = PAD_LEFT;
        Settings.PlayerInput[player][12] = PAD_RIGHT;
        Settings.PlayerInput[player][13] = PAD_CIRCLE;
        Settings.PlayerInput[player][14] = PAD_TRIANGLE;
    }
    Settings.PlayerInput[0][0] = PAD_L1;
    Settings.PlayerInput[0][1] = PAD_R2;
    Settings.PlayerInput[0][2] = PAD_L2;
    Settings.PlayerInput[0][3] = PAD_L3;
    Settings.PlayerInput[0][4] = PAD_R3;
}

void Default_Skin_CNF()
{
    FCEUSkin.frame     = GS_SETREG_RGBAQ(0xFF, 0xFF, 0xFF, 0x80, 0x00);
    FCEUSkin.textcolor = GS_SETREG_RGBAQ(0xFF, 0xFF, 0xFF, 0x80, 0x00);
    FCEUSkin.highlight = GS_SETREG_RGBAQ(0x80, 0x80, 0x40, 0x80, 0x00);
    FCEUSkin.bgColor1  = GS_SETREG_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x00);
    FCEUSkin.bgColor2  = GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);
    FCEUSkin.bgColor3  = GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);
    FCEUSkin.bgColor4  = GS_SETREG_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x00);
    strcpy(FCEUSkin.bgTexture, "mc0:/FCEUMM/backg.jpg");
    strcpy(FCEUSkin.bgMenu, "mc0:/FCEUMM/strtg.jpg");
}
