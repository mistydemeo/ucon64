/*
ucon64gui.c - a GUI for ucon64 (using html2gui framework)

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
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <stdlib.h>
#include "config.h"
#include "ucon64gui.h"
#include "html2gui/html2gui.h"
#include "misc.h"
#include "url.h"

#include "top.h"
#include "bottom.h"
#include "configmenu.h"

#include "console/snes.h"
#include "console/n64.h"
#include "console/nes.h"
#include "console/gb.h"
#include "console/swan.h"
#include "console/gba.h"
#include "console/sms.h"
#include "console/genesis.h"
#include "console/dc.h"
#include "console/lynx.h"
#include "console/jaguar.h"
#include "console/pce.h"
#include "console/neogeo.h"
#include "console/ngp.h"

const char *ucon64gui_title = "uCON64gui " UCON64GUI_VERSION " (for uCON64 " UCON64_VERSION_S ") 2002 by NoisyB ";


static void ucon64gui_root (void);
//static void ucon64gui_system (void);
static void ucon64gui_output (char *output);
ucon64gui_t ucon64gui;

const struct option long_options[] = {
  {"1991", 0, 0, UCON64_1991},
  {"3do", 0, 0, UCON64_3DO},
  {"?", 0, 0, UCON64_HELP},
  {"a", 0, 0, UCON64_A},
  {"ata", 0, 0, UCON64_ATA},
  {"b", 0, 0, UCON64_B},
  {"b0", 0, 0, UCON64_B0},
  {"b1", 0, 0, UCON64_B1},
  {"bios", 0, 0, UCON64_BIOS},
  {"bot", 0, 0, UCON64_BOT},
  {"c", 0, 0, UCON64_C},
  {"cd", 0, 0, UCON64_CD},
//    {"cd32", 0, 0, UCON64_CD32},
//    {"cdi", 0, 0, UCON64_CDI},
  {"chk", 0, 0, UCON64_CHK},
  {"col", 0, 0, UCON64_COL},
  {"coleco", 0, 0, UCON64_COLECO},
  {"crc", 0, 0, UCON64_CRC},
  {"crchd", 0, 0, UCON64_CRCHD},
  {"crp", 0, 0, UCON64_CRP},
  {"cs", 0, 0, UCON64_CS},
  {"db", 0, 0, UCON64_DB},
  {"dbs", 0, 0, UCON64_DBS},
  {"dbv", 0, 0, UCON64_DBV},
  {"dc", 0, 0, UCON64_DC},
  {"dint", 0, 0, UCON64_DINT},
  {"e", 0, 0, UCON64_E},
  {"f", 0, 0, UCON64_F},
//    {"fds", 0, 0, UCON64_FDS},
//    {"fdsl", 0, 0, UCON64_FDSL},
  {"ffe", 0, 0, UCON64_FFE},
  {"fig", 0, 0, UCON64_FIG},
  {"figs", 0, 0, UCON64_FIGS},
  {"file", 1, 0, UCON64_FILE},
  {"find", 0, 0, UCON64_FIND},
  {"frontend", 0, 0, UCON64_FRONTEND},
  {"gb", 0, 0, UCON64_GB},
  {"gba", 0, 0, UCON64_GBA},
  {"gbx", 0, 0, UCON64_GBX},
  {"gc", 0, 0, UCON64_GC},
  {"gd3", 0, 0, UCON64_GD3},
  {"gdf", 0, 0, UCON64_GDF},
  {"gen", 0, 0, UCON64_GEN},
  {"gg", 0, 0, UCON64_GG},
  {"ggd", 0, 0, UCON64_GGD},
  {"gge", 0, 0, UCON64_GGE},
  {"gp32", 0, 0, UCON64_GP32},
  {"h", 0, 0, UCON64_HELP},
  {"help", 0, 0, UCON64_HELP},
  {"hex", 0, 0, UCON64_HEX},
  {"i", 0, 0, UCON64_I},
  {"idppf", 0, 0, UCON64_IDPPF},
  {"ines", 0, 0, UCON64_INES},
  {"ineshd", 0, 0, UCON64_INESHD},
  {"ins", 0, 0, UCON64_INS},
  {"insn", 1, 0, UCON64_INSN},
  {"intelli", 0, 0, UCON64_INTELLI},
  {"ip", 0, 0, UCON64_IP},
  {"iso", 0, 0, UCON64_ISO},
  {"ispad", 0, 0, UCON64_ISPAD},
  {"j", 0, 0, UCON64_J},
  {"jag", 0, 0, UCON64_JAG},
  {"k", 0, 0, UCON64_K},
  {"l", 0, 0, UCON64_L},
  {"lnx", 0, 0, UCON64_LNX},
  {"logo", 0, 0, UCON64_LOGO},
  {"ls", 0, 0, UCON64_LS},
  {"lsv", 0, 0, UCON64_LSV},
  {"lynx", 0, 0, UCON64_LYNX},
  {"lyx", 0, 0, UCON64_LYX},
  {"mgd", 0, 0, UCON64_MGD},
//    {"mgh", 0, 0, UCON64_MGH},
  {"mka", 0, 0, UCON64_MKA},
  {"mkcue", 0, 0, UCON64_MKCUE},
  {"mki", 0, 0, UCON64_MKI},
  {"mkppf", 0, 0, UCON64_MKPPF},
  {"mktoc", 0, 0, UCON64_MKTOC},
  {"multi", 0, 0, UCON64_MULTI},
  {"multi1", 0, 0, UCON64_MULTI1},
  {"multi2", 0, 0, UCON64_MULTI2},
  {"multi3", 0, 0, UCON64_MULTI3},
  {"mvs", 0, 0, UCON64_MVS},
  {"n", 0, 0, UCON64_N},
  {"n2", 0, 0, UCON64_N2},
  {"n2gb", 0, 0, UCON64_N2GB},
  {"n64", 0, 0, UCON64_N64},
  {"na", 0, 0, UCON64_NA},
  {"nbak", 0, 0, UCON64_NBAK},
  {"nes", 0, 0, UCON64_NES},
  {"ng", 0, 0, UCON64_NG},
  {"ngp", 0, 0, UCON64_NGP},
  {"nppf", 0, 0, UCON64_NPPF},
  {"nrot", 0, 0, UCON64_NROT},
  {"ns", 0, 0, UCON64_NS},
  {"p", 0, 0, UCON64_P},
  {"pad", 0, 0, UCON64_PAD},
  {"padhd", 0, 0, UCON64_PADHD},
  {"pasofami", 0, 0, UCON64_PASOFAMI},
  {"pce", 0, 0, UCON64_PCE},
  {"port", 1, 0, UCON64_PORT},
  {"ppf", 0, 0, UCON64_PPF},
  {"ps2", 0, 0, UCON64_PS2},
  {"psx", 0, 0, UCON64_PSX},
  {"rrom", 0, 0, UCON64_RROM},
  {"rr83", 0, 0, UCON64_RR83},
  {"rl", 0, 0, UCON64_RL},
  {"rom", 1, 0, UCON64_ROM},
  {"rotl", 0, 0, UCON64_ROTL},
  {"rotr", 0, 0, UCON64_ROTR},
  {"ru", 0, 0, UCON64_RU},
  {"s", 0, 0, UCON64_S},
  {"s16", 0, 0, UCON64_S16},
  {"sam", 0, 0, UCON64_SAM},
  {"sat", 0, 0, UCON64_SAT},
  {"sgb", 0, 0, UCON64_SGB},
  {"smc", 0, 0, UCON64_SMC},
  {"smd", 0, 0, UCON64_SMD},
  {"smds", 0, 0, UCON64_SMDS},
  {"smg", 0, 0, UCON64_SMG},
  {"sms", 0, 0, UCON64_SMS},
  {"snes", 0, 0, UCON64_SNES},
  {"sram", 0, 0, UCON64_SRAM},
  {"ssc", 0, 0, UCON64_SSC},
  {"stp", 0, 0, UCON64_STP},
  {"stpn", 1, 0, UCON64_STPN},
  {"strip", 0, 0, UCON64_STRIP},
  {"swan", 0, 0, UCON64_SWAN},
  {"swap", 0, 0, UCON64_SWAP},
  {"swc", 0, 0, UCON64_SWC},
  {"swcs", 0, 0, UCON64_SWCS},
#ifdef DEBUG
  {"test", 0, 0, UCON64_TEST},
#endif // DEBUG
  {"ufos", 0, 0, UCON64_UFOS},
  {"unif", 0, 0, UCON64_UNIF},
  {"usms", 0, 0, UCON64_USMS},
  {"v64", 0, 0, UCON64_V64},
  {"vboy", 0, 0, UCON64_VBOY},
  {"vec", 0, 0, UCON64_VEC},
  {"xbox", 0, 0, UCON64_XBOX},
#ifdef BACKUP_CD
  {"xcdrw", 0, 0, UCON64_XCDRW},
#endif // BACKUP_CD
#ifdef BACKUP
  {"xdjr", 0, 0, UCON64_XDJR},
  {"xfal", 0, 0, UCON64_XFAL},
  {"xfalb", 1, 0, UCON64_XFALB},
  {"xfalc", 1, 0, UCON64_XFALC},
  {"xfals", 0, 0, UCON64_XFALS},
  {"xmccl", 0, 0, UCON64_XMCCL},
  {"xgbx", 0, 0, UCON64_XGBX},
  {"xgbxb", 1, 0, UCON64_XGBXB},
  {"xgbxs", 0, 0, UCON64_XGBXS},
  {"xsmd", 0, 0, UCON64_XSMD},
  {"xsmds", 0, 0, UCON64_XSMDS},
  {"xswc", 0, 0, UCON64_XSWC},
  {"xswcs", 0, 0, UCON64_XSWCS},
  {"xv64", 0, 0, UCON64_XV64},
#endif // BACKUP
  {"z64", 0, 0, UCON64_Z64},
  {"hd", 0, 0, UCON64_HD},
  {"hdn", 1, 0, UCON64_HDN},
  {"nhd", 0, 0, UCON64_NHD},
  {"int", 0, 0, UCON64_INT},
  {"int2", 0, 0, UCON64_INT2},
  {"nint", 0, 0, UCON64_NINT},
  {"hi", 0, 0, UCON64_HI},
  {"nhi", 0, 0, UCON64_NHI},
  {"bs", 0, 0, UCON64_BS},
  {"nbs", 0, 0, UCON64_NBS},
  {"ctrl", 1, 0, UCON64_CTRL},
  {"ntsc", 0, 0, UCON64_NTSC},
  {"pal", 0, 0, UCON64_PAL},
  {"bat", 0, 0, UCON64_BAT},
  {"nbat", 0, 0, UCON64_NBAT},
  {"vram", 0, 0, UCON64_VRAM},
  {"nvram", 0, 0, UCON64_NVRAM},
  {"mirr", 1, 0, UCON64_MIRR},
  {"mapr", 1, 0, UCON64_MAPR},
  {"dumpinfo", 0, 0, UCON64_DUMPINFO},
  {"version", 0, 0, UCON64_VERSION},

  {"root", 0, 0, UCON64GUI_ROOT},
  {"config", 0, 0, UCON64GUI_CONFIG},
  {"surfto", 1, 0, UCON64GUI_SURFTO},

  {0, 0, 0, 0}
};


void
html2gui_request (const char *uri, const char *query)
{
  FILE *fh;
  int len;
  int c = 0, option_index = 0;
  char name[MAXBUFSIZE];
  const char *value;
  char buf[MAXBUFSIZE];
  const char *p = NULL;
  st_url_t url;

#ifdef DEBUG
  fprintf (stderr, "query: %s\n", query);
#endif // DEBUG

  p = strchr (query, '=');
  if (*(++p) != '-')
    p = query;

  sprintf (buf, "localhost%s%s%s", uri, p ? "?" : "", p ? p : "");
    
  url_parser (&url, buf);

#ifdef DEBUG
  printf ("cmdline: %s\n", url.cmd);
#endif

  sprintf (buf, "%s %s", url.cmd,
           NULL_TO_EMPTY (ucon64gui.console));


  if (ucon64gui.rom[0])
    {
      strcat (buf, " " OPTION_LONG_S "rom=\"");
      strcat (buf, ucon64gui.rom);
      strcat (buf, "\"");
    }

  if (ucon64gui.file[0])
    {
      strcat (buf, " " OPTION_LONG_S "file=\"");
      strcat (buf, ucon64gui.file);
      strcat (buf, "\"");
    }

//  while ((
  c = html2gui_getopt (uri, query, long_options, &option_index);
//  ) != -1)
    {
      switch (c)
        {
          case UCON64_ROM:
            strcpy (ucon64gui.rom, optindex);
            ucon64gui.sub = 0;
            ucon64gui_root ();
            return;
            
          case UCON64_FILE:
            strcpy (ucon64gui.file, optindex);
            ucon64gui.sub = 0;
            ucon64gui_root ();
            return;

          case UCON64_SNES:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "snes";
            snes_gui ();
            return;

          case UCON64_GB:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "gb";
            gb_gui ();
            return;

          case UCON64_N64:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "n64";
            n64_gui ();
            return;

          case UCON64_NES:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "nes";
            nes_gui ();
            return;

          case UCON64_SWAN:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "swan";
            swan_gui ();
            return;

          case UCON64_GBA:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "gba";
            gba_gui ();
            return;

          case UCON64_SMS:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "sms";
            sms_gui ();
            return;

          case UCON64_GEN:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "gen";
            genesis_gui ();
            return;

          case UCON64_DC:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "dc";
            dc_gui ();
            return;

          case UCON64_LYNX:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "lynx";
            lynx_gui ();
            return;

          case UCON64_JAGUAR:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "jaguar";
            jaguar_gui ();
            return;

          case UCON64_PCE:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "pce";
            pce_gui ();
            return;

          case UCON64_NEOGEO:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "neogeo";
            neogeo_gui ();
            return;

          case UCON64_NGP:
            ucon64gui.sub = 1;
            ucon64gui.console =  OPTION_LONG_S "ngp";
            ngp_gui ();
            return;

          case UCON64_XCDRW:
            return;

          case UCON64_XSWC:
            return;
#if 0
          case UCON64_XSMD:
            return;

          case UCON64_XFAL:
            return;

          case UCON64_XGBX:
            return;

          case UCON64_XD64:
            return;

          case UCON64_X64JR:
            return;
#endif
          case UCON64GUI_ROOT:
            ucon64gui.sub = 0;
            ucon64gui.console = NULL;
            ucon64gui_root ();
            return;
            
          case UCON64GUI_CONFIG:
            ucon64gui.sub = 1;
            ucon64gui.console = NULL;
            ucon64gui_config ();
            return;

          case UCON64GUI_SURFTO:
//            fsystem (stderr, "netscape uco64.sf.net &");
            return;

          default:
#if 0

  if (!(fh = popen (buf, "r")))
    break;

  ucon64gui.ucon64_output[0] = 0;

  while (fgets (buf, MAXBUFSIZE, fh) != NULL)
    strcat (ucon64gui.ucon64_output, buf);

  pclose (fh);
#endif
  printf ("system (%s);\n\n", buf);
  fflush (stdout);

  ucon64gui_output(ucon64gui.ucon64_output);
          break;
        }
    }
  return;
}



int
main (int argc, char *argv[])
{
/*
   configfile handling
*/
  sprintf (ucon64gui.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
           "ucon64.cfg"
#else
           ".ucon64rc"
#endif
           , getenv2 ("HOME"));


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
  h2g_title (ucon64gui_title, icon_xpm);
  h2g_head_end ();
  h2g_body (NULL, "#c0c0c0");

  h2g_input_image ("Back", OPTION_LONG_S "root", back_xpm, 0, 0, "Back");

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
#include "xpm/icon.xpm"

  h2g_html (0, 0, 0);
  h2g_head ();
  h2g_title (ucon64gui_title, icon_xpm);
  h2g_head_end ();
  h2g_body (NULL, "#c0c0c0");
  h2g_form (UCON64GUI_FORMTARGET);

  ucon64gui_top ();

  h2g_ ("Console specific options");
  h2g_br ();


  html2gui_parser ("<input type=\"submit\" name=\"abl\">"
  "kjhsdgkhdfgkjhsdfgkjh"
  "<input type=\"submit\" name=\"l2\">"
  );

#if 0
  h2g_input_submit ("NES", OPTION_LONG_S "nes",
                    "(" OPTION_LONG_S "nes) options for Nintendo Entertainment System/NES\n1983 Nintendo http://www.nintendo.com");
  h2g_input_submit ("GameBoy", OPTION_LONG_S "gb",
                    "(" OPTION_LONG_S "gb) options for GameBoy/(Super GB)/GB Pocket/Color GB/(GB Advance)\n1989/1994/1996/1998/2001 Nintendo http://www.nintendo.com");
  h2g_input_submit ("Super Nintendo", OPTION_LONG_S "snes",
                    "(" OPTION_LONG_S "snes) options for Super Nintendo/SNES/Super Famicon\n1990 Nintendo http://www.nintendo.com");
  h2g_input_submit ("Nintendo 64", OPTION_LONG_S "n64",
                    "(" OPTION_LONG_S "n64) options for Nintendo 64\n1996 Nintendo http://www.nintendo.com");
  h2g_input_submit ("GameBoy Advance", OPTION_LONG_S "gba",
                    "(" OPTION_LONG_S "gba) options for GameBoy Advance\n2001 Nintendo http://www.nintendo.com");

  ucon64gui_spacer ();

  h2g_input_submit ("Sega Master System/Game Gear", OPTION_LONG_S "sms",
                    "(" OPTION_LONG_S "sms) options for Sega Master System(II/III)/GameGear (Handheld)\n1986/19XX SEGA http://www.sega.com");
  h2g_input_submit ("Genesis", OPTION_LONG_S "gen",
                    "(" OPTION_LONG_S "gen) options for Genesis/Sega Mega Drive/Sega CD/32X/Nomad\n1989/19XX/19XX SEGA http://www.sega.com");
  h2g_input_submit ("Dreamcast", OPTION_LONG_S "dc",
                    "(" OPTION_LONG_S "dc) options for Dreamcast\n1998 SEGA http://www.sega.com");

  ucon64gui_spacer ();

  h2g_input_submit ("Lynx", OPTION_LONG_S "lynx",
                    "(" OPTION_LONG_S "lynx) options for Handy(prototype)/Lynx/Lynx II\n1987 Epyx/1989 Atari/1991 Atari");
  h2g_input_submit ("Jaguar", OPTION_LONG_S "jag",
                    "(" OPTION_LONG_S "jag) options for Panther(32bit prototype)/Jaguar64/Jaguar64 CD\n1989 Flare2/1993 Atari/1995 Atari");

  h2g_ (" ");
  h2g_input_submit ("PC-Engine", OPTION_LONG_S "pce",
                    "(" OPTION_LONG_S "pce) options for PC-Engine (CD Unit/Core Grafx(II)/Shuttle/GT/LT/Super CDROM/DUO(-R(X)))/Super Grafx/Turbo (Grafx(16)/CD/DUO/Express)\n1987/19XX/19XX NEC");

  h2g_ (" ");

  h2g_input_submit ("Neo Geo", OPTION_LONG_S "ng",
                    "(" OPTION_LONG_S "ng) options for Neo Geo/Neo Geo CD(Z)/MVS\n1990/1994 SNK http://www.neogeo.co.jp");
  h2g_input_submit ("Neo Geo Pocket", OPTION_LONG_S "ngp",
                    "(" OPTION_LONG_S "ngp) options for Neo Geo Pocket/Neo Geo Pocket Color\n1998/1999 SNK http://www.neogeo.co.jp");

  h2g_ (" ");

  h2g_input_submit ("WonderSwan", OPTION_LONG_S "wswan",
                    "(" OPTION_LONG_S "swan) options for WonderSwan/WonderSwan Color\n19XX/19XX Bandai");
#else
  h2g_select ("page", 0, 0, "Choose your console system here",
    "NES",
    OPTION_LONG_S "nes", 
    "GameBoy",
    OPTION_LONG_S "gb", 
    "Super Nintendo",
    OPTION_LONG_S "snes",
    "Nintendo 64",
    OPTION_LONG_S "n64", 
    "GameBoy Advance", 
    OPTION_LONG_S "gba", 
    "Sega Master System/Game Gear",
    OPTION_LONG_S "sms",
    "Genesis",
    OPTION_LONG_S "gen",
    "Dreamcast",
    OPTION_LONG_S "dc",
    "Lynx",
    OPTION_LONG_S "lynx",
    "Jaguar",
    OPTION_LONG_S "jag",
    "PC-Engine",
    OPTION_LONG_S "pce",
    "Neo Geo",
    OPTION_LONG_S "ng",
    "Neo Geo Pocket",
    OPTION_LONG_S "ngp",
    "WonderSwan",
    OPTION_LONG_S "swan",
    0);
#endif


  ucon64gui_bottom ();

  h2g_form_end ();
  h2g_body_end ();

  h2g_html_end ();
}


void
ucon64gui_divider (void)
{
#include "xpm/trans.xpm"
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();
  h2g_hr ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();
}


void
ucon64gui_spacer (void)
{
#include "xpm/trans.xpm"
  h2g_br ();
  h2g_img (trans_xpm, 0, 3, 0, NULL);
  h2g_br ();
}


