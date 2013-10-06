/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 Mad Dumper, CaH4e3
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * Panda prince pirate.
 */

#include "mapinc.h"
#include "mmc3.h"

static DECLFW(M121Write)
{
  if((A&0xF003)==0x8003)
  {
    if(V==0xAB) setprg8(0xE000,7);
    else if(V==0xFF) setprg8(0xE000,9);
    else 
    {
      FixMMC3PRG(MMC3_cmd);
      MMC3_CMDWrite(A,V);
    }
  }
  else
  {
    FixMMC3PRG(MMC3_cmd);
    MMC3_CMDWrite(A,V);
  }
}

static DECLFR(M121Read)
{
  return 0x9F;
}

static void M121Power(void)
{
  GenMMC3Power();
  SetReadHandler(0x5000,0x5FFF,M121Read);
  SetWriteHandler(0x8000,0x9FFF,M121Write);
}

void Mapper121_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 256, 0, 0);
  info->Power=M121Power;
}
