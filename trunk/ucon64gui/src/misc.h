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

#include <time.h>                               // gauge() prototype contains time_t
#include <dirent.h>                             // for DIR2_t

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

// BeOS already has these (included from param.h via OS.h)
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define ROL(i, b) (((i)<<(b)) | ((i)>>(32-(b))))
#define BYTES2LONG(b, s) ( (((b)[0^(s)] & 0xffL) << 24) | \
                           (((b)[1^(s)] & 0xffL) << 16) | \
                           (((b)[2^(s)] & 0xffL) <<  8) | \
                           (((b)[3^(s)] & 0xffL)) )

#define LONG2BYTES(l, b, s)  (b)[0^(s)] = ((l)>>24)&0xff; \
                             (b)[1^(s)] = ((l)>>16)&0xff; \
                             (b)[2^(s)] = ((l)>> 8)&0xff; \
                             (b)[3^(s)] = ((l)    )&0xff;

#define RANDOM(min, max) ((rand () + min) % max)

#ifdef __GNUC__
#define NULL_TO_EMPTY(str) ((str)?:"")
#else
#define NULL_TO_EMPTY(str) ((str)?(str):"")
#endif // __GNUC__

#define OFFSET(a, offset) ((((unsigned char *)&(a))+(offset))[0])

extern int renlwr (const char *dir, int lower);
#define RENLWR(dir) (renlwr (dir, TRUE))
#define RENUPR(dir) (renlwr (dir, FALSE))

#if 0
#define CAT(a,b)  a##b
#define CAT3(a,b,c)       a##b##c
#define CAT4(a,b,c,d)     a##b##c##d
#endif

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE

#ifndef ARGS_MAX
#define ARGS_MAX 128
#endif // ARGS_MAX

/* A gambit to see whether we're on Solaris */
#ifdef __sun
#ifdef __SVR4
#define solaris
#endif
#endif

/*
  Some platforms use big-endian order internally (Mac,
  IBM 390); some use little-endian order (Intel).
*/
#ifdef WORDS_BIGENDIAN
#undef WORDS_BIGENDIAN
#endif

#ifdef _LIBC
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif
#endif

#ifdef WORDS_BIGENDIAN
#define CURRENT_ENDIAN_S  "Big-Endian"
#else
#define CURRENT_ENDIAN_S  "Little-Endian"
#endif

#ifdef  __MSDOS__                               // __MSDOS__ must come before __unix__, because DJGPP defines both
  #define CURRENT_OS_S "MSDOS"
#elif   defined __unix__
  #ifdef  __CYGWIN__
    #define CURRENT_OS_S "Win32"
  #elif   defined __FreeBSD__
    #define CURRENT_OS_S "Unix (FreeBSD)"
  #elif   defined __linux__
    #define CURRENT_OS_S "Unix (Linux)"
  #elif   defined sun
    #define CURRENT_OS_S "Unix (Solaris)"
  #else
    #define CURRENT_OS_S "Unix"
  #endif
#elif   defined __BEOS__
  #define CURRENT_OS_S "BeOS"
#else
  #define CURRENT_OS_S "?"
#endif

#ifdef __MSDOS__
#ifndef stderr
#define stderr          stdout                  // Stupid DOS has no error
#endif                                          //  stream (direct video writes)
#define FILE_SEPARATOR '\\'                     //  this makes redir possible
#define FILE_SEPARATOR_S "\\"
#if 0
#define OPTION '/'
#define OPTION_S " /"
#define OPTION_LONG_S " /"
#else
#define OPTION '-'
#define OPTION_S " -"
#define OPTION_LONG_S "--"
#endif
#else
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_S "/"
#define OPTION '-'
#define OPTION_S " -"
#define OPTION_LONG_S "--"
#endif

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
extern int kbhit (void);                        // may only be used after init_conio()!
#endif

extern unsigned short bswap_16(unsigned short x);
extern unsigned int bswap_32(unsigned int x);
extern unsigned long long int bswap_64(unsigned long long int x);


/*
  File extension handling

  setext() set/replace extension of filename with ext
  GETEXT() get extension of filename
  EXTCMP() compare extension of filename with ext

  extension means in this case the extension INCLUDING the dot '.'
*/
extern char *setext (char *filename, const char *ext);
#ifdef __GNUC__
#define GETEXT(filename) (strrchr ((strrchr (filename, FILE_SEPARATOR)?:filename), '.')?:"")
#else
extern const char *getext (const char *filename);
#define GETEXT getext
#endif // __GNUC__
#define EXTCMP(filename, ext) (stricmp (GETEXT (filename), ext))


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

  FILENAME_ONLY() returns only the filename from a complete path
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
#define FILENAME_ONLY(path) (strchr (path, FILE_SEPARATOR)?(&path[strrcspn (path, FILE_SEPARATOR_S) + 1]):path)
#if 0
extern const char *filenameonly (const char *str);
#define FILENAME_ONLY filenameonly
#endif


/*
  Advanced mem functions

  memwcmp()    memcmp with wildcard support
  memswap()    swap n Bytes from add on
  memhexdump() hexdump n Bytes from add on; you can use here a virtual_start for the displayed counter
  mem_crc32()   calculate the crc32 of add for n bytes
*/
extern int memwcmp (const void *add, const void *add_with_wildcards, size_t n, int wildcard);
extern void *memswap (void *add, size_t n);
extern void memhexdump (const void *add, size_t n, long virtual_start);
extern unsigned long mem_crc32 (unsigned int size, unsigned long crc, const void *buffer);


/*
  Quick IO functions

  quickfread()  same as fread but takes start and src is a filename
  quickfwrite() same as fwrite but takes start and dest is a filename; mode
                is the same as fopen() modes
  quickfgetc()  same as fgetc but takes filename instead of FILE and a pos
  quickfputc()  same as fputc but takes filename instead of FILE and a pos
  quickftell()  return size of file
*/
extern size_t quickfread (void *dest, size_t start, size_t len, const char *src);
extern size_t quickfwrite (const void *src, size_t start, size_t len, const char *dest, const char *mode);
extern int quickfgetc (const char *filename, long pos);
extern int quickfputc (const char *filename, long pos, int c, const char *mode);
extern long quickftell (const char *filename);


/*
  Advanced IO functions

  filencmp()    search in filename from start for len for the first appearance
                of search which has searchlen wildcard could be one character
                or -1 (wildcard off)
  filecopy()    copy src from start for len to dest with mode (fopen(..., mode))
  filebackup()  backup filename; if (move_name != NULL) filename will just be
                moved (renamed) and NOT duplicated (faster); move_name will
                contain the new name then
  fileswap()    byteswap file from start for len
  filehexdump() hexdump file from start for len (looks same as memhexdump())
  filefile()    compare two files for diff's or similarities
                similar must be TRUE or FALSE; TRUE==find similarities;
                FALSE==find differences
  filereplace() search filename from start for search which has slen and
                replace with replace which has rlen
  file_crc32()           calculate the crc32 of filename from start
*/
extern long filencmp (const char *filename, long start, long len,
  const char *search, long searchlen, int wildcard);
extern int filecopy (const char *src, long start, long len, const char *dest, const char *mode);
extern const char *filebackup (char *move_name, const char *filename);
extern int fileswap (const char *filename, long start, long len);
extern int filehexdump (const char *filename, long start, long len);
extern unsigned long filefile (const char *filename, long start, const char *filename2, long start2, int similar);
extern int filereplace (const char *filename, long start, const char *search, long slen, const char *replace, long rlen);
extern unsigned long file_crc32 (const char *filename, long start);


/*
  Misc stuff

  change_string() see header of implementation for usage
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
TODO: opendir2()  opendir() wrapper, additionally extracts archives (zip, lha, etc...)
                  into a temp_dir and returnes DIR pointer of temp_dir
                  returns NULL if the archive could'nt be extracted or archive
                  type not supported
TODO: closedir2() closedir() wrapper, recursively deletes temp_dir which was
                  created by opendir2()
  rmdir_R()       remove non-empty directory
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
extern int gauge (time_t init_time, long pos, long size);
extern char *getenv2 (const char *variable);
extern char *tmpnam2 (char *temp);
typedef struct
{
  DIR *dir;
  char fullpath[FILENAME_MAX];//==temppath
} DIR2_t;

extern DIR2_t *opendir2 (const char *archive_or_dir);
extern int closedir2 (DIR2_t *p);
int rmdir_R (const char *path);
extern int fsystem (FILE *output, const char *cmdline);
#ifdef  __unix__
extern int drop_privileges (void);
#endif
extern int register_func (void (*func) (void));
extern int unregister_func (void (*func) (void));
extern void handle_registered_funcs (void);


/*
  Configfile handling

  getProperty()    get value of propname from filename or return value of env
                   with name like propname or return def
  setProperty()    set propname with value in filename
  DELETEPROPERTY() like setProperty but when value of propname is NULL the
                   whole property will disappear from filename
*/
extern const char *getProperty (const char *filename, const char *propname, char *value, const char *def);
extern int setProperty (const char *filename, const char *propname, const char *value);
#define DELETEPROPERTY(a, b) (setProperty(a, b, NULL))


/*
  Net specific stuff

TODO: html_parser() little html_parser code (for config files?)
  tag2cmd()         parses a html tag into a system() conform cmdline
  query2cmd()       parses a query into a system() conform cmdline

  cmd2args()        parses a system() conform cmdline into main() conform args
*/
#if 0
extern int tag2cmd (char *str, const char *html_tag);
extern char *html_parser (const char *filename, char *buffer, ...);
"<tag>", "replacement", etc
#endif
extern char *html_parser (const char *filename, char *buffer);
extern int cmd2args (char **argv, const char *cmdline);
extern char *query2cmd (char *str, const char *uri, const char *query);

#endif // #ifndef MISC_H
