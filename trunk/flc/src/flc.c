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

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "config.h"
#include "getopt.h"
#include "misc.h"
#include "flc.h"
#include "extract.h"
#include "sort.h"
#include "output.h"

static void flc_exit (void);
static int flc_configfile (void);

struct flc_ flc;

void
flc_exit (void)
{
  printf ("+++EOF");
  fflush (stdout);
}

int
main (int argc, char *argv[])
{
  char buf[FILENAME_MAX + 1];
  struct file_ *file0 = NULL, *file = NULL, file_ns;
  struct dirent *ep;
  struct stat puffer;
  long x = 0;
//int single_file=0;
  DIR *dp;
  int c;
  int option_index = 0;
  struct option long_options[] = {
    {"frontend", 0, 0, 1},
    {"t", 0, 0, 't'},
    {"X", 0, 0, 'X'},
    {"S", 0, 0, 'S'},
    {"fr", 0, 0, 2},
    {"k", 0, 0, 'k'},
    {"html", 0, 0, 3},
    {"c", 0, 0, 'c'},
    {"h", 0, 0, 'h'},
    {"help", 0, 0, 'h'},
    {"?", 0, 0, 'h'},
    {0, 0, 0, 0}
  };

  memset (&flc, 0, sizeof (struct flc_));

  flc_configfile ();

  while ((c =
          getopt_long_only (argc, argv, "", long_options,
                            &option_index)) != -1)

    {
      switch (c)
        {
        case 1:
          atexit (flc_exit);
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
          if (flc.sort)
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

        default:
          printf ("Try '%s --help' for more information.\n", argv[0]);
          return -1;
        }
    }

  if (optind < argc)
    {
      while (optind < argc)
        strcpy (flc.path, argv[optind++]);
    }

  flc.argc = argc;
  for (x = 0; x < argc; x++)
    flc.argv[x] = argv[x];

  getProperty (flc.configfile, "file_id_diz", flc.config, "file_id.diz");

  if (flc.html)
    printf ("<html><head><title></title></head><body><pre><tt>");

  if (!flc.path[0])
    getcwd (flc.path, (size_t) sizeof (flc.path));
  if (flc.path[strlen (flc.path) - 1] == FILE_SEPARATOR
      && strlen (flc.path) != 1)
    flc.path[strlen (flc.path) - 1] = 0;

/*
    single file handling
*/
  if (stat (flc.path, &puffer) != -1 && S_ISREG (puffer.st_mode) == TRUE)
    {
      file = &file_ns;

      file->next = NULL;
      (file->sub).date = puffer.st_mtime;
      (file->sub).size = puffer.st_size;
      (file->sub).checked = 'N';
      strcpy ((file->sub).name, flc.path);
      flc.path[0] = 0;

      extract (&file->sub);

      output (&file->sub);

      return 0;
    }

  if (!(dp = opendir (flc.path)))
    {
      flc_usage (argc, argv);
      return -1;
    }

  while ((ep = readdir (dp)) != NULL)
    {
      sprintf (buf, "%s/%s", flc.path, ep->d_name);

      if (stat (buf, &puffer) == -1)
        continue;
      if (S_ISREG (puffer.st_mode) != TRUE)
        continue;

      if (file0 == NULL)
        {
          if (!(file = (struct file_ *) malloc (sizeof (struct file_))))
            {
              printf ("%s: Error allocating memory\n",
                      argv[0]);
              (void) closedir (dp);
              return -1;
            }

          file0 = file;
        }
      else
        {
          if (!
              ((file->next) =
               (struct file_ *) malloc (sizeof (struct file_))))
            {
              printf ("%s: Error allocating memory\n",
                      argv[0]);
              (void) closedir (dp);
              return -1;
            }
          file = file->next;
        }

      (file->sub).date = puffer.st_mtime;
      (file->sub).size = puffer.st_size;
      (file->sub).checked = 'N';
      strcpy ((file->sub).name, ep->d_name);
      extract (&file->sub);
    }

  (void) closedir (dp);
  file->next = NULL;
  file = file0;

  if (flc.sort)
    sort (file);

  for (;;)
    {
      output (&file->sub);
      if (file->next == NULL)
        break;
      file = file->next;
    }
  free (file0);

  if (flc.html)
    printf ("</pre></tt></body></html>\n");

  return 0;
}

void
flc_usage (int argc, char *argv[])
{
  printf ("\n%s\n"
          "This may be freely redistributed under the terms of the GNU Public License\n\n"
          "USAGE: %s [OPTION]... [FILE]...\n\n"
          "  " OPTION_S "c            also test every possible archive in DIRECTORY for errors\n"
          "                return flags: N=not checked (default), P=passed, F=failed\n"
          "  " OPTION_LONG_S "html        output as HTML document with links to the files\n"
          "  " OPTION_S "t            sort by modification time\n"
          "  " OPTION_S "X            sort alphabetical\n"
          "  " OPTION_S "S            sort by byte size\n"
          "  " OPTION_LONG_S "fr          sort reverse\n"
          "  " OPTION_S "k            show sizes in kilobytes\n"
          "\n"
          "Amiga version: noC-FLC Version v1.O (File-Listing Creator) - (C)1994 nocTurne deSign/MST\n"
          "Report problems to noisyb@gmx.net or go to http://ucon64.sf.net\n\n",
          flc_TITLE, argv[0]);
}

int
flc_configfile (void)
{
  char buf[FILENAME_MAX + 1];
  char buf2[FILENAME_MAX];
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
           , getchd (buf2, FILENAME_MAX));


  if (access (flc.configfile, F_OK) == -1)
    printf ("ERROR: %s not found: creating...", flc.configfile);
  else if (getProperty (flc.configfile, "version", buf2, NULL) == NULL)
    {
      strcpy (buf2, buf);
      newext (buf2, ".OLD");

      printf ("NOTE: updating config: will be renamed to %s...", buf2);

      rename (flc.configfile, buf2);

      sync ();
    }

  if (access (flc.configfile, F_OK) == -1)
    {
      FILE *fh;

      if (!(fh = fopen (flc.configfile, "wb")))
        {
          printf ("FAILED\n\n");

          return -1;
        }
      else
        {
          fputs ("# flc config\n"
                 "#\n"
                 "version=101\n"
                 "#\n"
                 "# LHA support\n"
                 "#\n"
                 "lha_test=lha 2>&1 >/dev/null t\n"
                 "lha_extract=lha 2>&1 >/dev/null efi \n"
                 "#lha_extract=lha 2>&1 >/dev/null e \n"
                 "#\n"
                 "# LZH support\n"
                 "#\n"
                 "lzh_test=lha 2>&1 >/dev/null t\n"
                 "lzh_extract=lha 2>&1 >/dev/null efi\n"
                 "#lzh_extract=lha 2>&1 >/dev/null e\n"
                 "#\n"
                 "# ZIP support\n"
                 "#\n"
                 "zip_test=unzip 2>&1 >/dev/null -t\n"
                 "zip_extract=unzip 2>&1 >/dev/null -xojC\n"
                 "#zip_extract=unzip 2>&1 >/dev/null -xoj\n"
                 "#\n"
                 "# RAR support\n"
                 "#\n"
                 "rar_test=unrar 2>&1 >/dev/null t\n"
                 "rar_extract=unrar 2>&1 >/dev/null x\n"
                 "#\n"
                 "# ACE support\n"
                 "#\n"
                 "ace_test=unace 2>&1 >/dev/null t\n"
                 "ace_extract=unace 2>&1 >/dev/null e\n"
                 "#\n"
                 "# TXT/NFO/FAQ support\n"
                 "#\n"
                 "txt_extract=txtextract\n"
                 "nfo_extract=txtextract\n"
                 "faq_extract=txtextract\n"
                 "#\n"
                 "# FILE_ID.DIZ names/synonyms\n"
                 "#\n"
                 "file_id_diz=*_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]\n",
                 fh);

          fclose (fh);
          printf ("OK\n\n");
        }

      return 0;
    }
  return 0;
}
