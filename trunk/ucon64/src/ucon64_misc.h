/*
ucon64_misc.h - miscellaneous functions for uCON64

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
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/ioctl.h>
#ifdef BACKUP
	#include <sys/perm.h>
#endif
#include <sys/stat.h>
//#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "misc.h"

#define MBIT 131072

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

size_t filepad(	char *filename	//pad file (if necessary) from start
		,long start	//ignore start bytes (if file has header or something)
		,long unit	//size of block (example: MBIT)
);

long filetestpad(	char *filename	//test if EOF is padded (repeating bytes) beginning from start
);

int testsplit(	char *filename		//test if ROM is splitted into parts
);

/*
unsigned char inportb(	unsigned int arg1	//read a byte from the parallel port
);

unsigned char outportb(	unsigned int arg1	//write a byte to the p.p.
			,unsigned int arg2
);

unsigned short int inport(	unsigned int arg1	//read a word from the p.p.
);

unsigned short int outport(	unsigned int arg1	//write a word to the p.p.
				,unsigned int arg2
);
*/

#define outportb(p,x)	out1byte(p,x)
#define inportb(p)	in1byte(p)


void out1byte(	unsigned int port		//read a byte from the p.p.
		,unsigned char c
);

unsigned char in1byte(	unsigned int port	//write a byte to the p.p.
);



unsigned int parport_probe(	unsigned int parport	//detect parallel port
);

int parport_write(	char src[]
			,unsigned int len
			,unsigned int parport
);

int parport_read(	char dest[]
			,unsigned int len
			,unsigned int parport
);

int parport_gauge(	time_t init_time
			,long pos
			,long size
);

