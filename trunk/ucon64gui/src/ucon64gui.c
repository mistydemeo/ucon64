/*
ucon64gui.c - a GUI for ucon64

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
#include "ucon64gui.h"

#include "snes/snes.h"

#include "backup/swc.h"

#include "xpm/trans_1x3.xpm"
#include "xpm/icon_16x16.xpm"
#include "xpm/icon.xpm"
#include "xpm/open.xpm"
//#include "xpm/ucon64gui.xpm"

void
ucon64gui_bottom (void)
{

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br ();
  html2gui_img (icon_16x16_xpm, 0, 0, 0);

//  html2gui_img (ucon64gui_xpm,0,0,0);

  html2gui_ ("uCON64gui "
#ifdef __GTK__
             "(GTK) "
#endif
             "0.1.0 2002 by NoisyB ");
  html2gui_a ("http://ucon64.sf.net", "_blank");

}


void
ucon64gui_system (void)
{
//  FILE *fh;
  char buf[MAXBUFSIZE];

  switch (ucon64gui.console)
    {
    case ucon64_SNES:
      strcat (ucon64gui.cmd, " -snes");
      break;

    default:
      break;
    }

  sprintf(buf,"xterm -e %s|less &",ucon64gui.cmd);

  html2gui_html_end();
  system (buf);
  html2gui_html(640,400,0);


/*
  if (!(fh = popen (buf, "r")))
    {
      strcpy (ucon64gui.ucon64_output, "");
      return;
    }

  while (fgets (buf, sizeof buf, fh) != NULL)
    {
      strcat (ucon64gui.ucon64_output, buf);
    }
  pclose (fh);
*/
}

void
ucon64gui_rom (void)
{
  html2gui_input_file ("Select ROM", ucon64gui.rom);

}

void
ucon64gui_file (void)
{
//_text nutzen?
  html2gui_input_file ("Select ROM", ucon64gui.file);
}

void
ucon64gui_info (void)
{
  strcpy (ucon64gui.rom, html2gui_filename);
  sprintf (ucon64gui.cmd, "ucon64 \"%s\"", ucon64gui.rom);
  ucon64gui_system ();
}

void
ucon64gui_ls (void)
{
  strcpy (ucon64gui.cmd, "ucon64 -ls .");
  ucon64gui_system ();
}

void
ucon64gui_e (void)
{
  strcpy (ucon64gui.rom, html2gui_filename);
  sprintf (ucon64gui.cmd, "ucon64 -e \"%s\"", ucon64gui.rom);
  ucon64gui_system ();
}



void
ucon64gui_top(void)
{
  char buf[NAME_MAX];
  
  sprintf(buf,"Configure uCON64 (%s)",ucon64gui.configfile);

//<html>

  html2gui_input_submit (ucon64gui_info, "Config",
                         buf, 0, 0,
                         NULL);
  html2gui_br();
  html2gui_img(trans_1x3_xpm, 0, 0, 0);

  html2gui_br();

  html2gui_("$ROM: ");
  strcpy (ucon64gui.rom, html2gui_filename);
  
  html2gui_input_text (ucon64gui_ls,ucon64gui.rom,"test",20,1,0);
  html2gui_input_submit (ucon64gui_rom, "Open", "Open", 0, 0,
                         open_xpm);

  html2gui_br();
  html2gui_("$FILE:  ");
  strcpy (ucon64gui.rom, html2gui_filename);
  
  html2gui_input_text (ucon64gui_ls,ucon64gui.file,"test",20,1,0);

  html2gui_input_submit (ucon64gui_file, "Open", "Open", 0, 0,
                         open_xpm);
  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br ();

  html2gui_ ("Miscellaneous options");
  html2gui_br ();
  html2gui_input_submit (ucon64gui_info, "Show info",
                         "Click here to see information about ROM", 0, 0,
                         NULL);

  html2gui_(" ");


  sprintf(buf,"(-e) emulate/run ROM (see %s for more)",ucon64gui.configfile);
  html2gui_input_submit (ucon64gui_e, "Emulate", buf, 0,
                         0, NULL);

  html2gui_ (" ");

  html2gui_input_submit (ucon64gui_e, "CRC32", "(-crc) show CRC32 value of ROM", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_e, "CRC32 (w/ hd)", "(-crchd) show CRC32 value of ROM (regarding to +512 Bytes header)", 0,
                         0, NULL);

  html2gui_ (" ");

  html2gui_input_submit (ucon64gui_ls, "Strip", "(-stp) strip first 512 Bytes (possible header) from ROM", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_ls, "Insert", "(-ins) insert 512 Bytes (0x00) before ROM", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_ls, "Truncate", "(-strip) strip Bytes from end of ROM; $FILE=VALUE", 0,
                         0, NULL);

  html2gui_br();
  html2gui_img(trans_1x3_xpm, 0, 0, 0);

  html2gui_br();

  html2gui_input_submit (ucon64gui_ls, "Hexdump", "(-hex) show ROM as hexdump", 0,
                         0, NULL);

  html2gui_ (" ");
  html2gui_input_submit (ucon64gui_ls, "Find String", "(-find) find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)", 0,
                         0, NULL);

  html2gui_ (" ");

  html2gui_input_submit (ucon64gui_ls, "Swap/(De)Interleave ROM", "(-swap) swap/(de)interleave ALL Bytes in ROM (1234<->2143)", 0,
                         0, NULL);
  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br ();

  html2gui_("List ROMs: ");

  html2gui_input_submit (ucon64gui_ls, "verbose", "(-ls) generate ROM list for all ROMs; $ROM=DIRECTORY", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_ls, "more verbose", "(-lsv) like [ROM list] but more verbose; $ROM=DIRECTORY", 0,
                         0, NULL);

  html2gui_(" Compare ROMs: ");

  html2gui_input_submit (ucon64gui_ls, "differencies", "(-c) compare ROMs for differencies; $FILE=OTHER_ROM", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_ls, "similarities", "(-cs) compare ROMs for similarities; $FILE=OTHER_ROM", 0,
                         0, NULL);

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br ();

  html2gui_("Database: ");
  
  html2gui_input_submit (ucon64gui_ls, "Search", "(-dbs) search ROM database (all entries) by CRC32; $ROM=0xCRC32", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_ls, "Stats", "(-db) ROM database statistics (# of entries)", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_ls, "View", "view ROM database (all entries)", 0,
                         0, NULL);

  html2gui_(" Padding: ");


  html2gui_input_submit (ucon64gui_ls, "Pad ROM", "(-pad) pad ROM to full Mb", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_ls, "Pad ROM (w/ hd)", "(-padhd) pad ROM to full Mb (regarding to +512 Bytes header)", 0,
                         0, NULL);

  html2gui_input_submit (ucon64gui_ls, "Check", "(-ispad) check if ROM is padded", 0,
                         0, NULL);

  html2gui_br();
  html2gui_img(trans_1x3_xpm, 0, 0, 0);

  html2gui_br();

  html2gui_hr();
  html2gui_br();
  html2gui_img(trans_1x3_xpm, 0, 0, 0);

  html2gui_br();
  html2gui_("Patches");
  
  html2gui_br();
  html2gui_("Baseline/BSL: ");


  html2gui_input_submit (ucon64gui_ls, "Apply", "(-b) apply Baseline/BSL patch (<=x.x); $FILE=PATCHFILE", 0,
                         0, NULL);
                         
//  html2gui_br();
  
  html2gui_(" IPS: ");

  html2gui_input_submit (ucon64gui_ls, "Apply", "(-i) apply IPS patch (<=1.2); $FILE=PATCHFILE", 0,
                         0, NULL);
  
  html2gui_input_submit (ucon64gui_ls, "Create", "(-mki) create IPS patch; $FILE=CHANGED_ROM", 0,
                         0, NULL);
  
//  html2gui_br();

  html2gui_(" APS: ");

  html2gui_input_submit (ucon64gui_ls, "Apply", "(-a) apply APS patch (<=1.2); $FILE=PATCHFILE", 0,
                         0, NULL);
  
  html2gui_input_submit (ucon64gui_ls, "Create", "(-mka) create APS patch; $FILE=CHANGED_ROM", 0,
                         0, NULL);
  
  html2gui_input_submit (ucon64gui_ls, "Rename", "(-na) change APS description; $ROM=PATCHFILE $FILE=DESCRIPTION", 0,
                         0, NULL);
  
  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br ();
  
  html2gui_("PPF: ");

  html2gui_input_submit (ucon64gui_ls, "Apply", "(-ppf) apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE", 0,
                         0, NULL);
  
  html2gui_input_submit (ucon64gui_ls, "Create", "(-mkppf) create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE", 0,
                         0, NULL);
  
  html2gui_input_submit (ucon64gui_ls, "Rename", "(-nppf) change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION", 0,
                         0, NULL);
  
  html2gui_input_submit (ucon64gui_ls, "FILE_ID.DIZ", "(-idppf) change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ", 0,
                         0, NULL);
  
  
  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
  html2gui_br();

  html2gui_hr();
  html2gui_br();

}


void
ucon64gui_root (void)
{

  ucon64gui.console = ucon64_UNKNOWN;
  html2gui_html (0, 0, 0);

  html2gui_title ("uCON64gui", icon_xpm);

  
  ucon64gui_top();

  html2gui_ ("Console specific options");
  html2gui_br ();
  html2gui_input_submit (ucon64gui_snes, "Super Nintendo",
                         "(-snes) Options for Super Nintendo", 0, 0, NULL);

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0);
//  html2gui_hr ();
  html2gui_ ("Backup unit specific options");
  html2gui_br ();
  html2gui_input_submit (ucon64gui_swc, "Super Wild Card",
                         "Options for Super Wild Card", 0, 0, NULL);

  ucon64gui_bottom ();

//</html>
}

int
main (int argc, char *argv[])
{
#ifdef	__DOS__
  strcpy (ucon64gui.configfile, "ucon64.cfg");
#else
  sprintf (ucon64gui.configfile, "%s%c.ucon64rc", getenv ("HOME"), FILE_SEPARATOR);
#endif


  html2gui_start (argc, argv);

  ucon64gui_root ();

  html2gui_end ();

  return (0);
}

















