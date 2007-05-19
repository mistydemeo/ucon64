/*
win32.h - miscellaneous win32 functions

Copyright (c) 2006 NoisyB


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
#ifndef MISC_WIN32_H
#define MISC_WIN32_H
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DIR DIR;

struct dirent
{
  char *d_name;
};

DIR *opendir (const char *);
int closedir (DIR *);
struct dirent *readdir (DIR *);
void rewinddir (DIR *);


#ifdef __cplusplus
}
#endif
#endif
