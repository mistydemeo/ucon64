/*
codec_zip.h - g(un)zip and unzip support

Copyright (c) 2001 - 2003 dbjh


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
#ifndef MISC_ARCHIVE_H
#define MISC_ARCHIVE_H
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef  USE_ZLIB
#include <stdio.h>
#include <zlib.h>
#include "unzip.h"
// Returns the number of files in the "central dir of this disk" or -1 if
//  filename is not a ZIP file or an error occured.
extern int unzip_get_number_entries (const char *filename);
extern int unzip_goto_file (unzFile file, int file_index);
extern int unzip_current_file_nr;

extern int unzip_inflate (FILE *source, FILE *dest);

#endif // USE_ZLIB

#ifdef  __cplusplus
}
#endif
#endif // MISC_ARCHIVE_H
