/*
djimport.c - discmage import library for DJGPP

written by 2002 - 2003 dbjh


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
#include "dlopen.h"
#include "dxedll_pub.h"
#include "libdiscmage.h"

#define CHECK \
  if (!dxe_loaded) \
    { \
      load_dxe (); \
      dxe_loaded = 1; \
    }

static int dxe_loaded = 0;
char djimport_path[FILENAME_MAX] = "discmage.dxe"; // default value

static void *libdm;
static dm_image_t *(*dm_open_ptr) (const char *);
static int (*dm_close_ptr) (dm_image_t *);
static int32_t (*dm_rip_ptr) (dm_image_t *);
static int32_t (*dm_cdirip_ptr) (dm_image_t *);
static int32_t (*dm_nrgrip_ptr) (dm_image_t *);
static int (*dm_disc_read_ptr) (dm_image_t *);
static int (*dm_disc_write_ptr) (dm_image_t *);
static int32_t (*dm_mksheets_ptr) (dm_image_t *);
static int32_t (*dm_mktoc_ptr) (dm_image_t *);
static int32_t (*dm_mkcue_ptr) (dm_image_t *);

void
load_dxe (void)
{
  libdm = open_module (djimport_path);

  dm_open_ptr = get_symbol (libdm, "dm_open");
  dm_close_ptr = get_symbol (libdm, "dm_close");

  dm_rip_ptr = get_symbol (libdm, "dm_rip");
  dm_cdirip_ptr = get_symbol (libdm, "dm_cdirip");
  dm_nrgrip_ptr = get_symbol (libdm, "dm_nrgrip");

  dm_disc_read_ptr = get_symbol (libdm, "dm_disc_read");
  dm_disc_write_ptr = get_symbol (libdm, "dm_disc_write");

  dm_mksheets_ptr = get_symbol (libdm, "dm_mksheets");
  dm_mktoc_ptr = get_symbol (libdm, "dm_mktoc");
  dm_mkcue_ptr = get_symbol (libdm, "dm_mkcue");
}


dm_image_t *
dm_open (const char *a)
{
  CHECK
  return dm_init_ptr (a);
}


int
dm_close (dm_image_t *a)
{
  CHECK
  return dm_close_ptr (a);
}


int32_t
dm_rip (dm_image_t *a)
{
  CHECK
  return dm_rip_ptr (a);
}


int32_t
dm_cdirip (dm_image_t *a)
{
  CHECK
  return dm_cdirip_ptr (a);
}


int32_t
dm_nrgrip (dm_image_t *a)
{
  CHECK
  return dm_nrgrip_ptr (a);
}


int
dm_disc_read (dm_image_t *a)
{
  CHECK
  return dm_disc_read_ptr (a);
}


int
dm_disc_write (dm_image_t *a)
{
  CHECK
  return dm_disc_write_ptr (a);
}


int32_t
dm_mksheets (dm_image_t *a)
{
  CHECK
  return dm_mksheets_ptr (a);
}


int32_t
dm_mktoc (dm_image_t *a)
{
  CHECK
  return dm_mktoc_ptr (a);
}


int32_t
dm_mkcue (dm_image_t *a)
{
  CHECK
  return dm_mkcue_ptr (a);
}
