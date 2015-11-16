/*
gb.c - Game Boy support for uCON64

Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2001 - 2004 dbjh


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
#include <stdlib.h>
#include "misc/archive.h"
#include "misc/file.h"
#include "misc/misc.h"
#include "misc/string.h"
#include "ucon64_misc.h"
#include "console/console.h"
#include "console/gb.h"
#include "console/nes.h"
#include "backup/backup.h"
#include "backup/mgd.h"
#include "backup/ssc.h"


#define GB_HEADER_START 0x100
#define GB_HEADER_LEN (sizeof (st_gb_header_t))


static st_ucon64_obj_t gb_obj[] =
  {
    {0, WF_DEFAULT},
    {UCON64_GB, WF_SWITCH},
    {UCON64_GB, WF_DEFAULT}
  };

const st_getopt2_t gb_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Game Boy/(Super GB)/GB Pocket/Color GB"
      /*"1989/1994/1996/1998/2001 Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      UCON64_GB_S, 0, 0, UCON64_GB,
      NULL, "force recognition",
      &gb_obj[1]
    },
    {
      "n", 1, 0, UCON64_N,
      "NEW_NAME", "change internal ROM name to NEW_NAME",
      &gb_obj[0]
    },
    {
      "logo", 0, 0, UCON64_LOGO,
      NULL, "restore ROM logo character data (offset: 0x104-0x134)",
      &gb_obj[0]
    },
    {
      "mgd", 0, 0, UCON64_MGD,
      NULL, "convert to Multi Game*/MGD2/RAW",
      &gb_obj[0]
    },
    {
      "ssc", 0, 0, UCON64_SSC,
      NULL, "convert to Super Smart Card/SSC",
      &gb_obj[2]
    },
    {
      "sgb", 0, 0, UCON64_SGB,
      NULL, "convert from GB Xchanger/GB/GBC to Super Backup Card/GX/GBX",
      &gb_obj[2]
    },
    {
      "gbx", 0, 0, UCON64_GBX,
      NULL, "convert from Super Backup Card/GX/GBX to GB Xchanger/GB/GBC",
      &gb_obj[2]
    },
    {
      "n2gb", 1, 0, UCON64_N2GB,
      "NESROM", "KAMI's FC EMUlator (NES emulator);\n"
      "ROM should be KAMI's FC Emulator ROM image\n"
      "NESROM should contain 16 kB of PRG data and 8 kB of CHR data",
      &gb_obj[2]
    },
    {
      "chk", 0, 0, UCON64_CHK,
      NULL, "fix ROM checksum",
      &gb_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
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
typedef struct st_gb_header
{
  unsigned char opcode1;                        // 0x00 usually 0x00, NOP
  unsigned char opcode2;                        // 0x01 usually 0xc3, JP
  unsigned char start_low;                      // 0x02 first byte of parameter of JP
  unsigned char start_high;                     // 0x03 second byte
  unsigned char logo[GB_LOGODATA_LEN];          // 0x04
  unsigned char name[GB_NAME_LEN];              // 0x34
  unsigned char gb_type;                        // 0x43 last byte of name if not 0x80 or 0xc0
  unsigned char maker_high;                     // 0x44
  unsigned char maker_low;                      // 0x45
  unsigned char sgb_features;                   // 0x46
  unsigned char rom_type;                       // 0x47
  unsigned char rom_size;                       // 0x48
  unsigned char sram_size;                      // 0x49
  unsigned char country;                        // 0x4a
  unsigned char maker;                          // 0x4b
  unsigned char version;                        // 0x4c
  unsigned char header_checksum;                // 0x4d
  unsigned char checksum_high;                  // 0x4e
  unsigned char checksum_low;                   // 0x4f
} st_gb_header_t;

static st_gb_header_t gb_header;
const unsigned char gb_logodata[] =             // Note: not a static variable
  {
    0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d,
    0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
    0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99,
    0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc,
    0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e
  },
  rocket_logodata[] =                           // idem
  {
    0x11, 0x23, 0xf1, 0x1e, 0x01, 0x22, 0xf0, 0x00,
    0x08, 0x99, 0x78, 0x00, 0x08, 0x11, 0x9a, 0x48,
    0x11, 0x23, 0xf0, 0x0e, 0x70, 0x01, 0xf8, 0x80,
    0x22, 0x44, 0x44, 0x22, 0x22, 0x21, 0x00, 0x1e,
    0x99, 0x10, 0x00, 0x1e, 0x19, 0x22, 0x44, 0x22,
    0x22, 0x47, 0x00, 0x0e, 0x11, 0x22, 0x00, 0x00
  };

typedef struct st_gb_chksum
{
  unsigned short value;
  unsigned char header;
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
} st_gb_chksum_t;
#ifdef  _MSC_VER
#pragma warning(pop)
#endif

static st_gb_chksum_t checksum;
static st_gb_chksum_t gb_chksum (st_ucon64_nfo_t *rominfo);


int
gb_logo (st_ucon64_nfo_t *rominfo)
{
  char dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");
  ucon64_fwrite ((unsigned char *)
    ((gb_header.rom_type >= 0x97 && gb_header.rom_type <= 0x99) ?
      rocket_logodata : gb_logodata),
    rominfo->backup_header_len + GB_HEADER_START + 4, GB_LOGODATA_LEN,
    dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gb_n2gb (st_ucon64_nfo_t *rominfo, const char *nesrom)
{
#define EMULATOR_LEN 0x10000
  st_ines_header_t ines_header;
  int n = 0, crc = 0;
  unsigned char *buf;
  char dest_name[FILENAME_MAX];

  if (ucon64.file_size - rominfo->backup_header_len != EMULATOR_LEN)
    {
      fprintf (stderr, "ERROR: %s does not appear to be KAMI's FC emulator\n", ucon64.fname);
      return -1;
    }

  ucon64_fread (&ines_header, 0, INES_HEADER_LEN, nesrom);
  if (memcmp (ines_header.signature, INES_SIG_S, 4))
    {
      fprintf (stderr, "ERROR: Only NES ROMs with iNES header are supported\n");
      return -1;
    }
  if (ines_header.prg_size != 1 || ines_header.chr_size != 1)
    {
      fprintf (stderr,
               "ERROR: Only NES ROMs with 0.1250 Mb of ROM (PRG) and 0.0625 Mb of VROM (CHR)\n"
               "       are supported by KAMI's FC emulator\n");
      return -1;
    }

  if ((buf = (unsigned char *) malloc (ucon64.file_size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], ucon64.file_size);
      return -1;
    }
  ucon64_fread (buf, 0, ucon64.file_size, ucon64.fname);
  ucon64_fread (rominfo->backup_header_len + buf + 0x4000, INES_HEADER_LEN,
                0x4000 + 0x2000, nesrom);       // read PRG & CHR data

  for (n = 0; n < ucon64.file_size - rominfo->backup_header_len; n++)
    {
      if ((n == GB_HEADER_START + 0x4e) || (n == GB_HEADER_START + 0x4f))
        continue;
      else
        crc += buf[rominfo->backup_header_len + n];
    }

  buf[rominfo->backup_header_len + GB_HEADER_START + 0x4e] = (unsigned char) (crc >> 8);
  buf[rominfo->backup_header_len + GB_HEADER_START + 0x4f] = (unsigned char) crc;
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  ucon64_fwrite (buf, 0, ucon64.file_size, dest_name, "wb");

  free (buf);
  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


static int
gb_convert_data (st_ucon64_nfo_t *rominfo, unsigned char *conversion_table,
                 const char *suffix)
{
  char dest_name[FILENAME_MAX], src_name[FILENAME_MAX];
  unsigned char buf[MAXBUFSIZE];
  int x, n, n_bytes;

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, suffix);
  ucon64_file_handler (dest_name, src_name, 0);

  x = rominfo->backup_header_len;
  while ((n_bytes = ucon64_fread (buf, x, MAXBUFSIZE, src_name)) != 0)
    {
      for (n = 0; n < n_bytes; n++)
        buf[n] = conversion_table[(int) buf[n]];
      ucon64_fwrite (buf, x, n_bytes, dest_name, x == rominfo->backup_header_len ? "wb" : "ab");
      x += n_bytes;
    }

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gb_gbx (st_ucon64_nfo_t *rominfo)
{
  unsigned char gbx2gbc[] =
    {
      0xB4, 0xBC, 0xA4, 0xAC, 0x94, 0x9C, 0x84, 0x8C,
      0xF4, 0xFC, 0xE4, 0xEC, 0xD4, 0xDC, 0xC4, 0xCC,
      0x34, 0x3C, 0x24, 0x2C, 0x14, 0x1C, 0x04, 0x0C,
      0x74, 0x7C, 0x64, 0x6C, 0x54, 0x5C, 0x44, 0x4C,
      0xB5, 0xBD, 0xA5, 0xAD, 0x95, 0x9D, 0x85, 0x8D,
      0xF5, 0xFD, 0xE5, 0xED, 0xD5, 0xDD, 0xC5, 0xCD,
      0x35, 0x3D, 0x25, 0x2D, 0x15, 0x1D, 0x05, 0x0D,
      0x75, 0x7D, 0x65, 0x6D, 0x55, 0x5D, 0x45, 0x4D,
      0xB6, 0xBE, 0xA6, 0xAE, 0x96, 0x9E, 0x86, 0x8E,
      0xF6, 0xFE, 0xE6, 0xEE, 0xD6, 0xDE, 0xC6, 0xCE,
      0x36, 0x3E, 0x26, 0x2E, 0x16, 0x1E, 0x06, 0x0E,
      0x76, 0x7E, 0x66, 0x6E, 0x56, 0x5E, 0x46, 0x4E,
      0xB7, 0xBF, 0xA7, 0xAF, 0x97, 0x9F, 0x87, 0x8F,
      0xF7, 0xFF, 0xE7, 0xEF, 0xD7, 0xDF, 0xC7, 0xCF,
      0x37, 0x3F, 0x27, 0x2F, 0x17, 0x1F, 0x07, 0x0F,
      0x77, 0x7F, 0x67, 0x6F, 0x57, 0x5F, 0x47, 0x4F,
      0xB0, 0xB8, 0xA0, 0xA8, 0x90, 0x98, 0x80, 0x88,
      0xF0, 0xF8, 0xE0, 0xE8, 0xD0, 0xD8, 0xC0, 0xC8,
      0x30, 0x38, 0x20, 0x28, 0x10, 0x18, 0x00, 0x08,
      0x70, 0x78, 0x60, 0x68, 0x50, 0x58, 0x40, 0x48,
      0xB1, 0xB9, 0xA1, 0xA9, 0x91, 0x99, 0x81, 0x89,
      0xF1, 0xF9, 0xE1, 0xE9, 0xD1, 0xD9, 0xC1, 0xC9,
      0x31, 0x39, 0x21, 0x29, 0x11, 0x19, 0x01, 0x09,
      0x71, 0x79, 0x61, 0x69, 0x51, 0x59, 0x41, 0x49,
      0xB2, 0xBA, 0xA2, 0xAA, 0x92, 0x9A, 0x82, 0x8A,
      0xF2, 0xFA, 0xE2, 0xEA, 0xD2, 0xDA, 0xC2, 0xCA,
      0x32, 0x3A, 0x22, 0x2A, 0x12, 0x1A, 0x02, 0x0A,
      0x72, 0x7A, 0x62, 0x6A, 0x52, 0x5A, 0x42, 0x4A,
      0xB3, 0xBB, 0xA3, 0xAB, 0x93, 0x9B, 0x83, 0x8B,
      0xF3, 0xFB, 0xE3, 0xEB, 0xD3, 0xDB, 0xC3, 0xCB,
      0x33, 0x3B, 0x23, 0x2B, 0x13, 0x1B, 0x03, 0x0B,
      0x73, 0x7B, 0x63, 0x6B, 0x53, 0x5B, 0x43, 0x4B
    };
  const char *old_suffix = get_suffix (ucon64.fname), *new_suffix;
  new_suffix = stricmp (old_suffix, ".GBX") ? ".GB" : ".GBC";
  return gb_convert_data (rominfo, gbx2gbc, new_suffix);
}


int
gb_sgb (st_ucon64_nfo_t *rominfo)
{
  unsigned char gbc2gbx[] =
    {
      0x96, 0xB6, 0xD6, 0xF6, 0x16, 0x36, 0x56, 0x76,
      0x97, 0xB7, 0xD7, 0xF7, 0x17, 0x37, 0x57, 0x77,
      0x94, 0xB4, 0xD4, 0xF4, 0x14, 0x34, 0x54, 0x74,
      0x95, 0xB5, 0xD5, 0xF5, 0x15, 0x35, 0x55, 0x75,
      0x92, 0xB2, 0xD2, 0xF2, 0x12, 0x32, 0x52, 0x72,
      0x93, 0xB3, 0xD3, 0xF3, 0x13, 0x33, 0x53, 0x73,
      0x90, 0xB0, 0xD0, 0xF0, 0x10, 0x30, 0x50, 0x70,
      0x91, 0xB1, 0xD1, 0xF1, 0x11, 0x31, 0x51, 0x71,
      0x9E, 0xBE, 0xDE, 0xFE, 0x1E, 0x3E, 0x5E, 0x7E,
      0x9F, 0xBF, 0xDF, 0xFF, 0x1F, 0x3F, 0x5F, 0x7F,
      0x9C, 0xBC, 0xDC, 0xFC, 0x1C, 0x3C, 0x5C, 0x7C,
      0x9D, 0xBD, 0xDD, 0xFD, 0x1D, 0x3D, 0x5D, 0x7D,
      0x9A, 0xBA, 0xDA, 0xFA, 0x1A, 0x3A, 0x5A, 0x7A,
      0x9B, 0xBB, 0xDB, 0xFB, 0x1B, 0x3B, 0x5B, 0x7B,
      0x98, 0xB8, 0xD8, 0xF8, 0x18, 0x38, 0x58, 0x78,
      0x99, 0xB9, 0xD9, 0xF9, 0x19, 0x39, 0x59, 0x79,
      0x86, 0xA6, 0xC6, 0xE6, 0x06, 0x26, 0x46, 0x66,
      0x87, 0xA7, 0xC7, 0xE7, 0x07, 0x27, 0x47, 0x67,
      0x84, 0xA4, 0xC4, 0xE4, 0x04, 0x24, 0x44, 0x64,
      0x85, 0xA5, 0xC5, 0xE5, 0x05, 0x25, 0x45, 0x65,
      0x82, 0xA2, 0xC2, 0xE2, 0x02, 0x22, 0x42, 0x62,
      0x83, 0xA3, 0xC3, 0xE3, 0x03, 0x23, 0x43, 0x63,
      0x80, 0xA0, 0xC0, 0xE0, 0x00, 0x20, 0x40, 0x60,
      0x81, 0xA1, 0xC1, 0xE1, 0x01, 0x21, 0x41, 0x61,
      0x8E, 0xAE, 0xCE, 0xEE, 0x0E, 0x2E, 0x4E, 0x6E,
      0x8F, 0xAF, 0xCF, 0xEF, 0x0F, 0x2F, 0x4F, 0x6F,
      0x8C, 0xAC, 0xCC, 0xEC, 0x0C, 0x2C, 0x4C, 0x6C,
      0x8D, 0xAD, 0xCD, 0xED, 0x0D, 0x2D, 0x4D, 0x6D,
      0x8A, 0xAA, 0xCA, 0xEA, 0x0A, 0x2A, 0x4A, 0x6A,
      0x8B, 0xAB, 0xCB, 0xEB, 0x0B, 0x2B, 0x4B, 0x6B,
      0x88, 0xA8, 0xC8, 0xE8, 0x08, 0x28, 0x48, 0x68,
      0x89, 0xA9, 0xC9, 0xE9, 0x09, 0x29, 0x49, 0x69
    };
  const char *old_suffix = get_suffix (ucon64.fname), *new_suffix;
  new_suffix = stricmp (old_suffix, ".GBC") ? ".GX" : ".GBX";
  return gb_convert_data (rominfo, gbc2gbx, new_suffix);
}


int
gb_n (st_ucon64_nfo_t *rominfo, const char *name)
{
  char buf[GB_NAME_LEN + 1], dest_name[FILENAME_MAX];
  int gb_name_len =
    (gb_header.gb_type == 0x80 || gb_header.gb_type == 0xc0) ?
      GB_NAME_LEN : GB_NAME_LEN + 1;

  memset (buf, 0, gb_name_len);
  strncpy (buf, name, gb_name_len);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");
  ucon64_fwrite (buf, rominfo->backup_header_len + GB_HEADER_START + 0x34,
                 gb_name_len, dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gb_chk (st_ucon64_nfo_t *rominfo)
{
  char buf[4], dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");

  buf[0] = checksum.header;
  buf[1] = (char) (rominfo->current_internal_crc >> 8);
  buf[2] = (char) rominfo->current_internal_crc;
  ucon64_fwrite (buf, rominfo->backup_header_len + GB_HEADER_START + 0x4d, 3,
                 dest_name, "r+b");

  dumper (stdout, buf, 3, GB_HEADER_START + rominfo->backup_header_len + 0x4d, DUMPER_HEX);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gb_mgd (st_ucon64_nfo_t *rominfo)
// TODO: convert the ROM data
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  int size = ucon64.file_size - rominfo->backup_header_len;

  strcpy (src_name, ucon64.fname);
  mgd_make_name (ucon64.fname, UCON64_GB, size, dest_name);
  ucon64_file_handler (dest_name, src_name, OF_FORCE_BASENAME);

  fcopy (src_name, rominfo->backup_header_len, size, dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  mgd_write_index_file ((char *) basename2 (dest_name), 1);
  return 0;
}


int
gb_ssc (st_ucon64_nfo_t *rominfo)
// TODO: convert the ROM data
{
  st_unknown_backup_header_t header;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  const char *p = NULL;
  int size = ucon64.file_size - rominfo->backup_header_len;

  memset (&header, 0, UNKNOWN_BACKUP_HEADER_LEN);

  header.size_low = (unsigned char) (size / 8192);
  header.size_high = (unsigned char) (size / 8192 >> 8);
  header.id1 = 0xaa;
  header.id2 = 0xbb;
#if 0 // TODO: find out correct value. 2 is used for Magic Super Griffin
  header.type = 2;
#endif

  strcpy (src_name, ucon64.fname);
  p = basename2 (ucon64.fname);
  // TODO: find out if this is correct (giving the file name a prefix)
  if ((p[0] == 'G' || p[0] == 'g') && (p[1] == 'B' || p[1] == 'b'))
    strcpy (dest_name, p);
  else
    sprintf (dest_name, "gb%s", p);
  set_suffix (dest_name, ".gb");

  ucon64_file_handler (dest_name, src_name, 0);
  ucon64_fwrite (&header, 0, UNKNOWN_BACKUP_HEADER_LEN, dest_name, "wb");
  fcopy (src_name, rominfo->backup_header_len, size, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
gb_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1, value, x;
  char buf[MAXBUFSIZE];
  static const char *gb_romtype1[0x20] =
    {
      "ROM only",
      "ROM + MBC1",
      "ROM + MBC1 + RAM",
      "ROM + MBC1 + RAM + Battery",
      NULL,
      "ROM + MBC2",
      "ROM + MBC2 + Battery",
      NULL,
      "ROM + RAM",                              // correct? - dbjh
      "ROM + RAM + Battery",                    // correct? - dbjh
      NULL,
      "ROM + MMM01",
      "ROM + MMM01 + SRAM",
      "ROM + MMM01 + SRAM + Battery",
      NULL,
      "ROM + MBC3 + Battery + Timer",
      "ROM + MBC3 + RAM + Battery + Timer",
      "ROM + MBC3",
      "ROM + MBC3 + RAM",
      "ROM + MBC3 + RAM + Battery",
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      "ROM + MBC5",
      "ROM + MBC5 + RAM",
      "ROM + MBC5 + RAM + Battery",
      "ROM + MBC5 + Rumble",
      "ROM + MBC5 + SRAM + Rumble",
      "ROM + MBC5 + SRAM + Battery + Rumble",
      "Nintendo Pocket Camera"
    },
    *gb_romtype2[3] =
    {
      "Rocket Games",
      NULL,
      "Rocket Games 2-in-1"
    },
    *gb_romtype3[3] =
    {
      "Bandai TAMA5",
      "Hudson HuC-3",
      "Hudson HuC-1"
    },
    *str;

  rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ? ucon64.backup_header_len : 0;

  if (ucon64.file_size <
      (int) (rominfo->backup_header_len + GB_HEADER_START + GB_HEADER_LEN))
    return -1;                                  // Don't continue if it makes no sense

  ucon64_fread (&gb_header, rominfo->backup_header_len + GB_HEADER_START,
                GB_HEADER_LEN, ucon64.fname);
  if (gb_header.opcode1 == 0x00 && gb_header.opcode2 == 0xc3)
    result = 0;
  else
    {
      rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ?
        ucon64.backup_header_len : (int) SSC_HEADER_LEN;

      ucon64_fread (&gb_header, rominfo->backup_header_len + GB_HEADER_START,
                    GB_HEADER_LEN, ucon64.fname);
      if (gb_header.opcode1 == 0x00 && gb_header.opcode2 == 0xc3)
        result = 0;
      else
        result = -1;
    }
  if (ucon64.console == UCON64_GB)
    result = 0;

  rominfo->header_start = GB_HEADER_START;
  rominfo->header_len = GB_HEADER_LEN;
  rominfo->header = &gb_header;

  // internal ROM name
  x = (gb_header.gb_type == 0x80 || gb_header.gb_type == 0xc0) ?
        GB_NAME_LEN : GB_NAME_LEN + 1;
  strncpy (rominfo->name, (const char *) gb_header.name, x);
  rominfo->name[x] = 0;                         // terminate string

  // ROM maker
  if (gb_header.maker == 0x33)
    {
      int ih = gb_header.maker_high <= '9' ?
                 gb_header.maker_high - '0' : gb_header.maker_high - 'A' + 10,
          il = gb_header.maker_low <= '9' ?
                 gb_header.maker_low - '0' : gb_header.maker_low - 'A' + 10;
      x = ih * 36 + il;
    }
  else
    x = (gb_header.maker >> 4) * 36 + (gb_header.maker & 0x0f);

  /*
    I added the first if statement, because I didn't want to expand
    nintendo_maker by a large amount for only one publisher and because index 0
    is used when the publisher code is unknown (so it shouldn't be set to
    "Rocket Games"). - dbjh
  */
  if (x == 33 * 36 + 33 || x == 0)              // publisher code XX/00
    x = 2;                                      // Rocket Games
  else if (x < 0 || x >= NINTENDO_MAKER_LEN)
    x = 0;
  rominfo->maker = NULL_TO_UNKNOWN_S (nintendo_maker[x]);

  // ROM country
  rominfo->country = gb_header.country == 0 ? "Japan" : "U.S.A. & Europe";

  // misc stuff
  // Don't move division by 4 to shift parameter (gb_header.rom_size can be < 2)
  sprintf (buf, "Internal size: %.4f Mb\n", (1 << gb_header.rom_size) / 4.0f);
  strcat (rominfo->misc, buf);

  if (gb_header.rom_type <= 0x1f)
    str = NULL_TO_UNKNOWN_S (gb_romtype1[gb_header.rom_type]);
  else if (gb_header.rom_type >= 0x97 && gb_header.rom_type <= 0x99)
    str = gb_romtype2[gb_header.rom_type - 0x97];
  else if (gb_header.rom_type >= 0xfd)
    str = gb_romtype3[gb_header.rom_type - 0xfd];
  else
    str = "Unknown";
  sprintf (buf, "ROM type: %s\n", str);
  strcat (rominfo->misc, buf);

  if (!gb_header.sram_size)
    sprintf (buf, "Save RAM: No\n");
  else
    {
      value = (gb_header.sram_size & 7) << 1; // 0/1/2/4/5
      if (value)
        value = 1 << (value - 1);

      sprintf (buf, "Save RAM: Yes, %d kBytes\n", value);
    }
  strcat (rominfo->misc, buf);

  sprintf (buf, "Version: 1.%d\n", gb_header.version);
  strcat (rominfo->misc, buf);

  if (gb_header.gb_type == 0x80)
    {
      if (gb_header.sgb_features == 3)
        str = "Game Boy/Super Game Boy (SGB features present)/Game Boy Color";
      else
        str = "Game Boy/Super Game Boy/Game Boy Color";
    }
  else if (gb_header.gb_type == 0xc0)
    str = "Game Boy Color";                     // GBC _only_
  else
    {
      if (gb_header.sgb_features == 3)
        str = "Game Boy/Super Game Boy (SGB features present)";
      else
        str = "Game Boy/Super Game Boy";
    }
  sprintf (buf, "Game type: %s\n", str);
  strcat (rominfo->misc, buf);

  value = gb_header.start_high << 8;
  value += gb_header.start_low;
  sprintf (buf, "Start address: 0x%04x\n", value);
  strcat (rominfo->misc, buf);

  strcat (rominfo->misc, "Logo data: ");
  if (memcmp (gb_header.logo,
              (gb_header.rom_type >= 0x97 && gb_header.rom_type <= 0x99) ?
                rocket_logodata : gb_logodata,
              GB_LOGODATA_LEN) == 0)
    {
#ifdef  USE_ANSI_COLOR
      if (ucon64.ansi_color)
        strcat (rominfo->misc, "\x1b[01;32mOK\x1b[0m");
      else
#endif
        strcat (rominfo->misc, "OK");
    }
  else
    {
#ifdef  USE_ANSI_COLOR
      if (ucon64.ansi_color)
        strcat (rominfo->misc, "\x1b[01;31mBad\x1b[0m");
      else
#endif
        strcat (rominfo->misc, "Bad");
    }

  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      rominfo->has_internal_crc = 1;
      rominfo->internal_crc_len = 2;
      checksum = gb_chksum (rominfo);
      rominfo->current_internal_crc = checksum.value;

      rominfo->internal_crc = (gb_header.checksum_high << 8) +
                              gb_header.checksum_low;

      x = gb_header.header_checksum;
      sprintf (rominfo->internal_crc2,
               "Header checksum: %s, 0x%02x (calculated) %c= 0x%02x (internal)",
#ifdef  USE_ANSI_COLOR
               ucon64.ansi_color ?
                 ((checksum.header == x) ?
                   "\x1b[01;32mOK\x1b[0m" : "\x1b[01;31mBad\x1b[0m")
                 :
                 ((checksum.header == x) ? "OK" : "Bad"),
#else
               (checksum.header == x) ? "OK" : "Bad",
#endif
               checksum.header, (checksum.header == x) ? '=' : '!', x);
    }

  rominfo->console_usage = gb_usage[0].help;
  rominfo->backup_usage = (!rominfo->backup_header_len ? mgd_usage[0].help : ssc_usage[0].help);

  return result;
}


st_gb_chksum_t
gb_chksum (st_ucon64_nfo_t *rominfo)
{
  st_gb_chksum_t sum = { 0, 0 };
  unsigned char *rom_buffer;
  int size = ucon64.file_size - rominfo->backup_header_len, i;

  if ((rom_buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      return sum;
    }
  ucon64_fread (rom_buffer, rominfo->backup_header_len, size, ucon64.fname);

  for (i = GB_HEADER_START + 0x34; i < GB_HEADER_START + 0x4d; i++)
    sum.header += ~rom_buffer[i];
  for (i = 0; i < size; i++)
    sum.value += rom_buffer[i];
  sum.value -= (rom_buffer[GB_HEADER_START + 0x4d] - sum.header) +
               rom_buffer[GB_HEADER_START + 0x4e] +
               rom_buffer[GB_HEADER_START + 0x4f];

  free (rom_buffer);

  return sum;
}
