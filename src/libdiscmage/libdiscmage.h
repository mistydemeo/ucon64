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
#include "inttypes.h"


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
  uint32_t track_start; // embedded? 
  uint32_t track_len;
  st_iso_header_t iso_header;


  uint32_t mode;
  uint32_t sector_size;
  uint32_t seek_header;
  uint32_t seek_ecc;

  char *desc;
  char *cdrdao_desc;
} dm_track_t;


typedef struct
{
  int type; // image type DM_CDI, DM_NRG, DM_TOC, etc.
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
extern void dm_set_gauge (void (*gauge) (int, int));
extern int dm_bin2iso (const dm_image_t *image);
extern int dm_rip (const dm_image_t *image);
extern int dm_isofix (const dm_image_t *image, int start_lba);
extern int dm_mktoc (const dm_image_t *image);
extern int dm_mkcue (const dm_image_t *image);
extern int dm_disc_read (const dm_image_t *image);
extern int dm_disc_write (const dm_image_t *image);

#endif  // LIBDISCMAGE_H
