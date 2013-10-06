#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <malloc.h>
#include <libmc.h>
#include <libpad.h>
#include <string.h>

#include <gsKit.h>

//FCEUltra headers
#include "../../driver.h"
#include "../../types.h"

#include "ps2fceu.h"
extern vars Settings;
/************************************/
/* FCEUltra Variables               */
/************************************/
#ifdef SOUND_ON
  #include <audsrv.h>
  #define SAMPLERATE 44100
#else
  #define SAMPLERATE 0
#endif
FCEUGI *CurGame=NULL;

/************************************/
/* gsKit Variables                  */
/************************************/
GSTEXTURE NES_TEX;
extern GSGLOBAL *gsGlobal;
//unsigned int ps2palette[256];
/* normal palette
u32 NesPalette[ 64 ] =
{
      0x757575, 0x271b8f, 0x0000ab, 0x47009f,
	  0x8f0077, 0xab0013, 0xa70000, 0x7f0b00,
	  0x432f00, 0x004700, 0x005100, 0x003f17,
	  0x1b3f5f, 0x000000, 0x000000, 0x000000,
	  0xbcbcbc, 0x0073ef, 0x233bef, 0x8300f3,
	  0xbf00bf, 0xe7005b, 0xdb2b00, 0xcb4f0f,
	  0x8b7300, 0x009700, 0x00ab00, 0x00933b,
	  0x00838b, 0x000000, 0x000000, 0x000000,
	  0xffffff, 0x3fbfff, 0x5f97ff, 0xa78bfd,
	  0xf77bff, 0xff77b7, 0xff7763, 0xff9b3b,
	  0xf3bf3f, 0x83d313, 0x4fdf4b, 0x58f898,
	  0x00ebdb, 0x000000, 0x000000, 0x000000,
	  0xffffff, 0xabe7ff, 0xc7d7ff, 0xd7cbff,
	  0xffc7ff, 0xffc7db, 0xffbfb3, 0xffdbab,
	  0xffe7a3, 0xe3ffa3, 0xabf3bf, 0xb3ffcf,
	  0x9ffff3, 0x000000, 0x000000, 0x000000
};*/

u32 NesPalette[ 64 ] = //modified palette for GS
{
      0x757575, 0x271b8f, 0x0000ab, 0x47009f, 0x8f0077, 0xab0013, 0xa70000, 0x7f0b00, //1 start
	  0xbcbcbc, 0x0073ef, 0x233bef, 0x8300f3, 0xbf00bf, 0xe7005b, 0xdb2b00, 0xcb4f0f, //2 start
	  0x432f00, 0x004700, 0x005100, 0x003f17, 0x1b3f5f, 0x000000, 0x000000, 0x000000, //1 finish
	  0x8b7300, 0x009700, 0x00ab00, 0x00933b, 0x00838b, 0x000000, 0x000000, 0x000000, //2 finish
	  0xffffff, 0x3fbfff, 0x5f97ff, 0xa78bfd, 0xf77bff, 0xff77b7, 0xff7763, 0xff9b3b, //3 start
	  0xffffff, 0xabe7ff, 0xc7d7ff, 0xd7cbff, 0xffc7ff, 0xffc7db, 0xffbfb3, 0xffdbab, //4 start
      0xf3bf3f, 0x83d313, 0x4fdf4b, 0x58f898, 0x00ebdb, 0x000000, 0x000000, 0x000000, //3 finish
	  0xffe7a3, 0xe3ffa3, 0xabf3bf, 0xb3ffcf, 0x9ffff3, 0x000000, 0x000000, 0x000000  //4 finish
};

/************************************/
/* Prototypes                       */
/************************************/
extern void Set_NESInput();
extern int Get_NESInput();
int PS2_LoadGame(char *path);
void SetupNESTexture();
void SetupNESGS();
void DoFun();

int main(int argc, char *argv[])
{
    int ret;

    //Setup PS2 here
    InitPS2();
    setupPS2Pad();

    Settings.offset_x = -1;

    Load_Global_CNF("mc0:/FCEUMM/FCEUltra.cnf");

    if((Settings.offset_x == -1)) { //initialize default values and try to save them
        printf("Load Settings Failed\n");
        //fioMkdir("mc0:/FCEUMM/");
        Settings.offset_x = gsGlobal->StartX;
        Settings.offset_y = gsGlobal->StartY;
        Settings.interlace = 0;
        //Settings.display already defined
        //Settings.emulation already defined
        strcpy(Settings.elfpath, "mc0:/BOOT/BOOT.elf");
        strcpy(Settings.savepath,"mc0:/FCEUMM/");
        Settings.PlayerInput[0][0]  = PAD_TRIANGLE;
        Settings.PlayerInput[0][1]  = PAD_R2;
        Settings.PlayerInput[0][2]  = PAD_L2;
        Settings.PlayerInput[0][3]  = PAD_CROSS;
        Settings.PlayerInput[0][4]  = PAD_SQUARE;
        Settings.PlayerInput[0][5]  = PAD_SELECT;
        Settings.PlayerInput[0][6]  = PAD_START;
        Settings.PlayerInput[0][7]  = PAD_UP;
        Settings.PlayerInput[0][8]  = PAD_DOWN;
        Settings.PlayerInput[0][9]  = PAD_LEFT;
        Settings.PlayerInput[0][10] = PAD_RIGHT;
        Settings.PlayerInput[1][0]  = 0xFFFF;
        Settings.PlayerInput[1][1]  = 0xFFFF;
        Settings.PlayerInput[1][2]  = 0xFFFF;
        Settings.PlayerInput[1][3]  = PAD_CROSS;
        Settings.PlayerInput[1][4]  = PAD_SQUARE;
        Settings.PlayerInput[1][5]  = PAD_SELECT;
        Settings.PlayerInput[1][6]  = PAD_START;
        Settings.PlayerInput[1][7]  = PAD_UP;
        Settings.PlayerInput[1][8]  = PAD_DOWN;
        Settings.PlayerInput[1][9]  = PAD_LEFT;
        Settings.PlayerInput[1][10] = PAD_RIGHT;
    }

	gsGlobal->StartX = Settings.offset_x;
	gsGlobal->StartY = Settings.offset_y;

    if(!(ret=FCEUI_Initialize())) {
		printf("FCEUltra did not initialize.\n");
		return(0);
	}
	//printf("%s\n",Settings.savepath);

    //Setup FCEUltra here
    FCEUI_SetVidSystem(Settings.emulation); //0=ntsc 1=pal
	FCEUI_SetGameGenie(1);
	FCEUI_DisableSpriteLimitation(1);
#ifdef SOUND_ON
	FCEUI_SetSoundVolume(1024);
#else
	FCEUI_SetSoundVolume(0);
#endif
	FCEUI_SetSoundQuality(0);
	FCEUI_SetLowPass(0);
	FCEUI_Sound(SAMPLERATE);


#ifdef SOUND_ON
	struct audsrv_fmt_t format;
	format.bits = 16;
	format.freq = 44100;
	format.channels = 1;
	audsrv_set_format(&format);
	audsrv_set_volume(MAX_VOLUME);
#endif

    SetupNESTexture();

//main emulation loop
Start_PS2Browser:
    if(PS2_LoadGame((char *)Browser(1,0)) == 0) {
        goto Start_PS2Browser;
    }

    Set_NESInput();
    SetupNESGS();

    while(CurGame) //FCEUI_CloseGame turns this false
        DoFun();

#ifdef SOUND_ON
    audsrv_stop_audio();
#endif
    goto Start_PS2Browser;

    return(0);
}

int PS2_LoadGame(char *path)
{
    FCEUGI *tmp;

//	CloseGame();
    if((tmp=FCEUI_LoadGame(path))) {
        printf("Loaded!\n");
        CurGame=tmp;
        return 1;
    }
    else {
        printf("Didn't load!\n");
        return 0;
    }
}

void SetupNESTexture()
{
    int i,r,g,b;

    //comments after settings are for regular clut lookup
    //Setup NES_TEX Texture
    NES_TEX.PSM = GS_PSM_T8; //GS_PSM_CT32
    NES_TEX.ClutPSM = GS_PSM_CT32; //comment out
    NES_TEX.Clut = memalign(128, gsKit_texture_size_ee(16, 16, NES_TEX.ClutPSM)); //NULL
    NES_TEX.VramClut = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(16, 16, NES_TEX.ClutPSM), GSKIT_ALLOC_USERBUFFER); //0
	NES_TEX.Width = 256;
	NES_TEX.Height = 240;
	NES_TEX.TBW = 4;
	NES_TEX.Filter = GS_FILTER_LINEAR;
    NES_TEX.Mem = memalign(128, gsKit_texture_size_ee(NES_TEX.Width, NES_TEX.Height, NES_TEX.PSM));
    NES_TEX.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(NES_TEX.Width, NES_TEX.Height, NES_TEX.PSM), GSKIT_ALLOC_USERBUFFER);

    //Setup NES Clut
    for( i = 0; i< 64 ; i++ )
    {
        // 32-bit bgr -< rgb
        r =  ( NesPalette[ i ] & 0xff0000 )>>16;
        g =  ( NesPalette[ i ] & 0xff00 )>>8;
        b =  ( NesPalette[ i ] & 0xff )<<0;
        NES_TEX.Clut[ i ] = ((b<<16)|(g<<8)|(r<<0));  //NES_TEX.Clut = ps2palette;
        NES_TEX.Clut[i+64] = ((b<<16)|(g<<8)|(r<<0));
        NES_TEX.Clut[i+128] = ((b<<16)|(g<<8)|(r<<0));
        NES_TEX.Clut[i+196] = ((b<<16)|(g<<8)|(r<<0));
    }

}

void SetupNESGS(void)
{
    gsGlobal->DrawOrder = GS_OS_PER;
    gsKit_mode_switch(gsGlobal, GS_PERSISTENT);

    gsKit_clear(gsGlobal, GS_SETREG_RGBA(0x00,0x00,0x00,0x00));
    gsKit_queue_reset(gsGlobal->Per_Queue);

    //gsKit_prim_sprite_striped_texture( gsGlobal, &NES_TEX,  //thought this might be needed for different modes, but it just looks bad
    gsKit_prim_sprite_texture( gsGlobal, &NES_TEX,
						0.0f, /* X1 */
						0.0f, /* Y1 */
						0.0f, /* U1 */
						0.0f, /* V1 */
                        gsGlobal->Width, /* X2 */ //stretch to screen width
						gsGlobal->Height, /* Y2 */ //stretch to screen height
						NES_TEX.Width, /* U2 */
						NES_TEX.Height, /* V2*/
						1, /* Z */
						GS_SETREG_RGBA(0x80,0x80,0x80,0x80) /* RGBA */
						);

}

void RenderFrame(uint8 *frame)
{
    //int w,h,c;
    //int i;

/*
    for(h=0; h<240; h++) { //correctly displays 256x240 nes screen
        for(w=0; w<256; w++) {
            c = (h << 8) + w; //color index, increments height by 256, then adds width
            NES_TEX.Mem[c] = ps2palette[frame[c]];
        }
    }
*/
    NES_TEX.Mem=(u32 *)frame; //set frame as NES_TEX.Mem location

    gsKit_texture_upload(gsGlobal, &NES_TEX);

    /* vsync and flip buffer */
    gsKit_sync_flip(gsGlobal);

    /* execute render queue */
    gsKit_queue_exec(gsGlobal);
}

#ifdef SOUND_ON
void inline OutputSound(int32 *tmpsnd, int32 ssize)
{
    //used as an example from the windows driver
    /*static int16 MBuffer[2 * 96000 / 50];  // * 2 for safety.
    int P;

    if(!bittage) {
        for(P=0;P<Count;P++)
            *(((uint8*)MBuffer)+P)=((int8)(Buffer[P]>>8))^128;
        RawWrite(MBuffer,Count);
    }
    else {
        for(P=0;P<Count;P++)
        MBuffer[P]=Buffer[P];
        //FCEU_printf("Pre: %d\n",RawCanWrite() / 2);
        RawWrite(MBuffer,Count * 2);
        //FCEU_printf("Post: %d\n",RawCanWrite() / 2);
     }*/

    int i;
    s16 ssound[ssize]; //no need for an 2*ssized 8bit array with this

    //audsrv_wait_audio(ssize<<1); //commented out because the sound buffer is filled at need
    for (i=0;i<ssize;i++) {
        //something[i]=((tmpsnd[i]>>8))^128; //for 8bit sound
        ssound[i]=tmpsnd[i];
    }

    audsrv_play_audio((s8 *)ssound,ssize<<1); //
}
#endif

void FCEUD_Update(uint8 *XBuf, int32 *tmpsnd, int32 ssize)
{
    RenderFrame(XBuf);
#ifdef SOUND_ON
    OutputSound(tmpsnd, ssize);
#endif
    if ( Get_NESInput() ) {
        FCEUI_CloseGame();
        CurGame=0;
    }
}

void DoFun()
{
    uint8 *gfx;
    int32 *sound;
    int32 ssize;

    FCEUI_Emulate(&gfx, &sound, &ssize, 0);
    FCEUD_Update(gfx, sound, ssize);
}
