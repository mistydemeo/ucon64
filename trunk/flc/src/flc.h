/*
    flc 1999/2000/2001 by NoisyB (noisyb@gmx.net)
    A filelisting creator which uses FILE_ID.DIZ from archives/files
    to generate a bbs style filelisting
    Copyright (C) 1999/2000/2001 by NoisyB (noisyb@gmx.net)

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

#include <limits.h>
#include "misc.h"

#define flc_NAME 0
#define flc_FILE 1

int flc_usage(int argc, char *argv[]);

#define flc_TITLE "flc 0.9.3 1999/2000/2001 by NoisyB (noisyb@gmx.net)"

struct flc_
{
  int argc;
  //  char argv[128][4096];
  char *argv[128];

  int kb;
  int files;
  int sort;
  char path[4096];
};

struct file_
{
  unsigned long pos;
  char name[NAME_MAX+1];
  off_t size;
  time_t date;
  int checked;
#define FID_LINES_MAX 20
  char file_id[FID_LINES_MAX+1][49];
};


#endif
