/*
flc_defines.h - Definitions for flc

Copyright (C) 1999-2004 by NoisyB (noisyb@gmx.net)

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
#ifndef FLC_DEFINES_H
#define FLC_DEFINES_H


#define ARGS_MAX 128
#define MAXBUFSIZE 32768
#define FLC_MAX_FILES ARGS_MAX
#define FLC_ID_NAMES "*_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]"
//extern void flc_usage (int argc, char *argv[]);

#define FLC_VERSION_S "1.2.0"
//#define FLC_VERSION 104
#define FLC_CONFIG_VERSION 106

#define FLC_KBYTE 1
#define FLC_CHECK (1 << 1)
#define FLC_HTML  (1 << 2)
#define FLC_BBS   (1 << 3)
#define FLC_SORT  (1 << 4)
#define FLC_DATE  (1 << 5)
#define FLC_SIZE  (1 << 6)
#define FLC_NAME  (1 << 7)
#define FLC_FR    (1 << 8)

enum
{
  FLC_UNKNOWN = 0,
  FLC_ACE,
  FLC_ZIP,
  FLC_LZH,
  FLC_RAR,
  FLC_TXT,
  FLC_ID3,
  FLC_MET,
  FLC_CUSTOM
};


#endif  // FLC_DEFINES_H
