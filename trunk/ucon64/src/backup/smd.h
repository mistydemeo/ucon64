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
extern char *smd_title;

#ifdef BACKUP
extern void smd_usage (void);
extern int smd_read_rom (char *filename, unsigned int parport);
extern int smd_write_rom (char *filename, unsigned int parport);
extern int smd_read_sram (char *filename, unsigned int parport);
extern int smd_write_sram (char *filename, unsigned int parport);
#endif // BACKUP

#define smd_HEADER_START 0
#define smd_HEADER_LEN 512

#endif // #ifndef SMD_H
