/*
pce.c - PC-Engine support for uCON64

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "pce.h"
#include "backup/mgd.h"
#include "backup/cdrw.h"


const char *pcengine_usage[] =
  {
    "PC-Engine (CD Unit/Core Grafx(II)/Shuttle/GT/LT/Super CDROM/DUO(-R(X)))\nSuper Grafx/Turbo (Grafx(16)/CD/DUO/Express)",
    "1987/19XX/19XX NEC",
    "  " OPTION_LONG_S "pce         force recognition"
#ifndef DB
    "; NEEDED"
#endif
    "\n"
#if 0
    "  " OPTION_LONG_S "hd          force ROM has SMG header (+512 Bytes)\n"
    "  " OPTION_LONG_S "nhd         force ROM has no SMG header (MGD2/RAW)\n"
#endif
    "  " OPTION_LONG_S "smg         convert to Super Magic Griffin/SMG\n"
    "  " OPTION_LONG_S "mgd         convert to Multi Game Doctor*/MGD2/RAW\n",
    NULL
};


typedef struct st_pce_header
{
  char pad[48];
} st_pce_header_t;
#define PCENGINE_HEADER_START 0x448
#define PCENGINE_HEADER_LEN (sizeof (st_pce_header_t))

st_pce_header_t pce_header;


int
pcengine_smg (st_rominfo_t *rominfo)
{
  char buf[MAXBUFSIZE];
  st_unknown_header_t header;
  long size = q_fsize (ucon64.rom) - rominfo->buheader_len;

  if (rominfo->buheader_len != 0)
    {
      fprintf (stderr, "ERROR: Already in SMG format\n");
      return -1;
    }

  memset (&header, 0, UNKNOWN_HEADER_LEN);
  header.size_low = size / 8192;
  header.size_high = size / 8192 >> 8;
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 2;

  strcpy (buf, ucon64.rom);
  setext (buf, ".SMG");

  ucon64_fbackup (NULL, buf);
  q_fwrite (&header, 0, UNKNOWN_HEADER_LEN, buf, "wb");

  q_fcpy (ucon64.rom, 0, size, buf, "ab");
  ucon64_wrote (buf);

/*

1) Created by JSI/Front Far East

2) 512 Bytes Length

3) Byte
    0   - Low Byte of 8k-Bytes page Counts
    1   - High Byte of 8k-Bytes page Counts
    2   - Emulation Mode Select
          Bit 7 6 5 4 3 2 1
              x             : 1=Run in Mode 0 (Jump $8000)
                x           : 0=Last File of the Game (Multi-File)
                  x         : 0=Mode 1, 1=Mode 2 (SRAM Mapping)
                    x       : 0=Mode 20, 1=Mode 21 (DRAM Mapping)
                        x   : 0=Run in Mode 3, 1=Run in Mode 2 (JMP Reset)
                          x : 0=Disable, 1=Enable (external cartridge
                              memory image at Bank 205F,A0 System Mode 2,3)
    8   - File ID Code 1 (Should be 'AA')
    9   - File ID Code 2 (Should be 'BB')
    10  - Check this byte if ID 1 & 2 Match
          02 : Magic Griffin Game File (PC Engine)
          03 : Magic Griffin SRAM Data File
          04 : SWC & SMC Game File (SNES)
          05 : SWC & SMC Password, SRAM data, Saver Data File.
          06 : SMD Game File (Megadrive)
          07 : SMD SRAM Data File
    37  - Reserved (Should be 00)
    11511 - Reserved (Should be 00)


*/
  return 0;
}


int
pcengine_mgd (st_rominfo_t *rominfo)
{
/*
The MGD2 only accepts certain filenames, and these filenames
must be specified in an index file, "MULTI-GD", otherwise the
MGD2 will not recognize the file.  In the case of multiple games
being stored in a single disk, simply enter its corresponding
MULTI-GD index into the "MULTI-GD" file.

Super Famiciom:

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

PC Engine:
game size       # of files      names           MUTLI-GD
================================================================
1M              1               PC1XXX.040      PC1XXX
2M              1               PC2XXX.040      PC2XXX
4M              1               PC4XXX.048      PC4XXX
8M              1               PC8XXX.058      PC8XXX


The Game Doctor does not use a 512 byte header like the SWC,
instead it uses specially designed filenames to distinguish
between multi files. I'm not sure if it used the filename for
information about the size of the image though.
<p>
Usually, the filename is in the format of: SFXXYYYZ.078
<p>
Where SF means Super Famicom, XX refers to the size of the
image in Mbit. If the size is only one character (i.e. 2, 4 or
8 Mbit) then no leading "0" is inserted.
<p>
YYY refers to a catalogue number in Hong Kong shops
identifying the game title. (0 is Super Mario World, 1 is F-
Zero, etc). I was told that the Game Doctor copier produces a
random number when backing up games.
<p>
Z indicates a multi file. Like XX, if it isn't used it's
ignored.
<p>
A would indicate the first file, B the second, etc. I am told
078 is not needed, but is placed on the end of the filename by
systems in Asia.
<p>
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

  strcpy (buf, areupper (basename2 (ucon64.rom)) ? "PC" : "pc");
  strcat (buf, basename2 (ucon64.rom));
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

  ucon64_wrote (buf2);
  return 0;
}


int
pcengine_init (st_rominfo_t *rominfo)
{
  int result = -1;

  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
    ucon64.buheader_len : 0;

  q_fread (&pce_header, PCENGINE_HEADER_START +
      rominfo->buheader_len, PCENGINE_HEADER_LEN, ucon64.rom);

//#ifdef CONSOLE_PROBE
//#endif // CONSOLE_PROBE

  rominfo->header_start = PCENGINE_HEADER_START;
  rominfo->header_len = PCENGINE_HEADER_LEN;
  rominfo->header = &pce_header;

#if 0
//TODO what kind of chksum does pcengine_chksum() return?
  rominfo->has_internal_crc = 0;
  rominfo->internal_crc_len=2;

  rominfo->internal_crc = pcengine_chksum(rominfo);

  rominfo->internal_crc2[0] = 0;
#endif

  rominfo->console_usage = pcengine_usage;
  rominfo->copier_usage = (!rominfo->buheader_len) ? mgd_usage : unknown_usage;

  return result;
}


int
pcengine_chksum (st_rominfo_t *rominfo)
{
  unsigned long chksumconst[] = {
    0x0,
    0x77073096,
    0xee0e612c,
    0x990951ba,
    0x76dc419,
    0x706af48f,
    0xe963a535,
    0x9e6495a3,
    0xedb8832,
    0x79dcb8a4,
    0xe0d5e91e,
    0x97d2d988,
    0x9b64c2b,
    0x7eb17cbd,
    0xe7b82d07,
    0x90bf1d91,
    0x1db71064,
    0x6ab020f2,
    0xf3b97148,
    0x84be41de,
    0x1adad47d,
    0x6ddde4eb,
    0xf4d4b551,
    0x83d385c7,
    0x136c9856,
    0x646ba8c0,
    0xfd62f97a,
    0x8a65c9ec,
    0x14015c4f,
    0x63066cd9,
    0xfa0f3d63,
    0x8d080df5,
    0x3b6e20c8,
    0x4c69105e,
    0xd56041e4,
    0xa2677172,
    0x3c03e4d1,
    0x4b04d447,
    0xd20d85fd,
    0xa50ab56b,
    0x35b5a8fa,
    0x42b2986c,
    0xdbbbc9d6,
    0xacbcf940,
    0x32d86ce3,
    0x45df5c75,
    0xdcd60dcf,
    0xabd13d59,
    0x26d930ac,
    0x51de003a,
    0xc8d75180,
    0xbfd06116,
    0x21b4f4b5,
    0x56b3c423,
    0xcfba9599,
    0xb8bda50f,
    0x2802b89e,
    0x5f058808,
    0xc60cd9b2,
    0xb10be924,
    0x2f6f7c87,
    0x58684c11,
    0xc1611dab,
    0xb6662d3d,
    0x76dc4190,
    0x1db7106,
    0x98d220bc,
    0xefd5102a,
    0x71b18589,
    0x6b6b51f,
    0x9fbfe4a5,
    0xe8b8d433,
    0x7807c9a2,
    0xf00f934,
    0x9609a88e,
    0xe10e9818,
    0x7f6a0dbb,
    0x86d3d2d,
    0x91646c97,
    0xe6635c01,
    0x6b6b51f4,
    0x1c6c6162,
    0x856530d8,
    0xf262004e,
    0x6c0695ed,
    0x1b01a57b,
    0x8208f4c1,
    0xf50fc457,
    0x65b0d9c6,
    0x12b7e950,
    0x8bbeb8ea,
    0xfcb9887c,
    0x62dd1ddf,
    0x15da2d49,
    0x8cd37cf3,
    0xfbd44c65,
    0x4db26158,
    0x3ab551ce,
    0xa3bc0074,
    0xd4bb30e2,
    0x4adfa541,
    0x3dd895d7,
    0xa4d1c46d,
    0xd3d6f4fb,
    0x4369e96a,
    0x346ed9fc,
    0xad678846,
    0xda60b8d0,
    0x44042d73,
    0x33031de5,
    0xaa0a4c5f,
    0xdd0d7cc9,
    0x5005713c,
    0x270241aa,
    0xbe0b1010,
    0xc90c2086,
    0x5768b525,
    0x206f85b3,
    0xb966d409,
    0xce61e49f,
    0x5edef90e,
    0x29d9c998,
    0xb0d09822,
    0xc7d7a8b4,
    0x59b33d17,
    0x2eb40d81,
    0xb7bd5c3b,
    0xc0ba6cad,
    0xedb88320,
    0x9abfb3b6,
    0x3b6e20c,
    0x74b1d29a,
    0xead54739,
    0x9dd277af,
    0x4db2615,
    0x73dc1683,
    0xe3630b12,
    0x94643b84,
    0xd6d6a3e,
    0x7a6a5aa8,
    0xe40ecf0b,
    0x9309ff9d,
    0xa00ae27,
    0x7d079eb1,
    0xf00f9344,
    0x8708a3d2,
    0x1e01f268,
    0x6906c2fe,
    0xf762575d,
    0x806567cb,
    0x196c3671,
    0x6e6b06e7,
    0xfed41b76,
    0x89d32be0,
    0x10da7a5a,
    0x67dd4acc,
    0xf9b9df6f,
    0x8ebeeff9,
    0x17b7be43,
    0x60b08ed5,
    0xd6d6a3e8,
    0xa1d1937e,
    0x38d8c2c4,
    0x4fdff252,
    0xd1bb67f1,
    0xa6bc5767,
    0x3fb506dd,
    0x48b2364b,
    0xd80d2bda,
    0xaf0a1b4c,
    0x36034af6,
    0x41047a60,
    0xdf60efc3,
    0xa867df55,
    0x316e8eef,
    0x4669be79,
    0xcb61b38c,
    0xbc66831a,
    0x256fd2a0,
    0x5268e236,
    0xcc0c7795,
    0xbb0b4703,
    0x220216b9,
    0x5505262f,
    0xc5ba3bbe,
    0xb2bd0b28,
    0x2bb45a92,
    0x5cb36a04,
    0xc2d7ffa7,
    0xb5d0cf31,
    0x2cd99e8b,
    0x5bdeae1d,
    0x9b64c2b0,
    0xec63f226,
    0x756aa39c,
    0x26d930a,
    0x9c0906a9,
    0xeb0e363f,
    0x72076785,
    0x5005713,
    0x95bf4a82,
    0xe2b87a14,
    0x7bb12bae,
    0xcb61b38,
    0x92d28e9b,
    0xe5d5be0d,
    0x7cdcefb7,
    0xbdbdf21,
    0x86d3d2d4,
    0xf1d4e242,
    0x68ddb3f8,
    0x1fda836e,
    0x81be16cd,
    0xf6b9265b,
    0x6fb077e1,
    0x18b74777,
    0x88085ae6,
    0xff0f6a70,
    0x66063bca,
    0x11010b5c,
    0x8f659eff,
    0xf862ae69,
    0x616bffd3,
    0x166ccf45,
    0xa00ae278,
    0xd70dd2ee,
    0x4e048354,
    0x3903b3c2,
    0xa7672661,
    0xd06016f7,
    0x4969474d,
    0x3e6e77db,
    0xaed16a4a,
    0xd9d65adc,
    0x40df0b66,
    0x37d83bf0,
    0xa9bcae53,
    0xdebb9ec5,
    0x47b2cf7f,
    0x30b5ffe9,
    0xbdbdf21c,
    0xcabac28a,
    0x53b39330,
    0x24b4a3a6,
    0xbad03605,
    0xcdd70693,
    0x54de5729,
    0x23d967bf,
    0xb3667a2e,
    0xc4614ab8,
    0x5d681b02,
    0x2a6f2b94,
    0xb40bbe37,
    0xc30c8ea1,
    0x5a05df1b,
    0x2d02ef8d
  };
  unsigned char *buf;
  register unsigned long x, crc = -1, size;
  unsigned int taille;
  FILE *fh;

  if (!(fh = fopen (ucon64.rom, "rb")))
    return -1;

  taille = (q_fsize (ucon64.rom) - rominfo->buheader_len);
  size = taille & 0xfffff000;
//  if ((taille & 0x0fff)==0)
//    rominfo->buheader_len=0;
  fseek (fh, taille & 0x0fff, SEEK_SET);
  if (!(buf = (unsigned char *) (malloc (size))))
    return -1;

  fread (buf, size, 1, fh);

  for (x = 0; x < size; x++)
    {
      buf[x] ^= crc;
      crc >>= 8;
      crc ^= chksumconst[buf[x]];
      crc ^= buf[x];
    }
  free (buf);
  crc = ~crc;
  fclose (fh);
  return crc;
}
