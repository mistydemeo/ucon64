/*
misc.h - miscellaneous functions

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh
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
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __sun
#ifdef __SVR4
#define __solaris__
#endif
#endif

#include <string.h>
#include <limits.h>
#include <time.h>                               // gauge() prototype contains time_t
#include <stdio.h>
#include <dirent.h>
#include "config.h"                             // HAVE_ZLIB_H, ANSI_COLOR support

#ifdef  HAVE_ZLIB_H
#include <zlib.h>
#include "unzip.h"

extern FILE *fopen2 (const char *filename, const char *mode);
extern int fclose2 (FILE *file);
extern int fseek2 (FILE *file, long offset, int mode);
extern size_t fread2 (void *buffer, size_t size, size_t number, FILE *file);
extern int fgetc2 (FILE *file);
extern char *fgets2 (char *buffer, int maxlength, FILE *file);
extern int feof2 (FILE *file);
extern size_t fwrite2 (const void *buffer, size_t size, size_t number, FILE *file);
extern int fputc2 (int character, FILE *file);

#undef feof                                     // necessary on (at least) Cygwin

#define fopen   fopen2
#define fclose  fclose2
#define fseek   fseek2
#define fread   fread2
#define fgetc   fgetc2
#define fgets   fgets2
#define feof    feof2
#define fwrite  fwrite2
#define fputc   fputc2

#endif                                          // HAVE_ZLIB_H

#if     defined __linux__ || defined __FreeBSD__ || \
        defined __BEOS__ || defined __solaris__ || HAVE_INTTYPES_H
#include <inttypes.h>
#elif   defined __CYGWIN__
#include <sys/types.h>
typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
typedef u_int64_t uint64_t;
#else
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

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
#elif   defined __APPLE__
  #if   defined __POWERPC__ || defined __ppc__
    #define CURRENT_OS_S "Apple (ppc)"
  #else
    #define CURRENT_OS_S "Apple"
  #endif
#elif   defined __BEOS__
  #define CURRENT_OS_S "BeOS"
#elif defined AMIGA
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

#ifdef __MSDOS__
#define FILE_SEPARATOR '\\'
#define FILE_SEPARATOR_S "\\"
#else
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_S "/"
#endif

#define OPTION '-'
#define OPTION_S " -"
#define OPTION_LONG_S "--"

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


/*
  String manipulation

  areprint() like isprint() but for a whole string
  areupper() like isupper() but for a whole string
  strupr()   convert string to upper-case
  strlwr()   convert string to lower-case
  mkprint()  convert all chars to isprint()'s
  mkfile()   convert string into a correct file name
  strtrim()  trim isspace()'s from start and end of string

  setext() set/replace extension of filename with ext
  getext() get extension of filename
    extension means in this case the extension INCLUDING the dot '.'

  basename() GNU basename() clone
*/
extern int areupper (const char *str);
extern int areprint (const char *str, int size);
extern char *strupr (char *str);
extern char *strlwr (char *str);
extern char *strtrim (char *str);
extern char *mkprint (char *str, const unsigned char replacement);
extern char *mkfile (char *str, const unsigned char replacement);
#define stricmp strcasecmp
#define strnicmp strncasecmp
extern char *setext (char *filename, const char *ext);
extern const char *getext (const char *filename);
#define EXTCMP(filename, ext) (strcasecmp (getext (filename), ext))
extern char *basename2 (const char *str);
#define basename basename2

/*
  mem functions

  memwcmp()    memcmp with wildcard support
  mem_swap()    swap n Bytes from add on
  mem_hexdump() hexdump n Bytes from add on; you can use here a virtual_start for the displayed counter
  mem_crc16()  calculate the crc16 of buffer for size bytes
  mem_crc32()  calculate the crc32 of buffer for size bytes
*/
extern int memwcmp (const void *add, const void *add_with_wildcards, uint32_t n, int wildcard);
extern void mem_hexdump (const void *add, uint32_t n, int virtual_start);
extern unsigned short mem_crc16 (unsigned int size, unsigned short crc16, const void *buffer);
#ifdef  HAVE_ZLIB_H
#define mem_crc32(SIZE, CRC, BUF)       crc32(CRC, BUF, SIZE)
#else
extern unsigned int mem_crc32 (unsigned int size, unsigned int crc32, const void *buffer);
#endif
extern void *mem_swap (void *add, uint32_t n);
#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#else
extern unsigned short int bswap_16 (unsigned short int x);
extern unsigned int bswap_32 (unsigned int x);
extern unsigned long long int bswap_64 (unsigned long long int x);
#endif


/*
  Misc stuff

  change_string() see header of implementation for usage
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
  rmdir2()        like rmdir but removes non-empty directories recursively
  tmpnam2()       replacement for tmpnam() temp must have the size of FILENAME_MAX
  ren()           renames all files tolower() or toupper() 
  mkstr()         transforms a string to isalnum(), isalpha(), etc.
  drop_privileges() switch to the real user and group id (leave "root mode")
  register_func() atexit() replacement
                  returns -1 if it fails, 0 if it was successful
  unregister_func() unregisters a previously registered function
                  returns -1 if it fails, 0 if it was successful
  handle_registered_funcs() calls all the registered functions
  wait2           wait (sleep) a specified number of milliseconds
*/
extern void change_string (char *searchstr, int strsize, char wc, char esc,
                           char *newstr, int newsize, char *buf, int bufsize,
                           int offset, ...);
extern int ansi_init (void);
extern char *ansi_strip (char *str);
extern int gauge (time_t init_time, int pos, int size);
extern char *getenv2 (const char *variable);
//#define getenv getenv2
extern char *tmpnam2 (char *temp);
extern int rmdir2 (const char *path);
extern int ren (const char *path, int (*func) (int));
//extern char *mkstr (char *str, int (*func) (int), int replacement);
#if     defined __unix__ && !defined __MSDOS__
extern int drop_privileges (void);
#endif
extern int register_func (void (*func) (void));
extern int unregister_func (void (*func) (void));
extern void handle_registered_funcs (void);
extern void wait2 (int nmillis);


/*
  Map functions

  map_create()    create a new map (associative array)
  map_copy()      copy map src to map dest
                  dest must be a larger map than src
  map_put()       put object in map under key
                  Callers should always reset the passed map pointer with the
                  one this function returns. This is necessary in case the map
                  had to be resized.
  map_get()       get object from map stored under key
                  returns NULL if there is no object with key in map
  map_del()       remove the object stored under key from map
  map_dump()      display the current contents of map

  The value MAP_FREE_KEY is reserved as a special key value. Don't use
  that value.

  NOTE: Currently the map functions are only used by the zlib wrapper
        functions, so they are only available if HAVE_ZLIB_H is defined.
        Remove the #ifdef HAVE_ZLIB_H in this file and in misc.c if other code should
        use them.
*/
#ifdef  HAVE_ZLIB_H

#define MAP_FREE_KEY 0

typedef struct st_map_element
{
  void *key;
  void *object;
} st_map_element_t;

typedef struct st_map
{
  st_map_element_t *data;
  int size;
} st_map_t;


extern st_map_t *map_create (int n_elements);
extern void map_copy (st_map_t *dest, st_map_t *src);
extern st_map_t *map_put (st_map_t *map, void *key, void *object);
extern void *map_get (st_map_t *map, void *key);
extern void map_del (st_map_t *map, void *key);
extern void map_dump (st_map_t *map);
#endif // HAVE_ZLIB_H

/*
  Configuration file handling

  get_property()  get value of propname from filename or return value of env
                  with name like propname or return def
  set_property()  set propname with value in filename
  DELETE_PROPERTY() like set_property but when value of propname is NULL the
                  whole property will disappear from filename
*/
extern const char *get_property (const char *filename, const char *propname, char *value, const char *def);
extern int set_property (const char *filename, const char *propname, const char *value);
#define DELETE_PROPERTY(a, b) (set_property(a, b, NULL))


#ifdef  HAVE_ZLIB_H
// Returns the number of files in the "central dir of this disk" or -1 if
//  filename is not a ZIP file or an error occured.
extern int unzip_get_number_entries (const char *filename);
extern int unzip_goto_file (unzFile file, int file_index);
extern int unzip_current_file_nr;
#endif


#ifdef __cplusplus
}
#endif
#endif // #ifndef MISC_H
