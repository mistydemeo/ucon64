/*
fig.c - Super PRO Fighter support for uCON64

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh


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

#ifdef PARALLEL
#include "misc.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "fig.h"
#endif // PARALLEL

const char *fig_usage[] =
  {
    "Super Pro Fighter (Q/Q+)/Pro Fighter X (Turbo 2)/Double Pro Fighter (X Turbo)",
    // adding "/FIG" causes a wrap around on 80 column terminals (DOS!) which is ugly
    "1993/1994/19XX China Coach Limited/CCL http://www.ccltw.com.tw",
#ifdef TODO
#warning TODO --xfig        send/receive ROM to/from *Pro Fighter* /(all)FIG
#endif // TODO
#if 0
    "TODO:  " OPTION_LONG_S "xfig        send/receive ROM to/from *Pro Fighter* /(all)FIG; " OPTION_LONG_S "file=PORT\n"
    "                  receives automatically when " OPTION_LONG_S "rom does not exist\n",
#endif
    NULL
  };
