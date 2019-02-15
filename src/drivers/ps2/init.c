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
  extern unsigned char audsrv_irx;
  extern unsigned int size_audsrv_irx;
  extern unsigned char freesd_irx;
  extern unsigned int size_freesd_irx;
#endif

#include "ps2fceu.h"
extern vars Settings;

// Input
#define NEW_PADMAN
#include <libpad.h>
#include <libmtap.h>
extern unsigned char freesio2_irx;
extern unsigned int size_freesio2_irx;
extern unsigned char mcman_irx;
extern unsigned int size_mcman_irx;
extern unsigned char mcserv_irx;
extern unsigned int size_mcserv_irx;
extern unsigned char freemtap_irx;
extern unsigned int size_freemtap_irx;
extern unsigned char freepad_irx;
extern unsigned int size_freepad_irx;

// Video
GSGLOBAL *gsGlobal;

extern unsigned char poweroff_irx;
extern unsigned int size_poweroff_irx;
extern unsigned char iomanX_irx;
extern unsigned int size_iomanX_irx;
extern unsigned char fileXio_irx;
extern unsigned int size_fileXio_irx;
extern unsigned char ps2dev9_irx;
extern unsigned int size_ps2dev9_irx;
extern unsigned char ps2atad_irx;
extern unsigned int size_ps2atad_irx;
extern unsigned char ps2hdd_irx;
extern unsigned int size_ps2hdd_irx;
extern unsigned char ps2fs_irx;
extern unsigned int size_ps2fs_irx;
extern unsigned char usbd_irx;
extern unsigned int size_usbd_irx;
extern unsigned char usbhdfsd_irx;
extern unsigned int size_usbhdfsd_irx;
#ifdef CDSUPPORT
extern unsigned char cdvd_irx;
extern unsigned int size_cdvd_irx;
#endif
extern unsigned char SMSUTILS_irx;
extern unsigned int size_SMSUTILS_irx;

static int VCK;

void poweroffps2(int i)
{
    poweroffShutdown();
}

void SetupGSKit()
{
    /* detect and set screentype */
    if (gsGlobal != NULL) gsKit_deinit_global(gsGlobal);
    gsGlobal = gsKit_init_global();

    /* initialize dmaKit */
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

    dmaKit_chan_init(DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_FROMSPR);
    dmaKit_chan_init(DMA_CHANNEL_TOSPR);

    gsGlobal->DoubleBuffering = GS_SETTING_OFF;
    gsGlobal->ZBuffering      = GS_SETTING_OFF;

    // 640x448, ntsc, tv
    // 640x512, pal, tv
}

void InitPS2()
{
    int i, sometime;
    static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
    static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";

    SifInitRpc(0);
    // Reset IOP borrowed from uLaunchelf
    while (!SifIopReset(NULL, 0)){};
    while (!SifIopSync()){};
    SifInitRpc(0);

    sbv_patch_enable_lmb();

    SifExecModuleBuffer(&iomanX_irx, size_iomanX_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&fileXio_irx, size_fileXio_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&freesio2_irx, size_freesio2_irx, 0, NULL, NULL);

 /* SifLoadModule("rom0:XSIO2MAN", 0, NULL);
    SifLoadModule("rom0:XMCMAN", 0, NULL);
    SifLoadModule("rom0:XMCSERV", 0, NULL);
    SifLoadModule("rom0:XMTAPMAN", 0, NULL);
    SifLoadModule("rom0:XPADMAN", 0, NULL); */

    SifExecModuleBuffer(&mcman_irx, size_mcman_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&mcserv_irx, size_mcserv_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&freemtap_irx, size_freemtap_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&freepad_irx, size_freepad_irx, 0, NULL, NULL);

#ifdef SOUND_ON
    SifExecModuleBuffer(&freesd_irx, size_freesd_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx, 0, NULL, NULL);
#endif
    SifExecModuleBuffer(&SMSUTILS_irx, size_SMSUTILS_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, NULL);
    for (i = 0; i < 3; i++) { // Taken from ulaunchelf
        sometime = 0x01000000;
        while (sometime--) asm("nop\nnop\nnop\nnop");
    }

    SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, NULL);
//    SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
//    SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, NULL);
//    SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg, NULL);
//    SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx, sizeof(pfsarg), pfsarg, NULL);


#ifdef CDSUPPORT
    SifExecModuleBuffer(&cdvd_irx, size_cdvd_irx, 0, NULL, NULL);
    cdInit(CDVD_INIT_INIT);
    CDVD_Init();
#endif

#ifdef SOUND_ON
    audsrv_init();
#endif

    mcInit(MC_TYPE_XMC);

    mtapInit();
    padInit(0);
    mtapPortOpen(0);
}

void init_custom_screen()
{
    if (Settings.display == 0) {
        gsGlobal->Mode = GS_MODE_NTSC;
        gsGlobal->Interlace = GS_INTERLACED;
        gsGlobal->Field = GS_FIELD;
        gsGlobal->Width = 640;
        gsGlobal->Height = 448;
        VCK = 4;
    }
    else if (Settings.display == 1) {
        gsGlobal->Mode = GS_MODE_PAL;
        gsGlobal->Interlace = GS_INTERLACED;
        gsGlobal->Field = GS_FIELD;
        gsGlobal->Width = 640;
        gsGlobal->Height = 512;
        VCK = 4;
    }
    else if (Settings.display == 2) {
        gsGlobal->Mode = GS_MODE_DTV_480P;
        gsGlobal->Interlace = GS_NONINTERLACED;
        gsGlobal->Field = GS_FRAME;
        gsGlobal->Width = 640;
        gsGlobal->Height = 480;
        VCK = 2;
    }

    if (Settings.display == 0 || Settings.display == 1) {
        if (!Settings.interlace) {
            gsGlobal->Interlace = GS_NONINTERLACED;
            gsGlobal->Height = gsGlobal->Height / 2;
            VCK = 2;
        }
    }

    gsKit_vram_clear(gsGlobal);
    gsKit_init_screen(gsGlobal); /* Apply settings. */
    gsKit_set_display_offset(gsGlobal, Settings.offset_x * VCK, Settings.offset_y);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
}

void SetDisplayOffset()
{
    gsKit_set_display_offset(gsGlobal, Settings.offset_x * VCK, Settings.offset_y);
}

void DrawScreen(GSGLOBAL *gsGlobal)
{
    int i;

    i = 0x10000;
    while (i--) asm("nop\nnop\nnop\nnop");

    gsKit_sync_flip(gsGlobal);

    gsKit_queue_exec(gsGlobal);
}

