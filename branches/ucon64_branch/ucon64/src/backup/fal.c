/*
fal.c - Flash Linker Advance support for uCON64

written by 2001        Jeff Frohwein
           2001        NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh


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
#include <time.h>
#include <string.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "fal.h"


const st_usage_t fal_usage[] =
  {
    {NULL, NULL, "Flash Advance Linker"},
    {NULL, NULL, "2001 Visoly http://www.visoly.com"},
#ifdef PARALLEL
    {"xfal", NULL, "send/receive ROM to/from Flash Advance Linker; " OPTION_LONG_S "port=PORT\n"
                "receives automatically (32 Mbits) when ROM does not exist"},
    {"xfalmulti", "SIZE", "send multiple ROMs to Flash Advance Linker (makes temporary\n"
                          "multirom truncated to SIZE Mbit); file with loader must be\n"
                          "specified first, then all the ROMs; " OPTION_LONG_S "port=PORT"},
    {"xfalc", "N", "receive N Mbits of ROM from Flash Advance Linker; " OPTION_LONG_S "port=PORT\n"
                   "N can be 8, 16, 32, 64, 128 or 256"},
    {"xfals", NULL, "send/receive SRAM to/from Flash Advance Linker; " OPTION_LONG_S "port=PORT\n"
                 "receives automatically when SRAM does not exist"},
    {"xfalb", "BANK", "send/receive SRAM to/from Flash Advance Linker BANK\n"
                     "BANK can be 1, 2, 3 or 4; " OPTION_LONG_S "port=PORT\n"
                     "receives automatically when SRAM does not exist"},
    {"xfalm", NULL, "try to enable EPP mode, default is SPP mode"},
#endif // PARALLEL
    {NULL, NULL, NULL}
};


#ifdef PARALLEL

/********************************************************/
/* Flash Linker Advance                                 */
/*   by Jeff Frohwein, 2001-Jun-28                      */
/* Compiled with DJGPP & linux                          */
/********************************************************/
// V1.0  - 01/06/28 - Original release
// V1.1  - 01/06/29 - Add -w option to slow down I/O transfer for some.
// V1.11 - 01/06/30 - Set ECP chipsets to proper parallel port mode.
// V1.2  - 01/07/23 - Fixed programming bug for Visoly carts.
//                  - Only the first block was getting erased.
//                  - -v option now appears on help (-h) menu.
//                  - -v & -s options now work properly after fixing a bug.
// V1.3  - 01/07/24 - Added support for Visoly turbo carts.
// V1.4  - 01/07/27 - Added support for longer filenames.
//                  - Fixed bug where files the size of the cart overwrite
//                     the first 16 half-words of the cart. Thnx goes to
//                     Richard W for the code fix. Thanks Richard!
//                    Fixed random lockup bug when programming Turbo carts.
//                    Added -n option. Header is now repaired by default.
// V1.5  - 01/09/25 - Added error retries/checking for older Visoly carts.
//                  - Odd length files no longer give a verify error on last byte+1 location.
// V1.6  - 01/11/11 - Made IRQ 7 instead of 5 and DMA 3 instead of 1 default
//                  - linux values. (Thanks to Massimiliano Marsiglietti.)
//                  - Added -D & -I to allow linux to change IRQ & DMA defaults.
//                  - Added LPT3 support.
//                  - Added error checking for space between switch & parameters.
//                  - Added -2 options for faster operation for some EPP ports.
// V1.7  - 01/11/13 - Added -b option to backup game save SRAM or game save Flash.
//                  - Added -r option to restore game save SRAM. (No flash support.)
// V1.71 - 01/11/23 - Fixed bug introduced in v1.7 where -d option printed out twice.
// V1.72 - 01/12/12 - Force 0x96 at location 0xb2 in header since it's required.

// To compile source on linux:
//   cc -o fl fl.c -O2
// You must have root access to run this under linux.
//
// NOTE: This file is filled with cr+lf line terminators. This may
// lead to unhelpful and weird error messages with gcc for linux.
// Strip out the cr characters to prevent this problem. The following
// unix command line will work for that:
//   tr -d \\r < dosfile > unixfile
// On some unix distributions you can also use the following command:
//   dos2unix
// (Thanks to Massimiliano Marsiglietti for locating this problem.)

// RAM Detect notes for dev. (Just ignore!)
//-----------------------------------------
// To detect backup type.
//  1. First check for eeprom.
//  2. Read byte from 0xe000000.
//  3. Write new byte to 0xe000000.
//  4. If diff, write back data & exit with SRAM detect flag.
//  5. If no diff get device Manuf ID for flash.
//  6. If no Manuf ID detected then report no cart backup available.

#define outpb(p, v) outportb((unsigned short) (p), (unsigned char) (v)); iodelay()
#define inpb(p)     inportb((unsigned short) (p))
#define outpw(p, v) outportw((unsigned short) (p), (unsigned char) (v)); iodelay()
#define inpw(p)     inportw((unsigned short) (p))

//#define HEADER_LENGTH 0xc0
//#define OUTBUFLEN 256                   // Must be a multiple of 2! (ex:64,128,256...)

#define INTEL28F_BLOCKERASE 0x20
#define INTEL28F_CLEARSR    0x50
#define INTEL28F_CONFIRM    0xD0
#define INTEL28F_QUIRY      0x98
#define INTEL28F_READARRAY  0xff
#define INTEL28F_READSR     0x70
#define INTEL28F_RIC        0x90
#define INTEL28F_WRTOBUF    0xe8

#define SHARP28F_BLOCKERASE 0x20
#define SHARP28F_CONFIRM    0xD0
#define SHARP28F_READARRAY  0xff
#define SHARP28F_WORDWRITE  0x10

#define u8      unsigned char
#define u16     unsigned short
#define u32     unsigned int
#define CONST_U8 const unsigned char


// ***Global Variables ***

//int WaitDelay,WaitNCDelay;
unsigned SPPDataPort;
unsigned SPPStatPort;
unsigned SPPCtrlPort;
unsigned EPPAddrPort;
unsigned EPPDataPort;
unsigned ECPRegECR;

// prototypes
void WriteFlash (int addr, int data);
int ReadFlash (int addr);
void iodelay (void);
int PPReadWord (void);
void PPWriteWord (int i);
void SetCartAddr (int addr);
void l4021d0 (int i);
void l40226c (void);

#define FLINKER 1

#include "cartlib.c"

static int debug, verbose, DataSize16, Device, EPPMode, RepairHeader,
           VisolyTurbo, WaitDelay, FileHeader[0xc0], HeaderBad, Complement = 0;

const u8 GoodHeader[] = {
  46, 0, 0, 234, 36, 255, 174, 81, 105, 154, 162, 33, 61, 132, 130, 10,
  132, 228, 9, 173, 17, 36, 139, 152, 192, 129, 127, 33, 163, 82, 190, 25,
  147, 9, 206, 32, 16, 70, 74, 74, 248, 39, 49, 236, 88, 199, 232, 51,
  130, 227, 206, 191, 133, 244, 223, 148, 206, 75, 9, 193, 148, 86, 138, 192,
  19, 114, 167, 252, 159, 132, 77, 115, 163, 202, 154, 97, 88, 151, 163, 39,
  252, 3, 152, 118, 35, 29, 199, 97, 3, 4, 174, 86, 191, 56, 132, 0,
  64, 167, 14, 253, 255, 82, 254, 3, 111, 149, 48, 241, 151, 251, 192, 133,
  96, 214, 128, 37, 169, 99, 190, 3, 1, 78, 56, 226, 249, 162, 52, 255,
  187, 62, 3, 68, 120, 0, 144, 203, 136, 17, 58, 148, 101, 192, 124, 99,
  135, 240, 60, 175, 214, 37, 228, 139, 56, 10, 172, 114, 33, 212, 248, 7
// 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,49,150,0,0,0,0,0,
// 0,0,0,0,0,240,0,0
};

void
iodelay (void)
{
  int i;
  for (i = 0; i < WaitDelay; i++)
    {
      i++;
      i--;
    }
}

void
ProgramExit (int code)
{
  exit (code);
}

#if 0
void
usage (char *name)
{
  char _small[255];
  char smaller[255];
  int i = 0;

  strcpy (_small, name);

#if 0
  if (strchr (name, '.') != NULL)
    _small[strlen (_small) - 4] = 0;    /* remove trailing file type */
#endif

  while ((_small[strlen (_small) - i] != 0x2f) &&       /* loop until we find a / */
         ((strlen (_small) - i) > 0))
    i++;

  if ((strlen (_small) - i) == 0)
    i++;

  strcpy (smaller, (char *) (&_small[strlen (_small) - i + 1]));

  fprintf (stderr, "GBA FLinker v1.72 by Jeff F.\n");
  fprintf (stderr, "Usage: %s [options]\n", smaller);

  fprintf (stderr,
           "\t-2      Use 16bit EPP data path for faster operation (default=8bit)\n");
  fprintf (stderr,
           "\t-b o s file   Backup game save SRAM or Flash to file\n");
  fprintf (stderr, "\t               (o = Bank Number [1-4])\n");
  fprintf (stderr, "\t               (s=1 - Backup 32K bytes to file.)\n");
  fprintf (stderr, "\t               (s=2 - Backup 64K bytes to file.)\n");
  fprintf (stderr, "\t               (s=3 - Backup 128K bytes to file.)\n");
  fprintf (stderr, "\t               (s=4 - Backup 256K bytes to file.)\n");
  fprintf (stderr,
           "\t-c n    Specify chip size in mbits (8,16,32,64,128,256) (default=32)\n");
  fprintf (stderr,
           "\t-d n    Dump 256 bytes of ROM to screen (default: n=0)\n");
  fprintf (stderr, "\t-h      This help screen\n");
  fprintf (stderr,
           "\t-l n    Specify the parallel port to use (default is 1 = LPT1)\n");
//   fprintf (stderr, "\t-m\tSet Standard Parallel Port (SPP) mode (default = EPP)\n");
  fprintf (stderr,
           "\t-n      Do not repair incorrect header (default = repair header)\n");
  fprintf (stderr, "\t-p file Program flash cart with file\n");
  fprintf (stderr,
           "\t-r o file     Restore game save SRAM from file (No save flash support)\n");
  fprintf (stderr, "\t               (o = Bank Number [1-4])\n");
  fprintf (stderr,
           "\t-s file Save the cart into a file (Use -c to specify size)\n");
  fprintf (stderr, "\t-v file Verify flash cart with file\n");
  fprintf (stderr, "\t-w n    Add delay to make transfer more reliable\n");
}
#endif

void
InitPort (int port)
{
//   int ECPRegECR;

  switch (port)
    {
    case 2:
      SPPDataPort = 0x278;
      break;
    case 3:
      SPPDataPort = 0x3bc;
      break;
    default:
      SPPDataPort = 0x378;
    }

//   if (port == 1)
//       SPPDataPort = 0x378;
//   else
//       SPPDataPort = 0x278;

  SPPStatPort = SPPDataPort + 1;
  SPPCtrlPort = SPPDataPort + 2;
  EPPAddrPort = SPPDataPort + 3;
  EPPDataPort = SPPDataPort + 4;
  ECPRegECR = SPPDataPort + 0x402;

//#ifndef __linux__
//   if (EPPMode)
//      { outpb (ECPRegECR, 4); }             // Set EPP mode for ECP chipsets
//   else
//      { outpb (ECPRegECR, 0); }             // Set SPP mode for ECP chipsets
//#endif
}

void
l4020a4 (int reg)
{
  outpb (SPPCtrlPort, 1);
  outpb (SPPDataPort, reg);
  outpb (SPPCtrlPort, 9);
  outpb (SPPCtrlPort, 1);
}

void
SPPWriteByte (int i)            // l4020dc
{
  outpb (SPPDataPort, i);
  outpb (SPPCtrlPort, 3);
  outpb (SPPCtrlPort, 1);
}

void
l402108 (int reg, int adr)
{
  l4020a4 (reg);
  SPPWriteByte (adr);
}

int
SPPReadByte (void)              // 402124
{
  int v;

  outpb (SPPCtrlPort, 0);
  outpb (SPPCtrlPort, 2);
  v = ((inpb (SPPStatPort)) >> 3) & 0xf;
  outpb (SPPCtrlPort, 6);
  v += (((inpb (SPPStatPort)) << 1) & 0xf0);
  outpb (SPPCtrlPort, 0);

  return v;
}

void
l402188 (int reg)
{
  outpb (SPPCtrlPort, 1);
  outpb (EPPAddrPort, reg);
}

void
l4021a8 (int reg, int adr)
{
  l402188 (reg);
  outpb (SPPCtrlPort, 1);
  outpb (EPPDataPort, adr);
}

void
l4021d0 (int i)
{
  outpb (SPPCtrlPort, 1);

  if (EPPMode)
    l402188 (i);
  else
    l4020a4 (i);
}

void
l402200 (int reg, int adr)
{
  if (EPPMode)
    l4021a8 (reg, adr);
  else
    l402108 (reg, adr);
}

void
l402234 (void)
{
  if (EPPMode)
    l402200 (4, 0x83);
  else
    l402200 (4, 0xc3);
}

void
l40226c (void)
{
  if (EPPMode)
    l402200 (4, 7);
  else
    l402200 (4, 0x47);
}

void
PPWriteByte (int i)             // 4022d0
{
  if (EPPMode)
    {
      outpb (EPPDataPort, i);
    }
  else
    SPPWriteByte (i);
}

void
PPWriteWord (int i)             // 4022d0
{
  if (EPPMode)
    {
      if (DataSize16)
        {
          outpw (EPPDataPort, i);
        }
      else
        {
          outpb (EPPDataPort, i);
          outpb (EPPDataPort, (i >> 8));
        }
    }
  else
    {
      SPPWriteByte (i);
      SPPWriteByte (i >> 8);
    }
}

int
PPReadByte (void)               // 40234c
{
  int v;

  if (EPPMode)
    v = inpb (EPPDataPort);
  else
    v = SPPReadByte ();

  return v;
}

int
PPReadWord (void)               // 402368  // ReadFlash
{
  int v = 0;

  if (EPPMode)
    {
      if (DataSize16)
        v = inpw (EPPDataPort);
      else
        {
          v = inpb (EPPDataPort);
          v += (inpb (EPPDataPort) << 8);
        }
    }
  else
    {
      v = SPPReadByte ();       //402124
      v += (SPPReadByte () << 8);
    }
  return v;
}

void
SetCartAddr (int addr)          // 4023cc
{
  l402200 (2, addr >> 16);
  l402200 (1, addr >> 8);
  l402200 (0, addr);
}

void
WriteFlash (int addr, int data) // 402414
{
  SetCartAddr (addr);
  l4021d0 (3);
  PPWriteWord (data);
}

int
ReadFlash (int addr)
{
  SetCartAddr (addr);
  l4021d0 (3);
  outpb (SPPCtrlPort, 0);
  return PPReadWord ();
}

void
l402684 (void)
{
  outpb (SPPStatPort, 1);
  l40226c ();
}

int
LookForLinker (void)            // 4026a8
{
  l402684 ();
  l402200 (2, 0x12);
  l402200 (1, 0x34);
  l402200 (0, 0x56);
  l4021d0 (2);
  outpb (SPPCtrlPort, 0);
  if (PPReadByte () != 0x12)    // 40234c
    return 0;

  l4021d0 (1);
  outpb (SPPCtrlPort, 0);
  if (PPReadByte () != 0x34)
    return 0;

  l4021d0 (0);
  outpb (SPPCtrlPort, 0);
  if (PPReadByte () != 0x56)
    return 0;

  outpb (SPPCtrlPort, 4);
  return 1;
}

void
LinkerInit (void)               // 4027c4
{
  int linker_found = 0;
  
  /*
    uCON64 comment:
    Accessing I/O ports with addresses higher than 0x3ff causes an access
    violation under Windows XP (NT/2000) for _Windows_ executables without the
    use of an appropriate I/O port driver. UserPort is an example of an
    *inappropriate* I/O port driver, because it enables access to I/O ports up
    to 0x3ff. For some (ridiculous) reason, DOS executables are allowed to
    _access_ at least the ECP register this code uses. That doesn't mean it
    will result in the expected behaviour like enabling EPP.
  */
  if (EPPMode)
    {
      outpb (ECPRegECR, 4);     // Set EPP mode for ECP chipsets
      if (LookForLinker ())
        {
          // Linker found using EPP mode.
          linker_found = 1;
          printf ("Linker found. EPP found.\n");
  
          if (SPPDataPort == 0x3bc)
            return;
          outpb (SPPCtrlPort, 4);
        }
    }
  if (!linker_found)
    {
      // Look for linker in SPP mode.
      if (EPPMode)
        outpb (ECPRegECR, 0);   // Set EPP mode for ECP chipsets

      EPPMode = 0;
      if (LookForLinker ())
        printf ("Linker found. EPP not found or not enabled - SPP used.\n");
      else
        {
          fprintf (stderr,
                   "ERROR: Flash Advance Linker not found or not turned on.\n");
          ProgramExit (1);
        }
    }
}

int
ReadStatusRegister (int addr)   // 402dd8
{
  int v;
  WriteFlash (addr, INTEL28F_READSR);
  outpb (SPPCtrlPort, 0);
  v = PPReadWord ();            // & 0xff;
  v = PPReadWord ();            // & 0xff;
  return v;
}

//void OldDumpSRAM (void)                    // 4046f4
//   {
//   int i,j,k,v;
//
//   i=1;
//   SetVisolyBackupRWMode (i>>1);
//   l402234 ();
//
//   for (j=0; j<0x80; j++)
//      {
//      l402200 (0, 0);
//      if (i==1)
//         l402200 (1, j);
//      else
//         l402200 (1, j|0x80);
//      l402200 (2, 0);
//      l4021d0 (3);
//      outpb (SPPCtrlPort, 0);
//
//      for (k=0; k<0x100; k++)
//         {
//         v = PPReadByte ();
//         printf ("%x ", v);
//         }
//      }
//   printf("\n");
//   }

// StartOffSet: 1 = 0, 2 = 64k, 3 = 128k, 4 = 192k
// Size: 1 = 32k, 2 = 64k, 3 = 128k, 4 = 256k

void
BackupSRAM (FILE * fp, int StartOS, int Size)   // 4046f4
{
  int j, k, v;
  int m;
  int n = 1 << (Size - 1);
  int size = n * 32 * 1024, bytesread = 0;
  time_t starttime = time (NULL);

  for (m = ((StartOS - 1) << 1); m < (((StartOS - 1) << 1) + n); m++)
    {
      if ((m & 1) == 0)
        {
          SetVisolyBackupRWMode (m >> 1);
          l402234 ();
        }

      // Backup a 32k byte chunk
      for (j = 0; j < 0x80; j++)
        {
          l402200 (0, 0);
          l402200 (1, j + (0x80 * (m & 1)));
          l402200 (2, 0);
          l4021d0 (3);
          outpb (SPPCtrlPort, 0);

          for (k = 0; k < 0x100; k++)
            {
              v = PPReadByte ();
              fputc (v, fp);
            }
          bytesread += 256;
          if ((bytesread & 0x1fff) == 0)        // call ucon64_gauge() after receiving 8 kB
            ucon64_gauge (starttime, bytesread, size);
        }
    }
}

// StartOffSet: 1 = 0, 2 = 64k, 3 = 128k, 4 = 192k

void
RestoreSRAM (FILE * fp, int StartOS)
{
  int i;
  int j, k;
  int m = ((StartOS - 1) << 1);
  int byteswritten = 0;
  time_t starttime;

  starttime = time (NULL);

  i = fgetc (fp);
  while (!feof (fp))
    {
      if ((m & 1) == 0)
        {
          SetVisolyBackupRWMode (m >> 1);
          l402234 ();
        }

      // Restore a 32k byte chunk
      for (j = 0; j < 0x80; j++)
        {
          l402200 (0, 0);
          l402200 (1, j + (0x80 * (m & 1)));
          l402200 (2, 0);
          l4021d0 (3);
          outpb (SPPCtrlPort, 0);

          for (k = 0; k < 0x100; k++)
            {
              PPWriteByte (i);
              i = fgetc (fp);
            }
          byteswritten += 256;
          if ((byteswritten & 0x1fff) == 0)     // call ucon64_gauge() after sending 8 kB
            ucon64_gauge (starttime, byteswritten, ucon64.file_size);
        }
      m++;
    }
}

void
BackupROM (FILE * fp, int SizekW)
{
  u16 valw;
  u32 i, j;
  int size = SizekW << 1, bytesread = 0;
  time_t starttime;

  WriteFlash (0, INTEL28F_READARRAY);   // Set flash (intel 28F640J3A) Read Mode

  starttime = time (NULL);
  for (i = 0; i < (u32) (SizekW >> 8); i++)
    {
      SetCartAddr (i << 8);     // Set cart base addr to 0
      l4021d0 (3);

      outpb (SPPCtrlPort, 0);

      for (j = 0; j < 256; j++)
        {
          valw = PPReadWord ();
          fputc (valw & 0xff, fp);
          fputc (valw >> 8, fp);
        }
      bytesread += 256 << 1;    // 256 words
      if ((bytesread & 0xffff) == 0)    // call ucon64_gauge() after receiving 64 kB
        ucon64_gauge (starttime, bytesread, size);
    }
}

void
dump (u8 BaseAdr)
{
//   unsigned char low, high;
  int i;
  u8 First = 1;
  u16 v;
  u8 val1, val2;
  u8 Display[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  WriteFlash (0, INTEL28F_READARRAY);   // Set flash (intel 28F640J3A) Read Mode

  SetCartAddr (BaseAdr << 7);   // Set cart base addr to read
  l4021d0 (3);

  outpb (SPPCtrlPort, 0);

  for (i = 0; i < 128; i++)
    {
      if (First == 1)
        {
          if (i * 2 < 256)
            printf ("0");
          if (i * 2 < 16)
            printf ("0");
          printf ("%hx - ", (i * 2));
          First = 0;
        }

      v = PPReadWord ();
      val2 = v >> 8;
      val1 = v & 255;

      if ((val1 > 31) & (val1 < 127))
        Display[(i & 7) * 2] = val1;
      else
        Display[(i & 7) * 2] = 46;

      if (val1 < 16)
        printf ("0");
      printf ("%hx ", val1);

      if ((val2 > 31) & (val2 < 127))

        Display[(i & 7) * 2 + 1] = val2;
      else
        Display[(i & 7) * 2 + 1] = 46;

      if (val2 < 16)
        printf ("0");
      printf ("%hx ", val2);

      if ((i & 7) == 7)
        {
          First = 1;
          printf ("   %3s", (&Display[0]));
          printf ("\n");
        }
    }
}

void
CheckForFC (void)
{
  LinkerInit ();
//   if (LinkerInit () == 0)
//      {
//      fprintf (stderr, "ERROR: Flash Advance Linker not found or not turned on.\n");
//      ProgramExit (1);
//      }

  SetVisolyFlashRWMode ();
  Device = CartTypeDetect ();
  VisolyTurbo = 0;

  printf ("Device ID = 0x%x : ", Device);

  switch (Device)
    {
    case 0x16:
      printf ("FA 32M (i28F320J3A)");
      break;
    case 0x17:
      printf ("FA 64M (i28F640J3A)");
      break;
    case 0x18:
      printf ("FA 128M (i28F128J3A)");
      break;
    case 0x2e:
      printf ("Standard ROM");
      break;
    case 0x96:
      printf ("Turbo FA 64M (2 x i28F320J3A)");
      VisolyTurbo = 1;
      break;
    case 0x97:
      printf ("Turbo FA 128M (2 x i28F640J3A)");
      VisolyTurbo = 1;
      break;
    case 0x98:
      printf ("Turbo FA 256M (2 x i28F128J3A");
      VisolyTurbo = 1;
      break;
    case 0xdc:
      printf ("Hudson");
      break;
    case 0xe2:
      printf ("Nintendo Flash Cart (LH28F320BJE)");
      break;
    default:
      printf ("Unknown");
      break;
    }
  printf ("\n");
}

int
GetFileByte (FILE * fp)
{
  static int FilePos = 0;
  int i = 0;

  if (RepairHeader)
    {
      // Set file pointer just past header
      if (FilePos == 0)
        for (i = 0; i < 0xa0; i++)
          (void) fgetc (fp);

      if (FilePos < 0xa0)
        {
          if ((HeaderBad) && (FilePos > 3))
            i = GoodHeader[FilePos];
          else
            i = FileHeader[FilePos];
        }
      else if ((FilePos == 0xb2) || (FilePos == 0xbd))
        {
          if (FilePos == 0xb2)
            i = 0x96;           // Required
          else
            i = Complement;
          (void) fgetc (fp);    // Discard file value
        }
      else
        i = fgetc (fp);
    }
  else
    i = fgetc (fp);

  FilePos++;
  return i;
}

int
GetFileSize2 (FILE * fp)
/*
  The name "GetFileSize" conflicts with a Windows function.
  For some odd reason Jeff Frohwein returned the file size minus 1. I (dbjh)
  guess that's a bug. I disabled the (terribly inefficient) file size code
  because uCON64 already determined the file size at this point.
*/
{
  int FileSize;

  if (RepairHeader)
    {
      int i;
      int j;

      FileSize = 0;
      while (!feof (fp) && (FileSize < 0xc0))
        {
          FileHeader[FileSize++] = fgetc (fp);
        }

      if (feof (fp))
        {
          fprintf (stderr, "ERROR: File must be 192 bytes or larger\n");
          ProgramExit (1);
        }
#if 0
      else
        FileSize--;

      while (!feof (fp))
        {
          (void) fgetc (fp);
          FileSize++;
        }
#endif

      HeaderBad = 0;
      i = 4;
      while (i < 0xa0)          //9c)
        {
          if (FileHeader[i] != GoodHeader[i])
            HeaderBad = 1;
          i++;
        }
      if (HeaderBad)
        printf ("Fixing logo area.");

      Complement = 0;
      FileHeader[0xb2] = 0x96;  // Required
      for (j = 0xa0; j < 0xbd; j++)
        Complement += FileHeader[j];
      Complement = (0 - (0x19 + Complement)) & 0xff;
      //printf("[Complement = 0x%x]", (int)Complement);
      //printf("[HeaderComp = 0x%x]", (int)FileHeader[0xbd]);
      if (FileHeader[0xbd] != Complement)
        printf ("Fixing complement check.");

      if ((FileHeader[0xbd] != Complement) || HeaderBad)
        printf ("\n");
      rewind (fp);
    }
  else
    {
#if 0
      FileSize = -1;
      while (!feof (fp))
        {
          (void) fgetc (fp);
          FileSize++;
        }
#endif
      rewind (fp);
    }
#if 1
  return ucon64.file_size;
#else
  return FileSize;
#endif
}

// Program older (non-Turbo) Visoly flash cart
// (Single flash chip)

void
ProgramNonTurboIntelFlash (FILE * fp)
{
  int i, j, k;
  int addr = 0;
  int FileSize;
  int Ready = 0;
  int Timeout;
  time_t starttime;

  // Get file size
  FileSize = GetFileSize2 (fp);

  printf ("Erasing Visoly non-turbo flash cart...\n\n");

  // Erase as many 128k blocks as are required
  Ready = EraseNonTurboFABlocks (0, ((FileSize - 1) >> 17) + 1);

  printf ("\r                                                                              \r");     // remove "erase gauge"
  if (Ready)
    {
      printf ("Programming Visoly non-turbo flash cart...\n\n");
      //403018

      starttime = time (NULL);
      j = GetFileByte (fp);

      while (!feof (fp))
        {
          Ready = 0;
          Timeout = 0x4000;

          while ((Ready == 0) && (Timeout != 0))
            {
              WriteFlash (addr, INTEL28F_WRTOBUF);
              outpb (SPPCtrlPort, 0);
              Ready = PPReadWord () & 0x80;

              Timeout--;
            }

          if (Ready)
            {
              WriteFlash (addr, 15);    // Write 15+1 16bit words

              SetCartAddr (addr);       // Set cart base addr to 0
              l4021d0 (3);

              for (i = 0; i < 16; i++)
                {
                  k = j;
                  if (j != EOF)
                    j = GetFileByte (fp);
                  k += (j << 8);
                  PPWriteWord (k);

                  if (j != EOF)
                    j = GetFileByte (fp);
                }

              addr += 16;
              if ((addr & 0x3fff) == 0) // call ucon64_gauge() after sending 32 kB
                ucon64_gauge (starttime, addr << 1, FileSize);

              PPWriteWord (INTEL28F_CONFIRM);   // Comfirm block write

              Ready = 0;
              Timeout = 0x4000;

              while ((Ready == 0) && (Timeout != 0))
                {
                  WriteFlash (0, INTEL28F_READSR);
                  outpb (SPPCtrlPort, 0);
                  i = PPReadWord () & 0xff;
                  Ready = i & 0x80;

                  Timeout--;
                }

              if (Ready)
                {
                  if (i & 0x7f)
                    {
                      // One or more status register error bits are set
                      outpb (SPPCtrlPort, 1);
                      WriteFlash (0, INTEL28F_CLEARSR);
                      Ready = 0;
                      break;
                    }
                }
              else
                {
                  outpb (SPPCtrlPort, 1);
                  WriteFlash (0, INTEL28F_CLEARSR);
                  break;
                }
            }
          else
            {
              break;
            }
        }

      printf ("\r                                                                              \r"); // remove last gauge
      ucon64_gauge (starttime, addr << 1, FileSize);   // make gauge reach 100% when size % 32 k != 0
      WriteFlash (0, INTEL28F_READARRAY);
      outpb (SPPCtrlPort, 0);

      if (Ready)
        ; //printf ("\n\nDone.\n");
      else
        {
          printf ("\n\nFlash cart write failed!\n");
        }
    }
  else
    {
      printf ("\nFlash cart erase failed!\n");
    }
}

// Program newer (Turbo) Visoly flash cart
// (Dual chip / Interleave)

void
ProgramTurboIntelFlash (FILE * fp)
{
  int i, j = 0;
  int k;                        //z;
  int addr = 0;
  int done1, done2;
  int FileSize;
  int Timeout;
  int Ready;                    //= 0;
  time_t starttime;

  // Get file size
  FileSize = GetFileSize2 (fp);

  printf ("Erasing Visoly turbo flash cart...\n\n");

  // Erase as many 256k blocks as are required
  Ready = EraseTurboFABlocks (0, ((FileSize - 1) >> 18) + 1);

  printf ("\r                                                                              \r");     // remove "erase gauge"
  if (Ready)
    {
      printf ("Programming Visoly turbo flash cart...\n\n");
      //403018
      starttime = time (NULL);
      j = GetFileByte (fp);

      while (!feof (fp))
        {
          done1 = 0;
          done2 = 0;
          Ready = 0;
          Timeout = 0x4000;

          while ((!Ready) && (Timeout != 0))
            {
              if (done1 == 0)
                WriteFlash (addr + 0, INTEL28F_WRTOBUF);
              if (done2 == 0)
                WriteFlash (addr + 1, INTEL28F_WRTOBUF);

              SetCartAddr (addr);       // Set cart base addr
              l4021d0 (3);
              outpb (SPPCtrlPort, 0);

              done1 = PPReadWord () & 0x80;
              done2 = PPReadWord () & 0x80;
              Ready = ((done1 + done2) == 0x100);

              Timeout--;
            }

          if (Ready)
            {
              WriteFlash (addr, 15);    // Write 15+1 16bit words
              PPWriteWord (15);

              SetCartAddr (addr);       // Set cart base addr
              l4021d0 (3);

              for (i = 0; i < 32; i++)
                {
                  k = j;
                  if (j != EOF)
                    j = GetFileByte (fp);
                  k += (j << 8);
                  PPWriteWord (k);

                  if (j != EOF)
                    j = GetFileByte (fp);
                }
              addr += 32;
              if ((addr & 0x3fff) == 0) // call ucon64_gauge() after sending 32 kB
                ucon64_gauge (starttime, addr << 1, FileSize);
              PPWriteWord (INTEL28F_CONFIRM);   // Comfirm block write
              PPWriteWord (INTEL28F_CONFIRM);   // Comfirm block write

              Ready = 0;
              Timeout = 0x4000;
              k = 0;

              while (((k & 0x8080) != 0x8080) && (Timeout != 0))
                {
                  outpb (SPPCtrlPort, 0);
                  k = PPReadWord () & 0xff;
                  k += ((PPReadWord () & 0xff) << 8);
                  Ready = (k == 0x8080);

                  Timeout--;
                }

              if (!Ready)
                break;
            }
          else
            break;
        }

      printf ("\r                                                                              \r"); // remove last gauge
      ucon64_gauge (starttime, addr << 1, FileSize);   // make gauge reach 100% when size % 32 k != 0
      WriteFlash (0, INTEL28F_READARRAY);
      outpb (SPPCtrlPort, 0);
      WriteFlash (1, INTEL28F_READARRAY);
      outpb (SPPCtrlPort, 0);

      if (Ready)
        ; //printf ("\n\nDone.\n");
      else
        {
          WriteFlash (0, INTEL28F_CLEARSR);
          PPWriteWord (INTEL28F_CLEARSR);
          printf ("\n\nFlash cart write failed!\n");
        }
    }
  else
    {
      printf ("\nFlash cart erase failed!\n");
    }
}

// Program official Nintendo flash cart

void
ProgramSharpFlash (FILE * fp)
{
  int i, j;
  int k = 0;
  int addr = 0;
  int FileSize;
  int Ready;
  time_t starttime;

  // Get file size
  FileSize = GetFileSize2 (fp);

  printf ("Erasing flash cart...\n\n");

  // Erase as many 64k blocks as are required
  Ready = EraseNintendoFlashBlocks (0, ((FileSize - 1) >> 16) + 1);

  printf ("\r                                                                              \r");     // remove "erase gauge"
  if (Ready)
    {
      printf ("Programming Nintendo flash cart...\n\n");

      starttime = time (NULL);
      j = GetFileByte (fp);

      while (!feof (fp))
        {
          if (j != EOF)
            k = GetFileByte (fp);

          i = ((k & 0xff) << 8) + (j & 0xff);

          while ((ReadStatusRegister (0) & 0x80) == 0)
            {
            }

          WriteFlash (addr, SHARP28F_WORDWRITE);
          WriteFlash (addr, i);
          addr += 1;

          j = GetFileByte (fp);
          if ((addr & 0x3fff) == 0)     // call ucon64_gauge() after sending 32 kB
            ucon64_gauge (starttime, addr << 1, FileSize);
        }

      printf ("\r                                                                              \r"); // remove last gauge
      ucon64_gauge (starttime, addr << 1, FileSize);   // make gauge reach 100% when size % 32 k != 0
      WriteFlash (0, INTEL28F_READARRAY);
      outpb (SPPCtrlPort, 0);

      //printf ("\n\nDone.\n");
    }
  else
    printf ("\n\nFlash cart erase failed!\n");
}

void
VerifyFlash (FILE * fp)
{
  int addr = 0;
  int CompareFail = 0;
  int k = 0;
  int i, j, m, n;

  WriteFlash (0, INTEL28F_READARRAY);   // Set flash (intel 28F640J3A) Read Mode

  j = 0;
  while (!feof (fp))
    {
      j = fgetc (fp);
      if (j != EOF)
        {
          if ((addr & 0x1ff) == 0)
            {
              SetCartAddr (addr >> 1);  // Set cart base addr to read
              l4021d0 (3);

              outpb (SPPCtrlPort, 0);
            }

          k = fgetc (fp);

          i = PPReadWord ();
          m = i & 0xff;
          n = i >> 8;

          if (m != j)
            {
              printf ("Address %x - Cartridge %x : File %hx\n", addr, m, j);
              CompareFail = 1;
            }
          if ((n != k) && (k != EOF))
            {
              printf ("Address %x - Cartridge %x : File %hx\n", addr + 1, n,
                      k);
              CompareFail = 1;
            }
          addr += 2;
        }
    }

  // Correct verify length if at EOF
  if (k == EOF)
    addr--;

  if (CompareFail == 0)
    printf ("%d bytes compared ok.\n", addr);
}

void
SpaceCheck (char c)
{
  if (c != 0)
    {
      fprintf (stderr,
               "ERROR: Space required between option and parameter\n");
      ProgramExit (1);
    }
}

int
fal_main (int argc, char **argv)
{
  int arg, i;
  u8 Base = 0;
  FILE *fp;
  char fname[128], fname2[128];
  int OptB = 0;
  int OptD = 0;
  int OptP = 0;
  int OptR = 0;
  int OptS = 0;
  int OptV = 0;
  int OptZ = 0;
  int port = 1;
  int ChipSize = 32;
  int BackupMemOffset = 0;
  int BackupMemSize = 0;

  if (argc < 2)
    {
//      usage (argv[0]);
      ProgramExit (1);
    }

  debug = 0;
  verbose = 1;
  EPPMode = 0;                                  // uCON64 comment: use the most compatible setting as default
  DataSize16 = 0;
  WaitDelay = 0;
  VisolyTurbo = 0;
  RepairHeader = 1;

  for (arg = 1; arg < argc; arg++)
    {
      if (argv[arg][0] != '-')
        {
//          usage (argv[0]);
          ProgramExit (1);
        }

      switch (argv[arg][1])
        {
        case 'b':
          SpaceCheck (argv[arg][2]);
          BackupMemOffset = *(char *) argv[++arg] - 0x30;
          SpaceCheck (argv[arg][1]);
          BackupMemSize = *(char *) argv[++arg] - 0x30;

          if ((BackupMemSize < 1) || (BackupMemSize > 4) ||
              (BackupMemOffset < 1) || (BackupMemOffset > 4))
            {
              fprintf (stderr,
                       "ERROR: -b parameter values must be between 1-4\n");
              ProgramExit (1);
            }
//                    printf ("Param val = %x\n", param);
          SpaceCheck (argv[arg][1]);
          strcpy (fname, argv[++arg]);
          OptB = 1;
          break;
        case '2':
          // 16bit EPP support enable
          DataSize16 = 1;
          break;
        case 'c':
          // Set cart size
          SpaceCheck (argv[arg][2]);
          ChipSize = (u16) (atoi (argv[++arg]));
          if ((ChipSize != 8) &&
              (ChipSize != 16) &&
              (ChipSize != 32) &&
              (ChipSize != 64) && (ChipSize != 128) && (ChipSize != 256))
            {
              fprintf (stderr,
                       "ERROR: Chip size must be 8,16,32,64,128 or 256.\n");
              ProgramExit (1);
            }
          break;
        case 'd':
          // Dump 256 bytes to screen
          SpaceCheck (argv[arg][2]);
          if (argv[++arg] != NULL)
            Base = (u8) (atoi (argv[arg]));
          printf ("Base address : %hx\n", Base * 256);
          OptD = 1;
          break;
        case 's':
          // Backup flash cart
          SpaceCheck (argv[arg][2]);
          strcpy (fname, argv[++arg]);
          OptS = 1;
          break;
        case 'p':
          // Program flash cart
          SpaceCheck (argv[arg][2]);
          strcpy (fname, argv[++arg]);
          OptP = 1;
          break;
        case 'r':
          SpaceCheck (argv[arg][2]);
          BackupMemOffset = *(char *) argv[++arg] - 0x30;
//                    SpaceCheck (argv[arg][1]);
//                    BackupMemSize = *(char *)argv[++arg] - 0x30;
//                    if ( (BackupMemSize<1) || (BackupMemSize>4) ||
          if ((BackupMemOffset < 1) || (BackupMemOffset > 4))
            {
              fprintf (stderr,
                       "ERROR: -r parameter value must be between 1-4\n");
              ProgramExit (1);
            }
//                    printf ("Param val = %x\n", param);
          SpaceCheck (argv[arg][1]);
          strcpy (fname, argv[++arg]);
          OptR = 1;
          break;
        case 'v':
          // Verify flash cart
          SpaceCheck (argv[arg][2]);
          strcpy (fname2, argv[++arg]);
          OptV = 1;
          break;
        case 'l':
          SpaceCheck (argv[arg][2]);
          i = atoi (argv[++arg]);
          if ((i > 0) && (i < 4))
            port = i;
          else
            {
              fprintf (stderr,
                       "ERROR: Only LPT1, LPT2, & LPT3 are supported");
              ProgramExit (1);
            }
          break;
        case 'm': 
          // uCON64 comment: See comment in LinkerInit(). Note that we reverse
          //  the meaning compared to the original code.
          EPPMode = 1;                          // Set EPP mode
          break;
        case 'n':
          // Don't repair header
          RepairHeader = 0;
          break;
        case 'w':
          SpaceCheck (argv[arg][2]);
          WaitDelay = atoi (argv[++arg]);
          break;
        case 'z':
          OptZ = 1;
          break;
        default:
//          usage (argv[0]);
          ProgramExit (1);
        }
    }

  InitPort (port);

  CheckForFC ();

  if (OptB)
    {
      //DumpSRAM ();
#if 0
      if (strchr (fname, '.') == NULL)
        strcat (fname, ".sav");
#endif
      if ((fp = fopen (fname, "wb")) == NULL)
        {
          fprintf (stderr, "ERROR: Can't open file \"%s\"\n", fname);
          ProgramExit (1);
        }
      printf ("Backing up backup SRAM to file \"%s\". Please wait...\n\n",
              fname);

      BackupSRAM (fp, BackupMemOffset, BackupMemSize);
      printf ("\n");
      fclose (fp);
    }

  if (OptD)
    dump (Base);

  if ((OptP) && ((Device == 0) || (Device == 0x2e) || (Device == 0xff)))
    {
      fprintf (stderr, "ERROR: Device type not recognized as programmable\n");
      ProgramExit (1);
    }

  if (OptR)
    {
#if 0
      if (strchr (fname, '.') == NULL)
        strcat (fname, ".sav");
#endif
      if ((fp = fopen (fname, "rb")) == NULL)
        {
          fprintf (stderr, "ERROR: Can't open file \"%s\"\n", fname);
          ProgramExit (1);
        }
      printf ("Restoring backup SRAM from file \"%s\". Please wait...\n\n",
              fname);

      RestoreSRAM (fp, BackupMemOffset);        //, BackupMemSize);
      printf ("\n");
      fclose (fp);
    }

//   if (OptD) dump (Base);

//   if ( (OptP) &&
//        ((Device == 0) || (Device == 0x2e) || (Device == 0xff)) )
//      {
//      fprintf (stderr, "ERROR: Device type not recognized as programmable\n");
//      ProgramExit (1);
//      }
//
  if (OptP)
    {
#if 0
      if (strchr (fname, '.') == NULL)
        strcat (fname, ".gba");
#endif
      if ((fp = fopen (fname, "rb")) == NULL)
        {
          fprintf (stderr, "ERROR: Can't open file \"%s\"\n", fname);
          ProgramExit (1);
        }
      printf ("Programming flash with file \"%s\".\n", fname);

      if (Device == 0xe2)
        ProgramSharpFlash (fp);
      else
        {
          if (VisolyTurbo)
            ProgramTurboIntelFlash (fp);
          else
            ProgramNonTurboIntelFlash (fp);
        }
      printf ("\n");
      fclose (fp);
    }

  if (OptS)
    {
#if 0
      if (strchr (fname, '.') == NULL)
        strcat (fname, ".gba");
#endif
      if ((fp = fopen (fname, "wb")) == NULL)
        {
          fprintf (stderr, "ERROR: Can't open file \"%s\"\n", fname);
          ProgramExit (1);
        }
      printf ("Backing up %d Mbits of ROM to file \"%s\". Please wait...\n\n",
              ChipSize, fname);

      BackupROM (fp, ChipSize << 16);
      printf ("\n");
      fclose (fp);
    }

  if (OptV)
    {
#if 0
      if (strchr (fname2, '.') == NULL)
        strcat (fname2, ".gba");
#endif
      if ((fp = fopen (fname2, "rb")) == NULL)
        {
          fprintf (stderr, "ERROR: Can't open file \"%s\"\n", fname2);
          ProgramExit (1);
        }
      printf ("Comparing flash with file \"%s\".\n", fname2);

      VerifyFlash (fp);
      fclose (fp);
    }

//   if (OptZ)
//      {
//      printf("Erase N blocks\n");
//      EraseNintendoFlashBlocks (0, 2);
//      }

  ProgramExit (0);
  exit (0);
}


/*
  It will save you some work if you don't fully integrate the code above with
  uCON64's code, because it is a project separate from the uCON64 project.
*/
int fal_argc = 0;
char *fal_argv[128];

void
fal_args (unsigned int parport)
{
  fal_argv[fal_argc++] = "fl";
  if (parport != 0x3bc && parport != 0x378 && parport != 0x278)
    {
      fprintf (stderr, "ERROR: PORT must be 0x3bc, 0x378 or 0x278\n");
      exit (1);
    }
  fal_argv[fal_argc++] = "-l";
  if (parport == 0x3bc)
    fal_argv[fal_argc++] = "3";
  else if (parport == 0x278)
    fal_argv[fal_argc++] = "2";
  else
    fal_argv[fal_argc++] = "1";                 // 0x378

  if (ucon64.parport_mode == UCON64_EPP)
    fal_argv[fal_argc++] = "-m";
}


int
fal_read_rom (const char *filename, unsigned int parport, int size)
{
  fal_args (parport);

  fal_argv[fal_argc++] = "-c";
  if (size != UCON64_UNKNOWN)
    {
      if (size == 8)
        fal_argv[fal_argc++] = "8";
      else if (size == 16)
        fal_argv[fal_argc++] = "16";
      else if (size == 32)
        fal_argv[fal_argc++] = "32";
      else if (size == 64)
        fal_argv[fal_argc++] = "64";
      else if (size == 128)
        fal_argv[fal_argc++] = "128";
      else if (size == 256)
        fal_argv[fal_argc++] = "256";
      else
        {
          fprintf (stderr, "ERROR: Invalid argument for -xfalc=n\n"
                           "       n can be 8, 16, 32, 64, 128 or 256\n");
          exit (1);
        }
    }
  else
    fal_argv[fal_argc++] = "32";

  fal_argv[fal_argc++] = "-s";
  fal_argv[fal_argc++] = (char *) filename;     // this is safe (FAL code
                                                //  doesn't modify argv)
  if (!fal_main (fal_argc, fal_argv))
    return 0;

  return -1;
}


int
fal_write_rom (const char *filename, unsigned int parport)
{
  fal_args (parport);

  fal_argv[fal_argc++] = "-p";
  fal_argv[fal_argc++] = (char *) filename;

  if (!fal_main (fal_argc, fal_argv))
    return 0;

  return -1;
}


int
fal_read_sram (const char *filename, unsigned int parport, int bank)
{
  char bank_str[2];

  fal_args (parport);

  fal_argv[fal_argc++] = "-b";
  if (bank == UCON64_UNKNOWN)
    {
      fal_argv[fal_argc++] = "1";
      fal_argv[fal_argc++] = "4";               // 256 kB
    }
  else
    {
      if (bank < 1 || bank > 4)
        {
          fprintf (stderr, "ERROR: Bank must be 1, 2, 3 or 4\n");
          exit (1);
        }
      bank_str[0] = '0' + bank;
      bank_str[1] = 0;                          // terminate string
      fal_argv[fal_argc++] = bank_str;
      fal_argv[fal_argc++] = "2";               // 64 kB
    }
  fal_argv[fal_argc++] = (char *) filename;

  if (!fal_main (fal_argc, fal_argv))
    return 0;

  return -1;
}


int
fal_write_sram (const char *filename, unsigned int parport, int bank)
{
  char bank_str[2];

  fal_args (parport);

  fal_argv[fal_argc++] = "-r";
  if (bank == UCON64_UNKNOWN)
    fal_argv[fal_argc++] = "1";
  else
    {
      if (bank < 1 || bank > 4)
        {
          fprintf (stderr, "ERROR: Bank must be 1, 2, 3 or 4\n");
          exit (1);
        }
      bank_str[0] = '0' + bank;
      bank_str[1] = 0;                          // terminate string
      fal_argv[fal_argc++] = bank_str;
    }
  fal_argv[fal_argc++] = (char *) filename;

  if (!fal_main (fal_argc, fal_argv))
    return 0;

  return -1;
}

#endif // PARALLEL
