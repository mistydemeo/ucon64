/*
gbx.h - GameBoy Xchanger support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
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
#ifndef GBX_H
#define GBX_H
#define gbx_TITLE "GameBoy Xchanger/GBDoctor\n19XX Bung Enterprises Ltd http://www.bung.com.hk"
#define gbx_HEADER_START 0
#define gbx_HEADER_LEN 0

#ifdef BACKUP
extern int gbx_read_rom (char *filename, unsigned int parport);
extern int gbx_write_rom (char *filename, unsigned int parport);
extern int gbx_read_sram (char *filename, unsigned int parport, int bank);
extern int gbx_write_sram (char *filename, unsigned int parport, int bank);
extern int gbx_usage (int argc, char *argv[]);
#endif // BACKUP

#endif /* GBX_H */
