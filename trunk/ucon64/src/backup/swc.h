/*
swc.h - Super Wild Card support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh


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
#ifndef SWC_H
#define SWC_H

extern const st_usage_t swc_usage[];

// For the header format, see ffe.h
typedef struct st_swc_header
{
/*
  Don't create fields that are larger than one byte! For example size_low and size_high
  could be combined in one unsigned short int. However, this gives problems with little
  endian vs. big endian machines (e.g. writing the header to disk).
*/
  unsigned char size_low;
  unsigned char size_high;
  unsigned char emulation;
  unsigned char pad[5];
  unsigned char id1;
  unsigned char id2;
  unsigned char type;
  unsigned char pad2[501];
} st_swc_header_t;

#define SWC_HEADER_START 0
#define SWC_HEADER_LEN (sizeof (st_swc_header_t))

#ifdef PARALLEL
extern int swc_read_rom (const char *filename, unsigned int parport, int superdump);
extern int swc_write_rom (const char *filename, unsigned int parport, int enableRTS);
extern int swc_read_sram (const char *filename, unsigned int parport);
extern int swc_write_sram (const char *filename, unsigned int parport);
extern void swc_unlock (unsigned int parport);
#endif // PARALLEL

#endif // SWC_H
