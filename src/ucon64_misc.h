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

#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>             // ioperm() (libc5)
#include "ucon64.h"
#include "misc.h"

#ifdef  BACKUP
#ifdef  __linux__
#ifdef  __GLIBC__
#include <sys/io.h>             // ioperm() (glibc)
#endif

#elif   defined __MSDOS__
#include <pc.h>                 // inportb(), inportw()

#elif   defined __BEOS__
#include <fcntl.h>

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
#if     defined __UNIX__ || defined __BEOS__
unsigned char inportb (unsigned short port);
unsigned short inportw (unsigned short port);
void outportb (unsigned short port, unsigned char byte);
void outportw (unsigned short port, unsigned short word);

#ifdef __FreeBSD__
  #define ioperm(p,x,y) (i386_set_ioperm(p,x,y))
#endif // __FreeBSD__

#endif

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
unsigned long unif_crc32 (unsigned long dummy, unsigned char *prg_code, size_t size);

unsigned long fileCRC32 (char *filename, long start);   // calculate CRC32 of filename beginning from start

//ucon64 specific wrapper for misc.c/filebackup()
char *ucon64_fbackup (struct ucon64_ *rom, char *filename);

size_t filepad (char *filename, long start, long unit);//pad a ROM in Mb
long filetestpad (char *filename); //test if a ROM is padded

int testsplit (char *filename);//test if a ROM is splitted

unsigned int parport_probe (unsigned int parport);

int fparport_gauge (FILE *output, time_t init_time, long pos, long size);
#define parport_gauge(a, b, c) (fparport_gauge(frontend_file, a, b, c))

int trackmode (long imagesize);

int raw2iso (char *filename);

#endif // #ifndef UCON64_MISC_H
