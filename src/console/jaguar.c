/*
jaguar.c - Atari Jaguar support for uCON64

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
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "jaguar.h"
#include "backup/cdrw.h"


const char *jaguar_usage[] =
  {
    "Panther(32bit prototype)/Jaguar64/Jaguar64 CD",
    "1989 Flare2/1993 Atari/1995 Atari",
    "  " OPTION_LONG_S "jag         force recognition\n"
#if 0
    "  " OPTION_LONG_S "hd          force ROM has header (+512 Bytes)\n"
    "  " OPTION_LONG_S "nhd         force ROM has no header\n"
#endif
    ,
    NULL
};

typedef struct st_jaguar
{
  char pad[16];
} st_jaguar_t;
#define JAGUAR_HEADER_START 0x400
#define JAGUAR_HEADER_LEN (sizeof (st_jaguar_t))

st_jaguar_t jaguar_header;


int
jaguar_init (st_rominfo_t *rominfo)
{
  int result = -1;
#ifdef CONSOLE_PROBE
  int x, value;
#endif // CONSOLE_PROBE

  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
    ucon64.buheader_len : 0;

  q_fread (&jaguar_header, JAGUAR_HEADER_START +
    rominfo->buheader_len, JAGUAR_HEADER_LEN, ucon64.rom);
#ifdef CONSOLE_PROBE
  value = 0;
  for (x = 0; x < 12; x++)
    value += OFFSET (jaguar_header, x);
  if (value == 0xb0)
    result = 0;
  else
    {
      rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
        ucon64.buheader_len : UNKNOWN_HEADER_LEN;

      q_fread (&jaguar_header, JAGUAR_HEADER_START +
          rominfo->buheader_len, JAGUAR_HEADER_LEN, ucon64.rom);
      value = 0;
      for (x = 0; x < 12; x++)
        value += OFFSET (jaguar_header, x);

      if (value == 0xb0)
        result = 0;
      else
        result = -1;
    }
#endif // CONSOLE_PROBE
  if (ucon64.console == UCON64_JAG)
    result = 0;

  rominfo->header_start = JAGUAR_HEADER_START;
  rominfo->header_len = JAGUAR_HEADER_LEN;
  rominfo->header = &jaguar_header;

  rominfo->console_usage = jaguar_usage;
  rominfo->copier_usage = unknown_usage;

  return result;
}
