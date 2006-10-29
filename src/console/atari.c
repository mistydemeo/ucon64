/*
atari.c - Atari 2600/5200/7800 support for uCON64

Copyright (c) 2004 - 2005 NoisyB (noisyb@gmx.net)

Inspired by code from makewav v4.1 and MakeBin v1.0, written by Bob Colbert
  <rcolbert1@home.com>
atari_init() uses some code from Stella - An Atari 2600 VCS Emulator
  Copyright (c) 1995-2005 Bradford W. Mott


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
#include <ctype.h>
#ifdef  HAVE_MATH_H
#include <math.h>                               // sin(), floor()
#endif
#include "misc/itypes.h"
#include "misc/bswap.h"
#include "misc/misc.h"
#include "misc/hash.h"
#include "misc/getopt2.h"
#include "misc/file.h"
#include "misc/string.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "atari.h"
#include "backup/yoko.h"
#include "backup/cc2.h"
#include "backup/spsc.h"


static st_ucon64_obj_t atari_obj[] =
  {
    {UCON64_ATA, WF_SWITCH},
    {UCON64_ATA, WF_PROBE | WF_INIT | WF_NFO},
    {UCON64_ATA, WF_PROBE | WF_INIT | WF_NFO | WF_NO_SPLIT}
  };

const st_getopt2_t atari_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Atari VCS 2600/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr",
      /*"1977/1982/1984/1986 Atari"*/
      NULL
    },
    {
      UCON64_ATA_S, 0, 0, UCON64_ATA,
      NULL, "force recognition",
      &atari_obj[0]
    },
#ifdef  HAVE_MATH_H
#if 0
    {
      "cc2", 2, 0, UCON64_CC2,
      "BSM", "convert BIN to Cuttle Card (2)/(Starpath) Supercharger/WAV"
      "\n"
      "BSM=0 auto  BSM=10 FA\n"
      "BSM=1 2K    BSM=11 F6SC\n"
      "BSM=2 CV    BSM=12 F6\n"
      "BSM=3 4K    BSM=13 E7\n"
      "BSM=4 F8SC  BSM=14 E7NR\n"
      "BSM=5 F8    BSM=15 F4SC\n"
      "BSM=6 FE    BSM=16 F4\n"
      "BSM=7 3F    BSM=17 MB\n"
      "BSM=8 E0\n"
      "BSM=9 FANR",
      &atari_obj[1]
    },
#else
    {
      "cc2", 0,
      0, UCON64_CC2,
      NULL,
      "convert BIN to Cuttle Card (2)/(Starpath) Supercharger/WAV",
      &atari_obj[1]
    },
#endif
#endif
#if 0
    {
      "bin", 0, 0, UCON64_BIN,
      NULL, "convert Cuttle Card (2)/(Starpath) Supercharger/WAV to BIN",
      &atari_obj[2]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#define ATARI_ROM_SIZE 0x20000
#define ATARI_ROM_HEADER_START 0
#define ATARI_ROM_HEADER_LEN 0


typedef struct
{
  int bsm;
  unsigned char ctrl_byte;
  unsigned char start_hi;
  unsigned char start_low;
  unsigned char speed_hi;
  unsigned char speed_low;
  int game_page_count;
  unsigned char empty_page[1024];
  unsigned char page_list[24];
  unsigned char multi_byte;
} st_atari_rominfo_t;


static st_atari_rominfo_t atari_rominfo;


enum {
  BSM_2K = 1,
  BSM_AR,
  BSM_CV,
  BSM_4K,
  BSM_F8SC,
  BSM_F8,
  BSM_FE,
  BSM_3F,
  BSM_DPC,
  BSM_E0,
  BSM_FANR,
  BSM_FASC,
  BSM_FA,
  BSM_F6SC,
  BSM_F6,
  BSM_E7,
  BSM_E7NR,
  BSM_F4SC,
  BSM_F4,
  BSM_MB,
  BSM_MC,
  BSM_UA
};


typedef struct
{
  int bsm; // id
  char *bsm_s;
  uint32_t fsize;
  unsigned char ctrl_byte;
  int start_page;
} st_atari_bsmode_t;


static st_atari_bsmode_t bsmode[] = {
  {BSM_2K,   "2K",   0x800,   0xca, 7},
  {BSM_CV,   "CV",   0x800,   0xea, 7},
  {BSM_4K,   "4K",   0x1000,  0xc8, 15},
  {BSM_F8SC, "F8SC", 0x2000,  0xe6, 15},
  {BSM_F8,   "F8",   0x2000,  0xc6, 15},
  {BSM_FE,   "FE",   0x2000,  0xcc, 15},
  {BSM_3F,   "3F",   0x2000,  0xce, 31},
  {BSM_E0,   "E0",   0x2000,  0xc1, 31},
  {BSM_FANR, "FANR", 0x2000,  0xc0, 15},
  {BSM_UA,   "UA",   0x2000,  0,    0}, // ctrl_byte? start_page?
  {BSM_AR,   "AR",   0x2100,  0,    0}, // ctrl_byte? start_page?
  {BSM_DPC,  "DPC",  0x2800,  0,    0}, // ctrl_byte? start_page?
  {BSM_DPC,  "DPC",  0x28ff,  0,    0}, // ctrl_byte? start_page?
  {BSM_FA,   "FA",   0x3000,  0xe0, 15},
  {BSM_F6SC, "F6SC", 0x4000,  0xe4, 15},
  {BSM_F6,   "F6",   0x4000,  0xc4, 15},
  {BSM_E7,   "E7",   0x4000,  0xe3, 63},
  {BSM_E7NR, "E7NR", 0x4000,  0xc3, 63},
  {BSM_F4SC, "F4SC", 0x8000,  0xe2, 15},
  {BSM_F4,   "F4",   0x8000,  0xc2, 15},
  {BSM_MB,   "MB",   0x10000, 0xc9, 15},
  {BSM_MC,   "MC",   0x20000, 0,    0}, // ctrl_byte? start_page?
  {0,        NULL,   0,       0,    0}
};


typedef struct
{
  uint32_t crc;
  const char *md5;
  int bsmode;
} st_atari_game_bsmode_t;


st_atari_game_bsmode_t atari_bsmodes[] = {
  // CommaVid
  {0x34ae2945, NULL, BSM_CV},                   // Magicard (CommaVid)
//  {0x30eb4f7a, NULL, BSM_CV},                   // Video Life (4K)
//  {0x9afa761f, NULL, BSM_CV},                   // Magicard (Life)
  {0x266bd1b6, NULL, BSM_CV},                   // Video Life (CommaVid)
  // Parker Brothers
  {0x2843d776, NULL, BSM_E0},                   // Frogger II - Threedeep!
  {0x690ada72, NULL, BSM_E0},                   // Gyruss [b]
  {0x525ee7e9, NULL, BSM_E0},                   // Gyruss
  {0x95da4070, NULL, BSM_E0},                   // James Bond 007 [b]
  {0x3216c1bb, NULL, BSM_E0},                   // James Bond 007
  {0xae4114d8, NULL, BSM_E0},                   // Montezuma's Revenge
  {0x00e44527, NULL, BSM_E0},                   // Mr. Do!'s Castle
  {0xf723b8a6, NULL, BSM_E0},                   // Popeye
  {0xe44c244e, NULL, BSM_E0},                   // Q-bert's Qubes [a]
  {0xb8f2dca6, NULL, BSM_E0},                   // Q-bert's Qubes
  {0xe77f6742, NULL, BSM_E0},                   // Star Wars - Death Star Battle (Parker Bros)
  {0xce09fcd4, NULL, BSM_E0},                   // Star Wars - The Arcade Game (Parker Bros)
  {0xdd85f0e7, NULL, BSM_E0},                   // Super Cobra [b]
  {0x8d372730, NULL, BSM_E0},                   // Super Cobra
  {0xd9088807, NULL, BSM_E0},                   // Tooth Protectors (DSD-Camelot)
  {0x7eed7362, NULL, BSM_E0},                   // Tutankham
  {0xc87fc312, NULL, BSM_E0},                   // Popeye_(eks)
  {0xef3ec01e, NULL, BSM_E0},                   // Super Cobra_(eks)
  {0x84a101d4, NULL, BSM_E0},                   // Star Wars - Death Star Battle_(eks)
  {0x2fc06cb0, NULL, BSM_E0},                   // Tutankham_(eks)
  {0xab50bf11, NULL, BSM_E0},                   // Star Wars - The Arcade Game (proto)
  {0x549a1b6b, NULL, BSM_E0},                   // Star Wars - The Arcade Game (PAL)
  {0x36910e4d, NULL, BSM_E0},                   // Frogger II - Threedeep! (PAL)
  {0xb8bb2361, NULL, BSM_E0},                   // Gyruss (PAL)
  // Tigervision
  {0x584f6777, NULL, BSM_3F},                   // Espial [b]
  {0x8d70fa42, NULL, BSM_3F},                   // Espial
  {0x8beb03d4, NULL, BSM_3F},                   // Miner 2049er [b1]
  {0x33f2856f, NULL, BSM_3F},                   // Miner 2049er [b2]
  {0xf859122e, NULL, BSM_3F},                   // Miner 2049er Vol. 2 [b1]
  {0x281a1ca1, NULL, BSM_3F},                   // Miner 2049er Vol. 2 [b2]
  {0x350c63ba, NULL, BSM_3F},                   // Miner 2049er Vol. 2
  {0x728b941c, NULL, BSM_3F},                   // Miner 2049er
  {0x13bf2da3, NULL, BSM_3F},                   // Polaris [b]
  {0x7ce5312e, NULL, BSM_3F},                   // Polaris
  {0x40706361, NULL, BSM_3F},                   // River Patrol (Tigervision)
  {0x2c34898f, NULL, BSM_3F},                   // Springer
  // Activision 8K flat model
  {0x7d23e780, NULL, BSM_FE},                   // Decathlon
  {0xa51c0236, NULL, BSM_FE},                   // Robot Tank
  {0xd8ecf576, NULL, BSM_FE},                   // Decathlon (PAL)
  {0x0e8757b0, NULL, BSM_FE},                   // Robot Tank (PAL)
  {0x94e8df6b, NULL, BSM_FE},                   // Space Shuttle (PAL)
  // 16K Superchip that can't be recognized automatically
  {0xa972c32b, NULL, BSM_F6SC},                 // Dig Dug
  {0x66cdb94b, NULL, BSM_F6SC},                 // Off the Wall [o]
  {0xbd75d92b, NULL, BSM_F6SC},                 // Off the Wall
  // M Network 16K
  {0x8eed6b02, NULL, BSM_E7},                   // Bump n Jump [b]
  {0xd523e776, NULL, BSM_E7},                   // Bump n Jump
  {0x24c35820, NULL, BSM_E7},                   // Burgertime
  {0x5c161fe4, NULL, BSM_E7},                   // Masters of the Universe - The Power of He-Man
//  {0xbe1047cf, NULL, 0},                        // Fatal Run (PAL)
//  {0x6a31beac, NULL, 0},                        // Private Eye (CCE)
//  {0x3fa749c0, NULL, 0},                        // Private Eye [b]
//  {0x33242242, NULL, 0},                        // Private Eye
  // from Stella sources
  {0, "5336f86f6b982cc925532f2e80aa1e17", BSM_E0},    // Death Star
  {0, "b311ab95e85bc0162308390728a7361d", BSM_E0},    // Gyruss
  {0, "c29f8db680990cb45ef7fef6ab57a2c2", BSM_E0},    // Super Cobra
  {0, "085322bae40d904f53bdcc56df0593fc", BSM_E0},    // Tutankamn
  {0, "c7f13ef38f61ee2367ada94fdcc6d206", BSM_E0},    // Popeye
  {0, "6339d28c9a7f92054e70029eb0375837", BSM_E0},    // Star Wars, Arcade
  {0, "27c6a2ca16ad7d814626ceea62fa8fb4", BSM_E0},    // Frogger II
  {0, "3347a6dd59049b15a38394aa2dafa585", BSM_E0},    // Montezuma's Revenge
  {0, "6dda84fb8e442ecf34241ac0d1d91d69", BSM_F6SC},  // Dig Dug
  {0, "57fa2d09c9e361de7bd2aa3a9575a760", BSM_F8SC},  // Stargate
  {0, "3a771876e4b61d42e3a3892ad885d889", BSM_F8SC},  // Defender ][
  {0, "efefc02bbc5258815457f7a5b8d8750a", BSM_FASC},  // Tunnel runner
  {0, "7e51a58de2c0db7d33715f518893b0db", BSM_FASC},  // Mountain King
  {0, "9947f1ebabb56fd075a96c6d37351efa", BSM_FASC},  // Omega Race
  {0, "0443cfa9872cdb49069186413275fa21", BSM_E7},    // Burger Timer
  {0, "76f53abbbf39a0063f24036d6ee0968a", BSM_E7},    // Bump-N-Jump
  {0, "3b76242691730b2dd22ec0ceab351bc6", BSM_E7},    // He-Man
  {0, "ac7c2260378975614192ca2bc3d20e0b", BSM_FE},    // Decathlon
  {0, "4f618c2429138e0280969193ed6c107e", BSM_FE},    // Robot Tank
  {0, "6d842c96d5a01967be9680080dd5be54", BSM_DPC},   // Pitfall II
  {0, "d3bb42228a6cd452c111c1932503cc03", BSM_UA},    // Funky Fish
  {0, "8bbfd951c89cc09c148bfabdefa08bec", BSM_UA},    // Pleiades
  {0, NULL, 0}
};


static st_atari_bsmode_t *
get_bsmode_by_id (int bsm)
{
  int i = 0;

  while (bsmode[i].bsm)
    {
      if (bsmode[i].bsm == bsm)
        return &bsmode[i];
      i++;
    }

  return NULL;
}


static int
get_game_bsmode_by_crc (uint32_t crc)
{
  int i = 0;

  while (atari_bsmodes[i].bsmode)
    {
      if (atari_bsmodes[i].crc)
        if (atari_bsmodes[i].crc == crc)
          return atari_bsmodes[i].bsmode;
      i++;
    }

  return -1;
}


static int
get_game_bsmode_by_md5 (const char *md5)
{
  int i = 0;

  while (atari_bsmodes[i].bsmode)
    {
      if (atari_bsmodes[i].md5)
        if (!stricmp (atari_bsmodes[i].md5, md5))
          return atari_bsmodes[i].bsmode;
      i++;
    }

  return -1;
}


static int
is_probably_3f (const unsigned char *image, unsigned int size)
{
  unsigned long count = 0, i;

  for (i = 0; i < size - 1; i++)
    if ((image[i] == 0x85) && (image[i + 1] == 0x3F))
      ++count;

  return count > 2 ? TRUE : FALSE;
}


int
atari_init (st_ucon64_nfo_t * rominfo)
{
  int i, j, bsmode, size = ucon64.file_size;
  unsigned int crc32;
  static char backup_usage[80];
  unsigned char first, image[ATARI_ROM_SIZE], buffer[0x200];
  char md5[32];

  if (size > ATARI_ROM_SIZE)
    return -1;

  if (size != 0x800  && size != 0x1000  && size != 0x2000 && size != 0x2100 &&
      size != 0x2800 && size != 0x28ff  && size != 0x3000 && size != 0x4000 &&
      size != 0x8000 && size != 0x10000 && size != 0x20000)
    return -1;

  ucon64_fread (image, 0, size, ucon64.fname);
  ucon64_chksum (NULL, md5, &crc32, ucon64.fname, 0);

  bsmode = get_game_bsmode_by_crc (crc32);
  if (bsmode == -1)
    bsmode = get_game_bsmode_by_md5 (md5);

  if (bsmode == -1)
    {
      if (!(size % 8448))
        bsmode = BSM_AR;
      else if (size == 2048 || !memcmp (image, image + 2048, 2048))
        bsmode = BSM_2K;
      else if (size == 4096 || !memcmp (image, image + 4096, 4096))
        bsmode = BSM_4K;
      else if (size == 8192 || !memcmp (image, image + 8192, 8192))
        bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_F8;
      else if (size == 10495 || (size == 10240))
        bsmode = BSM_DPC;
      else if (size == 12288)
        bsmode = BSM_FASC;
      else if (size == 32768)
        {
          // Assume this is a 32K super-cart then check to see if it is
          bsmode = BSM_F4SC;

          first = image[0];
          for (i = 0; i < 0x100; i++)
            if (image[i] != first)
              {
                // It's not a super cart (probably)
                bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_F4;
                break;
              }
        }
      else if (size == 65536)
        bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_MB;
      else if (size == 131072)
        bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_MC;
      else
        {
          // Assume this is a 16K super-cart then check to see if it is
          bsmode = BSM_F6SC;

          first = image[0];
          for (i = 0; i < 0x100; i++)
            if (image[i] != first)
              {
                // It's not a super cart (probably)
                bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_F6;
                break;
              }
        }
    }

  if (bsmode > 0)
    {
      memset (&atari_rominfo, 0, sizeof (st_atari_rominfo_t));

      atari_rominfo.bsm = bsmode;

      // set game_page_count and empty_page[]
      for (i = 0; i < size / 0x100; i++)
        {
          ucon64_fread (buffer, i * 0x100, 0x100, ucon64.fname);
          atari_rominfo.empty_page[i] = 1;

          for (j = 0; j < 0x100 - 1; j++)
            if (buffer[j] != buffer[j + 1])
              {
                atari_rominfo.empty_page[i] = 0;
                atari_rominfo.game_page_count++;
                break;
              }
        }

      atari_rominfo.speed_hi = atari_rominfo.game_page_count / 21 + 1;
      atari_rominfo.speed_low = atari_rominfo.game_page_count * 0x100 / 21 - (atari_rominfo.speed_hi - 1) * 0x100;

      atari_rominfo.ctrl_byte = get_bsmode_by_id (atari_rominfo.bsm)->ctrl_byte;

      // the first two bytes of data indicate the beginning address of the code
      if (atari_rominfo.bsm != BSM_3F)
        {
          ucon64_fread (buffer, get_bsmode_by_id (atari_rominfo.bsm)->start_page * 0x100, 0x100, ucon64.fname);
          atari_rominfo.start_low = buffer[0xfc];
          atari_rominfo.start_hi = buffer[0xfd];
        }

      rominfo->console_usage = atari_usage[0].help;
      // "Cuttle Card (2)/Starpath) Supercharger/YOKO backup unit"
      snprintf (backup_usage, 80, "%s/%s/%s", cc2_usage[0].help,
                spsc_usage[0].help, yoko_usage[0].help);
      backup_usage[80 - 1] = 0;
      rominfo->backup_usage = backup_usage;

      sprintf (rominfo->misc,
               "Bankswitch type: %s\n"
               "Start address: 0x%02x%02x\n"
               "Speed: 0x%02x%02x\n"
               "Control byte: 0x%02x\n"
               "Page count: %d\n"
               "Blank pages: %d\n"
               "Start page: %d",
               get_bsmode_by_id (atari_rominfo.bsm)->bsm_s,
               atari_rominfo.start_hi,
               atari_rominfo.start_low,
               atari_rominfo.speed_hi,
               atari_rominfo.speed_low,
               atari_rominfo.ctrl_byte,
               atari_rominfo.game_page_count,
               size / 0x100 - atari_rominfo.game_page_count,
               get_bsmode_by_id (atari_rominfo.bsm)->start_page);
      return 0;
    }

  return -1;
}


#ifdef  HAVE_MATH_H
static void
wav_generator (unsigned char *bit, int bit_length, float volume, int type)
{
#define TYPE_SQUARE_WAVE 0
#define TYPE_SINE_WAVE 1
  int i;

  if (type == TYPE_SQUARE_WAVE)
    {
      int half_bit_length = (int) (floor ((float) bit_length) / 2.0);
      int odd = (int) (ceil ((float) bit_length / 2.0) - half_bit_length);

      for (i = 0; i < half_bit_length; i++)
        bit[i] = (unsigned char) floor (0xfc * volume);

      if (odd)
        bit[i++] = 0x80;

      for (; i < bit_length; i++)
        bit[i] = (unsigned char) floor (0x06 * volume);
    }
  else // SINE_WAV
    for (i = 0; i < bit_length; i++)
      bit[i] = (unsigned char) floor (((
        sin
        ((((double) 2 * (double) M_PI) / (double) bit_length) * (double) i) * volume + 1) * 128));
}


// Set header tone length in seconds (default = 1.0)
#define WAV_HEADER_SECONDS ((float)1.0)
// Set clearing tone length in seconds (default = 0.1)
#define WAV_CLEARING_SECONDS ((float)0.1)
#define WAV_CLEARINGTONE_LEN 51
// Set volume level of .wav file (0.10-10 Default=0.98)
#define WAV_VOLUME ((float)0.98)
// Set the # of bytes for a zero bit
#define WAV_ZEROBIT_LEN 6
// Set the # of bytes for a bit
#define WAV_ONEBIT_LEN 10


typedef struct
{
  uint8_t magic[4];       // 'RIFF'
  uint32_t total_length;  // length of file minus the 8 byte riff header

  uint8_t type[4];        // 'WAVE'

  uint8_t fmt[4];         // 'fmt '
  uint32_t header_length; // length of format chunk minus 8 byte header
  uint16_t format;        // identifies WAVE_FORMAT_PCM
  uint16_t channels;
  uint32_t freq;          // samples per second per channel
  uint32_t bytespersecond;
  uint16_t blockalign;    // basic block size
  uint16_t bitspersample;

  // PCM formats then go straight to the data chunk
  uint8_t data[4];        // 'data'
  uint32_t data_length;   // length of data chunk minus 8 byte header
} st_audio_wav_t;


#warning
static int
ucon64_write_wavheader (FILE *fh, int channels, int freq, int bitspersample, int data_length)
{
  st_audio_wav_t wav_header;
  memset (&wav_header, 0, sizeof (st_audio_wav_t));

  strncpy ((char *) wav_header.magic, "RIFF", 4);
  wav_header.total_length =           me2le_32 (data_length + sizeof (st_audio_wav_t) - 8);
  strncpy ((char *) wav_header.type,  "WAVE", 4);
  strncpy ((char *) wav_header.fmt,   "fmt ", 4);
  wav_header.header_length =          me2le_32 (16); // always 16
  wav_header.format =                 me2le_16 (1); // WAVE_FORMAT_PCM == default
  wav_header.channels =               me2le_16 (channels);
  wav_header.freq =                   me2le_32 (freq);
  wav_header.bytespersecond =         me2le_32 (freq * channels * bitspersample / 8);
  wav_header.blockalign =             me2le_16 (channels * bitspersample / 8);
  wav_header.bitspersample =          me2le_16 (bitspersample);
  strncpy ((char *) wav_header.data,  "data", 4);
  wav_header.data_length =            me2le_32 (data_length);

  return fwrite (&wav_header, 1, sizeof (st_audio_wav_t), fh);
}


static void
write_byte_as_wav (FILE *wav_file, unsigned char b, int times)
{
  int i = 0, p = 0, len = 0;
  static unsigned char zerobit[0x100], onebit[0x100];
  static int generated = 0;

  if (!generated)
    {
      wav_generator (zerobit, WAV_ZEROBIT_LEN, WAV_VOLUME, TYPE_SQUARE_WAVE);
      wav_generator (onebit, WAV_ONEBIT_LEN, WAV_VOLUME, TYPE_SQUARE_WAVE);
      generated = 1;
    }

  for (; i < times; i++)
    for (p = 7; p >= 0; p--)
      len += (b >> p) & 1 ?
        fwrite (onebit, 1, WAV_ONEBIT_LEN, wav_file) :
        fwrite (zerobit, 1, WAV_ZEROBIT_LEN, wav_file);
}


int
atari_cc2 (const char *fname, int bsmode)
{
  (void) bsmode; // TODO: bsmode override
  unsigned char buffer[0x200];
  int i, j, page;
  int force_empty = 0, init_bank;
//  int multi = 0;
  unsigned char in_byte;
  char pg_bank_byte;
#if     FILENAME_MAX > MAXBUFSIZE
  char dest_name[FILENAME_MAX];
#else
  char dest_name[MAXBUFSIZE];
#endif
  const char *source_fname = NULL;
  FILE *wav_file = NULL;
  FILE *bin_file = NULL;
  time_t start_time = time (0);
  st_audio_wav_t wav_header;

  wav_generator (buffer, WAV_CLEARINGTONE_LEN, WAV_VOLUME, TYPE_SINE_WAVE);

  strcpy (dest_name, fname);
  set_suffix (dest_name, ".wav");
  if (!(wav_file = fopen (dest_name, "wb")))
    {
      fprintf (stderr, "ERROR: unable to create WAV file %s\n", dest_name);
      return -1;
    }

  // write low frequency clearing tone
  for (i = 0; i < (int) (WAV_CLEARING_SECONDS * (44100 / WAV_CLEARINGTONE_LEN)); i++)
    fwrite (buffer, 1, WAV_CLEARINGTONE_LEN, wav_file);

  // write empty wav header
  memset (&wav_header, 0, sizeof (st_audio_wav_t));
  fwrite (&wav_header, 1, sizeof (st_audio_wav_t), wav_file);
  fseek (wav_file, 0, SEEK_END);

  // TODO: multiple ROMs in a single wav
#if 1
  source_fname = fname;
#else
  while (source_fname) // multiple files
#endif
    {
      if (!(bin_file = fopen (ucon64.fname, "rb")))
        {
          printf ("ERROR: unable to open BIN file %s\n", ucon64.fname);

          fclose (wav_file);

          return -1;
        }

#if 0
      // TODO: custom bsmode and re-init if multiple files
      if (bsmode != 0 && bsmode != atari_rominfo.bsm)
        {
        }
      else
        {
          ucon64.fname ...
          atari_open ()
        }

      // multiple ROMs in a single wav
      atari_rominfo.multi_byte = multi;
#endif

      init_bank = 0;
      if (atari_rominfo.bsm == BSM_3F)
        {
          if (ucon64.file_size == 0x800)
            init_bank = 2;
          else if (ucon64.file_size == 0x1000)
            init_bank = 1;
        }
      else if (atari_rominfo.bsm == BSM_MB)
        {
          printf ("Forcing blank pages to be transferred for bankswitch mode MB\n");
          force_empty = 1;
        }
  
        // write game header tone, just a series of alternating zero and one bits
      write_byte_as_wav (wav_file, 0x55,
        ((int) (WAV_HEADER_SECONDS * ((int) (44100 / (WAV_ONEBIT_LEN + WAV_ZEROBIT_LEN)) / 4))) - 1);

      // two zero bits in a row indicate the beginning of the data
      write_byte_as_wav (wav_file, 0x54, 1);

      write_byte_as_wav (wav_file, atari_rominfo.start_low, 1);
      write_byte_as_wav (wav_file, atari_rominfo.start_hi, 1);
      write_byte_as_wav (wav_file, atari_rominfo.ctrl_byte, 1);
      // # of pages to load
      write_byte_as_wav (wav_file, (unsigned char) (atari_rominfo.game_page_count), 1);
      // game header checksum -- first 8 bytes must add up to 0x55
      write_byte_as_wav (wav_file, (unsigned char) (0x55 -
                                                    atari_rominfo.start_low -
                                                    atari_rominfo.start_hi -
                                                    atari_rominfo.multi_byte -
                                                    atari_rominfo.ctrl_byte -
                                                    atari_rominfo.game_page_count -
                                                    atari_rominfo.speed_low -
                                                    atari_rominfo.speed_hi), 1);
      write_byte_as_wav (wav_file, atari_rominfo.multi_byte, 1);
      write_byte_as_wav (wav_file, atari_rominfo.speed_low, 1);
      write_byte_as_wav (wav_file, atari_rominfo.speed_hi, 1);

      printf ("Converting to (44100 Hz, 1 ch, 8 bit) WAV format with a %0.2f second header tone\n\n",
              WAV_HEADER_SECONDS);

      for (page = 0; page < ucon64.file_size / 0x100; page++)
        if (!atari_rominfo.empty_page[page] || force_empty)
          {
            ucon64_fread (buffer, page * 0x100, 0x100, ucon64.fname);
            ucon64_gauge (start_time, page * 0x100, atari_rominfo.game_page_count * 0x100);
  
            // If we just got the page, put the page header which consists of two bytes. The first byte
            // is a counter that begins at zero for the first page and is incremented by 4 for each     
            // subsequent page.  If the value is greater than 0x1f, then 0x1f is subtracted from it.    
            if (atari_rominfo.bsm == BSM_3F)
              pg_bank_byte = atari_rominfo.page_list[page];
            else
              pg_bank_byte =
                (page % 8) * 4 + init_bank +
                (int) ((page - (int) (page / 32) * 32) / 8) +
                (int) (page / 32) * 32;
  
            // Get the sum of all 256 bytes of the current page
            for (i = j = 0; i < 0x100; i++)
              j += (char) buffer[i];
  
            // The second byte of the page header is 0x55 - the first byte - the sum of the 256 bytes of 
            // program data.
            in_byte = (unsigned char) (0x55 - pg_bank_byte - j);
  
#ifdef  DEBUG
            fprintf (stderr, "bank %2.2d\n"
                             "page %2.2d\n"
                             "page&bank byte %2.2x\n"
                             "checksum %2.2x\n",
                             (int) (pg_bank_byte & 0x3) +
                             (int) (pg_bank_byte & 0xE0) / 8,
                             (int) ((pg_bank_byte & 0x1c) / 4),
                             (unsigned char) pg_bank_byte,
                             (unsigned char) in_byte);
#endif
            write_byte_as_wav (wav_file, pg_bank_byte, 1);
            write_byte_as_wav (wav_file, in_byte, 1);
            for (i = 0; i < 0x100; i++)
              write_byte_as_wav (wav_file, buffer[i], 1);
          }

      fclose (bin_file);

      ucon64_gauge (start_time, atari_rominfo.game_page_count * 0x100, atari_rominfo.game_page_count * 0x100);

      // put a tone at the end of the game (not really necessary)
#if 0
      // TODO: multiple ROMs in a single wav
      if (multi) // multiple files are separated by a new header
        write_byte_as_wav (wav_file, 0x55,
          WAV_HEADER_SECONDS * ((int) (44100 / (WAV_ONEBIT_LEN + WAV_ZEROBIT_LEN)) / 4) * 2 - 1);
      else
#endif
        write_byte_as_wav (wav_file, 0x55,
          ((int) (44100 / (WAV_ONEBIT_LEN + WAV_ZEROBIT_LEN)) / 4) / 2 - 1);
    }
              

  // rewrite wav header with real data length
  fseek (wav_file, 0, SEEK_SET);
#warning
#if 0
  ucon64_write_wavheader (wav_file,
                          1,
                          44100,
                          8,
                          fsizeof (dest_name) - sizeof (st_audio_wav_t));
#endif
  fclose (wav_file);

  puts ("\n");
  printf (ucon64_msg[WROTE], dest_name);

  return 0;
}


int
atari_bin (const char *fname)
{
  // TODO: turn wav back to bin
  (void) fname;
  return 0;
}
#endif  // HAVE_MATH_H

