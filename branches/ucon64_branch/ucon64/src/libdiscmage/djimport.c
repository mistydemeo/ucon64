/*
djimport.c - discmage import library for DJGPP

written by 2002 - 2003 dbjh


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
static void (*dm_set_gauge_ptr) (void (*) (int, int));

static FILE *(*dm_fdopen_ptr) (dm_image_t *, int, const char *);
static dm_image_t *(*dm_open_ptr) (const char *, uint32_t);
static dm_image_t *(*dm_reopen_ptr) (const char *, uint32_t, dm_image_t *);
static int (*dm_close_ptr) (dm_image_t *);

static int (*dm_disc_read_ptr) (const dm_image_t *);
static int (*dm_disc_write_ptr) (const dm_image_t *);

static int (*dm_read_ptr) (char *, int, int, const dm_image_t *);
static int (*dm_write_ptr) (const char *, int, int, const dm_image_t *);

static dm_image_t *(*dm_toc_read_ptr) (dm_image_t *, const char *);
static int (*dm_toc_write_ptr) (const dm_image_t *);

static dm_image_t *(*dm_cue_read_ptr) (dm_image_t *, const char *);
static int (*dm_cue_write_ptr) (const dm_image_t *);

static int (*dm_rip_ptr) (const dm_image_t *, int, uint32_t);

static int (*dm_lba_to_msf_ptr) (int, int *, int *, int *);
static int (*dm_msf_to_lba_ptr) (int, int, int, int);
static int (*dm_bcd_to_int_ptr) (int);
static int (*dm_int_to_bcd_ptr) (int);


static void
load_dxe (void)
{
  libdm = open_module (djimport_path);

  dm_get_version_ptr = get_symbol (libdm, "dm_get_version");
  dm_set_gauge_ptr = get_symbol (libdm, "dm_set_gauge");

  dm_open_ptr = get_symbol (libdm, "dm_open");
  dm_fdopen_ptr = get_symbol (libdm, "dm_fdopen");
  dm_reopen_ptr = get_symbol (libdm, "dm_reopen");
  dm_close_ptr = get_symbol (libdm, "dm_close");

  dm_disc_read_ptr = get_symbol (libdm, "dm_disc_read");
  dm_disc_write_ptr = get_symbol (libdm, "dm_disc_write");

  dm_read_ptr = get_symbol (libdm, "dm_read");
  dm_write_ptr = get_symbol (libdm, "dm_write");

  dm_toc_read_ptr = get_symbol (libdm, "dm_toc_read");
  dm_toc_write_ptr = get_symbol (libdm, "dm_toc_write");

  dm_cue_read_ptr = get_symbol (libdm, "dm_cue_read");
  dm_cue_write_ptr = get_symbol (libdm, "dm_cue_write");

  dm_rip_ptr = get_symbol (libdm, "dm_rip");

  dm_lba_to_msf_ptr = get_symbol (libdm, "dm_lba_to_msf");
  dm_msf_to_lba_ptr = get_symbol (libdm, "dm_msf_to_lba");
  dm_bcd_to_int_ptr = get_symbol (libdm, "dm_bcd_to_int");
  dm_int_to_bcd_ptr = get_symbol (libdm, "dm_int_to_bcd");
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


void
dm_set_gauge (void (*a) (int, int))
{
  CHECK
  return dm_set_gauge_ptr (a);
}


FILE *
dm_fdopen (dm_image_t *a, int b, const char *c)
{
  CHECK
  return dm_fdopen_ptr (a, b, c);
}


dm_image_t *
dm_open (const char *a, uint32_t b)
{
  CHECK
  return dm_open_ptr (a, b);
}


dm_image_t *
dm_reopen (const char *a, uint32_t b, dm_image_t *c)
{
  CHECK
  return dm_reopen_ptr (a, b, c);
}


int
dm_close (dm_image_t *a)
{
  CHECK
  return dm_close_ptr (a);
}


int
dm_disc_read (const dm_image_t *a)
{
  CHECK
  return dm_disc_read_ptr (a);
}


int
dm_disc_write (const dm_image_t *a)
{
  CHECK
  return dm_disc_write_ptr (a);
}


int
dm_read (char *a, int b, int c, const dm_image_t *d)
{
  CHECK
  return dm_read_ptr (a, b, c, d);
}


int
dm_write (const char *a, int b, int c, const dm_image_t *d)
{
  CHECK
  return dm_write_ptr (a, b, c, d);
}


dm_image_t *
dm_toc_read (dm_image_t *a, const char *b)
{
  CHECK
  return dm_toc_read_ptr (a, b);
}


int
dm_toc_write (const dm_image_t *a)
{
  CHECK
  return dm_toc_write_ptr (a);
}


dm_image_t *
dm_cue_read (dm_image_t *a, const char *b)
{
  CHECK
  return dm_cue_read_ptr (a, b);
}


int
dm_cue_write (const dm_image_t *a)
{
  CHECK
  return dm_cue_write_ptr (a);
}


int
dm_rip (const dm_image_t *a, int b, uint32_t c)
{
  CHECK
  return dm_rip_ptr (a, b, c);
}


int
dm_lba_to_msf (int lba, int *m, int *s, int *f)
{
  CHECK
  return dm_lba_to_msf_ptr (lba, m, s, f);
}


int
dm_msf_to_lba (int m, int s, int f, int force_positive)
{
  CHECK
  return dm_msf_to_lba_ptr (m, s, f, force_positive);
}


int
dm_bcd_to_int (int b)
{
  CHECK
  return dm_bcd_to_int_ptr (b);
}


int
dm_int_to_bcd (int i)
{
  CHECK
  return dm_int_to_bcd_ptr (i);
}
