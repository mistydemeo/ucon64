/*
snes.h - Super Nintendo support for uCON64

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
#ifndef SNES_H
#define SNES_H

extern const char *snes_usage[];

extern int snes_chk (st_rominfo_t *rominfo);
extern int snes_col (st_rominfo_t *rominfo);
extern int snes_dint (st_rominfo_t *rominfo);
extern int snes_f (st_rominfo_t *rominfo);
extern int snes_fig (st_rominfo_t *rominfo);
extern int snes_figs (st_rominfo_t *rominfo);
extern int snes_gd3 (st_rominfo_t *rominfo);
extern int snes_gdf (st_rominfo_t *rominfo);
extern int snes_init (st_rominfo_t *rominfo);
extern int snes_j (st_rominfo_t *rominfo);
extern int snes_k (st_rominfo_t *rominfo);
extern int snes_l (st_rominfo_t *rominfo);
extern int snes_mgd (st_rominfo_t *rominfo);
extern int snes_mgh (st_rominfo_t *rominfo);
extern int snes_n (st_rominfo_t *rominfo);
extern int snes_s (st_rominfo_t *rominfo);
extern int snes_smc (st_rominfo_t *rominfo);
extern int snes_swc (st_rominfo_t *rominfo);
extern int snes_swcs (st_rominfo_t *rominfo);
extern int snes_ufos (st_rominfo_t *rominfo);
#endif /* SNES_H */
