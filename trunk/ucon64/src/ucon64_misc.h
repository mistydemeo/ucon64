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

#ifdef  PARALLEL
#define out1byte(p,x)   outportb(p,x)
#define in1byte(p)      inportb(p)

// DJGPP (DOS) has these, but it's better that all code uses the same functions
extern unsigned char inportb (unsigned short port);
extern unsigned short inportw (unsigned short port);
extern void outportb (unsigned short port, unsigned char byte);
extern void outportw (unsigned short port, unsigned short word);
#endif // PARALLEL

#define MBIT 131072

#define PARPORT_DATA    0                       // output
#define PARPORT_STATUS  1                       // input
#define PARPORT_CONTROL 2

/*
  defines for unknown backup units/emulators
*/
typedef struct //st_unknown_header
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
extern const char *unknown_usage[];
extern const char *atari_usage[];
extern const char *cd32_usage[];
extern const char *cdi_usage[];
extern const char *channelf_usage[];
extern const char *coleco_usage[];
extern const char *gamecom_usage[];
extern const char *gc_usage[];
extern const char *gp32_usage[];
extern const char *intelli_usage[];
extern const char *odyssey2_usage[];
extern const char *odyssey_usage[];
extern const char *ps2_usage[];
extern const char *real3do_usage[];
extern const char *s16_usage[];
extern const char *sat_usage[];
extern const char *vboy_usage[];
extern const char *vc4000_usage[];
extern const char *vectrex_usage[];
extern const char *xbox_usage[];
extern const char *mame_usage[];

extern char *ucon64_temp_file;
extern int (*ucon64_testsplit_callback) (const char *filename);
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
  DAT_ERROR,
  READ_CONFIG_FILE
};

extern const char *ucon64_msg[];


/*
  wrapper for misc.c/q_fbackup()

  Read the comment at the header of handle_existing_file() to see how it works!
*/
extern void handle_existing_file (const char *dest, char *src);
#define ucon64_fbackup(a,b) (handle_existing_file(b,a))
extern void remove_temp_file (void); // possible temp file created by handle_existing_file()

extern char *ucon64_output_fname (char *requested_fname, int force_requested_fname);

extern int ucon64_fhexdump (const char *filename, int start, int len);

extern unsigned int ucon64_filefile (const char *filename1, int start1, const char *filename2, int start2, int similar);

/*
  wrapper for misc.c/gauge()
*/
extern int ucon64_gauge (time_t init_time, int pos, int size);
/*
  libdiscmage can use a gauge function

  unlike ucon64_gauge() the timer will reset if pos == 0
*/
extern int ucon64_dm_gauge (int pos, int size);

extern int ucon64_pad (const char *filename, int start, int size); // pad ROM to a certain size
extern int ucon64_testpad (const char *filename, st_rominfo_t *rominfo); // test if ROM is padded

extern int ucon64_testsplit (const char *filename); // test if ROM is split

extern unsigned int ucon64_parport_init (unsigned int parport);


extern int ucon64_e (const char *rominfo);
extern int ucon64_ls (const char *path, int mode);
extern int ucon64_configfile (void);

#endif // #ifndef UCON64_MISC_H
