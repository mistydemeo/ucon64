/*
libdiscmage.h - libdiscmage

written by 2002 - 2003 NoisyB (noisyb@gmx.net)
           2002 - 2003 dbjh


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
#ifndef LIBDISCMAGE_H
#define LIBDISCMAGE_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "libdm_cfg.h"

#if     defined __linux__ || defined __FreeBSD__ || \
        defined __BEOS__ || defined __solaris__ || HAVE_INTTYPES_H
#include <inttypes.h>
#elif   defined __CYGWIN__
#include <sys/types.h>
#ifndef OWN_INTTYPES
#define OWN_INTTYPES                            // signal that these are defined
typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
typedef u_int64_t uint64_t;
#endif // OWN_INTTYPES
#else
#ifndef OWN_INTTYPES
#define OWN_INTTYPES                            // signal that these are defined
typedef unsigned char uint8_t;
typedef uint16_t int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;
#endif // OWN_INTTYPES
#endif


//  version of this libdiscmage
extern const char *dm_version;


/*
  track nfo
*/
typedef struct
{
  uint16_t global_current_track;  // current track
  uint16_t number;
  uint32_t position;
  uint32_t mode;
  uint32_t sector_size;
  uint32_t sector_size_value;
  uint32_t track_length;
  uint32_t pregap_length;
  uint32_t total_length;
  uint32_t start_lba;
  uint32_t filename_length;
} dm_track_t;


// this struct contains all important data and is init'ed by dm_open()
typedef struct
{
/*
  image nfo
*/
  uint32_t header_offset;
  uint32_t header_position;
  uint32_t image_length;
  uint32_t version;
  uint16_t sessions;      // # of sessions
  uint16_t tracks;        // # of tracks
  uint16_t remaining_sessions;    // sessions left
  uint16_t remaining_tracks;      // tracks left
  uint16_t global_current_session;        // current session

//  dm_track_t **track; // TODO make an array of this
  dm_track_t *track;

/*
  workflow
  TODO make this dissappear
*/
  char filename[FILENAME_MAX];
//  FILE *fh;

  int track_type, save_as_iso, pregap,convert, fulldata, cutall, cutfirst;
  char do_convert, do_cut;

  uint32_t seek_header;
  int type;
  uint32_t seek_ecc;
  char *common;
  char *cdrdao;
} dm_image_t;


/*
  dm_open()  this is the first function to call with the filename of the
             image; it will try to recognize the image format, etc.
  dm_close() the last function; close image

  dm_image_t *img;
  if (!(img = dm_open("image")))
    {
      dm_cdirip (img);
      dm_close (img);
    }
*/
extern dm_image_t *dm_open (const char *image_filename);
extern int dm_close (dm_image_t *image);


/*
  some ready to use functions

  dm_set_gauge () enter here the name of a function that takes two integers

    gauge (pos, total)
      {
        printf ("%d of %d done", pos, total);
      }

  dm_bin2iso()  convert M2/2048 image to M1/2048
  dm_cdirip()   rip tracks from cdi image
TODO:  dm_nrgrip()  rip tracks from nero image
TODO:  dm_rip()     rip files from track
TODO:  dm_cdi2nero() <- this will become dm_neroadd()
TODO:  dm_isofix()   fix an iso image
TODO:  dm_cdifix()   fix a cdi image

  dm_mktoc()  automagically generates toc sheets
  dm_mkcue()  automagically generates cue sheets
*/
extern int dm_set_gauge (int (*gauge) (uint32_t, uint32_t));
extern int dm_bin2iso (dm_image_t *image);
extern int dm_cdirip (dm_image_t *image);
extern int dm_nrgrip (dm_image_t *image);
extern int dm_rip (dm_image_t *image);
extern int dm_cdi2nero (dm_image_t *image);
extern int dm_isofix (dm_image_t *image);
//extern int dm_cdifix (dm_image_t *image);
extern int dm_mktoc (dm_image_t *image);
extern int dm_mkcue (dm_image_t *image);
extern int dm_mksheets (dm_image_t *image);
//#define dm_mksheets(i) (MIN(dm_mktoc(i),dm_mkcue(i)))


/*
  dm_disc_read() and dm_disc_write() are deprecated
  reading or writing images is done by those scripts in contrib/
*/
extern int dm_disc_read (dm_image_t *image);
extern int dm_disc_write (dm_image_t *image);

#endif  // LIBDISCMAGE_H
