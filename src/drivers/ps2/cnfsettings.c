#include <stdio.h>
#include <string.h>

#include "ps2fceu.h"

vars Settings;

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

int get_CNF_string(unsigned char **CNF_p_p, unsigned char **name_p_p, unsigned char **value_p_p)
{
    unsigned char *np, *vp, *tp = *CNF_p_p;

start_line:
    while((*tp<=' ') && (*tp>'\0')) tp+=1; //Skip leading whitespace, if any
    if(*tp=='\0') return false;            //but exit at EOF
    np = tp;                               //Current pos is potential name
    if(*tp<'A') {                          //but may be a comment line
        while((*tp!='\r')&&(*tp!='\n')&&(tp!='\0'))
            tp+=1;                         //Seek line end to skip comment

        goto start_line;                   //Go back to try next line
    }

    while((*tp>='A')||((*tp>='0')&&(*tp<='9')))
        tp+=1;                             //Seek name end

    if(*tp=='\0') return false;            //but exit at EOF
    *tp++ = '\0';                          //terminate name string (passing)
    while((*tp<=' ') && (*tp>'\0')) tp+=1; //Skip post-name whitespace, if any
    if(*tp!='=') return false;             //exit (syntax error) if '=' missing
    tp += 1;                               //skip '='
    while((*tp<=' ') && (*tp>'\0')) tp+=1; //Skip pre-value whitespace, if any
    if(*tp=='\0') return false;            //but exit at EOF
    vp = tp;                               //Current pos is potential value

    while((*tp!='\r')&&(*tp!='\n')&&(tp!='\0'))
        tp+=1;                             //Seek line end

    if(*tp!='\0') *tp++ = '\0';            //terminate value (passing if not EOF)
    while((*tp<=' ') && (*tp>'\0')) tp+=1; //Skip following whitespace, if any

    *CNF_p_p = tp;                          //return new CNF file position
    *name_p_p = np;                         //return found variable name
    *value_p_p = vp;                        //return found variable value
    return true;                            //return control to caller
}  //Ends get_CNF_string

//---------------------------------------------------------------------------
void Load_Global_CNF(char *CNF_path_p)
{
    int fd, var_cnt = 0;
    size_t TST_size, CNF_size;
    unsigned char  *RAM_p, *CNF_p, *name, *value;

    fd = fioOpen(CNF_path_p,O_RDONLY);
    if(fd < 0)	{
        printf("Load_CNF %s Open failed %d.\r\n", CNF_path_p, fd);
        return;
    }
    CNF_size = fioLseek(fd, 0, SEEK_END);
    fioLseek(fd, 0, SEEK_SET);
    CNF_p = (RAM_p = (char *)malloc(CNF_size+1));
    if(CNF_p==NULL) {
        printf("Load_CNF failed malloc(%d).\r\n", CNF_size);
        return;
    }
    TST_size = fioRead(fd, CNF_p, CNF_size);
    fioClose(fd);
    CNF_p[CNF_size] = '\0';

    for(var_cnt = 0; get_CNF_string(&CNF_p, &name, &value); var_cnt++) {
        // A variable was found, now we dispose of its value.
        printf("Found variable \"%s\" with value \"%s\"\r\n", name, value);
        if(!strcmp(name,"OffsetX"))             { Settings.offset_x  = atoi(value); }
        else if(!strcmp(name,"OffsetY"))        { Settings.offset_y  = atoi(value); }
        else if(!strcmp(name,"Display"))        { Settings.display   = atoi(value); }
        else if(!strcmp(name,"Emulation"))      { Settings.emulation = atoi(value); }
        else if(!strcmp(name,"Interlace"))      { Settings.interlace = atoi(value); }
        else if(!strcmp(name,"Elfpath"))        { strcpy(Settings.elfpath,value);    }
        else if(!strcmp(name,"Savepath"))       { strcpy(Settings.savepath,value);   }
        //Player 1 Settings
        else if(!strcmp(name,"JOY1_Menu"))      { Settings.PlayerInput[0][0]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_SaveState")) { Settings.PlayerInput[0][1]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_LoadState")) { Settings.PlayerInput[0][2]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_A"))         { Settings.PlayerInput[0][3]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_B"))         { Settings.PlayerInput[0][4]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_Select"))    { Settings.PlayerInput[0][5]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_Start"))     { Settings.PlayerInput[0][6]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_Up"))        { Settings.PlayerInput[0][7]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_Down"))      { Settings.PlayerInput[0][8]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_Left"))      { Settings.PlayerInput[0][9]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY1_Right"))     { Settings.PlayerInput[0][10] = (u16)strtoul(value,NULL,16); }
        //Player 2 Settings
        else if(!strcmp(name,"JOY2_A"))         { Settings.PlayerInput[1][3]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY2_B"))         { Settings.PlayerInput[1][4]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY2_Select"))    { Settings.PlayerInput[1][5]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY2_Start"))     { Settings.PlayerInput[1][6]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY2_Up"))        { Settings.PlayerInput[1][7]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY2_Down"))      { Settings.PlayerInput[1][8]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY2_Left"))      { Settings.PlayerInput[1][9]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY2_Right"))     { Settings.PlayerInput[1][10] = (u16)strtoul(value,NULL,16); }
    }

    //Set so only first player controls emulator controls
    Settings.PlayerInput[1][0] = 0xFFFF;
    Settings.PlayerInput[1][1] = 0xFFFF;
    Settings.PlayerInput[1][2] = 0xFFFF;

    if(strlen(CNF_p))  //Was there any unprocessed CNF remainder ?
        CNF_edited = false;  //false == current settings match CNF file
    else
	printf("Syntax error in CNF file at position %d.\r\n", (CNF_p-RAM_p));

    free(RAM_p);

}  //Ends Load_Global_CNF

//---------------------------------------------------------------------------
char* Load_Control_CNF(char *CNF_path_p, int port)
{
    int fd, var_cnt = 0;
    size_t TST_size, CNF_size;
    unsigned char  *RAM_p, *CNF_p, *name, *value;

    fd = fioOpen(CNF_path_p,O_RDONLY);
    if(fd < 0)	{
        printf("Load_CNF %s Open failed %d.\r\n", CNF_path_p, fd);
        return 0;
    }
    CNF_size = fioLseek(fd, 0, SEEK_END);
    fioLseek(fd, 0, SEEK_SET);
    CNF_p = (RAM_p = (char *)malloc(CNF_size+1));
    if(CNF_p==NULL) {
        printf("Load_CNF failed malloc(%d).\r\n", CNF_size);
        return 0;
    }
    TST_size = fioRead(fd, CNF_p, CNF_size);
    fioClose(fd);
    CNF_p[CNF_size] = '\0';

    for(var_cnt = 0; get_CNF_string(&CNF_p, &name, &value); var_cnt++) {
        // A variable was found, now we dispose of its value.
        printf("Found variable \"%s\" with value \"%s\"\r\n", name, value);
        if(!strcmp(name,"JOY_Menu"))           { Settings.PlayerInput[port][0]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_SaveState")) { Settings.PlayerInput[port][1]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_LoadState")) { Settings.PlayerInput[port][2]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_A"))         { Settings.PlayerInput[port][3]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_B"))         { Settings.PlayerInput[port][4]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_Select"))    { Settings.PlayerInput[port][5]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_Start"))     { Settings.PlayerInput[port][6]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_Up"))        { Settings.PlayerInput[port][7]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_Down"))      { Settings.PlayerInput[port][8]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_Left"))      { Settings.PlayerInput[port][9]  = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"JOY_Right"))     { Settings.PlayerInput[port][10] = (u16)strtoul(value,NULL,16); }
        else if(!strcmp(name,"name"))          { strcpy(name,value); }
    }

    //Set so only first player controls emulator controls
    Settings.PlayerInput[1][0] = 0xFFFF;
    Settings.PlayerInput[1][1] = 0xFFFF;
    Settings.PlayerInput[1][2] = 0xFFFF;

    /*if(strlen(CNF_p))  //Was there any unprocessed CNF remainder ?
        CNF_edited = false;  //false == current settings match CNF file
    else
	printf("Syntax error in CNF file at position %d.\r\n", (CNF_p-RAM_p));*/

    free(RAM_p);

    return name;

}  //Ends Load_Control_CNF

//---------------------------------------------------------------------------
void Save_Global_CNF(char *CNF_path_p)
{
    int fd, CNF_error;
    size_t CNF_size = 4096; //safe preliminary value
    char  *CNF_p;

    CNF_error = true;
    CNF_p = (char *)malloc(CNF_size);
    if(CNF_p == NULL) return;
    sprintf(CNF_p,
        "# FCEULTRA.CNF == Configuration file for the emulator FCEUltra\r\n"
        "# CNF Handling Code (c)2006 Ronald Andersson aka dlanor       \r\n"
        "# ------------------------------------------------------------\r\n"
        "OffsetX     = %d\r\n"
        "OffsetY     = %d\r\n"
        "Display     = %d\r\n"
        "Emulation   = %d\r\n"
        "Interlace   = %d\r\n"
        "Elfpath     = %s\r\n"
        "Savepath    = %s\r\n"
        ";Player 1 Controls\r\n"
        "JOY1_Menu      = 0x%04x\r\n"
        "JOY1_SaveState = 0x%04x\r\n"
        "JOY1_LoadState = 0x%04x\r\n"
        "JOY1_A         = 0x%04x\r\n"
        "JOY1_B         = 0x%04x\r\n"
        "JOY1_Select    = 0x%04x\r\n"
        "JOY1_Start     = 0x%04x\r\n"
        "JOY1_Up        = 0x%04x\r\n"
        "JOY1_Down      = 0x%04x\r\n"
        "JOY1_Left      = 0x%04x\r\n"
        "JOY1_Right     = 0x%04x\r\n"
        ";Player 2 Controls\r\n"
        "JOY2_A         = 0x%04x\r\n"
        "JOY2_B         = 0x%04x\r\n"
        "JOY2_Select    = 0x%04x\r\n"
        "JOY2_Start     = 0x%04x\r\n"
        "JOY2_Up        = 0x%04x\r\n"
        "JOY2_Down      = 0x%04x\r\n"
        "JOY2_Left      = 0x%04x\r\n"
        "JOY2_Right     = 0x%04x\r\n"
        "# ------------------------------------------------------------\r\n"
        "# End-Of-File for FCEUltra.CNF\r\n"
        "%n", //NB: The %n specifier causes NO output, but only a measurement
        Settings.offset_x,
        Settings.offset_y,
        Settings.display,
        Settings.emulation,
        Settings.interlace,
        Settings.elfpath,
        Settings.savepath,
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
        Settings.PlayerInput[1][3],
        Settings.PlayerInput[1][4],
        Settings.PlayerInput[1][5],
        Settings.PlayerInput[1][6],
        Settings.PlayerInput[1][7],
        Settings.PlayerInput[1][8],
        Settings.PlayerInput[1][9],
        Settings.PlayerInput[1][10],
        &CNF_size);
// Note that the final argument above measures accumulated string size,
// used for fioWrite below, so it's not one of the config variables.

    fd = fioOpen(CNF_path_p,O_CREAT|O_WRONLY|O_TRUNC);
    if(fd < 0)
        goto abort;

    if(CNF_size == fioWrite(fd, CNF_p, CNF_size))
        CNF_edited = false;

    fioClose(fd);

abort:
    free(CNF_p);
}  //Ends Save_CNF

//---------------------------------------------------------------------------
