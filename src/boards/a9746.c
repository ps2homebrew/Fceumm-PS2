/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 CaH4e3
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
 */

#include "mapinc.h"
#include "mmc3.h"

static DECLFW(UNLA9746Write)
{
   FCEU_printf("bs %04x %02x\n",A,V);
   FCEU_printf("gp\n");
   FCEU_printf("gc\n");
   if(A<0xC000)
     MMC3_CMDWrite(A,V);
   else
     MMC3_IRQWrite(A,V);
}

static DECLFW(UNLA9746ProtWrite)
{
   FCEU_printf("bs %04x %02x\n",A,V);
   FCEU_printf("gp\n");
   FCEU_printf("gc\n");
}

static DECLFR(UNLA9746ProtRead)
{
//   FCEU_printf("read %04x\n",A);
   if(A==0x5000) return 0xBF;
   else if(A==0x5800) return 0x3C;
   return 0x60;
}

static void UNLA9746Power(void)
{
  GenMMC3Power();
  SetWriteHandler(0x4020,0x7fff,UNLA9746ProtWrite);
  SetWriteHandler(0x8000,0xffff,UNLA9746Write);
  SetReadHandler(0x4020,0x7fff,UNLA9746ProtRead);
  SetReadHandler(0x8000,0xffff,CartBR);
}

void UNLA9746_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 256, 0, 0);
  info->Power=UNLA9746Power;
  AddExState(EXPREGS, 6, 0, "EXPR");
}
