/*
doctor64.h - Bung Doctor 64 support for uCON64

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
#ifndef DOCTOR64_H
#define DOCTOR64_H

#ifdef BACKUP
extern int doctor64_read (char *filename, unsigned int parport);

extern int doctor64_write (char *filename, long start, long len,
                    unsigned int parport);

extern int doctor64_usage (int argc, char *argv[]);
#endif // BACKUP

#define doctor64_TITLE "Doctor V64\n19XX Bung Enterprises Ltd http://www.bung.com.hk"
#define doctor64_HEADER_START 0
#define doctor64_HEADER_LEN 0

#endif /* DOCTOR64_H */
