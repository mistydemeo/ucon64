/*
dex.c - DexDrive support for uCON64

written by 2002 NoisyB (noisyb@gmx.net)


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
#include "config.h"
#include "misc.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"

const char *dex_usage[] =
  {
    "DexDrive",
    "19XX InterAct http://www.dexdrive.de",
    "TODO: " OPTION_LONG_S "xdex    send/receive SRAM to/from DexDrive; " OPTION_LONG_S "file=PORT\n"
    "                  receives automatically when " OPTION_LONG_S "rom(=SRAM) does not exist\n",
    NULL
  };


#ifdef BACKUP
#include "dex.h"
#include "psxpblib.h"

static int print_data = 0x378;

#define CONPORT 1
#define TAP 1
#define DELAY 4


char *
read_block (int block_num, char *data)
{
  data = psx_memcard_read_block (print_data, CONPORT, TAP, DELAY, block_num);
  return data;
}

int
write_block (int block_num, char *data)
{
  return psx_memcard_write_block (print_data, CONPORT, TAP, DELAY, block_num,
                                  data);
}

char *
read_frame (int frame, char *data)
{
  data = psx_memcard_read_frame (print_data, CONPORT, TAP, DELAY, frame);
  return data;
}

int
write_frame (int frame, char *data)
{
  return psx_memcard_write_frame (print_data, CONPORT, TAP, DELAY, frame,
                                  data);
}

/*
  It will save you some work if you don't fully integrate the code above with uCON64's code,
  because it is a project separate from the uCON64 project.
*/
int dex_argc;
char *dex_argv[128];


int
dex_read_rom (const char *filename, unsigned int parport)
{
  print_data = parport;
#if 0
  dex_argv[0] = "ucon64";
  dex_argv[1] = "READ";
  strcpy (buf, filename);
  dex_argv[2] = buf;
  dex_argc = 3;

  return dex_main (dex_argc, dex_argv);
#endif  
  return 0;
}


int
dex_write_rom (const char *filename, unsigned int parport)
{
  print_data = parport;
#if 0
  dex_argv[0] = "ucon64";
  dex_argv[1] = "WRITE";
  strcpy (buf, filename);
  dex_argv[2] = buf;
  dex_argc = 3;

  return dex_main (dex_argc, dex_argv);
#endif
  return 0;
}

#endif // BACKUP
