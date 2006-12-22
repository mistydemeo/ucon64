/*
cc2.c - Cuttle Card (2) support for uCON64

Copyright (c) 2004 NoisyB (noisyb@gmx.net)


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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "misc/itypes.h"
#include "misc/misc.h"
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "cc2.h"


const st_getopt2_t cc2_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Cuttle Card (2)"/*"2001 Shell's Electronic http://www.shells.com"*/
    },
#if 0
    {
      "xcc2", 0, 0, UCON64_XCC2,
      NULL, "send/receive ROM to/from Cuttle Card (2)\n"
      "actually Cuttle Card (2) backup units use audio\n"
      "input/output to transfer ROMs"
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL}
  };

