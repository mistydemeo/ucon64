/*
gd.h - Game Doctor support for uCON64

written by 2002 John Weidman
           2002 dbjh


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
#ifndef GD_H
#define GD_H

extern const char *gd_usage[];


#ifdef BACKUP
int gd_read_rom (const char *filename, unsigned int parport);
int gd_write_rom (const char *filename, unsigned int parport,
                  st_rominfo_t *rominfo);
int gd_read_sram (const char *filename, unsigned int parport);
int gd_write_sram (const char *filename, unsigned int parport);
#endif // BACKUP

#define GD_HEADER_START 0
#define GD_HEADER_LEN 512
#endif // MGD_H
