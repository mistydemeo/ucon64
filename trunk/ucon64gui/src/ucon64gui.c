#include "ucon64gui.h"

struct html2gui_ index_html;
struct html2gui_ snes_html;

struct ucon64gui_
{

  char cmd[NAME_MAX];
  char rom[NAME_MAX];
  char file[NAME_MAX];

  char *ucon64_output;
}
ucon64;

void
ucon64_system (void)
{
//TODO pipe?!
  system (ucon64.cmd);
}

void
ucon64_rom (void)
{
  html2gui_file (&index_html, "Select ROM", ucon64.rom);
}

void
ucon64_nfo (void)
{
  strcpy(ucon64.rom, html2gui_filename);
  sprintf (ucon64.cmd, "ucon64 \"%s\"", ucon64.rom);
  ucon64_system ();
}

void
ucon64_ls (void)
{
  strcpy (ucon64.cmd, "ucon64 -ls .");
  ucon64_system ();
}


int
main (int argc, char *argv[])
{
#include "xpm/icon.xpm"
#include "xpm/selectrom.xpm"

  gtk_init (&argc, &argv);

//<html>
  html2gui_html (&index_html, 640, 400, 0);

  html2gui_title (&index_html, "ucon64gui", icon_xpm);

  html2gui_button (&index_html, ucon64_rom, "Select ROM", "Click here to open a ROM", 10, 10, selectrom_xpm);

  html2gui_button (&index_html, ucon64_nfo, "NFO", "Click here to see information about ROM", 10, 10, icon_xpm);

  html2gui_br (&index_html);

  html2gui_button (&index_html, ucon64_ls, "ROM list", "Click here to see romlist of current dir", 10, 10, icon_xpm);

  html2gui_html_end (&index_html);
//</html>

//<html>
  html2gui_html (&snes_html, 640, 400, 0);

  html2gui_title (&snes_html, "ucon64gui", icon_xpm);

  html2gui_button (&snes_html, ucon64_nfo, "NFO", "Click here to see ROM info", 10, 10, icon_xpm);

  html2gui_html_end (&snes_html);
//</html>

  gtk_main ();

  return (0);
}
