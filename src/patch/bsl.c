/*
bsl.c - Baseline patcher support for uCON64

Copyright (c) ???? - ???? The White Knight
Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2003, 2019  dbjh


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
#include <string.h>
#include "misc/archive.h"
#include "misc/file.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "patch/bsl.h"


static st_ucon64_obj_t bsl_obj[] =
  {
    {0, WF_STOP}
  };

const st_getopt2_t bsl_usage[] =
  {
    {
      "b", 0, 0, UCON64_B,
      NULL, "apply Baseline/BSL PATCH to ROM",
      &bsl_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


int
bsl_apply (const char *mod, const char *bslname)
{
  FILE *modfile, *bslfile;
  char modname[FILENAME_MAX];
  int data, nbytes, offset;

  strcpy (modname, mod);
  ucon64_file_handler (modname, NULL, 0);
  fcopy (mod, 0, fsizeof (mod), modname, "wb"); // no copy if one file

  if ((modfile = fopen (modname, "r+b")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], modname);
      return -1;
    }
  if ((bslfile = fopen (bslname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], bslname);
      fclose (modfile);
      return -1;
    }

  puts ("Applying BSL/Baseline patch...");

  while (!feof (bslfile))
    {
      if (fscanf (bslfile, "%d\n", &offset) != 1 ||
          fscanf (bslfile, "%d\n", &data) != 1)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], bslname);
          fclose (bslfile);
          fclose (modfile);
          return -1;
        }
      if (offset == -1 && data == -1)
        break;

      fseek (modfile, offset, SEEK_SET);
      fputc (data, modfile);
    }

  if (fscanf (bslfile, "%d\n", &offset) != 1 ||
      fscanf (bslfile, "%d\n", &nbytes) != 1)
    {
      fprintf (stderr, ucon64_msg[READ_ERROR], bslname);
      fclose (bslfile);
      fclose (modfile);
      return -1;
    }
  fseek (modfile, offset, SEEK_SET);
  if (nbytes > 0)
    {
      char buf[4096];

      while (nbytes > (int) sizeof buf)
        {
          size_t nbytes2 = fread (buf, 1, sizeof buf, bslfile);
          nbytes -= nbytes2;
          if (nbytes2 == 0)
            {
              nbytes = -1;
              break;
            }
          fwrite (buf, 1, nbytes2, modfile);
        }
      while (nbytes-- >= 0)                     // yes, one byte more than the
        {                                       //  _value_ read from the BSL file
          if ((data = fgetc (bslfile)) == EOF)
            break;
          fputc (data, modfile);
        }
    }

  puts ("Patching complete\n");
  printf (ucon64_msg[WROTE], modname);
  puts ("\n"
        "NOTE: Sometimes you have to add/strip a 512 bytes header when you patch a ROM\n"
        "      This means you must modify for example a SNES ROM with -swc or -stp or\n"
        "      the patch will not work");

  fclose (bslfile);
  fclose (modfile);

  return 0;
}
