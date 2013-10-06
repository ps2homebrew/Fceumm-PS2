#no cdfs support, 1 - have support
CDSUPPORT = 0
EE_BIN = fceu.elf
EE_PACKED_BIN = fceu-packed.elf
EE_CFLAGS = -Winline -DHAVE_ASPRINTF -D_GNU_SOURCE -DLSB_FIRST=1 -DPSS_STYLE=1 -DZLIB -DFCEU_VERSION_NUMERIC=9813 -DFCEU_VERSION=\"0.98.13mm\"
#EE_CFLAGS += -fsched-interblock
#EE_CFLAGS += -finline-functions -funroll-loops
EE_CFLAGS += -Dmemcpy=mips_memcpy -Dmemset=mips_memset -ffast-math  -funroll-loops -fomit-frame-pointer -fstrict-aliasing -funsigned-char -fno-builtin-printf
EE_INCS = -I$(PS2SDK)/ee/include -I$(PS2SDK)/sbv/include -I$(PS2SDK)/ports/include -I$(GSKIT)/include -I$(GSKIT)/ee/dma/include -I$(GSKIT)/ee/gs/include -I$(GSKIT)/ee/toolkit/include
EE_LDFLAGS = -L$(GSKIT)/lib
EE_LIBS = -lgskit_toolkit -lgskit -ldmakit -ljpeg -lpng -lz -lm -lfileXio -lhdd -lmc -lpadx -laudsrv -lpoweroff -lpatches -ldebug
DRIVER_OBJS   = drivers/ps2/cnfsettings.o drivers/ps2/Reboot_ELF.o drivers/ps2/init.o \
    drivers/ps2/font_uLE.o drivers/ps2/ps2font.o drivers/ps2/ps2input.o drivers/ps2/menu.o
ifeq ($(CDSUPPORT),0)
DRIVER_CFLAGS := -D_EE -DSOUND_ON -Dmemcpy=mips_memcpy -Dmemset=mips_memset -O2 -G0 -Wall
endif
ifeq ($(CDSUPPORT),1)
EE_INCS += -I../libcdvd/ee
DRIVER_CFLAGS := -D_EE -DSOUND_ON -DCDSUPPORT -Dmemcpy=mips_memcpy -Dmemset=mips_memset -O2 -G0 -Wall
EE_LDFLAGS += -L../libcdvd/lib
EE_LIBS += -lcdvdfs
DRIVER_OBJS  += drivers/ps2/cd.o
endif
EE_LDFLAGS += -L$(PS2SDK)/ports/lib -s

RM = rm -f

default: all

all: $(EE_BIN)
	ee-strip $(EE_BIN)
	ps2-packer $(EE_BIN) $(EE_PACKED_BIN)


EE_OBJS = $(OBJECTS) $(MAPPER_OBJS) $(BOARD_OBJS) $(INPUT_OBJS) $(DRIVER_OBJS)

DRIVER_OBJS  += drivers/ps2/browser.o drivers/ps2/main.o drivers/ps2/ps2stuff.o \
    drivers/ps2/irx/iomanX_irx.o drivers/ps2/irx/fileXio_irx.o drivers/ps2/irx/ps2dev9_irx.o \
    drivers/ps2/irx/ps2atad_irx.o drivers/ps2/irx/ps2hdd_irx.o drivers/ps2/irx/ps2fs_irx.o \
    drivers/ps2/irx/poweroff_irx.o drivers/ps2/irx/freesd_irx.o drivers/ps2/irx/audsrv_irx.o drivers/ps2/irx/usbd_irx.o \
    drivers/ps2/irx/usbhdfsd_irx.o drivers/ps2/irx/SMSUTILS_irx.o drivers/ps2/SMS_Utils.o
ifeq ($(CDSUPPORT),1)	
DRIVER_OBJS  += drivers/ps2/irx/cdvd_irx.o
endif

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

drivers/ps2/irx/freesd_irx.s:
	bin2s $(PS2SDK)/iop/irx/freesd.irx drivers/ps2/irx/freesd_irx.s freesd_irx

drivers/ps2/irx/audsrv_irx.s:
	bin2s $(PS2SDK)/iop/irx/audsrv.irx drivers/ps2/irx/audsrv_irx.s audsrv_irx

drivers/ps2/irx/poweroff_irx.s:
	bin2s $(PS2SDK)/iop/irx/poweroff.irx drivers/ps2/irx/poweroff_irx.s poweroff_irx

drivers/ps2/irx/usbd_irx.s:
	bin2s $(PS2SDK)/iop/irx/usbd.irx drivers/ps2/irx/usbd_irx.s usbd_irx

drivers/ps2/irx/usbhdfsd_irx.s:
	bin2s $(PS2SDK)/iop/irx/usbhdfsd.irx drivers/ps2/irx/usbhdfsd_irx.s usbhdfsd_irx

ifeq ($(CDSUPPORT),1)
drivers/ps2/irx/cdvd_irx.s:
	bin2s ../libcdvd/lib/cdvd.irx drivers/ps2/irx/cdvd_irx.s cdvd_irx
endif

drivers/ps2/irx/SMSUTILS_irx.s:
	bin2s drivers/ps2/irx/SMSUTILS.irx drivers/ps2/irx/SMSUTILS_irx.s SMSUTILS_irx

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

clean:
	$(RM) $(EE_BIN) $(EE_PACKED_BIN) $(EE_OBJS) drivers/ps2/irx/*.s

include Makefile.base
include Makefile.eeglobal
include $(PS2SDK)/samples/Makefile.pref
#include $(PS2SDK)/samples/Makefile.eeglobal
