/*
misc.h - miscellaneous functions

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh

           
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

#include <limits.h>
#include <time.h>                               // gauge() prototype contains time_t
#include <dirent.h>

#if 1
#include "config.h"                             // ZLIB, ANSI_COLOR, NETWORK support
#else
/*
  if no config.h is present you may set the defines here
*/
#define ZLIB                                    // ZLIB support
#define ANSI_COLOR                              // support for ANSI color
#define NETWORK                                 // support for network 
                                          // (http_open(), ftp_open(), etc...)
#endif

#ifdef  ZLIB
#include <zlib.h>

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

#define fopen                           fopen2
#define fclose                          fclose2
#define fseek                           fseek2
#define fread                           fread2
#define fgetc                           fgetc2
#define fgets                           fgets2
#define feof                            feof2
#define fwrite                          fwrite2
#define fputc                           fputc2

#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE

#ifndef MAX_HOSTNAME
#define MAX_HOSTNAME 256
#endif // MAX_HOSTNAME

#ifndef ARGS_MAX
#define ARGS_MAX 128
#endif // ARGS_MAX

// BeOS already has these (included from param.h via OS.h)
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define NULL_TO_EMPTY(str) ((str)?(str):"")

#define RANDOM(min, max) ((rand () + min) % max)

#define OFFSET(a, offset) ((((unsigned char *)&(a))+(offset))[0])

#ifdef WORDS_BIGENDIAN
#undef WORDS_BIGENDIAN
#endif

#if     defined _LIBC || defined __GLIBC__
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif
#elif   defined __sparc__
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
#elif   defined __BEOS__
  #define CURRENT_OS_S "BeOS"
#else
  #define CURRENT_OS_S "?"
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

typedef struct st_func_node
{
  void (*func) (void);
  struct st_func_node *next;
} st_func_node_t;

#define stricmp strcasecmp
#define strnicmp strncasecmp

#if     defined __unix__ || defined __BEOS__
#define getch           getchar                 // getchar() acts like DOS getch() after init_conio()

extern void init_conio (void);
extern void deinit_conio (void);
extern int kbhit (void);                        // may only be used after init_conio()!
#endif


/*
  Byteswapping

  bswap_16() swap 16 bit integer
  bswap_32() swap 32 bit integer
  bswap_64() swap 64 bit integer
*/
extern unsigned short int bswap_16 (unsigned short int x);
extern unsigned int bswap_32 (unsigned int x);
extern unsigned long long int bswap_64 (unsigned long long int x);


/*
  File extension handling

  setext() set/replace extension of filename with ext
  getext() get extension of filename
    extension means in this case the extension INCLUDING the dot '.'

  filename_only() returns only the filename from a complete path
*/
extern char *setext (char *filename, const char *ext);
extern const char *getext (const char *filename);
#define EXTCMP(filename, ext) (stricmp (getext (filename), ext))
extern const char *filename_only (const char *str);


/*
  Advanced string manipulation

  strupr()
  strlwr()
  stpblk()   strip blanks/spaces/tabs from start of str
  strtrim()  trim blanks/spaces/tabs from start and end
  stpclr()   kill all returns at end of str
  areprint() like isprint() but for a whole string
  mkprint()  convert any str with ctrl chars to a clean string
  mkfile()   like mkprint but the resulting string could be used as a filename too
  strrcspn() same as strcspn() but looks for the LAST appearance of str2 and returns
             the position; if str2 could'nt be found strlen() will be returned
             example: strrcspn (".1234.6789",".") == 5
  findlwr()  test str for ANY lowercase characters
*/
extern char *strupr (char *str);
extern char *strlwr (char *str);
extern char *stpblk (char *str);
extern char *strtrim (char *str);
extern char *stplcr (char *str);
extern int areprint (const char *str, int size);
extern char *mkprint (char *str, const char replacement);
extern char *mkfile (char *str, const char replacement);
extern int strrcspn (const char *str, const char *str2);
extern int findlwr (const char *str);


/*
  Advanced mem functions

  memwcmp()    memcmp with wildcard support
  memswap()    swap n Bytes from add on
  mem_hexdump() hexdump n Bytes from add on; you can use here a virtual_start for the displayed counter
  mem_crc16()  calculate the crc16 of buffer for size bytes
  mem_crc32()  calculate the crc32 of buffer for size bytes
*/
extern int memwcmp (const void *add, const void *add_with_wildcards, size_t n, int wildcard);
extern void *memswap (void *add, size_t n);
extern void mem_hexdump (const void *add, size_t n, long virtual_start);
extern unsigned short mem_crc16 (unsigned int size, unsigned short crc16, const void *buffer);
#ifdef ZLIB
#define mem_crc32(SIZE, CRC, BUF)       crc32(CRC, BUF, SIZE)
#else
extern unsigned int mem_crc32 (unsigned int size, unsigned int crc32, const void *buffer);
#endif


/*
  Quick IO functions

  quick_fread()  same as fread but takes start and src is a filename
  quick_fwrite() same as fwrite but takes start and dest is a filename; mode
                is the same as fopen() modes
  quick_fgetc()  same as fgetc but takes filename instead of FILE and a pos
  quick_fputc()  same as fputc but takes filename instead of FILE and a pos
*/
extern size_t quick_fread (void *dest, size_t start, size_t len, const char *src);
extern size_t quick_fwrite (const void *src, size_t start, size_t len, const char *dest, const char *mode);
extern int quick_fgetc (const char *filename, long pos);
extern int quick_fputc (const char *filename, long pos, int c, const char *mode);


/*
  Advanced IO functions

  file_size()  return size of file
  filencmp()    search in filename from start for len for the first appearance
                of search which has searchlen wildcard could be one character
                or -1 (wildcard off)
  filecopy()    copy src from start for len to dest with mode (fopen(..., mode))
  file_backup()  backup filename; if (move_name != NULL) filename will just be
                moved (renamed) and NOT duplicated (faster); move_name will
                contain the new name then
  fileswap()    byteswap file from start for len
  file_hexdump() hexdump file from start for len (looks same as mem_hexdump())
  filefile()    compare two files for diff's or similarities
                similar must be TRUE or FALSE; TRUE==find similarities;
                FALSE==find differences
  file_replace() search filename from start for search which has slen and
                replace with replace which has rlen
  file_crc32()  calculate the crc32 of filename from start
*/
extern int file_size (const char *filename);
extern long filencmp (const char *filename, long start, long len,
  const char *search, long searchlen, int wildcard);
extern int filecopy (const char *src, long start, long len, const char *dest, const char *mode);
extern char *file_backup (char *move_name, const char *filename);
extern int fileswap (const char *filename, long start, long len);
extern int file_hexdump (const char *filename, long start, long len);
extern unsigned long filefile (const char *filename, long start, const char *filename2, long start2, int similar);
extern int file_replace (const char *filename, long start, const char *search, long slen, const char *replace, long rlen);
extern unsigned int file_crc32 (const char *filename, int start);


/*
  Misc stuff

  change_string() see header of implementation for usage
  ansi_init ()    initalize ansi output
  ansi_strip ()   strip ansi codes from a non-const string
  gauge()         init_time == time when gauge() was first started or when
                  the transfer did start
                  pos == current position
                  size == full size
                  gauge given these three values will calculate many
                  informative things like time, status bar, cps, etc.
                  it can be used for procedures which take some time to
                  inform the user about the actual progress
  getenv2()       getenv() clone suitable for enviroments w/o HOME, TMP or
                  TEMP variables
  opendir2()      opendir() wrapper, additionally extracts archives (zip, lha, etc...)
                  into a temp_dir and returnes DIR pointer of temp_dir
                  returns NULL if the archive could'nt be extracted or archive
                  type not supported
  closedir2()     closedir() wrapper, recursively deletes temp_dir which was
                  created by opendir2()
  rmdir_R()       remove non-empty directory
  renlwr()        rename all files in dir to lowercase
  fsystem()       system (or popen) wrapper, FILE *output could be stderr or
                  stdout or a filepointer
  drop_privileges() switch to the real user and group id (leave "root mode")
  tmpnam2()       replacement for buggy tmpnam() temp must have the size of FILENAME_MAX
  register_func() atexit() replacement
                  returns -1 if it fails, 0 if it was successful
  unregister_func() unregisters a previously registered function
                  returns -1 if it fails, 0 if it was successful
  handle_registered_funcs() calls all the registered functions
*/
extern void change_string (char *searchstr, int strsize, char wc, char esc,
                           char *end, int endsize, char *buf, int bufsize,
                           int offset, ...);
#ifdef  ANSI_COLOR
extern int ansi_init (void);
extern char *ansi_strip (char *str);
#endif
extern int gauge (time_t init_time, int pos, int size);
extern char *getenv2 (const char *variable);
extern char *tmpnam2 (char *temp);
extern DIR *opendir2 (char *archive_or_dir, const char *config_file, const char *property_format, const char *filename);
extern int closedir2 (DIR *p);
int rmdir_R (const char *path);
extern int renlwr (const char *dir);
extern int fsystem (FILE *output, const char *cmdline);
#ifdef  __unix__
extern int drop_privileges (void);
#endif
extern int register_func (void (*func) (void));
extern int unregister_func (void (*func) (void));
extern void handle_registered_funcs (void);


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
        functions, so they are only available if ZLIB is defined.
        Remove the #ifdef ZLIB if other code should use them.
*/
#ifdef  ZLIB

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


st_map_t *map_create (int n_elements);
void map_copy (st_map_t *dest, st_map_t *src);
st_map_t *map_put (st_map_t *map, void *key, void *object);
void *map_get (st_map_t *map, void *key);
void map_del (st_map_t *map, void *key);
void map_dump (st_map_t *map);
#endif // ZLIB

/*
  Configfile handling

  get_property()    get value of propname from filename or return value of env
                   with name like propname or return def
  set_property()    set propname with value in filename
  DELETE_PROPERTY() like set_property but when value of propname is NULL the
                   whole property will disappear from filename
*/
extern const char *get_property (const char *filename, const char *propname, char *value, const char *def);
extern int set_property (const char *filename, const char *propname, const char *value);
#define DELETE_PROPERTY(a, b) (set_property(a, b, NULL))


/*
  Network specific stuff

  url_parser()         splits any url into parts and returns st_url_t

  tcp_open()           open tcp socket, returns (int) file ptr
  udp_open()           open udp socket, returns (int) file ptr
*/
//extern int tag_to_cmd (char *str, const char *html_tag);
typedef struct st_url
{
/*
  [protocol://]host[:port][/path][?query]
*/
  char protocol[MAX_HOSTNAME];
  char host[MAX_HOSTNAME];
  int port;
  char path[FILENAME_MAX]; // uri
  char query[MAXBUFSIZE];

  char cmd[MAXBUFSIZE]; // the query parsed into a cmdline compatible string

/*
  [/path][?query] will be transformed into argc and argv for main() or getopt()
  by doing this http request's could be handled with getopt()

  (path == argv[0])
*/
  int argc;
  char *argv[ARGS_MAX];
} st_url_t;

extern st_url_t *url_parser (st_url_t *url_p, const char *url);
#ifdef NETWORK
extern int tcp_open (char *address, int port);
extern int udp_open (char *address, int port);
#endif // NETWORK

#ifdef __cplusplus
}
#endif
#endif // #ifndef MISC_H
