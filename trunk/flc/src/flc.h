/*
flc
shows the FILE_ID.DIZ of archives/files
useful for FTP admins or people who have a lot to do with FILE_ID.DIZ

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
#ifndef FLC_H
#define FLC_H

#define MAXBUFSIZE 32768
#define FLC_MAX_FILES ARGS_MAX
#define FLC_ID_NAMES "*_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]"
//extern void flc_usage (int argc, char *argv[]);

#define FLC_VERSION_S "1.2.0"
//#define FLC_VERSION 104
#define FLC_CONFIG_VERSION 106

typedef struct
{
  int kb;
  int check;

  int html;
  int bbs;

  int sort;
  int bydate;
  int bysize;
  int byname;
  int fr;

  int files;

  char temp[FILENAME_MAX];
  char configfile[FILENAME_MAX];
  char output[FILENAME_MAX];
  char cwd[FILENAME_MAX];
} st_flc_t; // workflow

extern st_flc_t flc;


typedef struct
{
  char fname[FILENAME_MAX + 1];
  unsigned long size;
  int date;
  int checked;
#define FLC_MAX_ID_ROWS 20
#define FLC_MAX_ID_COLS 49
  char file_id[FLC_MAX_ID_ROWS + 1][FLC_MAX_ID_COLS + 1];
} st_file_t;

#endif
