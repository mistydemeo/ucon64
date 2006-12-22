/*
dat.c - DAT files support with index files for faster access

Copyright (c) 1999 - 2006 NoisyB


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
#include <sys/stat.h>
#include <time.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/itypes.h"
#include "misc/property.h"
#include "misc/file.h"
#ifdef  _WIN32
#include "misc/win32.h"
#endif
#include "dat.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif
#define MAXBUFSIZE 32768


typedef struct
{
  uint32_t crc32;
  long filepos;
} st_idx_entry_t;


static const char *dat_country[28][2] =
  {
    {"(FC)", "French Canada"},
    {"(FN)", "Finland"},
    {"(G)", "Germany"},
    {"(GR)", "Greece"},
    {"(H)", "Holland"},               // other (incorrect) name for The Netherlands
    {"(HK)", "Hong Kong"},
    {"(I)", "Italy"},
    {"(J)", "Japan"},
    {"(JE)", "Japan & Europe"},
    {"(JU)", "Japan & U.S.A."},
    {"(JUE)", "Japan, U.S.A. & Europe"},
    {"(K)", "Korea"},
    {"(NL)", "The Netherlands"},
    {"(PD)", "Public Domain"},
    {"(S)", "Spain"},
    {"(SW)", "Sweden"},
    {"(U)", "U.S.A."},
    {"(UE)", "U.S.A. & Europe"},
    {"(UK)", "England"},
    {"(Unk)", "Unknown country"},
    /*
      At least (A), (B), (C), (E) and (F) have to come after the other
      countries, because some games have (A), (B) etc. in their name (the
      non-country part). For example, the SNES games
      "SD Gundam Generations (A) 1 Nen Sensouki (J) (ST)" or
      "SD Gundam Generations (B) Guripus Senki (J) (ST)".
    */
    {"(1)", "Japan & Korea"},
    {"(4)", "U.S.A. & Brazil NTSC"},
    {"(A)", "Australia"},
    {"(B)", "non U.S.A. (Genesis)"},
    {"(C)", "China"},
    {"(E)", "Europe"},
    {"(F)", "France"},
    {NULL, NULL}
  };
static const char *dat_flags[][2] =
  {
    // Often flags contain numbers, so don't search for the closing bracket
    {"[a", "Alternate"},
    {"[p", "Pirate"},
    {"[b", "Bad dump"},
    {"[t", "Trained"},
    {"[f", "Fixed"},
    {"[T", "Translation"},
    {"[h", "Hack"},
    {"[x", "Bad checksum"},
    {"[o", "Overdump"},
    {"[!]", "Verified good dump"}, // [!] is ok
    {NULL, NULL}
  };


static int
dat_log (FILE *log, const char *s)
{
  char buf[32];
  time_t t = time (0);

  strftime (buf, 32, "%b %d %H:%M:%S", localtime (&t));

  fprintf (log, "%s rsstool(%d): ", buf, getpid());
  fputs (s, log);
  fputc ('\n', log);
  fflush (log);

  return 0;
}


static const char *
dat_get_idx_name (const char *dat_fname)
{
  static char buf[FILENAME_MAX];
  strncpy (buf, dat_fname, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
  set_suffix (buf, ".idx");
  return buf;
}


static const char *
dat_get_log_name (const char *dat_fname)
{
  static char buf[FILENAME_MAX];
  strncpy (buf, dat_fname, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
  set_suffix (buf, ".log");
  return buf;
}


static st_dat_entry_t *
dat_get_next_entry (FILE *d)
{
  uint32_t pos = 0;
  int x = 0;
  static st_dat_entry_t dat_entry;
  char line[MAXBUFSIZE];
  char buf[MAXBUFSIZE], *p = NULL;

#define MAX_FIELDS_IN_DAT 32
#define DAT_FIELD_SEPARATOR (0xac)
#define DAT_FIELD_SEPARATOR_S "\xac"

  char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL };

#if 0
  if (fgets (line, MAXBUFSIZE, d))
    {
      if ((unsigned char) *line != DAT_FIELD_SEPARATOR)
        return NULL;

      strncpy (buf, line, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;

  strarg (dat_field, buf, DAT_FIELD_SEPARATOR_S, MAX_FIELDS_IN_DAT);

  memset (&dat, 0, sizeof (st_ucon64_dat_t));

  strcpy (dat->datfile, basename2 (fname));

  if (dat_field[3])
    strcpy (dat->name, dat_field[3]);

  if (dat_field[4])
    strcpy (dat->fname, dat_field[4]);

  if (dat_field[5])
    sscanf (dat_field[5], "%x", (unsigned int *) &dat->crc32);

  if (dat_field[6][0] == 'N' && dat_field[7][0] == 'O')
    // e.g. GoodSNES bad crc & Nintendo FDS DAT
    sscanf (dat_field[8], "%d", (int *) &dat->fsize);
  else
    sscanf (dat_field[6], "%d", (int *) &dat->fsize);

  *buf = 0;
  for (x = 0, p = buf; dat_flags[x][0]; x++, p += strlen (p))
    if (strstr (dat->name, dat_flags[x][0]))
      sprintf (p, "%s, ", dat_flags[x][1]);
  if (buf[0])
    {
      if ((p = strrchr (buf, ',')))
        *p = 0;
      sprintf (dat->misc, "Flags: %s", buf);
    }

  p = dat->name;
  dat->country = NULL;
  for (pos = 0; dat_country[pos][0]; pos++)
    if (stristr (p, dat_country[pos][0]))
      {
        dat->country = dat_country[pos][1];
        break;
      }

  fname_to_console (dat->datfile, dat);
  dat->backup_usage = unknown_backup_usage[0].help;

  return dat;

    }
#endif

  return NULL;
}


static int
dat_index (const char *dat_fname)
{
  char idx_fname[FILENAME_MAX];
  char log_fname[FILENAME_MAX];
  int entries = 0;
  char buf[MAXBUFSIZE];
  st_idx_entry_t idx_entry;
  struct stat dat_state, idx_state;
  FILE *d = NULL, *idx = NULL, *log = NULL;

  strncpy (idx_fname, dat_get_idx_name (dat_fname), FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  if (!stat (dat_fname, &dat_state) && !stat (idx_fname, &idx_state))
    if (dat_state.st_mtime < idx_state.st_mtime)
      return 0;                   // index is present and up-to-date

  if (!(d = fopen (dat_fname, "rb")))
    return -1;

  strncpy (log_fname, dat_get_log_name (dat_fname), FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  if (!(idx = fopen (idx_fname, "w")))
    {
      fclose (d);
      return -1;
    }

  if (!(log = fopen (log_fname, "a")))
    {
      fclose (d);
      fclose (idx);
      return -1;
    }

  idx_entry.filepos = ftell (d);
  while ((fgets (buf, MAXBUFSIZE, d)))
    {
      st_dat_entry_t *dat_entry = dat_get_next_entry (d);

      if (dat_entry)
        {
          idx_entry.crc32 = dat_entry->crc32;
          fwrite (&idx_entry, 1, sizeof (st_idx_entry_t), idx);
        }

      idx_entry.filepos = ftell (d);
      entries++;
    }

  sprintf (buf, "%d entries found in %s", entries, dat_fname);
  dat_log (log, buf);

  fclose (d);
  fclose (idx);
  fclose (log);

  return 0;
}


st_dat_t *
dat_open (const char *dat_fname, int flags)
{
  static st_dat_t dat;
  const char *p = NULL;

  if (flags & DAT_FLAG_IDX)
    dat_index (dat_fname);

  strncpy (dat.dat_fname, dat_fname, FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  // read dat header
  if ((p = get_property (dat_fname, "author", PROPERTY_MODE_TEXT)))
    strncpy (dat.author, p, DAT_MAXBUFLEN)[DAT_MAXBUFLEN - 1] = 0;
  else
    strcpy (dat.author, "Unknown");

  if ((p = get_property (dat_fname, "version", PROPERTY_MODE_TEXT)))
    strncpy (dat.version, p, DAT_MAXBUFLEN)[DAT_MAXBUFLEN - 1] = 0;
  else
    strcpy (dat.version, "?");

  if ((p = get_property (dat_fname, "refname", PROPERTY_MODE_TEXT)))
    strncpy (dat.refname, p, DAT_MAXBUFLEN)[DAT_MAXBUFLEN - 1] = 0;
  else
    *(dat.refname) = 0;

  if ((p = get_property (dat_fname, "comment", PROPERTY_MODE_TEXT)))
    strncpy (dat.comment, p, DAT_MAXBUFLEN)[DAT_MAXBUFLEN - 1] = 0;
  else
    *(dat.comment) = 0;

  if ((p = get_property (dat_fname, "date", PROPERTY_MODE_TEXT)))
    strncpy (dat.date, p, DAT_MAXBUFLEN)[DAT_MAXBUFLEN - 1] = 0;
  else
    strcpy (dat.date, "?");

  return &dat;
}


#if 0
st_dat_t *
dat_create (const char *dat_fname,
            const char *author,
            const char *version,
            const char *refname,
            const char *comment,
            const char *date)
{
  return 0;
}
#endif


st_dat_entry_t *
dat_read (st_dat_t *dat, uint32_t crc32)
{
  st_dat_entry_t *dat_entry = NULL;
  char idx_fname[FILENAME_MAX];
  st_idx_entry_t idx_entry;
  FILE *idx = NULL, *d = NULL;

  strncpy (idx_fname, dat_get_idx_name (dat->dat_fname), FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  if (!(idx = fopen (idx_fname, "rb")))
    return NULL;

  if (!(d = fopen (dat->dat_fname, "rb")))
    {
      fclose (idx);
      return NULL;
    }

  while ((fread (&idx_entry, 1, sizeof (st_idx_entry_t), idx)))
    if (idx_entry.crc32 == crc32)
      break;

  fclose (idx);


  fseek (d, idx_entry.filepos, SEEK_SET);
  dat_entry = dat_get_next_entry (d);

  fclose (d);

  return dat_entry;
}


#if 0
int
dat_write (st_dat_t *dat, uint32_t crc32,
                          const char *name,
                          const char *misc,
                          const char *rom_name,
                          uint32_t rom_size)
{
  return 0;
}
#endif


int
dat_close (st_dat_t *dat)
{
  dat = NULL;

  return 0;
}


#ifdef  TEST
int
main (int argc, char **argv)
{
  st_dat_t *dat = dat_open (argv[1], DAT_FLAG_IDX);

  if (dat)
    {
      st_dat_entry_t *dat_entry = dat_read (dat, 0x12345678);
      dat_close (dat);
    }
 
  return 0;
}
#endif
