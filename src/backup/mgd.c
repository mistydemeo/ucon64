/*
mgd.c - Multi Game Doctor/Hunter support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh


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
#include <ctype.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "mgd.h"


const st_usage_t mgd_usage[] =
  {
    {NULL, NULL, "Multi Game Doctor (2)/Multi Game Hunter/MGH"},
    {NULL, NULL, "19XX Bung Enterprises Ltd http://www.bung.com.hk"},
    {NULL, NULL, "?Makko Toys Co., Ltd.?"},
#if 0
    {"xmgd", "(TODO) send/receive ROM to/from Multi Game* /MGD2/MGH; " OPTION_LONG_S "port=PORT\n"
                "receives automatically when " OPTION_LONG_S "rom does not exist"},
#endif
    {NULL, NULL, NULL}
  };


// the following four functions are used by non-transfer code in genesis.c
void
mgd_interleave (unsigned char **buffer, int size)
{
  int n;
  unsigned char *src = *buffer;

  if (!(*buffer = (unsigned char *) malloc (size)))
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      exit (1);
    }
  for (n = 0; n < size / 2; n++)
    {
      (*buffer)[n] = src[n * 2 + 1];
      (*buffer)[size / 2 + n] = src[n * 2];
    }
  free (src);
}


void
mgd_deinterleave (unsigned char **buffer, int data_size, int buffer_size)
{
  int n = 0, offset;
  unsigned char *src = *buffer;

  if (!(*buffer = (unsigned char *) malloc (buffer_size)))
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], buffer_size);
      exit (1);
    }
  for (offset = 0; offset < data_size / 2; offset++)
    {
      (*buffer)[n++] = src[data_size / 2 + offset];
      (*buffer)[n++] = src[offset];
    }
  free (src);
}


int
fread_mgd (void *buffer, size_t size, size_t number, FILE *fh)
/*
  This function is used to handle a Genesis MGD file as if it wasn't
  interleaved, without the overhead of reading the entire file into memory.
  This is important for genesis_init(). When the file turns out to be a Genesis
  dump in MGD format it is much more efficient for compressed files to read the
  entire file into memory and then deinterleave it (as load_rom() does).
  In order to speed this function up a bit ucon64.file_size is used. That means
  it can't be used for an arbitrary file.
*/
{
  int n = 0, bpos = 0, fpos, fpos_org, block_size, bytesread = 0,
      len = number * size, fsize = ucon64.file_size /* q_fsize (filename) */;
  unsigned char tmp1[MAXBUFSIZE], tmp2[MAXBUFSIZE];

  fpos = fpos_org = ftell (fh);
  if (fpos >= fsize)
    return 0;

  if (len == 0)
    return 0;
  else if (len == 1)
    {
      if (fpos_org & 1)
        {
          fseek (fh, fpos / 2, SEEK_SET);
          *((unsigned char *) buffer) = fgetc (fh);
        }
      else
        {
          fseek (fh, fpos / 2 + fsize / 2, SEEK_SET);
          *((unsigned char *) buffer) = fgetc (fh);
        }
      fseek (fh, fpos_org + 1, SEEK_SET);
      return 1;
    }

  while (len > 0 && !feof (fh))
    {
      block_size = len > MAXBUFSIZE ? MAXBUFSIZE : len;

      fseek (fh, fpos / 2, SEEK_SET);
      bytesread += fread (tmp1, 1, block_size / 2, fh); // read odd bytes
      fseek (fh, (fpos + 1) / 2 + fsize / 2, SEEK_SET);
      bytesread += fread (tmp2, 1, block_size / 2, fh); // read even bytes

      if (fpos_org & 1)
        for (n = 0; n < block_size / 2; n++)
          {
            ((unsigned char *) buffer)[bpos + n * 2] = tmp1[n];
            ((unsigned char *) buffer)[bpos + n * 2 + 1] = tmp2[n];
          }
      else
        for (n = 0; n < block_size / 2; n++)
          {
            ((unsigned char *) buffer)[bpos + n * 2] = tmp2[n];
            ((unsigned char *) buffer)[bpos + n * 2 + 1] = tmp1[n];
          }
      fpos += block_size;
      bpos += block_size;
      len -= block_size;
    }
  fseek (fh, fpos_org + bytesread, SEEK_SET);
  return bytesread / size;
}


int
q_fread_mgd (void *buffer, size_t start, size_t len, const char *filename)
{
  int result;
  FILE *fh;

  if ((fh = fopen (filename, "rb")) == NULL)
    return -1;
  fseek (fh, start, SEEK_SET);
  result = (int) fread_mgd (buffer, 1, len, fh);
  fclose (fh);

  return result;
}


static void
remove_mgd_id (char *name, const char *id)
{
  char *p = name;
  while ((p = strstr (p, id)))
    {
      *p = 'X';
      *(p + 1) = 'X';
      p += 2;
    }
}


void
mgd_make_name (const char *filename, const char *prefix, int size, char *name)
// these characters are also valid in MGD file names: !@#$%^&_
{
  char *p, *fname, *size_str = 0, *suffix = 0;
  int n;

  fname = basename (filename);
  if (strlen (prefix) > 2)
    {
      fprintf (stderr, "INTERNAL ERROR: Prefix is longer than 2 characters\n");
      exit (1);
    }

  if (prefix[0] == 'S' && prefix[1] == 'F')
    {
      if (size <= 4 * MBIT)
        {
          size_str = "4";
          suffix = ".048";
        }
      else if (size <= 8 * MBIT)
        {
          size_str = "8";
          suffix = ".058";
        }
      else
        {
          suffix = ".078";
          if (size <= 16 * MBIT)
            size_str = "16";
          else if (size <= 20 * MBIT)
            size_str = "20";
          else if (size <= 24 * MBIT)
            size_str = "24";
          else // MGD supports SNES games with sizes up to 32 Mbit
            size_str = "32";
        }
    }
  else if (prefix[0] == 'M' && prefix[1] == 'D')
    {
      suffix = ".000";
      if (size <= 1 * MBIT)
        size_str = "1";
      else if (size <= 2 * MBIT)
        size_str = "2";
      else if (size <= 4 * MBIT)
        size_str = "4";
      else
        {
          if (size <= 8 * MBIT)
            {
              size_str = "8";
              suffix = ".008";
            }
          else if (size <= 16 * MBIT)
            {
              size_str = "16";
              suffix = ".018";
            }
          else
            {
              suffix = ".038";
              if (size <= 20 * MBIT)
                size_str = "20";
              else if (size <= 24 * MBIT)
                size_str = "24";
              else // MGD supports Genesis games with sizes up to 32 Mbit
                size_str = "32";
            }
        }
    }
  else if (prefix[0] == 'P' && prefix[1] == 'C')
    {
      if (size <= 1 * MBIT)
        {
          size_str = "1";
          suffix = ".040";
        }
      else if (size <= 2 * MBIT)
        {
          size_str = "2";
          suffix = ".040";
        }
      else if (size <= 3 * MBIT)
        {
          size_str = "3";
          suffix = ".030";
        }
      else if (size <= 4 * MBIT)
        {
          size_str = "4";
          suffix = ".048";
        }
      else if (size <= 6 * MBIT)
        {
          size_str = "6";
          suffix = ".058";
        }
      else // MGD supports PC-Engine games with sizes up to 8 Mbit
        {
          size_str = "8";
          suffix = ".058";
        }
    }
  else if (prefix[0] == 'G' && prefix[1] == 'G')
    {
      if (size <= MBIT)
        {
          suffix = ".000";
          if (size <= MBIT / 4)
            size_str = "04";
          else if (size <= MBIT / 2)
            size_str = "05";
          else
            size_str = "1";
        }
      else
        {
          if (size <= 2 * MBIT)
            {
              size_str = "2";
              suffix = ".024";
            }
          else if (size <= 4 * MBIT)
            {
              size_str = "4";
              suffix = ".058";
            }
          else // MGD supports Game Gear/Sega Master System games with sizes up to 6 Mbit
            {
              size_str = "6";
              suffix = ".078";
            }
        }
    }
  else if (prefix[0] == 'G' && prefix[1] == 'B')
    /*
      TODO: More info is needed about GB MGD2 file names as these are pure
            guesses
      What is the maximum game size the MGD2 supports for GB (color) games? At
      least one 64 Mbit game exists, Densha De Go! 2 (J) [C][!].
    */
    {
      if (size <= 1 * MBIT)
        {
          size_str = "1";
          suffix = ".040";
        }
      else if (size <= 2 * MBIT)
        {
          size_str = "2";
          suffix = ".040";
        }
      else if (size <= 4 * MBIT)
        {
          size_str = "4";
          suffix = ".048";
        }
      else
        {
          size_str = "8";
          suffix = ".058";
        }
    }

  // Do NOT mess with prefix (strupr()/strlwr()). See below (remove_mgd_id()).
  sprintf (name, "%s%s%s", prefix, size_str, fname);
  if (size >= 10 * MBIT ||
      (prefix[0] == 'G' && prefix[1] == 'G' && size < MBIT))
    {
      if (!strnicmp (name, fname, 4))
        strcpy (name, fname);
    }
  else
    {
      if (!strnicmp (name, fname, 3))
        strcpy (name, fname);
    }
  if ((p = strchr (name, '.')))
    *p = 0;
  n = strlen (name);
  if (n > 7)
    n = 7;
  name[n] = '0';                                // last character must be a number
  name[n + 1] = 0;
  for (n = 3; n < 7; n++)                       // we can skip the prefix
    if (name[n] == ' ')
      name[n] = '_';

  /*
    the transfer program "pclink" contains a bug in that it looks at the
    entire file name for an ID string (it should look only at the first 2
    characters).
  */
  strupr (name);
  remove_mgd_id (name + 3, "SF");
  remove_mgd_id (name + 3, "MD");
  remove_mgd_id (name + 3, "PC");
  remove_mgd_id (name + 3, "GG");
  remove_mgd_id (name + 3, "GB");

  set_suffix (name, suffix);
}

#ifdef PARALLEL
#endif // PARALLEL
