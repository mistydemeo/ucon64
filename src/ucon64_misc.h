/*
ucon64_misc.h - miscellaneous functions for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh
                  2001 Caz


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
#ifndef UCON64_MISC_H
#define UCON64_MISC_H

#ifdef  BACKUP
#ifdef  __BEOS__
#define DRV_READ_IO_8 'r'
#define DRV_WRITE_IO_8 'w'
#define DRV_READ_IO_16 'r16'
#define DRV_WRITE_IO_16 'w16'

typedef struct IO_Tuple
{
  unsigned long Port;
  unsigned char Data;
  unsigned short Data16;
}
IO_Tuple;
#endif // __BEOS__

#define out1byte(p,x)   outportb(p,x)
#define in1byte(p)      inportb(p)

// DJGPP (DOS) has outportX() & inportX()
#if     defined __unix__ || defined __BEOS__
  unsigned char inportb (unsigned short port);
  unsigned short inportw (unsigned short port);
  void outportb (unsigned short port, unsigned char byte);
  void outportw (unsigned short port, unsigned short word);
#endif // defined __unix__ || defined __BEOS__
#endif // BACKUP

#define MBIT 131072

#define PARPORT_DATA    0       // output
#define PARPORT_STATUS  1       // input
#define PARPORT_CONTROL 2

// GameGenie "codec" routines
char hexDigit (int value);
int hexValue (char digit);
//#define hexByteValue(x ,y) ((hexValue(x) << 4) + hexValue(y))
int hexByteValue (char x, char y);

// CRC32 routines
void BuildCRCTable ();
unsigned long CalculateBufferCRC (unsigned int count, unsigned long crc,
                                  void *buffer);
unsigned long CalculateFileCRC (FILE * file);

/*
  like zlib/crc32(); uCON64 has it's own crc calc. stuff
  this is just a wrapper
*/
//unsigned long unif_crc32 (unsigned long dummy, unsigned char *prg_code, size_t size);
unsigned long fileCRC32 (char *filename, long start);   // calculate CRC32 of filename beginning from start

//ucon64 specific wrapper for misc.c/filebackup()
char *ucon64_fbackup (struct ucon64_ *rom, char *filename);

size_t filepad (char *filename, long start, long unit);//pad a ROM in Mb
long filetestpad (char *filename); //test if a ROM is padded

int testsplit (char *filename);//test if a ROM is splitted

unsigned int parport_probe (unsigned int parport);

//ucon64 specific wrapper for misc.c/gauge()
int
ucon64_gauge (struct ucon64_ *rom, time_t init_time, long pos, long size);

int trackmode_probe (long imagesize);

#endif // #ifndef UCON64_MISC_H
