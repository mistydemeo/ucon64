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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "misc.h"
#include "ucon64.h"
#include "bsl.h"

int
bsl (char *name, char *option2)
{
  FILE *fh;
  FILE *fh2;
  char inchar[1], addstr[10], datstr[10], inchar2[4096];
  long numdat, i;
  long done = 0, add;
  int dat;

  if (!(fh = fopen (name, "r+b")))
    return -1;
  if (!(fh2 = fopen (option2, "rb")))
    return -1;


  printf ("BSL/Baseline\n");
  printf ("%ld (%.4f Mb)\n", quickftell (option2),
          (float) quickftell (option2) / MBIT);
  printf ("\n"
          "NOTE: sometimes you have to add/strip a 512 bytes header when you patch a ROM\n"
          "      This means you must convert for example a Super Nintendo ROM with -swc\n"
          "      or -mgd or the patch will not work\n");

//      printf("Internal Size: %.4f Mb\n",fsize(romname,8));
//      printf("Version: 1.%d\n","?");
//      printf("\n");

  while (!done)
    {
      memset (addstr, ' ', sizeof (addstr));
      fscanf (fh2, "%[-1234567890]\n", addstr);
      fread (inchar, sizeof (inchar), 1, fh2);

      add = atol (addstr);

      memset (datstr, ' ', sizeof (datstr));
      fscanf (fh2, "%[-1234567890]\n", addstr);
      fread (inchar, sizeof (inchar), 1, fh2);

      dat = atoi (datstr);
      inchar[0] = dat;

      if ((add == -1) && (dat == -1))
        done = 1;

      if (done == 0)
        {
//                printf("(Offset:  %lX)\n", add);

          fseek (fh, add, SEEK_SET);
          fwrite (inchar, sizeof (inchar), 1, fh);
        }
    }

  memset (addstr, ' ', sizeof (addstr));
  fscanf (fh2, "%[-1234567890]\n", addstr);
  fread (inchar, sizeof (inchar), 1, fh2);

  add = atol (addstr);

  memset (datstr, ' ', sizeof (datstr));
  fscanf (fh2, "%[-1234567890]\n", addstr);
  fread (inchar, sizeof (inchar), 1, fh2);

  numdat = atol (datstr);
  fseek (fh, add, SEEK_SET);

  if (numdat > 0)
    {
      while (numdat > 4096)
        {
//                        printf("(Offset:  %lX)\n", add);
          fread (inchar2, sizeof (inchar2), 1, fh2);
          fwrite (inchar2, sizeof (inchar2), 1, fh);
          numdat = numdat - 4096;
          add = add + 4096;
        }
      for (i = 0; i < (numdat + 1); i++)
        {
//                        printf("(Offset:  %lX)\n", (add+i));
          fread (inchar, sizeof (inchar), 1, fh2);
          fwrite (inchar, sizeof (inchar), 1, fh);
        }
    }

  fclose (fh2);
  fclose (fh);
  return 0;
}


int
bsl_usage (int argc, char *argv[])
{
  printf ("  -b            apply Baseline/BSL patch (<=x.x); $FILE=PATCHFILE\n");
  return 0;
}

//TODO make bsl patch
