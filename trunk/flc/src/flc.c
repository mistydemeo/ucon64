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


static void flc_exit (void);
static int flc_configfile (void);
static void flc_usage (int argc, char *argv[]);
static const char *flc_title = "flc " FLC_VERSION_S " " CURRENT_OS_S " 1999-2004 by NoisyB (noisyb@gmx.net)";

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
  st_file_t file[FLC_MAX_FILES];
  int x = 0, c;
  FILE *fh = NULL;
  int option_index = 0;
  static const struct option long_options[] = {
    {"frontend", 0, 0, 1},
    {"o", 1, 0, 'o'},
    {"t", 0, 0, 't'},
    {"X", 0, 0, 'X'},
    {"S", 0, 0, 'S'},
    {"fr", 0, 0, 2},
    {"k", 0, 0, 'k'},
    {"html", 0, 0, 3},
    {"c", 0, 0, 'c'},
    {"h", 0, 0, 'h'},
    {"help", 0, 0, 'h'},
    {"bbs", 0, 0, 4},
    {"?", 0, 0, 'h'},
    {"version", 0, 0, 'v'},
    {0, 0, 0, 0}
  };

  memset (&flc, 0, sizeof (st_flc_t));
  
  flc_configfile ();

  while ((c = getopt_long (argc, argv, "o:tXSkchv", long_options, &option_index)) != -1)
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
          flc_usage (argc, argv);
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
      flc_usage (argc, argv);
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


void
flc_usage (int argc, char *argv[])
{
  (void) argc;

  printf ("\n%s\n"
          "This may be freely redistributed under the terms of the GNU Public License\n\n"
          "Usage: %s [OPTION]... [FILE]...\n\n"
          "  " OPTION_S "c            also test every possible archive in DIRECTORY for errors\n"
          "                  return flags: N=not checked (default), P=passed, F=failed\n"
          "  " OPTION_LONG_S "html        output as HTML document with links to the files\n"
          "  " OPTION_LONG_S "bbs         output as BBS style filelisting (default)\n"
          "  " OPTION_LONG_S "o=FILE      write output into FILE\n"
          "  " OPTION_S "t            sort by modification time\n"
          "  " OPTION_S "X            sort alphabetical\n"
          "  " OPTION_S "S            sort by byte size\n"
          "  " OPTION_LONG_S "fr          sort reverse\n"
          "  " OPTION_S "k            show sizes in kilobytes\n"
          "  " OPTION_LONG_S "help        display this help and exit\n"
          "  " OPTION_LONG_S "version     output version information and exit\n"
          "\n"
          "Amiga version: noC-flc Version v1.O (File-Listing Creator) - (C)1994 nocTurne deSign/MST\n"
          "Report problems to noisyb@gmx.net or go to http://ucon64.sf.net\n\n",
          flc_title, argv[0]);
}


int
flc_configfile (void)
{
  char buf[FILENAME_MAX + 1];
  FILE *fh;
/*
   configfile handling
*/
  sprintf (flc.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
           "flc.cfg"
#else
           /*
              Use getchd() here too. If this code gets compiled under Windows the compiler has to be
              Cygwin. So, uCON64 will be a Win32 executable which will run only in an environment
              where long filenames are available and where files can have more than three characters
              as "extension". Under Bash HOME will be set, but most Windows people will propably run
              uCON64 under cmd or command where HOME is not set by default. Under Windows XP/2000
              (/NT?) USERPROFILE and/or HOMEDRIVE+HOMEPATH will be set which getchd() will "detect".
            */
           ".flcrc"
#endif
           , getenv2 ("HOME"));


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


  if (!(fh = fopen (flc.configfile, "wb")))
    {
      printf ("FAILED\n\n");
      return -1;
    }

  fprintf (fh, "# flc config\n"
         "#\n"
         "version=%d\n"
         "#\n"
         "# LHA support\n"
         "#\n"
         "lha_test=lha t \"%%s\"\n"
         "lha_extract=lha efi \"%%s\" " FLC_ID_NAMES "\n"
         "#\n"
         "# LZH support\n"
         "#\n"
         "lzh_test=lha t \"%%s\"\n"
         "lzh_extract=lha efi \"%%s\" " FLC_ID_NAMES "\n"
         "#\n"
         "# ZIP support\n"
         "#\n"
         "zip_test=unzip -t \"%%s\"\n"
         "zip_extract=unzip -xojC \"%%s\" " FLC_ID_NAMES "\n"
         "#\n"
         "# RAR support\n"
         "#\n"
         "rar_test=unrar t \"%%s\"\n"
         "rar_extract=unrar x \"%%s\" " FLC_ID_NAMES "\n"
         "#\n"
         "# ACE support\n"
         "#\n"
         "ace_test=unace t \"%%s\"\n"
         "ace_extract=unace e \"%%s\" " FLC_ID_NAMES "\n"
         "#\n"
         "# more?\n"
         "#\n"
         "# %%s will be replaced with the file/archive name\n"
         "#\n",
         FLC_CONFIG_VERSION);

  fclose (fh);
  printf ("OK\n\n");

  return 0;
}
