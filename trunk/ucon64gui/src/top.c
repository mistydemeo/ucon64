#include "ucon64gui.h"
#include "html2gui/src/html2gui.h"
#include "top.h"

void
ucon64gui_top(void)
{
#include "xpm/trans_1x3.xpm"
#include "xpm/open.xpm"

  char buf[FILENAME_MAX];
  
  sprintf(buf,"Configure uCON64 (%s)",ucon64gui.configfile);

//<html>

  html2gui_input_submit ("Config", "",
                         buf);
  html2gui_br();
  html2gui_img(trans_1x3_xpm, 0, 0, 0, NULL);

  html2gui_br();

  html2gui_("$ROM: ");

  html2gui_input_text ("ROM=", ucon64gui.rom, "Enter $ROM", 20, 0, FALSE);
  html2gui_input_image ("Open", "ucon64gui_rom", open_xpm, 0, 0, "Open");

  html2gui_br();
  html2gui_("$FILE:  ");

  html2gui_input_text ("FILE=", ucon64gui.file, "Enter $FILE", 20, 0, FALSE);
  html2gui_input_image ("Open", "ucon64gui_file", open_xpm, 0, 0, "Open");

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0, NULL);
  html2gui_br ();

  html2gui_ ("Miscellaneous options");
  html2gui_br ();
  html2gui_input_submit ("Show info", "*",
                         "Click here to see information about ROM");

  html2gui_(" ");


  sprintf(buf,"(-e) emulate/run ROM (see %s for more)",ucon64gui.configfile);
  html2gui_input_submit ("Emulate", "-e", buf);

  html2gui_ (" ");

  html2gui_input_submit ("CRC32", "-crc", "(-crc) show CRC32 value of ROM");

  html2gui_input_submit ("CRC32 (w/ hd)", "-crchd", "(-crchd) show CRC32 value of ROM (regarding to +512 Bytes header)");

  html2gui_ (" ");

  html2gui_input_submit ("Strip", "-stp", "(-stp) strip first 512 Bytes (possible header) from ROM");

  html2gui_input_submit ("Insert", "-ins", "(-ins) insert 512 Bytes (0x00) before ROM");

  html2gui_input_submit ("Truncate", "-strip", "(-strip) strip Bytes from end of ROM; $FILE=VALUE");

  html2gui_br();
  html2gui_img(trans_1x3_xpm, 0, 0, 0, NULL);

  html2gui_br();

  html2gui_input_submit ("Hexdump", "-hex", "(-hex) show ROM as hexdump");

  html2gui_ (" ");
  html2gui_input_submit ("Find String", "-find", "(-find) find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)");

  html2gui_ (" ");

  html2gui_input_submit ("Swap/(De)Interleave ROM", "-swap", "(-swap) swap/(de)interleave ALL Bytes in ROM (1234<->2143)");
  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0, NULL);
  html2gui_br ();

  html2gui_("List ROMs: ");

  html2gui_input_submit ("verbose", "-ls", "(-ls) generate ROM list for all ROMs; $ROM=DIRECTORY");

  html2gui_input_submit ("more verbose", "-lsv", "(-lsv) like [ROM list] but more verbose; $ROM=DIRECTORY");

  html2gui_(" Compare ROMs: ");

  html2gui_input_submit ("differencies", "-c", "(-c) compare ROMs for differencies; $FILE=OTHER_ROM");

  html2gui_input_submit ("similarities", "-cs", "(-cs) compare ROMs for similarities; $FILE=OTHER_ROM");

  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0, NULL);
  html2gui_br ();

  html2gui_("Database: ");
  
  html2gui_input_submit ("Search", "-dbs", "(-dbs) search ROM database (all entries) by CRC32; $ROM=0xCRC32");

  html2gui_input_submit ("Stats", "-db", "(-db) ROM database statistics (# of entries)");

  html2gui_input_submit ("View", "-dbv", "(-dbv) view ROM database (all entries)");

  html2gui_(" Padding: ");


  html2gui_input_submit ("Pad ROM", "-pad", "(-pad) pad ROM to full Mb");

  html2gui_input_submit ("Pad ROM (w/ hd)", "-padhd", "(-padhd) pad ROM to full Mb (regarding to +512 Bytes header)");

  html2gui_input_submit ("Check", "-ispad", "(-ispad) check if ROM is padded");

  html2gui_br();
  html2gui_img(trans_1x3_xpm, 0, 0, 0, NULL);

  html2gui_br();

  html2gui_hr();
  html2gui_br();
  html2gui_img(trans_1x3_xpm, 0, 0, 0, NULL);

  html2gui_br();
  html2gui_("Patches");
  
  html2gui_br();
  html2gui_("Baseline/BSL: ");


  html2gui_input_submit ("Apply", "-b", "(-b) apply Baseline/BSL patch (<=x.x); $FILE=PATCHFILE");
                         
//  html2gui_br();
  
  html2gui_(" IPS: ");

  html2gui_input_submit ("Apply", "-i", "(-i) apply IPS patch (<=1.2); $FILE=PATCHFILE");
  
  html2gui_input_submit ("Create", "-mki", "(-mki) create IPS patch; $FILE=CHANGED_ROM");
  
//  html2gui_br();

  html2gui_(" APS: ");

  html2gui_input_submit ("Apply", "-a", "(-a) apply APS patch (<=1.2); $FILE=PATCHFILE");
  
  html2gui_input_submit ("Create", "-mka", "(-mka) create APS patch; $FILE=CHANGED_ROM");
  
  html2gui_input_submit ("Rename", "-na", "(-na) change APS description; $ROM=PATCHFILE $FILE=DESCRIPTION");
  
  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0, NULL);
  html2gui_br ();
  
  html2gui_("PPF: ");

  html2gui_input_submit ("Apply", "-ppf", "(-ppf) apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE");
  
  html2gui_input_submit ("Create", "-mkppf", "(-mkppf) create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE");
  
  html2gui_input_submit ("Rename", "-nppf", "(-nppf) change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION");
  
  html2gui_input_submit ("FILE_ID.DIZ", "-idppf", "(-idppf) change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ");
  
  
  html2gui_br ();
  html2gui_img (trans_1x3_xpm, 0, 0, 0, NULL);
  html2gui_br();

  html2gui_hr();
  html2gui_br();

}
