/*
gd.c - Game Doctor support for uCON64

written by 2002 John Weidman
           2002 dbjh


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
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "gd.h"
#include "console/snes.h"                       // for snes_make_gd_names() &
                                                //  snes_get_snes_hirom()

const char *gd_usage[] =
  {
    "Game Doctor SF3(SF6/SF7)",
    "19XX Bung Enterprises Ltd http://www.bung.com.hk",
#ifdef PARALLEL

#if 1 // dumping is not yet supported
    "  " OPTION_LONG_S "xgd3        send ROM to Game Doctor SF3(SF6/SF7); " OPTION_LONG_S "port=PORT\n",
#else
    "  " OPTION_LONG_S "xgd3        send/receive ROM to/from Game Doctor SF3(SF6/SF7); " OPTION_LONG_S "port=PORT\n"
    "                  receives automatically when " OPTION_LONG_S "ROM does not exist\n",
#endif

#else
    "",
#endif // PARALLEL
    NULL
  };

#ifdef PARALLEL
#define BUFFERSIZE      8192
#define OK 0
#define ERROR 1

static void init_io (unsigned int port);
static void deinit_io (void);
static void io_error (void);
static void gd_checkabort (int status);
static void gd3_send_byte (unsigned char data);
static void gd3_send_bytes (unsigned len, unsigned char *data);
static int gd3_send_prolog_byte (unsigned char data);
static int gd3_send_prolog_bytes (unsigned len, unsigned char *data);
static int gd3_send_unit_prolog (int header, unsigned size);

typedef struct st_gd3_memory_unit
{
  char name[12];                                // Exact size is 11 chars but I'll
//  unsigned char *data;                        //  add one extra for string terminator
  unsigned int size;                            // Usually either 0x100000 or 0x80000
} st_gd3_memory_unit_t;

st_gd3_memory_unit_t gd3_dram_unit[GD3_MAX_UNITS];

static int gd_port, gd_bytessend, gd_fsize, gd_name_i = 0;
static time_t gd_starttime;
static char **gd_names;


void
init_io (unsigned int port)
/*
  - sets global `gd_port'. Then the send/receive functions don't need to pass
    `port' to all the I/O functions.
  - calls init_conio(). Necessary for kbhit() and DOS-like behaviour of getch().
*/
{
  gd_port = port;
  if (gd_port != 0x3bc && gd_port != 0x378 && gd_port != 0x278)
    {
      fprintf (stderr, "ERROR: PORT must be 0x3bc, 0x378 or 0x278\n");
      exit (1);
    }

#if     defined __unix__ || defined __BEOS__
  init_conio ();
#endif

  printf ("Using I/O port 0x%x\n", gd_port);
}


void
deinit_io (void)
{
// Put possible transfer cleanup stuff here
#if     defined __unix__ || defined __BEOS__
  deinit_conio ();
#endif
}


void
io_error (void)
// This function could be changed to take a string argument that describes the
//  error. Or take an integer code that we can interpret here.
{
  fprintf (stderr, "ERROR: Communication with Game Doctor failed\n");
  exit (1);
}


void
gd_checkabort (int status)
{
  if (((!ucon64.frontend) ? kbhit () : 0) && getch () == 'q')
    {
      puts ("\nProgram aborted");
      exit (status);
    }
}


int
gd_read_rom (const char *filename, unsigned int parport)
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
  return fprintf (stderr, "ERROR: This function is not yet implemented\n");
#endif
}


/*
  General data output routine
  Use this routine for sending ROM data bytes to the Game Doctor SF3 (SF6/SF7
  too).
*/
void
gd3_send_byte (unsigned char data)
{
  // Wait until SF3 is not busy
  while ((inportb (gd_port + PARPORT_STATUS) & 0x80) == 0)
    ;

  outportb (gd_port, data);                     // set data
  outportb (gd_port + PARPORT_CONTROL, 5);      // Clock data out to SF3
  outportb (gd_port + PARPORT_CONTROL, 4);
}


void
gd3_send_bytes (unsigned len, unsigned char *data)
{
  int i = len;

  for (i = 0; i < len; i++)
    {
      gd3_send_byte (data[i]);
      gd_bytessend++;
      if ((gd_bytessend - GD_HEADER_LEN) % 8192 == 0)
        {
          ucon64_gauge (gd_starttime, gd_bytessend, gd_fsize);
          gd_checkabort (2);                    // 2 to return something other than 1
        }
    }
}


/*
  Prolog specific data output routine
  We could probably get away with using the general routine but the
  transfer program I (JW) traced to analyze the protocol did this for
  the bytes used to set up the transfer so here it is.
*/
int
gd3_send_prolog_byte (unsigned char data)
{
  // Wait until SF3 is not busy
  do
    {
      if ((inportb (gd_port + PARPORT_STATUS) & 0x08) == 0)
        return ERROR;
    }
  while ((inportb (gd_port + PARPORT_STATUS) & 0x80) == 0);

  outportb (gd_port, data);                     // set data
  outportb (gd_port + PARPORT_CONTROL, 5);      // Clock data out to SF3
  outportb (gd_port + PARPORT_CONTROL, 4);

  return OK;
}


int
gd3_send_prolog_bytes (unsigned len, unsigned char *data)
{
  int i = len;

  for (i = 0; i < len; i++)
    {
      if (gd3_send_prolog_byte (*data++) == ERROR)
        return ERROR;
    }
  return OK;
}


int
gd3_send_unit_prolog (int header, unsigned size)
{
  if (gd3_send_prolog_byte (0x00) == ERROR)
    return ERROR;
  if (gd3_send_prolog_byte ((header != 0) ? 0x02 : 0x00) == ERROR)
    return ERROR;
  if (gd3_send_prolog_byte (size >> 16) == ERROR) // 0x10 = 8Mbit
    return ERROR;
  if (gd3_send_prolog_byte (0x00) == ERROR)
    return ERROR;
  return OK;
}


int
gd_add_filename (const char *filename)
{
  char buf[FILENAME_MAX], *p;

  if (gd_name_i < GD3_MAX_UNITS)
    {
      strcpy (buf, filename);
      p = strrchr (buf, '.');
      if (p)
        *p = 0;
      strncpy (gd_names[gd_name_i], basename (buf), 11);
      gd_names[gd_name_i][11] = 0;
      gd_name_i++;
    }
  return 0;
}


/*
Note: On most Game Doctor's the way you enter link mode to be able to upload
      the ROM to the unit is to hold down the R key on the controller while
      resetting the SNES. You will see the Game Doctor menu has a message that
      says "LINKING.."
*/
/*
  This function only supports the SF3 protocol. If we know the details about
  the SF7 protocol this function should be renamed to gd3_write_rom() and a
  function gd7_write_rom should be added.
*/
int
gd_write_rom (const char *filename, unsigned int parport, st_rominfo_t *rominfo)
{
  FILE *file = NULL;
  unsigned char *buffer, *names[GD3_MAX_UNITS], names_mem[GD3_MAX_UNITS][12],
                filenames[GD3_MAX_UNITS][8 + 1 + 3 + 1]; // +1 for period, +1 for ASCII-z;
  int num_units, i, send_header, x, split = 1, hirom = snes_get_snes_hirom();

  init_io (parport);

  // We don't want to malloc() ridiculously small chunks (of 12 bytes)
  for (i = 0; i < GD3_MAX_UNITS; i++)
    names[i] = names_mem[i];

  gd_names = (char **) names;
  ucon64_testsplit_callback = gd_add_filename;
  num_units = ucon64.split = ucon64_testsplit (filename); // this will call gd_add_filename()
  ucon64_testsplit_callback = NULL;
  if (!ucon64.split)
    {
      split = 0;
      num_units = snes_make_gd_names (filename, rominfo, (char **) names);
    }

  gd_fsize = 0;
  for (i = 0; i < num_units; i++)
    {
      /*
        No extension is necessary but the name entry must be upper case and
        MUST be 11 characters long, padded at the end with spaces if necessary.
      */
      memset (gd3_dram_unit[i].name, ' ', 11);  // "pad" with spaces
      gd3_dram_unit[i].name[11] = 0;            // terminate string so we can print it (debug)
      // Use memcpy() instead of strcpy() so that the string terminator in
      //  names[i] won't be copied.
      memcpy (gd3_dram_unit[i].name, strupr(names[i]), strlen (names[i]));

      sprintf (filenames[i], "%s.078", names[i]); // should match with what code of -s does
      if (split)
        {
          x = q_fsize (filenames[i]);
          gd_fsize += x;
          gd3_dram_unit[i].size = x;
          if (i == 0)                           // Correct for header of first file
            gd3_dram_unit[i].size -= GD_HEADER_LEN;
        }
      else
        {
          if (!gd_fsize)                        // Don't call q_fsize() more
            gd_fsize = q_fsize (filename);      //  often than necessary
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
    { // a DRAM unit can hold 8 MBit at maximum
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], 8 * MBIT);
      exit (1);
    }

  printf ("Send: %d Bytes (%.4f Mb)\n", gd_fsize, (float) gd_fsize / MBIT);

  // Put this just before the real transfer begins or else the ETA won't be
  //  correct.
  gd_starttime = time (NULL);

  // Send the ROM to the hardware
  if (gd3_send_prolog_bytes (4, "DSF3") == ERROR)
    io_error ();
  if (gd3_send_prolog_byte ((unsigned char) num_units) == ERROR)
    io_error ();

  printf ("Press q to abort\n\n");
  for (i = 0; i < num_units; i++)
    {
#ifdef  DEBUG
      printf ("\nfilename (%d): \"%s\", ", split, (split ? (char *) filenames[i] : filename));
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
        {
          if (file == NULL)                     // don't open the file more than once
            {
              if ((file = fopen (filename, "rb")) == NULL)
                {
                  fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
                  exit (1);
                }
            }
        }

      send_header = (i == 0) ? 1 : 0;
      if (gd3_send_unit_prolog (send_header, gd3_dram_unit[i].size) == ERROR)
        io_error ();
      if (gd3_send_prolog_bytes (11, gd3_dram_unit[i].name) == ERROR)
        io_error ();
      if (send_header)
        {
          // Send the Game Doctor 512 byte header
          fread (buffer, 1, GD_HEADER_LEN, file);
          if (gd3_send_prolog_bytes (GD_HEADER_LEN, buffer) == ERROR)
            io_error ();
          gd_bytessend += GD_HEADER_LEN;
        }
      if (split == 0)                           // Not pre-split -- have to split it ourself
        {
          if (hirom)
            fseek (file, i * gd3_dram_unit[0].size + GD_HEADER_LEN, SEEK_SET);
          else
            fseek (file, i * 8 * MBIT + GD_HEADER_LEN, SEEK_SET);
        }
      fread (buffer, 1, gd3_dram_unit[i].size, file);
      gd3_send_bytes (gd3_dram_unit[i].size, buffer);

      if (split || i == num_units - 1)
        fclose (file);
   }

  free (buffer);
  deinit_io ();

  return 0;
}


int
gd_read_sram (const char *filename, unsigned int parport)
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
  return fprintf (stderr, "ERROR: This function is not yet implemented\n");
#endif
}


int
gd_write_sram (const char *filename, unsigned int parport)
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
  return fprintf (stderr, "ERROR: This function is not yet implemented\n");
#endif
}

#endif // PARALLEL
