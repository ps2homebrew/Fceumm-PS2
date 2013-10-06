EE_BIN = fceu.elf
EE_CFLAGS = -Winline -DHAVE_ASPRINTF -D_GNU_SOURCE -DLSB_FIRST=1 -DPSS_STYLE=1 -DZLIB -DFCEU_VERSION_NUMERIC=9813 -DFCEU_VERSION=\"0.98.13mm\"
#EE_CFLAGS += -fsched-interblock -falign-loops=32 -falign-jumps=32 -falign-functions=32
#EE_CFLAGS += -finline-functions
EE_CFLAGS += -ffast-math -fomit-frame-pointer -fstrict-aliasing -funroll-loops -funsigned-char -fno-builtin-printf
EE_INCS = -I$(PS2SDK)/sbv/include -I$(PS2SDK)/ports/include -I$(PS2DEV)/gsKit/include
DRIVER_CFLAGS := -D_EE -DSOUND_ON -O2 -G0 -Wall
EE_LDFLAGS = -L$(PS2DEV)/gskit/lib -L$(PS2DEV)/ps2sdk/ports/lib -s
EE_LIBS = -lgskit -ldmakit -ljpg -lpng -lz -lm -lfileXio -lhdd -lmc -lpadx -laudsrv -lpoweroff -lpatches -ldebug
RM = rm -f

all: $(EE_BIN)
	ee-strip $(EE_BIN)

include mappers/Makefile
include boards/Makefile
include input/Makefile
include Makefile.base

EE_OBJS = $(OBJECTS) $(MAPPER_OBJS) $(BOARD_OBJS) $(INPUT_OBJS) $(DRIVER_OBJS)

DRIVER_OBJS   = drivers/ps2/cnfsettings.o drivers/ps2/Reboot_ELF.o drivers/ps2/init.o drivers/ps2/font_uLE.o drivers/ps2/ps2font.o drivers/ps2/ps2input.o drivers/ps2/menu.o drivers/ps2/browser.o drivers/ps2/main.o drivers/ps2/ps2stuff.o drivers/ps2/irx/iomanX_irx.o drivers/ps2/irx/fileXio_irx.o drivers/ps2/irx/ps2dev9_irx.o drivers/ps2/irx/ps2atad_irx.o drivers/ps2/irx/ps2hdd_irx.o drivers/ps2/irx/ps2fs_irx.o drivers/ps2/irx/poweroff_irx.o drivers/ps2/irx/audsrv_irx.o drivers/ps2/irx/usbd_irx.o drivers/ps2/irx/usbhdfsd_irx.o

#DRIVER_OBJS

drivers/ps2/ps2input.o: drivers/ps2/ps2input.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

#drivers/ps2/main.o: drivers/ps2/main.c
#	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

drivers/ps2/%.o: drivers/ps2/%.c
	$(EE_CC) $(DRIVER_CFLAGS) $(EE_INCS) -c $< -o $@

drivers/ps2/irx/iomanX_irx.s:
	bin2s $(PS2SDK)/iop/irx/iomanX.irx drivers/ps2/irx/iomanX_irx.s iomanX_irx

drivers/ps2/irx/fileXio_irx.s:
	bin2s $(PS2SDK)/iop/irx/fileXio.irx drivers/ps2/irx/fileXio_irx.s fileXio_irx

drivers/ps2/irx/ps2dev9_irx.s:
	bin2s $(PS2SDK)/iop/irx/ps2dev9.irx drivers/ps2/irx/ps2dev9_irx.s ps2dev9_irx

drivers/ps2/irx/ps2atad_irx.s:
	bin2s $(PS2SDK)/iop/irx/ps2atad.irx drivers/ps2/irx/ps2atad_irx.s ps2atad_irx

drivers/ps2/irx/ps2hdd_irx.s:
	bin2s $(PS2SDK)/iop/irx/ps2hdd.irx drivers/ps2/irx/ps2hdd_irx.s ps2hdd_irx

drivers/ps2/irx/ps2fs_irx.s:
	bin2s $(PS2SDK)/iop/irx/ps2fs.irx drivers/ps2/irx/ps2fs_irx.s ps2fs_irx

drivers/ps2/irx/audsrv_irx.s:
	bin2s $(PS2SDK)/iop/irx/audsrv.irx drivers/ps2/irx/audsrv_irx.s audsrv_irx

drivers/ps2/irx/poweroff_irx.s:
	bin2s $(PS2SDK)/iop/irx/poweroff.irx drivers/ps2/irx/poweroff_irx.s poweroff_irx

drivers/ps2/irx/usbd_irx.s:
	bin2s $(PS2SDK)/iop/irx/usbd.irx drivers/ps2/irx/usbd_irx.s usbd_irx

drivers/ps2/irx/usbhdfsd_irx.s:
	bin2s $(PS2SDK)/iop/irx/usbhdfsd.irx drivers/ps2/irx/usbhdfsd_irx.s usbhdfsd_irx

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

clean:
	$(RM) fceu.elf $(EE_OBJS) drivers/ps2/irx/*.s

include $(PS2SDK)/samples/Makefile.pref
include Makefile.eeglobal
