/*
sms.c - Sega Master System/GameGear support for uCON64

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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "sms.h"
#include "backup/mgd.h"
#include "backup/smd.h"


const char *sms_usage[] =
  {
    "Sega Master System(II/III)/GameGear (Handheld)",
    "1986/19XX SEGA http://www.sega.com",
    "  " OPTION_LONG_S "sms         force recognition"
    "\n"
#if 0
    "  " OPTION_LONG_S "hd          force ROM has header (+512 Bytes)\n"
    "  " OPTION_LONG_S "nhd         force ROM has no header (MGD2/MGH/RAW)\n"
    "TODO: show more detailed ROM info\n"
#endif
    "  " OPTION_LONG_S "mgd         convert to Multi Game*/MGD2/MGH/RAW\n"
    "  " OPTION_LONG_S "smd         convert to Super Magic Drive/SMD (+512 Bytes)\n"
    "  " OPTION_LONG_S "smds        convert emulator (*.srm) SRAM to Super Magic Drive/SMD\n"
#if 0
    "TODO:  " OPTION_LONG_S "chk   fix ROM checksum\n"
    "  " OPTION_LONG_S "gge         encode GameGenie code; " OPTION_LONG_S "rom=AAAA:VV or " OPTION_LONG_S "rom=AAAA:VV:CC\n"
    "  " OPTION_LONG_S "ggd         decode GameGenie code; " OPTION_LONG_S "rom=XXX-XXX or " OPTION_LONG_S "rom=XXX-XXX-XXX\n"
    "  " OPTION_LONG_S "gg          apply GameGenie code (permanent); " OPTION_LONG_S "file=CODE-CODE\n"
#endif
    ,
    NULL
  };


#if 0
typedef struct st_sms_header
{
  char pad[64];
} st_sms_header_t;
#define SMS_HEADER_START 0
#define SMS_HEADER_LEN (sizeof (st_sms_header_t))

st_sms_header_t sms_header;
#endif


int
sms_mgd (st_rominfo_t *rominfo)
{
/*
The MGD2 only accepts certain filenames, and these filenames
must be specified in an index file, "MULTI-GD", otherwise the
MGD2 will not recognize the file.  In the case of multiple games
being stored in a single disk, simply enter its corresponding
MULTI-GD index into the "MULTI-GD" file.

Mega Drive:

game size       # of files      names           MUTLI-GD
================================================================
1M              1               MD1XXX.000      MD1XXX
2M              1               MD2XXX.000      MD2XXX
4M              1               MD4XXX.000      MD4XXX
8M              1               MD8XXX.008      MD8XXX
16M             2               MD16XXXA.018    MD16XXXA
                                MD16XXXB.018    MD16XXXB
24M             3               MD24XXXA.038    MD24XXXA
                                MD24XXXB.038    MD24XXXB
                                MD24XXXC.038    MD24XXXC
32M             4               MD32XXXA.038    MD32XXXA
                                MD32XXXB.038    MD32XXXB
                                MD32XXXC.038    MD32XXXC
                                MD32XXXD.038    MD32XXXD

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

e.g. The first 16Mbit file of Donkey Kong Country (assuming it
  is cat. no. 475) would look like:  SF16475A.078
*/
  char buf[MAXBUFSIZE];
  char buf2[MAXBUFSIZE], *p = NULL;

  if (!rominfo->buheader_len)
    {
      fprintf (stderr, "ERROR: Already in MGD format\n");
      return -1;
    }

  strcpy (buf, areupper (basename (ucon64.rom)) ? "GG" : "gg");
  strcat (buf, basename (ucon64.rom));
  if ((p = strrchr (buf, '.')))
    *p = 0;
  strcat (buf, "________");
  buf[7] = '_';
  buf[8] = 0;
  sprintf (buf2, "%s.%03lu", buf,
           (unsigned long) ((q_fsize (ucon64.rom) - rominfo->buheader_len) /
                            MBIT));

  ucon64_fbackup (NULL, buf2);
  q_fcpy (ucon64.rom, rominfo->buheader_len, q_fsize (ucon64.rom),
            buf2, "wb");

  fprintf (stdout, ucon64_msg[WROTE], buf2);
  return 0;
}


int
sms_smd (st_rominfo_t *rominfo)
{
  st_smd_header_t header;
  char buf[MAXBUFSIZE];
  long size = q_fsize (ucon64.rom) - rominfo->buheader_len;

  if (rominfo->buheader_len != 0)
    {
      fprintf (stderr, "ERROR: Already in SMD format\n");
      return -1;
    }

  memset (&header, 0, UNKNOWN_HEADER_LEN);
  header.size = size / 8192 >> 8;
  header.id0 = 3; //size / 8192;
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 6;

  strcpy (buf, ucon64.rom);
  setext (buf, ".SMD");

  ucon64_fbackup (NULL, buf);
  q_fwrite (&header, 0, UNKNOWN_HEADER_LEN, buf, "wb");

  q_fcpy (ucon64.rom, 0, size, buf, "ab");

  fprintf (stdout, ucon64_msg[WROTE], buf);
  return 0;
}


int
sms_smds (st_rominfo_t *rominfo)
{
  st_smd_header_t header;
  char buf[MAXBUFSIZE];

  memset (&header, 0, SMD_HEADER_LEN);

  header.size = 0;
  header.id0 = 0;
  header.split = 0;
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 7;                              // SRAM file

  strcpy (buf, ucon64.rom);
  setext (buf, ".TMP");
  ucon64_fbackup (NULL, ucon64.rom);
  rename (ucon64.rom, buf);

  q_fwrite (&header, 0, SMD_HEADER_LEN, ucon64.rom, "wb");
  q_fcpy (buf, 0, q_fsize (ucon64.rom), ucon64.rom, "ab");
  fprintf (stdout, ucon64_msg[WROTE], ucon64.rom);

  remove (buf);
  return 0;
}


int
sms_init (st_rominfo_t *rominfo)
{
  int result = -1;

  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
    ucon64.buheader_len : 0;

  rominfo->console_usage = sms_usage;
  rominfo->copier_usage = (ucon64.buheader_len) ? mgd_usage : smd_usage;

  return result;
}
