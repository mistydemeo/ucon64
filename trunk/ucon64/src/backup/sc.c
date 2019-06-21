/*
sc.c - support for SuperCard

Copyright (c) 2004 NoisyB


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
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <stdio.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "backup/sc.h"


const st_getopt2_t sc_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Super Card (CF to GBA Adapter)"
      /* "2004 Super Card http://www.supercard.cn" */,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };
