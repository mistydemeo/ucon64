/*
sms.h - Sega Master System/Game Gear support for uCON64

Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2003 - 2004 dbjh


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

extern const st_getopt2_t sms_usage[];

extern UCON64_FILTER_TYPE (sms_gg);
extern UCON64_FILTER_TYPE (sms_ggd);
extern UCON64_FILTER_TYPE (sms_gge);
extern UCON64_FILTER_TYPE (sms_init);
extern UCON64_FILTER_TYPE (sms_smd);
extern UCON64_FILTER_TYPE (sms_smds);
extern UCON64_FILTER_TYPE (sms_chk);
extern UCON64_FILTER_TYPE (sms_sc);
extern UCON64_FILTER_TYPE (sms_mgdgg);
extern UCON64_FILTER_TYPE (sms_mgdsms);
extern UCON64_FILTER_TYPE (sms_multi);

//extern int sms_multi_fname (int truncate_size, char *fname);


#endif
