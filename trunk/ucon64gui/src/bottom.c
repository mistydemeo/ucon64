#include <stdlib.h>
#include <stdio.h>

#include "misc.h"
#include "ucon64gui.h"
#include "html2gui/src/html2gui.h"
#include "bottom.h"

void
ucon64gui_bottom (void)
{
#include "xpm/trans.xpm"
#include "xpm/icon_16x16.xpm"

  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();
  h2g_img (icon_16x16_xpm, 0, 0, 0, "uCON64gui");

//  h2g_img (ucon64gui_xpm, 0, 0, 0, NULL);

  h2g_ ("uCON64gui "
#ifdef __GTK__
             "(GTK) "
#endif
             "0.1.0alpha 2002 by NoisyB ");
  h2g_input_submit ("ucon64.sf.net", "http://ucon64.sf.net", "Surf to uCON64 and uCON64gui homepage");
//  h2g_input_image ("Emulate", "-e", emulate_xpm, 0, 0, buf);

}
