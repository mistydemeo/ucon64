/*
smd.h - Super Magic Drive support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)


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

#define smd_TITLE "Super Com Pro (HK)/Super Magic Drive/SMD\n" \
                   "19XX Front Far East/FFE http://www.front.com.tw"

int smd_usage(int argc,char *argv[]);
#define smd_HEADER_START 0
#define smd_HEADER_LEN 512
