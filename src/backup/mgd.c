/*
mgd.c - Multi Game Doctor/Hunter support for uCON64

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
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
const char *mgd_usage[] =
  {
    "Multi Game Doctor(2)/Multi Game Hunter/MGH/RAW",
    "19XX Bung Enterprises Ltd http://www.bung.com.hk\n"
    "?Makko Toys Co., Ltd.?",
#if 0
    "TODO:  " OPTION_LONG_S "xmgd    send/receive ROM to/from Multi Game* /MGD2/MGH/RAW; " OPTION_LONG_S "file=PORT\n"
    "		receives automatically when " OPTION_LONG_S "rom does not exist\n",
#endif
    NULL
  };
  
#ifdef BACKUP 
#include "misc.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "mgd.h"
#endif // BACKUP
