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


// the following three functions are used by non-transfer code in genesis.c
void
mgd_interleave (unsigned char **buffer, int size)
{
  int n;
  unsigned char *src = *buffer;

  *buffer = (unsigned char *) malloc (size);
  for (n = 0; n < size / 2; n++)
    {
      (*buffer)[n] = src[n * 2 + 1];
      (*buffer)[size / 2 + n] = src[n * 2];
    }
  free (src);
}


void
mgd_deinterleave (unsigned char **buffer, int size)
{
  int n = 0, offset;
  unsigned char *src = *buffer;

  *buffer = (unsigned char *) malloc (size);
  for (offset = 0; offset < size / 2; offset++)
    {
      (*buffer)[n++] = src[size / 2 + offset];
      (*buffer)[n++] = src[offset];
    }
  free (src);
}


int
q_fread_mgd (void *buffer, size_t fpos, size_t len, const char *filename)
/*
  This function is used to handle an MGD file as if it wasn't interleaved,
  without the overhead of reading the entire file into memory. This is
  important for genesis_init(). When the file turns out to be a Genesis dump in
  MGD format it is much more efficient for compressed files to read the entire
  file into memory and then deinterleave it (as load_rom() does).
  In order to speed this function up a bit ucon64.file_size is used. That means
  it can't be used for an arbitrary file.
*/
{
  int bpos = 0, n, block_size, fpos_odd = fpos & 1, // flag if starting fpos is odd
      size = ucon64.file_size /* q_fsize (filename) */;
  FILE *fh;
  unsigned char tmp1[MAXBUFSIZE], tmp2[MAXBUFSIZE];

  if ((fh = fopen (filename, "rb")) == NULL)
    return -1;
  while (len > 0)
    {
      block_size = len > MAXBUFSIZE ? MAXBUFSIZE : len;

      fseek (fh, fpos / 2, SEEK_SET);
      fread (tmp1, 1, block_size / 2, fh);      // read odd bytes
      fseek (fh, (fpos + 1) / 2 + size / 2, SEEK_SET);
      fread (tmp2, 1, block_size / 2, fh);      // read even bytes

      if (fpos_odd)
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
  fclose (fh);

  return 0;
}


#ifdef PARALLEL
#endif // PARALLEL
