/*
bottom.c

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
#include "html2gui/src/html2gui.h"
#include "ucon64.h"
#include "bottom.h"

void
ucon64gui_bottom (void)
{
#include "xpm/icon_16x16.xpm"

  ucon64gui_spacer ();
  
  h2g_img (icon_16x16_xpm, 0, 0, 0, "uCON64gui");

//  h2g_img (ucon64gui_xpm, 0, 0, 0, NULL);

//  h2g_ ( ucon64gui_TITLE );

  h2g_a ("ucon64.sf.net", "http://ucon64.sf.net",
         "Surf to uCON64 and uCON64gui homepage");
//  h2g_input_image ("Emulate", "-e", emulate_xpm, 0, 0, buf);

}
