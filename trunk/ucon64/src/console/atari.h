/*
atari.h - Atari 2600/5200/7800 support for uCON64

Copyright (c) 2004 NoisyB (noisyb@gmx.net)

Inspired by code from makewav v4.1 and MakeBin v1.0, written by Bob Colbert
  <rcolbert1@home.com>


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
#ifndef ATARI_H
#define ATARI_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
extern const st_getopt2_t atari_usage[];

extern UCON64_FILTER_TYPE (atari_init);
#ifdef  HAVE_MATH_H
extern UCON64_FILTER_TYPE (atari_cc2);
#endif
#endif
