/*
file.h - miscellaneous file functions

Copyright (c) 1999 - 2004 NoisyB
Copyright (c) 2001 - 2004 dbjh
Copyright (c) 2002 - 2004 Jan-Erik Karlsson (Amiga)


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
#ifndef MISC_FILE_H
#define MISC_FILE_H
#include "defines.h"


/*
  Miscellaneous file operations

  isfname()   test if char could be used for filenames
  tofname()   replaces char that can not be used for filenames
  set_suffix() set/replace suffix of filename with suffix
                suffix means in this case the suffix INCLUDING the dot '.'
  get_suffix() get suffix of filename
  basename2() basename() replacement
  dirname2()  dirname() replacement
  realpath2() realpath() replacement
  same_file() returns 1 if two filenames refer to one file, otherwise it
                returns 0
  same_filesystem() returns 1 if two filenames refer to files on one file
                system, otherwise it returns 0
  rename2()   renames oldname to newname even if oldname and newname are not
                on one file system
  truncate2() don't use truncate() to enlarge files, because the result is
                undefined (by POSIX) use truncate2() instead which does both
  tmpnam2()   replacement for tmpnam() temp must have the size of FILENAME_MAX
  baknam()    produces a backup name for a filename
                bla.txt would return bla.bak or bla.b01 if bla.bak already exists
  fcopy()     copy src from start for len to dest with mode
  fsizeof()   returns size of a file in bytes

  quick_io()   returns number of bytes read or written
  quick_io_c() returns byte read or fputc()'s status

  quick_io_func()
              malloc()s a buffer as large as possible and starts reading
              from fname into the buffer and passes chunks of it to func
                func()      func(buffer, buffer_len, object)
                func_maxlen max. size of those chunks the func can process at once
                object      a freely defineavble object the will also passed to func
                start       seeks to start pos of fname
                len         a vector from start

  getfile()           runs callback_func with the realpath() of file/dir as string
                        flags:
  0                           pass all files/dirs with their realpath()
  GETFILE_FILES_ONLY     pass only files with their realpath()
  GETFILE_RECURSIVE      pass all files/dirs with their realpath()'s recursively
  GETFILE_RECURSIVE_ONCE like GETOPT2_FILE_RECURSIVE, but only one level deep
  (GETFILE_FILES_ONLY|GETFILE_RECURSIVE)
                           pass only files with their realpath()'s recursively

  callback_func()       getfile() expects the callback_func to return the following
                          values:
                          0 == ok, 1 == skip the rest/break, -1 == failure/break
*/
extern int isfname (int c);
extern int tofname (int c);
extern char *realpath2 (const char *path, char *full_path);
extern char *dirname2 (const char *path, char *dir);
extern const char *basename2 (const char *path);
extern const char *get_suffix (const char *filename);
extern char *set_suffix (char *filename, const char *suffix);
extern int mkdir2 (const char *path);
extern int rmdir2 (const char *path);
extern int same_file (const char *filename1, const char *filename2);
extern int same_filesystem (const char *filename1, const char *filename2);
extern int rename2 (const char *oldname, const char *newname);
extern int truncate2 (const char *filename, unsigned long size);
extern char *tmpnam2 (char *temp);
extern char *baknam (char *fname);
extern int fsizeof (const char *filename);
extern int fcopy (const char *src, size_t start, size_t len, const char *dest,
                  const char *dest_mode);

extern int quick_io (void *buffer, size_t start, size_t len, const char *fname,
                     const char *mode);
extern int quick_io_c (int value, size_t pos, const char *fname, const char *mode);

extern int quick_io_func (int (*func) (void *, int, void *),
                          int func_maxlen, void *object, size_t start,
                          size_t len, const char *fname);

#define GETFILE_FILES_ONLY     1
#define GETFILE_RECURSIVE      (1 << 1)
#define GETFILE_RECURSIVE_ONCE (1 << 2)
extern int getfile (int argc, char **argv, int (*callback_func) (const char *), int flags);


#endif // MISC_FILE_H
