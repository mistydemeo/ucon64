/*
sc.h - support for SuperCard

Copyright (c) 2004 NoisyB


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
#ifndef SC_H
#define SC_H

extern const st_getopt2_t sc_usage[];


#define GBA_MENU_SIZE 20916

/*
  sc_sram()  write SuperCard SAV/SRAM template
*/
extern const unsigned char sc_menu_bin[GBA_MENU_SIZE];
extern int sc_sram (const char *fname);

#endif
