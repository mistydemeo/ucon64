/*
ucon64_misc.h - miscellaneous functions for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh
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

#include "ucon64.h"                             // st_rominfo_t
#include "ucon64_defines.h"                     // MBIT

#ifdef  PARALLEL
// DJGPP (DOS) has these, but it's better that all code uses the same functions.
extern unsigned char inportb (unsigned short port);
extern unsigned short inportw (unsigned short port);
extern void outportb (unsigned short port, unsigned char byte);
extern void outportw (unsigned short port, unsigned short word);
#endif // PARALLEL

#define PARPORT_DATA    0                       // output
#define PARPORT_STATUS  1                       // input
#define PARPORT_CONTROL 2

/*
  defines for unknown backup units/emulators
*/
typedef struct // st_unknown_header
{
/*
  Don't create fields that are larger than one byte! For example size_low and size_high
  could be combined in one unsigned short int. However, this gives problems with little
  endian vs. big endian machines (e.g. writing the header to disk).
*/
  unsigned char size_low;
  unsigned char size_high;
  unsigned char emulation;
  unsigned char hirom;
  unsigned char emulation1;
  unsigned char emulation2;
  unsigned char pad[2];
  unsigned char id1;
  unsigned char id2;
  unsigned char type;
  unsigned char pad2[501];
} st_unknown_header_t;

#define UNKNOWN_HEADER_START 0
#define UNKNOWN_HEADER_LEN (sizeof (st_unknown_header_t))

/*
  usage for consoles not directly supported by uCON64
*/
extern const st_usage_t unknown_usage[];
extern const st_usage_t atari_usage[];
extern const st_usage_t cd32_usage[];
extern const st_usage_t cdi_usage[];
extern const st_usage_t channelf_usage[];
extern const st_usage_t coleco_usage[];
extern const st_usage_t gamecom_usage[];
extern const st_usage_t gc_usage[];
extern const st_usage_t gp32_usage[];
extern const st_usage_t intelli_usage[];
extern const st_usage_t odyssey2_usage[];
extern const st_usage_t odyssey_usage[];
extern const st_usage_t ps2_usage[];
extern const st_usage_t real3do_usage[];
extern const st_usage_t s16_usage[];
extern const st_usage_t sat_usage[];
extern const st_usage_t vboy_usage[];
extern const st_usage_t vc4000_usage[];
extern const st_usage_t vectrex_usage[];
extern const st_usage_t xbox_usage[];
extern const st_usage_t mame_usage[];

extern int unknown_init (st_rominfo_t *rominfo);


extern const st_usage_t ucon64_options_usage[];
extern const st_usage_t ucon64_padding_usage[];
extern const st_usage_t ucon64_patching_usage[];


/*
  uCON64 workflow flags for st_ucon64_wf_t

  WF_PROBE          probe for console type
  WF_INIT           init ROM info (ucon64_init()) necessary
                      w/o this flag WF_NFO and WF_NFO_AFTER
                      will be ignored
  WF_NFO            show info output before processing rom
  WF_NFO_AFTER      show info output AFTER processing rom
  WF_NO_ROM         for this option no ROM is required
  WF_NOCRC32        no CRC32 calculation necessary for this option; this
                      overrides even WF_INIT, WF_NFO and WF_NFO_AFTER
  WF_STOP           a "stop" option:
                    - -multi (and -xfalmulti) takes more than one file as
                      argument, but should be executed only once.
                    - stop after sending one ROM to a copier ("multizip")
                    - stop after applying a patch so that the patch file won't
                      be interpreted as ROM
  WF_PAR            this option requires a parallel port
  WF_USB            this option requires a USB port
  WF_SERIAL         this option requires a serial port
  WF_NO_SPLIT       this option does not work with split ROMs
  WF_DEFAULT        same as WF_INIT|WF_PROBE|WF_NFO

  example:
  WF_NFO|WF_MFO_AFTER
                    a ROM is required and info will be shown before and
                    after it has been processed
*/
#define WF_DEFAULT (WF_PROBE|WF_INIT|WF_NFO)
#define WF_PROBE 1
#define WF_INIT 2
#define WF_NFO 4
#define WF_STOP 8
#define WF_NFO_AFTER 16
#define WF_NO_ROM 32
#define WF_PAR 64
#define WF_USB 128
#define WF_SERIAL 256
#define WF_NOCRC32 512
#define WF_NO_SPLIT 1024
#define WF_SWITCH 2048
typedef struct
{
  int option;
  int console;                                // UCON64_SNES, ...
  const st_usage_t *usage;
  uint32_t flags;                             // flags for workflow, etc..
} st_ucon64_wf_t;

extern const st_ucon64_wf_t *ucon64_get_wf (const int option); // get workflow struct for option
extern const st_ucon64_wf_t ucon64_wf[];

extern char *ucon64_temp_file;
extern int (*ucon64_testsplit_callback) (const char *filename);

#define NINTENDO_MAKER_LEN 684
extern const char *nintendo_maker[];

/*
  uCON64 messages

  usage example: fprintf (stdout, ucon64_msg[WROTE], filename);
*/
enum
{
  PARPORT_ERROR = 0,
  CONSOLE_ERROR,
  WROTE,
  OPEN_READ_ERROR,
  OPEN_WRITE_ERROR,
  READ_ERROR,
  WRITE_ERROR,
  BUFFER_ERROR,                                 // not enough memory
  ROM_BUFFER_ERROR,
  FILE_BUFFER_ERROR,
  DAT_NOT_FOUND,
  DAT_NOT_ENABLED,
  READ_CONFIG_FILE,
  NO_LIB
};

extern const char *ucon64_msg[];


/*
  ucon64_file_handler() handles backups (before modifying the ROM) and ROMs
                        inside archives. Read the comment at the header to
                        see how it and the flags work
  remove_temp_file()    remove possible temp file created by ucon64_file_handler()
*/
#define OF_FORCE_BASENAME 1
#define OF_FORCE_SUFFIX   2
extern int ucon64_file_handler (char *dest, char *src, int flags);
extern void remove_temp_file (void);
extern char *ucon64_output_fname (char *requested_fname, int flags);

extern int ucon64_fhexdump (const char *filename, int start, int len);
extern int ucon64_filefile (const char *filename1, int start1, const char *filename2, int start2, int similar);

//  wrapper for misc.c/gauge()
extern int ucon64_gauge (time_t init_time, int pos, int size);
extern int ucon64_testpad (const char *filename); // test if ROM is padded

extern int ucon64_testsplit (const char *filename); // test if ROM is split

extern unsigned int ucon64_parport_init (unsigned int parport);

extern int ucon64_configfile (void);

/*
  ucon64_get_options_s() returns option name
                         ucon64_get_option_s(UCON64_HELP) == "help"
  ucon64_get_desc()      returns option description
                         ucon64_get_desc(UCON64_HELP) == "to get more information"
*/
extern const char *ucon64_get_option_s (int option);
extern const char *ucon64_get_desc (int option);

#endif // #ifndef UCON64_MISC_H
