/*
genesis.c - Sega Genesis/Mega Drive support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2002 - 2003 dbjh


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
// NOTE: The people at Sega refer to their company as Sega in normal text (not as SEGA)
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
#include <sys/stat.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "console/genesis.h"
#include "backup/mgd.h"
#include "backup/smd.h"


#define GENESIS_HEADER_START 256
#define GENESIS_HEADER_LEN (sizeof (st_genesis_header_t))
#define GENESIS_NAME_LEN 48

static int genesis_chksum (unsigned char *rom_buffer);
static unsigned char *load_rom (st_rominfo_t *rominfo, const char *name,
                                unsigned char *rom_buffer);
static int save_smd (const char *name, unsigned char *buffer,
                     st_smd_header_t *header, long size);
static int save_bin (const char *name, unsigned char *buffer, long size);
static int save_mgd (const char *name, unsigned char **buffer, long size);


const st_usage_t genesis_usage[] =
  {
    {NULL, NULL, "Genesis/Sega Mega Drive/Sega CD/32X/Nomad"},
    {NULL, NULL, "1989/19XX/19XX Sega http://www.sega.com"},
    {"gen", NULL, "force recognition"},
    {"int", NULL, "force ROM is in interleaved format (SMD)"},
    {"int2", NULL, "force ROM is in interleaved format 2 (MGD)"},
    {"nint", NULL, "force ROM is not in interleaved format (BIN/RAW)"},
    {"n", "NEW_NAME", "change foreign ROM name to NEW_NAME"},
    {"n2", "NEW_NAME", "change Japanese ROM name to NEW_NAME"},
    {"smd", NULL, "convert to Super Magic Drive/SMD"},
    {"smds", NULL, "convert emulator (*.srm) SRAM to Super Magic Drive/SMD"},
    {"bin", NULL, "convert to binary format/BIN/RAW"},
    {"mgd", NULL, "convert to Multi Game*/MGD2/MGH"},
#if 0
    {"gf", NULL, "convert Sega CD country code to Europe; ROM=$CD_IMAGE"},
    {"ga", NULL, "convert Sega CD country code to U.S.A.; ROM=$CD_IMAGE"},
    {"gc", NULL, "convert to Genecyst (emulator)/GSV save state; " OPTION_LONG_S "rom=SAVESTATE"},
    {"ge", NULL, "convert to GenEm (emulator)/SAV save state; " OPTION_LONG_S "rom=SAVESTATE"},
    {"gym", NULL, "convert GYM (Genecyst) sound to WAV; " OPTION_LONG_S "rom=GYMFILE"},
    {"cym", NULL, "convert CYM (Callus emulator) sound to WAV; " OPTION_LONG_S "rom=CYMFILE"},
#endif
    {"stp", NULL, "convert SRAM from backup unit for use with an emulator\n"
               OPTION_LONG_S "stp just strips the first 512 bytes"},
    {"j", NULL, "join split ROM"},
    {"s", NULL, "split ROM; default part size is 4 Mb"},
    {"ssize", "SIZE", "specify split part size in Mbit"},
#if 0
    {"p", NULL, "pad ROM to full Mb"},
#endif
    {"f", NULL, "remove NTSC/PAL protection"},
    {"chk", NULL, "fix ROM checksum"},
    {"1991", NULL, "fix old third party ROMs to work with consoles build after\n"
                "October 1991 by inserting \"(C) SEGA\" and \"(C)SEGA\""},
    {"multi", "SIZE", "make multi-game file for use with MD-PRO flash card,\n"
                      "truncated to SIZE Mbit; file with loader must be specified\n"
                      "first, then all the ROMs, multi-game file to create last"},
    {"pal", NULL, "specify console to use multi-game file on is a\n"
                  "European Mega Drive (PAL); use in combination with -multi"},
    {NULL, NULL, NULL}
  };

const st_usage_t bin_usage[] =
  {
    {NULL, NULL, "Binary file/BIN/RAW"},
    {NULL, NULL, "Emulator format?"},
    {NULL, NULL, NULL}
  };

typedef struct st_genesis_header
{
  char pad[256];
} st_genesis_header_t;

static st_genesis_header_t genesis_header;
static genesis_file_t type;
static int genesis_rom_size, genesis_has_ram, genesis_tv_standard;


genesis_file_t
genesis_get_file_type (void)
{
  return type;
}


int
genesis_smd (st_rominfo_t *rominfo)
{
  st_smd_header_t header;
  char dest_name[FILENAME_MAX];
  unsigned char *rom_buffer = NULL;

  if ((rom_buffer = load_rom (rominfo, ucon64.rom, rom_buffer)) == NULL)
    return -1;

  memset (&header, 0, SMD_HEADER_LEN);
  header.size = genesis_rom_size / 16384;
  header.id0 = 3;
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 6;

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".SMD");
  ucon64_file_handler (dest_name, NULL, 0);

  save_smd (dest_name, rom_buffer, &header, genesis_rom_size);
  free (rom_buffer);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
genesis_smds (void)
{
  char dest_name[FILENAME_MAX];
  unsigned char buf[32768];
  st_smd_header_t header;

  memset (&header, 0, SMD_HEADER_LEN);
  memset (&buf, 0, 32768);
  q_fread (buf, 0, ucon64.file_size, ucon64.rom);

  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 7;                              // SRAM file

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".SAV");
  ucon64_file_handler (dest_name, NULL, 0);

  save_smd (dest_name, buf, &header, 32768);    // SMD SRAM files are interleaved

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
genesis_bin (st_rominfo_t *rominfo)
{
  char dest_name[FILENAME_MAX];
  unsigned char *rom_buffer = NULL;

  if ((rom_buffer = load_rom (rominfo, ucon64.rom, rom_buffer)) == NULL)
    return -1;

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".BIN");
  ucon64_file_handler (dest_name, NULL, 0);

  save_bin (dest_name, rom_buffer, genesis_rom_size);
  free (rom_buffer);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


// see src/backup/mgd.h for the file naming scheme
int
genesis_mgd (st_rominfo_t *rominfo)
{
#define CC (const char)
  int x, y;
  unsigned char *rom_buffer = NULL;
  char dest_name[FILENAME_MAX], buf[FILENAME_MAX], mgh[512];
  const char mghcharset[1024] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* ! */ 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* & */ 0x3c, 0x66, 0x18, 0x3c, 0x66, 0x66, 0x3c, 0x02,
    /* ' */ 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    /* ( */ 0x0c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x0c, 0x00,
    /* ) */ 0x30, 0x18, 0x18, 0x18, 0x18, 0x18, 0x30, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* , */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x08,
    /* - */ 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00,
    /* . */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00,
    /* / */ 0x06, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x60, 0x00,
    /* 0 */ 0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00,
    /* 1 */ 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
    /* 2 */ 0x3c, 0x66, 0x06, 0x06, 0x7c, 0x60, 0x7e, 0x00,
    /* 3 */ 0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00,
    /* 4 */ 0x18, 0x38, 0x58, 0x7c, 0x18, 0x18, 0x3c, 0x00,
    /* 5 */ 0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00,
    /* 6 */ 0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00,
    /* 7 */ 0x7e, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x18, 0x00,
    /* 8 */ 0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00,
    /* 9 */ 0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00,
    /* : */ 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00,
    /* ; */ 0x00, 0x00, 0x18, 0x00, 0x18, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* = */ 0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* A */ 0x18, 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x00,
    /* B */ 0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00,
    /* C */ 0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00,
    /* D */ 0x7c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7c, 0x00,
    /* E */ 0x7e, 0x60, 0x60, 0x7e, 0x60, 0x60, 0x7e, 0x00,
    /* F */ 0x7e, 0x60, 0x60, 0x7e, 0x60, 0x60, 0x60, 0x00,
    /* G */ 0x3e, 0x60, 0x60, 0x6e, 0x66, 0x66, 0x3e, 0x00,
    /* H */ 0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00,
    /* I */ 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
    /* J */ 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3c, 0x00,
    /* K */ 0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00,
    /* L */ 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00,
    /* M */ CC 0xc6, CC 0xee, CC 0xfe, CC 0xd6, CC 0xc6, CC 0xc6, CC 0xc6, 0x00,
    /* N */ 0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00,
    /* O */ 0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00,
    /* P */ 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00,
    /* Q */ 0x3c, 0x66, 0x66, 0x66, 0x66, 0x6e, 0x3e, 0x00,
    /* R */ 0x7c, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0x66, 0x00,
    /* S */ 0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00,
    /* T */ 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
    /* U */ 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00,
    /* V */ 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x3c, 0x00,
    /* W */ 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x2c, 0x00,
    /* X */ 0x66, 0x66, 0x3c, 0x10, 0x3c, 0x66, 0x66, 0x00,
    /* Y */ 0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00,
    /* Z */ 0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* _ */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* a */ 0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00,
    /* b */ 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00,
    /* c */ 0x00, 0x00, 0x3c, 0x66, 0x60, 0x66, 0x3c, 0x00,
    /* d */ 0x06, 0x06, 0x6e, 0x66, 0x66, 0x66, 0x3e, 0x00,
    /* e */ 0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00,
    /* f */ 0x0c, 0x18, 0x18, 0x3c, 0x18, 0x18, 0x3c, 0x00,
    /* g */ 0x00, 0x00, 0x3c, 0x66, 0x66, 0x3e, 0x06, 0x7c,
    /* h */ 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00,
    /* i */ 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
    /* j */ 0x00, 0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0x6c, 0x38,
    /* k */ 0x60, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00,
    /* l */ 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
    /* m */ 0x00, 0x00, 0x5c, 0x76, 0x56, 0x56, 0x56, 0x00,
    /* n */ 0x00, 0x00, 0x6c, 0x7e, 0x66, 0x66, 0x66, 0x00,
    /* o */ 0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00,
    /* p */ 0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60,
    /* q */ 0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06,
    /* r */ 0x00, 0x00, 0x18, 0x1a, 0x18, 0x18, 0x3c, 0x00,
    /* s */ 0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00,
    /* t */ 0x00, 0x18, 0x3c, 0x18, 0x18, 0x18, 0x3c, 0x00,
    /* u */ 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3a, 0x00,
    /* v */ 0x00, 0x00, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x00,
    /* w */ 0x00, 0x00, 0x56, 0x56, 0x56, 0x56, 0x7e, 0x00,
    /* x */ 0x00, 0x00, 0x66, 0x66, 0x18, 0x66, 0x66, 0x00,
    /* y */ 0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x7c,
    /* z */ 0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* | */ 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  // pad the file to the next valid MGD2 size if it doesn't have a valid size
  if (genesis_rom_size <= 1 * MBIT)
    genesis_rom_size = 1 * MBIT;
  else if (genesis_rom_size <= 2 * MBIT)
    genesis_rom_size = 2 * MBIT;
  else if (genesis_rom_size <= 4 * MBIT)
    genesis_rom_size = 4 * MBIT;
  else if (genesis_rom_size <= 8 * MBIT)
    genesis_rom_size = 8 * MBIT;
  else if (genesis_rom_size <= 16 * MBIT)
    genesis_rom_size = 16 * MBIT;
  else if (genesis_rom_size <= 24 * MBIT)
    genesis_rom_size = 24 * MBIT;
  else
    genesis_rom_size = 32 * MBIT;

  if ((rom_buffer = load_rom (rominfo, ucon64.rom, rom_buffer)) == NULL)
    return -1;

  mgd_make_name (ucon64.rom, "MD", genesis_rom_size, dest_name);
  ucon64_file_handler (dest_name, NULL, OF_FORCE_BASENAME);

  save_mgd (dest_name, &rom_buffer, genesis_rom_size);
  printf (ucon64_msg[WROTE], dest_name);
  free (rom_buffer);

  // automatically create MGH name file
  memset (mgh, 0, sizeof (mgh));
  mgh[0] = 'M';
  mgh[1] = 'G';
  mgh[2] = 'H';
  mgh[3] = 0x1a;
  mgh[4] = 0x06;
  mgh[5] = (char) 0xf0;
  mgh[31] = (char) 0xff;

  // In addition to the above, uCON also does "memcpy (mgh + 16, "MGH By uCON/chp", 15);"

  memcpy (buf, rominfo->name, 15);              // copy first 15 bytes (don't use strlen() or strcpy())
  for (x = 0; x < 15; x++)
    {
      for (y = 0; y < 4; y++)
        mgh[((x + 2) * 16) + y + 4] = mghcharset[(((unsigned char *) buf)[x] * 8) + y];
      for (y = 4; y < 8; y++)
        mgh[((x + 2) * 16) + y + 244] = mghcharset[(((unsigned char *) buf)[x] * 8) + y];
    }

  set_suffix (dest_name, ".MGH");
  ucon64_output_fname (dest_name, OF_FORCE_BASENAME);
  /*
    If a backup would be created it would overwrite the backup of the ROM. The
    ROM backup is more important, so we don't write a backup of the MGH file.
  */
  q_fwrite (mgh, 0, sizeof (mgh), dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


#define PARTSIZE  (4 * MBIT)                    // default split part size
int
genesis_s (st_rominfo_t *rominfo)
{
  st_smd_header_t smd_header;
  char dest_name[FILENAME_MAX];
  int x, nparts, surplus, size = ucon64.file_size - rominfo->buheader_len,
      part_size;

  if (type == BIN)
    {
      fprintf (stderr, "ERROR: Splitting binary files is not supported\n");
      return -1;
    }

  if (UCON64_ISSET (ucon64.part_size))
    {
      part_size = ucon64.part_size;
      // Don't allow too small part sizes, see src/console/snes.c (snes_s())
      if (part_size < 4 * MBIT)
        {
          fprintf (stderr,
            "ERROR: Split part size must be larger than or equal to 4 Mbit\n");
          return -1;
        }
    }
  else
    part_size = PARTSIZE;

  if (size <= part_size)
    {
      printf (
        "NOTE: ROM size is smaller than or equal to %d Mbit -- won't be split\n",
        part_size / MBIT);
      return -1;
    }

  nparts = size / part_size;
  surplus = size % part_size;

  if (type == SMD)
    {
      q_fread (&smd_header, 0, rominfo->buheader_len, ucon64.rom);

      strcpy (dest_name, ucon64.rom);
      set_suffix (dest_name, ".1");
      ucon64_output_fname (dest_name, 0);

      smd_header.size = part_size / 16384;
      smd_header.id0 = 3;
      // if smd_header.split bit 6 == 0 -> last file of the ROM
      smd_header.split |= 0x40;
      for (x = 0; x < nparts; x++)
        {
          if (surplus == 0 && x == nparts - 1)
            smd_header.split = 0;               // last file -> clear bit 6

          // don't write backups of parts, because one name is used
          q_fwrite (&smd_header, 0, SMD_HEADER_LEN, dest_name, "wb");
          q_fcpy (ucon64.rom, x * part_size + rominfo->buheader_len, part_size, dest_name, "ab");
          printf (ucon64_msg[WROTE], dest_name);

          (*(strrchr (dest_name, '.') + 1))++;
        }

      if (surplus != 0)
        {
          smd_header.size = surplus / 16384;
          smd_header.split = 0;                 // last file -> clear bit 6

          // don't write backups of parts, because one name is used
          q_fwrite (&smd_header, 0, SMD_HEADER_LEN, dest_name, "wb");
          q_fcpy (ucon64.rom, x * part_size + rominfo->buheader_len, surplus, dest_name, "ab");
          printf (ucon64_msg[WROTE], dest_name);
        }
    }
  else // type == MGD_GEN
    {
      char suffix[5];

      mgd_make_name (ucon64.rom, "MD", genesis_rom_size, dest_name);
      strcpy (suffix, (char *) get_suffix (dest_name));
      dest_name[7] = is_func (dest_name, strlen (dest_name), isupper) ? 'A' : 'a';
      dest_name[8] = 0;
      strcat (dest_name, suffix);
      ucon64_output_fname (dest_name, OF_FORCE_BASENAME);

      if (surplus)
        nparts++;
      for (x = 0; x < nparts; x++)
        {
          // don't write backups of parts, because one name is used
          // write first half of file
          q_fcpy (ucon64.rom, x * (part_size / 2), part_size / 2, dest_name, "wb");
          // write second half of file; don't do: "(nparts / 2) * part_size"!
          q_fcpy (ucon64.rom, nparts * (part_size / 2) + x * (part_size / 2),
            part_size / 2, dest_name, "ab");
          printf (ucon64_msg[WROTE], dest_name);
          (*(strrchr (dest_name, '.') - 1))++;
        }
    }
  return 0;
}


int
genesis_j (st_rominfo_t *rominfo)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  int block_size, total_size = 0;

  if (type == BIN)
    {
      fprintf (stderr, "ERROR: Joining binary files is not supported\n");
      return -1;
    }

  if (type == SMD)
    {
      unsigned char buf[3];

      strcpy (dest_name, ucon64.rom);
      set_suffix (dest_name, ".SMD");
      strcpy (src_name, ucon64.rom);
      ucon64_file_handler (dest_name, NULL, 0);

      q_fcpy (src_name, 0, rominfo->buheader_len, dest_name, "wb");
      block_size = ucon64.file_size - rominfo->buheader_len;
      while (q_fcpy (src_name, rominfo->buheader_len, block_size, dest_name, "ab") != -1)
        {
          printf ("Joined: %s\n", src_name);
          total_size += block_size;
          (*(strrchr (src_name, '.') + 1))++;
          block_size = q_fsize (src_name) - rominfo->buheader_len;
        }

      if (rominfo->buheader_len)
        {                                       // fix header
          buf[0] = total_size / 16384;          // # 16K blocks
          buf[1] = 3;                           // ID 0
          buf[2] = 0;                           // last file -> clear bit 6
          q_fwrite (buf, 0, 3, dest_name, "r+b");
          buf[0] = 0xaa;                        // ID 1
          buf[1] = 0xbb;                        // ID 2
          buf[2] = 6;                           // type Genesis
          q_fwrite (buf, 8, 3, dest_name, "r+b");
        }

      printf (ucon64_msg[WROTE], dest_name);
    }
  else // type == MGD_GEN
    {
      /*
        file1 file2 file3 file4
        1/2   3/4   5/6   7/8 (1st half/2nd half)
        joined file
        1/3/5/7/2/4/6/8
      */
      strcpy (dest_name, ucon64.rom);
      set_suffix (dest_name, ".TMP");           // users should use -mgd to get
      ucon64_file_handler (dest_name, NULL, 0); //  a correct name
      remove (dest_name);

      strcpy (src_name, ucon64.rom);
      block_size = (ucon64.file_size - rominfo->buheader_len) / 2;
      while (q_fcpy (src_name, rominfo->buheader_len, block_size, dest_name, "ab") != -1)
        (*(strrchr (src_name, '.') - 1))++;

      strcpy (src_name, ucon64.rom);
      while (q_fcpy (src_name, rominfo->buheader_len + block_size,
             block_size, dest_name, "ab") != -1)
        {
          printf ("Joined: %s\n", src_name);    // print this here, not in the
          (*(strrchr (src_name, '.') - 1))++;   //  previous loop
        }

      printf (ucon64_msg[WROTE], dest_name);
    }

  return 0;
}


static int
genesis_name (st_rominfo_t *rominfo, const char *name1, const char *name2)
{
  unsigned char *rom_buffer = NULL;
  char buf[FILENAME_MAX];

  if ((rom_buffer = load_rom (rominfo, ucon64.rom, rom_buffer)) == NULL)
    return -1;

  if (name1)
    {
      memset (buf, ' ', GENESIS_NAME_LEN);
      strncpy (buf, name1, strlen (name1) > GENESIS_NAME_LEN ?
        GENESIS_NAME_LEN : strlen (name1));
      memcpy (&rom_buffer[GENESIS_HEADER_START + 32], buf, GENESIS_NAME_LEN);
    }
  if (name2)
    {
      memset (buf, ' ', GENESIS_NAME_LEN);
      strncpy (buf, name2, strlen (name2) > GENESIS_NAME_LEN ?
        GENESIS_NAME_LEN : strlen (name2));
      memcpy (&rom_buffer[GENESIS_HEADER_START + 32 + GENESIS_NAME_LEN], buf,
        GENESIS_NAME_LEN);
    }

  strcpy (buf, ucon64.rom);
  ucon64_file_handler (buf, NULL, 0);
  if (type == SMD)
    {
      st_smd_header_t smd_header;

      q_fread (&smd_header, 0, rominfo->buheader_len, ucon64.rom);
      save_smd (buf, rom_buffer, &smd_header, genesis_rom_size);
    }
  else if (type == MGD_GEN)
    save_mgd (buf, &rom_buffer, genesis_rom_size);
  else
    save_bin (buf, rom_buffer, genesis_rom_size);

  free (rom_buffer);
  printf (ucon64_msg[WROTE], buf);
  return 0;
}


int
genesis_n (st_rominfo_t *rominfo, const char *name)
{
  return genesis_name (rominfo, name, NULL);
}


int
genesis_n2 (st_rominfo_t *rominfo, const char *name)
{
  return genesis_name (rominfo, NULL, name);
}


int
genesis_1991 (st_rominfo_t *rominfo)
{
  return genesis_name (rominfo, "(C) SEGA", "(C)SEGA");
}


int
genesis_chk (st_rominfo_t *rominfo)
{
  unsigned char *rom_buffer = NULL;
  char dest_name[FILENAME_MAX];

  if ((rom_buffer = load_rom (rominfo, ucon64.rom, rom_buffer)) == NULL)
    return -1;

  rom_buffer[GENESIS_HEADER_START + 143] = rominfo->current_internal_crc;      // low byte of checksum
  rom_buffer[GENESIS_HEADER_START + 142] = rominfo->current_internal_crc >> 8; // high byte of checksum

  mem_hexdump (&rom_buffer[GENESIS_HEADER_START + 0x8e], 2, GENESIS_HEADER_START + 0x8e);

  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, NULL, 0);
  if (type == SMD)
    {
      st_smd_header_t smd_header;

      q_fread (&smd_header, 0, rominfo->buheader_len, ucon64.rom);
      save_smd (dest_name, rom_buffer, &smd_header, genesis_rom_size);
    }
  else if (type == MGD_GEN)
    save_mgd (dest_name, &rom_buffer, genesis_rom_size);
  else
    save_bin (dest_name, rom_buffer, genesis_rom_size);

  free (rom_buffer);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


static int
genesis_fix_pal_protection (st_rominfo_t *rominfo)
{
/*
  This function searches for PAL protection codes. If it finds one it will
  fix the code so that the game will run on a Genesis (NTSC).
*/
  char header[512], src_name[FILENAME_MAX], dest_name[FILENAME_MAX],
       buffer[16 * 1024];
  FILE *srcfile, *destfile;
  int bytesread, n = 0, n_extra_patterns, n2;
  st_cm_pattern_t *patterns = NULL;

  n_extra_patterns = build_cm_patterns (&patterns, "genpal.txt", src_name);
  if (n_extra_patterns >= 0)
    printf ("Found %d additional code%s in %s\n",
            n_extra_patterns, n_extra_patterns != 1 ? "s" : "", src_name);

  puts ("Attempting to fix PAL protection code...");

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }
  if (rominfo->buheader_len)                    // copy header (if present)
    {
      fread (header, 1, SMD_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->buheader_len, SEEK_SET);
      fwrite (header, 1, SMD_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 16 * 1024, srcfile)))
    {
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
genesis_fix_ntsc_protection (st_rominfo_t *rominfo)
{
/*
  This function searches for NTSC protection codes. If it finds one it will
  fix the code so that the game will run on a Mega Drive (PAL).
*/
  char header[512], src_name[FILENAME_MAX], dest_name[FILENAME_MAX],
       buffer[16 * 1024];
  FILE *srcfile, *destfile;
  int bytesread, n = 0, n_extra_patterns, n2;
  st_cm_pattern_t *patterns = NULL;

  n_extra_patterns = build_cm_patterns (&patterns, "mdntsc.txt", src_name);
  if (n_extra_patterns >= 0)
    printf ("Found %d additional code%s in %s\n",
            n_extra_patterns, n_extra_patterns != 1 ? "s" : "", src_name);

  puts ("Attempting to fix NTSC protection code...");

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }
  if (rominfo->buheader_len)                    // copy header (if present)
    {
      fread (header, 1, SMD_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->buheader_len, SEEK_SET);
      fwrite (header, 1, SMD_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 16 * 1024, srcfile)))
    {
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
genesis_f (st_rominfo_t *rominfo)
{
  if (rominfo->interleaved)
    {
      if (ucon64.quiet <= 0)
        fprintf (stderr, "ERROR: This option works only on non-interleaved files\n"
                         "TIP: Create a deinterleaved file with -mgd\n");
      return -1;
    }

  /*
    In the Philipines the television standard is NTSC, but do games made
    for the Philipines exist?
    Just like with SNES we don't guarantee anything for files that needn't be
    fixed/cracked/patched.
  */
  if (genesis_tv_standard == 0)               // NTSC (Japan or U.S.A.)
    return genesis_fix_ntsc_protection (rominfo);
  else
    return genesis_fix_pal_protection (rominfo);
}


unsigned char *
load_rom (st_rominfo_t *rominfo, const char *name, unsigned char *rom_buffer)
{
  FILE *file;
  int bytesread;

  if ((file = fopen (name, "rb")) == NULL)
    return NULL;
  if (!(rom_buffer = (unsigned char *) malloc (genesis_rom_size)))
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], genesis_rom_size);
      fclose (file);
      return NULL;
    }
  fseek (file, rominfo->buheader_len, SEEK_SET); // don't do this only for SMD!
  bytesread = fread (rom_buffer, 1, genesis_rom_size, file);
  if (bytesread < genesis_rom_size)
    memset (rom_buffer + bytesread, 0, genesis_rom_size - bytesread);

  if (type != BIN)
    {
      if (ucon64.fcrc32 == 0)
        ucon64.fcrc32 = crc32 (ucon64.fcrc32, rom_buffer, genesis_rom_size);

      if (type == SMD)
        smd_deinterleave (rom_buffer, bytesread);
      else // type == MGD_GEN
        mgd_deinterleave (&rom_buffer, bytesread, genesis_rom_size);
    }

  if (ucon64.crc32 == 0)                        // Calculate the CRC32 only once
    ucon64.crc32 = crc32 (0, rom_buffer, bytesread);

  fclose (file);
  return rom_buffer;
}


int
save_smd (const char *name, unsigned char *buffer, st_smd_header_t *header,
          long size)
{
  smd_interleave (buffer, size);
  q_fwrite (header, 0, SMD_HEADER_LEN, name, "wb");
  return q_fwrite (buffer, SMD_HEADER_LEN, size, name, "ab");
}


int
save_mgd (const char *name, unsigned char **buffer, long size)
{
  mgd_interleave (buffer, size);
  return q_fwrite (*buffer, 0, size, name, "wb");
}


int
save_bin (const char *name, unsigned char *buffer, long size)
{
  return q_fwrite (buffer, 0, size, name, "wb");
}


static void
write_game_table_entry (FILE *destfile, int file_no, st_rominfo_t *rominfo,
                        int totalsize)
{
//  static int sram_page = 0, file_no_sram = 0;
  int m;
  unsigned char flags = 0; // SRAM/region flags: F, D (reserved), E, P, V, T, S1, S0

  fseek (destfile, 0x8000 + (file_no - 1) * 0x20, SEEK_SET);
  if (rominfo->name[0] == 0)
    rominfo->name[0] = 'D';                     // if table[0] == 0 => no next game
  for (m = 0; m < 0x1d; m++)
    if (!isprint (rominfo->name[m]))
      rominfo->name[m] = '.';
  fwrite (rominfo->name, 1, 0x1d, destfile);    // 0x0 - 0x1c = name
  fputc (0, destfile);                          // 0x1d = 0
  fputc (totalsize / (2 * MBIT), destfile);     // 0x1e = bank code

  if (genesis_has_ram)
    {
#if 0 // TODO: ask Leo how to use S1 & S0
      flags = sram_page++;
      if (sram_page == 3)
        file_no_sram = file_no;
      else if (sram_page > 3)
        {
          printf ("WARNING: This ROM will share SRAM with ROM %d\n", file_no_sram);
          sram_page = 3;
        }
#endif
      if ((ucon64.file_size - rominfo->buheader_len) > 16 * MBIT)
        flags |= 0x80;                          // set F (>16 Mb & SRAM)
    }
  else
    flags |= 4;                                 // set T (no SRAM)

  if (genesis_tv_standard == 1)
    flags |= 0x10;                              // set P(AL)

  if (ucon64.tv_standard == 1)
    flags |= 0x20;                              // set E(uro)

  if (genesis_tv_standard == 0 && ucon64.tv_standard == 1)
    {
      flags |= 8;                               // set V (Euro console & NTSC game)
      flags &= ~0x30;                           // clear E(uro) & P(AL)
    }

  fputc (flags, destfile);                      // 0x1f = flags
}


int
genesis_multi (int truncate_size, char *fname)
{
#define BUFSIZE (32 * 1024)                     // must be a multiple of 16kB
  int n, n_files, file_no, bytestowrite, byteswritten, totalsize = 0, done,
      truncated = 0, paddedsize;
  struct stat fstate;
  FILE *srcfile, *destfile;
  char *destname;
  unsigned char buffer[BUFSIZE];

  if (truncate_size == 0)
    {
      fprintf (stderr, "ERROR: Can't make multi-game file of 0 bytes\n");
      return -1;
    }

  if (fname != NULL)
    {
      destname = fname;
      n_files = ucon64.argc;
    }
  else
    {
      destname = ucon64.argv[ucon64.argc - 1];
      n_files = ucon64.argc - 1;
    }

  ucon64_file_handler (destname, NULL, OF_FORCE_BASENAME);
  if ((destfile = fopen (destname, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], destname);
      return -1;
    }
  printf ("Creating multi-game file: %s\n", destname);

  file_no = 0;
  for (n = 1; n < n_files; n++)
    {
      if (access (ucon64.argv[n], F_OK))
        continue;                               // "file" does not exist (option)
      stat (ucon64.argv[n], &fstate);
      if (!S_ISREG (fstate.st_mode))
        continue;
      if (file_no == 32)                        // loader + 31 games
        {
          printf ("WARNING: A multi-game file can contain a maximum of 31 games. The other files\n"
                  "         are ignored.\n");
          break;
        }

      ucon64.console = UCON64_UNKNOWN;
      ucon64.rom = ucon64.argv[n];
      ucon64.file_size = q_fsize (ucon64.rom);
      // DON'T use fstate.st_size, because file could be compressed
      ucon64.rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
                                       ucon64.buheader_len : 0;
      ucon64.rominfo->interleaved = UCON64_ISSET (ucon64.interleaved) ?
                                       ucon64.interleaved : 0;
      if (genesis_init (ucon64.rominfo) != 0)
        printf ("WARNING: %s does not appear to be a Genesis ROM\n", ucon64.rom);
      /*
        NOTE: This is NOT the place to mess with ucon64.console. When this
              function is entered ucon64.console must have been UCON64_GEN. We
              modify ucon64.console temporarily only to be able to help detect
              problems with incorrect files.
      */

      if ((srcfile = fopen (ucon64.rom, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.rom);
          continue;
        }
      if (ucon64.rominfo->buheader_len)
        fseek (srcfile, ucon64.rominfo->buheader_len, SEEK_SET);

      if (file_no == 0)
        {
          printf ("Loader: %s\n", ucon64.rom);
          if (ucon64.file_size - ucon64.rominfo->buheader_len != MBIT)
            printf ("WARNING: Are you sure %s is a loader binary?\n", ucon64.rom);
        }
      else
        {
          printf ("ROM%d: %s\n", file_no, ucon64.rom);
          write_game_table_entry (destfile, file_no, ucon64.rominfo, totalsize);
          fseek (destfile, totalsize, SEEK_SET); // restore file pointer
        }

      done = 0;
      byteswritten = 0;                         // # of bytes written per file
      while (!done)
        {
          if (ucon64.rominfo->interleaved == 2)
            bytestowrite = fread_mgd (buffer, 1, BUFSIZE, srcfile);
          else
            {
              bytestowrite = fread (buffer, 1, BUFSIZE, srcfile);
              if (ucon64.rominfo->interleaved)
                smd_deinterleave (buffer, BUFSIZE);
                // yes, BUFSIZE. bytestowrite might not be n * 16 kB
            }
          if (totalsize + bytestowrite > truncate_size)
            {
              bytestowrite = truncate_size - totalsize;
              done = 1;
              truncated = 1;
              printf ("Output file is %d Mbit, truncating %s, skipping %d bytes\n",
                      truncate_size / MBIT, ucon64.rom, ucon64.file_size -
                        ucon64.rominfo->buheader_len - (byteswritten + bytestowrite));
            }
          totalsize += bytestowrite;
          if (bytestowrite == 0)
            done = 1;
          fwrite (buffer, 1, bytestowrite, destfile);
          byteswritten += bytestowrite;
        }
      fclose (srcfile);
      if (truncated)
        break;

      // games have to be aligned to (start at) a 2 Mbit boundary
      paddedsize = (totalsize + 2 * MBIT - 1) & ~(2 * MBIT - 1);
//      printf ("paddedsize: %d (%f); totalsize: %d (%f)\n",
//              paddedsize, paddedsize / (1.0 * MBIT), totalsize, totalsize / (1.0 * MBIT));
      if (paddedsize > totalsize)
        {
          // I (dbjh) don't think it is really necessary to pad to the truncate
          //  size, but it won't hurt
          if (paddedsize > truncate_size)
            {
              truncated = 1;                    // not *really* truncated
              paddedsize = truncate_size;
            }

          memset (buffer, 0, BUFSIZE);
          while (totalsize < paddedsize)
            {
              bytestowrite = paddedsize - totalsize > BUFSIZE ?
                              BUFSIZE : paddedsize - totalsize;
              fwrite (buffer, 1, bytestowrite, destfile);
              totalsize += bytestowrite;
            }
        }
      if (truncated)
        break;

      file_no++;
    }
  // fill the next game table entry
  fseek (destfile, 0x8000 + file_no * 0x20, SEEK_SET);
  fputc (0, destfile);                          // indicate no next game
  fclose (destfile);
  ucon64.console = UCON64_GEN;

  return 0;
}


static int
genesis_testinterleaved (st_rominfo_t *rominfo)
{
  unsigned char buf[16384];

  q_fread (buf, rominfo->buheader_len, 8192 + (GENESIS_HEADER_START + 4) / 2, ucon64.rom);
  if (!memcmp (buf + GENESIS_HEADER_START, "SEGA", 4))
    return 0;

  smd_deinterleave (buf, 16384);
  if (!memcmp (buf + GENESIS_HEADER_START, "SEGA", 4))
    return 1;
  
  q_fread_mgd (buf, rominfo->buheader_len + GENESIS_HEADER_START, 4, ucon64.rom);
  if (!memcmp (buf, "SEGA", 4))
    return 2;

  return 0;                                     // unknown, act as if it's BIN
}


int
genesis_init (st_rominfo_t *rominfo)
{
  int result = -1, value = 0, x, y;
  unsigned char *rom_buffer = NULL, buf[MAXBUFSIZE], name[GENESIS_NAME_LEN + 1];
  static char maker[9], country[200]; // 200 characters should be enough for 5 country names
  static const char *genesis_maker[0x100] = {
    NULL, "Accolade/Infogrames", "Virgin Games", "Parker Brothers", "Westone",
    NULL, NULL, NULL, NULL, "Westone",
    "Takara", "Taito/Accolade", "Capcom", "Data East", "Namco/Tengen",
    "Sunsoft", "Bandai", "Dempa", "Technosoft", "Technosoft",
    "Asmik", NULL, "Extreme/Micronet", "Vic Tokai", "American Sammy",
    "NCS", "Sigma Enterprises", "Toho", NULL, "Kyugo",
    NULL, NULL, "Wolfteam", "Kaneko", NULL,
    "Toaplan", "Tecmo", NULL, NULL, NULL,
    "Toaplan", "Unipac", "UFL Company Ltd.", "Human", NULL,
    "Game Arts", "Hot-B", "Sage's Creation", "Tengen/Time Warner", "Renovation/Telenet",
    "Electronic Arts", NULL, NULL, NULL, NULL,
    "Psygnosis", "Razorsoft", NULL, "Mentrix", NULL,
    "JVC/Victor Musical Industries", NULL, NULL, NULL, "IGS Corp.",
    NULL, NULL, "CRI/Home Data", "Arena", "Virgin Games",
    NULL, "Nichibutsu", NULL, "Soft Vision", "Palsoft",
    NULL, "KOEI", NULL, NULL, "U.S. Gold",
    NULL, "Acclaim/Flying Edge", NULL, "Gametek", NULL,
    NULL, "Absolute", "Mindscape", "Domark", "Parker Brothers",
    NULL, NULL, NULL, "Sony Imagesoft", "Sony Imagesoft",
    "Konami", NULL, "Tradewest/Williams", NULL, "Codemasters",
    "T*HQ Software", "TecMagik", NULL, "Takara", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, "Hi Tech Entertainment/Designer Software", "Psygnosis", NULL,
    NULL, NULL, NULL, NULL, "Accolade",
    "Code Masters", NULL, NULL, NULL, "Spectrum HoloByte",
    "Interplay", NULL, NULL, NULL, NULL,
    "Activision", NULL, "Shiny & Playmates", NULL, NULL,
    NULL, NULL, NULL, NULL, "Viacom International",
    NULL, NULL, NULL, NULL, "Atlus",
    NULL, NULL, NULL, NULL, NULL,
    NULL, "Infogrames", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, "Fox Interactive", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "Psygnosis",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, "Disney Interactive",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL},
#define GENESIS_COUNTRY_MAX 0x57
      *genesis_country[GENESIS_COUNTRY_MAX] = {
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, "Brazil", NULL, NULL,
    NULL, "Hong Kong", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    "Asia", "Brazil", NULL, NULL, "Europe",
    "France", NULL, NULL, NULL, "Japan",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    "U.S.A.", NULL},
#define GENESIS_IO_MAX 0x58
      *genesis_io[GENESIS_IO_MAX] = {
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, "Joystick for MS", NULL,
    NULL, NULL, "Team Play", NULL, "6 Button Pad",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, "Control Ball", "CD-ROM", NULL, NULL,
    "Floppy Disk Drive", NULL, NULL, NULL, "3 Button Pad",
    "Keyboard", "Activator", "Mega Mouse", NULL, NULL,
    "Printer", NULL, "Serial RS232C", NULL, "Tablet",
    NULL, "Paddle Controller", NULL};

  q_fread (&buf, 0, 11, ucon64.rom);
  if (buf[8] == 0xaa && buf[9] == 0xbb && buf[10] == 7)
    {
      rominfo->buheader_len = SMD_HEADER_LEN;
      strcpy (rominfo->name, "Name: N/A");
      rominfo->console_usage = NULL;
      rominfo->copier_usage = smd_usage;
      rominfo->maker = "Publisher: You?";
      rominfo->country = "Country: Your country?";
      rominfo->has_internal_crc = 0;
      strcat (rominfo->misc, "Type: Super Magic Drive SRAM file\n");
      ucon64.split = 0;                         // SRAM files are never split
      type = SMD;
      return 0;                                 // rest is nonsense for SRAM file
    }

  if (buf[8] == 0xaa && buf[9] == 0xbb && buf[10] == 6)
    {
      type = SMD;
      rominfo->buheader_len = SMD_HEADER_LEN;
    }

  if (UCON64_ISSET (ucon64.buheader_len))       // -hd, -nhd or -hdn option was specified
    rominfo->buheader_len = ucon64.buheader_len;

  rominfo->interleaved = (UCON64_ISSET (ucon64.interleaved)) ?
    ucon64.interleaved : genesis_testinterleaved (rominfo);

  if (rominfo->interleaved == 0)
    type = BIN;
  else if (rominfo->interleaved == 1)
    type = SMD;
  else if (rominfo->interleaved == 2)
    type = MGD_GEN;

  if (type == SMD)
    {
      genesis_rom_size = ((ucon64.file_size - rominfo->buheader_len) / 16384) * 16384;
      if (genesis_rom_size != ucon64.file_size - rominfo->buheader_len)
        rominfo->data_size = genesis_rom_size;

      q_fread (buf, rominfo->buheader_len,
        8192 + (GENESIS_HEADER_START + GENESIS_HEADER_LEN) / 2, ucon64.rom);
      smd_deinterleave (buf, 16384);            // buf will contain the deinterleaved data
      memcpy (&genesis_header, buf + GENESIS_HEADER_START, GENESIS_HEADER_LEN);
    }
  else if (type == MGD_GEN)
    {
      // We use rominfo->buheader_len to make it user controllable. Normally it
      //  should be 0 for MGD_GEN.
      genesis_rom_size = ucon64.file_size - rominfo->buheader_len;
      q_fread_mgd (&genesis_header, rominfo->buheader_len + GENESIS_HEADER_START,
        GENESIS_HEADER_LEN, ucon64.rom);
    }
  else // type == BIN
    {
      // We use rominfo->buheader_len to make it user controllable.
      genesis_rom_size = ucon64.file_size - rominfo->buheader_len;
      q_fread (&genesis_header, rominfo->buheader_len + GENESIS_HEADER_START,
        GENESIS_HEADER_LEN, ucon64.rom);
    }

  if (!memcmp (&OFFSET (genesis_header, 0), "SEGA", 4))
    result = 0;
  else
    result = -1;

  if (ucon64.console == UCON64_GEN)
    result = 0;

  rominfo->header_start = GENESIS_HEADER_START;
  rominfo->header_len = GENESIS_HEADER_LEN;
  rominfo->header = &genesis_header;

  // internal ROM name
  memcpy (rominfo->name, &OFFSET (genesis_header, 80), GENESIS_NAME_LEN);
  rominfo->name[GENESIS_NAME_LEN] = 0;

  // ROM maker
  memcpy (maker, &OFFSET (genesis_header, 16), 8);
  if (maker[3] == 'T' && maker[4] == '-')
    {
      sscanf (&maker[5], "%03d", &value);
      rominfo->maker = NULL_TO_UNKNOWN_S (genesis_maker[value & 0xff]);
    }
  else
    {
      // Don't use genesis_maker here. If it would be corrected an incorrect
      //  publisher name would be displayed.
      rominfo->maker =
        (!strncmp (maker, "(C)ACLD", 7)) ? "Ballistic" :
        (!strncmp (maker, "(C)AESI", 7)) ? "ASCII" :
        (!strncmp (maker, "(C)ASCI", 7)) ? "ASCII" :
        (!strncmp (maker, "(C)KANEKO", 9)) ? "Kaneko" :
        (!strncmp (maker, "(C)PPP", 6)) ? "Gametek" :
        (!strncmp (maker, "(C)RSI", 6)) ? "Razorsoft" : // or is it "(C)1RSI"?
        (!strncmp (maker, "(C)SEGA", 7)) ? "Sega" :
        (!strncmp (maker, "(C)TREC", 7)) ? "Treco" :
        (!strncmp (maker, "(C)VRGN", 7)) ? "Virgin Games" :
        (!strncmp (maker, "(C)WADN", 7)) ? "Parker Brothers" :
        (!strncmp (maker, "(C)WSTN", 7)) ? "Westone" : NULL;
      if (!rominfo->maker)
        {
          maker[8] = 0;
          rominfo->maker = maker;
        }
    }

  genesis_tv_standard = 1;              // default to PAL; NTSC has higher precedence
  country[0] = 0;
  // ROM country
  for (x = 0; x < 5; x++)
    {
      int country_code = OFFSET (genesis_header, 240 + x);

      if ((x > 0 && country_code == 0) || country_code == ' ')
        continue;
      if (country_code == 'J' || country_code == 'U') // Japan or U.S.A.
        genesis_tv_standard = 0;
      strcat (country, NULL_TO_UNKNOWN_S
               (genesis_country[MIN (country_code, GENESIS_COUNTRY_MAX - 1)]));
      strcat (country, ", ");
    }
  x = strlen (country);
  if (x >= 2 && country[x - 2] == ',' && country[x - 1] == ' ')
    country[x - 2] = 0;
  rominfo->country = country;

  // misc stuff
  memcpy (name, &OFFSET (genesis_header, 32), GENESIS_NAME_LEN);
  name[GENESIS_NAME_LEN] = 0;
  sprintf ((char *) buf, "Japanese game name: %s\n", name);
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "Date: %.8s\n", &OFFSET (genesis_header, 24));
  strcat (rominfo->misc, (char *) buf);

  x = (OFFSET (genesis_header, 160) << 24) +
      (OFFSET (genesis_header, 161) << 16) +
      (OFFSET (genesis_header, 162) << 8) +
       OFFSET (genesis_header, 163);
  y = (OFFSET (genesis_header, 164) << 24) +
      (OFFSET (genesis_header, 165) << 16) +
      (OFFSET (genesis_header, 166) << 8) +
       OFFSET (genesis_header, 167);
  sprintf ((char *) buf, "Internal size: %.4f Mb\n", (float) (y - x) / MBIT);
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "ROM start: %08x\n", x);
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "ROM end: %08x\n", y);
  strcat (rominfo->misc, (char *) buf);

  genesis_has_ram = OFFSET (genesis_header, 176) == 'R' &&
                    OFFSET (genesis_header, 177) == 'A';
  if (genesis_has_ram)
    {
      x = (OFFSET (genesis_header, 180) << 24) +
          (OFFSET (genesis_header, 181) << 16) +
          (OFFSET (genesis_header, 182) << 8) +
           OFFSET (genesis_header, 183);
      y = (OFFSET (genesis_header, 184) << 24) +
          (OFFSET (genesis_header, 185) << 16) +
          (OFFSET (genesis_header, 186) << 8) +
           OFFSET (genesis_header, 187);
      sprintf ((char *) buf, "Cartridge RAM: Yes, %d kBytes (%s)\n",
               y - x >> 10,
               OFFSET (genesis_header, 178) & 0x40 ? "backup" : "non-backup");
      strcat (rominfo->misc, (char *) buf);

      sprintf ((char *) buf, "RAM start: %08x\n", x);
      strcat (rominfo->misc, (char *) buf);

      sprintf ((char *) buf, "RAM end: %08x\n", y);
      strcat (rominfo->misc, (char *) buf);
    }
  else
    strcat (rominfo->misc, "Cartridge RAM: No\n");

#if 1
/*
  This code seems to give better results than the old code.
  "Officially" "GM" indicates it's a game and "Al" that it's educational.
*/
  sprintf ((char *) buf, "Product type: %s\n",
           (OFFSET (genesis_header, 128) == 'G') ? "Game" : "Education");
  strcat (rominfo->misc, (char *) buf);
#else
  if (OFFSET (genesis_header, 128) == 'G')
    {
      sprintf ((char *) buf, "ROM type: %s\n",
               (OFFSET (genesis_header, 129) == 'M') ? "Game" : "Education");
      strcat (rominfo->misc, (char *) buf);
    }
#endif

  sprintf ((char *) buf, "I/O device(s): %s",
    NULL_TO_UNKNOWN_S (genesis_io[MIN ((int) OFFSET (genesis_header, 144), GENESIS_IO_MAX - 1)]));
  for (x = 0; x < 3; x++)
    {
      const char *io_device = genesis_io[MIN (OFFSET (genesis_header, 145 + x), GENESIS_IO_MAX - 1)];
      if (!io_device)
        continue;
      strcat ((char *) buf, ", ");
      strcat ((char *) buf, io_device);
    }
  strcat ((char *) buf, "\n");
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "Modem data: %.10s\n", &OFFSET (genesis_header, 188));
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "Memo: %.40s\n", &OFFSET (genesis_header, 200));
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "Product code: %.8s\n", &OFFSET (genesis_header, 131));
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "Version: 1.%c%c", OFFSET (genesis_header, 140), OFFSET (genesis_header, 141));
  strcat (rominfo->misc, (char *) buf);

  // internal ROM crc
  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      if ((rom_buffer = load_rom (rominfo, ucon64.rom, rom_buffer)) == NULL)
        return -1;

      rominfo->has_internal_crc = 1;
      rominfo->internal_crc_len = 2;

      rominfo->current_internal_crc = genesis_chksum (rom_buffer);
      rominfo->internal_crc = OFFSET (genesis_header, 143);          // low byte of checksum
      rominfo->internal_crc += (OFFSET (genesis_header, 142)) << 8;  // high byte of checksum

      rominfo->internal_crc2[0] = 0;
      free (rom_buffer);
    }
  rominfo->console_usage = genesis_usage;
  if (type == SMD)
    rominfo->copier_usage = smd_usage;
  else if (type == MGD_GEN)
    rominfo->copier_usage = mgd_usage;
  else // type == BIN
    rominfo->copier_usage = bin_usage;

  return result;
}


int
genesis_chksum (unsigned char *rom_buffer)
{
  int i, len = genesis_rom_size - 2;
  unsigned short checksum = 0;

  for (i = 512; i <= len; i += 2)
    checksum += (rom_buffer[i] << 8) + (rom_buffer[i + 1]);

  return checksum;
}
