/*
nds.h - Nintendo DS support for uCON64

Copyright (c) 2005 NoisyB


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
#ifndef NDS_H
#define NDS_H

extern const st_getopt2_t nds_usage[];


extern UCON64_FILTER_TYPE (nds_init);
extern UCON64_FILTER_TYPE (nds_logo);
extern UCON64_FILTER_TYPE (nds_chk);
extern UCON64_FILTER_TYPE (nds_sc);


extern int nds_n (st_ucon64_nfo_t *rominfo, const char *name);

#endif
