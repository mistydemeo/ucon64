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

#ifdef BACKUP
extern int doctor64jr_read (char *filename, unsigned int parport);

extern int doctor64jr_write (char *filename, long start, long len,
                      unsigned int parport);
extern void doctor64jr_usage (void);
#endif // BACKUP

extern char *doctor64jr_title;

#define doctor64jr_HEADER_START 0
#define doctor64jr_HEADER_LEN 0
#endif /* DOCTOR64JR_H */
