/*
spsc.h - (Starpath) Supercharger support for uCON64

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
#ifndef SPSC_H
#define SPSC_H

#include "misc/getopt2.h"                       // st_getopt2_t


extern const st_getopt2_t spsc_usage[];

typedef struct st_spsc_header
{
  unsigned char pad[8448];
} st_spsc_header_t;

#define SPSC_HEADER_LEN (sizeof (st_spsc_header_t))

#endif
