/*
fig.h - Super PRO Fighter support for uCON64

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef FIG_H
#define FIG_H
extern const char *fig_usage[];

#ifdef BACKUP
#endif // BACKUP

/*
Offset Function
  0    Lower 8 bits of size word
  1    Upper 8 bits of size word
  2    40h - Multi image
       00h - Last image in set (or single image)
  3    80h - if HiROM
       00h - if LoROM
  4    If using FX microchip:
           11h
       Else if using DSP1 microchip:
           FDh - If using SRAM (SRAM size>0)
           47h - If no SRAM (SRAM size=0)
       If not using DSP1 microchip:
           00h - If using SRAM
           77h - If no SRAM
  5    If using FX microchip:
           2
       Else if using DSP1 microchip:
           82h - If using SRAM
           83h - If no SRAM
       If not using DSP1 microchip:
           80h - If using SRAM
           83h - If no SRAM
*/
typedef struct st_fig_header
{
/*
  Don't create fields that are larger than one byte! For example size_low and size_high
  could be combined in one unsigned short int. However, this gives problems with little
  endian vs. big endian machines (e.g. writing the header to disk).
*/
  unsigned char size_low;
  unsigned char size_high;
  unsigned char multi;
  unsigned char hirom;
  unsigned char emulation1;
  unsigned char emulation2;
  unsigned char pad[506];
} st_fig_header_t;

#define FIG_HEADER_START 0
#define FIG_HEADER_LEN (sizeof (st_fig_header_t))
#endif // FIG_H
