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
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "config.h"
#include "getopt.h"
#include "misc.h"
#include "flc.h"
#include "flc_misc.h"

static void flc_exit (void);
static int flc_configfile (void);

static const char *flc_title = "flc " FLC_VERSION_S " " CURRENT_OS_S
                       " 1999-2002 by NoisyB (noisyb@gmx.net)";

st_flc_t flc;

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
  char temp[FILENAME_MAX];
  char cwd[FILENAME_MAX];
  st_file_t file, *file_p = NULL, *file_0 = NULL;
  char path[FILENAME_MAX];
  struct dirent *ep;
  struct stat puffer;
  DIR *dp;
  int c;
  int option_index = 0;
  static const struct option long_options[] = {
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
    {"version", 0, 0, 'v'},
    {0, 0, 0, 0}
  };

  memset (&flc, 0, sizeof (st_flc_t));

  flc_configfile ();

  while ((c =
          getopt_long (argc, argv, "tXSkchv", long_options,
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

        case 'v':
          printf ("%s\n", FLC_VERSION_S);
          return 0;

        default:
          printf ("Try '%s --help' for more information.\n", argv[0]);
          return -1;
        }
    }
                    
  if (flc.html)
    printf ("<html><head><title></title></head><body><pre><tt>");
  if (optind < argc)
    {
      while (optind < argc)
        strcpy (path, argv[optind++]);
    }

  if (!path[0])
    getcwd (path, FILENAME_MAX);
  else 
    if (stat (path, &puffer) != -1)
      if (S_ISREG (puffer.st_mode) == TRUE)
      {
/*
    single file handling
*/
        file.next = NULL;
        file.sub.date = puffer.st_mtime;
        file.sub.size = puffer.st_size;
        file.sub.checked = 'N';
        strcpy (file.sub.name, path);

        extract (&file.sub);

        output (&file.sub);

        return 0;
      }

/*
  turn relative path into absolute path
*/
  getcwd (cwd, FILENAME_MAX);
  chdir (path);
  getcwd (path, FILENAME_MAX);
  chdir (cwd);

/*
  multiple file handling
*/
  if (!(dp = opendir (path)))
    {
      flc_usage (argc, argv);
      return -1;
    }

  if (!tmpnam2 (temp))
    {
      fprintf (stderr, "ERROR: could not create temp dir");
      return -1;
    }
  if (mkdir (temp, S_IRUSR|S_IWUSR) == -1)
    {
      fprintf (stderr, "ERROR: could not create temp dir");
      return -1;
    }

  getcwd (cwd, FILENAME_MAX);
  chdir (temp);

  file_0 = file_p = NULL;

  while ((ep = readdir (dp)) != NULL)
    {
      sprintf (buf, "%s/%s", path, ep->d_name);

      if (stat (buf, &puffer) == -1)
        continue;
      if (S_ISREG (puffer.st_mode) != TRUE)
        continue;

      if (!file_p) file_0 = file_p = (st_file_t *) malloc (sizeof (st_file_t));
      else
        {
          file_p->next = (st_file_t *) malloc (sizeof (st_file_t));

          if (!file_p->next)
            {
              fprintf (stderr, "ERROR: allocating memory\n");
              (void) closedir (dp);
              return -1;
            }

          file_p = file_p->next;
        }
        
      file_p->sub.date = puffer.st_mtime;
      file_p->sub.size = puffer.st_size;
      file_p->sub.checked = 'N';
      strcpy (file_p->sub.fullpath, buf);
      strcpy (file_p->sub.name, basename2 (buf));
      extract (&file_p->sub);
    }
  (void) closedir (dp);
  chdir (cwd);
  rmdir2 (temp);
  if (file_p) file_p->next = NULL;
  file_p = file_0;

  if (flc.sort) sort (file_p);

  while (file_p)
    {
      output (&file_p->sub);
      file_p = file_p->next;
    }

  free (file_p);

  if (flc.html)
    printf ("</pre></tt></body></html>\n");

  return 0;
}


void
flc_usage (int argc, char *argv[])
{
  printf ("\n%s\n"
          "This may be freely redistributed under the terms of the GNU Public License\n\n"
          "Usage: %s [OPTION]... [FILE]...\n\n"
          "  " OPTION_S
          "c           also test every possible archive in DIRECTORY for errors\n"
          "                return flags: N=not checked (default), P=passed, F=failed\n"
          "  " OPTION_LONG_S
          "html        output as HTML document with links to the files\n" "  "
          OPTION_S "t           sort by modification time\n" "  " OPTION_S
          "X           sort alphabetical\n" "  " OPTION_S
          "S           sort by byte size\n" "  " OPTION_LONG_S
          "fr          sort reverse\n" "  " OPTION_S
          "k           show sizes in kilobytes\n" "  " OPTION_LONG_S
          "help        display this help and exit\n" "  " OPTION_LONG_S
          "version     output version information and exit\n" "\n"
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
      if (strtol (get_property (flc.configfile, "version", buf, "0"), NULL, 10) < FLC_CONFIG_VERSION)
        {
          strcpy (buf, flc.configfile);
          setext (buf, ".OLD");

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
         "lha_extract=lha efi \"%%s\" *_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]\n"
         "#\n"
         "# LZH support\n"
         "#\n"
         "lzh_test=lha t \"%%s\"\n"
         "lzh_extract=lha efi \"%%s\" *_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]\n"
         "#\n"
         "# ZIP support\n"
         "#\n"
         "zip_test=unzip -t \"%%s\"\n"
         "zip_extract=unzip -xojC \"%%s\" *_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]\n"
         "#\n"
         "# RAR support\n"
         "#\n"
         "rar_test=unrar t \"%%s\"\n"
         "rar_extract=unrar x \"%%s\" *_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]\n"
         "#\n"
         "# ACE support\n"
         "#\n"
         "ace_test=unace t \"%%s\"\n"
         "ace_extract=unace e \"%%s\" *_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]\n"
         "#\n"
         "# TXT/NFO/FAQ support\n"
         "#\n"
         "txt_extract=txtextract \"%%s\"\n"
         "nfo_extract=txtextract \"%%s\"\n"
         "faq_extract=txtextract \"%%s\"\n"
         "#\n"
         "# MP3 (ID3) support\n"
         "#\n"
         "mp3_extract=id3extract \"%%s\"\n",
         FLC_CONFIG_VERSION);

  fclose (fh);
  printf ("OK\n\n");

  return 0;
}
