/*
libhtk_cfg.h - configfile for libhtk

written by 2002 Dirk Reinelt (d_i_r_k_@gmx.net)
           

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
#ifndef HTML2GUI_CFG_H
#define HTML2GUI_CFG_H
#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE

//#define USE_GETOPT  // crashes
//#define USE_HTML4

//#define SDL
#define GTK
//#define CURSES

#ifdef GTK
#define img_src *img_src
#endif


#endif // HTML2GUI_CFG_H
