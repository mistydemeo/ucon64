/*
genesis.h - Sega Genesis/Mega Drive support for uCON64

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
#ifndef GENESIS_H
#define GENESIS_H
extern int genesis_1991 (st_rominfo_t *rominfo);
extern int genesis_chk (st_rominfo_t *rominfo);
extern int genesis_j (st_rominfo_t *rominfo);
extern int genesis_mgd (st_rominfo_t *rominfo);
extern int genesis_n (st_rominfo_t *rominfo, const char *name);
extern int genesis_n2 (st_rominfo_t *rominfo, const char *name);
extern int genesis_s (st_rominfo_t *rominfo);
extern int genesis_smd (st_rominfo_t *rominfo);
extern int genesis_smds (void);
extern int genesis_init (st_rominfo_t *rominfo);

extern const st_usage_t genesis_usage[];
#endif // GENESIS_H
