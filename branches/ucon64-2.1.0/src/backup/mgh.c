/*
mgh.c - Multi Game Hunter support for uCON64

Copyright (c) 2017 dbjh


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
#include <stdio.h>
#include "misc/file.h"
#include "misc/string.h"
#include "ucon64_defines.h"
#include "backup/mgh.h"


void
mgh_make_name (const char *filename, int console, unsigned int size, char *name)
{
  char *p, suffix[5];
  const char *fname;
  unsigned int n;

  fname = basename2 (filename);
  sprintf (name, "%s%.6s", console == UCON64_SNES ? "SF" : "MD", fname);
  if (!strnicmp (name, fname, 2))
    strncpy (name, fname, 8)[8] = '\0';
  if ((p = strchr (name, '.')) != NULL)
    *p = '\0';

  strupr (name);
  n = strlen (name);
  if (n == 8 && name[7] == 'A')                 // 'A' indicates first part of split file
    name[7] = '\0';

  for (n--; n >= 2; n--)                        // skip prefix
    if (name[n] == ' ' || name[n] == '.')
      name[n] = '_';

  sprintf (suffix, ".%03u", size / MBIT);
  set_suffix (name, suffix);
}
