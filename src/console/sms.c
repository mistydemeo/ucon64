/*
sms.c - Sega Master System/GameGear support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2003 dbjh


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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "sms.h"
#include "backup/mgd.h"
#include "backup/smd.h"


const st_usage_t sms_usage[] =
  {
    {NULL, NULL, "Sega Master System(II/III)/GameGear (Handheld)"},
    {NULL, NULL, "1986/19XX SEGA http://www.sega.com"},
    {"sms", NULL, "force recognition"},
    {"mgd", NULL, "convert to Multi Game*/MGD2/MGH/RAW"},
    {"smd", NULL, "convert to Super Magic Drive/SMD (+512 Bytes)"},
    {"smds", NULL, "convert emulator (*.srm) SRAM to Super Magic Drive/SMD"},
    {NULL, NULL, NULL}
  };


#if 0
typedef struct st_sms_header
{
  char pad[64];
} st_sms_header_t;
#define SMS_HEADER_START 0
#define SMS_HEADER_LEN (sizeof (st_sms_header_t))

st_sms_header_t sms_header;
#endif


int
sms_mgd (st_rominfo_t *rominfo)
{
/*
The MGD2 only accepts certain filenames, and these filenames
must be specified in an index file, "MULTI-GD", otherwise the
MGD2 will not recognize the file.  In the case of multiple games
being stored in a single disk, simply enter its corresponding
MULTI-GD index into the "MULTI-GD" file.

Mega Drive:

game size       # of files      names           MUTLI-GD
================================================================
1M              1               MD1XXX.000      MD1XXX
2M              1               MD2XXX.000      MD2XXX
4M              1               MD4XXX.000      MD4XXX
8M              1               MD8XXX.008      MD8XXX
16M             2               MD16XXXA.018    MD16XXXA
                                MD16XXXB.018    MD16XXXB
24M             3               MD24XXXA.038    MD24XXXA
                                MD24XXXB.038    MD24XXXB
                                MD24XXXC.038    MD24XXXC
32M             4               MD32XXXA.038    MD32XXXA
                                MD32XXXB.038    MD32XXXB
                                MD32XXXC.038    MD32XXXC
                                MD32XXXD.038    MD32XXXD

Usually, the filename is in the format of: SFXXYYYZ.078
Where SF means Super Famicom, XX refers to the size of the
image in Mbit. If the size is only one character (i.e. 2, 4 or
8 Mbit) then no leading "0" is inserted.

YYY refers to a catalogue number in Hong Kong shops
identifying the game title. (0 is Super Mario World, 1 is F-
Zero, etc). I was told that the Game Doctor copier produces a
random number when backing up games.

Z indicates a multi file. Like XX, if it isn't used it's
ignored.

A would indicate the first file, B the second, etc. I am told
078 is not needed, but is placed on the end of the filename by
systems in Asia.

e.g. The first 16Mbit file of Donkey Kong Country (assuming it
  is cat. no. 475) would look like:  SF16475A.078

NOTE: Can anyone explain to me (dbjh) what the relationship is between the
      comment above and the code below? This question applies to all source
      files that contain similar info about the Game Doctor file naming
      scheme...
*/
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], *p = NULL, suffix[5];
  unsigned char *buffer;
  int size = ucon64.file_size - rominfo->buheader_len;

  strcpy (src_name, ucon64.rom);
  p = basename (ucon64.rom);
  sprintf (dest_name, "%s%s", is_func (dest_name, strlen (p), isupper) ? "GG" : "gg", p);
  if ((p = strrchr (dest_name, '.')))
    *p = 0;
  strcat (dest_name, "_____");
  dest_name[7] = '_';
  dest_name[8] = 0;
  sprintf (suffix, ".%03u", size / MBIT);
  set_suffix (dest_name, suffix);

  ucon64_file_handler (dest_name, src_name, OF_FORCE_BASENAME);

  if (!(buffer = (unsigned char *) malloc (size)))
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  q_fread (buffer, rominfo->buheader_len, size, src_name);
  if (rominfo->interleaved)
    smd_deinterleave (buffer, size);

  q_fwrite (buffer, 0, size, dest_name, "wb");
  free (buffer);

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
sms_smd (st_rominfo_t *rominfo)
{
  st_smd_header_t header;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *buffer;
  int size = ucon64.file_size - rominfo->buheader_len;

  memset (&header, 0, SMD_HEADER_LEN);
  header.size = size / 8192 >> 8;
  header.id0 = 3; //size / 8192;
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 6;

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".SMD");
  ucon64_file_handler (dest_name, src_name, 0);

  if (!(buffer = (unsigned char *) malloc (size)))
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  q_fread (buffer, rominfo->buheader_len, size, src_name);
  if (!rominfo->interleaved)
    smd_interleave (buffer, size);

  q_fwrite (&header, 0, SMD_HEADER_LEN, dest_name, "wb");
  q_fwrite (buffer, rominfo->buheader_len, size, dest_name, "ab");
  free (buffer);

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
sms_smds (void)
{
  st_smd_header_t header;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  memset (&header, 0, SMD_HEADER_LEN);
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 7;                              // SRAM file

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".SAV");
  ucon64_file_handler (dest_name, src_name, 0);

  q_fwrite (&header, 0, SMD_HEADER_LEN, dest_name, "wb");
  q_fcpy (src_name, 0, q_fsize (src_name), dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
sms_init (st_rominfo_t *rominfo)
{
  int result = -1;
  unsigned char magic[11], *buffer;

  if (UCON64_ISSET (ucon64.buheader_len))       // -hd, -nhd or -hdn option was specified
    rominfo->buheader_len = ucon64.buheader_len;

  q_fread (&magic, 0, 11, ucon64.rom);
  // Note that the identification bytes are the same as for Genesis SMD files
  //  The init function for Genesis files is called before this function so it
  //  is alright to set result to 0
  if (magic[8] == 0xaa && magic[9] == 0xbb && magic[10] == 6)
    {
      if (!UCON64_ISSET (ucon64.buheader_len))
        rominfo->buheader_len = SMD_HEADER_LEN;

      if (!(UCON64_ISSET (ucon64.interleaved) && !ucon64.interleaved) &&
          !UCON64_ISSET (ucon64.do_not_calc_crc))
        {
          int size = ucon64.file_size - rominfo->buheader_len;

          if (!(buffer = (unsigned char *) malloc (size)))
            {
              fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
              return -1;
            }
          q_fread (buffer, rominfo->buheader_len, size, ucon64.rom);

          ucon64.fcrc32 = crc32 (0, buffer, size);
          smd_deinterleave (buffer, size);
          ucon64.crc32 = crc32 (0, buffer, size);

          free (buffer);
        }
      result = 0;
    }
  rominfo->interleaved = UCON64_ISSET (ucon64.interleaved) ?
    ucon64.interleaved : (result == 0 ? 1 : 0);

  rominfo->console_usage = sms_usage;
  rominfo->copier_usage = rominfo->buheader_len ? smd_usage : mgd_usage;

  return result;
}
