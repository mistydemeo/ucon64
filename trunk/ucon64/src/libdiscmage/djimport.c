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
static uint32_t (*dm_get_version_ptr) (void);
static dm_image_t *(*dm_open_ptr) (const char *);
static dm_image_t *(*dm_reopen_ptr) (const char *, dm_image_t *);
static int (*dm_close_ptr) (dm_image_t *);
static int32_t (*dm_rip_ptr) (dm_image_t *);
static int32_t (*dm_cdirip_ptr) (dm_image_t *);
static int32_t (*dm_nrgrip_ptr) (dm_image_t *);
static int (*dm_disc_read_ptr) (dm_image_t *);
static int (*dm_disc_write_ptr) (dm_image_t *);
static int32_t (*dm_mktoc_ptr) (dm_image_t *);
static int32_t (*dm_mkcue_ptr) (dm_image_t *);
static int32_t (*dm_bin2iso_ptr) (dm_image_t *);
static void (*dm_set_gauge_ptr) (void (*) (int, int));
static int (*dm_isofix_ptr) (dm_image_t *, int);

void
load_dxe (void)
{
  libdm = open_module (djimport_path);

  dm_get_version_ptr = get_symbol (libdm, "dm_get_version");

  dm_open_ptr = get_symbol (libdm, "dm_open");
  dm_reopen_ptr = get_symbol (libdm, "dm_reopen");
  dm_close_ptr = get_symbol (libdm, "dm_close");

  dm_rip_ptr = get_symbol (libdm, "dm_rip");
  dm_cdirip_ptr = get_symbol (libdm, "dm_cdirip");
  dm_nrgrip_ptr = get_symbol (libdm, "dm_nrgrip");

  dm_disc_read_ptr = get_symbol (libdm, "dm_disc_read");
  dm_disc_write_ptr = get_symbol (libdm, "dm_disc_write");

  dm_mktoc_ptr = get_symbol (libdm, "dm_mktoc");
  dm_mkcue_ptr = get_symbol (libdm, "dm_mkcue");

  dm_set_gauge_ptr = get_symbol (libdm, "dm_set_gauge");
  dm_bin2iso_ptr = get_symbol (libdm, "dm_bin2iso");
  dm_isofix_ptr = get_symbol (libdm, "dm_isofix");
}


uint32_t
dm_get_version (void)
/*
  Our DXE code can export (pointers to) variables. However, Windows DLLs can
  only export (pointers to) functions. To avoid platform-specific code we
  let all code use these functions in libdiscmage (dm_get_version()).
*/
{
  CHECK
  return dm_get_version_ptr ();
}


dm_image_t *
dm_open (const char *a)
{
  CHECK
  return dm_open_ptr (a);
}


dm_image_t *
dm_reopen (const char *a, dm_image_t *b)
{
  CHECK
  return dm_reopen_ptr (a, b);
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


int32_t
dm_bin2iso (dm_image_t *a)
{
  CHECK
  return dm_bin2iso_ptr (a);
}


void
dm_set_gauge (void (*a) (int, int))
{
  CHECK
  return dm_set_gauge_ptr (a);
}


int32_t
dm_isofix (dm_image_t *a, int b)
{
  CHECK
  return dm_isofix_ptr (a, b);
}
