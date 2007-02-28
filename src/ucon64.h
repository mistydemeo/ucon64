/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

Copyright (c) 1999 - 2004 NoisyB
Copyright (c) 2001 - 2004 dbjh


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
#include "config.h"
#endif
#include "ucon64_defines.h"                     // MAXBUFSIZE, etc..
#include "ucon64_dat.h"


/*
  st_ucon64_nfo_t this struct contains very specific informations only
                    <console>_init() can supply after the correct console
                    type was identified.
  st_ucon64_t     this struct containes st_ucon64_nfo_t and unspecific
                    informations and some workflow stuff
*/
typedef struct
{
  const char *fname;                            // file name of ROM (w/ path)
  unsigned int crc32;                           // crc32 of ROM (w/o backup header, w/ intro)
  int interleaved;                              // ROM is interleaved (swapped)
  int data_size;                                // ROM data size without "red tape"

  const char *console_usage;                    // console system of the ROM
  const char *backup_usage;                     // backup unit of the ROM

  const void *backup_header;                    // (possible) header of backup unit
  int backup_header_start;                      // start of backup unit header (mostly 0)
  int backup_header_len;                        // length of backup unit header 0 == no bu hdr

  const void *header;                           // (possible) internal ROM header
  int header_start;                             // start of internal ROM header
  int header_len;                               // length of internal ROM header 0 == no hdr

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
} st_ucon64_nfo_t;


// parallel port modes
typedef enum
{
  UCON64_SPP,
  UCON64_EPP,
  UCON64_ECP
} parport_mode_t;


typedef struct
{
  int argc;
  char *const *argv;

  int console;                                  // the detected console system
  int option;                                   // current option
  const char *optarg;                           // pointer to current options' optarg

  const char *fname;                            // file name of ROM (or archive with ROM inside; w/ path)
  int file_size;
  unsigned int fcrc32;                          // crc32 of file

  const char *fname_last;                       // last file name specified on cmdline (hack for some options)
  const char *fname_optarg;                     // file name specified via optarg (default: same as fname_last)

  char *temp_file;                              // global temp_file
  char output_path[FILENAME_MAX];               // -o argument (default: cwd)

#ifdef  USE_ANSI_COLOR
  int ansi_color;
#endif
  int backup;                                   // flag if backups files should be created
  int frontend;                                 // flag if uCON64 was started by a frontend
  int dat_enabled;                              // flag if DAT file(s) are usable/enabled
  int quiet;                                    // quiet == -1 means verbose + 1
  int recursive;

  char configfile[FILENAME_MAX];                // path and name of the config file
  char configdir[FILENAME_MAX];                 // directory for config
  char datdir[FILENAME_MAX];                    // directory for DAT files

#if     defined USE_PPDEV || defined AMIGA
  char parport_dev[FILENAME_MAX];               // parallel port device (e.g.
#endif                                          //  /dev/parport0 or parallel.device)
  int parport_needed;
  int parport;                                  // parallel port address
  parport_mode_t parport_mode;                  // parallel port mode: ECP, EPP, SPP

#ifdef  USE_USB
  int usbport;                                  // non-zero => use usbport, 1 = USB0, 2 = USB1
  char usbport_dev[FILENAME_MAX];               // usb port device (e.g. /dev/usb/hiddev0)
#endif

#if 1
  // TODO: move this to st_ucon64_nfo_t and use (st_ucon64_nfo_t *) force instead
  unsigned int crc32;                           // crc32 of ROM (used for DAT files)
  int do_not_calc_crc;                          // disable checksum calc. to speed up --ls,--lsv, etc.
  int id;                                       // generate unique name (currently
                                                //  only used by snes_gd3())
  // the following values are for SNES, NES, Genesis and Nintendo 64
  int battery;                                  // NES UNIF/iNES/Pasofami
  int bs_dump;                                  // SNES "ROM" is a Broadcast Satellaview dump
  const char *comment;                          // NES UNIF
  int controller;                               // NES UNIF & SNES NSRT
  int controller2;                              // SNES NSRT
  const char *dump_info;                        // NES UNIF
  int io_mode;                                  // SNES SWC, Nintendo 64 CD64 & Cyan's Megadrive copier
  const char *mapr;                             // NES UNIF board name or iNES mapper number
  int mirror;                                   // NES UNIF/iNES/Pasofami
  int part_size;                                // SNES/Genesis split part size
  int region;                                   // Genesis (for -multi)
  int snes_header_base;                         // SNES ROM is "Extended" (or Sufami Turbo)
  int snes_hirom;                               // SNES ROM is HiROM
  int split;                                    // ROM is split
  int tv_standard;                              // NES UNIF
  int use_dump_info;                            // NES UNIF
  int vram;                                     // NES UNIF

  int backup_header_len;                        // length of backup unit header 0 == no bu hdr
  int interleaved;                              // ROM is interleaved (swapped)
#else
  st_ucon64_nfo_t *force;                       // this overrides the default values for nfo
#endif
  st_ucon64_nfo_t *nfo;                         // info from ucon64_console_open()

  st_ucon64_dat_t *dat;                         // info from DATabase ucon64_dat_open()
} st_ucon64_t;


/*
  ucon64_init()         init st_ucon64_nfo_t, st_ucon64_dat_t and dm_image_t
  ucon64_nfo()          display contents of st_ucon64_nfo_t, st_ucon64_dat_t and
                          dm_image_t
  ucon64_usage()        print usage
  ucon64                global (st_ucon64_t *)
*/
extern int ucon64_init (void);
extern int ucon64_nfo (void);


enum {
  USAGE_VIEW_SHORT = 0,
  USAGE_VIEW_LONG,
  USAGE_VIEW_PAD,
  USAGE_VIEW_DAT,
  USAGE_VIEW_PATCH,
  USAGE_VIEW_BACKUP
};
extern void ucon64_usage (int argc, char *argv[], int view);

extern st_ucon64_t ucon64;


#endif // UCON64_H
