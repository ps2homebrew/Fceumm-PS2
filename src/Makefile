#PS2SDK = $(PS2DEV)/fceups2sdk

EE_BIN = fceu.elf
EE_CFLAGS = -O3 -Winline -D_GNU_SOURCE -DLSB_FIRST=1 -DPSS_STYLE=1 -DZLIB -DSOUND_ON -DFCEU_VERSION_NUMERIC=9813 -DFCEU_VERSION=\"0.98.13mm\"
#EE_CFLAGS += -fsched-interblock -falign-loops=16 -falign-jumps=16 -falign-functions=16
EE_CFLAGS += -ffast-math -finline-functions -fstrict-aliasing -funsigned-char -fomit-frame-pointer -funroll-loops
EE_INCS = -I$(PS2SDK)/sbv/include -I$(PS2SDK)/ports/include -I$(PS2DEV)/gsKit/include

EE_LDFLAGS = -L$(PS2DEV)/gskit/lib
EE_LIBS = -lm -lgskit -ldmakit -lmc -lpad -laudsrv -lpatches $(PS2SDK)/ports/lib/libz.a
#EE_LIBS += drivers/ps2/lib/libsjpcm.a
RM = rm -f

all: $(EE_BIN)
	ee-strip $(EE_BIN)

include mappers/Makefile
include boards/Makefile
include input/Makefile
include Makefile.base

EE_OBJS = $(OBJECTS) $(MAPPER_OBJS) $(BOARD_OBJS) $(INPUT_OBJS) $(DRIVER_OBJS)

DRIVER_OBJS   = drivers/ps2/irx/audsrv_irx.o drivers/ps2/irx/usbd_irx.o drivers/ps2/irx/usbhdfsd_irx.o drivers/ps2/cnfsettings.o drivers/ps2/Reboot_ELF.o drivers/ps2/init.o drivers/ps2/ps2input.o drivers/ps2/menu.o drivers/ps2/browser.o drivers/ps2/main.o drivers/ps2/ps2stuff.o

#DRIVER_OBJS
drivers/ps2/Reboot_ELF.o:       drivers/ps2/Reboot_ELF.c
drivers/ps2/init.o:             drivers/ps2/init.c
drivers/ps2/cnfsettings.o:      drivers/ps2/cnfsettings.c
drivers/ps2/ps2input.o:         drivers/ps2/ps2input.c
drivers/ps2/menu.o:             drivers/ps2/menu.c
drivers/ps2/browser.o:          drivers/ps2/browser.c
drivers/ps2/ps2stuff.o:         drivers/ps2/ps2stuff.c
drivers/ps2/main.o:             drivers/ps2/main.c

#drivers/ps2/irx/freesd_irx.s:
#	bin2s drivers/ps2/irx/freesd.irx drivers/ps2/irx/freesd_irx.s freesd_irx

drivers/ps2/irx/audsrv_irx.s:
	bin2s drivers/ps2/irx/audsrv.irx drivers/ps2/irx/audsrv_irx.s audsrv_irx

drivers/ps2/irx/usbd_irx.s:
	bin2s drivers/ps2/irx/usbd.irx drivers/ps2/irx/usbd_irx.s usbd_irx

drivers/ps2/irx/usbhdfsd_irx.s:
	bin2s drivers/ps2/irx/usbhdfsd.irx drivers/ps2/irx/usbhdfsd_irx.s usbhdfsd_irx

clean:
	$(RM) fceu.elf $(EE_OBJS) drivers/ps2/irx/*.s

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
