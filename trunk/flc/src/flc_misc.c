/*
flc_misc.c - miscellaneous functions for flc

Copyright (C) 1999-2002 by NoisyB (noisyb@gmx.net)

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
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include "misc.h"
#include "file.h"
#include "flc.h"
#include "flc_misc.h"
#include "flc_defines.h"
#include "format/format.h"
#include "format/id3.h"
#include "format/met.h"
#include "format/txt.h"
#include "format/custom.h"


#ifndef TRUE
#define FALSE 0
#define TRUE !FALSE
#endif


#ifndef __USE_GNU
#undef strcasestr
char *
strcasestr (const char *str, const char *search)
{
  if (!(*search))
    return (char *) str;
  else
    {
      int len = strlen (search);

      for (; *str; str++)
        if (!strncasecmp (str, search, len))
          return (char *) str;
    }

  return NULL;
}
#endif  // __USE_GNU
#define stristr strcasestr


st_format_t *function[] = {
  &id3_format,
  &met_format,
  &txt_format,
  &custom_format,
  NULL
};


st_format_t *
get_function_by_suffix (const char *suffix)
{
  int x = 0;

  if (*suffix)
    for (; function[x]; x++)
      if (stristr (function[x]->suffix, suffix))
        return function[x];

  return get_function_by_suffix (".???");
}


int
extract (st_file_t *file, const char *fname)
{
  st_format_t *p = NULL;
  struct stat puffer;

  memset (file, 0, sizeof (st_file_t));

  // defaults
  file->checked = 'N';
  if (stat (fname, &puffer) == -1)
    return -1;
  if (S_ISREG (puffer.st_mode) != TRUE)
    return -1;
  file->date = puffer.st_mtime;
  file->size = puffer.st_size;
  strcpy (file->fname, fname);
  strncpy (file->file_id[0], basename2 (file->fname), 46);
  file->file_id[0][46] = 0;

  if (!(p = get_function_by_suffix (get_suffix (fname))))
    return 0;

  if (flc.flags & FLC_CHECK && p->test)
    {
      if (!p->test (file->fname))
        file->checked = 'P';
      else
        file->checked = 'F'; // failed
    }

  if (p->extract)
    p->extract (file, file->fname);

  return 0;
}


int
output (FILE *fp, const st_file_t *file)
{
  int x;
  char buf[MAXBUFSIZE];

  if (!file)
    return -1;
    
  if (flc.flags & FLC_HTML)
    {
      int len = strlen (basename2 (file->fname));

      strcpy (buf, basename2 (file->fname));
      buf[12] = 0;
      fprintf (fp, "<a href=\"%s\">%s</a>", file->fname, buf);

      if (len < 12)
        {
          strcpy (buf, "              ");
          buf[12 - len] = 0;
          fprintf (fp, buf);
        }
    }
  else
    fprintf (fp, "%-12.12s", basename2 (file->fname));

  fprintf (fp, " %c", file->checked ? file->checked : 'N');

  if (flc.flags & FLC_KBYTE && file->size > 1024)
    sprintf (buf, "%6luk", file->size / 1024);
  else
    sprintf (buf, "%7lu", file->size);
  strcat (buf, "                                            ");
  fprintf (fp, "%-9.9s", buf);

  strftime (buf, 11, "%m-%d-%Y   ", localtime ((const time_t *)&file->date));
  buf[6] = buf[8];
  buf[7] = buf[9];
  buf[8] = 0;
  fprintf (fp, "%-10.10s", buf);

  for (x = 0; file->file_id[x][0]; x++)
    fprintf (fp, !x ? "%-45.45s\n" : "                                 %-45.45s\n", file->file_id[x]);

  return 0;
}


static int
compare (const void *a, const void *b)
{
  int result = 0;
  st_file_t *p = (st_file_t *) a, *p2 = (st_file_t *) b;

  if (flc.flags & FLC_DATE)
    {
      if (p->date > p2->date)
        result = 1;
    }
  else if (flc.flags & FLC_SIZE)
    {
      if (p->size > p2->size)
        result = 1;
    }
  else if (flc.flags & FLC_NAME)
    {
      if ((basename2 (p->fname))[0] > (basename2 (p2->fname))[0])
        result = 1;
    }
  
  if (flc.flags & FLC_FR)
    result = !result;
  
  return result;
}


int
sort (st_file_t * file)
{
  qsort (file, flc.files, sizeof (st_file_t), compare);

  return 0;
}
