/*
sms.h - Sega Master System/Game Gear support for uCON64

Copyright (c) 1999 - 2001                    NoisyB
Copyright (c) 2003 - 2004, 2015, 2017 - 2018 dbjh


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
#ifndef SMS_H
#define SMS_H

#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"


extern const st_getopt2_t sms_usage[];

extern int sms_gg (st_ucon64_nfo_t *rominfo);
extern int sms_ggd (st_ucon64_nfo_t *rominfo);
extern int sms_gge (st_ucon64_nfo_t *rominfo);
extern int sms_init (st_ucon64_nfo_t *rominfo);
extern int sms_mgd (st_ucon64_nfo_t *rominfo, int console);
extern int sms_smd (st_ucon64_nfo_t *rominfo);
extern int sms_smds (void);
extern int sms_chk (st_ucon64_nfo_t *rominfo);
extern int sms_multi (unsigned int truncate_size);

#endif
