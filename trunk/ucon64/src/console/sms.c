/*
sms.c - Sega Master System/Game Gear support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2003 - 2004 dbjh


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
#include "misc_chk.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "sms.h"
#include "backup/mgd.h"
#include "backup/smd.h"


#define SMS_HEADER_START 0x7ff0
#define SMS_HEADER_LEN (sizeof (st_sms_header_t))

static int sms_chksum (unsigned char *rom_buffer, int rom_size);


const st_usage_t sms_usage[] =
  {
    {NULL, NULL, "Sega Master System(II/III)/Game Gear (Handheld)"},
    {NULL, NULL, "1986/19XX SEGA http://www.sega.com"},
    {"sms", NULL, "force recognition"},
    {"int", NULL, "force ROM is in interleaved format (SMD)"},
    {"nint", NULL, "force ROM is not in interleaved format (RAW)"},
    {"mgd", NULL, "convert to Multi Game*/MGD2/MGH/RAW (gives SMS name)"},
    {"mgdgg", NULL, "same as " OPTION_LONG_S "mgd, but gives GG name"},
    {"smd", NULL, "convert to Super Magic Drive/SMD"},
    {"smds", NULL, "convert emulator (*.srm) SRAM to Super Magic Drive/SMD"},
    {"chk", NULL, "fix ROM checksum (SMS only)"},
    {NULL, NULL, NULL}
  };

typedef struct st_sms_header
{
  char signature[8];                            // "TMR "{"SEGA", "ALVS", "SMSC"}/"TMG SEGA"
  unsigned char pad[2];                         // 8
  unsigned char checksum_low;                   // 10
  unsigned char checksum_high;                  // 11
  unsigned char partno_low;                     // 12
  unsigned char partno_high;                    // 13
  unsigned char version;                        // 14
  unsigned char checksum_range;                 // 15, and country info
} st_sms_header_t;

static st_sms_header_t sms_header;
static int is_gamegear;


// see src/backup/mgd.h for the file naming scheme
int
sms_mgd (st_rominfo_t *rominfo, int console)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *buffer;
  int size = ucon64.file_size - rominfo->buheader_len;

  strcpy (src_name, ucon64.rom);
  mgd_make_name (ucon64.rom, console, size, dest_name);
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

  mgd_write_index_file (basename2 (dest_name), 1);

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
sms_chk (st_rominfo_t *rominfo)
{
  char buf[2], dest_name[FILENAME_MAX];
  int offset = rominfo->header_start + 10;

  if (is_gamegear)
    {
      fprintf (stderr, "ERROR: This option works only for SMS (not Game Gear) files\n");
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, NULL, 0);
  q_fcpy (ucon64.rom, 0, ucon64.file_size, dest_name, "wb");

  buf[0] = rominfo->current_internal_crc;       // low byte
  buf[1] = rominfo->current_internal_crc >> 8;  // high byte
  // change checksum
  if (rominfo->interleaved)
    {
      q_fputc (dest_name, rominfo->buheader_len +
        (offset & ~0x3fff) + 0x2000 + (offset & 0x3fff) / 2, buf[0], "r+b");
      q_fputc (dest_name, rominfo->buheader_len +
        (offset & ~0x3fff) + (offset & 0x3fff) / 2, buf[1], "r+b");
    }
  else
    q_fwrite (buf, rominfo->buheader_len + offset, 2, dest_name, "r+b");

  mem_hexdump (buf, 2, rominfo->buheader_len + offset);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


static int
sms_testinterleaved (st_rominfo_t *rominfo)
{
  unsigned char buf[0x4000] = { 0 };

  q_fread (buf, rominfo->buheader_len + 0x4000, // header in 2nd 16 kB block
           0x2000 + (SMS_HEADER_START - 0x4000 + 8) / 2, ucon64.rom);
  if (!(memcmp (buf + SMS_HEADER_START - 0x4000, "TMR SEGA", 8) &&
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMR ALVS", 8) && // SMS
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMR SMSC", 8) && // SMS (unofficial)
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMG SEGA", 8)))  // GG
    return 0;

  smd_deinterleave (buf, 0x4000);
  if (!(memcmp (buf + SMS_HEADER_START - 0x4000, "TMR SEGA", 8) &&
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMR ALVS", 8) &&
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMR SMSC", 8) &&
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMG SEGA", 8)))
    return 1;

  return 0;                                     // unknown, act as if it's not interleaved
}


#define SEARCHBUFSIZE (SMS_HEADER_START + 8 + 16 * 1024)
#define N_SEARCH_STR 4
static int
sms_header_len (void)
/*
  At first sight it seems reasonable to also determine whether the file is
  interleaved in this function. However, we run into a chicken-and-egg problem:
  in order to deinterleave the data we have to know the header length. And in
  order to determine the header length we have to know whether the file is
  interleaved :-) Of course we could assume the header has an even size, but it
  turns out that that is not always the case. For example, there is a copy of
  GG Shinobi (E) [b1] floating around with a "header" of 5 bytes.
  In short: this function works only for files that are not interleaved.
*/
{
  // first two hacks for Majesco Game Gear BIOS (U) [!]
  if (ucon64.file_size == 1024)
    return 0;
  else if (ucon64.file_size == 1024 + SMD_HEADER_LEN)
    return SMD_HEADER_LEN;
  else
    {
      char buffer[SEARCHBUFSIZE], *ptr, search_str[N_SEARCH_STR][9] =
             { "TMR SEGA", "TMR ALVS", "TMR SMSC", "TMG SEGA" };
      int n;

      q_fread (buffer, 0, SEARCHBUFSIZE, ucon64.rom);

      for (n = 0; n < N_SEARCH_STR; n++)
        if ((ptr = (char *)
               mem_search (buffer, SEARCHBUFSIZE, search_str[n], 8)) != NULL)
          return ptr - buffer - SMS_HEADER_START;

      n = ucon64.file_size % (16 * 1024);       // SMD_HEADER_LEN
      if (ucon64.file_size > n)
        return n;
      else
        return 0;
    }
}
#undef SEARCHBUFSIZE
#undef N_SEARCH_STR


int
sms_init (st_rominfo_t *rominfo)
{
  int result = -1, x;
  unsigned char buf[16384] = { 0 }, *rom_buffer;

  is_gamegear = 0;
  memset (&sms_header, 0, SMS_HEADER_LEN);

  if (UCON64_ISSET (ucon64.buheader_len))       // -hd, -nhd or -hdn option was specified
    rominfo->buheader_len = ucon64.buheader_len;
  else
    rominfo->buheader_len = sms_header_len ();

  rominfo->interleaved = UCON64_ISSET (ucon64.interleaved) ?
    ucon64.interleaved : sms_testinterleaved (rominfo);

  if (rominfo->interleaved)
    {
      q_fread (buf, rominfo->buheader_len + 0x4000, // header in 2nd 16 kB block
        0x2000 + (SMS_HEADER_START - 0x4000 + SMS_HEADER_LEN) / 2, ucon64.rom);
      smd_deinterleave (buf, 0x4000);
      memcpy (&sms_header, buf + SMS_HEADER_START - 0x4000, SMS_HEADER_LEN);
    }
  else
    q_fread (&sms_header, rominfo->buheader_len + SMS_HEADER_START,
      SMS_HEADER_LEN, ucon64.rom);

  rominfo->header_start = SMS_HEADER_START;
  rominfo->header_len = SMS_HEADER_LEN;
  rominfo->header = &sms_header;

  q_fread (buf, 0, 11, ucon64.rom);
  // Note that the identification bytes are the same as for Genesis SMD files
  //  The init function for Genesis files is called before this function so it
  //  is alright to set result to 0
  if ((buf[8] == 0xaa && buf[9] == 0xbb && buf[10] == 6) ||
      !(memcmp (sms_header.signature, "TMR SEGA", 8) &&
        memcmp (sms_header.signature, "TMR ALVS", 8) &&  // SMS
        memcmp (sms_header.signature, "TMR SMSC", 8) &&  // SMS (unofficial)
        memcmp (sms_header.signature, "TMG SEGA", 8)) || // GG
      ucon64.console == UCON64_SMS)
    result = 0;
  else
    result = -1;

  x = sms_header.checksum_range & 0xf0;
  if (x == 0x50 || x == 0x60 || x == 0x70)
    is_gamegear = 1;

  switch (x)
    {
    case 0x30:                                  // SMS, falling through
    case 0x50:                                  // GG
      rominfo->country = "Japan";
      break;
    case 0x40:                                  // SMS, falling through
    case 0x70:                                  // GG
      rominfo->country = "U.S.A. & Europe";
      break;
    case 0x60:                                  // GG
      rominfo->country = "Japan, U.S.A. & Europe";
      break;
    default:
      rominfo->country = "Unknown";
      break;
    }

  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      int size = ucon64.file_size - rominfo->buheader_len;
      if (!(rom_buffer = (unsigned char *) malloc (size)))
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
          return -1;
        }
      q_fread (rom_buffer, rominfo->buheader_len, size, ucon64.rom);

      if (rominfo->interleaved)
        {
          ucon64.fcrc32 = crc32 (0, rom_buffer, size);
          smd_deinterleave (rom_buffer, size);
        }
      ucon64.crc32 = crc32 (0, rom_buffer, size);

      if (!is_gamegear)
        {
          rominfo->has_internal_crc = 1;
          rominfo->internal_crc_len = 2;
          rominfo->current_internal_crc = sms_chksum (rom_buffer, size);
          rominfo->internal_crc = sms_header.checksum_low;
          rominfo->internal_crc += sms_header.checksum_high << 8;
        }

      free (rom_buffer);
    }

  sprintf ((char *) buf, "Part number: 0x%04x\n",
    sms_header.partno_low + (sms_header.partno_high << 8) +
    ((sms_header.version & 0xf0) << 12));
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "Version: %d", sms_header.version & 0xf);
  strcat (rominfo->misc, (char *) buf);

  rominfo->console_usage = sms_usage;
  rominfo->copier_usage = rominfo->buheader_len ? smd_usage : mgd_usage;

  return result;
}


int
sms_chksum (unsigned char *rom_buffer, int rom_size)
{
  unsigned short int sum;
  int i, i_end;

  switch (sms_header.checksum_range & 0xf)
    {
    case 0xc:
      i_end = 0x7ff0;
      break;
    case 0xe:                                   // falling through
    case 0xf:
      i_end = 0x20000;
      break;
    case 0:
      i_end = 0x40000;
      break;
    case 1:
      i_end = 0x80000;
      break;
    default:
      i_end = rom_size;
      break;
    }
  if (i_end > rom_size)
    i_end = rom_size;

  sum = 0;
  for (i = 0; i < i_end; i++)
    sum += rom_buffer[i];

  if (i_end >= (int) (SMS_HEADER_START + SMS_HEADER_LEN))
    for (i = SMS_HEADER_START; i < (int) (SMS_HEADER_START + SMS_HEADER_LEN); i++)
      sum -= rom_buffer[i];

  return sum;
}
