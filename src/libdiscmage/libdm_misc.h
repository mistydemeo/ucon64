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
// wav header
typedef struct
{
  uint8_t magic[4];
  uint32_t total_length;
  uint8_t type[4];
  uint8_t fmt[4];
  uint32_t header_length;
  uint16_t format;
  uint16_t channels;
  uint32_t samplerate;
  uint32_t bitrate;
  uint16_t blockalign;
  uint16_t bitspersample;
  uint8_t data[4];
  uint32_t data_length;
} wav_header_t;


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


/*
  dm_track_init()     fillup current dm_track_t
  dm_free()           free all dm_track_t, dm_session_t and dm_image_t recursively
*/
extern const dm_track_t *dm_track_init (dm_track_t *track, FILE *fh);
extern int dm_free (dm_image_t *image);


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
#endif  // LIBDM_MISC_H
