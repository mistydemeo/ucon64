/*
dat.c - DAT files support with index files for faster access

Copyright (c) 1999 - 2006 NoisyB


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
#ifndef DAT_H
#define DAT_H


#define DAT_MAXBUFLEN 2048


typedef struct
{
  uint32_t crc32;                               // CRC32 checksum of the ROM
  char name[DAT_MAXBUFLEN];                     // name of the ROM
  char misc[DAT_MAXBUFLEN];                     // miscellaneous information about the ROM
  char rom_name[FILENAME_MAX];                  // filename of the ROM
  uint32_t rom_size;                            // size in bytes
} st_dat_entry_t;


typedef struct
{
  char dat_fname[FILENAME_MAX];                 // dat filename
  char datfile[FILENAME_MAX];                   // internal dat name
  char author[DAT_MAXBUFLEN];                   // dat author
  char version[DAT_MAXBUFLEN];                  // dat version
  char date[DAT_MAXBUFLEN];                     // dat date
  char comment[DAT_MAXBUFLEN];                  // dat comment
  char refname[DAT_MAXBUFLEN];                  // dat ref name
} st_dat_t;


#define DAT_FLAG_IDX 1
extern st_dat_t *dat_open (const char *dat_fname, int flags);
#if 0
extern st_dat_t *dat_create (const char *dat_fname,
                             const char *author,
                             const char *version,
                             const char *refname,
                             const char *comment,
                             const char *date);
#endif
extern int dat_close (st_dat_t *dat);


//#define DAT_SEEK_SET 0
//#define DAT_SEEK_CUR 1
//#define DAT_SEEK_END 2
//extern int dat_seek_by_entry (st_dat_t *dat, int entry, int offset);


extern st_dat_entry_t *dat_read (st_dat_t *dat, uint32_t crc32);
#if 0
extern int dat_write (st_dat_t *dat, uint32_t crc32,
                                     const char *name,
                                     const char *misc,
                                     const char *rom_name,
                                     uint32_t rom_size);
#endif

                   
#endif // DAT_H
