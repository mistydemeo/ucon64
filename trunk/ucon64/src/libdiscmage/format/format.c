/*
format.c - support of different image formats for libdiscmage

written by 2004 NoisyB (noisyb@gmx.net)


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifdef  HAVE_CONFIG_H
#include "../config.h"
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#ifdef  DEBUG
#ifdef  __GNUC__
#warning DEBUG active
#else
#pragma message ("DEBUG active")
#endif
#endif
#include <sys/stat.h>
#include "../misc.h"
#include "../libdiscmage.h"
#include "../libdm_misc.h"
#include "format.h"
#ifdef  DJGPP                                   // DXE's are specific to DJGPP
#include "../dxedll_priv.h"
#endif


int
format_free (dm_image_t *image)
{
#if 1
  memset (image, 0, sizeof (dm_image_t));
#else
  free (image);
#endif
  image = NULL;
  return 0;
}


FILE *
callibrate (const char *s, int len, FILE *fh)
// brute force callibration
{
  int32_t pos = ftell (fh);
  char buf[MAXBUFSIZE];
//   malloc ((len + 1) * sizeof (char));
  int size = 0;
  int tries = 0; //TODO: make this an arg

  fseek (fh, 0, SEEK_END);
  size = ftell (fh);
  fseek (fh, pos, SEEK_SET);

  for (; pos < size - len && tries < 32768; pos++, tries++)
    {
      fseek (fh, pos, SEEK_SET);
      fread (&buf, len, 1, fh);
#ifdef  DEBUG
  mem_hexdump (buf, len, ftell (fh) - len);
  mem_hexdump (s, len, ftell (fh) - len);
#endif
      if (!memcmp (s, buf, len))
        {
          fseek (fh, -len, SEEK_CUR);
          return fh;
        }
    }
  
  return NULL;
}


int
format_track_init (dm_track_t *track, FILE *fh)
{
  int pos = 0; 
  int x = 0, identified = 0;
  const char sync_data[] = {0, (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff, 0};
  char value_s[32];
  uint8_t value8 = 0;
  
  fseek (fh, track->track_start, SEEK_SET);
#if 1
  fread (value_s, 1, 16, fh);
#else
// callibrate
  fseek (fh, -15, SEEK_CUR);
  for (x = 0; x < 64; x++)
    {
      if (fread (&value_s, 1, 16, fh) != 16)
        return -1;
      fseek (fh, -16, SEEK_CUR);
      if (!memcmp (sync_data, value_s, 12))
        break;
      fseek (fh, 1, SEEK_CUR);
    }
#endif

  if (!memcmp (sync_data, value_s, 12))
    {
      value8 = (uint8_t) value_s[15];
    
      for (x = 0; track_probe[x].sector_size; x++)
        if (track_probe[x].mode == value8)
          {
            // search for valid PVD in sector 16 of source image
            pos = (track_probe[x].sector_size * 16) +
                  track_probe[x].seek_header + track->track_start;
            fseek (fh, pos, SEEK_SET);
            fread (value_s, 1, 16, fh);
            if (!memcmp (pvd_magic, &value_s, 8) ||
                !memcmp (svd_magic, &value_s, 8) ||
                !memcmp (vdt_magic, &value_s, 8))
              {
                identified = 1;
                break;
              }
          }
    }
    
  // no sync_data found? probably MODE1/2048
  if (!identified)
    {
      x = 0;
      if (track_probe[x].sector_size != 2048)
        fprintf (stderr, "ERROR: format_track_init()\n");

      fseek (fh, (track_probe[x].sector_size * 16) +
             track_probe[x].seek_header + track->track_start, SEEK_SET);
      fread (value_s, 1, 16, fh);

      if (!memcmp (pvd_magic, &value_s, 8) ||
          !memcmp (svd_magic, &value_s, 8) ||
          !memcmp (vdt_magic, &value_s, 8))
        identified = 1;
    }

  if (!identified)
    {
      fprintf (stderr, "ERROR: could not find iso header of current track\n");
      return -1;
    }

  track->sector_size = track_probe[x].sector_size;
  track->mode = track_probe[x].mode;
  track->seek_header = track_probe[x].seek_header;
  track->seek_ecc = track_probe[x].seek_ecc;
  track->iso_header_start = (track_probe[x].sector_size * 16) + track_probe[x].seek_header;
  track->desc = dm_get_track_desc (track->mode, track->sector_size, TRUE);

  return 0;
}


