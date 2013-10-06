#define FONT_HEIGHT 15.0f

typedef struct {
    char displayname[32];
    int  dircheck;
    char filename[256];
} entries;

typedef struct {
    int offset_x;
    int offset_y;
    int display;
    int emulation;
    int interlace;
    char elfpath[2048];
    char savepath[2048];
    u16 PlayerInput[2][11];
} vars;


void SetupGSKit(void);
void InitPS2(void);
void setupPS2Pad(void);

void RunLoaderElf(char *filename, char *party);

char* Browser(int files_too, int inside_menu);
void menu_primitive(char *title, float x1, float y1, float x2, float y2);

void Load_Global_CNF(char *CNF_path_p);
char* Load_Control_CNF(char *CNF_path_p, int port);
void Save_Global_CNF(char *CNF_path_p);
