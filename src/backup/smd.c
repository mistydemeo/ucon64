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
#include "config.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "ffe.h"
#include "smd.h"


const char *smd_usage[] =
  {
    "Super Com Pro (HK)/Super Magic Drive/SMD",
    "19XX Front Far East/FFE http://www.front.com.tw",
#ifdef BACKUP
    "TEST: " OPTION_LONG_S "xsmd    send/receive ROM to/from Super Magic Drive/SMD; " OPTION_LONG_S "file=PORT\n"
    "                  receives automatically when ROM does not exist\n"
    "TEST: " OPTION_LONG_S "xsmds   send/receive SRAM to/from Super Magic Drive/SMD; " OPTION_LONG_S "file=PORT\n"
    "                  receives automatically when SRAM does not exist\n",
#endif // BACKUP
    NULL};


#ifdef BACKUP


#define BUFFERSIZE      16384


int
smd_read_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer, byte;
  int size, blocksdone = 0, blocksleft, bytesreceived = 0;
  time_t starttime;

  ffe_init_io (parport);

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, "ERROR: Can't open %s for writing\n", filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, "ERROR: Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE);
      exit (1);
    }

  ffe_send_command (1, 0xdff1, 1);
  byte = ffe_receiveb ();
  if ((0x81 ^ byte) != ffe_receiveb ())
    printf ("received data is corrupt\n");

  blocksleft = 8 * byte;
  if (blocksleft == 0)
    {
      fprintf (stderr, "ERROR: There is no cartridge present in the Super Magic Drive\n");
      fclose (file);
      remove (filename);
      exit (1);
    }

  memset (buffer, 0, SMD_HEADER_LEN);
  buffer[0] = blocksleft;
  buffer[1] = 3;
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 6;
  fwrite (buffer, 1, SMD_HEADER_LEN, file);     // write header

  size = blocksleft * 16384;                    // size in bytes for ucon64_gauge() below
  printf ("Receive: %d Bytes (%.4f Mb)\n", size, (float) size / MBIT);

  wait (32);
  ffe_send_command0 (0x2001, 0);

  printf ("Press q to abort\n\n");

  starttime = time (NULL);
  while (blocksleft > 0)
    {
      ffe_send_command (5, blocksdone, 0);
      ffe_receive_block (0x4000, buffer, BUFFERSIZE);
      blocksdone++;
      blocksleft--;
      fwrite (buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, size);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
smd_write_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend, blocksdone = 0, fsize;
  time_t starttime;

  ffe_init_io (parport);

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, "ERROR: Can't open %s for reading\n", filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, "ERROR: Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE);
      exit (1);
    }

  fsize = q_fsize (filename);
  printf ("Send: %d Bytes (%.4f Mb)\n", fsize, (float) fsize / MBIT);

  fread (buffer, 1, SMD_HEADER_LEN, file);
  ffe_send_block (0xdc00, buffer, SMD_HEADER_LEN); // send header
  bytessend = SMD_HEADER_LEN;

  ffe_send_command0 (0x2001, 0);

  printf ("Press q to abort\n\n");

  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      ffe_send_command (5, blocksdone, 0);
      ffe_send_block (0x8000, buffer, bytesread);
      blocksdone++;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, fsize);
      ffe_checkabort (2);
    }

  // ROM dump > 128 16 KB blocks? (=16 Mb (=2 MB))
  ffe_send_command0 (0x2001, blocksdone > 0x80 ? 7 : 3);

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
smd_read_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int blocksleft, bytesreceived = 0;
  unsigned short address;
  time_t starttime;

  ffe_init_io (parport);

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, "ERROR: Can't open %s for writing\n", filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, "ERROR: Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE);
      exit (1);
    }

  printf ("Receive: %d Bytes\n", 32 * 1024);
  memset (buffer, 0, SMD_HEADER_LEN);
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 7;
  fwrite (buffer, 1, SMD_HEADER_LEN, file);

  ffe_send_command0 (0x2001, 4);

  printf ("Press q to abort\n\n");

  blocksleft = 2;                               // SRAM is 2*16 KB
  address = 0x4000;
  starttime = time (NULL);
  while (blocksleft > 0)
    {
      ffe_receive_block (address, buffer, BUFFERSIZE);
      blocksleft--;
      address += 0x4000;
      fwrite (buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, 32 * 1024);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
smd_write_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend = 0, size;
  unsigned short address;
  time_t starttime;

  ffe_init_io (parport);

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, "ERROR: Can't open %s for reading\n", filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, "ERROR: Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE);
      exit (1);
    }

  size = q_fsize (filename) - SMD_HEADER_LEN;
  printf ("Send: %d Bytes\n", size);
  fseek (file, SMD_HEADER_LEN, SEEK_SET);       // skip the header

  ffe_send_command0 (0x2001, 4);

  printf ("Press q to abort\n\n");

  address = 0x4000;
  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      ffe_send_block (address, buffer, bytesread);
      address += 0x4000;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}
#endif // BACKUP
