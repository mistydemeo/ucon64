/*
dc.c - Dreamcast support for uCON64gui

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
#include "misc.h"
#include "ucon64gui.h"
#include "html2gui.h"
#include "top.h"
#include "bottom.h"
#include "configmenu/configmenu.h"
#include "dc.h"

void
dc_gui (void)
{
  ucon64gui_top ();

//  h2g_img (swc_xpm, 0, 0, 0, NULL);

  ucon64gui_divider ();

  h2g_input_submit ("fix Checksum", OPTION_LONG_S "chk", "(" OPTION_LONG_S "chk) fix ROM checksum");

  ucon64gui_bottom ();
}
