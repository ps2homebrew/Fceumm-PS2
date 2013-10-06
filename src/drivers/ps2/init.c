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
#include <cdvd_rpc.h>
#include "cd.h"

#include <gsKit.h>
#include <dmaKit.h>

#ifdef SOUND_ON
  #include <audsrv.h>
  extern void audsrv_irx;
  extern int size_audsrv_irx;
#endif

#include "ps2fceu.h"
extern vars Settings;
int defaultx;
int defaulty;

//input
#define NEW_PADMAN
#include "libpad.h"

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
extern void cdvd_irx;
extern int size_cdvd_irx;
extern void SMSUTILS_irx;
extern int size_SMSUTILS_irx;

int LoadBasicModules(void)
{
    int ret = 0,old = 0;

    smod_mod_info_t	mod_t;

    if(!smod_get_mod_by_name("sio2man", &mod_t)) {
        ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL);
    }
    if(mod_t.version == 257)
        old = 1;
    if (ret < 0) {
        printf("Failed to load module: SIO2MAN");
    }
    if(!smod_get_mod_by_name("mcman", &mod_t)) {
        ret = SifLoadModule("rom0:XMCMAN", 0, NULL);
    }
    if(mod_t.version == 257)
        old = 1;
    if (ret < 0) {
        printf("Failed to load module: MCMAN");
    }
    if(!smod_get_mod_by_name("mcserv", &mod_t)) {
        ret = SifLoadModule("rom0:XMCSERV", 0, NULL);
    }
    if(mod_t.version == 257)
        old = 1;
    else
        mcReset();
    if (ret < 0) {
        printf("Failed to load module: MCSERV");
    }
    if(!smod_get_mod_by_name("padman", &mod_t)) {
        ret = SifLoadModule("rom0:XPADMAN", 0, NULL);
    }
    if(mod_t.version == 276)
        old = 1;
    else
        padReset();
    if (ret < 0) {
        printf("Failed to load module: PADMAN");
    }

    return old;
}
void LoadExtraModules(void)
{
    int i,ret,sometime;

#ifdef SOUND_ON
    ret = SifLoadModule("rom0:LIBSD", 0, NULL);
    if (ret < 0) {
        printf("Failed to load module: LIBSD");
    }
    ret = SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: AUDSRV.IRX");
    }
#endif

    ret = SifExecModuleBuffer(&SMSUTILS_irx, size_SMSUTILS_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: SMSUTILS.IRX");
    }

    ret = SifExecModuleBuffer(&usbd_irx, size_usbd_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: USBD.IRX");
    }

    ret = SifExecModuleBuffer(&cdvd_irx, size_cdvd_irx, 0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: CDVD.IRX");
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

void poweroffps2(int i)
{
    poweroffShutdown();
}

void LoadHDDModules(void)
{
    int ret;
    smod_mod_info_t	mod_t;

    if(!smod_get_mod_by_name("Poweroff_Handler", &mod_t))
        ret = SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: POWEROFF.IRX");
    }
    poweroffInit();
    poweroffSetCallback((void *)poweroffps2, NULL);
    if(!smod_get_mod_by_name("IOX/File_Manager", &mod_t))
        ret = SifExecModuleBuffer(&iomanX_irx, size_iomanX_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: IOMANX.IRX");
    }
    if(!smod_get_mod_by_name("IOX/File_Manager_Rpc", &mod_t))
        ret = SifExecModuleBuffer(&fileXio_irx, size_fileXio_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: IOMANX.IRX");
    }
    if(!smod_get_mod_by_name("dev9_driver", &mod_t))
        ret = SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: PS2DEV9.IRX");
    }
    if(!smod_get_mod_by_name("atad", &mod_t))
        ret = SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx,0, NULL, &ret);
    if (ret < 0) {
        printf("Failed to load module: PS2ATAD.IRX");
    }
    static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
    if(!smod_get_mod_by_name("hdd_driver", &mod_t))
        ret = SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx,sizeof(hddarg), hddarg, &ret);
    if (ret < 0) {
        printf("Failed to load module: PS2HDD.IRX");
    }
    static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";
    if(!smod_get_mod_by_name("pfs_driver", &mod_t))
        ret = SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx,sizeof(pfsarg), pfsarg, &ret);
    if (ret < 0) {
        printf("Failed to load module: PS2FS.IRX");
    }
}

void SetupGSKit(void)
{
    /* detect and set screentype */
    //gsGlobal = gsKit_init_global(GS_MODE_PAL);
    gsGlobal = gsKit_init_global_custom(GS_MODE_PAL, GS_RENDER_QUEUE_OS_POOLSIZE+GS_RENDER_QUEUE_OS_POOLSIZE/2, GS_RENDER_QUEUE_PER_POOLSIZE+GS_RENDER_QUEUE_PER_POOLSIZE/2);
    gsGlobal->Height = 512;

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
    gsGlobal->Width  = 640;

}

void InitPS2(void)
{
    SifInitRpc(0);

    //Reset IOP borrowed from uLaunchelf

    if(LoadBasicModules()) {
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

        LoadBasicModules();
    }

    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    LoadExtraModules();
    LoadHDDModules();

    mcInit(MC_TYPE_XMC);
    cdInit(CDVD_INIT_INIT);
    CDVD_Init();

#ifdef SOUND_ON
    audsrv_init();
#endif

    padInit(0);

}

void normalize_screen(void)
{
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

    gsGlobal->StartX = defaultx + Settings.offset_x;
    gsGlobal->StartY = defaulty + Settings.offset_y;

    if(!Settings.interlace) {
        gsGlobal->Interlace = GS_NONINTERLACED;
        gsGlobal->Field = GS_FRAME;
        //gsGlobal->StartY = gsGlobal->StartY/2 + 1;
    }
    //else if (gsGlobal->Mode == GS_MODE_NTSC)
        //gsGlobal->StartY = gsGlobal->StartY + 22;

    SetGsCrt(gsGlobal->Interlace,gsGlobal->Mode,gsGlobal->Field);

    normalize_screen();

}

void DrawScreen(GSGLOBAL *gsGlobal)
{
    int i;

    i = 0x10000;
    while(i--) asm("nop\nnop\nnop\nnop");

    gsKit_sync_flip(gsGlobal);

    gsKit_queue_exec(gsGlobal);
}

