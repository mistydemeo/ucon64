/*
n64.c - Nintendo 64 support for uCON64gui

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
#include "html2gui.h"
#include "ucon64gui.h"
#include "top.h"
#include "bottom.h"
#include "configmenu/configmenu.h"

void
n64_gui (void)
{
#include "xpm/n64.xpm"
#include "xpm/d64.xpm"

  ucon64gui_top ();

  h2g_img (n64_xpm, 0, 0, 0, NULL);
  h2g_img (d64_xpm, 0, 0, 0, NULL);

  ucon64gui_divider ();

  h2g_input_submit ("Rename ROM", OPTION_S "n",
                    "(" OPTION_S "n) change ROM name; $FILE=NEWNAME");

  h2g_ (" ");

  h2g_input_submit ("Boot Code", OPTION_LONG_S "bot",
                    "(" OPTION_LONG_S "bot) add/extract boot code to/from ROM; $FILE=BOOTCODE\nextracts automatically when $FILE does not exist (4032 Bytes)");

  h2g_ (" ");

  h2g_input_submit ("LAC's Makesram", OPTION_LONG_S "sram",
                    "(" OPTION_LONG_S "sram) LAC's Makesram; $ROM=(LAC's SRAM ROM image) $FILE=SRAMFILE\nthe SRAMFILE must have a size of 512 Bytes");

  h2g_ (" ");

  h2g_input_submit ("ultraSMS", OPTION_LONG_S "usms",
                    "(" OPTION_LONG_S "usms) Jos Kwanten's ultraSMS (Sega Master System/GameGear Emulator)\n$ROM=(Jos Kwanten's ultraSMS ROM image) $FILE=SMSROM(<=4Mb)");

  h2g_ (" ");

  h2g_input_submit ("Fix Chksum", OPTION_LONG_S "chk", "(" OPTION_LONG_S "chk) fix ROM checksum");

  ucon64gui_spacer ();

  h2g_ ("Convert ROM to: ");

  h2g_input_submit ("V64", OPTION_LONG_S "v64",
                    "(" OPTION_LONG_S "v64) convert to Doctor V64 (and compatibles/swapped)");

  h2g_input_submit ("Z64", OPTION_LONG_S "z64",
                    "(" OPTION_LONG_S "z64) convert to Z64 (Zip Drive/not swapped)");

  ucon64gui_bottom ();
}
