/*
patch.c - patch support for uCON64

Copyright (c) 2006 NoisyB


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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/itypes.h"
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/file.h"
#include "misc/misc.h"
#include "ucon64_misc.h"
#include "ucon64_defines.h"
#include "patch.h"


const st_getopt2_t patch_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Patching"
    },
    {
      "poke", 1, 0, UCON64_POKE,
      "OFF:V", "change byte at file offset OFF to value V (both in hexadecimal)"
    },
    {
      "pattern", 1, 0, UCON64_PATTERN,
      "FILE", "change ROM based on patterns specified in FILE"
    },
    {
      "patch", 1, 0, UCON64_PATCH,
      "PATCH", "specify the PATCH for the following options\n"
      "use this option or uCON64 expects the last commandline\n"
      "argument to be the name of the PATCH file"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };


#warning test patch_poke()
int
patch_poke (st_ucon64_nfo_t *rominfo)
{
  st_ucon64_t *p = &ucon64;
  int value = 0, x = 0;
  char buf[MAXBUFSIZE], src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  const char *optarg = p->optarg;

  if (ucon64.fname)
    {
      strcpy (src_name, ucon64.fname);
      strcpy (dest_name, ucon64.fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, 0, ucon64.file_size, dest_name, "wb");

  sscanf (optarg, "%x:%x", &x, &value);
  if (x >= ucon64.file_size)
    {
      fprintf (stderr, "ERROR: Offset 0x%x is too large\n", x);
      remove (dest_name);
      return -1;
    }
  fputc ('\n', stdout);
  buf[0] = ucon64_fgetc (dest_name, x);
  dumper (stdout, buf, 1, x, 0);

  ucon64_fputc (dest_name, x, value, "r+b");

  buf[0] = value;
  dumper (stdout, buf, 1, x, 0);
  fputc ('\n', stdout);

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}


#define PATTERN_BUFSIZE (64 * 1024)
/*
  In order for this function to be really useful for general purposes
  change_mem2() should be changed so that it will return detailed status
  information. Since we don't use it for general purposes, this has not a high
  priority. It will be updated as soon as there is a need.
  The thing that currently goes wrong is that offsets that fall outside the
  buffer (either positive or negative) won't result in a change. It will result
  in memory corruption...
*/
int
ucon64_pattern (st_ucon64_nfo_t *rominfo)
{
  const char *pattern_fname = ucon64.optarg;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX],
       buffer[PATTERN_BUFSIZE];
  FILE *srcfile, *destfile;
  int bytesread = 0, n, n_found = 0, n_patterns, overlap = 0;
  st_cm_pattern_t *patterns = NULL;

  realpath2 (pattern_fname, src_name);
  // First try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    sprintf (src_name, "%s" FILE_SEPARATOR_S "%s", ucon64.configdir, pattern_fname);
  n_patterns = build_cm_patterns (&patterns, src_name, ucon64.quiet == -1 ? 1 : 0);
  if (n_patterns == 0)
    {
      fprintf (stderr, "ERROR: No patterns found in %s\n", src_name);
      cleanup_cm_patterns (&patterns, n_patterns);
      return -1;
    }
  else if (n_patterns < 0)
    {
      char dir1[FILENAME_MAX], dir2[FILENAME_MAX];
      dirname2 (pattern_fname, dir1);
      dirname2 (src_name, dir2);

      fprintf (stderr, "ERROR: Could not read from %s, not in %s nor in %s\n",
                       basename2 (pattern_fname), dir1, dir2);
      // when build_cm_patterns() returns -1, cleanup_cm_patterns() should not be called
      return -1;
    }

  printf ("Found %d pattern%s in %s\n", n_patterns, n_patterns != 1 ? "s" : "", src_name);

  for (n = 0; n < n_patterns; n++)
    {
      if (patterns[n].search_size > overlap)
        {
          overlap = patterns[n].search_size;
          if (overlap > PATTERN_BUFSIZE)
            {
              fprintf (stderr,
                       "ERROR: Pattern %d is too large, specify a shorter pattern\n",
                       n + 1);
              cleanup_cm_patterns (&patterns, n_patterns);
              return -1;
            }
        }

      if ((patterns[n].offset < 0 && patterns[n].offset <= -patterns[n].search_size) ||
           patterns[n].offset > 0)
        printf ("WARNING: The offset of pattern %d falls outside the search pattern.\n"
                "         This can cause problems with the current implementation of --pattern.\n"
                "         Please consider enlarging the search pattern.\n",
                n + 1);
    }
  overlap--;

  puts ("Searching for patterns...");

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }
  if (rominfo->backup_header_len)                    // copy header (if present)
    {
      n = rominfo->backup_header_len;
      while ((bytesread = fread (buffer, 1, MIN (n, PATTERN_BUFSIZE), srcfile)))
        {
          fwrite (buffer, 1, bytesread, destfile);
          n -= bytesread;
        }
    }

  n = fread (buffer, 1, overlap, srcfile);      // keep bytesread set to 0
  if (n < overlap)                              // DAMN special cases!
    {
      n_found += change_mem2 (buffer, n, patterns[n].search,
                              patterns[n].search_size, patterns[n].wildcard,
                              patterns[n].escape, patterns[n].replace,
                              patterns[n].replace_size, patterns[n].offset,
                              patterns[n].sets);
      fwrite (buffer, 1, n, destfile);
      n = -1;
    }
  else
    do
      {
        if (bytesread)                          // the code also works without this if
          {
            for (n = 0; n < n_patterns; n++)
              {
                int x = 1 - patterns[n].search_size;
                n_found += change_mem2 (buffer + overlap + x,
                                        bytesread + patterns[n].search_size - 1,
                                        patterns[n].search, patterns[n].search_size,
                                        patterns[n].wildcard, patterns[n].escape,
                                        patterns[n].replace, patterns[n].replace_size,
                                        patterns[n].offset, patterns[n].sets);
              }
            fwrite (buffer, 1, bytesread, destfile);
            memmove (buffer, buffer + bytesread, overlap);
          }
      }
    while ((bytesread = fread (buffer + overlap, 1, PATTERN_BUFSIZE - overlap, srcfile)));
  if (n != -1)
    fwrite (buffer, 1, overlap, destfile);

  fclose (srcfile);
  fclose (destfile);
  cleanup_cm_patterns (&patterns, n_patterns);

  printf ("Found %d pattern%s\n", n_found, n_found != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return n_found;
}
#undef PATTERN_BUFSIZE


