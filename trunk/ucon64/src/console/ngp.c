/*
ngp.c - NeoGeo Pocket support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2001 Gulliver


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
#include <string.h>
#include "config.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "ngp.h"
#include "backup/fpl.h"


const char *ngp_usage[] =
  {
    "Neo Geo Pocket/Neo Geo Pocket Color",
    "1998/1999 SNK http://www.neogeo.co.jp",
    "  " OPTION_LONG_S "ngp         force recognition"
#ifndef DB
    "; NEEDED"
#endif
    "\n",
    NULL
};


typedef struct st_ngp_header
{
  char pad[48];
} st_ngp_header_t;
#define NGP_HEADER_START 0
#define NGP_HEADER_LEN (sizeof (st_ngp_header_t))


st_ngp_header_t ngp_header;


int
ngp_init (st_rominfo_t *rominfo)
{
  int result = -1;
  char *snk_code = "COPYRIGHT BY SNK CORPORATION",
       *third_code = " LICENSED BY SNK CORPORATION", buf[MAXBUFSIZE];

  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
    ucon64.buheader_len : 0;

  q_fread (&ngp_header, NGP_HEADER_START +
      rominfo->buheader_start, NGP_HEADER_LEN, ucon64.rom);

  if (!strncmp (&OFFSET (ngp_header, 0), snk_code, strlen(snk_code)) ||
      !strncmp (&OFFSET (ngp_header, 0), third_code, strlen(third_code)))
      result = 0;
  else
    result = -1;
  if (ucon64.console == UCON64_NGP)
    result = 0;

  rominfo->header_start = NGP_HEADER_START;
  rominfo->header_len = NGP_HEADER_LEN;
  rominfo->header = &ngp_header;

//internal ROM name
  strncpy (rominfo->name, &OFFSET (ngp_header, 0x24), 12);
  rominfo->name[12] = 0;

//ROM maker
  rominfo->maker = (!strncmp (&OFFSET (ngp_header, 0), snk_code, strlen(snk_code))) ?
    "SNK" : "third party";

//misc stuff
  sprintf (buf, "Mode: %s",
      (OFFSET (ngp_header, 0x23) == 0x00) ? "Mono" :
      (OFFSET (ngp_header, 0x23) == 0x10) ? "Color" :
      "Unknown");
  strcat (rominfo->misc, buf);

  rominfo->console_usage = ngp_usage;
  rominfo->copier_usage = (!rominfo->buheader_len) ? fpl_usage : unknown_usage;

  return result;
}
