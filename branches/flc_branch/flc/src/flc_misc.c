/*
    flc 1999-2002 by NoisyB (noisyb@gmx.net)
    flc lists information about the FILEs (the current directory by default)
    But most important it shows the FILE_ID.DIZ of every file (if present)
    Very useful for FTP Admins or people who have a lot to do with FILE_ID.DIZ
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
#include "flc.h"
#include "flc_misc.h"


st_file_t *
extract (st_file_t *file, const char *fname)
{
  int x = 0;
  struct stat puffer;
  char suffix[FILENAME_MAX];
  struct dirent *ep;
  DIR *dp;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], *p = NULL;
  char property_name[MAXBUFSIZE];
  char cwd[FILENAME_MAX];
  FILE *fh;

  memset (file, 0, sizeof (st_file_t));
  file->checked = 'N';

  if (stat (fname, &puffer) == -1)
    return NULL;
  if (S_ISREG (puffer.st_mode) != TRUE)
    return NULL;
  file->date = puffer.st_mtime;
  file->size = puffer.st_size;
  strcpy (file->fname, fname);

  strcpy (suffix, get_suffix (fname));

  strncpy (file->file_id[0], basename (file->fname), 46);
    file->file_id[0][46] = 0;

  if (flc.check)
    {
      sprintf (property_name, "%s_test", &suffix[1]);
      sprintf (buf, NULL_TO_EMPTY (get_property (flc.configfile, strlwr (property_name), buf2, NULL)), file->fname);

      if (buf[0])
        {
          int result = system (buf)
#ifndef __MSDOS__
          >> 8                                      // the exit code is coded in bits 8-15
#endif                                          //  (that is, under non-DOS)
          ;
          sync ();
          file->checked = !result ? 'P' : 'F';
        }
    }

  sprintf (property_name, "%s_extract", &suffix[1]);
  sprintf (buf, NULL_TO_EMPTY (get_property (flc.configfile, strlwr (property_name), buf2, NULL)), file->fname);

  if (!buf[0]) return file;

  system (buf);
  sync ();

/*
  file_id.diz stuff here
*/

  if (!(dp = opendir (getcwd (cwd, FILENAME_MAX)))) return NULL;

  while ((ep = readdir (dp)) != 0)
    {
      if (!stat (ep->d_name, &puffer))
        if (S_ISREG (puffer.st_mode))
          {
            if (puffer.st_mtime < file->date)
              file->date = puffer.st_mtime;

/*
  read the file_id.diz
*/
            if (!(fh = fopen (ep->d_name, "rb"))) return file;

            for (x = 0; x < FID_LINES_MAX; x++)
              {
                if (!(fgets (buf, MAXBUFSIZE, fh)))
                  {
                    file->file_id[x][0] = 0;
                    break;
                  }

                if ((p = strpbrk (buf, "\x0a\x0d"))) // strip any returns
                  *p = 0;
                  
                buf[46] = 0;
                strcpy (file->file_id[x], buf[0] ? buf : " ");
              }

            fclose (fh);

            remove (ep->d_name);
          }
    }
  
  return file;
}


int
output (const st_file_t *file)
{
  int x;
  char buf[MAXBUFSIZE];

  if (!file) return -1;

  if (flc.html)
    fprintf (stdout, "<a href=\"%s\">", file->fname);

  fprintf (stdout, "%-12.12s%s %c", 
    basename2 (file->fname),
    flc.html ? "</a>" : "",
    file->checked ? file->checked : 'N');

  sprintf (buf, flc.kb ? "%6luk%s" : "%7lu%s",
    flc.kb ? (file->size / 1024) : file->size,
    "                                            ");
  fprintf (stdout, "%-9.9s", buf);

  strftime (buf, 11, "%m-%d-%y   ", localtime ((const time_t *)&file->date));
  fprintf (stdout, "%-10.10s", buf);

  for (x = 0; file->file_id[x][0]; x++)
    fprintf (stdout, !x ? "%-45.45s\n" : "                                 %-45.45s\n", file->file_id[x]);

  fflush (stdout);

  return 0;
}


static int
compare (st_file_t *a, st_file_t *b)
{
#if 1
  return (flc.fr ?
    ((flc.bydate && a->date < b->date) ||
     (flc.byname && (basename2 (a->fname))[0] < (basename2 (b->fname))[0]) ||
     (flc.bysize && a->size < b->size)) :
    ((flc.bydate && a->date > b->date) ||
     (flc.byname && (basename2 (a->fname))[0] > (basename2 (b->fname))[0]) ||
     (flc.bysize && a->size > b->size))) ? 1 : 0;
#else
  if (flc.bysize)
    return (flc.fr && *(uint32_t *)a < *(uint32_t *)b) ||
      *(uint32_t *)a > *(uint32_t *)b ? 1 : 0;
  else if (flc.bydate)
    return (flc.fr && *(uint32_t *)a < *(uint32_t *)b) ||
      *(uint32_t *)a > *(uint32_t *)b ? 1 : 0;
//  else if (flc.byname) // default
    return (flc.fr && *(const char *)a < *(const char *)b) ||
      *(const char *)a > *(const char *)b ? 1 : 0;
#endif
}


#if 0
int
sort (st_file_t * file)
{
  st_file_t *file_p;
  int file_size = sizeof (st_file_t);
  
  while (file->next)
    {
//      file_p = file->next;

      for (;;)
        {
          if (flc.fr ?
              ((flc.bydate && file->date < file_p->date) ||
               (flc.byname && (basename2 (file->fname))[0] < (basename2 (file_p->fname))[0]) ||
               (flc.bysize && file->size < file_p->size)) :
              ((flc.bydate && file->date > file_p->date) ||
               (flc.byname && (basename2 (file->fname))[0] > (basename2 (file_p->fname))[0]) ||
               (flc.bysize && file->size > file_p->size)))
            {
              memcpy (&sub, &file->sub, file_size);
              memcpy (&file->sub, &file_p->sub, file_size);
              memcpy (&file_p->sub, &sub, file_size);
            }

          if (!file_p->next)
            break;
          file_p = file_p->next;
        }
      file = file->next;
    }

  return 0;
}
#else
int
sort (st_file_t * file)
{
  st_file_t *file_p;
  uint32_t size = sizeof (st_file_t);

//  qsort (file, flc.files, size, compare);

  return 0;
}
#endif
