#include "ucon64gui.h"
#include "html2gui/src/html2gui.h"
#include "bottom.h"

void
ucon64gui_bottom (void)
{
#include "xpm/trans_1x3.xpm"
#include "xpm/icon_16x16.xpm"

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0, NULL);
  html2gui_br ();
  html2gui_img (icon_16x16_xpm, 0, 0, 0, "uCON64gui");

//  html2gui_img (ucon64gui_xpm, 0, 0, 0, NULL);

  html2gui_ ("uCON64gui "
#ifdef __GTK__
             "(GTK) "
#endif
             "0.1.0 2002 by NoisyB ");
  html2gui_input_image ("http://ucon64.sf.net", "http://ucon64.sf.net", NULL, 0, 0, "http://ucon64.sf.net");

}
