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

enum
{
  MODE1_2048,
  MODE1_2352,
  MODE2_2336,
  MODE2_2352
};
extern const char *track_modes[];

#define MBIT 131072

#define PARPORT_DATA    0       // output
#define PARPORT_STATUS  1       // input
#define PARPORT_CONTROL 2
/*
  defines for unknown backup units/emulators
*/
#define UNKNOWN_HEADER_START 0
#define unknown_HEADER_LEN 512
extern const char *unknown_title;

// GameGenie "codec" routines
extern char hexDigit (int value);
extern int hexValue (char digit);
//#define hexByteValue(x ,y) ((hexValue(x) << 4) + hexValue(y))
extern int hexByteValue (char x, char y);


// CRC32 routines
extern unsigned long CalculateBufferCRC (unsigned int count, unsigned long crc,
                                  void *buffer);
/*
  like zlib/crc32(); uCON64 has it's own crc calc. stuff
  this is just a wrapper
*/
//extern unsigned long unif_crc32 (unsigned long dummy, unsigned char *prg_code, size_t size);
extern unsigned long fileCRC32 (char *filename, long start);   // calculate CRC32 of filename beginning from start

//ucon64 specific wrapper for misc.c/filebackup()
extern char *ucon64_fbackup (char *filename);

extern size_t filepad (const char *filename, long start, long unit);//pad a ROM in Mb
extern long filetestpad (const char *filename); //test if a ROM is padded

extern int ucon64_testsplit (const char *filename);//test if a ROM is splitted

extern unsigned int ucon64_parport_probe (unsigned int parport); 

//ucon64 specific wrapper for misc.c/gauge()
extern int ucon64_gauge (time_t init_time, long pos, long size);

extern int ucon64_bin2iso (const char *image, int track_mode);
extern int ucon64_trackmode_probe (const char *image);

#define ISODCL(from, to) (to - from + 1)

typedef struct st_iso_primary_descriptor
{
  char type[ISODCL (1, 1)];   /* 711 */
  char id[ISODCL (2, 6)];
  char version[ISODCL (7, 7)];        /* 711 */
  char unused1[ISODCL (8, 8)];
  char system_id[ISODCL (9, 40)];     /* achars */
  char volume_id[ISODCL (41, 72)];    /* dchars */
  char unused2[ISODCL (73, 80)];
  char volume_space_size[ISODCL (81, 88)];    /* 733 */
  char unused3[ISODCL (89, 120)];
  char volume_set_size[ISODCL (121, 124)];    /* 723 */
  char volume_sequence_number[ISODCL (125, 128)];     /* 723 */
  char logical_block_size[ISODCL (129, 132)]; /* 723 */
  char path_table_size[ISODCL (133, 140)];    /* 733 */
  char type_l_path_table[ISODCL (141, 144)];  /* 731 */
  char opt_type_l_path_table[ISODCL (145, 148)];      /* 731 */
  char type_m_path_table[ISODCL (149, 152)];  /* 732 */
  char opt_type_m_path_table[ISODCL (153, 156)];      /* 732 */
  char root_directory_record[ISODCL (157, 190)];      /* 9.1 */
  char volume_set_id[ISODCL (191, 318)];      /* dchars */
  char publisher_id[ISODCL (319, 446)];       /* achars */
  char preparer_id[ISODCL (447, 574)];        /* achars */
  char application_id[ISODCL (575, 702)];     /* achars */
  char copyright_file_id[ISODCL (703, 739)];  /* 7.5 dchars */
  char abstract_file_id[ISODCL (740, 776)];   /* 7.5 dchars */
  char bibliographic_file_id[ISODCL (777, 813)];      /* 7.5 dchars */
  char creation_date[ISODCL (814, 830)];      /* 8.4.26.1 */
  char modification_date[ISODCL (831, 847)];  /* 8.4.26.1 */
  char expiration_date[ISODCL (848, 864)];    /* 8.4.26.1 */
  char effective_date[ISODCL (865, 881)];     /* 8.4.26.1 */
  char file_structure_version[ISODCL (882, 882)];     /* 711 */
  char unused4[ISODCL (883, 883)];
  char application_data[ISODCL (884, 1395)];
  char unused5[ISODCL (1396, 2048)];
} st_iso_primary_descriptor_t;
#endif // #ifndef UCON64_MISC_H
