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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "misc.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "quick_io.h"
#include "dex.h"


const st_usage_t dex_usage[] =
  {
    {NULL, NULL, "DexDrive"},
    {NULL, NULL, "19XX InterAct http://www.dexdrive.de"},
    {"xdex", "N", "send/receive Block N to/from DexDrive; " OPTION_LONG_S "port=PORT\n"
                 "receives automatically when SRAM does not exist"},
    {NULL, NULL, NULL}
  };


#ifdef PARALLEL
#include "dex.h"
#include "psxpblib.h"

static int print_data = 0x378;

#define CONPORT 1
#define TAP 1
#define DELAY 4

static unsigned char *
read_block (int block_num, unsigned char *data)
{
  data = psx_memcard_read_block (print_data, CONPORT, TAP, DELAY, block_num);
  return data;
}

static int
write_block (int block_num, unsigned char *data)
{
  return psx_memcard_write_block (print_data, CONPORT, TAP, DELAY, block_num,
                                  data);
}

#if 0
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
#endif


/*
  It will save you some work if you don't fully integrate the code above with uCON64's code,
  because it is a project separate from the uCON64 project.
*/
int dex_argc;
char *dex_argv[128];
#define FRAME_SIZE 128
#define BLOCK_SIZE (64*(FRAME_SIZE))

int
dex_read_block (const char *filename, int block_num, unsigned int parport)
{
  unsigned char *result = NULL, data[BLOCK_SIZE];
  print_data = parport;

  result = read_block (block_num, data);

  q_fwrite (data, 0, BLOCK_SIZE, filename, "wb");

  return result == NULL ? (-1) : 0;
}


int
dex_write_block (const char *filename, int block_num, unsigned int parport)
{
  int result;
  unsigned char data[BLOCK_SIZE];
  print_data = parport;

  q_fread (data, 0, BLOCK_SIZE, filename);

  result = write_block (block_num, data);

  return result == (-1) ? (-1) : 0;
}

#undef FRAME_SIZE
#undef BLOCK_SIZE

#endif // PARALLEL
