/*
ucon64gui.h - a GUI for ucon64

written by 2002 NoisyB (noisyb@gmx.net)
           

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
#ifndef UCON64GUI_H
#define UCON64GUI_H

#include "misc.h"
#include "ucon64.h"
#include "html2gui/src/html2gui.h"

//#include "xpm/snes.xpm"
//#include "xpm/snes2.xpm"
//#include "xpm/snes3.xpm"
#include "xpm/snes_96.xpm"
#include "xpm/snes2_96.xpm"
#include "xpm/snes3_96.xpm"
#include "xpm/back.xpm"
#include "xpm/open.xpm"
#include "xpm/icon.xpm"
#include "xpm/trans_1x3.xpm"
#include "xpm/icon_16x16.xpm"

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif

struct ucon64gui_
{

  char cmd[NAME_MAX];
  char rom[NAME_MAX];
  char file[NAME_MAX];

  char ucon64_output[MAXBUFSIZE];

  int console;
}
ucon64gui;


void ucon64_root (void);

void ucon64_bottom (void);

void ucon64_system (void);

void ucon64_rom (void);

void ucon64_file (void);

void ucon64_info (void);

void ucon64_ls (void);

void ucon64_e (void);

void ucon64_root (void);


#endif /* UCON64GUI_H */
