/*
libdiscmage.h - libdiscmage

written by 2002 NoisyB (noisyb@gmx.net)
           2003 dbjh


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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "libdiscmage_cfg.h"
#ifdef  HAVE_INTTYPES_H
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
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;
#endif // OWN_INTTYPES
#endif // HAVE_INTTYPES_H

#if 0
#define MODE1_2048 0
#define MODE1_2352 1
#define MODE2_2336 2
#define MODE2_2352 3
// Macintosh
#define MODE2_2056 4
#endif

#define DEFAULT_FORMAT   0
#define ISO_FORMAT       1
#define BIN_FORMAT       2
#define CDI_FORMAT       3
#define NRG_FORMAT       4
#define CCD_FORMAT       5

#define READ_BUF_SIZE  (1024*1024)
#define WRITE_BUF_SIZE (1024*1024)




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
  uint32_t seek_ecc;
  char *common;
  char *cdrdao;
} dm_image_t;


/*
  mksheets()  automagically generates cue and toc sheets
              src could be images or existing cue or toc sheets

              0  (success)
              -1 (error)
*/
extern int32_t dm_mksheets (dm_image_t *image);

extern int32_t dm_mktoc (dm_image_t *image);
extern int32_t dm_mkcue (dm_image_t *image);

#define CDI_V2  0x80000004
#define CDI_V3  0x80000005
#define CDI_V35 0x80000006
//#define CDI_V4 0x80000006


extern int cdi_init (dm_image_t *image);

extern int32_t ask_type (FILE * fsource, int32_t header_position);
extern int32_t cdi_read_track (dm_image_t * cdi_image);
extern void cdi_get_sessions (dm_image_t * cdi_image);
extern void cdi_get_tracks (dm_image_t * cdi_image);
extern void cdi_skip_next_session (dm_image_t * cdi_image);

extern int nrg_init (dm_image_t * image);

extern void nrg_write_cues_hdr (char *fcues, int32_t *fcues_i, dm_image_t * image);
extern void nrg_write_daoi_hdr (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image);
extern void nrg_write_cues_track (char *fcues, int32_t *fcues_i, dm_image_t * image);
extern void nrg_write_daoi_track (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image, int32_t nrg_offset);
extern void nrg_write_cues_tail (char *fcues, int32_t *fcues_i, dm_image_t * image);
extern void nrg_write_sinf (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image);
extern void nrg_write_etnf_hdr (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image);
extern void nrg_write_etnf_track (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image, int32_t nrg_offset);


/*
  some support routines
*/
//extern int lba_to_msf (uint32_t lba, struct cdrom_msf * mp);
//extern uint32_t msf_to_lba (int m, int s, int f, int force_positive);
//extern int from_bcd (int b);
//extern int to_bcd (int i);


/*
  dm_open()  this will init libdiscmage and try to recognize the image format, etc.
*/
extern dm_image_t *dm_open (const char *image_filename);
extern int dm_close (dm_image_t *image);


/*
  dm_bin2iso()  convert Mx/>2048 image to M1/2048
  dm_cdirip()   rip tracks from cdi image
TODO:  dm_nerorip()  rip tracks from nero image
TODO:  dm_cdi2nero() <- this will become dm_neroadd()
TODO:  dm_cdiadd()
TODO:  dm_isofix()   fix an iso image
TODO:  dm_cdifix()   fix a cdi image
*/
extern int32_t dm_bin2iso (dm_image_t *image);
extern int32_t dm_cdirip (dm_image_t *image);
extern int32_t dm_nrgrip (dm_image_t *image);
extern int32_t dm_rip (dm_image_t *image);
extern int32_t dm_cdi2nero (dm_image_t *image);
//extern int32_t dm_isofix (dm_image_t *image);

extern int dm_disc_read (dm_image_t *image);
extern int dm_disc_write (dm_image_t *image);

extern const char *dm_version;
#endif  // LIBDISCMAGE_H
