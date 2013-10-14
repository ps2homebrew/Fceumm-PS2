#include <libpad.h>

#include <dmaKit.h>
#include <gsToolkit.h>

typedef struct {
    char displayname[64];
    int  dircheck;
    char filename[256];
} entries;

typedef struct {
    int offset_x;
    int offset_y;
    u8 display;
    u8 emulation;
    u8 interlace;
    u8 filter;
    u8 lowpass;
    u8 turbo;
    char elfpath[1024];
    char savepath[1024];
    char skinpath[1024];
    u16 PlayerInput[2][13];
	int current_palette;
} vars;

typedef struct {
    u64 frame;
    u64 textcolor;
    u64 highlight;
    u64 bgColor1;
    u64 bgColor2;
    u64 bgColor3;
    u64 bgColor4;
    char bgTexture[1024];
    char bgMenu[1024];
} skin;

//Initialization prototypes
void SetupGSKit(void);
void InitPS2(void);
void normalize_screen(void);
void init_custom_screen(void);
void setupPS2Pad(void);

void DrawScreen(GSGLOBAL *gsGlobal);

//Text related prototypes
int loadFont(char *path_arg);
int printXY(const unsigned char *s, int x, int y, int z, u64 colour, int draw, int space);

//GUI related prototypes
void RunLoaderElf(char *filename, char *party);
char* Browser(int files_too, int menu_id);
void menu_background(float x1, float y1, float x2, float y2, int z);
void menu_primitive(char *title, GSTEXTURE *gsTexture, float x1, float y1, float x2, float y2);
void browser_primitive(char *title1, char *title2, GSTEXTURE *gsTexture, float x1, float y1, float x2, float y2);

//CNF handling prototypes
  //Global
void Load_Global_CNF(char *CNF_path_p);
void Save_Global_CNF(char *CNF_path_p);
void Default_Global_CNF(void);
  //Control
char* Load_Control_CNF(char *CNF_path_p, int port);
  //Skin
void Load_Skin_CNF(char *CNF_path_p);
void Save_Skin_CNF(char *CNF_path_p);
void Default_Skin_CNF(void);
