/*
misc.h - miscellaneous functions

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh
                  2002 Jan-Erik Karlsson (Amiga)


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
#include "config.h"                             // HAVE_ZLIB_H, ANSI_COLOR support
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include <string.h>
#include <limits.h>
#include <time.h>                               // gauge() prototype contains time_t
#include <stdio.h>
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef  HAVE_ZLIB_H
#include "miscz.h"
#endif                                          // HAVE_ZLIB_H

#ifdef __sun
#ifdef __SVR4
#define __solaris__
#endif
#endif

// WIN32 stands for __WIN32, _WIN32 and WIN32
#if     defined __WIN32 || defined _WIN32 || defined WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

#if     defined __linux__ || defined __FreeBSD__ || \
        defined __BEOS__ || defined __solaris__ || defined HAVE_INTTYPES_H
#include <inttypes.h>
#elif   defined __CYGWIN__
#include <sys/types.h>
#ifndef OWN_INTTYPES
#define OWN_INTTYPES                            // signal that these are defined
typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
typedef u_int64_t uint64_t;
#endif // OWN_INTTYPES
#else
#ifndef OWN_INTTYPES
#define OWN_INTTYPES                            // signal that these are defined
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
#ifndef WIN32
typedef unsigned long long int uint64_t;
#endif
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
#ifndef WIN32
typedef signed long long int int64_t;
#endif
#endif // OWN_INTTYPES
#endif

#if     (!defined TRUE || !defined FALSE)
#define FALSE 0
#define TRUE (!FALSE)
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define LIB_VERSION(ver, rel, seq) (((ver) << 16) | ((rel) << 8) | (seq))
#define NULL_TO_EMPTY(str) ((str) ? (str) : (""))
//#define RANDOM(min, max) ((rand () % (max - min)) + min)
#define OFFSET(a, offset) ((((unsigned char *)&(a))+(offset))[0])

#ifdef WORDS_BIGENDIAN
#undef WORDS_BIGENDIAN
#endif

#if     defined _LIBC || defined __GLIBC__
  #include <endian.h>
  #if __BYTE_ORDER == __BIG_ENDIAN
    #define WORDS_BIGENDIAN 1
  #endif
#elif   defined AMIGA || defined __sparc__ || defined __BIG_ENDIAN__ || defined __APPLE__
  #define WORDS_BIGENDIAN 1
#endif

#ifdef  __MSDOS__                               // __MSDOS__ must come before __unix__,
  #define CURRENT_OS_S "MSDOS"                  //  because DJGPP defines both
#elif   defined __unix__
  #ifdef  __CYGWIN__
    #define CURRENT_OS_S "Win32"
  #elif   defined __FreeBSD__
    #define CURRENT_OS_S "Unix (FreeBSD)"
  #elif   defined __linux__
    #define CURRENT_OS_S "Unix (Linux)"
  #elif   defined __solaris__
    #ifdef __sparc__
      #define CURRENT_OS_S "Unix (Solaris/Sparc)"
    #else
      #define CURRENT_OS_S "Unix (Solaris/i386)"
    #endif
  #else
    #define CURRENT_OS_S "Unix"
  #endif
#elif   defined WIN32
    #define CURRENT_OS_S "Win32"
#elif   defined __APPLE__
  #if   defined __POWERPC__ || defined __ppc__
    #define CURRENT_OS_S "Apple (ppc)"
  #else
    #define CURRENT_OS_S "Apple"
  #endif
#elif   defined __BEOS__
  #define CURRENT_OS_S "BeOS"
#elif   defined AMIGA
  #if defined __PPC__
    #define CURRENT_OS_S "Amiga (ppc)"
  #else
    #define CURRENT_OS_S "Amiga (68k)"
  #endif
#else
  #define CURRENT_OS_S "?"
#endif

#ifdef WORDS_BIGENDIAN
#define me2be_16(x) (x)
#define me2be_32(x) (x)
#define me2be_64(x) (x)
#define me2le_16(x) (bswap_16(x))
#define me2le_32(x) (bswap_32(x))
#define me2le_64(x) (bswap_64(x))
#else
#define me2be_16(x) (bswap_16(x))
#define me2be_32(x) (bswap_32(x))
#define me2be_64(x) (bswap_64(x))
#define me2le_16(x) (x)
#define me2le_32(x) (x)
#define me2le_64(x) (x)
#endif

#if     ((defined __unix__ || defined __BEOS__) && !defined __MSDOS__)
// Cygwin, GNU/Linux, Solaris, FreeBSD, BeOS
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_S "/"
#else // DJGPP, Win32
#define FILE_SEPARATOR '\\'
#define FILE_SEPARATOR_S "\\"
#endif

#define OPTION '-'
#define OPTION_S "-"
#define OPTION_LONG_S "--"
#define OPTARG '='
#define OPTARG_S "="

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE

#ifndef ARGS_MAX
#define ARGS_MAX 128
#endif // ARGS_MAX

#if (defined __unix__ || defined __BEOS__ || defined AMIGA)
#ifndef __MSDOS__
#define getch           getchar                 // getchar() acts like DOS getch() after init_conio()
extern int kbhit (void);                        // may only be used after init_conio()!
#else
#include <conio.h>                              // getch()
#include <pc.h>                                 // kbhit()
#endif
// Should we leave these prototypes visible to the DOS code? The advantage
//  would be a few less #ifdef's
extern void init_conio (void);
extern void deinit_conio (void);
#endif

#ifdef  __CYGWIN__
char *cygwin_fix (char *value);
#endif


/*
  String manipulation

  isfname()   test if char could be used for filenames
  isprint2()  test if char could be used for stdout
  tofname()   replaces chars that can not be used for filenames
  toprint2()  replaces chars that should not be used for stdout

  is_func()   use all is*() functions on an array of unsigned char
  to_func()   use all to*() functions on an array of unsigned char

  strtrim()   trim isspace()'s from start and end of string

  get_suffix()    get suffix of filename
  set_suffix()    set/replace suffix of filename with suffix
                  suffix means in this case the suffix INCLUDING the dot '.'

  basename()  GNU basename() clone
  realpath2() realpath() clone
  mkdir2()    mkdir() wrapper who automatically cares for rights, etc.
  truncate2() don't use truncate() to enlarge files, because the result is 
              undefined (by POSIX) use truncate2() instead which does both
  strargv()   wapper for argz_* to convert a cmdline into an argv[]
              like array
*/
extern int isfname (int c);
extern int isprint2 (int c);
extern int tofname (int c);
extern int toprint2 (int c);
extern int is_func (unsigned char *s, int size, int (*func) (int));
extern char *to_func (unsigned char *s, int size, int (*func) (int));
#define strupr(s) (to_func(s, strlen(s), toupper))
#define strlwr(s) (to_func(s, strlen(s), tolower))
//#ifndef HAVE_STRCASESTR
// strcasestr is GNU only
extern char *strcasestr2 (const char *str, const char *search);
#define stristr strcasestr2
//#else
//#define stristr strcasestr
//#endif
#define stricmp strcasecmp
#define strnicmp strncasecmp
extern char *strtrim (char *str);
extern const char *get_suffix (const char *filename);
extern char *set_suffix (char *filename, const char *suffix);
//#ifndef HAVE_BASENAME
extern char *basename2 (const char *path);
//  the following define will override a possible XPG basename() which mods. the src
#define basename basename2
//#endif
//#ifndef HAVE_DIRNAME
extern char *dirname2 (const char *path);
#define dirname dirname2
//#endif
extern char *realpath2 (const char *src, char *full_path);
extern int mkdir2 (const char *name);
extern int truncate2 (const char *filename, int size);
extern char ***strargv (int *argc, char ***argv, char *cmdline, int separator_char);


#ifdef  WIN32
/*
  VC++ support (MinGW lite; mainly unistd.h and dirent.h)
  
  access()  see libc documentation
*/
#ifndef R_OK
#define R_OK
#define W_OK
#define F_OK
#define X_OK
#endif
typedef struct
{
  char pad;
} DIR;


struct dirent
{
  char d_name[FILENAME_MAX + 1];
};


extern DIR *opendir (const char *path);
extern struct dirent *readdir (DIR *p);
void rewinddir (DIR *p);
extern int closedir (DIR *p);
extern int access (const char *fname, void *mode);
extern void sync (void);
extern char *getcwd (char *p, size_t p_size);
extern int chdir (const char *path);
extern int mkdir (const char *path, void *mode);
extern int rmdir (const char *path);
//extern int isatty
#endif  // WIN32


/*
  mem functions

  memwcmp()    memcmp with wildcard support
  mem_swap()   swap n Bytes from add on
  mem_hexdump() hexdump n Bytes from add on; you can use here a virtual_start for the displayed counter
  crc16()      calculate the crc16 of buffer for size bytes
  crc32()      calculate the crc32 of buffer for size bytes
*/
extern int memwcmp (const void *add, const void *add_with_wildcards, uint32_t n, int wildcard);
extern void mem_hexdump (const void *add, uint32_t n, int virtual_start);
extern void *mem_swap (void *add, uint32_t size);
#ifdef  HAVE_BYTESWAP_H
#include <byteswap.h>
#else
#ifndef OWN_BYTESWAP
#define OWN_BYTESWAP                            // signal that these are defined
extern uint16_t bswap_16 (uint16_t x);
extern uint32_t bswap_32 (uint32_t x);
extern uint64_t bswap_64 (uint64_t x);
#endif // OWN_BYTESWAP
#endif
//extern unsigned short crc16 (unsigned short crc16, const void *buffer, unsigned int size);
#ifndef  HAVE_ZLIB_H
// use zlib's crc32() if HAVE_ZLIB_H is defined
extern unsigned int crc32 (unsigned int crc32, const void *buffer, unsigned int size);
#endif


/*
  Misc stuff

  change_mem()    see header of implementation for usage
  ansi_init ()    initialize ANSI output
  ansi_strip ()   strip ANSI codes from a string
  gauge()         init_time == time when gauge() was first started or when
                  the transfer did start
                  pos == current position
                  size == full size
                  gauge given these three values will calculate many
                  informative things like time, status bar, cps, etc.
                  it can be used for procedures which take some time to
                  inform the user about the actual progress
  getenv2()       getenv() clone for enviroments w/o HOME, TMP or TEMP variables
  rmdir2()        like rmdir() but removes non-empty directories recursively
  tmpnam2()       replacement for tmpnam() temp must have the size of FILENAME_MAX
  renlwr()        renames all files tolower()
  drop_privileges() switch to the real user and group id (leave "root mode")
  register_func() atexit() replacement
                  returns -1 if it fails, 0 if it was successful
  unregister_func() unregisters a previously registered function
                  returns -1 if it fails, 0 if it was successful
  handle_registered_funcs() calls all the registered functions
  wait2           wait (sleep) a specified number of milliseconds
*/
extern int change_mem (char *buf, int bufsize, char *searchstr, int strsize,
                       char wc, char esc, char *newstr, int newsize, int offset, ...);
extern int ansi_init (void);
extern char *ansi_strip (char *str);
extern int gauge (time_t init_time, int pos, int size);
extern char *getenv2 (const char *variable);
extern char *tmpnam2 (char *temp);
extern int rmdir2 (const char *path);
//extern int renlwr (const char *path);
#if     defined __unix__ && !defined __MSDOS__
extern int drop_privileges (void);
#endif
extern int register_func (void (*func) (void));
extern int unregister_func (void (*func) (void));
extern void handle_registered_funcs (void);
extern void wait2 (int nmillis);


/*
  q_fncmp()    search in filename from start len bytes for the first appearance
               of search which has searchlen
               wildcard could be one character or -1 (wildcard off)
  q_fcpy()     copy src from start for len to dest with mode (fopen(..., mode))
  q_fswap()    byteswap len bytes of file starting from start
  q_fcrc32()   calculate the crc32 of filename from start
  q_fbackup()

  modes

    BAK_DUPE (default)
      rename file to keep attributes and copy it back to old name and return
      new name

      filename -> rename() -> buf -> f_cpy() -> filename -> return buf

    BAK_MOVE
      just rename file and return new name (static)

      filename -> rename() -> buf -> return buf
*/
//TODO: give non-q_* names
extern int q_fncmp (const char *filename, int start, int len,
                    const char *search, int searchlen, int wildcard);
extern int q_fcpy (const char *src, int start, int len, const char *dest, const char *mode);
extern int q_fswap (const char *filename, int start, int len);
extern unsigned int q_fcrc32 (const char *filename, int start);
#if 1
#define BAK_DUPE 0
#define BAK_MOVE 1
extern char *q_fbackup (const char *filename, int mode);
#else
extern char *q_fbackup (char *move_name, const char *filename);
#endif
#ifndef  HAVE_ZLIB_H
extern int q_fsize (const char *filename);
#endif

/*
  Configuration file handling

  get_property()  get value of propname from filename or return value of env
                  with name like propname or return def
  get_property_int()  like get_property() but returns an integer which is 0
                  if the value of propname was 0, [Nn] or [Nn][Oo] and an
                  integer or at least 1 for every other case
  set_property()  set propname with value in filename
  DELETE_PROPERTY() like set_property but when value of propname is NULL the
                  whole property will disappear from filename
*/
extern const char *get_property (const char *filename, const char *propname, char *value, const char *def);
extern int32_t get_property_int (const char *filename, const char *propname, char divider);
extern int set_property (const char *filename, const char *propname, const char *value);
#define DELETE_PROPERTY(a, b) (set_property(a, b, NULL))

#ifdef __cplusplus
}
#endif
#endif // #ifndef MISC_H
