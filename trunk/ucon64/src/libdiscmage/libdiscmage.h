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

//TODO: make this dynamic
#define DM_MAX_TRACKS (0xfff)


#define DM_UNKNOWN (-1)

#define DM_SHEET (1)
#define DM_CUE (DM_SHEET)
#define DM_TOC (DM_SHEET)
#define DM_CDI (2)
#define DM_NRG (3)
#define DM_CCD (4)


#define ISODCL(from, to) (to - from + 1)
typedef struct
{
  char type[ISODCL (1, 1)];   /* 711 */
  char id[ISODCL (2, 6)];
  char version[ISODCL (7, 7)];        /* 711 */
  char unused1[ISODCL (8, 8)];
  char system_id[ISODCL (9, 40)];     /* achars */
  char volume_id[ISODCL (41, 72)];    /* dchars */
  char unused2[ISODCL (73, 80)];
  char volume_space_size[ISODCL (81, 88)];    /* 733 */
  char unused3[ISODCL (89, 120)];
  char volume_set_size[ISODCL (121, 124)];    /* 723 */
  char volume_sequence_number[ISODCL (125, 128)];     /* 723 */
  char logical_block_size[ISODCL (129, 132)]; /* 723 */
  char path_table_size[ISODCL (133, 140)];    /* 733 */
  char type_l_path_table[ISODCL (141, 144)];  /* 731 */
  char opt_type_l_path_table[ISODCL (145, 148)];      /* 731 */
  char type_m_path_table[ISODCL (149, 152)];  /* 732 */
  char opt_type_m_path_table[ISODCL (153, 156)];      /* 732 */
  char root_directory_record[ISODCL (157, 190)];      /* 9.1 */
  char volume_set_id[ISODCL (191, 318)];      /* dchars */
  char publisher_id[ISODCL (319, 446)];       /* achars */
  char preparer_id[ISODCL (447, 574)];        /* achars */
  char application_id[ISODCL (575, 702)];     /* achars */
  char copyright_file_id[ISODCL (703, 739)];  /* 7.5 dchars */
  char abstract_file_id[ISODCL (740, 776)];   /* 7.5 dchars */
  char bibliographic_file_id[ISODCL (777, 813)];      /* 7.5 dchars */
  char creation_date[ISODCL (814, 830)];      /* 8.4.26.1 */
  char modification_date[ISODCL (831, 847)];  /* 8.4.26.1 */
  char expiration_date[ISODCL (848, 864)];    /* 8.4.26.1 */
  char effective_date[ISODCL (865, 881)];     /* 8.4.26.1 */
  char file_structure_version[ISODCL (882, 882)];     /* 711 */
  char unused4[ISODCL (883, 883)];
  char application_data[ISODCL (884, 1395)];
  char unused5[ISODCL (1396, 2048)];
} st_iso_header_t;


typedef struct
{
// TODO?: replace those uint32_t with uint64_t or so...

// some formats use to have the track "embedded" (like: cdi, nrg, etc..)
// this is the start offset inside the image
  uint32_t track_start; // in bytes
  
  int pregap_len; // in sectors
  int start_lba;  // in sectors?

  uint32_t track_len; // in sectors
  uint32_t total_len; // int sectors; pregap_len + track_len (less if the track is truncated)

  st_iso_header_t iso_header;

  int mode;        // 0 == AUDIO, 1 == MODE1, 2 == MODE2
  int sector_size; // in bytes; includes seek_header + seek_ecc
  int seek_header; // in bytes
  int seek_ecc;    // in bytes

  char *desc;
  char *cdrdao_desc;
} dm_track_t;


typedef struct
{
  int type;               // image type DM_CDI, DM_NRG, DM_TOC, etc.
  char *desc;             // like type but more verbose
  int flags;              // DM_FIX, ...
  char fname[FILENAME_MAX];
  uint32_t session[DM_MAX_TRACKS + 1]; // array with tracks per session
  int sessions;
  dm_track_t track[DM_MAX_TRACKS];
  int tracks;
} dm_image_t;


/*
  dm_get_version()  returns version of libdiscmage as uint32_t
TODO: DM_FILES   rip files from track instead of track
  DM_RIP_2048    (bin2iso) convert binary Mx/xxxx image to M1/2048
  DM_FIX         (isofix) takes an ISO image with PVD pointing
                 to bad DR offset and add padding data so actual DR
                 gets located in right absolute address.
                 Original boot area, PVD, SVD and VDT are copied to
                 the start of new, fixed ISO image.
                 Supported input images are: 2048, 2336,
                 2352 and 2056 bytes per sector. All of them are
                 converted to 2048 bytes per sector when writing
                 excluding 2056 image which is needed by Mac users.
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
  dm_rip()       rip track from image
  dm_read()      read single sector from track (in image)
TODO: dm_write()     write single sector to track (in image)
  dm_mktoc()     automagically generates toc sheets
  dm_mkcue()     automagically generates cue sheets
  dm_disc_read() deprecated reading or writing images is done
                 by those scripts in contrib/
  dm_disc_write()deprecated reading or writing images is done
                 by those scripts in contrib/
*/
extern uint32_t dm_get_version (void);
extern void dm_set_gauge (void (*gauge) (int, int));

#define DM_RDONLY (1)
//#define DM_WRONLY (2)
//#define DM_RDWR (4)
//#define DM_CREAT (8)
#define DM_FIX (16)
#define DM_2048 (32)
#define DM_FILES (64)
extern dm_image_t *dm_open (const char *fname, uint32_t flags);
extern dm_image_t *dm_reopen (const char *fname, uint32_t flags, dm_image_t *image);
extern int dm_close (dm_image_t *image);

//extern int dm_fseek (FILE *fp, int track_num, int how);
extern FILE *dm_fdopen (dm_image_t *image, int track_num, const char *mode);

extern int dm_read (char buffer, int track_num, int sector, const dm_image_t *image);
extern int dm_write (const char buffer, int track_num, int sector, const dm_image_t *image);

extern int dm_mktoc (const dm_image_t *image);
extern int dm_mkcue (const dm_image_t *image);

extern int dm_rip (const dm_image_t *image, int track_num);

extern int dm_disc_read (const dm_image_t *image);
extern int dm_disc_write (const dm_image_t *image);

#ifdef  __cplusplus
}
#endif
#endif  // LIBDISCMAGE_H
