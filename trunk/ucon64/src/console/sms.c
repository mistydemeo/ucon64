/*
sms.c - Sega Master System/Game Gear support for uCON64

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
    {NULL, NULL, "Sega Master System(II/III)/Game Gear (Handheld)"},
    {NULL, NULL, "1986/19XX SEGA http://www.sega.com"},
    {"sms", NULL, "force recognition"},
    {"int", NULL, "force ROM is in interleaved format (SMD)"},
    {"nint", NULL, "force ROM is not in interleaved format (RAW)"},
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


// see src/backup/mgd.h for the file naming scheme
int
sms_mgd (st_rominfo_t *rominfo)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *buffer;
  int size = ucon64.file_size - rominfo->buheader_len;

  strcpy (src_name, ucon64.rom);
  mgd_make_name (ucon64.rom, "GG", size, dest_name);
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
  if (size <= 4 * MBIT)
    printf ("NOTE: It may be necessary to change the suffix in order to make the game work\n"
            "      on an MGD2. You could try suffixes like .010, .024, .040, .048 or .078.\n");
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
  unsigned char magic[11] = "", *buffer;

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

      // don't deinterleave if -nint was specified
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
  else if (!UCON64_ISSET (ucon64.buheader_len))
    rominfo->buheader_len = ucon64.file_size % (16 * 1024);
  rominfo->interleaved = UCON64_ISSET (ucon64.interleaved) ?
    ucon64.interleaved : (result == 0 ? 1 : 0);

  rominfo->console_usage = sms_usage;
  rominfo->copier_usage = rominfo->buheader_len ? smd_usage : mgd_usage;

  return result;
}
