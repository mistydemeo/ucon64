/*
dxedll_priv.h - DXE support (code/data private to DXE)

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

#ifndef DXEDLL_PRIV_H
#define DXEDLL_PRIV_H

#include "dxedll_pub.h"

#ifdef __cplusplus
extern "C" {
#endif

st_symbol_t import_export;

#define printf import_export.printf
#define fprintf import_export.fprintf
#define sprintf import_export.sprintf
#define fputs import_export.fputs

#ifndef HAVE_ZLIB_H
#define fopen import_export.fopen
#define fclose import_export.fclose
#define fseek import_export.fseek
#define fread import_export.fread
#define fgetc import_export.fgetc
#define fgets import_export.fgets
#define feof import_export.feof
#define fwrite import_export.fwrite
#define fputc import_export.fputc
#else
#error currently libdiscmage must be compiled without zlib support
#define fopen2 import_export.fopen
#define fclose2 import_export.fclose
#define fseek2 import_export.fseek
#define fread2 import_export.fread
#define fgetc2 import_export.fgetc
#define fgets2 import_export.fgets
#define feof2 import_export.feof
#define fwrite2 import_export.fwrite
#define fputc2 import_export.fputc
#endif

#define ftell import_export.ftell
#define fflush import_export.fflush
#define rename import_export.rename

#define free import_export.free
#define malloc import_export.malloc
#define exit import_export.exit
#define strtol import_export.strtol
#define getenv import_export.getenv
#define srand import_export.srand
#define rand import_export.rand

#define memcpy import_export.memcpy
#define memset import_export.memset
#define strcmp import_export.strcmp
#define strcpy import_export.strcpy
#define strcat import_export.strcat
#define strncat import_export.strncat
#define strcasecmp import_export.strcasecmp
#define strncasecmp import_export.strncasecmp
#define strchr import_export.strchr
#define strrchr import_export.strrchr
#define strpbrk import_export.strpbrk
#define strspn import_export.strspn
#define strcspn import_export.strcspn

// We have to do this, because there's also a struct stat
// TODO?: do this for all #defines in this file.
#define stat(FILE, STATBUF) import_export.stat(FILE, STATBUF)
#define time import_export.time
#define delay import_export.delay
#define __dpmi_int import_export.__dpmi_int
#undef  tolower
#define tolower import_export.tolower

#define opendir import_export.opendir
#define readdir import_export.readdir
#define closedir import_export.closedir

#define access import_export.access
#define rmdir import_export.rmdir
#define isatty import_export.isatty
#define chdir import_export.chdir
#define getcwd import_export.getcwd

#define __dj_stdin import_export.__dj_stdin
#define __dj_stdout import_export.__dj_stdout
#define __dj_stderr import_export.__dj_stderr
#define __dj_ctype_flags import_export.__dj_ctype_flags
#define __dj_ctype_tolower import_export.__dj_ctype_tolower
#define __dj_ctype_toupper import_export.__dj_ctype_toupper
#define errno import_export.errno

#ifdef __cplusplus
}
#endif

#endif // DXEDLL_PRIV_H
