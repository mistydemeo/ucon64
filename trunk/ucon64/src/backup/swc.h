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
#include "../ucon64.h"

#define swc_TITLE "Super WildCard 1.6XC/Super WildCard 2.8CC/Super Wild Card DX(2)/SWC\n" \
                  "1993/1994/1995/19XX Front Far East/FFE http://www.front.com.tw"
#define swc_HEADER_START 0
#define swc_HEADER_LEN 512

int swc_read_rom (char *filename, unsigned int parport);
int swc_write_rom (char *filename, unsigned int parport, int sram_size);
int swc_read_sram (char *filename, unsigned int parport);
int swc_write_sram (char *filename, unsigned int parport);
void swc_unlock (unsigned int parport);
int swc_usage (int argc, char *argv[]);

#endif /* SWC_H */
