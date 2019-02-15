#------------------------------------------------------------------
# GCC 3.2.3 / PS2SDK 1.2 / GSKIT / JPG / PNG
#------------------------------------------------------------------
#no cdfs support, 1 - have support
CDSUPPORT = 0
DEBUG = 0
EE_BIN = fceu.elf
EE_PACKED_BIN = fceu-packed.elf
ZLIB_DIR := ./src/zlib
ENDIANNESS_DEFINES = -DLSB_FIRST -DLOCAL_LE=1
PLATFORM_DEFINES = -D__PS2__ -DHAVE_ASPRINTF -Dmemcpy=mips_memcpy -Dmemset=mips_memset

#Nor stripping neither compressing binary ELF after compiling.
NOT_PACKED ?= 0

ifeq ($(DEBUG), 1)
	PLATFORM_DEFINES += -O0 -g
else
	PLATFORM_DEFINES += -O3 -DNDEBUG
endif

FCEU_DEFINES := -DPATH_MAX=1024 -DINLINE=inline -DPSS_STYLE=1 -DFCEU_VERSION_NUMERIC=9813 $(PLATFORM_DEFINES)

#EE_CFLAGS = -Winline -DHAVE_ASPRINTF -D_GNU_SOURCE -DPSS_STYLE=1 -DZLIB -DFCEU_VERSION_NUMERIC=9813 -D__PS2__
#EE_CFLAGS += -fsched-interblock
#EE_CFLAGS += -finline-functions -funroll-loops
EE_CFLAGS += -ffast-math  -funroll-loops -fomit-frame-pointer -fstrict-aliasing -funsigned-char -fno-builtin-printf
EE_CFLAGS += $(FCEU_DEFINES) $(ENDIANNESS_DEFINES)
EE_LDFLAGS = -L$(GSKIT)/lib

EE_INCS = -I$(PS2SDK)/ee/include -I$(PS2SDK)/sbv/include -I$(PS2SDK)/ports/include \
	-I$(GSKIT)/include -I$(GSKIT)/ee/dma/include -I$(GSKIT)/ee/gs/include -I$(GSKIT)/ee/toolkit/include
EE_LIBS = -lgskit_toolkit -lgskit -ldmakit -ljpeg -lpng -lz -lm -lfileXio -lhdd -lmc -lpadx -lc -lmtap -laudsrv -lpoweroff -lpatches -ldebug

ifeq ($(CDSUPPORT),1)
	EE_INCS += -Ilibcdvd/ee
	EE_LDFLAGS += -Llibcdvd/lib
	EE_LIBS += -lcdvdfs
endif

EE_LDFLAGS += -L$(PS2SDK)/ports/lib -s

RM = rm -f

default: all

all: $(EE_BIN)
	$(EE_STRIP) $(EE_BIN)
ifneq ($(NOT_PACKED),1)
	ps2-packer $(EE_BIN) $(EE_PACKED_BIN)
endif

PS2_DIR := ./src/drivers/ps2
IRX_DIR=$(PS2SDK)/iop/irx
FCEU_DIR := ./src

#DRIVER_OBJS
DRIVER_OBJS += iomanX_irx.o fileXio_irx.o ps2dev9_irx.o \
    ps2atad_irx.o ps2hdd_irx.o ps2fs_irx.o \
    poweroff_irx.o freesd_irx.o audsrv_irx.o usbd_irx.o \
    usbhdfsd_irx.o SMSUTILS_irx.o
ifeq ($(CDSUPPORT),1)	
	DRIVER_OBJS  += cdvd_irx.o
endif
DRIVER_OBJS += freesio2_irx.o mcman_irx.o mcserv_irx.o freemtap_irx.o freepad_irx.o
DRIVER_OBJS := $(DRIVER_OBJS:%=$(PS2_DIR)/irx/%)
DRIVER_OBJS += $(PS2_DIR)/SMS_Utils.o
DRIVER_CFLAGS := -D_EE -DSOUND_ON -O2 -G0 -Wall $(PLATFORM_DEFINES)
#DRIVER_CFLAGS := -D_EE -O2 -G0 -Wall $(PLATFORM_DEFINES)
ifeq ($(CDSUPPORT),1)
	DRIVER_CFLAGS += -DCDSUPPORT
	DRIVER_OBJS  += $(PS2_DIR)/cd/cd.o
endif


FCEU_SRC_DIRS := $(PS2_DIR) $(FCEU_DIR) $(FCEU_DIR)/boards $(FCEU_DIR)/input  $(FCEU_DIR)/mappers $(ZLIB_DIR)

FCEU_CSRCS := $(foreach dir,$(FCEU_SRC_DIRS),$(wildcard $(dir)/*.c))
FCEU_COBJ := $(FCEU_CSRCS:.c=.o)

EE_OBJS = $(FCEU_COBJ) $(DRIVER_OBJS)

$(PS2_DIR)/ps2input.o: $(PS2_DIR)/ps2input.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

#$(PS2_DIR)/main.o: $(PS2_DIR)/main.c+
#	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(PS2_DIR)/%.o: $(PS2_DIR)/%.c
	$(EE_CC) $(DRIVER_CFLAGS) $(EE_INCS) -c $< -o $@

$(PS2_DIR)/irx/freesio2_irx.c:
	bin2c $(IRX_DIR)/freesio2.irx $(PS2_DIR)/irx/freesio2_irx.c freesio2_irx

$(PS2_DIR)/irx/mcman_irx.c:
	bin2c $(IRX_DIR)/mcman.irx $(PS2_DIR)/irx/mcman_irx.c mcman_irx

$(PS2_DIR)/irx/mcserv_irx.c:
	bin2c $(IRX_DIR)/mcserv.irx $(PS2_DIR)/irx/mcserv_irx.c mcserv_irx

$(PS2_DIR)/irx/freemtap_irx.c:
	bin2c $(IRX_DIR)/freemtap.irx $(PS2_DIR)/irx/freemtap_irx.c freemtap_irx

$(PS2_DIR)/irx/freepad_irx.c:
	bin2c $(IRX_DIR)/freepad.irx $(PS2_DIR)/irx/freepad_irx.c freepad_irx

$(PS2_DIR)/irx/iomanX_irx.c:
	bin2c $(IRX_DIR)/iomanX.irx $(PS2_DIR)/irx/iomanX_irx.c iomanX_irx

$(PS2_DIR)/irx/fileXio_irx.c:
	bin2c $(IRX_DIR)/fileXio.irx $(PS2_DIR)/irx/fileXio_irx.c fileXio_irx

$(PS2_DIR)/irx/ps2dev9_irx.c:
	bin2c $(IRX_DIR)/ps2dev9.irx $(PS2_DIR)/irx/ps2dev9_irx.c ps2dev9_irx

$(PS2_DIR)/irx/ps2atad_irx.c:
	bin2c $(IRX_DIR)/ps2atad.irx $(PS2_DIR)/irx/ps2atad_irx.c ps2atad_irx

$(PS2_DIR)/irx/ps2hdd_irx.c:
	bin2c $(IRX_DIR)/ps2hdd.irx $(PS2_DIR)/irx/ps2hdd_irx.c ps2hdd_irx

$(PS2_DIR)/irx/ps2fs_irx.c:
	bin2c $(IRX_DIR)/ps2fs.irx $(PS2_DIR)/irx/ps2fs_irx.c ps2fs_irx

$(PS2_DIR)/irx/freesd_irx.c:
	bin2c $(IRX_DIR)/freesd.irx $(PS2_DIR)/irx/freesd_irx.c freesd_irx

$(PS2_DIR)/irx/audsrv_irx.c:
	bin2c $(IRX_DIR)/audsrv.irx $(PS2_DIR)/irx/audsrv_irx.c audsrv_irx

$(PS2_DIR)/irx/poweroff_irx.c:
	bin2c $(IRX_DIR)/poweroff.irx $(PS2_DIR)/irx/poweroff_irx.c poweroff_irx

$(PS2_DIR)/irx/usbd_irx.c:
	bin2c $(IRX_DIR)/usbd.irx $(PS2_DIR)/irx/usbd_irx.c usbd_irx

$(PS2_DIR)/irx/usbhdfsd_irx.c:
	bin2c $(IRX_DIR)/usbhdfsd.irx $(PS2_DIR)/irx/usbhdfsd_irx.c usbhdfsd_irx

ifeq ($(CDSUPPORT),1)
$(PS2_DIR)/irx/cdvd_irx.c:
	$(MAKE) -C libcdvd
	bin2c libcdvd/lib/cdvd.irx $(PS2_DIR)/irx/cdvd_irx.c cdvd_irx
endif

$(PS2_DIR)/irx/SMSUTILS_irx.c:
	$(MAKE) -C SMSUTILS
	bin2c SMSUTILS/SMSUTILS.irx $(PS2_DIR)/irx/SMSUTILS_irx.c SMSUTILS_irx

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

clean:
	$(MAKE) -C libcdvd clean
	$(MAKE) -C SMSUTILS clean
	$(RM) $(EE_BIN) $(EE_PACKED_BIN) $(EE_OBJS) $(PS2_DIR)/irx/*.c

#include Makefile.eeglobal
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
