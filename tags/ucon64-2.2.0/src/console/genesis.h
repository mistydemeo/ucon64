/*
genesis.h - Sega Genesis/Mega Drive support for uCON64

Copyright (c) 1999 - 2001                    NoisyB
Copyright (c) 2002 - 2004, 2015, 2017 - 2018 dbjh


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
#ifndef GENESIS_H
#define GENESIS_H

#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"


typedef enum { SMD = 1, BIN, MGD_GEN } genesis_copier_t;

extern const st_getopt2_t genesis_usage[];

extern genesis_copier_t genesis_get_copier_type (void);
extern int genesis_1991 (st_ucon64_nfo_t *rominfo);
extern int genesis_chk (st_ucon64_nfo_t *rominfo);
extern int genesis_j (st_ucon64_nfo_t *rominfo);
extern int genesis_n (st_ucon64_nfo_t *rominfo, const char *name);
extern int genesis_n2 (st_ucon64_nfo_t *rominfo, const char *name);
extern int genesis_s (st_ucon64_nfo_t *rominfo);
extern int genesis_smgh (st_ucon64_nfo_t *rominfo);
extern int genesis_smd (st_ucon64_nfo_t *rominfo);
extern int genesis_smds (void);
extern int genesis_bin (st_ucon64_nfo_t *rominfo);
extern int genesis_mgd (st_ucon64_nfo_t *rominfo);
extern int genesis_mgh (st_ucon64_nfo_t *rominfo);
extern int genesis_multi (unsigned int truncate_size);
extern int genesis_init (st_ucon64_nfo_t *rominfo);
extern int genesis_f (st_ucon64_nfo_t *rominfo);

#endif
