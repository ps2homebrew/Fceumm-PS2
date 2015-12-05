#include <kernel.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sifrpc.h>
#include <smod.h>
#include <loadfile.h>
#include <libmc.h>
#include <stdio.h>
#include <libpwroff.h>
#include <sbv_patches.h>
#ifdef CDSUPPORT
#include <cdvd_rpc.h>
//#include <SMS_CDVD.h>
#include "cd/cd.h"
#endif
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
int defaultx;
int defaulty;

//input
#define NEW_PADMAN
#include <libpad.h>

//video
GSGLOBAL *gsGlobal;

extern void poweroff_irx;
extern int size_poweroff_irx;
extern void iomanX_irx;
extern int size_iomanX_irx;
extern void fileXio_irx;
extern int size_fileXio_irx;
extern void ps2dev9_irx;
extern int size_ps2dev9_irx;
extern void ps2atad_irx;
extern int size_ps2atad_irx;
extern void ps2hdd_irx;
extern int size_ps2hdd_irx;
extern void ps2fs_irx;
extern int size_ps2fs_irx;
extern void usbd_irx;
extern int size_usbd_irx;
extern void usbhdfsd_irx;
extern int size_usbhdfsd_irx;
#ifdef CDSUPPORT
extern void cdvd_irx;
extern int size_cdvd_irx;
#endif
extern void SMSUTILS_irx;
extern int size_SMSUTILS_irx;

void poweroffps2(int i)
{
    poweroffShutdown();
}

void SetupGSKit(void)
{
    /* detect and set screentype */
    //gsGlobal = gsKit_init_global(GS_MODE_PAL);
    //gsGlobal = gsKit_init_global_custom(GS_RENDER_QUEUE_OS_POOLSIZE+GS_RENDER_QUEUE_OS_POOLSIZE/2, GS_RENDER_QUEUE_PER_POOLSIZE+GS_RENDER_QUEUE_PER_POOLSIZE/2);
	if(gsGlobal!=NULL) gsKit_deinit_global(gsGlobal);
	gsGlobal=gsKit_init_global();
	
    //gsGlobal->Height = 512;//no need for it

    defaultx = gsGlobal->StartX;
    defaulty = gsGlobal->StartY;

    /* initialize dmaKit */
    //dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8);
    dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

    dmaKit_chan_init(DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_FROMSPR);
    dmaKit_chan_init(DMA_CHANNEL_TOSPR);

    gsGlobal->DoubleBuffering = GS_SETTING_OFF;
    gsGlobal->ZBuffering      = GS_SETTING_OFF;

    //640x448, ntsc, tv
    //640x512, pal, tv
    //gsGlobal->Width  = 640;//no need for it

}

void InitPS2(void)
{
	int i, sometime;
	static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
	static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";

	SifInitRpc(0);
	//Reset IOP borrowed from uLaunchelf
	while(!SifIopReset(NULL, 0)){};
	while(!SifIopSync()){};
	SifInitRpc(0);

	sbv_patch_enable_lmb();
		
	SifExecModuleBuffer(&iomanX_irx, size_iomanX_irx, 0, NULL, NULL);
	SifExecModuleBuffer(&fileXio_irx, size_fileXio_irx, 0, NULL, NULL);

	SifLoadModule("rom0:SIO2MAN", 0, NULL);
	SifLoadModule("rom0:MCMAN", 0, NULL);
	SifLoadModule("rom0:MCSERV", 0, NULL);
	SifLoadModule("rom0:PADMAN", 0, NULL);

	SifExecModuleBuffer(&freesd_irx, size_freesd_irx, 0, NULL, NULL);
	SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx, 0, NULL, NULL);
	SifExecModuleBuffer(&SMSUTILS_irx, size_SMSUTILS_irx, 0, NULL, NULL);
	SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, NULL);
	SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, NULL);
	for (i  = 0; i < 3; i++) { //taken from ulaunchelf
		sometime = 0x01000000;
		while(sometime--) asm("nop\nnop\nnop\nnop");
	}

	SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, NULL	);
	SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
	SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, NULL);
	SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx,sizeof(hddarg), hddarg, NULL);
	SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx,sizeof(pfsarg), pfsarg, NULL);
	
    mcInit(MC_TYPE_MC);

#ifdef CDSUPPORT
    cdInit(CDVD_INIT_INIT);
printf("Failed to load module: cdInit");
    CDVD_Init();
printf("Failed to load module: cdInit2\n");
#endif

#ifdef SOUND_ON
    audsrv_init();
#endif

    padInit(0);
}

void normalize_screen(void)
{
    GS_SET_DISPLAY1(gsGlobal->StartX,		// X position in the display area (in VCK unit
        gsGlobal->StartY,		// Y position in the display area (in Raster u
        gsGlobal->MagH,			// Horizontal Magnification
        gsGlobal->MagV,			// Vertical Magnification
        gsGlobal->DW - 1,	// Display area width
        gsGlobal->DH - 1);		// Display area height

    GS_SET_DISPLAY2(gsGlobal->StartX,		// X position in the display area (in VCK units)
        gsGlobal->StartY,		// Y position in the display area (in Raster units)
        gsGlobal->MagH,			// Horizontal Magnification
        gsGlobal->MagV,			// Vertical Magnification
        gsGlobal->DW - 1,	// Display area width
        gsGlobal->DH - 1);		// Display area height
}

void init_custom_screen(void)
{
    //init real non-interlaced mode
    if(Settings.display) {
        gsGlobal->Mode = GS_MODE_PAL;
        gsGlobal->Height = 512;
        defaulty = 72;
    }
    else {
        gsGlobal->Mode = GS_MODE_NTSC;
        gsGlobal->Height = 448;
        defaulty = 50;
    }
	gsGlobal->Field = GS_FIELD;
	gsGlobal->Width = 640;
	
    gsGlobal->StartX = defaultx + Settings.offset_x;
    gsGlobal->StartY = defaulty + Settings.offset_y;

    if(!Settings.interlace) {
        gsGlobal->Interlace = GS_NONINTERLACED;
		gsGlobal->Height = gsGlobal->Height /2;
		gsGlobal->StartY = gsGlobal->StartY/2 + 1;
    }
    //else if (gsGlobal->Mode == GS_MODE_NTSC)
        //gsGlobal->StartY = gsGlobal->StartY + 22;

//    SetGsCrt(gsGlobal->Interlace,gsGlobal->Mode,gsGlobal->Field);
	gsKit_init_screen(gsGlobal);	/* Apply settings. */
	gsKit_mode_switch(gsGlobal, GS_ONESHOT);

//    normalize_screen();

}

void DrawScreen(GSGLOBAL *gsGlobal)
{
    int i;

    i = 0x10000;
    while(i--) asm("nop\nnop\nnop\nnop");

    gsKit_sync_flip(gsGlobal);

    gsKit_queue_exec(gsGlobal);
}

