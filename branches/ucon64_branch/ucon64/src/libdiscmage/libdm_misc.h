/*
libdm_misc.h - libdiscmage miscellaneous

written by 2002 - 2003 NoisyB (noisyb@gmx.net)
           2002 - 2003 dbjh


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
  ALREADY_2048,
  ALPHA
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


enum {
  DM_UNKNOWN = -1,
  DM_CUE = 1,
  DM_TOC,
  DM_CDI,
  DM_NRG,
//  DM_CCD,
  DM_OTHER
};

/*
  writewavheader()    write header for a wav file
  dm_get_track_desc() returns a string like "MODE1/2352" depending on the 
                      mode and sector_size specified; if cue == FALSE
                      it will return the string in TOC format
*/
//extern void writewavheader (FILE * fdest, int track_length);
extern const char *dm_get_track_desc (int mode, int sector_size, int cue);

extern const char pvd_magic[];
extern const char svd_magic[];
extern const char vdt_magic[];
#endif  // LIBDM_MISC_H
