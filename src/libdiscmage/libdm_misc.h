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
#ifndef LIBDM_MISC_H
#define LIBDM_MISC_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "libdiscmage_cfg.h"
#include "misc.h"                               // or let every source file include




//  version of this libdiscmage
extern const char *dm_version;


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
  unsigned short sessions;      // # of sessions
  unsigned short tracks;        // # of tracks
  unsigned short remaining_sessions;    // sessions left
  unsigned short remaining_tracks;      // tracks left
  unsigned short global_current_session;        // current session
/*
  track nfo
  TODO make an array of this
*/
  unsigned short global_current_track;  // current track
  unsigned short number;
  uint32_t position;
  uint32_t mode;
  uint32_t sector_size;
  uint32_t sector_size_value;
  uint32_t track_length;
  uint32_t pregap_length;
  uint32_t total_length;
  uint32_t start_lba;
  uint32_t filename_length;
/*
  workflow
  TODO make this dissappear
*/
  char filename[FILENAME_MAX];
  FILE *fh;

  int32_t track_type, save_as_iso, pregap,convert, fulldata, cutall, cutfirst;
  char do_convert, do_cut;

  uint32_t seek_header;
  int type;
  uint32_t seek_ecc;
  char *common;
  char *cdrdao;
} dm_image_t;


// struct for LBA <-> MSF conversions
typedef struct
{
  uint8_t    cdmsf_min0;     /* start minute */
  uint8_t    cdmsf_sec0;     /* start second */
  uint8_t    cdmsf_frame0;   /* start frame */
  uint8_t    cdmsf_min1;     /* end minute */
  uint8_t    cdmsf_sec1;     /* end second */
  uint8_t    cdmsf_frame1;   /* end frame */
} dm_msf_t;


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

  dm_lba_to_msf() convert LBA to minutes, seconds, frames
  dm_msf_to_lba() convert minutes, seconds, frames to LBA
  dm_from_bcd()   convert BCD to integer
  dm_to_bcd()     convert integer to BCD

  dm_mktoc()  automagically generates toc sheets
  dm_mkcue()  automagically generates cue sheets
*/
extern int32_t dm_set_gauge (int (*gauge) (uint32_t, uint32_t));
extern int32_t dm_bin2iso (dm_image_t *image);
extern int32_t dm_cdirip (dm_image_t *image);
extern int32_t dm_nrgrip (dm_image_t *image);
extern int32_t dm_rip (dm_image_t *image);
extern int32_t dm_cdi2nero (dm_image_t *image);
extern int32_t dm_isofix (dm_image_t *image);
//extern int32_t dm_cdifix (dm_image_t *image);

extern int32_t dm_lba_to_msf (int32_t lba, dm_msf_t * mp);
extern int32_t dm_msf_to_lba (int32_t m, int32_t s, int32_t f, int32_t force_positive);
extern int32_t dm_from_bcd (int32_t b);
extern int32_t dm_to_bcd (int32_t i);

extern int32_t dm_mktoc (dm_image_t *image);
extern int32_t dm_mkcue (dm_image_t *image);
//extern int32_t dm_mksheets (dm_image_t *image);
#define dm_mksheets(i) (MIN(dm_mktoc(i),dm_mkcue(i)))


/*
  dm_disc_read() and dm_disc_write() are depricated
  reading or writing images is done by those scripts in contrib/
*/
extern int32_t dm_disc_read (dm_image_t *image);
extern int32_t dm_disc_write (dm_image_t *image);

#if 0
extern int32_t cdi_init (dm_image_t *image);

extern int32_t ask_type (FILE * fsource, int32_t header_position);
extern int32_t cdi_read_track (dm_image_t * cdi_image);
extern void cdi_get_sessions (dm_image_t * cdi_image);
extern void cdi_get_tracks (dm_image_t * cdi_image);
extern void cdi_skip_next_session (dm_image_t * cdi_image);

extern int32_t nrg_init (dm_image_t * image);

extern void nrg_write_cues_hdr (char *fcues, int32_t *fcues_i, dm_image_t * image);
extern void nrg_write_daoi_hdr (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image);
extern void nrg_write_cues_track (char *fcues, int32_t *fcues_i, dm_image_t * image);
extern void nrg_write_daoi_track (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image, int32_t nrg_offset);
extern void nrg_write_cues_tail (char *fcues, int32_t *fcues_i, dm_image_t * image);
extern void nrg_write_sinf (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image);
extern void nrg_write_etnf_hdr (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image);
extern void nrg_write_etnf_track (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image, int32_t nrg_offset);
#endif
#endif  // LIBDM_MISC_H
