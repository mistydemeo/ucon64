/*
cue.h - CUE support for libdiscmage

written by 2002 - 2003 NoisyB (noisyb@gmx.net)


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
#ifndef CUE_H
#define CUE_H
extern const st_track_desc_t cue_track_desc[];

extern int dm_mkcue (const dm_image_t *image);
extern int cue_init (dm_image_t *image);
#endif // CUE_H
