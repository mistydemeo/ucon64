/*
dxedll_pub.h - DXE client support code for uCON64

written by 2002 dbjh


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
  int (*fflush) (FILE *);

  void (*free) (void *);
  void *(*malloc) (size_t);
  void (*exit) (int);
  long (*strtol) (const char *, char **, int);

  void *(*memcpy) (void *, const void *, size_t);
  void *(*memset) (void *, int, size_t);
  int (*strcmp) (const char *, const char *);
  char *(*strcpy) (char *, const char *);
  char *(*strcat) (char *, const char *);
  int (*strcasecmp) (const char *, const char *);
  int (*strncasecmp) (const char *, const char *, size_t);
  char *(*strrchr) (const char *, int);

  int (*stat) (const char *, struct stat *);

  time_t (*time) (time_t *);

  // Put all variables AFTER the functions. This makes it easy to catch
  //  uninitialized function pointers.
  FILE __dj_stdin, __dj_stdout, __dj_stderr;
} st_symbol_t;

#ifdef __cplusplus
}
#endif

#endif // DXEDLL_PUB_H
