/*
mgd.c - Multi Game Doctor/Hunter support for uCON64

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
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "mgd.h"


const char *mgd_usage[] =
  {
    "Multi Game Doctor(2/3)/Multi Game Hunter/MGH/RAW",
    "19XX Bung Enterprises Ltd http://www.bung.com.hk\n"
    "?Makko Toys Co., Ltd.?",
#ifdef TODO
#warning TODO  --xmgd    send/receive ROM to/from Multi Game* /MGD2/MGH/RAW
#endif // TODO
    "TODO:  " OPTION_LONG_S "xmgd   send/receive ROM to/from Multi Game* /MGD2/MGH/RAW; " OPTION_LONG_S "file=PORT\n"
    "                  receives automatically when " OPTION_LONG_S "rom does not exist\n",
    NULL
  };

#ifdef BACKUP

#define BUFFERSIZE      8192
#define OK 0
#define ERROR 1
#define GD3_MAX_UNITS 16                        // Maximum that the hardware supports
// Each logical memory unit is 8Mbit in size (internally it's 2*4Mbit)


static void init_io (unsigned int port);
static void deinit_io (void);
static void io_error (void);
static void mgd_checkabort (int status);
static void gd3_send_byte (unsigned char data);
static void gd3_send_bytes (unsigned len, unsigned char *data);
static int gd3_send_prolog_byte (unsigned char data);
static int gd3_send_prolog_bytes (unsigned len, unsigned char *data);
static int gd3_send_unit_prolog (int header, unsigned size);

typedef struct st_gd3_memory_unit
{
  char name[12];                                // Exact size is 11 chars but I'll
  unsigned char *data;                          //  add one extra for string terminator
  unsigned int size;                              // Usually either 0x100000 or 0x80000
} st_gd3_memory_unit_t;

st_gd3_memory_unit_t gd3_dram_unit[GD3_MAX_UNITS];

static int mgd_port;


void
init_io (unsigned int port)
/*
  - sets global `mgd_port'. Then the send/receive functions don't need to pass
    `port' to all the I/O functions.
  - calls init_conio(). Necessary for kbhit() and DOS-like behaviour of getch().
*/
{
  mgd_port = port;
  if (mgd_port != 0x3bc && mgd_port != 0x378 && mgd_port != 0x278)
    {
      fprintf (stderr, "ERROR: PORT must be 0x3bc, 0x378 or 0x278\n");
      exit (1);
    }

#if     defined __unix__ || defined __BEOS__
  init_conio ();
#endif

  printf ("Using I/O port 0x%x\n", mgd_port);
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
// This function could be changed to take an string argument that describes the
//  error. Or take an integer code that we can interpret here.
{
  fprintf (stderr, "ERROR: Communication with Game Doctor failed\n");
  exit (1);
}


void
mgd_checkabort (int status)
{
  if (((!ucon64.frontend) ? kbhit () : 0) && getch () == 'q')
    {
      puts ("\nProgram aborted");
      exit (status);
    }
}


int
mgd_read_rom (const char *filename, unsigned int parport)
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
  while ((inportb (mgd_port + PARPORT_STATUS) & 0x80) == 0)
    ;

  outportb (mgd_port, data);                    // set data
  outportb (mgd_port + PARPORT_CONTROL, 5);     // Clock data out to SF3
  outportb (mgd_port + PARPORT_CONTROL, 4);
}


void
gd3_send_bytes (unsigned len, unsigned char *data)
{
  int i = len;

  for (i = 0; i < len; i++)
    gd3_send_byte (*data++);

  return;
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
      if ((inportb (mgd_port + PARPORT_STATUS) & 0x08) == 0)
        return ERROR;
    }
  while ((inportb (mgd_port + PARPORT_STATUS) & 0x80) == 0);

  outportb (mgd_port, data);                    // set data
  outportb (mgd_port + PARPORT_CONTROL, 5);     // Clock data out to SF3
  outportb (mgd_port + PARPORT_CONTROL, 4);

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
  if (gd3_send_prolog_byte ((header != 0) ? 0x02: 0x00) == ERROR)
    return ERROR;
  if (gd3_send_prolog_byte (size >> 16) == ERROR) // 0x10 = 8Mbit
    return ERROR;
  if (gd3_send_prolog_byte (0x00) == ERROR)
    return ERROR;
  return OK;
}


int
mgd_write_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char header[MGD_HEADER_LEN], *buffer;
  int fsize, num_units, bytessend = 0, i;
  time_t starttime;

  init_io (parport);

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  fsize = q_fsize (filename);
  printf ("Send: %d Bytes (%.4f Mb)\n", fsize, (float) fsize / MBIT);

  if ((buffer = (unsigned char *) malloc (fsize - MGD_HEADER_LEN)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], fsize - MGD_HEADER_LEN);
      exit (1);
    }

  starttime = time (NULL);
  fread (header, 1, MGD_HEADER_LEN, file);

/*
  Let's make a simple sample load from a 24Mbit LoRom game, Super Metroid.
  This gets split into three memory units with no interleave.

  Assume we have the original Super Metroid ROM data pointed to by romptr.
  The length of the data pointed to by romptr is 0x300000 (24Mbit).

  Assume the Game Doctor 512 byte header is pointed to by hdr_ptr;

  Our split code put it into the memory units like this.
  No extension is necessary but the name entry must be upper case and
  MUST be 11 characters long, padded at the end with spaces if necessary.

  The 3 character ID, in this example "SUP", got set by the split code, based
  off of the file name, Super Metroid.
  For the purpose of loading the SF3 it could be any three characters.

  TODO: make this code generic, i.e. for non-Super Metroid games ;-)
*/

  fread (buffer, 1, fsize - MGD_HEADER_LEN, file);
  num_units = 3;       // number of memory units we must load  3 * 8Mbit = 24Mb

  // The names must be all upper case
  strcpy (gd3_dram_unit[0].name, "SF24SUPA078"); // could also use "SF24SUPA   "
  gd3_dram_unit[0].data = &buffer[0x000000];
  gd3_dram_unit[0].size = 0x100000;

  // The names must be all upper case
  strcpy (gd3_dram_unit[1].name, "SF24SUPB078"); // could also use "SF24SUPB   "
  gd3_dram_unit[1].data = &buffer[0x100000];
  gd3_dram_unit[1].size = 0x100000;

  // The names must be all upper case
  strcpy (gd3_dram_unit[2].name, "SF24SUPC078"); // could also use "SF24SUPC   "
  gd3_dram_unit[2].data = &buffer[0x200000];
  gd3_dram_unit[2].size = 0x100000;

/*
Note:  On most Game Doctor's the way you enter link mode to be able to upload
       the ROM to the unit is to hold down the R key on the controller while
       resetting the SNES.  You will see the Game Doctor menu has a message
       that says "LINKING.."
*/

  printf ("Press q to abort\n\n");

  // Send the ROM to the hardware
  if (gd3_send_prolog_bytes (4, "DSF3") == ERROR)
    io_error ();
  if (gd3_send_prolog_byte ((unsigned char) num_units) == ERROR)
    io_error ();
  for (i = 0; i < num_units; i++)
    {
      int send_header;

      send_header = (i == 0) ? 1: 0;
      if (gd3_send_unit_prolog (send_header, gd3_dram_unit[i].size) == ERROR)
        io_error ();
      if (gd3_send_prolog_bytes (11, gd3_dram_unit[i].name) == ERROR)
        io_error ();
      if (send_header)
        {
          // send the Game Doctor 512 byte header
          if (gd3_send_prolog_bytes (MGD_HEADER_LEN, header) == ERROR)
            io_error ();
          bytessend += MGD_HEADER_LEN;
        }
      gd3_send_bytes (gd3_dram_unit[i].size, gd3_dram_unit[i].data);

      bytessend += gd3_dram_unit[i].size;
      ucon64_gauge (starttime, bytessend, fsize);
   }

  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
}


int
mgd_read_sram (const char *filename, unsigned int parport)
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
mgd_write_sram (const char *filename, unsigned int parport)
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

#endif // BACKUP
