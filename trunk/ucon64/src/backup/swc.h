/*
swc.h - Super Wild Card support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2001 dbjh


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
#ifndef SWC_H
#define SWC_H

extern const char *swc_usage[];
/*
    0      - Low byte of 8kB page count
    1      - High byte of 8kB page count
    2      - Emulation Mode Select
             Bit 7 6 5 4 3 2 1 0
                 x               : 0 = Run program in mode 1 (JMP RESET Vector)
                                 : 1 = Run in mode 0 (JMP $8000)
                   x             : 0 = Last file of the ROM dump (multi-file)
                                 : 1 = Multi-file (there is another file to follow)
                     x           : 0 = SRAM memory mapping mode 20, 1 = mode 21 (HiROM)
                       x         : 0 = DRAM memory mapping mode 20, 1 = mode 21 (HiROM)
                         x x     : 00 = 256kb SRAM, 01 = 64kb, 10 = 16kb, 11 = no SRAM
                             x   : 0 = Disable, 1 = Enable external cartridge
                               x : reserved
    3-7    - 0, reserved
    8      - File ID code 1 (0xaa)
    9      - File ID code 2 (0xbb)
    10     - File type; check this byte only if ID 1 & 2 match
             2 : Magic Griffin program (PC Engine)
             3 : Magic Griffin SRAM data
             4 : SNES program
             5 : SWC & SMC password, SRAM data
             6 : Megadrive program
             7 : SMD SRAM data
    11-511 - 0, reserved
*/
typedef struct st_swc_header
{
/*
  Don't create fields that are larger than one byte! For example size_low and size_high
  could be combined in one unsigned short int. However, this gives problems with little
  endian vs. big endian machines (e.g. writing the header to disk).
*/
  unsigned char size_low;
  unsigned char size_high;
  unsigned char emulation;
  unsigned char pad[5];
  unsigned char id_code1;
  unsigned char id_code2;
  unsigned char type;
  unsigned char pad2[501];
} st_swc_header_t;

#define SWC_HEADER_START 0
#define SWC_HEADER_LEN (sizeof (st_swc_header_t))

#ifdef BACKUP
extern int swc_read_rom (const char *filename, unsigned int parport);
extern int swc_write_rom (const char *filename, unsigned int parport);
extern int swc_read_sram (const char *filename, unsigned int parport);
extern int swc_write_sram (const char *filename, unsigned int parport);
extern void swc_unlock (unsigned int parport);
#endif // BACKUP

#endif // SWC_H
