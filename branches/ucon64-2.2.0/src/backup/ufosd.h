/*
ufosd.h - Super UFO Pro 8 SD support for uCON64

Copyright (c) 2017 - 2018 dbjh


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
#ifndef UFOSD_H
#define UFOSD_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t


extern const st_getopt2_t ufosd_usage[];

/*
Super UFO Pro 8 SD Header Format

Offset       Content
------------ -------------------------------------------------------------------
  0          Size in Mbit
  1          0
  2          Bank type: 0 = HiROM, 1 = LoROM
  3-7        0
  8-0xF      "SFCUFOSD"
  0x10       0
  0x11       Internal size in Mbit
  0x12       SRAM size: 0 = 0 kb, 1 = 16 kb, 2 = 64 kb, 3 = 256 kb, 7 = 1 Mb
  0x13-0x16  Memory map control. Empirical data obtained by dumping cartridges:

             LoROM:
             4 Mbit, 0/2 kB SRAM: [0x13] = 05, [0x14] = 2A
               0 kB SRAM: [0x15] = 0, [0x16] = 0
               2 kB SRAM: [0x15] = 10, [0x16] = 3F
             8 Mbit, 0/2/64 kB SRAM: [0x13] = 15, [0x14] = 28
               0/64 kB SRAM (Wild Trax (J) (V1.1), 64): [0x15] = 0, [0x16] = 0
               2 kB SRAM: [0x15] = 20, [0x16] = 3F
             8/10/16/24/32 Mbit, 0/2/8/32 kB SRAM (not: 8 Mbit, 0/2 kB SRAM): [0x13] = 55
               8/24/32 Mbit, 8/32 kB SRAM: [0x14] = 0
                 8 Mbit, 32 kB SRAM (Star Fox (U) (V1.0)): [0x15] = 40, [0x16] = 0
                 8 Mbit, 8 kB SRAM: [0x15] = 50, [0x16] = BF
                 24 Mbit, 8 kB SRAM: [0x15] = 60, [0x16] = BF
                 32 Mbit, 8 kB SRAM (Hoshi no Kirby Super Deluxe (J) (V1.1)): [0x15] = 80, [0x16] = 0
               10/16 Mbit, 0/2/8 kB SRAM: [0x14] = 20
                 10/16 Mbit, 0 kB SRAM: [0x15] = 0, [0x16] = 0
                 16 Mbit, 2/8 kB SRAM: [0x15] = 20, [0x16] = 3F

             HiROM:
             4 Mbit, 0/2 kB SRAM: [0x13] = 09, [0x14] = 0, [0x15] = 0
               0 kB SRAM: [0x16] = 0
               2 kB SRAM (Super Mario Kart (J)): [0x16] = 2C
             8 Mbit, 0 kB SRAM: [0x13] = 25, [0x14] = 0, [0x15] = 0, [0x16] = 0
             16 Mbit, 8 kB SRAM: [0x13] = 95, [0x14] = 0, [0x15] = 0, [0x16] = 2C
             24 Mbit, 0 kB SRAM: [0x13] = F5, [0x14] = 0, [0x15] = 0, [0x16] = 0
             32 Mbit, 0/2/8 kB SRAM: [0x13] = 55, [0x14] = 0, [0x15] = 80
               0 kB SRAM: [0x16] = 0
               2/8 kB SRAM: [0x16] = 2C

             0x15 SRAM A15 control:
                    00 = A15 not used for SRAM control?
                         LoROM: Use this if SRAM size = 0 kb (no SRAM)
                    10 = A15=x selects SRAM
                    20 = A15=0 selects SRAM
                    30 = A15=1 selects SRAM
                    40/50/60/80 = ? (see LoROM 8/24/32 Mbit, SRAM & HiROM 32 Mbit)
             0x16 SRAM A20 - A23 control:
                  Bit 7 6 5 4 3 2 1 0
                      x x             : A23
                          x x         : A22
                              x x     : A21
                                  x x : A20
                                        00 = Address bit = x selects SRAM
                                        01 = Not used
                                        10 = Address bit = 0 selects SRAM
                                        11 = Address bit = 1 selects SRAM
  0x17       Bank type: 0 = HiROM, 1 = LoROM
  0x18       Television standard: 0 = NTSC, 2 = PAL
  0x19       For non-special chip cartridges: 0; for
             Hoshi no Kirby Super Deluxe (J) (V1.1), Star Fox (U) (V1.0),
             Super Mario Kart (J), Wild Trax (J) (V1.1): 0xFF
  0x1A-0x1F  0
  0x20-0x3F  Copy of last 32 bytes of internal ROM header
  0x40-0x1FF 0
*/

typedef struct st_ufosd_header
{
  unsigned char size;
  unsigned char pad1;
  unsigned char banktype_copy;
  unsigned char pad2[5];
  unsigned char id[8];
  unsigned char pad3;
  unsigned char internal_size;
  unsigned char sram_size;
  unsigned char map_control[4];
  unsigned char banktype;
  unsigned char tvtype;
  unsigned char special_chip;
  unsigned char pad4[6];
  unsigned char internal_header_data[32];
  unsigned char pad5[448];
} st_ufosd_header_t;

#define UFOSD_HEADER_LEN (sizeof (st_ufosd_header_t))

#ifdef  USE_USB
extern int ufosd_write_rom (const char *filename);
#endif

#endif
