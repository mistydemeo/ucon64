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
extern const char *smd_title;

#if 0
        Important SMD Header Bytes:

        Offset 00h:     Number of 16KB blocks. This number may be incorrect in
                        some files, it is recommended you calculate it
                        manually: num_blocks = (sizeof(file) - 512) div 16384
        Offset 02h:     Indicates wether ROM is a part of a series of a split
                        ROM (1, or possibly just non-zero,) or wether it is
                        a standalone ROM or the last ROM in a split series (0)
        Offset 08h:     AAh
        Offset 09h:     BBh

        * Note: Some documentation claims that byte 1 is always 3 and all
          other bytes besides those mentioned as important are 0. This is not
          always the case. The header information in this document should be
          considered fairly accurate.
#endif
typedef struct st_smd_header
{
  unsigned char size;
  char pad;
  unsigned char split;
  char pad2[5];
  unsigned char code1;
  unsigned char code2;
  char pad3[502];
} st_smd_header_t;

#ifdef BACKUP
extern void smd_usage (void);
extern int smd_read_rom (char *filename, unsigned int parport);
extern int smd_write_rom (char *filename, unsigned int parport);
extern int smd_read_sram (char *filename, unsigned int parport);
extern int smd_write_sram (char *filename, unsigned int parport);
#endif // BACKUP

#define SMD_HEADER_START 0
#define SMD_HEADER_LEN (sizeof (st_smd_header_t))

#endif // #ifndef SMD_H
