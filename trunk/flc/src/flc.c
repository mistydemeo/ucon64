/*
flc
shows the FILE_ID.DIZ of archives/files
useful for FTP admins or people who have a lot to do with FILE_ID.DIZ

Copyright (C) 1999-2004 by NoisyB

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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/getopt.h"
#include "misc/getopt2.h"
#include "misc/property.h"
#include "misc/file.h"
#include "misc/filter.h"
#include "misc/misc.h"
#include "flc.h"
#include "flc_defines.h"
#include "flc_misc.h"
#include "misc/rmdir2.h"  

static void flc_exit (void);

st_flc_t flc; // workflow
static st_file_t file[FLC_MAX_FILES];


void
flc_exit (void)
{
  chdir (flc.cwd);

#if 0
  if (flc.temp[0])
#if 0
    rmdir2 (flc.temp);
#else
    rmdir (flc.temp);
#endif
#endif
}


static int
flc_file_handler (const char *path)
{
  if (flc.files > FLC_MAX_FILES)
    {
      fprintf (stderr, "ERROR: can not open more than %d inputs\n\n", FLC_MAX_FILES);
      return -1; // skip
    }
    
  if (!extract (&file[flc.files], path))
    flc.files++;

  return 0;
}


int
main (int argc, char *argv[])
{
#if     FILENAME_MAX > MAXBUFSIZE
  char path[FILENAME_MAX];
#else
  char path[MAXBUFSIZE];
#endif
  char short_options[ARGS_MAX];
  struct option long_only_options[ARGS_MAX];
  uint32_t *flags = NULL;
  int x = 0, c;
  FILE *fh = NULL;
  int option_index = 0;
  int result = 0;
  st_property_t props[] = {
    {
      "lha_test", "lha t \"%s\"",
      "LHA support\n"
      "%s will be replaced with the file/archive name"
    },
    {
      "lha_extract", "lha efi \"%s\" " FLC_ID_NAMES,
      NULL
    },
    {
      "lzh_test", "lha t \"%s\"",
      "LZH support"
    },
    {
      "lzh_extract", "lha efi \"%s\" " FLC_ID_NAMES,
      NULL
    },
    {
      "zip_test", "unzip -t \"%s\"",
      "ZIP support"
    },
    {
      "zip_extract", "unzip -xojC \"%s\" " FLC_ID_NAMES,
      NULL
    },
    {
      "rar_test", "unrar t \"%s\"",
      "RAR support"
    },
    {
      "rar_extract", "unrar x \"%s\" " FLC_ID_NAMES,
      NULL
    },
    {
      "ace_test", "unace t \"%s\"",
      "ACE support"
    },
    {
      "ace_extract", "unace e \"%s\" " FLC_ID_NAMES,
      NULL
    },
    {NULL, NULL, NULL}
  };
  const st_getopt2_t options[] = {
    {NULL,      0, 0, 0, NULL,
      "flc " FLC_VERSION_S " " CURRENT_OS_S " 1999-2004 by NoisyB\n"
      "This may be freely redistributed under the terms of the GNU Public License\n\n"
      "Usage: flc [OPTION]... [FILE]...\n", NULL},
    {"t",       0, 0, 't', NULL,   "sort by modification time", (void *) (FLC_SORT|FLC_DATE)},
    {"X",       0, 0, 'X', NULL,   "sort alphabetical", (void *) (FLC_SORT|FLC_NAME)},
    {"S",       0, 0, 'S', NULL,   "sort by byte size", (void *) (FLC_SORT|FLC_SIZE)},
    {"fr",      0, 0, 2,   NULL,   "sort reverse", (void *) (FLC_SORT|FLC_FR)},
    {"k",       0, 0, 'k', NULL,   "show sizes in kilobytes", (void *) FLC_KBYTE},
    {"bbs",     0, 0, 4,   NULL,   "output as BBS style filelisting (default)", (void *) FLC_BBS},
    {"html",    0, 0, 3,   NULL,   "output as HTML document with links to the files", (void *) FLC_HTML},
    {"o",       1, 0, 'o', "FILE", "write output into FILE", NULL},
    {"c",       0, 0, 'c', NULL,   "also test every possible archive for errors\n"
                                   "return flags: N=not checked (default), P=passed, F=failed",
                                   (void *) FLC_CHECK},
    {"cache",   1, 0, 'C', "CACHE", "get a default file_id.diz from CACHE filelisting\n"
                                    "(searches for filename)", NULL},
//    {"stats",   1, 0, 's', "LIST", "show stats of fileLISTing", NULL},
    {"R",       0, 0, 'R', NULL,   "scan subdirectories recursively", (void *) FLC_RECURSIVE},
    {"version", 0, 0, 'v', NULL,   "output version information and exit", NULL},
    {"ver",     0, 0, 'v', NULL,   NULL, NULL},
    {"help",    0, 0, 'h', NULL,   "display this help and exit", NULL},
    {"h",       0, 0, 'h', NULL,   NULL, NULL},
    {NULL,      0, 0, 0,   NULL,
      "\nAmiga version: noC-flc Version v1.O (File-Listing Creator) - (C)1994 nocTurne deSign/MST\n"
      "Report problems to ucon64-main@lists.sf.net or go to http://ucon64.sf.net\n", NULL},
    {NULL,      0, 0, 0,   NULL,   NULL, NULL}
  };

  memset (&flc, 0, sizeof (st_flc_t));

  realpath2 (PROPERTY_HOME_RC ("flc"), flc.configfile);

  result = property_check (flc.configfile, FLC_CONFIG_VERSION, 1);
  if (result == 1) // update needed
    result = set_property_array (flc.configfile, props);
  if (result == -1) // property_check() or update failed
    return -1;

  getcwd (flc.cwd, FILENAME_MAX);
  atexit (flc_exit);
                      
  getopt2_short (short_options, options, ARGS_MAX);
  getopt2_long_only (long_only_options, options, ARGS_MAX);
  
  while ((c = getopt_long_only (argc, argv, short_options, long_only_options, &option_index)) != -1)
    switch (c)
      {
        case 4:
        case 't':
        case 'X':
        case 'S':
        case 2:
        case 'k':
        case 3:
        case 'c':
        case 'R':
          flags = (uint32_t *) &getopt2_get_index_by_val (options, c)->object;
          if (flags)
            flc.flags |= *flags;
          break;

        case 'C':
          if (optarg)
            strncpy (flc.cache, optarg, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
          break;

        case 'o':
          if (optarg)
            strncpy (flc.output, optarg, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
          break;
          
        case 'h':
          getopt2_usage (options);
          return 0;

        case 'v':
          printf ("%s\n", FLC_VERSION_S);
          return 0;

        default:
          printf ("Try '%s --help' for more information.\n", argv[0]);
          return -1;
      }

  if (argc < 2 || !optind)
    {
      getopt2_usage (options);
      exit (-1);
    }

  if (tmpnam2 (path))
    {
      realpath2 (path, flc.temp);

      if (mkdir (flc.temp, S_IRUSR|S_IWUSR) == -1)
        {
          fprintf (stderr, "ERROR: could not create %s\n", flc.temp);
          exit (-1);
        }
    }

  flc.files = 0;

  if (!getopt2_file (argc, argv, flc_file_handler, (GETOPT2_FILE_FILES_ONLY | // extract works only for files
                   (flc.flags & FLC_RECURSIVE ? GETOPT2_FILE_RECURSIVE : 0)))) // recursively?
    {
      getopt2_usage (options);
      exit (-1);
    }

  if (flc.flags & FLC_SORT)
    sort (file);

  if (flc.output[0])
    fh = fopen (flc.output, "wb");
  if (!fh)
    fh = stdout;

  if (flc.flags & FLC_HTML)
    fprintf (fh, "<html><head><title></title></head><body><pre><tt>");

  for (x = 0; x < flc.files; output (fh, &file[x++]));

  if (flc.flags & FLC_HTML)
    fprintf (fh, "</pre></tt></body></html>\n");

  if (fh != stdout)
    fclose (fh);

  return 0;
}
