#ifndef _CDVD_IOP_H
#define _CDVD_IOP_H

#include "../common/cdvd.h"
#include <cdvdman.h>

/* Use SDK types directly to avoid conflicts with libcdvd-common.h macro aliases */
typedef sceCdCLOCK   CdCLOCK;
typedef sceCdlFILE   CdlFILE;
typedef sceCdlLOCCD  CdlLOCCD;
typedef sceCdRMode   CdRMode;

/* Local functions not provided by the SDK */
int CdFlushCache(void);
unsigned int CdGetSize(void);

#endif  // _CDVD_IOP_H
