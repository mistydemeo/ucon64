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
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;
#endif // OWN_INTTYPES
#endif


//  version of this libdiscmage
extern const uint32_t dm_version;
extern uint32_t dm_get_version (void);

//  usage (example) of libdiscmage
typedef struct
{
  const char *option_s;                         // "chk", ..
  const char *optarg;
  const char *desc;                             // "fix checksum", ...
//  const char *desc_long;                        // long description
//  int status;                                   // development status of option
                                                  // 0 = OK, 1 = TODO, 2 = TEST
} st_dm_usage_t;


extern const st_dm_usage_t dm_usage[];
extern st_dm_usage_t *dm_get_usage (void);

/*
  track nfo
*/
typedef struct
{
  uint32_t track_length;

#if 1
  uint16_t global_current_track;  // current track
  uint16_t number;
  uint32_t position;
  uint32_t sector_size_value;
  uint32_t pregap_length;
  uint32_t total_length;
  uint32_t start_lba;
  uint32_t filename_length;
#endif

  uint32_t mode;
  uint32_t sector_size;
  uint32_t seek_header;
  uint32_t seek_ecc;
} dm_track_t;


typedef struct
{
  int type; // image type DM_CDI, DM_NRG, DM_BIN, ...
  char *desc; // like type but verbal

  uint32_t image_length;

  uint16_t sessions;      // # of sessions
  uint16_t tracks;        // # of tracks

//  dm_track_t **track; // TODO make an array of this(!)
  dm_track_t *track;

  char layout[80];
  char layout_ansi[32768]; // TODO: 32768

/*
  workflow
  TODO make this dissappear
*/
  char filename[FILENAME_MAX];
//  FILE *fh;

  int track_type, save_as_iso, pregap,convert, fulldata, cutall, cutfirst;
  char do_convert, do_cut;

//  char *common;
//  char *cdrdao;
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
#if 1
extern int dm_fseek (dm_image_t *image, int session, int track);
#else
extern int dm_fseek (dm_image_t *image, long offset, int how);
#endif
extern int dm_fgetc (dm_image_t *image);
extern size_t dm_fread (void *buffer, size_t blk_size, size_t blk_num, dm_image_t *image);
#if 0
extern int dm_fputc (dm_image_t *image);
extern size_t dm_fwrite (const void *buffer, size_t blk_size, size_t blk_num, dm_image_t *image);
#endif


/*
  get_session()
  get_track()
  get_num_of_sessions()
  get_num_of_tracks()
*/
#if 0
extern dm_session_t *get_session(int session);
extern dm_track_t *get_track(int track);
extern int get_num_of_sessions(void);
extern int get_num_of_tracks(void);
#endif


/*
  some ready to use functions

  dm_set_gauge () enter here the name of a function that takes two integers

    gauge (pos, total)
      {
        printf ("%d of %d done", pos, total);
      }
*/
extern void dm_set_gauge (void (* gauge) (int, int));


/*
  dm_bin2iso() convert binary Mx/xxxx image to M1/2048
  dm_cdirip()  rip tracks from cdi image
TODO:  dm_nrgrip()  rip tracks from nero image
TODO:  dm_rip()  rip files from track
TODO:  dm_cdi2nero() <- this will become dm_neroadd()
  dm_isofix()  ISO start LBA fixing routine
 
               This tool will take an ISO image with PVD pointing
               to bad DR offset and add padding data so actual DR
               gets located in right absolute address.

               Original boot area, PVD, SVD and VDT are copied to
               the start of new, fixed ISO image.

               Supported input images are: 2048, 2336,
               2352 and 2056 bytes per sector. All of them are
               converted to 2048 bytes per sector when writing
               excluding 2056 image which is needed by Mac users.

TODO:  dm_cdifix()  fix a cdi image

  dm_mktoc()   automagically generates toc sheets
  dm_mkcue()   automagically generates cue sheets
*/
extern int dm_bin2iso (dm_image_t *image);
extern int dm_cdirip (dm_image_t *image);
extern int dm_nrgrip (dm_image_t *image);
extern int dm_rip (dm_image_t *image);
extern int dm_cdi2nero (dm_image_t *image);
extern int dm_isofix (dm_image_t *image, int start_lba);
//extern int dm_cdifix (dm_image_t *image);
extern int dm_mktoc (dm_image_t *image);
extern int dm_mkcue (dm_image_t *image);


/*
  dm_disc_read() and dm_disc_write() are deprecated
  reading or writing images is done by those scripts in contrib/
*/
extern int dm_disc_read (dm_image_t *image);
extern int dm_disc_write (dm_image_t *image);

#endif  // LIBDISCMAGE_H
