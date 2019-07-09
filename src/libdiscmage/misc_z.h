/*
misc_z.h - miscellaneous zlib functions

Copyright (c) 2001 - 2004, 2016 - 2017, 2019 dbjh


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
#ifndef MISC_Z_H
#define MISC_Z_H

#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef  USE_ZLIB
// make sure ZLIB support is enabled everywhere
//#warning USE_ZLIB is defined

#include <stdio.h>
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
extern long ftell2 (FILE *file);
extern void rewind2 (FILE *file);
extern FILE *popen2 (const char *command, const char *mode);
extern int pclose2 (FILE *stream);

extern off_t q_fsize2 (const char *filename);

#define fopen(FILE, MODE) fopen2(FILE, MODE)
#define fclose(FILE) fclose2(FILE)
#define fseek(FILE, OFFSET, MODE) fseek2(FILE, OFFSET, MODE)
#define fread(BUF, SIZE, NUM, FILE) fread2(BUF, SIZE, NUM, FILE)
#define fgetc(FILE) fgetc2(FILE)
#undef  fgets                                   // necessary on (at least) NetBSD
#define fgets(BUF, MAXLEN, FILE) fgets2(BUF, MAXLEN, FILE)
#undef  feof                                    // necessary for (at least) Cygwin
#define feof(FILE) feof2(FILE)
#define fwrite(BUF, SIZE, NUM, FILE) fwrite2(BUF, SIZE, NUM, FILE)
#define fputc(CHAR, FILE) fputc2(CHAR, FILE)
#define ftell(FILE) ftell2(FILE)
#define rewind(FILE) rewind2(FILE)
#undef  popen
#define popen(COMMAND, MODE) popen2(COMMAND, MODE)
#undef  pclose
#define pclose(FILE) pclose2(FILE)
#define q_fsize(FILENAME) q_fsize2(FILENAME)

// Returns the number of files in the "central dir of this disk" or -1 if
//  filename is not a ZIP file or an error occured.
extern int unzip_get_number_entries (const char *filename);
extern int unzip_goto_file (unzFile file, int file_index);
extern int unzip_current_file_nr;
#endif

#ifdef  __cplusplus
}
#endif

#endif // MISC_Z_H
