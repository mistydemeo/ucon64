/*
doctor64jr.h - Bung Doctor 64jr support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)


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
#ifndef DOCTOR64JR_H
#define DOCTOR64JR_H

#ifdef PARALLEL
extern int doctor64jr_read (const char *filename, unsigned int parport);
extern int doctor64jr_write (const char *filename, unsigned int parport);
#endif // PARALLEL

extern const st_usage_t doctor64jr_usage[];

#define DOCTOR64JR_HEADER_START 0
#define DOCTOR64JR_HEADER_LEN 0
#endif // DOCTOR64JR_H
