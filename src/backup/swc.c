/*
swc.c - Super Wild Card support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2004 dbjh
                  2001 Caz
                  2003 John Weidman
                  2004 JohnDie


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
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ffe.h"
#include "swc.h"
#include "console/snes.h"                       // for snes_get_file_type ()


const st_usage_t swc_usage[] =
  {
    {NULL, NULL, "Super Com Pro/Super Magicom/SMC/Super Wild Card (1.6XC/2.7CC/2.8CC/DX/DX2)/SWC"},
    {NULL, NULL, "1993/1994/1995/19XX Front Far East/FFE http://www.front.com.tw"},
#ifdef PARALLEL
    {"xswc", NULL, "send/receive ROM to/from Super Wild Card*/(all)SWC; " OPTION_LONG_S "port=PORT\n"
                   "receives automatically when ROM does not exist"},
    {"xswc2", NULL, "same as " OPTION_LONG_S "xswc, but enables Real Time Save mode (SWC only)"},
#if 0
// hidden, undocumented option because we don't want people to "accidentally"
//  create overdumps
    {"xswc-super", NULL, "receive ROM (forced 32 Mb) from Super Wild Card*/(all)SWC"},
#endif
    {"xswcs", NULL, "send/receive SRAM to/from Super Wild Card*/(all)SWC;\n"
                    OPTION_LONG_S "port=PORT\n"
                    "receives automatically when SRAM does not exist"},
    {"xswcc", NULL, "send/receive SRAM to/from cartridge in Super Wild Card*/(all)SWC;\n"
                    OPTION_LONG_S "port=PORT\n"
                    "receives automatically when SRAM does not exist"},
    {"xswcr", NULL, "send/receive RTS data to/from Super Wild Card*/(all)SWC;\n"
                    OPTION_LONG_S "port=PORT\n"
                    "receives automatically when RTS file does not exist"},
#endif // PARALLEL
    {NULL, NULL, NULL}
  };

#ifdef PARALLEL

#define BUFFERSIZE 8192                         // don't change, only 8192 works!

//#define DUMP_MMX2
//#define DUMP_SA1
//#define DUMP_SDD1

static int receive_rom_info (unsigned char *buffer, int superdump);
static int get_rom_size (unsigned char *info_block);
static int check1 (unsigned char *info_block, int index);
static int check2 (unsigned char *info_block, int index, unsigned char value);
static int check3 (unsigned char *info_block, int index1, int index2, int size);
static unsigned char get_emu_mode_select (unsigned char byte, int size);
static void handle_fig_header (unsigned char *header);
static int sub (void);
static int mram_helper (int x);
static int mram (void);

static int hirom;                               // `hirom' was `special'

#ifdef  DUMP_SA1
static void set_sa1_map (unsigned short chunk);
/*extern*/ int snes_sa1 = 0;            // change to non-zero to dump SA-1 cartridges
#endif
#ifdef  DUMP_SDD1
static void set_sdd1_map (unsigned short chunk);
/*extern*/ int snes_sdd1 = 0;           // change to non-zero to dump S-DD1 cartridges
#endif


#if BUFFERSIZE < 512
#error receive_rom_info() and swc_read_sram() expect BUFFERSIZE to be at least \
       512 bytes.
#endif
int
receive_rom_info (unsigned char *buffer, int superdump)
/*
  - returns size of ROM in Mb (128 kB) units
  - returns ROM header in buffer (index 2 (emulation mode select) is not yet
    filled in)
  - sets global `hirom'
*/
{
  int n, m, size;
  unsigned short address;
  unsigned char byte;

#ifdef  DUMP_MMX2
  /*
    MMX2 can be dumped after writing a 0 to SNES register 0x7f52. Before we can
    write to that register we have to enable cartridge page mapping. That is
    done by writing to SWC register 0xe00c. When cartridge page mapping is
    enabled we can access SNES registers by reading or writing to the SWC
    address range 0x2000-0x3fff. Before reading or writing to an address in that
    range we have to "announce" the address to the SWC (via command 5). Because
    we access a SNES register we only set the page number bits (0-1).
  */
  address = 0x7f52;
  ffe_send_command0 (0xe00c, 0);

  ffe_send_command (5, address / 0x2000, 0);
  ffe_receive_block ((address & 0x1fff) + 0x2000, buffer, 8);
  mem_hexdump (buffer, 8, address);

  ffe_send_command (5, address / 0x2000, 0);
  ffe_send_command0 ((address & 0x1fff) + 0x2000, 0);

  ffe_send_command (5, address / 0x2000, 0);
  ffe_receive_block ((address & 0x1fff) + 0x2000, buffer, 8);
  mem_hexdump (buffer, 8, address);
#endif

  ffe_send_command0 (0xe00c, 0);
  ffe_send_command (5, 3, 0);

  byte = ffe_send_command1 (0xbfd5);
  // I don't know if it's okay to skip the above call to ffe_send_command1() if
  //  ucon64.snes_hirom is set - dbjh
  if (UCON64_ISSET (ucon64.snes_hirom))
    hirom = ucon64.snes_hirom ? 1 : 0;
  else if ((byte & 1 && byte != 0x23) || byte == 0x3a) // & 1 => 0x21, 0x31, 0x35
    hirom = 1;

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
      buffer[n] = ffe_send_command1 (0xa0a0);

      address++;
    }

  if (superdump)
    {
      if (!UCON64_ISSET (ucon64.snes_hirom))
        hirom = 1;                              // default to super HiROM dump
      size = 32;                                // dump 32 Mbit
    }
  else
    {
      size = get_rom_size (buffer);
#ifdef  DUMP_SA1
      if (!snes_sa1)
#endif
      if (hirom)
        size <<= 1;
    }

#ifdef  DUMP_SDD1
  // Adjust size to 48 Mbit for Star Ocean
  if (snes_sdd1 && size == 0x20)
    {
      ffe_send_command (5, 3, 0);
      byte = ffe_send_command1 (0xbfd7);
      if (byte == 0x0d)
        size = 0x30;
    }
#endif

#ifdef  DUMP_SA1
  // Fixup size for SA-1 chips
  if (snes_sa1)
    {
      ffe_send_command (5, 3, 0);
      byte = ffe_send_command1 (0xbfd7);
      switch (byte)
        {
        case 0x09:
          size = 0x04;
          break;
        case 0x0a:
          size = 0x08;
          break;
        case 0x0b:
          size = 0x10;
          break;
        case 0x0c:
          size = 0x20;
          break;
        default:
          break;
        }
    }
#endif

  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[0] = size << 4;                        // *16 for 8 kB units; low byte
  buffer[1] = size >> 4;                        // *16 for 8 kB units /256 for high byte
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

//      if (size <= 8)                          // JohnDie: This bit should always be 0
//        ems++;
    }

  return ems;
}


void
handle_fig_header (unsigned char *header)
{
  if ((header[4] == 0x77 && header[5] == 0x83) ||
      (header[4] == 0xf7 && header[5] == 0x83) ||
      (header[4] == 0x47 && header[5] == 0x83))
    header[2] = 0x0c;                           // 0 kB
  else if (header[4] == 0xfd && header[5] == 0x82)
    header[2] = 0x08;                           // 2 kB
  else if ((header[4] == 0xdd && header[5] == 0x82) ||
           (header[4] == 0x00 && header[5] == 0x80))
    /*
      8 kB *or* 2 kB (shortcoming of FIG header format). We give the emu mode
      select byte a value as if the game uses 8 kB. At least this makes games
      that use 8 kB work.
      Users should not complain if the game doesn't work because of a SRAM
      protection, because they should have converted the ROM to SWC format in
      the first place.
    */
    header[2] = 0x04;
  else // if ((header[4] == 0xdd && header[5] == 0x02) ||
       //     (header[4] == 0x00 && header[5] == 0x00) ||
       //     (header[4] == 0x11 && header[5] == 0x02))
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


#ifdef  DUMP_SA1
void
set_sa1_map (unsigned short chunk)
{
  int m;

  chunk = (chunk & 0x07) | 0x80;

  // map the 8 Mbit ROM chunk specified by chunk into the F0 bank
  ffe_send_command (5, 1, 0);
  ffe_send_command0 (0x2223, (unsigned char) chunk); // $00:2223
  for (m = 0; m < 65536; m++)
    ;
}
#endif


#ifdef  DUMP_SDD1
void
set_sdd1_map (unsigned short chunk)
{
  int m;

  chunk &= 0x07;

  // map the 8 Mbit ROM chunk specified by chunk into the F0 bank
  ffe_send_command (5, 2, 0);
  ffe_send_command0 (0x2807, (unsigned char) chunk); // $00:4807
  for (m = 0; m < 65536; m++)
    ;
}
#endif


int
swc_read_rom (const char *filename, unsigned int parport, int superdump)
{
  FILE *file;
  unsigned char *buffer, byte;
  int n, size, blocksleft, bytesreceived = 0;
  unsigned short address1, address2;
  time_t starttime;
#if     defined DUMP_SA1 || defined DUMP_SDD1
  int s_chip = 0;
  unsigned short chunk_num = 0;         // 0 = 1st 8 Mb ROM chunk, 1 = 2nd 8 Mb, ...
#endif

#ifdef  DUMP_SA1
  s_chip = snes_sa1;
#endif
#ifdef  DUMP_SDD1
  if (!s_chip)
    s_chip = snes_sdd1;
#endif

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

  size = receive_rom_info (buffer, superdump);
  if (size == 0)
    {
      fprintf (stderr, "ERROR: There is no cartridge present in the Super Wild Card\n");
      fclose (file);
      remove (filename);
      exit (1);
    }
  blocksleft = size * 16;                       // 1 Mb (128 kB) unit == 16 8 kB units
  printf ("Receive: %d Bytes (%.4f Mb)\n", size * MBIT, (float) size);
#ifdef  DUMP_SA1
  if (snes_sa1)
    puts ("NOTE: Dumping SA-1 cartridge");
#endif
#ifdef  DUMP_SDD1
  if (snes_sdd1)
    puts ("NOTE: Dumping S-DD1 cartridge");
#endif
  size *= MBIT;                                 // size in bytes for ucon64_gauge() below

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00c, 0);
  ffe_send_command0 (0xe003, 0);
  byte = ffe_send_command1 (0xbfd8);
  buffer[2] = get_emu_mode_select (byte, blocksleft / 16);
  fwrite (buffer, 1, SWC_HEADER_LEN, file);     // write header (other necessary fields are
                                                //  filled in by receive_rom_info())
  if (hirom
#if     defined DUMP_SA1 || defined DUMP_SDD1
      || s_chip
#endif
     )
    blocksleft >>= 1;                           // this must come _after_ get_emu_mode_select()!

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
#if     defined DUMP_SA1 || defined DUMP_SDD1
  if (!s_chip)
    {
#endif
      address1 = 0x300;                         // address1 = 0x100, address2 = 0 should
      address2 = 0x200;                         //  also work
#if     defined DUMP_SA1 || defined DUMP_SDD1
    }
  else                                          // SA-1 or S-DD1
    {
      address1 = 0x3c0;
      address2 = 0x3c0;

#ifdef  DUMP_SA1
      if (snes_sa1)
        set_sa1_map (chunk_num++);
#endif
#ifdef  DUMP_SDD1
      if (snes_sdd1)
        set_sdd1_map (chunk_num++);
#endif
    }
#endif

  starttime = time (NULL);
  while (blocksleft > 0)
    {
      if (hirom
#if     defined DUMP_SA1 || defined DUMP_SDD1
          || s_chip
#endif
         )
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

#if     defined DUMP_SA1 || defined DUMP_SDD1
      if (s_chip && address2 == 0x400)
        {
#ifdef  DUMP_SA1
          if (snes_sa1)
            set_sa1_map (chunk_num++);
#endif
#ifdef  DUMP_SDD1
          if (snes_sdd1)
            set_sdd1_map (chunk_num++);
#endif
          address1 = 0x3c0;
          address2 = 0x3c0;
        }
#endif
    }

#ifdef  DUMP_SA1
  if (snes_sa1)
    set_sa1_map (3);
#endif
#ifdef  DUMP_SDD1
  if (snes_sdd1)
    set_sdd1_map (3);
#endif
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

  if (snes_get_file_type () == FIG)
    handle_fig_header (buffer);
#if 1
  /*
    0x0c == no SRAM & LoROM; we use the header, so that the user can override this
    bit 4 == 0 => DRAM mode 20 (LoROM); disable SRAM by setting SRAM mem map mode 21
  */
  if ((buffer[2] & 0x1c) == 0x0c)
    buffer[2] |= 0x20;
#else
  // The code below doesn't work for some HiROM games that don't use SRAM.
  if ((buffer[2] & 0x0c) == 0x0c)               // 0x0c == no SRAM; we use the header, so
    {                                           //  that the user can override this
      if (buffer[2] & 0x10)                     // bit 4 == 1 => DRAM mode 21 (HiROM)
        buffer[2] &= ~0x20;                     // disable SRAM by setting SRAM mem map mode 20
      else                                      // bit 4 == 0 => DRAM mode 20 (LoROM)
        buffer[2] |= 0x20;                      // disable SRAM by setting SRAM mem map mode 21
    }
#endif
  emu_mode_select = buffer[2];                  // this byte is needed later

#if 1                                           // sending the header is not required
  ffe_send_command (5, 0, 0);
  ffe_send_block (0x400, buffer, SWC_HEADER_LEN); // send header
#endif
  bytessend = SWC_HEADER_LEN;

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

  if (blocksdone > 0x200)                       // ROM dump > 512 8 kB blocks (=32 Mb (=4 MB))
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
  blocksleft = 4;                               // SRAM is 4*8 kB
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

  size = q_fsize (filename) - SWC_HEADER_LEN;   // SWC SRAM is 4*8 kB, emu SRAM often not
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


int
sub (void)
{
  ffe_send_command (5, 7 * 4, 0);
  ffe_send_command0 (0xe00d, 0);
  ffe_send_command0 (0xe003, 0);

  if (ffe_send_command1 (0xb080) != 'S')
    return 0;
  if (ffe_send_command1 (0xb081) != 'U')
    return 0;
  if (ffe_send_command1 (0xb082) != 'B')
    return 0;

  return 1;
}


int
mram_helper (int x)
{
  ffe_send_command (5, (unsigned short) x, 0);
  x = ffe_send_command1 (0x8000);
  ffe_send_command0 (0x8000, (unsigned char) (x ^ 0xff));
  if (ffe_send_command1 (0x8000) != (unsigned char) (x ^ 0xff))
    return 0;

  ffe_send_command0 (0x8000, (unsigned char) x);
  return 1;
}


int
mram (void)
{
  if (mram_helper (0x76 * 4))
    return 0x76 * 4;
  if (mram_helper (0x56 * 4))
    return 0x56 * 4;
  if (mram_helper (0x36 * 4))
    return 0x36 * 4;
  return 0x16 * 4;
}


int
swc_read_rts (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int blocksleft, bytesreceived = 0;
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

  printf ("Receive: %d Bytes\n", 256 * 1024);
  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 8;
  fwrite (buffer, 1, SWC_HEADER_LEN, file);

  printf ("Press q to abort\n\n");
  blocksleft = 32;                              // RTS data is 32*8 kB

  if (sub ())
    {
      address1 = 0;
      address2 = 0xa000;
    }
  else
    {
      address1 = mram ();
      address2 = 0x8000;
    }

  starttime = time (NULL);
  while (blocksleft > 0)
    {
      ffe_send_command (5, address1, 0);
      if (address2 == 0x8000)
        ffe_send_command0 (0xc010, 1);
      ffe_receive_block (address2, buffer, BUFFERSIZE);

      blocksleft--;
      address1++;
      fwrite (buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, 256 * 1024);
      ffe_checkabort (2);
    }
  ffe_send_command (6, 3, 0);

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_write_rts (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend = 0, size;
  unsigned short address1, address2;
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

  size = q_fsize (filename) - SWC_HEADER_LEN;
  printf ("Send: %d Bytes\n", size);
  fseek (file, SWC_HEADER_LEN, SEEK_SET);       // skip the header

  printf ("Press q to abort\n\n");
  if (sub ())
    {
      address1 = 0;
      address2 = 0xa000;
    }
  else
    {
      address1 = mram ();
      address2 = 0x8000;
    }

  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      ffe_send_command (5, address1, 0);
      if (address2 == 0x8000)
        ffe_send_command0 (0xc010, 1);
      ffe_send_block (address2, buffer, bytesread);
      address1++;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }
  ffe_send_command (6, 3, 0);

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_read_cart_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer, byte;
  int bytesreceived = 0, size;
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

  size = receive_rom_info (buffer, 0);
  if (size == 0)
    {
      fprintf (stderr, "ERROR: There is no cartridge present in the Super Wild Card\n");
      fclose (file);
      remove (filename);
      exit (1);
    }

  ffe_send_command (5, 3, 0);                   // detect cartridge SRAM size because
  ffe_send_command0 (0xe00c, 0);                //  we don't want to read too few data
  byte = ffe_send_command1 (0xbfd8);

  size = MAX ((byte ? 1 << (byte + 10) : 0), 32 * 1024);
  printf ("Receive: %d Bytes\n", size);

  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 5;
  fwrite (buffer, 1, SWC_HEADER_LEN, file);

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00c, 0);
//  ffe_send_command0 (0xc008, 0);

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = hirom ? 0x2c3 : 0x1c0;

  starttime = time (NULL);
  while (bytesreceived < size)
    {
      ffe_send_command (5, address, 0);
      ffe_receive_block (hirom ? 0x6000 : 0x2000, buffer, BUFFERSIZE);
      fwrite (buffer, 1, BUFFERSIZE, file);
      address += hirom ? 4 : 1;

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
swc_write_cart_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer, byte;
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

  size = receive_rom_info (buffer, 0);
  if (size == 0)
    {
      fprintf (stderr, "ERROR: There is no cartridge present in the Super Wild Card\n");
      fclose (file);
      remove (filename);
      exit (1);
    }

  ffe_send_command (5, 3, 0);                   // detect cartridge SRAM size because we don't
  ffe_send_command0 (0xe00c, 0);                //  want to write more data than necessary
  byte = ffe_send_command1 (0xbfd8);

  size = q_fsize (filename) - SWC_HEADER_LEN;   // SWC SRAM is 4*8 kB, emu SRAM often not
  size = MIN ((byte ? 1 << (byte + 10) : 0), size);

  printf ("Send: %d Bytes\n", size);
  fseek (file, SWC_HEADER_LEN, SEEK_SET);       // skip the header

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00c, 0);
//  ffe_send_command0 (0xc008, 0);

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = hirom ? 0x2c3 : 0x1c0;

  starttime = time (NULL);
  while ((bytessend < size) && (bytesread = fread (buffer, 1, MIN (size, BUFFERSIZE), file)))
    {
      ffe_send_command (5, address, 0);
      ffe_send_block (hirom ? 0x6000 : 0x2000, buffer, bytesread);
      address += hirom ? 4 : 1;

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
