/*
lynx.c - Atari Lynx support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2002 dbjh


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
#include <fcntl.h>
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
#include "lynx.h"


const char *lynx_usage[] =
  {
    "Handy(prototype)/Lynx/Lynx II",
    "1987 Epyx/1989 Atari/1991 Atari",
    "  " OPTION_LONG_S "lynx        force recognition"
#ifndef DB
    "; NEEDED"
#endif
    "\n"
    "  " OPTION_LONG_S "lyx         convert to LYX/RAW (strip 64 Bytes LNX header)\n"
    "  " OPTION_LONG_S "lnx         convert to LNX (uses default values for the header);\n"
    "                  adjust the LNX header with the following options\n"
    "  " OPTION_S "n           change ROM name (LNX only); " OPTION_LONG_S "file=NEWNAME\n"
    "  " OPTION_LONG_S "nrot        set no rotation (LNX only)\n"
    "  " OPTION_LONG_S "rotl        set rotation left (LNX only)\n"
    "  " OPTION_LONG_S "rotr        set rotation right (LNX only)\n"
    "  " OPTION_LONG_S "b0          change Bank0 kBytes size (LNX only); " OPTION_LONG_S "file={0,64,128,256,512}\n"
    "  " OPTION_LONG_S "b1          change Bank1 kBytes size (LNX only); " OPTION_LONG_S "file={0,64,128,256,512}\n",
    NULL
};



//static const char *lnx_usage[] = "LNX header";
#define LNX_HEADER_START 0
#define LNX_HEADER_LEN (sizeof (st_lnx_header_t))

st_lnx_header_t lnx_header;


int
lynx_lyx (st_rominfo_t *rominfo)
{
  char buf[MAXBUFSIZE];

  if (!rominfo->buheader_len)
    {
      fprintf (stderr, "ERROR: this is no LNX file\n\n");
      return -1;
    }

  strcpy (buf, ucon64.rom);
  setext (buf, ".LYX");

  q_fcpy (ucon64.rom, rominfo->buheader_len, q_fsize (ucon64.rom),
            ucon64_fbackup (NULL, buf), "wb");

  ucon64_wrote (buf);
  return 0;
}


int
lynx_lnx (st_rominfo_t *rominfo)
{
  st_lnx_header_t header;
  char buf[MAXBUFSIZE];
  long size = q_fsize (ucon64.rom);

  if (rominfo->buheader_len != 0)
    {
      fprintf (stderr, "ERROR: this seems to be already an LNX file\n\n");
      return -1;
    }

  header.page_size_bank0 = size > (4 * MBIT) ? (4 * MBIT / 256) : (size / 256);
  header.page_size_bank1 = size > (4 * MBIT) ? (size - (4 * MBIT)) / 256 : 0;
#ifdef  WORDS_BIGENDIAN
  header.page_size_bank0 = bswap_16 (header.page_size_bank0);
  header.page_size_bank1 = bswap_16 (header.page_size_bank1);
#endif

  memset (header.cartname, 0, sizeof (header.cartname));
  memset (header.manufname, 0, sizeof (header.manufname));
  memset (header.spare, 0, sizeof (header.spare));

#ifdef  WORDS_BIGENDIAN
  header.version = bswap_16 (1);
#else
  header.version = 1;
#endif

  memcpy (header.magic, "LYNX", 4);
  header.rotation = 0;
  strncpy (header.cartname, ucon64.rom, sizeof (header.cartname));
  strcpy (header.manufname, "Atari");

  strcpy (buf, ucon64.rom);
  setext (buf, ".LNX");

  q_fwrite (&header, 0, sizeof (st_lnx_header_t), ucon64_fbackup (NULL, buf), "wb");
  q_fcpy (ucon64.rom, 0, q_fsize (ucon64.rom), buf, "ab");

  ucon64_wrote (buf);
  return 0;
}


static int
lynx_rot (st_rominfo_t *rominfo, int rotation)
{
  st_lnx_header_t header;

  if (!rominfo->buheader_len)
    {
      fprintf (stderr, "ERROR: this is no LNX file\n\n");
      return -1;
    }

  q_fread (&header, 0, sizeof (st_lnx_header_t), ucon64.rom);

  header.rotation = rotation;

  q_fwrite (&header, 0, sizeof (st_lnx_header_t), ucon64_fbackup (NULL, ucon64.rom),
               "r+b");

  ucon64_wrote (ucon64.rom);

  return 0;
}


int
lynx_nrot (st_rominfo_t *rominfo)
{
  return lynx_rot (rominfo, 0);                 // no rotation
}


int
lynx_rotl (st_rominfo_t *rominfo)
{
  return lynx_rot (rominfo, 1);                 // rotate left
}


int
lynx_rotr (st_rominfo_t *rominfo)
{
  return lynx_rot (rominfo, 2);                 // rotate right
}


int
lynx_n (st_rominfo_t *rominfo)
{
  st_lnx_header_t header;

  if (!rominfo->buheader_len)
    {
      fprintf (stderr, "ERROR: this is no LNX file\n\n");
      return -1;
    }

  q_fread (&header, 0, sizeof (st_lnx_header_t), ucon64.rom);

  memset (header.cartname, 0, sizeof (header.cartname));
  strncpy (header.cartname, ucon64.file, sizeof (header.cartname));

  q_fwrite (&header, 0, sizeof (st_lnx_header_t), ucon64_fbackup (NULL, ucon64.rom),
               "r+b");

  ucon64_wrote (ucon64.rom);
  return 0;
}


static int
lynx_b (st_rominfo_t *rominfo, int bank)
{
  st_lnx_header_t header;
  short int *bankvar;

  if (!rominfo->buheader_len)
    {
      fprintf (stderr, "ERROR: this is no LNX file\n\n");
      return -1;
    }

  q_fread (&header, 0, sizeof (st_lnx_header_t), ucon64.rom);

  bankvar = (bank == 0 ? &header.page_size_bank0 : &header.page_size_bank1);
  if ((atol (ucon64.file) % 64) != 0 || (atol (ucon64.file) > 512))
    *bankvar = 0;
  else
#ifdef  WORDS_BIGENDIAN
    *bankvar = bswap_16 (atol (ucon64.file) * 4);
#else
    *bankvar = atol (ucon64.file) * 4;
#endif

  q_fwrite (&header, 0, sizeof (st_lnx_header_t), ucon64_fbackup (NULL, ucon64.rom),
               "r+b");

  ucon64_wrote (ucon64.rom);
  return 0;
}


int
lynx_b0 (st_rominfo_t *rominfo)
{
  return lynx_b (rominfo, 0);
}


int
lynx_b1 (st_rominfo_t *rominfo)
{
  return lynx_b (rominfo, 1);
}


int
lynx_init (st_rominfo_t *rominfo)
{
  int result = -1;

  rominfo->console_usage = lynx_usage;
  rominfo->copier_usage = unknown_usage;

  q_fread (&lnx_header, 0, LNX_HEADER_LEN, ucon64.rom);
#ifdef CONSOLE_PROBE
  if (!strncmp (lnx_header.magic, "LYNX", 4))
    result = 0;
  else
    result = -1;
#endif // CONSOLE_PROBE
  if (ucon64.console == UCON64_LYNX)
    result = 0;

  if (!strncmp (lnx_header.magic, "LYNX", 4))
    {
      rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
        ucon64.buheader_len : LNX_HEADER_LEN;

      if (UCON64_ISSET (ucon64.buheader_len) && !ucon64.buheader_len)
        return ucon64.console == UCON64_LYNX ? 0 : result;

      q_fread (&lnx_header, 0, LNX_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &lnx_header;

      // internal ROM name
      strcpy (rominfo->name, lnx_header.cartname);

      // ROM maker
      rominfo->maker = lnx_header.manufname;

      // misc stuff
      sprintf (rominfo->misc,
        "Internal Size: Bank0 %hd Bytes (%.4f Mb)\n"
        "               Bank1 %hd Bytes (%.4f Mb)\n"
        "Version: %d\n"
        "Rotation: %s",
#ifdef  WORDS_BIGENDIAN
        bswap_16 (lnx_header.page_size_bank0) * 256,
        TOMBIT_F (bswap_16 (lnx_header.page_size_bank0) * 256),
        bswap_16 (lnx_header.page_size_bank1) * 256,
        TOMBIT_F (bswap_16 (lnx_header.page_size_bank1) * 256),
        bswap_16 (lnx_header.version),
#else
        lnx_header.page_size_bank0 * 256,
        TOMBIT_F (lnx_header.page_size_bank0 * 256),
        lnx_header.page_size_bank1 * 256,
        TOMBIT_F (lnx_header.page_size_bank1 * 256),
        lnx_header.version,
#endif
        (!lnx_header.rotation) ? "No" : ((lnx_header.rotation == 1) ? "Left" : "Right"));
    }

  return result;
}
