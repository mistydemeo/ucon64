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
#include "n64/n64.h"
#include "nes/nes.h"

#include "backup/swc.h"
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
//  char buf[MAXBUFSIZE];
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

  if (!strncmp (value, "ucon64gui_", 10))
    {
      if (!strdcmp (value, "ucon64gui_snes"))
        ucon64gui_snes ();
      if (!strdcmp (value, "ucon64gui_n64"))
        ucon64gui_n64 ();
      if (!strdcmp (value, "ucon64gui_root"))
        ucon64gui_root ();
      if (!strdcmp (value, "ucon64gui_nes"))
        ucon64gui_nes ();
      if (!strdcmp (value, "ucon64gui_swc"))
        ucon64gui_swc ();
      if (!strdcmp (value, "ucon64gui_config"))
        ucon64gui_config ();

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
  sprintf (buf2, "ucon64 %s \"%s\" \"%s\"", value,
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
#ifdef  __DOS__
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

  return (0);
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

  h2g_textarea ("output", output, 80, 20, TRUE, FALSE, NULL);

  ucon64gui_bottom ();

  h2g_form_end ();
  h2g_body_end ();

  h2g_html_end ();
}











void
ucon64gui_root (void)
{
#include "xpm/trans.xpm"
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
  h2g_input_submit ("Super Nintendo", "ucon64gui_snes",
                    "(-snes) Options for Super Nintendo");
  h2g_input_submit ("NES", "ucon64gui_nes",
                    "(-nes) Options for Nintendo Entertainment System");
  h2g_input_submit ("Nintendo 64", "ucon64gui_n64",
                    "(-n64) Options for Nintendo 64");

  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
//  h2g_hr ();
  h2g_ ("Backup unit specific options");
  h2g_br ();
  h2g_input_submit ("Super Wild Card", "ucon64gui_swc",
                    "Options for Super Wild Card");

  ucon64gui_bottom ();

  h2g_form_end ();
  h2g_body_end ();

  h2g_html_end ();
}
