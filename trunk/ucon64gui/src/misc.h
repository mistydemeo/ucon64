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

/* First a gambit to see whether we're on Solaris */
#ifdef __sun
#ifdef __SVR4
#define solaris
#endif
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef __MSDOS__
#ifndef stderr
#define stderr          stdout                  // Stupid DOS has no error
#endif
#define FILE_SEPARATOR '\\'                     //  stream (direct video writes)
#define FILE_SEPARATOR_S "\\"                   //  this makes redir possible
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

#define stricmp strcasecmp
#define strnicmp strncasecmp

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define ROL(i, b) (((i)<<(b)) | ((i)>>(32-(b))))
#define BYTES2LONG(b, s) ( (((b)[0^(s)] & 0xffL) << 24) | \
                           (((b)[1^(s)] & 0xffL) << 16) | \
                           (((b)[2^(s)] & 0xffL) <<  8) | \
                           (((b)[3^(s)] & 0xffL)) )

#define LONG2BYTES(l, b, s)  (b)[0^(s)] = ((l)>>24)&0xff; \
                             (b)[1^(s)] = ((l)>>16)&0xff; \
                             (b)[2^(s)] = ((l)>> 8)&0xff; \
                             (b)[3^(s)] = ((l)    )&0xff;


#ifdef __GNUC__
#define NULL_TO_EMPTY(str) ((str)?:"")
#else
#define NULL_TO_EMPTY(str) ((str)?(str):"")
#endif // __GNUC__

#ifdef _LIBC
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif
#endif

#ifdef WORDS_BIGENDIAN
#define SWAP(n) \
  (((n) << 24) | (((n) & 0xff00) << 8) | (((n) >> 8) & 0xff00) | ((n) >> 24))
#else
#define SWAP(n) (n)
#endif

#define MEM(a, offset) ((((unsigned char *)&(a))+(offset))[0])

extern int rencase (const char *dir, const char *mode);
#define RENLWR(dir) (rencase (dir, "lwr"))
#define RENUPR(dir) (rencase (dir, "upr"))

#define CAT(a,b)  a##b
#define CAT3(a,b,c)       a##b##c
#define CAT4(a,b,c,d)     a##b##c##d

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE

#ifndef ARGS_MAX
#define ARGS_MAX 128
#endif // ARGS_MAX

#if     defined __unix__ || defined __BEOS__
#define getch           getchar                 // getchar() acts like DOS getch() after init_conio()

extern void init_conio (void);
extern int kbhit (void);                        // may only be used after init_conio()!
#endif

/*
  File extension handling

  setext() set/replace extension of filename with ext
  GETEXT() get extension of filename
  EXTCMP() compare extension of filename with ext
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
*/
extern int memwcmp (const void *add, const void *add_with_wildcards, size_t n, int wildcard);
extern void *memswap (void *add, size_t n);
extern void memhexdump (const void *add, size_t n, long virtual_start);


/*
  Quick IO functions

  quickfread()  same as fread but takes start and src is a filename
  quickfwrite() same as fwrite but takes start and dest is a filename; mode
                is the same as fopen() modes
  quickfgetc()  same as fgetc but takes filename instead of FILE and a pos
  quickfputc()  same as fputc but takes filename instead of FILE and a pos
  quickftell()  return size of file
*/
extern size_t quickfread (const void *dest, size_t start, size_t len, const char *src);
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
*/  
extern long filencmp (const char *filename, long start, long len,
  const char *search, long searchlen, int wildcard);
extern int filecopy (const char *src, long start, long len, const char *dest, const char *mode);
extern const char *filebackup (char *move_name, const char *filename);
extern int fileswap (const char *filename, long start, long len);
extern int filehexdump (const char *filename, long start, long len);
extern unsigned long filefile (const char *filename, long start, const char *filename2, long start2, int similar);
extern int filereplace (const char *filename, long start, const char *search, long slen, const char *replace, long rlen);


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
*/                  
extern void change_string (char *searchstr, int strsize, char wc, char esc,
                    char *end, int endsize, char *buf, int bufsize,
                    int offset, ...);
extern int gauge (time_t init_time, long pos, long size);
extern char *getenv2 (const char *variable);
#if 0
extern DIR *opendir2 (const char *archive_or_dir);
extern int closedir2 (DIR *p);
#endif


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

  st_html_replace_t datatype that holds an array with stuff that should
                    be replaced by the html_parser
TODO: html_parser() little html_parser code (for config files?)
  query2args()      parses a query into main() conform args
  query2cmd()       parses a query into a system() conform cmdline

  query2args() and query2cmd() could be used in apache modules (not DSO, DSO sucks)
*/
typedef struct st_html_replace
{
  char *before;
  char *after;
} st_html_replace_t;
#if 0
extern char *html_parser (const char *filename, char *buffer, st_html_replace_t **replacements);
#endif 
extern char *html_parser (const char *filename, char *buffer);
extern int query2args (char **argv, const char *uri, const char *query);
extern char *query2cmd (char *str, const char *uri, const char *query);

#endif // #ifndef MISC_H
