/*
swc.c - Super Wild Card support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh
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

#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../misc.h"
#include "../ucon64.h"
#include "../ucon64_db.h"
#include "../ucon64_misc.h"
#include "swc.h"


#define INPUT_MASK      0x78
#define IBUSY_BIT       0x80
#define STROBE_BIT      1

#define N_TRY_MAX       65536                   // # times to test if SWC ready

#define BUFFERSIZE      8192                    // don't change, only 8192 works!
#define HEADERSIZE      512                     // SWC header is 512 bytes

static void init_io (unsigned int port);
static void checkabort (int status);
static void send_block (unsigned short address, unsigned char *buffer, int len);
static void send_command0 (unsigned short address, unsigned char byte);
static void send_command (unsigned char command_code, unsigned short a, unsigned short l);
static void sendb (unsigned char byte);
static int receive_rom_info (unsigned char *buffer);
static int get_rom_size (unsigned char *info_block);
static int check1 (unsigned char *info_block, int index);
static int check2 (unsigned char *info_block, int index, unsigned char value);
static int check3 (unsigned char *info_block, int index1, int index2, int size);
static unsigned char get_emu_mode_select (unsigned char byte, int size);
static void receive_block (unsigned short address, unsigned char *buffer, int len);
static unsigned char receiveb (void);
static inline unsigned char wait_while_busy (void);
static inline void wait_for_ready (void);

static int swc_port, hirom;                     // `hirom' was `special'

void
init_io (unsigned int port)
/*
  - sets static global `swc_port'. Then the send/receive functions don't need to pass `port' all
    the way to sendb()/receiveb().
  - calls init_conio(). Necessary for kbhit() and DOS-like behaviour of getch().
*/
{
  swc_port = port;
  if (swc_port != 0x3bc && swc_port != 0x378 && swc_port != 0x278)
    {
      printf ("PORT must be 0x3bc, 0x378 or 0x278\n"); // stdout for frontend
      exit (1);
    }

#if     defined __UNIX__ || defined __BEOS__
  init_conio ();
#endif

  printf ("Using I/O port 0x%x\n", swc_port);
}

int
swc_write_rom (char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend, totalblocks, blocksdone = 0, emu_mode_select;
  unsigned short address;
  struct stat fstate;
  time_t starttime;

  init_io (parport);

  if ((file = fopen (filename, "rb")) == NULL)
    {
      printf ("Can't open %s for reading\n", filename); // stdout for frontend
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      printf ("Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE); // stdout for frontend
      exit (1);
    }

  stat (filename, &fstate);
  printf ("Send: %d Bytes (%.4f Mb)\n", (int) fstate.st_size, (float) fstate.st_size / MBIT);

  send_command0 (0xc008, 0);
  fread (buffer, 1, HEADERSIZE, file);
  send_command (5, 0, 0);
  send_block (0x400, buffer, HEADERSIZE);       // send header
  bytessend = HEADERSIZE;

  emu_mode_select = buffer[2];                  // this byte is needed later
  if (buffer[3] & 0x80)                         // Pro Fighter (FIG) HiROM dump
    emu_mode_select |= 0x30;                    // set bit 5&4 (SRAM & DRAM mem map mode 21)
  if ((emu_mode_select & 0x0c) == 0x0c)         // 0x0c == no SRAM; we use the header, so
    {                                           //  that the user can override this
      if (emu_mode_select & 0x10)               // bit 4 == 1 => DRAM mode 21 (HiROM)
        emu_mode_select &= ~0x20;               // disable SRAM by setting SRAM mem map mode 20
      else                                      // bit 4 == 0 => DRAM mode 20 (LoROM)
        emu_mode_select |= 0x20;                // disable SRAM by setting SRAM mem map mode 21
    }

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = 0x200;                              // vgs '00 uses 0x200, vgs '96 uses 0,
  starttime = time (NULL);                      //  but then some ROMs don't work
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      send_command0 (0xc010, blocksdone >> 9);  // only 3 ROM dumps exist where 2nd arg != 0
      send_command (5, address, 0);
      send_block (0x8000, buffer, bytesread);
      address++;
      blocksdone++;

      bytessend += bytesread;
      ucon64_gauge (&rom, starttime, bytessend, fstate.st_size);
      checkabort (2);
    }

  if (blocksdone > 0x200)                       // ROM dump > 512 8KB blocks (=32Mb (=4MB))
    send_command0 (0xc010, 2);

  send_command (5, 0, 0);
  totalblocks = (fstate.st_size - HEADERSIZE + BUFFERSIZE - 1) / BUFFERSIZE; // round up
  send_command (6, 5 | (totalblocks << 8), totalblocks >> 8); // bytes: 6, 5, #8K L, #8K H, 0
  send_command (6, 1 | (emu_mode_select << 8), 0);

  wait_for_ready ();
  outportb (swc_port + PARPORT_DATA, 0);
  outportb (swc_port + PARPORT_CONTROL, inportb (swc_port + PARPORT_CONTROL) ^ STROBE_BIT); // invert strobe

  free (buffer);
  fclose (file);
  return 0;
}

int
swc_write_sram (char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend = 0, size;
  unsigned short address;
  struct stat fstate;
  time_t starttime;

  init_io (parport);

  if ((file = fopen (filename, "rb")) == NULL)
    {
      printf ("Can't open %s for reading\n", filename); // stdout for frontend
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      printf ("Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE); // stdout for frontend
      exit (1);
    }

  stat (filename, &fstate);                     // SWC SRAM is 4*8KB, emu SRAM often not
  size = fstate.st_size - HEADERSIZE;
  printf ("Send: %d Bytes\n", size);
  fseek (file, HEADERSIZE, SEEK_SET);           // skip the header

  send_command (5, 0, 0);
  send_command0 (0xe00d, 0);
  send_command0 (0xc008, 0);

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = 0x100;
  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      send_command (5, address, 0);
      send_block (0x2000, buffer, bytesread);
      address++;

      bytessend += bytesread;
      ucon64_gauge (&rom, starttime, bytessend, size);
      checkabort (2);
    }

  free (buffer);
  fclose (file);
  return 0;
}

void
send_block (unsigned short address, unsigned char *buffer, int len)
{
  int checksum = 0x81, n;

  send_command (0, address, len);
  for (n = 0; n < len; n++)
    {
      sendb (buffer[n]);
      checksum ^= buffer[n];
    }
  sendb (checksum);
}

void
send_command0 (unsigned short address, unsigned char byte)
// command 0 for 1 byte
{
  send_command (0, address, 1);
  sendb (byte);
  sendb (0x81 ^ byte);
}

void
send_command (unsigned char command_code, unsigned short a, unsigned short l)
{
  sendb (0xd5);
  sendb (0xaa);
  sendb (0x96);
  sendb (command_code);
  sendb (a);                                    // low byte
  sendb (a >> 8);                               // high byte
  sendb (l);                                    // low byte
  sendb (l >> 8);                               // high byte
  sendb (0x81 ^ command_code ^ a ^ (a >> 8) ^ l ^ (l >> 8)); // checksum
}

void
sendb (unsigned char byte)
{
  wait_for_ready ();
  outportb (swc_port + PARPORT_DATA, byte);
  outportb (swc_port + PARPORT_CONTROL, inportb (swc_port + PARPORT_CONTROL) ^ STROBE_BIT); // invert strobe
  wait_for_ready ();                            // necessary if followed by receiveb()
}

int
swc_read_rom (char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer, byte;
  int n, size, blocksleft, bytesreceived = 0;
  unsigned short address1, address2;
  time_t starttime;

  init_io (parport);

  if ((file = fopen (filename, "wb")) == NULL)
    {
      printf ("Can't open %s for writing\n", filename); // stdout for frontend
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      printf ("Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE); // stdout for frontend
      exit (1);
    }

  size = receive_rom_info (buffer);
  if (size == 0)
    {
      printf ("There is no cartridge present in the Super Wild Card\n"); // stdout for frontend
      fclose (file);
      remove (filename);
      exit (1);
    }
  blocksleft = size * 16;                       // 1 Mb (128KB) unit == 16 8KB units
  printf ("Receive: %d Bytes (%.4f Mb)\n", size * MBIT, (float) size);
  size *= MBIT;                                 // size in bytes for ucon64_gauge() below

  send_command (5, 0, 0);
  send_command0 (0xe00c, 0);
  send_command0 (0xe003, 0);
  send_command (1, 0xbfd8, 1);
  byte = receiveb ();
  if ((0x81 ^ byte) != receiveb ())
    printf ("received data is corrupt\n");

  buffer[2] = get_emu_mode_select (byte, blocksleft / 16);
  fwrite (buffer, 1, HEADERSIZE, file);         // write header (other necessary fields are
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
              send_command (5, address1, 0);
              receive_block (0x2000, buffer, BUFFERSIZE);
              address1++;
              fwrite (buffer, 1, BUFFERSIZE, file);

              bytesreceived += BUFFERSIZE;
              ucon64_gauge (&rom, starttime, bytesreceived, size);
              checkabort (2);
            }
        }

      for (n = 0; n < 4; n++)
        {
          send_command (5, address2, 0);
          receive_block (0xa000, buffer, BUFFERSIZE);
          blocksleft--;
          address2++;
          fwrite (buffer, 1, BUFFERSIZE, file);

          bytesreceived += BUFFERSIZE;
          ucon64_gauge (&rom, starttime, bytesreceived, size);
          checkabort (2);
        }
    }
  send_command (5, 0, 0);

  free (buffer);
  fclose (file);
  return 0;
}

#if BUFFERSIZE < HEADERSIZE
#error receive_rom_info() and swc_read_sram() expect BUFFERSIZE to be at least \
       HEADERSIZE bytes.
#endif
int
receive_rom_info (unsigned char *buffer)
/*
  - returns size of ROM in Mb (128KB) units
  - returns ROM header in buffer (index 2 (emulation mode select) is not yet
    filled in)
  - sets global `hirom'
*/
{
  int n, m, size;
  unsigned short address;
  unsigned char byte;

  send_command0 (0xe00c, 0);
  send_command (5, 3, 0);
  send_command (1, 0xbfd5, 1);
  byte = receiveb ();
  if ((0x81 ^ byte) != receiveb ())
    printf ("received data is corrupt\n");
  hirom = byte & 1;                             // Caz (vgs '96) does (byte & 0x21) == 0x21 ? 1 : 0;

  address = 0x200;
  for (n = 0; n < HEADERSIZE; n++)
    {
      for (m = 0; m < 65536; m++)               // a delay is necessary here
        ;
      send_command (5, address, 0);
      send_command (1, 0xa0a0, 1);
      buffer[n] = receiveb ();
      if ((0x81 ^ buffer[n]) != receiveb ())
        printf ("received data is corrupt\n");

      address++;
    }

  size = get_rom_size (buffer);
  if (hirom)
    size <<= 1;

  memset (buffer, 0, HEADERSIZE);
  buffer[0] = size << 4 & 0xff;                 // *16 for 8KB units; low byte
  buffer[1] = size >> 4;                        // *16 for 8KB units /256 for high byte
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

int
swc_read_sram (char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int blocksleft, bytesreceived = 0;
  unsigned short address;
  time_t starttime;

  init_io (parport);

  if ((file = fopen (filename, "wb")) == NULL)
    {
      printf ("Can't open %s for writing\n", filename); // stdout for frontend
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      printf ("Not enough memory for file buffer (%d bytes)\n", BUFFERSIZE); // stdout for frontend
      exit (1);
    }

  printf ("Receive: %d Bytes\n", 32 * 1024);
  memset (buffer, 0, HEADERSIZE);
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 5;
  fwrite (buffer, 1, HEADERSIZE, file);

  send_command (5, 0, 0);
  send_command0 (0xe00d, 0);
  send_command0 (0xc008, 0);

  printf ("Press q to abort\n\n");              // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  blocksleft = 4;                               // SRAM is 4*8KB
  address = 0x100;
  starttime = time (NULL);
  while (blocksleft > 0)
    {
      send_command (5, address, 0);
      receive_block (0x2000, buffer, BUFFERSIZE);
      blocksleft--;
      address++;
      fwrite (buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (&rom, starttime, bytesreceived, 32 * 1024);
      checkabort (2);
    }

  free (buffer);
  fclose (file);
  return 0;
}

void
receive_block (unsigned short address, unsigned char *buffer, int len)
{
  int checksum = 0x81, n, m;

  send_command (1, address, len);
  for (n = 0; n < len; n++)
    {
      buffer[n] = receiveb ();
      checksum ^= buffer[n];
    }
  if (checksum != receiveb ())
    printf ("\nreceived data is corrupt\n");

  for (m = 0; m < 65536; m++)                   // a delay is necessary here
    ;
}

unsigned char
receiveb (void)
{
  unsigned char byte;

  byte = (wait_while_busy () & INPUT_MASK) >> 3; // receive low nibble
  outportb (swc_port + PARPORT_CONTROL, inportb (swc_port + PARPORT_CONTROL) ^ STROBE_BIT); // invert strobe
  byte |= (wait_while_busy () & INPUT_MASK) << 1; // receive high nibble
  outportb (swc_port + PARPORT_CONTROL, inportb (swc_port + PARPORT_CONTROL) ^ STROBE_BIT); // invert strobe

  return byte;
}

unsigned char
wait_while_busy (void)
{
  unsigned char input;
  int n_try = 0;

  do
    {
      input = inportb (swc_port + PARPORT_STATUS);
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
      fprintf (STDERR, "The Super Wild Card is not ready\n" // yes, "ready" :-)
               "Turn it off for a few seconds then turn it on and try again\n");
      exit (1);
    }
#endif

  return input;
}

void
wait_for_ready (void)
{
  unsigned char input;
  int n_try = 0;

  do
    {
      input = inportb (swc_port + PARPORT_STATUS);
      n_try++;
    }
  while (!(input & IBUSY_BIT) && n_try < N_TRY_MAX);

#if 0
  if (n_try >= N_TRY_MAX)
    {
      fprintf (STDERR, "The Super Wild Card is not ready\n"
               "Turn it off for a few seconds then turn it on and try again\n");
      exit (1);
    }
#endif
}

void
checkabort (int status)
{
  if (((!rom.frontend) ? kbhit () : 0) && getch () == 'q')
    {
      puts ("\nProgram aborted");
      exit (status);
    }
//  send_command (5, 0, 0);                       // vgs: when sending/receiving a ROM
}

void
swc_unlock (unsigned int parport)
/*
  "Unlock" the SWC. However, just starting to send, then stopping with ^C,
  gives the same result.
*/
{
  init_io (parport);

  send_command (6, 0, 0);
}

int
swc_usage (int argc, char *argv[])
{
  printf ("%s\n", swc_TITLE);

  printf
    ("  -xswc         send/receive ROM to/from Super Wild Card*/(all)SWC; $FILE=PORT\n"
     "                receives automatically when $ROM does not exist\n"
     "                Press q to abort ^C will cause invalid state of backup unit\n"
     "  -xswcs        send/receive SRAM to/from Super Wild Card*/(all)SWC; $FILE=PORT\n"
     "                receives automatically when $ROM(=SRAM) does not exist\n"
     "                Press q to abort ^C will cause invalid state of backup unit\n");

  printf ("\n"
          "                You only need to specify PORT if uCON64 doesn't detect the\n"
          "                (right) parallel port. If that is the case give a hardware\n"
          "                address, for example:\n"
          "                ucon64 -xswc \"Super Mario World (U).swc\" 0x378\n");
  printf ("\n"
          "                In order to connect the Super Wild Card to a PC's parallel port\n"
          "                you need a standard bidirectional parallel cable like for the\n"
          "                most backup units\n"
/*
                           , i.e. a cable with male DB-25\n"
          "                connectors at both ends where the pins are connected in the\n"
          "                following way:\n"
          "                pin 1  <-> pin 1\n"
          "                pin 2  <-> pin 2\n"
          "                pin 3  <-> pin 3\n"
          "                pin 4  <-> pin 4\n"
          "                pin 5  <-> pin 5\n"
          "                pin 6  <-> pin 6\n"
          "                pin 7  <-> pin 7\n"
          "                pin 8  <-> pin 8\n"
          "                pin 9  <-> pin 9\n"
          "                pin 10 <-> pin 10\n"
          "                pin 11 <-> pin 11\n"
          "                pin 12 <-> pin 12\n"
          "                pin 13 <-> pin 13\n"
          "                pin 15 <-> pin 15\n"
          "                pin 25 <-> pin 25\n"
          "                Pins 2-9 are used for output, pins 10-13 & 15 for input and pin\n"
          "                25 is ground. The other pins may be left unconnected.\n"
*/
         );

  return 0;
}
