/*
fig.c - Super PRO Fighter support for uCON64

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

char *fig_title = "Super Pro Fighter (Q)/Pro Fighter X (Turbo 2)/Double Pro Fighter (X Turbo)/FIG\n"
                  "1993/1994/19XX China Coach Limited/CCL http://www.ccltw.com.tw";

#ifdef BACKUP
#include "misc.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "fig.h"

void
fig_usage (void)
{
#if 0
    printf (fig_title "\n"
    "TODO:  -xfig	send/receive ROM to/from *Pro Fighter* /(all)FIG; $FILE=PORT\n"
     "		receives automatically when $ROM does not exist\n");
#endif
}
#endif // BACKUP
