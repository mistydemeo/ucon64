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

#define DEBUG

void h2g_system(char *query)
{
char buf[MAXBUFSIZE];

#ifdef DEBUG
  printf ("%s\n", query);
  fflush (stdout);
#endif // DEBUG

if(!strdcmp(query,"ucon64gui_snes"))
{
  ucon64gui_snes();
  return;
}
if(!strdcmp(query,"ucon64gui_root"))
{
  ucon64gui_root();
  return;
}
if(!strdcmp(query,"ucon64gui_swc"))
{
  ucon64gui_swc();
  return;
}

// switches/overrides
if(!strdcmp(query,"-hd"))
{
  ucon64gui.hd = 1;
  return;
}

if(!strdcmp(query,"-nhd"))
{
  ucon64gui.hd = 0;
  return;
}

if(!strdcmp(query,"-ns"))
{
  ucon64gui.ns = (ucon64gui.ns == 1) ? 0 : 1;
  return;
}

/*
  options
*/

  sprintf(buf,"xterm -e \"ucon64 %s %s %s\" &", query
  , (ucon64gui.rom != NULL) ? ucon64gui.rom : ""
  , (ucon64gui.file != NULL) ? ucon64gui.file : ""
  );

//  system (buf);

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

  return;
}



int
main (int argc, char *argv[])
{
#ifdef	__DOS__
  strcpy (ucon64gui.configfile, "ucon64gui.cfg");
#else
  sprintf (ucon64gui.configfile, "%s%c.ucon64guirc", getenv ("HOME"), FILE_SEPARATOR);
#endif


  h2g_start (argc, argv);

  ucon64gui_root ();

  h2g_end ();

  return (0);
}


















void
ucon64gui_root (void)
{
#include "xpm/trans_1x3.xpm"
#include "xpm/icon.xpm"

  h2g_html (0, 0, 0);
  h2g_head();
  h2g_title ("uCON64gui", icon_xpm);
  h2g_head_end();
  h2g_body(NULL,"#c0c0c0");
  h2g_form("http://ucon64");

  ucon64gui_top();

  h2g_ ("Console specific options");
  h2g_br ();
  h2g_input_submit ("Super Nintendo", "ucon64gui_snes", "(-snes) Options for Super Nintendo");

  h2g_br ();
  h2g_img (trans_1x3_xpm, 0, 0, 0, NULL);
//  h2g_hr ();
  h2g_ ("Backup unit specific options");
  h2g_br ();
  h2g_input_submit ("Super Wild Card", "ucon64gui_swc", "Options for Super Wild Card");

  ucon64gui_bottom ();
  
  h2g_form_end();
  h2g_body_end();

  h2g_html_end();
}
