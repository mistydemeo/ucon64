/*
gd.c - Game Doctor support for uCON64

Copyright (c) 2002 - 2003              John Weidman
Copyright (c) 2002 - 2006, 2015 - 2019 dbjh


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
#include <ctype.h>
#include <stdlib.h>
#include "misc/archive.h"
#include "misc/file.h"
#include "misc/misc.h"
#include "misc/parallel.h"
#include "misc/property.h"
#include "misc/string.h"
#include "misc/term.h"
#include "ucon64_misc.h"
#include "console/snes.h"                       // for snes_gd_make_names() &
#include "backup/gd.h"                          //  snes_get_snes_hirom()


#ifdef  USE_PARALLEL
static st_ucon64_obj_t gd_obj[] =
  {
    {UCON64_SNES, WF_DEFAULT | WF_STOP | WF_NO_ROM},
    {UCON64_SNES, WF_STOP | WF_NO_ROM}
  };
#endif

const st_getopt2_t gd_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Game Doctor SF3(SF6/SF7)/Professor SF(SF II)"/*"19XX Bung Enterprises Ltd http://www.bung.com.hk"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xgd3", 0, 0, UCON64_XGD3, // supports split files
      NULL, "send ROM to Game Doctor SF3/SF6/SF7; " OPTION_LONG_S "port" OPTARG_S "PORT\n"
      "this option uses the Game Doctor SF3 protocol",
      &gd_obj[0]
    },
    {
      "xgd6", 0, 0, UCON64_XGD6,
#if 1 // dumping is not yet supported
      NULL, "send ROM to Game Doctor SF6/SF7; " OPTION_LONG_S "port" OPTARG_S "PORT\n" // supports split files
#else
      NULL, "send/receive ROM to/from Game Doctor SF6/SF7; " OPTION_LONG_S "port" OPTARG_S "PORT\n"
      "receives automatically when ROM does not exist\n"
#endif
      "this option uses the Game Doctor SF6 protocol",
      &gd_obj[0]
    },
    {
      "xgd3s", 0, 0, UCON64_XGD3S,
      NULL, "send SRAM to Game Doctor SF3/SF6/SF7; " OPTION_LONG_S "port" OPTARG_S "PORT",
      &gd_obj[1]
    },
    // --xgd3r should remain hidden until receiving works
    {
      "xgd3r", 0, 0, UCON64_XGD3R,
      NULL, NULL,
      &gd_obj[1]
    },
    {
      "xgd6s", 0, 0, UCON64_XGD6S,
      NULL, "send/receive SRAM to/from Game Doctor SF6/SF7; " OPTION_LONG_S "port" OPTARG_S "PORT\n"
      "receives automatically when SRAM does not exist",
      &gd_obj[1]
    },
    {
      "xgd6r", 0, 0, UCON64_XGD6R,
      NULL, "send/receive saver (RTS) data to/from Game Doctor SF6/SF7;\n" OPTION_LONG_S "port" OPTARG_S "PORT\n"
      "receives automatically when saver file does not exist",
      &gd_obj[1]
    },
#endif // USE_PARALLEL
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

#define BUFFERSIZE 8192
#define GD_OK 0
#define GD_ERROR 1
#define GD3_PROLOG_STRING "DSF3"
#define GD6_READ_PROLOG_STRING "GD6R"           // GD reading, PC writing
#define GD6_WRITE_PROLOG_STRING "GD6W"          // GD writing, PC reading
#define GD6_TIMEOUT_ATTEMPTS 0x4000
#define GD6_RX_SYNC_TIMEOUT_ATTEMPTS 0x2000
#define GD6_SYNC_RETRIES 16

#define STOCKPPDEV_MSG "WARNING: This will not work with a stock ppdev on PC. See the FAQ, question 55"

static void deinit_io (void);
static int gd_write_rom (const char *filename, unsigned short parport,
                         st_ucon64_nfo_t *rominfo, const char *prolog_str);
static int gd_write_sram (const char *filename, unsigned short parport,
                          const char *prolog_str);
static int gd_write_saver (const char *filename, unsigned short parport,
                           const char *prolog_str);


typedef struct st_gd3_memory_unit
{
  char name[12];                                // exact size is 11 chars, but I'll
//  unsigned char *data;                        //  add one extra for string terminator
  unsigned int size;                            // usually either 0x100000 or 0x80000
} st_gd3_memory_unit_t;

typedef struct st_add_filename_data
{
  unsigned int index;
  char **names;
} st_add_filename_data_t;

static int (*gd_send_prolog_byte) (unsigned char data);
static int (*gd_send_prolog_bytes) (unsigned char *data, unsigned int len);
static int (*gd_send_bytes) (unsigned char *data, unsigned int len);
static st_gd3_memory_unit_t gd3_dram_unit[GD3_MAX_UNITS];
static unsigned short gd_port;
static unsigned int gd6_send_byte_delay, gd_bytessent, gd_fsize;
static time_t gd_starttime;
static char gd_destfname[FILENAME_MAX] = "";
static FILE *gd_destfile;


static void
init_io (unsigned short port)
{
  gd_port = port;

#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  init_conio ();
#endif
  if (register_func (deinit_io) == -1)
    {
      fputs ("ERROR: Could not register function with register_func()\n", stderr);
      exit (1);
    }

  parport_print_info ();

  gd6_send_byte_delay = get_property_int (ucon64.configfile, "gd6_send_byte_delay");
}


static void
deinit_io (void)
{
  // put possible transfer cleanup stuff here
#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  deinit_conio ();
#endif
}


static void
io_error (const char *func_name)
{
  fflush (stdout);
  fprintf (stderr,
           "ERROR: Communication with Game Doctor failed in function %s\n",
           func_name);
  fflush (stderr);
  // calling fflush() seems to be necessary in Msys in order to make the error
  //  message be displayed before the "Removing <filename>" message
  exit (1);
}


static void
gd_checkabort (int status)
{
  if ((!ucon64.frontend ? kbhit () : 0) && getch () == 'q')
    {
      puts ("\nProgram aborted");
      exit (status);
    }
}


static void
remove_destfile (void)
{
  if (gd_destfname[0])
    {
      printf ("Removing %s\n", gd_destfname);
      fclose (gd_destfile);
      remove (gd_destfname);
      gd_destfname[0] = '\0';
    }
}


static int
gd3_send_prolog_byte (unsigned char data)
/*
  Prolog specific data output routine.
  We could probably get away with using the general routine but the
  transfer program I (JW) traced to analyze the protocol did this for
  the bytes used to set up the transfer so here it is.
*/
{
  // wait until SF3 is not busy
  do
    {
      if ((inportb (gd_port + PARPORT_STATUS) & 0x08) == 0)
        return GD_ERROR;
    }
  while ((inportb (gd_port + PARPORT_STATUS) & 0x80) == 0);

  outportb (gd_port + PARPORT_DATA, data);      // set data
  outportb (gd_port + PARPORT_CONTROL, 5);      // clock data out to SF3
  inportb (gd_port + PARPORT_CONTROL);          // let data "settle down"
  outportb (gd_port + PARPORT_CONTROL, 4);

  return GD_OK;
}


static int
gd3_send_prolog_bytes (unsigned char *data, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
    if (gd3_send_prolog_byte (data[i]) == GD_ERROR)
      return GD_ERROR;
  return GD_OK;
}


static void
gd3_send_byte (unsigned char data)
/*
  General data output routine.
  Use this routine for sending ROM data bytes to the Game Doctor SF3 (SF6/SF7
  too).
*/
{
  // wait until SF3 is not busy
  while ((inportb (gd_port + PARPORT_STATUS) & 0x80) == 0)
    ;

  outportb (gd_port + PARPORT_DATA, data);      // set data
  outportb (gd_port + PARPORT_CONTROL, 5);      // clock data out to SF3
  inportb (gd_port + PARPORT_CONTROL);          // let data "settle down"
  outportb (gd_port + PARPORT_CONTROL, 4);
}


static int
gd3_send_bytes (unsigned char *data, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
    {
      gd3_send_byte (data[i]);
      gd_bytessent++;
      if ((gd_bytessent - GD_HEADER_LEN) % 8192 == 0)
        {
          ucon64_gauge (gd_starttime, gd_bytessent, gd_fsize);
          gd_checkabort (2);                    // 2 to return something other than 1
        }
    }
  return GD_OK;
}


static inline void
microwait2 (int nmicros)
{
#if     (defined __unix__ && !defined __MSDOS__) || defined __APPLE__ || \
        defined __BEOS__ || defined _WIN32
  microwait (nmicros);
#elif   defined __i386__
  /*
    On x86, the in and out instructions are slow by themselves (without actual
    hardware mapped), even on modern hardware. I have read claims of about 1
    microsecond (for both in and out) and a rough test performed by myself
    suggested around 600 nanoseconds for inb for an unmapped address.
  */
  unsigned int n;

  for (n = 0; n < nmicros * 2; n++)
    inportb (gd_port + PARPORT_STATUS);
#endif
}


static int
gd6_sync_hardware (void)
// sets the SF7 up for an SF6/SF7 protocol transfer
{
  unsigned int retries;
  volatile int delay;

  for (retries = GD6_SYNC_RETRIES; retries > 0; retries--)
    {
      unsigned int timeout = GD6_TIMEOUT_ATTEMPTS;

      outportb (gd_port + PARPORT_CONTROL, 4);
      outportb (gd_port + PARPORT_DATA, 0);
      outportb (gd_port + PARPORT_CONTROL, 4);

      for (delay = 0x1000; delay > 0; delay--)  // a delay may not be necessary here
        ;

      outportb (gd_port + PARPORT_DATA, 0xaa);
      outportb (gd_port + PARPORT_CONTROL, 0);

      if (gd6_send_byte_delay)
        microwait2 (2000);
      else
        {
          while ((inportb (gd_port + PARPORT_CONTROL) & 8) == 0)
            if (--timeout == 0)
              break;
          if (timeout == 0)
            continue;
        }

      outportb (gd_port + PARPORT_CONTROL, 4);

      if (gd6_send_byte_delay)
        microwait2 (2000);
      else
        {
          while ((inportb (gd_port + PARPORT_CONTROL) & 8) != 0)
            if (--timeout == 0)
              break;
          if (timeout == 0)
            continue;
        }

      outportb (gd_port + PARPORT_DATA, 0x55);
      outportb (gd_port + PARPORT_CONTROL, 0);

      if (gd6_send_byte_delay)
        microwait2 (2000);
      else
        {
          while ((inportb (gd_port + PARPORT_CONTROL) & 8) == 0)
            if (--timeout == 0)
              break;
          if (timeout == 0)
            continue;
        }

      outportb (gd_port + PARPORT_CONTROL, 4);

      if (gd6_send_byte_delay)
        microwait2 (2000);
      else
        {
          while ((inportb (gd_port + PARPORT_CONTROL) & 8) != 0)
            if (--timeout == 0)
              break;
          if (timeout == 0)
            continue;
        }

      return GD_OK;
    }
  return GD_ERROR;
}


static int
gd6_sync_receive_start (void)
// sync with the start of the received data
{
  unsigned int timeout = GD6_RX_SYNC_TIMEOUT_ATTEMPTS;

  if (gd6_send_byte_delay)
    microwait2 (2000);
  else
    while (((inportb (gd_port + PARPORT_CONTROL) & 3) != 3) &&
           ((inportb (gd_port + PARPORT_CONTROL) & 3) != 0))
      if (--timeout == 0)
        return GD_ERROR;

  outportb (gd_port + PARPORT_DATA, 0);

  timeout = GD6_RX_SYNC_TIMEOUT_ATTEMPTS;
  while ((inportb (gd_port + PARPORT_STATUS) & 0x80) != 0)
    if (--timeout == 0)
      return GD_ERROR;

  return GD_OK;
}


static inline int
gd6_send_byte (unsigned char data)
{
  static unsigned char send_toggle = 0;
  unsigned int timeout = 0x1e0000;

  if (gd6_send_byte_delay)
    microwait2 (gd6_send_byte_delay);
  else
    while (((inportb (gd_port + PARPORT_CONTROL) >> 1) & 1) != send_toggle)
      if (--timeout == 0)
        return GD_ERROR;

  send_toggle ^= 1;
  outportb (gd_port + PARPORT_DATA, data);
  outportb (gd_port + PARPORT_CONTROL, 4 | send_toggle);

  return GD_OK;
}


static int
gd6_send_prolog_bytes (unsigned char *data, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
    if (gd6_send_byte (data[i]) != GD_OK)
      return GD_ERROR;
  return GD_OK;
}


static int
gd6_send_bytes (unsigned char *data, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
    {
      if (gd6_send_byte (data[i]) != GD_OK)
        return GD_ERROR;

      gd_bytessent++;
      if ((gd_bytessent - GD_HEADER_LEN) % 8192 == 0)
        {
          ucon64_gauge (gd_starttime, gd_bytessent, gd_fsize);
          gd_checkabort (2);                    // 2 to return something other than 1
        }
    }
  return GD_OK;
}


static int
gd6_receive_bytes (unsigned char *buffer, unsigned int len)
{
  unsigned int i, timeout = 0x1e0000;

  outportb (gd_port + PARPORT_DATA, 0x80);      // signal the SF6/SF7 to send the next nibble
  for (i = 0; i < len; i++)
    {
      unsigned char nibble1, nibble2;

      while ((inportb (gd_port + PARPORT_STATUS) & 0x80) == 0)
        if (--timeout == 0)
          return GD_ERROR;

      nibble1 = (inportb (gd_port + PARPORT_STATUS) >> 3) & 0x0f;
      outportb (gd_port + PARPORT_DATA, 0x00);  // signal the SF6/SF7 to send the next nibble

      while ((inportb (gd_port + PARPORT_STATUS) & 0x80) != 0)
        if (--timeout == 0)
          return GD_ERROR;

      nibble2 = (inportb (gd_port + PARPORT_STATUS) << 1) & 0xf0;
      buffer[i] = nibble2 | nibble1;
      outportb (gd_port + PARPORT_DATA, 0x80);
    }
  return GD_OK;
}


static int
gd_send_unit_prolog (unsigned char header, unsigned int size)
{
  if (gd_send_prolog_byte (0) == GD_ERROR ||
      gd_send_prolog_byte (header ? 2 : 0) == GD_ERROR ||
      gd_send_prolog_byte ((unsigned char) (size >> 16)) == GD_ERROR || // 0x10 = 8 Mbit
      gd_send_prolog_byte (0) == GD_ERROR)
    return GD_ERROR;
  else
    return GD_OK;
}


static void
gd_add_filename (const char *filename, void *cb_data)
{
  st_add_filename_data_t *add_filename_data = (st_add_filename_data_t *) cb_data;

  if (add_filename_data->index < GD3_MAX_UNITS)
    {
      char buf[12], *p;

      strncpy (buf, basename2 (filename), 11)[11] = '\0';
      p = strrchr (buf, '.');
      if (p)
        *p = '\0';
      strcpy (add_filename_data->names[add_filename_data->index], buf);
      add_filename_data->index++;
    }
}


int
gd3_read_rom (const char *filename, unsigned short parport)
{
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  fputs ("ERROR: The function for dumping a cartridge is not yet implemented for the SF3", stderr);
  return -1;
}


int
gd6_read_rom (const char *filename, unsigned short parport)
{
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  fputs ("ERROR: The function for dumping a cartridge is not yet implemented for the SF6", stderr);
  return -1;
}


int
gd3_write_rom (const char *filename, unsigned short parport, st_ucon64_nfo_t *rominfo)
{
  gd_send_prolog_byte = gd3_send_prolog_byte;   // for gd_send_unit_prolog()
  gd_send_prolog_bytes = gd3_send_prolog_bytes;
  gd_send_bytes = gd3_send_bytes;

  return gd_write_rom (filename, parport, rominfo, GD3_PROLOG_STRING);
}


int
gd6_write_rom (const char *filename, unsigned short parport, st_ucon64_nfo_t *rominfo)
{
  gd_send_prolog_byte = gd6_send_byte;          // for gd_send_unit_prolog()
  gd_send_prolog_bytes = gd6_send_prolog_bytes;
  gd_send_bytes = gd6_send_bytes;

  return gd_write_rom (filename, parport, rominfo, GD6_READ_PROLOG_STRING);
}


/*
  NOTE: On most Game Doctors the way you enter link mode to be able to upload
        the ROM to the unit is to hold down the R key on the controller while
        resetting the SNES. You will see the Game Doctor menu has a message that
        says "LINKING..".
  My Game Doctor SF7 V7.11 enters link mode automatically if it is connected to
  a parallel port -- no controller input is required. - dbjh
*/
static int
gd_write_rom (const char *filename, unsigned short parport, st_ucon64_nfo_t *rominfo,
              const char *prolog_str)
{
  FILE *file = NULL;
  unsigned char *buffer;
  char *names[GD3_MAX_UNITS], names_mem[GD3_MAX_UNITS][12] = { { 0 } },
       *filenames[GD3_MAX_UNITS], dir[FILENAME_MAX];
  unsigned int num_units, i, split, hirom = snes_get_snes_hirom (),
               gd6_protocol = !memcmp (prolog_str, GD6_READ_PROLOG_STRING, 4);
  st_add_filename_data_t add_filename_data = { 0, NULL };

  init_io (parport);

#ifdef  USE_PPDEV
  if (gd6_protocol && !gd6_send_byte_delay)
    puts (STOCKPPDEV_MSG);
#endif

  // we don't want to malloc() ridiculously small chunks (of 12 bytes)
  for (i = 0; i < GD3_MAX_UNITS; i++)
    names[i] = names_mem[i];
  add_filename_data.names = names;

  split = ucon64_testsplit (filename, gd_add_filename, &add_filename_data);
  if (UCON64_ISSET (ucon64.split) ? ucon64.split : (int) split)
    num_units = split;
  else
    {
      split = 0;
      num_units = snes_gd_make_names (filename, rominfo, names);
    }

  dirname2 (filename, dir);
  gd_fsize = 0;
  for (i = 0; i < num_units; i++)
    {
      unsigned int x;

      // No suffix is necessary but the name entry must be upper case and MUST
      //  be 11 characters long, padded at the end with spaces if necessary.
      memset (gd3_dram_unit[i].name, ' ', 11);  // "pad" with spaces
      gd3_dram_unit[i].name[11] = '\0';         // terminate string so we can print it (debug)
      // Use memcpy() instead of strcpy() so that the string terminator in
      //  names[i] won't be copied.
      memcpy (gd3_dram_unit[i].name, strupr (names[i]), strlen (names[i]));

      x = strlen (dir) + strlen (names[i]) + 6; // file sep., suffix, ASCII-z => 6
      if ((filenames[i] = (char *) malloc (x)) == NULL)
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], x);
          exit (1);
        }
      sprintf (filenames[i], "%s" DIR_SEPARATOR_S "%s.078", dir, names[i]); // should match with what code of -s does

      if (split)
        {
          x = fsizeof (filenames[i]);
          gd_fsize += x;
          gd3_dram_unit[i].size = x;
          if (i == 0)                           // correct for header of first file
            gd3_dram_unit[i].size -= GD_HEADER_LEN;
        }
      else
        {
          if (!gd_fsize)
            gd_fsize = (unsigned int) ucon64.file_size;
          if (hirom)
            gd3_dram_unit[i].size = (gd_fsize - GD_HEADER_LEN) / num_units;
          else
            {
              if ((i + 1) * 8 * MBIT <= gd_fsize - GD_HEADER_LEN)
                gd3_dram_unit[i].size = 8 * MBIT;
              else
                gd3_dram_unit[i].size = gd_fsize - GD_HEADER_LEN - i * 8 * MBIT;
            }
        }
    }

  if ((buffer = (unsigned char *) malloc (8 * MBIT)) == NULL)
    { // a DRAM unit can hold 8 Mbit at maximum
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], 8 * MBIT);
      exit (1);
    }

  printf ("Send: %d Bytes (%.4f Mb)\n", gd_fsize, (float) gd_fsize / MBIT);

  // Put this just before the real transfer begins or else the ETA won't be
  //  correct.
  gd_starttime = time (NULL);

  // send the ROM to the hardware
  if (gd6_protocol && gd6_sync_hardware () == GD_ERROR)
    io_error ("gd6_sync_hardware()");
  memcpy (buffer, prolog_str, 4);
  buffer[4] = (unsigned char) num_units;
  if (gd_send_prolog_bytes (buffer, 5) == GD_ERROR)
    io_error (gd6_protocol ? "gd6_send_prolog_bytes(5)" : "gd3_send_prolog_bytes(5)");

  puts ("Press q to abort\n");
  for (i = 0; i < num_units; i++)
    {
      unsigned char send_header = i == 0 ? 1 : 0;

#ifdef  DEBUG
      printf ("\nfilename (%d): \"%s\", ", split, split ? (char *) filenames[i] : filename);
      printf ("name: \"%s\", size: %d\n", gd3_dram_unit[i].name, gd3_dram_unit[i].size);
#endif
      if (split)
        {
          if ((file = fopen (filenames[i], "rb")) == NULL)
            {
              fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filenames[i]);
              exit (1);
            }
        }
      else
        if (file == NULL)                       // don't open the file more than once
          if ((file = fopen (filename, "rb")) == NULL)
            {
              fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
              exit (1);
            }
      /*
        When sending a "pre-split" file, opening the next file often causes
        enough of a delay for the GDSF7. Sending an unsplit file without an
        extra delay often results in the infamous "File Size Error !".
        Adding the delay also results in the transfer code catching an I/O
        error more often, instead of simply hanging (while the GDSF7 displays
        "File Size Error !"). The delay seems *much* more important for HiROM
        games than for LoROM games.
        There's more to the "File Size Error !" than this as the transfer
        reliability is still far from 100%, but the delay helps me a lot.
        Note that this applies to the SF3 protocol. Using the SF6 protocol does
        not work for me. - dbjh
      */
      wait2 (100);                              // 100 ms seems a good value

      if (gd_send_unit_prolog (send_header, gd3_dram_unit[i].size) == GD_ERROR)
        io_error ("gd_send_unit_prolog()");
      if (gd_send_prolog_bytes ((unsigned char *) gd3_dram_unit[i].name, 11) == GD_ERROR)
        io_error (gd6_protocol ? "gd6_send_prolog_bytes(11)" : "gd3_send_prolog_bytes(11)");
      if (send_header)
        {
          // send the Game Doctor 512 byte header
          fread (buffer, 1, GD_HEADER_LEN, file);
          if (gd_send_prolog_bytes (buffer, GD_HEADER_LEN) == GD_ERROR)
            io_error (gd6_protocol ? "gd6_send_prolog_bytes(512)" : "gd3_send_prolog_bytes(512)");
          gd_bytessent += GD_HEADER_LEN;
        }
      if (!split)                               // not pre-split -- have to split it ourselves
        {
          if (hirom)
            fseek (file, i * gd3_dram_unit[0].size + GD_HEADER_LEN, SEEK_SET);
          else
            fseek (file, i * 8 * MBIT + GD_HEADER_LEN, SEEK_SET);
        }
      fread (buffer, 1, gd3_dram_unit[i].size, file);
      if (gd_send_bytes (buffer, gd3_dram_unit[i].size) == GD_ERROR)
        io_error (gd6_protocol ? "gd6_send_bytes()" : "gd3_send_bytes()");

      if (split || i == num_units - 1)
        fclose (file);
    }

  for (i = 0; i < num_units; i++)
    free (filenames[i]);
  free (buffer);

  return 0;
}


int
gd3_read_sram (const char *filename, unsigned short parport)
{
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  fputs ("ERROR: The function for dumping SRAM is not yet implemented for the SF3", stderr);
  return -1;
}


int
gd6_read_sram (const char *filename, unsigned short parport)
{
  FILE *file;
  unsigned char *buffer;
  unsigned int retry = 0, transfer_size, len, bytesreceived = 0;
  time_t starttime;

  init_io (parport);

#ifdef  USE_PPDEV
  if (!gd6_send_byte_delay)
    puts (STOCKPPDEV_MSG);
#endif

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

  // be nice to the user and automatically remove the file on an error (or abortion)
  strcpy (gd_destfname, filename);
  gd_destfile = file;
  register_func (remove_destfile);

  for (;;)
    {
      if (gd6_sync_hardware () == GD_ERROR)
        {
          if (retry)
            fputc ('\n', stdout);
          io_error ("gd6_sync_hardware()");
        }
      if (gd6_send_prolog_bytes ((unsigned char *) GD6_WRITE_PROLOG_STRING, 4) == GD_ERROR)
        {
          if (retry)
            fputc ('\n', stdout);
          io_error ("gd6_send_prolog_bytes(4)");
        }

      /*
        The BRAM (SRAM) filename doesn't have to exactly match any game loaded
        in the SF7. It needs to match any valid Game Doctor filename AND have an
        extension of .B## (where # is a digit from 0-9).

        TODO: We might need to make a GD filename from the real one.
      */
      if (gd6_send_prolog_bytes ((unsigned char *) "SF16497 B00", 11) == GD_ERROR)
        {
          if (retry)
            fputc ('\n', stdout);
          io_error ("gd6_send_prolog_bytes(11)");
        }

      /*
        On Raspberry Pi 3B+, when microwait() was implemented using
        clock_nanosleep(CLOCK_MONOTONIC, ...) -xgd6s would successfully dump
        SRAM data only the first time after booting. After microwait() was
        implemented using only clock_gettime(CLOCK_MONOTONIC, ...) in order to
        improve its accuracy, it would never work. It worked again after adding
        calls to clock_nanosleep() during the transfer .
        On Raspberry Pi 2B the behavior is different in that only resetting the
        SNES was usually enough to be able to dump SRAM data again after having
        used -xgd6s once. However, sometimes resetting the SNES would no longer
        be enough and a reboot would be required. Another difference is that
        calling clock_nanosleep() during the transfer made no difference in the
        success rate of -xgd6s.
        On both Raspberry models first uploading either ROM or SRAM data using
        -xgd3 or -xgd3s respectively, would make dumping SRAM data with -xgd6s
        work again, once, provided there would be a delay of at least 3 seconds
        before dumping.
        Ultimately, the result proved to be restarting the transfer after the
        first call to gd6_receive_bytes() fails. This makes -xgd6s work at least
        as well as after first uploading data, if not better. It can easily take
        10 retries before a transfer succeeds. If the first call to
        gd6_receive_bytes() succeeds the transfer of the actual SRAM data will
        usually also succeed. Since gd6_send_byte_delay needs to be specified
        (the pi-parport does not support reading from the Control register),
        several opportunites offered by the GD to detect an error cannot be
        taken. The first opportunity that can be taken is offered by
        gd6_sync_receive_start(), but synchronization errors are more often
        detected in gd6_receive_bytes().
      */
      if (gd6_sync_receive_start () == GD_ERROR)
        {
          if (retry < GD6_SYNC_RETRIES)
            {
              retry++;
              printf ("\rWARNING: Failed to synchronize with Game Doctor. Retrying... (%d/%d)",
                      retry, GD6_SYNC_RETRIES);
              fflush (stdout);
              continue;
            }
          else
            {
              fputc ('\n', stdout);
              io_error ("gd6_sync_receive_start()");
            }
        }

      if (gd6_receive_bytes (buffer, 16) != GD_ERROR)
        {
          if (retry)
            fputc ('\n', stdout);
          break;
        }
      else if (retry < GD6_SYNC_RETRIES)
        {
          retry++;
          printf ("\rWARNING: Failed to synchronize with Game Doctor. Retrying... (%d/%d)",
                  retry, GD6_SYNC_RETRIES);
          fflush (stdout);
        }
      else
        {
          fputc ('\n', stdout);
          io_error ("gd6_receive_bytes(16)");
        }
    }

  transfer_size = buffer[1] | (buffer[2] << 8) | (buffer[3] << 16) | (buffer[4] << 24);
  if (transfer_size == 0)
    {
      fputs ("ERROR: SRAM transfer size from Game Doctor is 0 bytes\n", stderr);
      exit (1);
    }
  else if (transfer_size > 0x8000)
    {
      fprintf (stderr, "ERROR: SRAM transfer size from Game Doctor > 0x8000 bytes (0x%x bytes)\n",
               transfer_size);
      exit (1);
    }
  else if (transfer_size < 0x8000)
    printf ("WARNING: SRAM transfer size from Game Doctor < 0x8000 bytes (0x%x bytes)\n",
            transfer_size);

  printf ("Receive: %d Bytes\n"
          "Press q to abort\n\n", transfer_size);

  starttime = time (NULL);
  while (bytesreceived < transfer_size)
    {
      len = transfer_size >= bytesreceived + BUFFERSIZE ?
        BUFFERSIZE : transfer_size - bytesreceived;

      if (gd6_receive_bytes (buffer, len) == GD_ERROR)
        io_error ("gd6_receive_bytes()");
      fwrite (buffer, 1, len, file);

      bytesreceived += len;
      ucon64_gauge (starttime, bytesreceived, transfer_size);
      gd_checkabort (2);
    }

  unregister_func (remove_destfile);
  free (buffer);
  fclose (file);

  return 0;
}


int
gd3_write_sram (const char *filename, unsigned short parport)
{
  gd_send_prolog_bytes = gd3_send_prolog_bytes;
  gd_send_bytes = gd3_send_bytes;

  return gd_write_sram (filename, parport, GD3_PROLOG_STRING);
}


int
gd6_write_sram (const char *filename, unsigned short parport)
{
  gd_send_prolog_bytes = gd6_send_prolog_bytes;
  gd_send_bytes = gd6_send_bytes;

  return gd_write_sram (filename, parport, GD6_READ_PROLOG_STRING);
}


static int
gd_write_sram (const char *filename, unsigned short parport, const char *prolog_str)
{
  FILE *file;
  unsigned char *buffer;
  unsigned int bytesread, bytessent = 0, size, header_size,
               gd6_protocol = !memcmp (prolog_str, GD6_READ_PROLOG_STRING, 4);
  time_t starttime;

  init_io (parport);

#ifdef  USE_PPDEV
  if (gd6_protocol && !gd6_send_byte_delay)
    puts (STOCKPPDEV_MSG);
#endif

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

  size = (unsigned int) ucon64.file_size;       // GD SRAM is 4*8 KB, emu SRAM often not

  if (size == 0x8000)
    header_size = 0;
  else if (size == 0x8200)
    {
      header_size = 0x200;
      size = 0x8000;
    }
  else
    {
      fputs ("ERROR: Game Doctor SRAM file size must be 32768 or 33280 bytes\n",
             stderr);
      exit (1);
    }

  printf ("Send: %d Bytes\n", size);
  fseek (file, header_size, SEEK_SET);          // skip the header

  if (gd6_protocol && gd6_sync_hardware () == GD_ERROR)
    io_error ("gd6_sync_hardware()");
  memcpy (buffer, prolog_str, 4);
  buffer[4] = 1;
  if (gd_send_prolog_bytes (buffer, 5) == GD_ERROR)
    io_error (gd6_protocol ? "gd6_send_prolog_bytes(5)" : "gd3_send_prolog_bytes(5)");

  buffer[0] = 0x00;
  buffer[1] = 0x80;
  buffer[2] = 0x00;
  buffer[3] = 0x00;
  if (gd_send_prolog_bytes (buffer, 4) == GD_ERROR)
    io_error (gd6_protocol ? "gd6_send_prolog_bytes(4)" : "gd3_send_prolog_bytes(4)");

  /*
    The BRAM (SRAM) filename doesn't have to exactly match any game loaded in
    the SF7. It needs to match any valid Game Doctor filename AND have an
    extension of .B## (where # is a digit from 0-9).

    TODO: We might need to make a GD filename from the real one.
  */
  if (gd_send_prolog_bytes ((unsigned char *) "SF8123  B00", 11) == GD_ERROR)
    io_error (gd6_protocol ? "gd6_send_prolog_bytes(11)" : "gd3_send_prolog_bytes(11)");

  puts ("Press q to abort\n");                  // print here, NOT before first GD I/O,
                                                //  because if we get here q works ;-)
  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)) != 0)
    {
      if (gd_send_bytes (buffer, bytesread) == GD_ERROR)
        io_error (gd6_protocol ? "gd6_send_bytes()" : "gd3_send_bytes()");

      bytessent += bytesread;
      ucon64_gauge (starttime, bytessent, size);
      gd_checkabort (2);
    }

  free (buffer);
  fclose (file);

  return 0;
}


int
gd3_read_saver (const char *filename, unsigned short parport)
{
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  fputs ("ERROR: The function for dumping saver data is not yet implemented for the SF3", stderr);
  return -1;
}


int
gd6_read_saver (const char *filename, unsigned short parport)
{
  FILE *file;
  unsigned char *buffer;
  unsigned int retry = 0, transfer_size, len, bytesreceived = 0;
  time_t starttime;

  init_io (parport);

#ifdef  USE_PPDEV
  if (!gd6_send_byte_delay)
    puts (STOCKPPDEV_MSG);
#endif

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

  // be nice to the user and automatically remove the file on an error (or abortion)
  strcpy (gd_destfname, filename);
  gd_destfile = file;
  register_func (remove_destfile);

  for (;;)
    {
      if (gd6_sync_hardware () == GD_ERROR)
        {
          if (retry)
            fputc ('\n', stdout);
          io_error ("gd6_sync_hardware()");
        }
      if (gd6_send_prolog_bytes ((unsigned char *) GD6_WRITE_PROLOG_STRING, 4) == GD_ERROR)
        {
          if (retry)
            fputc ('\n', stdout);
          io_error ("gd6_send_prolog_bytes(4)");
        }

      /*
        TODO: Graceful handling of an abort because of a name error?
              Currently we fail with a generic error.

        TODO: We could make a GD filename from the real one but a valid dummy
              name seems to work OK here. The user must have the proper game
              selected in the SF7 menu even if the real name is used.
      */
      if (gd6_send_prolog_bytes ((unsigned char *) "SF16497 S00", 11) == GD_ERROR)
        {
          if (retry)
            fputc ('\n', stdout);
          io_error ("gd6_send_prolog_bytes(11)");
        }

      if (gd6_sync_receive_start () == GD_ERROR)
        {
          if (retry < GD6_SYNC_RETRIES)
            {
              retry++;
              printf ("\rWARNING: Failed to synchronize with Game Doctor. Retrying... (%d/%d)",
                      retry, GD6_SYNC_RETRIES);
              fflush (stdout);
              continue;
            }
          else
            {
              fputc ('\n', stdout);
              io_error ("gd6_sync_receive_start()");
            }
        }

      if (gd6_receive_bytes (buffer, 16) != GD_ERROR)
        {
          if (retry)
            fputc ('\n', stdout);
          break;
        }
      else if (retry < GD6_SYNC_RETRIES)
        {
          retry++;
          printf ("\rWARNING: Failed to synchronize with Game Doctor. Retrying... (%d/%d)",
                  retry, GD6_SYNC_RETRIES);
          fflush (stdout);
        }
      else
        {
          fputc ('\n', stdout);
          io_error ("gd6_receive_bytes(16)");
        }
    }

  transfer_size = buffer[1] | (buffer[2] << 8) | (buffer[3] << 16) | (buffer[4] << 24);
  if (transfer_size == 0)
    {
      fputs ("ERROR: Saver transfer size from Game Doctor is 0 bytes\n", stderr);
      exit (1);
    }
  else if (transfer_size > 0x38000)
    {
      fprintf (stderr, "ERROR: Saver transfer size from Game Doctor > 0x38000 bytes (0x%x bytes)\n",
               transfer_size);
      exit (1);
    }
  else if (transfer_size < 0x38000)
    printf ("WARNING: Saver transfer size from Game Doctor < 0x38000 bytes (0x%x bytes)\n",
            transfer_size);

  printf ("Receive: %d Bytes\n"
          "Press q to abort\n\n", transfer_size);

  starttime = time (NULL);
  while (bytesreceived < transfer_size)
    {
      len = transfer_size >= bytesreceived + BUFFERSIZE ?
        BUFFERSIZE : transfer_size - bytesreceived;

      if (gd6_receive_bytes (buffer, len) == GD_ERROR)
        io_error ("gd6_receive_bytes()");
      fwrite (buffer, 1, len, file);

      bytesreceived += len;
      ucon64_gauge (starttime, bytesreceived, transfer_size);
      gd_checkabort (2);
    }

  unregister_func (remove_destfile);
  free (buffer);
  fclose (file);

  return 0;
}


int
gd3_write_saver (const char *filename, unsigned short parport)
{
  gd_send_prolog_bytes = gd3_send_prolog_bytes;
  gd_send_bytes = gd3_send_bytes;

  return gd_write_saver (filename, parport, GD3_PROLOG_STRING);
}


int
gd6_write_saver (const char *filename, unsigned short parport)
{
  gd_send_prolog_bytes = gd6_send_prolog_bytes;
  gd_send_bytes = gd6_send_bytes;

  return gd_write_saver (filename, parport, GD6_READ_PROLOG_STRING);
}


static int
gd_write_saver (const char *filename, unsigned short parport, const char *prolog_str)
{
  FILE *file;
  unsigned char *buffer, gdfilename[12];
  const char *p;
  unsigned int bytesread, bytessent = 0, size, fn_length,
               gd6_protocol = !memcmp (prolog_str, GD6_READ_PROLOG_STRING, 4);
  time_t starttime;

  init_io (parport);

#ifdef  USE_PPDEV
  if (gd6_protocol && !gd6_send_byte_delay)
    puts (STOCKPPDEV_MSG);
#endif

  /*
    Check that filename is a valid Game Doctor saver filename.
    It should start with SF, followed by the game ID, followed by the extension.
    The extension is of the form .S## (where # is a digit from 0-9).
    E.g., SF16123.S00
    The saver base filename must match the base name of the game (loaded in the
    Game Doctor) that you are loading the saver data for.
  */

  // strip the path out of filename for use in the GD
  p = basename2 (filename);
  fn_length = strlen (p);

  if (fn_length < 6 || fn_length > 11   // 7 ("base") + 1 (period) + 3 (extension)
      || toupper ((int) p[0]) != 'S' || toupper ((int) p[1]) != 'F'
      || p[fn_length - 4] != '.' || toupper ((int) p[fn_length - 3]) != 'S')
    {
      fprintf (stderr, "ERROR: Filename (%s) is not a saver filename (SF*.S##)\n", p);
      exit (1);
    }

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

  size = (unsigned int) ucon64.file_size;
  if (size != 0x38000)                  // GD saver size is always 0x38000 bytes -- no header
    {
      fputs ("ERROR: Game Doctor saver file size must be 229376 bytes\n",
             stderr);
      exit (1);
    }

  // make a GD filename from the real one
  memset (gdfilename, ' ', 11);                 // "pad" with spaces
  gdfilename[11] = '\0';                        // terminate string
  memcpy (gdfilename, p, fn_length - 4);        // copy name except extension
  memcpy (&gdfilename[8], "S00", 3);            // copy extension S00
  strupr ((char *) gdfilename);

  printf ("Send: %d Bytes\n", size);
  fseek (file, 0, SEEK_SET);

  if (gd6_protocol && gd6_sync_hardware () == GD_ERROR)
    io_error ("gd6_sync_hardware()");
  memcpy (buffer, prolog_str, 4);
  buffer[4] = 1;
  if (gd_send_prolog_bytes (buffer, 5) == GD_ERROR)
    io_error (gd6_protocol ? "gd6_send_prolog_bytes(5)" : "gd3_send_prolog_bytes(5)");

  // transfer 0x38000 bytes
  buffer[0] = 0x00;
  buffer[1] = 0x80;
  buffer[2] = 0x03;
  buffer[3] = 0x00;
  if (gd_send_prolog_bytes (buffer, 4) == GD_ERROR)
    io_error (gd6_protocol ? "gd6_send_prolog_bytes(4)" : "gd3_send_prolog_bytes(4)");

  if (gd_send_prolog_bytes (gdfilename, 11) == GD_ERROR)
    io_error (gd6_protocol ? "gd6_send_prolog_bytes(11)" : "gd3_send_prolog_bytes(11)");

  puts ("Press q to abort\n");                  // print here, NOT before first GD I/O,
                                                //  because if we get here q works ;-)
  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)) != 0)
    {
      if (gd_send_bytes (buffer, bytesread) == GD_ERROR)
        io_error (gd6_protocol ? "gd6_send_bytes()" : "gd3_send_bytes()");

      bytessent += bytesread;
      ucon64_gauge (starttime, bytessent, size);
      gd_checkabort (2);
    }

  free (buffer);
  fclose (file);

  return 0;
}

#endif // USE_PARALLEL
