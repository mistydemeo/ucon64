#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "ucon64gui.h"
#include "html2gui/src/html2gui.h"
#include "top.h"

void
ucon64gui_top(void)
{
#include "xpm/trans.xpm"
#include "xpm/open.xpm"
#include "xpm/emulate.xpm"

  char buf[FILENAME_MAX];
  
  sprintf(buf,"Configure uCON64 (%s)",ucon64gui.configfile);

//<html>

  h2g_input_submit ("Config", "ucon64gui_config",
                         buf);
  h2g_br();
  h2g_img(trans_xpm, 0, 3, 0, NULL);

  h2g_br();

  h2g_("$ROM: ");

  h2g_input_file ("rom",ucon64gui.rom, 50, 0, open_xpm, 0, 0, "Select $ROM");
  
  h2g_br();
  h2g_("$FILE:  ");

  h2g_input_file ("file", ucon64gui.file, 50, 0, open_xpm, 0, 0,
    "Select $FILE or enter a value by hand");
  
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();

  h2g_ ("Miscellaneous options");
  h2g_br ();
  h2g_input_submit ("Show info", "*",
                         "Click here to see information about ROM");

  h2g_(" ");


  sprintf(buf,"(-e) emulate/run ROM (see %s for more)",ucon64gui.configfile);
  h2g_input_image ("Emulate", "-e", emulate_xpm, 0, 0, buf);

  h2g_ (" ");

  h2g_input_submit ("CRC32", "-crc", "(-crc) show CRC32 value of ROM");

  h2g_input_submit ("CRC32 (w/ hd)", "-crchd", "(-crchd) show CRC32 value of ROM (regarding to +512 Bytes header)");

  h2g_ (" ");

  h2g_input_submit ("Strip", "-stp", "(-stp) strip first 512 Bytes (possible header) from ROM");

  h2g_input_submit ("Insert", "-ins", "(-ins) insert 512 Bytes (0x00) before ROM");

  h2g_input_submit ("Truncate", "-strip", "(-strip) strip Bytes from end of ROM; $FILE=VALUE");

  h2g_br();
  h2g_img(trans_xpm, 0, 3, 0, NULL);

  h2g_br();

  h2g_input_submit ("Hexdump", "-hex", "(-hex) show ROM as hexdump");

  h2g_ (" ");
  h2g_input_submit ("Find String", "-find", "(-find) find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)");

  h2g_ (" ");

  h2g_input_submit ("Swap/(De)Interleave ROM", "-swap", "(-swap) swap/(de)interleave ALL Bytes in ROM (1234<->2143)");
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();

  h2g_("List ROMs: ");

  h2g_input_submit ("verbose", "-ls", "(-ls) generate ROM list for all ROMs; $ROM=DIRECTORY");

  h2g_input_submit ("VERBOSE", "-lsv", "(-lsv) like [ROM list] but more verbose; $ROM=DIRECTORY");

  h2g_(" Compare ROMs: ");

  h2g_input_submit ("differencies", "-c", "(-c) compare ROMs for differencies; $FILE=OTHER_ROM");

  h2g_input_submit ("similarities", "-cs", "(-cs) compare ROMs for similarities; $FILE=OTHER_ROM");

  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();

#ifdef DB

  h2g_("Database: ");
  
  h2g_input_submit ("Search", "-dbs", "(-dbs) search ROM database (all entries) by CRC32; $ROM=0xCRC32");

  h2g_input_submit ("Stats", "-db", "(-db) ROM database statistics (# of entries)");

  h2g_input_submit ("View", "-dbv", "(-dbv) view ROM database (all entries)");

  h2g_ (" ");

#endif // DB

  h2g_("Padding: ");


  h2g_input_submit ("Pad ROM", "-pad", "(-pad) pad ROM to full Mb");

  h2g_input_submit ("Pad ROM (w/ hd)", "-padhd", "(-padhd) pad ROM to full Mb (regarding to +512 Bytes header)");

  h2g_input_submit ("Check", "-ispad", "(-ispad) check if ROM is padded");

  h2g_br();
  h2g_img(trans_xpm, 0, 3, 0, NULL);

  h2g_br();

  h2g_hr();
  h2g_br();
  h2g_img(trans_xpm, 0, 3, 0, NULL);

  h2g_br();
  h2g_("Patches");
  
  h2g_br();
  h2g_("Baseline/BSL: ");


  h2g_input_submit ("Apply", "-b", "(-b) apply Baseline/BSL patch (<=x.x); $FILE=PATCHFILE");
                         
//  h2g_br();
  
  h2g_(" IPS: ");

  h2g_input_submit ("Apply", "-i", "(-i) apply IPS patch (<=1.2); $FILE=PATCHFILE");
  
  h2g_input_submit ("Create", "-mki", "(-mki) create IPS patch; $FILE=CHANGED_ROM");
  
//  h2g_br();

  h2g_(" APS: ");

  h2g_input_submit ("Apply", "-a", "(-a) apply APS patch (<=1.2); $FILE=PATCHFILE");
  
  h2g_input_submit ("Create", "-mka", "(-mka) create APS patch; $FILE=CHANGED_ROM");
  
  h2g_input_submit ("Rename", "-na", "(-na) change APS description; $ROM=PATCHFILE $FILE=DESCRIPTION");
  
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();
  
  h2g_("PPF: ");

  h2g_input_submit ("Apply", "-ppf", "(-ppf) apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE");
  
  h2g_input_submit ("Create", "-mkppf", "(-mkppf) create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE");
  
  h2g_input_submit ("Rename", "-nppf", "(-nppf) change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION");
  
  h2g_input_submit ("FILE_ID.DIZ", "-idppf", "(-idppf) change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ");
  
  
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br();

  h2g_hr();
  h2g_br();

}
