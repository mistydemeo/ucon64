/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh


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

#include <dirent.h>                             // for *temp
#include "config.h"                             // ANSI_COLOR
#include "getopt.h"                             // for struct option
#include "ucon64_defines.h"

/*
  this struct holds only workflow relevant information
*/
typedef struct st_ucon64
{
#ifdef TODO
//#warning TODO get rid of argc and argv here
#endif // TODO
  int argc;
  char **argv;

  const char *rom;                              // rom (cmdline) with path
  char rom_in_archive[FILENAME_MAX];            // filename holder if the rom comes from an archive

  const char *file;                             // file (cmdline) with path

//  char temp_file[FILENAME_MAX];

  char output_path[FILENAME_MAX];               // -o argument (default: cwd)

#ifdef  ANSI_COLOR
  int ansi_color;
#endif

  unsigned int parport;                         // parallel port address
  int parport_mode;                             // parallel port mode: ECP, EPP, SPP, other
  char configfile[FILENAME_MAX];                // path and name of the config file

  int backup;                                   // flag if backups files should be created
  int frontend;                                 // flag if uCON64 was started by a frontend

  int show_nfo;                                 // show or skip info output for ROM

#define UCON64_ISSET(x) (x != UCON64_UNKNOWN)
  int buheader_len;                            // length of backup unit header 0 == no bu hdr
  int split;                                    // rom is split
  int snes_hirom;                               // SNES ROM is HiROM
  int bs_dump;                                  // SNES "ROM" is a Broadcast Satellaview dump
  int controller;                               // NES UNIF & SNES NSRT
  int controller2;                              // SNES NSRT
  int tv_standard;                              // NES UNIF
  int battery;                                  // NES UNIF/iNES/Pasofami
  int vram;                                     // NES UNIF
  int mirror;                                   // NES UNIF/iNES/Pasofami
  const char *mapr;                             // NES UNIF board name or iNES mapper number
  int use_dump_info;                            // NES UNIF
  const char *comment;                          // NES UNIF
  int interleaved;                              // rom is interleaved (swapped)
  int console;                                  // the detected console system
  int do_not_calc_crc;                          // disable checksum calc. to speed up --ls,--lsv, etc.

#define UCON64_TYPE_ISROM(x) (x == UCON64_ROM)
#define UCON64_TYPE_ISCD(x) (x == UCON64_CD)
  int type;                                     // rom type ROM or CD image
} st_ucon64_t;

extern st_ucon64_t ucon64;

typedef struct st_rominfo
{
  const char **console_usage;                   // console system usage
  const char **copier_usage;                    // backup unit usage

  int interleaved;                              // ROM is interleaved (swapped)
  int snes_hirom;                               // SNES ROM is HiROM
  int data_size;                                // ROM data size without "red tape"
  int file_size;                                // (uncompressed) ROM file size

  int buheader_start;                           // start of backup unit header (mostly 0)
  int buheader_len;                             // length of backup unit header 0 == no bu hdr
  const void *buheader;                         // (possible) header of backup unit

  int header_start;                             // start of internal ROM header
  int header_len;                               // length of internal ROM header 0 == no hdr
  const void *header;                           // (possible) internal ROM header

  char name[MAXBUFSIZE];                        // internal ROM name
  const char *maker;                            // maker name of the ROM
  const char *country;                          // country name of the ROM
  char misc[MAXBUFSIZE];                        // some miscellaneous information
                                                //  about the ROM in one single string
  unsigned int current_crc32;                   // current crc32 value of ROM
  unsigned int db_crc32;                        // crc32 value of ROM in internal database

  int has_internal_crc;                         // ROM has internal checksum (SNES, Mega Drive, Game Boy)
  unsigned int current_internal_crc;            // calculated checksum

  unsigned int internal_crc;                    // internal checksum
  int internal_crc_start;                       // start of internal checksum in ROM header
  int internal_crc_len;                         // length (in bytes) of internal checksum in ROM header

  char internal_crc2[MAXBUFSIZE];               // 2nd or inverse internal checksum
  int internal_crc2_start;                      // start of 2nd/inverse internal checksum
  int internal_crc2_len;                        // length (in bytes) of 2nd/inverse internal checksum
} st_rominfo_t;

extern const struct option long_options[];

extern int ucon64_nfo (const st_rominfo_t *);
extern int ucon64_init (const char *romfile, st_rominfo_t *);
extern st_rominfo_t *ucon64_flush (st_rominfo_t *);
extern int ucon64_console_probe (st_rominfo_t *);

#endif // #ifndef UCON64_H
