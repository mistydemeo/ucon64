/*
dxedll_priv.h - DXE support (code/data private to DXE)

written by 2002 - 2003 dbjh


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef DXEDLL_PRIV_H
#define DXEDLL_PRIV_H

#include "dxedll_pub.h"

#ifdef __cplusplus
extern "C" {
#endif

extern st_symbol_t import_export;

#define printf import_export.printf
#define fprintf import_export.fprintf
#define vfprintf import_export.vfprintf
#define sprintf import_export.sprintf
#define vsprintf import_export.vsprintf
#define fputs import_export.fputs

#if 0
// These are now defined in dxe_misc.c
#define fopen import_export.fopen
#define fclose import_export.fclose
#define popen import_export.popen
#define pclose import_export.pclose
#define fseek import_export.fseek
#define fread import_export.fread
#define fgetc import_export.fgetc
#define fgets import_export.fgets
#define feof import_export.feof
#define fwrite import_export.fwrite
#define fputc import_export.fputc
#define ftell import_export.ftell
#define rewind import_export.rewind
#endif

#define fdopen import_export.fdopen
#define fflush import_export.fflush
#define ferror import_export.ferror
#define rename import_export.rename
#define remove import_export.remove

#define free import_export.free
#define malloc import_export.malloc
#define calloc import_export.calloc
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
#define strlen import_export.strlen
#define strstr import_export.strstr

#undef  tolower
#define tolower import_export.tolower
#undef  toupper
#define toupper import_export.toupper
#undef  isupper
#define isupper import_export.isupper

#define opendir import_export.opendir
#define readdir import_export.readdir
#define closedir import_export.closedir

#define access import_export.access
#define rmdir import_export.rmdir
#define isatty import_export.isatty
#define chdir import_export.chdir
#define getcwd import_export.getcwd
#define getuid import_export.getuid
#define sync import_export.sync
#define truncate import_export.truncate

// We have to do this, because there's also a struct stat
// TODO?: do this for all #defines in this file.
#define stat(FILE, STATBUF) import_export.stat(FILE, STATBUF)
#define chmod import_export.chmod
#define mkdir import_export.mkdir
#define time import_export.time
#define delay import_export.delay
#define __dpmi_int import_export.__dpmi_int

// zlib functions
#if 0
#define gzopen import_export.gzopen
#define gzclose import_export.gzclose
#define gzwrite import_export.gzwrite
#define gzgets import_export.gzgets
#define gzeof import_export.gzeof
#define gzseek import_export.gzseek
#define gzputc import_export.gzputc
#define gzread import_export.gzread
#define gzgetc import_export.gzgetc
#define gzrewind import_export.gzrewind
#define gztell import_export.gztell

// unzip functions
#define unzOpen import_export.unzOpen
#define unzOpenCurrentFile import_export.unzOpenCurrentFile
#define unzGoToFirstFile import_export.unzGoToFirstFile
#define unzClose import_export.unzClose
#define unzGetGlobalInfo import_export.unzGetGlobalInfo
#define unzGoToNextFile import_export.unzGoToNextFile
#define unzCloseCurrentFile import_export.unzCloseCurrentFile
#define unzeof import_export.unzeof
#define unzReadCurrentFile import_export.unzReadCurrentFile
#define unztell import_export.unztell
#define unzGetCurrentFileInfo import_export.unzGetCurrentFileInfo
#endif

// variables
#define __dj_stdin import_export.__dj_stdin
#define __dj_stdout import_export.__dj_stdout
#define __dj_stderr import_export.__dj_stderr
#define __dj_ctype_flags import_export.__dj_ctype_flags
#define __dj_ctype_tolower import_export.__dj_ctype_tolower
#define __dj_ctype_toupper import_export.__dj_ctype_toupper
//#define errno import_export.errno

#ifdef __cplusplus
}
#endif

#endif // DXEDLL_PRIV_H
