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

#define ucon64gui_TITLE "uCON64gui"

#define ucon64gui_FORMTARGET "http://ucon64"

#ifndef MAXBUFSIZE
  #define MAXBUFSIZE 32768
#endif

extern struct ucon64gui_
{
  char cmd[FILENAME_MAX];
  char rom[FILENAME_MAX];
  char file[FILENAME_MAX];

  char console[4096];

  char ucon64_output[MAXBUFSIZE];

  int sub;

  int page; //current "html page"

  int hd; // header
  int ns; // not splitted

  char configfile[FILENAME_MAX];
}
ucon64gui;


extern void ucon64gui_root (void);

extern void ucon64gui_system (void);

extern void ucon64gui_output (char *output);


#endif /* UCON64GUI_H */
