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
#ifdef BACKUP 
#include "misc.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "mgd.h"

char *mgd_title = "Multi Game Doctor(2)/Multi Game Hunter/MGH/RAW\n"
                  "19XX Bung Enterprises Ltd http://www.bung.com.hk\n"
                  "?Makko Toys Co., Ltd.?";


void
mgd_usage (void)
{
#if 0
    printf (mgd_title "\n"

    "TODO:  -xmgd    send/receive ROM to/from Multi Game* /MGD2/MGH/RAW; $FILE=PORT\n"
     "		receives automatically when $ROM does not exist\n");
#endif
}
#endif // BACKUP
