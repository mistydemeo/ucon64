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


int
extract (st_sub_t *file)
{
  int x = 0;
  struct stat puffer;
  struct dirent *ep;
  DIR *dp;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], *p = NULL;
  char property_name[MAXBUFSIZE];
  char cwd[FILENAME_MAX];
  FILE *fh;
  for (x = 0; x < FID_LINES_MAX; x++)
    file->file_id[x][0] = 0;

  strncpy (file->file_id[0], basename (file->fullpath), 46);
    file->file_id[0][46] = 0;

  file->checked = 'N';

  if (flc.check)
    {
      sprintf (property_name, "%s_test", &getext (file->fullpath)[1]);
      sprintf (buf, NULL_TO_EMPTY (get_property (flc.configfile, strlwr (property_name), buf2, NULL)), file->fullpath);

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

  sprintf (property_name, "%s_extract", &getext (file->fullpath)[1]);
  sprintf (buf, NULL_TO_EMPTY (get_property (flc.configfile, strlwr (property_name), buf2, NULL)), file->fullpath);

  if (!buf[0]) return 0;

  system (buf);
  sync ();

/*
  file_id.diz stuff here
*/

  if (!(dp = opendir (getcwd (cwd, FILENAME_MAX)))) return -1;

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
            if (!(fh = fopen (ep->d_name, "rb"))) return -1;

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
  
  return 0;
}


int
output (const st_sub_t *file)
{
  int x;
  char buf[MAXBUFSIZE];

  if (!file) return -1;

  if (flc.html)
    fprintf (stdout, "<a href=\"%s\">", file->fullpath);

  fprintf (stdout, "%-12.12s%s %c", 
    file->name,
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


#if 1
int
sort (st_file_t * file)
{
  st_file_t *file_p;
  st_sub_t sub;
  int sub_size;

  sub_size = sizeof (st_sub_t);
  while (file->next)
    {
      file_p = file->next;

      for (;;)
        {
          if (flc.fr ?
              ((flc.bydate && file->sub.date < file_p->sub.date) ||
               (flc.byname && (file->sub.name)[0] < (file_p->sub.name)[0]) ||
               (flc.bysize && file->sub.size < file_p->sub.size)) :
              ((flc.bydate && file->sub.date > file_p->sub.date) ||
               (flc.byname && (file->sub.name)[0] > (file_p->sub.name)[0]) ||
               (flc.bysize && file->sub.size > file_p->sub.size)))
            {
              memcpy (&sub, &file->sub, sub_size);
              memcpy (&file->sub, &file_p->sub, sub_size);
              memcpy (&file_p->sub, &sub, sub_size);
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
  st_sub_t sub;
  int sub_size;

  sub_size = sizeof (st_sub_t);
/* qsort example */
#include <stdio.h>
#include <stdlib.h>

  int values[] = { 40, 10, 100, 90, 20, 25 };

  int compare (const void *a, const void *b)
  {
    return (*(int *) a - *(int *) b);
  }

  int main ()
  {
    int *pItem;
    int n;
    qsort (values, 6, sizeof (int), compare);
    for (n = 0; n < 6; n++)
      {
        printf ("%d ", values[n]);
      }
    return 0;
  }
  return 0;
}
#endif
