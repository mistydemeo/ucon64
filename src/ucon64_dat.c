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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
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

typedef struct
{
  const char *id;                              // string to detect console from datfile name
  int (*compare) (const void *a, const void *b); // the function which compares the id with the filename
     // compare() == 0 means success
  int8_t console;                              // UCON64_SNES, UCON64_NES, etc.
  const st_usage_t *console_usage;
} st_console_t;

typedef struct
{
  uint32_t crc32;
  long filepos;
} st_idx_entry_t;

static DIR *ddat = NULL;
static FILE *fdat = NULL;
static long filepos_line = 0;
static int warning = 0;                         // show the warning only once when indexing


const st_usage_t ucon64_dat_usage[] = {
  {NULL, NULL, "DATabase (support of DAT files)"},
  {"db", NULL, "DATabase statistics"},
  {"dbv", NULL, "like " OPTION_LONG_S "db but more verbose"},
  {"dbs", "CRC32", "search ROM with CRC32 in DATabase"},
  {"lsd", NULL, "generate ROM list for all ROMs using DATabase; " OPTION_LONG_S "rom" OPTARG_S "ROM or DIR"},
  {"rrom", NULL, "rename ROMs in DIR to their internal names; " OPTION_LONG_S "rom" OPTARG_S "ROM or DIR"},
  {"rr83", NULL, "like " OPTION_LONG_S "rrom but with 8.3 filenames; " OPTION_LONG_S "rom" OPTARG_S "ROM or DIR"},
  {"good", NULL, "used with " OPTION_LONG_S "rrom and " OPTION_LONG_S "rr83 ROMs will be renamed and sorted\n"
              "into subdirs according to the DATabase (\"ROM manager\")"},
/*
GoodSNES: Copyright 1999-2002 Cowering (hotemu@hotmail.com) V 0.999.5 BETA
*visit NEWNet #rareroms*

Usage: GoodSNES [rename|move|scan[d]|scannew[d]|list[d]|audit[2]
              [changes[-]][changesnew[-]][quiet][dirs|inplace][deep][sepX]

rename     = Rename files
renamebad  = Rename bad checksummed files
move       = Move files
move       = Move bad checksummed files
scan       = Generate listing of all files [w/dirs]
scannew    = Generate listing of unknown files [w/dirs]
list       = Lists files you have/need without renaming [w/dirs]
audit      = Prints names not matching any line in Goodinfo.cfg
inplace    = Renames files in same dir, not making 'Ren' dir
changes    = Log rename/move operations (- reverses)
changesnew = Log rename/move operations that change a filename
dirs       = Move to dirs using Goodinfo.cfg info
quiet      = Suppress most non-error messages
sepX       = Replaces space in filenames with char X ('nosep' removes spaces)
deep       = Adds more detail to scanfiles (where applicable)
force63    = Force all filenames into Joliet CD format
Hit ENTER to continue
Several output files are created depending on selected options:

GoodSNES.db  = database of scanned ROMs
SNESMiss     = ROMs missing (if in rename/move/list mode)
SNESHave     = ROMS present (if in rename/move/list mode)
SNESScan     = log of 'scan' and 'scannew'
SNESLog      = log of 'changes' and 'changesnew'
Good_ZIP     = log of ZIP errors
Good_RAR     = log of RAR errors

Stats: 3792 entries, 290 redumps, 83 hacks/trainers, 5 bad/overdumps
*/
  {NULL, NULL, NULL}
};


static void
closedir_ddat (void)
{
  if (ddat)
    closedir (ddat);
  ddat = NULL;
}


static void
fclose_fdat (void)
{
  if (fdat)
    fclose (fdat);
  fdat = NULL;
}


static int
custom_stristr (const void *a, const void *b)
{
  return !stristr (a, b);
}


static int
custom_strnicmp (const void *a, const void *b)
{
  return strnicmp (a, b, MIN (strlen (a), strlen (b)));
}


#if 0
static int
custom_stricmp (const void *a, const void *b)
{
  return stricmp (a, b);
}
#endif


static char *
get_next_file (char *fname)
{
  struct dirent *ep;

  if (!ddat)
    if (!(ddat = opendir (ucon64.configdir)))
      {
        fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.configdir);
        return NULL;
      }

  while ((ep = readdir (ddat)) != NULL)
    if (!stricmp (get_suffix (ep->d_name), ".dat"))
      {
        sprintf (fname, "%s" FILE_SEPARATOR_S "%s", ucon64.configdir,
                 ep->d_name);
        return fname;
      }

  closedir_ddat ();
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

  // Hell yes! I (NoisyB) use get_property() here...
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


static int
fname_to_console (const char *fname, st_ucon64_dat_t *dat)
{
  int pos;
  // We use the filename to find out for what console a DAT file is meant.
  //  The field "refname" seems too unreliable.
  static const st_console_t console_type[] = {
    {"GoodSNES", custom_strnicmp, UCON64_SNES, snes_usage},
    {"GoodNES", custom_strnicmp, UCON64_NES, nes_usage},
    {"FDS", custom_stristr, UCON64_NES, nes_usage},
    {"GoodGBA", custom_strnicmp, UCON64_GBA, gba_usage},
    {"GoodGBX", custom_strnicmp, UCON64_GB, gameboy_usage},
    {"GoodGEN", custom_strnicmp, UCON64_GENESIS, genesis_usage},
    {"GoodGG", custom_strnicmp, UCON64_SMS, sms_usage},
    {"GoodSMS", custom_strnicmp, UCON64_SMS, sms_usage},
    {"GoodJAG", custom_strnicmp, UCON64_JAGUAR, jaguar_usage},
    {"GoodLynx", custom_strnicmp, UCON64_LYNX, lynx_usage},
    {"GoodN64", custom_strnicmp, UCON64_N64, n64_usage},
    {"GoodPCE", custom_strnicmp, UCON64_PCE, pcengine_usage},
    {"Good2600", custom_strnicmp, UCON64_ATARI, atari_usage},
    {"Good5200", custom_strnicmp, UCON64_ATARI, atari_usage},
    {"Good7800", custom_strnicmp, UCON64_ATARI, atari_usage},
//  more or less unique names could be compared with custom_stristr()
    {"Neo-Geo", custom_strnicmp, UCON64_NEOGEO, neogeo_usage},
    {"MAME", custom_stristr, UCON64_MAME, mame_usage},
    {"Dreamcast", custom_stristr, UCON64_DC, dc_usage},
    {"Saturn", custom_stristr, UCON64_SATURN, sat_usage},
    {"3do", custom_stristr, UCON64_REAL3DO, real3do_usage},
    {"CDi", custom_stristr, UCON64_CDI, cdi_usage},
    {"XBox", custom_stristr, UCON64_XBOX, xbox_usage},
    {"CD32", custom_stristr, UCON64_CD32, cd32_usage},
    {"Vectrex", custom_stristr, UCON64_VECTREX, vectrex_usage},
    {"swan", custom_stristr, UCON64_WONDERSWAN, swan_usage},
    {"Coleco", custom_stristr, UCON64_COLECO, coleco_usage},
    {"Intelli", custom_stristr, UCON64_INTELLI, intelli_usage},
/* TODO:
    {"psx", custom_stristr, UCON64_PSX, psx_usage},
    {"ps1", custom_stristr, UCON64_PSX, psx_usage},
    {"psone", custom_stristr, UCON64_PSX, psx_usage},
    {"ps2", custom_stristr, UCON64_PS2, ps2_usage},
    {"dc", custom_stristr, UCON64_DC, dc_usage},
    {"system", custom_stristr, UCON64_SYSTEM16, s16_usage},
    {"pocket", custom_stristr, UCON64_NEOGEOPOCKET, ngp_usage},
    {"virtual", custom_stristr, UCON64_VIRTUALBOY, vboy_usage},
    {"", custom_stristr, 0, cd32_usage},
    {"", custom_stristr, 0, cdi_usage},
    {"", custom_stristr, 0, channelf_usage},
    {"", custom_stristr, 0, coleco_usage},
    {"", custom_stristr, 0, gamecom_usage},
    {"", custom_stristr, 0, gc_usage},
    {"", custom_stristr, 0, gp32_usage},
    {"", custom_stristr, 0, intelli_usage},
    {"", custom_stristr, 0, odyssey2_usage},
    {"", custom_stristr, 0, odyssey_usage},
    {"", custom_stristr, 0, real3do_usage},
    {"", custom_stristr, 0, s16_usage},
    {"", custom_stristr, 0, sat_usage},
    {"", custom_stristr, 0, vboy_usage},
    {"", custom_stristr, 0, vc4000_usage},
    {"", custom_stristr, 0, vectrex_usage},
*/
    {0, 0, 0, 0}
  };

  for (pos = 0; console_type[pos].id; pos++)
    {
      if (!console_type[pos].compare (fname, console_type[pos].id))
        {
          dat->console = console_type[pos].console;
          dat->console_usage = (console_type[pos].console_usage);
          break;
        }
    }

  if (console_type[pos].id == 0)
    {
      if (!warning)
        {
          printf ("WARNING: \"%s\" is meant for a console unknown to uCON64\n\n", fname);
          warning = 1;
        }
      dat->console = UCON64_UNKNOWN;
      dat->console_usage = NULL;
    }

  return dat->console;
}


static st_ucon64_dat_t *
line_to_dat (const char *fname, const char *dat_entry, st_ucon64_dat_t *dat)
// parse a dat entry into st_ucon64_dat_t
{
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
    "(JE) Japan & Europe",
    "(JU) Japan & U.S.A.",
    "(JUE) Japan, U.S.A. & Europe",
    "(UE) U.S.A. & Europe",
    NULL
  };
  unsigned char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL }, buf[MAXBUFSIZE],
                *p = NULL;
  uint32_t pos = 0;

  if ((unsigned char) dat_entry[0] != DAT_FIELD_SEPARATOR)
    return NULL;

  strcpy (buf, dat_entry);

  for (pos = 0;
       (dat_field[pos] = strtok (!pos ? buf : NULL, DAT_FIELD_SEPARATOR_S))
       && pos < (MAX_FIELDS_IN_DAT - 1); pos++)
    ;

  memset (dat, 0, sizeof (st_ucon64_dat_t));

  strcpy (dat->datfile, basename (fname));

  if (dat_field[3])
    strcpy (dat->name, dat_field[3]);

  if (dat_field[4])
    strcpy (dat->fname, dat_field[4]);

  if (dat_field[5])
    sscanf (dat_field[5], "%x", &dat->crc32);

  if (dat_field[6][0] == 'N' && dat_field[7][0] == 'O')
    // e.g. GoodSNES bad crc & Nintendo FDS DAT
    sscanf (dat_field[8], "%d", &dat->fsize);
  else
    sscanf (dat_field[6], "%d", &dat->fsize);

  p = dat->name;
  // Often flags contain numbers, so don't search for the closing bracket
  sprintf (buf,
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
  if (buf[0])
    {
      if ((p = strrchr (buf, ',')))
        *p = 0;
      sprintf (dat->misc, "Flags: %s", buf);
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

  fname_to_console (dat->datfile, dat);
  dat->copier_usage = unknown_usage;

  return dat;
}


uint32_t
line_to_crc (const char *dat_entry)
// get crc32 of current line
{
  unsigned char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL };
  uint32_t pos = 0, crc32 = 0;
  char buf[MAXBUFSIZE];

  if ((unsigned char) dat_entry[0] != DAT_FIELD_SEPARATOR)
    return 0;

  strcpy (buf, dat_entry);

  for (pos = 0;
       (dat_field[pos] = strtok (!pos ? buf : NULL, DAT_FIELD_SEPARATOR_S))
       && pos < (MAX_FIELDS_IN_DAT - 1); pos++)
    ;

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
ucon64_dat_view (int console, int verbose)
{
  char fname_dat[FILENAME_MAX], fname_index[FILENAME_MAX], *fname, *p;
  static st_ucon64_dat_t dat;
  uint32_t n_entries, n_entries_sum = 0, n_datfiles = 0;
  int n, fsize;
  st_idx_entry_t *idx_entry;

  while (get_next_file (fname_dat))
    {
      fname = basename (fname_dat);
      if (console != UCON64_UNKNOWN)
        if (fname_to_console (fname, &dat) != console)
          continue;

      get_dat_header (fname_dat, &dat);
      strcpy (fname_index, fname_dat);
      set_suffix (fname_index, ".idx");

      fsize = q_fsize (fname_index);
      n_entries = fsize / sizeof (st_idx_entry_t);
      n_entries_sum += n_entries;
      n_datfiles++;

      printf ("DAT info:\n"
        "  %s\n"
//        "  Console: %s\n"
        "  Version: %s (%s, %s)\n"
        "  Author: %s\n"
        "  Comment: %s\n"
        "  Entries: %d\n\n",
        fname,
//        dat.console_usage[0],
        dat.version,
        dat.date,
        dat.refname,
        dat.author,
        dat.comment,
        n_entries);

      if (!(p = (unsigned char *) malloc (fsize)))
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], fsize);
          continue;
        }

      if (q_fread (p, 0, fsize, fname_index) != fsize)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], fname_index);
          free (p);
          continue;
        }

      if (verbose)
        {
          // display all DAT entries
          for (n = 0; n < n_entries; n++)
            {
              idx_entry = &((st_idx_entry_t *) p)[n];
              printf ("Checksum (CRC32): 0x%08x\n", idx_entry->crc32);
              if (get_dat_entry (fname_dat, &dat, idx_entry->crc32, idx_entry->filepos))
                ucon64_dat_nfo (&dat, 0);
              puts ("");
            }
          fclose_fdat ();
        }
      free (p);
    }

  printf ("DAT files: %d; entries: %d; total entries: %d\n",
    n_datfiles, n_entries_sum, ucon64_dat_total_entries (UCON64_UNKNOWN));

  return 0;
}


unsigned int
ucon64_dat_total_entries (int console)
{
  uint32_t entries = 0;
  int fsize;
  char fname[FILENAME_MAX];

  if (!ucon64.dat_enabled)
    return 0;

  while (get_next_file (fname))
    {
      set_suffix (fname, ".idx");
      fsize = q_fsize (fname);
      entries += (fsize < 0 ? 0 : fsize / sizeof (st_idx_entry_t));
    }

  return entries;
}


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


st_ucon64_dat_t *
ucon64_dat_search (uint32_t crc32, st_ucon64_dat_t *dat)
{
  char fname_dat[FILENAME_MAX], fname_index[FILENAME_MAX], *fname;
  unsigned char *p = NULL;
  uint32_t fsize = 0;
  st_idx_entry_t *idx_entry, key;

  if (!crc32)
    return NULL;

  while (get_next_file (fname_dat))
    {
      fname = basename (fname_dat);
      if (ucon64.console != UCON64_UNKNOWN)
        if (fname_to_console (fname, dat) != ucon64.console)
          continue;

      strcpy (fname_index, fname_dat);
      set_suffix (fname_index, ".idx");
      fsize = q_fsize (fname_index);

      if (!(p = (unsigned char *) malloc (fsize)))
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], fsize);
          closedir_ddat ();
          return NULL;
        }

      // load the index for the current dat file
      if (q_fread (p, 0, fsize, fname_index) != fsize)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], fname_index);
          closedir_ddat ();
          free (p);
          return NULL;
        }

      // search index for crc
      key.crc32 = crc32;
      idx_entry = bsearch (&key, p, fsize / sizeof (st_idx_entry_t),
                           sizeof (st_idx_entry_t), idx_compare);
      if (idx_entry)                            // crc32 found
        {
          // open dat file and read entry
          if (get_dat_entry (fname_dat, dat, crc32, idx_entry->filepos))
            if (crc32 == dat->crc32)
              {
                strcpy (dat->datfile, basename (fname_dat));
                get_dat_header (fname_dat, dat);
                closedir_ddat ();
                fclose_fdat ();
                free (p);
                return dat;
              }
          fclose_fdat ();
        }
      free (p);
    }
  return NULL;
}


#define MAX_GAMES_FOR_CONSOLE 50000             // TODO?: dynamic size
int
ucon64_dat_indexer (void)
// create or update index of DAT file
{
  char fname_dat[FILENAME_MAX], fname_index[FILENAME_MAX], errorfname[FILENAME_MAX];
  struct stat fstate_dat, fstate_index;
  st_ucon64_dat_t dat;
  FILE *errorfile;
  uint32_t size = 0, pos;
  time_t start_time = 0;
  int update = 0, n_duplicates, n;
  st_idx_entry_t *idx_entries, *idx_entry;

  warning = 0; // enable warning again for DATs with unrecognized console systems

  if (!(idx_entries = (st_idx_entry_t *)
          malloc (MAX_GAMES_FOR_CONSOLE * sizeof (st_idx_entry_t))))
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR],
        MAX_GAMES_FOR_CONSOLE * sizeof (st_idx_entry_t));
      exit (1);
    }

  while (get_next_file (fname_dat))
    {
      strcpy (fname_index, fname_dat);
      set_suffix (fname_index, ".idx");

      if (!stat (fname_dat, &fstate_dat) && !stat (fname_index, &fstate_index))
        {
          if (fstate_dat.st_mtime < fstate_index.st_mtime)
            continue;                   // index file seems to be present and up-to-date
          update = 1;
        }

      start_time = time (0);
      size = q_fsize (fname_dat);

#if 0 // PR decision
      printf ("%s: %s\n", "Create", basename (fname_index));
#else
      printf ("%s: %s\n", (update ? "Update" : "Create"), basename (fname_index));
#endif
      pos = 0;
      n_duplicates = 0;
      errorfile = NULL;
      while (get_dat_entry (fname_dat, &dat, 0, -1))
        {
          if (pos == MAX_GAMES_FOR_CONSOLE)
            {
              fprintf (stderr,
                       "\n"
                       "INTERNAL ERROR: MAX_GAMES_FOR_CONSOLE is too small (%d)\n",
                       MAX_GAMES_FOR_CONSOLE);
              break;
            }

          /*
            Doing a linear search removes the need of using the slow qsort()
            function inside the loop. Doing a binary search doesn't improve the
            speed much, but is much more efficient of course. Using qsort()
            inside the loop slows it down with a factor greater than 10.
          */
          idx_entry = NULL;
          for (n = 0; n < pos; n++)
            if (idx_entries[n].crc32 == dat.crc32)
              idx_entry = &idx_entries[n];
          if (idx_entry)
            {
              // This really makes one loose trust in the DAT files...
              char current_name[2 * 80];
              long current_filepos = ftell (fdat);

              if (!errorfile)
                {
                  strcpy (errorfname, fname_index);
                  set_suffix (errorfname, ".err");
                  if (!(errorfile = fopen (errorfname, "w"))) // text file for WinDOS
                    {
                      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], errorfname);
                      continue;
                    }
                }

              strcpy (current_name, dat.name);
              get_dat_entry (fname_dat, &dat, 0, idx_entry->filepos);
              fprintf (errorfile,
                       "\n"
                       "WARNING: DAT file contains a duplicate CRC32 (0x%x)!\n"
                       "  First game with this CRC32: \"%s\"\n"
                       "  Ignoring game:              \"%s\"\n",
                       dat.crc32, dat.name, current_name);

              n_duplicates++;
              fseek (fdat, current_filepos, SEEK_SET);
              continue;
            }

          idx_entries[pos].crc32 = dat.crc32;
          idx_entries[pos].filepos = filepos_line;

          if (!(pos % 20))
            ucon64_gauge (start_time, ftell (fdat), size);
          pos++;
        }
      fclose_fdat ();

      qsort (idx_entries, pos, sizeof (st_idx_entry_t), idx_compare);
      if (q_fwrite (idx_entries, 0, pos * sizeof (st_idx_entry_t), fname_index, "wb")
          != pos * sizeof (st_idx_entry_t))
        {
          fputc ('\n', stderr);
          fprintf (stderr, ucon64_msg[WRITE_ERROR], fname_index);
        }

      ucon64_gauge (start_time, size, size);
      if (n_duplicates > 0)
        printf ("\n"
                "\n"
                "WARNING: DAT file contains %d duplicate CRC32%s\n"
                "         Warnings have been written to \"%s\"",
                n_duplicates, n_duplicates != 1 ? "s" : "", errorfname);
      if (errorfile)
        {
          fclose (errorfile);
          errorfile = NULL;
        }
      printf ("\n\n");
    }
// stats
  free (idx_entries);

  return 0;
}


void
ucon64_dat_nfo (const st_ucon64_dat_t *dat, int display_version)
{
  char buf[MAXBUFSIZE], *p = NULL;
  int n;

  if (!dat)
    {
      printf (ucon64_msg[DAT_NOT_FOUND], ucon64.crc32);
      return;
    }

  printf ("DAT info:\n");
  // console type?
  if (dat->console_usage != NULL)
    {
      strcpy (buf, dat->console_usage[0].desc);
      printf ("  %s\n", to_func (buf, strlen (buf), toprint2));

#if 0
      if (dat->console_usage[1].desc)
        {
          strcpy (buf, dat->console_usage[1]->desc);
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
    really differs from the canonical game name (usually file name without
    suffix).
  */
  n = strlen (dat->fname);
  p = (char *) get_suffix (dat->fname);
  if (stricmp (p, ".nes") &&                    // NES
      stricmp (p, ".fds") &&                    // NES FDS
      stricmp (p, ".gb") &&                     // Game Boy
      stricmp (p, ".gbc") &&                    // Game Boy Color
      stricmp (p, ".gba") &&                    // Game Boy Advance
      stricmp (p, ".smc") &&                    // SNES
//    stricmp (p, ".smd") &&                    // Genesis
      stricmp (p, ".v64"))                      // Nintendo 64
    {
      if (stricmp (dat->name, dat->fname) != 0)
        printf ("  Filename: %s\n", dat->fname);
    }
  else
    {
      n -= strlen (p);
      if (strnicmp (dat->name, dat->fname, n) != 0)
        printf ("  Filename: %s\n", dat->fname);
    }

  printf ("  %d Bytes (%.4f Mb)\n",
          dat->fsize,
          TOMBIT_F (dat->fsize));

  if (dat->misc[0])
    printf ("  %s\n", dat->misc);

  if (display_version)
    {
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
}
