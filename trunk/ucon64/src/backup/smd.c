/*
smd.c - Super Magic Drive support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh

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
#include <time.h>
#include "config.h"                             // config.h might define BACKUP
#ifdef  __unix__
#include <unistd.h>                             // usleep(), microseconds
#elif   defined __MSDOS__
#include <dos.h>                                // delay(), milliseconds
#elif   defined __BEOS__
#include <OS.h>                                 // snooze(), microseconds
#endif
#include "misc.h"                               // including misc.h after OS.h
#include "ucon64.h"                             //  avoids warnings about MIN & MAX
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "smd.h"
const char *smd_usage[] =
  {
    "Super Com Pro (HK)/Super Magic Drive/SMD",
    "19XX Front Far East/FFE http://www.front.com.tw",
#ifdef BACKUP
    "  " OPTION_LONG_S "xsmd        send/receive ROM to/from Super Magic Drive/SMD; " OPTION_LONG_S "file=PORT\n"
    "                  receives automatically when ROM does not exist\n"
    "  " OPTION_LONG_S "xsmds       send/receive SRAM to/from Super Magic Drive/SMD; " OPTION_LONG_S "file=PORT\n"
    "                  receives automatically when SRAM does not exist\n",
#endif // BACKUP
    NULL
};


#ifdef BACKUP
int
smd_read_rom (const char *filename, unsigned int parport)
{


  return 0;
}

int
smd_write_rom (const char *filename, unsigned int parport)
{


  return 0;
}

int
smd_read_sram (const char *filename, unsigned int parport)
{


  return 0;
}

int
smd_write_sram (const char *filename, unsigned int parport)
{


  return 0;
}
#endif // BACKUP
