/*
fal.h - Flash Linker Advance support for uCON64

written by 2001 Jeff Frohwein
           2001 NoisyB (noisyb@gmx.net)
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
#include "../ucon64.h"

#define fal_TITLE "Flash Advance Linker\n2001 Visoly http://www.visoly.com"

int fal_read(char *filename, unsigned int parport, int argc, char *argv[]);
int fal_write(char *filename, long start, long len, unsigned int parport, int argc, char *argv[]);
int fal_usage(int argc, char *argv[]);
