/*
ucon64gui.c - a GUI for ucon64

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
#include "ucon64gui.h"

struct ucon64gui_
{

  char cmd[NAME_MAX];
  char rom[NAME_MAX];
  char file[NAME_MAX];

  char *ucon64_output;
}
ucon64;

int snes_window=0;

void
ucon64_system (void)
{
//TODO pipe?!
  system (ucon64.cmd);
}

void
ucon64_rom (void)
{
  html2gui_file ("Select ROM", ucon64.rom);
}

void
ucon64_file (void)
{
//_text nutzen
  html2gui_file ("Select ROM", ucon64.file);
}

void
ucon64_nfo (void)
{
  strcpy(ucon64.rom, html2gui_filename);
  sprintf (ucon64.cmd, "ucon64 \"%s\"", ucon64.rom);
  ucon64_system ();
}

void
ucon64_ls (void)
{
  strcpy (ucon64.cmd, "ucon64 -ls .");
  ucon64_system ();
}

void
ucon64_snes (void)
{
#include "xpm/snes.xpm"
//#include "xpm/icon.xpm"

//<html>
  html2gui_html (640, 400, 0);

//  html2gui_title ("ucon64gui_snes", icon_xpm);

  html2gui_img (snes_xpm, 10, 10, 0);

  html2gui_br ();

  html2gui_button (ucon64_main, "Back", "Return", 10, 10, NULL);
  
  html2gui_br ();

  html2gui_button (ucon64_nfo, "Show info", "Click here to see information about ROM", 10, 10, NULL);
//</html>
}

void
ucon64_main (void)
{
#include "xpm/icon.xpm"

//<html>
  html2gui_html (640, 400, 0);

  html2gui_title ("ucon64gui", icon_xpm);

  html2gui_img (icon_xpm, 1, 10, 0);

  html2gui_ ("uCON64_GUI 0.1.0 by NoisyB");

  html2gui_br ();
  html2gui_hr ();
  html2gui_br ();

  html2gui_button (ucon64_rom, "Open ROM", "Click here to select a Video Game Console ROM from a FileDialog", 10, 10, NULL);

  html2gui_button (ucon64_nfo, "Show info", "Click here to see information about ROM", 10, 10, NULL);

  html2gui_br ();
  html2gui_button (ucon64_snes, "Super Nintendo", "Super Nintendo specific options", 10, 10, NULL);
//</html>
}

int
main (int argc, char *argv[])
{
  html2gui_start(argc, argv);

  ucon64_main();

  html2gui_end();

  return(0);
}
