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
  dm_lba_to_msf() convert LBA to minutes, seconds, frames
  dm_msf_to_lba() convert minutes, seconds, frames to LBA
  dm_from_bcd()   convert BCD to integer
  dm_to_bcd()     convert integer to BCD
*/

extern int32_t dm_lba_to_msf (int32_t lba, dm_msf_t * mp);
extern int32_t dm_msf_to_lba (int32_t m, int32_t s, int32_t f, int32_t force_positive);
extern int32_t dm_from_bcd (int32_t b);
extern int32_t dm_to_bcd (int32_t i);


/*
  libdm messages
  
  usage example: fprintf (stdout, libdm_msg[DEPRECATED], filename);
*/
enum
{
  DEPRECATED = 0
};
                          
extern const char *libdm_msg[];

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

#define ISO_HEADER_START 0
#define ISO_HEADER_LEN (sizeof (st_iso_header_t))


#define DEFAULT_FORMAT   0
#define ISO_FORMAT       1
#define BIN_FORMAT       2
#define CDI_FORMAT       3
#define NRG_FORMAT       4
#define CCD_FORMAT       5

#define READ_BUF_SIZE  (1024*1024)
#define WRITE_BUF_SIZE (1024*1024)

extern int fsize (const char *filename);

#endif  // LIBDM_MISC_H
