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
#include "snes/snes.h"

#include "backup/swc.h"
#include "top.h"
#include "bottom.h"


void html2gui_system(char *name, char *value)
{
printf("%s=%s\n", name, value);
fflush(stdout);

strcpy(ucon64gui.rom,value);

if(!strdcmp(value,"ucon64gui_rom"))  html2gui_input_file ("ROM=", ucon64gui.rom, "Select ROM");
if(!strdcmp(value,"ucon64gui_file"))  html2gui_input_file ("FILE=", ucon64gui.file, "Select ROM");

if(!strdcmp(value,"ucon64gui_snes")) return ucon64gui_snes();
if(!strdcmp(value,"ucon64gui_root")) return ucon64gui_root();
if(!strdcmp(value,"ucon64gui_swc")) return ucon64gui_swc();

//ucon64gui_root();

return;

/*
//  FILE *fh;
  char buf[MAXBUFSIZE];

  switch (ucon64gui.console)
    {
    case ucon64_SNES:
      strcat (ucon64gui.cmd, " -snes");
      break;

    default:
      break;
    }

  sprintf(buf,"xterm -e %s|less &",ucon64gui.cmd);

  html2gui_html_end();
  system (buf);
  html2gui_html(640,400,0);
*/

/*
  if (!(fh = popen (buf, "r")))
    {
      strcpy (ucon64gui.ucon64_output, "");
      return;
    }

  while (fgets (buf, sizeof buf, fh) != NULL)
    {
      strcat (ucon64gui.ucon64_output, buf);
    }
  pclose (fh);
*/
}



int
main (int argc, char *argv[])
{
#ifdef	__DOS__
  strcpy (ucon64gui.configfile, "ucon64.cfg");
#else
  sprintf (ucon64gui.configfile, "%s%c.ucon64rc", getenv ("HOME"), FILE_SEPARATOR);
#endif


  html2gui_start (argc, argv);

  ucon64gui_root ();

  html2gui_end ();

  return (0);
}


















void
ucon64gui_root (void)
{
#include "xpm/trans_1x3.xpm"
#include "xpm/icon.xpm"

  ucon64gui.console = ucon64_UNKNOWN;
  html2gui_html (0, 0, 0);

  html2gui_title ("uCON64gui", icon_xpm);

  
  ucon64gui_top();

  html2gui_ ("Console specific options");
  html2gui_br ();
  html2gui_input_submit ("Super Nintendo", "ucon64gui_snes", "(-snes) Options for Super Nintendo");

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0, NULL);
//  html2gui_hr ();
  html2gui_ ("Backup unit specific options");
  html2gui_br ();
  html2gui_input_submit ("Super Wild Card", "ucon64gui_swc", "Options for Super Wild Card");

  ucon64gui_bottom ();

//</html>
}
