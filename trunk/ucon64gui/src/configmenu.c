/*
configmenu.c - configmenu for uCON64gui

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
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "libhtmlgtk/libhtmlgtk.h"
#include "configmenu.h"
#include "ucon64gui.h"
#include "top.h"
#include "bottom.h"
#include "misc.h"

void
ucon64gui_config (void)
{
  char buf[MAXBUFSIZE];

#include "xpm/back.xpm"
#include "xpm/icon.xpm"

  h2g_html (0, 0, 0);

  h2g_head ();
  h2g_title (ucon64gui_title, icon_xpm);
  h2g_head_end ();

  h2g_body (NULL, "#c0c0c0");

  h2g_form (UCON64GUI_FORMTARGET);

  h2g_input_image ("Back", OPTION_LONG_S "root", back_xpm, 0, 0, "Back");

  h2g_br ();

  sprintf (buf, "Configuration of (%s)", ucon64gui.configfile);
  h2g_ (buf);

  h2g_br ();
  h2g_ ("backups = ");

  h2g_input_text ("backups",
                  get_property (ucon64gui.configfile, "backups", buf, "1"), 2,
                  0, FALSE,
                  "create backups of files? (1=yes; 0=no)\nbefore processing a ROM uCON64 will make a backup of it");

  h2g_ (" (1=yes; 0=no); check the tooltips for more information");

#ifdef BACKUP_CD
  ucon64gui_spacer ();

  h2g_br ();
  h2g_ ("cdrw_read = ");
  h2g_input_text ("cdrw_read",
                  get_property (ucon64gui.configfile, "cdrw_read", buf, ""),
                  70, 0, FALSE,
                  "uCON64 can operate as frontend for CD burning software to make backups"
                  "for CD-based consoles \n"
                  "We suggest cdrdao (http://cdrdao.sourceforge.net) as burn engine for uCON64"
                  "Make sure you check this configfile for the right settings\n"
                  "--device [bus,id,lun] (cdrdao)\n"
                  "bin and toc filenames are added by ucon64 at the end");

  h2g_br ();
  h2g_ ("cdrw_write = ");
  h2g_input_text ("cdrw_write",
                  get_property (ucon64gui.configfile, "cdrw_write", buf, ""),
                  70, 0, FALSE,
                  "uCON64 can operate as frontend for CD burning software to make backups"
                  "for CD-based consoles \n"
                  "We suggest cdrdao (http://cdrdao.sourceforge.net) as burn engine for uCON64"
                  "Make sure you check this configfile for the right settings\n"
                  "--device [bus,id,lun] (cdrdao)\n"
                  "toc filename is added by ucon64 at the end");

#endif // BACKUP_CD

  ucon64gui_bottom ();
}
