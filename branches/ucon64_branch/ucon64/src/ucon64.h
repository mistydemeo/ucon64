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
#include "misc.h"

typedef struct
{
  const char *option_s;                         // "chk", ...
  const char *optarg;
  const char *desc;                             // "fix checksum", ...
//  const char *desc_long;                        // long description
//  int status;                                   // development status of option
                                                  // 0 = OK, 1 = TODO, 2 = TEST
} st_usage_t;


#ifdef  DISCMAGE
#include "ucon64_dm.h"
#endif
#ifdef  GUI
#include "ucon64_ng.h"
#endif

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
  const st_usage_t *console_usage;              // console system usage
  const st_usage_t *copier_usage;               // backup unit usage

  int interleaved;                              // ROM is interleaved (swapped)
  int snes_header_base;                         // SNES ROM is "Extended" (or Sufami Turbo)
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


#include "ucon64_dat.h"


typedef enum { UCON64_SPP, UCON64_EPP, UCON64_ECP } parport_mode_t;


/*
  this struct holds workflow relevant information
*/
typedef struct
{
  int argc;
  char **argv;

// TODO?: rename to fname
  const char *rom;                              // ROM (cmdline) with path

  int fname_len;                                // force fname output format for --rrom, --rename, etc...
        // fname_len can be UCON64_83 (8.3) or UCON64_FORCE63 (63.x)
  char fname_arch[FILENAME_MAX];                // filename in archive (currently only for zip)
  int file_size;                                // (uncompressed) ROM file size
  unsigned int crc32;                           // crc32 value of ROM (used for DAT files)
  unsigned int fcrc32;                          // if non-zero: crc32 of ROM as it is on disk

  /*
    if console == UCON64_UNKNOWN or st_rominfo_t == NULL ucon64_rom_nfo() won't
    be shown
  */
  int console;                                  // the detected console system

  const char *file;                             // FILE (cmdline) with path

  char configfile[FILENAME_MAX];                // path and name of the config file
  char configdir[FILENAME_MAX];                 // directory for config
  char datdir[FILENAME_MAX];                    // directory for DAT files
#ifdef  GUI
  char skindir[FILENAME_MAX];                 // path to the pictures used with the netgui DLL
#endif
  char output_path[FILENAME_MAX];               // -o argument (default: cwd)
#ifdef  DISCMAGE
  char discmage_path[FILENAME_MAX];             // path to the discmage DLL
#endif
#ifdef  GUI
  char netgui_path[FILENAME_MAX];               // path to the netgui DLL
#endif
  unsigned int parport;                         // parallel port address
  parport_mode_t parport_mode;                  // parallel port mode: ECP, EPP, SPP
  
#ifdef  ANSI_COLOR
  int ansi_color;
#endif
  int backup;                                   // flag if backups files should be created
  int frontend;                                 // flag if uCON64 was started by a frontend
#ifdef  DISCMAGE
  int discmage_enabled;                         // flag if discmage DLL is loaded
#endif
#ifdef  GUI
  int netgui_enabled;                           // flag if netgui DLL is loaded
#endif
  int dat_enabled;                              // flag if DAT file(s) are usable/enabled
  int quiet;                                    // quiet == -1 means verbose + 1

  int force_disc;                               // --disc was used
  uint32_t flags;                                 // detect and init ROM info

  // has higher priority than crc_big_files!
  int do_not_calc_crc;                          // disable checksum calc. to speed up --ls,--lsv, etc.

  // only used in switches.c for --crc (!)
  int crc_big_files;                            // enable checksum calc. for files bigger than MAXROMSIZE (512Mb)

#define UCON64_ISSET(x) (x != UCON64_UNKNOWN)
  /*
    These values override values in st_rominfo_t. Use UCON64_ISSET()
    to check them. When adding new ones don't forget to update ucon64_execute_options()
    too.
  */
  int buheader_len;                             // length of backup unit header 0 == no bu hdr
  int interleaved;                              // ROM is interleaved (swapped)
  int id;                                       // generate unique name (currently
                                                  //  only used by snes_gd3())
  // the following values are for the SNES, NES and the Genesis
  int snes_header_base;                         // SNES ROM is "Extended" (or Sufami Turbo)
  int snes_hirom;                               // SNES ROM is HiROM

  // the following values are for the SNES, NES and the Genesis
  int part_size;                                // SNES split part size
  int split;                                    // ROM is split
  int bs_dump;                                  // SNES "ROM" is a Broadcast Satellaview dump
  int controller;                               // NES UNIF & SNES NSRT
  int controller2;                              // SNES NSRT
  int tv_standard;                              // NES UNIF/Genesis
  int battery;                                  // NES UNIF/iNES/Pasofami
  int vram;                                     // NES UNIF
  int mirror;                                   // NES UNIF/iNES/Pasofami
  const char *mapr;                             // NES UNIF board name or iNES mapper number
  int use_dump_info;                            // NES UNIF
  const char *dump_info;                        // NES UNIF
  const char *comment;                          // NES UNIF

#ifdef  GUI
  netgui_t *netgui;                             // pointer to netgui GUI
#endif
#ifdef  DISCMAGE
  dm_image_t *image;                            // info from libdiscmage
#endif
  st_ucon64_dat_t *dat;                         // info from DATabase
  st_rominfo_t *rominfo;                        // info from <console>_init()
} st_ucon64_t;

/*
  ucon64_init()      init st_rominfo_t, st_ucon64_dat_t and st_dm_image_t
  ucon64_nfo()       display contents of st_rominfo_t, st_ucon64_dat_t and
                       st_dm_image_t
  ucon64_flush()     flush contents of st_rominfo_t, st_ucon64_dat_t and
                       st_dm_image_t
  ucon64_flush_rom() flush only st_rominfo_t
*/
extern int ucon64_init (void);
extern int ucon64_nfo (void);
//extern void ucon64_flush (void);
extern st_rominfo_t *ucon64_flush_rom (st_rominfo_t *);

extern void ucon64_usage (int argc, char *argv[]);
#ifdef  HAVE_ZLIB_H
extern void ucon64_fname_arch (const char *fname);
#endif

extern st_ucon64_t ucon64;
extern int ucon64_parport_needed;
extern const struct option options[];

#endif // #ifndef UCON64_H
