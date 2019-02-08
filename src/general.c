/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fceu-types.h"
#include "fceu.h"

#include "general.h"
#include "state.h"
#include "movie.h"

#include "driver.h"

#include "md5.h"

static char BaseDirectory[2048];
static char FileBase[2048];
static char FileExt[2048];	/* Includes the . character, as in ".nes" */

static char FileBaseDirectory[2048];

void FCEUI_SetBaseDirectory(char *dir) {
	strncpy(BaseDirectory, dir, 2047);
	BaseDirectory[2047] = 0;
}

static char *odirs[FCEUIOD__COUNT] = { 0, 0, 0, 0, 0, 0 };		// odirs, odors. ^_^

void FCEUI_SetDirOverride(int which, char *n) {
	odirs[which] = n;

	if (GameInfo) {	/* Rebuild cache of present states/movies. */
		if (which == FCEUIOD_STATE)
			FCEUSS_CheckStates();
		else if (which == FCEUIOD_MOVIE)
			FCEUMOV_CheckMovies();
	}
}

#ifndef HAVE_ASPRINTF
static int asprintf(char **strp, const char *fmt, ...) {
	va_list ap;
	int ret;

	va_start(ap, fmt);
	if (!(*strp = malloc(2048)))
		return(0);
	ret = vsnprintf(*strp, 2048, fmt, ap);
	va_end(ap);
	return(ret);
}
#endif

char *FCEU_MakeFName(int type, int id1, char *cd1) {
	char *ret = 0;
	struct stat tmpstat;

	switch (type) {
	case FCEUMKF_NPTEMP: asprintf(&ret, "%s"PSS "m590plqd94fo.tmp", BaseDirectory); break;
	case FCEUMKF_MOVIE:
		if (odirs[FCEUIOD_STATE])
			asprintf(&ret, "%s"PSS "%s.%d.fcm", odirs[FCEUIOD_STATE], FileBase, id1);
		else
			asprintf(&ret, "%s"PSS "fcs"PSS "%s.%d.fcm", BaseDirectory, FileBase, id1);
		break;
	case FCEUMKF_STATE:
		if (odirs[FCEUIOD_STATE])
			asprintf(&ret, "%s"PSS "%s.fc%d", odirs[FCEUIOD_STATE], FileBase, id1);
		else
			asprintf(&ret, "%s"PSS "fcs"PSS "%s.fc%d", BaseDirectory, FileBase, id1);
		break;
	case FCEUMKF_SNAP:
		if (FSettings.SnapName) {
			if (odirs[FCEUIOD_SNAPS])
				asprintf(&ret, "%s"PSS "%s-%d.%s", odirs[FCEUIOD_SNAPS], FileBase, id1, cd1);
			else
				asprintf(&ret, "%s"PSS "snaps"PSS "%s-%d.%s", BaseDirectory, FileBase, id1, cd1);
		} else {
			if (odirs[FCEUIOD_SNAPS])
				asprintf(&ret, "%s"PSS "%d.%s", odirs[FCEUIOD_SNAPS], id1, cd1);
			else
				asprintf(&ret, "%s"PSS "snaps"PSS "%d.%s", BaseDirectory, id1, cd1);
		}
		break;
	case FCEUMKF_FDS:
		if (odirs[FCEUIOD_NV])
			asprintf(&ret, "%s"PSS "%s.%s.fds", odirs[FCEUIOD_NV], FileBase, md5_asciistr(GameInfo->MD5));
		else
			asprintf(&ret, "%s"PSS "sav"PSS "%s.%s.fds", BaseDirectory, FileBase, md5_asciistr(GameInfo->MD5));
		break;
	case FCEUMKF_SAV:
		if (odirs[FCEUIOD_NV])
			asprintf(&ret, "%s"PSS "%s.%s", odirs[FCEUIOD_NV], FileBase, cd1);
		else
			asprintf(&ret, "%s"PSS "sav"PSS "%s.%s", BaseDirectory, FileBase, cd1);
		break;
	case FCEUMKF_CHEAT:
		if (odirs[FCEUIOD_CHEATS])
			asprintf(&ret, "%s"PSS "%s.cht", odirs[FCEUIOD_CHEATS], FileBase);
		else
			asprintf(&ret, "%s"PSS "cheats"PSS "%s.cht", BaseDirectory, FileBase);
		break;
	case FCEUMKF_IPS:  asprintf(&ret, "%s"PSS "%s%s.ips", FileBaseDirectory, FileBase, FileExt);
		break;
	case FCEUMKF_GGROM: asprintf(&ret, "%s"PSS "gg.rom", BaseDirectory); break;
	case FCEUMKF_FDSROM: asprintf(&ret, "%s"PSS "disksys.rom", BaseDirectory); break;
	case FCEUMKF_PALETTE:
		if (odirs[FCEUIOD_MISC])
			asprintf(&ret, "%s"PSS "%s.pal", odirs[FCEUIOD_MISC], FileBase);
		else
			asprintf(&ret, "%s"PSS "gameinfo"PSS "%s.pal", BaseDirectory, FileBase);
		break;
	}
	return(ret);
}

void GetFileBase(const char *f) {
	const char *tp1, *tp3;

 #if PSS_STYLE == 4
	tp1 = ((char*)strrchr(f, ':'));
 #elif PSS_STYLE == 1
	tp1 = ((char*)strrchr(f, '/'));
 #else
	tp1 = ((char*)strrchr(f, '\\'));
	#if PSS_STYLE != 3
	tp3 = ((char*)strrchr(f, '/'));
	if (tp1 < tp3) tp1 = tp3;
	#endif
 #endif
	if (!tp1) {
		tp1 = f;
		strcpy(FileBaseDirectory, ".");
	} else {
		memcpy(FileBaseDirectory, f, tp1 - f);
		FileBaseDirectory[tp1 - f] = 0;
		tp1++;
	}

	if (((tp3 = strrchr(f, '.')) != NULL) && (tp3 > tp1)) {
		memcpy(FileBase, tp1, tp3 - tp1);
		FileBase[tp3 - tp1] = 0;
		strcpy(FileExt, tp3);
	} else {
		strcpy(FileBase, tp1);
		FileExt[0] = 0;
	}
}

uint32 uppow2(uint32 n) {
	int x;

	for (x = 31; x >= 0; x--)
		if (n & (1 << x)) {
			if ((1 << x) != n)
				return(1 << (x + 1));
			break;
		}
	return n;
}

