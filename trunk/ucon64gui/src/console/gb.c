/*
gb.c - Game Boy support for uCON64gui

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
#include "libhtmltk.h"
#include "ucon64gui.h"
#include "top.h"
#include "bottom.h"
#include "configmenu.h"

void
gb_gui (void)
{
#include "xpm/gb1.xpm"

  ucon64gui_top ();

  htk_img (gb1_xpm, 0, 0, 0, NULL);

  ucon64gui_divider ();

  htk_input_submit ("Rename ROM", OPTION_S "n",
                    "(" OPTION_S "n) change ROM name; $FILE=NEWNAME");

  ucon64gui_spacer ();

  htk_ ("Convert ROM to: ");

  htk_input_submit ("MGD",  OPTION_LONG_S "mgd", "(" OPTION_LONG_S "mgd) convert to Multi Game*/MGD2/RAW");

  htk_input_submit ("SSC",  OPTION_LONG_S "ssc",
                    "(" OPTION_LONG_S "ssc) convert to Super Smart Card/SSC (+512 Bytes)");

  htk_input_submit ("SGB",  OPTION_LONG_S "sgb",
                    "(" OPTION_LONG_S "sgb) convert from GB Xchanger/GB/GBC to Super Backup Card/GX/GBX");

  htk_input_submit ("GBX",  OPTION_LONG_S "gbx",
                    "(" OPTION_LONG_S "gbx) convert from Super Backup Card/GX/GBX to GB Xchanger/GB/GBC");

  htk_ (" ");

  htk_input_submit ("NES Emulator",  OPTION_LONG_S "n2gb",
                    "(" OPTION_LONG_S "n2gb) convert for use with Kami's FC Emulator (NES Emulator); $ROM=NES_ROM $FILE=FC.GB (the Emulator)\nm-kami@da2.so-net.ne.jp www.playoffline.co");

  ucon64gui_bottom ();
}
