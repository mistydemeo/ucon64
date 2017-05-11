/*
pce.h - PC-Engine support for uCON64

Copyright (c) 1999 - 2001             NoisyB
Copyright (c) 2003 - 2004, 2015, 2017 dbjh


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
#ifndef PCE_H
#define PCE_H

#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"


extern const st_getopt2_t pce_usage[];

extern int pce_init (st_ucon64_nfo_t *rominfo);
extern int pce_mgd (st_ucon64_nfo_t *rominfo);
extern int pce_msg (st_ucon64_nfo_t *rominfo);
extern int pce_swap (st_ucon64_nfo_t *rominfo);
extern int pce_f (st_ucon64_nfo_t *rominfo);
extern int pce_multi (unsigned int truncate_size, char *fname);

#endif
