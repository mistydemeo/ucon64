/*
misc.h - miscellaneous functions

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2001 dbjh


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

#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/ioctl.h>
#include <sys/stat.h>
//#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef __DOS__
#include <conio.h>		// getch()
#include <pc.h>			// kbhit()
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef __DOS__
#define STDERR          stderr
#else
#define STDERR          stdout	// Stupid DOS has no error
#endif //  stream (direct video writes)
						//  this makes redir possible
#if     (__UNIX__ || __BEOS__)
#define getch           getchar	// getchar() acts like DOS getch() after init_conio()

void init_conio (void);
int kbhit (void);		// may only be used after init_conio()!
#endif

int allascii (char *b, int size);	// test if a string is completely ascii


int strdcmp (char *str, char *str1);

int argcmp (int argc		// check the cmdline options for str
	    , char *argv[], char *str);

int argncmp (int argc		//same as argcmp() but with length
	     , char *argv[], char *str, size_t len);

char *getarg (int argc		//get arg pos from cmdline options
	      , char *argv[], int pos);

long getarg_intval (int argc, char **argv, char *argname);

int findlwr (char *str);	//find any lower char in str

char *strupr (char *str);

char *strlwr (char *str);

char *cmd2url (char *cmd, char *url);	//converts a commandline into an url(!)

char *url2cmd (char *url, char *cmd);	//converts an url into a commandline(!)

char *ip2domain (char *ip, char *domain);	//dns

char *domain2ip (char *domain, char *ip);	//dns

char *newext (char *filename	//replace extension of str with ext
	      , char *ext);

int extcmp (char *filename	//compares if filename has the ext extension and returnes -1 if false or 0 if true
	    , char *ext);

int findlast (const char *str	//find last appearance of str2 in str
	      , const char *str2);

char *stpblk (char *str);	//strip blanks from beginning of str

char *strtrim (char *dest	//trim blanks from start and end of str
	       , char *str);

char *stplcr (char *str);	//kill all returns at end of str

char *strswap (char *str	//swap bytes in str from start for len
	       , long start, long len);

int strhexdump (char *str	//hexdump str from start for len
		, long start, long virtual_start	//if str is only a part of a bigger buffer you can write here the start number of the counter which is displayed left
		, long len);

int renlwr (char *dir);		//rename all files in dir to lower case

int renupr (char *dir);		//rename all files in dir to upper case

long quickftell (char *filename);	//return size of filename

long filencmp (char *filename	//search filename for search with searchlen from start for len
	       , long start, long len, char *search, long searchlen	//length of *search in Bytes
  );

long filencmp2 (char *filename	//same as filencmp but with wildcard support
		, long start, long len, char *search, long searchlen	//length of *search in Bytes
		, char wildcard);

size_t quickfread (void *dest	//same as fread but takes start and src is a filename
		   , size_t start, size_t len, char *src);


size_t quickfwrite (const void *src	//same as fwrite but takes start and dest is a filename
		    , size_t start, size_t len, char *dest, char *mode	//same like fopen() modes
  );


int quickfgetc (char *filename	//same as fgetc but takes filename instead of FILE and a pos
		, long pos);

int quickfputc (char *filename, long pos, int c, char *mode);

int filehexdump (char *filename	//same as strhexdump
		 , long start, long len);


int filecopy (char *src		//copy src from start for len to dest
	      , long start, long len, char *dest, char *mode	//same like fopen() modes
  );

char *filebackup (char *filename);

char *filenameonly (char *str);	//extracts only the filename from a complete path

unsigned long filefile (char *filename	//compare filename from start with filename2 from start2
			, long start, char *filename2, long start2, int similar	//TRUE==find similarities; FALSE==find differences
  );

int filereplace (char *filename	//search filename from start for search which has slen and replace with replace which has rlen
		 , long start, char *search, long slen, char *replace,
		 long rlen);

// see header of implementation for usage
void change_string (char *searchstr, int strsize, char wc, char esc,
		    char *end, int endsize, char *buf, int bufsize,
		    int offset, ...);

int fileswap (char *filename	//bytesswap filename from start for len
	      , long start, long len);

char *getProperty (char *filename, char *propname, char *buffer, char *def);

char **getTags (char *filename, char *tagname, char **buffer);

#ifdef __DOS__
#define FILE_SEPARATOR '\\'
#define FILE_SEPARATOR_S "\\"
#else
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_S "/"
#endif

#endif // #ifndef MISC_H
