/*
fal.c - Flash Advance Linker support for uCON64

written by 2001 Jeff Frohwein
           2001 NoisyB (noisyb@gmx.net)
           2001 dbjh

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

// To compile source on linux:
//   cc -o fl fl.c -O2
// You must have root access to run this under linux.


// RAM Detect notes for dev. (Just ignore!)
//-----------------------------------------
// To detect backup type.
//  1. First check for eeprom.
//  2. Read byte from 0xe000000.
//  3. Write new byte to 0xe000000.
//  4. If diff, write back data & exit with SRAM detect flag.
//  5. If no diff get device Manuf ID for flash.
//  6. If no Manuf ID detected then report no cart backup available.

#include "fal.h"
#include <time.h>

#define outp(p,v)  outportb(p,v); iodelay()
#define inp(p)   inportb(p)

//#define HEADER_LENGTH 0xc0
//#define OUTBUFLEN 256                   // Must be a multiple of 2! (ex:64,128,256...)

#define INTEL28F_BLOCKERASE 0x20
#define INTEL28F_CONFIRM    0xD0
#define INTEL28F_QUIRY      0x98
#define INTEL28F_READARRAY  0xff
#define INTEL28F_READSR     0x70
#define INTEL28F_RIC        0x90
#define INTEL28F_WRTOBUF    0xe8

#define SHARP28F_BLOCKERASE 0x20
#define SHARP28F_CONFIRM    0xD0
#define SHARP28F_WORDWRITE  0x10

#define u8      unsigned char
#define u16     unsigned int
#define u32     unsigned long
#define CONST_U8 const unsigned char

// ***Global Variables ***

//int WaitDelay,WaitNCDelay;
unsigned SPPDataPort;
unsigned SPPStatPort;
unsigned SPPCtrlPort;
unsigned EPPAddrPort;
unsigned EPPDataPort;

int debug,verbose;
int DataSize16;
int Device;
int EPPMode;
int RepairHeader;
int VisolyTurbo;
int WaitDelay;
int FileHeader[0xc0];
int HeaderBad;
int Complement = 0;

const unsigned char GoodHeader [] = {
 46,0,0,234,36,255,174,81,105,154,162,33,61,132,130,10,
 132,228,9,173,17,36,139,152,192,129,127,33,163,82,190,25,
 147,9,206,32,16,70,74,74,248,39,49,236,88,199,232,51,
 130,227,206,191,133,244,223,148,206,75,9,193,148,86,138,192,
 19,114,167,252,159,132,77,115,163,202,154,97,88,151,163,39,
 252,3,152,118,35,29,199,97,3,4,174,86,191,56,132,0,
 64,167,14,253,255,82,254,3,111,149,48,241,151,251,192,133,
 96,214,128,37,169,99,190,3,1,78,56,226,249,162,52,255,
 187,62,3,68,120,0,144,203,136,17,58,148,101,192,124,99,
 135,240,60,175,214,37,228,139,56,10,172,114,33,212,248,7
// 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,49,150,0,0,0,0,0,
// 0,0,0,0,0,240,0,0
};

void iodelay (void)
   {
   int i;
   for (i=0; i<WaitDelay; i++)
      {
      i++;
      i--;
      }
   }

void ProgramExit(int code)
   {
   exit(code);
   }

void usage(char *name)
   {
   char _small[255];
   char smaller[255];
   int i = 0;

   strcpy(_small, name);

#ifndef __linux__
   if (strchr(name, '.') != NULL)
      _small[strlen(_small)-4] = 0;               /* remove trailing file type */
#endif

   while ( (_small[strlen(_small)-i] != 0x2f) &&   /* loop until we find a / */
           ((strlen(_small)-i) > 0 ) )
      i++;

   if ((strlen(_small)-i) == 0) i++;

   strcpy (smaller, (char *)(&_small[strlen(_small)-i+1]) );

   fprintf (stderr, "GBA FLinker v1.4 by Jeff F.\n");
   fprintf (stderr, "Usage: %s [-c n][-d n][-h][-l n][-m][-p file][-s file][-v file][-w n]\n\n", smaller);

//   fprintf (stderr, "\t-b n\tBackup 32k bytes of SRAM into file\n");
   fprintf (stderr, "\t-c n\tSpecify chip size in mbits (8,16,32,64,128,256) (default=32)\n");
   fprintf (stderr, "\t-d n\tDump 256 bytes of ROM to screen (default: n=0)\n");
   fprintf (stderr, "\t-h\tThis help screen\n");
   fprintf (stderr, "\t-l n\tSpecify the parallel port to use (default is 1 = LPT1)\n");
   fprintf (stderr, "\t-m\tSet Standard Parallel Port (SPP) mode (default = EPP)\n");
   fprintf (stderr, "\t-n\tDo not repair incorrect header (default = repair header)\n");
   fprintf (stderr, "\t-p file\tProgram flash cart with file\n");
   fprintf (stderr, "\t-s file\tSave the cart into a file (Use -c to specify size)\n");
   fprintf (stderr, "\t-v file\tVerify flash cart with file\n");
   fprintf (stderr, "\t-w n\tAdd delay to make transfer more reliable\n");
   }

void InitPort (int port)
   {
   int ECPRegECR;

   if (port == 1)
       SPPDataPort = 0x378;
   else
       SPPDataPort = 0x278;

   SPPStatPort = SPPDataPort + 1;
   SPPCtrlPort = SPPDataPort + 2;
   EPPAddrPort = SPPDataPort + 3;
   EPPDataPort = SPPDataPort + 4;
   ECPRegECR   = SPPDataPort + 0x402;

//#ifndef __linux__
   if (EPPMode)
      { outp (ECPRegECR, 4); }             // Set EPP mode for ECP chipsets
   else
      { outp (ECPRegECR, 0); }             // Set SPP mode for ECP chipsets
//#endif
   }

void l4020a4 (int reg)
   {
   outp (SPPCtrlPort, 1);
   outp (SPPDataPort, reg);
   outp (SPPCtrlPort, 9);
   outp (SPPCtrlPort, 1);
   }

void l4020dc (int i)
   {
   outp (SPPDataPort, i);
   outp (SPPCtrlPort, 3);
   outp (SPPCtrlPort, 1);
   }

void l402108 (int reg, int adr)
   {
   l4020a4 (reg);
   l4020dc (adr);
   }

int SPPReadByte (void)                      // 402124
   {
   int v;

   outp (SPPCtrlPort, 0);
   outp (SPPCtrlPort, 2);
   v = ((inp(SPPStatPort)) >> 3) & 0xf;
   outp (SPPCtrlPort, 6);
   v += (((inp(SPPStatPort)) << 1) & 0xf0);
   outp (SPPCtrlPort, 0);

   return (v);
   }

void l402188 (int reg)
   {
   outp (SPPCtrlPort, 1);
   outp (EPPAddrPort, reg);
   }

void l4021a8 (int reg, int adr)
   {
   l402188 (reg);
   outp (SPPCtrlPort, 1);
   outp (EPPDataPort, adr);
   }

void l4021d0 (int i)
   {
   outp (SPPCtrlPort, 1);

   if (EPPMode)
      l402188 (i);
   else
      l4020a4 (i);
   }

void l402200 (int reg, int adr)
   {
   if (EPPMode)
      l4021a8 (reg, adr);
   else
      l402108 (reg, adr);
   }

void l402234 (void)
   {
   if (EPPMode)
      l402200 (4, 0x83);
   else
      l402200 (4, 0xc3);
   }

void l40226c (void)
   {
   if (EPPMode)
      l402200 (4, 7);
   else
      l402200 (4, 0x47);
   }

void l4022d0 (int i)
   {
   if (EPPMode)
      {
      outp (EPPDataPort, i);
      if (DataSize16)
         {
         outp (EPPDataPort, (i>>8));
         }
      }
   else
      {
      l4020dc(i);
      l4020dc(i>>8);
      }
   }

int PPReadByte (void)                  // 40234c
   {
   int v;

   if (EPPMode)
      v = inp (EPPDataPort);
   else
      v = SPPReadByte ();

   return (v);
   }

int ReadFlash (void)                      // 402368
   {
   int v = 0;

   if (EPPMode)
      {
      v = inp (EPPDataPort);
      if (DataSize16)
         v += (inp (EPPDataPort) << 8);
      }
   else
      {
      v = SPPReadByte (); //402124
      v += (SPPReadByte () << 8);
      }
   return (v);
   }

void SetCartAddr (int addr)             // 4023cc
   {
   l402200 (2,addr>>16);
   l402200 (1,addr>>8);
   l402200 (0,addr);
   }

void WriteFlash (int addr, int data)    // 402414
   {
   SetCartAddr (addr);
   l4021d0 (3);
   l4022d0 (data);
   }

void WriteRepeat (int addr, int data, int count)
   {
   int i;
   for (i=0; i<count; i++)
      WriteFlash (addr, data);
   }


void VisolyModePreamble (void)  // 402438
   {
   l40226c ();
   WriteRepeat (0x987654, 0x5354, 1);
   WriteRepeat ( 0x12345, 0x1234, 500);
   WriteRepeat (  0x7654, 0x5354, 1);
   WriteRepeat ( 0x12345, 0x5354, 1);
   WriteRepeat ( 0x12345, 0x5678, 500);
   WriteRepeat (0x987654, 0x5354, 1);
   WriteRepeat ( 0x12345, 0x5354, 1);
   WriteRepeat (0x765400, 0x5678, 1);
   WriteRepeat ( 0x13450, 0x1234, 1);
   WriteRepeat ( 0x12345, 0xabcd, 500);
   WriteRepeat (0x987654, 0x5354, 1);
   }

void SetVisolyFlashRWMode (void)
   {
   VisolyModePreamble ();
   WriteFlash (0xf12345, 0x9413);
   }

void SetVisolyBackupRWMode (int i)                     // 402550
   {
   VisolyModePreamble ();
   WriteFlash (0xa12345, i>>1);
   }

void l402684 (void)
   {
   outp (SPPStatPort, 1);
   l40226c ();
   }

void LinkerInit (void)
   {
   l402684 ();
//   l402200 (2,0x12);
//   l402200 (1,0x34);
//   l402200 (0,0x56);
   l4021d0 (2);
   outp (SPPCtrlPort, 0);
   outp (SPPCtrlPort, 4);
   }

//void l4027c4 (void)
//   {
//   LinkerInit();
//   }

int ReadStatusRegister (int addr)                     // 402dd8
   {
   int v;
   WriteFlash (addr,INTEL28F_READSR);
   outp (SPPCtrlPort, 0);
   v = ReadFlash (); // & 0xff;
   v = ReadFlash (); // & 0xff;
   return (v);
   }

void DumpSRAM (void)                    // 4046f4
   {
   int i,j,k,v;

   i=1;
   SetVisolyBackupRWMode (i);
   l402234 ();

   for (j=0; j<0x80; j++)
      {
      l402200 (0, 0);
      if (i==1)
         l402200 (1, j);
      else
         l402200 (1, j|0x80);
      l402200 (2, 0);
      l4021d0 (3);
      outp (SPPCtrlPort, 0);

      for (k=0; k<0x100; k++)
         {
         v = PPReadByte ();
         printf("%x ", v);
         }
      }
   printf("\n");
   }

void BackupROM (FILE *fp, int SizekW)
   {
   u16 valw;
   u32 i,j;
   int size = SizekW << 1, bytesread = 0;
   time_t starttime;

   WriteFlash (0, INTEL28F_READARRAY);         // Set flash (intel 28F640J3A) Read Mode


   starttime = time(NULL);
   for (i = 0; i < (SizekW>>8); i++)
      {
      SetCartAddr (i<<8);                            // Set cart base addr to 0
      l4021d0 (3);

      outp (SPPCtrlPort, 0);

      for (j = 0; j < 256; j++)
         {
         valw = ReadFlash ();
         fputc(valw & 0xff, fp);
         fputc(valw >> 8, fp);
         }
      bytesread += 256 << 1;                    // 256 words
      if ((bytesread & 0xffff) == 0)            // call parport_gauge() after receiving 64kB
         parport_gauge (starttime, bytesread, size);
      }
   }

void dump (u8 BaseAdr)
   {
//   unsigned char low, high;
   int i;
   u8 First = 1;
   u16 v;
   u8 val1,val2;
   u8 Display[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

   WriteFlash (0, INTEL28F_READARRAY);         // Set flash (intel 28F640J3A) Read Mode

   SetCartAddr (BaseAdr<<7);                   // Set cart base addr to read
   l4021d0 (3);

   outp (SPPCtrlPort, 0);

   for(i=0; i<128; i++)
      {
      if (First == 1)
         {
         if (i*2 < 256) printf("0");
         if (i*2 < 16) printf("0");
         printf("%hx - ",(i*2));
         First = 0;
         }

      v = ReadFlash();
      val2 = v >> 8;
      val1 = v & 255;

      if ((val1 > 31) & (val1 < 127))
       Display[(i & 7)*2] = val1;
      else
       Display[(i & 7)*2] = 46;

      if (val1 < 16)
         printf("0");
      printf("%hx ",val1);

      if ((val2 > 31) & (val2 < 127))

       Display[(i & 7)*2+1] = val2;
      else
       Display[(i & 7)*2+1] = 46;

      if (val2 < 16)
         printf("0");
      printf("%hx ",val2);

      if ((i & 7)==7)
         {
         First = 1;
         printf("   %3s",(&Display[0]));
         printf("\n");
         }
      }
   }

void CheckForFC (void)                  // 402f40
   {
   int Manuf;

   LinkerInit ();
   SetVisolyFlashRWMode ();
   WriteFlash (0, INTEL28F_RIC);          // Read Identifier codes from flash.
                                          // Works for intel 28F640J3A & Sharp LH28F320BJE.

   SetCartAddr (0);                     // Set cart addr to 0
   l4021d0 (3);

   outp (SPPCtrlPort, 0);

   Manuf = ReadFlash ();
   Device = ReadFlash ();

//   if (Manuf == 0x2e)
//      printf("Manufacturer ID = Standard ROM\n");
//   else
//      printf("Manufacturer ID = 0x%x: Device ID = 0x%x: ", Manuf, Device);

   printf("Manufacturer ID = 0x%x ", Manuf);
   switch (Manuf)
      {
      case 0x2e:
         // Standard ROM
         printf("(Standard ROM)\n");
         break;
      case 0x89:
         // Intel chips
         printf("(Intel): Device ID = 0x");
         switch (Device)
            {
            case 0x16:
               printf("16 (i28F320J3A)\n");
               break;
            case 0x17:
               printf("17 (i28F640J3A)\n");
               break;
            case 0x18:
               printf("18 (i28F128J3A)\n");
               break;
            default:
               // Check to see if this is a Visoly "Turbo" cart
               Device = ReadFlash ();
               printf("%x (Turbo - 2 x ", Device);
               VisolyTurbo = 1;
               switch (Device)
                  {
                  case 0x16:
                     printf("i28F320J3A)\n");
                     break;
                  case 0x17:
                     printf("i28F640J3A)\n");
                     break;
                  case 0x18:
                     printf("i28F128J3A)\n");
                     break;
                  default:
                     printf("(unknown)\n");
                     VisolyTurbo = 0;
                     break;
                  }
            }
         break;
      case 0xb0:
         // Sharp chips
         printf("(Sharp): Device ID = 0x%x ", Device);
         switch (Device)
            {
            case 0xe2:
               printf("(LH28F320BJE)\n");
               break;
            default:
               printf("(unknown)\n");
               break;
            }
         break;
      default:
         printf("(Unknown)\n");
         Device = 0;
      }
//   WriteFlash (0, INTEL28F_QUIRY);          // Read Identifier codes from flash.
//   SetCartAddr (0);                     // Set cart addr to 0
//   l4021d0 (3);

//   outp (SPPCtrlPort, 0);

//   for (i=0; i<32; i++)
//      printf("%x ", ReadFlash ());
//   printf("\n");

   }

int GetFileByte (FILE *fp)
   {
   static int FilePos = 0;
   int i = 0;

   if (RepairHeader)
      {
      // Set file pointer just past header
      if (FilePos == 0)
         for (i = 0; i < 0xa0; i++)
            (void) fgetc(fp);

      if (FilePos < 0xa0)
         {
         if ((HeaderBad) && (FilePos>3))
            i = GoodHeader [FilePos];
         else
            i = FileHeader [FilePos];
         }
      else
         if (FilePos == 0xbd)
            {
            (void) fgetc(fp);           // Discard complement in file
            i = Complement;
            }
         else
            i = fgetc(fp);
      }
   else
      i = fgetc(fp);

   FilePos++;
   return (i);
   }

int GetFileSize (FILE *fp)
   {
   int FileSize;

   if (RepairHeader)
      {
      int i;
      int j;

      FileSize = 0;
      while (!feof(fp) && (FileSize < 0xc0))
         {
         FileHeader[FileSize++] = fgetc(fp);
         }

      if (feof(fp))
         {
         fprintf(stderr, "ERROR: File must be 192 bytes or larger\n");
         ProgramExit(1);
         }
      else
         FileSize--;

      while (!feof(fp))
         {
         (void) fgetc(fp);
         FileSize++;
         }

      HeaderBad = 0;
      i = 4;
      while (i < 0x9c)
         {
         if (FileHeader[i] != GoodHeader[i])
            HeaderBad = 1;
         i++;
         }
      if (HeaderBad)
         printf("Fixing logo area. ");

      for (j=0xa0; j<0xbd; j++)
         Complement += FileHeader [j];
      Complement = (0-(0x19+Complement)) & 0xff;
//      printf("[Complement = 0x%x]", (int)Complement);
      if (FileHeader[0xbd] != Complement)
         printf("Fixing complement check.");

      if ((FileHeader[0xbd] != Complement) || HeaderBad)
         printf("\n");
      rewind(fp);
      }
   else
      {
      FileSize = -1;
      while (!feof(fp))
         {
         (void) fgetc(fp);
         FileSize++;
         }
      rewind(fp);
      }
   return (FileSize);
   }

// Program older (non-Turbo) Visoly flash cart

void ProgramNonIntIntelFlash (FILE *fp)
   {
   int i,j,k;
   int addr = 0;
   int done;
   int FileSize;
   time_t starttime;

   // Get file size
   FileSize = GetFileSize (fp);

   printf("Erasing Visoly non-turbo flash cart...\n");

//   j=1;
   for (i=0; i<(FileSize>>1); i=i+65536)
      {
      WriteFlash (i, INTEL28F_BLOCKERASE);          // Erase a block
      WriteFlash (i, INTEL28F_CONFIRM);             // Comfirm block erase
      while ((ReadStatusRegister(i) & 0x80)==0)
         {
         }
      }
//   ProgramExit(1);
   printf("Programming Visoly non-turbo flash cart...\n\n");
   //403018

   starttime = time(NULL);
   j = GetFileByte (fp);

   while (!feof(fp))
      {
      done = 0;
      while (!done)
         {
         WriteFlash (addr, INTEL28F_WRTOBUF);
         outp (SPPCtrlPort, 0);

         done = ReadFlash() & 0x80;
         }

      WriteFlash (addr, 0xf);              // Write 0xf+1 words

      SetCartAddr (addr);                           // Set cart base addr to 0
      l4021d0 (3);

      for (i=0; i<16; i++)
         {
         k = j;
         if (j != EOF)
            j = GetFileByte (fp);
         k += (j << 8);
         l4022d0 (k);

         if (j != EOF)
            j = GetFileByte (fp);
         }
      addr += 16;
      if ((addr & 0x3fff) == 0)                 // call parport_gauge() after sending 32kB
         parport_gauge (starttime, addr << 1, FileSize);
      l4022d0 (INTEL28F_CONFIRM);             // Comfirm block write
      }

   WriteFlash (0, INTEL28F_READARRAY);
   outp (SPPCtrlPort, 0);

   printf("\nDone.\n");
   }

// Program newer (Turbo) Visoly flash cart

void ProgramInterleaveIntelFlash (FILE *fp)
   {
   int i,j,k,z;
   int addr = 0;
   int done1,done2;
   int FileSize;
   time_t starttime;

   // Get file size
   FileSize = GetFileSize (fp);

   printf("Erasing Visoly turbo flash cart...\n");

   j=1;
   for (i=0; i<(FileSize>>1); i=i+(65536*2))
      {
      WriteFlash (i, INTEL28F_BLOCKERASE);          // Erase a block
      WriteFlash (i+1, INTEL28F_BLOCKERASE);        // Erase a block
      WriteFlash (i, INTEL28F_CONFIRM);             // Comfirm block erase
      while (((z=ReadStatusRegister(i)) & 0x80)==0)
         {
         }
      WriteFlash (i+1, INTEL28F_CONFIRM);             // Comfirm block erase
      while (((z=ReadStatusRegister(i+1)) & 0x80)==0)
         {
         }
      }
//   ProgramExit(1);
   printf("Programming Visoly turbo flash cart...\n\n");
   //403018
   starttime = time(NULL);
   j = GetFileByte (fp);

   while (!feof(fp))
      {
      done1=0;
      done2=0;
      while ((done1+done2)!=0x100)
         {
         if (done1 == 0) WriteFlash (addr+0, INTEL28F_WRTOBUF);
         if (done2 == 0) WriteFlash (addr+1, INTEL28F_WRTOBUF);

         SetCartAddr (addr);                           // Set cart base addr to 0
         l4021d0 (3);

         outp (SPPCtrlPort, 0);

         done1 = ReadFlash() & 0x80;
         done2 = ReadFlash() & 0x80;
         }

      WriteFlash (addr, 0xf);              // Write 0xf+1 words
      l4022d0 (0xf);

      SetCartAddr (addr);                           // Set cart base addr to 0
      l4021d0 (3);

      for (i=0; i<32; i++)
         {
         k = j;
         if (j != EOF)
            j = GetFileByte (fp);
         k += (j << 8);
         l4022d0 (k);

         if (j != EOF)
            j = GetFileByte (fp);
         }
      addr += 32;
      if ((addr & 0x3fff) == 0)                 // call parport_gauge() after sending 32kB
         parport_gauge (starttime, addr << 1, FileSize);
      l4022d0 (INTEL28F_CONFIRM);             // Comfirm block write
      l4022d0 (INTEL28F_CONFIRM);             // Comfirm block write
      }

   WriteFlash (0, INTEL28F_READARRAY);
   outp (SPPCtrlPort, 0);
   WriteFlash (1, INTEL28F_READARRAY);
   outp (SPPCtrlPort, 0);

   printf("\nDone.\n");
   }

// Program official Nintendo flash cart

void ProgramSharpFlash (FILE *fp)
   {
   int i,j;
   int k = 0;
   int addr = 0;
//   int done;
   int FileSize;
   time_t starttime;

   // Get file size
   FileSize = GetFileSize (fp);

   printf("Erasing flash cart...\n");
//   j = 0;
   for (i=0; i<(FileSize>>1); i=i+32768)
      {
//      printf("%d,%ld\n", ++j,i);
      WriteFlash (addr, SHARP28F_BLOCKERASE);          // Erase a block
      WriteFlash (addr, SHARP28F_CONFIRM);             // Comfirm block erase
      while ((ReadStatusRegister (0) & 0x80)==0)
         {
//         printf("RSR = %d\n", ReadStatusRegister ());
         }
//      printf(" %d,%ld\n", j,i);
      }

   printf("Programming Nintendo flash cart...\n\n");

   starttime = time(NULL);
   j = GetFileByte (fp);

   while (!feof(fp))
      {
      if (j != EOF)
         k = GetFileByte (fp);

      i = ((k & 0xff)<<8)+(j & 0xff);

      while ((ReadStatusRegister (0) & 0x80)==0)
         {
         }

      WriteFlash(addr, SHARP28F_WORDWRITE);
      WriteFlash(addr, i);
      addr += 1;

      j = GetFileByte (fp);
      if ((addr & 0x3fff) == 0)                 // call parport_gauge() after sending 32kB
         parport_gauge (starttime, addr << 1, FileSize);
      }

   WriteFlash (0, INTEL28F_READARRAY);
   outp (SPPCtrlPort, 0);

   printf("\nDone.\n");
   }

void VerifyFlash (FILE *fp)
   {
   int addr = 0;
   int CompareFail = 0;
   int k = 0;
   int i,j,m,n;

   WriteFlash (0, INTEL28F_READARRAY);         // Set flash (intel 28F640J3A) Read Mode

   j=0;
   while (!feof(fp))
      {
      j = fgetc(fp);
      if (j != EOF)
         {
         if ((addr & 0x1ff) == 0)
            {
            SetCartAddr (addr>>1);                            // Set cart base addr to read
            l4021d0 (3);

            outp (SPPCtrlPort, 0);
            }

         k = fgetc(fp);

         i = ReadFlash();
         m = i & 0xff;
         n = i >> 8;

         if (m != j)
            {
            printf("Address %x - Cartridge %x : File %hx\n", addr, m, j);
            CompareFail = 1;
            }
         if (n != k)
            {
            printf("Address %x - Cartridge %x : File %hx\n", addr+1, n, k);
            CompareFail = 1;
            }
         addr += 2;
         }
      }
   if (CompareFail == 0)
      printf("%d bytes compared ok.\n", addr);
   }

int fal_main(int argc, char **argv)
   {
   int arg,i;
   u8 Base = 0;
   FILE *fp;
   char fname[128],fname2[128];
   int OptB = 0;
   int OptD = 0;
   int OptP = 0;
   int OptS = 0;
   int OptV = 0;
   int port = 1;
   int ChipSize = 32;

   if (argc < 2)
      {
      usage(argv[0]);
      ProgramExit(1);
      }

   debug = 0;
   verbose = 1;
   EPPMode = 1;
   DataSize16 = 1;
   WaitDelay = 0;
   VisolyTurbo = 0;
   RepairHeader = 1;

   for (arg = 1; arg < argc; arg++)
      {
      if (argv[arg][0] != '-')
         {
         usage(argv[0]);
         ProgramExit(1);
         }

      switch(argv[arg][1])
         {
//            case 'b':
//                    OptB=1;
//                    break;
            case 'c':
                    // Set cart size
                    ChipSize = (u16)(atoi(argv[++arg]));
                    if ( (ChipSize != 8) &&
                         (ChipSize != 16) &&
                         (ChipSize != 32) &&
                         (ChipSize != 64) &&
                         (ChipSize != 128) &&
                         (ChipSize != 256) )
                       {
                       fprintf(stderr, "ERROR: Chip size must be 8,16,32,64,128 or 256.\n");
                       ProgramExit(1);
                       }
                    break;
            case 'd':
                    // Dump 256 bytes to screen
                    if (argv[++arg] != NULL)
                       Base = (u8)(atoi(argv[arg]));
                    printf("Base address : %hx\n",Base*256);
                    OptD = 1;
                    break;
            case 's':
                    // Backup flash cart
                    strcpy(fname, argv[++arg]);
                    OptS = 1;
                    break;
            case 'p':
                    // Program flash cart
                    strcpy(fname, argv[++arg]);
                    OptP = 1;
                    break;
            case 'v':
                    // Verify flash cart
                    strcpy(fname2, argv[++arg]);
                    OptV = 1;
                    break;
            case 'l':
                    i = atoi(argv[++arg]);
                    if ((i==1) || (i==2))
                       port = i;
                    else
                       {
                       fprintf(stderr, "ERROR: Only LPT1 & LPT2 are supported");
                       ProgramExit(1);
                       }
                    break;
            case 'm':
                    // Set SPP mode
                    EPPMode = 0;
                    break;
            case 'n':
                    // Don't repair header
                    RepairHeader = 0;
                    break;
            case 'w':
                    WaitDelay = atoi(argv[++arg]);
                    break;
            default:
                    usage(argv[0]);
                    ProgramExit(1);
         }
      }

   InitPort(port);

   CheckForFC ();

   if (OptB) DumpSRAM ();

   if (OptD) dump (Base);

   if ( (OptP) &&
        ((Device == 0) || (Device == 0xffff)) )
      {
      fprintf(stderr, "ERROR: Device type unrecognized\n");
      ProgramExit(1);
      }

   if (OptP)
      {
#ifndef __linux__
      if (strchr(fname, '.') == NULL)
         strcat(fname, ".gba");
#endif
      if ((fp = fopen(fname, "rb")) == NULL)
         {
         fprintf(stderr, "ERROR trying to open file '%s'\n", fname);
         ProgramExit(1);
         }
      printf("Programming flash with file '%s'.\n", fname);

      if (Device == 0xe2)
         ProgramSharpFlash (fp);
      else
         {
         if (VisolyTurbo)
            ProgramInterleaveIntelFlash (fp);
         else
            ProgramNonIntIntelFlash (fp);
         }
      fclose(fp);
      }

   if (OptS)
      {
#ifndef __linux__
      if (strchr(fname, '.') == NULL)
         strcat(fname, ".gba");
#endif
      if ((fp = fopen(fname, "wb")) == NULL)
         {
         fprintf(stderr, "ERROR trying to open file '%s'\n", fname);
         ProgramExit(1);
         }
      printf("Backing up %d mbits of ROM to file '%s'. Please wait...\n\n", ChipSize, fname);

      BackupROM (fp, ChipSize<<16);
      printf("\nDone.\n");
      fclose(fp);
      }

   if (OptV)
      {
#ifndef __linux__
      if (strchr(fname2, '.') == NULL)
         strcat(fname2, ".gba");
#endif
      if ((fp = fopen(fname2, "rb")) == NULL)
         {
         fprintf(stderr, "ERROR trying to open file '%s'\n", fname2);
         ProgramExit(1);
         }
      printf("Comparing flash with file '%s'.\n", fname2);

      VerifyFlash (fp);
      fclose(fp);
      }

   ProgramExit(0);
   exit(0);
   }


/*
  It will save you some work if you don't fully integrate the code above with uCON64's code,
  because it is a project separate from the uCON64 project.
*/
int fal_argc;
char *fal_argv[128];

int fal_read(char *filename, unsigned int parport, int argc, char *argv[])
{
  fal_argv[0] = "fal";

  fal_argv[1] = "-c";
  if (argncmp(argc, argv, "-xfalc", 6))		// strlen("-xfalc") == 6
  {
    if (argcmp(argc, argv, "-xfalc8"))
      fal_argv[2] = "8";
    else if (argcmp(argc, argv, "-xfalc16"))
      fal_argv[2] = "16";
    else if (argcmp(argc, argv, "-xfalc32"))
      fal_argv[2] = "32";
    else if (argcmp(argc, argv, "-xfalc64"))
      fal_argv[2] = "64";
    else if (argcmp(argc, argv, "-xfalc128"))
      fal_argv[2] = "128";
    else if (argcmp(argc, argv, "-xfalc256"))
      fal_argv[2] = "256";
    else
    {
      fprintf(stderr, "Invalid argument for -xfalc<n>\n"
                      "n can be 8, 16, 32, 64, 128 or 256; default is -xfalc32\n");
      exit(1);
    }
  }
  else
    fal_argv[2] = "32";

  if (parport != 0x378 && parport != 0x278)
  {
    fprintf(stderr,
            "PORT must be 0x378 or 0x278\n"
            "If you didn't specify PORT on the command line, change the address in your BIOS\n"
            "setup\n");
    fflush(stdout);
    exit(1);
  }
  fal_argv[3] = "-l";
  if (parport == 0x378)
    fal_argv[4] = "1";
  else
    fal_argv[4] = "2";

  fal_argv[5] = "-s";
  fal_argv[6] = filename;
  fal_argc = 7;

  if (argcmp(argc, argv, "-xfalm"))
  {
    fal_argv[7] = "-m";
    fal_argc++;
  }

  if (!fal_main(fal_argc, fal_argv))
  {
    return 0;
  }

  return -1;
}

int fal_write(char *filename, long start, long len, unsigned int parport, int argc, char *argv[])
{
  fal_argv[0] = "fal";

  if (argncmp(argc, argv, "-xfalc", 6))		// strlen("-xfalc") == 6
  {
    fprintf(stderr, "-xfalc<n> can only be used when receiving a ROM\n");
    exit(1);
  }
  if (parport != 0x378 && parport != 0x278)
  {
    fprintf(stderr,
            "PORT must be 0x378 or 0x278\n"
            "If you didn't specify PORT on the command line, change the address in your BIOS\n"
            "setup\n");
    fflush(stdout);
    exit(1);
  }
  fal_argv[1] = "-l";
  if (parport == 0x378)
    fal_argv[2] = "1";
  else
    fal_argv[2] = "2";

  fal_argv[3] = "-p";
  fal_argv[4] = filename;
  fal_argc = 5;

  if (argcmp(argc, argv, "-xfalm"))
  {
    fal_argv[5] = "-m";
    fal_argc++;
  }

  if (!fal_main(fal_argc, fal_argv))
  {
    return 0;
  }

  return -1;
}

int fal_usage(int argc, char *argv[])
{
  if (argcmp(argc, argv, "-help"))
    printf("\n%s\n", fal_TITLE);

  printf("  -xfal         send/receive ROM to/from Flash Advance Linker; $FILE=PORT\n"
         "                receives automatically when $ROM does not exist\n"
         "  -xfalc<n>     specify chip size in mbits of ROM in Flash Advance Linker when\n"
         "                receiving. n can be 8,16,32,64,128 or 256. default is -xfalc32\n"
         "  -xfalm        use SPP mode, default is EPP\n");

  if (argcmp(argc, argv, "-help"))
  {
  //TODO more info like technical info about cabeling and stuff for the copier
  }

  return 0;
}
