/*
nes.c - Nintendo Entertainment System/NES support for uCON64gui

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
#include "libhtk/libhtk.h"
#include "ucon64gui.h"
#include "top.h"
#include "bottom.h"
#include "configmenu.h"

void
nes_gui (void)
{
#include "../xpm/nes.xpm"

  ucon64gui_top ();

  htk_img (nes_xpm, 0, 0, 0, NULL);

  ucon64gui_divider ();

  htk_input_submit ("iNES header", OPTION_LONG_S "ineshd",
                    "(" OPTION_LONG_S "ineshd) extract iNES header from ROM (16 Bytes)");

  ucon64gui_spacer ();

  htk_ ("Convert ROM to: ");

  htk_input_submit ("FFE", OPTION_LONG_S "ffe", "(" OPTION_LONG_S "ffe) convert to FFE (+512 Bytes)");

  htk_input_submit ("iNES", OPTION_LONG_S "ines", "(" OPTION_LONG_S "ines) convert to iNES(Emu)");

  htk_input_submit ("UNIF", OPTION_LONG_S "unif", "(" OPTION_LONG_S "unif) convert to UNIF format/UNF");

  ucon64gui_bottom ();
}
