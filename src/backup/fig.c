/*
fig.c - Super PRO Fighter support for uCON64

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh


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
#include <time.h>
#include <string.h>
#include "misc.h"                               // kbhit(), getch()
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ffe.h"
#include "fig.h"
#include "console/snes.h"                       // for snes_get_file_type () &&
                                                //  snes_get_snes_hirom()


const st_usage_t fig_usage[] =
  {
    {NULL, NULL, "Super Pro Fighter (Q/Q+)/Pro Fighter X (Turbo 2)/Double Pro Fighter (X Turbo)"},
    {NULL, NULL, "1993/1994/19XX China Coach Limited/CCL http://www.ccltw.com.tw"},
#ifdef  PARALLEL
#if 1 // dumping is not yet supported
    {"xfig", NULL, "send ROM to *Pro Fighter*/(all)FIG; " OPTION_LONG_S "port=PORT"},
#else
    {"xfig", NULL, "send/receive ROM to/from *Pro Fighter*/(all)FIG; " OPTION_LONG_S "port=PORT\n"
                "receives automatically when " OPTION_LONG_S "rom does not exist"},
#endif
#endif  // PARALLEL
    {NULL, NULL, NULL}
  };

#ifdef PARALLEL


#define BUFFERSIZE      8192                    // don't change, only 8192 works!


static void handle_fig_header (unsigned char *header);


void
handle_fig_header (unsigned char *header)
{
  if ((header[4] == 0x77 && header[5] == 0x83) ||
      (header[4] == 0xf7 && header[5] == 0x83) ||
      (header[4] == 0x47 && header[5] == 0x83) ||
      (header[4] == 0x11 && header[5] == 0x02))
    header[2] = 0x0c;                           // 0 kB
  else if (header[4] == 0xfd && header[5] == 0x82)
    header[2] = 0x08;                           // 2 kB
  else if ((header[4] == 0xdd && header[5] == 0x82) ||
           (header[4] == 0x00 && header[5] == 0x80))
    /*
      8 kB *or* 2 kB (shortcoming of FIG header format). We give the emu mode
      select byte a value as if the game uses 8 kB. At least this makes games
      that use 8 kB work.
    */
    header[2] = 0x04;
  else // if ((header[4] == 0xdd && header[5] == 0x02) ||
       //     (header[4] == 0x00 && header[5] == 0x00))
    header[2] = 0;                              // 32 kB

  if (header[3] & 0x80)                         // Pro Fighter (FIG) HiROM dump
    header[2] |= 0x30;                          // set bit 5&4 (SRAM & DRAM mem map mode 21)
}


int
fig_read_rom (const char *filename, unsigned int parport)
{
#if 0
  FILE *file;
  unsigned char *buffer;

  init_io (parport);
  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
#else
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  return fprintf (stderr, "ERROR: The function for dumping a cartridge is not yet implemented\n");
#endif
}


int
fig_write_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread = 0, bytessend = 0, totalblocks, blocksdone = 0, blocksleft,
      emu_mode_select, fsize, hirom, n;
  unsigned short address;
  time_t starttime;

  ffe_init_io (parport);

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  fsize = q_fsize (filename);
  printf ("Send: %d Bytes (%.4f Mb)\n", fsize, (float) fsize / MBIT);

  hirom = snes_get_snes_hirom ();
  if (hirom)
    ffe_send_command0 (0xe00f, 0);              // seems to enable HiROM mode,
                                                //  value doesn't seem to matter
  fread (buffer, 1, FIG_HEADER_LEN, file);
  if (snes_get_file_type () == FIG)
    handle_fig_header (buffer);
  emu_mode_select = buffer[2];                  // this byte is needed later

  printf ("Press q to abort\n\n");              // print here, NOT before first FIG I/O,
                                                //  because if we get here q works ;-)
  totalblocks = (fsize - FIG_HEADER_LEN + BUFFERSIZE - 1) / BUFFERSIZE; // round up
  blocksleft = totalblocks;
  address = 0x200;
  starttime = time (NULL);
  while (blocksleft > 0)
    {
      if (hirom)
        {
          for (n = 0; n < 4; n++)
            {
              bytesread = fread (buffer, 1, BUFFERSIZE, file);
              ffe_send_command0 ((unsigned short) 0xc010, (unsigned char) (blocksdone >> 9));
              ffe_send_command (5, address + 0x100, 0);
              ffe_send_block (0x0000, buffer, bytesread);
              address++;
              blocksleft--;
              blocksdone++;

              bytessend += bytesread;
              ucon64_gauge (starttime, bytessend, fsize);
              ffe_checkabort (2);
            }
        }

      for (n = 0; n < 4; n++)
        {
          bytesread = fread (buffer, 1, BUFFERSIZE, file);
          ffe_send_command0 ((unsigned short) 0xc010, (unsigned char) (blocksdone >> 9));
          ffe_send_command (5, address, 0);
          ffe_send_block (0x8000, buffer, bytesread);
          address++;
          blocksleft--;
          blocksdone++;

          bytessend += bytesread;
          ucon64_gauge (starttime, bytessend, fsize);
          ffe_checkabort (2);
        }
    }

  if (blocksdone > 0x200)                       // ROM dump > 512 8 KB blocks (=32 Mb (=4 MB))
    ffe_send_command0 (0xc010, 2);

  ffe_send_command0 (0xc008, 0);
  fseek (file, 0, SEEK_SET);
  fread (buffer, 1, FIG_HEADER_LEN, file);
  ffe_send_command (5, 0, 0);
  ffe_send_block (0x400, buffer, FIG_HEADER_LEN); // send header
  bytessend += FIG_HEADER_LEN;
  ucon64_gauge (starttime, bytessend, fsize);

  ffe_send_command (5, 0, 0);
  ffe_send_command (6, (unsigned short) (5 | (totalblocks << 8)), (unsigned short) (totalblocks >> 8)); // bytes: 6, 5, #8 K L, #8 K H, 0
  ffe_send_command (6, (unsigned short) (1 | (emu_mode_select << 8)), 0);

  ffe_wait_for_ready ();
  outportb ((unsigned short) (parport + PARPORT_DATA), 0);
  outportb ((unsigned short) (parport + PARPORT_CONTROL),
            (unsigned char) (inportb ((unsigned short) (parport + PARPORT_CONTROL)) ^ STROBE_BIT)); // invert strobe

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
fig_read_sram (const char *filename, unsigned int parport)
{
#if 0
  FILE *file;
  unsigned char *buffer;

  init_io (parport);
  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
#else
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  return fprintf (stderr, "ERROR: The function for dumping SRAM is not yet implemented\n");
#endif
}


int
fig_write_sram (const char *filename, unsigned int parport)
{
#if 0
  FILE *file;
  unsigned char *buffer;

  init_io (parport);
  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
#else
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  return fprintf (stderr, "ERROR: The function for sending SRAM is not yet implemented\n");
#endif
}

#endif // PARALLEL
