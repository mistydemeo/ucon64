/*
ffe.c - General Front Far East copier routines for uCON64

written by 2002 dbjh


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
#include <time.h>
#include <string.h>
#include "config.h"
#include "misc.h"                               // kbhit(), getch()
#include "quick_io.h"
#include "ucon64_misc.h"
#include "ffe.h"


#ifdef BACKUP


#define INPUT_MASK      0x78
#define IBUSY_BIT       0x80
#define N_TRY_MAX       65536                   // # times to test if copier ready


static void ffe_sendb (unsigned char byte);
static unsigned char ffe_wait_while_busy (void);

int ffe_port;


void
ffe_init_io (unsigned int port)
/*
  - sets static global `ffe_port'. Then the send/receive functions don't need to pass `port' all
    the way to ffe_sendb()/ffe_receiveb().
  - calls init_conio(). Necessary for kbhit() and DOS-like behaviour of getch().
*/
{
  ffe_port = port;
  if (ffe_port != 0x3bc && ffe_port != 0x378 && ffe_port != 0x278)
    {
      fprintf (stderr, "ERROR: PORT must be 0x3bc, 0x378 or 0x278\n");
      exit (1);
    }

#if     defined __unix__ || defined __BEOS__
  init_conio ();
#endif

  printf ("Using I/O port 0x%x\n", ffe_port);
}


void
ffe_deinit_io (void)
{
#if     defined __unix__ || defined __BEOS__
  deinit_conio ();
#endif
}


void
ffe_send_block (unsigned short address, unsigned char *buffer, int len)
{
  int checksum = 0x81, n;

  ffe_send_command (0, address, len);
  for (n = 0; n < len; n++)
    {
      ffe_sendb (buffer[n]);
      checksum ^= buffer[n];
    }
  ffe_sendb (checksum);
}


void
ffe_send_command0 (unsigned short address, unsigned char byte)
// command 0 for 1 byte
{
  ffe_send_command (0, address, 1);
  ffe_sendb (byte);
  ffe_sendb (0x81 ^ byte);
}


void
ffe_send_command (unsigned char command_code, unsigned short a, unsigned short l)
{
  ffe_sendb (0xd5);
  ffe_sendb (0xaa);
  ffe_sendb (0x96);
  ffe_sendb (command_code);
  ffe_sendb (a);                                // low byte
  ffe_sendb (a >> 8);                           // high byte
  ffe_sendb (l);                                // low byte
  ffe_sendb (l >> 8);                           // high byte
  ffe_sendb (0x81 ^ command_code ^ a ^ (a >> 8) ^ l ^ (l >> 8)); // checksum
}


void
ffe_sendb (unsigned char byte)
{
  ffe_wait_for_ready ();
  outportb (ffe_port + PARPORT_DATA, byte);
  outportb (ffe_port + PARPORT_CONTROL, inportb (ffe_port + PARPORT_CONTROL) ^ STROBE_BIT); // invert strobe
  ffe_wait_for_ready ();                        // necessary if followed by ffe_receiveb()
}


void
ffe_receive_block (unsigned short address, unsigned char *buffer, int len)
{
  int checksum = 0x81, n, m;

  ffe_send_command (1, address, len);
  for (n = 0; n < len; n++)
    {
      buffer[n] = ffe_receiveb ();
      checksum ^= buffer[n];
    }
  if (checksum != ffe_receiveb ())
    printf ("\nreceived data is corrupt\n");

  for (m = 0; m < 65536; m++)                   // a delay is necessary here
    ;
}


unsigned char
ffe_receiveb (void)
{
  unsigned char byte;

  byte = (ffe_wait_while_busy () & INPUT_MASK) >> 3; // receive low nibble
  outportb (ffe_port + PARPORT_CONTROL, inportb (ffe_port + PARPORT_CONTROL) ^ STROBE_BIT); // invert strobe
  byte |= (ffe_wait_while_busy () & INPUT_MASK) << 1; // receive high nibble
  outportb (ffe_port + PARPORT_CONTROL, inportb (ffe_port + PARPORT_CONTROL) ^ STROBE_BIT); // invert strobe

  return byte;
}


unsigned char
ffe_wait_while_busy (void)
{
  unsigned char input;
  int n_try = 0;

  do
    {
      input = inportb (ffe_port + PARPORT_STATUS);
      n_try++;
    }
  while (input & IBUSY_BIT && n_try < N_TRY_MAX);

#if 0
/*
  vgs doesn't check for this, and it seems to happen quite regularly, so it
  is currently commented out
*/
  if (n_try >= N_TRY_MAX)
    {
      fprintf (stderr, "ERROR: The copier is not ready\n" // yes, "ready" :-)
                       "       Turn it off for a few seconds then turn it on and try again\n");
      exit (1);
    }
#endif

  return input;
}


void
ffe_wait_for_ready (void)
{
  unsigned char input;
  int n_try = 0;

  do
    {
      input = inportb (ffe_port + PARPORT_STATUS);
      n_try++;
    }
  while (!(input & IBUSY_BIT) && n_try < N_TRY_MAX);

#if 0
  if (n_try >= N_TRY_MAX)
    {
      fprintf (stderr, "ERROR: The copier is not ready\n"
                       "       Turn it off for a few seconds then turn it on and try again\n");
      exit (1);
    }
#endif
}


void
ffe_checkabort (int status)
{
  if (((!ucon64.frontend) ? kbhit () : 0) && getch () == 'q')
    {
//      ffe_send_command (5, 0, 0);               // vgs: when sending/receiving a SNES ROM
      puts ("\nProgram aborted");
      exit (status);
    }
}

#endif // BACKUP
