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

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdarg.h>
#include <ctype.h>
#include <dos.h>
#include <dpmi.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_symbol
{
  // functions exported by the DXE module
  int (*dxe_init) (void);
  void *(*dxe_symbol) (char *symbol_name);

  // variables exported by the DXE module
  int size;                                     // yes, this var needs to be here
                                                //  (or it could be a function)
  /*
     functions imported by the DXE module
     Note that _every_ function used by the DXE module and not defined in it
     should be listed here. That includes standard C library functions and
     variables.
  */
  int (*printf) (const char *, ...);
  int (*fprintf) (FILE *, const char *, ...);
  int (*vfprintf) (FILE *, const char *, va_list);
  int (*sprintf) (char *, const char *, ...);
  int (*vsprintf) (char *, const char *, va_list);
  int (*fputs) (const char *, FILE *);
  FILE *(*fopen) (const char *, const char *);
  FILE *(*fdopen) (int, const char *);
  int (*fclose) (FILE *);
  int (*fseek) (FILE *, long, int);
  long (*ftell) (FILE *);
  void (*rewind) (FILE *);
  size_t (*fread) (void *, size_t, size_t, FILE *);
  size_t (*fwrite) (const void *, size_t, size_t, FILE *);
  int (*fgetc) (FILE *file);
  char *(*fgets) (char *buffer, int maxlength, FILE *file);
  int (*feof) (FILE *file);
  int (*fputc) (int character, FILE *file);
  int (*fflush) (FILE *);
  int (*ferror) (FILE *);
  int (*rename) (const char *, const char *);

  void (*free) (void *);
  void *(*malloc) (size_t);
  void *(*calloc) (size_t, size_t);
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

  int (*tolower) (int);
  int (*toupper) (int);
  int (*isupper) (int);

  DIR *(*opendir) (const char *);
  struct dirent *(*readdir) (DIR *);
  int (*closedir) (DIR *);

  // va_start(), va_arg() and va_end() are macros

  int (*access) (const char *, int);
  int (*rmdir) (const char *);
  int (*isatty) (int);
  int (*chdir) (const char *);
  char *(*getcwd) (char *, size_t);
  int (*getuid) (void);

  int (*stat) (const char *, struct stat *);
  int (*mkdir) (const char *, mode_t);
  time_t (*time) (time_t *);
  void (*delay) (unsigned);
  int (*__dpmi_int) (int, __dpmi_regs *);

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
