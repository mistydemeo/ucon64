/*
    flc 1999-2002 by NoisyB (noisyb@gmx.net)
    flc lists information about the FILEs (the current directory by default)
    But most important it shows the FILE_ID.DIZ of every file (if present)
    Very useful for FTP Admins or people who have a lot to do with FILE_ID.DIZ
    Copyright (C) 1999-2002 by NoisyB (noisyb@gmx.net)

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

extern void flc_usage(int argc, char *argv[]);

#define flc_VERSION "1.0.1"

#define flc_TITLE "flc " flc_VERSION " 1999-2002 by NoisyB (noisyb@gmx.net)"

typedef struct st_flc
{
  int argc;
  //  char argv[128][4096];
  char *argv[128];

//  long files;

  int kb;
  int html;

  int sort;
  int bydate;
  int bysize;
  int byname;
  int fr;

  int check;
  
  char tmppath[FILENAME_MAX];

  char path[FILENAME_MAX];

  char configfile[FILENAME_MAX];
  char config[4096];
}st_flc_t;

extern st_flc_t flc;

#define FID_LINES_MAX 20

typedef struct st_sub
{
  char name[FILENAME_MAX+1];
  off_t size;
  unsigned long date;
  int checked;
  char file_id[FID_LINES_MAX+1][49];
}st_sub_t;

struct st_file
{
  struct st_file *next;

  st_sub_t sub;
};

typedef struct st_file st_file_t;

#endif
