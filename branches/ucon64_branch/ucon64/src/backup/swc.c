/*
swc.c - Super Wild Card support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh
                  2001 Caz


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
#include "swc.h"
#include "../console/snes.h"


const st_usage_t swc_usage[] =
  {
    {NULL, NULL, "Super Com Pro/Super Magicom/SMC/Super Wild Card (1.6XC/2.7CC/2.8CC/DX/DX2)/SWC"},
    {NULL, NULL, "1993/1994/1995/19XX Front Far East/FFE http://www.front.com.tw"},
#ifdef PARALLEL
    {"xswc", NULL, "send/receive ROM to/from Super Wild Card*/(all)SWC; " OPTION_LONG_S "port=PORT\n"
                "receives automatically when ROM does not exist\n"
                "Press q to abort; ^C will cause invalid state of backup unit"},
    {"xswc2", NULL, "same as " OPTION_LONG_S "xswc, but enables Real Time Save mode (SWC only)"},
    {"xswcs", NULL, "send/receive SRAM to/from Super Wild Card*/(all)SWC;\n"
                 OPTION_LONG_S "port=PORT\n"
                 "receives automatically when SRAM does not exist\n"
                 "Press q to abort; ^C will cause invalid state of backup unit"},
#endif // PARALLEL
    {NULL, NULL, NULL}
  };

#ifdef PARALLEL


#define BUFFERSIZE      8192                    // don't change, only 8192 works!


static int receive_rom_info (unsigned char *buffer);
static int get_rom_size (unsigned char *info_block);
static int check1 (unsigned char *info_block, int index);
static int check2 (unsigned char *info_block, int index, unsigned char value);
static int check3 (unsigned char *info_block, int index1, int index2, int size);
static unsigned char get_emu_mode_select (unsigned char byte, int size);
static void handle_fig_header (unsigned char *header);

static int hirom;                               // `hirom' was `special'


#if BUFFERSIZE < 512
#error receive_rom_info() and swc_read_sram() expect BUFFERSIZE to be at least \
       512 bytes.
#endif
int
receive_rom_info (unsigned char *buffer)
/*
  - returns size of ROM in Mb (128 KB) units
  - returns ROM header in buffer (index 2 (emulation mode select) is not yet
    filled in)
  - sets global `hirom'
*/
{
  int n, m, size;
  unsigned short address;
  unsigned char byte;

  ffe_send_command0 (0xe00c, 0);
  ffe_send_command (5, 3, 0);
  ffe_send_command (1, 0xbfd5, 1);
  byte = ffe_receiveb ();
  if ((0x81 ^ byte) != ffe_receiveb ())
    printf ("received data is corrupt\n");
  hirom = byte & 1;                             // Caz (vgs '96) does (byte & 0x21) == 0x21 ? 1 : 0;

  address = 0x200;
  for (n = 0; n < (int) SWC_HEADER_LEN; n++)
    {
#ifdef  _WIN32
      /*
        Talk about strange. Somehow this loop takes unusually long (like 1
        minute) if no Windows function is called. As a side effect (?) an
        incorrect size value will be obtained, resulting in an overdump. For
        example calling kbhit() or printf() solves the problem...
      */
      kbhit ();
#endif
      for (m = 0; m < 65536; m++)               // a delay is necessary here
        ;
      ffe_send_command (5, address, 0);
      ffe_send_command (1, 0xa0a0, 1);
      buffer[n] = ffe_receiveb ();
      if ((0x81 ^ buffer[n]) != ffe_receiveb ())
        printf ("received data is corrupt\n");

      address++;
    }

  size = get_rom_size (buffer);
  if (hirom)
    size <<= 1;

  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[0] = size << 4 & 0xff;                 // *16 for 8 KB units; low byte
  buffer[1] = size >> 4;                        // *16 for 8 KB units /256 for high byte
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 4;

  return size;
}


int
get_rom_size (unsigned char *info_block)
// returns size of ROM in Mb units
{
  if (check1 (info_block, 0))
    return 0;
  if (check2 (info_block, 0x10, 0x84))
    return 0;
  if (check3 (info_block, 0, 0x20, 0x20))
    return 2;
  if (check3 (info_block, 0, 0x40, 0x20))
    return 4;
  if (check3 (info_block, 0x40, 0x60, 0x20))
    return 6;
  if (check3 (info_block, 0, 0x80, 0x10))
    return 8;
  if (check1 (info_block, 0x80))
    return 8;
  if (check3 (info_block, 0x80, 0x90, 0x10))
    return 8;
  if (check2 (info_block, 0x80, 0xa0))
    return 8;
  if (check3 (info_block, 0x80, 0xa0, 0x20))
    return 0xa;
  if (check1 (info_block, 0xc0))
    return 0xc;
  if (check2 (info_block, 0xc0, 0xb0))
    return 0xc;
  if (check3 (info_block, 0x80, 0xc0, 0x20))
    return 0xc;
  if (check3 (info_block, 0x100, 0, 0x10))
    return 0x10;
  if (check2 (info_block, 0x100, 0xc0))
    return 0x10;
  if (check3 (info_block, 0x100, 0x120, 0x10))
    return 0x12;
  if (check3 (info_block, 0x100, 0x140, 0x10))
    return 0x14;
  if (check2 (info_block, 0x140, 0xd0))
    return 0x14;
  if (check3 (info_block, 0x100, 0x180, 0x10))
    return 0x18;
  if (check2 (info_block, 0x180, 0xe0))
    return 0x18;
  if (check3 (info_block, 0x180, 0x1c0, 0x10))
    return 0x1c;
  if (check3 (info_block, 0x1f0, 0x1f0, 0x10))
    return 0x20;

  return 0;
}


int
check1 (unsigned char *info_block, int index)
{
  int n;

  for (n = 0; n < 16; n++)
    if (info_block[n + index] != info_block[index])
      return 0;

  return 1;
}


int
check2 (unsigned char *info_block, int index, unsigned char value)
{
  int n;

  for (n = 0; n < 4; n++)
    if (info_block[n + index] != value)
      return 0;

  return 1;
}


int
check3 (unsigned char *info_block, int index1, int index2, int size)
{
  int n;

  for (n = 0; n < size; n++)
    if (info_block[n + index1] != info_block[n + index2])
      return 0;

  return 1;
}


unsigned char
get_emu_mode_select (unsigned char byte, int size)
{
  int x;
  unsigned char ems;

  if (byte == 0)
    x = 0xc;
  else if (byte == 1)
    x = 8;
  else if (byte == 3)
    x = 4;
  else
    x = 0;

  if (hirom)
    {
      if (x == 0xc && size <= 0x1c)
        ems = 0x1c;
      else
        ems = x + 0x30;
    }
  else
    {
      if (x == 0xc)
        ems = 0x2c;
      else
        ems = x;

      if (size <= 8)
        ems++;
    }

  return ems;
}


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
      8 kB *or* 2 kB (shortcomming of FIG header format). We give the emu mode
      select byte a value as if the game uses 8 kB. At least this makes games
      that use 8 kB work.
      Users should not complain if the game doesn't work because of a SRAM
      protection, because they should have converted the ROM to SWC format in
      the first place.
    */
    header[2] = 0x04;
  else // if ((header[4] == 0xdd && header[5] == 0x02) ||
       //     (header[4] == 0x00 && header[5] == 0x00))
    header[2] = 0;                              // 32 kB

  if (header[3] & 0x80)                         // Pro Fighter (FIG) HiROM dump
    header[2] |= 0x30;                          // set bit 5&4 (SRAM & DRAM mem map mode 21)
}


void
swc_unlock (unsigned int parport)
/*
  "Unlock" the SWC. However, just starting to send, then stopping with ^C,
  gives the same result.
*/
{
  ffe_init_io (parport);
  ffe_send_command (6, 0, 0);
  ffe_deinit_io ();
}


int
swc_read_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer, byte;
  int n, size, blocksleft, bytesreceived = 0;
  unsigned short address1, address2;
  time_t starttime;

  ffe_init_io (parport);

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

  size = receive_rom_info (buffer);
  if (size == 0)
    {
      fprintf (stderr, "ERROR: There is no cartridge present in the Super Wild Card\n");
      fclose (file);
      remove (filename);
      exit (1);
    }
  blocksleft = size * 16;                       // 1 Mb (128 KB) unit == 16 8 KB units
  printf ("Receive: %d Bytes (%.4f Mb)\n", size * MBIT, (float) size);
  size *= MBIT;                                 // size in bytes for ucon64_gauge() below

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00c, 0);
  ffe_send_command0 (0xe003, 0);
  ffe_send_command (1, 0xbfd8, 1);
  byte = ffe_receiveb ();
  if ((0x81 ^ byte) != ffe_receiveb ())
    printf ("received data is corrupt\n");

  buffer[2] = get_emu_mode_select (byte, blocksleft / 16);
  fwrite (buffer, 1, SWC_HEADER_LEN, file);     // write header (other necessary fields are
                                                //  filled in by receive_rom_info())
  if (hirom)
    blocksleft >>= 1;                           // this must come _after_ get_emu_mode_select()!

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address1 = 0x300;                             // address1 = 0x100, address2 = 0 should
  address2 = 0x200;                             //  also work
  starttime = time (NULL);
  while (blocksleft > 0)
    {
      if (hirom)
        {
          for (n = 0; n < 4; n++)
            {
              ffe_send_command (5, address1, 0);
              ffe_receive_block (0x2000, buffer, BUFFERSIZE);
              address1++;
              fwrite (buffer, 1, BUFFERSIZE, file);

              bytesreceived += BUFFERSIZE;
              ucon64_gauge (starttime, bytesreceived, size);
              ffe_checkabort (2);
            }
        }

      for (n = 0; n < 4; n++)
        {
          ffe_send_command (5, address2, 0);
          ffe_receive_block (0xa000, buffer, BUFFERSIZE);
          blocksleft--;
          address2++;
          fwrite (buffer, 1, BUFFERSIZE, file);

          bytesreceived += BUFFERSIZE;
          ucon64_gauge (starttime, bytesreceived, size);
          ffe_checkabort (2);
        }
    }
  ffe_send_command (5, 0, 0);

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_write_rom (const char *filename, unsigned int parport, int enableRTS)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend, totalblocks, blocksdone = 0, emu_mode_select, fsize;
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

  ffe_send_command0 (0xc008, 0);
  fread (buffer, 1, SWC_HEADER_LEN, file);
  ffe_send_command (5, 0, 0);
  ffe_send_block (0x400, buffer, SWC_HEADER_LEN); // send header
  bytessend = SWC_HEADER_LEN;

  if (snes_get_file_type () == FIG)
    handle_fig_header (buffer);
  emu_mode_select = buffer[2];                  // this byte is needed later
#if 1
  /*
    0x0c == no SRAM & LoROM; we use the header, so that the user can override this
    bit 4 == 0 => DRAM mode 20 (LoROM); disable SRAM by setting SRAM mem map mode 21
  */
  if ((emu_mode_select & 0x1c) == 0x0c)
    emu_mode_select |= 0x20;
#else
  // The code below doesn't work for some HiROM games that don't use SRAM.
  if ((emu_mode_select & 0x0c) == 0x0c)         // 0x0c == no SRAM; we use the header, so
    {                                           //  that the user can override this
      if (emu_mode_select & 0x10)               // bit 4 == 1 => DRAM mode 21 (HiROM)
        emu_mode_select &= ~0x20;               // disable SRAM by setting SRAM mem map mode 20
      else                                      // bit 4 == 0 => DRAM mode 20 (LoROM)
        emu_mode_select |= 0x20;                // disable SRAM by setting SRAM mem map mode 21
    }
#endif

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = 0x200;                              // vgs '00 uses 0x200, vgs '96 uses 0,
  starttime = time (NULL);                      //  but then some ROMs don't work
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      ffe_send_command0 ((unsigned short) 0xc010, (unsigned char) (blocksdone >> 9));
      ffe_send_command (5, address, 0);
      ffe_send_block (0x8000, buffer, bytesread);
      address++;
      blocksdone++;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, fsize);
      ffe_checkabort (2);
    }

  if (blocksdone > 0x200)                       // ROM dump > 512 8 KB blocks (=32 Mb (=4 MB))
    ffe_send_command0 (0xc010, 2);

  ffe_send_command (5, 0, 0);
  totalblocks = (fsize - SWC_HEADER_LEN + BUFFERSIZE - 1) / BUFFERSIZE; // round up
  ffe_send_command (6, (unsigned short) (5 | (totalblocks << 8)), (unsigned short) (totalblocks >> 8)); // bytes: 6, 5, #8 K L, #8 K H, 0
  ffe_send_command (6, (unsigned short) (1 | (emu_mode_select << 8)), (unsigned short) enableRTS); // last arg = 1 enables RTS
                                                               //  mode, 0 disables it
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
swc_read_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int blocksleft, bytesreceived = 0;
  unsigned short address;
  time_t starttime;

  ffe_init_io (parport);

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

  printf ("Receive: %d Bytes\n", 32 * 1024);
  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 5;
  fwrite (buffer, 1, SWC_HEADER_LEN, file);

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00d, 0);
  ffe_send_command0 (0xc008, 0);

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  blocksleft = 4;                               // SRAM is 4*8 KB
  address = 0x100;
  starttime = time (NULL);
  while (blocksleft > 0)
    {
      ffe_send_command (5, address, 0);
      ffe_receive_block (0x2000, buffer, BUFFERSIZE);
      blocksleft--;
      address++;
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
swc_write_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend = 0, size;
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

  size = q_fsize (filename) - SWC_HEADER_LEN;   // SWC SRAM is 4*8 KB, emu SRAM often not
  printf ("Send: %d Bytes\n", size);
  fseek (file, SWC_HEADER_LEN, SEEK_SET);       // skip the header

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00d, 0);
  ffe_send_command0 (0xc008, 0);

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = 0x100;
  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      ffe_send_command (5, address, 0);
      ffe_send_block (0x2000, buffer, bytesread);
      address++;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}

#endif // PARALLEL