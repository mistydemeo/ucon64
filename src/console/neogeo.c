/*
neogeo.c - NeoGeo support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)


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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "neogeo.h"

const char *neogeo_usage[] =
  {
    "Neo Geo/Neo Geo CD(Z)/MVS",
    "1990/1994 SNK http://www.neogeo.co.jp",
    "  " OPTION_LONG_S "ng          force recognition"
#ifndef HAVE_ZLIB_H
    "; NEEDED"
#endif
    "\n"
#if 0
    "  " OPTION_LONG_S "ns          force ROM is not split\n"
    "TODO:  " OPTION_LONG_S "mgd   convert to Multi Game Doctor/MGD2/RAW\n"
    "TODO:  " OPTION_LONG_S "mvs   convert to Arcade/MVS\n"
#endif
    "  " OPTION_LONG_S "bios        convert NeoCD BIOS to work with NeoCD emulator; " OPTION_LONG_S "rom=BIOS\n"
    "                  http://www.illusion-city.com/neo/\n"
#if 0
    "TODO:  " OPTION_S "j     join split ROM\n"
    "TODO:  " OPTION_S "s     split ROM into 4Mb parts (for backup unit(s) with fdd)\n"
    "TODO:  " OPTION_LONG_S "ngs   convert Neo Geo sound to WAV; " OPTION_LONG_S "rom=*_m1.rom or *_v*.rom\n"
#endif
    "  " OPTION_LONG_S "sam         convert SAM/M.A.M.E. sound to WAV; " OPTION_LONG_S "rom=SAMFILE\n"
//    "TODO: " OPTION_LONG_S "chkm    check/fix Multiple Arcade Machine Emulator/M.A.M.E. ROMs;\n"
//    "                  " OPTION_LONG_S "rom=DIRECTORY\n"
//    "INFO: actually this option does the same as Goodxxxx, Romcenter, etc.\n"
//    "      Therefore you must have the DAT files for Arcade installed\n"
    "                  for more options check the support for DISC-based consoles\n"
    ,
    NULL
  };


static int sam2wav (const char *filename);

#define NEOGEO_HEADER_START 0
#define NEOGEO_HEADER_LEN 0


int
neogeo_bios (st_rominfo_t *rominfo)
{
  char buf[MAXBUFSIZE];
  strcpy (buf, ucon64.rom);
  setext (buf, ".NEW");

  ucon64_fbackup (NULL, buf);
  q_fcpy (ucon64.rom, 0, MBIT, buf, "wb");
  q_fswap (buf, 0, MBIT);

  return 0;
}


int
neogeo_sam (st_rominfo_t *rominfo)
{
  if (sam2wav (ucon64.rom) == -1)
    {
      fprintf (stderr, "ERROR: SAM header seems to be corrupt\n");
    }
  return 0;
}


int
neogeo_mgd (st_rominfo_t *rominfo)
{
  return 0;
}


int
neogeo_mvs (st_rominfo_t *rominfo)
{
  return 0;
}


int
neogeo_s (st_rominfo_t *rominfo)
{
  return 0;
}


int
neogeo_init (st_rominfo_t *rominfo)
{
  int result = -1;

  rominfo->console_usage = neogeo_usage;
  rominfo->copier_usage = unknown_usage;

  return result;
}


int
sam2wav (const char *filename)
{
  unsigned char buf[32];
  FILE *fh, *fh2;
  unsigned datasize, wavesize, riffsize, freq, bits, rate;

  if (q_fsize (filename) < 16)
    return -1;
  if (!(fh = fopen (filename, "rb")))
    return -1;

  strcpy (buf, filename);
  setext (buf, ".WAV");

  if (!(fh2 = fopen (buf, "wb")))
    return -1;
  fread (buf, 1, 4, fh);

  if (strncmp (buf, "MAME", 4) != 0)
    return -1;
  fread (buf, 1, 4, fh);

  datasize = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
  wavesize = /* "fmt " */ 4 + 4 + 4 + 2 + 2;
  riffsize = /* RIFF */ 4 + 4 + 4 + 4 + 4 + wavesize + datasize;

  buf[0] = riffsize & 0xff;
  buf[1] = (riffsize >> 8) & 0xff;
  buf[2] = (riffsize >> 16) & 0xff;
  buf[3] = (riffsize >> 24) & 0xff;

  fwrite ("RIFF", 1, 4, fh2);
  fwrite (buf, 1, 4, fh2);

  fwrite ("WAVE", 1, 4, fh2);
  fwrite ("fmt ", 1, 4, fh2);

  buf[0] = wavesize & 0xff;
  buf[1] = (wavesize >> 8) & 0xff;
  buf[2] = (wavesize >> 16) & 0xff;
  buf[3] = (wavesize >> 24) & 0xff;
  fwrite (buf, 1, 4, fh2);

  /* number of channels - alway 1 for MAME samples */
  fwrite ("\001\000\001\000", 1, 4, fh2);

  /* read frequency */
  fread (buf, 1, 4, fh);
  freq = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);

  /* write frequency */
  fwrite (buf, 1, 4, fh2);

  /* read bits, amplitude and extension */
  fread (buf, 1, 4, fh);

  bits = buf[0];

  if (bits == 16)
    rate = freq * 2;
  else
    rate = freq;

  buf[0] = rate & 0xff;
  buf[1] = (rate >> 8) & 0xff;
  buf[2] = (rate >> 16) & 0xff;
  buf[3] = (rate >> 24) & 0xff;

  /* write bytes per second */
  fwrite (buf, 1, 4, fh2);

  /* write alignment */
  if (bits == 16)
    fwrite ("\002\000", 1, 2, fh2);
  else
    fwrite ("\001\000", 1, 2, fh2);

  buf[0] = bits & 0xff;
  buf[1] = (bits >> 8) & 0xff;

  /* write bits per sample */
  fwrite (buf, 1, 2, fh2);
  fwrite ("data", 1, 4, fh2);

  buf[0] = datasize & 0xff;
  buf[1] = (datasize >> 8) & 0xff;
  buf[2] = (datasize >> 16) & 0xff;
  buf[3] = (datasize >> 24) & 0xff;
  fwrite (buf, 1, 4, fh2);

  if (bits == 16)
    {
      for (;;)
        {
          if (fread (buf, 1, 2, fh) != 2)
            break;
          fwrite (buf, 1, 2, fh2);
        }
    }
  else
    {
      for (;;)
        {
          if (fread (buf, 1, 1, fh) != 1)
            break;
          buf[0] = buf[0] ^ 0x80;
          fwrite (buf, 1, 1, fh2);
        }
    }
  fclose (fh2);
  fclose (fh);
  return 0;
}
