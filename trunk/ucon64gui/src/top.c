/*
top.c 

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
#include "ucon64gui.h"
#include "html2gui/src/html2gui.h"
#include "top.h"
#include "misc.h"

void
ucon64gui_top (void)
{
#include "xpm/trans.xpm"
#include "xpm/open.xpm"
#include "xpm/emulate.xpm"
#include "xpm/icon.xpm"
#include "xpm/back.xpm"
#include "xpm/db.xpm"
#ifdef BACKUP_CD
#include "xpm/cdrw.xpm"
#include "xpm/cdrw2.xpm"
#endif // BACKUP_CD

  char buf[FILENAME_MAX];

  sprintf (buf, "Configure uCON64 (%s)", ucon64gui.configfile);

  h2g_html (0, 0, 0);

  h2g_head ();
  h2g_title (ucon64gui_title, icon_xpm);
  h2g_head_end ();

  h2g_body (NULL, "#c0c0c0");

  h2g_form (UCON64GUI_FORMTARGET);

  h2g_input_submit ("Config", "--config", buf);
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);

  h2g_br ();

  h2g_ ("$ROM: ");

  h2g_input_file ("--rom", ucon64gui.rom, 50, 0, open_xpm, 0, 0,
                  "Select $ROM");

  h2g_br ();
  h2g_ ("$FILE:  ");

  h2g_input_file ("--file", ucon64gui.file, 50, 0, open_xpm, 0, 0,
                  "Select $FILE or enter a value by hand");


  ucon64gui_spacer ();

  h2g_ ("Miscellaneous options");
  h2g_br ();
  h2g_input_submit ("Show info", "",
                    "Click here to see information about ROM");

  h2g_ (" ");


  sprintf (buf, "(-e) emulate/run ROM (see %s for more)",
           ucon64gui.configfile);
  h2g_input_image ("Emulate", "-e", emulate_xpm, 0, 0, buf);

  h2g_ (" ");

  if (ucon64gui.sub && ucon64gui.console)
    {
      char buf[MAXBUFSIZE], buf2[MAXBUFSIZE];
      const char *p = NULL;

      p = ucon64gui.console;

      while (*p == '-') p++;
      
      sprintf (buf, "--emulate_%s", p);
      h2g_input_text (buf,
                  getProperty (ucon64gui.configfile, &buf[2], buf2, ""),
                  45, 0, FALSE,
                  "uCON64 can operate as frontend for many Emulators\n"
                  "Enter here the commandline used for the emulator the name of the ROM will be attached to it");

    }

  ucon64gui_spacer ();

  h2g_input_submit ("CRC32", "--crc", "(--crc) show CRC32 value of ROM");

  h2g_ (" ");

  h2g_input_submit ("Strip", "--stp",
                    "(--stp) strip first 512 Bytes (possible header) from ROM");

  h2g_input_submit ("Insert", "--ins",
                    "(--ins) insert 512 Bytes (0x0) before ROM");

  h2g_input_submit ("Truncate", "--strip",
                    "(--strip) strip Bytes from end of ROM; $FILE=VALUE");

  h2g_input_submit ("Hexdump", "--hex", "(--hex) show ROM as hexdump");

  h2g_ (" ");
  h2g_input_submit ("Find String", "--find",
                    "(--find) find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)");

  h2g_ (" ");

  h2g_input_submit ("De-/Interleave", "--swap",
                    "(--swap) swap/(de)interleave ALL Bytes in ROM (1234<->2143)");

  ucon64gui_spacer ();

  h2g_ ("List/Rename: ");

  h2g_input_submit ("Normal", "--ls",
                    "(--ls) generate ROM list for all ROMs; $ROM=DIRECTORY");

  h2g_input_submit ("Verbose", "--lsv",
                    "(--lsv) generate more verbose ROM list; $ROM=DIRECTORY");

  h2g_input_submit ("FILE_ID.DIZ", "--lsfid",
                    "(--lsfid) generate ROM list with FILE_ID.DIZ; $ROM=DIRECTORY");

  h2g_input_submit ("Rename", "--rrom",
                    "(--rrom) rename all ROMs in DIRECTORY to their internal names; --rom=DIR");

  h2g_input_submit ("Rename 8.3", "--rr83",
                    "(--rr83) rename all ROMs in DIRECTORY to their internal names in 8.3 format; --rom=DIR");

  ucon64gui_spacer ();

  h2g_ ("Compare: ");

  h2g_input_submit ("differencies", "--c",
                    "(--c) compare ROMs for differencies; $FILE=OTHER_ROM");

  h2g_input_submit ("similarities", "--cs",
                    "(--cs) compare ROMs for similarities; $FILE=OTHER_ROM");

  ucon64gui_spacer ();

#ifdef DB

  h2g_img (db_xpm, 0, 0, 0, NULL);

  h2g_ ("Database: ");

  h2g_input_submit ("Search", "--dbs",
                    "(--dbs) search ROM database (all entries) by CRC32; $ROM=0xCRC32");

  h2g_input_submit ("Stats", "--db",
                    "(--db) ROM database statistics (# of entries)");

  h2g_input_submit ("View", "--dbv",
                    "(--dbv) view ROM database (all entries)");

  h2g_ (" ");

#endif // DB

  h2g_ ("Padding: ");


  h2g_input_submit ("Pad ROM", "--pad", "(--pad) pad ROM to full Mb");

  h2g_input_submit ("Check", "--ispad", "(--ispad) check if ROM is padded");


#ifdef BACKUP_CD
  h2g_br ();

  ucon64gui_spacer();

  h2g_img (cdrw_xpm, 0, 0, 0, NULL);

  h2g_ (" Read/Write CD: ");
  
  h2g_input_image ("CD -> Image -> CD", "--xcdrw", cdrw2_xpm, 0, 0,
                   "(--xcdrw) read/write IMAGE from/to CD-Writer; $ROM=CD_IMAGE\nreads automatically when $ROM does not exist");

  ucon64gui_spacer ();
 
  h2g_ ("Track Mode: ");

  h2g_select ("file", 0, 0, "Choose the desired Track Mode here",
    "MODE2_RAW (2352 Bytes; default)",
    "--file=MODE2_RAW",
    "MODE1 (2048 Bytes; standard ISO9660)",
    "--file=MODE1",
    "MODE1_RAW (2352 Bytes)",
    "--file=MODE1_RAW",
    "MODE2 (2336 Bytes)",
    "--file=MODE2"
    ,0);

#endif // BACKUP_CD


  ucon64gui_divider ();

  h2g_ ("Patches");

  h2g_br ();
  h2g_ ("Baseline/BSL: ");


  h2g_input_submit ("Apply", "-b",
                    "(-b) apply Baseline/BSL patch (<=x.x); $FILE=PATCHFILE");

//  h2g_br();

  h2g_ (" IPS: ");

  h2g_input_submit ("Apply", "-i",
                    "(-i) apply IPS patch (<=1.2); $FILE=PATCHFILE");

  h2g_input_submit ("Create", "--mki",
                    "(--mki) create IPS patch; $FILE=CHANGED_ROM");

//  h2g_br();

  h2g_ (" APS: ");

  h2g_input_submit ("Apply", "-a",
                    "(-a) apply APS patch (<=1.2); $FILE=PATCHFILE");

  h2g_input_submit ("Create", "--mka",
                    "(--mka) create APS patch; $FILE=CHANGED_ROM");

  h2g_input_submit ("Rename", "--na",
                    "(--na) change APS description; $ROM=PATCHFILE $FILE=DESCRIPTION");

  ucon64gui_spacer ();

  h2g_ ("PPF: ");

  h2g_input_submit ("Apply", "--ppf",
                    "(--ppf) apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE");

  h2g_input_submit ("Create", "--mkppf",
                    "(--mkppf) create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE");

  h2g_input_submit ("Rename", "--nppf",
                    "(--nppf) change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION");

  h2g_input_submit ("FILE_ID.DIZ", "--idppf",
                    "(--idppf) change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ");


  ucon64gui_spacer ();

  h2g_ ("GameGenie: ");

  h2g_input_submit ("Encode", "--gge",
                    "(--gge) encode GameGenie code; $ROM=AAAAAA:VV");

  h2g_input_submit ("Decode", "--ggd",
                    "(--ggd) decode GameGenie code; $ROM=XXXX-XXXX");

  h2g_input_submit ("Apply", "--gg",
                    "(--gg) apply GameGenie code (permanent); $FILE=XXXX-XXXX");

  ucon64gui_divider ();

  if (ucon64gui.sub)
    h2g_input_image ("Back",  OPTION_LONG_S "root", back_xpm, 0, 0, "Back");

  h2g_br ();
}



