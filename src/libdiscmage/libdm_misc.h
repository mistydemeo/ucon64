/*
libdm_misc.h - libdiscmage miscellaneous

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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  HAVE_CDROM_H
#include <linux/cdrom.h>
#else
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

/* CD-ROM address types (cdrom_tocentry.cdte_format) */
#define	CDROM_LBA 0x01 /* "logical block": first frame is #0 */
#define	CDROM_MSF 0x02 /* "minute-second-frame": binary, not bcd here! */

/* bit to tell whether track is data or audio (cdrom_tocentry.cdte_ctrl) */
#define	CDROM_DATA_TRACK	0x04

/* The leadout track is always 0xAA, regardless of # of tracks on disc */
#define	CDROM_LEADOUT		0xAA

#if 0
#define ISODCL(from, to) (to - from + 1)
typedef struct st_iso_header
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
#endif
#endif  // HAVE_CDROM_H


/*
  dm_msf_t struct for LBA <-> MSF conversions

  dm_lba_to_msf() convert LBA to minutes, seconds, frames
  dm_msf_to_lba() convert minutes, seconds, frames to LBA
  dm_from_bcd()   convert BCD to integer
  dm_to_bcd()     convert integer to BCD
*/
typedef struct
{
  uint8_t    cdmsf_min0;     /* start minute */
  uint8_t    cdmsf_sec0;     /* start second */
  uint8_t    cdmsf_frame0;   /* start frame */
  uint8_t    cdmsf_min1;     /* end minute */
  uint8_t    cdmsf_sec1;     /* end second */
  uint8_t    cdmsf_frame1;   /* end frame */
} dm_msf_t;
extern int lba_to_msf (int lba, dm_msf_t * mp);
extern int msf_to_lba (int m, int s, int f, int force_positive);
extern int from_bcd (int b);
extern int to_bcd (int i);


/*
  libdm messages
  
  usage example: fprintf (stdout, dm_msg[DEPRECATED], filename);
*/
enum
{
  DEPRECATED = 0,
  UNKNOWN_IMAGE,
  ALREADY_2048
};
extern const char *dm_msg[];

// get file size of fname
extern int fsize (const char *fname);


// wav header
typedef struct
{
  unsigned char magic[4];
  uint32_t total_length;
  unsigned char type[4];
  unsigned char fmt[4];
  uint32_t header_length;
  unsigned short format;
  unsigned short channels;
  uint32_t samplerate;
  uint32_t bitrate;
  unsigned short blockalign;
  unsigned short bitspersample;
  unsigned char data[4];
  uint32_t data_length;
} wav_header_t;


#define ISO_HEADER_START 0
#define ISO_HEADER_LEN (sizeof (st_iso_header_t))

#define DM_UNKNOWN (-1)

#define DM_TOC 1
#define DM_CUE 2
#define DM_CDI 3
#define DM_NRG 4
#define DM_CCD 5

//TODO: remove these
#define READ_BUF_SIZE  (1024*1024)
#define WRITE_BUF_SIZE (1024*1024)

#define DM_TRACK_TYPE (TRUE)
#define DM_SAVE_AS_ISO (TRUE)
#define DM_PREGAP (TRUE)
#define DM_CONVERT (TRUE)
#define DM_FULLDATA (TRUE)
#define DM_CUTALL (TRUE)
#define DM_CUTFIRST (TRUE)
#define DM_DO_CONVERT (TRUE)
#define DM_DO_CUT (TRUE)

#define MAXBUFSIZE 32768


#endif  // LIBDM_MISC_H
