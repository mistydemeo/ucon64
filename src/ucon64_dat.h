/*
ucon64_dat.h - support for DAT files as known from Romcenter, Goodxxx, etc.

written by 1999 - 2003 NoisyB (noisyb@gmx.net)


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
#ifndef UCON64_DAT_H
#define UCON64_DAT_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "misc.h"

typedef struct
{
  uint32_t current_crc32;       // standard current_crc32 checksum of the ROM
  uint8_t console;              // integer for the console system
  char name[MAXBUFSIZE];        // name of the ROM
  const char *maker;                // maker of the ROM
  const char *country;            // country of the ROM
  char misc[MAXBUFSIZE];        // miscellaneous information about the ROM
  char fname[FILENAME_MAX];     // filename of the ROM
  uint32_t fsize;               // size in bytes

  char datfile[FILENAME_MAX];   // name of the dat file
  char author[100];             // author of dat file
  char version[100];            // version of dat file
  char date[20];                // date of dat file
  char comment[MAXBUFSIZE];     // comment of dat file
  char refname[100];            // ref name (could this be used to find out which console system?)
  
  const char **console_usage;   // console system usage
  const char **copier_usage;                    // backup unit usage
} ucon64_dat_t;

extern ucon64_dat_t *ucon64_dat_search (uint32_t crc32, ucon64_dat_t *dat); //search dat files for crc and return ucon64_dat_t
extern unsigned int ucon64_dat_total_entries (int console);       // returns # of ROMs in all DAT's
extern int ucon64_dat_view (int console);        // printf the complete dat collection
extern int ucon64_dat_indexer (void);          // create or update index file for DAT's
extern void ucon64_dat_nfo (const ucon64_dat_t *dat); // view contents of ucon64_dat_t

#endif // UCON64_DAT_H

