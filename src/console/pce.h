/*
pce.c - PC-Engine support for uCON64

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
#ifndef PCE_H
#define PCE_H
extern const st_usage_t pcengine_usage[];

extern int pcengine_init (st_rominfo_t *rominfo);
extern int pcengine_mgd (st_rominfo_t *rominfo);
extern int pcengine_smg (st_rominfo_t *rominfo);
extern int pcengine_invert (st_rominfo_t *rominfo);

#endif // PCE_H
