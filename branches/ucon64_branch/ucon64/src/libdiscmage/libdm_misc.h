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

typedef struct
{
  int mode;
  int seek_header; // sync, head, sub
  int sector_size; // data
  int seek_ecc;    // EDC, zero, ECC, spare
  const char *cue;
  const char *toc;
} st_track_probe_t;
extern const st_track_probe_t track_probe[];

/*
  dm_track_init()     fillup current dm_track_t
  dm_free()           free all dm_track_t, dm_session_t and dm_image_t recursively
  writewavheader()    write header for a wav file
  dm_get_track_desc() returns a string like "MODE1/2352" depending on the 
                        mode and sector_size specified; if cue == FALSE
                        it will return the string in TOC format
  callibrate()        a brute force function that tries to find a iso header
                        or anything else that could identify a file as an
                        image (can be very slow)
*/
extern const dm_track_t *dm_track_init (dm_track_t *track, FILE *fh);
extern int dm_free (dm_image_t *image);
//extern void writewavheader (FILE * fdest, int track_length);
extern const char *dm_get_track_desc (int mode, int sector_size, int cue);
//extern int callibrate (const char *fname, int track_num);


/*
  dm_lba_to_msf() convert LBA to minutes, seconds, frames
  dm_msf_to_lba() convert minutes, seconds, frames to LBA

  LBA represents the logical block address for the CD-ROM absolute
  address field or for the offset from the beginning of the current track
  expressed as a number of logical blocks in a CD-ROM track relative
  address field.
  MSF represents the physical address written on CD-ROM discs,
  expressed as a sector count relative to either the beginning of the
  medium or the beginning of the current track.

  dm_bcd_to_int() convert BCD to integer
  dm_int_to_bcd() convert integer to BCD
*/
extern int dm_lba_to_msf (int lba, int *m, int *s, int *f);
extern int dm_msf_to_lba (int m, int s, int f, int force_positive);
extern int dm_bcd_to_int (int b);
extern int dm_int_to_bcd (int i);
#endif  // LIBDM_MISC_H
