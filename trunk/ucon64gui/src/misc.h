/*
misc.h - miscellaneous functions

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
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

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifdef __MSDOS__
#include <conio.h>                              // getch()
#include <pc.h>                                 // kbhit()
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef __MSDOS__
#define STDERR          stderr
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_S "/"
#else
#define STDERR          stdout                  // Stupid DOS has no error
#define FILE_SEPARATOR '\\'                     //  stream (direct video writes)
#define FILE_SEPARATOR_S "\\"                   //  this makes redir possible
#endif                                          

#if     defined __UNIX__ || defined __BEOS__
#define getch           getchar                 // getchar() acts like DOS getch() after init_conio()

void init_conio (void);
int kbhit (void);                               // may only be used after init_conio()!
#endif

int areprint (char *str, int size);    // like isprint() but for a whole string

int strdcmp (char *str, char *str1);            //compares case AND length

int stricmp(const char *s1, const char *s2);

int strnicmp(const char *s1, const char *s2, size_t n);

/*
  don't use _splitpath() and or _makepath()
*/
void _makepath (char *path, const char *node, const char *dir,
                const char *fname, const char *ext);//do not use

void _splitpath (const char *path,
                 char *node, char *dir, char *fname, char *ext);//do not use

int argcmp (int argc, char *argv[], char *str); // check the cmdline options for str

int argncmp (int argc, char *argv[], char *str, size_t len); // same as argcmp() but with length

char *getarg (int argc, char *argv[], int pos); // get arg pos from cmdline options

long getarg_intval (int argc, char **argv, char *argname);

int findlwr (char *str);                        // find any lower char in str

char *strupr (char *str);

char *strlwr (char *str);

char *newext (char *filename, char *ext);       // replace extension of str with ext

int extcmp (char *filename, char *ext);         // compares if filename has the ext extension and returnes -1 if false or 0 if true

int findlast (const char *str, const char *str2); // find last appearance of str2 in str

char *stpblk (char *str);                       // strip blanks from beginning of str

char *strtrim (char *dest, char *str);          // trim blanks from start and end of str

char *stplcr (char *str);                       // kill all returns at end of str

char *strswap (char *str, long start, long len);// swap bytes in str from start for len

/*
  hexdump str from start for len
  if str is only a part of a bigger buffer you can write here the start number
  of the counter which is displayed left
*/
int strhexdump (char *str, long start, long virtual_start, long len);

int renlwr (char *dir);                         // rename all files in dir to lower case

int renupr (char *dir);                         // rename all files in dir to upper case

long quickftell (char *filename);               // return size of filename

// search filename for search with searchlen from start for len
// searchlen is length of *search in Bytes
long filencmp (char *filename, long start, long len, char *search, long searchlen);

// same as filencmp but with wildcard support
// searchlen is length of *search in Bytes
long filencmp2 (char *filename, long start, long len, char *search, long searchlen,
                char wildcard);

// same as fread but takes start and src is a filename
size_t quickfread (void *dest, size_t start, size_t len, char *src);

// same as fwrite but takes start and dest is a filename
// mode is the same as fopen() modes
size_t quickfwrite (const void *src, size_t start, size_t len, char *dest, char *mode);

// same as fgetc but takes filename instead of FILE and a pos
int quickfgetc (char *filename, long pos);

int quickfputc (char *filename, long pos, int c, char *mode);

int filehexdump (char *filename, long start, long len); // same as strhexdump

// copy src from start for len to dest
// mode is the same as fopen() modes
int filecopy (char *src, long start, long len, char *dest, char *mode);

char *filebackup (char *filename);

char *filenameonly (char *str);                 // extracts only the filename from a complete path

// compare filename from start with filename2 from start2
// similar must be TRUE or FALSE; TRUE==find similarities; FALSE==find differences
unsigned long filefile (char *filename, long start, char *filename2, long start2, int similar);

// search filename from start for search which has slen and replace with replace which has rlen
int filereplace (char *filename, long start, char *search, long slen, char *replace,
                 long rlen);

// see header of implementation for usage
void change_string (char *searchstr, int strsize, char wc, char esc,
                    char *end, int endsize, char *buf, int bufsize,
                    int offset, ...);

int fileswap (char *filename, long start, long len); // bytesswap filename from start for len

char *getchd (char *buffer, size_t buffer_size);// getenv("HOME") for stupid DOS
                                                // acts like getcwd()
  
char *getProperty (char *filename, char *propname, char *value, char *def); //get value of propname from filename or return value of env with name like propname or return def
int setProperty (char *filename, char *propname, char *value); //set propname with value in filename
#define deleteProperty(a, b) (setProperty(a, b, NULL))//like setProperty but when value of propname is NULL the whole property will disappear from filename

// extract only the links from ugly HTML pages for wget -m --input-file=FILE
char *getLinks (char *filename, char *buffer); //do not use!

#endif // #ifndef MISC_H
