/*
misc.h - miscellaneous functions

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh
           2002 - 2003 Jan-Erik Karlsson (Amiga)


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

#ifdef  __cplusplus
extern "C" {
#endif

#include <string.h>
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

#if     defined __linux__ || defined __FreeBSD__ || \
        defined __BEOS__ || defined __solaris__ || defined HAVE_INTTYPES_H
#include <inttypes.h>
#elif   defined __CYGWIN__
#include <sys/types.h>
#ifndef OWN_INTTYPES
#define OWN_INTTYPES                            // signal that these are defined
#if     __GNUC__ < 3
typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
typedef u_int64_t uint64_t;
#endif
#endif                                          // OWN_INTTYPES
#else                                           // __MSDOS__, _WIN32, AMIGA
#ifndef OWN_INTTYPES
#define OWN_INTTYPES                            // signal that these are defined
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
#ifndef _WIN32
typedef unsigned long long int uint64_t;
#else
typedef unsigned __int64 uint64_t;
#endif
#ifndef AMIGA                                   // __BIT_TYPES_DEFINED__?
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
#ifndef _WIN32
typedef signed long long int int64_t;
#else
typedef signed __int64 int64_t;
#endif
#endif                                          // AMIGA
#endif                                          // OWN_INTTYPES
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
#elif   defined AMIGA || defined __sparc__ || defined __BIG_ENDIAN__ || \
        defined __APPLE__
  #define WORDS_BIGENDIAN 1
#endif

#ifdef  __MSDOS__                               // __MSDOS__ must come before __unix__,
  #define CURRENT_OS_S "MSDOS"                  //  because DJGPP defines both
#elif   defined __unix__
  #ifdef  __CYGWIN__
    #define CURRENT_OS_S "Win32 (Cygwin)"
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
#elif   defined _WIN32
  #ifdef  __MINGW32__
    #define CURRENT_OS_S "Win32 (MinGW)"
  #else
    #define CURRENT_OS_S "Win32 (Visual C++)"
  #endif
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
    #define CURRENT_OS_S "AmigaPPC"
  #else
    #define CURRENT_OS_S "Amiga"                // 68k
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
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) (bswap_16(x))
#define le2me_32(x) (bswap_32(x))
#define le2me_64(x) (bswap_64(x))
#else
#define me2be_16(x) (bswap_16(x))
#define me2be_32(x) (bswap_32(x))
#define me2be_64(x) (bswap_64(x))
#define me2le_16(x) (x)
#define me2le_32(x) (x)
#define me2le_64(x) (x)
#define be2me_16(x) (bswap_16(x))
#define be2me_32(x) (bswap_32(x))
#define be2me_64(x) (bswap_64(x))
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

#if     (defined __unix__ || defined __BEOS__ || defined AMIGA) && !defined __MSDOS__
// Cygwin, GNU/Linux, Solaris, FreeBSD, BeOS, Amiga
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

#if (defined __unix__ && !defined __MSDOS__) || defined __BEOS__
extern void init_conio (void);
extern void deinit_conio (void);
#define getch           getchar                 // getchar() acts like DOS getch() after init_conio()
extern int kbhit (void);                        // may only be used after init_conio()!

#elif   defined __MSDOS__
#include <conio.h>                              // getch()
#include <pc.h>                                 // kbhit()

#elif   defined _WIN32
#include <conio.h>                              // kbhit() & getch()

#elif   defined AMIGA
extern int kbhit (void);
//#define getch           getchar
// Gonna use my (Jan-Erik) fake one. Might work better and more like the real
//  getch().
#endif

#ifdef  __CYGWIN__
extern char *fix_character_set (char *value);
#endif

/*
  String manipulation

  isfname()   test if char could be used for filenames
  isprint2()  test if char could be used for stdout
  tofname()   replaces chars that can not be used for filenames
  toprint2()  replaces chars that should not be used for stdout

  is_func()   use all is*() functions on an array of char
  to_func()   use all to*() functions on an array of char

  strtrim()   trim isspace()'s from start and end of string

  get_suffix() get suffix of filename
  set_suffix() set/replace suffix of filename with suffix
              suffix means in this case the suffix INCLUDING the dot '.'
  set_suffix_i() like set_suffix(), but doesn't change the case

  basename2() DJGPP basename() clone
  dirname2()  DJGPP dirname() clone
  realpath2() realpath() replacement
  one_file()  returns 1 if two filenames refer to one file, otherwise it
              returns 0
  one_filesystem() returns 1 if two filenames refer to files on one file
              system, otherwise it returns 0
  mkdir2()    mkdir() wrapper who automatically cares for rights, etc.
  rename2()   renames oldname to newname even if oldname and newname are not
              on one file system
  truncate2() don't use truncate() to enlarge files, because the result is
              undefined (by POSIX) use truncate2() instead which does both
  argz_extract2() simplified argz_extract() replacement
  argz_extract3() like argz_extract2() but for spaces only
*/
extern int isfname (int c);
extern int isprint2 (int c);
extern int tofname (int c);
extern int toprint2 (int c);
extern int is_func (char *s, int size, int (*func) (int));
extern char *to_func (char *s, int size, int (*func) (int));
#define strupr(s) (to_func(s, strlen(s), toupper))
#define strlwr(s) (to_func(s, strlen(s), tolower))
//#ifndef HAVE_STRCASESTR
// strcasestr is GNU only
extern char *strcasestr2 (const char *str, const char *search);
#define stristr strcasestr2
//#else
//#define stristr strcasestr
//#endif
#ifndef _WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif
extern char *strtrim (char *str);
extern const char *get_suffix (const char *filename);
extern char *set_suffix (char *filename, const char *suffix);
extern char *set_suffix_i (char *filename, const char *suffix);
extern char *basename2 (const char *path);
// override a possible XPG basename() which modifies its arg
#define basename basename2
extern char *dirname2 (const char *path);
#define dirname dirname2
extern int one_file (const char *filename1, const char *filename2);
extern int one_filesystem (const char *filename1, const char *filename2);
extern char *realpath2 (const char *src, char *full_path);
extern int mkdir2 (const char *name);
extern int rename2 (const char *oldname, const char *newname);
extern int truncate2 (const char *filename, int size);
extern int argz_extract2 (char **argv, char *str, const char *separator_s, int max_args);
#define argz_extract3(a,c,m) argz_extract2(a,c," ",m)


/*
  mem functions

  memwcmp()    memcmp with wildcard support
  mem_search() search for a byte sequence
  mem_swap_b() swap n bytes of buffer
  mem_swap_w() swap n/2 words of buffer
  mem_hexdump() hexdump n bytes of buffer; you can use here a virtual_start for the displayed counter
  crc16()      calculate the crc16 of buffer for size bytes
  crc32()      calculate the crc32 of buffer for size bytes
*/
extern int memwcmp (const void *buffer, const void *search, uint32_t searchlen, int wildcard);
extern void *mem_search (const void *buffer, uint32_t buflen, const void *search, uint32_t searchlen);
extern void *mem_swap_b (void *buffer, uint32_t n);
extern void *mem_swap_w (void *buffer, uint32_t n);
extern void mem_hexdump (const void *buffer, uint32_t n, int virtual_start);
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

  change_mem{2}() see header of implementation for usage
  build_cm_patterns() helper function for change_mem2() to read search patterns
                  from a file
  cleanup_cm_patterns() helper function for build_cm_patterns() to free all
                  memory allocated for a (list of) st_pattern_t structure(s)
  ansi_init()     initialize ANSI output
  ansi_strip()    strip ANSI codes from a string
  gauge()         init_time == time when gauge() was first started or when
                  the transfer started
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
#if     defined UCON64 && !defined DLL
extern int build_cm_patterns (st_cm_pattern_t **patterns, const char *filename,
                              char *fullfilename);
#endif
extern void cleanup_cm_patterns (st_cm_pattern_t **patterns, int n_patterns);
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
  q_rfcpy()    copy src to dest without looking at the file data (no
               decompression like with q_fcpy())
  q_fswap()    swap len bytes of file starting from start
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
// TODO: give non-q_* names
typedef enum { SWAP_BYTE, SWAP_WORD } swap_t;

extern int q_fncmp (const char *filename, int start, int len,
                    const char *search, int searchlen, int wildcard);
extern int q_fcpy (const char *src, int start, int len, const char *dest, const char *mode);
extern int q_rfcpy (const char *src, const char *dest);
extern int q_fswap (const char *filename, int start, int len, swap_t type);
#define q_fswap_b(f, s, l) q_fswap(f, s, l, SWAP_BYTE)
#define q_fswap_w(f, s, l) q_fswap(f, s, l, SWAP_WORD)
extern int q_fcrc32 (const char *filename, int start);
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


#ifdef  _WIN32
// Note that _WIN32 is defined by cl.exe while the other constants (like WIN32)
//  are defined in header files. MinGW's gcc.exe defines all constants.

#include <sys/types.h>

extern int truncate (const char *path, off_t size);
extern int sync (void);
// For MinGW popen() and pclose() are unavailable for DLL's. For DLL's _popen()
//  and _pclose() should be used. Visual C++ only has the latter two.
#ifndef pclose                                  // miscz.h's definition gets higher "precedence"
#define pclose  _pclose
#endif
#ifndef popen                                   // idem
#define popen   _popen
#endif

#ifdef  ANSI_COLOR
#include <stdarg.h>

extern int vprintf2 (const char *format, va_list argptr);
extern int printf2 (const char *format, ...);
extern int fprintf2 (FILE *file, const char *format, ...);
#define vprintf vprintf2
#define printf  printf2
#define fprintf fprintf2
#endif // ANSI_COLOR

#ifndef __MINGW32__
#include <io.h>
#include <direct.h>
#include <sys/stat.h>                           // According to MSDN <sys/stat.h> must
                                                //  come after <sys/types.h>. Yep, that's M$.
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
#define strnicmp _strnicmp
#endif // DLL

#endif // !__MINGW32__

#elif   defined AMIGA                           // _WIN32
// custom _popen() and _pclose(), because the standard ones (named popen() and
//  pclose()) are buggy
#ifndef pclose                                  // miscz.h's definition gets higher "precedence"
#define pclose  _pclose
#endif
#ifndef popen                                   // idem
#define popen   _popen
#endif
extern FILE *_popen (const char *path, const char *mode);
extern int _pclose (FILE *stream);
#endif                                          // AMIGA

#ifdef  __cplusplus
}
#endif

#endif // #ifndef MISC_H
