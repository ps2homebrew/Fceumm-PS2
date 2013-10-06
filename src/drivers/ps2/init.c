#include <kernel.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libmc.h>
#include <stdio.h>

#include <sbv_patches.h>

#include <gsKit.h>
#include <dmaKit.h>

#ifdef SOUND_ON
  #include <audsrv.h>
  extern void audsrv_irx;
  extern int size_audsrv_irx;
  extern void freesd_irx;
  extern int size_freesd_irx;
#endif

#include "ps2fceu.h"
extern vars Settings;

//defines TYPE_XMC && NEW_PADMAN must go together based on what I read
//memory card
#define TYPE_MC
//#define TYPE_XMC

//input
#define ROM_PADMAN
//#define NEW_PADMAN
#include "libpad.h"

//video
GSGLOBAL *gsGlobal;
GSFONT *gsFont;

extern void usbd_irx;
extern int size_usbd_irx;
extern void usbhdfsd_irx;
extern int size_usbhdfsd_irx;

void LoadModules(void)
{
    int i,ret,sometime;

#ifdef TYPE_MC
	ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: SIO2MAN");
	}

	ret = SifLoadModule("rom0:MCMAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCMAN");
	}

	ret = SifLoadModule("rom0:MCSERV", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCSERV");
	}
#else
	ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: SIO2MAN");
	}

	ret = SifLoadModule("rom0:XMCMAN", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCMAN");
	}

	ret = SifLoadModule("rom0:XMCSERV", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: MCSERV");
    }
#endif
#ifdef ROM_PADMAN
    ret = SifLoadModule("rom0:PADMAN", 0, NULL);
#else
    ret = SifLoadModule("rom0:XPADMAN", 0, NULL);
#endif
    if (ret < 0) {
        printf("sifLoadModule pad failed: %d\n", ret);
    }

#ifdef SOUND_ON
    ret = SifLoadModule("rom0:LIBSD", 0, NULL);
    //ret = SifExecModuleBuffer(&freesd_irx, size_freesd_irx,0, NULL, &ret);
	if (ret < 0) {
        printf("Failed to load module: LIBSD");
    }
    ret = SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx,0, NULL, &ret);
	if (ret < 0) {
        printf("Failed to load module: AUDSRV.IRX");
	}
#endif

    ret = SifExecModuleBuffer(&usbd_irx, size_usbd_irx,0, NULL, &ret);
	if (ret < 0) {
        printf("Failed to load module: USBD.IRX");
	}

    ret = SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx,0, NULL, &ret);
    for (i  = 0; i < 3; i++) { //taken from ulaunchelf
        sometime = 0x01000000;
		while(sometime--) asm("nop\nnop\nnop\nnop");
	}
	if (ret < 0) {
        printf("Failed to load module: USBHDFSD.IRX");
	}
}

int InitGSKit(void)
{
    /* detect and set screentype */
    if(gsKit_detect_signal()==GS_MODE_PAL) {
        gsGlobal = gsKit_init_global(GS_MODE_PAL);
        Settings.emulation = 1;
        Settings.display = 1;
        gsGlobal->Height = 512;
    }
    else {
        gsGlobal = gsKit_init_global(GS_MODE_NTSC);
        Settings.emulation = 0;
        Settings.display = 0;
        gsGlobal->Height = 448;
    }

    /* initialize dmaKit */
    //dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8);
    dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

    dmaKit_chan_init(DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_FROMSPR);
    dmaKit_chan_init(DMA_CHANNEL_TOSPR);

	gsGlobal->DoubleBuffering = GS_SETTING_OFF;
	gsGlobal->ZBuffering      = GS_SETTING_OFF;
	gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
    //gsGlobal->PSM = GS_PSM_CT32;

    gsKit_init_screen(gsGlobal);

    //640x448, ntsc, tv
    gsGlobal->Width  = 640;
    //gsGlobal->Height = 448;

    //640x512, pal, tv
    //gsGlobal->Height = 512;

    gsFont = gsKit_init_font(GSKIT_FTYPE_FONTM, NULL);
    //gsFont = gsKit_init_font(GSKIT_FTYPE_BMP_DAT, "host:arial.fnt"); //I couldn't get this to work...
    gsFont->FontM_Spacing = 0.90f;
    gsKit_font_upload(gsGlobal, gsFont); //upload once
    return 0;
}

void InitPS2(void)
{
	SifInitRpc(0);
//Reset IOP borrowed from uLaunchelf
	SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
	while(!SifIopSync());
	fioExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();
	SifInitRpc(0);
	FlushCache(0);
	FlushCache(2);

	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();

	LoadModules();

#ifdef TYPE_MC
	if(mcInit(MC_TYPE_MC) < 0) {
		printf("Failed to initialise memcard server!\n");
		SleepThread();
	}
#else
	if(mcInit(MC_TYPE_XMC) < 0) {
		printf("Failed to initialise memcard server!\n");
		SleepThread();
	}
#endif

	InitGSKit();

#ifdef SOUND_ON
    audsrv_init();
#endif

    padInit(0);
}
