/*
txt.c - TXT support for flc

Copyright (c) 2004 by NoisyB

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
#include <string.h>
#include <time.h>
#include "misc/itypes.h"
#include "misc/string.h"
#include "misc/filter.h"
#include "flc.h"
#include "flc_defines.h"
#include "txt.h"


int
txt_open (st_flc_t *flc) 
{
  int found = 0;
  int row = 0;
  char buf[MAXBUFSIZE];
  FILE *fh = NULL, *tmp = NULL;

  if (!(tmp = fopen (flc->dstfile, "wb")))
    return -1;

  if (!(fh = fopen (flc->srcfile, "rb")))
    {
      fclose (tmp);
      return -1;
    }

  while ((fgets (buf, MAXBUFSIZE, fh)) != 0)
    {
      if (!strnicmp (buf, "@END_FILE_ID.DIZ", 11))
        break;

      if (!strnicmp (buf, "@BEGIN_FILE_ID.DIZ", 18))
        found = 1;

      if (found && row < FLC_MAX_ID_ROWS)
        {
          char *p = NULL;

          if ((p = strpbrk (buf, "\x0a\x0d")))
            *p = 0;

          fprintf (tmp, "%s\n", !row ? &buf[18] : buf);

          row++;
        }
    }

  fclose (fh);
  fclose (tmp);

  return 0;
}


const st_filter_t txt_filter = {
  FLC_TXT,
  "txt, nfo",
  ".txt.nfo",
  -1,
  .open = (int (*) (void *)) &txt_open
};
