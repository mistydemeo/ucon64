/*
yoko.c - support for Yoko backup unit (Atari 2600, etc.)

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
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
const char *yoko_usage[] =
  {
    "YOKO backup unit",
    NULL
  };

#ifdef PARALLEL

#include "misc.h"                               // kbhit(), getch()
#include "ucon64.h"
#include "ucon64_misc.h"
#include "yoko.h"



#endif // PARALLEL
