/*
format.c - support of different image formats for libdiscmage

written by 2004 NoisyB (noisyb@gmx.net)


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
#ifndef FORMAT_H
#define FORMAT_H
#include "cdi.h"
#include "cue.h"
#include "nero.h"
#include "other.h"
#include "toc.h"
#include "ccd.h"

/*
  callibrate()        a brute force function that tries to find a iso header
                      or anything else that could identify a file as an
                      image (can be very slow)
*/
//extern int callibrate (const char *fname, int track_num);

extern int format_track_init (dm_track_t *track, FILE *fh);
extern int format_free (dm_image_t *image);
#endif  // FORMAT_H
