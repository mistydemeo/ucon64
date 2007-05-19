/*
gb.h - Game Boy support for uCON64

Copyright (c) 1999 - 2001 NoisyB


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

#define GB_LOGODATA_LEN 48
#define GB_NAME_LEN 15                          // correct for GBC games, not
                                                //  for {S}GB games (should be 16)
extern const st_getopt2_t gb_usage[];
extern const unsigned char gb_logodata[], rocket_logodata[];

extern UCON64_FILTER_TYPE (gb_sc);
extern UCON64_FILTER_TYPE (gb_chk);
extern UCON64_FILTER_TYPE (gb_gbx);
extern UCON64_FILTER_TYPE (gb_mgd);
extern UCON64_FILTER_TYPE (gb_sgb);
extern UCON64_FILTER_TYPE (gb_ssc);
extern UCON64_FILTER_TYPE (gb_init);
extern UCON64_FILTER_TYPE (gb_logo);
extern UCON64_FILTER_TYPE (gb_n);
extern UCON64_FILTER_TYPE (gb_n2gb);

#endif
