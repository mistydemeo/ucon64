/*
gb.c - Game Boy support for uCON64

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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "gb.h"

#include "console/nes.h"

#include "backup/gbx.h"
#include "backup/mgd.h"
#include "backup/ssc.h"

#include "patch/ips.h"
#include "patch/bsl.h"


const st_usage_t gameboy_usage[] =
  {
    {NULL, NULL, "Game Boy/(Super GB)/GB Pocket/Color GB/(GB Advance)"},
    {NULL, NULL, "1989/1994/1996/1998/2001 Nintendo http://www.nintendo.com"},
    {"gb", NULL, "force recognition"},
    {"n", "NEW_NAME", "change internal ROM name to NEW_NAME"},
    {"mgd", NULL, "convert to Multi Game*/MGD2/RAW"},
    {"ssc", NULL, "convert to Super Smart Card/SSC (+512 Bytes)"},
    {"sgb", NULL, "convert from GB Xchanger/GB/GBC to Super Backup Card/GX/GBX"},
    {"gbx", NULL, "convert from Super Backup Card/GX/GBX to GB Xchanger/GB/GBC"},
    {"n2gb", "EMU", "convert for use with Kami's FC EMUlator (NES emulator);\n"
                    OPTION_LONG_S "rom" OPTARG_S "NES_ROM"},
    {"chk", NULL, "fix ROM checksum"},
    {NULL, NULL, NULL}
  };


/*
0148       ROM size:
           0 - 256kBit =  32kB =  2 banks
           1 - 512kBit =  64kB =  4 banks
           2 -   1Mb = 128kB =  8 banks
           3 -   2Mb = 256kB = 16 banks
           4 -   4Mb = 512kB = 32 banks
0149       RAM size:
           0 - None
           1 -  16kBit =  2kB = 1 bank
           2 -  64kBit =  8kB = 1 bank
           3 - 256kBit = 32kB = 4 banks
*/
typedef struct st_gameboy_header
{
  char pad[80];
} st_gameboy_header_t;
#define GAMEBOY_HEADER_START 0x100
#define GAMEBOY_HEADER_LEN (sizeof (st_gameboy_header_t))

st_gameboy_header_t gameboy_header;

typedef struct st_gameboy_chksum
{
  unsigned short value;
  unsigned char complement;
} st_gameboy_chksum_t;

static st_gameboy_chksum_t checksum;
static st_gameboy_chksum_t gameboy_chksum (st_rominfo_t *rominfo);
static st_ines_header_t ines_header;
static st_unknown_header_t unknown_header;


int
gameboy_n2gb (st_rominfo_t *rominfo, const char *emu_rom)
{
#define EMULATOR_LEN 0x10000
  int n = 0, crc = 0;
  unsigned char buf[EMULATOR_LEN];

//  printf("FC emulator for Game Boy\n");

  if (q_fsize (ucon64.rom) != EMULATOR_LEN)
    {
//      fprintf (stderr, "ERROR: only NES ROMs with iNES header are supported\n");
      return -1;
    }

  memset (buf, 0, EMULATOR_LEN);
  q_fread (buf, 0, EMULATOR_LEN, emu_rom);

  q_fread (&ines_header, 0, INES_HEADER_LEN, ucon64.rom);

  if (!strncmp (ines_header.signature, "NES", 3))
    {
      fprintf (stderr, "ERROR: Only NES ROMs with iNES header are supported\n");
      return -1;
    }

  q_fread (&buf[0x4000], 0, 0x4000, ucon64.rom);
  q_fread (&buf[0x8000], 0x4000, 0x2000, ucon64.rom);

  for (n = 0; n < EMULATOR_LEN; n++)
    {
      if (n == 0x14e || n == 0x14f)
        continue;
      else
        crc += buf[n];
    }

  buf[0x14e] = (crc >> 8) & 0xff;
  buf[0x14f] = crc & 0xff;

  q_fwrite (buf, 0, EMULATOR_LEN, emu_rom, "wb");
  printf (ucon64_msg[WROTE], emu_rom);
  return 0;
}


int
gameboy_gbx (st_rominfo_t *rominfo)
{
  long x;
  char dest_name[FILENAME_MAX];
  int c, gbx2gbc[] = {
    0xB4, 0xBC, 0xA4, 0xAC, 0x94, 0x9C, 0x84, 0x8C, 0xF4, 0xFC, 0xE4, 0xEC,
    0xD4, 0xDC, 0xC4, 0xCC,
    0x34, 0x3C, 0x24, 0x2C, 0x14, 0x1C, 0x04, 0x0C, 0x74, 0x7C, 0x64, 0x6C,
    0x54, 0x5C, 0x44, 0x4C,
    0xB5, 0xBD, 0xA5, 0xAD, 0x95, 0x9D, 0x85, 0x8D, 0xF5, 0xFD, 0xE5, 0xED,
    0xD5, 0xDD, 0xC5, 0xCD,
    0x35, 0x3D, 0x25, 0x2D, 0x15, 0x1D, 0x05, 0x0D, 0x75, 0x7D, 0x65, 0x6D,
    0x55, 0x5D, 0x45, 0x4D,
    0xB6, 0xBE, 0xA6, 0xAE, 0x96, 0x9E, 0x86, 0x8E, 0xF6, 0xFE, 0xE6, 0xEE,
    0xD6, 0xDE, 0xC6, 0xCE,
    0x36, 0x3E, 0x26, 0x2E, 0x16, 0x1E, 0x06, 0x0E, 0x76, 0x7E, 0x66, 0x6E,
    0x56, 0x5E, 0x46, 0x4E,
    0xB7, 0xBF, 0xA7, 0xAF, 0x97, 0x9F, 0x87, 0x8F, 0xF7, 0xFF, 0xE7, 0xEF,
    0xD7, 0xDF, 0xC7, 0xCF,
    0x37, 0x3F, 0x27, 0x2F, 0x17, 0x1F, 0x07, 0x0F, 0x77, 0x7F, 0x67, 0x6F,
    0x57, 0x5F, 0x47, 0x4F,
    0xB0, 0xB8, 0xA0, 0xA8, 0x90, 0x98, 0x80, 0x88, 0xF0, 0xF8, 0xE0, 0xE8,
    0xD0, 0xD8, 0xC0, 0xC8,
    0x30, 0x38, 0x20, 0x28, 0x10, 0x18, 0x00, 0x08, 0x70, 0x78, 0x60, 0x68,
    0x50, 0x58, 0x40, 0x48,
    0xB1, 0xB9, 0xA1, 0xA9, 0x91, 0x99, 0x81, 0x89, 0xF1, 0xF9, 0xE1, 0xE9,
    0xD1, 0xD9, 0xC1, 0xC9,
    0x31, 0x39, 0x21, 0x29, 0x11, 0x19, 0x01, 0x09, 0x71, 0x79, 0x61, 0x69,
    0x51, 0x59, 0x41, 0x49,
    0xB2, 0xBA, 0xA2, 0xAA, 0x92, 0x9A, 0x82, 0x8A, 0xF2, 0xFA, 0xE2, 0xEA,
    0xD2, 0xDA, 0xC2, 0xCA,
    0x32, 0x3A, 0x22, 0x2A, 0x12, 0x1A, 0x02, 0x0A, 0x72, 0x7A, 0x62, 0x6A,
    0x52, 0x5A, 0x42, 0x4A,
    0xB3, 0xBB, 0xA3, 0xAB, 0x93, 0x9B, 0x83, 0x8B, 0xF3, 0xFB, 0xE3, 0xEB,
    0xD3, 0xDB, 0xC3, 0xCB,
    0x33, 0x3B, 0x23, 0x2B, 0x13, 0x1B, 0x03, 0x0B, 0x73, 0x7B, 0x63, 0x6B,
    0x53, 0x5B, 0x43, 0x4B
  };

  strcat (dest_name, basename (ucon64.rom));
  set_suffix (dest_name, ((OFFSET (dest_name, strlen (dest_name) - 2) == 'B' ||
                       OFFSET (dest_name, strlen (dest_name) - 2) == 'b') ? ".GBC" : ".GB"));

  ucon64_file_handler (dest_name, NULL, 0);
  q_fcpy (ucon64.rom, 0, rominfo->buheader_len, dest_name, "wb");

  x = 0;
  while ((c = q_fgetc (ucon64.rom, rominfo->buheader_len + x)) != -1)
    {
      q_fwrite (&gbx2gbc[c], rominfo->buheader_len + x, 1, dest_name, "ab");
      x++;
    }
  printf (ucon64_msg[WROTE], dest_name);

  return 0;
}


int
gameboy_sgb (st_rominfo_t *rominfo)
{
  long x;
  int c;
  char buf[MAXBUFSIZE];
  int gbc2gbx[] = {
    0x96, 0xB6, 0xD6, 0xF6, 0x16, 0x36, 0x56, 0x76, 0x97, 0xB7, 0xD7, 0xF7,
    0x17, 0x37, 0x57, 0x77,
    0x94, 0xB4, 0xD4, 0xF4, 0x14, 0x34, 0x54, 0x74, 0x95, 0xB5, 0xD5, 0xF5,
    0x15, 0x35, 0x55, 0x75,
    0x92, 0xB2, 0xD2, 0xF2, 0x12, 0x32, 0x52, 0x72, 0x93, 0xB3, 0xD3, 0xF3,
    0x13, 0x33, 0x53, 0x73,
    0x90, 0xB0, 0xD0, 0xF0, 0x10, 0x30, 0x50, 0x70, 0x91, 0xB1, 0xD1, 0xF1,
    0x11, 0x31, 0x51, 0x71,
    0x9E, 0xBE, 0xDE, 0xFE, 0x1E, 0x3E, 0x5E, 0x7E, 0x9F, 0xBF, 0xDF, 0xFF,
    0x1F, 0x3F, 0x5F, 0x7F,
    0x9C, 0xBC, 0xDC, 0xFC, 0x1C, 0x3C, 0x5C, 0x7C, 0x9D, 0xBD, 0xDD, 0xFD,
    0x1D, 0x3D, 0x5D, 0x7D,
    0x9A, 0xBA, 0xDA, 0xFA, 0x1A, 0x3A, 0x5A, 0x7A, 0x9B, 0xBB, 0xDB, 0xFB,
    0x1B, 0x3B, 0x5B, 0x7B,
    0x98, 0xB8, 0xD8, 0xF8, 0x18, 0x38, 0x58, 0x78, 0x99, 0xB9, 0xD9, 0xF9,
    0x19, 0x39, 0x59, 0x79,
    0x86, 0xA6, 0xC6, 0xE6, 0x06, 0x26, 0x46, 0x66, 0x87, 0xA7, 0xC7, 0xE7,
    0x07, 0x27, 0x47, 0x67,
    0x84, 0xA4, 0xC4, 0xE4, 0x04, 0x24, 0x44, 0x64, 0x85, 0xA5, 0xC5, 0xE5,
    0x05, 0x25, 0x45, 0x65,
    0x82, 0xA2, 0xC2, 0xE2, 0x02, 0x22, 0x42, 0x62, 0x83, 0xA3, 0xC3, 0xE3,
    0x03, 0x23, 0x43, 0x63,
    0x80, 0xA0, 0xC0, 0xE0, 0x00, 0x20, 0x40, 0x60, 0x81, 0xA1, 0xC1, 0xE1,
    0x01, 0x21, 0x41, 0x61,
    0x8E, 0xAE, 0xCE, 0xEE, 0x0E, 0x2E, 0x4E, 0x6E, 0x8F, 0xAF, 0xCF, 0xEF,
    0x0F, 0x2F, 0x4F, 0x6F,
    0x8C, 0xAC, 0xCC, 0xEC, 0x0C, 0x2C, 0x4C, 0x6C, 0x8D, 0xAD, 0xCD, 0xED,
    0x0D, 0x2D, 0x4D, 0x6D,
    0x8A, 0xAA, 0xCA, 0xEA, 0x0A, 0x2A, 0x4A, 0x6A, 0x8B, 0xAB, 0xCB, 0xEB,
    0x0B, 0x2B, 0x4B, 0x6B,
    0x88, 0xA8, 0xC8, 0xE8, 0x08, 0x28, 0x48, 0x68, 0x89, 0xA9, 0xC9, 0xE9,
    0x09, 0x29, 0x49, 0x69
  };

  strcat (buf, basename (ucon64.rom));
  set_suffix (buf, ((OFFSET (buf, strlen (buf) - 2) == 'B' ||
                 OFFSET (buf, strlen (buf) - 2) == 'b') ? ".GBX" : ".GX"));

  ucon64_file_handler (buf, NULL, 0);
  q_fwrite (ucon64.rom, 0, rominfo->buheader_len, buf, "wb");

  x = 0;
  while ((c = q_fgetc (ucon64.rom, rominfo->buheader_len + x)) != -1)
    {
      q_fwrite (&gbc2gbx[c], rominfo->buheader_len + x, 1, buf, "ab");
      x++;
    }
  printf (ucon64_msg[WROTE], buf);

  return 0;
}


int
gameboy_n (st_rominfo_t *rominfo, const char *name)
{
  char buf[MAXBUFSIZE], dest_name[FILENAME_MAX];

  memset (buf, 0, MAXBUFSIZE);
  strcpy (buf, name);
  strcpy (dest_name, ucon64.rom);
  if (!ucon64_file_handler (dest_name, NULL, 0))
    q_fcpy (ucon64.rom, 0, q_fsize (ucon64.rom), dest_name, "wb");
  q_fwrite (buf, GAMEBOY_HEADER_START + rominfo->buheader_len + 0x034, 16,
            dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gameboy_chk (st_rominfo_t *rominfo)
{
  char buf[4], dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.rom);
  if (!ucon64_file_handler (dest_name, NULL, 0))
    q_fcpy (ucon64.rom, 0, q_fsize (ucon64.rom), dest_name, "wb");

  q_fputc (dest_name,
              GAMEBOY_HEADER_START + rominfo->buheader_len + 0x4d,
              checksum.complement, "r+b");
  q_fputc (dest_name,
              GAMEBOY_HEADER_START + rominfo->buheader_len + 0x4e,
              (rominfo->current_internal_crc & 0xff00) >> 8, "r+b");
  q_fputc (dest_name, GAMEBOY_HEADER_START + rominfo->buheader_len + 0x4f,
              rominfo->current_internal_crc & 0xff, "r+b");

  q_fread (buf, GAMEBOY_HEADER_START + rominfo->buheader_len + 0x4d, 3, dest_name);

  mem_hexdump (buf, 3, GAMEBOY_HEADER_START + rominfo->buheader_len + 0x4d);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gameboy_mgd (st_rominfo_t *rominfo)
{
  char buf[FILENAME_MAX], dest_name[FILENAME_MAX], *p = NULL;

  if (!rominfo->buheader_len)
    {
      fprintf (stderr, "ERROR: Already in MGD format\n");
      return -1;
    }

  p = basename (ucon64.rom);
  strcpy (buf, is_func (p, strlen (p), isupper) ? "GB" : "gb");
  strcat (buf, p);
  if ((p = strrchr (buf, '.')))
    *p = 0;
  strcat (buf, "________");
  buf[7] = '_';
  buf[8] = 0;

  sprintf (dest_name, "%s.%03lu", buf,
           (unsigned long) ((q_fsize (ucon64.rom) - rominfo->buheader_len) / MBIT));
  ucon64_file_handler (dest_name, NULL, OF_FORCE_BASENAME);
  q_fcpy (ucon64.rom, rominfo->buheader_len, q_fsize (ucon64.rom), dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gameboy_ssc (st_rominfo_t *rominfo)
{
  char dest_name[FILENAME_MAX], *p = NULL;
  long size = q_fsize (ucon64.rom) - rominfo->buheader_len;

  if (rominfo->buheader_len != 0)
    {
      fprintf (stderr, "ERROR: Already in SSC format\n");
      return -1;
    }

  memset (&unknown_header, 0, UNKNOWN_HEADER_LEN);

  unknown_header.size_low = size / 8192;
  unknown_header.size_high = size / 8192 >> 8;
  unknown_header.id1 = 0xaa;
  unknown_header.id2 = 0xbb;
  unknown_header.type = 2;

  p = basename (ucon64.rom);
  strcpy (dest_name, is_func (p, strlen (p), isupper) ? "GB" : "gb");
  strcat (dest_name, p);
  set_suffix (dest_name, ".GB");

  ucon64_file_handler (dest_name, NULL, 0);
  q_fwrite (&unknown_header, 0, UNKNOWN_HEADER_LEN, dest_name, "wb");
  q_fcpy (ucon64.rom, 0, size, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gameboy_init (st_rominfo_t *rominfo)
{
  int result = -1, value, x;
  char buf[MAXBUFSIZE];
  static const char *gameboy_maker[0x100] = {
    NULL, "Nintendo", NULL, NULL, NULL,
    NULL, NULL, NULL, "Capcom", NULL,
    "Jaleco", "Coconuts", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "Hudson Soft",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, "Nintendo", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, "Spectrum Holobyte", NULL, "Irem", NULL,
    NULL, NULL, NULL, NULL, NULL,
    "Absolute Entertainment", "Acclaim", "Activision", "American Sammy", "Gametek",
    "Park Place", "LJN", NULL, NULL, NULL,
    "Bitmap Brothers/Mindscape", NULL, NULL, "Tradewest", NULL,
    NULL, "Titus", "Virgin", NULL, NULL,
    NULL, NULL, NULL, "Ocean", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, "ElectroBrain", "Infogrames", NULL, "Broderbund",
    NULL, NULL, NULL, NULL, NULL,
    NULL, "Accolade", "Triffix Entertainment", NULL, NULL,
    NULL, NULL, "Kemco", NULL, NULL,
    NULL, "Lozc", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "Bullet-Proof Software",
    "Vic Tokai", NULL, NULL, NULL, NULL,
    NULL, NULL, "Tsuburava", NULL, NULL,
    NULL, NULL, NULL, "ARC", NULL,
    NULL, "Imagineer", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "Konami",
    NULL, "Kawada", "Takara", NULL, "Technos Japan",
    "Broderbund", "Namcot", NULL, NULL, NULL,
    NULL, NULL, "ASCII/Nexoft", NULL, NULL,
    "Enix", NULL, "HAL", NULL, NULL,
    NULL, NULL, "SunSoft", NULL, "Imagesoft",
    NULL, "Sammy", "Taito", NULL, "Kemco",
    "SquareSoft", NULL, "Data East", "Tonkin House", NULL,
    NULL, NULL, "Palcom/Ultra", "VAP", NULL,
    NULL, "FCI/Pony Canyon", NULL, NULL, "Sofel",
    "Quest", NULL, NULL, NULL, NULL,
    NULL, NULL, "Banpresto", "Tomy", NULL,
    NULL, "NCS", NULL, "Altron", NULL,
    "Towachiki", NULL, NULL, NULL, "Epoch",
    NULL, NULL, "Asmik", NULL, "King Records",
    "Atlus", NULL, NULL, "IGS", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL},
  *gameboy_romtype[0x100] = {
    "ROM only",
    "ROM and MBC1",
    "ROM, MBC1 and RAM",
    "ROM, MBC1, RAM and Battery",
    NULL,
    "ROM and MBC2",
    "ROM, MBC2 and Battery",
    NULL,
    "ROM and RAM",
    "ROM, RAM and Battery",
    NULL,
    "ROM and MMM01",
    "ROM, MMM01 and RAM",
    "ROM, MMM01, RAM and Battery",
    NULL,
    "ROM, MBC3, Battery and TIMER",
    "ROM, MBC3, RAM, Battery and TIMER",
    "ROM and MBC3",
    "ROM, MBC3 and RAM",
    "ROM, MBC3, RAM and Battery",
    NULL,
    "ROM and MBC4",
    "ROM, MBC4 and RAM",
    "ROM, MBC4, RAM and Battery",
    NULL,
    "ROM and MBC5",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "ROM, MBC5 and RAM",
    "ROM, MBC5, RAM and Battery",
    "ROM, MBC5 and Rumble",
    "ROM, MBC5, RAM and Rumble",
    "ROM, MBC5, RAM, Battery and Rumble",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "Nintendo Pocket Camera",
    "Bandai TAMA5",
    "Hudson HuC-3",
    "Hudson HuC-1"};

  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ? ucon64.buheader_len : 0;

  q_fread (&gameboy_header, GAMEBOY_HEADER_START +
    rominfo->buheader_len, GAMEBOY_HEADER_LEN, ucon64.rom);
  if (OFFSET (gameboy_header, 0) == 0x00 && OFFSET (gameboy_header, 1) == 0xc3)
    result = 0;
  else
    {
      rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
        ucon64.buheader_len : SSC_HEADER_LEN;

      q_fread (&gameboy_header, GAMEBOY_HEADER_START +
        rominfo->buheader_len, GAMEBOY_HEADER_LEN, ucon64.rom);
      if (OFFSET (gameboy_header, 0) == 0x00 && OFFSET (gameboy_header, 1) == 0xc3)
        result = 0;
      else
        result = -1;
    }
  if (ucon64.console == UCON64_GB)
    result = 0;

  rominfo->header_start = GAMEBOY_HEADER_START;
  rominfo->header_len = GAMEBOY_HEADER_LEN;
  rominfo->header = &gameboy_header;

  // internal ROM name
  strncpy (rominfo->name, &OFFSET (gameboy_header, 0x34), 20);
  rominfo->name[20] = 0;                        // terminate string

  // ROM maker
  rominfo->maker = NULL_TO_UNKNOWN_S (gameboy_maker[OFFSET (gameboy_header, 0x4b)]);

  // ROM country
  rominfo->country = (OFFSET (gameboy_header, 0x4a) == 0) ? "Japan" : "U.S.A./Europe";

  // misc stuff
  sprintf (buf, "ROM type: %s\n", NULL_TO_UNKNOWN_S (gameboy_romtype[OFFSET (gameboy_header, 0x47)]));
  strcat (rominfo->misc, buf);

  value = OFFSET (gameboy_header, 0x49);
  if (!value)
    sprintf (buf, "Save RAM: No\n");
  else
    {
      value = (value & 0x03) * 2;
      value = (value ? (1 << (value - 1)) : 0);

      sprintf (buf, "Save RAM: Yes, %d kBytes\n", value);
    }
  strcat (rominfo->misc, buf);

  sprintf (buf, "Version: 1.%d\n", OFFSET (gameboy_header, 0x4c));
  strcat (rominfo->misc, buf);

  sprintf (buf, "Game Boy type: %s\n",
    (OFFSET (gameboy_header, 0x43) == 0x80) ? "Color" :
//    (OFFSET (gameboy_header, 0x46) == 0x3) ? "Super" :
    "Standard (4 Colors)");
  strcat (rominfo->misc, buf);

  value = 0;
  value += OFFSET (gameboy_header, 0x03) << 8;
  value += OFFSET (gameboy_header, 0x02);

  sprintf (buf, "Start address: %04x", value);
  strcat (rominfo->misc, buf);

  rominfo->console_usage = gameboy_usage;
  rominfo->copier_usage = (!rominfo->buheader_len ? mgd_usage : ssc_usage);

  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      rominfo->has_internal_crc = 1;
      rominfo->internal_crc_len = 2;
      rominfo->internal_crc2_len = 1;
      checksum = gameboy_chksum (rominfo);
      rominfo->current_internal_crc = checksum.value;

      rominfo->internal_crc =
        (q_fgetc (ucon64.rom, GAMEBOY_HEADER_START + rominfo->buheader_len + 0x4e) << 8) +
         q_fgetc (ucon64.rom, GAMEBOY_HEADER_START + rominfo->buheader_len + 0x4f);

      sprintf (buf,
               "Complement checksum: %%s, 0x%%0%dlx (calculated) %%s= 0x%%0%dlx (internal)",
               rominfo->internal_crc2_len * 2, rominfo->internal_crc2_len * 2);

      x = q_fgetc (ucon64.rom, GAMEBOY_HEADER_START + rominfo->buheader_len + 0x4d);
      sprintf (rominfo->internal_crc2, buf,
#ifdef  ANSI_COLOR
               ucon64.ansi_color ?
                 ((checksum.complement == x) ?
                   "\x1b[01;32mOk\x1b[0m" : "\x1b[01;31mBad\x1b[0m")
                 :
                 ((checksum.complement == x) ? "Ok" : "Bad"),
#else
               (checksum.complement == x) ? "Ok" : "Bad",
#endif
               checksum.complement,
               (checksum.complement == x) ? "=" : "!", x);
    }
  return result;
}


st_gameboy_chksum_t
gameboy_chksum (st_rominfo_t *rominfo)
{
  FILE *fh;
  st_gameboy_chksum_t sum = {0, 0};
  int ch, i = 0;

  if (!(fh = fopen (ucon64.rom, "rb")))
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.rom);
      exit (1);
    }

  fseek (fh, rominfo->buheader_len, SEEK_SET);
  while ((ch = fgetc (fh)) != EOF)
    {
      if (i != 0x014d && i != 0x014e && i != 0x014f)
        sum.value += ch;
      if (i >= 0x0134 && i < 0x014d)
        sum.complement += ch;
      i++;
    }
  fclose (fh);

  sum.complement = 0xe7 - sum.complement;
  sum.value += sum.complement;

  return sum;
}
