/*
snes.c - Super NES support for uCON64

Copyright (c) 1999 - 2002              NoisyB
Copyright (c) 2001 - 2005, 2015 - 2019 dbjh
Copyright (c) 2002 - 2003              John Weidman
Copyright (c) 2004                     JohnDie


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
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <ctype.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <sys/stat.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "misc/archive.h"
#include "misc/bswap.h"
#include "misc/chksum.h"
#include "misc/file.h"
#include "misc/misc.h"
#include "misc/string.h"
#include "misc/term.h"
#include "ucon64_misc.h"
#include "console/console.h"
#include "console/snes.h"
#include "backup/backup.h"
#include "backup/gd.h"
#include "backup/mgd.h"
#include "backup/mgh.h"
#include "backup/smcic2.h"
#include "backup/swc.h"
#include "backup/ufo.h"
#include "backup/ufosd.h"


#define SNES_HEADER_LEN (sizeof (st_snes_header_t))
#define SNES_NAME_LEN 21
#define GD3_HEADER_MAPSIZE 0x18
#define NSRT_HEADER_VERSION 22                  // version 2.2 header
#define DETECT_NOTGOOD_DUMPS                    // makes _a_ complete GoodSNES 0.999.5 set detected
#define DETECT_SMC_COM_FUCKED_UP_LOROM          // adds support for interleaved LoROMs
#define DETECT_INSNEST_FUCKED_UP_LOROM          // only adds support for its 24 Mbit
                                                //  interleaved LoROM "format"
//#define PAD_40MBIT_GD3_DUMPS                  // padding works for
                                                //  Dai Kaiju Monogatari 2 (J)
// block sizes used for calculating the SRAM checksum of special chip games on
//  the SNES/Super Famicom Classic Mini
#define INITIAL_MAX_BLOCK_SIZE 0x2000
#define DEFAULT_MAX_BLOCK_SIZE 0x280
#define MIN_BLOCK_SIZE 0xff

static int snes_chksum (st_ucon64_nfo_t *rominfo, unsigned char *rom_buffer,
                        size_t rom_size);
static int snes_deinterleave (st_ucon64_nfo_t *rominfo, unsigned char **rom_buffer,
                              size_t rom_size);
static int snes_check_bs (void);
static int check_banktype (unsigned char *rom_buffer, int header_offset);
static void reset_header (void *header);
static void set_nsrt_info (st_ucon64_nfo_t *rominfo, unsigned char *header);
static void get_nsrt_info (unsigned char *rom_buffer, int header_start,
                           unsigned char *backup_header);
static void handle_nsrt_header (st_ucon64_nfo_t *rominfo, unsigned char *header,
                                const char **snes_country);


static st_ucon64_obj_t snes_obj[] =
  {
    {0, WF_SWITCH},
    {0, WF_DEFAULT},
    {0, WF_DEFAULT | WF_NO_SPLIT},
    {0, WF_INIT | WF_PROBE},
    {0, WF_INIT | WF_PROBE | WF_STOP},
    {0, WF_INIT | WF_PROBE | WF_NO_SPLIT},
    {UCON64_SNES, WF_SWITCH},
    {UCON64_SNES, WF_DEFAULT},
    {UCON64_SNES, WF_DEFAULT | WF_NO_SPLIT},
    {UCON64_SNES, WF_NO_ROM},
    {UCON64_SNES, WF_INIT | WF_PROBE}
  };

const st_getopt2_t snes_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Super Nintendo Entertainment System/SNES/Super Famicom"
      /*"1990 Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      UCON64_SNES_S, 0, 0, UCON64_SNES,
      NULL, "force recognition",
      &snes_obj[6]
    },
    {
      "hi", 0, 0, UCON64_HI,
      NULL, "force ROM is HiROM",
      &snes_obj[6]
    },
    {
      "nhi", 0, 0, UCON64_NHI,
      NULL, "force ROM is not HiROM",
      &snes_obj[6]
    },
    {
      "erom", 0, 0, UCON64_EROM,
      NULL, "force ROM is \"Extended\" (combine with " OPTION_LONG_S "hi for Extended HiROM)",
      &snes_obj[6]
    },
#if 0
    {
      "hd", 0, 0, UCON64_HD,
      NULL, "force ROM has SMC/FIG/SWC header (+512 Bytes)",
      &snes_obj[0]
    },
    {
      "nhd", 0, 0, UCON64_NHD,
      NULL, "force ROM has no SMC/FIG/SWC header (MGD2/MGH/RAW)",
      &snes_obj[0]
    },
    {
      "ns", 0, 0, UCON64_NS,
      NULL, "force ROM is not split",
      &snes_obj[0]
    },
#endif
    {
      "int", 0, 0, UCON64_INT,
      NULL, "force ROM is in interleaved format (GD3/UFO)",
      &snes_obj[0]
    },
    {
      "int2", 0, 0, UCON64_INT2,
      NULL, "force ROM is in interleaved format 2 (SFX)",
      &snes_obj[0]
    },
    {
      "nint", 0, 0, UCON64_NINT,
      NULL, "force ROM is not in interleaved format",
      &snes_obj[0]
    },
    {
      "bs", 0, 0, UCON64_BS,
      NULL, "force ROM is a Broadcast Satellaview dump",
      &snes_obj[6]
    },
    {
      "nbs", 0, 0, UCON64_NBS,
      NULL, "force ROM is a regular cartridge dump",
      &snes_obj[6]
    },
    {
      "n", 1, 0, UCON64_N,
      "NEW_NAME", "change internal ROM name to NEW_NAME",
      &snes_obj[1]
    },
    {
      "fig", 0, 0, UCON64_FIG,
      NULL, "convert to *Pro Fighter*/FIG",
      &snes_obj[8]
    },
    {
      "figs", 0, 0, UCON64_FIGS,
      NULL, "convert *.srm (SRAM) file to *Pro Fighter*/FIG",
      &snes_obj[10]
    },
    {
      "gd3", 0, 0, UCON64_GD3,
      NULL, "convert to Game Doctor SF3(SF6/SF7)/Professor SF(SF 2)",
      &snes_obj[8]
    },
    {
      "id", 0, 0, UCON64_ID,
      NULL, "force " OPTION_LONG_S "gd3 to produce a unique filename",
      &snes_obj[6]
    },
    {
      "idnum", 1, 0, UCON64_IDNUM,
      "NUM", "make " OPTION_LONG_S "gd3 produce filenames where first file has numerical\n"
      "identifier NUM, next NUM + 1, etc.",
      &snes_obj[6]
    },
    {
      "gd3s", 0, 0, UCON64_GD3S,
      NULL, "convert *.srm (SRAM) file to GD SF3(SF6/SF7)/Professor SF*",
      &snes_obj[10]
    },
    {
      "mgd", 0, 0, UCON64_MGD,
      NULL, "convert to Multi Game Doctor 2/MGD2/RAW",
      &snes_obj[2]
    },
    {
      "mgh", 0, 0, UCON64_MGH,
      NULL, "convert to Multi Game Hunter/MGH",
      &snes_obj[2]
    },
    {
      "smc", 0, 0, UCON64_SMC,
      NULL, "convert to Super Magicom/SMC",
      &snes_obj[8]
    },
    {
      "swc", 0, 0, UCON64_SWC,
      NULL, "convert to Super Wild Card*/SWC",
      &snes_obj[8]
    },
    {
      "swcs", 0, 0, UCON64_SWCS,
      NULL, "convert *.srm (SRAM) file to Super Wild Card*/SWC",
      &snes_obj[10]
    },
    {
      "ufo", 0, 0, UCON64_UFO,
      NULL, "convert to Super UFO Super Drive PRO 7/8",
      &snes_obj[8]
    },
    {
      "ufos", 0, 0, UCON64_UFOS,
      NULL, "convert *.srm (SRAM) file to Super UFO Super Drive PRO 7/8",
      &snes_obj[10]
    },
    {
      "ufosd", 0, 0, UCON64_UFOSD,
      NULL, "convert to Super UFO Pro 8 SD",
      &snes_obj[8]
    },
    {
      "ufosds", 0, 0, UCON64_UFOSDS,
      NULL, "convert *.srm (SRAM) file to Super UFO Pro 8 SD",
      &snes_obj[10]
    },
    {
      "ic2", 0, 0, UCON64_IC2,
      NULL, "convert to Super Magicom/SMC IC2",
      &snes_obj[8]
    },
#ifdef  USE_ZLIB
    {
      "sminis", 1, 0, UCON64_SMINIS,
      "ID", "convert *.srm (SRAM) file to SNES/Super Famicom Classic Mini\n"
      "(hakchi2) format for game with identifier ID",
      &snes_obj[10]
    },
#endif
    {
      "range", 1, 0, UCON64_RANGE,
      "O1:O2", "specify range for calculating checksum of SNES/Super Famicom\n"
      "Classic Mini (hakchi2) SRAM data\n"
      "O1" OPTARG_S "offset (hexadecimal) of first byte of range\n"
      "O2" OPTARG_S "offset (hexadecimal) of last byte of range",
      &snes_obj[6]
    },
    {
      "smini2srm", 0, 0, UCON64_SMINI2SRM,
      NULL, "convert SNES/Super Famicom Classic Mini (hakchi2) SRAM data to\n"
      "*.srm (SRAM) file",
      &snes_obj[7]
    },
    {
      "ctrl", 1, 0, UCON64_CTRL,
      "TYPE", "specify type of controller in port 1 for emu when converting\n"
      "TYPE" OPTARG_S "0 gamepad\n"
      "TYPE" OPTARG_S "1 mouse\n"
      "TYPE" OPTARG_S "2 mouse / gamepad\n"
      "TYPE" OPTARG_S "6 multitap",
      &snes_obj[0]
    },
    {
      "ctrl2", 1, 0, UCON64_CTRL2,
      "TYPE", "specify type of controller in port 2 for emu when converting\n"
      "TYPE" OPTARG_S "0 gamepad\n"
      "TYPE" OPTARG_S "1 mouse\n"
      "TYPE" OPTARG_S "2 mouse / gamepad\n"
      "TYPE" OPTARG_S "3 super scope\n"
      "TYPE" OPTARG_S "4 super scope / gamepad\n"
      "TYPE" OPTARG_S "5 Konami's justifier\n"
      "TYPE" OPTARG_S "6 multitap\n"
      "TYPE" OPTARG_S "7 mouse / super scope / gamepad",
      &snes_obj[6]
    },
    {
      "stp", 0, 0, UCON64_STP,
      NULL, "convert SRAM from backup unit for use with an emulator\n"
      OPTION_LONG_S "stp just strips the first 512 bytes",
      NULL
    },
    {
      "dbuh", 0, 0, UCON64_DBUH,
      NULL, "display (relevant part of) backup unit header and interpret it",
      &snes_obj[7]
    },
    {
      "dint", 0, 0, UCON64_DINT,
      NULL, "deinterleave ROM (regardless whether the ROM is interleaved)",
      &snes_obj[5]
    },
    {
      "col", 1, 0, UCON64_COL,
      "0xCOLOR", "convert 0xRRGGBB (HTML) <-> 0xXXXX (SNES)"
      /*"this routine was used to find green colors in games and\n"
      "to replace them with red colors (blood mode)"*/,
      &snes_obj[9]
    },
    {
      "j", 0, 0, UCON64_J,
      NULL, "join split ROM",
      &snes_obj[3]
    },
    {
      "s", 0, 0, UCON64_S,
      NULL, "split ROM (not for UFO Pro 8 SD); default part size is 8 Mb",
      &snes_obj[2]
    },
    {
      "smgh", 0, 0, UCON64_SMGH,
      NULL, "split ROM for Multi Game Hunter/MGH",
      &snes_obj[2]
    },
    {
      "ssize", 1, 0, UCON64_SSIZE,
      "SIZE", "specify split part size in Mbit (not for Game Doctor SF3)",
      &snes_obj[0]
    },
#if 0
    {
      "p", 0, 0, UCON64_P,
      NULL, "pad ROM to full Mb",
      &snes_obj[1]
    },
#endif
    {
      "k", 0, 0, UCON64_K,
      NULL, "remove protection (crack)",
      &snes_obj[7]
    },
    {
      "f", 0, 0, UCON64_F,
      NULL, "remove NTSC/PAL protection",
      &snes_obj[1]
    },
    {
      "l", 0, 0, UCON64_L,
      NULL, "remove SlowROM checks",
      &snes_obj[7]
    },
    {
      "chk", 0, 0, UCON64_CHK,
      NULL, "fix ROM checksum",
      &snes_obj[1]
    },
    {
      "multi", 1, 0, UCON64_MULTI,
      "SIZE", "make multi-game file for use with Super Flash flash card,\n"
      "truncated to SIZE Mbit; file with loader must be specified\n"
      "first, then all the ROMs, multi-game file to create last",
      &snes_obj[4]
    },
    {
      "dmirr", 0, 0, UCON64_DMIRR,
      NULL, "\"de-mirror\" ROM (strip mirrored block from end of ROM)",
      &snes_obj[7]
    },
    {
      "dnsrt", 0, 0, UCON64_DNSRT,
      NULL, "\"de-NSRT\" ROM (restore name and checksum from NSRT header)",
      &snes_obj[7]
    },
    {
      "mksrm", 0, 0, UCON64_MKSRM,
      NULL, "create *.srm (SRAM) file with size based on ROM information",
      &snes_obj[8]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

typedef struct st_snes_header
{
  unsigned char maker_high;                     // 0
  unsigned char maker_low;                      // 1
  unsigned char game_id_prefix;                 // 2
  unsigned char game_id_low;                    // 3
  unsigned char game_id_high;                   // 4
  unsigned char game_id_country;                // 5
  // 'E' = USA, 'F' = France, 'G' = Germany, 'J' = Japan, 'P' = Europe, 'S' = Spain
  unsigned char pad1[7];                        // 6
  unsigned char sfx_sram_size;                  // 13
  unsigned char pad2[2];                        // 14
  unsigned char name[SNES_NAME_LEN];            // 16
  unsigned char map_type;                       // 37, a.k.a. ROM makeup
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union
#endif
  union
  {
    unsigned char rom_type;                     // 38
    unsigned char bs_month;                     // release date, month
  };
  union
  {
    unsigned char rom_size;                     // 39
    unsigned char bs_day;                       // release date, day
  };
  union
  {
    unsigned char sram_size;                    // 40
    unsigned char bs_map_type;
  };
  union
  {
    unsigned char country;                      // 41
    unsigned char bs_type;
  };
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
  unsigned char maker;                          // 42
  unsigned char version;                        // 43
  /*
    If we combine the following 4 bytes in 2 short int variables,
    inverse_checksum and checksum, they will have an incorrect value on big
    endian machines.
  */
  unsigned char inverse_checksum_low;           // 44
  unsigned char inverse_checksum_high;          // 45
  unsigned char checksum_low;                   // 46
  unsigned char checksum_high;                  // 47
} st_snes_header_t;

typedef struct st_split_info
{
  int nparts;
  struct st_part
  {
    char *fname;
    size_t size;
  } parts[2];
} st_split_info_t;

typedef struct st_ustar_header
{
  char name[100];
  char file_mode[8];
  char uid[8];
  char gid[8];
  char file_size[12];
  char mtime[12];
  char header_checksum[8];
  char file_type;
  char link_name[100];
  char magic[8];
  char uname[32];
  char gname[32];
  char dev_major[8];
  char dev_minor[8];
  char name_prefix[155];
  char pad[12];
} st_ustar_header_t;

static st_snes_header_t snes_header;
static int snes_sram_size, snes_sfx_sram_size, snes_header_base, snes_hirom,
           snes_hirom_ok, nsrt_header, bs_dump, st_dump;
static snes_copier_t copier_type;
static st_split_info_t split_info = { 0, { { 0, 0 }, { 0, 0 } } };

static unsigned char gd3_hirom_8mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22
  };
static unsigned char gd3_hirom_16mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21,
    0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21,
    0x22, 0x23, 0x22, 0x23, 0x22, 0x23, 0x22, 0x23
  };
static unsigned char gd3_hirom_24mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x21, 0x22, 0x00, 0x20, 0x21, 0x22, 0x00,
    0x20, 0x21, 0x22, 0x00, 0x20, 0x21, 0x22, 0x00,
    0x24, 0x25, 0x23, 0x00, 0x24, 0x25, 0x23, 0x00
  };
static unsigned char gd3_hirom_32mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23,
    0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x24, 0x25, 0x26, 0x27
  };
// map for Dai Kaiju Monogatari 2 (J)
static unsigned char gd3_hirom_40mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x22, 0x23, 0x24, 0x25, 0x22, 0x23, 0x24, 0x25,
    0x21, 0x21, 0x21, 0x21, 0x26, 0x27, 0x28, 0x29
  };
// map for Tales of Phantasia (J)
static unsigned char gd3_hirom_48mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x40, 0x40,
    0x24, 0x25, 0x26, 0x27, 0x24, 0x25, 0x26, 0x27,
    0x22, 0x23, 0x40, 0x40, 0x28, 0x29, 0x2a, 0x2b
  };

static unsigned char gd3_lorom_4mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
  };
static unsigned char gd3_lorom_8mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21,
    0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21,
    0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21
  };
static unsigned char gd3_lorom_16mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23,
    0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23,
    0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23
  };
static unsigned char gd3_lorom_32mb_map[GD3_HEADER_MAPSIZE] =
  {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x24, 0x25, 0x26, 0x27, 0x24, 0x25, 0x26, 0x27
  };


int
snes_get_snes_hirom (void)
{
  return snes_hirom;
}


snes_copier_t
snes_get_copier_type (void)
{
  return copier_type;
}


static unsigned char
get_internal_size (void)
{
  unsigned int size = !bs_dump ?
                        (1 << snes_header.rom_size) >> 7 :
                        8 - (snes_header.bs_type >> (4 + 1)) * 4;
  return size <= 128 ? (unsigned char) size : 128;
}


static size_t
get_rom_data_size (size_t backup_header_len)
{
  return copier_type != IC2 ?
           (size_t) ucon64.file_size - backup_header_len :
           2 * (split_info.parts[1].size - backup_header_len);
}


static void
read_rom_data (unsigned char *buffer, size_t start, size_t length,
               const char *filename)
{
  if (copier_type != IC2)
    ucon64_fread (buffer, start, length, filename);
  else
    {
      ucon64_fread (buffer, start, length / 2, filename);
      ucon64_fread (buffer + length / 2, start, length / 2,
                    split_info.parts[1].fname);
    }
}


static unsigned int
get_header_start (st_ucon64_nfo_t *rominfo, unsigned int size)
{
  unsigned int header_start;

  if (rominfo->interleaved)
    header_start = SNES_HEADER_START + (snes_hirom ?
                     (copier_type == UFO ? snes_header_base / 2U : 0) :
                     size / 2);                 // (Ext.) HiROM : LoROM
  else if (st_dump)                             // ignore interleaved ST dumps
    header_start = 8 * MBIT;
  else
    header_start = rominfo->header_start;
  return header_start;
}


static unsigned int
update_chksum (st_ucon64_nfo_t *rominfo, unsigned char *sum, unsigned int size,
               const char *dest_name)
{
  unsigned int header_start = get_header_start (rominfo, size);

  /*
    The internal checksum bytes have been included in the checksum calculation,
    but they may be changed after this function returns. Usually the bytes add
    up to 0x1fe (because the bytes consist of the checksum and its inverse
    (one's complement)), but they may have been corrupted. We account for that.
    Otherwise we would have to run uCON64 on the ROM twice.
  */
  rominfo->current_internal_crc += -(snes_header.inverse_checksum_low +
                                     snes_header.inverse_checksum_high +
                                     snes_header.checksum_low +
                                     snes_header.checksum_high) +
                                   2 * 0xff; // + 2 * 0;
  // change inverse checksum
  sum[0] = (unsigned char) ~rominfo->current_internal_crc; // low byte
  sum[1] = (unsigned char) (~rominfo->current_internal_crc >> 8); // high byte
  // change checksum
  sum[2] = (unsigned char) rominfo->current_internal_crc; // low byte
  sum[3] = (unsigned char) (rominfo->current_internal_crc >> 8); // high byte
  ucon64_fwrite (sum, header_start + rominfo->backup_header_len + 44, 4,
                 dest_name, "r+b");

  return header_start;
}


int
snes_col (const char *color)
/*
The Nintendo Super Famicom is capable of displaying 256 colors from a palette
of 32768. These 256 colors are split into 8 palettes of 32 colors each.

To change the colors the following needs to be done:

Loading the palette control register ($2121) with the color number you wish to
change (0-255, 0=background).
Then load the color into the palette data register first the low 8 bits,
followed by the high 7 bits (this gives you the maximum 32768 colors possible
$0000-$7fff).

Color data is made up of 3 components (Red,Green,Blue) each of 5 bits (The
Amiga uses exactly the same system, but only using 4 bits per component).
Saying that, Nintendo decided that R,G,B wasn't alphabetically correct and so
opted to store the bits as B,G,R.

                      00000 00000 00000
                      \   / \   / \   /
                       \ /   \ /   \ /
                        B     G     R

Examples:
~~~~~~~~~
         11111 00000 00000 = $7C00 (Bright Blue)
         00000 11111 00000 = $03E0 (Bright Green)
         00000 00000 11111 = $001F (Bright Red)
         00000 00000 00000 = $0000 (Black)
         11111 11111 11111 = $7FFF (White)

Remember to load the lowest 8 bits first, then the top 7 bits.
*/
{
  int r, g, b;
  unsigned int col;

  sscanf (color, "%x", &col);

  r = (col & 0xff0000) >> 16;
  g = (col & 0xff00) >> 8;
  b = col & 0xff;

  printf ("0x%02x%02x%02x (html) == ", r, g, b);

  r = (r * 0x1f) / 0xff;
  g = (g * 0x1f) / 0xff;
  b = (b * 0x1f) / 0xff;

  col = b;
  col = col << 5;
  col = col + g;
  col = col << 5;
  col = col + r;

  printf ("0x%02x%02x (snes)\n", col & 0xff, (col & 0x7f00) / 0x100);

  sscanf (color, "%x", &col);

  if (col < 0xff7f + 1)
    {
      printf ("0x%04x (snes) == ", col & 0xff7f);

      col = ((col & 0x7f) * 0x100) + ((col & 0xff00) / 0x100);

      r = col & 0x1f;
      g = (col & (0x1f << 5)) >> 5;
      b = (col & (0x1f << 10)) >> 10;

      r = r * 0xff / 0x1f;
      g = g * 0xff / 0x1f;
      b = b * 0xff / 0x1f;

      printf ("0x%02x%02x%02x (html)\n", r, g, b);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
snes_convert_sramfile (size_t org_header_len, const void *new_header)
{
  FILE *srcfile, *destfile;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], buffer[32 * 1024];
  size_t block_size, byteswritten, new_header_len;

  strcpy (src_name, ucon64.fname);
  if (new_header)
    {
      new_header_len = SWC_HEADER_LEN;
      strcpy (dest_name, ucon64.fname);
      set_suffix (dest_name, ".sav");
    }
  else // code for Game Doctor SRAM file
    {
      int n;

      new_header_len = 0;
      snprintf (dest_name, 7, "SF8%.3s__", basename2 (ucon64.fname));
      dest_name[6] = '\0';
      strupr (dest_name);
      // avoid trouble with filenames containing spaces
      for (n = 3; n < 6; n++)                   // skip "SF" and first digit
        if (dest_name[n] == ' ' || dest_name[n] == '.')
          dest_name[n] = '_';
      set_suffix (dest_name, ".B00");
    }
  ucon64_file_handler (dest_name, src_name, 0);

  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      fclose (srcfile);
      return -1;
    }

  fseek (srcfile, org_header_len, SEEK_SET);
  if (new_header)
    byteswritten = fwrite (new_header, 1, new_header_len, destfile); // write header
  else
    byteswritten = 0;

  block_size = fread (buffer, 1, 32 * 1024, srcfile); // read 32 kB at max
  while (byteswritten < 32 * 1024 + new_header_len)
    {
      // Pad SRAM data to 32 kB by repeating it. At least the SWC DX2 does
      //  something similar.
      if (byteswritten + block_size > 32 * 1024 + new_header_len)
        block_size = 32 * 1024 + new_header_len - byteswritten;
      fwrite (buffer, 1, block_size, destfile);
      byteswritten += block_size;
    }

  fclose (srcfile);
  fclose (destfile);
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
snes_swcs (st_ucon64_nfo_t *rominfo)
{
  st_swc_header_t header;

  memset (&header, 0, SWC_HEADER_LEN);
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 5;                              // size needn't be set for the SWC
                                                //  (SWC itself doesn't set it either)
  return snes_convert_sramfile (rominfo->backup_header_len, &header);
}


int
snes_figs (st_ucon64_nfo_t *rominfo)
{
  st_fig_header_t header;

  memset (&header, 0, FIG_HEADER_LEN);
  header.size_low = 4;                          // 32 kB == 4*8 kB, size_high is already 0

  return snes_convert_sramfile (rominfo->backup_header_len, &header);
}


int
snes_ufos (st_ucon64_nfo_t *rominfo)
{
  st_ufo_header_t header;

  memset (&header, 0, UFO_HEADER_LEN);
  memcpy (&header.id, "SUPERUFO", 8);

  return snes_convert_sramfile (rominfo->backup_header_len, &header);
}


static void
set_ufosd_sram_pattern (char *buffer, int size)
// Fill buffer with the pattern that the Super UFO Pro 8 SD uses to pad SRAM
//  files. The pattern repeats after 128 bytes.
{
  uint32_t pattern;
  char *pattern_ptr = (char *) &pattern;
  int n = 0;

  pattern_ptr[0] = 0;
  pattern_ptr[1] = 0x5a;                        // 0101 1010
  pattern_ptr[2] = 0 - 4;
  pattern_ptr[3] = 0x5a + 4;
  while (n < size / 4)
    {
      ((uint32_t *) buffer)[n] = pattern;
      pattern_ptr[0] -= 8;
      pattern_ptr[1] -= 8;
      pattern_ptr[2] -= 8;
      pattern_ptr[3] -= 8;
      n++;
      if (n % 8 == 0)
        {
          pattern_ptr[1] -= 0x40;
          pattern_ptr[3] -= 0x40;
        }
      else if (n % 4 == 0)
        {
          pattern_ptr[1] += 0x40;
          pattern_ptr[3] += 0x40;
        }
    }
}


int
snes_ufosds (st_ucon64_nfo_t *rominfo)
{
  FILE *srcfile, *destfile;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], buffer[32 * 1024];
  unsigned int byteswritten = 0, n;

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".sav");
  ucon64_file_handler (dest_name, src_name, 0);

  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      fclose (srcfile);
      return -1;
    }

  fseek (srcfile, rominfo->backup_header_len, SEEK_SET);
  while (byteswritten < MBIT &&
         (n = fread (buffer, 1, sizeof buffer, srcfile)) != 0)
    byteswritten += fwrite (buffer, 1, n, destfile);

  // fill buffer with pattern
  set_ufosd_sram_pattern (buffer, sizeof buffer);

  // pad file to 128 kB with pattern
  n = sizeof buffer;
  while (byteswritten < MBIT)
    {
      if (byteswritten + n > MBIT)
        n = MBIT - byteswritten;
      fwrite (buffer, 1, n, destfile);
      byteswritten += n;
    }

  fclose (srcfile);
  fclose (destfile);
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
snes_gd3s (st_ucon64_nfo_t *rominfo)
{
  return snes_convert_sramfile (rominfo->backup_header_len, NULL);
}


static unsigned int
ustar_chksum (const st_ustar_header_t *header)
{
  unsigned int n, sum = 0;

  for (n = 0; n < sizeof (st_ustar_header_t); n++)
    sum += ((unsigned char *) header)[n];
  for (n = 0; n < 8; n++)
    sum -= (unsigned char) header->header_checksum[n];
  sum += 8 * ' ';
  return sum;
}


#ifdef  USE_ZLIB
static void
set_ustar_header (st_ustar_header_t *header, const char *filename, int mode,
                  size_t file_size, time_t mtime)
{
  strncpy (header->name, filename, sizeof header->name - 1)
    [sizeof header->name - 1] = '\0';

  snprintf (header->file_mode, sizeof header->file_mode, "%07o", mode);
  header->file_mode[sizeof header->file_mode - 1] = '\0';

  strcpy (header->uid, "0000000");
  strcpy (header->gid, "0000000");

#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4777)
/*
  In this case Visual Studio Community 2015 warns with: '_snprintf' : format
  string '%011o' requires an argument of type 'unsigned int', but variadic
  argument 1 has type 'size_t'
  This is a bug in Visual Studio Community 2015.
*/
#endif
  snprintf (header->file_size, sizeof header->file_size, "%011o",
            (unsigned int) file_size);
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
  header->file_size[sizeof header->file_size - 1] = '\0';

  snprintf (header->mtime, sizeof header->mtime, "%011o", (unsigned int) mtime);
  header->mtime[sizeof header->mtime - 1] = '\0';

  header->file_type = file_size > 0 ? '0' : '5';

  memset (header->link_name, 0, sizeof header->link_name);
  strcpy (header->magic, "ustar  ");
  strncpy (header->uname, "root", sizeof header->uname);
  strncpy (header->gname, "root", sizeof header->gname);
  memset (header->dev_major, 0, sizeof header->dev_major);
  memset (header->dev_minor, 0, sizeof header->dev_minor);
  memset (header->name_prefix, 0, sizeof header->name_prefix);
  memset (header->pad, 0, sizeof header->pad);

  snprintf (header->header_checksum, 7, "%06o", ustar_chksum (header));
  header->header_checksum[6] = '\0';
  header->header_checksum[7] = ' ';
}
#endif


//#define DISPLAY_USTAR_HEADER
#ifdef  DISPLAY_USTAR_HEADER
static void
ustar_field_mkprint (const char *field, size_t size, char *buffer)
{
  size_t n;

  for (n = 0; n < size - 1; n++)
    buffer[n] = isprint ((int) field[n]) ? field[n] : '.';
  buffer[n] = '\0';
}


static void
ustar_string_mkprint (const char *field, size_t size, char *buffer)
{
  size_t n;

  for (n = 0; n < size - 1 && field[n] != '\0'; n++)
    buffer[n] = isprint ((int) field[n]) ? field[n] : '.';
  buffer[n] = '\0';
}


static void
display_ustar_header (const st_ustar_header_t *header)
{
  char buffer[155];

  fputc ('\n', stdout);

  ustar_string_mkprint (header->name, sizeof header->name, buffer);
  printf ("name: \"%s\"\n", buffer);

  ustar_field_mkprint (header->file_mode, sizeof header->file_mode, buffer);
  printf ("file_mode: %03o (%s)\n",
          (unsigned int) strtol (header->file_mode, NULL, 8), buffer);

  ustar_field_mkprint (header->uid, sizeof header->uid, buffer);
  printf ("uid: %u (%s)\n", (unsigned int) strtol (header->uid, NULL, 8), buffer);

  ustar_field_mkprint (header->gid, sizeof header->gid, buffer);
  printf ("gid: %u (%s)\n", (unsigned int) strtol (header->gid, NULL, 8), buffer);

  ustar_field_mkprint (header->file_size, sizeof header->file_size, buffer);
  printf ("file_size: %u (%s)\n",
          (unsigned int) strtol (header->file_size, NULL, 8), buffer);

  {
    char str[20];
    time_t file_mtype = (time_t) strtol (header->mtime, NULL, 8);

    ustar_field_mkprint (header->mtime, sizeof header->mtime, buffer);
    strftime (str, sizeof str, "%Y-%m-%d %H:%M:%S", localtime (&file_mtype));
    printf ("mtime: %s (%s)\n", str, buffer);
  }
  {
    ustar_field_mkprint (header->header_checksum,
                         sizeof header->header_checksum, buffer);
    buffer[6] = '\0';
    printf ("header_checksum: %s, %s\n",
            (unsigned int) strtol (header->header_checksum, NULL, 8) ==
              ustar_chksum (header) ? "OK" : "Bad", buffer);
  }

  buffer[0] = isprint ((int) header->file_type) ? header->file_type : '.';
  printf ("file_type: %c\n", buffer[0]);

  ustar_string_mkprint (header->link_name, sizeof header->link_name, buffer);
  printf ("link_name: \"%s\"\n", header->link_name);

  ustar_field_mkprint (header->magic, sizeof header->magic, buffer);
  printf ("magic: \"%s\"\n", buffer);

  ustar_string_mkprint (header->uname, sizeof header->uname, buffer);
  printf ("uname: \"%s\"\n", buffer);

  ustar_string_mkprint (header->gname, sizeof header->gname, buffer);
  printf ("gname: \"%s\"\n", buffer);

  printf ("dev_major: %u\n", (unsigned int) strtol (header->dev_major, NULL, 8));
  printf ("dev_minor: %u\n", (unsigned int) strtol (header->dev_minor, NULL, 8));

  ustar_string_mkprint (header->name_prefix, sizeof header->name_prefix, buffer);
  printf ("name_prefix: \"%s\"\n", buffer);
}
#endif


static int
set_smini_chksum_range (unsigned int *base, unsigned int *size,
                        unsigned int sram_data_size, int check_max_block_size)
{
  int range_start_set = UCON64_ISSET (ucon64.range_start),
      range_length_set = UCON64_ISSET2 (ucon64.range_length, size_t);
  *base = range_start_set ? ucon64.range_start : 0;
  *size = range_length_set ? ucon64.range_length : sram_data_size - *base;

  // largest valid offset is sram_data_size - 1
  if (sram_data_size && *base >= sram_data_size)
    {
      fprintf (stderr,
               "ERROR: A range was specified where the offset of the first byte is beyond the\n"
               "       end of the SRAM data (0x%04x > 0x%04x)\n", *base,
               sram_data_size - 1);
      return -1;
    }
  if (*base + *size > sram_data_size)
    {
      fprintf (stderr,
               "ERROR: A range was specified where the offset of the last byte is beyond the\n"
               "       end of the SRAM data (0x%04x > 0x%04x)\n", *base + *size - 1,
               sram_data_size - 1);
      return -1;
    }
  if (check_max_block_size && range_start_set && range_length_set &&
      ((*base == 0 && *size > INITIAL_MAX_BLOCK_SIZE && *size < sram_data_size) ||
       (*base > 0 && (*size < MIN_BLOCK_SIZE || *size > DEFAULT_MAX_BLOCK_SIZE ||
                      (*base % 0x100 != 0 && *base % 0x100 != 0x12)))))
    printf ("WARNING: A range was specified that uCON64 will not automatically detect.\n"
            "         uCON64 will not be able to correctly calculate the SRAM checksum\n"
            "         unless you specify " OPTION_LONG_S "range=%04x:%04x whenever it operates on this\n"
            "         file\n", *base, *base + *size - 1);
  return 0;
}


#ifdef  USE_ZLIB
int
snes_sminis (st_ucon64_nfo_t *rominfo, const char *id)
{
  unsigned char *buffer;
  int sram_data_size;
  s_sha1_ctx_t sha1_ctx;
  char dest_name[FILENAME_MAX], buffer2[sizeof (st_ustar_header_t)];
  FILE *destfile;
  st_ustar_header_t header;
  struct stat fstate;
  unsigned int base, size;

  if ((buffer = (unsigned char *) malloc (128 * 1024 + 20)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], 128 * 1024 + 20);
      exit (1);
    }
  sram_data_size = ucon64_fread (buffer, rominfo->backup_header_len,
                                 128 * 1024, ucon64.fname);
  if (sram_data_size <= 0)
    {
      fprintf (stderr, ucon64_msg[READ_ERROR], ucon64.fname);
      free (buffer);
      return -1;
    }
  if (set_smini_chksum_range (&base, &size, sram_data_size, 1))
    {
      free (buffer);
      return -1;
    }
  sha1_begin (&sha1_ctx);
  sha1 (&sha1_ctx, buffer + base, size);
  sha1_end (buffer + sram_data_size, &sha1_ctx);

  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".clvs");
  ucon64_file_handler (dest_name, NULL, 0);

  // NOTE: We actually open a gzip stream by specifying a compression level.
  if ((destfile = fopen (dest_name, "wb9")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      free (buffer);
      return -1;
    }

  // directory
  snprintf (buffer2, sizeof header.name, "%s/", id);
  buffer2[sizeof header.name - 1] = '\0';
  set_ustar_header (&header, buffer2, 0755, 0, time (NULL));
#ifdef  DISPLAY_USTAR_HEADER
  display_ustar_header (&header);
#endif
  fwrite (&header, 1, sizeof header, destfile);

  // hash file (optional, same data is also appended to SRAM file)
  snprintf (buffer2, sizeof header.name, "%s/cartridge.sram.hash", id);
  buffer2[sizeof header.name - 1] = '\0';
  set_ustar_header (&header, buffer2, 0644, 20, time (NULL));
#ifdef  DISPLAY_USTAR_HEADER
  display_ustar_header (&header);
#endif
  fwrite (&header, 1, sizeof header, destfile);
  fwrite (buffer + sram_data_size, 1, 20, destfile);
  // pad to multiple of st_ustar_header_t
  memset (buffer2, 0, sizeof buffer2 - 20);
  fwrite (buffer2, 1, sizeof buffer2 - 20, destfile);

  // SRAM file (including the 20-byte SHA-1 hash)
  snprintf (buffer2, sizeof header.name, "%s/cartridge.sram", id);
  buffer2[sizeof header.name - 1] = '\0';
  stat (ucon64.fname, &fstate);
  set_ustar_header (&header, buffer2, 0644, sram_data_size + 20, fstate.st_mtime);
#ifdef  DISPLAY_USTAR_HEADER
  display_ustar_header (&header);
#endif
  fwrite (&header, 1, sizeof header, destfile);
  fwrite (buffer, 1, sram_data_size + 20, destfile);
  // pad to multiple of st_ustar_header_t
  memset (buffer2, 0, sizeof buffer2);
  fwrite (buffer2, 1, sizeof buffer2 - (sram_data_size + 20) % sizeof buffer2,
          destfile);
  // EOF is marked by 2 empty blocks
  fwrite (buffer2, 1, sizeof buffer2, destfile);
  fwrite (buffer2, 1, sizeof buffer2, destfile);

  fclose (destfile);

  printf (ucon64_msg[WROTE], dest_name);
  free (buffer);
  return 0;
}
#endif


static int
get_smini_sram_data (const char *src_name, unsigned char *sram_data,
                     size_t max_sram_data_size, unsigned char *sram_chksum)
{
  FILE *srcfile;
  size_t file_pos = 0, sram_data_size = 0;

  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }

  if (ucon64.file_size >= 3 * sizeof (st_ustar_header_t))
    for (;;)
      {
        st_ustar_header_t header;
        size_t file_size, bytesread = fread (&header, 1, sizeof header, srcfile);
        char *p;

        file_pos += bytesread;
        if (bytesread < sizeof header)
          break;
#ifdef  DISPLAY_USTAR_HEADER
        display_ustar_header (&header);
#endif

        if ((unsigned int) strtol (header.header_checksum, NULL, 8) !=
              ustar_chksum (&header))
          break;

        file_size = (unsigned int) strtol (header.file_size, NULL, 8);
        header.name[sizeof header.name - 1] = '\0';
        p = strrchr (header.name, '/');
        /*
          header->name is 100 bytes large, but should also store the string
          terminator, which leaves 99 bytes for the name. There should be
          enough space after the slash to store the string
          "cartridge.sram.hash" (19 bytes). We are comparing offsets, so
          subtract 1 more (the highest valid offset for a character is 98),
          hence > instead of >=.
        */
        if (p != NULL && sizeof header.name - 1 - (p - header.name) > 19)
          {
            if (!strcmp (p + 1, "cartridge.sram"))
              sram_data_size = fread (sram_data, 1,
                                      file_size > max_sram_data_size ?
                                        max_sram_data_size : file_size,
                                      srcfile);
            else if (sram_chksum && !strcmp (p + 1, "cartridge.sram.hash"))
              fread (sram_chksum, 1, file_size > 20 ? 20 : file_size, srcfile);
          }

        // align on 512-byte boundary (size of st_ustar_header_t)
        file_pos = (file_pos + file_size + 0x1ff) & ~0x1ff;
        if (file_pos >= ucon64.file_size - 2 * sizeof (st_ustar_header_t))
          break;
        fseek (srcfile, file_pos, SEEK_SET);
      }

  fclose (srcfile);
  return sram_data_size;
}


int
snes_smini2srm (void)
{
  unsigned char *buffer;
  int sram_data_size = 0;

  if ((buffer = (unsigned char *) malloc (128 * 1024 + 20)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], 128 * 1024 + 20);
      exit (1);
    }

  sram_data_size = get_smini_sram_data (ucon64.fname, buffer, 128 * 1024 + 20,
                                        NULL);
  if (sram_data_size > 20)
    {
      char dest_name[FILENAME_MAX];

      strcpy (dest_name, ucon64.fname);
      set_suffix (dest_name, ".srm");
      ucon64_file_handler (dest_name, NULL, 0);

      if (ucon64_fwrite (buffer, 0, sram_data_size - 20, dest_name, "wb") <
            sram_data_size - 20)
        {
          fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name);
          free (buffer);
          return -1;
        }

      printf (ucon64_msg[WROTE], dest_name);
      free (buffer);
      return 0;
    }
  else
    {
      fputs ("ERROR: Cannot find SRAM data\n", stderr);
      free (buffer);
      return -1;
    }
}


static void
write_deinterleaved_data (st_ucon64_nfo_t *rominfo, const char *src_name,
                          const char *dest_name, size_t backup_header_len)
{
  // backup_header_len is the backup unit header length of the destination file
  unsigned char *buffer;
  size_t size = get_rom_data_size (rominfo->backup_header_len);

  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  read_rom_data (buffer, rominfo->backup_header_len, size, src_name);
  snes_deinterleave (rominfo, &buffer, size);
  ucon64_fwrite (buffer, backup_header_len, size, dest_name,
                 backup_header_len ? "ab" : "wb");
  free (buffer);
}


int
snes_dint (st_ucon64_nfo_t *rominfo)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  size_t backup_header_len = rominfo->backup_header_len > SWC_HEADER_LEN ?
                               SWC_HEADER_LEN : rominfo->backup_header_len;

  puts ("Converting to deinterleaved format...");

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".tmp");
  ucon64_file_handler (dest_name, src_name, 0);

  if (!rominfo->interleaved)
    puts ("WARNING: Deinterleaving a ROM that was not detected as being interleaved");
  fcopy (src_name, 0, backup_header_len, dest_name, "wb");
  write_deinterleaved_data (rominfo, src_name, dest_name, backup_header_len);

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
snes_ffe (st_ucon64_nfo_t *rominfo, char *suffix)
{
  st_swc_header_t header;
  size_t size = get_rom_data_size (rominfo->backup_header_len);
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  ucon64_fread (&header, 0, rominfo->backup_header_len > SWC_HEADER_LEN ?
                  SWC_HEADER_LEN : rominfo->backup_header_len, ucon64.fname);
  reset_header (&header);
  header.size_low = (unsigned char) (size / 8192);
  header.size_high = (unsigned char) (size / 8192 >> 8);

  header.emulation = snes_hirom ? 0x30 : 0;
  // bit 3 & 2 are already OK for 32 kB SRAM size
  if (snes_sram_size == 8 * 1024)
    header.emulation |= 0x04;
  else if (snes_sram_size == 2 * 1024)
    header.emulation |= 0x08;
  else if (snes_sram_size == 0)
    header.emulation |= 0x0c;

  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 4;

  set_nsrt_info (rominfo, (unsigned char *) &header);

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, suffix);
  ucon64_file_handler (dest_name, src_name, 0);

  ucon64_fwrite (&header, 0, SWC_HEADER_LEN, dest_name, "wb");
  if (rominfo->interleaved)
    write_deinterleaved_data (rominfo, src_name, dest_name, SWC_HEADER_LEN);
  else
    fcopy (src_name, rominfo->backup_header_len, size, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


// header format is specified in src/backup/ffe.h
int
snes_smc (st_ucon64_nfo_t *rominfo)
{
  if ((bs_dump ? snes_header.bs_map_type : snes_header.map_type) & 0x10)
    puts ("NOTE: This game may not work with a Super Magicom because it is a FastROM game");

  return snes_ffe (rominfo, ".smc");
}


// header format is specified in src/backup/ffe.h
int
snes_swc (st_ucon64_nfo_t *rominfo)
{
  return snes_ffe (rominfo, ".swc");
}


// header format is specified in src/backup/fig.h
void
snes_set_fig_header (st_fig_header_t *header, size_t backup_header_len)
{
  size_t size = get_rom_data_size (backup_header_len);
  int uses_DSP;

  reset_header (header);
  header->size_low = (unsigned char) (size / 8192);
  header->size_high = (unsigned char) (size / 8192 >> 8);
  header->hirom = snes_hirom ? 0x80 : 0;

  uses_DSP = snes_header.rom_type == 3 || snes_header.rom_type == 5 ||
             snes_header.rom_type == 0xf6;

  if ((snes_header.rom_type & 0xf0) == 0x10)    // uses FX(2) chip
    {
      header->emulation1 = 0x11;
      header->emulation2 = 0x02;
    }
  else
    {
#if 0                                           // reset_header() set all fields to 0
      header->emulation1 = 0;                   // default value for LoROM dumps
      if (snes_sram_size == 32 * 1024)
        header->emulation2 = 0;
      else
#endif
      if (snes_sram_size == 8 * 1024 || snes_sram_size == 2 * 1024)
        header->emulation2 = 0x80;
      else if (snes_sram_size == 0)
        {
          header->emulation1 = 0x77;
          header->emulation2 = 0x83;
        }

      if (snes_hirom)
        {
          header->emulation2 |= 2;
          if (uses_DSP)
            header->emulation1 |= 0xf0;
          if (snes_sram_size)
            header->emulation1 |= 0xdd;
        }
      else if (uses_DSP)                        // LoROM
        {
          header->emulation1 &= 0x0f;
          header->emulation1 |= 0x40;           // LoROM && SRAM == 0 && DSP => 0x47
        }
    }
}


int
snes_fig (st_ucon64_nfo_t *rominfo)
{
  st_fig_header_t header;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  ucon64_fread (&header, 0, rominfo->backup_header_len > FIG_HEADER_LEN ?
                  FIG_HEADER_LEN : rominfo->backup_header_len, ucon64.fname);
  snes_set_fig_header (&header, rominfo->backup_header_len);
  set_nsrt_info (rominfo, (unsigned char *) &header);

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".fig");
  ucon64_file_handler (dest_name, src_name, 0);

  ucon64_fwrite (&header, 0, FIG_HEADER_LEN, dest_name, "wb");
  if (rominfo->interleaved)
    write_deinterleaved_data (rominfo, src_name, dest_name, FIG_HEADER_LEN);
  else
    fcopy (src_name, rominfo->backup_header_len, (size_t) ucon64.file_size -
             rominfo->backup_header_len, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static void
snes_int_blocks (const unsigned char *deintptr, unsigned char *ipl,
                 unsigned char *iph, int nblocks)
{
  int i;

  // interleave 64 K blocks
  for (i = nblocks; i > 0; i--)
    {
      memmove (ipl, deintptr, 0x8000);
      memmove (iph, deintptr + 0x8000, 0x8000);
      deintptr += 0x10000;
      ipl += 0x8000;
      iph += 0x8000;
    }
}


static void
snes_mirror (unsigned char *dstbuf, unsigned int start, unsigned int data_end,
             unsigned int mirror_end)
{
  int datasize, totsize, nchunks, surplus;

  datasize = data_end - start;
  totsize = mirror_end - start;

  if (datasize >= totsize)
    return;

  nchunks = totsize / datasize - 1;
  surplus = totsize % datasize;
  while (nchunks-- > 0)
    {
      memcpy (dstbuf + data_end, dstbuf + start, datasize);
      data_end += datasize;
    }
  if (surplus > 0)
    memmove (dstbuf + data_end, dstbuf + start, mirror_end - data_end);
}


static void
gd_make_name (const char *filename, size_t backup_header_len, char *name,
              unsigned char *buffer, size_t newsize)
{
  char *p, id_str[4];
  unsigned int n;

  if (UCON64_ISSET (ucon64.id))
    {
      if (ucon64.id >= 0)
        // code for -idnum=NUM
        {
          static int current_id = -1;

          if (current_id == -1)
            current_id = ucon64.id;
          else if (current_id > 999)
            {
              fputs ("ERROR: Not enough IDs available for all files\n", stderr);
              exit (1);
            }
          sprintf (id_str, "%03d", current_id);
          current_id++;

          p = id_str;
        }
      else
        // code for -id
        {
          /*
            We include extra characters that are valid on a FAT file system to
            enlarge the set of possible IDs. Without the extra characters we
            can encode a base 36 number (10 digits + 26 characters in alphabet
            (case insensitive) = 36). 36^3 = 46656 different IDs. With the
            extra characters we can encode a base 50 number which results in
            50^3 = 125000 different IDs.
            We can't use the SNES checksum because several ROM dumps have the
            same checksum (not only PD files!). Nor can we use the internal
            SNES checksum, because several beta ROM dumps have an internal
            checksum of 0 or 0xffff.
          */
          size_t size;
          unsigned int local_buffer = !buffer, id = 0;
          const char *base50_chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!#$%&()-@^_{}~";

          if (local_buffer)
            {
              size = (size_t) ucon64.file_size - backup_header_len;
              if ((buffer = (unsigned char *) malloc (size)) == NULL)
                {
                  fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
                  exit (1);
                }
              ucon64_fread (buffer, backup_header_len, size, filename);
            }
          else
            size = newsize;

          /*
            With the (hashing) algorithm below there are 5 "collisions" between
            proper dumps in a GoodSNES 2.04 set:
            - Battle Dodgeball (J) / BS Sousa Sentai Wappers 2 (J)
            - Dokapon Gaiden - Honoo no Audition (J) / BS Super Earth Defense Force (J)
            - Plok! (U) [!] / BS Yung Hakase no Shinsatsu Shitsu 1 (J)
            - Rocko's Modern Life - Spunky's Dangerous Day (U) / Yokoyama Mitsuteru - Sangokushi Bangi - Sugoroku Eiyuuki (J)
            - Super Ice Hockey (E) / Keiba Eight Special (J) (V1.1) [!]
          */
          for (n = 0; n < size; n++)
            id += (id << 1) + (buffer[n] ^ n);
          id &= 0x7fffffff;                     // reduces # "collisions" from 7 to 5
          id %= 50 * 50 * 50;                   // ensure value can be encoded with 3 base 50 digits

          if (local_buffer)
            free (buffer);

          id_str[0] = base50_chars[id / (50 * 50)];
          id_str[1] = base50_chars[(id % (50 * 50)) / 50];
          id_str[2] = base50_chars[id % 50];
          id_str[3] = '\0';                     // terminate string

          p = id_str;
        }
    }
  else
    p = (char *) basename2 (filename);

#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4777)
/*
  In this case Visual Studio Community 2015 warns with: '_snprintf' : format
  string '%u' requires an argument of type 'unsigned int', but variadic
  argument 1 has type 'size_t'
  This is a bug in Visual Studio Community 2015.
*/
#endif
  snprintf (name, 8, "sf%u%.3s__", (unsigned int) newsize / MBIT, p);
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
  name[7] = '\0';
  if (!strnicmp (name, p, newsize < 10 * MBIT ? 3 : 4))
    snprintf (name, 8, "%s___", p);
  n = newsize < 10 * MBIT ? 6 : 7;
  name[n] = '\0';
  // avoid trouble with filenames containing spaces
  for (n--; n >= 3; n--)                        // skip "sf" and first digit
    if (name[n] == ' ' || name[n] == '.')
      name[n] = '_';
}


static void
snes_set_gd3_header (unsigned int total4Mbparts, unsigned char *header)
{
  reset_header (header);
  memcpy (header, "GAME DOCTOR SF 3", 0x10);

  if (snes_sram_size == 8 * 1024)
    header[0x10] = 0x81;                        // 64 kb
  else if (snes_sram_size == 2 * 1024)
    header[0x10] = 0x82;                        // 16 kb
  else
    header[0x10] = 0x80;                        // 0 kb or 256 kb

  if (snes_hirom)
    {
      if (total4Mbparts <= 2)
        memcpy (&header[0x11], gd3_hirom_8mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 4)
        memcpy (&header[0x11], gd3_hirom_16mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 6)
        memcpy (&header[0x11], gd3_hirom_24mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 8)
        memcpy (&header[0x11], gd3_hirom_32mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 10)
        memcpy (&header[0x11], gd3_hirom_40mb_map, GD3_HEADER_MAPSIZE);
      else
        memcpy (&header[0x11], gd3_hirom_48mb_map, GD3_HEADER_MAPSIZE);

      if (snes_sram_size)
        {
          if (snes_header_base == SNES_EROM)
            {
              header[0x29] = 0x00;
              header[0x2a] = 0x0f;
            }
          else
            {
              header[0x29] = 0x0c;
              header[0x2a] = 0x0c;
            }
        }
      // Adjust SRAM map for exceptions - a couple of 10-12 Mb HiROM games
      //  (Liberty or Death, Brandish). May not be necessary.
    }
  else
    {
      if (total4Mbparts <= 1)
        memcpy (&header[0x11], gd3_lorom_4mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 2)
        memcpy (&header[0x11], gd3_lorom_8mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 4)
        memcpy (&header[0x11], gd3_lorom_16mb_map, GD3_HEADER_MAPSIZE);
      else
        memcpy (&header[0x11], gd3_lorom_32mb_map, GD3_HEADER_MAPSIZE);

      if (snes_header.rom_type == 3 || snes_header.rom_type == 5 || // DSP
          snes_header.rom_type == 0xf6)         // Seta DSP
        {
          header[0x14] = 0x60;
          header[0x1c] = 0x60;
        }

      if (snes_sram_size)
        {
          header[0x24] = 0x40;
          header[0x28] = 0x40;
        }
    }
}


static int
snes_convert_to_gd (st_ucon64_nfo_t *rominfo,
                    void (*write_file) (st_ucon64_nfo_t *rominfo,
                                        unsigned char *buffer, size_t newsize,
                                        size_t total4Mbparts))
{
  unsigned char *srcbuf, *dstbuf;
  size_t size = get_rom_data_size (rominfo->backup_header_len), newsize,
         n4Mbparts = size / (4 * MBIT), surplus4Mb = size % (4 * MBIT),
         total4Mbparts = n4Mbparts + (surplus4Mb > 0 ? 1 : 0);

  if ((srcbuf = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  read_rom_data (srcbuf, rominfo->backup_header_len, size, ucon64.fname);
  if (rominfo->interleaved)
    snes_deinterleave (rominfo, &srcbuf, size);

  if (snes_hirom)
    {
      size_t pad;

      if (!((size >= 2 * MBIT && total4Mbparts <= 8) ||
            total4Mbparts == 10 || total4Mbparts == 12))
        {
          // use 5 decimals here to get byte resolution (for 32 Mbit + 1 byte dumps)
          fprintf (stderr,
                   "ERROR: ROM size is %.5f Mbit -- conversion not yet implemented/verified\n",
                   TOMBIT_F (size));
          free (srcbuf);
          return -1;
        }
      else if (total4Mbparts > 8 && snes_header_base != SNES_EROM)
        {
          fputs ("ERROR: Normal ROM > 32 Mbit -- conversion not yet implemented\n", stderr);
          free (srcbuf);
          return -1;
        }

      if (total4Mbparts == 5)
        total4Mbparts = 6;                      // 20 Mbit HiROMs get padded to 24 Mbit
      else if (total4Mbparts == 7)
        total4Mbparts = 8;                      // 28 Mbit HiROMs get padded to 32 Mbit
#ifdef  PAD_40MBIT_GD3_DUMPS                    //  (a 28 Mbit ROM needs 40 Mbit of GD DRAM)
      else if (total4Mbparts == 10)
        {
          total4Mbparts = 12;                   // 40 Mbit HiROMs get padded to 48 Mbit
          puts ("NOTE: Paddding to 48 Mbit");
        }
#endif

      // interleave the image
      if (n4Mbparts)
        newsize = 4 * total4Mbparts * MBIT;
      else
        newsize = ((size + MBIT - 1) / MBIT) * MBIT;

      // special pad code should only be executed for 10, 20 and if
      //  PAD_40MBIT_GD3_DUMPS is defined, 40 Mbit ROMs
      if (total4Mbparts == 3 || total4Mbparts == 6 || total4Mbparts == 12)
        pad = (newsize - size) / 2;
      else
        pad = 0;

      if ((dstbuf = (unsigned char *) malloc (newsize)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], newsize);
          exit (1);
        }
      if (newsize > size)
        {
          unsigned char *old_srcbuf = srcbuf;
          if ((srcbuf = (unsigned char *) realloc (old_srcbuf, newsize)) == NULL)
            {
              fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], newsize);
              free (old_srcbuf);
              exit (1);
            }
          memset (srcbuf + size, 0, newsize - size);
          memset (dstbuf + size, 0, newsize - size);
        }

      if (snes_header_base == SNES_EROM)
        {
          size_t size2 = newsize - 32 * MBIT;   // size of second ROM (16 Mbit if ToP)
          // interleave the 32 Mbit ROM
          snes_int_blocks (srcbuf, dstbuf + size2 + 16 * MBIT, dstbuf + size2,
                           32 * MBIT / 0x10000);
          // interleave the second ROM
          snes_int_blocks (srcbuf + 32 * MBIT, dstbuf + size2 / 2, dstbuf,
                           size2 / 0x10000);
          if (pad > 0)
            {
              snes_mirror (dstbuf, 0, 4 * MBIT, 8 * MBIT);
              snes_mirror (dstbuf, 8 * MBIT, 12 * MBIT, 16 * MBIT);
            }
        }
      else if (total4Mbparts == 6)
        {
          snes_int_blocks (srcbuf, dstbuf + 16 * MBIT, dstbuf, 16 * MBIT / 0x10000);
          snes_int_blocks (srcbuf + 16 * MBIT, dstbuf + 12 * MBIT,
                           dstbuf + 8 * MBIT, (size - 16 * MBIT) / 0x10000);
          if (pad > 0)
            {
              snes_mirror (dstbuf, 8 * MBIT, 10 * MBIT, 12 * MBIT);
              snes_mirror (dstbuf, 12 * MBIT, 14 * MBIT, 16 * MBIT);
            }
        }
      else
        {
          size_t n = newsize / 2;
          snes_int_blocks (srcbuf, dstbuf + n, dstbuf, newsize / 0x10000);
          if (pad > 0)
            {
              unsigned int half_size_4Mb = (size / 2) & ~(4 * MBIT - 1);
              unsigned int half_size_1Mb = (size / 2 + MBIT - 1) & ~(MBIT - 1);
              snes_mirror (dstbuf, half_size_4Mb, half_size_1Mb, n);
              snes_mirror (dstbuf, n + half_size_4Mb, n + half_size_1Mb, newsize);
            }
        }
    }
  else
    {
      if (total4Mbparts > 8)
        {
          fputs ("ERROR: LoROM > 32 Mbit -- cannot convert\n", stderr);
          free (srcbuf);
          return -1;
        }

      dstbuf = srcbuf;
      newsize = size;
    }

  write_file (rominfo, dstbuf, newsize, total4Mbparts);

  free (srcbuf);
  if (snes_hirom)
    free (dstbuf);

  return 0;
}


static void
write_gd3_file (st_ucon64_nfo_t *rominfo, unsigned char *buffer,
                size_t newsize, size_t total4Mbparts)
{
  unsigned char header[GD_HEADER_LEN];
  char dest_name[FILENAME_MAX];

  // create the header
  ucon64_fread (header, 0, rominfo->backup_header_len > GD_HEADER_LEN ?
                  GD_HEADER_LEN : rominfo->backup_header_len, ucon64.fname);
  snes_set_gd3_header (total4Mbparts, header);
  set_nsrt_info (rominfo, header);

  gd_make_name (ucon64.fname, rominfo->backup_header_len, dest_name, buffer, newsize);
  ucon64_file_handler (dest_name, NULL, OF_FORCE_BASENAME);
  ucon64_fwrite (header, 0, GD_HEADER_LEN, dest_name, "wb");
  ucon64_fwrite (buffer, GD_HEADER_LEN, newsize, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);

  if (snes_hirom)
    puts ("NOTE: This ROM has to be split with " OPTION_S "s in order to work with a Game Doctor,\n"
          "      unless it is transferred over a parallel port connection");
}


int
snes_gd3 (st_ucon64_nfo_t *rominfo)
{
  return snes_convert_to_gd (rominfo, write_gd3_file);
}


// see src/backup/mgd.h for the file naming scheme
static void
write_mgd_files (st_ucon64_nfo_t *rominfo, unsigned char *buffer,
                 size_t newsize, size_t total4Mbparts)
{
  char dest_name[FILENAME_MAX];

  (void) rominfo;
  (void) total4Mbparts;
  mgd_make_name (ucon64.fname, UCON64_SNES, newsize, dest_name);
  ucon64_file_handler (dest_name, NULL, OF_FORCE_BASENAME);
  ucon64_fwrite (buffer, 0, newsize, dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);

  mgd_write_index_file ((char *) basename2 (dest_name), 1);
}


int
snes_mgd (st_ucon64_nfo_t *rominfo)
{
  return snes_convert_to_gd (rominfo, write_mgd_files);
}


static void
write_mgh_name_file (st_ucon64_nfo_t *rominfo, const char *dest_name)
{
  unsigned char mgh_data[32] = "MGH\x1a";
  int n;

  for (n = 0; n < 15 && rominfo->name[n] != '\0'; n++)
    mgh_data[16 + n] = isprint ((int) rominfo->name[n]) ? rominfo->name[n] : '.';
  mgh_data[31] = 0xff;

  // If a backup would be created it would overwrite the backup of the ROM. The
  //  ROM backup is more important, so we don't write a backup of the MGH file.
  ucon64_fwrite (mgh_data, 0, sizeof mgh_data, dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
}


static void
write_mgh_files (st_ucon64_nfo_t *rominfo, unsigned char *buffer,
                 size_t newsize, size_t total4Mbparts)
{
  char dest_name[FILENAME_MAX];

  (void) total4Mbparts;
  mgh_make_name (ucon64.fname, UCON64_SNES, newsize, dest_name);
  ucon64_file_handler (dest_name, NULL, OF_FORCE_BASENAME);
  ucon64_fwrite (buffer, 0, newsize, dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);

  set_suffix (dest_name, ".MGH");
  write_mgh_name_file (rominfo, dest_name);

  if (snes_hirom)
    puts ("NOTE: This ROM has to be split with " OPTION_LONG_S "smgh in order to work with an MGH");
}


int
snes_mgh (st_ucon64_nfo_t *rominfo)
{
  return snes_convert_to_gd (rominfo, write_mgh_files);
}


// header format is specified in src/backup/ufo.h
int
snes_ufo (st_ucon64_nfo_t *rominfo)
{
  st_ufo_header_t header;
  size_t size = get_rom_data_size (rominfo->backup_header_len);
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  ucon64_fread (&header, 0, rominfo->backup_header_len > UFO_HEADER_LEN ?
                  UFO_HEADER_LEN : rominfo->backup_header_len, ucon64.fname);
  reset_header (&header);
  memcpy (header.id, "SUPERUFO", 8);
  header.uses_sram = snes_sram_size ? 1 : 0;
  header.banktype = snes_hirom ? 0 : 1;

  if (snes_sram_size > 32 * 1024)
    header.sram_size = 8;
  else if (snes_sram_size > 8 * 1024)           // 64 kb < size <= 256 kb
    header.sram_size = 3;
  else if (snes_sram_size > 2 * 1024)           // 16 kb < size <= 64 kb
    header.sram_size = 2;
  else if (snes_sram_size > 0)                  // 1 - 16 kb
    header.sram_size = 1;
  // header.sram_size is already OK for snes_sram_size == 0

  header.sram_type = snes_hirom ? 0 : 3;

  set_nsrt_info (rominfo, (unsigned char *) &header);

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".ufo");
  ucon64_file_handler (dest_name, src_name, 0);

  if (snes_hirom)
    {
      unsigned char *srcbuf, *dstbuf;
      size_t newsize = size >= 10 * MBIT && size <= 12 * MBIT ?
                         12 * MBIT : ((size + MBIT - 1) & ~(MBIT - 1)),
             half_newsize = newsize / 2, pad = (newsize - size) / 2;

      header.size_low = (unsigned char) (newsize / 8192);
      header.size_high = (unsigned char) (newsize / 8192 >> 8);
      header.size = (unsigned char) (newsize / MBIT);

      if (snes_sram_size)
        header.sram_a20_a21 = 0x0c;             // try 3 if game gives protection message
      header.sram_a22_a23 = 2;
      // Tales of Phantasia (J) & Dai Kaiju Monogatari 2 (J) [14-17]: 0 0x0e 0 0

      if ((srcbuf = (unsigned char *) malloc (size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
          exit (1);
        }
      if ((dstbuf = (unsigned char *) malloc (newsize)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], newsize);
          exit (1);
        }

      read_rom_data (srcbuf, rominfo->backup_header_len, size, src_name);
      if (rominfo->interleaved)
        snes_deinterleave (rominfo, &srcbuf, size);
      if (newsize > size)
        memset (dstbuf + size, 0, newsize - size);

      snes_int_blocks (srcbuf, dstbuf + half_newsize, dstbuf, size / 0x10000);
      if (pad > 0)
        {
          size_t half_size_4Mb = (size / 2) & ~(4 * MBIT - 1);
          size_t half_size_1Mb = (size / 2 + MBIT - 1) & ~(MBIT - 1);
          snes_mirror (dstbuf, half_size_4Mb, half_size_1Mb, half_newsize);
          snes_mirror (dstbuf, half_newsize + half_size_4Mb,
                       half_newsize + half_size_1Mb, newsize);
        }

      ucon64_fwrite (&header, 0, UFO_HEADER_LEN, dest_name, "wb");
      ucon64_fwrite (dstbuf, UFO_HEADER_LEN, newsize, dest_name, "ab");

      free (srcbuf);
      free (dstbuf);
    }
  else // LoROM
    {
      header.size_low = (unsigned char) (size / 8192);
      header.size_high = (unsigned char) (size / 8192 >> 8);
      header.size = (unsigned char) (size / MBIT);

      if (snes_sram_size == 0)
        {
          // check if the game uses a DSP chip
          if (snes_header.rom_type == 3 || snes_header.rom_type == 5 ||
              snes_header.rom_type == 0xf6)
            {
              header.sram_a15 = 1;
              header.sram_a20_a21 = 0x0c;
            }
          else // no SRAM & doesn't use a DSP chip
            {
              header.sram_a22_a23 = 2;
              header.sram_type = 0;
            }
        }
      else // cartridge contains SRAM
        {
          header.sram_a15 = 2;                  // try 1 if game gives protection error
          header.sram_a20_a21 = 0x0f;
          header.sram_a22_a23 = 3;
        }

      ucon64_fwrite (&header, 0, UFO_HEADER_LEN, dest_name, "wb");
      if (rominfo->interleaved)
        write_deinterleaved_data (rominfo, src_name, dest_name, UFO_HEADER_LEN);
      else
        fcopy (src_name, rominfo->backup_header_len, size, dest_name, "ab");
    }

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  if (snes_hirom)
    puts ("NOTE: This ROM has to be split with " OPTION_S "s in order to work with a UFO Super Drive");

  return 0;
}


// header format is specified in src/backup/ufosd.h
int
snes_ufosd (st_ucon64_nfo_t *rominfo)
{
  st_ufosd_header_t header;
  size_t size = get_rom_data_size (rominfo->backup_header_len), ufosd_size = 0,
         n, rom_sizes[] =
           { 4 * MBIT,  8 * MBIT, 10 * MBIT, 12 * MBIT,
            16 * MBIT, 20 * MBIT, 24 * MBIT, 32 * MBIT };
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  // The Super UFO Pro 8 SD only accepts files with certain sizes.
  if (size > 32 * MBIT)
    {
      fputs ("ERROR: ROM > 32 Mbit -- cannot convert\n", stderr);
      return -1;
    }
  for (n = 0; n < sizeof rom_sizes / sizeof rom_sizes[0]; n++)
    if (size <= rom_sizes[n])
      {
        ufosd_size = rom_sizes[n];
        break;
      }

  ucon64_fread (&header, 0, rominfo->backup_header_len > UFOSD_HEADER_LEN ?
                  UFOSD_HEADER_LEN : rominfo->backup_header_len, ucon64.fname);
  reset_header (&header);
  header.size = (unsigned char) (ufosd_size / MBIT);
  header.banktype_copy = snes_hirom ? 0 : 1;
  memcpy (header.id, "SFCUFOSD", 8);
  header.internal_size = get_internal_size ();

  if (snes_header.rom_type == 3 || snes_header.rom_type == 5 || // DSP
      snes_header.rom_type == 0x13 ||           // SRAM + Super FX (Mario Chip 1)
      snes_header.rom_type == 0x1a ||           // Super FX
      snes_header.rom_type == 0x14 || snes_header.rom_type == 0x15 || // Super FX (2)
      snes_header.rom_type == 0x25 ||           // OBC1
      snes_header.rom_type == 0x34 || snes_header.rom_type == 0x35 || // SA-1
      snes_header.rom_type == 0x43 || snes_header.rom_type == 0x45 || // S-DD1
      snes_header.rom_type == 0x55 ||           // S-RTC
      snes_header.rom_type == 0xe3 ||           // Game Boy data
      snes_header.rom_type == 0xf3 ||           // C4
      snes_header.rom_type == 0xf5 ||           // Seta RISC / SPC7110
      snes_header.rom_type == 0xf6 ||           // Seta DSP
      snes_header.rom_type == 0xf9)             // SPC7110 + RTC
    header.special_chip = 0xff;

  if (header.special_chip == 0 ||
      snes_header.rom_type == 3 || snes_header.rom_type == 5 || // DSP
      snes_header.rom_type == 0xf6)             // Seta DSP
    {
      if (snes_sram_size > 32 * 1024)
        header.sram_size = 7;
      else if (snes_sram_size > 8 * 1024)       // 64 kb < size <= 256 kb
        header.sram_size = 3;
      else if (snes_sram_size > 2 * 1024)       // 16 kb < size <= 64 kb
        header.sram_size = 2;
      else if (snes_sram_size > 0)              // 1 - 16 kb
        header.sram_size = 1;
      // header.sram_size is already OK for snes_sram_size == 0
    }

  if (snes_hirom)
    {
      switch (ufosd_size)
        {
        case 4 * MBIT:
          header.map_control[0] = 0x09;
          break;
        case 8 * MBIT:
          header.map_control[0] = 0x25;
          break;
        case 10 * MBIT:
          header.map_control[0] = 0x37;
          break;
        case 12 * MBIT:
          header.map_control[0] = 0x3d;
          break;
        case 16 * MBIT:
          header.map_control[0] = 0x95;
          break;
        case 20 * MBIT:
        case 24 * MBIT:
          header.map_control[0] = 0xf5;
          break;
        case 32 * MBIT:
          memcpy (header.map_control, "\x55\x00\x80", 3);
          break;
        }
      header.map_control[3] = snes_sram_size ? 0x2c : 0;
    }
  else
    {
      switch (ufosd_size)
        {
        case 4 * MBIT:
          memcpy (header.map_control, "\x05\x2a", 2);
          break;
        case 8 * MBIT:
          memcpy (header.map_control, "\x15\x28", 2);
          break;
        case 10 * MBIT:
        case 12 * MBIT:
        case 16 * MBIT:
          header.map_control[1] = 0x20;
        case 20 * MBIT:                         // falling through
        case 24 * MBIT:
        case 32 * MBIT:
          header.map_control[0] = 0x55;
          break;
        }
      if (snes_sram_size)
        {
#if 0
          switch (ufosd_size)
            {
            case 4 * MBIT:
              memcpy (&header.map_control[2], "\x10\x3f", 2);
              break;
            case 8 * MBIT:
              if (!header.special_chip)
                {
                  if (snes_sram_size == 2 * 1024)
                    memcpy (&header.map_control[2], "\x20\x3f", 2);
                  else
                    memcpy (header.map_control, "\x55\x00\x50\xbf", 4);
                }
              else if ((snes_sfx_sram_size ?
                          snes_sfx_sram_size : snes_sram_size) == 32 * 1024)
                memcpy (header.map_control, "\x55\x00\x40\x00", 4);
              break;
            case 10 * MBIT:
            case 12 * MBIT:
            case 16 * MBIT:
              memcpy (&header.map_control[2], "\x20\x3f", 2);
              break;
            case 20 * MBIT:
            case 24 * MBIT:
              memcpy (&header.map_control[2], "\x60\xbf", 2);
              break;
            case 32 * MBIT:
              header.map_control[2] = 0x80;
              break;
            }
#else
          header.map_control[2] = ufosd_size > 16 * MBIT ?
                                    0x60 : snes_sram_size <= 32 * 1024 ?
                                      0x10 : 0x20;
          header.map_control[3] = 0x3f;
#endif
        }
    }

  header.banktype = snes_hirom ? 0 : 1;
  header.tvtype = snes_header.country == 0 || snes_header.country == 1 ? 0 : 2;
  // copy last 32 bytes of internal header to backup unit header
  memcpy (header.internal_header_data,
          ((char *) &snes_header) + sizeof snes_header - 32, 32);

  set_nsrt_info (rominfo, (unsigned char *) &header);

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".ufo");
  ucon64_file_handler (dest_name, src_name, 0);

  ucon64_fwrite (&header, 0, UFOSD_HEADER_LEN, dest_name, "wb");
  if (rominfo->interleaved)
    write_deinterleaved_data (rominfo, src_name, dest_name, UFOSD_HEADER_LEN);
  else
    fcopy (src_name, rominfo->backup_header_len, size, dest_name, "ab");

  if (size != ufosd_size)
    {
      // NOTE: The checksum doesn't change, because we pad with zeros.
      if (truncate2 (dest_name, ufosd_size + UFOSD_HEADER_LEN) == -1)
        {
          fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
          return -1;
        }
      size = ufosd_size;
    }

  {
    // The Super UFO Pro 8 SD refuses to load images with an incorrect checksum.
    unsigned char sum[4];

    rominfo->interleaved = 0;
    rominfo->backup_header_len = UFOSD_HEADER_LEN;
    update_chksum (rominfo, sum, size, dest_name);
    // update the checksum in the backup unit header as well
    ucon64_fwrite (sum, 0x3c, 4, dest_name, "r+b");
  }

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
snes_ic2 (st_ucon64_nfo_t *rominfo)
{
  st_swc_header_t header;
  unsigned char *buffer;
  size_t size = get_rom_data_size (rominfo->backup_header_len), n;
  char dest_name[2][FILENAME_MAX], suffix[] = ".1";

  if (!snes_hirom)
    {
      fputs ("ERROR: ROM is not HiROM -- cannot convert\n", stderr);
      return -1;
    }
  else if (size > 16 * MBIT)
    {
      fputs ("ERROR: ROM > 16 Mbit -- cannot convert\n", stderr);
      return -1;
    }

  for (n = 0; n < 2; n++)
    {
      strcpy (dest_name[n], ucon64.fname);
      set_suffix (dest_name[n], suffix);
      ucon64_output_fname (dest_name[n], 0);
      suffix[1]++;
    }

  ucon64_fread (&header, 0, rominfo->backup_header_len > SWC_HEADER_LEN ?
                  SWC_HEADER_LEN : rominfo->backup_header_len, ucon64.fname);
  reset_header (&header);

  header.size_low = (unsigned char) (8 * MBIT / 8192); // header.size_high == 0
  header.emulation = 0x40;
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 4;

  set_nsrt_info (rominfo, (unsigned char *) &header);

  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  read_rom_data (buffer, rominfo->backup_header_len, size, ucon64.fname);

  ucon64_fwrite (&header, 0, SWC_HEADER_LEN, dest_name[0], "wb");

  header.size_low = (unsigned char) (size / (8192 * 2)); // header.size_high == 0
  header.emulation = 0;                         // last file => clear bit 6

  ucon64_fwrite (&header, 0, SWC_HEADER_LEN, dest_name[1], "wb");

  if (!rominfo->interleaved)
    {
      for (n = 0; n < size / 32768; n++)
        if (ucon64_fwrite (&buffer[n * 32768],
                           rominfo->backup_header_len + (n / 2) * 32768,
                           32768, dest_name[~n & 1], "ab") < 0)
          {
            fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name[~n & 1]);
            free (buffer);
            return -1;
          }
    }
  else
    for (n = 0; n < 2; n++)
      if (ucon64_fwrite (&buffer[n * size / 2], rominfo->backup_header_len,
                         size / 2, dest_name[n], "ab") < 0)
        {
          fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name[n]);
          free (buffer);
          return -1;
        }
  free (buffer);
  // first part should always be 8 Mbit, so pad it if it is smaller
  if (truncate2 (dest_name[0], 8 * MBIT + SWC_HEADER_LEN) == -1)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name[0]);
      return -1;
    }
  printf (ucon64_msg[WROTE], dest_name[0]);
  printf (ucon64_msg[WROTE], dest_name[1]);

  return 0;
}


int
snes_gd_make_names (const char *filename, size_t backup_header_len, char **names)
{
  char dest_name[FILENAME_MAX];
  // don't use PARTSIZE here, because the Game Doctor doesn't support
  //  arbitrary part sizes
  size_t size = (size_t) ucon64.file_size - backup_header_len,
         nparts = size / (8 * MBIT), surplus = size % (8 * MBIT), name_i = 0;

  gd_make_name (filename, backup_header_len, dest_name, NULL, size);
  strupr (dest_name);
  dest_name[7] = 'A';
  dest_name[8] = '\0';

  if (snes_hirom && size <= 16 * MBIT)
    {
      // 8 Mbit or less HiROMs, X is used to pad filename to 8 (SF4###XA)
      if (size < 10 * MBIT)
        dest_name[8 - 2] = 'X';
      strcpy (names[name_i++], dest_name);

      dest_name[8 - 1]++;
      strcpy (names[name_i++], dest_name);
    }
  else
    {
      size_t n;

      for (n = 0; n < nparts; n++)
        {
          strcpy (names[name_i++], dest_name);
          dest_name[8 - 1]++;
        }
      if (surplus)
        strcpy (names[name_i++], dest_name);
    }
  if (name_i == 1)
    names[0][7] = '\0';                         // don't use 'A' if not split
  return name_i;
}


static void
snes_split_gd3 (size_t backup_header_len, size_t size)
{
  char dest_name[FILENAME_MAX], *names[GD3_MAX_UNITS],
       names_mem[GD3_MAX_UNITS][9] = { { 0 } };
  // don't use ucon64.part_size here, because the Game Doctor doesn't support
  //  arbitrary part sizes
  size_t nparts = size / (8 * MBIT), surplus = size % (8 * MBIT), n, name_i = 0;

  if (UCON64_ISSET (ucon64.part_size))
    puts ("NOTE: ROM will be split as Game Doctor SF3 ROM, ignoring switch " OPTION_LONG_S "ssize");

  // we don't want to malloc() ridiculously small chunks (of 9 bytes)
  for (n = 0; n < sizeof names / sizeof names[0]; n++)
    names[n] = names_mem[n];
  if (!(snes_hirom && size <= 16 * MBIT) &&
      nparts + (surplus ? 1 : 0) > sizeof names / sizeof names[0])
    {
      fprintf (stderr,
               "ERROR: Splitting this ROM would result in %u parts (of 8 Mbit).\n"
               "       %u is the maximum number of parts for GD3 and MGD2\n",
               (unsigned) nparts + (surplus ? 1 : 0),
               (unsigned) (sizeof names / sizeof names[0]));
      return;
    }
  snes_gd_make_names (ucon64.fname, backup_header_len, (char **) names);

  if (snes_hirom && size <= 16 * MBIT)
    {
      size_t half_size = size / 2;

      sprintf (dest_name, "%s.078", names[name_i++]);
      ucon64_output_fname (dest_name, OF_FORCE_BASENAME);
      // don't write backups of parts, because one name is used
      fcopy (ucon64.fname, 0, half_size + backup_header_len, dest_name, "wb");
      printf (ucon64_msg[WROTE], dest_name);

      sprintf (dest_name, "%s.078", names[name_i++]);
      ucon64_output_fname (dest_name, OF_FORCE_BASENAME);
      fcopy (ucon64.fname, half_size + backup_header_len, size - half_size, dest_name, "wb");
      printf (ucon64_msg[WROTE], dest_name);
    }
  else
    {
      for (n = 0; n < nparts; n++)
        {
          // don't write backups of parts, because one name is used
          sprintf (dest_name, "%s.078", names[name_i++]);
          ucon64_output_fname (dest_name, OF_FORCE_BASENAME);
          fcopy (ucon64.fname, n * 8 * MBIT + (n ? backup_header_len : 0),
                 8 * MBIT + (n ? 0 : backup_header_len), dest_name, "wb");
          printf (ucon64_msg[WROTE], dest_name);
        }

      if (surplus)
        {
          // don't write backups of parts, because one name is used
          sprintf (dest_name, "%s.078", names[name_i++]);
          ucon64_output_fname (dest_name, OF_FORCE_BASENAME);
          fcopy (ucon64.fname, n * 8 * MBIT + (n ? backup_header_len : 0),
                 surplus + (n ? 0 : backup_header_len), dest_name, "wb");
          printf (ucon64_msg[WROTE], dest_name);
        }
    }

  // An index file is not used by the GD, but by the MGD2. We don't have a
  //  special function for splitting MGD2 files, so we do it here.
  if (!backup_header_len)
    mgd_write_index_file (names, name_i);
}


static void
snes_split_ufo (size_t backup_header_len, size_t size, size_t part_size)
{
  st_ufo_header_t header;
  char dest_name[FILENAME_MAX], *p;
  size_t nparts = 0, surplus = 0, n, nbytesdone;

  if (snes_hirom)
    {
      if (size > 32 * MBIT)
        {
          fputs ("ERROR: HiROM > 32 Mbit -- conversion not yet implemented\n", stderr);
          return;
        }
      if (UCON64_ISSET (ucon64.part_size))
        puts ("NOTE: Splitting Super UFO HiROM, ignoring switch " OPTION_LONG_S "ssize");
    }
  else
    {
      nparts = size / part_size;
      surplus = size % part_size;
      if (nparts + (surplus ? 1 : 0) > 9)
        {
          fprintf (stderr,
                   "ERROR: This part size (%u Mbit) would result in more than the maximum number\n"
                   "       of parts (%u). Specify a part size of %u Mbit or larger with " OPTION_LONG_S "ssize\n",
                   (unsigned) part_size / MBIT,
                   (unsigned) nparts + (surplus ? 1 : 0),
                   (unsigned) (size + 9 * MBIT - 1) / (9 * MBIT));
          return;
        }
    }

  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".1gm");
  ucon64_output_fname (dest_name, 0);
  p = strrchr (dest_name, '.') + 1;

  ucon64_fread (&header, 0, UFO_HEADER_LEN, ucon64.fname);

  if (snes_hirom)
    {
      typedef struct
      {
        unsigned char value;
        unsigned char size;
        unsigned char list[8];
      } st_value_list_t;

      st_value_list_t size_to_partsizes[5] =
        {
          { 2, 2, { 1, 1 } },
          { 4, 2, { 2, 2 } },
          { 12, 4, { 4, 2, 4, 2 } },            // 10 Mbit files are padded by snes_ufo()
          { 20, 6, { 4, 4, 2, 4, 4, 2 } },
          { 32, 8, { 4, 4, 4, 4, 4, 4, 4, 4 } }
        }, *size_to_partsizes_ptr = NULL;
      st_value_list_t size_to_flags[8] =
        {
          { 2, 2, { 0x10, 0 } },
          { 4, 2, { 0x10, 0 } },
          { 8, 2, { 0x10, 0 } },
          { 12, 4, { 0x40, 0x10, 0x10, 0 } },
          { 16, 4, { 0x40, 0x10, 0x10, 0 } },
          { 20, 6, { 0x40, 0x40, 0x10, 0x10, 0x10, 0 } },
          { 24, 6, { 0x40, 0x40, 0x10, 0x10, 0x10, 0 } },
          { 32, 8, { 0x40, 0x40, 0x40, 0x10, 0x10, 0x10, 0x10, 0 } }
        }, *size_to_flags_ptr = NULL;
      int x = size / MBIT;

      nparts = 0;                               // warning remover
      surplus = 0;                              // warning remover
      for (n = 0; n < 4; n++)
        if (size_to_partsizes[n].value == x)
          {
            size_to_partsizes_ptr = &size_to_partsizes[n];
            nparts = size_to_partsizes[n].size;
            surplus = 0;
          }
      if (!size_to_partsizes_ptr)               // size was not found
        {
          size_to_partsizes_ptr = &size_to_partsizes[4];
          nparts = size / (4 * MBIT);
          surplus = size % (4 * MBIT);
        }

      for (n = 0; n < 7; n++)
        if (size_to_flags[n].value == x)
          size_to_flags_ptr = &size_to_flags[n];
      if (!size_to_flags_ptr)
        size_to_flags_ptr = &size_to_flags[7];

      nbytesdone = backup_header_len;
      for (n = 0; n < nparts; n++)
        {
          part_size = size_to_partsizes_ptr->list[n] * MBIT;
          header.size_low = (unsigned char) (part_size / 8192);
          header.size_high = (unsigned char) (part_size / 8192 >> 8);
          header.multi = size_to_flags_ptr->list[n];

          if (surplus == 0 && n == nparts - 1)
            header.multi = 0;

          ucon64_fwrite (&header, 0, UFO_HEADER_LEN, dest_name, "wb");
          fcopy (ucon64.fname, nbytesdone, part_size, dest_name, "ab");
          printf (ucon64_msg[WROTE], dest_name);

          nbytesdone += part_size;
          (*p)++;
        }
    }
  else
    {
      header.size_low = (unsigned char) (part_size / 8192);
      header.size_high = (unsigned char) (part_size / 8192 >> 8);
      header.multi = 0x40;

      nbytesdone = backup_header_len;
      for (n = 0; n < nparts; n++)
        {
          if (surplus == 0 && n == nparts - 1)
            header.multi = 0;

          ucon64_fwrite (&header, 0, UFO_HEADER_LEN, dest_name, "wb");
          fcopy (ucon64.fname, nbytesdone, part_size, dest_name, "ab");
          printf (ucon64_msg[WROTE], dest_name);

          nbytesdone += part_size;
          (*p)++;
        }
    }

  if (surplus)
    {
      header.size_low = (unsigned char) (surplus / 8192);
      header.size_high = (unsigned char) (surplus / 8192 >> 8);
      header.multi = 0;

      ucon64_fwrite (&header, 0, UFO_HEADER_LEN, dest_name, "wb");
      fcopy (ucon64.fname, nbytesdone, surplus, dest_name, "ab");
      printf (ucon64_msg[WROTE], dest_name);
    }
}


static void
snes_split_smc (size_t backup_header_len, size_t size, size_t part_size)
// this function splits both SWC and FIG files
{
  st_swc_header_t header;
  char dest_name[FILENAME_MAX], *p;
  size_t nparts = size / part_size, surplus = size % part_size, n;

  if (nparts + (surplus ? 1 : 0) > 9)
    {
      fprintf (stderr,
               "ERROR: This part size (%u Mbit) would result in more than the maximum number\n"
               "       of parts (%u). Specify a part size of %u Mbit or larger with " OPTION_LONG_S "ssize\n",
               (unsigned) part_size / MBIT,
               (unsigned) nparts + (surplus ? 1 : 0),
               (unsigned) (size + 9 * MBIT - 1) / (9 * MBIT));
      return;
    }

  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".1");
  ucon64_output_fname (dest_name, 0);
  p = strrchr (dest_name, '.') + 1;

  ucon64_fread (&header, 0, SWC_HEADER_LEN, ucon64.fname);
  header.size_low = (unsigned char) (part_size / 8192);
  header.size_high = (unsigned char) (part_size / 8192 >> 8);
  // if header[2], bit 6 == 0 => SWC/FIG knows this is the last file of the ROM
  header.emulation |= 0x40; // "header[2]" is called multi for st_fig_header_t

  for (n = 0; n < nparts; n++)
    {
      if (surplus == 0 && n == nparts - 1)
        header.emulation &= ~0x40;              // last file => clear bit 6

      // don't write backups of parts, because one name is used
      ucon64_fwrite (&header, 0, SWC_HEADER_LEN, dest_name, "wb");
      fcopy (ucon64.fname, n * part_size + backup_header_len, part_size,
             dest_name, "ab");
      printf (ucon64_msg[WROTE], dest_name);

      (*p)++;
    }

  if (surplus)
    {
      header.size_low = (unsigned char) (surplus / 8192);
      header.size_high = (unsigned char) (surplus / 8192 >> 8);
      header.emulation &= ~0x40;                // last file => clear bit 6

      // don't write backups of parts, because one name is used
      ucon64_fwrite (&header, 0, SWC_HEADER_LEN, dest_name, "wb");
      fcopy (ucon64.fname, n * part_size + backup_header_len, surplus,
             dest_name, "ab");
      printf (ucon64_msg[WROTE], dest_name);
    }
}


#define PARTSIZE  (8 * MBIT)                    // default split part size
int
snes_s (st_ucon64_nfo_t *rominfo)
{
  size_t size = (size_t) ucon64.file_size - rominfo->backup_header_len, part_size;

  if (UCON64_ISSET (ucon64.part_size) &&
      !(copier_type == GD3 || (copier_type == UFO && snes_hirom) ||
        copier_type == UFOSD || copier_type == IC2))
    {
      part_size = ucon64.part_size;
      /*
        Don't allow too small part sizes, because then the files that come
        after the file with suffix ".9" will get filenames that can't be stored
        on a FAT filesystem (filesystem that SWC requires to be on diskette).
        For example the first file after the file with suffix ".9" will get
        suffix ".:". The SWC does ask for that filename though.
        Also don't base the minimum part size on the actual file size, because
        that will probably only confuse users.
        We ignore the few ROMs that are larger than 32 MBit. Just use -ssize to
        specify a larger part size for those.
      */
      if (part_size < 4 * MBIT)
        {
          fputs ("ERROR: Split part size must be larger than or equal to 4 Mbit\n",
                 stderr);
          return -1;
        }
    }
  else
    part_size = PARTSIZE;

  if (copier_type == GD3)
    // part_size is ignored for Game Doctor
    {
      if (size < 4 * MBIT && size != 2 * MBIT)
        { // "&& size != 2 * MBIT" is a fix for BS Chrono Trigger - Jet Bike Special (J)
          fputs ("ERROR: ROM size is smaller than 4 Mbit -- will not be split\n", stderr);
          return -1;
        }
    }
  else if (copier_type == UFO && snes_hirom)
    {
      if (size < 2 * MBIT)
        {
          fputs ("ERROR: ROM size is smaller than 2 Mbit -- will not be split\n", stderr);
          return -1;
        }
    }
  else if (copier_type == UFOSD)
    {
      fputs ("ERROR: ROM is in Super UFO Pro 8 SD format -- will not be split\n", stderr);
      return -1;
    }
  else if (copier_type == IC2)
    {
      fputs ("ERROR: ROM is in Super Magicom IC2 format -- already split\n", stderr);
      return -1;
    }
  else if (size <= part_size)
    {
      fprintf (stderr,
               "ERROR: ROM size is smaller than or equal to %u Mbit -- will not be split\n",
               (unsigned) part_size / MBIT);
      return -1;
    }

  if (!rominfo->backup_header_len || copier_type == GD3) // GD3 format
    snes_split_gd3 (rominfo->backup_header_len, size);
  else if (copier_type == UFO)
    snes_split_ufo (rominfo->backup_header_len, size, part_size);
  else
    snes_split_smc (rominfo->backup_header_len, size, part_size);

  return 0;
}


int
snes_smgh (st_ucon64_nfo_t *rominfo)
{
  size_t size = (size_t) ucon64.file_size - rominfo->backup_header_len,
         part_size, nparts, surplus, n;
  const char *p0;
  char dest_name[FILENAME_MAX], *p, *suffix;

  if (copier_type == IC2)
    {
      fputs ("ERROR: ROM is in Super Magicom IC2 format -- already split", stderr);
      return -1;
    }

  if (UCON64_ISSET (ucon64.part_size))
    {
      part_size = ucon64.part_size;
      // don't allow too small part sizes, see snes_s()
      if (part_size < 4 * MBIT)
        {
          fputs ("ERROR: Split part size must be larger than or equal to 4 Mbit\n",
                 stderr);
          return -1;
        }
    }
  else
    part_size = PARTSIZE;

  if (size <= part_size && !(snes_hirom && size <= 16 * MBIT))
    {
      fprintf (stderr,
               "ERROR: ROM size is smaller than or equal to %u Mbit -- will not be split\n",
               (unsigned) part_size / MBIT);
      return -1;
    }

  nparts = size / part_size;
  surplus = size % part_size;

  mgh_make_name (ucon64.fname, UCON64_SNES, size, dest_name);
  ucon64_output_fname (dest_name, OF_FORCE_BASENAME);
  p0 = basename2 (dest_name);
  p = strrchr (p0, '.');
  suffix = strdup (p);
  n = p - p0;
  if (n <= 7)
    {
      memset (p, '_', 7 - n);
      p += 7 - n;
    }
  else // n == 8
    p--;
  *p = 'A';
  *(p + 1) = '\0';
  set_suffix (dest_name, suffix);
  free (suffix);

  if (snes_hirom && size <= 16 * MBIT)
    {
      unsigned int half_size = size / 2;

      if (UCON64_ISSET (ucon64.part_size))
        puts ("NOTE: Splitting Multi Game Hunter HiROM <= 16 Mbit, ignoring switch " OPTION_LONG_S "ssize");

      // don't write backups of parts, because one name is used
      fcopy (ucon64.fname, rominfo->backup_header_len, half_size, dest_name, "wb");
      printf (ucon64_msg[WROTE], dest_name);
      (*p)++;

      fcopy (ucon64.fname, half_size + rominfo->backup_header_len,
             size - half_size, dest_name, "wb");
      printf (ucon64_msg[WROTE], dest_name);
    }
  else
    {
      for (n = 0; n < nparts; n++)
        {
          // don't write backups of parts, because one name is used
          fcopy (ucon64.fname, n * part_size + rominfo->backup_header_len,
                 part_size, dest_name, "wb");
          printf (ucon64_msg[WROTE], dest_name);
          (*p)++;
        }

      if (surplus)
        {
          // don't write backups of parts, because one name is used
          fcopy (ucon64.fname, n * part_size + rominfo->backup_header_len,
                 surplus, dest_name, "wb");
          printf (ucon64_msg[WROTE], dest_name);
        }
    }

  *p = 'A';
  set_suffix (dest_name, ".MGH");
  write_mgh_name_file (rominfo, dest_name);
  return 0;
}


int
snes_j (st_ucon64_nfo_t *rominfo)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], *p;
  size_t block_size, total_size = 0, header_len = rominfo->backup_header_len;

  if (copier_type == IC2 && get_rom_data_size (rominfo->backup_header_len) < 16 * MBIT)
    {
      fputs ("ERROR: ROM < 16 Mbit and in Super Magicom IC2 format -- resulting file would\n"
             "       be corrupt. Use a regular conversion option, like " OPTION_LONG_S "swc\n",
             stderr);
      return -1;
    }

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".tmp");

  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (src_name, 0, rominfo->backup_header_len, dest_name, "wb"); // copy header (if any)

  p = strrchr (src_name, '.');
  if (p == NULL)                                // filename doesn't contain a period
    p = src_name + strlen (src_name) - 1;
  else
    (copier_type == GD3 || copier_type == MGD_SNES) ? p-- : p++;

  // split GD3 files don't have a header _except_ the first one
  block_size = fsizeof (src_name) - rominfo->backup_header_len;
  while (fcopy (src_name, header_len, block_size, dest_name, "ab") != -1)
    {
      printf ("Joined %s\n", src_name);
      total_size += block_size;
      (*p)++;

      if (copier_type == GD3)
        header_len = 0;
      block_size = fsizeof (src_name) - header_len;
    }

  if (rominfo->backup_header_len && copier_type != GD3)
    {                                           // fix header
      unsigned char buffer[3];
      buffer[0] = (unsigned char) (total_size / 8192); // # 8K blocks low byte
      buffer[1] = (unsigned char) (total_size / 8192 >> 8); // # 8K blocks high byte
      buffer[2] = ucon64_fgetc (dest_name, 2) & // last file => clear bit 6
                    ~(copier_type == UFO ? 0x50 : 0x40);
      ucon64_fwrite (buffer, 0, sizeof buffer, dest_name, "r+b");
    }

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
snes_k (st_ucon64_nfo_t *rominfo)
/*
See the document "src/backup/SWC-compatibility.txt".
Don't touch this code if you don't know what you're doing!

Some SNES games check to see how much SRAM is connected to the SNES as a form
of copy protection. As most copiers have 256 kbits standard, the game will
know it's running on a backup unit and stop to prevent people copying the
games. However, newer copiers like the SWC DX2 get around this detection by
limiting the SRAM size for the game to the size specified in the backup unit
header.

If you want to add patterns for games for which we already have one or more
patterns, please add them to snescopy.txt, not this function. The patches
resulting from the old (built-in) and new patterns may interfere. Either add
them disabled or make sure you don't use snescopy.txt when using -k. The best
way to test a new pattern in isolation is --pattern:
  ucon64 --pattern=snescopy.txt copyprotectedgame.swc

(original uCON)
   8f/9f XX YY 70 cf/df XX YY 70 d0
=> 8f/9f XX YY 70 cf/df XX YY 70 ea ea          if snes_sram_size == 64 kbits
=> 8f/9f XX YY 70 cf/df XX YY 70 80

   sta $70YYXX/sta $70YYXX,x; cmp $70YYXX/cmp $70YYXX,x; bne

TODO: The following three codes should be verified for many games. For example,
the first code replaces D0 (bne) with 80 (bra), but for some games (like Donkey
Kong Country (U|E)) it should do the opposite, i.e., writing EA EA (nop nop).
   8f/9f XX YY 30/31/32/33 cf/df XX YY 30/31/32/33 d0
=> 8f/9f XX YY 30/31/32/33 cf/df XX YY 30/31/32/33 80

   8f/9f XX YY 30/31/32/33 cf/df XX YY 30/31/32/33 f0     beq
=> 8f/9f XX YY 30/31/32/33 cf/df XX YY 30/31/32/33 ea ea  nop; nop

   8f/9f XX YY 30/31/32/33 af XX YY 30/31/32/33 c9 XX YY d0   bne
=> 8f/9f XX YY 30/31/32/33 af XX YY 30/31/32/33 c9 XX YY 80   bra

(uCON64)
- Mega Man X
   8f/9f XX YY 70 cf/df XX YY 70 f0
=> 8f/9f XX YY 70 cf/df XX YY 70 ea ea
The code above could be combined with the first original uCON code. However, we
don't want to copy (or remove) the SRAM size determined behaviour, without
knowing when that is necessary.

- Mega Man X
   af/bf XX 80 00 cf/df XX 80 40 f0
=> af/bf XX 80 00 cf/df XX 80 40 80

   lda $0080XX/lda $0080XX,x; cmp $408000/cmp $408000,x; beq ...

- Demon's Crest (af, 80, cf) / Breath of Fire II (bf, c0, df)
   af/bf XX ff 80/c0 cf/df XX ff 40 f0
=> af/bf XX ff 80/c0 cf/df XX ff 40 80

- Breath of Fire II (bf, 30, df, 31)
   af/bf XX YY 30/31/32/33 cf/df XX YY 30/31/32/33 f0
=> af/bf XX YY 30/31/32/33 cf/df XX YY 30/31/32/33 80

- Super Metroid
   a9 00 00 a2 fe 1f df 00 00 70 d0     lda #$0000; ldx #$1ffe; cmp $700000,x; bne ...
=> a9 00 00 a2 fe 1f df 00 00 70 ea ea  lda #$0000; ldx #$1ffe; cmp $700000,x; nop; nop

- Tetris Attack
(generic)
   8f XX YY 70 af XX YY 70 c9 XX YY d0
=> 8f XX YY 70 af XX YY 70 c9 XX YY 80
(game specific)
   c2 30 ad fc 1f c9 50 44 d0
=> c2 30 4c d1 80 c9 50 44 d0

- Uniracers/Unirally
   8f XX YY 77 e2 XX af XX YY 77 c9 XX f0
=> 8f XX YY 77 e2 XX af XX YY 77 c9 XX 80

- Mario no Super Picross
   8f/af XX YY b0 cf XX YY b1 d0
=> 8f/af XX YY b0 cf XX YY b1 ea ea

- most probably only Killer Instinct
   5c 7f d0 83 18 fb 78 c2 30           jmp $83d07f; clc; xce; sei; rep #$30
=> ea ea ea ea ea ea ea ea ea           nop; nop; nop; nop; nop; nop; nop; nop; nop

- most probably only Donkey Kong Country (8f, 30, cf, 30)
Note that this code must be searched for before the less specific uCON code.
   8f/9f 57/59 60/68 30/31/32/33 cf/df 57/59 60 30/31/32/33 d0
=> 8f/9f 57/59 60/68 30/31/32/33 cf/df 57/59 60 30/31/32/33 ea ea

- most probably only Diddy's Kong Quest
   26 38 e9 48 12 c9 af 71 f0
=> 26 38 e9 48 12 c9 af 71 80

- most probably only Diddy's Kong Quest
   a0 5c 2f 77 32 e9 c7 04 f0
=> a0 5c 2f 77 32 e9 c7 04 80

- most probably only Diddy's Kong Quest
   'K' 'O' 'N' 'G' 00 f8 f7
=> 'K' 'O' 'N' 'G' 00 f8 f8
TODO: make sense of Diddy's Kong Quest codes

- most probably only BS The Legend of Zelda Remix
   22 08 5c 10 b0 28                    jsl $105c08; bcs ...
=> ea ea ea ea ea ea                    nop; nop; nop; nop; nop; nop

- most probably only BS The Legend of Zelda Remix (enables music)
   da e2 30 c9 01 f0 18 c9 02
=> da e2 30 c9 09 f0 18 c9 07

- most probably only BS The Legend of Zelda Remix (enables music)
   29 ff 00 c9 07 00 90 16
=> 29 ff 00 c9 00 00 90 16

- most probably only Kirby's Dream Course
   ca 10 f8 38 ef 1a 80 81 8d
=> ca 10 f8 38 ef 1a 80 81 9c

- most probably only Kirby's Dream Course
   81 ca 10 f8 cf 39 80 87 f0
=> 81 ca 10 f8 cf 39 80 87 80

- probably only Earthbound
   84 26 ad 39 b5 d0 1a
=> 84 26 ad 39 b5 ea ea
I don't know what this does, but it isn't necessary to start the game.

- probably only Earthbound
   10 f8 38 ef ef ff c1
=> 10 f8 38 ea a9 00 00

- probably only Earthbound
   10 f8 38 ef f2 fd c3 f0
=> 10 f8 38 ea a9 00 00 80
Same here.

- Dixie Kong's Double Trouble (E). U version looks like it already has been "patched"
   a9 c3 80 dd ff ff f0 6c
=> a9 c3 f0 cc ff ff 80 7d

- Front Mission - Gun Hazard
   d0 f4 ab cf ae ff 00 d0 01
=> d0 f4 ab cf ae ff 00 d0 00
Modification protection. Not needed to play the game on an NTSC SNES. Needed
when it has been patched with -f.
*/
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], buffer[32 * 1024];
  FILE *srcfile, *destfile;
  int bytesread, n = 0, n_extra_patterns, n2;
  st_cm_pattern_t *patterns = NULL;

  strcpy (src_name, "snescopy.txt");
  // first try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    {
      snprintf (src_name, FILENAME_MAX, "%s" DIR_SEPARATOR_S "snescopy.txt",
                ucon64.configdir);
      src_name[FILENAME_MAX - 1] = '\0';
    }
  n_extra_patterns = build_cm_patterns (&patterns, src_name);
  if (n_extra_patterns >= 0)
    printf ("Found %d additional code%s in %s\n",
            n_extra_patterns, n_extra_patterns != 1 ? "s" : "", src_name);

  puts ("Attempting crack...");

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      fclose (srcfile);
      return -1;
    }
  if (rominfo->backup_header_len)               // copy header (if present)
    {
      unsigned char header[SWC_HEADER_LEN];

      fread (header, 1, SWC_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->backup_header_len, SEEK_SET);
      fwrite (header, 1, SWC_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)) != 0)
    {                                           // '!' == ASCII 33 (\x21), '*' == 42 (\x2a)
      // first use the extra patterns, so that their precedence is higher than
      //  the built-in patterns
      for (n2 = 0; n2 < n_extra_patterns; n2++)
        n += change_mem2 (buffer, bytesread,
                          patterns[n2].search,
                          patterns[n2].search_size,
                          patterns[n2].wildcard,
                          patterns[n2].escape,
                          patterns[n2].replace,
                          patterns[n2].replace_size,
                          patterns[n2].offset,
                          patterns[n2].sets);

      // SRAM
      if (snes_sram_size == 8 * 1024)           // 8 kB == 64 kb
        {
          n += change_mem (buffer, bytesread, "!**\x70!**\x70\xd0", 9, '*', '!', "\xea\xea", 2, 0,
                           "\x8f\x9f", 2, "\xcf\xdf", 2);
          // actually Kirby's Dream Course, Lufia II - Rise of the Sinistrals
          n += change_mem (buffer, bytesread, "!**\x70!**\x70\xf0", 9, '*', '!', "\x80", 1, 0,
                           "\x8f\x9f", 2, "\xcf\xdf", 2);
#if 1 // TODO: check which games really need this
          n += change_mem (buffer, bytesread, "!**!!**!\xf0", 9, '*', '!', "\x80", 1, 0,
                           "\x8f\x9f", 2, "\x30\x31\x32\x33", 4, "\xcf\xdf", 2, "\x30\x31\x32\x33", 4);
#endif
        }
      else
        {
          n += change_mem (buffer, bytesread, "!**\x70!**\x70\xd0", 9, '*', '!', "\x80", 1, 0,
                           "\x8f\x9f", 2, "\xcf\xdf", 2);
          // Mega Man X
          n += change_mem (buffer, bytesread, "!**\x70!**\x70\xf0", 9, '*', '!', "\xea\xea", 2, 0,
                           "\x8f\x9f", 2, "\xcf\xdf", 2);
          n += change_mem (buffer, bytesread, "!**!!**!\xf0", 9, '*', '!', "\xea\xea", 2, 0,
                           "\x8f\x9f", 2, "\x30\x31\x32\x33", 4, "\xcf\xdf", 2, "\x30\x31\x32\x33", 4);
        }

      n += change_mem (buffer, bytesread, "\x8f**\x77\xe2*\xaf**\x77\xc9*\xf0", 13, '*', '!', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "!!!!!!\x60!\xd0", 9, '*', '!', "\xea\xea", 2, 0,
                       "\x8f\x9f", 2, "\x57\x59", 2, "\x60\x68", 2, "\x30\x31\x32\x33", 4,
                       "\xcf\xdf", 2, "\x57\x59", 2, "\x30\x31\x32\x33", 4);

      n += change_mem (buffer, bytesread, "!**!!**!\xd0", 9, '*', '!', "\x80", 1, 0,
                       "\x8f\x9f", 2, "\x30\x31\x32\x33", 4, "\xcf\xdf", 2, "\x30\x31\x32\x33", 4);
      n += change_mem (buffer, bytesread, "!**\xb0\xcf**\xb1\xd0", 9, '*', '!', "\xea\xea", 2, 0,
                       "\x8f\xaf", 2);
      n += change_mem (buffer, bytesread, "!**!\xaf**!\xc9**\xd0", 12, '*', '!', "\x80", 1, 0,
                       "\x8f\x9f", 2, "\x30\x31\x32\x33", 4, "\x30\x31\x32\x33", 4);
      n += change_mem (buffer, bytesread, "\xa9\x00\x00\xa2\xfe\x1f\xdf\x00\x00\x70\xd0", 11, '*', '!', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\x8f**\x70\xaf**\x70\xc9**\xd0", 12, '*', '!', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "!**!!**!\xf0", 9, '*', '!', "\x80", 1, 0,
                       "\xaf\xbf", 2, "\x30\x31\x32\x33", 4, "\xcf\xdf", 2, "\x30\x31\x32\x33", 4);

      // mirroring
      n += change_mem (buffer, bytesread, "!*\x80\x00!*\x80\x40\xf0", 9, '*', '!', "\x80", 1, 0,
                       "\xaf\xbf", 2, "\xcf\xdf", 2);
      n += change_mem (buffer, bytesread, "!*\xff!!*\xff\x40\xf0", 9, '*', '!', "\x80", 1, 0,
                       "\xaf\xbf", 2, "\x80\xc0", 2, "\xcf\xdf", 2);

      // game specific
      n += change_mem (buffer, bytesread, "\x5c\x7f\xd0\x83\x18\xfb\x78\xc2\x30", 9, '*', '!',
                                          "\xea\xea\xea\xea\xea\xea\xea\xea\xea", 9, -8);

      n += change_mem (buffer, bytesread, "KONG\x00\xf8\xf7", 7, '*', '!', "\xf8", 1, 0);
      n += change_mem (buffer, bytesread, "\x26\x38\xe9\x48\x12\xc9\xaf\x71\xf0", 9, '*', '!', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xa0\x5c\x2f\x77\x32\xe9\xc7\x04\xf0", 9, '*', '!', "\x80", 1, 0);

      n += change_mem (buffer, bytesread, "\x22\x08\x5c\x10\xb0\x28", 6, '*', '!',
                                          "\xea\xea\xea\xea\xea\xea", 6, -5);
      n += change_mem (buffer, bytesread, "\xda\xe2\x30\xc9\x01\xf0\x18\xc9\x02", 9, '*', '!',
                                          "\x09\xf0\x18\xc9\x07", 5, -4);
      n += change_mem (buffer, bytesread, "\x29\xff\x00\xc9\x07\x00\x90\x16", 8, '*', '!', "\x00", 1, -3);

      n += change_mem (buffer, bytesread, "\xca\x10\xf8\x38\xef\x1a\x80\x81\x8d", 9, '*', '!', "\x9c", 1, 0);
      n += change_mem (buffer, bytesread, "\x81\xca\x10\xf8\xcf\x39\x80\x87\xf0", 9, '*', '!', "\x80", 1, 0);

      n += change_mem (buffer, bytesread, "\x84\x26\xad\x39\xb5\xd0\x1a", 7, '*', '!', "\xea\xea", 2, -1);
      n += change_mem (buffer, bytesread, "\x10\xf8\x38\xef\xef\xff\xc1", 7, '*', '!',
                                                      "\xea\xa9\x00\x00", 4, -3);
      n += change_mem (buffer, bytesread, "\x10\xf8\x38\xef\xf2\xfd\xc3\xf0", 8, '*', '!',
                                                      "\xea\xa9\x00\x00\x80", 5, -4);

      n += change_mem (buffer, bytesread, "\xc2\x30\xad\xfc\x1f\xc9\x50\x44\xd0", 9, '*', '!', "\x4c\xd1\x80", 3, -6);
      n += change_mem (buffer, bytesread, "\xa9\xc3\x80\xdd\xff\xff\xf0\x6c", 8, '*', '!', "\xf0\xcc\xff\xff\x80\x7d", 6, -5);
      n += change_mem (buffer, bytesread, "\xd0\xf4\xab\xcf\xae\xff\x00\xd0\x01", 9, '*', '!', "\x00", 1, 0);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);
  cleanup_cm_patterns (&patterns, n_extra_patterns);

  printf ("Found %d pattern%s\n", n, n != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return n;
}


static int
snes_fix_pal_protection (st_ucon64_nfo_t *rominfo)
/*
This function searches for PAL protection codes. If it finds one it will
fix the code so that the game will run on an NTSC SNES.
Don't touch this code if you don't know what you're doing!

Search for                            Replace with
ad 3f 21 89 10 d0                     ad 3f 21 89 10 80                - Terranigma
ad 3f 21 89 10 f0                     ad 3f 21 89 10 ea ea             - Super Metroid E
ad 3f 21 29 10 00 d0                  ad 3f 21 29 10 00 80
ad 3f 21 89 10 00 d0                  a9 10 00 89 10 00 d0             - Eric Cantona Football?
ad 3f 21 89 10 c2 XX f0               ad 3f 21 89 10 c2 XX ea ea       - Soul Blazer F/G
ad 3f 21 29 10 cf bd ff XX f0         ad 3f 21 29 10 cf bd ff XX 80    - Pop'n Twinbee E
af 3f 21 00 29 10 d0                  af 3f 21 00 29 10 80
af 3f 21 00 29 10 00 d0               af 3f 21 00 29 10 00 ea ea
af 3f 21 00 29 XX c9 XX f0            af 3f 21 00 29 XX c9 XX 80       - Secret of Mana E
a2 18 01 bd 27 20 89 10 00 f0 01      a2 18 01 bd 27 20 89 10 00 ea ea - Donkey Kong Country E
*/
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], buffer[32 * 1024];
  FILE *srcfile, *destfile;
  int bytesread, n = 0, n_extra_patterns, n2;
  st_cm_pattern_t *patterns = NULL;

  strcpy (src_name, "snespal.txt");
  // first try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    {
      snprintf (src_name, FILENAME_MAX, "%s" DIR_SEPARATOR_S "snespal.txt",
                ucon64.configdir);
      src_name[FILENAME_MAX - 1] = '\0';
    }
  n_extra_patterns = build_cm_patterns (&patterns, src_name);
  if (n_extra_patterns >= 0)
    printf ("Found %d additional code%s in %s\n",
            n_extra_patterns, n_extra_patterns != 1 ? "s" : "", src_name);

  puts ("Attempting to fix PAL protection code...");

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      fclose (srcfile);
      return -1;
    }
  if (rominfo->backup_header_len)               // copy header (if present)
    {
      unsigned char header[SWC_HEADER_LEN];

      fread (header, 1, SWC_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->backup_header_len, SEEK_SET);
      fwrite (header, 1, SWC_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)) != 0)
    {
      // first use the extra patterns, so that their precedence is higher than
      //  the built-in patterns
      for (n2 = 0; n2 < n_extra_patterns; n2++)
        n += change_mem2 (buffer, bytesread,
                          patterns[n2].search,
                          patterns[n2].search_size,
                          patterns[n2].wildcard,
                          patterns[n2].escape,
                          patterns[n2].replace,
                          patterns[n2].replace_size,
                          patterns[n2].offset,
                          patterns[n2].sets);

      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x89\x10\xd0", 6, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x89\x10\xf0", 6, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x29\x10\x00\xd0", 7, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x89\x10\x00\xd0", 7, '\x01', '\x02', "\xa9\x10\x00", 3, -6);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x89\x10\xc2\x01\xf0", 8, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x29\x10\xcf\xbd\xff\x01\xf0", 10, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\x00\x29\x10\xd0", 7, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\x00\x29\x10\x00\xd0", 8, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\x00\x29\x01\xc9\x01\xf0", 9, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xa2\x18\x01\xbd\x27\x20\x89\x10\x00\xf0\x01", 11, '*', '!', "\xea\xea", 2, -1);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);
  cleanup_cm_patterns (&patterns, n_extra_patterns);

  printf ("Found %d pattern%s\n", n, n != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return n;
}


static int
snes_fix_ntsc_protection (st_ucon64_nfo_t *rominfo)
/*
This function searches for NTSC protection codes. If it finds one it will
fix the code so that the game will run on a PAL SNES.
Don't touch this code if you don't know what you're doing!

Search for                            Replace with
3f 21 29/89 10 f0                     3f 21 29/89 10 80
ad 3f 21 29 10 d0                     ad 3f 21 29 10 ea ea
ad 3f 21 89 10 d0                     ad 3f 21 89 10 80/(ea ea)        - Live-a-Live (ea ea)
3f 21 29/89 10 00 f0                  3f 21 29/89 10 00 80             - Clock Tower (29)
3f 21 29/89 10 00 d0                  3f 21 29/89 10 00 ea ea          - Mario no Super Picross (89)
   3f 21 89 10 c2 XX f0                  3f 21 89 10 c2 XX 80          - Front Mission - Gun Hazard
   3f 21 89 10 c2 XX d0                  3f 21 89 10 c2 XX ea ea       - Robotrek
3f 21 29/89 10 c9 10 f0               3f 21 29/89 10 c9 10 80
ad 3f 21 29 10 c9 00 f0               ad 3f 21 29 10 c9 00 80/(ea ea) <= original uCON used 80
ad 3f 21 29 10 c9 00 d0               ad 3f 21 29 10 c9 00 80
ad 3f 21 29 10 c9 10 d0               ad 3f 21 29 10 c9 10 ea ea
   3f 21 29 10 cf XX YY 80 f0            3f 21 29 10 cf XX YY 80 80    - Gokujyou Parodius/Tokimeki Memorial
ad 3f 21 8d XX YY 29 10 8d            ad 3f 21 8d XX YY 29 00 8d       - Dragon Ball Z - Super Butoden 2?
   3f 21 00 29/89 10 f0                  3f 21 00 29/89 10 80          - Kirby's Dream Course U (29)
af 3f 21 00 29/89 10 d0               af 3f 21 00 29/89 10 ea ea       - Kirby No Kira Kizzu (29)/Final Fight Guy (89)
af 3f 21 00 29/89 10 00 f0            af 3f 21 00 29/89 10 00 80
af 3f 21 00 29 XX c9 XX f0            af 3f 21 00 29 XX c9 XX 80       - Seiken Densetsu 3
af 3f 21 00 29 10 80 2d 00 1b         af 3f 21 00 29 00 80 2d 00 1b    - Seiken Densetsu 2/Secret of Mana U
   3f 21 00 89 10 c2 XX f0               3f 21 00 89 10 c2 XX 80       - Dragon - The Bruce Lee Story U
af 3f 21 00 XX YY 29 10 00 d0         af 3f 21 00 XX YY 29 10 00 ea ea - Fatal Fury Special?
   3f 21 c2 XX 29 10 00 f0               3f 21 c2 XX 29 10 00 80       - Metal Warriors
   3f 21 c2 XX 29 10 00 d0               3f 21 c2 XX 29 10 00 ea ea    - Dual Orb 2
af 3f 21 ea 89 10 00 d0               a9 00 00 ea 89 10 00 d0          - Super Famista 3?
a2 18 01 bd 27 20 89 10 00 d0 01      a2 18 01 bd 27 20 89 10 00 ea ea - Donkey Kong Country U
29 10 00 a2 00 00 c9 10 00 d0         29 10 00 a2 00 00 c9 10 00 80    - Wolfenstein 3D U
*/
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], buffer[32 * 1024];
  FILE *srcfile, *destfile;
  int bytesread, n = 0, n_extra_patterns, n2;
  st_cm_pattern_t *patterns = NULL;

  strcpy (src_name, "snesntsc.txt");
  // first try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    {
      snprintf (src_name, FILENAME_MAX, "%s" DIR_SEPARATOR_S "snesntsc.txt",
                ucon64.configdir);
      src_name[FILENAME_MAX - 1] = '\0';
    }
  n_extra_patterns = build_cm_patterns (&patterns, src_name);
  if (n_extra_patterns >= 0)
    printf ("Found %d additional code%s in %s\n",
            n_extra_patterns, n_extra_patterns != 1 ? "s" : "", src_name);

  puts ("Attempting to fix NTSC protection code...");

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      fclose (srcfile);
      return -1;
    }
  if (rominfo->backup_header_len)               // copy header (if present)
    {
      unsigned char header[SWC_HEADER_LEN];

      fread (header, 1, SWC_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->backup_header_len, SEEK_SET);
      fwrite (header, 1, SWC_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)) != 0)
    {
      // first use the extra patterns, so that their precedence is higher than
      //  the built-in patterns
      for (n2 = 0; n2 < n_extra_patterns; n2++)
        n += change_mem2 (buffer, bytesread,
                          patterns[n2].search,
                          patterns[n2].search_size,
                          patterns[n2].wildcard,
                          patterns[n2].escape,
                          patterns[n2].replace,
                          patterns[n2].replace_size,
                          patterns[n2].offset,
                          patterns[n2].sets);

      n += change_mem (buffer, bytesread, "\x3f\x21\x02\x10\xf0", 5, '\x01', '\x02', "\x80", 1, 0,
                       "\x29\x89", 2);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x29\x10\xd0", 6, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x89\x10\xd0", 6, '\x01', '\x02', "\xea\xea", 2, 0);
// The next statement could be the alternative for the previous one. Leave it
//  disabled until we find a game that needs it.
//      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x89\x10\xd0", 6, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\x3f\x21\x02\x10\x00\xf0", 6, '\x01', '\x02', "\x80", 1, 0,
                       "\x29\x89", 2);
      n += change_mem (buffer, bytesread, "\x3f\x21\x02\x10\x00\xd0", 6, '\x01', '\x02', "\xea\xea", 2, 0,
                       "\x29\x89", 2);
      n += change_mem (buffer, bytesread, "\x3f\x21\x89\x10\xc2\x01\xf0", 7, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\x3f\x21\x89\x10\xc2\x01\xd0", 7, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\x3f\x21\x02\x10\xc9\x10\xf0", 7, '\x01', '\x02', "\x80", 1, 0,
                       "\x29\x89", 2);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x29\x10\xc9\x00\xf0", 8, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x29\x10\xc9\x00\xd0", 8, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x29\x10\xc9\x10\xd0", 8, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\x3f\x21\x29\x10\xcf\x01\x01\x80\xf0", 9, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xad\x3f\x21\x8d\x01\x01\x29\x10\x8d", 9, '\x01', '\x02', "\x00", 1, -1);
      n += change_mem (buffer, bytesread, "\x3f\x21\x00\x02\x10\xf0", 6, '\x01', '\x02', "\x80", 1, 0,
                       "\x29\x89", 2);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\x00\x02\x10\xd0", 7, '\x01', '\x02', "\xea\xea", 2, 0,
                       "\x29\x89", 2);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\x00\x02\x10\x00\xf0", 8, '\x01', '\x02', "\x80", 1, 0,
                       "\x29\x89", 2);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\x00\x29\x01\xc9\x01\xf0", 9, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\x00\x29\x10\x80\x2d\x00\x1b", 10, '\x01', '\x02', "\x00", 1, -4);
      n += change_mem (buffer, bytesread, "\x3f\x21\x00\x89\x10\xc2\x01\xf0", 8, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\x00\x01\x01\x29\x10\x00\xd0", 10, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\x3f\x21\xc2\x01\x29\x10\x00\xf0", 8, '\x01', '\x02', "\x80", 1, 0);
      n += change_mem (buffer, bytesread, "\x3f\x21\xc2\x01\x29\x10\x00\xd0", 8, '\x01', '\x02', "\xea\xea", 2, 0);
      n += change_mem (buffer, bytesread, "\xaf\x3f\x21\xea\x89\x10\x00\xd0", 8, '\x01', '\x02', "\xa9\x00\x00", 3, -7);
      n += change_mem (buffer, bytesread, "\xa2\x18\x01\xbd\x27\x20\x89\x10\x00\xd0\x01", 11, '*', '!', "\xea\xea", 2, -1);
      n += change_mem (buffer, bytesread, "\x29\x10\x00\xa2\x00\x00\xc9\x10\x00\xd0", 10, '\x01', '\x02', "\x80", 1, 0);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);
  cleanup_cm_patterns (&patterns, n_extra_patterns);

  printf ("Found %d pattern%s\n", n, n != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return n;
}


int
snes_f (st_ucon64_nfo_t *rominfo)
// See the document "src/backup/NTSC-PAL notes.txt".
{
  switch (snes_header.country)
    {
    // In the Philipines the television standard is NTSC, but do games made
    //  for the Philipines exist?
    case 0:                                     // Japan
    case 1:                                     // U.S.A.
      return snes_fix_ntsc_protection (rominfo);
    default:
      return snes_fix_pal_protection (rominfo);
    }
}


int
snes_l (st_ucon64_nfo_t *rominfo)
/*
The order is important. Don't touch this code if you don't know what you're doing!

Search for                      Replace with
(uCON64)
8c/8d/8e/8f 0d 42               9c 0d 42
01 0d 42                        00 0d 42
a9 01 85 0d                     a9 00 85 0d // special one (used by Konami and Jaleco? sometimes)
a2 01 86 0d                     a2 00 86 0d
a0 01 84 0d                     a0 00 84 0d

(original uCON)
a9/a2 01 8d/8e 0d 42            a9/a2 00 8d/8e 0d 42
a9 01 00 8d 0d 42               a9 00 00 8d 0d 42
a9 01 8f 0d 42 00               a9 00 8f 0d 42 00
*/
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], buffer[32 * 1024];
  FILE *srcfile, *destfile;
  int bytesread, n = 0, n_extra_patterns, n2;
  st_cm_pattern_t *patterns = NULL;

  strcpy (src_name, "snesslow.txt");
  // first try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    {
      snprintf (src_name, FILENAME_MAX, "%s" DIR_SEPARATOR_S "snesslow.txt",
                ucon64.configdir);
      src_name[FILENAME_MAX - 1] = '\0';
    }
  n_extra_patterns = build_cm_patterns (&patterns, src_name);
  if (n_extra_patterns >= 0)
    printf ("Found %d additional code%s in %s\n",
            n_extra_patterns, n_extra_patterns != 1 ? "s" : "", src_name);

  puts ("Attempting SlowROM fix...");

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      fclose (srcfile);
      return -1;
    }
  if (rominfo->backup_header_len)               // copy header (if present)
    {
      unsigned char header[SWC_HEADER_LEN];

      fread (header, 1, SWC_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->backup_header_len, SEEK_SET);
      fwrite (header, 1, SWC_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)) != 0)
    {                                           // '!' == ASCII 33 (\x21), '*' == 42 (\x2a)
      // first use the extra patterns, so that their precedence is higher than
      //  the built-in patterns
      for (n2 = 0; n2 < n_extra_patterns; n2++)
        n += change_mem2 (buffer, bytesread,
                          patterns[n2].search,
                          patterns[n2].search_size,
                          patterns[n2].wildcard,
                          patterns[n2].escape,
                          patterns[n2].replace,
                          patterns[n2].replace_size,
                          patterns[n2].offset,
                          patterns[n2].sets);

      n += change_mem (buffer, bytesread, "!\x0d\x42", 3, '*', '!', "\x9c", 1, -2,
                       "\x8c\x8d\x8e\x8f", 4);
      n += change_mem (buffer, bytesread, "\x01\x0d\x42", 3, '*', '!', "\x00", 1, -2);
      n += change_mem (buffer, bytesread, "\xa9\x01\x85\x0d", 4, '*', '!', "\x00", 1, -2);
      n += change_mem (buffer, bytesread, "\xa2\x01\x86\x0d", 4, '*', '!', "\x00", 1, -2);
      n += change_mem (buffer, bytesread, "\xa0\x01\x84\x0d", 4, '*', '!', "\x00", 1, -2);

      // original uCON
      n += change_mem (buffer, bytesread, "!\x01!\x0d\x42", 5, '*', '!', "\x00", 1, -3,
                       "\xa9\xa2", 2, "\x8d\x8e", 2);
      n += change_mem (buffer, bytesread, "\xa9\x01\x00\x8d\x0d\x42", 6, '*', '!', "\x00", 1, -4);
      n += change_mem (buffer, bytesread, "\xa9\x01\x8f\x0d\x42\x00", 6, '*', '!', "\x00", 1, -4);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);
  cleanup_cm_patterns (&patterns, n_extra_patterns);

  printf ("Found %d pattern%s\n", n, n != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return n;
}


int
snes_n (st_ucon64_nfo_t *rominfo, const char *name)
{
  char buf[SNES_NAME_LEN], dest_name[FILENAME_MAX];
  size_t name_len = (bs_dump || st_dump) ? 16 : SNES_NAME_LEN,
         len = strlen (name);

  memset (buf, ' ', name_len);
  if (len > name_len)
    len = name_len;
  strncpy (buf, name, len);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);

  fcopy (ucon64.fname, 0, (size_t) ucon64.file_size, dest_name, "wb");
  ucon64_fwrite (buf,
                 get_header_start (rominfo, (unsigned int) ucon64.file_size -
                                     rominfo->backup_header_len) +
                   rominfo->backup_header_len + 16,
                 name_len, dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
snes_chk (st_ucon64_nfo_t *rominfo)
{
  char dest_name[FILENAME_MAX];
  unsigned char sum[4];
  unsigned int header_start;

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, (size_t) ucon64.file_size, dest_name, "wb");

  header_start = update_chksum (rominfo, sum, (unsigned int) ucon64.file_size -
                                  rominfo->backup_header_len, dest_name);
  dumper (stdout, sum, 4, header_start + rominfo->backup_header_len + 44,
          DUMPER_HEX);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


static int
snes_check_interleaved (unsigned char *rom_buffer, int size, int banktype_score)
/*
  The only way to determine whether a HiROM dump is interleaved or not seems to
  be to check the value of the map type byte. Valid HiROM values (hexadecimal):
  21, 31, 35, 3a
  Valid LoROM values:
  20, 23, 30, 32, 44 [, 41, 53]
  41 is the hexadecimal value of 'A' (WWF Super Wrestlemania (E)). 53 is the
  hexadecimal value value of 'S' (Contra III - The Alien Wars (U)).
  So, if a ROM dump seems LoROM, but the map type byte is that of a HiROM dump
  we assume it is interleaved. Interleaved LoROM dumps are not produced by any
  copier, but by incorrect ROM tools...
*/
{
  int interleaved = 0, check_map_type = 1;
  unsigned int crc;

  if (size < 64 * 1024)                         // snes_deinterleave() reads blocks of 32 kB
    return 0;                                   // file cannot be interleaved

  crc = crc32 (0, rom_buffer, 512);
  /*
    Special case hell

    0x4a70ad38: Double Dragon, Return of (J), Super Double Dragon (E/U) {[!], [a1]}
    0x0b34ddad: Kakinoki Shogi (J)
    0x348b5357: King of Rally, The (J)
    0xc39b8d3a: Pro Kishi Simulation Kishi no Hanamichi (J)
    0xbd7bc39f: Shin Syogi Club (J)
    0x9b4638d0: Street Fighter Alpha 2 (E/U) {[b1]}, Street Fighter Zero 2 (J)
    Only really necessary for (U). The other versions can be detected because
    one of the two internal headers has checksum bytes ff ff 00 00.
    0x0085b742: Super Bowling (U)
    0x30cbf83c: Super Bowling (J)
    These games have two headers.

    BUG ALERT: We don't check for 0xbd7bc39f. The first 512 bytes of what
    uCON64 detects as the interleaved dump of Shin Syogi Club (J) are identical
    to the first 512 bytes of what we detect as the uninterleaved dump of
    Kakinoki Shogi (J). We prefer uninterleaved dumps. Besides, concluding a
    dump is interleaved if the first 512 bytes have CRC32 0xbd7bc39f would mess
    up the detection of some BS dumps. See below.

    0x7039388a: Ys 3 - Wanderers from Ys (J)
    This game has 31 internal headers...

    0xd7470b37/0x9f1d6284: Dai Kaiju Monogatari 2 (J) (GD3/UFO)
    0xa2c5fd29/0xfe536fc9: Tales of Phantasia (J) (GD3/UFO)
    These are Extended HiROM games. By "coincidence" ToP can be detected in
    another way, but DKM2 (40 Mbit) can't. The CRC32s are checked for below.

    0xdbc88ebf: BS Satella2 1 (J)
    This game has a LoROM map type byte while it is a HiROM game.

    0x29226b62: BS Busters - Digital Magazine 5-24-98 (J),
                BS Do-Re-Mi No.2 5-10 (J),
                BS Do-Re-Mi No.2 5-25 (J),
                BS Furoito No Chousenjou {2, 3, 4, 5, 6} (J),
                BS Nintendo HP 5-17 (J),
                BS Nintendo HP 5-31 (J)
    0xbd7bc39f: BS Goods Press 6 Gatsu Gou (J),
                BS NP Magazine 107 (J),
                BS Tora no Maki 5-17 (J),
                BS Tora no Maki 5-31 (J)
    0x4ef3d27b: BS Lord Monarke (J)
    These games are *not* special cases. uCON64 detects them correctly, but the
    tool that was used to create GoodSNES - 0.999.5 for RC 2.5.dat, does not.
    This has been verified on a real SNES for the games with CRC 0x29226b62 and
    0x4ef3d27b. The games with CRC 0xbd7bc39f don't seem to run on a copier.

    0xc3194ad7: Yu Yu No Quiz De Go! Go! (J)
    0x89d09a77: Infernal's Evil Demo! (PD)
    0xd3095af3: Legend - SNDS Info, Incredible Hulk Walkthru (PD)
    0x9b161d4d: Pop 'N Twinbee Sample (J)
    0x6910700a: Rock Fall (PD)
    0x447df9d5: SM Choukyousi Hitomi (PD)
    0x02f401df: SM Choukyousi Hitomi Vol 2 (PD)
    0xf423997a: World of Manga 2 (PD)
    These games/dumps have a HiROM map type byte while they are LoROM.

    0x0f802e41: Mortal Kombat 3 Final (Anthrox Beta Hack)
    0xbd8f1b20: Rise of the Robots (Beta)
    0x05926d17: Shaq Fu (E)/(J)(NG-Dump Known)
    0x3e2e5619: Super Adventure Island II (Beta)
    0x023e1298: Super Air Driver (E) [b]
    These are also not special cases (not: HiROM map type byte + LoROM game).
    GoodSNES - 0.999.5 for RC 2.5.dat simply contains errors.

    0x2a4c6a9b: Super Noah's Ark 3D (U)
    0xfa83b519: Mortal Kombat (Beta)
    0xf3aa1eca: Power Piggs of the Dark Age (Pre-Release) {[h1]}
    0x65485afb: Super Aleste (J) {[t1]} <= header == trainer
    0xaad23842/0x5ee74558: Super Wild Card DX DOS ROM V1.122/interleaved
    0x422c95c4: Time Slip (Beta)
    0x7a44bd18: Total Football (E)(NG-Dump Known)
    0xf0bf8d7c/0x92180571: Utyu no Kishi Tekkaman Blade (Beta) {[h1]}/interleaved
    0x8e1933d0: Wesley Orangee Hotel (PD)
    0xe2b95725/0x9ca5ed58: Zool (Sample Cart)/interleaved
    These games/dumps have garbage in their header.
  */
  if (crc == 0xc3194ad7
#ifdef  DETECT_NOTGOOD_DUMPS
      ||
      crc == 0x89d09a77 || crc == 0xd3095af3 || crc == 0x9b161d4d ||
      crc == 0x6910700a || crc == 0x447df9d5 || crc == 0x02f401df ||
      crc == 0xf423997a || crc == 0xfa83b519 || crc == 0xf3aa1eca ||
      crc == 0xaad23842 || crc == 0x422c95c4 || crc == 0x7a44bd18 ||
      crc == 0xf0bf8d7c || crc == 0x8e1933d0 || crc == 0xe2b95725
#endif
     )
    check_map_type = 0;                         // not interleaved
  else if (crc == 0x4a70ad38 || crc == 0x0b34ddad || crc == 0x348b5357 ||
           crc == 0xc39b8d3a || crc == 0x9b4638d0 || crc == 0x0085b742 ||
           crc == 0x30cbf83c || crc == 0x7039388a || crc == 0xdbc88ebf ||
           crc == 0x2a4c6a9b
#ifdef  DETECT_NOTGOOD_DUMPS
           ||
           crc == 0x65485afb || crc == 0x5ee74558 || crc == 0x92180571 ||
           crc == 0x9ca5ed58
#endif
          )
    {
      interleaved = 1;
      snes_hirom = 0;
      snes_hirom_ok = 1;
      check_map_type = 0;                       // interleaved
    }
  // WARNING: st_dump won't be set if it's an interleaved dump
  else if (st_dump)
    check_map_type = 0;
  else
    {
#ifdef  DETECT_SMC_COM_FUCKED_UP_LOROM
      if (check_banktype (rom_buffer, size / 2) > banktype_score)
        {
          interleaved = 1;
          snes_hirom = 0;
          snes_hirom_ok = 1;                    // keep snes_deinterleave()
          check_map_type = 0;                   //  from changing snes_hirom
        }
#endif
#ifdef  DETECT_INSNEST_FUCKED_UP_LOROM
      /*
        "the most advanced and researched Super Nintendo ROM utility available"
        What a joke. They don't support their own "format"...
        For some games we never reach this code, because the previous code
        detects them (incorrectly). I (dbjh) don't think there are games in
        this format available on the internet, so I won't add special-case code
        (like CRC32 checks) to fix that -- it's a bug in inSNESt. Examples are:
        Lufia II - Rise of the Sinistrals (H)
        Super Mario All-Stars & World (E) [!]
      */
      if (!interleaved && size == 24 * MBIT &&
          check_banktype (rom_buffer, 16 * MBIT) > banktype_score)
        {
          interleaved = 1;
          snes_hirom = 0;
          snes_hirom_ok = 2;                    // fix for snes_deinterleave()
          check_map_type = 0;
        }
#endif
    }
  if (check_map_type && !snes_hirom)
    {
      // first check if it's an interleaved Extended HiROM dump
      if (ucon64.file_size >= SNES_HEADER_START + SNES_EROM + SNES_HEADER_LEN)
        {
          // don't set snes_header_base to SNES_EROM for too small files (split files)
          if (crc == 0xd7470b37 || crc == 0xa2c5fd29) // GD3
            snes_header_base = SNES_EROM;
          else if (crc == 0x9f1d6284 || crc == 0xfe536fc9) // UFO
            {
              snes_header_base = SNES_EROM;
              interleaved = 1;
            }
        }
      if (snes_header.map_type == 0x21 || snes_header.map_type == 0x31 ||
          snes_header.map_type == 0x35 || snes_header.map_type == 0x3a ||
          snes_header.bs_map_type == 0x21 || snes_header.bs_map_type == 0x31)
        interleaved = 1;
    }

  return interleaved;
}


static int
snes_deinterleave (st_ucon64_nfo_t *rominfo, unsigned char **rom_buffer,
                   size_t rom_size)
{
  uint16_t blocks[512] = { 0 }, i, nblocks = rom_size >> 16; // # 32 kB blocks / 2
  unsigned char *rom_buffer2;

  if (nblocks * 2 > 512)
    return -1;                                  // file > 16 MB

  if (rominfo->interleaved == 2)                // SFX(2) games (Doom, Yoshi's Island)
    {
      for (i = 0; i < nblocks * 2; i++)
        {
          blocks[i] = (i & ~0x1e) | ((i & 2) << 2) | ((i & 4) << 2) |
                      ((i & 8) >> 2) | ((i & 16) >> 2);
          if (blocks[i] * 0x8000U + 0x8000U > rom_size)
            {
              puts ("WARNING: This ROM cannot be handled as if it is in interleaved format 2");
              rominfo->interleaved = 0;
              return -1;
            }
        }
    }
  else // rominfo->interleaved == 1
    {
      int blocksset = 0;

      if (!snes_hirom_ok)
        {
          snes_hirom = SNES_HIROM;
          snes_hirom_ok = 1;
        }

      if (copier_type == GD3 || rominfo->backup_header_len == 0)
        {
          // deinterleaving schemes specific for the Game Doctor
          if ((snes_hirom || snes_hirom_ok == 2) && rom_size == 24 * MBIT)
            {
              for (i = 0; i < nblocks; i++)
                {
                  blocks[i * 2] = i + ((i < (16 * MBIT >> 16) ? 16 : 4) * (uint16_t) (MBIT >> 15));
                  blocks[i * 2 + 1] = i;
                }
              blocksset = 1;
            }
          else if (snes_header_base == SNES_EROM)
            {
              size_t size2 = rom_size - 32 * MBIT; // size of second ROM
              uint16_t j = 32 * MBIT >> 16;
              for (i = 0; i < j; i++)
                {
                  blocks[i * 2] = i + j + (uint16_t) (size2 >> 15);
                  blocks[i * 2 + 1] = i + (uint16_t) (size2 >> 15);
                }
              j = size2 >> 16;
              for (; i < j + (32 * MBIT >> 16); i++)
                {
                  blocks[i * 2] = i + j - (32 * MBIT >> 16);
                  blocks[i * 2 + 1] = i - (32 * MBIT >> 16);
                }
              blocksset = 1;
            }
        }
      if (!blocksset)
        for (i = 0; i < nblocks; i++)
          {
            blocks[i * 2] = i + nblocks;
            blocks[i * 2 + 1] = i;
          }
    }

  if ((rom_buffer2 = (unsigned char *) malloc (rom_size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], rom_size);
      exit (1);
    }
  for (i = 0; i < nblocks * 2; i++)
    memcpy (rom_buffer2 + i * 0x8000, *rom_buffer + blocks[i] * 0x8000, 0x8000);

  free (*rom_buffer);
  *rom_buffer = rom_buffer2;
  return 0;
}


static const char *
matches_deviates (int equal)
{
  return equal ?
#ifdef  USE_ANSI_COLOR
    ucon64.ansi_color ? "\x1b[01;32mMatches\x1b[0m" : "Matches" :
    ucon64.ansi_color ? "\x1b[01;33mDeviates\x1b[0m" : "Deviates";
#else
    "Matches" : "Deviates";
#endif
}


int
snes_backup_header_info (st_ucon64_nfo_t *rominfo)
// -dbuh
{
  unsigned char header[512];
  int x = 0, y;
  snes_copier_t org_copier_type = copier_type;

  if (rominfo->backup_header_len == 0)          // copier_type == MGD_SNES
    {
      puts ("NOTE: This ROM has no backup unit header");
      return -1;
    }
  else
    {
      printf ("Backup unit header info (%s)\n\n",
              copier_type == SWC ? "SWC" :
              copier_type == IC2 ? "IC2" :
              copier_type == FIG ? "FIG" :
              copier_type == GD3 ? "GD3" :
              copier_type == UFO ? "UFO" :
              copier_type == UFOSD ? "UFOSD" :
              "unknown header type, but interpreted as SWC");
      if (copier_type == SMC)
        copier_type = SWC;
    }

  ucon64_fread (header, 0, 512, ucon64.fname);
  dumper (stdout, header, 64, 0, DUMPER_HEX);   // show only the part that is
  fputc ('\n', stdout);                         //  interpreted by copier

  if (copier_type == SWC || copier_type == IC2 || copier_type == FIG ||
      copier_type == UFO)
    {
      x = (int) ucon64.file_size - rominfo->backup_header_len;
      y = (header[0] + (header[1] << 8)) * 8 * 1024;
      printf ("[0-1]    File size: %d Bytes (%.4f Mb) => %s\n",
              y, TOMBIT_F (y), matches_deviates (x == y));
    }
  else if (copier_type == UFOSD)
    {
      x = (int) ucon64.file_size - rominfo->backup_header_len;
      y = header[0] * MBIT;
      printf ("[0]      File size: %d Bytes (%.4f Mb) => %s\n",
              y, TOMBIT_F (y), matches_deviates (x == y));
    }

  if (copier_type == SWC)
    {
      st_swc_header_t *swc_header = (st_swc_header_t *) header;
      int z;
      unsigned char sram_sizes[] = { 0, 2, 8, 32 };

      if (swc_header->emulation & 0x80)         // bit 7 has higher precedence
        z = 0;                                  //  than bit 1
      else if (swc_header->emulation & 0x02)
        z = 2;
      else
        z = 3;

      printf ("[2:7]    Run program in mode: %d", z);
      if (z == 2)
        puts (" (bit 1=1)");
      else
        fputc ('\n', stdout);

      y = swc_header->emulation & 0x40 ? 1 : 0;
      // ucon64.split is always set at this point, see snes_init()
      printf ("[2:6]    Split: %s => %s\n",
              y ? "Yes" : "No", matches_deviates ((ucon64.split ? 1 : 0) == y));

      x = snes_hirom ? 1 : 0;
      y = swc_header->emulation & 0x20 ? 1 : 0;
      printf ("[2:5]    SRAM mapping mode: %s => %s\n",
              y ? "HiROM" : "LoROM", matches_deviates (x == y));

      y = swc_header->emulation & 0x10 ? 1 : 0;
      printf ("[2:4]    DRAM mapping mode: %s => %s\n",
              y ? "HiROM" : "LoROM", matches_deviates (x == y));

      y = sram_sizes[(~swc_header->emulation & 0x0c) >> 2]; // 32 => 12, 8 => 8, 2 => 4, 0 => 0
      printf ("[2:3-2]  SRAM size: %d kB => %s\n",
              y, matches_deviates (snes_sram_size == y * 1024));

      printf ("[2:1]    Run program in mode: %d", z);
      if (z == 0)
        puts (" (bit 7=1)");
      else
        fputc ('\n', stdout);

      printf ("[2:0]    External cartridge memory: %s\n",
              swc_header->emulation & 0x01 ? "Enabled" : "Disabled");
    }
  else if (copier_type == IC2)
    {
      st_swc_header_t *swc_header = (st_swc_header_t *) header;

      y = swc_header->emulation & 0x40 ? 1 : 0;
      // ucon64.split is always set at this point, see snes_init()
      printf ("[2:6]    Split: %s => %s\n",
              y ? "Yes" : "No", matches_deviates ((ucon64.split ? 1 : 0) == y));
    }
  else if (copier_type == FIG)
    {
      st_fig_header_t *fig_header = (st_fig_header_t *) header;

      y = fig_header->multi & 0x40 ? 1 : 0;
      // ucon64.split is always set at this point, see snes_init()
      printf ("[2]      Split: %s => %s\n",
              y ? "Yes" : "No", matches_deviates ((ucon64.split ? 1 : 0) == y));

      y = fig_header->hirom & 0x80 ? 1 : 0;
      printf ("[3]      Memory mapping mode: %s => %s\n",
              y ? "HiROM" : "LoROM", matches_deviates ((snes_hirom ? 1 : 0) == y));

      y = -1;
      if (snes_hirom)
        {
          if ((fig_header->emulation1 == 0x77 || fig_header->emulation1 == 0xf7) &&
              fig_header->emulation2 == 0x83)
            y = 0;
          else if ((fig_header->emulation1 == 0xdd || fig_header->emulation1 == 0xfd) &&
                   fig_header->emulation2 == 0x82)
            y = 8; // or 2
          else if (fig_header->emulation1 == 0xdd && fig_header->emulation2 == 0x02)
            y = 32;
        }
      else
        {
          if ((fig_header->emulation1 == 0x77 || fig_header->emulation1 == 0x47) &&
              fig_header->emulation2 == 0x83)
            y = 0;
          else if ((fig_header->emulation1 == 0x00 || fig_header->emulation1 == 0x40) &&
                   fig_header->emulation2 == 0x80)
            y = 8; // or 2
          else if (((fig_header->emulation1 == 0x00 || fig_header->emulation1 == 0x40) &&
                    fig_header->emulation2 == 0x00) ||
                   (fig_header->emulation1 == 0x11 && fig_header->emulation2 == 0x02))
            y = 32;
        }

      if (y == 8)
        printf ("[4-5]    SRAM size: 2 kB / 8 kB => %s\n",
                matches_deviates (snes_sram_size == 2 * 1024 ||
                                  snes_sram_size == 8 * 1024));
      else
        printf ("[4-5]    SRAM size: %d kB => %s\n",
                y, matches_deviates (snes_sram_size == y * 1024));
    }
  else if (copier_type == UFO)
    {
      st_ufo_header_t *ufo_header = (st_ufo_header_t *) header;
      unsigned char sram_sizes[] = { 0, 2, 8, 32 };
      int z;

      y = ufo_header->multi ? 1 : 0;
      // ucon64.split is always set at this point, see snes_init()
      printf ("[2]      Split: %s => %s\n",
              y ? "Yes" : "No", matches_deviates ((ucon64.split ? 1 : 0) == y));

      y = ufo_header->uses_sram;
      printf ("[10]     SRAM: %s => %s\n",
              y == 1 ? "Yes" : y == 0 ? "No" : "Unknown",
              matches_deviates ((snes_sram_size ? 1 : 0) == y));

      y = ufo_header->size * MBIT;
      printf ("[11]     ROM size: %d Bytes (%d.0000 Mb) => %s\n",
              y, y / MBIT, matches_deviates (x == y));

      y = ufo_header->banktype;
      printf ("[12]     DRAM mapping mode: %s => %s\n",
              y == 1 ? "LoROM" : y == 0 ? "HiROM" : "Unknown",
              matches_deviates ((snes_hirom ? 0 : 1) == y));

      y = ufo_header->sram_size <= 3 ? sram_sizes[ufo_header->sram_size] : 128;
      printf ("[13]     SRAM size: %d kB => %s\n",
              y, matches_deviates (snes_sram_size == y * 1024));

      y = ufo_header->sram_a15;
      if (y)
        printf ("[14]     A15=%s selects SRAM\n", y == 1 ? "x" : y == 2 ? "0" : "1");
      else
        puts ("[14]     A15 not used for SRAM control");
      for (x = 0; x < 4; x++)
        {
          if (x & 1)
            {
              y = header[0x15 + x / 2] & 3;
              z = 0;
            }
          else
            {
              y = header[0x15 + x / 2] >> 2;
              z = 2;
            }
          if (y != 1)
            printf ("[%x:%d-%d] A%d=%s selects SRAM\n",
                    0x15 + x / 2, z + 1, z, 20 + (x | 1) - (x & 1),
                    y == 0 ? "x" : y == 2 ? "0" : "1");
        }

      y = ufo_header->sram_type;
      printf ("[17]     SRAM mapping mode: %s => %s\n",
              y == 3 ? "LoROM" : y == 0 ? "HiROM" : "Unknown",
              matches_deviates ((snes_hirom ? 0 : 3) == y));
    }
  else if (copier_type == UFOSD)
    {
      st_ufosd_header_t *ufosd_header = (st_ufosd_header_t *) header;
      unsigned char sram_sizes[] = { 0, 2, 8, 32 };

      y = ufosd_header->banktype_copy;
      printf ("[2]      DRAM mapping mode: %s => %s\n",
              y == 1 ? "LoROM" : y == 0 ? "HiROM" : "Unknown",
              matches_deviates ((snes_hirom ? 0 : 1) == y));

      y = ufosd_header->internal_size;
      printf ("[11]     Internal size: %d Mb => %s\n",
              y, matches_deviates (get_internal_size () == y));

      y = ufosd_header->sram_size <= 3 ? sram_sizes[ufosd_header->sram_size] : 128;
      printf ("[12]     SRAM size: %d kB => %s\n",
              y, matches_deviates (snes_sram_size == y * 1024));

      y = ufosd_header->map_control[2];
      if (y == 0 || y == 0x10 || y == 0x20 || y == 0x30)
        {
          if (y)
            printf ("[15]     A15=%s selects SRAM\n", y == 0x10 ? "x" : y == 0x20 ? "0" : "1");
          else
            puts ("[15]     A15 not used for SRAM control");
        }
      for (x = 0; x < 4; x++)
        {
          int shift = 6 - x * 2;

          y = (ufosd_header->map_control[3] & (3 << shift)) >> shift;
          if (y != 1)
            printf ("[16:%d-%d] A%d=%s selects SRAM\n",
                    shift + 1, shift, 23 - x, y == 0 ? "x" : y == 2 ? "0" : "1");
        }

      y = ufosd_header->banktype;
      printf ("[17]     DRAM mapping mode: %s => %s\n",
              y == 1 ? "LoROM" : y == 0 ? "HiROM" : "Unknown",
              matches_deviates ((snes_hirom ? 0 : 1) == y));

      y = ufosd_header->tvtype;
      printf ("[18]     Television standard: %s => %s\n",
              y == 0 ? "NTSC" : y == 2 ? "PAL" : "Unknown",
              matches_deviates (
                (snes_header.country == 0 || snes_header.country == 1 ? 0 : 2) == y));

      printf ("[20-3f]  Copy of last 32 bytes of internal header => %s\n",
              matches_deviates (
                memcmp (((char *) &snes_header) + sizeof snes_header - 32,
                        ufosd_header->internal_header_data, 32) == 0));
    }
  else if (copier_type == GD3)
    {
      unsigned int bank_offset = 0x8000;

      y = -1;
      if (header[0x10] == 0x81)
        y = 8 * 1024;
      else if (header[0x10] == 0x82)
        y = 2 * 1024;
      else if (header[0x10] == 0x80)
        y = 32 * 1024; // or 0
      if (y == 32 * 1024)
        printf ("[10]     SRAM size: 0 kB / 32 kB => %s\n",
                matches_deviates (snes_sram_size == 0 || snes_sram_size == 32 * 1024));
      else
        printf ("[10]     SRAM size: %d kB => %s\n",
                y / 1024, matches_deviates (snes_sram_size == y));

      y = 0;
      for (x = 0; x < 24; x++)
        {
          unsigned char mapping = header[0x11 + x];

          if (x == 16)
            {
              y = 0x40;
              bank_offset = 0;
            }
          else if (x == 20)
            y = 0xc0;

          printf ("[%x]     SNES 0x%02x-0x%02x:0x%04x-0x%x: ",
                  0x11 + x, y, y + 0x0f, bank_offset, bank_offset + 0x7fff);

          if (mapping == 0x60)
            puts ("pass-through to/from cartridge");
          else if (mapping == 0x40)
            // extended ROM check to match the (tested) header of Tales of Phantasia...
            printf ("SRAM (LoROM) => %s\n",
                    matches_deviates (snes_sram_size &&
                                      (!snes_hirom || snes_header_base == SNES_EROM)));
          else if (mapping < 0x40 && mapping >= 0x20)
            {
              unsigned int offset = (mapping - 0x20) * 4 * MBIT,
                           size = (unsigned int) ucon64.file_size - rominfo->backup_header_len;

              printf ("mapped from ROM data 0x%06x-0x%06x\n",
                      offset, offset + 4 * MBIT - 1);
              if (offset + 4 * MBIT > size)
                printf ("                                       "
                        "(beyond end of ROM data (0x%06x))\n", size - 1);
            }
          else if (mapping == 0x00)
            puts ("unmapped");
          else
            printf ("invalid mapping value: 0x%02x\n", mapping);

          y += 0x10;
        }

      for (y = 0; y < 2; y++)
        {
          char bank_str[8 * 5 + 1];
          int pos = 0;

          for (x = 0; x < 8; x++)
            if (header[0x29 + y] & (1 << x))
              pos += sprintf (bank_str + pos, "0x%xx,", y * 8 + x);
          if (pos)
            {
              bank_str[pos - 1] = '\0';
              printf ("[%x]     SRAM (HiROM) mapped to %s:0x6000-0x7fff => %s\n",
                      0x29 + y, bank_str,
                      matches_deviates (snes_sram_size && snes_hirom));
            }
          else
            // stating that the value matches with what snes_init() found
            //  actually applies to the combination of header[0x29] and header[0x2a]
            printf ("[%x]     no SRAM (HiROM) mapped to 0x%xx-0x%xx:0x6000-0x7fff => %s\n",
                    0x29 + y, y * 8, y * 8 + 7,
                    matches_deviates (!snes_sram_size || !snes_hirom ||
                                      (header[0x29] | header[0x2a])));
        }
    }

  copier_type = org_copier_type;

  return 0;
}


static unsigned short int
get_internal_sums (size_t backup_header_len)
/*
  Returns the sum of the internal checksum and the internal inverse checksum if
  the values for snes_hirom and backup_header_len are correct. If the values
  are correct the sum will be 0xffff. Note that the sum for bad ROM dumps can
  also be 0xffff, because this function adds the internal checksum bytes and
  doesn't do anything with the real, i.e. calculated, checksum.
*/
{
  size_t internal_header = SNES_HEADER_START + snes_header_base + snes_hirom +
                             backup_header_len;
  // don't use rominfo->header_start here!
  unsigned char buf[4];

  ucon64_fread (buf, internal_header + 44, 4, ucon64.fname);
  return buf[0] + (buf[1] << 8) + buf[2] + (buf[3] << 8);
}


static void
snes_handle_backup_header (st_ucon64_nfo_t *rominfo, st_unknown_backup_header_t *header)
/*
  Determine the size of a possible backup unit header. This function also tries
  to determine the bank type in the process. However, snes_set_hirom() has the
  final word about that.
*/
{
  unsigned short int x = 0;
  /*
    Check for "Extended" ROM dumps first, because at least one of them
    (Tales of Phantasia (J)) has two headers; an incorrect one at the normal
    location and a correct one at the Extended HiROM location.
  */
  if (ucon64.file_size >= SNES_HEADER_START + SNES_EROM + SNES_HEADER_LEN)
    {
      snes_header_base = SNES_EROM;
      snes_hirom = SNES_HIROM;
      rominfo->backup_header_len = 0;
      if ((x = get_internal_sums (rominfo->backup_header_len)) != 0xffff)
        {
          rominfo->backup_header_len = SWC_HEADER_LEN;
          if ((x = get_internal_sums (rominfo->backup_header_len)) != 0xffff)
            {
              snes_hirom = 0;
              if ((x = get_internal_sums (rominfo->backup_header_len)) != 0xffff)
                {
                  rominfo->backup_header_len = 0;
                  x = get_internal_sums (rominfo->backup_header_len);
                }
            }
        }
    }
  if (x != 0xffff)
    {
      snes_header_base = 0;
      snes_hirom = 0;
      rominfo->backup_header_len = 0;
      if ((x = get_internal_sums (rominfo->backup_header_len)) != 0xffff)
        {
          rominfo->backup_header_len = SWC_HEADER_LEN;
          if ((x = get_internal_sums (rominfo->backup_header_len)) != 0xffff)
            {
              snes_hirom = SNES_HIROM;
              if ((x = get_internal_sums (rominfo->backup_header_len)) != 0xffff)
                {
                  rominfo->backup_header_len = 0;
                  x = get_internal_sums (rominfo->backup_header_len);
                }
            }
        }
      }

  if (header->id1 == 0xaa && header->id2 == 0xbb && header->type == 4)
    copier_type = SWC;
  else if (!strncmp ((char *) header, "GAME DOCTOR SF 3", 16))
    copier_type = GD3;
  else if (!strncmp ((char *) header + 8, "SUPERUFO", 8))
    copier_type = UFO;
  else if (!strncmp ((char *) header + 8, "SFCUFOSD", 8))
    copier_type = UFOSD;
  else if ((header->hirom == 0x80 &&            // HiROM
             (((header->emulation1 == 0x77 || header->emulation1 == 0xf7) &&
                header->emulation2 == 0x83) ||
              (header->emulation1 == 0xdd &&
                (header->emulation2 == 0x82 || header->emulation2 == 0x02)) ||
              (header->emulation1 == 0xfd && header->emulation2 == 0x82)))
            ||
           (header->hirom == 0x00 &&            // LoROM
             (((header->emulation1 == 0x77 || header->emulation1 == 0x47) &&
                header->emulation2 == 0x83) ||
              // Testing for emulation1 & emulation2 being 0 makes NES FFE ROMs
              //  & Game Boy ROMs be detected as SNES ROMs, see
              //  src/console/nes.c & src/console/gb.c.
              ((header->emulation1 == 0x00 || header->emulation1 == 0x40) &&
                (header->emulation2 == 0x80 || header->emulation2 == 0x00)) ||
              (header->emulation1 == 0x11 && header->emulation2 == 0x02)))
          )
    copier_type = FIG;
  else if (rominfo->backup_header_len == 0 && x == 0xffff)
    copier_type = MGD_SNES;

  /*
    x can be better trusted than copier_type == FIG, but x being 0xffff is
    definitely not a guarantee that rominfo->backup_header_len already has the
    right value (e.g. Earthworm Jim (U), Alfred Chicken (U|E), Soldiers of
    Fortune (U)).
  */
#if 0
  if (copier_type != MGD_SNES) // don't do "&& copier_type != SMC" or we'll miss a lot of PD ROMs
#endif
    {
      unsigned int size = ((header->size_high << 8) + header->size_low) * 8 * 1024;
      size += SWC_HEADER_LEN;                   // if SWC-like header => hdr[1] high byte,
      if (size == ucon64.file_size)             //  hdr[0] low byte of # 8 kB blocks in ROM
        rominfo->backup_header_len = SWC_HEADER_LEN;
      else
        {
          unsigned int surplus = ucon64.file_size % 32768;
          if (surplus == 0)
            // most likely we guessed the copier type wrong
            {
              rominfo->backup_header_len = 0;
              copier_type = MGD_SNES;
            }
          /*
            Check for surplus being smaller than 31232 instead of MAXBUFSIZE
            (32768) to detect "Joystick Sampler with Still Picture (PD)" (64000
            bytes, including SWC header).
            "Super Wild Card V2.255 DOS ROM (BIOS)" is 16384 bytes (without
            header), so check for surplus being smaller than 16384.
            Shadow, The (Beta) [b3] has a surplus of 7680 bytes (15 * 512). So,
            accept a surplus of up to 7680 bytes as a header...
          */
          else if (surplus % SWC_HEADER_LEN == 0 &&
                   surplus < 15 * SWC_HEADER_LEN &&
                   ucon64.file_size > surplus)
            rominfo->backup_header_len = surplus;
          // Special case for Infinity Demo (PD)... (has odd size, but SWC
          //  header). Don't add "|| copier_type == FIG" as it is too unreliable.
          else if (copier_type == SWC || copier_type == GD3 ||
                   copier_type == UFO || copier_type == UFOSD)
            rominfo->backup_header_len = SWC_HEADER_LEN;
        }
    }
  if (UCON64_ISSET2 (ucon64.backup_header_len, unsigned int)) // -hd, -nhd or -hdn switch was specified
    rominfo->backup_header_len = ucon64.backup_header_len;
  if (copier_type == MGD_SNES && rominfo->backup_header_len)
    copier_type = SMC;

  if (rominfo->backup_header_len && !memcmp ((unsigned char *) header + 0x1e8, "NSRT", 4))
    nsrt_header = 1;
  else
    nsrt_header = 0;
}


static int
snes_set_hirom (unsigned char *rom_buffer, unsigned int size)
/*
  This function tries to determine if the ROM dump is LoROM or HiROM. It returns
  the highest value that check_banktype() returns. A higher value means a higher
  chance the bank type is correct.
*/
{
  int x, score_hi = 0, score_lo = 0;

  if (size >= 8 * MBIT + SNES_HEADER_START + SNES_HIROM + SNES_HEADER_LEN &&
      !strncmp ((char *) rom_buffer + SNES_HEADER_START + 16, "ADD-ON BASE CASSETE", 19))
    { // A Sufami Turbo dump contains 4 copies of the ST BIOS, which is 2 Mbit.
      //  After the BIOS comes the game data.
      st_dump = 1;
      snes_header_base = 8 * MBIT;
      x = 8 * MBIT + SNES_HIROM;
    }
  else if (snes_header_base == SNES_EROM)
    x = SNES_EROM + SNES_HIROM;
  else
    {
      snes_header_base = 0;
      x = SNES_HIROM;
    }

  if (size > SNES_HEADER_START + SNES_HIROM + 0x4d)
    {
      score_hi = check_banktype (rom_buffer, x);
      score_lo = check_banktype (rom_buffer, snes_header_base);
    }
  if (score_hi > score_lo)                      // yes, a preference for LoROM
    {                                           //  (">" vs. ">=")
      snes_hirom = SNES_HIROM;
      x = score_hi;
    }
  else
    {
      snes_hirom = 0;
      x = score_lo;
    }
  /*
    It would be nice if snes_header.map_type & 1 could be used to verify that
    snes_hirom has the correct value, but it doesn't help much. For games like
    Batman Revenge of the Joker (U) it matches what check_banktype() finds.
    snes_hirom must be 0x8000 for that game in order to display correct
    information. However it should be 0 when writing a copier header.
    So, snes_header.map_type can't be used to recognize such cases.
  */

  // step 3.
  if (UCON64_ISSET (ucon64.snes_hirom))         // -hi or -nhi switch was specified
    {
      snes_hirom = ucon64.snes_hirom;
      // keep snes_deinterleave() from changing snes_hirom
      snes_hirom_ok = 1;
      if (size < SNES_HEADER_START + SNES_HIROM + SNES_HEADER_LEN)
        snes_hirom = 0;
    }

  if (UCON64_ISSET (ucon64.snes_header_base))   // -erom switch was specified
    {
      snes_header_base = ucon64.snes_header_base;
      if (snes_header_base &&
          size < snes_header_base + SNES_HEADER_START + snes_hirom + SNES_HEADER_LEN)
        snes_header_base = 0;                   // don't let -erom crash on a too small ROM
    }

  return x;
}


static void
snes_set_bs_dump (st_ucon64_nfo_t *rominfo, unsigned char *rom_buffer,
                  unsigned int size)
{
  bs_dump = snes_check_bs ();
  /*
    Do the following check before checking for ucon64.bs_dump. Then it's
    possible to specify both -erom and -bs with effect, for what it's worth ;-)
    The main reason to test this case is to display correct info for "SD Gundam
    G-NEXT + Rom Pack Collection (J) [!]". Note that testing for SNES_EROM
    causes the code to be skipped for Sufami Turbo dumps.
  */
  if (bs_dump &&
      snes_header_base == SNES_EROM && !UCON64_ISSET (ucon64.snes_header_base))
    {
      bs_dump = 0;
      snes_header_base = 0;
      snes_set_hirom (rom_buffer, size);
      rominfo->header_start = snes_header_base + SNES_HEADER_START + snes_hirom;
      memcpy (&snes_header, rom_buffer + rominfo->header_start, rominfo->header_len);
    }
  if (UCON64_ISSET (ucon64.bs_dump))            // -bs or -nbs switch was specified
    {
      bs_dump = ucon64.bs_dump;
      if (bs_dump && snes_header_base == SNES_EROM)
        bs_dump = 2;                            // Extended ROM => must be add-on cart
    }
}


static int
check_ufosd_sram (int *sram_size)
{
  int result = 0;
  FILE *file;
  char buffer[32 * 1024], pattern[128];

  if (ucon64.file_size != MBIT)
    return result;

  if ((file = fopen (ucon64.fname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.fname);
      return -1;
    }

  *sram_size = 0;
  set_ufosd_sram_pattern (pattern, sizeof pattern);

  fseek (file, (long) ucon64.file_size - 64 * 1024, SEEK_SET);
  fread (buffer, 1, sizeof pattern, file);
  if (memcmp (buffer, pattern, sizeof pattern) == 0) // pattern at 64 kB?
    {
      unsigned int next_step = 0;

      do
        {
          fseek (file, *sram_size, SEEK_SET);
          fread (buffer, 1, sizeof pattern, file);
          // check offsets 0, 2, 4, 8, 16, 32 and 64 kB
          if (memcmp (buffer, pattern, sizeof pattern) != 0)
            *sram_size = *sram_size ? *sram_size * 2 : 2 * 1024;
          else
            next_step = 1;
        }
      while (!next_step);

      if (*sram_size > 0)
        {
          unsigned int n;

          while ((n = fread (buffer, 1, sizeof buffer, file)) != 0)
            {
              unsigned int m = 0, block_size = sizeof pattern <= n ? sizeof pattern : n;
              while (memcmp (buffer + m, pattern, block_size) == 0)
                {
                  m += block_size;
                  if (m == n)
                    break;
                  if (m + block_size > n)
                    block_size = n - m;
                }
              if (m < n)
                {
                  next_step = 0;
                  break;
                }
            }

          if (next_step)
            result = UFOSD;                     // Super UFO Pro 8 SD SRAM file
        }
    }
  fclose (file);

  return result;
}


static int
check_smini_save (unsigned int *sram_size, st_ucon64_nfo_t *rominfo)
{
  st_ustar_header_t header;
  unsigned char *buffer, sram_checksum[20] = { 0 };
  int result, sram_data_size = 0;

  if (ucon64_fread (&header, 0, sizeof header, ucon64.fname) <
        (int) sizeof header)
    {
      *sram_size = 0;
      return 0; // not necessarily an error, just not a .clvs file
    }
  header.name[sizeof header.name - 1] = '\0';

  if ((buffer = (unsigned char *) malloc (128 * 1024 + 20)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], 128 * 1024 + 20);
      exit (1);
    }

  sram_data_size = get_smini_sram_data (ucon64.fname, buffer, 128 * 1024 + 20,
                                        sram_checksum);
  if (sram_data_size >= 20) // some .clvs files contain a 20 byte cartridge.sram
    {
      s_sha1_ctx_t sha1_ctx;
      unsigned char calculated_hash[20];
      char calculated_hash_str[41], internal_hash_str[41],
           internal_hash2_str[41], *p;
      unsigned int i = 0;

      // get the game ID and put it in rominfo->name
      snprintf (rominfo->name, sizeof header.name + 9, "Game ID: %.99s",
                header.name);
      rominfo->name[sizeof header.name + 9 - 1] = '\0';
      p = strchr (rominfo->name, '/');
      if (p)
        *p = '\0';

      *sram_size = sram_data_size - 20;

      {
        unsigned int base, size;

        if (set_smini_chksum_range (&base, &size, *sram_size, 0) == 0)
          {
            sha1_begin (&sha1_ctx);
            sha1 (&sha1_ctx, buffer + base, size);
            sha1_end (calculated_hash, &sha1_ctx);
          }
        else
          memset (calculated_hash, 0, sizeof calculated_hash);
      }
      if (!(UCON64_ISSET (ucon64.range_start) &&
            UCON64_ISSET2 (ucon64.range_length, size_t)) &&
          *sram_size > 0 && memcmp (calculated_hash, buffer + *sram_size, 20))
        {
          unsigned int max_block_size = INITIAL_MAX_BLOCK_SIZE; // Super Mario RPG is an exception

          while (i < *sram_size)
            {
              unsigned int j, j_max = i + max_block_size < *sram_size ?
                             i + max_block_size : *sram_size, match_found = 0;

              printf ("\rScanning SNES Classic Mini SRAM data, base 0x%04x", i);
              fflush (stdout);
              // smallest block size is 0xff bytes for Hoshi no Kirby Super Deluxe
              for (j = i + MIN_BLOCK_SIZE; j <= j_max; j++)
                {
                  unsigned char calculated_hash2[20];

                  sha1_begin (&sha1_ctx);
                  sha1 (&sha1_ctx, buffer + i, j - i);
                  sha1_end (calculated_hash2, &sha1_ctx);
                  if (!memcmp (calculated_hash2, buffer + *sram_size, 20))
                    {
                      match_found = 1;
                      memcpy (calculated_hash, calculated_hash2, 20);
                      // Super Mario RPG: --range=0000:1ffc
                      // Super Mario World 2 - Yoshi's Island: --range=7c00:7e7b
                      // Hoshi no Kirby Super Deluxe (J)(/Kirby Super Star (U)/Kirby's Fun Pak (E)): --range=1f00:1ffe
                      // Star Fox 2: --range=3912:3a45
                      printf (", block size 0x%04x bytes\n"
                              "Found block with matching checksum. Use "
                              OPTION_LONG_S "range" OPTARG_S "%04x:%04x with "
                              OPTION_LONG_S "sminis", j - i, i, j - 1);
                      break;
                    }
                }
              if (match_found)
                break;
              // All the cases with base > 0 use a very small range for the checksum (< 0x280).
              max_block_size = DEFAULT_MAX_BLOCK_SIZE;
#if 1
              if (i % 0x100 == 0) // Star Fox 2 uses an odd base
                i += 0x12;
#else
              // a bit less specific, but significantly slower
              if (i % 0x100 < 0x12) // Star Fox 2 uses an odd base
                i++;
#endif
              else
                i += 0x100 - i % 0x100;
            }
          puts ("\n");
        }

      p = calculated_hash_str;
      for (i = 0; i < 20; i++, p = strchr (p, 0))
        sprintf (p, "%02x", calculated_hash[i]);

      p = internal_hash_str;
      for (i = 0; i < 20; i++, p = strchr (p, 0))
        sprintf (p, "%02x", (buffer + *sram_size)[i]);

      p = internal_hash2_str;
      for (i = 0; i < 20; i++, p = strchr (p, 0))
        sprintf (p, "%02x", sram_checksum[i]);

      i = !memcmp (calculated_hash, buffer + *sram_size, 20);
      sprintf (rominfo->internal_crc2,
               "SRAM checksum: %s, 0x%s (calculated)\n"
               "               %s  0x%s (internal)\n"
               "Checksum file:     %s0x%s",
               i ?
#ifdef  USE_ANSI_COLOR
                 ucon64.ansi_color ? "\x1b[01;32mOK\x1b[0m" : "OK" :
                 ucon64.ansi_color ? "\x1b[01;31mBad\x1b[0m" : "Bad",
#else
                 "OK" : "Bad",
#endif
               calculated_hash_str, i ? "==" : "!= ", internal_hash_str,
               i ? "" : " ", internal_hash2_str);

      result = SMINI;
    }
  else
    {
      // some valid .clvs files do not contain cartridge.sram
      *sram_size = 0;
      if (strncmp (header.name, "CLV-", 4) == 0 &&
          strlen (header.name) == 12 && // include the trailing slash
          (unsigned int) strtol (header.header_checksum, NULL, 8) ==
            ustar_chksum (&header))
        {
          char *p;

          // get the game ID and put it in rominfo->name
          snprintf (rominfo->name, sizeof header.name + 9, "Game ID: %.99s",
                    header.name);
          rominfo->name[sizeof header.name + 9 - 1] = '\0';
          p = strchr (rominfo->name, '/');
          if (p)
            *p = '\0';

          result = SMINI;
        }
      else
        result = 0;
    }

  free (buffer);
  return result;
}


static void
check_split (const char *filename, void *cb_data)
{
  st_split_info_t *info = (st_split_info_t *) cb_data;
  unsigned char multi_magic[9] = { '\0' };

  if (ucon64_fread (&multi_magic, 2, 9, filename) == 9)
    {
      if (info->nparts < 2 &&
          multi_magic[6] == 0xaa && multi_magic[7] == 0xbb && multi_magic[8] == 4)
        {
          char **fname = &info->parts[info->nparts].fname;
          if (*fname == NULL &&
              (*fname = (char *) malloc (FILENAME_MAX)) == NULL)
            {
              fprintf (stderr, ucon64_msg[BUFFER_ERROR], FILENAME_MAX);
              return;
            }
          snprintf (*fname, FILENAME_MAX, "%s", filename);
          (*fname)[FILENAME_MAX - 1] = '\0';
        }
      // byte at offset 2 is for SWC header "emulation", for FIG/UFO header "multi"
      if (multi_magic[0] & 0x40 || (copier_type == UFO && multi_magic[0] & 0x10))
        info->nparts++;
    }
}


static int
check_smc_ic2_rom (int nparts, st_split_info_t *info, size_t backup_header_len)
{
  if (nparts == 2 && info->parts[0].fname && info->parts[1].fname)
    {
      info->parts[0].size = fsizeof (info->parts[0].fname);
      info->parts[1].size = fsizeof (info->parts[1].fname);
      // assume the two parts have an identical backup unit header size
      if (info->parts[0].size == 8 * MBIT + backup_header_len &&
          info->parts[1].size < 8 * MBIT + backup_header_len)
        {
          /*
            Test whether the first part is padded with 8 Mbit minus the size of
            the second part. So, if the second part is 8 Mbit, the first part
            is not padded and we have to rely on snes_check_interleaved()
            alone. Thus we cannot detect whether a 16 Mbit ROM is in SMC IC2
            format in this function.
          */
          unsigned char buffer[MAXBUFSIZE];
          int value = -1;
          size_t pos = info->parts[1].size, n;
          while ((n = ucon64_fread (buffer, pos, sizeof buffer,
                                    info->parts[0].fname)) > 0)
            {
              size_t m;
              // uCON pads with 0xff, but uCON64 pads with 0
              if (value == -1)
                {
                  value = buffer[0];
                  m = 1;
                }
              else
                m = 0;
              for (; m < n; m++)
                if (buffer[m] != value)
                  break;
              if (m != n)
                break;
              pos += n;
            }
          return pos == info->parts[0].size;
        }
    }
  return 0;
}


int
snes_init (st_ucon64_nfo_t *rominfo)
{
  int x = 0, y = 0, result = -1;                // it's no SNES ROM dump until detected otherwise
  size_t size;
  unsigned int calc_checksums, pos = strlen (rominfo->misc);
  unsigned char *rom_buffer;
  st_unknown_backup_header_t header = { 0, 0, 0, 0, 0, 0, { 0 }, 0, 0, 0, { 0 } };
#define SNES_COUNTRY_MAX 0xe
  static const char *snes_country[SNES_COUNTRY_MAX] =
    {
      "Japan",
      "U.S.A.",
      "Europe, Oceania and Asia",               // Australia is part of Oceania
      "Sweden",
      "Finland",
      "Denmark",
      "France",
      "The Netherlands",                        // Holland is an incorrect name for The Netherlands
      "Spain",
      "Germany, Austria and Switzerland",
      "Italy",
      "Hong Kong and China",
      "Indonesia",
      "South Korea"
    };

  snes_hirom_ok = 0;                            // init these vars here, for -lsv
  snes_sram_size = 0;                           // idem
  copier_type = SMC;                            // idem, SMC indicates unknown copier type
  bs_dump = 0;                                  // for -lsv, but also just to init it
  st_dump = 0;                                  // idem

  split_info.nparts = 0;
  for (x = 0; x < 2; x++)
    {
      if (split_info.parts[x].fname)
        split_info.parts[x].fname[0] = '\0';
      split_info.parts[x].size = 0;
    }

  ucon64_fread (&header, 0, UNKNOWN_BACKUP_HEADER_LEN, ucon64.fname);
  if (header.id1 == 0xaa && header.id2 == 0xbb)
    x = SWC;
  else if (!strncmp ((char *) &header + 8, "SUPERUFO", 8))
    x = UFO;
  else if ((x = check_ufosd_sram (&y)) != 0)
    {
      if (x < 0)
        return -1;
    }
  else
    x = check_smini_save ((unsigned int *) &y, rominfo);

  if ((x == SWC && (header.type == 5 || header.type == 8)) ||
      (x == UFO && OFFSET (header, 0x11) == 0) ||
      (x == UFOSD && y > 0) ||
      (x == SMINI))
    {
      if (!*rominfo->name)
        strcpy (rominfo->name, "Name: N/A");
      rominfo->console_usage = NULL;
      rominfo->maker = "Publisher: You?";
      rominfo->country = "Country: Your country?";
      rominfo->has_internal_crc = 0;
      ucon64.split = 0;                         // SRAM & RTS files are never split
      switch (x)
        {
        case SWC:
          rominfo->backup_header_len = SWC_HEADER_LEN;
          rominfo->backup_usage = swc_usage[0].help;
          copier_type = SWC;
          if (header.type == 5)
            pos += sprintf (rominfo->misc + pos, "Type: Super Wild Card SRAM file");
          else if (header.type == 8)
            pos += sprintf (rominfo->misc + pos, "Type: Super Wild Card RTS file");
          break;
        case UFO:
          rominfo->backup_header_len = UFO_HEADER_LEN;
          rominfo->backup_usage = ufo_usage[0].help;
          copier_type = UFO;
          pos += sprintf (rominfo->misc + pos, "Type: Super UFO SRAM file");
          break;
        case UFOSD:
          rominfo->backup_header_len = 0;
          rominfo->backup_usage = ufosd_usage[0].help;
          copier_type = UFOSD;
          pos += sprintf (rominfo->misc + pos, "Type: Super UFO Pro 8 SD SRAM file\n"
                                               "SRAM size: %d kBytes", y / 1024);
          break;
        case SMINI:
          rominfo->backup_header_len = 0;
          rominfo->backup_usage = "SNES/Super Famicom Classic Mini";
          copier_type = SMINI;
          pos += sprintf (rominfo->misc + pos, "Type: SNES/Super Famicom Classic Mini SRAM/save state file\n"
                                               "SRAM size: %d kBytes", y / 1024);
          break;
        }
      return 0;                                 // rest is nonsense for SRAM/RTS file
    }

  /*
    snes_check_interleaved() needs the correct value for snes_hirom and
    rominfo->header_start. snes_hirom may be used only after the check for
    -hi/-nhi has been done. However, rominfo->backup_header_len must have the
    correct value in order to determine the value for snes_hirom. This can only
    be known after the backup unit header length detection (including the check
    for -hd/-nhd/-hdn). So, the order must be
    1. - rominfo->backup_header_len
    2. - snes_hirom
    3. - check for -hi/-nhi
    4. - snes_check_interleaved()
  */

  snes_handle_backup_header (rominfo, &header); // step 1. & first part of step 2.

  if (!UCON64_ISSET (ucon64.split))
    {
      if (copier_type == UFOSD)
        ucon64.split = 0;                       // UFO SD ROMs cannot be split
      else
        {
          if (copier_type == SWC || copier_type == FIG || copier_type == UFO)
            {
              int nparts = ucon64_testsplit (ucon64.fname, check_split, &split_info);
              // force displayed info to be correct if not split (see ucon64.c)
              ucon64.split = nparts == split_info.nparts + 1 ? nparts : 0;
              if (check_smc_ic2_rom (ucon64.split, &split_info,
                                     rominfo->backup_header_len))
                copier_type = IC2;
            }
          else
            ucon64.split = ucon64_testsplit (ucon64.fname, NULL, NULL);
        }
    }

  size = get_rom_data_size (rominfo->backup_header_len);
  if (size < SNES_HEADER_START + SNES_HEADER_LEN)
    {
      snes_hirom = 0;
      if (UCON64_ISSET (ucon64.snes_hirom))     // see snes_set_hirom()
        snes_hirom = ucon64.snes_hirom;
      snes_hirom_ok = 1;

      rominfo->interleaved = 0;
      if (UCON64_ISSET (ucon64.interleaved))
        rominfo->interleaved = ucon64.interleaved;
      return -1;                                // don't continue (seg faults!)
    }
  if (ucon64.console == UCON64_SNES ||
      (copier_type != SMC && size <= 16 * 1024 * 1024))
    result = 0;                                 // it seems to be a SNES ROM dump

  if ((rom_buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      return -1;                                // don't exit(), we might've been
    }                                           //  called with -lsv
  read_rom_data (rom_buffer, rominfo->backup_header_len, size, ucon64.fname);

  x = snes_set_hirom (rom_buffer, size);        // second part of step 2. & step 3.

  rominfo->header_start = snes_header_base + SNES_HEADER_START + snes_hirom;
  rominfo->header_len = SNES_HEADER_LEN;
  // set snes_header before calling snes_check_interleaved()
  memcpy (&snes_header, rom_buffer + rominfo->header_start, rominfo->header_len);
  rominfo->header = &snes_header;

  // step 4.
  rominfo->interleaved = UCON64_ISSET (ucon64.interleaved) ?
    ucon64.interleaved : snes_check_interleaved (rom_buffer, size, x);

  // check_smc_ic2_rom() sets split_info.parts[n].size only if ucon64.split is 2
  if (rominfo->interleaved &&
      split_info.parts[0].size == 8 * MBIT + rominfo->backup_header_len &&
      split_info.parts[1].size == 8 * MBIT + rominfo->backup_header_len)
    {
      // first part turns out to be interleaved => 16 Mbit ROM in SMC IC2 format
      size_t size2 = split_info.parts[1].size - rominfo->backup_header_len;
      unsigned char *rom_buffer0 = rom_buffer;
      copier_type = IC2;
      size = 2 * size2;
      if ((rom_buffer = (unsigned char *) realloc (rom_buffer0, size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
          free (rom_buffer0);
          return -1;
        }
      ucon64_fread (rom_buffer + size2, rominfo->backup_header_len, size2,
                    split_info.parts[1].fname);
    }

  calc_checksums = !UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0;
  // we want the CRC32 of the "raw" data (too)
  if (calc_checksums)
    ucon64.fcrc32 = crc32 (0, rom_buffer, size);
  if (copier_type == IC2)
    rominfo->data_size = size;

  // bs_dump has to be set before calling snes_chksum(), but snes_check_bs()
  //  needs snes_header to be filled with the correct data
  if (rominfo->interleaved)
    {
      snes_deinterleave (rominfo, &rom_buffer, size);
      snes_set_hirom (rom_buffer, size);
      rominfo->header_start = snes_header_base + SNES_HEADER_START + snes_hirom;
      memcpy (&snes_header, rom_buffer + rominfo->header_start, rominfo->header_len);
    }

  snes_set_bs_dump (rominfo, rom_buffer, size);

  // internal ROM name
  if (!bs_dump && st_dump)
    memcpy (rominfo->name, rom_buffer + 8 * MBIT + 16, SNES_NAME_LEN);
  else
    memcpy (rominfo->name, snes_header.name, SNES_NAME_LEN);
  rominfo->name[(bs_dump || st_dump) ? 16 : SNES_NAME_LEN] = '\0';
  // terminate string (at 1st byte _after_ string)

  if (calc_checksums)
    {
      // internal ROM crc
      rominfo->has_internal_crc = 1;
      rominfo->internal_crc_len = 2;
      rominfo->current_internal_crc = snes_chksum (rominfo, rom_buffer, size);
      rominfo->internal_crc = snes_header.checksum_low;
      rominfo->internal_crc += snes_header.checksum_high << 8;
      x = snes_header.inverse_checksum_low;
      x += snes_header.inverse_checksum_high << 8;
      y = ~rominfo->current_internal_crc & 0xffff;
      sprintf (rominfo->internal_crc2,
               "Inverse checksum: %s, 0x%04x (calculated) %c= 0x%04x (internal)",
               y == x ?
#ifdef  USE_ANSI_COLOR
                 ucon64.ansi_color ? "\x1b[01;32mOK\x1b[0m" : "OK" :
                 ucon64.ansi_color ? "\x1b[01;31mBad\x1b[0m" : "Bad",
#else
                 "OK" : "Bad",
#endif
               y, y == x ? '=' : '!', x);
      if (bs_dump == 1)                         // bs_dump == 2 for BS add-on dumps
        {
          unsigned short int *bs_date_ptr = (unsigned short int *)
            (rom_buffer + snes_header_base + SNES_HEADER_START + snes_hirom + 38);
          /*
            We follow the "uCONSRT standard" for calculating the CRC32 of BS
            dumps. At the time of this writing (20 June 2003) the uCONSRT
            standard defines that the date of BS dumps has to be "skipped"
            (overwritten with a constant number), because the date is variable.
            When a BS dump is made the BSX fills in the date. Otherwise two
            dumps of the same memory card would have a different CRC32.
            For BS add-on cartridge dumps we don't do anything special as they
            come from cartridges (with a constant date).
            Why 42? It's the answer to life, the universe and everything :-)
          */
          *bs_date_ptr = me2le_16 (0x0042);
          get_nsrt_info (rom_buffer, rominfo->header_start, (unsigned char *) &header);
          ucon64.crc32 = crc32 (0, rom_buffer, size);
        }
      else if (rominfo->interleaved || nsrt_header)
        {
          get_nsrt_info (rom_buffer, rominfo->header_start, (unsigned char *) &header);
          ucon64.crc32 = crc32 (0, rom_buffer, size);
        }
      else
        {
          ucon64.crc32 = ucon64.fcrc32;
          ucon64.fcrc32 = 0;
        }
    }

  rominfo->console_usage = snes_usage[0].help;
  if (!rominfo->backup_header_len)
    rominfo->backup_usage = mgd_usage[0].help;
  else
    {
      switch (copier_type)
        {
        case GD3:
          rominfo->backup_usage = gd_usage[0].help;
          break;
        case UFO:
          rominfo->backup_usage = ufo_usage[0].help;
          break;
        case UFOSD:
          rominfo->backup_usage = ufosd_usage[0].help;
          break;
        case FIG:
          rominfo->backup_usage = fig_usage[0].help;
          break;
        case IC2:
          rominfo->backup_usage = smcic2_usage[0].help;
          break;
        // just assume it's in SWC format... (there are _many_ ROMs on the
        //  internet with incorrect headers)
        default:
          rominfo->backup_usage = swc_usage[0].help;
        }
    }

  // ROM maker
  if (snes_header.maker == 0x33 || bs_dump)
    {
      int ih = snes_header.maker_high <= '9' ?
                 snes_header.maker_high - '0' : snes_header.maker_high - 'A' + 10,
          il = snes_header.maker_low <= '9' ?
                 snes_header.maker_low - '0' : snes_header.maker_low - 'A' + 10;
      x = ih * 36 + il;
    }
  else if (snes_header.maker != 0)
    x = (snes_header.maker >> 4) * 36 + (snes_header.maker & 0x0f);
  else
    x = 0;                                      // warning remover

  if (x < 0 || x >= NINTENDO_MAKER_LEN)
    x = 0;
  rominfo->maker = snes_header.maker == 0 ? "Demo or Beta ROM?" :
    NULL_TO_UNKNOWN_S (nintendo_maker[x]);

  if (!bs_dump)
    {
      const char *snes_rom_type[3] =
        {
          "ROM",                                // NOT ROM only, ROM + other chip is possible
          "ROM + SRAM",
          "ROM + SRAM + Battery"
        };

      // ROM country
      rominfo->country = NULL_TO_UNKNOWN_S (snes_country[MIN (snes_header.country, SNES_COUNTRY_MAX - 1)]);

      // misc stuff
      pos += sprintf (rominfo->misc + pos, "HiROM: %s\n", snes_hirom ? "Yes" : "No");
      pos += sprintf (rominfo->misc + pos, "Internal size: %u Mb\n", get_internal_size ());
//      pos += sprintf (rominfo->misc + pos, "Map type: %x\n", snes_header.map_type);
      pos += sprintf (rominfo->misc + pos, "ROM type: (%x) %s", snes_header.rom_type,
                      snes_rom_type[(snes_header.rom_type & 7) % 3]);
      if ((snes_header.rom_type & 0xf) >= 3)
        {
          char *str;

          if (snes_header.rom_type == 3 || snes_header.rom_type == 5)
            str = "DSP";
          else if (snes_header.rom_type == 0x13)
            str = "SRAM + Super FX (Mario Chip 1)";
          else if (snes_header.rom_type == 0x1a)
            str = "Super FX";
          else if (snes_header.rom_type == 0x14 || snes_header.rom_type == 0x15)
            {
              if (snes_header.rom_size > 10)
                str = "Super FX 2";             // larger than 8 Mbit
              else
                str = "Super FX";
            }
          else if (snes_header.rom_type == 0x25)
            str = "OBC1";
          else if (snes_header.rom_type == 0x34 || snes_header.rom_type == 0x35)
            str = "SA-1";
          else if (snes_header.rom_type == 0x43 || snes_header.rom_type == 0x45)
            str = "S-DD1";
          else if (snes_header.rom_type == 0x55)
            str = "S-RTC";
          else if (snes_header.rom_type == 0xe3)
            str = "Game Boy data";
          else if (snes_header.rom_type == 0xf3)
            str = "C4";
          else if (snes_header.rom_type == 0xf5)
            {
              if (snes_header.map_type == 0x30)
                str = "Seta RISC";
              else
                str = "SPC7110";
            }
          else if (snes_header.rom_type == 0xf6)
            str = "Seta DSP";
          else if (snes_header.rom_type == 0xf9)
            str = "SPC7110 + RTC";
          else
            str = "Unknown";

          pos += sprintf (rominfo->misc + pos, " + %s", str);
        }
      pos += sprintf (rominfo->misc + pos, "\n");

      pos += sprintf (rominfo->misc + pos, "ROM speed: %s\n",
                      snes_header.map_type & 0x10 ? "120 ns (FastROM)" : "200 ns (SlowROM)");

      if (snes_header.rom_type == 0x13 || snes_header.rom_type == 0x1a ||
          snes_header.rom_type == 0x14 || snes_header.rom_type == 0x15)
        {
          snes_sram_size = 32 * 1024;
          if (snes_header.maker == 0x33)
            snes_sfx_sram_size = snes_header.sfx_sram_size ? 1 << (snes_header.sfx_sram_size + 10) : 0;
          else
            snes_sfx_sram_size = 32 * 1024;
        }
      else
        {
          snes_sram_size = snes_header.sram_size ? 1 << (snes_header.sram_size + 10) : 0;
          snes_sfx_sram_size = 0;
        }

      if (!snes_sram_size && !snes_sfx_sram_size)
        pos += sprintf (rominfo->misc + pos, "SRAM: No\n");
      else
        pos += sprintf (rominfo->misc + pos, "SRAM: Yes, %d kBytes\n",
                        (snes_sfx_sram_size ? snes_sfx_sram_size : snes_sram_size) / 1024);
    }
  else                                          // BS info
    {
      const char *snes_bs_type[4] =
        {
          "Full size + Sound link",
          "Full size",
          "Part size + Sound link",
          "Part size"
        };

      // ROM country
      rominfo->country = "Japan";
      // misc stuff
      if (bs_dump == 2)
        pos += sprintf (rominfo->misc + pos, "\nBroadcast Satellaview add-on cartridge dump\n");
      else
        pos += sprintf (rominfo->misc + pos, "\nBroadcast Satellaview dump\n"); // new line is intentional

      x = snes_header.bs_day & 0x0f;
      if (x <= 3)
        y = (snes_header.bs_day >> 4) * 2;
      else if (x >= 8 && x <= 0xb)
        y = (snes_header.bs_day >> 4) * 2 + 1;
      else // incorrect data
        y = 0;
      pos += sprintf (rominfo->misc + pos, "Dumping date: %d/%d\n", y, snes_header.bs_month >> 4);
      pos += sprintf (rominfo->misc + pos, "HiROM: %s\n", snes_hirom ? "Yes" : "No");

      // misc stuff
      pos += sprintf (rominfo->misc + pos, "Internal size: %u Mb\n", get_internal_size ());
//      pos += sprintf (rominfo->misc + pos, "Map type: %x\n", snes_header.bs_map_type);
      x = snes_header.bs_type >> 4;
      pos += sprintf (rominfo->misc + pos, "ROM type: (%x) %s\n", snes_header.bs_type,
                      x > 3 ? "Unknown" : snes_bs_type[x]);

      /*
        It seems logical that the same condition as for regular cartridge dumps
        tells whether it's a FastROM or a SlowROM. The original condition was
        "(snes_header.bs_map_type >> 4) > 2".
      */
      pos += sprintf (rominfo->misc + pos, "ROM speed: %s\n",
                      snes_header.bs_map_type & 0x10 ? "120 ns (FastROM)" : "200 ns (SlowROM)");
    }

  pos += sprintf (rominfo->misc + pos, "Version: 1.%u", snes_header.version);

  if (nsrt_header)
    handle_nsrt_header (rominfo, (unsigned char *) &header, snes_country);

  free (rom_buffer);
  return result;
}


#if 1
static int
snes_check_bs (void)
{
  if ((snes_header.maker == 0x33 || snes_header.maker == 0xff) &&
      (snes_header.map_type == 0 || (snes_header.map_type & 0x83) == 0x80))
    {
      int date = (snes_header.bs_day << 8) | snes_header.bs_month;

      if (date == 0)
        return 2;                               // BS add-on cartridge dump
      else if (date == 0xffff ||
               ((snes_header.bs_month & 0xf) == 0 &&
                ((unsigned int) ((snes_header.bs_month >> 4) - 1)) < 12))
        return 1;                               // BS dump (via BSX)
    }
  return 0;
}
#else
static int
check_char (unsigned char c)
{
  if ((c & 0x80) == 0)
    return 0;

  if ((c - 0x20) & 0x40)
    return 1;
  else
    return 0;
}


static int
snes_bs_name (void)
{
  unsigned char value;
  int n, n_valid = 0;

  for (n = 0; n < 16; n++)
    {
      value = snes_header.name[n];
      if (check_char (value) != 0)
        {
          value = snes_header.name[n + 1];
          if (value < 0x20 && (n_valid != 11 || value != 0)) // Dr. Mario Hack
            break;
          n_valid++;
          n++;
        }
      else
        {
          if (value == 0)
            {
              if (n_valid == 0)
                break;
              continue;
            }
          if (value < 0x20 || (value >= 0x80 && value < 0xa0) || value >= 0xf0)
            break;
          n_valid++;
        }
    }

  return n == 16 && n_valid > 0 ? 1 : 0;
}


static int
snes_check_bs (void)
{
  unsigned int value;

  if (snes_header.bs_type & 0x4f)
    return 0;

  if (snes_header.maker != 0x33 && snes_header.maker != 0xff)
    return 0;

  value = (snes_header.bs_day << 8) | snes_header.bs_month;
  if (value != 0x0000 && value != 0xffff)
    {
      if ((value & 0x040f) != 0)
        return 0;
      if ((value & 0xff) > 0xc0)
        return 0;
    }

  if (snes_header.bs_map_type & 0xce || ((snes_header.bs_map_type & 0x30) == 0))
    return 0;

  if ((snes_header.map_type & 0x03) != 0)
    return 0;

  value = ((unsigned char *) &snes_header)[35];
  if (value != 0x00 && value != 0xff)
    return 0;

  if (((unsigned char *) &snes_header)[36] != 0x00)
    return 0;

  return snes_bs_name ();
}
#endif


static int
snes_chksum (st_ucon64_nfo_t *rominfo, unsigned char *rom_buffer, size_t rom_size)
#if 0
/*
  Calculate the checksum of a SNES ROM. A big difference between this version of
  snes_chksum() and the one below is that it calculates a checksum that is
  unlikely to match the internal checksum in case the ROM is an overdump or
  enlarged (see snes_ufosd()). It could be argued however, that the point is
  calculating a sum regardless whether it matches the internal checksum.
  This became an issue for snes_ufosd() where the output needs to have an
  internal checksum that matches with the result of the algorithm as implemented
  by the Super UFO Pro 8 SD. That implementation appears to be more like the
  other version of snes_chksum().
  By "chance" (what the data happens to be) for the following GoodSNES 2.04
  dumps this version produces matching checksums, while the version below
  correctly produces unmatching checksums:
  - Aero Fighters (U) [b1]
  - NCAA Basketball (Beta) [o1]
  - Prime (Beta) [h1C]
  - Wild Guns (J) (Sample) (NG-Dump Known) [h1]
*/
{
  size_t i;
  unsigned short int sum = 0;

  if ((snes_header.rom_type == 0xf5 && snes_header.map_type != 0x30) ||
      snes_header.rom_type == 0xf9 || bs_dump)
    {
      for (i = 0; i < rom_size; i++)
        sum += rom_buffer[i];                   // Far East of Eden Zero (J)

      if (rom_size == 24 * MBIT)
        sum *= 2;                               // Momotaro Dentetsu Happy (J)

      if (bs_dump)                              // Broadcast Satellaview "ROM"
        for (i = rominfo->header_start;
             i < rominfo->header_start + SNES_HEADER_LEN; i++)
          sum -= rom_buffer[i];
    }
  else
    {
      // largest known cart size is 48 Mbit (internal size 64 Mbit)
      size_t internal_rom_size =
               snes_header.rom_size > 0 && snes_header.rom_size <= 13 ?
                 1U << (snes_header.rom_size + 10) : st_dump ?
                   rom_size - 8 * MBIT : rom_size,
             half_internal_rom_size = internal_rom_size >> 1,
             i_start = st_dump ? 8 * MBIT : 0,
             i_end = i_start + (half_internal_rom_size > rom_size ?
                       rom_size : half_internal_rom_size),
             remainder = rom_size - i_start - half_internal_rom_size;
      unsigned short int sum2 = 0;

      for (i = i_start; i < i_end; i++)         // normal ROM
        sum += rom_buffer[i];
      for (i = i_start + half_internal_rom_size; i < rom_size; i++)
        sum2 += rom_buffer[i];

      if (!remainder)                           // don't divide by zero below
        remainder = half_internal_rom_size;
      sum += (unsigned short) (sum2 * (half_internal_rom_size / remainder));
    }

  return sum;
}
#else
// Calculate the checksum of a SNES ROM.
{
  // largest known cart size is 48 Mbit (internal size 64 Mbit)
  size_t internal_rom_size =
           snes_header.rom_size > 0 && snes_header.rom_size <= 13 ?
             1U << (snes_header.rom_size + 10) : st_dump ?
               rom_size - 8 * MBIT : rom_size,
         diff_size = internal_rom_size - rom_size <= rom_size ?
                       internal_rom_size - rom_size : 0,
         i, i_end = (snes_header.rom_type == 0xf5 &&
                     snes_header.map_type == 0x3a && rom_size == 24 * MBIT) ||
                    bs_dump ||
                    snes_header.rom_type == 0xf9 || // Far East of Eden Zero (J)
                    internal_rom_size <= rom_size ?
                      rom_size : rom_size - ((diff_size % (6 * MBIT)) ?
                                               diff_size : diff_size / 3);
  unsigned short int sum = 0;

  for (i = st_dump ? 8 * MBIT : 0; i < i_end; i++)
    sum += rom_buffer[i];
  if (snes_header.rom_type == 0xf5 && snes_header.map_type == 0x3a &&
      rom_size == 24 * MBIT)                    // Momotaro Dentetsu Happy (J)
    sum *= 2;
  else if (bs_dump)                             // Broadcast Satellaview "ROM"
    for (i = rominfo->header_start;
         i < rominfo->header_start + SNES_HEADER_LEN; i++)
      sum -= rom_buffer[i];
  else if (i_end < rom_size)
    {
      // diff_size >= internal_rom_size / 4 to match the snes_chksum() above (28 Mbit ROMs)
      unsigned char factor = diff_size % (6 * MBIT) == 0 ? // 6(16-10),12(32-20),24(64-40)
                               4 : diff_size >= internal_rom_size / 4 ?
                                 2 : 1;
      for (i = i_end; i < rom_size; i++)
        sum += factor * rom_buffer[i];
    }

  return sum;
}
#endif


static inline int
snes_isprint (char *s, int len)
{
  unsigned char *p = (unsigned char *) s;

  for (; len >= 0; p++, len--)
    // we don't use isprint(), because we don't want to get different results
    //  of check_banktype() for different locale settings
    if (*p < 0x20 || *p > 0x7e)
      return FALSE;

  return TRUE;
}


static int
check_banktype (unsigned char *rom_buffer, int header_offset)
/*
  This function is used to check if the value of header_offset is a good guess
  for the location of the internal SNES header (and thus of the bank type
  (LoROM, HiROM or Extended HiROM)). The higher the returned value, the higher
  the chance the guess was correct.
*/
{
  int score = 0, x, y;

//  dumper (stdout, (char *) rom_buffer + SNES_HEADER_START + header_offset,
//           SNES_HEADER_LEN, SNES_HEADER_START + header_offset, DUMPER_HEX);

  // game ID info (many games don't have useful info here)
  if (snes_isprint ((char *) rom_buffer + SNES_HEADER_START + header_offset + 2, 4))
    score += 1;

  if (!bs_dump)
    {
      if (snes_isprint ((char *) rom_buffer + SNES_HEADER_START + header_offset + 16,
                        SNES_NAME_LEN))
        score += 1;

      // map type
      x = rom_buffer[SNES_HEADER_START + header_offset + 37];
      if ((x & 0xf) < 4)
        score += 2;
      y = rom_buffer[SNES_HEADER_START + header_offset + 38];
      if (snes_hirom_ok && !(y == 0x34 || y == 0x35)) // ROM type for SA-1
        // map type, HiROM flag (only if we're sure about value of snes_hirom)
        if ((x & 1) == ((header_offset >= snes_header_base + SNES_HIROM) ? 1 : 0))
          score += 1;

      // ROM size
      if (1 << (rom_buffer[SNES_HEADER_START + header_offset + 39] - 7) <= 64)
        score += 1;

      // SRAM size
      if (1 << rom_buffer[SNES_HEADER_START + header_offset + 40] <= 256)
        score += 1;

      // country
      if (rom_buffer[SNES_HEADER_START + header_offset + 41] <= 13)
        score += 1;
    }
  else
    {
      // map type, HiROM flag
      if (snes_hirom_ok &&
          (rom_buffer[SNES_HEADER_START + header_offset + 40] & 1) ==
            (header_offset >= snes_header_base + SNES_HIROM ? 1 : 0))
        score += 1;
    }

  // publisher "escape code"
  if (rom_buffer[SNES_HEADER_START + header_offset + 42] == 0x33)
    score += 2;
  else // publisher code
    if (snes_isprint ((char *) rom_buffer + SNES_HEADER_START + header_offset, 2))
      score += 2;

  // version
  if (rom_buffer[SNES_HEADER_START + header_offset + 43] <= 2)
    score += 2;

  // checksum bytes
  x = rom_buffer[SNES_HEADER_START + header_offset + 44] +
      (rom_buffer[SNES_HEADER_START + header_offset + 45] << 8);
  y = rom_buffer[SNES_HEADER_START + header_offset + 46] +
      (rom_buffer[SNES_HEADER_START + header_offset + 47] << 8);
  if (x + y == 0xffff)
    {
      if (x == 0xffff || y == 0xffff)
        score += 3;
      else
        score += 4;
    }

  // reset vector
  if (rom_buffer[SNES_HEADER_START + header_offset + 0x4d] & 0x80)
    score += 3;

  return score;
}


int
snes_demirror (st_ucon64_nfo_t *rominfo)        // nice verb :-)
{
  size_t size = (size_t) ucon64.file_size - rominfo->backup_header_len,
         fixed = 0, mirror_size = 0;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *buffer;

  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  ucon64_fread (buffer, rominfo->backup_header_len, size, ucon64.fname);
  if (rominfo->interleaved)
    {
      puts ("NOTE: ROM is interleaved -- deinterleaving");
      snes_deinterleave (rominfo, &buffer, size);
    }

  if (size % (12 * MBIT) == 0 && size != 36 * MBIT) // 12, 24 or 48 Mbit dumps can be mirrored
    mirror_size = size / 12 * 2;
  else if (size == 16 * MBIT)                   // ...and some C4 dumps too
    mirror_size = 4 * MBIT;
  else if (size == 32 * MBIT)                   // ...and some SA-1 dumps too
    mirror_size = 8 * MBIT;

  if (mirror_size)
    {
      if (memcmp (buffer + size - mirror_size, buffer + size - 2 * mirror_size,
                  mirror_size) == 0)
        {
          if (ucon64.quiet < 0)
            printf ("Mirrored: %u - %u == %u - %u\n",
                    (unsigned) (size - 2 * mirror_size) / MBIT,
                    (unsigned) (size - mirror_size) / MBIT,
                    (unsigned) (size - mirror_size) / MBIT,
                    (unsigned) size / MBIT);
          size -= mirror_size;
          fixed = 1;
        }
    }

  if (!fixed)
    {
      if (ucon64.quiet < 1)
        puts ("NOTE: Did not detect a mirrored block -- no file has been written");
      free (buffer);
      return 1;
    }

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, src_name, 0);

  fcopy (src_name, 0, rominfo->backup_header_len, dest_name, "wb");
  ucon64_fwrite (buffer, rominfo->backup_header_len, size, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  free (buffer);
  return 0;
}


static void
write_game_table_entry (FILE *destfile, int file_no, st_ucon64_nfo_t *rominfo, int size)
{
  int n, uses_DSP;
  unsigned char name[0x1c], flags1, flags2;
  static int slot = 0;

  uses_DSP = snes_header.rom_type == 3 || snes_header.rom_type == 5 ||
             snes_header.rom_type == 0xf6;

  fseek (destfile, 0x4000 + (file_no - 1) * 0x20, SEEK_SET);
  fputc (0xff, destfile);                       // 0x0 = 0xff
  for (n = 0; rominfo->name[n] != '\0'; n++)    // name is max 21 chars (< 28)
    name[n] = isprint ((int) rominfo->name[n]) ?
                (unsigned char) toupper ((int) rominfo->name[n]) : '.';
  // the Super Flash loader (SFBOTX2.GS) only supports upper case characters
  memset (name + n, ' ', 0x1c - n);
  fwrite (name, 1, 0x1c, destfile);             // 0x1 - 0x1c = name

  if (snes_sram_size)
    {
      if (snes_sram_size == 2 * 1024)
        flags2 = 0x00;
      else if (snes_sram_size == 8 * 1024)
        flags2 = 0x10;
      else if (snes_sram_size == 32 * 1024)
        flags2 = 0x20;
      else // if (snes_sram_size == 128 * 1024) // default to 1024 kbit SRAM
        flags2 = 0x30;
    }
  else
    flags2 = 0x40;

  if (snes_header_base == SNES_EROM)            // enable Extended Map for >32 Mbit ROMs
    flags2 |= 0x80;

  flags1 = snes_hirom ? 0x10 : 0x00;

  if (!snes_hirom && uses_DSP)                  // set LoROM DSP flag if necessary
    flags1 |= 0x01;

  if (slot == 0)
    flags1 |= 0x00;
  else if (slot == 0x200000)
    flags1 |= 0x40;
  else if (slot == 0x400000)
    flags1 |= 0x20;
  else if (slot == 0x600000)
    flags1 |= 0x60;

  slot += (size + 16 * MBIT - 1) & ~(16 * MBIT - 1);

  fputc (flags1, destfile);                     // 0x1d = mapping flags
  fputc (flags2, destfile);                     // 0x1e = SRAM flags
  fputc (size / 0x8000, destfile);              // 0x1f = ROM size (not used by loader, but by us)
}


int
snes_multi (unsigned int truncate_size)
{
  unsigned int n, n_files, file_no, bytestowrite, byteswritten, done,
               truncated = 0, totalsize_disk = 0, totalsize_card = 0,
               org_do_not_calc_crc = ucon64.do_not_calc_crc;
  struct stat fstate;
  FILE *srcfile, *destfile;
  char destname[FILENAME_MAX];
  unsigned char buffer[32 * 1024];

  if (truncate_size == 0)
    {
      fputs ("ERROR: Cannot make multi-game file of 0 bytes\n", stderr);
      return -1;
    }

  n_files = ucon64.argc - 1;
  snprintf (destname, FILENAME_MAX, "%s", ucon64.argv[n_files]);
  destname[FILENAME_MAX - 1] = '\0';

  ucon64_file_handler (destname, NULL, OF_FORCE_BASENAME);
  if ((destfile = fopen (destname, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], destname);
      return -1;
    }

  printf ("Creating multi-game file for Super Flash: %s\n", destname);

  file_no = 0;
  for (n = 1; n < n_files; n++)
    {
      if (access (ucon64.argv[n], F_OK))
        continue;                               // "file" does not exist (option)
      stat (ucon64.argv[n], &fstate);
      if (!S_ISREG (fstate.st_mode))
        continue;
      if (file_no == 5)                         // loader + 4 games
        {
          puts ("WARNING: A multi-game file can contain a maximum of 4 games. The other files\n"
                "         are ignored");
          break;
        }

      ucon64.console = UCON64_UNKNOWN;
      ucon64.fname = ucon64.argv[n];
      ucon64.file_size = fsizeof (ucon64.fname);
      // DON'T use fstate.st_size, because file could be compressed
      ucon64.do_not_calc_crc = 1;
      if (snes_init (ucon64.nfo) != 0)
        printf ("WARNING: %s does not appear to be a SNES ROM\n", ucon64.fname);
      else if (ucon64.nfo->interleaved)
        printf ("WARNING: %s appears to be interleaved\n", ucon64.fname);

      if ((srcfile = fopen (ucon64.fname, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.fname);
          continue;
        }
      if (ucon64.nfo->backup_header_len)
        fseek (srcfile, ucon64.nfo->backup_header_len, SEEK_SET);

      if (file_no == 0)
        {
          printf ("Loader: %s\n", ucon64.fname);
          if (ucon64.file_size - ucon64.nfo->backup_header_len != 32 * 1024)
            printf ("WARNING: Are you sure %s is a loader binary?\n", ucon64.fname);
        }
      else
        {
          printf ("ROM%u: %s\n", file_no, ucon64.fname);
          write_game_table_entry (destfile, file_no, ucon64.nfo,
                                  (unsigned int) ucon64.file_size -
                                    ucon64.nfo->backup_header_len);
          fseek (destfile, totalsize_disk, SEEK_SET); // restore file pointer
        }

      done = 0;
      byteswritten = 0;                         // # of bytes written per file
      while (!done)
        {
          bytestowrite = fread (buffer, 1, sizeof buffer, srcfile);
          if (totalsize_disk + bytestowrite > truncate_size)
            {
              bytestowrite = truncate_size - totalsize_disk;
              done = 1;
              truncated = 1;
              printf ("Output file is %u Mbit, truncating %s, skipping %u bytes\n",
                      truncate_size / MBIT, ucon64.fname,
                      (unsigned int) ucon64.file_size -
                        ucon64.nfo->backup_header_len -
                        (byteswritten + bytestowrite));
            }
          else if (totalsize_card + bytestowrite > 64 * MBIT - 32 * 1024)
            {
              /*
                Note that it is correct to check for any size larger than 64
                Mbit - 32 kB, as we always overwrite the last 32 kB of the flash
                card. Note also that this means it's useless to write a smaller
                loader.
              */
              bytestowrite = 64 * MBIT - 32 * 1024 - totalsize_card;
              done = 1;
              truncated = 1;
              printf ("Output file needs 64 Mbit on flash card, truncating %s, skipping %u bytes\n",
                      ucon64.fname, (unsigned int) ucon64.file_size -
                        ucon64.nfo->backup_header_len - (byteswritten + bytestowrite));
            }
          totalsize_disk += bytestowrite;
          if (file_no > 0)
            totalsize_card += bytestowrite;
          if (bytestowrite == 0)
            done = 1;
          fwrite (buffer, 1, bytestowrite, destfile);
          byteswritten += bytestowrite;
        }

      file_no++;

      // We don't need padding for Super Flash as sf_write_rom() will care
      //  about alignment. Games have to be aligned to a 16 Mbit boundary.
      totalsize_card = (totalsize_card + 16 * MBIT - 1) & ~(16 * MBIT - 1);
      fclose (srcfile);
      if (truncated)
        break;
    }
  // fill the next game table entry
  fseek (destfile, 0x4000 + (file_no - 1) * 0x20, SEEK_SET);
  fputc (0, destfile);                          // indicate no next game
  fclose (destfile);
  ucon64.console = UCON64_SNES;
  ucon64.do_not_calc_crc = org_do_not_calc_crc;

  return 0;
}


int
snes_densrt (st_ucon64_nfo_t *rominfo)
{
  size_t size = (size_t) ucon64.file_size - rominfo->backup_header_len;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char backup_header[512], *buffer;

  if (!nsrt_header)
    {
      if (ucon64.quiet < 1)
        puts ("NOTE: ROM has no NSRT header -- no file has been written");
      return 1;
    }

  ucon64_fread (backup_header, 0, 512, ucon64.fname);
  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  ucon64_fread (buffer, rominfo->backup_header_len, size, ucon64.fname);

  get_nsrt_info (buffer, get_header_start (rominfo, size), backup_header);
  memset (backup_header + 0x1d0, 0, 32);        // remove NSRT header

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, src_name, 0);

  ucon64_fwrite (backup_header, 0, sizeof backup_header, dest_name, "wb");
  if (rominfo->backup_header_len > sizeof backup_header)
    fcopy (src_name, sizeof backup_header,
           rominfo->backup_header_len - sizeof backup_header, dest_name, "ab");
  ucon64_fwrite (buffer, rominfo->backup_header_len, size, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  free (buffer);
  return 0;
}


static void
set_nsrt_checksum (unsigned char *header)
{
  int n;
  char checksum = -1;

  for (n = 0x1d0; n <= 0x1ed; n++)
    checksum += header[n];
  header[0x1ee] = checksum;
  header[0x1ef] = ~checksum;
}


static void
set_nsrt_info (st_ucon64_nfo_t *rominfo, unsigned char *header)
/*
  This function will write an NSRT header if the user specified a controller
  type, but only if the checksum is correct. We write a complete NSRT header
  only to be 100% compatible with NSRT. We are only interested in the
  controller type feature, though.
  NSRT is a SNES ROM tool. See developers.html.

  NSRT header format (0x1d0 - 0x1ef, offsets in _copier header_):
  0x1d0                 low nibble = original country value
                        high nibble = bank type
                        1 = LoROM
                        2 = HiROM
                        3 = "Extended" HiROM
  0x1d1 - 0x1e5         original game name
  0x1e6                 low byte of original SNES checksum
  0x1e7                 high byte of original SNES checksum
  0x1e8 - 0x1eb         "NSRT"
  0x1ec                 header version; a value of for example 15 should be
                        interpreted as 1.5
  0x1ed                 low nibble = port 2 controller type
                        high nibble = port 1 controller type
                        0 = gamepad
                        1 = mouse
                        2 = mouse / gamepad
                        3 = super scope
                        4 = super scope / gamepad
                        5 = Konami's justifier
                        6 = multitap
                        7 = mouse / super scope / gamepad
  0x1ee                 NSRT header checksum
                        the checksum is calculated by adding all bytes of the
                        NSRT header (except the checksum bytes themselves)
                        and then subtracting 1
  0x1ef                 inverse NSRT header checksum
*/
{
  int x;

  if ((UCON64_ISSET (ucon64.controller) || UCON64_ISSET (ucon64.controller2)) &&
      !nsrt_header)                             // don't overwrite these values
    {
      if (rominfo->current_internal_crc != rominfo->internal_crc)
        {
          puts ("WARNING: The controller type info will be discarded (checksum is bad)");
          return;
        }

      header[0x1d0] = bs_dump ? 0 : snes_header.country;
      if (rominfo->header_start == SNES_EROM + SNES_HEADER_START + SNES_HIROM)
        header[0x1d0] |= 0x30;                  // NOTE: Extended LoROM is not supported
      else
        header[0x1d0] |= snes_hirom ? 0x20 : 0x10;

      if (!bs_dump && st_dump)
        memcpy (header + 0x1d1, rominfo->name, 16);
      else // for ST dumps, rominfo->name may be used
        memcpy (header + 0x1d1, &snes_header.name, SNES_NAME_LEN);
      header[0x1e6] = snes_header.checksum_low;
      header[0x1e7] = snes_header.checksum_high;
      memcpy (header + 0x1e8, "NSRT", 4);
      header[0x1ec] = NSRT_HEADER_VERSION;
    }

  if (UCON64_ISSET (ucon64.controller))
    {
      for (x = 0; x < 8; x++)
        if ((ucon64.controller >> x) & 1)
          break;
      if (x != 0 && x != 1 && x != 2 && x != 6)
        {
          puts ("WARNING: Invalid value for controller in port 1, using \"0\"");
          x = 0;
        }
      header[0x1ed] = (unsigned char) (x << 4);
    }
  if (UCON64_ISSET (ucon64.controller2))
    {
      for (x = 0; x < 8; x++)
        if ((ucon64.controller2 >> x) & 1)
          break;
      if (x >= 8)
        {
          puts ("WARNING: Invalid value for controller in port 2, using \"0\"");
          x = 0;
        }
      header[0x1ed] |= x;
    }

  // set the checksum bytes
  if (UCON64_ISSET (ucon64.controller) || UCON64_ISSET (ucon64.controller2))
    set_nsrt_checksum (header);
}


static void
get_nsrt_info (unsigned char *rom_buffer, int header_start, unsigned char *backup_header)
{
  if (nsrt_header)
    {
      memcpy (rom_buffer + header_start + 16 - (st_dump ? SNES_HEADER_START : 0),
              backup_header + 0x1d1, (bs_dump || st_dump) ? 16 : SNES_NAME_LEN); // name
      // we ignore interleaved ST dumps
      if (!bs_dump)
        {
          // According to the NSRT specification, the region byte should be set
          //  to 0 for BS dumps.
          rom_buffer[header_start + 41] = backup_header[0x1d0] & 0x0f; // region
          // NSRT only modifies the internal header. For BS dumps the internal
          //  checksum does not include the header. So, we don't have to
          //  overwrite the checksum.
          rom_buffer[header_start + 44] = ~backup_header[0x1e6]; // inverse checksum low
          rom_buffer[header_start + 45] = ~backup_header[0x1e7]; // inverse checksum high
          rom_buffer[header_start + 46] = backup_header[0x1e6]; // checksum low
          rom_buffer[header_start + 47] = backup_header[0x1e7]; // checksum high
        }
    }
}


static void
reset_header (void *header)
{
  // preserve possible NSRT header
  if (nsrt_header)
    {
      memset (header, 0, 0x1d0);
      memset ((unsigned char *) header + 0x1f0, 0, 16);
      ((unsigned char *) header)[0x1ec] = NSRT_HEADER_VERSION;
      set_nsrt_checksum ((unsigned char *) header);
    }
  else
    memset (header, 0, SWC_HEADER_LEN);
}


static void
handle_nsrt_header (st_ucon64_nfo_t *rominfo, unsigned char *header,
                    const char **snes_country)
{
  char name[SNES_NAME_LEN + 1], *str_list[9] =
    {
      "Gamepad", "Mouse", "Mouse / Gamepad", "Super Scope",
      "Super Scope / Gamepad", "Konami's Justifier", "Multitap",
      "Mouse / Super Scope / Gamepad", "Unknown"
    };
  int x = header[0x1ed], ctrl1 = x >> 4, ctrl2 = x & 0xf,
      name_len = (bs_dump || st_dump) ? 16 : SNES_NAME_LEN;

  memcpy (name, header + 0x1d1, name_len);
  name[name_len] = 0;
  for (x = 0; x < name_len; x++)
    if (!isprint ((int) name[x]))
      name[x] = '.';

  if (ctrl1 > 8)
    ctrl1 = 8;
  if (ctrl2 > 8)
    ctrl2 = 8;
  sprintf (rominfo->misc + strlen (rominfo->misc),
           "\nNSRT info:\n"
           "  Original country: %s\n"
           "  Original game name: \"%s\"\n"
           "  Original checksum: 0x%04x\n"
           "  Port 1 controller type: %s\n"
           "  Port 2 controller type: %s\n"
           "  Header version: %.1f",
           NULL_TO_UNKNOWN_S (snes_country[MIN (header[0x1d0] & 0xf, SNES_COUNTRY_MAX - 1)]),
           name,
           header[0x1e6] + (header[0x1e7] << 8),
           str_list[ctrl1],
           str_list[ctrl2],
           header[0x1ec] / 10.f);
}


int
snes_create_sram (void)
{
  int size = snes_sram_size > 256 * 1024 ? 256 * 1024 : snes_sram_size;
  char dest_name[FILENAME_MAX];
  FILE *destfile;
  unsigned char *buffer;

  if (size == 0)
    {
      if (ucon64.quiet < 1)
        puts ("NOTE: ROM does not use SRAM -- no file has been written");
      return 1;
    }

  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".srm");
  ucon64_file_handler (dest_name, NULL, 0);

  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }

  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      exit (1);
    }
  memset (buffer, 0, size);
  fwrite (buffer, 1, size, destfile);
  fclose (destfile);

  printf (ucon64_msg[WROTE], dest_name);
  free (buffer);
  return 0;
}
