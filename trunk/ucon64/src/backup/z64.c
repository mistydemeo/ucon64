/*
z64.c - Z64 support for uCON64

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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "ucon64.h"


const st_usage_t z64_usage[] =
  {
    {NULL, NULL, "Z64"},
    {NULL, NULL, NULL}
  };

#ifdef PARALLEL

#include "misc.h"                               // kbhit(), getch()
#include "ucon64.h"
#include "ucon64_misc.h"
#include "z64.h"



#endif // PARALLEL
