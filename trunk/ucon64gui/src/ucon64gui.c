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
#include "ucon64.h"

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

struct ucon64gui_
{

  char cmd[NAME_MAX];
  char rom[NAME_MAX];
  char file[NAME_MAX];

  char *ucon64_output;

  int console;
}
ucon64gui;

void ucon64_bottom(void)
{

  html2gui_br();
  html2gui_img(trans_1x3_xpm,0,0,0);
  html2gui_hr();
  html2gui_img(trans_1x3_xpm,0,0,0);
  html2gui_br();
  html2gui_img (icon_16x16_xpm, 48, 48, 0);

  html2gui_ ("uCON64gui "
#ifdef __GTK_GUI__
  "(GTK) "
#endif
"0.1.0 2002 by NoisyB "
  );


}


void
ucon64_system (void)
{
  switch(ucon64gui.console)
  {
    case ucon64_SNES:
      strcat(ucon64gui.cmd," -snes");
    break;
    
    default:
    break;
  }

//TODO pipe? ucon64gui.ucon64_output
  system (ucon64gui.cmd);
}

void
ucon64_rom (void)
{
  html2gui_file ("Select ROM", ucon64gui.rom);
}

void
ucon64_file (void)
{
//_text nutzen?
  html2gui_file ("Select ROM", ucon64gui.file);
}

void
ucon64_info (void)
{
  strcpy(ucon64gui.rom, html2gui_filename);
  sprintf (ucon64gui.cmd, "ucon64 \"%s\"", ucon64gui.rom);
  ucon64_system ();
}

void
ucon64_ls (void)
{
  strcpy (ucon64gui.cmd, "ucon64 -ls .");
  ucon64_system ();
}

void
ucon64_snes (void)
{
ucon64gui.console = ucon64_SNES;

//<html>
  html2gui_html (640, 400, 0);

  html2gui_img (snes_96_xpm, 96, 96, 0);
  html2gui_img (snes2_96_xpm, 96, 96, 0);
  html2gui_img (snes3_96_xpm, 96, 96, 0);

  html2gui_br ();
   html2gui_img (trans_1x3_xpm,0,0,0);
    html2gui_hr ();
    html2gui_img (trans_1x3_xpm,0,0,0);
    html2gui_br ();

  html2gui_button (ucon64_root, "Back", "Return", 10, 10, back_xpm);
  html2gui_button (ucon64_rom, "Open ROM", "Click here to select a Video Game Console ROM from a FileDialog", 100, 50, open_xpm);
  html2gui_br ();

  html2gui_button (ucon64_info, "Show info", "Click here to see information about ROM", 10, 10, NULL);

  ucon64_bottom();
//</html>
}

void
ucon64_root (void)
{

ucon64gui.console = ucon64_UNKNOWN;

//<html>
  html2gui_html (640, 400, 0);

  html2gui_title ("ucon64gui", icon_xpm);


  html2gui_button (ucon64_rom, "Open ROM", "Open ROM", 100, 50, open_xpm);

  html2gui_img (trans_1x3_xpm,0,0,0);
   html2gui_br();

  html2gui_("Miscellaneous options");
  html2gui_br();
  html2gui_button (ucon64_info, "Show info", "Click here to see information about ROM", 10, 10, NULL);
  html2gui_br();
  html2gui_img (trans_1x3_xpm,0,0,0);
//  html2gui_hr ();
  html2gui_("Console specific options");
  html2gui_br();
  html2gui_button (ucon64_snes, "Super Nintendo", "Options for Super Nintendo", 10, 10, NULL);

  ucon64_bottom();

//</html>
}

int
main (int argc, char *argv[])
{
  html2gui_start(argc, argv);

  ucon64_root();

  html2gui_end();

  return(0);
}
