/*
genesis.c - Genesis support for uCON64gui

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
#include "configmenu.h"
#include "genesis.h"

void
genesis_gui (void)
{
#include "xpm/snes_96.xpm"
#include "xpm/snes2_96.xpm"
#include "xpm/snes3_96.xpm"
#include "xpm/swc.xpm"

  ucon64gui_top ();

  h2g_img (snes_96_xpm, 0, 0, 0, NULL);
  h2g_img (snes2_96_xpm, 0, 0, 0, NULL);
  h2g_img (snes3_96_xpm, 0, 0, 0, NULL);
  h2g_img (swc_xpm, 0, 0, 0, NULL);

  ucon64gui_divider ();

  h2g_input_submit ("Join", OPTION_S "j", "(" OPTION_S "j) join splitted ROM");

  h2g_input_submit ("Split", OPTION_S "s",
                    "(" OPTION_S "s) split ROM into 4Mb parts (for backup unit(s) with fdd)");

  h2g_ (" ");

  h2g_input_submit ("Crack", OPTION_S "k", "(" OPTION_S "k) remove protection (crack)");

  h2g_ (" ");

  h2g_input_submit ("NTSC/PAL fix", OPTION_S "f", "(" OPTION_S "f) remove NTSC/PAL protection");

  h2g_ (" ");

  h2g_input_submit ("fix Checksum", OPTION_LONG_S "chk", "(" OPTION_LONG_S "chk) fix ROM checksum");

  h2g_ (" ");

  h2g_input_submit ("Rename ROM", OPTION_S "n",
                    "(" OPTION_S "n) change ROM name; $FILE=NEWNAME");

  h2g_ (" ");

  h2g_input_submit ("ColorCalc", OPTION_LONG_S "col",
                    "(" OPTION_LONG_S "col) convert 0xRRGGBB (html) <" OPTION_S "> 0xXXXX (snes); $ROM=0xCOLOR\nthis routine was used to find green colors in games and to replace them with red colors (blood mode)");

  ucon64gui_spacer ();

  h2g_ ("Convert ROM to: ");

  h2g_input_submit ("SMC", OPTION_LONG_S "smc", "(" OPTION_LONG_S "smc) convert to Super Magicom/SMC");

  h2g_input_submit ("FIG", OPTION_LONG_S "fig", "(" OPTION_LONG_S "fig) convert to *Pro Fighter(all)FIG");

  h2g_input_submit ("SWC", OPTION_LONG_S "swc",
                    "(" OPTION_LONG_S "swc) convert to Super Wild Card(all)SWC");

  h2g_input_submit ("MGD", OPTION_LONG_S "mgd",
                    "(" OPTION_LONG_S "mgd) convert to Multi GameMGD2/MGH/RAW");

  h2g_input_submit ("GD3", OPTION_LONG_S "gd3",
                    "(" OPTION_LONG_S "gd3) convert to Professor SF(2) Game Doctor SF3/6/7");

  h2g_ (" ");

  h2g_input_submit ("Non-interleaved", OPTION_LONG_S "dint",
                    "(" OPTION_LONG_S "dint) convert ROM to non-interleaved format");

  h2g_ (" ");

  h2g_input_submit ("SlowROM", OPTION_S "l", "(" OPTION_S "l) convert to SlowROM");

  ucon64gui_spacer ();

  h2g_ ("Convert SRAM to: ");

  h2g_input_submit ("FIG", OPTION_LONG_S "figs",
                    "(" OPTION_LONG_S "figs) convert Snes9x/ZSNES *.srm (SRAM) to *Pro Fighter(all)FIG; $ROM=SRAM");

  h2g_input_submit ("SWC", OPTION_LONG_S "swcs",
                    "(" OPTION_LONG_S "swcs) convert Snes9x/ZSNES *.srm (SRAM) to Super Wild Card(all)SWC; $ROM=SRAM");

  h2g_input_submit ("UFO", OPTION_LONG_S "ufos",
                    "(" OPTION_LONG_S "ufos) convert Snes9x/ZSNES *.srm (SRAM) to Super UFO; $ROM=SRAM");

  h2g_input_submit ("Snes9x/ZSNES", OPTION_LONG_S "stp",
                    "(" OPTION_LONG_S "stp) convert SRAM from backup unit for use with Snes9x/ZSNES; $ROM=SRAM\nNOTE: " OPTION_LONG_S "stp just strips the first 512 bytes");

  ucon64gui_bottom ();
}
