/*
snes.h - Super NES support for uCON64

Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2001 - 2004 dbjh
Copyright (c) 2002 - 2003 John Weidman


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

#include "backup/fig.h"                         // for st_fig_header_t

#define SNES_HEADER_START 0x7fb0
#define SNES_HIROM 0x8000
#define SNES_EROM 0x400000                      // "Extended" ROM, Hi or Lo

typedef enum { SWC = 1, GD3, UFO, FIG, MGD_SNES, SMC } snes_file_t;

extern const st_getopt2_t snes_usage[];


extern UCON64_FILTER_TYPE (snes_backup_header_info);
extern UCON64_FILTER_TYPE (snes_chk);
extern UCON64_FILTER_TYPE (snes_demirror);
extern UCON64_FILTER_TYPE (snes_densrt);
extern UCON64_FILTER_TYPE (snes_dint);
extern UCON64_FILTER_TYPE (snes_f);
extern UCON64_FILTER_TYPE (snes_fig);
extern UCON64_FILTER_TYPE (snes_figs);
extern UCON64_FILTER_TYPE (snes_gd3);
extern UCON64_FILTER_TYPE (snes_gd3s);
extern UCON64_FILTER_TYPE (snes_init);
extern UCON64_FILTER_TYPE (snes_j);
extern UCON64_FILTER_TYPE (snes_k);
extern UCON64_FILTER_TYPE (snes_l);
extern UCON64_FILTER_TYPE (snes_mgd);
extern UCON64_FILTER_TYPE (snes_mgh);
extern UCON64_FILTER_TYPE (snes_s);
extern UCON64_FILTER_TYPE (snes_smc);
extern UCON64_FILTER_TYPE (snes_swc);
extern UCON64_FILTER_TYPE (snes_swcs);
extern UCON64_FILTER_TYPE (snes_ufo);
extern UCON64_FILTER_TYPE (snes_ufos);


extern int snes_get_snes_hirom (void);
extern snes_file_t snes_get_file_type (void);
extern int snes_col (const char *color);
extern int snes_make_gd_names (const char *filename, st_ucon64_nfo_t *rominfo, char **names);
extern int snes_multi (int truncate_size, char *fname);
extern int snes_n (st_ucon64_nfo_t *rominfo, const char *name);
extern void snes_set_fig_header (st_ucon64_nfo_t *rominfo, st_fig_header_t *header);


#endif // SNES_H
