/*
dxe_misc.c - miscellaneous functions for the grand libdiscmage DXE hack

written by 2003 dbjh


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

/*
  The original reason this file was created was because we can do the
  (re)definition of the names of the buffered file I/O functions only once.
  For a DXE they would have to be (re)defined twice if we want to be able to
  use the zlib & unzip code in miscz.c; once to substitute the names to make
  code use the import/export "table" and once to make code use the f*2()
  functions in miscz.c.
*/

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "dxedll_pub.h"

extern st_symbol_t import_export;
int errno = 0; // TODO: verify how dangerous this is (is it?)


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


long
ftell (FILE *file)
{
  return import_export.ftell (file);
}


void
rewind (FILE *file)
{
  return import_export.rewind (file);
}


// The functions below are only necessary if zlib support is enabled. They are
//  used by zlib and/or unzip.c.
void
free (void *mem)
{
  return import_export.free (mem);
}


void *
malloc (size_t size)
{
  return import_export.malloc (size);
}


void *
calloc (size_t n_elements, size_t size)
{
  return import_export.calloc (n_elements, size);
}


void *
memcpy (void *dest, const void *src, size_t size)
{
  return import_export.memcpy (dest, src, size);
}


void *
memset (void *mem, int value, size_t size)
{
  return import_export.memset (mem, value, size);
}


int
strcmp (const char *s1, const char *s2)
{
  return import_export.strcmp (s1, s2);
}


char *
strcpy (char *dest, const char *src)
{
  return import_export.strcpy (dest, src);
}


char *
strcat (char *s1, const char *s2)
{
  return import_export.strcat (s1, s2);
}


int
fprintf (FILE *file, const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = import_export.vfprintf (file, format, argptr);
  va_end (argptr);
  return n_chars;
}


int
sprintf (char *buffer, const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = import_export.vsprintf (buffer, format, argptr);
  va_end (argptr);
  return n_chars;
}


int
vsprintf (char *buffer, const char *format, va_list argptr)
{
  return import_export.vsprintf (buffer, format, argptr);
}


int
fflush (FILE *file)
{
  return import_export.fflush (file);
}


int
ferror (FILE *file)
{
  return import_export.ferror (file);
}


FILE *
fdopen (int fd, const char *mode)
{
  return import_export.fdopen (fd, mode);
}


int
mkdir (const char *path, mode_t mode)
{
  return import_export.mkdir (path, mode);
}


int
getuid (void)
{
  return import_export.getuid ();
}
