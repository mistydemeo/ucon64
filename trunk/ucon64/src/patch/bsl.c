/*
bsl.c - Baseline patcher support for uCON64

written by ???? - ???? The White Knight
           1999 - 2001 NoisyB (noisyb@gmx.net)


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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "bsl.h"


const char *bsl_usage[] =
  {
    NULL,
    NULL,
    "  " OPTION_S "b=PATCH     apply Baseline/BSL PATCH to ROM\n",
    NULL
  };


int
bsl (const char *modname, const char *bslname)
{
  FILE *modfile, *bslfile;
  unsigned char byte, addstr[10], datstr[10], buf[4096];
  int dat, numdat, i, done = 0, add;

  if (!(modfile = fopen (modname, "r+b")))
    return -1;
  if (!(bslfile = fopen (bslname, "rb")))
    return -1;

  printf ("Applying BSL/Baseline patch...\n");

  while (!done)
    {
      memset (addstr, ' ', sizeof (addstr));
      fscanf (bslfile, "%[-1234567890]\n", addstr);
      byte = fgetc (bslfile);
      add = atoi (addstr);

      memset (datstr, ' ', sizeof (datstr));
      fscanf (bslfile, "%[-1234567890]\n", datstr);
      byte = fgetc (bslfile);
      dat = atoi (datstr);

      if ((add == -1) && (dat == -1))
        done = 1;
      else
        {
          fseek (modfile, add, SEEK_SET);
          fputc (dat, modfile);
        }
    }

  memset (addstr, ' ', sizeof (addstr));
  fscanf (bslfile, "%[-1234567890]\n", addstr);
  byte = fgetc (bslfile);
  add = atoi (addstr);

  memset (datstr, ' ', sizeof (datstr));
  fscanf (bslfile, "%[-1234567890]\n", datstr);
  byte = fgetc (bslfile);
  numdat = atoi (datstr);

  fseek (modfile, add, SEEK_SET);

  if (numdat > 0)
    {
      while (numdat > 4096)
        {
          fread (buf, 4096, 1, bslfile);
          fwrite (buf, 4096, 1, modfile);
          numdat -= 4096;
        }
      for (i = 0; i <= numdat; i++)
        {
          byte = fgetc (bslfile);
          fputc (byte, modfile);
        }
    }

  printf ("Patching complete\n"
          "NOTE: Sometimes you have to add/strip a 512 bytes header when you patch a ROM\n"
          "      This means you must convert for example a SNES ROM with -swc or -mgd or\n"
          "      the patch will not work\n");
  fclose (bslfile);
  fclose (modfile);
  return 0;
}

// TODO: make bsl patch
