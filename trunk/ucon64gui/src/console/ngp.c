/*
ngp.c - NeoGeo Pocket support for uCON64gui

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
#include "libhtmltk.h"
#include "top.h"
#include "bottom.h"
#include "configmenu.h"
#include "ngp.h"

void
ngp_gui (void)
{
  ucon64gui_top ();

//  htk_img (swc_xpm, 0, 0, 0, NULL);

  ucon64gui_divider ();

  htk_input_submit ("fix Checksum", OPTION_LONG_S "chk", "(" OPTION_LONG_S "chk) fix ROM checksum");

  ucon64gui_bottom ();
}
