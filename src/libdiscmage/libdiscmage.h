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
  char fname[FILENAME_MAX];

  uint16_t sessions;      // # of sessions
  uint16_t tracks;        // # of tracks

//  dm_track_t **track; // TODO make an array of this(!)
  dm_track_t *track;

//  TODO make this dissappear
  int track_type, 
      save_as_iso, 
      pregap,convert, 
      fulldata, 
      cutall, 
      cutfirst;
  char do_convert, 
       do_cut;
} dm_image_t;


/*
  dm_get_version()  returns version of libdiscmage as uint32_t
  dm_open()      this is the first function to call with the filename of the
                 image; it will try to recognize the image format, etc.
  dm_reopen()    like dm_open() but can reuse an existing dm_image_t
  dm_close()     the last function; close image
  dm_fseek()     seek for tracks inside image
                 dm_fseek (image, 2, SEEK_END);
                 will seek to the end of the 2nd track in image
  dm_set_gauge() enter here the name of a function that takes two integers
                 gauge (pos, total)
                   {
                     printf ("%d of %d done", pos, total);
                   }
  dm_bin2iso()   convert binary Mx/xxxx image to M1/2048
TODO:  dm_rip()  rip files from track
  dm_isofix()    takes an ISO image with PVD pointing
                 to bad DR offset and add padding data so actual DR
                 gets located in right absolute address.
                 Original boot area, PVD, SVD and VDT are copied to
                 the start of new, fixed ISO image.
                 Supported input images are: 2048, 2336,
                 2352 and 2056 bytes per sector. All of them are
                 converted to 2048 bytes per sector when writing
                 excluding 2056 image which is needed by Mac users.
TODO:  dm_cdifix()  fix a cdi image
  dm_mktoc()     automagically generates toc sheets
  dm_mkcue()     automagically generates cue sheets
  dm_disc_read() deprecated reading or writing images is done
                 by those scripts in contrib/
  dm_disc_write()deprecated reading or writing images is done
                 by those scripts in contrib/
*/
//extern const uint32_t dm_version;
extern uint32_t dm_get_version (void);
extern dm_image_t *dm_open (const char *fname);
extern dm_image_t *dm_reopen (const char *fname, dm_image_t *image);
extern int dm_close (dm_image_t *image);
extern int dm_fseek (FILE *fp, int track, int how);
extern void dm_set_gauge (void (* gauge) (int, int));
extern int dm_bin2iso (dm_image_t *image);
extern int dm_rip (dm_image_t *image);
extern int dm_isofix (dm_image_t *image, int start_lba);
extern int dm_mktoc (dm_image_t *image);
extern int dm_mkcue (dm_image_t *image);
extern int dm_disc_read (dm_image_t *image);
extern int dm_disc_write (dm_image_t *image);

#endif  // LIBDISCMAGE_H
