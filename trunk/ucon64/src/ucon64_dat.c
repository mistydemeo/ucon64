/*
ucon64_dat.c - support for DAT files as known from Romcenter, Goodxxx, etc.

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
#include "config.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "misc.h"
#include "quick_io.h"

#define MAX_FIELDS_IN_DAT 32
#define DAT_FIELD_SEPARATOR (0xac)
#define DAT_FIELD_SEPARATOR_S ("\xac")


static DIR *dptr = NULL;
static FILE *fdat = NULL;

static void
closedir_dptr (void)
{
  if (dptr) closedir (dptr);
  dptr = NULL;
}


static void
fclose_fdat (void)
{
  if (fdat) fclose (fdat);
  fdat = NULL;
}


static uint32_t
get_uint32 (const void *buf)
{
  uint32_t      ret;
  unsigned char *tmp;
    
  tmp = (unsigned char *) buf;
      
  ret = tmp[3] & 0xff;
  ret = (ret << 8) + (tmp[2] & 0xff);
  ret = (ret << 8) + (tmp[1] & 0xff);
  ret = (ret << 8) + (tmp[0] & 0xff);
              
  return ret;
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


static ucon64_dat_t *
get_dat_header (char *fname, ucon64_dat_t * dat)
{
#if     (MAXBUFSIZE < FILENAME_MAX)
  char buf[FILENAME_MAX];
#else
  char buf[MAXBUFSIZE];
#endif

//hell yes!!! i use get_property() here... 

  strncpy (dat->author, get_property (fname, "author", buf, "Unknown"), sizeof (dat->author));
  dat->author[sizeof (dat->author)] = 0;
  strncpy (dat->version, get_property (fname, "version", buf, "?"), sizeof (dat->version));
  dat->version[sizeof (dat->version)] = 0;
  strncpy (dat->refname, get_property (fname, "refname", buf, ""), sizeof (dat->refname));
  dat->refname[sizeof (dat->refname)] = 0;

  strcpy (dat->comment, get_property (fname, "comment", buf, ""));

  strncpy (dat->date, get_property (fname, "date", buf, "?"), sizeof (dat->date));
  dat->date[sizeof (dat->date)] = 0;

  return dat;
}


static ucon64_dat_t *
line_to_dat (const char *dat_entry, ucon64_dat_t * dat)
{
// parse a dat entry into ucon64_dat_t
  static const char *dat_country[] = {
    "(1) Japan & Korea",
    "(A) Australia",
    "(B) non USA (Genesis)",
    "(C) China",
    "(E) Europe",
    "(F) France",
    "(FC) French Canadian",
    "(FN) Finland",
    "(G) Germany",
    "(GR) Greece",
    "(HK) Hong Kong",
    "(4) USA & Brazil NTSC",
    "(J) Japan",
    "(K) Korea",
    "(NL) Netherlands",
    "(PD) Public Domain",
    "(S) Spain",
    "(SW) Sweden",
    "(U) USA",
    "(UK) England",
    "(Unk) Unknown Country",
    "(I) Italy",
    NULL
  };

#if 0
      fclose_fdat ();
      while (get_next_dat_entry (buf, &dat))
        {
          switch (dat.console)
            {
            case UCON64_GB:
              p = "UCON64_GB";
              break;

            case UCON64_GENESIS:
              p = "UCON64_GENESIS";
              break;

            case UCON64_SMS:
              p = "UCON64_SMS";
              break;

            case UCON64_JAGUAR:
              p = "UCON64_JAGUAR";
              break;

            case UCON64_LYNX:
              p = "UCON64_LYNX";
              break;

            case UCON64_N64:
              p = "UCON64_N64";
              break;

            case UCON64_NEOGEO:
              p = "UCON64_NEOGEO";
              break;

            case UCON64_NES:
              p = "UCON64_NES";
              break;

            case UCON64_PCE:
              p = "UCON64_PCE";
              break;
#if 0
            case UCON64_PSX:
              p = "UCON64_PSX";
              break;

            case UCON64_PS2:
              p = "UCON64_PS2";
              break;

            case UCON64_SATURN:
              p = "UCON64_SATURN";
              break;

            case UCON64_DC:
              p = "UCON64_DC";
              break;

            case UCON64_CD32:
              p = "UCON64_CD32";
              break;

            case UCON64_CDI:
              p = "UCON64_CDI";
              break;

            case UCON64_REAL3DO:
              p = "UCON64_REAL3DO";
              break;
#endif
            case UCON64_SNES:
              p = "UCON64_SNES";
              break;

            case UCON64_ATARI:
              p = "UCON64_ATARI";
              break;

            case UCON64_SYSTEM16:
              p = "UCON64_SYSTEM16";
              break;

            case UCON64_NEOGEOPOCKET:
              p = "UCON64_NEOGEOPOCKET";
              break;

            case UCON64_GBA:
              p = "UCON64_GBA";
              break;

            case UCON64_VECTREX:
              p = "UCON64_VECTREX";
              break;

            case UCON64_VIRTUALBOY:
              p = "UCON64_VIRTUALBOY";
              break;

            case UCON64_WONDERSWAN:
              p = "UCON64_WONDERSWAN";
              break;

            case UCON64_COLECO:
              p = "UCON64_COLECO";
              break;

            case UCON64_INTELLI:
              p = "UCON64_INTELLI";
              break;

            default:
              p = "UCON64_UNKNOWN";
              break;
            }
        }
#endif
  
  unsigned char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL };
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], *p = NULL;
  uint32_t pos = 0;

  if ((unsigned char) dat_entry[0] != DAT_FIELD_SEPARATOR) return NULL;

  strcpy (buf, dat_entry); 
    
  for (pos = 0;
       (dat_field[pos] = strtok (!pos ? buf : NULL, DAT_FIELD_SEPARATOR_S))
       && pos < (MAX_FIELDS_IN_DAT - 1); pos++);

   memset (dat, 0, sizeof (ucon64_dat_t));

  if (dat_field[5])
    sscanf (dat_field[5], "%x", &dat->current_crc32);
      
  dat->console = (uint8_t) ucon64.console;  // important

  if (dat_field[3])
    strcpy (dat->name, dat_field[3]);

  if (dat_field[6])
    sscanf (dat_field[6], "%d", &dat->fsize);

  if (dat_field[4])
    strcpy (dat->fname, dat_field[4]);

  p = dat->name;
  
  sprintf (buf2, 
    "%s%s%s%s%s%s%s%s%s%s",
    (strstr (p, "[a]") ? "Alternate, " : ""),
    (strstr (p, "[p]") ? "Pirate, " : ""),
    (strstr (p, "[b]") ? "Bad Dump, " : ""),
    (strstr (p, "[t]") ? "Trained, " : ""),
    (strstr (p, "[f]") ? "Fixed, " : ""),
    (strstr (p, "[T]") ? "Translation, " : ""),
    (strstr (p, "[h]") ? "Hack, " : ""),
    (strstr (p, "[x]") ? "Bad Checksum, " : ""),
    (strstr (p, "[o]") ? "Overdump, " : ""),
    (strstr (p, "[!]") ? "Verified Good Dump, " : ""));
  if (buf2[0])
    sprintf (dat->misc, "Flags: %s", buf2);

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

  return dat;
}


uint32_t
line_to_crc (const char *dat_entry)
{
// get crc32 of current line
  unsigned char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL };
  uint32_t pos = 0, crc32 = 0;
  char buf[MAXBUFSIZE];

  if ((unsigned char) dat_entry[0] != DAT_FIELD_SEPARATOR) return 0;

  strcpy (buf, dat_entry); 
    
  for (pos = 0;
       (dat_field[pos] = strtok (!pos ? buf : NULL, DAT_FIELD_SEPARATOR_S))
       && pos < (MAX_FIELDS_IN_DAT - 1); pos++);

  if (dat_field[5])
    sscanf (dat_field[5], "%x", &crc32);

  return crc32;
}


static ucon64_dat_t *
get_dat_entry (char *fname, ucon64_dat_t * dat, uint32_t crc32)
{
  char buf[MAXBUFSIZE];

  if (!fdat)
    if (!(fdat = fopen (fname, "r")))
      {
        fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], fname);
        return NULL;
      }

  while (fgets (buf, MAXBUFSIZE, fdat) != NULL)
    if ((unsigned char) buf[0] == DAT_FIELD_SEPARATOR)
      if (!crc32 || line_to_crc (buf) == crc32)
        if (line_to_dat (buf, dat)) return dat;

  fclose_fdat ();
  return NULL;
}


int
ucon64_dat_view (int console)
{
//  char *p = "";
  char buf[FILENAME_MAX], buf2[FILENAME_MAX];
  static ucon64_dat_t dat;
  uint32_t entries, dat_counter = 0;

  closedir_dptr ();
  while (get_next_file (buf))
    {
      get_dat_header (buf, &dat);
      strcpy (buf2, buf);
      setext (buf2, ".idx");
      entries = q_fsize (buf2) / sizeof (uint32_t);
      dat_counter++;

      printf ("DAT info:\n"
        "  %s\n"
        "  Version: %s (%s, %s)\n"
        "  Author: %s\n"
        "  Comment: %s\n"
        "  Entries: %d\n",
        basename2 (buf),
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
      entries += (fsize < 0 ? 0 : fsize);        // TODO: handle this case gracefully
    }

  return (entries / sizeof (uint32_t));
}


ucon64_dat_t *
ucon64_dat_search (uint32_t crc32, ucon64_dat_t * dat)
{
  uint32_t pos = 0;
  char buf[FILENAME_MAX];
  unsigned char *p = NULL;
  uint32_t fsize = 0;
  FILE *fh = NULL;

  closedir_dptr ();
  while (get_next_file (buf) != NULL)
    {
// load the index for the current dat file
      setext (buf, ".idx");

      if (!(fh = fopen (buf, "r")))
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
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], buf);
          fclose (fh);
          free (p);
          return NULL;
        }

      fclose (fh);

// search index for crc
      for (pos = 0; pos < fsize; pos += sizeof (uint32_t))
        if (get_uint32 (&p[pos]) == crc32) // crc32 found
          {
// open dat file and read entry 
            free (p);
            setext (buf, ".dat");

            fclose_fdat ();
            while (get_dat_entry (buf, dat, crc32))
              if (crc32 == dat->current_crc32)
                {
                  strcpy (dat->datfile, basename2 (buf));
                  get_dat_header (buf, dat);
                  return dat;
                }
          }
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
  ucon64_dat_t dat;
  FILE *fh = NULL;
  uint32_t size = 0, pos = 0;
  time_t start_time = 0;
  int update = 0;

  closedir_dptr ();
  while (get_next_file (buf))
    {
      strcpy (buf2, buf);
      setext (buf2, ".idx");

      if (!stat (buf, &cache) && !stat (buf2, &index))
        {
          if (cache.st_mtime < index.st_mtime)
            {
//index file seems to be present and up-to-date
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
     while (get_dat_entry (buf, &dat, 0))
       {
         fwrite (&dat.current_crc32, sizeof (uint32_t), 1, fh);
         if (!(pos % 20)) ucon64_gauge (start_time, ftell (fdat), size);
         pos++;
       }
     fprintf (stdout, "\n\n");
     fclose (fh);
   }
// stats

  return 0;
}


void
ucon64_dat_nfo (const ucon64_dat_t *dat)
{
   if (!dat)
     {
       fprintf (stdout, "DAT info: ROM not found\n");
       return;
     }

  printf ("DAT info:\n" "  %s\n", dat->name);

  if (dat->misc[0])
    printf ("  %s\n", dat->misc);

  if (dat->country)
    printf ("  Country: %s\n", dat->country);

  if (stricmp (dat->name, dat->fname) != 0)
    printf ("  Filename: %s\n", dat->fname);

  printf ("  Size: %d Bytes (%.4f Mb)\n"
          "  %s\n"
          "  Version: %s (%s, %s)\n",
          dat->fsize,
          TOMBIT_F (dat->fsize),
          dat->datfile,
          dat->version,
          dat->date,
          dat->refname);

}
