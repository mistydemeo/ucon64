/*
pce-pro.c - PCE-PRO flash card programmer support for uCON64

written by 2004 dbjh

Based on Delphi source code by ToToTEK Multi Media. Information in that source
code has been used with permission. However, ToToTEK Multi Media explicitly
stated that the information in that source code may be freely distributed.


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
#include "misc.h"
#include "misc_par.h"
#include "ucon64_misc.h"
#include "tototek.h"
#include "pce-pro.h"


const st_usage_t pcepro_usage[] =
  {
    {NULL, 0, NULL, "PCE-PRO flash card programmer", "2004 ToToTEK Multi Media http://www.tototek.com"},
#ifdef USE_PARALLEL
    {"xpce", 0, NULL, "send/receive ROM to/from PCE-PRO flash card programmer\n"
                      OPTION_LONG_S "port=PORT\n"
                      "receives automatically (32 Mbits) when ROM does not exist", NULL},
#endif // USE_PARALLEL
    {NULL, 0, NULL, NULL, NULL}
  };


#ifdef USE_PARALLEL

static void eep_reset (void);
static void write_rom_by_byte (int *addr, unsigned char *buf);
static void write_rom_by_page (int *addr, unsigned char *buf);


void
eep_reset (void)
{
  ttt_rom_enable ();
  ttt_write_mem (0x000000, 0xff);               // reset EEP
  ttt_write_mem (0x200000, 0xff);               // reset EEP
  ttt_rom_disable ();
}


void
write_rom_by_byte (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x4000; x++)
    {
      ttt_write_byte_sharp (*addr, buf[*addr & 0x3fff]);
      (*addr)++;
    }
}


void
write_rom_by_page (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x200; x++)
    {
      ttt_write_page_rom (*addr, buf);
      (*addr) += 0x20;
    }
}


int
pce_read_rom (const char *filename, unsigned int parport, int size)
{
  FILE *file;
  unsigned char buffer[0x100];
  int blocksleft, address = 0;
  time_t starttime;
  void (*read_block) (int, unsigned char *) = ttt_read_rom_w; // ttt_read_rom_b

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  ttt_init_io (parport);

  printf ("Receive: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  blocksleft = size >> 8;
  eep_reset ();
  ttt_rom_enable ();
  if (read_block == ttt_read_rom_w)
    ttt_set_ai_data (6, 0x94);          // rst=1, wei=0(dis.), rdi=0(dis.), inc mode, rom_CS
  starttime = time (NULL);
  while (blocksleft-- > 0)
    {
      read_block (address, buffer);             // 0x100 bytes read
      fwrite (buffer, 1, 0x100, file);
      address += 0x100;
      if ((address & 0x3fff) == 0)
        ucon64_gauge (starttime, address, size);
    }
  // original code doesn't call ttt_rom_disable() when byte-size function is
  //  used (ttt_read_rom_b() calls it)
  if (read_block == ttt_read_rom_w)
    ttt_rom_disable ();

  fclose (file);
  ttt_deinit_io ();

  return 0;
}


int
pce_write_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char buffer[0x4000];
  int size, fsize, address = 0, bytesread, bytessend = 0;
  time_t starttime;
  void (*write_block) (int *, unsigned char *) = write_rom_by_page; // write_rom_by_byte
  (void) write_rom_by_byte;

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  ttt_init_io (parport);

  size = fsize = q_fsize (filename);
  if (fsize == 4 * MBIT)
    size += 2 * MBIT;
  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  starttime = time (NULL);
  eep_reset ();
  if (ttt_get_id () == 0xb0d0)
    {
      eep_reset ();
      while ((bytesread = fread (buffer, 1, 0x4000, file)))
        {
          if ((address & 0xffff) == 0)
            ttt_erase_block (address);
          write_block (&address, buffer);
          if ((fsize == 3 * MBIT) && (address == 2 * MBIT))
            address += 2 * MBIT;
          if ((fsize == 4 * MBIT) && (address == 4 * MBIT))
            fseek (file, 2 * MBIT, SEEK_SET);

          bytessend += bytesread;
          ucon64_gauge (starttime, bytessend, size);
        }
    }
  else
    fputs ("ERROR: PCE-PRO flash card (programmer) not detected\n", stderr);

  fclose (file);
  ttt_deinit_io ();

  return 0;
}

#endif // USE_PARALLEL