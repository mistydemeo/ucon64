/*
smcic2.c - Super Magicom IC2 compatible copier support for uCON64

Copyright (c) 2019 dbjh


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
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <stdlib.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "misc/archive.h"
#include "misc/file.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "backup/ffe.h"
#include "backup/smcic2.h"
#include "backup/swc.h"


#ifdef  USE_PARALLEL
static st_ucon64_obj_t smcic2_obj[] =
  {
    {UCON64_SNES, WF_DEFAULT | WF_STOP}
  };
#endif

const st_getopt2_t smcic2_usage[] =
  {
    {
      NULL, 0, 0, 0,
      // not Future Supercom Hyper Effect Pro.9, unsure about Supercom Pro 2
      NULL, "UFO Super Drive PRO 6 HYPER VERSION/Future Supercom Pro.9/Twin Supercom\n"
            "(Modified) Supercom/(Modified) Super Magicom",
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xic2", 0, 0, UCON64_XIC2,                // send only
      NULL, "send ROM to SMC IC2 compatible backup unit; " OPTION_LONG_S "port" OPTARG_S "PORT",
      &smcic2_obj[0]
    },
#endif // USE_PARALLEL
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

#ifdef  USE_PARALLEL

#define BUFFERSIZE 8192
#define MAX_FILES 9 // maximum number of parts of a split ROM dump (.1 to .9)

typedef struct st_add_filename_data
{
  unsigned int index;
  char *names[MAX_FILES];
} st_add_filename_data_t;


static void
add_filename (const char *filename, void *cb_data)
{
  st_add_filename_data_t *add_filename_data = (st_add_filename_data_t *) cb_data;

  if (add_filename_data->index < MAX_FILES)
    {
      char **fname = &add_filename_data->names[add_filename_data->index];
      if ((*fname = (char *) malloc (FILENAME_MAX)) == NULL)
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], FILENAME_MAX);
          return;
        }
      snprintf (*fname, FILENAME_MAX, "%s", filename);
      (*fname)[FILENAME_MAX - 1] = '\0';

      add_filename_data->index++;
    }
}


int
smcic2_write_rom (const char *filename, unsigned short parport)
{
  unsigned char *buffer, emu_mode_select = 0;
  int bytessent = 0, fsize = 0;
  unsigned int nparts, n;
  unsigned short blocksdone = 0, address = 0x200; // VGS '00 uses 0x200, VGS '96 uses
  time_t starttime;                               //  0, but then some ROMs don't work
  st_add_filename_data_t add_filename_data = { 0, { NULL } };

  ffe_init_io (parport);

  if (ucon64.split)                             // snes_init() sets ucon64.split
    nparts = ucon64_testsplit (filename, add_filename, &add_filename_data);
  else
    {
      add_filename (filename, &add_filename_data);
      nparts = 1;
    }

  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  for (n = 0; n < nparts; n++)
    {
      char *fname = add_filename_data.names[n];

      if (fname == NULL)
        exit (1); // error message has already been printed, see add_filename()
      fsize += fsizeof (fname);
    }
  printf ("Send: %d Bytes (%.4f Mb)\n", fsize, (float) fsize / MBIT);

  ffe_send_command0 (0xc008, 0);

  puts ("Press q to abort\n");
  starttime = time (NULL);
  for (n = 0; n < nparts; n++)
    {
      char *fname = add_filename_data.names[n];
      FILE *file;
      int bytesread;

      if ((file = fopen (fname, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], fname);
          exit (1);
        }

      fread (buffer, 1, SWC_HEADER_LEN, file);
      emu_mode_select = buffer[2];              // this byte is needed later
      ffe_send_command (5, 0, 0);
      ffe_send_block (0x400, buffer, SWC_HEADER_LEN); // send header
      bytessent += SWC_HEADER_LEN;

      while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)) != 0)
        {
          ffe_send_command0 (0xc010, (unsigned char) (blocksdone >> 9));
          ffe_send_command (5, address, 0);
          ffe_send_block (0x8000, buffer, (unsigned short) bytesread);
          address++;
          blocksdone++;

          bytessent += bytesread;
          ucon64_gauge (starttime, bytessent, fsize);
          ffe_checkabort (2);
        }

      fclose (file);
    }

  // --xic2 is basically --xswc for split files...
  if (blocksdone > 0x200)                       // ROM dump > 512 8 kB blocks (=32 Mb (=4 MB))
    ffe_send_command0 (0xc010, 2);

  ffe_send_command (5, 0, 0);
  ffe_send_command (6, 5 | (blocksdone << 8), blocksdone >> 8); // bytes: 6, 5, #8 K L, #8 K H, 0
  ffe_send_command (6, 1 | (emu_mode_select << 8), 0);

  free (buffer);
  for (n = 0; n < nparts; n++)
    free (add_filename_data.names[n]);

  return 0;
}

#endif // USE_PARALLEL
