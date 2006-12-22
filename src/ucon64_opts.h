/*
ucon64_opts.h - switch()'es for all uCON64 options

Copyright (c) 2002 - 2004 NoisyB
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
#ifndef UCON64_OPTS_H
#define UCON64_OPTS_H
typedef struct
{
  int option;
  int console;                                  // UCON64_SNES, etc...
  int flags;                               // WF_INIT, etc..
  int (*func) (st_ucon64_t *);
  const char *optarg;  // option argument
} st_ucon64_filter_t;


extern st_ucon64_filter_t ucon64_filter[];
#endif // UCON64_OPTS_H
