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
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include "misc.h"
#include "ucon64.h"
#include "html2gui/src/html2gui.h"
#include "ucon64gui.h"
#include "snes/snes.h"
#include "n64/n64.h"
#include "nes/nes.h"
#include "gb/gb.h"

#include "backup/swc.h"
#include "backup/cdrw.h"

#include "top.h"
#include "bottom.h"
#include "config/config.h"

#define DEBUG

void
h2g_system (char *query)
{
  FILE *fh;
  int len;
  char name[4096];
  char value[4096];
  char buf[MAXBUFSIZE];
  char buf2[MAXBUFSIZE];

#ifdef DEBUG
  printf ("%s\n", query);
  fflush (stdout);
#endif // DEBUG

  len = findlast (query, "=");
  strcpy (value, query);
  value[len] = 0;

  strcpy (name, &query[len + 1]);


  if (!strncmp (query, "rom=", 4))
    {
      strcpy (ucon64gui.rom, &query[4]);
      return;
    }
  if (!strncmp (query, "file=", 5))
    {
      strcpy (ucon64gui.file, &query[5]);
      return;
    }

  if (!strncmp (value, "http://", 7))
    {
      sprintf (buf, 
#ifdef  __MSDOS__
      "netscape %s"
#else
      "netscape %s &"
#endif
      , value);

      system (buf);

      return;
    }

  if (!strncmp (value, "ucon64gui_", 10))
    {
      if (!strdcmp (value, "ucon64gui_gb"))
        {
          strcpy(ucon64gui.console,"-gb"); 
          ucon64gui_gb ();
        }
      if (!strdcmp (value, "ucon64gui_snes"))
        {
          strcpy(ucon64gui.console,"-snes");
          ucon64gui_snes ();
        }
      if (!strdcmp (value, "ucon64gui_n64"))
        {
          strcpy(ucon64gui.console,"-n64");
          ucon64gui_n64 ();
        }
      if (!strdcmp (value, "ucon64gui_nes"))
        {
          strcpy(ucon64gui.console,"-nes");
          ucon64gui_nes ();
        }
        
      if (!strdcmp (value, "ucon64gui_swc"))
        ucon64gui_swc ();

      if (!strdcmp (value, "ucon64gui_cdrw"))
        ucon64gui_cdrw ();

      if (!strdcmp (value, "ucon64gui_root"))
        {
           if(ucon64gui.sub != 0)
             {
               ucon64gui.sub = 0;

               if (!strdcmp (ucon64gui.console,"-gb"))
                 ucon64gui_gb ();
               if (!strdcmp (ucon64gui.console,"-snes"))
                 ucon64gui_snes ();
               if (!strdcmp (ucon64gui.console,"-n64"))
                 ucon64gui_n64 ();
               if (!strdcmp (ucon64gui.console,"-nes"))
                 ucon64gui_nes ();

               return;
            }

          ucon64gui.console[0] = 0;
          ucon64gui_root ();
        }

      if (!strdcmp (value, "ucon64gui_config"))
        {
          ucon64gui.sub = 1;
          ucon64gui_config ();
        }

      return;
    }

  if (!strncmp (value, "emulate_", 8) ||
      !strncmp (value, "cdrw_", 5) || !strdcmp (value, "backups"))
    {
      setProperty (ucon64gui.configfile, value, &query[len + 1]);
      return;
    }

// switches/overrides
  if (!strdcmp (name, "-hd"))
    {
      ucon64gui.hd = 1;
      return;
    }

  if (!strdcmp (name, "-nhd"))
    {
      ucon64gui.hd = 0;
      return;
    }

  if (!strdcmp (name, "-ns"))
    {
      ucon64gui.ns = (ucon64gui.ns == 1) ? 0 : 1;
      return;
    }

  if (value[0] == '*')
    value[0] = 0;

/*
  options
*/
  sprintf (buf2, "ucon64 %s %s \"%s\" \"%s\"", 
  (ucon64gui.console != NULL) ? ucon64gui.console : "",
  value,
           (ucon64gui.rom != NULL) ? ucon64gui.rom : "",
           (ucon64gui.file != NULL) ? ucon64gui.file : "");

#ifdef DEBUG
  printf ("%s\n", buf2);
  fflush (stdout);
#endif // DEBUG

//  system (buf2);

  if (!(fh = popen (buf2, "r"))) return;

  ucon64gui.ucon64_output[0]=0;

  while (fgets (buf2, MAXBUFSIZE, fh) != NULL)
    strcat (ucon64gui.ucon64_output, buf2);

  pclose (fh);

  ucon64gui.sub = 1;
  ucon64gui_output(ucon64gui.ucon64_output);

  return;
}



int
main (int argc, char *argv[])
{
  char buf2[32768];

/*
   configfile handling
*/
  sprintf (ucon64gui.configfile, "%s%c"
#ifdef  __MSDOS__
           "ucon64.cfg"
#else
           /*
              Use getchd() here too. If this code gets compiled under Windows the compiler has to be
              Cygwin. So, uCON64 will be a Win32 executable which will run only in an environment
              where long filenames are available and where files can have more than three characters
              as "extension". Under Bash HOME will be set, but most Windows people will propably run
              uCON64 under cmd or command where HOME is not set by default. Under Windows XP/2000
              (/NT?) USERPROFILE and/or HOMEDRIVE+HOMEPATH will be set which getchd() will "detect".
            */
           ".ucon64rc"
#endif
           , getchd (buf2, FILENAME_MAX), FILE_SEPARATOR);


  h2g_start (argc, argv);

  ucon64gui_root ();

  h2g_end ();

  return 0;
}


void
ucon64gui_output (char *output)
{
#include "xpm/icon.xpm"
#include "xpm/back.xpm"

  h2g_html (0, 0, 0);
  h2g_head ();
  h2g_title ("uCON64gui - Output", icon_xpm);
  h2g_head_end ();
  h2g_body (NULL, "#c0c0c0");
  
  h2g_input_image ("Back", "ucon64gui_root", back_xpm, 0, 0, "Back");

//  h2g_br();

  h2g_textarea ("output", output, 80, 25, TRUE, FALSE, NULL);

  ucon64gui_bottom ();

  h2g_form_end ();
  h2g_body_end ();

  h2g_html_end ();
}











void
ucon64gui_root (void)
{
#include "xpm/trans.xpm"
#ifdef BACKUP
/*
  h2g_input_submit ("Flash Advance Linker", "ucon64gui_fal",
                    "options for Flash Advance Linker\n2001 Visoly http://www.visoly.com");
  h2g_input_submit ("Doctor V64", "ucon64gui_doctor64",
                    "options for Doctor V64\n19XX Bung Enterprises Ltd http://www.bung.com.hk");
  h2g_input_submit ("Doctor64 Jr", "ucon64gui_doctor64jr",
                    "options for Doctor64 Jr\n19XX Bung Enterprises Ltd http://www.bung.com.hk");
  h2g_input_submit ("Super Wild Card", "ucon64gui_swc",
                    "options for Super WildCard 1.6XC/Super WildCard 2.8CC/Super Wild Card DX(2)/SWC\n1993/1994/1995/19XX Front Far East/FFE http://www.front.com.tw");
  h2g_input_submit ("Super Magic Drive", "ucon64gui_smd",
                    "options for Super Com Pro (HK)/Super Magic Drive/SMD\n19XX Front Far East/FFE http://www.front.com.tw");
  h2g_input_submit ("GameBoy Xchanger", "ucon64gui_gbx",
                    "options for GameBoy Xchanger");

*/
  char *backup_unit_options[]={
"Flash Advance Linker","Doctor V64","Doctor64 Jr","Super Wild Card", 
"Super Magic Drive","GameBoy Xchanger",NULL
};
  char *backup_unit_values[]={
"ucon64gui_fal","ucon64gui_doctor64","ucon64gui_doctor64jr",
"ucon64gui_swc","ucon64gui_smd","ucon64gui_gbx",
NULL
};

/*
  h2g_input_submit ("PC-Engine", "ucon64gui_pce",
                    "(-pce) options for PC-Engine (CD Unit/Core Grafx(II)/Shuttle/GT/LT/Super CDROM/DUO(-R(X)))/Super Grafx/Turbo (Grafx(16)/CD/DUO/Express)\n1987/19XX/19XX NEC");
  h2g_input_submit ("Neo Geo", "ucon64gui_ng",
                    "(-ng) options for Neo Geo/Neo Geo CD(Z)/MVS\n1990/1994 SNK http://www.neogeo.co.jp");
  h2g_input_submit ("Neo Geo Pocket", "ucon64gui_ngp",
                    "(-ngp) options for Neo Geo Pocket/Neo Geo Pocket Color\n1998/1999 SNK http://www.neogeo.co.jp");
  h2g_input_submit ("WonderSwan", "ucon64gui_wswan",
                    "(-swan) options for WonderSwan/WonderSwan Color\n19XX/19XX Bandai");
  h2g_input_submit ("Real3DO", "ucon64gui_3do",
                    "(-3do) options for Real3DO\n1993");
  h2g_input_submit ("CD32", "ucon64gui_cd32",
                    "(-cd32) options for CD32\n1993 Commodore");
  h2g_input_submit ("CD-i", "ucon64gui_cdi",
                    "(-cdi) options for CD-i\n1991");
  h2g_input_submit ("Vectrex", "ucon64gui_vectrex",
                    "(-vec) options for Vectrex\n1982");
  h2g_input_submit ("ColecoVision", "ucon64gui_coleco",
                    "(-coleco) options for ColecoVision\n1982");
  h2g_input_submit ("Intellivision", "ucon64gui_intelli",
                    "(-intelli) options for Intellivision\n1979 Mattel");
*/
  char *console_options[]={
"PC-Engine","Neo Geo","Neo Geo Pocket","WonderSwan", "Real3DO","CD32","CD-i",
"Vectrex","ColecoVision","Intellivision",NULL
};
  char *console_values[]={
"ucon64gui_pce","ucon64gui_ng","ucon64gui_ngp","ucon64gui_wswan",
"ucon64gui_3do","ucon64gui_cd32","ucon64gui_cdi","ucon64gui_vectrex","ucon64gui_coleco",
"ucon64gui_intelli",NULL

};

#endif
#include "xpm/icon.xpm"

  h2g_html (0, 0, 0);
  h2g_head ();
  h2g_title ("uCON64gui", icon_xpm);
  h2g_head_end ();
  h2g_body (NULL, "#c0c0c0");
  h2g_form ("http://ucon64");

  ucon64gui_top ();

  h2g_ ("Console specific options");
  h2g_br ();
  h2g_input_submit ("NES", "ucon64gui_nes",
                    "(-nes) options for Nintendo Entertainment System/NES\n1983 Nintendo http://www.nintendo.com");
  h2g_input_submit ("GameBoy", "ucon64gui_gb",
                    "(-gb) options for GameBoy/(Super GB)/GB Pocket/Color GB/(GB Advance)\n1989/1994/1996/1998/2001 Nintendo http://www.nintendo.com");
  h2g_input_submit ("Super Nintendo", "ucon64gui_snes",
                    "(-snes) options for Super Nintendo/SNES/Super Famicon\n1990 Nintendo http://www.nintendo.com");
  h2g_input_submit ("Virtual Boy", "ucon64gui_vboy",
                    "(-vboy) options for Nintendo Virtual Boy\n19XX Nintendo http://www.nintendo.com");
  h2g_input_submit ("Nintendo 64", "ucon64gui_n64",
                    "(-n64) options for Nintendo 64\n1996 Nintendo http://www.nintendo.com");
  h2g_input_submit ("GameBoy Advance", "ucon64gui_gba",
                    "(-gba) options for GameBoy Advance\n2001 Nintendo http://www.nintendo.com");

  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();

  h2g_input_submit ("Sega Master System", "ucon64gui_sms",
                    "(-sms) options for Sega Master System(II/III)/GameGear (Handheld)\n1986/19XX SEGA http://www.sega.com");
  h2g_input_submit ("Game Gear", "ucon64gui_sms",
                    "(-sms) options for Sega Master System(II/III)/GameGear (Handheld)\n1986/19XX SEGA http://www.sega.com");
  h2g_input_submit ("Sega System", "ucon64gui_s16",
                    "(-s16) options for Sega System 16(A/B)/Sega System 18/dual 68000\n1987/19XX/19XX SEGA http://www.sega.com");
  h2g_input_submit ("Genesis", "ucon64gui_gen",
                    "(-gen) options for Genesis/Sega Mega Drive/Sega CD/32X/Nomad\n1989/19XX/19XX SEGA http://www.sega.com");
  h2g_input_submit ("Saturn", "ucon64gui_sat",
                    "(-sat) options for Saturn\n1994 SEGA http://www.sega.com");
  h2g_input_submit ("Dreamcast", "ucon64gui_dc",
                    "(-dc) options for Dreamcast\n1998 SEGA http://www.sega.com");
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();

  h2g_input_submit ("Playstation", "ucon64gui_psx",
                    "(-psx) options for Playstation (One)/Playstation 2 (CD only)\n1994/(2000) Sony http://www.playstation.com");
  h2g_input_submit ("Playstation 2", "ucon64gui_ps2",
                    "(-ps2) options for Playstation 2\n2000 Sony http://www.playstation.com");

  h2g_ (" ");

  h2g_input_submit ("Atari VCS 2600", "ucon64gui_ata",
                    "(-ata) options for Atari VCS 2600(aka Stella)/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr\n1977/1982/1984/1986 Atari");
  h2g_input_submit ("Lynx", "ucon64gui_lynx",
                    "(-lynx) options for Handy(prototype)/Lynx/Lynx II\n1987 Epyx/1989 Atari/1991 Atari");
  h2g_input_submit ("Jaguar", "ucon64gui_jag",
                    "(-jag) options for Panther(32bit prototype)/Jaguar64/Jaguar64 CD\n1989 Flare2/1993 Atari/1995 Atari");

  h2g_br();
  h2g_("Others: ");

  h2g_select ("page", console_options, console_values, 0, 0,
"Choose the desired console here"//\n"
 );


/*
  h2g_ (" ");

  h2g_input_submit ("PC-Engine", "ucon64gui_pce",
                    "(-pce) options for PC-Engine (CD Unit/Core Grafx(II)/Shuttle/GT/LT/Super CDROM/DUO(-R(X)))/Super Grafx/Turbo (Grafx(16)/CD/DUO/Express)\n1987/19XX/19XX NEC");

  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();

  h2g_input_submit ("Neo Geo", "ucon64gui_ng",
                    "(-ng) options for Neo Geo/Neo Geo CD(Z)/MVS\n1990/1994 SNK http://www.neogeo.co.jp");
  h2g_input_submit ("Neo Geo Pocket", "ucon64gui_ngp",
                    "(-ngp) options for Neo Geo Pocket/Neo Geo Pocket Color\n1998/1999 SNK http://www.neogeo.co.jp");

  h2g_ (" ");

  h2g_input_submit ("WonderSwan", "ucon64gui_wswan",
                    "(-swan) options for WonderSwan/WonderSwan Color\n19XX/19XX Bandai");

  h2g_ (" ");
  h2g_input_submit ("Real3DO", "ucon64gui_3do",
                    "(-3do) options for Real3DO\n1993");
  h2g_ (" ");
  h2g_input_submit ("CD32", "ucon64gui_cd32",
                    "(-cd32) options for CD32\n1993 Commodore");
  h2g_ (" ");
  h2g_input_submit ("CD-i", "ucon64gui_cdi",
                    "(-cdi) options for CD-i\n1991");

  h2g_ (" ");
  h2g_input_submit ("Vectrex", "ucon64gui_vectrex",
                    "(-vec) options for Vectrex\n1982");
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();
  h2g_input_submit ("ColecoVision", "ucon64gui_coleco",
                    "(-coleco) options for ColecoVision\n1982");
  h2g_ (" ");
  h2g_input_submit ("Intellivision", "ucon64gui_intelli",
                    "(-intelli) options for Intellivision\n1979 Mattel");

  h2g_br ();
*/

#if defined BACKUP || defined BACKUP_CD
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();
  h2g_hr ();
  h2g_br ();
//  h2g_img (trans_xpm, 0, 3, 0, NULL);
//  h2g_br();

  h2g_ ("Backup unit specific options");
  h2g_br ();
#endif // BACKUP || BACKUP_CD

#ifdef BACKUP_CD
  h2g_ ("CD's: ");

  h2g_input_submit ("CD-Writer", "ucon64gui_cdrw",
//  h2g_input_image ("CD-Writer", "ucon64gui_cdrw", ccd_xpm, 0, 0,
                    "options for CD-Writer\nhttp://cdrdao.sourceforge.net/ (recommended burn engine)");

#endif // BACKUP_CD

#ifdef BACKUP

  h2g_ (" Cartridges: ");

  h2g_select ("page", backup_unit_options, backup_unit_values, 0, 0,
"Choose the desired backup unit here"//\n"
 );

/*
  h2g_input_submit ("Flash Advance Linker", "ucon64gui_fal",
                    "options for Flash Advance Linker\n2001 Visoly http://www.visoly.com");
  h2g_ (" ");
  h2g_input_submit ("Doctor V64", "ucon64gui_doctor64",
                    "options for Doctor V64\n19XX Bung Enterprises Ltd http://www.bung.com.hk");
  h2g_ (" ");
  h2g_input_submit ("Doctor64 Jr", "ucon64gui_doctor64jr",
                    "options for Doctor64 Jr\n19XX Bung Enterprises Ltd http://www.bung.com.hk");
  h2g_ (" ");
  h2g_input_submit ("Super Wild Card", "ucon64gui_swc",
                    "options for Super WildCard 1.6XC/Super WildCard 2.8CC/Super Wild Card DX(2)/SWC\n1993/1994/1995/19XX Front Far East/FFE http://www.front.com.tw");
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();
  h2g_input_submit ("Super Magic Drive", "ucon64gui_smd",
                    "options for Super Com Pro (HK)/Super Magic Drive/SMD\n19XX Front Far East/FFE http://www.front.com.tw");
  h2g_ (" ");
  h2g_input_submit ("GameBoy Xchanger", "ucon64gui_gbx",
                    "options for GameBoy Xchanger");
*/
#endif // BACKUP

  ucon64gui_bottom ();

  h2g_form_end ();
  h2g_body_end ();

  h2g_html_end ();
}
