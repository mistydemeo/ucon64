/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh


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

#ifdef  HAVE_CONFIG_H
#include "config.h"                             // ANSI_COLOR
#endif
#include "getopt.h"                             // for struct option
#include "ucon64_defines.h"
#include "ucon64_dat.h"
#include "misc.h"
#include "libdiscmage/libdiscmage.h"            // dm_image_t

/*
  this struct holds workflow relevant information
*/
typedef struct
{
  int argc;
  char **argv;

  const char *rom;                              // ROM (cmdline) with path
  char fname_arch[FILENAME_MAX];                // filename in archive (currently only for zip)
  int file_size;                                // (uncompressed) ROM file size
  unsigned int crc32;                           // crc32 value of ROM (used for DAT files)
  unsigned int fcrc32;                          // if non-zero: crc32 of ROM as it is on disk

#define UCON64_TYPE_ISROM(x) (x == UCON64_ROM)
#define UCON64_TYPE_ISDISC(x) (x == UCON64_DISC)
  int type;                                     // ROM type ROM or CD image

  /*
    if console == UCON64_UNKNOWN or st_rominfo_t == NULL ucon64_rom_nfo() won't
    be shown
  */
  int console;                                  // the detected console system

// gone for the sake of a "cleaner" getopt() use
//  const char *file;                             // FILE (cmdline) with path

  char configfile[FILENAME_MAX];                // path and name of the config file
  char configdir[FILENAME_MAX];                 // directory for config and DAT files
  char output_path[FILENAME_MAX];               // -o argument (default: cwd)
  char discmage_path[FILENAME_MAX];             // path to the discmage DLL

  unsigned int parport;                         // parallel port address
  int parport_mode;                             // parallel port mode: ECP, EPP, SPP, other

#ifdef  ANSI_COLOR
  int ansi_color;
#endif
  int backup;                                   // flag if backups files should be created
  int frontend;                                 // flag if uCON64 was started by a frontend
  int discmage_enabled;                         // flag if discmage DLL is loaded
  int dat_enabled;                              // flag if DAT file(s) are usable/enabled
  int good_enabled;                             // --good was used

  int show_nfo;                                 // show or skip info output for ROM

  // has higher priority than crc_big_files!
  int do_not_calc_crc;                          // disable checksum calc. to speed up --ls,--lsv, etc.

  // only used in switches.c for --crc (!)
  int crc_big_files;                            // enable checksum calc. for files bigger than MAXROMSIZE (512Mb)

#define UCON64_ISSET(x) (x != UCON64_UNKNOWN)
  /*
    These values override values in st_rominfo_t. Use UCON64_ISSET()
    to check them. When adding new ones don't forget to update ucon64_flush()
    too.
  */
  int buheader_len;                             // length of backup unit header 0 == no bu hdr
  int snes_hirom;                               // SNES ROM is HiROM
  int interleaved;                              // ROM is interleaved (swapped)

  // the following values are for the SNES and NES
  int part_size;                                // SNES split part size
  int split;                                    // ROM is split
  int bs_dump;                                  // SNES "ROM" is a Broadcast Satellaview dump
  int controller;                               // NES UNIF & SNES NSRT
  int controller2;                              // SNES NSRT
  int tv_standard;                              // NES UNIF
  int battery;                                  // NES UNIF/iNES/Pasofami
  int vram;                                     // NES UNIF
  int mirror;                                   // NES UNIF/iNES/Pasofami
  const char *mapr;                             // NES UNIF board name or iNES mapper number
  int use_dump_info;                            // NES UNIF
  const char *dump_info;                        // NES UNIF
  const char *comment;                          // NES UNIF
} st_ucon64_t;

extern st_ucon64_t ucon64;

/*
  This struct contains very specific informations only <console>_init() can
  supply after the correct console type was identified.

  file_size and current_crc32 are not console specific and were moved
  to st_ucon64_t.

  If all <console>_init()'s failed just ucon64_flush() this and ucon64_rom_nfo()
  won't be shown.
*/
typedef struct
{
  const char **console_usage;                   // console system usage
  const char **copier_usage;                    // backup unit usage

  int interleaved;                              // ROM is interleaved (swapped)
  int snes_hirom;                               // SNES ROM is HiROM

  int data_size;                                // ROM data size without "red tape"

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
  int has_internal_crc;                         // ROM has internal checksum (SNES, Mega Drive, Game Boy)
  unsigned int current_internal_crc;            // calculated checksum

  unsigned int internal_crc;                    // internal checksum
  int internal_crc_start;                       // start of internal checksum in ROM header
  int internal_crc_len;                         // length (in bytes) of internal checksum in ROM header

  char internal_crc2[MAXBUFSIZE];               // 2nd or inverse internal checksum
  int internal_crc2_start;                      // start of 2nd/inverse internal checksum
  int internal_crc2_len;                        // length (in bytes) of 2nd/inverse internal checksum
} st_rominfo_t;

extern dm_image_t *image;                       // DISC image (libdiscmage)
extern st_ucon64_dat_t *ucon64_dat;             // info from DAT

//extern const option_t options[];
extern const struct option options[];

extern int ucon64_nfo (const st_rominfo_t *);
extern int ucon64_init (const char *romfile, st_rominfo_t *);
extern st_rominfo_t *ucon64_flush (st_rominfo_t *);
extern int ucon64_console_probe (st_rominfo_t *);

#endif // #ifndef UCON64_H
