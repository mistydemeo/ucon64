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
#include "top.h"
#include "bottom.h"
#include "config/config.h"

#define DEBUG

void h2g_system(char *query)
{
int len;
char buf[MAXBUFSIZE];

#ifdef DEBUG
  printf ("%s\n", query);
  fflush (stdout);
#endif // DEBUG

len = findlast(query,"=");
strcpy(buf,query);
buf[len]=0;

//printf("%s\n", buf);
//fflush (stdout);


if(!strdcmp(buf,"ucon64gui_snes"))
{
  ucon64gui_snes();
  return;
}
if(!strdcmp(buf,"ucon64gui_root"))
{
  ucon64gui_root();
  return;
}

if(!strdcmp(buf,"ucon64gui_swc"))
{
  ucon64gui_swc();
  return;
}

if(!strdcmp(buf,"ucon64gui_config"))
{
  ucon64gui_config();
  return;
}

// switches/overrides
if(!strdcmp(buf,"-hd"))
{
  ucon64gui.hd = 1;
  return;
}

if(!strdcmp(buf,"-nhd"))
{
  ucon64gui.hd = 0;
  return;
}

if(!strdcmp(buf,"-ns"))
{
  ucon64gui.ns = (ucon64gui.ns == 1) ? 0 : 1;
  return;
}

/*
  options
*/

  sprintf(buf,"xterm -e \"ucon64 %s %s %s\" &", buf
  , (ucon64gui.rom != NULL) ? ucon64gui.rom : ""
  , (ucon64gui.file != NULL) ? ucon64gui.file : ""
  );

//  system (buf);

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
ucon64gui_root (void)
{
#include "xpm/trans_1x3.xpm"
#include "xpm/icon.xpm"

  h2g_html (0, 0, 0);
  h2g_head();
  h2g_title ("uCON64gui", icon_xpm);
  h2g_head_end();
  h2g_body(NULL,"#c0c0c0");
  h2g_form("http://ucon64");

  ucon64gui_top();

  h2g_ ("Console specific options");
  h2g_br ();
  h2g_input_submit ("Super Nintendo", "ucon64gui_snes", "(-snes) Options for Super Nintendo");

  h2g_br ();
  h2g_img (trans_1x3_xpm, 0, 0, 0, NULL);
//  h2g_hr ();
  h2g_ ("Backup unit specific options");
  h2g_br ();
  h2g_input_submit ("Super Wild Card", "ucon64gui_swc", "Options for Super Wild Card");

  ucon64gui_bottom ();
  
  h2g_form_end();
  h2g_body_end();

  h2g_html_end();
}
