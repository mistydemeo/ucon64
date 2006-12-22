/*
win32.c - win32 compat. stuff

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
#include "win32.h"
#include <errno.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>

struct DIR
{
  long handle;                  /* -1 for failed rewind */
  struct _finddata_t info;
  struct dirent result;         /* d_name null iff first time */
  char *name;                   /* NTBS */
};


DIR *
opendir (const char *name)
{
  DIR *dir = 0;

  if (name && name[0])
    {
      size_t base_length = strlen (name);
      const char *all =         /* the root directory is a special case... */
        strchr ("/\\", name[base_length - 1]) ? "*" : "/*";

      if ((dir = (DIR *) malloc (sizeof *dir)) != 0 &&
          (dir->name = (char *) malloc (base_length + strlen (all) + 1)) != 0)
        {
          strcat (strcpy (dir->name, name), all);

          if ((dir->handle = (long) _findfirst (dir->name, &dir->info)) != -1)
            {
              dir->result.d_name = 0;
            }
          else                  /* rollback */
            {
              free (dir->name);
              free (dir);
              dir = 0;
            }
        }
      else                      /* rollback */
        {
          free (dir);
          dir = 0;
          errno = ENOMEM;
        }
    }
  else
    {
      errno = EINVAL;
    }

  return dir;
}


int
closedir (DIR * dir)
{
  int result = -1;

  if (dir)
    {
      if (dir->handle != -1)
        {
          result = _findclose (dir->handle);
        }

      free (dir->name);
      free (dir);
    }

  if (result == -1)             /* map all errors to EBADF */
    {
      errno = EBADF;
    }

  return result;
}


struct dirent *
readdir (DIR * dir)
{
  struct dirent *result = 0;

  if (dir && dir->handle != -1)
    {
      if (!dir->result.d_name || _findnext (dir->handle, &dir->info) != -1)
        {
          result = &dir->result;
          result->d_name = dir->info.name;
        }
    }
  else
    {
      errno = EBADF;
    }

  return result;
}


void
rewinddir (DIR * dir)
{
  if (dir && dir->handle != -1)
    {
      _findclose (dir->handle);
      dir->handle = (long) _findfirst (dir->name, &dir->info);
      dir->result.d_name = 0;
    }
  else
    {
      errno = EBADF;
    }
}
