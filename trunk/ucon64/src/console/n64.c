/*
n64.c - Nintendo64 support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2002 dbjh


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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "n64.h"
#include "patch/ips.h"
#include "patch/aps.h"
#include "backup/doctor64.h"
#include "backup/doctor64jr.h"
#include "backup/cd64.h"
#include "backup/dex.h"
#include "backup/z64.h"


#define N64_HEADER_START 0
#define N64_HEADER_LEN (sizeof (st_n64_header_t))
#define N64_SRAM_SIZE 512
#define N64_NAME_LEN  20
#define N64_BOT_SIZE 4032
#define LAC_ROM_SIZE  1310720

const char *n64_usage[] =
  {
    "Nintendo 64",
    "1996 Nintendo http://www.nintendo.com",
    "  " OPTION_LONG_S "n64         force recognition\n"
#if 0   // unlike for SNES ROMs we can't make a mistake about this
    "  " OPTION_LONG_S "int         force ROM is in interleaved format\n"
    "  " OPTION_LONG_S "int2        force ROM is in interleaved format 2\n"
    "  " OPTION_LONG_S "nint        force ROM is not in interleaved format\n"
#endif
    "  " OPTION_S "n           change ROM name; " OPTION_LONG_S "file=NEWNAME\n"
    "  " OPTION_LONG_S "v64         convert to Doctor V64 (and compatibles/interleaved)\n"
    "  " OPTION_LONG_S "z64         convert to Z64 (Zip Drive/not interleaved)\n"
#ifdef TODO
#warning TODO  --f      remove NTSC/PAL protection
#endif // TODO
#if 0
    "TODO:  " OPTION_S "f      remove NTSC/PAL protection\n"
#endif
    "  " OPTION_LONG_S "bot         add/extract boot code to/from ROM; " OPTION_LONG_S "file=BOOTCODE (4032 Bytes)\n"
    "                  extracts automatically if BOOTCODE does not exist\n"
    "  " OPTION_LONG_S "sram        LAC's Makesram; " OPTION_LONG_S "rom=(LAC's SRAM ROM image) " OPTION_LONG_S "file=SRAMFILE\n"
    "                  the SRAMFILE must have a size of 512 Bytes\n"
    "  " OPTION_LONG_S "usms        Jos Kwanten's ultraSMS (Sega Master System/GameGear emulator);\n"
    "                  " OPTION_LONG_S "rom=(Jos Kwanten's ultraSMS ROM image) " OPTION_LONG_S "file=SMSROM\n"
    "                  works only for ROMs which are <= 4 Mb in size\n"
    "  " OPTION_LONG_S "chk         fix ROM checksum\n"
    "                  supports only 6101 and 6102 boot codes\n"
#ifdef TODO
#warning TODO  --bios   enable backup in Doctor V64 BIOS
#warning TODO  --gs     apply GameShark code (permanent)
#endif // TODO
#if 0
    "TODO:  " OPTION_LONG_S "bios   enable backup in Doctor V64 BIOS; " OPTION_LONG_S "rom=BIOS\n"
    "  " OPTION_S "p           pad ROM to full Mb\n"
    "TODO:  " OPTION_LONG_S "gs     apply GameShark code (permanent); " OPTION_LONG_S "file=CODE\n"
#endif
    ,
    NULL
};


typedef struct st_n64_header
{
  unsigned char pad[64];
#if 0
  unsigned char validation[2];
  unsigned char compression;
  unsigned char pad1;
  unsigned long clockrate;
  unsigned long programcounter;
  unsigned long release;
  unsigned long crc1;
  unsigned long crc2;
  unsigned char pad2[8];
  unsigned char name[20];
  unsigned char pad3[7];
  unsigned char maker;
  unsigned char cartridgeid[2];
  unsigned char countrycode;
  unsigned char pad4;
#endif
#if 0
  rominfo.validation           = *(tr_u16 *)(rom.header + (0x00 ^ 0x02));
  rominfo.compression          = *(tr_u8 *) (rom.header + (0x02 ^ 0x03));
  rominfo.unknown1             = *(tr_u8 *) (rom.header + (0x03 ^ 0x03));
  rominfo.clockrate            = *(tr_u32 *) (rom.header +  0x04);
  rominfo.programcounter       = *(tr_u32 *) (rom.header +  0x08);
  rominfo.release              = *(tr_u32 *) (rom.header +  0x0c);
  rominfo.crc1                 = *(tr_u32 *) (rom.header +  0x10);
  rominfo.crc2                 = *(tr_u32 *) (rom.header +  0x14);
  rominfo.unknown2_h           = *(tr_u32 *) (rom.header +  0x18);
  rominfo.unknown2_l           = *(tr_u32 *) (rom.header +  0x1c);
  for (i=0; i < 20; i++)
    rominfo.name[i ^ 0x03] = rom.header[i + 0x20];
  rominfo.unknown3             = *(tr_u8 *) (rom.header + (0x34 ^ 0x03));
  rominfo.unknown4             = *(tr_u8 *) (rom.header + (0x35 ^ 0x03));
  rominfo.unknown5             = *(tr_u8 *) (rom.header + (0x36 ^ 0x03));
  rominfo.unknown6             = *(tr_u8 *) (rom.header + (0x37 ^ 0x03));
  rominfo.unknown7             = *(tr_u8 *) (rom.header + (0x38 ^ 0x03));
  rominfo.unknown8             = *(tr_u8 *) (rom.header + (0x39 ^ 0x03));
  rominfo.unknown9             = *(tr_u8 *) (rom.header + (0x3a ^ 0x03));
  rominfo.makerid              = *(tr_u8 *) (rom.header + (0x3b ^ 0x03));
  rominfo.cartridgeid          = *(tr_u16 *)(rom.header + (0x3c ^ 0x02));
  rominfo.countrycode          = *(tr_u8 *) (rom.header + (0x3e ^ 0x03));
  rominfo.unknown10            = *(tr_u8 *) (rom.header + (0x3f ^ 0x03));
#endif
} st_n64_header_t;

st_n64_header_t n64_header;

typedef struct st_n64_chksum
{
  unsigned long crc1;
  unsigned long crc2;
} st_n64_chksum_t;

static st_n64_chksum_t n64crc;
static int n64_chksum (st_rominfo_t *rominfo);


// This is the support of LaC's makesram routine for uploading
//  SRAM files to a Cart SRAM. The .v64 file is his work, not mine
int
n64_sram (st_rominfo_t *rominfo)
{
  char sram[N64_SRAM_SIZE];

  if (q_fsize (ucon64.file) != N64_SRAM_SIZE ||
      q_fsize (ucon64.rom) != LAC_ROM_SIZE)
    {
      fprintf (stderr,
        "ERROR: Check if ROM has %d Bytes and SRAM has %d Bytes\n"
        "       or the ROM is too short to calculate checksum\n", LAC_ROM_SIZE, N64_SRAM_SIZE);
      return -1;
    }

  q_fread (sram, 0, N64_SRAM_SIZE, ucon64.file);

  if (rominfo->interleaved != 0)
    mem_swap (sram, N64_SRAM_SIZE);

  ucon64_fbackup (NULL, ucon64.rom);
  q_fwrite (sram, 0x286C0, N64_SRAM_SIZE, ucon64.rom, "r+b");
  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);

  return 0;
}


int
n64_v64 (st_rominfo_t *rominfo)
{
  long size = q_fsize (ucon64.rom);
  char buf[FILENAME_MAX];

  if (rominfo->interleaved != 0)
    {
      fprintf (stderr, "ERROR: Already in V64 format\n");
      return -1;
    }

  strcpy (buf, ucon64.rom);
  setext (buf, ".V64");

  ucon64_fbackup (NULL, buf);
  q_fcpy (ucon64.rom, 0, size, buf, "wb");
  q_fswap (buf, 0, size);

  fprintf (stdout, ucon64_msg[WROTE], buf);
  return 0;
}


int
n64_z64 (st_rominfo_t *rominfo)
{
  long size = q_fsize (ucon64.rom);
  char buf[FILENAME_MAX];

  if (!rominfo->interleaved)
    {
      fprintf (stderr, "ERROR: Already in Z64 format\n");
      return -1;
    }

  strcpy (buf, ucon64.rom);
  setext (buf, ".Z64");

  ucon64_fbackup (NULL, buf);
  q_fcpy (ucon64.rom, 0, size, buf, "wb");
  q_fswap (buf, 0, size);

  fprintf (stdout, ucon64_msg[WROTE], buf);
  return 0;
}


int
n64_n (st_rominfo_t *rominfo)
{
  char buf[N64_NAME_LEN];

  memset (buf, ' ', N64_NAME_LEN);
  strncpy (buf, ucon64.file, strlen (ucon64.file) > N64_NAME_LEN ?
           N64_NAME_LEN : strlen (ucon64.file));

  if (rominfo->interleaved != 0)
    mem_swap (buf, N64_NAME_LEN);

  ucon64_fbackup (NULL, ucon64.rom);
  q_fwrite (buf, N64_HEADER_START + rominfo->buheader_len + 32, 20,
               ucon64.rom, "r+b");

  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);
  return 0;
}


int
n64_f (st_rominfo_t *rominfo)
{
  // TODO: PAL/NTSC fix
  return 0;
}


int
n64_chk (st_rominfo_t *rominfo)
{
  int x;
  char buf[8];

  ucon64_fbackup (NULL, ucon64.rom);

  // n64crc is set by n64_checksum() when called from n64_init()
  for (x = 0; x < 4; x++)
    {
      q_fputc (ucon64.rom,
                  N64_HEADER_START + rominfo->buheader_len + 0x10 +
                  (x ^ rominfo->interleaved),
                  (n64crc.crc1 & 0xff000000) >> 24, "r+b");
      n64crc.crc1 <<= 8;
    }

  for (x = 0; x < 4; x++)
    {
      q_fputc (ucon64.rom,
                  N64_HEADER_START + rominfo->buheader_len + 0x14 +
                  (x ^ rominfo->interleaved),
                  (n64crc.crc2 & 0xff000000) >> 24, "r+b");
      n64crc.crc2 <<= 8;
    }

  q_fread (buf, 0x10 + rominfo->buheader_len, 8, ucon64.rom);
  mem_hexdump (buf, 8, 0x10 + rominfo->buheader_len);

  return 0;
}


int
n64_bot (st_rominfo_t *rominfo)
{
  char buf[FILENAME_MAX];

  if (!access (ucon64.file, F_OK))
    {
      q_fread (buf, 0, N64_BOT_SIZE, ucon64.file);

      if (rominfo->interleaved != 0)
        mem_swap (buf, N64_BOT_SIZE);

      ucon64_fbackup (NULL, ucon64.rom);
      q_fwrite (buf, N64_HEADER_START + rominfo->buheader_len + 0x40,
        N64_BOT_SIZE, ucon64.rom, "r+b");
    }
  else
    {
      strcpy (buf, ucon64.rom);
      setext (buf, ".BOT");

      ucon64_fbackup (NULL, buf);
      q_fcpy (ucon64.rom, N64_HEADER_START + rominfo->buheader_len + 0x040,
        N64_BOT_SIZE, buf, "wb");

      if (rominfo->interleaved != 0)
        q_fswap (buf, 0, q_fsize (buf));
    }

  fprintf (stdout, ucon64_msg[WROTE], buf);
  return 0;
}


int
n64_usms (st_rominfo_t *rominfo)
{
  char *usmsbuf, buf[FILENAME_MAX];

  if (!access (ucon64.file, F_OK))
    {
      long size = q_fsize (ucon64.file);
      // must be smaller than 4 Mbit, 524288 bytes will be inserted
      //  from 1b410 to 9b40f (7ffff)
      if (size > ((4 * MBIT) - 1))
        {
          fprintf (stderr, "ERROR: The Sega Master System/GameGear ROM must be smaller than 524288 Bytes\n");
          return -1;
        }

      if (!(usmsbuf = (char *) malloc ((size + 2) * sizeof (char))))
        return -1;
      q_fread (usmsbuf, 0, size, ucon64.file);

      if (rominfo->interleaved != 0)
        mem_swap (usmsbuf, size);

      ucon64_fbackup (NULL, "patched.v64");
      q_fwrite (usmsbuf, N64_HEADER_START + rominfo->buheader_len + 0x01b410,
        size, "patched.v64", "r+b");
      fprintf (stdout, ucon64_msg[WROTE], "patched.v64");

      free (usmsbuf);
    }
  else
    {
      strcpy (buf, ucon64.rom);
      setext (buf, ".GG");
      ucon64_fbackup (NULL, buf);

      q_fcpy (ucon64.rom, N64_HEADER_START + rominfo->buheader_len + 0x040,
        0x01000 - 0x040, buf, "wb");
      if (rominfo->interleaved != 0)
        q_fswap (buf, 0, q_fsize (buf));

      fprintf (stdout, ucon64_msg[WROTE], buf);
    }

  return 0;
}


int
n64_init (st_rominfo_t *rominfo)
{
  int result = -1;
  long x, value = 0;
  char buf[MAXBUFSIZE];
#define N64_MAKER_MAX 0x50
  const char *n64_maker[N64_MAKER_MAX] = {
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
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, "Nintendo", NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, "Nintendo", NULL},
#define N64_COUNTRY_MAX 0x5a
  *n64_country[N64_COUNTRY_MAX] = {
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
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, "Germany", "U.S.A.",
    "France", NULL, NULL, "Italy", "Japan",
    NULL, NULL, NULL, NULL, NULL,
    "Europe", NULL, NULL, "Spain", NULL,
    "Australia", NULL, NULL, "France, Germany, Holland", NULL};

  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
    ucon64.buheader_len : 0;

  q_fread (&n64_header, N64_HEADER_START + rominfo->buheader_len,
    N64_HEADER_LEN, ucon64.rom);

  value = OFFSET (n64_header, 0);
  value += OFFSET (n64_header, 1) << 8;
  value += OFFSET (n64_header, 2) << 16;
  value += OFFSET (n64_header, 3) << 24;
  if (value == 0x40123780) // 0x80371240
    {
      rominfo->interleaved = 0;
      result = 0;
    }
  else if (value == 0x12408037) // 0x37804012
    {
      rominfo->interleaved = 1;
      result = 0;
    }
#if 0
  /*
    What format is this?
    Conversion: 1234 -> 3412
  */
  else if (value == 0x80371240) // 0x40123780
    {
      rominfo->interleaved = 2;
      result = 0;
    }
#endif
  else
    result = -1;

  if (UCON64_ISSET (ucon64.interleaved))
    rominfo->interleaved = ucon64.interleaved;
  if (ucon64.console == UCON64_N64)
    result = 0;

  // internal ROM header
  rominfo->header_start = N64_HEADER_START;
  rominfo->header_len = N64_HEADER_LEN;
  rominfo->header = &n64_header;

  // internal ROM name
  strncpy (rominfo->name, &OFFSET (n64_header, 32), N64_NAME_LEN);
  if (rominfo->interleaved)
    mem_swap (rominfo->name, N64_NAME_LEN);
  rominfo->name[N64_NAME_LEN] = 0;

  // ROM maker
  rominfo->maker = NULL_TO_UNKNOWN_S (n64_maker[MIN (OFFSET
    (n64_header, 59 ^ rominfo->interleaved), N64_MAKER_MAX - 1)]);

  // ROM country
  rominfo->country = NULL_TO_UNKNOWN_S (n64_country[MIN (OFFSET
    (n64_header, 63 ^ (!rominfo->interleaved)), N64_COUNTRY_MAX - 1)]);

  // CRC stuff
  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      rominfo->has_internal_crc = 1;
      rominfo->internal_crc_len = rominfo->internal_crc2_len = 4;

      n64_chksum (rominfo);
      rominfo->current_internal_crc = n64crc.crc1;

      for (x = 0; x < 4; x++)
        {
          rominfo->internal_crc <<= 8;
          rominfo->internal_crc += OFFSET (n64_header, 0x10 + (x ^ rominfo->interleaved));
        }

      value = 0;
      for (x = 0; x < 4; x++)
        {
          value <<= 8;
          value += OFFSET (n64_header, 0x14 + (x ^ rominfo->interleaved));
        }

      sprintf (buf,
               "2nd Checksum: %%s, 0x%%0%dlx (calculated) %%s= 0x%%0%dlx (internal)\n"
               "NOTE: the checksum routine supports only 6101 and 6102 boot codes",
               rominfo->internal_crc2_len * 2, rominfo->internal_crc2_len * 2);

      sprintf (rominfo->internal_crc2, buf,
#ifdef  ANSI_COLOR
               ucon64.ansi_color ?
                 ((n64crc.crc2 == value) ?
                   "\x1b[01;32mok\x1b[0m" : "\x1b[01;31mbad\x1b[0m")
                 :
                 ((n64crc.crc2 == value) ? "ok" : "bad"),
#else
               (n64crc.crc2 == value) ? "ok" : "bad",
#endif
               n64crc.crc2,
               (n64crc.crc2 == value) ? "=" : "!", value);
    }

  rominfo->console_usage = n64_usage;
  rominfo->copier_usage = (!rominfo->buheader_len) ?
    ((!rominfo->interleaved) ? z64_usage : doctor64_usage
#if 0
doctor64jr_usage
cd64_usage
#endif
    ) : unknown_usage;
  return result;
}


// ROM Checksum Routine Courtesy of:
//  chksum64 V1.2, a program to calculate the ROM checksum of Nintendo64 ROMs.
//  Copyright (C) 1997  Andreas Sterbenz (stan@sbox.tu-graz.ac.at)

#define ROL(i, b) (((i)<<(b)) | ((i)>>(32-(b))))
#define BYTES2LONG(b, s) ( (((b)[0^(s)] & 0xffL) << 24) | \
                           (((b)[1^(s)] & 0xffL) << 16) | \
                           (((b)[2^(s)] & 0xffL) <<  8) | \
                           (((b)[3^(s)] & 0xffL)) )

#define CHECKSUM_START 0x1000
#define CHECKSUM_LENGTH 0x100000L
#define CHECKSUM_HEADERPOS 0x10
#define CHECKSUM_STARTVALUE 0xf8ca4ddc

int
n64_chksum (st_rominfo_t *rominfo)
{
  char readchunk3[MAXBUFSIZE];
  unsigned long i, c1, k1, k2, t1, t2, t3, t4, t5, t6, clen = CHECKSUM_LENGTH,
                rlen = (ucon64.file_size - rominfo->buheader_len) - CHECKSUM_START,
                sum1, sum2;
  unsigned int n;
  FILE *fh;
  t1 = CHECKSUM_STARTVALUE;
  t2 = CHECKSUM_STARTVALUE;
  t3 = CHECKSUM_STARTVALUE;
  t4 = CHECKSUM_STARTVALUE;
  t5 = CHECKSUM_STARTVALUE;
  t6 = CHECKSUM_STARTVALUE;

  if (rlen < 0x0100000)                         // 0x0101000
    return -1;                                  // rom is too short

  if (!(fh = fopen (ucon64.rom, "rb")))
    return -1;
  fseek (fh, CHECKSUM_START + rominfo->buheader_len, SEEK_SET);

  for (;;)
    {
      if (rlen > 0)
        {
          n = fread (readchunk3, 1, MIN (sizeof (readchunk3), clen), fh);
          if ((n & 3) != 0)
            n += fread (readchunk3 + n, 1, 4 - (n & 3), fh);
        }
      else
        n = MIN (sizeof (readchunk3), clen);

      if ((n == 0) || ((n & 3) != 0))
        {
          if ((clen != 0) || (n != 0))
            printf ("WARNING: Short read, checksum may be incorrect.\n");
          break;
        }
      for (i = 0; i < n; i += 4)
        {
          c1 = BYTES2LONG (&readchunk3[i], rominfo->interleaved);
          k1 = t6 + c1;
          if (k1 < t6)
            t4++;
          t6 = k1;
          t3 ^= c1;
          k2 = c1 & 0x1f;
          k1 = ROL (c1, k2);
          t5 += k1;
          if (c1 < t2)
            t2 ^= k1;
          else
            t2 ^= t6 ^ c1;
          t1 += c1 ^ t5;
        }
      if (rlen > 0)
        {
          rlen -= n;
          if (rlen <= 0)
            memset (readchunk3, 0, sizeof (readchunk3));
        }
      clen -= n;
    }
  sum1 = t6 ^ t4 ^ t3;
  sum2 = t5 ^ t2 ^ t1;

  fclose (fh);

  n64crc.crc1 = sum1;
  n64crc.crc2 = sum2;

  return 0;
}
