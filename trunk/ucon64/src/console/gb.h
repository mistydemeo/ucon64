/*
gb.h - Game Boy support for uCON64

Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2018        dbjh


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
#ifndef GB_H
#define GB_H

#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"


#define GB_LOGODATA_LEN 48
#define GB_NAME_LEN 15                          // correct for GBC games, not
                                                //  for {S}GB games (should be 16)
extern const st_getopt2_t gb_usage[];
extern const unsigned char gb_logodata[], rocket_logodata[];

extern int gb_chk (st_ucon64_nfo_t *rominfo);
extern int gb_gbx (st_ucon64_nfo_t *rominfo);
extern int gb_gp2bmp (void);
extern int gb_mgd (st_ucon64_nfo_t *rominfo);
extern int gb_n (st_ucon64_nfo_t *rominfo, const char *name);
extern int gb_n2gb (st_ucon64_nfo_t *rominfo, const char *emu_rom);
extern int gb_sgb (st_ucon64_nfo_t *rominfo);
extern int gb_ssc (st_ucon64_nfo_t *rominfo);
extern int gb_init (st_ucon64_nfo_t *rominfo);
extern int gb_logo (st_ucon64_nfo_t *rominfo);

#endif
