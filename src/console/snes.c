/*
snes.c - Super NES support for uCON64

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh
           2002 - 2003 John Weidman


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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "snes.h"
#include "backup/mgd.h"
#include "backup/gd.h"
#include "backup/swc.h"
#include "backup/fig.h"


#define SNES_HEADER_LEN (sizeof (st_snes_header_t))
#define SNES_HIROM 0x8000
#define SNES_NAME_LEN 21
#define ALT_HILO                                // use Snes9x' Hi/LoROM detection method
#define GD3_HEADER_MAPSIZE 0x18
#define NSRT_HEADER_VERSION 22                  // version 2.2 header

static int snes_chksum (st_rominfo_t *rominfo, unsigned char *rom_buffer);
static int snes_deinterleave (st_rominfo_t *rominfo, unsigned char *rom_buffer, int rom_size);
static int snes_convert_sramfile (const void *header);
static int snes_fix_pal_protection (st_rominfo_t *rominfo);
static int snes_fix_ntsc_protection (st_rominfo_t *rominfo);
static int get_internal_sums (st_rominfo_t *rominfo);
//static int snes_special_bs (void);
static int snes_bs_name(void);
static int snes_check_bs (void);
static int check_char (unsigned char c);
#ifdef  ALT_HILO
static int score_lorom (unsigned char *rom_buffer, int rom_size);
static int score_hirom (unsigned char *rom_buffer, int rom_size);
#endif


const char *snes_usage[] =
  {
    "Super Nintendo Entertainment System/SNES/Super Famicom",
    "1990 Nintendo http://www.nintendo.com",
    "  " OPTION_LONG_S "snes        force recognition"
    "\n"
    "  " OPTION_LONG_S "hi          force ROM is HiROM\n"
    "  " OPTION_LONG_S "nhi         force ROM is not HiROM\n"
#if 0
    "  " OPTION_LONG_S "hd          force ROM has SMC/FIG/SWC header (+512 Bytes)\n"
    "  " OPTION_LONG_S "nhd         force ROM has no SMC/FIG/SWC header (MGD2/MGH/RAW)\n"
    "  " OPTION_LONG_S "ns          force ROM is not split\n"
#endif
    "  " OPTION_LONG_S "int         force ROM is in interleaved format\n"
    "  " OPTION_LONG_S "int2        force ROM is in interleaved format 2\n"
    "  " OPTION_LONG_S "nint        force ROM is not in interleaved format\n"
    "  " OPTION_LONG_S "bs          force ROM is a Broadcast Satellaview dump\n"
    "  " OPTION_LONG_S "nbs         force ROM is a regular cartridge dump\n"
    "  " OPTION_S "n           change internal ROM name; " OPTION_LONG_S "file=NEWNAME\n"
    "  " OPTION_LONG_S "smc         convert to Super Magicom/SMC\n"
    "  " OPTION_LONG_S "fig         convert to *Pro Fighter*/(all)FIG\n"
    "  " OPTION_LONG_S "figs        convert Snes9x/ZSNES *.srm (SRAM) to *Pro Fighter*/(all)FIG;\n"
    "                  " OPTION_LONG_S "rom=SRAM\n"
    "  " OPTION_LONG_S "swc         convert to Super Wild Card*/(all)SWC\n"
    "  " OPTION_LONG_S "swcs        convert Snes9x/ZSNES *.srm (SRAM) to Super Wild Card*/(all)SWC;\n"
    "                  " OPTION_LONG_S "rom=SRAM\n"
    "  " OPTION_LONG_S "mgd         convert to Multi Game*/MGD2/MGH/RAW\n"
    "  " OPTION_LONG_S "gd3         convert to Professor SF(2) Game Doctor SF3/6/7\n"
    "  " OPTION_LONG_S "ufos        convert Snes9x/ZSNES *.srm (SRAM) to Super UFO; " OPTION_LONG_S "rom=SRAM\n"
    "  " OPTION_LONG_S "stp         convert SRAM from backup unit for use with an emulator\n"
    "                  " OPTION_LONG_S "stp just strips the first 512 bytes\n"
    "  " OPTION_LONG_S "dbuh        display (relevant part of) backup unit header\n"
    "  " OPTION_LONG_S "dint        convert ROM to non-interleaved format\n"
    "  " OPTION_LONG_S "ctrl=TYPE   specify type of controller in port 1 for emu when converting\n"
    "                  TYPE='0' gamepad\n"
    "                  TYPE='1' mouse\n"
    "                  TYPE='2' mouse / gamepad\n"
    "                  TYPE='6' multitap\n"
    "  " OPTION_LONG_S "ctrl2=TYPE  specify type of controller in port 2 for emu when converting\n"
    "                  TYPE='0' gamepad\n"
    "                  TYPE='1' mouse\n"
    "                  TYPE='2' mouse / gamepad\n"
    "                  TYPE='3' super scope\n"
    "                  TYPE='4' super scope / gamepad\n"
    "                  TYPE='5' Konami's justifier\n"
    "                  TYPE='6' multitap\n"
    "                  TYPE='7' mouse / super scope / gamepad\n"
    "  " OPTION_LONG_S "col         convert 0xRRGGBB (html) <-> 0xXXXX (SNES); " OPTION_LONG_S "rom=0xCOLOR\n"
    "                  this routine was used to find green colors in games and\n"
    "                  to replace them with red colors (blood mode)\n"
#if 0
    "TODO:  " OPTION_LONG_S "sx     convert to Snes9X (emulator)/S9X save state; " OPTION_LONG_S "rom=SAVESTATE\n"
    "TODO:  " OPTION_LONG_S "zs     convert to ZSNES (emulator) save state; " OPTION_LONG_S "rom=SAVESTATE\n"
    "TODO:  " OPTION_LONG_S "xzs    extract GFX from ZSNES (emulator) save state; " OPTION_LONG_S "rom=SAVESTATE\n"
    "TODO:  " OPTION_LONG_S "spc    convert SPC sound to WAV; " OPTION_LONG_S "rom=SPCFILE\n"
#endif
    "  " OPTION_S "j           join split ROM\n"
    "  " OPTION_S "s           split ROM into 8 Mb parts (for backup unit(s) with fdd)\n"
    "  " OPTION_LONG_S "ssize=SIZE  specify split part size in Mbit (not for Game Doctor SF3)\n"
#if 0
    "  " OPTION_S "p           pad ROM to full Mb\n"
#endif
    "  " OPTION_S "k           remove protection (crack)\n"
    "  " OPTION_S "f           remove NTSC/PAL protection\n"
    "  " OPTION_S "l           remove SlowROM checks\n"
    "  " OPTION_LONG_S "chk         fix ROM checksum\n"
    ,
    NULL
  };

typedef struct st_snes_header
{
  unsigned char maker_high;                     // 0
  unsigned char maker_low;                      // 1
  unsigned char game_id_prefix;                 // 2
  unsigned char game_id_low;                    // 3
  unsigned char game_id_high;                   // 4
  unsigned char game_id_country;                // 5
  // 'E' = USA, 'P' = Europe, 'F' = France, 'G' = Germany, 'S' = Spain
  unsigned char pad[10];                        // 6
  unsigned char name[SNES_NAME_LEN];            // 16
  unsigned char map_type;                       // 37, a.k.a. ROM makeup
  unsigned char rom_type;                       // 38
#define bs_month rom_type
  unsigned char size;                           // 39
  unsigned char sram_size;                      // 40
#define bs_makeup sram_size
  unsigned char country;                        // 41
#define bs_size country
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

st_snes_header_t snes_header;

static int snes_split, force_interleaved, bs_dump, rom_is_top,  // flag for interleaved ROM dump
           snes_sramsize, snes_hirom, snes_hirom_changed,       //  of "Tales of Phantasia"
           nsrt_header;
/*
  The flag `snes_hirom_changed' is necessary for ROMs that are in the normal
  interleaved format (type 1). The variable `snes_hirom' changes the first time
  when snes_deinterleave() is called from snes_chksum(). However, it should not
  change a second time. Without this flag it would change a second time when
  called from snes_dint().
*/

snes_copier_t type;

static unsigned char gd3_hirom_8mb_map[GD3_HEADER_MAPSIZE] = {
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22
};
static unsigned char gd3_hirom_16mb_map[GD3_HEADER_MAPSIZE] = {
  0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21,
  0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21,
  0x22, 0x23, 0x22, 0x23, 0x22, 0x23, 0x22, 0x23
};
static unsigned char gd3_hirom_24mb_map[GD3_HEADER_MAPSIZE] = {
  0x20, 0x21, 0x22, 0x00, 0x20, 0x21, 0x22, 0x00,
  0x20, 0x21, 0x22, 0x00, 0x20, 0x21, 0x22, 0x00,
  0x24, 0x25, 0x23, 0x00, 0x24, 0x25, 0x23, 0x00
};
static unsigned char gd3_hirom_32mb_map[GD3_HEADER_MAPSIZE] = {
  0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23,
  0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23,
  0x24, 0x25, 0x26, 0x27, 0x24, 0x25, 0x26, 0x27
};

static unsigned char gd3_lorom_4mb_map[GD3_HEADER_MAPSIZE] = {
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};
static unsigned char gd3_lorom_8mb_map[GD3_HEADER_MAPSIZE] = {
  0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21,
  0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21,
  0x20, 0x21, 0x20, 0x21, 0x20, 0x21, 0x20, 0x21
};
static unsigned char gd3_lorom_16mb_map[GD3_HEADER_MAPSIZE] = {
  0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23,
  0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23,
  0x20, 0x21, 0x22, 0x23, 0x20, 0x21, 0x22, 0x23
};
static unsigned char gd3_lorom_32mb_map[GD3_HEADER_MAPSIZE] = {
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
  return type;
}


int
snes_dint (st_rominfo_t *rominfo)
{
  st_unknown_header_t header;
  FILE *srcfile, *destfile;
  unsigned char *buffer;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  int size = rominfo->file_size - rominfo->buheader_len, success = 1;

  puts ("Converting to deinterleaved format...");
  if (!(buffer = (unsigned char *) malloc (size)))
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".TMP");
  strcpy (src_name, ucon64.rom);
  handle_existing_file (dest_name, src_name);
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

  if (rominfo->buheader_len)
    {
      if (!fread (&header, SWC_HEADER_LEN, 1, srcfile))
        success = 0;
      fseek (srcfile, rominfo->buheader_len, SEEK_SET);
    }
  if (!fread (buffer, size, 1, srcfile))
    success = 0;
  if (!success)
    {
      fprintf (stderr, "ERROR: Can't read from %s\n", ucon64.rom);
      free (buffer);
      fclose (srcfile);
      fclose (destfile);
      return -1;
    }

  force_interleaved = 1;                        // force snes_deinterleave() to do its work
  snes_deinterleave (rominfo, buffer, size);

  if (rominfo->buheader_len)
    fwrite (&header, 1, SWC_HEADER_LEN, destfile);
  fwrite (buffer, size, 1, destfile);

  free (buffer);
  fclose (srcfile);
  fclose (destfile);

  fprintf (stdout, ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}


int
snes_col (st_rominfo_t *rominfo)
{
/*
The Nintendo Super Famicom is capable of displaying 256 colours from a
palette of 32,768. These 256 colours are split into 8 palettes of 32 colours
each.

To change the colours the following needs to be done:

Loading the palette control register ($2121) with the colour number you wish
to change (0-255, 0=background).
Then load the colour into the palette data register first the low 8 bits,
followed by the high 7 bits (this gives you the maximum 32768 colours
possible $0000-$7fff).

Colour data is made up of 3 components (Red,Green,Blue) each of 5 bits (The
Amiga uses exactly the same system, but only using 4 bits per component).
Saying that, Nintendo being the stupid japanese idiots they are decided that
R,G,B wasn't alphabetically correct and so opted to store the bits as B,G,R.

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
  int r, g, b;
  unsigned int col;

  sscanf (ucon64.rom, "%x", &col);

  r = (col & 0xff0000) / 0x10000;
  g = (col & 0xff00) / 0x100;
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

  sscanf (ucon64.rom, "%x", &col);

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
  printf ("\n");
  return 0;
}


int
snes_convert_sramfile (const void *header)
{
  FILE *srcfile, *destfile;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], buf[32 * 1024];
  int blocksize, byteswritten;

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".SAV");
  handle_existing_file (dest_name, src_name);
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

  fwrite (header, 1, SWC_HEADER_LEN, destfile); // write header
  byteswritten = SWC_HEADER_LEN;

  blocksize = fread (buf, 1, 32 * 1024, srcfile); // read 32 kB at max
  while (byteswritten < 32 * 1024 + SWC_HEADER_LEN)
    {                                           // pad sram to 32.5 kB by
      fwrite (buf, 1, blocksize, destfile);     //  repeating the SRAM data
      byteswritten += blocksize;
    }

  fclose (srcfile);
  fclose (destfile);
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  return 0;
}


int
snes_swcs (st_rominfo_t *rominfo)
{
  st_swc_header_t header;

  memset (&header, 0, SWC_HEADER_LEN);
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 5;                              // size doesn't need to be set for the SWC

  return snes_convert_sramfile (&header);
}


int
snes_figs (st_rominfo_t *rominfo)
{
  st_fig_header_t header;

  memset (&header, 0, FIG_HEADER_LEN);
  header.size_low = 4;                          // 32 kB == 4*8 kB, size_high is already 0

  return snes_convert_sramfile (&header);
}


int
snes_ufos (st_rominfo_t *rominfo)
{
  unsigned char header[SWC_HEADER_LEN];

  memset (&header, 0, SWC_HEADER_LEN);
  memcpy (&header[8], "SUPERUFO", 8);

  return snes_convert_sramfile (&header);
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
set_nsrt_info (st_rominfo_t *rominfo, unsigned char *header)
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

  if ((UCON64_ISSET (ucon64.controller) || UCON64_ISSET (ucon64.controller2))
      && !nsrt_header)                          // don't overwrite these values
    {
      if (rominfo->current_internal_crc != rominfo->internal_crc)
        {
          printf ("WARNING: The controller type info will be discarded (checksum is bad)\n");
          return;
        }

      header[0x1d0] = bs_dump ? 0 : snes_header.country;
      if (rominfo->header_start == SNES_HEADER_START + SNES_HIROM + 0x400000)
        header[0x1d0] |= 0x30;
      else
        header[0x1d0] |= snes_hirom ? 0x20 : 0x10;

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
          printf ("WARNING: Invalid value for controller in port 1, using \"0\"\n");
          x = 0;
        }
      header[0x1ed] = x << 4;
    }
  if (UCON64_ISSET (ucon64.controller2))
    {
      for (x = 0; x < 8; x++)
        if ((ucon64.controller2 >> x) & 1)
          break;
      if (x >= 8)
        {
          printf ("WARNING: Invalid value for controller in port 2, using \"0\"\n");
          x = 0;
        }
      header[0x1ed] |= x;
    }

  // set the checksum bytes
  if (UCON64_ISSET (ucon64.controller) || UCON64_ISSET (ucon64.controller2))
    set_nsrt_checksum (header);
}


static void
reset_header (void *header)
{
  // preserve possible NSRT header
  if (nsrt_header)
    {
      memset (header, 0, 0x1d0);
      memset (header + 0x1f0, 0, 16);
      ((unsigned char *) header)[0x1ec] = NSRT_HEADER_VERSION;
      set_nsrt_checksum (header);
    }
  else
    memset (header, 0, SWC_HEADER_LEN);
}


static int
snes_ffe (st_rominfo_t *rominfo, char *ext)
{
  st_swc_header_t header;
  int size;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  q_fread (&header, 0, rominfo->buheader_len, ucon64.rom);
  reset_header (&header);
  size = rominfo->file_size - rominfo->buheader_len;
  header.size_low = size / 8192;
  header.size_high = size / 8192 >> 8;

  header.emulation = snes_split ? 0x40 : 0;
  header.emulation |= snes_hirom ? 0x30 : 0;
  // bit 3 & 2 are already ok for 32 kB SRAM size
  if (snes_sramsize == 8 * 1024)
    header.emulation |= 0x04;
  else if (snes_sramsize == 2 * 1024)
    header.emulation |= 0x08;
  else if (snes_sramsize == 0)
    header.emulation |= 0x0c;

  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 4;

  set_nsrt_info (rominfo, (unsigned char *) &header);

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ext);
  strcpy (src_name, ucon64.rom);
  handle_existing_file (dest_name, src_name);
  q_fwrite (&header, 0, SWC_HEADER_LEN, dest_name, "wb");
  q_fcpy (src_name, rominfo->buheader_len, size, dest_name, "ab");
  fprintf (stdout, ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}


// header format is specified in src/backup/ffe.h
int
snes_smc (st_rominfo_t *rominfo)
{
  if (snes_header.map_type & 0x10)
    printf ("NOTE: This game might not work with a Super Magicom because it's a FastROM game\n");

  return snes_ffe (rominfo, ".SMC");
}


// header format is specified in src/backup/ffe.h
int
snes_swc (st_rominfo_t *rominfo)
{
  return snes_ffe (rominfo, ".SWC");
}


// header format is specified in src/backup/fig.h
int
snes_fig (st_rominfo_t *rominfo)
{
  int size, uses_DSP;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  st_fig_header_t header;

  q_fread (&header, 0, rominfo->buheader_len, ucon64.rom);
  reset_header (&header);
  size = rominfo->file_size - rominfo->buheader_len;
  header.size_low = size / 8192;
  header.size_high = size / 8192 >> 8;
  header.multi = snes_split ? 0x40 : 0;
  header.hirom = snes_hirom ? 0x80 : 0;

  uses_DSP = snes_header.rom_type == 3 || snes_header.rom_type == 4 ||
             snes_header.rom_type == 5 || snes_header.rom_type == 0xf6;

  if ((snes_header.rom_type & 0xf0) == 0x10)    // uses FX(2) chip
    {
      header.emulation1 = 0x11;
      header.emulation2 = 2;
    }
  else
    {
#if 0                                           // memset() set all fields to 0
      header.emulation1 = 0;                    // default value for LoROM dumps
      if (snes_sramsize == 32 * 1024)
        header.emulation2 = 0;
      else
#endif
      if (snes_sramsize == 8 * 1024 || snes_sramsize == 2 * 1024)
        header.emulation2 = 0x80;
      else if (snes_sramsize == 0)
        {
          header.emulation1 = 0x77;
          header.emulation2 = 0x83;
        }

      if (snes_hirom)
        {
          header.emulation2 |= 2;
          if (uses_DSP)
            header.emulation1 |= 0xf0;
          if (snes_sramsize != 0)
            header.emulation1 |= 0xdd;
        }
      else if (uses_DSP)                        // LoROM
        {
          header.emulation1 &= 0x0f;
          header.emulation1 |= 0x40;            // LoROM && SRAM == 0 && DSP => 0x47
        }
    }

  set_nsrt_info (rominfo, (unsigned char *) &header);

  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".FIG");
  strcpy (src_name, ucon64.rom);
  handle_existing_file (dest_name, src_name);
  q_fwrite (&header, 0, FIG_HEADER_LEN, dest_name, "wb");
  q_fcpy (src_name, rominfo->buheader_len, size, dest_name, "ab");
  fprintf (stdout, ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}


/*
The MGD2 only accepts certain filenames, and these filenames
must be specified in an index file, "MULTI-GD", otherwise the
MGD2 will not recognize the file. In the case of multiple games
being stored in a single disk, simply enter its corresponding
MULTI-GD index into the "MULTI-GD" file.

game size       # of files      names           MULTI-GD
================================================================
4M              1               SF4XXX.048      SF4XXX
4M              2               SF4XXXxA.078    SF4XXXxA
                                SF4XXXxB.078    SF4XXXxB
8M              1               SF8XXX.058      SF8XXX
                2               SF8XXXxA.078    SF8XXXxA
                                SF8XXXxB.078    SF8xxxxB
16M             2               SF16XXXA.078    SF16XXXA
                                SF16XXXB.078    SF16XXXB
24M             3               SF24XXXA.078    SF24XXXA
                                SF24XXXB.078    SF24XXXB
                                SF24XXXC.078    SF24XXXC

Contrary to popular belief the Game Doctor *does* use a 512
byte header like the SWC, but it also accepts headerless files.
A header is necessary when things like SRAM size must be made
known to the Game Doctor. The Game Doctor also uses specially
designed filenames to distinguish between multi files.

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

e.g. The first 16 Mbit file of Donkey Kong Country (assuming it
is cat. no. 475) would look like: SF16475A.078
*/
int
snes_mgd (st_rominfo_t *rominfo)
// What should we do with this function? snes_gd3() is probably sufficient
//  (and tested on a real Game Doctor!).
{
#if 1
  char mgh[32], dest_name[FILENAME_MAX], *fname;
  int n, len;

  fname = basename (ucon64.rom);
  sprintf (dest_name, "%s%d", areupper (fname) ? "SF" : "sf",
    (rominfo->file_size - rominfo->buheader_len) / MBIT);
  strncat (dest_name, fname, 5);
  dest_name[8] = 0;

  len = strlen (dest_name);
  for (n = 0; n < len; n++)
    if (dest_name[n] == ' ')
      dest_name[n] = '_';

  // What is the format of this MULTI-GD file?
  memset (mgh, 0, sizeof (mgh));
  mgh[0] = 'M';
  mgh[1] = 'G';
  mgh[2] = 'H';
  mgh[3] = 0x1a;
  mgh[31] = 0xff;
  memcpy (&mgh[16], dest_name, strlen (dest_name));

  setext (dest_name, ".078");
  ucon64_fbackup (NULL, dest_name);
  q_fcpy (ucon64.rom, rominfo->buheader_len, rominfo->file_size, dest_name, "wb");
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  strcpy (dest_name, "MULTI-GD.MGH");
  q_fwrite (&mgh, 0, sizeof (mgh), dest_name, "wb");
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  return 0;
#else
  char mgh[32], buf[FILENAME_MAX], buf2[FILENAME_MAX], *p = NULL;

  strcpy (buf, areupper (basename (ucon64.rom)) ? "SF" : "sf");
  strcpy (buf2, ucon64.rom);
  strcat (buf, basename (buf2));
  if ((p = strrchr (buf, '.')))
    *p = 0;
  strcat (buf, "________");
  buf[7] = '_';
  buf[8] = 0;
  sprintf (buf2, "%s.%03u", buf, (rominfo->file_size - rominfo->buheader_len) / MBIT);

  ucon64_fbackup (NULL, buf2);
  q_fcpy (ucon64.rom, rominfo->buheader_len, rominfo->file_size, buf2, "wb");
  fprintf (stdout, ucon64_msg[WROTE], buf2);

  // create MGH name file
  memset (mgh, 0, sizeof (mgh));
  mgh[0] = 'M';
  mgh[1] = 'G';
  mgh[2] = 'H';
  mgh[3] = 0x1a;
  mgh[31] = 0xff;
  memcpy (&mgh[16], snes_header.name, 15);

  strcpy (buf, buf2);
  setext (buf, ".MGH");

  q_fwrite (&mgh, 0, sizeof (mgh), buf, "wb");
  fprintf (stdout, ucon64_msg[WROTE], buf);

  return 0;
#endif
}


void
snes_int_blocks (unsigned char *deintptr, unsigned char *ipl,
                 unsigned char *iph, int blocks)
{
  int i;

  // interleave 64K blocks
  for (i = blocks; i > 0; i--)
    {
      memmove (ipl, deintptr, 0x8000);
      memmove (iph, deintptr + 0x8000, 0x8000);
      deintptr += 0x10000;
      ipl += 0x8000;
      iph += 0x8000;
    }
  return;
}


void
snes_mirror (unsigned char *dstbuf, unsigned start, unsigned data_end,
             unsigned mirror_end)
{
  int datasize, totsize;
  int num_chunks, surplus;

  datasize = data_end - start;
  totsize = mirror_end - start;

  if (datasize >= totsize)
    return;

  num_chunks = totsize / datasize - 1;
  surplus = totsize % datasize;
  while (num_chunks-- > 0)
    {
      memcpy (dstbuf + data_end, dstbuf + start, datasize);
      data_end += datasize;
    }
  if (surplus > 0)
    memmove (dstbuf + data_end, dstbuf + start, mirror_end - data_end);
}


int
snes_gd3 (st_rominfo_t *rominfo)
{
  char header[512], dest_name[FILENAME_MAX], *p = NULL;
  unsigned char *srcbuf, *dstbuf;
  int n, len, n4Mbparts, surplus4Mb, total4Mbparts, size, newsize, pad;

  if (rominfo->interleaved)
    {
      fprintf (stderr, "ERROR: This ROM seems to be interleaved\n");
      return -1;
    }

  size = rominfo->file_size - rominfo->buheader_len;
  n4Mbparts = size / (4 * MBIT);
  surplus4Mb = size % (4 * MBIT);
  total4Mbparts = n4Mbparts + (surplus4Mb > 0 ? 1 : 0);

  p = basename (ucon64.rom);
  sprintf (dest_name, "%s%d", areupper (p) ? "SF" : "sf", total4Mbparts * 4);
  strcat (dest_name, p);
  // avoid trouble with filenames containing spaces
  len = strlen (dest_name);
  for (n = 0; n < len; n++)
    if (dest_name[n] == ' ')
      dest_name[n] = '_';
  if ((p = strrchr (dest_name, '.')))
    *p = 0;
  strcat (dest_name, "________");
  if (total4Mbparts < 3)
    dest_name[6] = 0;
  else
    dest_name[7] = 0;

  if (snes_hirom)
    {
      if (total4Mbparts > 8)
        {
          fprintf (stderr, "ERROR: This ROM > 32 Mbit -- conversion not yet implemented\n");
          return -1;
        }
      if (n4Mbparts < 1)
        {
          fprintf (stderr, "ERROR: This ROM < 4 Mbit -- conversion not yet implemented\n");
          return -1;
        }

      // create the header
      q_fread (header, 0, rominfo->buheader_len, ucon64.rom);
      reset_header (header);
      memcpy (header, "GAME DOCTOR SF 3", 0x10);

      if (snes_sramsize == 8 * 1024)
        header[0x10] = 0x81;                    // 64 kb
      else if (snes_sramsize == 2 * 1024)
        header[0x10] = 0x82;                    // 16 kb
      else
        header[0x10] = 0x80;                    // 0 kb or 256 kb

      if (total4Mbparts == 5)
        total4Mbparts = 6;                      // 20 Mbit HiROMs get padded to 24 Mbit

      if (total4Mbparts <= 2)
        memcpy (&header[0x11], gd3_hirom_8mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 4)
        memcpy (&header[0x11], gd3_hirom_16mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 6)
        memcpy (&header[0x11], gd3_hirom_24mb_map, GD3_HEADER_MAPSIZE);
      else
        memcpy (&header[0x11], gd3_hirom_32mb_map, GD3_HEADER_MAPSIZE);

      if (snes_sramsize != 0)
        {
          header[0x29] = 0x0c;
          header[0x2a] = 0x0c;
        }

      // Adjust sram map for exceptions - a couple of 10-12 Mb HiROM games
      //  (Liberty or Death, Brandish). May not be necessary

      // interleave the image
      newsize = 4 * total4Mbparts * MBIT;
      pad = (newsize - size) / 2;

      if (!(srcbuf = (unsigned char *) malloc (size)))
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], (int) size);
          exit (1);
        }
      if (!(dstbuf = (unsigned char *) malloc (newsize)))
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], (int) newsize);
          exit (1);
        }

      q_fread (srcbuf, rominfo->buheader_len, size, ucon64.rom);
      memset (dstbuf, 0, newsize);

      if (total4Mbparts != 6)
        {
          n = newsize / 2;
          snes_int_blocks (srcbuf, &dstbuf[n], dstbuf, size / 0x10000);
          if (pad > 0)
            {
              snes_mirror (dstbuf, (size / 2) & ~0x7ffff,
                           (size / 2 + 0x1ffff) & ~0x1ffff, newsize / 2);
              snes_mirror (dstbuf, n + (size / 2 & ~0x7ffff),
                           n + ((size / 2 + 0x1ffff) & ~0x1ffff), newsize);
            }
        }
      else
        {
          snes_int_blocks (srcbuf, &dstbuf[16 * MBIT], dstbuf,
                           16 * MBIT / 0x10000);
          snes_int_blocks (&srcbuf[16 * MBIT], &dstbuf[12 * MBIT],
                           &dstbuf[8 * MBIT], (size - 16 * MBIT) / 0x10000);
          if (size <= 20 * MBIT)
            {
              snes_mirror (dstbuf, 8 * MBIT, 10 * MBIT, 12 * MBIT);
              snes_mirror (dstbuf, 12 * MBIT, 14 * MBIT, 16 * MBIT);
            }
        }

      set_nsrt_info (rominfo, (unsigned char *) &header);

      ucon64_fbackup (NULL, dest_name);
      q_fwrite (header, 0, SWC_HEADER_LEN, dest_name, "wb");
      q_fwrite (dstbuf, SWC_HEADER_LEN, newsize, dest_name, "ab");
      fprintf (stdout, ucon64_msg[WROTE], dest_name);

      free (srcbuf);
      free (dstbuf);

      rominfo->buheader_len = SWC_HEADER_LEN;
      rominfo->interleaved = 1;
    }
  else
    {
      if (total4Mbparts > 8)
        {
          fprintf (stderr, "ERROR: This ROM > 32 Mbit LoROM -- can't convert\n");
          return -1;
        }

      q_fread (header, 0, rominfo->buheader_len, ucon64.rom);
      reset_header (header);
      memcpy (header, "GAME DOCTOR SF 3", 0x10);

      if (snes_sramsize == 8 * 1024)
        header[0x10] = 0x81;                    // 64 kb
      else if (snes_sramsize == 2 * 1024)
        header[0x10] = 0x82;                    // 16 kb
      else
        header[0x10] = 0x80;                    // 0 kb or 256 kb

      if (total4Mbparts <= 1)
        memcpy (&header[0x11], gd3_lorom_4mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 2)
        memcpy (&header[0x11], gd3_lorom_8mb_map, GD3_HEADER_MAPSIZE);
      else if (total4Mbparts <= 4)
        memcpy (&header[0x11], gd3_lorom_16mb_map, GD3_HEADER_MAPSIZE);
      else
        memcpy (&header[0x11], gd3_lorom_32mb_map, GD3_HEADER_MAPSIZE);

      if (snes_sramsize != 0)
        {
          header[0x24] = 0x40;
          header[0x28] = 0x40;
        }

      set_nsrt_info (rominfo, (unsigned char *) &header);

      ucon64_fbackup (NULL, dest_name);
      q_fwrite (header, 0, SWC_HEADER_LEN, dest_name, "wb");
      q_fcpy (ucon64.rom, rominfo->buheader_len, size, dest_name, "ab");
      fprintf (stdout, ucon64_msg[WROTE], dest_name);

      rominfo->buheader_len = SWC_HEADER_LEN;
      rominfo->interleaved = 1;
    }

  return 0;
}


int
snes_make_gd_names (const char *filename, st_rominfo_t *rominfo, char **names)
// This function assumes file with name filename is in GD3 format
{
  char dest_name[FILENAME_MAX], *p = NULL;
  int nparts, surplus, x, sf_romname, size, n_names = 0, len;

  size = rominfo->file_size - rominfo->buheader_len;
  sf_romname = (ucon64.rom[0] == 'S' || ucon64.rom[0] == 's') &&
               (ucon64.rom[1] == 'F' || ucon64.rom[1] == 'f');

  // Don't use PARTSIZE here, because the Game Doctor doesn't support
  //  arbitrary part sizes
  nparts = size / (8 * MBIT);
  surplus = size % (8 * MBIT);

  if (sf_romname)
    strcpy (dest_name, basename (ucon64.rom));
  else
    {
      strcpy (dest_name, "SF");
      strcat (dest_name, basename (ucon64.rom));
    }
  strupr (dest_name);
  if ((p = strrchr (dest_name, '.')))
    *p = 0;
  strcat (dest_name, "______");
  dest_name[7] = 'A';
  dest_name[8] = 0;
  len = strlen (dest_name);

  if (snes_hirom && ((nparts + ((surplus > 0) ? 1 : 0)) <= 2))
    {
      // 8 Mbit or less HiROMs, X is used to pad filename to 8 (SF4###XA)
      if (size < 10 * MBIT)
        dest_name[len - 2] = 'X';
      strcpy (names[n_names++], dest_name);

      dest_name[len - 1]++;
      strcpy (names[n_names++], dest_name);
    }
  else
    {
      for (x = 0; x < nparts; x++)
        {
          strcpy (names[n_names++], dest_name);
          dest_name[len - 1]++;
        }
      if (surplus != 0)
        strcpy (names[n_names++], dest_name);
    }
  return n_names;
}


#define PARTSIZE  (8 * MBIT)                    // default split part size
int
snes_s (st_rominfo_t *rominfo)
{
  char header[512], dest_name[FILENAME_MAX], *names[GD3_MAX_UNITS],
       names_mem[GD3_MAX_UNITS][12];
  int nparts, surplus, x, half_size, size, name_i = 0, part_size;

  size = rominfo->file_size - rominfo->buheader_len;

  if (UCON64_ISSET (ucon64.part_size))
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
        We ignore the few ROMs that are greater than 32 MBit.
      */
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

  if (!rominfo->buheader_len || type == GD3)    // GD3 format
    {
      if (UCON64_ISSET (ucon64.part_size))
        printf (
          "WARNING: ROM will be split as Game Doctor SF3 ROM, ignoring switch "OPTION_LONG_S"ssize\n");

      // Don't use part_size here, because the Game Doctor doesn't support
      //  arbitrary part sizes
      nparts = size / (8 * MBIT);
      surplus = size % (8 * MBIT);

      // We don't want to malloc() ridiculously small chunks (of 12 bytes)
      for (x = 0; x < GD3_MAX_UNITS; x++)
        names[x] = names_mem[x];
      snes_make_gd_names (ucon64.rom, rominfo, (char **) names);

      if (snes_hirom && ((nparts + ((surplus > 0) ? 1 : 0)) <= 2))
        {
          half_size = size / 2;

          sprintf (dest_name, "%s.078", names[name_i++]);
          // don't write backups of parts, because one name is used
          q_fcpy (ucon64.rom, 0, half_size + rominfo->buheader_len, dest_name, "wb");
          fprintf (stdout, ucon64_msg[WROTE], dest_name);

          sprintf (dest_name, "%s.078", names[name_i++]);
          q_fcpy (ucon64.rom, half_size + rominfo->buheader_len, size - half_size, dest_name, "wb");
          fprintf (stdout, ucon64_msg[WROTE], dest_name);
        }
      else
        {
          for (x = 0; x < nparts; x++)
            {
              // don't write backups of parts, because one name is used
              sprintf (dest_name, "%s.078", names[name_i++]);
              q_fcpy (ucon64.rom, x * 8 * MBIT + (x ? rominfo->buheader_len : 0),
                        8 * MBIT + (x ? 0 : rominfo->buheader_len), dest_name, "wb");
              fprintf (stdout, ucon64_msg[WROTE], dest_name);
            }

          if (surplus != 0)
            {
              // don't write backups of parts, because one name is used
              sprintf (dest_name, "%s.078", names[name_i++]);
              q_fcpy (ucon64.rom, x * 8 * MBIT + (x ? rominfo->buheader_len : 0),
                        surplus + (x ? 0 : rominfo->buheader_len), dest_name, "wb");
              fprintf (stdout, ucon64_msg[WROTE], dest_name);
            }
        }
      return 0;
    }
  else
    {
      nparts = size / part_size;
      surplus = size % part_size;

      strcpy (dest_name, ucon64.rom);
      setext (dest_name, ".1");

      q_fread (header, 0, SWC_HEADER_LEN, ucon64.rom);
      header[0] = part_size / 8192;
      header[1] = part_size / 8192 >> 8;
      // if header[2], bit 6 == 0 -> SWC/FIG knows this is the last file of the ROM
      header[2] |= 0x40;
      for (x = 0; x < nparts; x++)
        {
          if (surplus == 0 && x == nparts - 1)
            header[2] &= ~0x40;                 // last file -> clear bit 6

          // don't write backups of parts, because one name is used
          q_fwrite (header, 0, SWC_HEADER_LEN, dest_name, "wb");
          q_fcpy (ucon64.rom, x * part_size + rominfo->buheader_len, part_size, dest_name, "ab");
          fprintf (stdout, ucon64_msg[WROTE], dest_name);

          (*(strrchr (dest_name, '.') + 1))++;
        }

      if (surplus != 0)
        {
          header[0] = surplus / 8192;
          header[1] = surplus / 8192 >> 8;
          header[2] &= ~0x40;                   // last file -> clear bit 6

          // don't write backups of parts, because one name is used
          q_fwrite (header, 0, SWC_HEADER_LEN, dest_name, "wb");
          q_fcpy (ucon64.rom, x * part_size + rominfo->buheader_len, surplus, dest_name, "ab");
          fprintf (stdout, ucon64_msg[WROTE], dest_name);
        }

      return 0;
    }
}


int
snes_j (st_rominfo_t *rominfo)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], *p = NULL;
  int block_size, total_size = 0, header_len = rominfo->buheader_len;

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  setext (dest_name, ".TMP");

  ucon64_fbackup (NULL, dest_name);
  q_fcpy (src_name, 0, rominfo->buheader_len, dest_name, "wb"); // copy header (if any)
  block_size = q_fsize (src_name) - header_len;
  // Split GD3 files don't have a header _except_ the first one
  while (q_fcpy (src_name, header_len, block_size, dest_name, "ab") != -1)
    {
      printf ("Joined: %s\n", src_name);
      total_size += block_size;
      p = strrchr (src_name, '.');
      if (p == NULL)                            // filename didn't contain a period
        p = src_name + strlen (src_name) - 1;
      else
        type == GD3 ? p-- : p++;
      (*p)++;

      if (type == GD3)
        header_len = 0;
      block_size = q_fsize (src_name) - header_len;
    }

  if (rominfo->buheader_len && type != GD3)
    {                                           // fix header
      q_fputc (dest_name, 0, total_size / 8192, "r+b"); // # 8K blocks low byte
      q_fputc (dest_name, 1, total_size / 8192 >> 8, "r+b"); // # 8K blocks high byte
      q_fputc (dest_name, 2, q_fgetc (dest_name, 2) & ~0x40, "r+b"); // last file -> clear bit 6
    }
  fprintf (stdout, ucon64_msg[WROTE], dest_name);

  return 0;
}


int
snes_k (st_rominfo_t *rominfo)
{
/*
See the document "src/backup/SWC-compatibility.txt".
Don't touch this code if you don't know what you're doing!

Some SNES games check to see how much SRAM is connected to the SNES as a form
of copy protection. As most copiers have 256 kbits standard, the game will
know it's running on a backup unit and stop to prevent people copying the
games. However, newer copiers like the SWC DX2 get around this detection by
limiting the SRAM size for the game to the size specified in the backup unit
header.

(original uCON)
   8f/9f XX YY 70 cf/df XX YY 70 d0
=> 8f/9f XX YY 70 cf/df XX YY 70 ea ea          if snes_sramsize == 64 kbits
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

- Uniracers/Unirally
   8f XX YY 77 e2 XX af XX YY 77 c9 XX f0
=> 8f XX YY 77 e2 XX af XX YY 77 c9 XX 80

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
*/
  char header[512], buffer[32 * 1024], src_name[FILENAME_MAX];
  FILE *srcfile, *destfile;
  int bytesread;

  puts ("Attempting crack...");

  strcpy (src_name, ucon64.rom);
  handle_existing_file (ucon64.rom, src_name);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (ucon64.rom, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], ucon64.rom);
      return -1;
    }
  if (rominfo->buheader_len)                    // copy header (if present)
    {
      fread (header, 1, SWC_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->buheader_len, SEEK_SET);
      fwrite (header, 1, SWC_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)))
    {                                           // '!' == ASCII 33 (\x21), '*' == 42 (\x2a)
      // SRAM
      if (snes_sramsize == 8 * 1024)            // 8 kB == 64 kb
        {
          // unknown
          change_string ("!**\x70!**\x70\xd0", 9, '*', '!', "\xea\xea", 2, buffer, bytesread, 0,
                         "\x8f\x9f", 2, "\xcf\xdf", 2);
          // actually Kirby's Dream Course, Lufia II - Rise of the Sinistrals
          change_string ("!**\x70!**\x70\xf0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0,
                         "\x8f\x9f", 2, "\xcf\xdf", 2);
          // actually Earthbound
          change_string ("!**!!**!\xf0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0,
                         "\x8f\x9f", 2, "\x30\x31\x32\x33", 4, "\xcf\xdf", 2, "\x30\x31\x32\x33", 4);
        }
      else
        {
          // unknown
          change_string ("!**\x70!**\x70\xd0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0,
                         "\x8f\x9f", 2, "\xcf\xdf", 2);
          // Mega Man X
          change_string ("!**\x70!**\x70\xf0", 9, '*', '!', "\xea\xea", 2, buffer, bytesread, 0,
                         "\x8f\x9f", 2, "\xcf\xdf", 2);

          change_string ("!**!!**!\xf0", 9, '*', '!', "\xea\xea", 2, buffer, bytesread, 0,
                         "\x8f\x9f", 2, "\x30\x31\x32\x33", 4, "\xcf\xdf", 2, "\x30\x31\x32\x33", 4);
        }

      change_string ("\x8f**\x77\xe2*\xaf**\x77\xc9*\xf0", 13, '*', '!', "\x80", 1, buffer, bytesread, 0);
      change_string ("!!!!!!\x60!\xd0", 9, '*', '!', "\xea\xea", 2, buffer, bytesread, 0,
                     "\x8f\x9f", 2, "\x57\x59", 2, "\x60\x68", 2, "\x30\x31\x32\x33", 4,
                     "\xcf\xdf", 2, "\x57\x59", 2, "\x30\x31\x32\x33", 4);

      change_string ("!**!!**!\xd0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0,
                     "\x8f\x9f", 2, "\x30\x31\x32\x33", 4, "\xcf\xdf", 2, "\x30\x31\x32\x33", 4);
      change_string ("!**!\xaf**!\xc9**\xd0", 12, '*', '!', "\x80", 1, buffer, bytesread, 0,
                     "\x8f\x9f", 2, "\x30\x31\x32\x33", 4, "\x30\x31\x32\x33", 4);
      change_string ("\xa9\x00\x00\xa2\xfe\x1f\xdf\x00\x00\x70\xd0", 11, '*', '!', "\xea\xea", 2, buffer, bytesread, 0);
      change_string ("!**!!**!\xf0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0,
                     "\xaf\xbf", 2, "\x30\x31\x32\x33", 4, "\xcf\xdf", 2, "\x30\x31\x32\x33", 4);

      // mirroring
      change_string ("!*\x80\x00!*\x80\x40\xf0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0,
                     "\xaf\xbf", 2, "\xcf\xdf", 2);
      change_string ("!*\xff!!*\xff\x40\xf0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0,
                     "\xaf\xbf", 2, "\x80\xc0", 2, "\xcf\xdf", 2);

      // game specific
      change_string ("\x5c\x7f\xd0\x83\x18\xfb\x78\xc2\x30", 9, '*', '!',
                     "\xea\xea\xea\xea\xea\xea\xea\xea\xea", 9, buffer, bytesread, -8);

      change_string ("KONG\x00\xf8\xf7", 7, '*', '!', "\xf8", 1, buffer, bytesread, 0);
      change_string ("\x26\x38\xe9\x48\x12\xc9\xaf\x71\xf0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xa0\x5c\x2f\x77\x32\xe9\xc7\x04\xf0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0);

      change_string ("\x22\x08\x5c\x10\xb0\x28", 6, '*', '!',
                     "\xea\xea\xea\xea\xea\xea", 6, buffer, bytesread, -5);
      change_string ("\xda\xe2\x30\xc9\x01\xf0\x18\xc9\x02", 9, '*', '!',
                                     "\x09\xf0\x18\xc9\x07", 5, buffer, bytesread, -4);
      change_string ("\x29\xff\x00\xc9\x07\x00\x90\x16", 8, '*', '!', "\x00", 1, buffer, bytesread, -3);

      change_string ("\xca\x10\xf8\x38\xef\x1a\x80\x81\x8d", 9, '*', '!', "\x9c", 1, buffer, bytesread, 0);
      change_string ("\x81\xca\x10\xf8\xcf\x39\x80\x87\xf0", 9, '*', '!', "\x80", 1, buffer, bytesread, 0);

      change_string ("\x84\x26\xad\x39\xb5\xd0\x1a", 7, '*', '!', "\xea\xea", 2, buffer, bytesread, -1);
      change_string ("\x10\xf8\x38\xef\xef\xff\xc1", 7, '*', '!',
                                 "\xea\xa9\x00\x00", 4, buffer, bytesread, -3);
      change_string ("\x10\xf8\x38\xef\xf2\xfd\xc3\xf0", 8, '*', '!',
                                 "\xea\xa9\x00\x00\x80", 5, buffer, bytesread, -4);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);

  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);
  remove_temp_file ();

  return 0;
}


int
snes_fix_pal_protection (st_rominfo_t *rominfo)
{
/*
This function searches for PAL protection codes. If it finds one it will
fix the code so that the game will run on an NTSC SNES.
Don't touch this code if you don't know what you're doing!

Search for                            Replace with
ad 3f 21 89 10 d0                     ad 3f 21 89 10 80                - Terranigma
ad 3f 21 29 10 00 d0                  ad 3f 21 29 10 00 80
ad 3f 21 89 10 00 d0                  a9 10 00 89 10 00 d0             - Eric Cantona Football ?
ad 3f 21 29 10 cf bd ff 00 f0         ad 3f 21 29 10 cf bd ff 00 80    - Tiny Toons - Wild and Wacky Sports ?
af 3f 21 00 29 10 d0                  af 3f 21 00 29 10 80
af 3f 21 00 29 10 00 d0               af 3f 21 00 29 10 00 ea ea
af 3f 21 00 29 XX c9 XX f0            af 3f 21 00 29 XX c9 XX 80       - Secret of Mana E
a2 18 01 bd 27 20 89 10 00 f0 01      a2 18 01 bd 27 20 89 10 00 ea ea - Donkey Kong Country E
*/
  char header[512], buffer[32 * 1024], src_name[FILENAME_MAX];
  FILE *srcfile, *destfile;
  int bytesread;

  puts ("Attempting to fix PAL protection code...");

  strcpy (src_name, ucon64.rom);
  handle_existing_file (ucon64.rom, src_name);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (ucon64.rom, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], ucon64.rom);
      return -1;
    }
  if (rominfo->buheader_len)                    // copy header (if present)
    {
      fread (header, 1, SWC_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->buheader_len, SEEK_SET);
      fwrite (header, 1, SWC_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)))
    {
      change_string ("\xad\x3f\x21\x89\x10\xd0", 6, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xad\x3f\x21\x29\x10\x00\xd0", 7, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xad\x3f\x21\x89\x10\x00\xd0", 7, '\x01', '\x02', "\xa9\x10\x00", 3, buffer, bytesread, -6);
      change_string ("\xad\x3f\x21\x29\x10\xcf\xbd\xff\x00\xf0", 10, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xaf\x3f\x21\x00\x29\x10\xd0", 7, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xaf\x3f\x21\x00\x29\x10\x00\xd0", 8, '\x01', '\x02', "\xea\xea", 2, buffer, bytesread, 0);
      change_string ("\xaf\x3f\x21\x00\x29\x01\xc9\x01\xf0", 9, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xa2\x18\x01\xbd\x27\x20\x89\x10\x00\xf0\x01", 11, '*', '!', "\xea\xea", 2, buffer, bytesread, -1);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);

  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);
  remove_temp_file ();

  return 0;
}


int
snes_fix_ntsc_protection (st_rominfo_t *rominfo)
{
/*
This function searches for NTSC protection codes. If it finds one it will
fix the code so that the game will run on a PAL SNES.
Don't touch this code if you don't know what you're doing!

Search for                            Replace with
3f 21 29/89 10 f0                     3f 21 29/89 10 80
ad 3f 21 29 10 d0                     ad 3f 21 29 10 ea ea
ad 3f 21 89 10 d0                     ad 3f 21 89 10 80/(ea ea)        - Live-a-Live (ea ea)
3f 21 29/89 10 00 f0                  3f 21 29/89 10 00 80
   3f 21 89 10 c2 XX d0                  3f 21 89 10 c2 XX ea ea       - Robotrek
3f 21 29/89 10 c9 10 f0               3f 21 29/89 10 c9 10 80
ad 3f 21 29 10 c9 00 f0               ad 3f 21 29 10 c9 00 80/(ea ea) <= original uCON used 80
ad 3f 21 29 10 c9 00 d0               ad 3f 21 29 10 c9 00 80
ad 3f 21 29 10 c9 10 d0               ad 3f 21 29 10 c9 10 ea ea
   3f 21 29 10 cf XX YY 80 f0            3f 21 29 10 cf XX YY 80 80    - Gokujyou Parodius/Tokimeki Memorial
ad 3f 21 8d XX YY 29 10 8d            ad 3f 21 8d XX YY 29 00 8d       - Dragon Ball Z - Super Butoden 2 ?
3f 21 00 29/89 10 f0                  3f 21 00 29/89 10 80             - Kirby's Dream Course U (29)
af 3f 21 00 29/89 10 d0               af 3f 21 00 89 10 ea ea          - Kirby No Kira Kizzu/Final Fight Guy
af 3f 21 00 29/89 10 00 f0            af 3f 21 00 29/89 10 00 80
af 3f 21 00 29 XX c9 XX f0            af 3f 21 00 29 XX c9 XX 80       - Seiken Densetsu 3
af 3f 21 00 29 10 80 2d 00 1b         af 3f 21 00 29 00 80 2d 00 1b    - Seiken Densetsu 2/Secret of Mana U
af 3f 21 00 XX YY 29 10 00 d0         af 3f 21 00 XX YY 29 10 00 ea ea - Fatal Fury Special ?
af 3f 21 ea 89 10 00 d0               a9 00 00 ea 89 10 00 d0          - Super Famista 3 ?
a2 18 01 bd 27 20 89 10 00 d0 01      a2 18 01 bd 27 20 89 10 00 ea ea - Donkey Kong Country U
*/
  char header[512], buffer[32 * 1024], src_name[FILENAME_MAX];
  FILE *srcfile, *destfile;
  int bytesread;

  puts ("Attempting to fix NTSC protection code...");

  strcpy (src_name, ucon64.rom);
  handle_existing_file (ucon64.rom, src_name);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (ucon64.rom, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], ucon64.rom);
      return -1;
    }
  if (rominfo->buheader_len)                    // copy header (if present)
    {
      fread (header, 1, SWC_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->buheader_len, SEEK_SET);
      fwrite (header, 1, SWC_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)))
    {
      change_string ("\x3f\x21\x02\x10\xf0", 5, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0,
                     "\x29\x89", 2);
      change_string ("\xad\x3f\x21\x29\x10\xd0", 6, '\x01', '\x02', "\xea\xea", 2, buffer, bytesread, 0);

      if (snes_sramsize == 8 * 1024)            // actually Live-a-Live
        change_string ("\xad\x3f\x21\x89\x10\xd0", 6, '\x01', '\x02', "\xea\xea", 2, buffer, bytesread, 0);
      else
        change_string ("\xad\x3f\x21\x89\x10\xd0", 6, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);

      change_string ("\x3f\x21\x02\x10\x00\xf0", 6, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0,
                     "\x29\x89", 2);
      change_string ("\x3f\x21\x89\x10\xc2\x01\xd0", 7, '\x01', '\x02', "\xea\xea", 2, buffer, bytesread, 0);
      change_string ("\x3f\x21\x02\x10\xc9\x10\xf0", 7, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0,
                     "\x29\x89", 2);
      change_string ("\xad\x3f\x21\x29\x10\xc9\x00\xf0", 8, '\x01', '\x02', "\xea\xea", 2, buffer, bytesread, 0);
      change_string ("\xad\x3f\x21\x29\x10\xc9\x00\xd0", 8, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xad\x3f\x21\x29\x10\xc9\x10\xd0", 8, '\x01', '\x02', "\xea\xea", 2, buffer, bytesread, 0);
      change_string ("\x3f\x21\x29\x10\xcf\x01\x01\x80\xf0", 9, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xad\x3f\x21\x8d\x01\x01\x29\x10\x8d", 9, '\x01', '\x02', "\x00", 1, buffer, bytesread, -1);
      change_string ("\x3f\x21\x00\x02\x10\xf0", 6, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0,
                     "\x29\x89", 2);
      change_string ("\xaf\x3f\x21\x00\x02\x10\xd0", 7, '\x01', '\x02', "\xea\xea", 2, buffer, bytesread, 0,
                     "\x29\x89", 2);
      change_string ("\xaf\x3f\x21\x00\x02\x10\x00\xf0", 8, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0,
                     "\x29\x89", 2);
      change_string ("\xaf\x3f\x21\x00\x29\x01\xc9\x01\xf0", 9, '\x01', '\x02', "\x80", 1, buffer, bytesread, 0);
      change_string ("\xaf\x3f\x21\x00\x29\x10\x80\x2d\x00\x1b", 10, '\x01', '\x02', "\x00", 1, buffer, bytesread, -4);
      change_string ("\xaf\x3f\x21\x00\x01\x01\x29\x10\x00\xd0", 10, '\x01', '\x02', "\xea\xea", 2, buffer, bytesread, 0);
      change_string ("\xaf\x3f\x21\xea\x89\x10\x00\xd0", 8, '\x01', '\x02', "\xa9\x00\x00", 3, buffer, bytesread, -7);
      change_string ("\xa2\x18\x01\xbd\x27\x20\x89\x10\x00\xd0\x01", 11, '*', '!', "\xea\xea", 2, buffer, bytesread, -1);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);

  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);
  remove_temp_file ();

  return 0;
}


int
snes_f (st_rominfo_t *rominfo)
{
// See the document "src/backup/NTSC-PAL notes.txt".
  switch (snes_header.country)
    {
      // In the Philipines the television standard is NTSC, but do games made
      //  for the Philipines exist?
      case 0:                                   // Japan
      case 1:                                   // U.S.A.
        return snes_fix_ntsc_protection (rominfo);
      default:
        return snes_fix_pal_protection (rominfo);
    }
}


int
snes_l (st_rominfo_t *rominfo)
{
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
  char header[512], buffer[32 * 1024], src_name[FILENAME_MAX];
  FILE *srcfile, *destfile;
  int bytesread;

  puts ("Attempting SlowROM fix...");

  strcpy (src_name, ucon64.rom);
  handle_existing_file (ucon64.rom, src_name);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (ucon64.rom, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], ucon64.rom);
      return -1;
    }
  if (rominfo->buheader_len)                    // copy header (if present)
    {
      fread (header, 1, SWC_HEADER_LEN, srcfile);
      fseek (srcfile, rominfo->buheader_len, SEEK_SET);
      fwrite (header, 1, SWC_HEADER_LEN, destfile);
    }

  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)))
    {                                           // '!' == ASCII 33 (\x21), '*' == 42 (\x2a)
      change_string ("!\x0d\x42", 3, '*', '!', "\x9c", 1, buffer, bytesread, -2,
                     "\x8c\x8d\x8e\x8f", 4);
      change_string ("\x01\x0d\x42", 3, '*', '!', "\x00", 1, buffer, bytesread, -2);
      change_string ("\xa9\x01\x85\x0d", 4, '*', '!', "\x00", 1, buffer, bytesread, -2);
      change_string ("\xa2\x01\x86\x0d", 4, '*', '!', "\x00", 1, buffer, bytesread, -2);
      change_string ("\xa0\x01\x84\x0d", 4, '*', '!', "\x00", 1, buffer, bytesread, -2);

      // original uCON
      change_string ("!\x01!\x0d\x42", 5, '*', '!', "\x00", 1, buffer, bytesread, -3,
                     "\xa9\xa2", 2, "\x8d\x8e", 2);
      change_string ("\xa9\x01\x00\x8d\x0d\x42", 6, '*', '!', "\x00", 1, buffer, bytesread, -4);
      change_string ("\xa9\x01\x8f\x0d\x42\x00", 6, '*', '!', "\x00", 1, buffer, bytesread, -4);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);

  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);
  remove_temp_file ();

  return 0;
}


int
snes_n (st_rominfo_t *rominfo)
{
  char buf[SNES_NAME_LEN];

  memset (buf, ' ', SNES_NAME_LEN);
  strncpy (buf, ucon64.file, strlen (ucon64.file) > SNES_NAME_LEN ?
           SNES_NAME_LEN : strlen (ucon64.file));
  ucon64_fbackup (NULL, ucon64.rom);
  q_fwrite (buf, rominfo->header_start + rominfo->buheader_len + 16, SNES_NAME_LEN,
            ucon64.rom, "r+b");

  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);

  return 0;
}


int
snes_chk (st_rominfo_t *rominfo)
{
  char buf[10];
  int image = rominfo->header_start + rominfo->buheader_len;

  ucon64_fbackup (NULL, ucon64.rom);

  // change inverse checksum
  q_fputc (ucon64.rom, image + 44, (0xffff - rominfo->current_internal_crc), "r+b");      // low byte
  q_fputc (ucon64.rom, image + 45, (0xffff - rominfo->current_internal_crc) >> 8, "r+b"); // high byte
  // change checksum
  q_fputc (ucon64.rom, image + 46, rominfo->current_internal_crc, "r+b");      // low byte
  q_fputc (ucon64.rom, image + 47, rominfo->current_internal_crc >> 8, "r+b"); // high byte

  q_fread (buf, image + 44, 4, ucon64.rom);
  mem_hexdump (buf, 4, image + 44);

  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);

  return 0;
}


static int
snes_testinterleaved (st_rominfo_t *rominfo)
{
  int interleaved = 0;
  char name[22];                                // NOT SNES_NAME_LEN

  if (snes_hirom)
    {
      if ((snes_header.map_type & 0xf0) == 0x20 || (snes_header.map_type & 0xf0) == 0x30)
        {
          switch (snes_header.map_type & 0xf)
            {
            case 0:
            case 3:
              interleaved = 1;
              break;
            }
        }
    }
  else
    {
      if ((snes_header.map_type & 0xf0) == 0x20 || (snes_header.map_type & 0xf0) == 0x30)
        {
          switch (snes_header.map_type & 0xf)
            {
            case 1:
              if (strncmp (rominfo->name, "TREASURE HUNTER G", 17) != 0)
                interleaved = 1;
              break;
            case 5:
              interleaved = 1;
              rom_is_top = 1;
              break;
            }
        }

      q_fread (name, SNES_HEADER_START + rominfo->buheader_len + 16, 22, ucon64.rom);
      if (!strncmp (name, "YUYU NO QUIZ DE GO!GO!", 22))
        interleaved = 0;
      else if (!strncmp (name, "SP MOMOTAROU DENTETSU2", 22))
        interleaved = 0;
    }

  return interleaved;
}


int
snes_deinterleave (st_rominfo_t *rominfo, unsigned char *rom_buffer, int rom_size)
{
  unsigned char blocks[256], tmp[0x8000], b;
  int nblocks, i, j, org_hirom;
#ifdef  ALT_HILO
  int hi_score, lo_score;
#endif

  org_hirom = snes_hirom;
  nblocks = rom_size >> 16;                     // # 64 kB blocks
  if (nblocks * 2 > 256)
    return -1;                                  // file > 8 MB

  if (0)
    /*
       "if (rom_is_top)" does not seem to work for interleaved ToP, but this does!
       This code comes from Snes9x, but for Snes9x applies the same...
    */
    {
      nblocks = 0x60;
      for (i = 0; i < 0x40; i += 2)
        {
          blocks[i + 0] = (i >> 1) + 0x20;
          blocks[i + 1] = (i >> 1);
        }
      for (i = 0; i < 0x80; i += 2)
        {
          blocks[i + 0x40] = (i >> 1) + 0x80;
          blocks[i + 0x41] = (i >> 1) + 0x40;
        }
      snes_hirom = SNES_HIROM;
    }
  else if (rominfo->interleaved == 2)
    {
      for (i = 0; i < nblocks * 2; i++)
        blocks[i] = (i & ~0x1e) | ((i & 2) << 2) | ((i & 4) << 2) |
          ((i & 8) >> 2) | ((i & 16) >> 2);
    }
  else
    {
      if (!snes_hirom_changed)
        {
          snes_hirom = snes_hirom ? 0 : SNES_HIROM;
          snes_hirom_changed = 1;
        }
      if (snes_hirom && type == GD3 && rom_size == 24 * MBIT)
        {
          // Fix-up the weird 24 Mbit Game Doctor HiROM format
          unsigned char *p1, *p2, *p3;

          p1 = &rom_buffer[0x180000];
          p2 = &rom_buffer[0x200000];
          p3 = &rom_buffer[0x280000];
          for (; p1 < &rom_buffer[0x200000]; p1 += 0x8000, p2 += 0x8000, p3 += 0x8000)
            {
              memmove (tmp, p1, 0x8000);
              memmove (p1, p2, 0x8000);
              memmove (p2, p3, 0x8000);
              memmove (p3, tmp, 0x8000);
            }
        }
      for (i = 0; i < nblocks; i++)
        {
          blocks[i * 2] = i + nblocks;
          blocks[i * 2 + 1] = i;
        }
    }

  for (i = 0; i < nblocks * 2; i++)
    {
      for (j = i; j < nblocks * 2; j++)
        {
          if (blocks[j] == i)
            {
              memmove (tmp, &rom_buffer[blocks[j] * 0x8000], 0x8000);
              memmove (&rom_buffer[blocks[j] * 0x8000],
                       &rom_buffer[blocks[i] * 0x8000], 0x8000);
              memmove (&rom_buffer[blocks[i] * 0x8000], tmp, 0x8000);
              b = blocks[j];
              blocks[j] = blocks[i];
              blocks[i] = b;
              break;
            }
        }
    }

#ifdef  ALT_HILO
  hi_score = score_hirom (rom_buffer, rom_size);
  lo_score = score_lorom (rom_buffer, rom_size);

  if (!force_interleaved &&
       ((snes_hirom && (lo_score >= hi_score || hi_score < 0)) ||
         (!snes_hirom && (hi_score > lo_score || lo_score < 0))))
    {                                           // ROM seems to be non-interleaved after all
      q_fread (rom_buffer, rominfo->buheader_len, rom_size, ucon64.rom);
      rominfo->interleaved = 0;
      snes_hirom = org_hirom;
      return -1;
    }
#endif

  return 0;
}


static char *
matches_deviates (int equal)
{
  return
#ifdef  ANSI_COLOR
    ucon64.ansi_color ?
      (equal ? "\x1b[01;32mmatches\x1b[0m" : "\x1b[01;33mdeviates\x1b[0m") :
      (equal ? "matches" : "deviates");
#else
      (equal ? "matches" : "deviates");
#endif
}


int
snes_buheader_info (st_rominfo_t *rominfo)
{
  unsigned char header[512];
  int x, y;
  snes_copier_t org_type = type;

  if (rominfo->buheader_len == 0) // type == MGD
    {
      printf ("This ROM has no backup unit header\n");
      return -1;
    }
  else
    {
      printf ("Backup unit header info (%s)\n\n",
        type == SWC ? "SWC" :
        type == FIG ? "FIG" :
        type == GD3 ? "GD3" :
        "unknown header type, but interpreted as SWC");
      if (type == SMC)
        type = SWC;
    }

  q_fread (&header, 0, rominfo->buheader_len, ucon64.rom);
  mem_hexdump (header, 48, 0);                  // show only the part that is
  puts ("");                                    //  interpreted by copier

  if (type == SWC || type == FIG || type == SMC)
    {
      x = rominfo->file_size - rominfo->buheader_len;
      y = (header[0] + (header[1] << 8)) * 8 * 1024;
      printf ("[0-1]   File size: %d Bytes (%.4f Mb) => %s\n",
        y, TOMBIT_F (y), matches_deviates (x == y));
    }

  if (type == SWC || type == SMC)
    {
      unsigned char sram_sizes[] = {0, 2, 8, 32};

      y = header[2] & 0x01;
      printf ("[2:0]   (Reserved): %d\n", y);
      if (y)
        printf ("WARNING: [2:0] is set while it is a reserved bit\n");

      printf ("[2:1]   External cartridge memory: %s\n",
        header[2] & 0x02 ? "Enabled" : "Disabled");

      y = sram_sizes[(~header[2] & 0x0c) >> 2]; // 32 => 12, 8 => 8, 2 => 4, 0 => 0
      printf ("[2:2-3] SRAM size: %d kB => %s\n",
        y , matches_deviates (snes_sramsize == y * 1024));

      x = snes_hirom ? 1 : 0;
      y = header[2] & 0x10 ? 1 : 0;
      printf ("[2:4]   DRAM mapping mode: %s => %s\n",
        y ? "HiROM" : "LoROM", matches_deviates (x == y));

      y = header[2] & 0x20 ? 1 : 0;
      printf ("[2:5]   SRAM mapping mode: %s => %s\n",
        y ? "HiROM" : "LoROM", matches_deviates (x == y));

      y = header[2] & 0x40 ? 1 : 0;
      printf ("[2:6]   Split: %s => %s\n",
        y ? "Yes" : "No", matches_deviates ((snes_split ? 1 : 0) == y));

      printf ("[2:7]   Run program in mode: %d\n", (header[2] & 0x80) >> 7);
    }
  else if (type == FIG)
    {
      y = header[2] & 0x40 ? 1 : 0;
      printf ("[2]     Split: %s => %s\n",
        y ? "Yes" : "No", matches_deviates ((snes_split ? 1 : 0) == y));

      y = header[3] & 0x80 ? 1 : 0;
      printf ("[3]     Memory mapping mode: %s => %s\n",
        y ? "HiROM" : "LoROM", matches_deviates ((snes_hirom ? 1 : 0) == y));

      y = -1;
      if (snes_hirom)
        {
          if ((header[4] == 0x77 && header[5] == 0x83) ||
              (header[4] == 0xf7 && header[5] == 0x83))
            y = 0;
          else if (header[4] == 0xfd && header[5] == 0x82)
            y = 2 * 1024;
          else if (header[4] == 0xdd && header[5] == 0x82)
            y = 8 * 1024; // or 2 * 1024
          else if (header[4] == 0xdd && header[5] == 0x02)
            y = 32 * 1024;
        }
      else
        {
          if ((header[4] == 0x77 && header[5] == 0x83) ||
              (header[4] == 0x47 && header[5] == 0x83) ||
              (header[4] == 0x11 && header[5] == 0x02))
            y = 0;
          else if (header[4] == 0x00 && header[5] == 0x80)
            y = 8 * 1024; // or 2 * 1024
          else if (header[4] == 0x00 && header[5] == 0x00)
            y = 32 * 1024;
        }

      if (y == 8 * 1024)
        printf ("[4-5]   SRAM size: 2 kB / 8 kB => %s\n",
          matches_deviates (snes_sramsize == 2 * 1024 ||
                            snes_sramsize == 8 * 1024));
      else
        printf ("[4-5]   SRAM size: %d kB => %s\n",
          y / 1024, matches_deviates (snes_sramsize == y));
    }

  if (type == GD3)
    {
      y = -1;
      if (header[0x10] == 0x81)
        y = 8 * 1024;
      else if (header[0x10] == 0x82)
        y = 2 * 1024;
      else if (header[0x10] == 0x80)
        y = 32 * 1024; // or 0
      if (y == 32 * 1024)
        printf ("[10]    SRAM size: 0 kB / 32 kB => %s\n",
          matches_deviates (snes_sramsize == 0 || snes_sramsize == 32 * 1024));
      else
        printf ("[10]    SRAM size: %d kB => %s\n",
          y / 1024, matches_deviates (snes_sramsize == y));
    }

  type = org_type;

  return 0;
}


int
get_internal_sums (st_rominfo_t *rominfo)
/*
  Returns the sum of the internal checksum and the internal inverse checksum
  if the values for snes_hirom and rominfo->buheader_len are correct. If the
  values are correct the sum will be 0xffff. Note that the sum for bad ROM
  dumps can also be 0xffff, because this function adds the internal checksum
  bytes and doesn't do anything with the real, i.e. calculated, checksum.
*/
{
  int image = SNES_HEADER_START + snes_hirom + rominfo->buheader_len;
  // don't use rominfo->header_start here!
  unsigned short int internal_sums =
                   q_fgetc (ucon64.rom, image + 44);      // low byte
  internal_sums += q_fgetc (ucon64.rom, image + 45) << 8; // high byte
  internal_sums += q_fgetc (ucon64.rom, image + 46);      // low byte
  internal_sums += q_fgetc (ucon64.rom, image + 47) << 8; // high byte

  return internal_sums;
}


int
snes_init (st_rominfo_t *rominfo)
{
  int x, y, size, result = -1;                  // it's no SNES ROM dump until detected otherwise
  unsigned char *rom_buffer;
  st_unknown_header_t header;
  char buf[MAXBUFSIZE], *str;
  static const char *snes_maker[0x100] = {
    NULL, "Nintendo", NULL, "Imagineer-Zoom", NULL,
    "Zamuse", "Falcom", NULL, "Capcom", "HOT-B",
    "Jaleco", "Coconuts", "Rage Software", NULL, "Technos",
    "Mebio Software", NULL, "Starfish", "Gremlin Graphics", "Electronic Arts",
    NULL, "COBRA Team", "Human/Field", "KOEI", "Hudson Soft",
    "Game Village", "Yanoman", NULL, "Tecmo", NULL,
    "Open System", "Virgin Games", "KSS", "Sunsoft", "POW",
    "Micro World", NULL, NULL, "Enix", "Loriciel/Electro Brain",
    "Kemco", "Seta Co., Ltd.", NULL, NULL, NULL,
    "Visit Co., Ltd.", NULL, NULL, "Viacom New Media", "Carrozzeria",
    "Dynamic", "Nintendo", "Magifact", "Hect", NULL,
    NULL, "Capcom", NULL, NULL, "Arcade Zone",
    "Empire Software", "Loriciel", NULL, NULL, "Seika Corp.",
    "UBI SOFT Entertainment Software", NULL, NULL, NULL, NULL,
    "System 3", "Spectrum Holobyte", NULL, "Irem", NULL,
    "Raya Systems/Sculptured Software", "Renovation Products", "Malibu Games/Black Pearl",
      NULL, "U.S. Gold",
    "Absolute Entertainment", "Acclaim/LJN Ltd.", "Activision", "American Sammy",
      "Time Warner Interactive/Bitmasters" /*"GameTek"*/,
    "Hi Tech Expressions", "LJN Ltd.", NULL, NULL, NULL,
    "Mindscape", NULL, NULL, "Tradewest", NULL,
    "American Softworks Corp.", "Titus", "Virgin Interactive Entertainment",
      "Maxis", "Origin/FCI/Pony Canyon",
    NULL, NULL, NULL, "Ocean", NULL,
    "Electronic Arts", NULL, "Laser Beam", NULL, NULL,
    "Elite", "Electro Brain", "Infogrames", "Interplay", "LucasArts Entertainment",
    "Parker Brothers", "Konami", "STORM", NULL, NULL,
    "THQ Software", "Accolade Inc.", "Triffix Entertainment", NULL, "Microprose",
    NULL, NULL, "Kemco", "Misawa", "Teichio",
    "Namco Ltd.", "Lozc", "KOEI", NULL, "Tokuma Shoten Intermedia",
    NULL, "DATAM-Polystar", NULL, NULL, "Bullet-Proof Software",
    "Vic Tokai", NULL, "Character Soft", "I'Max", "Takara",
    "CHUN Soft", "Video System Co., Ltd.", "BEC", NULL, "Varie",
    NULL, "Kaneco", NULL, "Pack In Video", "Nichibutsu",
    "Tecmo", "Imagineer", NULL, NULL, NULL,
    "Telenet", "Hori", NULL, NULL, "Konami",
    "K. Amusement Leasing Co.", NULL, "Takara", NULL, "Technos",
    "JVC", NULL, "Toei Animation", "Toho", NULL,
    "Namco Ltd.", NULL, "ASCII Co./Activision", "BanDai America", NULL,
    "Enix", NULL, "Halken", NULL, NULL,
    NULL, "Culture Brain", "Sunsoft", "Toshiba EMI", "Sony/Imagesoft",
    NULL, "Sammy", "Taito", NULL, "Kemco",
    "Square", "Tokuma Soft", "Data East", "Tonkin House", NULL,
    "Falcom" /*"KOEI"*/, NULL, "Konami USA", "NTVIC", NULL,
    "Meldac", "Origin/FCI/Pony Canyon", "Angel/Sotsu Agency/Sunrise", "Disco/Taito",
      "Sofel",
    "Quest Corp.", "Sigma", NULL, NULL, "Naxat",
    NULL, "Capcom", "Banpresto", "Tomy", "Acclaim",
    NULL, "NCS", "Human Entertainment", "Altron", "Jaleco",
    NULL, "Yutaka", NULL, "T&Esoft", "Epoch Co., Ltd.",
    NULL, "Athena", "Asmik", "Natsume", "King Records",
    "Atlus", "Sony Music Entertainment", NULL, "IGS", NULL,
    NULL, "Motown Software", "Left Field Entertainment",
      "Beam Software/Extreme Ent. Grp.", "Tec Magik",
    NULL, NULL, NULL, NULL, "Cybersoft",
    NULL, NULL, NULL, NULL, "Davidson",
    "Hudson Soft"},
#define SNES_COUNTRY_MAX 0xe
      *snes_country[SNES_COUNTRY_MAX] = {
    "Japan",
    "U.S.A.",
    "Europe, Oceania and Asia",                 // Australia is part of Oceania
    "Sweden",
    "Finland",
    "Denmark",
    "France",
    "Holland",
    "Spain",
    "Germany, Austria and Switzerland",
    "Italy",
    "Hong Kong and China",
    "Indonesia",
    "South Korea"},
      *snes_romtype[3] = {
    "ROM",                                      // NOT ROM only, ROM + other chip is possible
    "ROM and RAM",
    "ROM and Save RAM"};

  rom_is_top = 0;                               // init these vars here, for -lsv
  snes_hirom_changed = 0;                       // idem
  snes_sramsize = 0;                            // idem
  type = SMC;                                   // idem, SMC indicates unknown copier type

  q_fread (&header, UNKNOWN_HEADER_START, UNKNOWN_HEADER_LEN, ucon64.rom);
  if (header.id1 == 0xaa && header.id2 == 0xbb && header.type == 5)
    {
      rominfo->buheader_len = SWC_HEADER_LEN;
      strcpy (rominfo->name, "Name: N/A");
      rominfo->console_usage = NULL;
      rominfo->copier_usage = swc_usage;
      rominfo->maker = "Manufacturer: You?";
      rominfo->country = "Country: Your country?";
      rominfo->has_internal_crc = 0;
      strcat (rominfo->misc, "Type: Super Wild Card SRAM file\n");
      ucon64.split = 0;                         // SRAM files are never split
      type = SWC;
      return 0;                                 // rest is nonsense for SRAM file
    }

  /*
    snes_testinterleaved() needs the correct value for snes_hirom and
    rominfo->header_start. snes_hirom may be used only after the check for -hi/-nhi
    has been done. If ALT_HILO is defined however, rominfo->buheader_len must have
    the correct value in order to determine the value for snes_hirom. This can only
    be known after the backup unit header length detection (including the check for
    -hd/-nhd/-hdn). So, the order must be
    1. - rominfo->buheader_len
    2. - snes_hirom
    3. - check for -hi/-nhi
    4. - snes_testinterleaved()

    step 1. & first part of step 2.
  */
  snes_hirom = 0;
  rominfo->buheader_len = 0;
  if ((x = get_internal_sums (rominfo)) != 0xffff)
    {
      snes_hirom = 0;
      rominfo->buheader_len = SWC_HEADER_LEN;
      if ((x = get_internal_sums (rominfo)) != 0xffff)
        {
          snes_hirom = SNES_HIROM;
          rominfo->buheader_len = 0;
          if ((x = get_internal_sums (rominfo)) != 0xffff)
            {
              snes_hirom = SNES_HIROM;
              rominfo->buheader_len = SWC_HEADER_LEN;
              x = get_internal_sums (rominfo);
            }
        }
    }

  if (header.id1 == 0xaa && header.id2 == 0xbb && header.type == 4)
    type = SWC;
  else if (!strncmp ((char *) &header, "GAME DOCTOR SF 3", 0x10))
    type = GD3;
  else if ((header.hirom == 0x80 &&             // HiROM
             ((header.emulation1 == 0x77 && header.emulation2 == 0x83) ||
              (header.emulation1 == 0xdd && header.emulation2 == 0x82) ||
              (header.emulation1 == 0xdd && header.emulation2 == 0x02) ||
              (header.emulation1 == 0xf7 && header.emulation2 == 0x83) ||
              (header.emulation1 == 0xfd && header.emulation2 == 0x82)))
            ||
           (header.hirom == 0x00 &&             // LoROM
             ((header.emulation1 == 0x77 && header.emulation2 == 0x83) ||
              (header.emulation1 == 0x00 && header.emulation2 == 0x80) ||
#if 1
              // This makes NES FFE ROMs & Game Boy ROMs be detected as SNES
              //  ROMs, see src/console/nes.c & src/console/gb.c
              (header.emulation1 == 0x00 && header.emulation2 == 0x00) ||
#endif
              (header.emulation1 == 0x47 && header.emulation2 == 0x83) ||
              (header.emulation1 == 0x11 && header.emulation2 == 0x02)))
          )
    type = FIG;
  else if (rominfo->buheader_len == 0 && x == 0xffff)
    type = MGD;

  /*
    x can be better trusted than type == FIG, but x being 0xffff is definitely
    not a guarantee that rominfo->buheader_len already has the right value
    (e.g. Earthworm Jim (U), Alfred Chicken (U|E), Soldiers of Fortune (U)).
  */
  if (type != MGD && type != SMC)
    {
      y = ((header.size_high << 8) + header.size_low) * 8 * 1024;
      y += SWC_HEADER_LEN;                      // if SWC-like header -> hdr[1] high byte,
      if (y == rominfo->file_size)              //  hdr[0] low byte of # 8 kB blocks in ROM
        rominfo->buheader_len = SWC_HEADER_LEN;
      else
        {
          int surplus = rominfo->file_size % MBIT;
          if (surplus == 0)
            // most likely we guessed the copier type wrong
            {
              rominfo->buheader_len = 0;
              type = MGD;
            }
          else if ((surplus % SWC_HEADER_LEN) == 0 && surplus < MAXBUFSIZE)
            rominfo->buheader_len = surplus;
        }
    }
  if (UCON64_ISSET (ucon64.buheader_len))       // -hd, -nhd or -hdn option was specified
    {
      rominfo->buheader_len = ucon64.buheader_len;
      if (type == MGD)
        type = SMC;
    }

  if (UCON64_ISSET (ucon64.split))
    snes_split = ucon64.split;
  else
    {
      if (type == SWC || type == FIG)
        {
          // TODO?: fix this code for last split file
          snes_split = 0;
          if (header.emulation & 0x40)
            snes_split = ucon64_testsplit (ucon64.rom);
          ucon64.split = snes_split;            // force displayed info to be correct
        }                                       //  if not split (see ucon64.c)
      else
        snes_split = ucon64_testsplit (ucon64.rom);
    }

  size = rominfo->file_size - rominfo->buheader_len;
  if (size < 0xfffd)
    return -1;                                  // don't continue (seg faults!)
  if (ucon64.console == UCON64_SNES || (type != SMC && size <= 16 * 1024 * 1024))
    result = 0;                                 // it seems to be a SNES ROM dump

  if (!(rom_buffer = (unsigned char *) malloc (size)))
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      return -1;                                // don't exit(), we might've been
    }                                           //  called from -lsv
  q_fread (rom_buffer, rominfo->buheader_len, size, ucon64.rom);

#ifdef  ALT_HILO
  // second part of step 2.
  snes_hirom = score_hirom (rom_buffer, size) > score_lorom (rom_buffer, size) ?
    SNES_HIROM : 0;
#endif
  /*
    It would be nice if snes_header.map_type & 1 could be used to verify that
    snes_hirom has the correct value, but it doesn't help much. For games like
    Batman Revenge of the Joker (U) it matches what score_hirom() finds.
    snes_hirom must be 0x8000 for that game in order to display correct
    information. However it should be 0 when writing a copier header.
    So, snes_header.map_type can't be used to recognize such cases.
  */

  // step 3.
  if (UCON64_ISSET (ucon64.snes_hirom))          // -hi or -nhi option was specified
    snes_hirom = ucon64.snes_hirom ? SNES_HIROM : 0;

  rominfo->header_start = SNES_HEADER_START + snes_hirom;
  /*
    Dai Kaiju Monogatari 2 (J) and Tales of Phantasia (J) have their SNES
    header at an odd location. We identify those ROM dumps by their internal
    name (including DeJap's ToP patch).
  */
  x = SNES_HEADER_START + SNES_HIROM + 0x400000;
  if (size > x + SNES_HEADER_LEN)
    if (!memcmp(rom_buffer + x + 16, "DAIKAIJYUMONOGATARI2", 20) || // yes, kaijYu
        !memcmp(rom_buffer + x + 16, "TALES OF PHANTASIA", 18) ||
        !memcmp(rom_buffer + x + 16, "ToP ", 4))
      {
        rominfo->header_start = x;
        // -hi/-nhi has no effect so we better give snes_hirom the right value
        snes_hirom = SNES_HIROM;
      }
  rominfo->header_len = SNES_HEADER_LEN;
  // set snes_header before calling snes_testinterleaved()
  memcpy (&snes_header, rom_buffer + rominfo->header_start, rominfo->header_len);
  rominfo->header = &snes_header;

  bs_dump = snes_check_bs ();
  if (!bs_dump)
    {
      // we might have to check the other header location
      if (!snes_hirom)
        {
          memcpy (&snes_header, rom_buffer + rominfo->header_start + SNES_HIROM,
                  rominfo->header_len);
          if (snes_check_bs ())
            {
              bs_dump = 1;
              snes_hirom = SNES_HIROM;
              rominfo->header_start += SNES_HIROM;
            }
          else
            memcpy (&snes_header, rom_buffer + rominfo->header_start, rominfo->header_len);
        }
    }
  if (UCON64_ISSET (ucon64.bs_dump))            // -bs or -nbs option was specified
    bs_dump = ucon64.bs_dump;

  // step 4.
  force_interleaved = UCON64_ISSET (ucon64.interleaved) ? 1 : 0;
  rominfo->interleaved = UCON64_ISSET (ucon64.interleaved) ?
    ucon64.interleaved : snes_testinterleaved (rominfo);

  // internal ROM name
  memcpy (rominfo->name, snes_header.name, SNES_NAME_LEN);
  for (x = 0; x < SNES_NAME_LEN; x++)
    if (!isprint ((int) rominfo->name[x]))      // we can't use mkprint(), because it skips \n
      rominfo->name[x] = '.';
  rominfo->name[SNES_NAME_LEN] = 0;             // terminate string (at 1st byte _after_ string)

  rominfo->console_usage = snes_usage;
  if (!rominfo->buheader_len)
    rominfo->copier_usage = mgd_usage;
  else
    {
      switch (type)
        {
        case GD3:
          rominfo->copier_usage = gd_usage;
          break;
        case FIG:
          rominfo->copier_usage = fig_usage;
          break;
        // just assume it's in SWC format... (there are _many_ ROMs on the
        //  internet with incorrect headers)
        default:
          rominfo->copier_usage = swc_usage;
        }
    }

  // ROM maker
  if (snes_header.maker == 0x33 || bs_dump)
    {
#if 1
      x = (snes_header.maker_high - '0') * 36 + snes_header.maker_low - '0';
      if (x < 0 || x > 791)
        x = 0;
      rominfo->maker = NULL_TO_UNKNOWN_S (nintendo_maker[x]);
#else
      x = (snes_header.maker_high < 0x3a ?
            (snes_header.maker_high - 0x30) * 16 :
            (snes_header.maker_high - 0x37) * 16) + (snes_header.maker_low < 0x3a ?
                                                      snes_header.maker_low - 0x30 :
                                                      snes_header.maker_low - 0x37);
      if (x < 0 || x > 255)
        x = 0;
      rominfo->maker = NULL_TO_UNKNOWN_S (snes_maker[x]);
#endif
    }
  else
    rominfo->maker = snes_header.maker == 0 ? "Demo or Beta ROM?" :
      NULL_TO_UNKNOWN_S (snes_maker[snes_header.maker]);

  if (!bs_dump)
    {
      // ROM country
      rominfo->country = NULL_TO_UNKNOWN_S (snes_country[MIN (snes_header.country, SNES_COUNTRY_MAX - 1)]);

      // misc stuff
      sprintf (buf, "Internal Size: %.4f Mb\n", (float) (1 << (snes_header.size - 7)));
      strcat (rominfo->misc, buf);

      sprintf (buf, "RomType: (%x) %s", snes_header.rom_type,
        snes_romtype[(snes_header.rom_type & 7) % 3]);
      strcat (rominfo->misc, buf);
      if ((snes_header.rom_type & 0xf) >= 3)
        {
          if (snes_header.rom_type == 3 || snes_header.rom_type == 4 ||
              snes_header.rom_type == 5)
            str = "DSP";
          else if (snes_header.rom_type == 0x13)
            str = "SuperFX";
          else if ((snes_header.rom_type & 0xf0) == 0x10) // 0x14, 0x15 or 0x1a
            str = "SuperFX2";
          else if (snes_header.rom_type == 0x25)
            str = "OBC1";
          else if ((snes_header.rom_type & 0xf0) == 0x30) // 0x34 or 0x35
            str = "SA-1";
          else if ((snes_header.rom_type & 0xf0) == 0x40) // 0x43 or 0x45
            str = "S-DD1";
          else if (snes_header.rom_type == 0x55)
            str = "S-RTC";
          else if (snes_header.rom_type == 0xe3)
            str = "Game Boy data";
          else if (snes_header.rom_type == 0xf3)
            str = "C4";
          else if (snes_header.rom_type == 0xf5 || snes_header.rom_type == 0xf9)
            str = "SPC7110";
          else if (snes_header.rom_type == 0xf6)
            str = "Seta's DSP";
          else
            str = "unknown";
          sprintf (buf, " and %s", str);
          strcat (rominfo->misc, buf);
        }
      strcat (rominfo->misc, "\n");
    }
  else                                          // BS info
    {
      // ROM country
      rominfo->country = "Japan";
      // misc stuff
      sprintf (buf, "\nBroadcast Satellaview dump\n");  // new line is intentional
      strcat (rominfo->misc, buf);
      sprintf (buf, "Size: 0x%x\n", snes_header.bs_size);
      strcat (rominfo->misc, buf);
      // TODO: reverse engineering SMC.COM ;-)
    }
  sprintf (buf, "RomSpeed: %s\n",
           snes_header.map_type & 0x10 ? "120ns (FastROM)" : "200ns (SlowROM)");
  strcat (rominfo->misc, buf);

  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      // internal ROM crc
      rominfo->has_internal_crc = 1;
      rominfo->internal_crc_len = rominfo->internal_crc2_len = 2;
      rominfo->current_internal_crc = snes_chksum (rominfo, rom_buffer);
      rominfo->internal_crc = snes_header.checksum_low;
      rominfo->internal_crc += snes_header.checksum_high << 8;
      x = snes_header.inverse_checksum_low;
      x += snes_header.inverse_checksum_high << 8;
      sprintf (buf,
               "Inverse Checksum: %%s, 0x%%0%dlx + 0x%%0%dlx = 0x%%0%dlx %%s",
               rominfo->internal_crc2_len * 2, rominfo->internal_crc2_len * 2,
               rominfo->internal_crc2_len * 2);
      sprintf (rominfo->internal_crc2, buf,
#ifdef  ANSI_COLOR
               ucon64.ansi_color ?
                 ((rominfo->current_internal_crc + x == 0xffff) ?
                   "\x1b[01;32mok\x1b[0m" : "\x1b[01;31mbad\x1b[0m")
                 :
                 ((rominfo->current_internal_crc + x == 0xffff) ? "ok" : "bad"),
#else
               (rominfo->current_internal_crc + x == 0xffff) ? "ok" : "bad",
#endif
               rominfo->current_internal_crc, x, rominfo->current_internal_crc + x,
               (rominfo->current_internal_crc + x == 0xffff) ? "" : "~0xffff");
    }

  // do the following after the call to snes_chksum() which might call
  //  snes_deinterleave() which might change the value of snes_hirom
  sprintf (buf, "HiROM: %s\n", snes_hirom ? "Yes" : "No");
  strcat (rominfo->misc, buf);

  if (!bs_dump)
    {
      snes_sramsize = snes_header.sram_size ? (1 << (snes_header.sram_size + 3)) * 128 : 0;
      if (!snes_sramsize)
        sprintf (buf, "Save RAM: No\n");
      else
        sprintf (buf, "Save RAM: Yes, %d kBytes\n", snes_sramsize / 1024);
      strcat (rominfo->misc, buf);
    }

  sprintf (buf, "Version: 1.%d", snes_header.version);
  strcat (rominfo->misc, buf);

  if (rominfo->buheader_len && !memcmp (&OFFSET (header, 0x1e8), "NSRT", 4))
    {
      char name[SNES_NAME_LEN + 1], *str_list[9] =
        {
          "Gamepad", "Mouse", "Mouse / Gamepad", "Super Scope",
          "Super Scope / Gamepad", "Konami's Justifier", "Multitap",
          "Mouse / Super Scope / Gamepad", "unknown"
        };
      int x = OFFSET (header, 0x1ed), ctrl1 = x >> 4, ctrl2 = x & 0xf;

      memcpy (name, &OFFSET (header, 0x1d1), SNES_NAME_LEN);
      name[SNES_NAME_LEN] = 0;
      for (x = 0; x < SNES_NAME_LEN; x++)
        if (!isprint ((int) name[x]))
          name[x] = '.';

      if (ctrl1 > 8)
        ctrl1 = 8;
      if (ctrl2 > 8)
        ctrl2 = 8;
      sprintf (buf, "\nNSRT info:\n"
                      "  Original country: %s\n"
                      "  Original game name: \"%s\"\n"
                      "  Original checksum: 0x%04x\n"
                      "  Port 1 controller type: %s\n"
                      "  Port 2 controller type: %s\n"
                      "  Header version: %.1f",
               NULL_TO_UNKNOWN_S (snes_country[MIN (OFFSET (header, 0x1d0) & 0xf, SNES_COUNTRY_MAX - 1)]),
               name,
               OFFSET (header, 0x1e6) + (OFFSET (header, 0x1e7) << 8),
               str_list[ctrl1],
               str_list[ctrl2],
               OFFSET (header, 0x1ec) / 10.f);
      strcat (rominfo->misc, buf);

      nsrt_header = 1;
    }
  else
    nsrt_header = 0;

  free (rom_buffer);
  return result;
}


#if 1
int
snes_check_bs (void)
{
  unsigned int value;

  if (snes_header.bs_size & 0x4f)
    return 0;

  if (snes_header.maker != 0x33 && snes_header.maker != 0xff)
    return 0;

  value = ((unsigned char *) &snes_header)[39] << 8 |
          ((unsigned char *) &snes_header)[38];
  if (value != 0x0000 && value != 0xffff)
    {
      if ((value & 0x040f) != 0)
        return 0;
      if ((value & 0xff) > 0xc0)
        return 0;
    }

  if (snes_header.bs_makeup & 0xce || ((snes_header.bs_makeup & 0x30) == 0))
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


int
snes_bs_name (void)
{
  unsigned int value;
  int n, n_valid = 0;

  for (n = 0; n < 16; n++)
    {
      value = snes_header.name[n];
      if (check_char (value) != 0)
        {
          value = snes_header.name[n + 1];
          if (value < 0x20)
            if ((n_valid != 11) || (value != 0)) // Dr. Mario Hack
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

          if (value < 0x20)
            break;

          if (value >= 0x80)
            if (value < 0xa0 || value >= 0xf0)
              break;
          n_valid++;
        }
    }

  return n == 16 && n_valid > 0 ? 1 : 0;
}


int
check_char (unsigned char c)
{
  if ((c & 0x80) == 0)
    return 0;

  if ((c - 0x20) & 0x40)
    return 1;
  else
    return 0;
}
#else
int
snes_special_bs (void)
{
  if (!strncmp ("BS DRAGON QUEST", snes_header.name, 15))
    return 1;
  if (!strncmp ("BUSTERS BS", snes_header.name, 10))
    return 1;
  if (!strncmp ("Mario Excite Bike", snes_header.name, 17))
    return 1;
  if (!strncmp ("BS_Dr Mario", snes_header.name, 11))
    return 1;
  if (!strncmp ("BS_SuperFam", snes_header.name, 11))
    return 1;
  if (!strncmp ("BS_Legend", snes_header.name, 9))
    return 1;
  if (!strncmp ("KARBYCOL", ((char *) &snes_header) + 7, 8))
    return 1;
  if (snes_header.name[0] == 0xdc && snes_header.name[1] == 0xb2 &&
      snes_header.name[5] == 0xaa)
    return 1;                                   // bs wai wai
  if (snes_header.name[0] == 0x8c && snes_header.name[1] == 0x8e &&
      snes_header.name[5] == 0xb2)
    return 1;                                   // bs coin deck
  if (snes_header.name[0] == 0x82 && snes_header.name[1] == 0xb3 &&
      snes_header.name[5] == 0xaa)
    return 1;                                   // bs same game koma data
  if (snes_header.name[0] == 0xbb && snes_header.name[1] == 0xc3 &&
      snes_header.name[5] == 0x2d)
    return 1;                                   // ?, not bs zelda 3 (checked
                                                //  remix, 1 & 3)
  return 0;
}


int
snes_check_bs (void)
{
  unsigned char byte;
  int i;

  if (snes_special_bs ())
    return 1;
  if (snes_header.name[SNES_NAME_LEN - 1])      // 36
    return 0;
  if (snes_header.maker != 0x33 && snes_header.maker != 0 && snes_header.maker != 0xff)
    return 0;
  if (snes_header.bs_makeup & 0x8e)
    return 0;
  if (!(snes_header.bs_makeup & 0x70))
    return 0;
  byte = ((char *) &snes_header)[32];           // 32
  if (byte != 0x01 && byte != 0x03 && byte != 0x07 && byte != 0x0f && byte != 0xff)
    return 0;
//  if (byte != 0 && byte != 0x01 && byte != 0x20 && byte != 0x21 && byte != 0x30 && byte != 0x31)
//    return 0;
  if (snes_header.bs_size != 0 && snes_header.bs_size != 0x10 &&
      snes_header.bs_size != 0x20 && snes_header.bs_size != 0x30 &&
      snes_header.bs_size != 0x80)
    return 0;
//  if (!isalnum (snes_header.maker_x) return 0;
//  if (!isalnum (snes_header.maker_y) return 0;
  for (i = 0; i < 12; i++)
    if (snes_header.name[i] == 0xff)
      return 0;
  if (snes_header.bs_month != 0 && snes_header.bs_month != 0xff)
    {
      if (snes_header.bs_month & 0x0f)
        return 0;
      if ((snes_header.bs_month >> 4) > 12)
        return 0;
    }

  return 1;
}
#endif


int
snes_chksum (st_rominfo_t *rominfo, unsigned char *rom_buffer)
/*
  Calculate the checksum of a SNES ROM. This function tries to calculate a
  checksum that matches the one that is stored inside the ROM. As a result of
  this the calculated checksum is not of much use if this function can't
  calculate a sum that matches the internal one. However, it's not *that* bad,
  because the CRC32 should be used anyway to identify SNES ROM dumps.
*/
{
  int i, rom_size, pow2size, pow2size_half, remainder;
  unsigned short int sum1, sum2, internal_sum;

  rom_size = rominfo->file_size - rominfo->buheader_len;
  if (rominfo->interleaved)
    snes_deinterleave (rominfo, rom_buffer, rom_size);

  pow2size = 1;
  while (pow2size < rom_size)
    pow2size <<= 1;
  pow2size_half = pow2size >> 1;

  sum1 = 0;
  if (pow2size_half >= MBIT && bs_dump)
    {                                           // Broadcast Satellaview "ROM"
      for (i = 0; i < rominfo->header_start; i++)
        sum1 += rom_buffer[i];
      for (i = rominfo->header_start + SNES_HEADER_LEN; i < pow2size_half; i++)
        sum1 += rom_buffer[i];
    }
  else
    for (i = 0; i < pow2size_half; i++)         // normal ROM
      sum1 += rom_buffer[i];

  sum2 = 0;
  remainder = rom_size - pow2size_half;
  for (i = pow2size_half; i < rom_size; i++)
    sum2 += rom_buffer[i];
  sum1 += sum2 * (pow2size_half / remainder);

  // here follows a fix for ROM dumps like Far East of Eden Zero (J)
  //  and Momotaro Dentetsu Happy (J)
  internal_sum = snes_header.checksum_low + (snes_header.checksum_high << 8);
  if (internal_sum != sum1)                     // checksum seems bad -> try another
    {                                           //  checksum algorithm
      sum2 = 0;
      for (i = 0; i < rom_size; i++)
        sum2 += rom_buffer[i];
      if (sum2 == internal_sum)
        sum1 = sum2;                            // Far East of Eden Zero
      sum2 *= 2;
      if (sum2 == internal_sum)
        sum1 = sum2;                            // Momotaro Dentetsu Happy
    }

  /*
    Load rom_buffer with the ROM again if uCON64 detected it as interleaved or
    if uCON64 was forced to handle it as being interleaved, so that rom_buffer
    matches with the ROM dump on disk. rom_buffer was "deinterleaved" (=changed)
    in order to calculate the checksum. However, it isn't necessary to load
    rom_buffer again in the case that uCON64 wasn't forced to handle the ROM as
    interleaved and snes_deinterleave() detected that the ROM wasn't interleaved
    after all. In that case snes_deinterleave() already reloaded rom_buffer. Of
    course, it also isn't necessary to reload rom_buffer if ROM is a normal ROM.
    In short:
      force_interleaved?  interleaved?    reload?
      yes                 yes             yes
      yes                 no              yes
      no                  yes             yes
      no                  no              no
  */
  if (force_interleaved || rominfo->interleaved)
    q_fread (rom_buffer, rominfo->buheader_len, rom_size, ucon64.rom);

  return sum1;
}


#ifdef  ALT_HILO
int
score_hirom (unsigned char *rom_buffer, int rom_size)
{
  int score = 0;

  if ((rom_buffer[0xff00 + 0xdc] + (rom_buffer[0xff00 + 0xdd] << 8) +
       rom_buffer[0xff00 + 0xde] + (rom_buffer[0xff00 + 0xdf] << 8)) ==
      0xffff)
    score += 2;

  if (rom_buffer[0xff00 + 0xda] == 0x33)
    score += 2;
  if ((rom_buffer[0xff00 + 0xd5] & 0xf) < 4)
    score += 2;
  if (!(rom_buffer[0xff00 + 0xfd] & 0x80))
    score -= 4;
#if 0
  // this seems to work by accident for Snes9x (letting rom_size (CalculatedSize)
  // be 0 for first call)
  if (rom_size > 1024 * 1024 * 3)
    score += 4;
#endif
  if ((1 << (rom_buffer[0xff00 + 0xd7] - 7)) > 48)
    score -= 1;
  if (!areprint (rom_buffer + 0xff00 + 0xb0, 6))
    score -= 1;
  if (!areprint (rom_buffer + 0xff00 + 0xc0, SNES_NAME_LEN))
    score -= 1;

  return score;
}


int
score_lorom (unsigned char *rom_buffer, int rom_size)
{
  int score = 0;

  if ((rom_buffer[0x7f00 + 0xdc] + (rom_buffer[0x7f00 + 0xdd] << 8) +
       rom_buffer[0x7f00 + 0xde] + (rom_buffer[0x7f00 + 0xdf] << 8)) ==
      0xffff)
    score += 2;

  if (rom_buffer[0x7f00 + 0xda] == 0x33)
    score += 2;
  if ((rom_buffer[0x7f00 + 0xd5] & 0xf) < 4)
    score += 2;
#if 1
  // this also seems strange, ROMs are always less than 16 MB
  if (rom_size <= 1024 * 1024 * 16)
    score += 2;
#endif
  if (!(rom_buffer[0x7f00 + 0xfd] & 0x80))
    score -= 4;
  if ((1 << (rom_buffer[0x7f00 + 0xd7] - 7)) > 48)
    score -= 1;
  if (!areprint (rom_buffer + 0x7f00 + 0xb0, 6))
    score -= 1;
  if (!areprint (rom_buffer + 0x7f00 + 0xc0, SNES_NAME_LEN))
    score -= 1;

  return score;
}
#endif
