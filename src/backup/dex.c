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
#include "dex.h"
#include "psxpblib.h"

#ifdef BACKUP

#define BASE_ADDR 0x378
#define CONPORT 1
#define TAP 1
#define DELAY 4



char *
read_block (int block_num, char *data)
{
  data = psx_memcard_read_block (BASE_ADDR, CONPORT, TAP, DELAY, block_num);
  return data;
}

int
write_block (int block_num, char *data)
{
  return psx_memcard_write_block (BASE_ADDR, CONPORT, TAP, DELAY, block_num,
                                  data);
}

char *
read_frame (int frame, char *data)
{
  data = psx_memcard_read_frame (BASE_ADDR, CONPORT, TAP, DELAY, frame);
  return data;
}

int
write_frame (int frame, char *data)
{
  return psx_memcard_write_frame (BASE_ADDR, CONPORT, TAP, DELAY, frame,
                                  data);
}

int
get_perm ()
{
  return psx_obtain_io_permission (BASE_ADDR);
}

#endif // BACKUP

int
dex_usage (int argc, char *argv[])
{
#ifdef BACKUP
#if 0
    printf ("%s\n", dex_TITLE);


  printf
    ("TODO:  -xdex    send/receive SRAM to/from DexDrive; $FILE=PORT\n"
     "		receives automatically when $ROM(=SRAM) does not exist\n");
#endif
#endif // BACKUP
  return 0;
}
