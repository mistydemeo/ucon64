/*
    flc 1999/2000/2001 by NoisyB (noisyb@gmx.net)
    flc lists information about the FILEs (the current directory by default)
    But most important it shows the FILE_ID.DIZ of every file (if present)
    Very useful for FTP Admins or people who have a lot to do with FILE_ID.DIZ
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

#define MAXBUFSIZE 32768

int flc_usage(int argc, char *argv[]);

#define flc_TITLE "flc 1.0.0 1999/2000/2001 by NoisyB (noisyb@gmx.net)"

struct flc_
{
  int argc;
  //  char argv[128][4096];
  char *argv[128];

  long files;

  int kb;
  int html;

  int sort;
  int bydate;
  int bysize;
  int byname;
  int fr;
  
  char path[MAXBUFSIZE];

  char configfile[MAXBUFSIZE];
  char config[4096];
};

#define FID_LINES_MAX 20

struct files_
{
  unsigned long pos;
  char name[NAME_MAX+1];
  off_t size;
  unsigned long date;
  int checked;
  char files_id[FID_LINES_MAX+1][49];
};


#endif
