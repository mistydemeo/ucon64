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

#ifdef  __cplusplus
extern "C" {
#endif

#if     defined __linux__ || defined __FreeBSD__ || \
        defined __BEOS__ || defined __solaris__ || defined HAVE_INTTYPES_H
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
#ifndef _WIN32
typedef unsigned long long int uint64_t;
#else
typedef unsigned __int64 uint64_t;
#endif
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
#ifndef _WIN32
typedef signed long long int int64_t;
#else
typedef signed __int64 int64_t;
#endif
#endif // OWN_INTTYPES
#endif

// a cd can have max. 99 tracks; this value might change in the future
#define DM_MAX_TRACKS (99)

#define DM_UNKNOWN (-1)

#define DM_SHEET (1)
#define DM_CUE (DM_SHEET)
#define DM_TOC (DM_SHEET)
#define DM_CDI (2)
#define DM_NRG (3)
#define DM_CCD (4)


typedef struct
{
// TODO?: replace those uint32_t with uint64_t or so...

// some formats use to have the tracks "embedded" (like: cdi, nrg, etc..)
// this is the start offset inside the image
  uint32_t track_start; // in bytes
  
  int16_t pregap_len; // in sectors
  int16_t start_lba;  // in sectors?

  uint32_t track_len; // in sectors
  uint32_t total_len; // in sectors; pregap_len + track_len == total_len 
                           // (less if the track is truncated)
  int min, sec, fps;

// start of the iso header inside the track (inside the image)
  int32_t iso_header_start; // if -1 then no iso header

  int8_t mode;        // 0 == AUDIO, 1 == MODE1, 2 == MODE2
  uint16_t sector_size; // in bytes; includes seek_header + seek_ecc
  int16_t seek_header; // in bytes
  int16_t seek_ecc;    // in bytes

  const char *desc;
//  const char *desc_toc;
} dm_track_t;


typedef struct
{
  int type;               // image type DM_CDI, DM_NRG, DM_TOC, etc.
  char *desc;             // like type but more verbose
  int flags;              // DM_FIX, ...
  char fname[FILENAME_MAX]; // filename of image
  uint32_t version; // version of image (used by CDI and  NRG)

  int sessions; // # of sessions
  int tracks;  // # of tracks

  dm_track_t track[DM_MAX_TRACKS]; // array of dm_track_t
  uint8_t session[DM_MAX_TRACKS + 1]; // array of uint32_t with tracks per session

// some image formats (CDI) use embedded headers instead of TOC or CUE files
  int header_start;
  int header_len; // if header_len == 0 then no header(!)
} dm_image_t;


/*
  dm_get_version()  returns version of libdiscmage as uint32_t
  dm_open()      this is the first function to call with the filename of the
                   image; it will try to recognize the image format, etc.
  dm_reopen()    like dm_open() but can reuse an existing dm_image_t
  dm_close()     the last function; close image
  dm_fdopen()    returns a FILE ptr from the start of a track (in image)
  dm_fseek()     seek for tracks inside image
                   dm_fseek (image, 2, SEEK_END);
                   will seek to the end of the 2nd track in image
  dm_set_gauge() enter here the name of a function that takes two integers
                 gauge (pos, total)
                   {
                     printf ("%d of %d done", pos, total);
                   }
  dm_read()      read single sector from track (in image)
TODO: dm_write()     write single sector to track (in image)
  dm_toc_read()  read TOC sheet into dm_image_t
  dm_toc_write() write dm_image_t as TOC sheet
  dm_cue_read()  read CUE sheet into dm_image_t
  dm_cue_write() write dm_image_t as CUE sheet
  dm_disc_read() deprecated reading or writing images is done
                   by those scripts in contrib/
  dm_disc_write()deprecated reading or writing images is done
                   by those scripts in contrib/
*/
extern uint32_t dm_get_version (void);
extern void dm_set_gauge (void (*gauge) (int, int));

#define DM_RDONLY (1)
#define DM_WRONLY (2)
#define DM_RDWR (4)
#define DM_CREAT (8)
extern dm_image_t *dm_open (const char *fname, uint32_t flags);
extern dm_image_t *dm_reopen (const char *fname, uint32_t flags, dm_image_t *image);
extern int dm_close (dm_image_t *image);

//extern int dm_fseek (FILE *fp, int track_num, int how);
extern FILE *dm_fdopen (dm_image_t *image, int track_num, const char *mode);

extern int dm_read (char *buffer, int track_num, int sector, const dm_image_t *image);
extern int dm_write (const char *buffer, int track_num, int sector, const dm_image_t *image);

extern dm_image_t *dm_toc_read (dm_image_t *image, const char *toc_sheet);
extern int dm_toc_write (const dm_image_t *image);

extern dm_image_t *dm_cue_read (dm_image_t *image, const char *cue_sheet);
extern int dm_cue_write (const dm_image_t *image);

extern int dm_disc_read (const dm_image_t *image);
extern int dm_disc_write (const dm_image_t *image);

/*
  dm_rip()       convert/rip a track from an image
TODO: DM_FILES   rip files from track instead of track
  DM_WAV         convert possible audio track to wav (instead of raw)
  DM_2048        (bin2iso) convert binary sector_size to 2048 bytes
                   (could result in a malformed output image)
TODO: DM_FIX     (isofix) takes an ISO image with PVD pointing
                   to bad DR offset and add padding data so actual DR
                   gets located in right absolute address.
                   Original boot area, PVD, SVD and VDT are copied to
                   the start of new, fixed ISO image.
                   Supported input images are: 2048, 2336,
                   2352 and 2056 bytes per sector. All of them are
                   converted to 2048 bytes per sector when writing
                   excluding 2056 image which is needed by Mac users.
*/
#define DM_FILES (1)
#define DM_WAV (2)
#define DM_2048 (4)
#define DM_FIX (8)
extern int dm_rip (const dm_image_t *image, int track_num, uint32_t flags);

#ifdef  __cplusplus
}
#endif
#endif  // LIBDISCMAGE_H
