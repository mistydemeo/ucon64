/*
ucon64_misc.h - miscellaneous functions for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2001 dbjh
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

#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>				// ioperm() (libc5)
#include "misc.h"

#ifdef  BACKUP
#ifdef  __linux__
#ifdef  __GLIBC__
#include <sys/io.h>                             // ioperm() (glibc)
#endif

#elif   __DOS__
#include <pc.h>                                 // inportb(), inportw()

#elif   __BEOS__
#include <fcntl.h>

#define DRV_READ_IO_8 'r'
#define DRV_WRITE_IO_8 'w'
#define DRV_READ_IO_16 'r16'
#define DRV_WRITE_IO_16 'w16'

typedef struct IO_Tuple
{
  unsigned long Port;
  unsigned char  Data;
  unsigned short Data16;
} IO_Tuple;
#endif                                          // __BEOS__
#endif                                          // BACKUP


#define MBIT 131072

#define PARPORT_DATA    0                       // output
#define PARPORT_STATUS  1                       // input
#define PARPORT_CONTROL 2

char hexDigit(	int value	//GameGenie "codec" routine
);

int hexValue(	char digit	//GameGenie "codec" routine
);

//#define hexByteValue(x ,y) ((hexValue(x) << 4) + hexValue(y))
int hexByteValue(	char x	//GameGenie "codec" routine
			,char y
);

void BuildCRCTable();		//needed for CRC32

unsigned long CalculateBufferCRC(	unsigned int count	//needed for CRC32
					,unsigned long crc
					,void *buffer
);

unsigned long CalculateFileCRC(	FILE *file	//needed for CRC32
);

unsigned long fileCRC32(	char *filename	//calculate CRC32 of filename beginning from start
				,long start
);

char *ucon64_fbackup(   char *filename
);

size_t filepad(	char *filename	//pad file (if necessary) from start
		,long start	//ignore start bytes (if file has header or something)
		,long unit	//size of block (example: MBIT)
);

long filetestpad(	char *filename	//test if EOF is padded (repeating bytes) beginning from start
);

int testsplit(	char *filename		//test if ROM is splitted into parts
);

#define out1byte(p,x)	outportb(p,x)
#define in1byte(p)	inportb(p)
// DJGPP (DOS) has outportX() & inportX()

#if     (__UNIX__ || __BEOS__)
inline unsigned char inportb(unsigned short port);
inline unsigned short inportw(unsigned short port);
inline void outportb(unsigned short port, unsigned char byte);
inline void outportw(unsigned short port, unsigned short word);
#endif

unsigned int parport_probe(	unsigned int parport	//detect parallel port
);

int parport_gauge(	time_t init_time
			,long pos
			,long size
);

int raw2iso(char *filename);	//convert RAW (2352 Bytes/Sector) images to ISO9660 (2048 Bytes/Sector) images

#endif                                          // #ifndef UCON64_MISC_H
