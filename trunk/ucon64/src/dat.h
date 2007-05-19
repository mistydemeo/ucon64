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
  const char *country;
} st_dat_entry_t;


typedef struct
{
  char fname[FILENAME_MAX];                     // dat filename

//  char datfile[FILENAME_MAX];                   // internal dat name
  char author[DAT_MAXBUFLEN];                   // dat author
  char version[DAT_MAXBUFLEN];                  // dat version
  char date[DAT_MAXBUFLEN];                     // dat date
  char comment[DAT_MAXBUFLEN];                  // dat comment
  char refname[DAT_MAXBUFLEN];                  // dat ref name
} st_dat_t;


/*
  dat_create()  create a empty DAT file
  dat_open()    open a existing DAT file
  dat_close()   close DAT file
  dat_read()    read entry with crc32 from DAT file
  dat_write()   add entry to DAT file

  dat_write_ansisql()
                write ANSI SQL script for importing
                  a DAT file into a SQL database
*/
extern int dat_create (const char *dat_fname,
            const char *author,
            const char *email,
            const char *homepage,
            const char *url,
            const char *version,
            const char *comment,
            const char *plugin,
            const char *refname);
extern st_dat_t *dat_open (const char *dat_fname);
extern int dat_close (st_dat_t *dat);
extern const st_dat_entry_t *dat_read (st_dat_t *dat, uint32_t crc32);
extern int dat_write (st_dat_t *dat, uint32_t crc32,
//                          const char *name,
//                          const char *misc,
                          const char *rom_name,
                          uint32_t rom_size);


extern int dat_write_ansisql (st_dat_t *dat, FILE *out);
                   
#endif // DAT_H
