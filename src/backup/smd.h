/*
smd.h - Super Magic Drive support for uCON64

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
#ifndef SMD_H
#define SMD_H
extern const char *smd_usage[];

/*
    0      - Low byte of 16kB page count
    1      - 3 (not: High byte of 16kB page count)
    2      - Multi
             Bit 7 6 5 4 3 2 1 0
                   x             : 0 = Last file of the ROM dump (multi-file)
                                 : 1 = Multi-file (there is another file to follow)
                 x   x x x x x x : reserved (meaning for other FFE copier types)
    3-7    - 0, reserved
    8      - File ID code 1 (0xaa)
    9      - File ID code 2 (0xbb)
    10     - File type; check this byte only if ID 1 & 2 match
             6 : Megadrive program
             7 : SMD SRAM data
    11-511 - 0, reserved
*/
typedef struct st_smd_header
{
  unsigned char low;
  unsigned char high;
  unsigned char split;
  char pad2[5];
  unsigned char code1;
  unsigned char code2;
  unsigned char type;
  char pad3[501];
} st_smd_header_t;

#ifdef BACKUP
extern int smd_read_rom (const char *filename, unsigned int parport);
extern int smd_write_rom (const char *filename, unsigned int parport);
extern int smd_read_sram (const char *filename, unsigned int parport);
extern int smd_write_sram (const char *filename, unsigned int parport);
#endif // BACKUP

#define SMD_HEADER_START 0
#define SMD_HEADER_LEN (sizeof (st_smd_header_t))

#endif // #ifndef SMD_H
