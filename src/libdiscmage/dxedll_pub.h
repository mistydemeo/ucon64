/*
dxedll_pub.h - DXE client support code

written by 2002 - 2003 dbjh


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

#ifndef DXEDLL_PUB_H
#define DXEDLL_PUB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <dos.h>
#include <ctype.h>
#include <dpmi.h>
#include "config.h"
#ifdef  HAVE_ZLIB_H
#include <zlib.h>
#include "unzip.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_symbol
{
  // functions exported by the DXE module
  int (*dxe_init) (void);
  void *(*dxe_symbol) (char *symbol_name);

  /*
     functions imported by the DXE module
     Note that _every_ function used by the DXE module and not defined in it
     should be listed here. That includes standard C library functions and
     variables.
  */
  int (*printf) (const char *, ...);
  int (*fprintf) (FILE *, const char *, ...);
  int (*sprintf) (char *, const char *, ...);
  int (*fputs) (const char *, FILE *);
  FILE *(*fopen) (const char *, const char *);
  int (*fclose) (FILE *);
  int (*fseek) (FILE *, long, int);
  long (*ftell) (FILE *);
  size_t (*fread) (void *, size_t, size_t, FILE *);
  size_t (*fwrite) (const void *, size_t, size_t, FILE *);
  int (*fgetc) (FILE *file);
  char *(*fgets) (char *buffer, int maxlength, FILE *file);
  int (*feof) (FILE *file);
  int (*fputc) (int character, FILE *file);
  int (*fflush) (FILE *);
  int (*rename) (const char *, const char *);

  void (*free) (void *);
  void *(*malloc) (size_t);
  void (*exit) (int);
  long (*strtol) (const char *, char **, int);
  char *(*getenv) (const char *);
  void (*srand) (unsigned);
  int (*rand) (void);

  void *(*memcpy) (void *, const void *, size_t);
  void *(*memset) (void *, int, size_t);
  int (*strcmp) (const char *, const char *);
  char *(*strcpy) (char *, const char *);
  char *(*strcat) (char *, const char *);
  char *(*strncat) (char *, const char *, size_t);
  int (*strcasecmp) (const char *, const char *);
  int (*strncasecmp) (const char *, const char *, size_t);
  char *(*strchr) (const char *, int);
  char *(*strrchr) (const char *, int);
  char *(*strpbrk) (const char *, const char *);
  size_t (*strspn) (const char *, const char *);
  size_t (*strcspn) (const char *, const char *);
  size_t (*strlen) (const char *);

  int (*stat) (const char *, struct stat *);
  time_t (*time) (time_t *);
  void (*delay) (unsigned);
  int (*__dpmi_int) (int, __dpmi_regs *);
  int (*tolower) (int);
  
  DIR *(*opendir) (const char *);
  struct dirent *(*readdir) (DIR *);
  int (*closedir) (DIR *);
  
  int (*access) (const char *, int);
  int (*rmdir) (const char *);
  int (*isatty) (int);
  int (*chdir) (const char *);
  char *(*getcwd) (char *, size_t);

#ifdef  HAVE_ZLIB_H
  // zlib functions
  gzFile (*gzopen) (const char *path, const char *mode);
  int (*gzclose) (gzFile file);
  int (*gzwrite) (gzFile file, const voidp buf, unsigned len);
  char *(*gzgets) (gzFile file, char *buf, int len);
  int (*gzeof) (gzFile file);
  z_off_t (*gzseek) (gzFile file, z_off_t offset, int whence);
  int (*gzputc) (gzFile file, int c);
  int (*gzread) (gzFile file, voidp buf, unsigned len);
  int (*gzgetc) (gzFile file);
  int (*gzrewind) (gzFile file);

  // unzip functions
  unzFile (*unzOpen) (const char *path);
  int (*unzOpenCurrentFile) (unzFile file);
  int (*unzGoToFirstFile) (unzFile file);
  int (*unzClose) (unzFile file);
  int (*unzGetGlobalInfo) (unzFile file, unz_global_info *pglobal_info);
  int (*unzGoToNextFile) (unzFile file);
  int (*unzCloseCurrentFile) (unzFile file);
  int (*unzeof) (unzFile file);
  int (*unzReadCurrentFile) (unzFile file, voidp buf, unsigned len);
  z_off_t (*unztell) (unzFile file);
  int (*unzGetCurrentFileInfo) (unzFile file, unz_file_info *pfile_info,
                                char *szFileName, uLong fileNameBufferSize,
                                void *extraField, uLong extraFieldBufferSize,
                                char *szComment, uLong commentBufferSize);
#endif // HAVE_ZLIB_H

  // Put all variables AFTER the functions. This makes it easy to catch
  //  uninitialized function pointers.
  FILE __dj_stdin, __dj_stdout, __dj_stderr;
  // WARNING: actually the __dj_ctype_X variables are arrays
  unsigned short *__dj_ctype_flags;
  unsigned char *__dj_ctype_tolower, *__dj_ctype_toupper;
  int errno;
} st_symbol_t;

#ifdef __cplusplus
}
#endif

#endif // DXEDLL_PUB_H
