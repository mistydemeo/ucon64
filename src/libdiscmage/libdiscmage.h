/*
libdiscmage.h - DISCmage support routines

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
              ? - 2002 Dext

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
#include <inttypes.h>
#include "config.h"
#include "quick_io.h"
#include "misc.h"
#include "sheets.h"
#include "unzip.h"
#ifdef  __linux__
#include <linux/cdrom.h>
#else
/*
  some useful structs and defines from <linux/cdrom.h>
*/

/*
 * -- <linux/cdrom.h>
 * General header file for linux CD-ROM drivers 
 * Copyright (C) 1992         David Giller, rafetmad@oxy.edu
 *               1994, 1995   Eberhard Moenkeberg, emoenke@gwdg.de
 *               1996         David van Leeuwen, david@tm.tno.nl
 *               1997, 1998   Erik Andersen, andersee@debian.org
 *               1998-2000    Jens Axboe, axboe@suse.de
 */
 
/*******************************************************
 * CDROM IOCTL structures
 *******************************************************/

/* Address in MSF libdiscmage */
struct cdrom_msf0		
{
	__u8	minute;
	__u8	second;
	__u8	frame;
};

/* Address in either MSF or logical libdiscmage */
union cdrom_addr		
{
	struct cdrom_msf0	msf;
	int			lba;
};

/* This struct is used by the CDROMPLAYMSF ioctl */ 
struct cdrom_msf 
{
	__u8	cdmsf_min0;	/* start minute */
	__u8	cdmsf_sec0;	/* start second */
	__u8	cdmsf_frame0;	/* start frame */
	__u8	cdmsf_min1;	/* end minute */
	__u8	cdmsf_sec1;	/* end second */
	__u8	cdmsf_frame1;	/* end frame */
};


/*
 * A CD-ROM physical sector size is 2048, 2052, 2056, 2324, 2332, 2336, 
 * 2340, or 2352 bytes long.  

*         Sector types of the standard CD-ROM data libdiscmages:
 *
 * libdiscmage   sector type               user data size (bytes)
 * -----------------------------------------------------------------------------
 *   1     (Red Book)    CD-DA          2352    (CD_FRAMESIZE_RAW)
 *   2     (Yellow Book) Mode1 Form1    2048    (CD_FRAMESIZE)
 *   3     (Yellow Book) Mode1 Form2    2336    (CD_FRAMESIZE_RAW0)
 *   4     (Green Book)  Mode2 Form1    2048    (CD_FRAMESIZE)
 *   5     (Green Book)  Mode2 Form2    2328    (2324+4 spare bytes)
 *
 *
 *       The layout of the standard CD-ROM data libdiscmages:
 * -----------------------------------------------------------------------------
 * - audio (red):                  | audio_sample_bytes |
 *                                 |        2352        |
 *
 * - data (yellow, mode1):         | sync - head - data - EDC - zero - ECC |
 *                                 |  12  -   4  - 2048 -  4  -   8  - 276 |
 *
 * - data (yellow, mode2):         | sync - head - data |
 *                                 |  12  -   4  - 2336 |
 *
 * - XA data (green, mode2 form1): | sync - head - sub - data - EDC - ECC |
 *                                 |  12  -   4  -  8  - 2048 -  4  - 276 |
 *
 * - XA data (green, mode2 form2): | sync - head - sub - data - Spare |
 *                                 |  12  -   4  -  8  - 2324 -  4    |
 *
 */

/* Some generally useful CD-ROM inlibdiscmageion -- mostly based on the above */
#define CD_MINS              74 /* max. minutes per CD, not really a limit */
#define CD_SECS              60 /* seconds per minute */
#define CD_FRAMES            75 /* frames per second */
#define CD_SYNC_SIZE         12 /* 12 sync bytes per raw data frame */
#define CD_MSF_OFFSET       150 /* MSF numbering offset of first frame */
#define CD_CHUNK_SIZE        24 /* lowest-level "data bytes piece" */
#define CD_NUM_OF_CHUNKS     98 /* chunks per frame */
#define CD_FRAMESIZE_SUB     96 /* subchannel data "frame" size */
#define CD_HEAD_SIZE          4 /* header (address) bytes per raw data frame */
#define CD_SUBHEAD_SIZE       8 /* subheader bytes per raw XA data frame */
#define CD_EDC_SIZE           4 /* bytes EDC per most raw data frame types */
#define CD_ZERO_SIZE          8 /* bytes zero per yellow book mode 1 frame */
#define CD_ECC_SIZE         276 /* bytes ECC per most raw data frame types */
#define CD_FRAMESIZE       2048 /* bytes per frame, "cooked" mode */
#define CD_FRAMESIZE_RAW   2352 /* bytes per frame, "raw" mode */
#define CD_FRAMESIZE_RAWER 2646 /* The maximum possible returned bytes */ 
/* most drives don't deliver everything: */
#define CD_FRAMESIZE_RAW1 (CD_FRAMESIZE_RAW-CD_SYNC_SIZE) /*2340*/
#define CD_FRAMESIZE_RAW0 (CD_FRAMESIZE_RAW-CD_SYNC_SIZE-CD_HEAD_SIZE) /*2336*/

#define CD_XA_HEAD        (CD_HEAD_SIZE+CD_SUBHEAD_SIZE) /* "before data" part of raw XA frame */
#define CD_XA_TAIL        (CD_EDC_SIZE+CD_ECC_SIZE) /* "after data" part of raw XA frame */
#define CD_XA_SYNC_HEAD   (CD_SYNC_SIZE+CD_XA_HEAD) /* sync bytes + header of XA frame */

/* CD-ROM address types (cdrom_tocentry.cdte_libdiscmage) */
#define	CDROM_LBA 0x01 /* "logical block": first frame is #0 */
#define	CDROM_MSF 0x02 /* "minute-second-frame": binary, not bcd here! */

/* bit to tell whether track is data or audio (cdrom_tocentry.cdte_ctrl) */
#define	CDROM_DATA_TRACK	0x04

/* The leadout track is always 0xAA, regardless of # of tracks on disc */
#define	CDROM_LEADOUT		0xAA
#endif // <linux/cdrom.h>

#if 0
typedef struct
  {
    char *extension;
    char *magic;                // magic bytes in file?
    int32_t (*play) (const char *, int32_t, int32_t);
  } libdiscmage_t;
  
extern const libdiscmage_t libdiscmage[];
#endif

#define MODE1_2048 0
#define MODE1_2352 1
#define MODE2_2336 2
#define MODE2_2352 3
// Macintosh
#define MODE2_2056 4

#define DEFAULT_FORMAT   0
#define ISO_FORMAT       1
#define BIN_FORMAT       2


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
  int32_t track_type,save_as_iso,pregap,convert, fulldata, cutall, cutfirst;
  char do_convert, do_cut;
}
st_image_t;

extern int32_t bcd_conv (int32_t value);

int lba_to_msf (uint32_t lba, struct cdrom_msf * mp);
uint32_t msf_to_lba (int m, int s, int f, int force_positive);
              

extern int32_t trackmode_probe (const char *image);

extern int32_t bin2iso (const char *image, int32_t track_mode);
extern int32_t cdirip (const char *image);
//extern int32_t nerorip (const char *image);
extern int32_t cdi2nero (const char *image);
//extern int32_t isofix (const char *image);

typedef struct
{
  uint32_t mode;
  uint32_t sector_size;
  uint32_t seek_header;
  uint32_t seek_ecc;
  char *common;
  char *cdrdao;
}
st_track_modes_t;

extern const st_track_modes_t track_modes[];

#include "cdi.h"
#include "bin.h"
#include "iso.h"
#include "nero.h"


#endif  // LIBDISCMAGE_H
