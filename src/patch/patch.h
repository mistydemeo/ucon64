/*
patch.h - patch support for uCON64

Copyright (c) 2003 - 2006 NoisyB


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
#ifndef PATCH_H
#define PATCH_H

#include "aps.h"
#include "bsl.h"
#include "gg.h"
#include "ips.h"
#include "ppf.h"


extern const st_getopt2_t patch_usage[];

/*
  ucon64_pattern()      change file based on patterns specified in pattern_fname
*/
extern int patch_poke (st_ucon64_t *p);
extern int ucon64_pattern (st_ucon64_nfo_t *nfo, const char *pattern_fname);


#endif
