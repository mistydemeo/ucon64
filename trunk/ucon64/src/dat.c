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
#include "misc/string.h"
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


// romcenter defaults
#define MAX_FIELDS_IN_DAT 32
#define DAT_FIELD_SEPARATOR (0xac)
#define DAT_FIELD_SEPARATOR_S "\xac"


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


static st_dat_entry_t *
dat_parse_line (const char *line)
{
  uint32_t pos = 0;
  int x = 0;
  static st_dat_entry_t dat_entry;
  char buf[MAXBUFSIZE], *p = NULL;
  char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL };

  if (!line)
    return NULL;

  if (!(*line))
    return NULL;

  if ((unsigned char) *line != DAT_FIELD_SEPARATOR)
    return NULL;

  memset (&dat_entry, 0, sizeof (st_dat_entry_t));

  strncpy (buf, line, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;

  strarg (dat_field, buf, DAT_FIELD_SEPARATOR_S, MAX_FIELDS_IN_DAT);

  if (dat_field[3])
    strcpy (dat_entry.name, dat_field[3]);

  if (dat_field[4])
    strcpy (dat_entry.rom_name, dat_field[4]);

  if (dat_field[5])
    sscanf (dat_field[5], "%x", (unsigned int *) &dat_entry.crc32);

  if (dat_field[6][0] == 'N' && dat_field[7][0] == 'O')
    // e.g. GoodSNES bad crc & Nintendo FDS DAT
    sscanf (dat_field[8], "%d", (int *) &dat_entry.rom_size);
  else
    sscanf (dat_field[6], "%d", (int *) &dat_entry.rom_size);

  *buf = 0;
  for (x = 0, p = buf; dat_flags[x][0]; x++, p += strlen (p))
    if (strstr (dat_entry.name, dat_flags[x][0]))
      sprintf (p, "%s, ", dat_flags[x][1]);
  if (buf[0])
    {
      if ((p = strrchr (buf, ',')))
        *p = 0;
      sprintf (dat_entry.misc, "Flags: %s", buf);
    }

  p = dat_entry.name;
  dat_entry.country = NULL;
  for (pos = 0; dat_country[pos][0]; pos++)
    if (stristr (p, dat_country[pos][0]))
      {
        dat_entry.country = dat_country[pos][1];
        break;
      }

  return &dat_entry;
}


static int
dat_create_index (const char *dat_fname)
{
  char idx_fname[FILENAME_MAX];
  char buf[MAXBUFSIZE];
  st_idx_entry_t idx_entry;
  st_dat_entry_t *dat_entry = NULL;
  struct stat dat_state, idx_state;
  FILE *d = NULL, *idx = NULL;

  strncpy (idx_fname, dat_fname, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
  set_suffix (idx_fname, ".idx");

  if (!stat (dat_fname, &dat_state) && !stat (idx_fname, &idx_state))
    if (dat_state.st_mtime < idx_state.st_mtime)
      return 0;                   // index is present and up-to-date

  if (!(d = fopen (dat_fname, "rb")))
    return -1;

  if (!(idx = fopen (idx_fname, "w")))
    {
      fclose (d);
      return -1;
    }

  while ((fgets (buf, MAXBUFSIZE, d)))
    {
      idx_entry.filepos = ftell (d);

      if ((dat_entry = dat_parse_line (buf)))
        {
          idx_entry.crc32 = dat_entry->crc32;
          fwrite (&idx_entry, 1, sizeof (st_idx_entry_t), idx);
        }
    }

  fclose (d);
  fclose (idx);

  return 0;
}


int
dat_create (const char *dat_fname,
            const char *author,
            const char *email,
            const char *homepage,
            const char *url,
            const char *version,
            const char *comment,
            const char *plugin,
            const char *refname)
{
//  char fname[FILENAME_MAX], *ptr;
  time_t time_t_val;
  struct tm *t;
  FILE *fh = NULL;

  if (!(fh = fopen (dat_fname, "w")))
    return -1;

  time_t_val = time (NULL);
  t = localtime (&time_t_val);

  // RomCenter uses files in DOS text format, so we generate a file in that format
  fprintf (fh, "[CREDITS]\r\n"
               "author=%s\r\n"
               "email=%s\r\n"
               "homepage=%s\r\n"
               "url=%s\r\n"
               "version=%s\r\n"
               "date=%d/%d/%d\r\n"
               "comment=%s\r\n"
               "[DAT]\r\n"
               "version=2.50\r\n" // required by RomCenter!
               "plugin=%s\r\n"
               "[EMULATOR]\r\n"
               "refname=%s\r\n"
               "version=\r\n"
               "[GAMES]\r\n",
               author,
               email,
               homepage,
               url,
               version,
               t->tm_mday,
               t->tm_mon + 1,
               t->tm_year + 1900,
               comment,
               plugin,
               refname);

  fclose (fh);

  return 0;
}


st_dat_t *
dat_open (const char *dat_fname)
{
  static st_dat_t dat;
  const char *p = NULL;

  // create index file if necessary
  dat_create_index (dat_fname);

  strncpy (dat.fname, dat_fname, FILENAME_MAX)[FILENAME_MAX - 1] = 0;

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


int
dat_close (st_dat_t *dat)
{
  dat = NULL;

  return 0;
}


const st_dat_entry_t *
dat_read (st_dat_t *dat, uint32_t crc32)
{
  st_dat_entry_t *dat_entry = NULL;
  st_idx_entry_t idx_entry;
  char idx_fname[FILENAME_MAX];
  char buf[MAXBUFSIZE];
  FILE *idx = NULL, *d = NULL;

  strncpy (idx_fname, dat->fname, FILENAME_MAX)[FILENAME_MAX - 1] = 0;                   
  set_suffix (idx_fname, ".idx");

  if (!(idx = fopen (idx_fname, "rb")))
    return NULL;

  while ((fread (&idx_entry, 1, sizeof (st_idx_entry_t), idx)))
    if (idx_entry.crc32 == crc32)
      {
        if (!(d = fopen (dat->fname, "r")))
          {
            fclose (idx);
            return NULL;
          }

        fseek (d, idx_entry.filepos, SEEK_SET);
        if ((fgets (buf, MAXBUFSIZE, d)))
          dat_entry = dat_parse_line (buf);

        fclose (d);

        break;
      }

  fclose (idx);

  return dat_entry;
}


int
dat_write (st_dat_t *dat, uint32_t crc32,
//                          const char *name,
//                          const char *misc,
                          const char *rom_name,
                          uint32_t rom_size)
{
  FILE *fh = NULL;

  if (!(fh = fopen (dat->fname, "a")))
    {
      fprintf (stderr, "ERROR: %s does not exist\n", dat->fname);
      return -1;
    }

  fseek (fh, 0, SEEK_END);
  fprintf (fh, DAT_FIELD_SEPARATOR_S "%s" // set file name
               DAT_FIELD_SEPARATOR_S "%s" // set full name
               DAT_FIELD_SEPARATOR_S "%s" // clone file name
               DAT_FIELD_SEPARATOR_S "%s" // clone full name
               DAT_FIELD_SEPARATOR_S "%s" // rom file name
               DAT_FIELD_SEPARATOR_S "%08x" // RC quirck: leading zeroes are required
               DAT_FIELD_SEPARATOR_S "%d"
               DAT_FIELD_SEPARATOR_S // merged clone name
               DAT_FIELD_SEPARATOR_S // merged rom name
               DAT_FIELD_SEPARATOR_S "\r\n",
               rom_name,
               rom_name,
               rom_name,
               rom_name,
               rom_name,
               crc32,
               rom_size);

  fclose (fh);

  return 0;
}


#ifdef  TEST
int
main (int argc, char **argv)
{
  st_dat_t *dat = dat_open (argv[1]);

  if (dat)
    {
      const st_dat_entry_t *dat_entry = dat_read (dat, 0x12345678);
      dat_close (dat);
    }
 
  return 0;
}
#endif
