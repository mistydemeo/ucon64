/*
wswan.c - WonderSwan support for uCON64

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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "swan.h"

static int swan_chksum (st_rominfo_t *rominfo, unsigned char *rom_buffer);

const st_usage_t swan_usage[] =
  {
    {NULL, NULL, "WonderSwan/WonderSwan Color/SwanCrystal"},
    {NULL, NULL, "19XX/19XX/2002 Bandai"},
    {"swan", NULL, "force recognition"},
    {"chk", NULL, "fix ROM checksum"},
    {NULL, NULL, NULL}
};


typedef struct st_swan_header
{
  char pad[10];
} st_swan_header_t;
#define SWAN_HEADER_START (ucon64.file_size - 10)
#define SWAN_HEADER_LEN (sizeof (st_swan_header_t))

st_swan_header_t swan_header;


int
swan_chk (st_rominfo_t *rominfo)
{
  char buf[3], dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.rom);
  if (!ucon64_file_handler (dest_name, NULL, 0))
    q_fcpy (ucon64.rom, 0, q_fsize (ucon64.rom), dest_name, "wb");

  q_fputc (dest_name, SWAN_HEADER_START + 8, rominfo->current_internal_crc, "r+b"); // low byte
  q_fputc (dest_name, SWAN_HEADER_START + 9, rominfo->current_internal_crc >> 8, "r+b"); // high byte

  q_fread (buf, SWAN_HEADER_START + 8, 2, dest_name);
  mem_hexdump (buf, 2, SWAN_HEADER_START + 8);

  return 0;
}


int
swan_init (st_rominfo_t *rominfo)
{
  int result = -1;
  unsigned char *rom_buffer, buf[MAXBUFSIZE];
#define SWAN_MAKER_MAX 0x30
  const char *swan_maker[SWAN_MAKER_MAX] = {
    "BAN", "BAN", NULL, NULL, NULL,
    "DTE", NULL, NULL, NULL, NULL,
    NULL, "SUM", "SUM", NULL, "BPR",
    NULL, NULL, NULL, "KNM", NULL,
    NULL, NULL, "KGT", NULL, NULL,
    NULL, NULL, "MGH", NULL, "BEC",
    "NAP", "BVL", NULL, NULL, NULL,
    NULL, NULL, NULL, "KDK", NULL,
    "SQR", NULL, NULL, NULL, NULL,
    "NMC", NULL, NULL};

  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
    ucon64.buheader_len : 0;

  q_fread (&swan_header, SWAN_HEADER_START +
      rominfo->buheader_len, SWAN_HEADER_LEN, ucon64.rom);

  rominfo->header = &swan_header;
  rominfo->header_start = SWAN_HEADER_START;
  rominfo->header_len = SWAN_HEADER_LEN;

  // ROM maker
  rominfo->maker = NULL_TO_UNKNOWN_S (swan_maker[MIN (OFFSET (swan_header, 0),
    SWAN_MAKER_MAX - 1)]);

  // misc stuff
  sprintf (buf, "Minimum supported system: %s",
           (!OFFSET (swan_header, 1) ? "WS Monochrome" : "WS Color"));
  strcat (rominfo->misc, buf);

/*
Byte2 - Cartridge ID number for this developer

Byte3 - ?Unknown?
00 - most roms
01 - Dig2/BAN032 & soro/KGT007
02 - Chocobo/BAN002
03 - sdej/BAN006
04 - srv2/BPR006

  sprintf (buf, "Internal Size: %.4f Mb\n", (float) rominfo->header[4]);
Byte4 - ROM Size:
01 - ?
02 - 4Mbit
03 - 8Mbit
04 - 16Mbit
05 - ?
06 - 32Mbit
07 - ?
08 - ?
09 - 128Mbit

Byte5 - SRAM/EEPROM Size:
00 - 0k
01 - 64k SRAM
02 - 256k SRAM
10 - 1k EEPROM (MGH001/NAP001)
20 - 16k EEPROM

Byte6 - Additional capabilities(?)
04 - ?? game played in "horizontal" position (most roms)
05 - ?? game played in "vertical" position
10 - ?? (SUN003)
*/

  if (!(rom_buffer = (unsigned char *) malloc (ucon64.file_size)))
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], ucon64.file_size);
      return -1;
    }
  q_fread (rom_buffer, 0, ucon64.file_size, ucon64.rom);

  rominfo->has_internal_crc = 1;
  rominfo->internal_crc_len = rominfo->internal_crc2_len = 2;
  rominfo->current_internal_crc = swan_chksum (rominfo, rom_buffer);

  rominfo->internal_crc = OFFSET (swan_header, 8);              // low byte of checksum
  rominfo->internal_crc += OFFSET (swan_header, 9) << 8;        // high byte of checksum
  if (rominfo->current_internal_crc == rominfo->internal_crc)
    result = 0;
  else
    result = -1;
  if (ucon64.console == UCON64_SWAN)
    result = 0;

  rominfo->console_usage = swan_usage;
  rominfo->copier_usage = unknown_usage;

  free (rom_buffer);
  return result;
}


int
swan_chksum (st_rominfo_t *rominfo, unsigned char *rom_buffer)
{
  unsigned int csum = 0;
  int t;
  unsigned char *ptr;

  if (ucon64.file_size % 4)
    return -1;

  t = ucon64.file_size - 3;
  ptr = rom_buffer;
  while (t >= 0)
    {
      csum += *ptr++;
      t--;
    }
  csum &= 0xffff;

  return csum;
}
