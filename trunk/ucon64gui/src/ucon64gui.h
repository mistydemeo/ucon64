/*
ucon64gui.h - a GUI for ucon64 (using libhtk framework)

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

#include "ucon64_defines.h"

#define UCON64GUI_HTMLTITLE "uCON64gui"
#define UCON64GUI_FORMTARGET "ucon64"
#define UCON64GUI_VERSION "1.0.0"

extern const char *ucon64gui_title;

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif

#define UCON64GUI_ROOT 1
#define UCON64GUI_CONFIG 2
#define UCON64GUI_SURFTO 3

typedef struct
{
  char rom[FILENAME_MAX];
  char file[FILENAME_MAX];

  const char *console;
//  int hd;
  const char *hd;                       // header
  const char *ns;                       // not splitted
  const char *interleaved;              // interleaved
  const char *snes_hirom;
  const char *bs;                  //Broadcast Satellaview dump

  char ucon64_output[MAXBUFSIZE];

  int sub;                      //sub screen insert back button, etc.

  char configfile[FILENAME_MAX];
}
ucon64gui_t;

extern ucon64gui_t ucon64gui;

extern void ucon64gui_divider (void);
extern void ucon64gui_spacer (void);


#endif // UCON64GUI_H