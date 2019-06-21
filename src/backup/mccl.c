/*
mccl.c - Mad Catz Camera Link (Game Boy Camera) support for uCON64

Copyright (c) 2002                    NoisyB
Copyright (c) 2004, 2015, 2017 - 2018 dbjh


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

/*
This cable is made by Mad Catz, Inc. and has a Game Boy link connector on one
end and a parallel port connector on the other. It is designed to interface
with the Game Boy Camera cart and comes with included software for this. It
works by simulating the Game Boy Printer with a PIC chip inside the parallel
connector shell. It doesn't do a particularly good job at that so it pretty
much only works with the Game Boy Camera.

Mad Catz Camera Link Communications Protocol

Printer I/O ports:
Base + 0: data register
Base + 1: status register
Base + 2: control register

Reset procedure:
1. Output 0x24 to control register (tristate data and set control register to 0100).
2. Wait for bit 5 of status register to become 1.
3. Read lower 4 bits of data register.
4. (Useless read of status register?)
5. If read data != 4, then go to step 1.
6. Output 0x22 to control register (tristate data and set control register to 0010).
7. Wait for bit 5 of status register to become 0.
8. Output 0x26 to control register (tristate data and set control register to 0110).

Data read procedure:
1. Output 0x26 to control register (tristate data and set control register to 0110).
2. Wait for bit 5 of status register to become 1.
3. Read lower 4 bits of data register, store to lower 4 bits of received byte.
4. (Useless read of status register?)
5. Output 0x22 to control register (tristate data and set control register to 0010).
6. Wait for bit 5 of status register to become 0.
7. Output 0x26 to control register (tristate data and set control register to 0110).
8. Wait for bit 5 of status register to become 1.
9. Read lower 4 bits of data register, store to upper 4 bits of received byte.
10. (Useless read of status register?)
11. Output 0x22 to control register (tristate data and set control register to 0010).
12. Wait for bit 5 of status register to become 0.
13. Go to step 1.
*/
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <string.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "misc/archive.h"
#include "misc/file.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "backup/mccl.h"


#ifdef  USE_PARALLEL
static st_ucon64_obj_t mccl_obj[] =
  {
    {UCON64_GB, WF_STOP | WF_NO_ROM}
  };
#endif

const st_getopt2_t mccl_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Mad Catz Camera Link (Game Boy Camera)"/*"XXXX Mad Catz Inc. http://www.madcatz.com"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xmccl", 0, 0, UCON64_XMCCL,
      NULL, "receives from Mad Catz Camera Link; " OPTION_LONG_S "port" OPTARG_S "PORT",
      &mccl_obj[0]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef USE_PARALLEL

#define GPDATASIZE 0x1760
#define DATA ((unsigned short) (parport + PARPORT_DATA))
#define STATUS ((unsigned short) (parport + PARPORT_STATUS))
#define CONTROL ((unsigned short) (parport + PARPORT_CONTROL))


static inline unsigned short
inportw2 (unsigned short port)
{
#ifdef  USE_PPDEV
  // Using the ppdev implementation of inportw() will certainly not work here.
  //  This should work though.
  unsigned short word = inportb (port);
  word |= inportb (port + 1) << 8;
  return word;
#else
  return inportw (port);
#endif
}


int
mccl_read (const char *filename, unsigned int parport)
{
  unsigned char buffer[GPDATASIZE];
  char dest_name[FILENAME_MAX];
  int count = 0;
  time_t starttime;

  parport_print_info ();
  puts ("Resetting device");
  do
    {
      outportb (CONTROL, 0x24);
      while ((inportb (STATUS) & 0x20) == 0)
        ;
    }
  while ((inportw2 (DATA) & 0xf) != 4);
  outportb (CONTROL, 0x22);
  while ((inportb (STATUS) & 0x20) != 0)
    ;
  outportb (CONTROL, 0x26);

  printf ("Receive: %d Bytes (%.4f Mb)\n\n", GPDATASIZE,
          (float) GPDATASIZE / MBIT);
  starttime = time (NULL);
  do
    {
      unsigned char inbyte;

      outportb (CONTROL, 0x26);
      while ((inportb (STATUS) & 0x20) == 0)
        ;
      inbyte = (unsigned char) (inportw2 (DATA) & 0xf);
      outportb (CONTROL, 0x22);
      while ((inportb (STATUS) & 0x20) != 0)
        ;
      outportb (CONTROL, 0x26);
      while ((inportb (STATUS) & 0x20) == 0)
        ;
      inbyte |= (unsigned char) ((inportw2 (DATA) & 0xf) << 4);
      outportb (CONTROL, 0x22);
      while ((inportb (STATUS) & 0x20) != 0)
        ;
      buffer[count++] = inbyte;
      if ((count & 0x1f) == 0)
        ucon64_gauge (starttime, count, GPDATASIZE);
    }
  while (count < GPDATASIZE);

  strcpy (dest_name, filename);
  ucon64_file_handler (dest_name, NULL, 0);
  ucon64_fwrite (buffer, 0, count, dest_name, "wb");
  printf ("\n\nYou can convert %s to BMP with " OPTION_LONG_S "gp2bmp\n", dest_name);
  return 0;
}

#endif // USE_PARALLEL
