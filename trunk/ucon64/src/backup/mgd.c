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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "mgd.h"


const st_usage_t mgd_usage[] =
  {
    {NULL, NULL, "Multi Game Doctor (2)/Multi Game Hunter/MGH/RAW"},
    {NULL, NULL, "19XX Bung Enterprises Ltd http://www.bung.com.hk"},
    {NULL, NULL, "?Makko Toys Co., Ltd.?"},
#ifdef TODO
#warning TODO  --xmgd    send/receive ROM to/from Multi Game* /MGD2/MGH/RAW
#endif // TODO
    {"xmgd", "(TODO) send/receive ROM to/from Multi Game* /MGD2/MGH/RAW; " OPTION_LONG_S "port=PORT\n"
                "receives automatically when " OPTION_LONG_S "rom does not exist"},
    {NULL, NULL, NULL}
  };

#ifdef PARALLEL
#endif // PARALLEL
