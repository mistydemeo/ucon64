
/*
flc
shows the FILE_ID.DIZ of archives/files
useful for FTP admins or people who have a lot to do with FILE_ID.DIZ

Copyright (C) 1999-2004 by NoisyB (noisyb@gmx.net)

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
#include "getopt.h"
#include "misc.h"
#include "flc.h"
#include "flc_misc.h"
#include "rmdir2.h"  

static void flc_exit (void);
static int flc_configfile (void);

st_flc_t flc; // workflow

void
flc_exit (void)
{
  chdir (flc.cwd);

  if (flc.temp[0])
    rmdir2 (flc.temp);

  printf ("+++EOF");
  fflush (stdout);
}


int
main (int argc, char *argv[])
{
#if     FILENAME_MAX > MAXBUFSIZE
  char path[FILENAME_MAX];
#else
  char path[MAXBUFSIZE];
#endif
  st_file_t file[ARGS_MAX];
  char short_options[ARGS_MAX];
  struct option long_options[ARGS_MAX];
  int x = 0, c;
  FILE *fh = NULL;
  int option_index = 0;
  const st_getopt2_t options[] = {
    {NULL,      0, 0, 0, NULL,
      "flc " FLC_VERSION_S " " CURRENT_OS_S " 1999-2004 by NoisyB (noisyb@gmx.net)\n"
      "This may be freely redistributed under the terms of the GNU Public License\n\n"
      "Usage: flc [OPTION]... [FILE]...\n", NULL},
    {"c",       0, 0, 'c', NULL,   "also test every possible archive in DIRECTORY for errors\n"
                                   "return flags: N=not checked (default), P=passed, F=failed",
                                   NULL},
    {"html",    0, 0, 3,   NULL,   "output as HTML document with links to the files", NULL},
    {"bbs",     0, 0, 4,   NULL,   "output as BBS style filelisting (default)", NULL},
    {"o",       1, 0, 'o', "FILE", "write output into FILE", NULL},
    {"t",       0, 0, 't', NULL,   "sort by modification time", NULL},
    {"X",       0, 0, 'X', NULL,   "sort alphabetical", NULL},
    {"S",       0, 0, 'S', NULL,   "sort by byte size", NULL},
    {"fr",      0, 0, 2,   NULL,   "sort reverse", NULL},
    {"k",       0, 0, 'k', NULL,   "show sizes in kilobytes", NULL},
    {"version", 0, 0, 'v', NULL,   "output version information and exit", NULL},
    {"help",    0, 0, 'h', NULL,   "display this help and exit", NULL},
    {"h",       0, 0, 'h', NULL,   NULL, NULL},
    {NULL,      0, 0, 0,   NULL,
      "\nAmiga version: noC-flc Version v1.O (File-Listing Creator) - (C)1994 nocTurne deSign/MST\n"
      "Report problems to noisyb@gmx.net or go to http://ucon64.sf.net\n", NULL},
    {NULL,      0, 0, 0,   NULL,   NULL, NULL}
  };

  memset (&flc, 0, sizeof (st_flc_t));
  
  flc_configfile ();

  getopt2_short (short_options, options, ARGS_MAX);
  getopt2_long (long_options, options, ARGS_MAX);
  
  while ((c = getopt_long_only (argc, argv, short_options, long_options, &option_index)) != -1)
    switch (c)
      {
        case 1:
          atexit (flc_exit);
          break;

        case 4:
          flc.bbs = 1;
          break;

        case 'o':
          if (optarg)
            strcpy (flc.output, optarg);
          break;
          
        case 't':
          flc.sort = 1;
          flc.bydate = 1;
          break;

        case 'X':
          flc.sort = 1;
          flc.byname = 1;
          break;

        case 'S':
          flc.sort = 1;
          flc.bysize = 1;
          break;

        case 2:
          flc.sort = 1;
          flc.fr = 1;
          break;

        case 'k':
          flc.kb = 1;
          break;

        case 3:
          flc.html = 1;
          break;

        case 'c':
          flc.check = 1;
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
      return -1;
    }

  if (tmpnam2 (path))
    {
      realpath2 (path, flc.temp);

      if (mkdir (flc.temp, S_IRUSR|S_IWUSR) == -1)
        {
          fprintf (stderr, "ERROR: could not create %s\n", flc.temp);
          return -1;
        }
    }
  getcwd (flc.cwd, FILENAME_MAX);

  flc.files = 0;
  for (x = optind; x < argc && flc.files < FLC_MAX_FILES; x++)
    {
      struct stat fstate;

      realpath2 ((const char *) argv[x], path); 
//      sprintf (path, "%s" FILE_SEPARATOR_S "%s", flc.cwd, argv[x]);

      if (stat (path, &fstate) == -1)
        continue;

      if (S_ISREG (fstate.st_mode))
        {
          chdir (flc.temp);
          if (!extract (&file[flc.files], path))
            flc.files++;
          chdir (flc.cwd);
        }
      else if (S_ISDIR (fstate.st_mode))
        {
#if 0
          struct dirent *ep;
          DIR *dp;
#if     FILENAME_MAX > MAXBUFSIZE
          char buf[FILENAME_MAX];
#else
          char buf[MAXBUFSIZE];
#endif

          if (!(dp = opendir (path)))
            continue;

          while ((ep = readdir (dp)))
            {
              sprintf (buf, "%s" FILE_SEPARATOR_S "%s", path, ep->d_name);
              if (!stat (buf, &fstate))
                if (S_ISREG (fstate.st_mode) && flc.files < FLC_MAX_FILES)
                  {
                    chdir (flc.temp);
                    if (!extract (&file[flc.files], buf))
                      flc.files++;
                    chdir (flc.cwd);
                  }
            }
          closedir (dp);
#else

#endif
        }
    }

  if (flc.sort)
    sort (file); // qsort() 

  if (flc.output[0])
    fh = fopen (flc.output, "wb");

  if (flc.html)
    fprintf (fh ? fh : stdout, "<html><head><title></title></head><body><pre><tt>");

  for (x = 0; x < flc.files; x++)
    output (fh ? fh : stdout, &file[x]);

  if (flc.html)
    fprintf (fh ? fh : stdout, "</pre></tt></body></html>\n");

  if (fh)
    fclose (fh);

  return 0;
}


// configfile handling
int
flc_configfile (void)
{
#if     FILENAME_MAX > MAXBUFSIZE
  char buf[FILENAME_MAX + 1];
#else
  char buf[MAXBUFSIZE + 1];
#endif
  char *dirname;
  FILE *fh = NULL;

  dirname = getenv2 ("HOME");
  sprintf (flc.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
    "flc.cfg"
#else
    ".flcrc"
#endif
    , dirname);
#ifdef  DJGPP
  // this is DJGPP specific - not necessary, but prevents confusion
  change_mem (flc.configfile, strlen (flc.configfile), "/", 1, 0, 0, "\\", 1, 0);
#endif

  if (!access (flc.configfile, F_OK))
    {
      if (get_property_int (flc.configfile, "version") < FLC_CONFIG_VERSION)
        {
          strcpy (buf, flc.configfile);
          set_suffix (buf, ".OLD");

          printf ("NOTE: updating config: will be renamed to %s...", buf);

          rename (flc.configfile, buf);

          sync ();
        }
      else return 0;
    }
  else printf ("WARNING: %s not found: creating...", flc.configfile);

  if (!(fh = fopen (flc.configfile, "w"))) // opening the file in text mode
    {                                         //  avoids trouble under DOS
      printf ("FAILED\n\n");
      return -1;
    }
 fclose (fh);                              // we'll use set_property() from now

  sprintf (buf, "%d", FLC_CONFIG_VERSION);
  set_property (flc.configfile, "version", buf, "flc config");

  set_property (flc.configfile, "lha_test",    "lha t \"%s\"",                     "LHA support\n"
                                                                  "%s will be replaced with the file/archive name");
  set_property (flc.configfile, "lha_extract", "lha efi \"%s\" " FLC_ID_NAMES,     NULL);

  set_property (flc.configfile, "lzh_test",    "lha t \"%s\"",                     "LZH support");
  set_property (flc.configfile, "lzh_extract", "lha efi \"%s\" " FLC_ID_NAMES,     NULL);

  set_property (flc.configfile, "zip_test",    "unzip -t \"%s\"",                  "ZIP support");
  set_property (flc.configfile, "zip_extract", "unzip -xojC \"%s\" " FLC_ID_NAMES, NULL);

  set_property (flc.configfile, "rar_test",    "rar_test=unrar t \"%s\"",          "RAR support");
  set_property (flc.configfile, "rar_extract", "rar_extract=unrar x \"%s\" " FLC_ID_NAMES, NULL);

  set_property (flc.configfile, "ace_test",    "unace t \"%s\"",                   "ACE support");
  set_property (flc.configfile, "ace_extract", "unace e \"%s\" " FLC_ID_NAMES,     NULL);

  printf ("OK\n\n");

  return 0;
}
