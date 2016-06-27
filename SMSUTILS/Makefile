IOP_BIN      = SMSUTILS.irx
IOP_OBJS     = SMSUTILS.o
IOP_CFLAGS  += -Wall -fno-builtin-printf
IOP_LDFLAGS  = -Wl,-s

all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.iopglobal
