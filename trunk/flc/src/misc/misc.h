/*
misc.h - miscellaneous functions

Copyright (c) 1999 - 2005 NoisyB
Copyright (c) 2001 - 2005 dbjh
Copyright (c) 2002 - 2004 Jan-Erik Karlsson (Amiga code)


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
#ifndef MISC_H
#define MISC_H
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ANSI_COLOR support
#endif
#ifdef  __cplusplus
extern "C" {
#endif
#include <string.h>
#include <time.h>                               // bytes_per_second() requires time()
#include <stdio.h>
#include "defines.h"


/*
  dumper()        dump n bytes of buffer
                    you can use here a virtual_start for the displayed counter
                    DUMPER_HEX
                    dump in hex (base: 16) (default)
                    DUMPER_DUAL
                    dump in dual (base: 2)
                    DUMPER_CODE
                    dump as C code
                    DUMPER_PRINT
                    printf() buffer after chars passed isprint() and isspace()
                    other chars will be printed as dots '.'
                    do NOT use DUMPER_PRINT in uCON64 code - dbjh
                    DUMPER_HEX_COUNT
                    show position as hex value (default)
                    DUMPER_DEC_COUNT
                    show position as decimal value
                    DUMPER_HEX_UPPER
                    show hex values in uppercase (default: lowercase)
  change_mem{2}() see header of implementation for usage
  build_cm_patterns() helper function for change_mem2() to read search patterns
                    from a file
  cleanup_cm_patterns() helper function for build_cm_patterns() to free all
                    memory allocated for a (list of) st_pattern_t structure(s)
  bytes_per_second() returns bytes per second (useful in combination with
                    gauge())
  misc_percent()  returns percentage of progress (useful in combination with
                    gauge())
  drop_privileges() switch to the real user and group id (leave "root mode")
  register_func() atexit() replacement
                    returns -1 if it fails, 0 if it was successful
  unregister_func() unregisters a previously registered function
                    returns -1 if it fails, 0 if it was successful
  handle_registered_funcs() calls all the registered functions
  wait2()         wait (sleep) a specified number of milliseconds
  getenv2()       getenv() clone for enviroments w/o HOME, TMP or TEMP variables
*/
#define DUMPER_HEX       (0)
#define DUMPER_HEX_COUNT (0)
#define DUMPER_DUAL      (1)
#define DUMPER_CODE      (1 << 1)
#define DUMPER_PRINT     (1 << 2)
#define DUMPER_DEC       (1 << 3)
#define DUMPER_DEC_COUNT (1 << 4)
#define DUMPER_DEFAULT   (DUMPER_HEX_COUNT|DUMPER_HEX)
//#define DUMPER_HEX_UPPER (1 << 5)
extern void dumper (FILE *output, const void *buffer, size_t bufferlen,
                    int virtual_start, unsigned int flags);

typedef struct st_cm_set
{
  char *data;
  int size;
} st_cm_set_t;

typedef struct st_cm_pattern
{
  char *search, wildcard, escape, *replace;
  int search_size, replace_size, offset, n_sets;
  st_cm_set_t *sets;
} st_cm_pattern_t;

extern int change_mem (char *buf, int bufsize, char *searchstr, int strsize,
                       char wc, char esc, char *newstr, int newsize, int offset, ...);
extern int change_mem2 (char *buf, int bufsize, char *searchstr, int strsize,
                        char wc, char esc, char *newstr, int newsize,
                        int offset, st_cm_set_t *sets);
extern int build_cm_patterns (st_cm_pattern_t **patterns, const char *filename, int verbose);
extern void cleanup_cm_patterns (st_cm_pattern_t **patterns, int n_patterns);

extern int bytes_per_second (time_t start_time, int nbytes);
extern int misc_percent (unsigned long pos, unsigned long len);
#if     defined __unix__ && !defined __MSDOS__
extern int drop_privileges (void);
#endif
extern int register_func (void (*func) (void));
extern int unregister_func (void (*func) (void));
extern void handle_registered_funcs (void);
extern void wait2 (int nmillis);
extern char *getenv2 (const char *variable);
extern int misc_digits (unsigned long value);


/*
  Portability and Fixes

  fix_character_set()  fixes some Cygwin problems with filenames
  truncate()
  popen()
  pclose()
*/
#ifdef  __CYGWIN__
extern char *fix_character_set (char *str);
#endif

#ifdef  _WIN32
// Note that _WIN32 is defined by cl.exe while the other constants (like WIN32)
//  are defined in header files. MinGW's gcc.exe defines all constants.
#include <sys/types.h>

extern int truncate (const char *path, off_t size);

// For MinGW popen() and pclose() are unavailable for DLL's. For DLL's _popen()
//  and _pclose() should be used. Visual C++ only has the latter two.
#ifndef pclose                                  // archive.h's definition gets higher "precedence"
#define pclose  _pclose
#endif
#ifndef popen                                   // idem
#define popen   _popen
#endif
#ifndef __MINGW32__
#include <io.h>
#include <direct.h>
#include <sys/stat.h>                           // According to MSDN <sys/stat.h> must

#define S_IWUSR _S_IWRITE
#define S_IRUSR _S_IREAD
#define S_ISDIR(mode) ((mode) & _S_IFDIR ? 1 : 0)
#define S_ISREG(mode) ((mode) & _S_IFREG ? 1 : 0)

#define F_OK 00
#define W_OK 02
#define R_OK 04
#define X_OK R_OK                               // this is correct for dirs, but not for exes

#define STDIN_FILENO (fileno (stdin))
#define STDOUT_FILENO (fileno (stdout))
#define STDERR_FILENO (fileno (stderr))

#else
#ifdef  DLL
#define access  _access
#define chmod   _chmod
#define fileno  _fileno
#define getcwd  _getcwd
#define isatty  _isatty
#define rmdir   _rmdir
#define stat    _stat
#define strdup  _strdup
#define stricmp _stricmp
#define strnicmp _strnicmp
#endif // DLL

#endif // !__MINGW32__
#define snprintf _snprintf

#endif // _WIN32

#ifdef  AMIGA
// The compiler used by Jan-Erik doesn't have snprintf(). - dbjh
#include "snprintf.h"

// custom _popen() and _pclose(), because the standard ones (named popen() and
//  pclose()) are buggy
#ifndef pclose                                  // archive.h's definition gets higher "precedence"
#define pclose  _pclose
#endif
#ifndef popen                                   // idem
#define popen   _popen
#endif
#endif

#ifdef  AMIGA
extern FILE *_popen (const char *path, const char *mode);
extern int _pclose (FILE *stream);
#endif


#ifdef  __cplusplus
}
#endif
#endif // MISC_H
