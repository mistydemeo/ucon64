/*
ssc.h - minimal support for Super Smart Card/SSC

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
#ifndef SCC_H
#define SCC_H

extern const st_usage_t ssc_usage[];

typedef struct st_ssc_header
{
  char pad[512];
} st_ssc_header_t;


#define SSC_HEADER_START 0
#define SSC_HEADER_LEN (sizeof (st_ssc_header_t))
#endif // SCC_H