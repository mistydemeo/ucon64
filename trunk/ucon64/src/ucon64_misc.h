/*
ucon64_misc.h - miscellaneous functions for uCON64

Copyright (c) 1999 - 2006 NoisyB
Copyright (c) 2001 - 2004 dbjh
Copyright (c) 2001        Caz


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

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "ucon64.h"      // st_ucon64_nfo_t


/*
  usage of miscellaneous options
*/
extern const st_getopt2_t ucon64_options_usage[];
extern const st_getopt2_t ucon64_padding_usage[];


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
  UNTESTED,
  UNKNOWN_MSG
};

extern const char *ucon64_msg[];

/*
  ucon64_file_handler() handles backups (before modifying the ROM) and ROMs
                        inside archives. Read the comment at the header to
                        see how it and the flags work
  remove_temp_file()    remove possible temp file created by ucon64_file_handler()
  ucon64_output_fname()
  mkbak()

  ucon64_gauge()        wrapper
  ucon64_testpad()      test if ROM is padded
  ucon64_testsplit()    test if ROM is split
                          optionally a callback function can be used for specific
                          testing
  ucon64_rename()       DAT or internal header based rename
  ucon64_get_binary()   choose to load external or internal binaries

  bswap16_n()       bswap16() n bytes of buffer

  ucon64_dump()     wrapper
  ucon64_find()     wrapper
  ucon64_chksum()   wrapper
  ucon64_filefile() compare two files for similarities or differencies
*/
#define OF_FORCE_BASENAME 1
#define OF_FORCE_SUFFIX   2

extern int ucon64_file_handler (char *dest, char *src, int flags);
extern void remove_temp_file (void);
extern char *ucon64_output_fname (char *requested_fname, int flags);
typedef enum { BAK_DUPE, BAK_MOVE } backup_t;
extern char *mkbak (const char *filename, backup_t type);

extern void parport_print_info (void);

extern int ucon64_gauge (time_t init_time, int pos, int size);
extern int ucon64_testpad (const char *filename);
extern int ucon64_testsplit (const char *filename, int (*testsplit_cb) (const char *));
extern int ucon64_rename (int mode);
extern int ucon64_get_binary (const unsigned char **data, char *id);

#define ucon64_fgetc quick_fgetc
#define ucon64_fputc quick_fputc
#define ucon64_fread quick_fread
#define ucon64_fwrite quick_fwrite

extern int bswap16_n (void *buffer, int n);

extern int ucon64_dump (FILE *output, const char *filename, size_t start,
                        size_t len, uint32_t flags);
// Be sure the following constant doesn't conflict with the MEMCMP2_* constants
#define UCON64_FIND_QUIET (1 << 31)
extern int ucon64_find (const char *filename, size_t start, size_t len,
                        const char *search, int searchlen, uint32_t flags, int flag2);
extern int ucon64_chksum (char *sha1, char *md5, unsigned int *crc32, // uint16_t *crc16,
                          const char *filename, size_t start);
#define UCON64_FILEFILE_DIFF 0
#define UCON64_FILEFILE_SAME 1
extern void ucon64_filefile (const char *filename1, int start1,
                             const char *filename2, int start2, int mode);


#endif // #ifndef UCON64_MISC_H
