/*
psx.c - Playstation support for uCON64

written by 2001 NoisyB (noisyb@gmx.net)


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
#include <string.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "psx.h"


const st_usage_t psx_usage[] =
  {
    {NULL, "Playstation (One)/Playstation 2 (CD only)"},
    {NULL, "1994/(2000) Sony http://www.playstation.com"},
    {"psx", "force recognition"},
    {NULL, NULL}
};


int
psx_init (st_rominfo_t *rominfo)
{
  int result = -1;

  rominfo->console_usage = psx_usage;
//  rominfo->copier_usage = cdrw_usage;

  return result;
}
