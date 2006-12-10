/*
patch.c - patch support for uCON64

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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc/itypes.h"
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/file.h"
#include "misc/misc.h"
#include "ucon64_misc.h"
#include "patch.h"


static st_ucon64_obj_t patch_obj[] =
  {
    {0, WF_SWITCH},
    {0, WF_INIT | WF_PROBE}
  };

const st_getopt2_t patch_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Patching",
      NULL
    },
    {
      "poke", 1, 0, UCON64_POKE,
      "OFF:V", "change byte at file offset OFF to value V (both in hexadecimal)",
      NULL
    },
    {
      "pattern", 1, 0, UCON64_PATTERN,
      "FILE", "change ROM based on patterns specified in FILE",
      &patch_obj[1]
    },
    {
      "patch", 1, 0, UCON64_PATCH,
      "PATCH", "specify the PATCH for the following options\n"
      "use this option or uCON64 expects the last commandline\n"
      "argument to be the name of the PATCH file",
      &patch_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#warning TEST
int
patch_poke (st_ucon64_t *p)
{
  int value = 0, x = 0;
  char buf[MAXBUFSIZE], src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  const char *optarg = p->optarg;

  if (ucon64.fname)
    {
      strcpy (src_name, ucon64.fname);
      strcpy (dest_name, ucon64.fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, 0, ucon64.file_size, dest_name, "wb");

  sscanf (optarg, "%x:%x", &x, &value);
  if (x >= ucon64.file_size)
    {
      fprintf (stderr, "ERROR: Offset 0x%x is too large\n", x);
      remove (dest_name);
      return -1;
    }
  fputc ('\n', stdout);
  buf[0] = ucon64_fgetc (dest_name, x);
  dumper (stdout, buf, 1, x, DUMPER_HEX);

  ucon64_fputc (dest_name, x, value, "r+b");

  buf[0] = value;
  dumper (stdout, buf, 1, x, DUMPER_HEX);
  fputc ('\n', stdout);

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}
