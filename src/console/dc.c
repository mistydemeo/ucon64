/*
dc.c - Dreamcast support for uCON64

Copyright (c) 2004 NoisyB <noisyb@gmx.net>


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
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "dc.h"


const st_getopt2_t dc_usage[] = {
    {
      NULL, 0, 0, 0,
      NULL, "Dreamcast" /* "1998 SEGA http://www.sega.com" */,
      NULL
    },
    {
      "dc", 0, 0, UCON64_DC,
      NULL, "force recognition",
      (void *) (UCON64_DC|WF_SWITCH)
    },
#if 0
    {
      "vms", 1, 0, UCON64_VMS,
      "SAV", "convert NES SAV file to a VMS file for use with NesterDC",
      NULL
    },
#endif
    {
      "scr", 0, 0, UCON64_SCR,
      NULL, "scramble 1ST_READ.BIN for selfboot CDs",
      (void *) (UCON64_DC|WF_DEFAULT)
    },
    {
      "unscr", 0, 0, UCON64_UNSCR,
      NULL, "unscramble 1ST_READ.BIN for non-selfboot CDs",
      (void *) (UCON64_DC|WF_DEFAULT)
    },
#if 0
    {
      "ip", 1, 0, UCON64_IP,
      "FILE", "extract ip.bin FILE from IMAGE; " OPTION_LONG_S "rom=IMAGE",
      NULL
    },
#endif
    {
      "mkip", 0, 0, UCON64_MKIP,
      NULL, "generate IP.BIN file with default values",
      (void *) (UCON64_DC|WF_NO_ROM)
    },
    {
      "parse", 1, 0, UCON64_PARSE,
      "TEMPLATE", "parse TEMPLATE file into a IP.BIN;\n"
      "creates an empty template when TEMPLATE does not exist",
      (void *) (UCON64_DC|WF_NO_ROM)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


static int
calc_crc (const unsigned char *buf, int size)
{
  int i, c, n = 0xffff;
  for (i = 0; i < size; i++)
    {
      n ^= (buf[i] << 8);
      for (c = 0; c < 8; c++)
        if (n & 0x8000)
          n = (n << 1) ^ 4129;
        else
          n = (n << 1);
    }
  return n & 0xffff;
}


static void
update_crc (char *ip)
{
  int n = calc_crc ((unsigned char *) (ip + 0x40), 16);
  char buf[5];
  sprintf (buf, "%04X", n);
  if (memcmp (buf, ip + 0x20, 4))
    {
      printf ("Setting CRC to %s (was %.4s)\n", buf, ip + 0x20);
      memcpy (ip + 0x20, buf, 4);
    }
}


static unsigned int seed;

static void
dc_srand (unsigned int n)
{
  seed = n & 0xffff;
}


static unsigned int
dc_rand ()
{
  seed = (seed * 2109 + 9273) & 0x7fff;
  return (seed + 0xc000) & 0xffff;
}


#if 0
// header for SAV -> VMS conversion 
static const uint8_t nstrsave_bin[1024] = {
  0x4e, 0x45, 0x53, 0x52, 0x4f, 0x4d, 0x2e, 0x4e, // 0x8 (8)
  0x45, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x10 (16)
  0x4e, 0x65, 0x73, 0x74, 0x65, 0x72, 0x44, 0x43, // 0x18 (24)
  0x20, 0x53, 0x61, 0x76, 0x65, 0x52, 0x61, 0x6d, // 0x20 (32)
  0x20, 0x46, 0x69, 0x6c, 0x65, 0x00, 0x00, 0x00, // 0x28 (40)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x30 (48)
  0x2a, 0x2a, 0x2a, 0x4e, 0x65, 0x73, 0x74, 0x65, // 0x38 (56)
  0x72, 0x44, 0x43, 0x2a, 0x2a, 0x2a, 0x00, 0x00, // 0x40 (64)
  0x01, 0x00, 0x00, 0x00, 0x01, 0x3f, 0x00, 0x00, // 0x48 (72)
  0x80, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x50 (80)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x58 (88)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x60 (96)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x68 (104)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x70 (112)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x78 (120)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 (128)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x88 (136)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 (144)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x98 (152)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0 (160)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa8 (168)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb0 (176)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb8 (184)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xc0 (192)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xc8 (200)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xd0 (208)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xd8 (216)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe0 (224)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe8 (232)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf0 (240)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf8 (248)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x100 (256)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x108 (264)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x110 (272)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x118 (280)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x120 (288)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x128 (296)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x130 (304)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x138 (312)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x140 (320)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x148 (328)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x150 (336)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x158 (344)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x160 (352)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x168 (360)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x170 (368)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x178 (376)
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, // 0x180 (384)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x188 (392)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x190 (400)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x198 (408)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1a0 (416)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1a8 (424)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1b0 (432)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1b8 (440)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1c0 (448)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1c8 (456)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1d0 (464)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1d8 (472)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1e0 (480)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1e8 (488)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1f0 (496)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x1f8 (504)
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // 0x200 (512)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x208 (520)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x210 (528)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x218 (536)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x220 (544)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x228 (552)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x230 (560)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x238 (568)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x240 (576)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x248 (584)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x250 (592)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x258 (600)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x260 (608)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x268 (616)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x270 (624)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x278 (632)
  0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, // 0x280 (640)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x288 (648)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x290 (656)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x298 (664)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2a0 (672)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2a8 (680)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2b0 (688)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2b8 (696)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2c0 (704)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2c8 (712)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2d0 (720)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2d8 (728)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2e0 (736)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2e8 (744)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2f0 (752)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x2f8 (760)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x300 (768)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x308 (776)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x310 (784)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x318 (792)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x320 (800)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x328 (808)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x330 (816)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x338 (824)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x340 (832)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x348 (840)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x350 (848)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x358 (856)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x360 (864)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x368 (872)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x370 (880)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x378 (888)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x380 (896)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x388 (904)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x390 (912)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x398 (920)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3a0 (928)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3a8 (936)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3b0 (944)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3b8 (952)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3c0 (960)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3c8 (968)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3d0 (976)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3d8 (984)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3e0 (992)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3e8 (1000)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3f0 (1008)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x3f8 (1016)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // 0x400 (1024)
};
#endif


int
dc_init (st_rominfo_t *rominfo)
{
  int result = -1;
  rominfo->console_usage = dc_usage;

  return result;
}


static int
check_areasym (char *ptr, int len)
{
  int i, a = 0;
  for (i = 0; i < len; i++)
    switch (ptr[i])
      {
      case 'J':
        a |= (1<<0);
        break;
      case 'U':
        a |= (1<<1);
        break;
      case 'E':
        a |= (1<<2);
        break;
      case ' ':
        break;
      default:
        fprintf (stderr, "ERROR: Unknown area symbol '%c'\n", ptr[i]);
        return -1;
      }
  for (i = 0; i < len; i++)
    if ((a & (1<<i)) == 0)
      ptr[i] = ' ';
    else
      ptr[i] = "JUE"[i];

  return 0;
}


typedef struct
{
  char *name;
  int pos;
  int len;
  int (*extra_check) (char *, int);
  char *def;
  char *comment;
} st_templ_t;


st_templ_t templ[] = {
  {"hardware_id",   0x0, 0x10,  NULL,          "SEGA SEGAKATANA",        "Hardware ID (always \"SEGA SEGAKATANA\")"},
  {"maker_id",      0x10, 0x10, NULL,          "SEGA ENTERPRISES",       "Maker ID (always \"SEGA ENTERPRISES\")"},
  {"device_info",   0x20, 0x10, NULL,          "0000 CD-ROM1/1",         "Device Information"},
  {"area_symbols",  0x30, 0x8,  check_areasym, "JUE",                    "Area Symbols"},
  {"peripherals",   0x38, 0x8,  NULL,          "E000F10",                "Peripherals"},
  {"product_no",    0x40, 0xa,  NULL,          "T0000",                  "Product number (\"HDR-nnnn\" etc.)"},
  {"version",       0x4a, 0x6,  NULL,          "V1.000",                 "Product version"},
  {"release_date",  0x50, 0x10, NULL,          "20000627",               "Release date (YYYYMMDD)"},
  {"boot_filename", 0x60, 0x10, NULL,          "1ST_READ.BIN",           "Boot filename (usually \"1ST_READ.BIN\")"},
  {"sw_maker_name", 0x70, 0x10, NULL,          "YOUR NAME HERE",         "Name of the company that produced the disc"},
  {"game_title",    0x80, 0x80, NULL,          "TITLE OF THE SOFTWARE",  "Name of the software"},
  {NULL,            0,    0,    NULL,          NULL,                     NULL}
};


static int
parse_templ (const char *templ_file, char *ip)
{
  int filled_in[MAXBUFSIZE];
  static char buf[MAXBUFSIZE];
  int i;

  memset (filled_in, 0, sizeof (filled_in));
  for (i = 0; templ[i].name; i++)
    {
      char *p = buf;
      get_property (templ_file, templ[i].name, p, templ[i].def);

      strtrim (p);

      if (!(*p))
        continue;

      memset (ip + templ[i].pos, ' ', templ[i].len);

      if ((int) strlen (p) > templ[i].len)
        {
          fprintf (stderr, "ERROR: Data for field \"%s\" is too long... stripping to %d chars\n",
                   templ[i].name, templ[i].len);
          p[templ[i].len] = 0;
        }

      memcpy (ip + templ[i].pos, p, strlen (p));

      if (templ[i].extra_check)
        if (templ[i].extra_check (ip + templ[i].pos, templ[i].len) == -1)
          return -1;

      filled_in[i] = 1;
    }

  for (i = 0; templ[i].name; i++)
    if (!filled_in[i])
      {
        fprintf (stderr, "ERROR: Missing value for \"%s\"\n", templ[i].name);
        return -1;
      }

  return 0;
}


int
dc_parse (const char *templ_file)
{
  char ip[0x8000], dest_name[FILENAME_MAX];

  if (access (templ_file, F_OK) == -1)
    {
      int i = 0;

      printf ("Creating empty template file: \"%s\"\n", templ_file);

      for (i = 0; templ[i].name; i++)
        set_property (templ_file, templ[i].name, templ[i].def, templ[i].comment);

      printf (ucon64_msg[WROTE], templ_file);
    }

  if (parse_templ (templ_file, ip) == -1)
    return -1;

  update_crc (ip);

  strcpy (dest_name, "ip.bin");
  ucon64_file_handler (dest_name, NULL, 0);

  q_fwrite (ip, 0, 0x8000, dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
dc_mkip (void)
{
  dc_parse ("default");

  return 0;
}


static int
load_chunk (FILE * fh, unsigned char *ptr, int32_t sz)
{
#define MAXCHUNK (2048*1024)
  static int idx[MAXCHUNK / 32];
  int32_t i;

  /* Convert chunk size to number of slices */
  sz /= 32;

  /* Initialize index table with unity,
     so that each slice gets loaded exactly once */
  for (i = 0; i < sz; i++)
    idx[i] = i;

  for (i = sz - 1; i >= 0; --i)
    {
      /* Select a replacement index */
      int x = (dc_rand () * i) >> 16;

      /* Swap */
      int tmp = idx[i];
      idx[i] = idx[x];
      idx[x] = tmp;

      /* Load resulting slice */
      if (fread (ptr + 32 * idx[i], 1, 32, fh) != 32)
        return -1;
    }
  return 0;
}


static int
load_file (FILE * fh, unsigned char *ptr, uint32_t filesz)
{
  uint32_t chunksz;

  dc_srand (filesz);

  /* Descramble 2 meg blocks for as long as possible, then
     gradually reduce the window down to 32 bytes (1 slice) */
  for (chunksz = MAXCHUNK; chunksz >= 32; chunksz >>= 1)
    while (filesz >= chunksz)
      {
        load_chunk (fh, ptr, chunksz);
        filesz -= chunksz;
        ptr += chunksz;
      }

  /* Load final incomplete slice */
  if (filesz)
    if (fread (ptr, 1, filesz, fh) != filesz)
      return -1;

  return 0;
}


static int
save_chunk (FILE * fh, unsigned char *ptr, int32_t sz)
{
  static int idx[MAXCHUNK / 32];
  int32_t i;

  /* Convert chunk size to number of slices */
  sz /= 32;

  /* Initialize index table with unity,
     so that each slice gets saved exactly once */
  for (i = 0; i < sz; i++)
    idx[i] = i;

  for (i = sz - 1; i >= 0; --i)
    {
      /* Select a replacement index */
      int x = (dc_rand () * i) >> 16;

      /* Swap */
      int tmp = idx[i];
      idx[i] = idx[x];
      idx[x] = tmp;

      /* Save resulting slice */
      if (fwrite (ptr + 32 * idx[i], 1, 32, fh) != 32)
        return -1;
    }

  return 0;
}


static int
save_file (FILE * fh, unsigned char *ptr, uint32_t filesz)
{
  uint32_t chunksz;

  dc_srand (filesz);

  /* Descramble 2 meg blocks for as long as possible, then
     gradually reduce the window down to 32 bytes (1 slice) */
  for (chunksz = MAXCHUNK; chunksz >= 32; chunksz >>= 1)
    while (filesz >= chunksz)
      {
        save_chunk (fh, ptr, chunksz);
        filesz -= chunksz;
        ptr += chunksz;
      }

  /* Save final incomplete slice */
  if (filesz)
    if (fwrite (ptr, 1, filesz, fh) != filesz)
      return -1;

  return 0;
}


static int
descramble (const char *src, char *dst)
{
  unsigned char *ptr = NULL;
  uint32_t sz = 0;
  FILE *fh;

  if (!(fh = fopen (src, "rb")))
    return -1;

  sz = q_fsize (src);
  if (!(ptr = (unsigned char *) malloc (sz)))
    return -1;

  load_file (fh, ptr, sz);
  fclose (fh);

  if (!(fh = fopen (dst, "wb")))
    return -1;

  if (fwrite (ptr, 1, sz, fh) != sz)
    return -1;

  fclose (fh);
  free (ptr);
  return 0;
}


static int
scramble (const char *src, char *dst)
{
  unsigned char *ptr = NULL;
  uint32_t sz = 0;
  FILE *fh;

  if (!(fh = fopen (src, "rb")))
    return -1;

  sz = q_fsize (src);

  if (!(ptr = (unsigned char *) malloc (sz)))
    return -1;

  if (fread (ptr, 1, sz, fh) != sz)
    return -1;

  fclose (fh);

  if (!(fh == fopen (dst, "wb")))
    return -1;
  save_file (fh, ptr, sz);

  fclose (fh);

  free (ptr);
  return 0;
}


int
dc_scramble (void)
{
  char dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, NULL, 0);
                        
  if (!scramble (ucon64.rom, dest_name))
    printf (ucon64_msg[WROTE], dest_name);
  else
    fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name);
  return 0;
}


int
dc_unscramble (void)
{
  char dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, NULL, 0);
                        
  if (!descramble (ucon64.rom, dest_name))
    printf (ucon64_msg[WROTE], dest_name);
  else
    fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name);
  return 0;
}
