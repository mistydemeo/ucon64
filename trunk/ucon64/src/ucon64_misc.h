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
  extern unsigned char inportb (unsigned short port);
  extern unsigned short inportw (unsigned short port);
  extern void outportb (unsigned short port, unsigned char byte);
  extern void outportw (unsigned short port, unsigned short word);
#endif // defined __unix__ || defined __BEOS__
#endif // BACKUP

#define MODE1_2048 0
#define MODE1_2352 1
#define MODE2_2336 2
#define MODE2_2352 3
extern const char *track_modes[];

#define MBIT 131072

#define PARPORT_DATA    0       // output
#define PARPORT_STATUS  1       // input
#define PARPORT_CONTROL 2
/*
  defines for unknown backup units/emulators
*/
typedef struct st_unknown_header //uses swc header layout
{
  unsigned char low;
  unsigned char high;
  unsigned char emulation;
  char pad[5];
  unsigned char code1; // 0xAA
  unsigned char code2; // 0xBB
  unsigned char type;
  char pad2[501];
} st_unknown_header_t;
#define UNKNOWN_HEADER_START 0
#define UNKNOWN_HEADER_LEN (sizeof (st_unknown_header_t))
extern const char *unknown_usage[];

/*
  CRC32 routines
*/
extern unsigned long CalculateBufferCRC (unsigned int count, unsigned long crc,
                                  void *buffer);
/*
  like zlib/crc32(); uCON64 has it's own crc calc. stuff
  this is just a wrapper
*/
#ifndef UNZIP
//extern unsigned long unif_crc32 (unsigned long dummy, unsigned char *prg_code, size_t size);
#endif
extern unsigned long fileCRC32 (const char *filename, long start);   // calculate CRC32 of filename beginning from start

/*
  wrapper for misc.c/filebackup()
*/
extern const char *ucon64_fbackup (const char *filename);
/*
  wrapper for misc.c/gauge()
*/
extern int ucon64_gauge (time_t init_time, long pos, long size);

extern size_t filepad (const char *filename, long start, long unit);//pad a ROM in Mb
extern long filetestpad (const char *filename); //test if a ROM is padded

extern int ucon64_testsplit (const char *filename);//test if a ROM is splitted

extern unsigned int ucon64_parport_probe (unsigned int parport); 
extern const char *ucon64_parport_error; //std. error message for parport
extern const char *ucon64_console_error; //std. error message if the correct console couldn't be found

extern int ucon64_bin2iso (const char *image, int track_mode);
extern int ucon64_trackmode_probe (const char *image);

extern int ucon64_e (const char *rominfo);
extern int ucon64_ls (const char *path, int mode);
extern int ucon64_configfile (void);

#endif // #ifndef UCON64_MISC_H
