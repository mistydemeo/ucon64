/*
dxe_misc.c - miscellaneous functions for the grand libdiscmage DXE hack

written by 2003 dbjh


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

/*
  The original reason this file was created was because we can do the
  (re)definition of the names of the buffered file I/O functions only once.
  For a DXE they would have to be (re)defined twice if we want to be able to
  use the zlib & unzip code in misc.c; once to substitute the names to make
  code use the import/export "table" and once to make code use the f*2()
  functions in misc.c.
*/

#include <stdio.h>
#include "dxedll_pub.h"

extern st_symbol_t import_export;


FILE *
fopen (const char *filename, const char *mode)
{
  return import_export.fopen (filename, mode);
}


int
fclose (FILE *file)
{
  return import_export.fclose (file);
}


int
fseek (FILE *file, long offset, int mode)
{
  return import_export.fseek (file, offset, mode);
}


size_t
fread (void *buffer, size_t size, size_t number, FILE *file)
{
  return import_export.fread (buffer, size, number, file);
}


int
fgetc (FILE *file)
{
  return import_export.fgetc (file);
}


char *
fgets (char *buffer, int maxlength, FILE *file)
{
  return import_export.fgets (buffer, maxlength, file);
}


int
feof (FILE *file)
{
  return import_export.feof (file);
}

     
size_t
fwrite (const void *buffer, size_t size, size_t number, FILE *file)
{
  return import_export.fwrite (buffer, size, number, file);
}


int
fputc (int character, FILE *file)
{
  return import_export.fputc (character, file);
}
