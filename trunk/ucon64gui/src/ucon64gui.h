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

  char configfile[NAME_MAX];
}
ucon64gui;


void ucon64gui_root (void);

void ucon64gui_bottom (void);

void ucon64gui_top (void);

void ucon64gui_system (void);

void ucon64gui_rom (void);

void ucon64gui_file (void);

void ucon64gui_info (void);

void ucon64gui_ls (void);

void ucon64gui_e (void);

void ucon64gui_root (void);


#endif /* UCON64GUI_H */
