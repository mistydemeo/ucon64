/*

  checkout the comments in the main routine

*/

#include "ucon64gui.h"

#define FILE_MAX 4096

#ifdef __GTK_GUI__
  GtkWidget *file;
#endif

struct ucon64gui_
{

  char cmd[FILE_MAX];
  char rom[FILE_MAX];
  char file[FILE_MAX];
  
  char *ucon64_output;
} ucon64;

void ucon64_system(void)
{
//TODO pipe?!
  system(ucon64.cmd);
}

void ucon64_rom_ (void)
{
  strcpy (ucon64.rom,gtk_file_selection_get_filename (GTK_FILE_SELECTION (file)));
}
void ucon64_rom (void)
{


  file=html2gui_file(ucon64_rom_,ucon64.rom );
//  strcpy (ucon64.rom,gtk_file_selection_get_filename (GTK_FILE_SELECTION (file)));
}

void ucon64_file_ (void)
{
//better use an input box than a filedialog

  strcpy (ucon64.rom,gtk_file_selection_get_filename (GTK_FILE_SELECTION (file)));
}

void ucon64_nfo(void)
{
  sprintf (ucon64.cmd, "ucon64 \"%s\"",ucon64.rom);
  ucon64_system();
}

void ucon64_ls(void)
{
  strcpy (ucon64.cmd, "ucon64 -ls .");
  ucon64_system();
}

int main (int argc, char *argv[])
{
#ifdef __GTK_GUI__
  GtkWidget *frame1=0,*frame2=0;
#endif

#include "xpm/icon.xpm"
#include "xpm/selectrom.xpm"

  html2gui_init (&argc, &argv);

/*
  The HTML code below explaines the following code!
  
  <BLA 1="A" 2="B" 3="C"> == bla("A","B","C")
  
  Expect syntactical changes to the functions


  <HTML>
  <FRAME1>
  <ICON>icon_xpm</ICON><!-- ICON is a custom tag! -->
  <TITLE>ucon64gtk!!!</TITLE>
  <BUTTON SRC="selectrom_xpm" target="ucon64_rom()">
  <BUTTON SRC="icon_xpm" target="ucon64_nfo()">
  <BR>
  <BUTTON SRC="icon_xpm" target="ucon64_ls()">
  </FRAME1>
  </HTML>
*/
  frame1=html2gui_body(frame1,icon_xpm,"ucon64gtk!!!");
  html2gui_button(frame1,ucon64_rom,selectrom_xpm,"Click here to open a ROM");
  html2gui_button(frame1,ucon64_nfo,icon_xpm,"Click here to see information about ROM");
  html2gui_br(frame1);
  html2gui_button(frame1,ucon64_ls,icon_xpm,"Click here to see romlist of current dir");

/*
  The HTML code below explaines the following code!
  
  <BLA 1="A" 2="B" 3="C"> == bla("A","B","C")
  
  Expect syntactical changes to the functions
  
  


  <HTML>
  <FRAME2><!-- in fact it's a new window -->
  <ICON>icon_xpm</ICON><!-- ICON is a custom tag! -->
  <TITLE>ucon123</TITLE>
  <BUTTON SRC="icon_xpm" target="ucon64_nfo()">
  </FRAME2>
  </HTML>
*/
  frame2=html2gui_body(frame2,icon_xpm,"ucon123");
  html2gui_button(frame2,ucon64_nfo,icon_xpm,"Click here to see ROM info");

  html2gui_main ();

  return (0);
}
