/*
ucon64_dat.c - support for DAT files as known from Romcenter, Goodxxxx, etc.

written by 1999 - 2003 NoisyB (noisyb@gmx.net)
           2002 - 2003 dbjh


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
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "misc.h"
#include "quick_io.h"

#include "console/snes.h"
#include "console/gb.h"
#include "console/gba.h"
#include "console/n64.h"
#include "console/lynx.h"
#include "console/sms.h"
#include "console/nes.h"
#include "console/genesis.h"
#include "console/pce.h"
#include "console/neogeo.h"
#include "console/ngp.h"
#include "console/swan.h"
#include "console/dc.h"
#include "console/jaguar.h"
#include "console/psx.h"

#define MAX_FIELDS_IN_DAT 32
#define DAT_FIELD_SEPARATOR (0xac)
#define DAT_FIELD_SEPARATOR_S ("\xac")

#define NEW_CODE

typedef struct
{
  const char *id; // strings to detect console from the datfile name
  uint8_t console; // UCON64_SNES, UCON64_NES, etc.
  const char **console_usage;
} st_console_t;

typedef struct
{
  uint32_t crc32;
  long filepos;
} st_idx_entry_t;

static DIR *dptr = NULL;
static FILE *fdat = NULL;
static long filepos_line = 0;


static void
closedir_dptr (void)
{
  if (dptr)
    closedir (dptr);
  dptr = NULL;
}


static void
fclose_fdat (void)
{
  if (fdat)
    fclose (fdat);
  fdat = NULL;
}


static uint32_t
get_uint32 (unsigned char *buf)
{
  return (uint32_t) (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];
}


static char *
get_next_file (char *fname)
{
  struct dirent *ep;

  if (!dptr)
    if (!(dptr = opendir (ucon64.configdir)))
      {
        fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.configdir);
        dptr = NULL;
        return NULL;
      }

  while ((ep = readdir (dptr)) != NULL)
    if (!stricmp (getext (ep->d_name), ".dat"))
      {
        sprintf (fname, "%s" FILE_SEPARATOR_S "%s", ucon64.configdir,
                 ep->d_name);
        return fname;
      }

  closedir_dptr ();
  return NULL;
}


static st_ucon64_dat_t *
get_dat_header (char *fname, st_ucon64_dat_t *dat)
{
#if     (MAXBUFSIZE < FILENAME_MAX)
  char buf[FILENAME_MAX];
#else
  char buf[MAXBUFSIZE];
#endif

  // Hell yes!!! I use get_property() here...
  strncpy (dat->author, get_property (fname, "author", buf, "Unknown"), sizeof (dat->author));
  dat->author[sizeof (dat->author) - 1] = 0;
  strncpy (dat->version, get_property (fname, "version", buf, "?"), sizeof (dat->version));
  dat->version[sizeof (dat->version) - 1] = 0;
  strncpy (dat->refname, get_property (fname, "refname", buf, ""), sizeof (dat->refname));
  dat->refname[sizeof (dat->refname) - 1] = 0;
  strcpy (dat->comment, get_property (fname, "comment", buf, ""));
  strncpy (dat->date, get_property (fname, "date", buf, "?"), sizeof (dat->date));
  dat->date[sizeof (dat->date) - 1] = 0;

  return dat;
}


static st_ucon64_dat_t *
line_to_dat (const char *fname, const char *dat_entry, st_ucon64_dat_t *dat)
{
// parse a dat entry into st_ucon64_dat_t
  static const char *dat_country[] = {
    "(1) Japan & Korea",
    "(A) Australia",
    "(B) non U.S.A. (Genesis)",
    "(C) China",
    "(E) Europe",
    "(F) France",
    "(FC) French Canadian",
    "(FN) Finland",
    "(G) Germany",
    "(GR) Greece",
    "(HK) Hong Kong",
    "(4) U.S.A. & Brazil NTSC",
    "(J) Japan",
    "(K) Korea",
    "(NL) Netherlands",
    "(PD) Public Domain",
    "(S) Spain",
    "(SW) Sweden",
    "(U) U.S.A.",
    "(UK) England",
    "(Unk) Unknown Country",
    "(I) Italy",
    "(UE) U.S.A. & Europe",
    "(JU) Japan & U.S.A.",
    "(JUE) Japan, U.S.A. & Europe",
    NULL
  };

  static const st_console_t console_type[] = {
    {"snes", UCON64_SNES, snes_usage},
    {"super nintendo", UCON64_SNES, snes_usage},
    {"goodnes", UCON64_NES, nes_usage},
    {"nes", UCON64_NES, nes_usage},
    {"gb", UCON64_GB, gameboy_usage},
    {"gbx", UCON64_GB, gameboy_usage},
    {"genesis", UCON64_GENESIS, genesis_usage},
    {"gen", UCON64_GENESIS, genesis_usage},
    {"sms", UCON64_SMS, sms_usage},
    {"jag", UCON64_JAGUAR, jaguar_usage},
    {"lynx", UCON64_LYNX, lynx_usage},
    {"n64", UCON64_N64, n64_usage},
    {"ultra", UCON64_N64, n64_usage},
    {"neo", UCON64_NEOGEO, neogeo_usage},
    {"geo", UCON64_NEOGEO, neogeo_usage},
    {"pce", UCON64_PCE, pcengine_usage},
    {"engine", UCON64_PCE, pcengine_usage},
    {"psx", UCON64_PSX, psx_usage},
    {"ps1", UCON64_PSX, psx_usage},
    {"psone", UCON64_PSX, psx_usage},
    {"ps2", UCON64_PS2, ps2_usage},
    {"sat", UCON64_SATURN, sat_usage},
    {"dreamcast", UCON64_DC, dc_usage},
    {"dc", UCON64_DC, dc_usage},
    {"cd32", UCON64_CD32, cd32_usage},
    {"cdi", UCON64_CDI, cdi_usage},
    {"3do", UCON64_REAL3DO, real3do_usage},
    {"2600", UCON64_ATARI, atari_usage},
    {"5200", UCON64_ATARI, atari_usage},
    {"7800", UCON64_ATARI, atari_usage},
    {"system", UCON64_SYSTEM16, s16_usage},
    {"pocket", UCON64_NEOGEOPOCKET, ngp_usage},
    {"gba", UCON64_GBA, gba_usage},
    {"advance", UCON64_GBA, gba_usage},
    {"vectrex", UCON64_VECTREX, vectrex_usage},
    {"virtual", UCON64_VIRTUALBOY, vboy_usage},
    {"swan", UCON64_WONDERSWAN, swan_usage},
    {"coleco", UCON64_COLECO, coleco_usage},
    {"intelli", UCON64_INTELLI, intelli_usage},
#if 0
    {"", 0, cd32_usage},
    {"", 0, cdi_usage},
    {"", 0, channelf_usage},
    {"", 0, coleco_usage},
    {"", 0, gamecom_usage},
    {"", 0, gc_usage},
    {"", 0, gp32_usage},
    {"", 0, intelli_usage},
    {"", 0, odyssey2_usage},
    {"", 0, odyssey_usage},
    {"", 0, real3do_usage},
    {"", 0, s16_usage},
    {"", 0, sat_usage},
    {"", 0, vboy_usage},
    {"", 0, vc4000_usage},
    {"", 0, vectrex_usage},
    {"", 0, xbox_usage},
#endif
    {0, 0, 0}
  };

  unsigned char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL };
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], *p = NULL;
  uint32_t pos = 0;

  if ((unsigned char) dat_entry[0] != DAT_FIELD_SEPARATOR)
    return NULL;

  strcpy (buf, dat_entry);

  for (pos = 0;
       (dat_field[pos] = strtok (!pos ? buf : NULL, DAT_FIELD_SEPARATOR_S))
       && pos < (MAX_FIELDS_IN_DAT - 1); pos++);

  memset (dat, 0, sizeof (st_ucon64_dat_t));

  strcpy (dat->datfile, basename2 (fname));

  if (dat_field[5])
    sscanf (dat_field[5], "%x", &dat->current_crc32);

  if (dat_field[3])
    strcpy (dat->name, dat_field[3]);

  if (dat_field[6])
    sscanf (dat_field[6], "%d", &dat->fsize);

  if (dat_field[4])
    strcpy (dat->fname, dat_field[4]);

  p = dat->name;
  // Often flags contain numbers, so don't search for the closing bracket
  sprintf (buf2,
    "%s%s%s%s%s%s%s%s%s%s",
    (strstr (p, "[a") ? "Alternate, " : ""),
    (strstr (p, "[p") ? "Pirate, " : ""),
    (strstr (p, "[b") ? "Bad Dump, " : ""),
    (strstr (p, "[t") ? "Trained, " : ""),
    (strstr (p, "[f") ? "Fixed, " : ""),
    (strstr (p, "[T") ? "Translation, " : ""),
    (strstr (p, "[h") ? "Hack, " : ""),
    (strstr (p, "[x") ? "Bad Checksum, " : ""),
    (strstr (p, "[o") ? "Overdump, " : ""),
    (strstr (p, "[!]") ? "Verified Good Dump, " : "")); // [!] is ok
  if (buf2[0])
    {
      if ((p = strrchr (buf2, ',')))
        *p = 0;
      sprintf (dat->misc, "Flags: %s", buf2);
    }

  p = dat->name;
  dat->country = NULL;
  for (pos = 0; dat_country[pos]; pos++)
    {
      strcpy (buf, dat_country[pos]);
      *(strchr (buf, ' ')) = 0;
      if (stristr (p, buf))
        {
          dat->country = dat_country[pos];
          break;
        }
    }

  dat->console = UCON64_UNKNOWN;
#if 0
  if (ucon64.console != UCON64_UNKNOWN)
    dat->console = (uint8_t) ucon64.console;  // important
  else
#endif
    {
      for (pos = 0; console_type[pos].id; pos++)
        {
          if (stristr (dat->datfile, console_type[pos].id) ||
              stristr (dat->refname, console_type[pos].id))
            {
              dat->console = console_type[pos].console;
              dat->console_usage = console_type[pos].console_usage;
              break;
            }
        }
    }

  dat->copier_usage = unknown_usage;

  return dat;
}


uint32_t
line_to_crc (const char *dat_entry)
{
// get crc32 of current line
  unsigned char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL };
  uint32_t pos = 0, crc32 = 0;
  char buf[MAXBUFSIZE];

  if ((unsigned char) dat_entry[0] != DAT_FIELD_SEPARATOR)
    return 0;

  strcpy (buf, dat_entry);

  for (pos = 0;
       (dat_field[pos] = strtok (!pos ? buf : NULL, DAT_FIELD_SEPARATOR_S))
       && pos < (MAX_FIELDS_IN_DAT - 1); pos++);

  if (dat_field[5])
    sscanf (dat_field[5], "%x", &crc32);

  return crc32;
}


static st_ucon64_dat_t *
get_dat_entry (char *fname, st_ucon64_dat_t *dat, uint32_t crc32, long start)
{
  char buf[MAXBUFSIZE];

  if (!fdat)
    if (!(fdat = fopen (fname, "rb")))
      {
        fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], fname);
        return NULL;
      }

  if (start >= 0)
    fseek (fdat, start, SEEK_SET);

  filepos_line = ftell (fdat);
  while (fgets (buf, MAXBUFSIZE, fdat) != NULL)
    {
      if ((unsigned char) buf[0] == DAT_FIELD_SEPARATOR)
        if (!crc32 || line_to_crc (buf) == crc32)
          if (line_to_dat (fname, buf, dat))
            return dat;
      filepos_line = ftell (fdat);
    }

  fclose_fdat ();
  return NULL;
}


int
ucon64_dat_view (int console)
{
//  char *p = "";
  char buf[FILENAME_MAX], buf2[FILENAME_MAX];
  static st_ucon64_dat_t dat;
  uint32_t entries, dat_counter = 0;

  closedir_dptr ();
  while (get_next_file (buf))
    {
      get_dat_header (buf, &dat);
      strcpy (buf2, buf);
      setext (buf2, ".idx");
#ifdef  NEW_CODE
      entries = q_fsize (buf2) / sizeof (st_idx_entry_t);
#else
      entries = q_fsize (buf2) / sizeof (uint32_t);
#endif
      dat_counter++;

      printf ("DAT info:\n"
        "  %s\n"
//        "  Console: %s\n"
        "  Version: %s (%s, %s)\n"
        "  Author: %s\n"
        "  Comment: %s\n"
        "  Entries: %d\n\n",
        basename2 (buf),
//        dat.console_usage[0],
        dat.version,
        dat.date,
        dat.refname,
        dat.author,
        dat.comment,
        entries);
    }

  printf ("Total DAT files: %d; Total entries: %d\n",
    dat_counter, ucon64_dat_total_entries (UCON64_UNKNOWN));

  return 0;
}


unsigned int
ucon64_dat_total_entries (int console)
{
  uint32_t entries = 0;
  int fsize;
  char buf[FILENAME_MAX];

  if (!ucon64.dat_enabled)
    return 0;

  closedir_dptr ();
  while (get_next_file (buf))
    {
      setext (buf, ".idx");
      fsize = q_fsize (buf);
#ifdef  NEW_CODE
      entries += (fsize < 0 ? 0 : fsize / sizeof (st_idx_entry_t)); // TODO: handle this case gracefully
#else
      entries += (fsize < 0 ? 0 : fsize / sizeof (uint32_t)); // TODO: handle this case gracefully
#endif
    }

  return entries;
}


#ifdef  NEW_CODE
static int
idx_compare (const void *key, const void *found)
{
  /*
    The return statement looks overly complicated, but is really necessary.
    This contruct:
      return ((st_idx_entry_t *) key)->crc32 - ((st_idx_entry_t *) found)->crc32;
    does *not* work correctly for all cases.
  */
  return ((int64_t) ((st_idx_entry_t *) key)->crc32 -
          (int64_t) ((st_idx_entry_t *) found)->crc32) / 2;
}
#endif


st_ucon64_dat_t *
ucon64_dat_search (uint32_t crc32, st_ucon64_dat_t *dat)
{
  char buf[FILENAME_MAX];
  unsigned char *p = NULL;
  uint32_t fsize = 0;
  FILE *fh = NULL;
#ifdef  NEW_CODE
  st_idx_entry_t *idx_entry, key;
#else
  uint32_t pos = 0;
#endif

  if (!crc32)
    return NULL;

  closedir_dptr ();
  while (get_next_file (buf) != NULL)
    {
      // load the index for the current dat file
      setext (buf, ".idx");

      if (!(fh = fopen (buf, "rb")))
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], buf);
          return NULL;
        }

      fsize = q_fsize (buf);

      if (!(p = (unsigned char *) malloc (fsize)))
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], fsize);
          fclose (fh);
          return NULL;
        }

      if (fread (p, 1, fsize, fh) != fsize)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], buf);
          fclose (fh);
          free (p);
          return NULL;
        }

      fclose (fh);

      // search index for crc
#ifdef  NEW_CODE
      key.crc32 = crc32;
      idx_entry = bsearch (&key, p, fsize / sizeof (st_idx_entry_t),
                           sizeof (st_idx_entry_t), idx_compare);
      if (idx_entry)                            // crc32 found
        {
          // open dat file and read entry
          setext (buf, ".dat");

          fclose_fdat ();
          while (get_dat_entry (buf, dat, crc32, idx_entry->filepos))
            if (crc32 == dat->current_crc32)
              {
                strcpy (dat->datfile, basename2 (buf));
                get_dat_header (buf, dat);
                free (p);
                return dat;
              }
        }
#else
      for (pos = 0; pos < fsize; pos += sizeof (uint32_t))
        if (get_uint32 (&p[pos]) == crc32) // crc32 found
          {
            // open dat file and read entry
            free (p);
            setext (buf, ".dat");

            fclose_fdat ();
            while (get_dat_entry (buf, dat, crc32, -1))
              if (crc32 == dat->current_crc32)
                {
                  strcpy (dat->datfile, basename2 (buf));
                  get_dat_header (buf, dat);
                  return dat;
                }
          }
#endif

      free (p);
    }

  return NULL;
}


int
ucon64_dat_indexer (void)
// create or update index of cache
{
  char buf[FILENAME_MAX], buf2[FILENAME_MAX];
  struct stat cache, index;
  st_ucon64_dat_t dat;
  FILE *fh = NULL;
  uint32_t size = 0, pos;
  time_t start_time = 0;
  int update = 0;
#ifdef  NEW_CODE
#define MAX_GAMES_FOR_CONSOLE 50000
  st_idx_entry_t *idx_entries, *idx_entry;
  int duplicates, n;
  FILE *errorfile;
  char errorfname[FILENAME_MAX];

  if (!(idx_entries = (st_idx_entry_t *)
        malloc (MAX_GAMES_FOR_CONSOLE * sizeof (st_idx_entry_t)))) // TODO?: dynamic size
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR],
        MAX_GAMES_FOR_CONSOLE * sizeof (st_idx_entry_t));
      exit (1);
    }
#endif

  closedir_dptr ();
  while (get_next_file (buf))
    {
      strcpy (buf2, buf);
      setext (buf2, ".idx");

      if (!stat (buf, &cache) && !stat (buf2, &index))
        {
          if (cache.st_mtime < index.st_mtime)
            {
              // index file seems to be present and up-to-date
              continue;
            }
          update = 1; // idx file is present
        }

      if (!(fh = fopen (buf2, "wb")))
        {
          fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], buf2);
          continue;
        }

      start_time = time (0);
      size = q_fsize (buf);

      fprintf (stdout, "%s: %s\n",
        (!update ? "Create" : "Update"), basename2 (buf2));

      fclose_fdat ();
      pos = 0;
      duplicates = 0;
      errorfile = NULL;
      while (get_dat_entry (buf, &dat, 0, -1))
        {
#ifdef  NEW_CODE
          /*
            Doing a linear search removes the need of using the slow qsort()
            function inside the loop. Doing a binary search doesn't improve the
            speed much, but is much more efficient of course. Using qsort()
            inside the loop slows it down with a factor of more than 10.
          */
          idx_entry = NULL;
          for (n = 0; n < pos; n++)
            if (idx_entries[n].crc32 == dat.current_crc32)
              idx_entry = &idx_entries[n];
          if (idx_entry)
            {
              // This really makes one loose trust in the DAT files...
              char current_name[2 * 80];
              long current_filepos = ftell (fdat);

              if (!errorfile)
                {
                  strcpy (errorfname, buf2);
                  setext (errorfname, ".err");
                  if (!(errorfile = fopen (errorfname, "w"))) // text file for WinDOS
                    {
                      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], errorfname);
                      continue;
                    }
                }

              strcpy (current_name, dat.name);
              get_dat_entry (buf, &dat, 0, idx_entry->filepos);
              fprintf (errorfile,
                       "\nWarning: DAT file contains a duplicate CRC32 (0x%x)!\n"
                       "  First game with this CRC32: \"%s\"\n"
                       "  Ignoring game:              \"%s\"\n",
                      dat.current_crc32, dat.name, current_name);

              duplicates++;
              fseek (fdat, current_filepos, SEEK_SET);
              continue;
            }

          idx_entries[pos].crc32 = dat.current_crc32;
          idx_entries[pos].filepos = filepos_line;
          if (pos == MAX_GAMES_FOR_CONSOLE - 1)
            {
              fprintf (stderr,
                       "\nINTERNAL ERROR: MAX_GAMES_FOR_CONSOLE is too small (%d)\n",
                       MAX_GAMES_FOR_CONSOLE);
              break;
            }
#else
          fwrite (&dat.current_crc32, sizeof (uint32_t), 1, fh);
#endif
          if (!(pos % 20))
            ucon64_gauge (start_time, ftell (fdat), size);
          pos++;
        }
#ifdef  NEW_CODE
      qsort (idx_entries, pos, sizeof (st_idx_entry_t), idx_compare);
      fwrite (idx_entries, 1, pos * sizeof (st_idx_entry_t), fh);
#endif
      ucon64_gauge (start_time, size, size);

#ifdef  NEW_CODE
      if (duplicates > 0)
        printf ("\n\nWarning: DAT file contains %d duplicate CRC32%s\n"
                "         Warnings have been written to \"%s\"",
                duplicates, duplicates != 1 ? "s" : "", errorfname);
      if (errorfile)
        {
          fclose (errorfile);
          errorfile = NULL;
        }
#endif
      fprintf (stdout, "\n\n");
      fclose (fh);
    }
// stats
#ifdef  NEW_CODE
  free (idx_entries);
#endif

  return 0;
}


void
ucon64_dat_nfo (const st_ucon64_dat_t *dat)
{
  char buf[MAXBUFSIZE], *p = NULL;
  int n;

  if (!dat)
    {
      fprintf (stdout, ucon64_msg[DAT_ERROR], ucon64.crc32);
      return;
    }

  printf ("DAT info:\n");
  // console type?
  if (dat->console_usage != NULL)
    {
      strcpy (buf, dat->console_usage[0]);
      printf ("  %s\n", to_func (buf, strlen (buf), toprint2));

#if 0
      if (dat->console_usage[1])
        {
          strcpy (buf, dat->console_usage[1]);
          printf ("  %s\n", to_func (buf, strlen (buf), toprint2));
        }
#endif
    }

  printf ("  %s\n", dat->name);

  if (dat->country)
    {
      if (!(p = strchr (dat->country, ' '))) // start after the (country)
        p = (char *)dat->country;
      else
        p++;

      printf ("  %s\n", p);
    }

  /*
    The DAT files are not consistent. Some include the file suffix, but
    others don't. We want to display the canonical file name only if it
    really differs from the canonical game name (usualy file name without
    suffix).
  */
  n = strlen (dat->fname);
  p = (char *) dat->fname + n - 4;
  if (stricmp (p, ".nes") &&                    // NES
      stricmp (p, ".gb") &&                     // Game Boy
      stricmp (p, ".smc") &&                    // SNES
      stricmp (p, ".smd") &&                    // Genesis
      stricmp (p, ".v64"))                      // Nintendo 64
    {
      if (stricmp (dat->name, dat->fname) != 0)
        printf ("  Filename: %s\n", dat->fname);
    }
  else
    {
      n -= 4;
      if (strnicmp (dat->name, dat->fname, n) != 0)
        printf ("  Filename: %s\n", dat->fname);
    }

  printf ("  %d Bytes (%.4f Mb)\n",
          dat->fsize,
          TOMBIT_F (dat->fsize));

  if (dat->misc[0])
    printf ("  %s\n", dat->misc);

  if (stristr (dat->datfile, dat->version))
    printf ("  %s (%s, %s)\n",
      dat->datfile,
      dat->date,
      dat->refname);
  else
    printf ("  %s (%s, %s, %s)\n",
      dat->datfile,
      dat->version,
      dat->date,
      dat->refname);
}
