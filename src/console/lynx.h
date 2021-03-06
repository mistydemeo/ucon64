/*
lynx.h - Atari Lynx support for uCON64

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
#ifndef LYNX_H
#define LYNX_H

#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"


typedef struct st_lnx_header
{
  char magic[4];
  short int page_size_bank0;
  short int page_size_bank1;
  short int version;
  char cartname[32];
  char manufname[16];
  unsigned char rotation;
  unsigned char spare[5];
} st_lnx_header_t;


extern const st_getopt2_t lynx_usage[];

extern int lynx_b0 (st_ucon64_nfo_t *rominfo, const char *value);
extern int lynx_b1 (st_ucon64_nfo_t *rominfo, const char *value);
extern int lynx_lnx (st_ucon64_nfo_t *rominfo);
extern int lynx_lyx (st_ucon64_nfo_t *rominfo);
extern int lynx_n (st_ucon64_nfo_t *rominfo, const char *name);
extern int lynx_nrot (st_ucon64_nfo_t *rominfo);
extern int lynx_rotl (st_ucon64_nfo_t *rominfo);
extern int lynx_rotr (st_ucon64_nfo_t *rominfo);
extern int lynx_init (st_ucon64_nfo_t *rominfo);

#endif // LYNX_H
