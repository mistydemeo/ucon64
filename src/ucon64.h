/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
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
#ifndef UCON64_H
#define UCON64_H

//#include "config.h"

#include "ucon64_misc.h"
#include "ucon64_db.h"

//#define ucon64_KNOWN		-1
#define ucon64_UNKNOWN		0
#define ucon64_GB		1
#define ucon64_GENESIS		2
#define ucon64_SMS		3
#define ucon64_JAGUAR		4
#define ucon64_LYNX		5
#define ucon64_N64		6
#define ucon64_NEOGEO		7
#define ucon64_NES		8
#define ucon64_PCE		9
#define ucon64_PSX		10
#define ucon64_PS2		11
#define ucon64_SNES		12
#define ucon64_SATURN		13
#define ucon64_DC		14
#define ucon64_CD32		15
#define ucon64_CDI		16
#define ucon64_REAL3DO		17
#define ucon64_ATARI		18
#define ucon64_SYSTEM16		19
#define ucon64_NEOGEOPOCKET	20
#define ucon64_GBA		21
#define ucon64_VECTREX		22
#define ucon64_VIRTUALBOY	23
#define ucon64_WONDERSWAN	24
#define ucon64_COLECO		25
#define ucon64_INTELLI		26

#define ucon64_VERSION "1.9.7"

  #ifdef __UNIX__
	#define ucon64_TITLE "uCON64 1.9.7 Unix 1999-2001 by (various)"
  #elif __DOS__
	#define ucon64_TITLE "uCON64 1.9.7 DOS 1999-2001 by (various)"
  #elif __BEOS__
	#define ucon64_TITLE "uCON64 1.9.7 BeOS 1999-2001 by (various)"
  #elif __SOLARIS__
	#define ucon64_TITLE "uCON64 1.9.7 Solaris 1999-2001 by (various)"
  #else
	#define ucon64_TITLE "uCON64 1.9.7 1999-2001 by (various)"
  #endif

#define MBIT	131072

#define MAXBUFSIZE 32768

struct ucon64_
{
  int argc;
//  char argv[128][4096];
  char *argv[128];

//  char arg0[4096];
  char rom[4096];		//$ROM (cmdline) with path
  char file[4096];	//$FILE (cmdline) with path

  long console;	//integer for the console system

  char title[4096];
  char copier[4096];
  unsigned long bytes;	//size in bytes
  float mbit;		//size in mbit
  int interleaved;
  unsigned long padded;
  unsigned long intro;
  int splitted[128];

  unsigned long current_crc32;	//current crc32 value of ROM
  unsigned long db_crc32;	//crc32 value of ROM in internal database

  int has_internal_crc;	//ROM has internal CRC (Super Nintendo, Mega Drive, Gameboy)
    unsigned long current_internal_crc;	//calculated CRC
    unsigned long internal_crc;	//internal CRC
    long internal_crc_start;	//start of internal CRC in ROM header
    int internal_crc_len;	//length (in bytes) of internal CRC in ROM header
//  int has_internal_inverse_crc;	//ROM has internal inverted (Super Nintendo)
//    unsigned long internal_inverse_crc;	//internal CRC inverted
  char internal_crc2[4096];	//2nd or inverse internal CRC
    long internal_crc2_start;	//start of 2nd/inverse internal CRC
    int internal_crc2_len;	//length (in bytes) of 2nd/inverse internal CRC

  char buheader[512];	//(possible) header of backup unit
  long buheader_start;	//start of backup unit header (mostly 0)
  long buheader_len;	//length of backup unit header (==0)?no bu header

  char header[4096];	//(possible) internal ROM header
  long header_start;	//start of internal ROM header
  long header_len;	//length of internal ROM header (==0)?no header

  char name[4096];	//ROM name
  long name_start;	//start of internal ROM name (==0)?name comes from database
  long name_len;		//length of ROM name
	
  char manufacturer[4096];	//manufacturer name of the ROM
  long manufacturer_start;	//start of internal manufacturer name (==0)?manufacturer comes from database
  long manufacturer_len;	//length of manufacturer name
	
  char country[4096];	//country name of the ROM
  long country_start;	//start of internal country name (==0)? country comes from database
  long country_len;	//length of country name
	
  char misc[MAXBUFSIZE];	//some miscellaneous information about the ROM in one single string
};




#define ucon64_NAME	0
#define ucon64_ROM	1
#define ucon64_FILE	2

int ucon64_usage(int argc,char *argv[]);
int ucon64_init(struct ucon64_ *rom);
int ucon64_main(int argc,char *argv[]);
int ucon64_nfo(struct ucon64_ *rom);

int ucon64_exit(int value,struct ucon64_ *rom);


unsigned int ucon64_parport;

int ucon64_flush(int argc,char *argv[],struct ucon64_ *rom);

#endif                                          // #ifndef UCON64_H
