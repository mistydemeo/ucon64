/*
gba.c - Game Boy Advance support for uCON64

written by 2001        NoisyB (noisyb@gmx.net)
           2001 - 2004 dbjh


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
#include <sys/stat.h>
#include "misc.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "gba.h"
#include "patch/ips.h"
#include "backup/fal.h"


#define GBA_NAME_LEN 12
#define GBA_HEADER_START 0
#define GBA_HEADER_LEN (sizeof (st_gba_header_t))


static int gba_chksum (void);
static int gbautil (const char *filein, const char *fileout);

const st_usage_t gba_usage[] =
  {
    {NULL, 0, NULL, "Game Boy Advance", "2001 Nintendo http://www.nintendo.com"},
    {"gba", 0, NULL, "force recognition", NULL},
    {"n", 1, "NEW_NAME", "change internal ROM name to NEW_NAME", NULL},
    {"logo", 0, NULL, "restore ROM logo character data (offset: 0x04-0x9F)", NULL},
    {"chk", 0, NULL, "fix ROM header checksum", NULL},
    {"sram", 0, NULL, "patch ROM for SRAM saving", NULL},
    {"crp", 1, "WAIT_TIME", "slow down Flash Advance Linker access for ROM (crash patch);\n"
                         "WAIT_TIME=0  default in most crash patches\n"
                         "WAIT_TIME=4  faster than 0, slower than 8\n"
                         "WAIT_TIME=8  faster than 4, slower than 28\n"
                         "WAIT_TIME=12 slowest cartridge access speed\n"
                         "WAIT_TIME=16 faster than 28, but slower than 20\n"
                         "WAIT_TIME=20 default in most original carts\n"
                         "WAIT_TIME=24 fastest cartridge access speed\n"
                         "WAIT_TIME=28 faster than 8 but slower than 16", NULL},
//  "n 0 and 28, with a stepping of 4. I.e. 0, 4, 8, 12 ...\n"
    {"multi", 1, "SIZE", "make multirom for Flash Advance Linker, truncated to SIZE Mbit;\n"
                      "file with loader must be specified first, then all the ROMs,\n"
                      "multirom to create last; use -o to specify output directory", NULL},
    {NULL, 0, NULL, NULL, NULL}
  };


/*
Offset 00h-03h - Start  Address - A 32 bit ARM B command with jump destination
                 to  the  start  address of the program, cannot be manipulated
                 with this tool, there's no reason.

Offset 04h-9fh - Nintendo logo character data - The fix Nintendo logo graphics
                 needed  to  start a ROM on the real machine as it is verified
                 by it.

Offset a0h-abh - Game  title  -  The game title is an ASCII string, officially
                 can  use only ASCII characters between the ASCII code 20h and
                 60h.  Although it is not a strict rule for hobby programmers,
                 it  is  fun  to  follow such a rules in my opinion. As I know
                 developers  can  choose  their own game title here describing
                 the product in short.

Offset ach-afh - Game  code  -  The  4 bytes long code of the game is an ASCII
                 string  too, officially can use only ASCII characters between
                 the ASCII code 20h and 60h. The first letter is always A as I
                 know,  probably  stands  for GBA, so it won't change unless a
                 higher   hardware   with  backwards  compatibility  won't  be
                 introduced  and  this letter could hold some more infos about
                 it. The second and third letters are the shortened version of
                 the  name of the game. And the fourth letter is the territory
                 code. Don't afraid, there's no territory lockout, this is for
                 information  purposes  only.  So  far  as I know J stands for
                 Japan  and  Asia,  E  stands  for  USA and the whole American
                 continent  and  P  stands  for  Europe,  Australia and Africa
                 (probably  came  from  that  these are the PAL video standard
                 territories,  but  I  could  be  wrong). Although it is not a
                 strict rule for hobby programmers, it is fun to follow such a
                 rules  in my opinion. Developers get this 4 letter code right
                 from Nintendo and they have to use that.

Offset b0h-b1h - Maker  code  - The 2 bytes long code of the developer company
                 is  an  ASCII  string  too,  officially  can  use  only ASCII
                 characters between the ASCII code 20h and 60h. Although it is
                 not  a strict rule for hobby programmers, it is fun to follow
                 such a rules in my opinion. Developers get this 2 letter code
                 right from Nintendo and they have to use that.

Offset b2h-b2h - 96h - Fixed 96h byte without any useful information.

Offset b3h-b3h - Main  unit  code  -  This hexadecimal byte is the destination
                 hardware  code.  It  is always 00h at the moment as it stands
                 for  Game Boy Advance, so it won't change in the future either
                 unless  a  higher hardware with backwards compatibility won't
                 be  introduced and this byte could hold some more infos about
                 it.  There's  no  reason  to  change  this or write something
                 different than 00h into it.

Offset b4h-b4h - Device type -  This hexadecimal byte is the device type code.
                 It  is always 00h as the only other possible value stands for
                 a  debugger  cart  what  I  assume  won't be available on the
                 streets  and  I  assume even if a developer works with such a
                 hardware, he or she doesn't have to change this byte, however
                 he  or  she  easily  can  of  course. So there's no reason to
                 change this or write something different than 00h into it.

Offset b5h-bbh - Reserved  area  -  Fixed,  00h filled area without any useful
                 information.

Offset bch-bch - Mask  ROM  version  number  - This hexadecimal byte holds the
                 version  number  of  the ROM. As I know it works somehow that
                 way,  the  first  published  (and released on the streets) is
                 always  the first version and for that 00h is stored here. In
                 the  case it is getting updated, so in the same territory the
                 very  same  game with the very same title is getting replaced
                 with a new version, what is happening rarely, the number here
                 is  getting  increased by one. So usually this byte holds 00h
                 and  there  isn't too much reason to write something here and
                 something else than 00h.

Offset bdh-bdh - Complement   check   -  This  hexadecimal  byte  have  to  be
                 calculated  automatically,  when  the  whole header is in its
                 final  state,  so nothing will change inside of it. (Manually
                 it  would be hard to calculate.) Add the bytes between offset
                 a0h  and bch together, take the number's two's complement and
                 add  19h  to  the  result.  Store  the lowest 8 bits here. Or
                 calculate   automatically   with   GBARM.   The  hardware  is
                 verifying  this  byte  just  like the Nintendo logo character
                 data  and  in the case it isn't correct, the game won't start
                 on the real machine.

Offset beh-bfh - Reserved  area  -  Fixed,  00h filled area without any useful
                 information.
*/
typedef struct st_gba_header
{
  unsigned char start[4];                       // 0x00
  unsigned char logo[GBA_LOGODATA_LEN];         // 0x04
  unsigned char name[GBA_NAME_LEN];             // 0xa0
  unsigned char game_id_prefix;                 // 0xac
  unsigned char game_id_low;                    // 0xad
  unsigned char game_id_high;                   // 0xae
  unsigned char game_id_country;                // 0xaf
  unsigned char maker_high;                     // 0xb0
  unsigned char maker_low;                      // 0xb1
  unsigned char pad1;
  unsigned char gba_type;                       // 0xb3
  unsigned char device_type;                    // 0xb4
  unsigned char pad2[7];
  unsigned char version;                        // 0xbc
  unsigned char checksum;                       // 0xbd
  unsigned char pad3[2];
} st_gba_header_t;

static st_gba_header_t gba_header;
const unsigned char gba_logodata[] = {          // Note: not a static variable
  0x24, 0xff, 0xae, 0x51,
  0x69, 0x9a, 0xa2, 0x21, 0x3d, 0x84, 0x82, 0x0a,
  0x84, 0xe4, 0x09, 0xad, 0x11, 0x24, 0x8b, 0x98,
  0xc0, 0x81, 0x7f, 0x21, 0xa3, 0x52, 0xbe, 0x19,
  0x93, 0x09, 0xce, 0x20, 0x10, 0x46, 0x4a, 0x4a,
  0xf8, 0x27, 0x31, 0xec, 0x58, 0xc7, 0xe8, 0x33,
  0x82, 0xe3, 0xce, 0xbf, 0x85, 0xf4, 0xdf, 0x94,
  0xce, 0x4b, 0x09, 0xc1, 0x94, 0x56, 0x8a, 0xc0,
  0x13, 0x72, 0xa7, 0xfc, 0x9f, 0x84, 0x4d, 0x73,
  0xa3, 0xca, 0x9a, 0x61, 0x58, 0x97, 0xa3, 0x27,
  0xfc, 0x03, 0x98, 0x76, 0x23, 0x1d, 0xc7, 0x61,
  0x03, 0x04, 0xae, 0x56, 0xbf, 0x38, 0x84, 0x00,
  0x40, 0xa7, 0x0e, 0xfd, 0xff, 0x52, 0xfe, 0x03,
  0x6f, 0x95, 0x30, 0xf1, 0x97, 0xfb, 0xc0, 0x85,
  0x60, 0xd6, 0x80, 0x25, 0xa9, 0x63, 0xbe, 0x03,
  0x01, 0x4e, 0x38, 0xe2, 0xf9, 0xa2, 0x34, 0xff,
  0xbb, 0x3e, 0x03, 0x44, 0x78, 0x00, 0x90, 0xcb,
  0x88, 0x11, 0x3a, 0x94, 0x65, 0xc0, 0x7c, 0x63,
  0x87, 0xf0, 0x3c, 0xaf, 0xd6, 0x25, 0xe4, 0x8b,
  0x38, 0x0a, 0xac, 0x72, 0x21, 0xd4, 0xf8, 0x07
};


int
gba_n (st_rominfo_t *rominfo, const char *name)
{
  char buf[GBA_NAME_LEN], dest_name[FILENAME_MAX];

  memset (buf, 0, GBA_NAME_LEN);
  strncpy (buf, name, GBA_NAME_LEN);
  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, NULL, 0);
  q_fcpy (ucon64.rom, 0, ucon64.file_size, dest_name, "wb");
  q_fwrite (buf, GBA_HEADER_START + rominfo->buheader_len + 0xa0, GBA_NAME_LEN,
            dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gba_logo (st_rominfo_t *rominfo)
{
  char dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, NULL, 0);
  q_fcpy (ucon64.rom, 0, ucon64.file_size, dest_name, "wb");
  q_fwrite (gba_logodata, GBA_HEADER_START + rominfo->buheader_len + 0x04,
            GBA_LOGODATA_LEN, dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gba_chk (st_rominfo_t *rominfo)
{
  char buf, dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, NULL, 0);
  q_fcpy (ucon64.rom, 0, ucon64.file_size, dest_name, "wb");

  buf = rominfo->current_internal_crc;
  q_fputc (dest_name, GBA_HEADER_START + rominfo->buheader_len + 0xbd,
    buf, "r+b");

  mem_hexdump (&buf, 1, GBA_HEADER_START + rominfo->buheader_len + 0xbd);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gba_sram (void)
{
  char buf[MAXBUFSIZE], dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, NULL, 0);
  q_fcpy (ucon64.rom, 0, ucon64.file_size, dest_name, "wb");

  strcpy (buf, dest_name);
  set_suffix (buf, ".TMP");
  rename (dest_name, buf);

  gbautil ((const char *) buf, (const char *) dest_name);

  if (access (dest_name, F_OK) != 0)
    rename (buf, dest_name);
  else
    remove (buf);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
gba_crp (st_rominfo_t *rominfo, const char *value)
{
  FILE *srcfile, *destfile;
  int bytesread, n = 0;
  char buffer[32 * 1024], backup_name[FILENAME_MAX], replace[2],
       wait_time = atoi (value);

  if (wait_time % 4 != 0 || wait_time > 28 || wait_time < 0)
    {
      fprintf (stderr, "ERROR: You specified a wrong WAIT_TIME value\n");
      return -1;
    }

  puts ("Applying crash patch...");

  strcpy (backup_name, ucon64.rom);
  set_suffix (backup_name, ".BAK");
  rename (ucon64.rom, backup_name);
  if ((srcfile = fopen (backup_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], backup_name);
      return -1;
    }
  if ((destfile = fopen (ucon64.rom, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], ucon64.rom);
      return -1;
    }
  if (rominfo->buheader_len)        // copy header (if present)
    {
      fread (buffer, 1, rominfo->buheader_len, srcfile);
      fwrite (buffer, 1, rominfo->buheader_len, destfile);
    }

  replace[0] = wait_time;
  replace[1] = 0x40;
  while ((bytesread = fread (buffer, 1, 32 * 1024, srcfile)))
    {                           // '!' == ASCII 33 (\x21), '*' == 42 (\x2a)
      n += change_mem (buffer, bytesread, "\x04\x02\x00\x04\x14\x40", 6, '*', '!', replace, 1, -1);
      n += change_mem (buffer, bytesread, "\x02\x00\x04\x14\x40\x00", 6, '*', '!', replace, 1, -2);
      n += change_mem (buffer, bytesread, "\x04\x02\x00\x04\xB4\x45", 6, '*', '!', replace, 2, -1);
      n += change_mem (buffer, bytesread, "\x3E\xE0\x00\x00\xB4\x45", 6, '*', '!', replace, 2, -1);
      n += change_mem (buffer, bytesread, "\x04\x02\x00\x04\x94\x44", 6, '*', '!', replace, 2, -1);

      fwrite (buffer, 1, bytesread, destfile);
    }
  fclose (srcfile);
  fclose (destfile);

  printf ("Found %d pattern%s\n", n, n != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], ucon64.rom);
  return 0;
}


int
gba_init (st_rominfo_t *rominfo)
{
  int result = -1, value;
  char buf[MAXBUFSIZE];

  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
    ucon64.buheader_len : 0;

  q_fread (&gba_header, GBA_HEADER_START +
           rominfo->buheader_len, GBA_HEADER_LEN, ucon64.rom);
  if (gba_header.game_id_prefix == 'A' && gba_header.gba_type == 0)
    result = 0;
  else
    {
#if 0 // AFAIK (dbjh) GBA ROMs never have a header
      rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
        ucon64.buheader_len : UNKNOWN_HEADER_LEN;

      q_fread (&gba_header, GBA_HEADER_START +
               rominfo->buheader_len, GBA_HEADER_LEN, ucon64.rom);
      if (gba_header.game_id_prefix == 'A' && gba_header.gba_type == 0)
        result = 0;
      else
#endif
        result = -1;
    }
  if (ucon64.console == UCON64_GBA)
    result = 0;

  rominfo->header_start = GBA_HEADER_START;
  rominfo->header_len = GBA_HEADER_LEN;
  rominfo->header = &gba_header;

  // internal ROM name
  strncpy (rominfo->name, (const char *) gba_header.name, GBA_NAME_LEN);
  rominfo->name[GBA_NAME_LEN] = 0;

  // ROM maker
  {
    int ih = gba_header.maker_high <= '9' ?
               gba_header.maker_high - '0' : gba_header.maker_high - 'A' + 10,
        il = gba_header.maker_low <= '9' ?
               gba_header.maker_low - '0' : gba_header.maker_low - 'A' + 10;
    value = ih * 36 + il;
  }
  if (value < 0 || value >= NINTENDO_MAKER_LEN)
    value = 0;
  rominfo->maker = NULL_TO_UNKNOWN_S (nintendo_maker[value]);

  // ROM country
  rominfo->country =
    (gba_header.game_id_country == 'J') ? "Japan/Asia" :
    (gba_header.game_id_country == 'E') ? "U.S.A." :
    (gba_header.game_id_country == 'P') ? "Europe, Australia and Africa" :
    "Unknown country";

  // misc stuff
  sprintf (buf, "Version: %02x\n", gba_header.version);
  strcat (rominfo->misc, buf);

  sprintf (buf, "Device type: %02x\n", gba_header.device_type);
  strcat (rominfo->misc, buf);

  value = gba_header.start[0] << 24 |
          gba_header.start[1] << 16 |
          gba_header.start[2] << 8 |
          gba_header.start[3];
  sprintf (buf, "Start address: %08x\n", value);
  strcat (rominfo->misc, buf);

  strcat (rominfo->misc, "Logo data: ");
  if (memcmp (gba_header.logo, gba_logodata, GBA_LOGODATA_LEN) == 0)
    {
#ifdef  USE_ANSI_COLOR
      if (ucon64.ansi_color)
        strcat (rominfo->misc, "\x1b[01;32mOk\x1b[0m");
      else
#endif
        strcat (rominfo->misc, "Ok");
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

  // internal ROM crc
  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      rominfo->has_internal_crc = 1;
      rominfo->internal_crc_len = 1;
      rominfo->current_internal_crc = gba_chksum ();

      rominfo->internal_crc = gba_header.checksum;
      rominfo->internal_crc2[0] = 0;
    }

  rominfo->console_usage = gba_usage;
  // We use fal_usage, but we could just as well use f2a_usage
  rominfo->copier_usage = (!rominfo->buheader_len ? fal_usage : unknown_usage);

  return result;
}


int
gba_chksum (void)
// Note that this function only calculates the checksum of the internal header
{
  unsigned char sum = 0x19, *ptr = (unsigned char *) &gba_header + 0xa0;

  while (ptr < (unsigned char *) &gba_header + 0xbd)
    sum += *ptr++;
  sum = -sum;

  return sum;
}


int
gba_multi (int truncate_size, char *multi_fname)
// TODO: Check if 1024 Mbit multiroms are supported by the FAL code
{
#define BUFSIZE (32 * 1024)
  int n, n_files, file_no, bytestowrite, byteswritten, totalsize = 0, done,
      truncated = 0, size_pow2_lesser = 1, size_pow2 = 1, truncate_size_ispow2 = 0;
  struct stat fstate;
  FILE *srcfile, *destfile;
  char buffer[BUFSIZE], *destname, *fname, loader_fname[FILENAME_MAX];


  if (truncate_size == 0)
    {
      fprintf (stderr, "ERROR: Can't make multirom of 0 bytes\n");
      return -1;
    }

#if 0
  if (truncate_size != 64 * MBIT && truncate_size != 128 * MBIT &&
      truncate_size != 256 * MBIT && truncate_size != 512 * MBIT &&
      truncate_size != 1024 * MBIT)
    {
      fprintf (stderr, "ERROR: Truncate size must be 64, 128, 256, 512 or 1024\n");
      return -1;
    }
#endif

  if (multi_fname != NULL)                      // -xfalmulti
    {
      destname = multi_fname;
      n_files = ucon64.argc;
    }
  else                                          // -multi
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
  printf ("Creating multirom file: %s\n", destname);

  file_no = 0;
  for (n = 1; n < n_files; n++)
    {
      if (access (ucon64.argv[n], F_OK))
        continue;                               // "file" does not exist (option)
      stat (ucon64.argv[n], &fstate);
      if (!S_ISREG (fstate.st_mode))
        continue;

      if (file_no == 0)
        {
          if (multi_fname != NULL)              // -xfalmulti
            {
              get_property_fname (ucon64.configfile, "gbaloader", loader_fname,
                                  "loader.bin");
              if (access (loader_fname, F_OK))
                {
                  fprintf (stderr, "ERROR: Cannot open loader binary (%s)\n",
                           loader_fname);
                  return -1;
                }
              fname = loader_fname;
              // NOTE: loop counter is modified, because we have to insert
              //       loader in the file list
              n--;
            }
          else                                  // -multi
            fname = ucon64.argv[n];

          printf ("Loader: %s\n", fname);
          if (q_fsize (fname) > 64 * 1024)
            printf ("WARNING: Are you sure %s is a loader binary?\n", fname);
        }
      else
        {
          fname = ucon64.argv[n];
          printf ("ROM%d: %s\n", file_no, fname);
        }

      if ((srcfile = fopen (fname, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], fname);
          continue;
        }
      done = 0;
      byteswritten = 0;                         // # of bytes written per file
      while (!done)
        {
          bytestowrite = fread (buffer, 1, BUFSIZE, srcfile);
          if (totalsize + bytestowrite > truncate_size)
            {
              bytestowrite = truncate_size - totalsize;
              done = 1;
              truncated = 1;
              printf ("Output file is %d Mbit, truncating %s, skipping %d bytes\n",
                      truncate_size / MBIT, fname,
                      q_fsize (fname) - (byteswritten + bytestowrite));
              // DON'T use fstate.st_size, because file could be compressed
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
      file_no++;
    }
  fclose (destfile);

  /*
    Display a notification if a truncate size was specified that is not exactly
    the size of one of the Flash Card sizes.
  */
  n = truncate_size;
  while (n >>= 1)
    size_pow2 <<= 1;
  if (truncate_size == size_pow2)
    truncate_size_ispow2 = 1;

  n = totalsize - 1;
  while (n >>= 1)
    size_pow2_lesser <<= 1;

  size_pow2 = size_pow2_lesser << 1;

  if (totalsize > 64 * MBIT && !truncate_size_ispow2)
    printf("\n"
           "NOTE: This multirom can only be written to a card >= %d Mbit.\n"
           "      Use -multi=%d to create a file truncated to %d Mbit.\n"
           "      Current size is %.5f Mbit\n", // 5 digits to have 1 byte resolution
           size_pow2 / MBIT, size_pow2_lesser / MBIT, size_pow2_lesser / MBIT,
           totalsize / (float) MBIT);

  return 0;
}


int
gbautil (const char *filein, const char *fileout)
/* gbautil version 1.1
 * SRAM, Wait State Patch, and FF Trimming Utility for Game Boy Advance
 *
 * Copyright (C) 2001 Omar Kilani <gbautil@aurore.net>
 *
 * NOTICE: You should only use this program on games you actually own.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
{
unsigned char version[8] = {
  0x45, 0x45, 0x50, 0x52, 0x4F, 0x4D, 0x5F
};

unsigned char st_orig[2][10] = {
  {0x0E, 0x48, 0x39, 0x68, 0x01, 0x60, 0x0E, 0x48, 0x79, 0x68},
  {0x13, 0x4B, 0x18, 0x60, 0x13, 0x48, 0x01, 0x60, 0x13, 0x49}
};

unsigned char st_repl[2][10] = {
  {0x00, 0x48, 0x00, 0x47, 0x01, 0xFF, 0xFF, 0x08, 0x79, 0x68},
  {0x01, 0x4C, 0x20, 0x47, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0x08}
};

unsigned char fl_orig[2][24] = {
  {0xD0, 0x20, 0x00, 0x05, 0x01, 0x88, 0x01, 0x22, 0x08, 0x1C, 0x10,
   0x40, 0x02, 0x1C, 0x11, 0x04, 0x08, 0x0C, 0x00, 0x28, 0x01, 0xD0,
   0x1B, 0xE0},
  {0xD0, 0x21, 0x09, 0x05, 0x01, 0x23, 0x0C, 0x4A, 0x08, 0x88, 0x18,
   0x40, 0x00, 0x28, 0x08, 0xD1, 0x10, 0x78, 0x00, 0x28, 0xF8, 0xD0,
   0x08, 0x88}
};

unsigned char fl_repl[2][24] = {
  {0xE0, 0x20, 0x00, 0x05, 0x01, 0x88, 0x01, 0x22, 0x08, 0x1C, 0x10,
   0x40, 0x02, 0x1C, 0x11, 0x04, 0x08, 0x0C, 0x00, 0x28, 0x01, 0xD0,
   0x1B, 0xE0},
  {0xE0, 0x21, 0x09, 0x05, 0x01, 0x23, 0x0C, 0x4A, 0x08, 0x88, 0x18,
   0x40, 0x00, 0x28, 0x08, 0xD1, 0x10, 0x78, 0x00, 0x28, 0xF8, 0xD0,
   0x08, 0x88}
};

unsigned char p_repl[2][188] = {
  {0x39, 0x68, 0x27, 0x48, 0x81, 0x42, 0x23, 0xD0, 0x89, 0x1C, 0x08,
   0x88, 0x01, 0x28, 0x02, 0xD1, 0x24, 0x48, 0x78, 0x60, 0x33, 0xE0,
   0x00, 0x23, 0x00, 0x22, 0x89, 0x1C, 0x10, 0xB4, 0x01, 0x24, 0x08,
   0x68, 0x20, 0x40, 0x5B, 0x00, 0x03, 0x43, 0x89, 0x1C, 0x52, 0x1C,
   0x06, 0x2A, 0xF7, 0xD1, 0x10, 0xBC, 0x39, 0x60, 0xDB, 0x01, 0x02,
   0x20, 0x00, 0x02, 0x1B, 0x18, 0x0E, 0x20, 0x00, 0x06, 0x1B, 0x18,
   0x7B, 0x60, 0x39, 0x1C, 0x08, 0x31, 0x08, 0x88, 0x09, 0x38, 0x08,
   0x80, 0x16, 0xE0, 0x15, 0x49, 0x00, 0x23, 0x00, 0x22, 0x10, 0xB4,
   0x01, 0x24, 0x08, 0x68, 0x20, 0x40, 0x5B, 0x00, 0x03, 0x43, 0x89,
   0x1C, 0x52, 0x1C, 0x06, 0x2A, 0xF7, 0xD1, 0x10, 0xBC, 0xDB, 0x01,
   0x02, 0x20, 0x00, 0x02, 0x1B, 0x18, 0x0E, 0x20, 0x00, 0x06, 0x1B,
   0x18, 0x08, 0x3B, 0x3B, 0x60, 0x0B, 0x48, 0x39, 0x68, 0x01, 0x60,
   0x0A, 0x48, 0x79, 0x68, 0x01, 0x60, 0x0A, 0x48, 0x39, 0x1C, 0x08,
   0x31, 0x0A, 0x88, 0x80, 0x21, 0x09, 0x06, 0x0A, 0x43, 0x02, 0x60,
   0x07, 0x48, 0x00, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x00,
   0x00, 0x00, 0x0E, 0x04, 0x00, 0x00, 0x0E, 0xD4, 0x00, 0x00, 0x04,
   0xD8, 0x00, 0x00, 0x04, 0xDC, 0x00, 0x00, 0x04, 0xFF, 0xFF, 0xFF,
   0x08},
  {0x22, 0x4C, 0x84, 0x42, 0x20, 0xD0, 0x80, 0x1C, 0x04, 0x88, 0x01,
   0x25, 0x2C, 0x40, 0x01, 0x2C, 0x02, 0xD1, 0x80, 0x1E, 0x1E, 0x49,
   0x2E, 0xE0, 0x00, 0x23, 0x00, 0x24, 0x80, 0x1C, 0x40, 0xB4, 0x01,
   0x26, 0x05, 0x68, 0x35, 0x40, 0x5B, 0x00, 0x2B, 0x43, 0x80, 0x1C,
   0x64, 0x1C, 0x06, 0x2C, 0xF7, 0xD1, 0x40, 0xBC, 0xDB, 0x01, 0x02,
   0x24, 0x24, 0x02, 0x1B, 0x19, 0x0E, 0x24, 0x24, 0x06, 0x1B, 0x19,
   0x19, 0x1C, 0x09, 0x3A, 0x16, 0xE0, 0x12, 0x48, 0x00, 0x23, 0x00,
   0x24, 0x40, 0xB4, 0x01, 0x26, 0x05, 0x68, 0x35, 0x40, 0x5B, 0x00,
   0x2B, 0x43, 0x80, 0x1C, 0x64, 0x1C, 0x06, 0x2C, 0xF7, 0xD1, 0x40,
   0xBC, 0xDB, 0x01, 0x02, 0x24, 0x24, 0x02, 0x1B, 0x19, 0x0E, 0x24,
   0x24, 0x06, 0x1B, 0x19, 0x08, 0x3B, 0x18, 0x1C, 0x08, 0x4C, 0x20,
   0x60, 0x08, 0x4C, 0x21, 0x60, 0x08, 0x49, 0x80, 0x20, 0x00, 0x06,
   0x02, 0x43, 0x0A, 0x60, 0x06, 0x4C, 0x20, 0x47, 0x00, 0x00, 0x00,
   0x0D, 0x00, 0x00, 0x00, 0x0E, 0x04, 0x00, 0x00, 0x0E, 0xD4, 0x00,
   0x00, 0x04, 0xD8, 0x00, 0x00, 0x04, 0xDC, 0x00, 0x00, 0x04, 0xFF,
   0xFF, 0xFF, 0x08}
};

int p_size[2] = { 188, 168 };

#define CP_PATTERN_SIZE 5

unsigned char c_orig[CP_PATTERN_SIZE][6] = {
  {0x04, 0x02, 0x00, 0x04, 0x14, 0x40},
  {0x02, 0x00, 0x04, 0x14, 0x40, 0x00},
  {0x04, 0x02, 0x00, 0x04, 0xB4, 0x45},
  {0x3E, 0xE0, 0x00, 0x00, 0xB4, 0x45},
  {0x04, 0x02, 0x00, 0x04, 0x94, 0x44}
};

unsigned char c_repl[CP_PATTERN_SIZE][6] = {
  {0x04, 0x02, 0x00, 0x04, 0x00, 0x40},
  {0x02, 0x00, 0x04, 0x00, 0x40, 0x00},
  {0x04, 0x02, 0x00, 0x04, 0x00, 0x40},
  {0x3E, 0xE0, 0x00, 0x00, 0x00, 0x40},
  {0x04, 0x02, 0x00, 0x04, 0x00, 0x40}
};

typedef struct
{
  int speed;
  unsigned char hex;
}
waitstate;

static waitstate waitstates[] = {
  {0, 0x00},                    /* default in most crash patches */
  {4, 0x04},                    /* faster than 0, slower than 8 */
  {8, 0x08},                    /* faster than 4, slower than 28 */
  {12, 0x0C},                   /* slowest cartridge access speed */
  {16, 0x10},                   /* faster than 28, but slower than 20 */
  {20, 0x14},                   /* default in most original carts */
  {24, 0x18},                   /* fastest cartridge access speed */
  {28, 0x1C}                    /* faster than 8 but slower than 16 */
};

#define WAITSTATE_SIZE (sizeof (waitstates) / sizeof (waitstate))
#define MAX_WAITSTATE waitstates[WAITSTATE_SIZE-1].speed


#define SWITCH_CRASH_PATCH 0x00000002
#define SWITCH_TRIM_FF 0x00000004
#define SWITCH_EEPROM_PATCH 0x00000008
#define SWITCH_DEFAULT SWITCH_EEPROM_PATCH
  unsigned short ff, major, minor, micro, i, switches, patched;
  unsigned long filesize, stloc, flloc, ploc, cloc;
  unsigned char title[13], mkcode[5];
  unsigned char *buffer, *bufferptr, *stptr, *cptr;
  int /* c, */ waitspeed = 0;
  FILE *in, *out;

  in = out = NULL;
  buffer = NULL;
  ff = i = major = minor = micro = patched = switches = 0;
  filesize = stloc = flloc = ploc = cloc = 0;

#if 0
    if (argc < 3) {
        cptr = (strchr(argv[0], FILE_SEPARATOR) == NULL) ? argv[0] : strchr(argv[0], FILE_SEPARATOR);
        printf("GBAUtil 1.0\n\n");
        printf("Usage:\t%s [-c] [-p] [-t] rominfo-in.gba rominfo-out.gba\n\n", cptr);
        printf("To patch a ROM for SRAM saving:\n");
        printf("\t%s -p rominfo-in.gba rominfo-out.gba\n\n", cptr);
        printf("To slow down flash access for a ROM (i.e, \"crash patch\"):\n");
        printf("\t%s -c 0 rominfo-in.gba rominfo-out.gba\n\n", cptr);
        printf("To trim a ROM so that you can flash multiple ROMs:\n");
        printf("\t%s -t rominfo-in.gba rominfo-out.gba\n\n", cptr);
        printf("You can combine two or more options, for example:\n");
        printf("\t%s -c 0 t rominfo-in.gba rominfo-out.gba\n\n", cptr);
        printf("Will both crash patch a ROM and trim it.\n\n");
        printf("The default action is to patch the ROM for SRAM saving.\n\n");
        printf("The wait state (argument to the -c flag) can be any number between 0 and 28, with a stepping of 4. I.e 0, 4, 8, 12 ...\n");
        printf("For a standard crash patch, use 0. If that does not help, try other arguments.\n\n");
        printf("NOTICE: You should only use this program on games you actually own.\n");
        return -1;
    }

    while ((c = getoptx(argc,argv,"tc:p")) != -1) {
        if (c == '?') {
                return -1;
        }
        switch(c) {
                case 'c':
                        if (isdigit(*optarg)) {
                        waitspeed = atol(optarg);
                        printf("Wait Speed = %d\n", waitspeed);
                        switches |= SWITCH_CRASH_PATCH;
                        } else {
                        fprintf(stderr, "You must specify a time with the -c flag.\n");
                        exit(1);
                        }
                        break;
                case 'p':
                        switches |= SWITCH_EEPROM_PATCH;
                        break;
                case 't':
                        switches |= SWITCH_TRIM_FF;
                        break;
        }
    }
#endif
  if (switches == 0)
    {
      switches |= SWITCH_DEFAULT;
    }

  if (strncmp (filein, fileout, strlen (filein)) == 0)
    {
      printf ("Input and output filenames must differ.\n");
      return -1;
    }

  in = fopen (filein, "rb");
  if (in == NULL)
    {
      perror ("fopen");
      return -1;
    }

  filesize = q_fsize (filein);
  if ((filesize % 1024) != 0 || filesize == 0)
    {
      fprintf (stderr, "ROM dimensions do not seem correct.\n");
      fprintf (stderr, "Size %ld is not cleanly divisable by 1024\n",
               filesize);
      fflush (stderr);
      fclose (in);
      return -1;
    }

//    printf("size = %.2fMBit\n", ((float) 8 * filesize / 1024 / 1024));
  buffer = (unsigned char *) calloc (filesize + 1, sizeof (unsigned char));
  if (buffer == NULL)
    {
      fprintf (stderr,
               "Failed to allocate %ld bytes.\nTry again after you have freed some memory.\n",
               filesize);
      fflush (stderr);
      fclose (in);
      return -1;
    }

  if (fread (buffer, sizeof (unsigned char), filesize, in) != filesize)
    {
      perror ("fread");
      fclose (in);
      return -1;
    }

  bufferptr = buffer;
  bufferptr += 160;
  strncpy ((char *) title, (const char *) bufferptr, 12);
  title[12] = 0;
//    printf("title = %s\n", title);
  bufferptr += 12;
  strncpy ((char *) mkcode, (const char *) bufferptr, 4);
  mkcode[4] = 0;
//    printf("gamecode = %s\n", mkcode);
  bufferptr += 4;
  stptr = bufferptr;
  cloc = flloc = stloc = 160 + 12 + 4;
  cptr = bufferptr;

  if (switches & SWITCH_EEPROM_PATCH)
    {
      while (cptr++, cloc++)
        {
          if (cloc == filesize)
            {
              printf ("This file does not contain EEPROM saving.\n");
              free (buffer);
              fclose (in);
              return -1;
            }
          if (*cptr == version[0])
            {
              if (memcmp (cptr, version, 7) == 0)
                {
                  printf ("version location = 0x%08lX (%ld)\n", cloc, cloc);
                  cptr += 8;
                  major = *cptr++ - '0';
                  minor = *cptr++ - '0';
                  micro = *cptr - '0';
                  break;
                }
            }
        }

      printf ("major = %d, minor = %d, micro = %d\n", major, minor, micro);

      while (stptr++, stloc++)
        {
          if (stloc == filesize)
            {
              printf ("You have previously patched this file.\n");
              free (buffer);
              fclose (in);
              return -1;
            }
          if (*stptr == st_orig[minor - 1][0])
            {
              if (memcmp
                  (stptr, st_orig[minor - 1],
                   sizeof (st_orig[minor - 1]) / sizeof (unsigned char)) == 0)
                {
                  printf ("st location = 0x%08lX (%ld)\n", stloc, stloc);
                  break;
                }
            }
        }

      /* these are the offsets to the caller function, it handles all saving
         * and is at stloc */
      switch (minor)
        {
        case 1:
          p_repl[minor - 1][184] = (unsigned char) (stloc + 0x21);
          p_repl[minor - 1][186] = (unsigned char) (stloc >> 16);
          break;
        case 2:
          p_repl[minor - 1][164] = (unsigned char) (stloc + 0x13);
          p_repl[minor - 1][165] = (unsigned char) (stloc >> 8);
          p_repl[minor - 1][166] = (unsigned char) (stloc >> 16);
          break;
        default:
          free (buffer);
          fclose (in);
          return -1;
        }

      bufferptr = stptr;
      flloc = stloc;
      while (bufferptr++, flloc++)
        {
          if (*bufferptr == fl_orig[minor - 1][0])
            {
              if (memcmp
                  (bufferptr, fl_orig[minor - 1],
                   sizeof (fl_orig[minor - 1]) / sizeof (unsigned char)) == 0)
                {
                  printf ("fl location = 0x%08lX (%ld)\n", flloc, flloc);
                  break;
                }
            }
        }
      memcpy (bufferptr, fl_repl[minor - 1],
              sizeof (fl_repl[minor - 1]) / sizeof (unsigned char));

      bufferptr = buffer + filesize - 1;
      ploc = filesize;

      /* here we skip all extrenouos 0xFF's (if the last byte in file is 0xFF)
         * we stop at the first difference in the pattern, and then we skip
         * x bytes so that we're aligned with a 256 byte code block */
      ff = (*bufferptr == 0xFF);
      while (*bufferptr == 0xFF || *bufferptr == 0x00)
        {
          ploc--;
          if (ff && *bufferptr == 0x00)
            {
              break;
            }
          bufferptr--;
        }

      ploc += (256 - (ploc % 256));
      /* if the sram function won't fit at the end of the ROM, abort */
      if ((minor == 1 && filesize - 188 < ploc)
          || (minor == 2 && filesize - 168 < ploc))
        {
          printf ("Not enough room at end of ROM, aborting ...\n");
          free (buffer);
          fclose (in);
          return -1;
        }

      bufferptr = buffer;
      bufferptr += (int) ploc;
      printf ("ploc = 0x%08lX (%ld)\n", ploc, ploc);

      /* tell the calling function where the sram function is (ploc) */
      switch (minor)
        {
        case 1:
          if (*--bufferptr == 0xFF)
            {
              p_repl[minor - 1][185] = (unsigned char) (stloc >> 8);
            }
          else
            {
              stloc += 0x1F;
              p_repl[minor - 1][185] = (unsigned char) (stloc >> 8);
            }
          st_repl[minor - 1][5] = (unsigned char) (ploc >> 8);
          st_repl[minor - 1][6] = (unsigned char) (ploc >> 16);
          bufferptr++;
          break;
        case 2:
          st_repl[minor - 1][7] = (unsigned char) (ploc >> 8);
          st_repl[minor - 1][8] = (unsigned char) (ploc >> 16);
          break;
        default:
          free (buffer);
          fclose (in);
          return -1;
        }
      memcpy (stptr, st_repl[minor - 1],
              sizeof (st_repl[minor - 1]) / sizeof (unsigned char));
      memcpy (bufferptr, p_repl[minor - 1], p_size[minor - 1]);
      printf ("SRAM patch applied.\n");
    }

  if (switches & SWITCH_CRASH_PATCH)
    {
      for (i = 0; i < CP_PATTERN_SIZE; i++)
        {
          cloc = 160 + 12 + 4;
          cptr = buffer + (int) cloc;
          while (cptr++, cloc++)
            {
              if (cloc == filesize)
                {
                  if (i == (CP_PATTERN_SIZE - 1) && patched == 0)
                    {
                      printf
                        ("This file has been previously crash patched.\n");
                      printf
                        ("Or, there is no support for this particular ROM.\n"
                         "Report it to gbautil@aurore.net\n");
                      if (switches == SWITCH_CRASH_PATCH)
                        {
                          free (buffer);
                          fclose (in);
                          return -1;
                        }
                      else
                        {
                          goto out;
                        }
                    }
                  else
                    {
                      break;
                    }
                }
              if (*cptr == c_orig[i][0])
                {
                  if (memcmp (cptr, c_orig[i], 6) == 0)
                    {
                      printf ("i = %d, c location = 0x%08lX (%ld)\n", i, cloc,
                              cloc);
                      patched++;
                      break;
                    }
                }
            }
          c_repl[0][4] = waitstates[waitspeed].hex;
          c_repl[1][3] = waitstates[waitspeed].hex;
          c_repl[2][4] = waitstates[waitspeed].hex;
          c_repl[3][4] = waitstates[waitspeed].hex;
          c_repl[4][4] = waitstates[waitspeed].hex;
          memcpy (cptr, c_repl[i], 6);
        }
      printf ("Crash patch applied.\n");
    }
out:

  if (switches & SWITCH_TRIM_FF)
    {
      bufferptr = buffer + filesize - 1;
      while (*bufferptr == 0xFF)
        {
          *bufferptr-- = 0;
        }
      printf ("File trimmed.\n");
    }

  out = fopen (fileout, "wb+");
  if (out == NULL)
    {
      perror ("fopen");
      free (buffer);
      fclose (in);
      return -1;
    }

  rewind (out);
  printf ("Writing %s... ", fileout);
  if (fwrite (buffer, sizeof (unsigned char), filesize, out) != filesize)
    {
      fputc ('\n', stdout);
      perror ("fwrite");
      free (buffer);
      fclose (in);
      fclose (out);
      return -1;
    }

  free (buffer);
  fclose (in);
  fclose (out);
//  printf ("Done\n");
  return 0;
}
