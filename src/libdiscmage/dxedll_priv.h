/*
dxedll_priv.h - DXE support (code/data private to DXE)

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
#define fopen import_export.fopen
#define fclose import_export.fclose
#define fseek import_export.fseek
#define ftell import_export.ftell
#define fread import_export.fread
#define fwrite import_export.fwrite
#define fflush import_export.fflush

#define free import_export.free
#define malloc import_export.malloc
#define exit import_export.exit
#define strtol import_export.strtol

#define memcpy import_export.memcpy
#define memset import_export.memset
#define strcmp import_export.strcmp
#define strcpy import_export.strcpy
#define strcat import_export.strcat
#define strcasecmp import_export.strcasecmp
#define strncasecmp import_export.strncasecmp
#define strrchr import_export.strrchr

// We have to do this, because there's also a struct stat
// TODO?: do this for all #defines in this file.
#define stat(FILE, STATBUF) import_export.stat(FILE, STATBUF)

#define time import_export.time

#define __dj_stdin import_export.__dj_stdin
#define __dj_stdout import_export.__dj_stdout
#define __dj_stderr import_export.__dj_stderr

#ifdef __cplusplus
}
#endif

#endif // DXEDLL_PRIV_H
