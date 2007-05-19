/*
genesis.h - Sega Genesis/Mega Drive support for uCON64

Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2002 - 2003 dbjh


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

typedef enum { SMD = 1, BIN, MGD_GEN } genesis_file_t;

extern const st_getopt2_t genesis_usage[];

extern genesis_file_t genesis_get_file_type (void);


extern UCON64_FILTER_TYPE (genesis_1991);
extern UCON64_FILTER_TYPE (genesis_chk);
extern UCON64_FILTER_TYPE (genesis_j);
extern UCON64_FILTER_TYPE (genesis_s);
extern UCON64_FILTER_TYPE (genesis_smd);
extern UCON64_FILTER_TYPE (genesis_bin);
extern UCON64_FILTER_TYPE (genesis_mgd);
extern UCON64_FILTER_TYPE (genesis_init);
extern UCON64_FILTER_TYPE (genesis_f);
extern UCON64_FILTER_TYPE (genesis_smds);
extern UCON64_FILTER_TYPE (genesis_n);
extern UCON64_FILTER_TYPE (genesis_n2);
extern UCON64_FILTER_TYPE (genesis_multi);

//extern int genesis_multi_fname (int truncate_size, char *fname);

#endif
