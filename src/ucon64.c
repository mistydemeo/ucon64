/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh


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

/*
first I want to thank SiGMA SEVEN! who was my mentor and taught me how to
write programs in C
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef  __linux__
#include <sys/io.h>
#endif

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  DEBUG
#warning DEBUG active
#endif
#include "getopt.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#ifdef  DLOPEN
#include "dlopen.h"
#endif
#include "libdiscmage/libdiscmage.h"

#include "console/snes.h"
#include "console/gb.h"
#include "console/gba.h"
#include "console/n64.h"
#include "console/lynx.h"
#include "console/sms.h"
#include "console/nes.h"
#include "console/genesis.h"
#include "console/pce.h"
#include "console/neogeo.h"
#include "console/ngp.h"
#include "console/swan.h"
#include "console/dc.h"
#include "console/jaguar.h"
#include "console/psx.h"

#include "patch/ppf.h"
#include "patch/xps.h"
#include "patch/pal4u.h"
#include "patch/aps.h"
#include "patch/ips.h"
#include "patch/bsl.h"
#include "patch/gg.h"

#include "backup/fig.h"
#include "backup/swc.h"
#include "backup/doctor64jr.h"
#include "backup/doctor64.h"
#include "backup/smd.h"
#include "backup/fal.h"
#include "backup/gbx.h"
#include "backup/cd64.h"
#include "backup/dex.h"
#include "backup/fpl.h"
#include "backup/mgd.h"
#include "backup/gd.h"
#include "backup/mccl.h"
#include "backup/lynxit.h"

static void ucon64_exit (void);
static void ucon64_usage (int argc, char *argv[]);
static int ucon64_execute_options (void);
static void ucon64_rom_nfo (const st_rominfo_t *rominfo);

st_ucon64_t ucon64;
static st_rominfo_t rom;
static ucon64_dat_t dat;
ucon64_dat_t *ucon64_dat = NULL;
dm_image_t *image = NULL;
static const char *ucon64_title = "uCON64 " UCON64_VERSION_S " " CURRENT_OS_S " 1999-2003";
static int ucon64_fsize = 0, ucon64_option = 0;

//const option_t options[] = {
const struct option options[] = {
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
    {"disc", 0, 0, UCON64_DISC},
//    {"cd32", 0, 0, UCON64_CD32},
//    {"cdi", 0, 0, UCON64_CDI},
    {"cdirip", 0, 0, UCON64_CDIRIP},
    {"chk", 0, 0, UCON64_CHK},
    {"col", 0, 0, UCON64_COL},
    {"coleco", 0, 0, UCON64_COLECO},
    {"crc", 0, 0, UCON64_CRC},
    {"crchd", 0, 0, UCON64_CRCHD},
    {"crp", 0, 0, UCON64_CRP},
    {"cs", 0, 0, UCON64_CS},
    {"db", 0, 0, UCON64_DB},
    {"dbs", 0, 0, UCON64_DBS},
    {"dbuh", 0, 0, UCON64_DBUH},
    {"dbv", 0, 0, UCON64_DBV},
    {"dc", 0, 0, UCON64_DC},
    {"dint", 0, 0, UCON64_DINT},
    {"e", 0, 0, UCON64_E},
    {"f", 0, 0, UCON64_F},
    {"fds", 0, 0, UCON64_FDS},
    {"fdsl", 0, 0, UCON64_FDSL},
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
    {"gen", 0, 0, UCON64_GEN},
    {"gg", 1, 0, UCON64_GG},
    {"ggd", 1, 0, UCON64_GGD},
    {"gge", 1, 0, UCON64_GGE},
    {"gp32", 0, 0, UCON64_GP32},
    {"good", 0, 0, UCON64_GOOD},
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
    {"lsd", 0, 0, UCON64_LSD},
    {"lsv", 0, 0, UCON64_LSV},
    {"lynx", 0, 0, UCON64_LYNX},
    {"lyx", 0, 0, UCON64_LYX},
    {"mgd", 0, 0, UCON64_MGD},
//    {"mgh", 0, 0, UCON64_MGH},
    {"mka", 0, 0, UCON64_MKA},
    {"mkcue", 0, 0, UCON64_MKCUE},
    {"mki", 0, 0, UCON64_MKI},
    {"mkppf", 0, 0, UCON64_MKPPF},
    {"mksheet", 0, 0, UCON64_MKSHEET},
    {"mktoc", 0, 0, UCON64_MKTOC},
    {"multi", 1, 0, UCON64_MULTI},
    {"mvs", 0, 0, UCON64_MVS},
    {"n", 0, 0, UCON64_N},
    {"n2", 0, 0, UCON64_N2},
    {"n2gb", 0, 0, UCON64_N2GB},
    {"n64", 0, 0, UCON64_N64},
    {"na", 0, 0, UCON64_NA},
    {"nbak", 0, 0, UCON64_NBAK},
    {"ncol", 0, 0, UCON64_NCOL},
    {"nrgrip", 0, 0, UCON64_NRGRIP},
    {"nes", 0, 0, UCON64_NES},
    {"ng", 0, 0, UCON64_NG},
    {"ngp", 0, 0, UCON64_NGP},
    {"nppf", 0, 0, UCON64_NPPF},
    {"nrot", 0, 0, UCON64_NROT},
    {"ns", 0, 0, UCON64_NS},
    {"o", 1, 0, UCON64_O},
    {"p", 0, 0, UCON64_P},
    {"pad", 0, 0, UCON64_PAD},
    {"padhd", 0, 0, UCON64_PADHD},
    {"padn", 1, 0, UCON64_PADN},
    {"pasofami", 0, 0, UCON64_PASOFAMI},
    {"pce", 0, 0, UCON64_PCE},
    {"port", 1, 0, UCON64_PORT},
    {"ppf", 0, 0, UCON64_PPF},
    {"ps2", 0, 0, UCON64_PS2},
    {"psx", 0, 0, UCON64_PSX},
    {"q", 0, 0, UCON64_Q},
    {"qq", 0, 0, UCON64_QQ},
    {"rrom", 0, 0, UCON64_RROM},
    {"rr83", 0, 0, UCON64_RR83},
    {"rl", 0, 0, UCON64_RL},
    {"rom", 1, 0, UCON64_ROM},
    {"rotl", 0, 0, UCON64_ROTL},
    {"rotr", 0, 0, UCON64_ROTR},
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
    {"ssize", 1, 0, UCON64_SSIZE},
    {"stp", 0, 0, UCON64_STP},
    {"stpn", 1, 0, UCON64_STPN},
    {"strip", 0, 0, UCON64_STRIP},
    {"swan", 0, 0, UCON64_SWAN},
    {"swap", 0, 0, UCON64_SWAP},
    {"swc", 0, 0, UCON64_SWC},
    {"swcs", 0, 0, UCON64_SWCS},
#ifdef  DEBUG
    {"test", 0, 0, UCON64_TEST},
#endif // DEBUG
    {"ufos", 0, 0, UCON64_UFOS},
    {"unif", 0, 0, UCON64_UNIF},
    {"usms", 0, 0, UCON64_USMS},
    {"v64", 0, 0, UCON64_V64},
    {"vboy", 0, 0, UCON64_VBOY},
    {"vec", 0, 0, UCON64_VEC},
    {"xbox", 0, 0, UCON64_XBOX},
    {"xcdrw", 0, 0, UCON64_XCDRW},
#ifdef  PARALLEL
    {"xdex", 1, 0, UCON64_XDEX},
    {"xdjr", 0, 0, UCON64_XDJR},
    {"xfal", 0, 0, UCON64_XFAL},
    {"xfalmulti", 1, 0, UCON64_XFALMULTI},
    {"xfalb", 1, 0, UCON64_XFALB},
    {"xfalc", 1, 0, UCON64_XFALC},
    {"xfals", 0, 0, UCON64_XFALS},
    {"xgbx", 0, 0, UCON64_XGBX},
    {"xgbxb", 1, 0, UCON64_XGBXB},
    {"xgbxs", 0, 0, UCON64_XGBXS},
    {"xgd3", 0, 0, UCON64_XGD3},
    {"xlit", 0, 0, UCON64_XLIT},
    {"xmccl", 0, 0, UCON64_XMCCL},
    {"xsmd", 0, 0, UCON64_XSMD},
    {"xsmds", 0, 0, UCON64_XSMDS},
    {"xswc", 0, 0, UCON64_XSWC},
    {"xswc2", 0, 0, UCON64_XSWC2},
    {"xswcs", 0, 0, UCON64_XSWCS},
    {"xv64", 0, 0, UCON64_XV64},
#endif // PARALLEL
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
    {"ctrl2", 1, 0, UCON64_CTRL2},
    {"ntsc", 0, 0, UCON64_NTSC},
    {"pal", 0, 0, UCON64_PAL},
    {"bat", 0, 0, UCON64_BAT},
    {"nbat", 0, 0, UCON64_NBAT},
    {"vram", 0, 0, UCON64_VRAM},
    {"nvram", 0, 0, UCON64_NVRAM},
    {"mirr", 1, 0, UCON64_MIRR},
    {"mapr", 1, 0, UCON64_MAPR},
    {"cmnt", 1, 0, UCON64_CMNT},                // will be active only if UNIF_REVISION > 7
    {"dumpinfo", 0, 0, UCON64_DUMPINFO},
    {"version", 0, 0, UCON64_VER},
    {0, 0, 0, 0}
  };

#ifdef  DLOPEN
void *libdm;

#define dm_open dm_open_ptr
#define dm_close dm_close_ptr
dm_image_t *(*dm_open_ptr) (const char *) = NULL;
int (*dm_close_ptr) (dm_image_t *) = NULL;

#define dm_rip dm_rip_ptr
#define dm_cdirip dm_cdirip_ptr
#define dm_nrgrip dm_nrgrip_ptr
int32_t (*dm_rip_ptr) (dm_image_t *) = NULL;
int32_t (*dm_cdirip_ptr) (dm_image_t *) = NULL;
int32_t (*dm_nrgrip_ptr) (dm_image_t *) = NULL;

#define dm_disc_read dm_disc_read_ptr
#define dm_disc_write dm_disc_write_ptr
int (*dm_disc_read_ptr) (dm_image_t *) = NULL;
int (*dm_disc_write_ptr) (dm_image_t *) = NULL;

#define dm_mksheets dm_mksheets_ptr
int32_t (*dm_mksheets_ptr) (dm_image_t *) = NULL;

#define dm_mktoc dm_mktoc_ptr
#define dm_mkcue dm_mkcue_ptr
int32_t (*dm_mktoc_ptr) (dm_image_t *) = NULL;
int32_t (*dm_mkcue_ptr) (dm_image_t *) = NULL;
#endif


void
ucon64_exit (void)
{
  handle_registered_funcs ();
  fflush (stdout);
}


int
main (int argc, char **argv)
{
  int c = 0, console, show_nfo, rom_index, file_message = 0;
  char buf[MAXBUFSIZE], *ptr;

  printf ("%s\n"
    "Uses code from various people. See 'developers.html' for more!\n"
    "This may be freely redistributed under the terms of the GNU Public License\n\n",
    ucon64_title);

  if (atexit (ucon64_exit) == -1)
    {
      fprintf (stderr, "ERROR: Could not register function with atexit()\n");
      exit (1);
    }

  memset (&ucon64, 0L, sizeof (st_ucon64_t));

#ifdef  __unix__
  // We need to modify the umask, because the configfile is made while we are
  //  still running in root mode. Maybe 0 is even better (in case root did
  //  `chmod +s').
  umask (002);
#endif

  ucon64_configfile ();

#ifdef  DLOPEN
  strcpy (ucon64.discmage_path, get_property (ucon64.configfile, "discmage_path", buf, ""));
#ifdef  __CYGWIN__
  strcpy (ucon64.discmage_path, cygwin_fix (ucon64.discmage_path));
#endif
  if (strlen (ucon64.discmage_path) >= 3)
    {
      strcpy (buf, ucon64.discmage_path);
      realpath2 (buf, ucon64.discmage_path);
    }

  // if ucon64.discmage_path points to an existing file then load it
  if (!access (ucon64.discmage_path, F_OK))
    {
      libdm = open_module (ucon64.discmage_path);

      dm_open = get_symbol (libdm, "dm_open");
      dm_close = get_symbol (libdm, "dm_close");

      dm_rip = get_symbol (libdm, "dm_rip");
      dm_cdirip = get_symbol (libdm, "dm_cdirip");
      dm_nrgrip = get_symbol (libdm, "dm_nrgrip");

      dm_disc_read = get_symbol (libdm, "dm_disc_read");
      dm_disc_write = get_symbol (libdm, "dm_disc_write");

      dm_mksheets = get_symbol (libdm, "dm_mksheets");
      dm_mktoc = get_symbol (libdm, "dm_mktoc");
      dm_mkcue = get_symbol (libdm, "dm_mkcue");

      ucon64.discmage_enabled = 1;
    }
  else
    ucon64.discmage_enabled = 0;
#else // !defined DLOPEN
#ifdef  DJGPP
  {
    /*
      The following piece of code makes the DLL "search" behaviour a bit like
      the search behaviour for Windows programs. A bit, because the import
      library just opens the file with the name that is stored in
      djimport_path. It won't search for the DXE in the Windows system
      directory, nor will it search the directories of the PATH environment
      variable.
    */
    extern char djimport_path[FILENAME_MAX];
    char *p;

    strcpy (buf, argv[0]);
    // the next statement is not necessary, but will avoid confusion
    change_string ("/", 1, 0, 0, FILE_SEPARATOR_S, 1, buf, strlen (buf), 0);
    if ((p = strrchr (buf, FILE_SEPARATOR)))
      *p = 0;
    sprintf (djimport_path, "%s"FILE_SEPARATOR_S"%s", buf, "discmage.dxe");
  }
#endif // DJGPP
  ucon64.discmage_enabled = 1;
#endif

  ucon64.show_nfo = UCON64_YES;

#ifdef  ANSI_COLOR
  ucon64.ansi_color = ((!strcmp (get_property (ucon64.configfile, "ansi_color", buf, "1"), "1")) ?
               1 : 0);
#else
  ucon64.ansi_color = 0;
#endif

//  ucon64.type =
  ucon64.buheader_len =
  ucon64.interleaved =
  ucon64.split =
  ucon64.part_size =
  ucon64.snes_hirom =
  ucon64.bs_dump =
  ucon64.controller =
  ucon64.controller2 =
  ucon64.tv_standard =
  ucon64.battery =
  ucon64.vram =
  ucon64.mirror =
  ucon64.use_dump_info =
  ucon64.console =
  ucon64.do_not_calc_crc =
  ucon64.crc_big_files = UCON64_UNKNOWN;

  ucon64.rom =
  ucon64.file =
  ucon64.mapr =
  ucon64.comment = "";

  getcwd (ucon64.output_path, FILENAME_MAX); // default output path
  if (OFFSET (ucon64.output_path, strlen (ucon64.output_path) - 1) != FILE_SEPARATOR)
    strcat (ucon64.output_path, FILE_SEPARATOR_S);

  strcpy (ucon64.configdir, get_property (ucon64.configfile, "configdir", buf, ""));
#ifdef  __CYGWIN__
  strcpy (ucon64.configdir, cygwin_fix (ucon64.configdir));
#endif
  if (strlen (ucon64.configdir) >= 3)
    {
      strcpy (buf, ucon64.configdir);
      realpath2 (buf, ucon64.configdir);
    }

  if (!access (ucon64.configdir, F_OK))
    ucon64.dat_enabled = 1;
  else
    ucon64.dat_enabled = 0;

  // if the config file doesn't contain a parport line use "0" to force probing
  sscanf (get_property (ucon64.configfile, "parport", buf, "0"), "%x", &ucon64.parport);

  ucon64.backup = ((!strcmp (get_property (ucon64.configfile, "backups", buf, "1"), "1")) ?
               1 : 0);

  if (argc < 2)
    {
       ucon64_usage (argc, argv);
       return 0;
    }

  ucon64.argc = argc;
  ucon64.argv = argv;

#ifdef  ANSI_COLOR
  if (ucon64.ansi_color)
    ucon64.ansi_color = ansi_init ();
#endif

  if (ucon64.dat_enabled)
    ucon64_dat_indexer (); // update cache index file (eventually)

  ucon64_flush (&rom);

  while ((c = getopt_long_only (argc, argv, "", options, NULL)) != -1)
    {
#include "switches.c"
    }

  if (!strlen (ucon64.rom) && optind < argc)
    ucon64.rom = argv[optind];

//  ucon64.rom = ucon64_extract (ucon64.rom);

  rom_index = optind;                           // save index of first file
  /*
    We want to make it as easy as possible to use uCON64. Therefore don't
    require that --file is specified when only two non-switch non-option
    arguments are specified on the command line. Of course this introduces the
    problem that when only two files match with a wildcard, the second file
    will be interpreted as the --file argument. I (dbjh) think it's better to
    display a message in that case instead of always having to use --file. Now
    we can do the following:
      ucon64 -swc *.fig
      ucon64 -i mario.swc mario.ips
      ucon64 -xswc2 mario.swc 0x378
      ucon64 -ls /usr/local/snesrom /mnt/xp/snesrom --file=0
    In the first case a message will be displayed if only two files match with
    *.fig. In the second case a message will be displayed. In the third case a
    message will be displayed in the unlikely situation that the file 0x378
    exists. In the fourth case no message will be displayed.
    Note that getopt[_long_only]() sorts argv so that all switches and options
    come before all non-switches and non-options.
  */
  if (argc - rom_index == 2 && !strlen (ucon64.file))
    {
      ucon64.file = argv[argc - 1];
      if (!access (ucon64.file, F_OK))
        file_message = 1;
      argc--;                                   // use argc, NOT ucon64.argc!
    }

#ifdef  PARALLEL
  if (ucon64.file)
    sscanf (ucon64.file, "%x", &ucon64.parport);
#endif

  // no ROM or FILE argument (for example -dbv)
  if (rom_index == argc)
    ucon64_execute_options ();

  console = ucon64.console;
  show_nfo = ucon64.show_nfo;
  while (rom_index < argc)                      // use argc, NOT ucon64.argc!
    {
      ucon64.rom = argv[rom_index];
      ucon64.console = console;

      ucon64.show_nfo = show_nfo;
//      if (!ucon64_init (ucon64.rom, &rom))
      ucon64_init (ucon64.rom, &rom);
        if (ucon64.show_nfo == UCON64_YES)
          ucon64_nfo (&rom);

      ucon64.show_nfo = UCON64_NO;

      ucon64_execute_options ();

      if (ucon64.show_nfo == UCON64_YES)
        {
//          if (!ucon64_init (ucon64.rom, &rom))
          ucon64_init (ucon64.rom, &rom);
          ucon64_nfo (&rom);
        }

      /*
        Some options take more than one file as argument, but should be
        executed only once. Options that use the --file argument, needn't be
        specified here, because argc has already been decremented. See the code
        that sets ucon64.file.
      */
      switch (ucon64_option)
        {
        case UCON64_MULTI:                      // falling through
        case UCON64_XFALMULTI:
          rom_index = argc;                     // this will stop the main loop
          break;
        default:
          ;
        }

      rom_index++;
    }

  if (file_message)
    printf ("NOTE: Use --file=0 if \"%s\"\n"
            "      should NOT be interpreted as --file argument\n", ucon64.file);

  return 0;
}


int
ucon64_execute_options (void)
// execute all options for a single file
{
  int ucon64_argc, c, result = 0, value = 0;
  unsigned int padded;
  char buf[MAXBUFSIZE], src_name[FILENAME_MAX];
  const char *ucon64_argv[128];

  optind = 0;                                   // start with first option
  while ((ucon64_option = c =
            getopt_long_only (ucon64.argc, ucon64.argv, "", options, NULL)) != -1)
    {
#include "options.c"
    }

  return result;
}


st_rominfo_t *
ucon64_flush (st_rominfo_t *rominfo)
{
  memset (rominfo, 0L, sizeof (st_rominfo_t));
  rominfo->data_size = UCON64_UNKNOWN;
  ucon64.file_size = ucon64_fsize;
  ucon64.crc32 = 0;
#if 0
  rominfo->maker = rominfo->country = "";
  rominfo->console_usage = rominfo->copier_usage = NULL;
#endif

// restoring the overrides from st_ucon64_t
  if (UCON64_ISSET (ucon64.buheader_len))
    rominfo->buheader_len = ucon64.buheader_len;

  if (UCON64_ISSET (ucon64.snes_hirom))
    rominfo->snes_hirom = ucon64.snes_hirom;

  if (UCON64_ISSET (ucon64.interleaved))
    rominfo->interleaved = ucon64.interleaved;

  return rominfo;
}


int
ucon64_console_probe (st_rominfo_t *rominfo)
{
  ucon64_flush (rominfo);

  switch (ucon64.console)
    {
    case UCON64_GB:
      gameboy_init (rominfo);
      break;

    case UCON64_GBA:
      gba_init (rominfo);
      break;

    case UCON64_GENESIS:
      genesis_init (rominfo);
      break;

    case UCON64_N64:
      n64_init (rominfo);
      break;

    case UCON64_SNES:
      snes_init (rominfo);
      break;

    case UCON64_SMS:
      sms_init (rominfo);
      break;

    case UCON64_LYNX:
      lynx_init (rominfo);
      break;

    case UCON64_NEOGEO:
      neogeo_init (rominfo);
      break;

    case UCON64_NES:
      nes_init (rominfo);
      break;

    case UCON64_PCE:
      pcengine_init (rominfo);
      break;

    case UCON64_NEOGEOPOCKET:
      ngp_init (rominfo);
      break;

    case UCON64_WONDERSWAN:
      swan_init (rominfo);
      break;

    case UCON64_DC:
      dc_init (rominfo);
      break;

    case UCON64_JAGUAR:
      jaguar_init (rominfo);
      break;

    case UCON64_PSX:
      psx_init (rominfo);
      break;

#if 0
    case UCON64_SATURN:
    case UCON64_CDI:
    case UCON64_CD32:
    case UCON64_GAMECUBE:
    case UCON64_XBOX:
    case UCON64_GP32:
    case UCON64_REAL3DO:
    case UCON64_COLECO:
    case UCON64_INTELLI:
    case UCON64_PS2:
    case UCON64_SYSTEM16:
    case UCON64_ATARI:
    case UCON64_VECTREX:
    case UCON64_VIRTUALBOY:
      break;
#endif

    case UCON64_UNKNOWN:
      if (UCON64_TYPE_ISROM (ucon64.type))
        ucon64.console =
          (!gba_init (ucon64_flush (rominfo))) ? UCON64_GBA :
          (!n64_init (ucon64_flush (rominfo))) ? UCON64_N64 :
          (!genesis_init (ucon64_flush (rominfo))) ? UCON64_GENESIS :
          (!lynx_init (ucon64_flush (rominfo))) ? UCON64_LYNX :
          (!gameboy_init (ucon64_flush (rominfo))) ? UCON64_GB :
          (!snes_init (ucon64_flush (rominfo))) ? UCON64_SNES :
          (!nes_init (ucon64_flush (rominfo))) ? UCON64_NES :
          (!ngp_init (ucon64_flush (rominfo))) ? UCON64_NEOGEOPOCKET :
          (!swan_init (ucon64_flush (rominfo))) ? UCON64_WONDERSWAN :
          (!jaguar_init (ucon64_flush (rominfo))) ? UCON64_JAGUAR :
          UCON64_UNKNOWN;

      if (ucon64.console == UCON64_UNKNOWN)
        ucon64_flush (rominfo);

      return (ucon64.console == UCON64_UNKNOWN) ? (-1) : 0;

    default:
      ucon64.console = UCON64_UNKNOWN;
      return -1;
    }
  return 0;
}


int
ucon64_init (const char *romfile, st_rominfo_t *rominfo)
{
  int result = -1;
  struct stat fstate;

  if (access (romfile, F_OK | R_OK) == -1)
    return -1;
  if (!stat (romfile, &fstate) == -1)
    return -1;
  if (S_ISREG (fstate.st_mode) != TRUE)
    return -1;

  ucon64_fsize = q_fsize (ucon64.rom);          // save size in ucon64_fsize
  ucon64.file_size = ucon64_fsize;

//  what media type? do not calc crc32 and stuff for DISC images (speed!)
  ucon64.type = (ucon64.file_size <= MAXROMSIZE) ? UCON64_ROM : UCON64_DISC;

  if (ucon64.discmage_enabled)
    {
#if 0
      image = dm_open (ucon64.rom);
      ucon64.type = (image ? UCON64_DISC : UCON64_ROM);
      dm_close (image);
#endif      
      image = NULL;
    }

  if (UCON64_TYPE_ISROM (ucon64.type))
    {
      ucon64_flush (rominfo); // clear rominfo

      result = ucon64_console_probe (rominfo);

      // Calculating the CRC for the ROM data of a UNIF file (NES) shouldn't
      //  be done with q_fcrc32(). nes_init() uses mem_crc32().
      if (ucon64.crc32 == 0)
//        if (ucon64.do_not_calc_crc != UCON64_UNKNOWN)
          if (ucon64.crc_big_files != UCON64_UNKNOWN ||
            UCON64_TYPE_ISROM (ucon64.type))
        ucon64.crc32 = q_fcrc32 (romfile, rominfo->buheader_len);

      if (ucon64.dat_enabled)
        {
          memset (&dat, 0, sizeof (ucon64_dat_t));
          ucon64_dat = ucon64_dat_search (ucon64.crc32, &dat);
        }
      else
        ucon64_dat = NULL;

      switch (ucon64.console)
        {
          case UCON64_SNES:
          case UCON64_GENESIS:
          case UCON64_GB:
          case UCON64_GBA:
          case UCON64_N64:
// These ROMs have internal headers with name, country, maker, etc.
            break;

          default:
            if (ucon64_dat)
              {
//                strcpy (rominfo->name, ucon64_dat->name);

//                if (ucon64.console == UCON64_UNKNOWN)
//                  ucon64.console = ucon64_dat->console;
              }
            break;
        }
    }
  else if (UCON64_TYPE_ISDISC (ucon64.type))
    {
#if 0
#endif
      result = -1;
    }

  return result;
}


int
ucon64_nfo (const st_rominfo_t *rominfo)
{
  printf ("%s\n\n", ucon64.rom);

  if (ucon64.console == UCON64_UNKNOWN)
    {
      fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
      printf ("\n");
    }

  if (UCON64_TYPE_ISDISC (ucon64.type))
    {
//      if (ucon64.discmage_enabled)
//        dm_image_nfo (image);
    }
  else if (UCON64_TYPE_ISROM (ucon64.type))
    {        
      if (rominfo && ucon64.console != UCON64_UNKNOWN)
        ucon64_rom_nfo (rominfo);
    }

  if (ucon64.crc32)
    printf ("Checksum (CRC32): 0x%08x\n", ucon64.crc32);

  if (ucon64.dat_enabled)
    ucon64_dat_nfo (ucon64_dat);

  printf ("\n");

  return 0;
}


void
ucon64_rom_nfo (const st_rominfo_t *rominfo)
{
  unsigned int padded = ucon64_testpad (ucon64.rom, (st_rominfo_t *) rominfo);
  unsigned int intro = ((ucon64.file_size - rominfo->buheader_len) > MBIT) ?
    ((ucon64.file_size - rominfo->buheader_len) % MBIT) : 0;
  int split = (UCON64_ISSET (ucon64.split)) ? ucon64.split :
    ucon64_testsplit (ucon64.rom);
  char buf[MAXBUFSIZE];
  int x;

// backup unit header
  if (rominfo->buheader && rominfo->buheader_len && rominfo->buheader_len != SWC_HEADER_LEN)
    {
      mem_hexdump (rominfo->buheader, rominfo->buheader_len, rominfo->buheader_start);
      printf ("\n");
    }

// backup unit type?
  if (rominfo->copier_usage != NULL)
    {
      strcpy (buf, rominfo->copier_usage[0]);
      printf ("%s\n", to_func (buf, strlen (buf), toprint2));

#if 0
      if (rominfo->copier_usage[1])
        {
          strcpy (buf, rominfo->copier_usage[1]);
          printf ("  %s\n", to_func (buf, strlen (buf), toprint2));
        }
#endif
      printf ("\n");
    }

// ROM header
  if (rominfo->header && rominfo->header_len)
    {
      mem_hexdump (rominfo->header, rominfo->header_len,
        rominfo->header_start + rominfo->buheader_len);
      printf ("\n");
    }

// console type?
  if (rominfo->console_usage != NULL)
    {
      strcpy (buf, rominfo->console_usage[0]);
      printf ("%s\n", to_func (buf, strlen (buf), toprint2));

#if 0
      if (rominfo->console_usage[1])
        {
          strcpy (buf, rominfo->console_usage[1]);
          printf ("  %s\n", to_func (buf, strlen (buf), toprint2));
        }
#endif
    }

// maker, country and size
  strcpy (buf, NULL_TO_EMPTY (rominfo->name));
  x = UCON64_ISSET (rominfo->data_size) ?
    rominfo->data_size :
    ucon64.file_size - rominfo->buheader_len;
  printf ("%s\n%s\n%s\n%d Bytes (%.4f Mb)\n\n",
          // some ROMs have a name with control chars in it -> replace control chars
          to_func (buf, strlen (buf), toprint2),
          NULL_TO_EMPTY (rominfo->maker),
          NULL_TO_EMPTY (rominfo->country),
          x,
          TOMBIT_F (x));


// padded?
      if (!padded)
        printf ("Padded: No\n");
      else
        printf ("Padded: Maybe, %d Bytes (%.4f Mb)\n", padded,
                TOMBIT_F (padded));

// intro, trainer?
      // nes.c determines itself whether or not there is a trainer
      if (intro && ucon64.console != UCON64_NES)
        printf ("Intro/Trainer: Maybe, %d Bytes\n", intro);

// interleaved?
      if (rominfo->interleaved != UCON64_UNKNOWN)
        // printing this is handy for SNES, N64 & Genesis ROMs, but maybe
        //  nonsense for others
        printf ("Interleaved/Swapped: %s\n",
          rominfo->interleaved ?
            (rominfo->interleaved > 1 ?
              "Yes (2)" :
              "Yes") :
            "No");

// backup unit header?
      if (rominfo->buheader_len)
        printf ("Backup unit/emulator header: Yes, %d Bytes\n",
          rominfo->buheader_len);
      else
// for NoisyB: <read only mode ON>
        printf ("Backup unit/emulator header: No\n"); // printing No is handy for SNES ROMs
// for NoisyB: <read only mode OFF>

// split?
      if (split)
        {
          printf ("Split: Yes, %d part%s\n", split, (split != 1) ? "s" : "");
          // nes.c calculates the correct checksum for split ROMs (=Pasofami
          // format), so there is no need to join the files
          if (ucon64.console != UCON64_NES)
            printf ("NOTE: to get the correct checksum the ROM must be joined\n");
        }


// miscellaneous info
  if (rominfo->misc[0])
    {
      strcpy (buf, rominfo->misc);
      printf ("%s\n", to_func (buf, strlen (buf), toprint2));
    }

// internal checksums?
      if (rominfo->has_internal_crc)
        {
          char *fstr;

          // the internal checksum of GBA ROMS stores only the checksum of the
          //  internal header
          if (ucon64.console != UCON64_GBA)
            fstr = "Checksum: %%s, 0x%%0%dlx (calculated) %%s= 0x%%0%dlx (internal)\n";
          else
            fstr = "Header checksum: %%s, 0x%%0%dlx (calculated) %%s= 0x%%0%dlx (internal)\n";

          sprintf (buf, fstr,
            rominfo->internal_crc_len * 2, rominfo->internal_crc_len * 2);
          printf (buf,
            ucon64.ansi_color ?
              ((rominfo->current_internal_crc == rominfo->internal_crc) ?
                "\x1b[01;32mok\x1b[0m" : "\x1b[01;31mbad\x1b[0m")
              :
              ((rominfo->current_internal_crc == rominfo->internal_crc) ? "ok" : "bad"),
            rominfo->current_internal_crc,
            (rominfo->current_internal_crc == rominfo->internal_crc) ? "=" : "!",
            rominfo->internal_crc);

          if (rominfo->internal_crc2[0])
            {
              strcpy (buf, rominfo->internal_crc2);
              printf ("%s\n", to_func (buf, strlen (buf), toprint2));
            }
        }

  fflush (stdout);
}


#if 1
#define UCON64_USAGE(s) printf("%s%s%s", \
                          NULL_TO_EMPTY (s[0]), \
                          s[0]?"\n":"", \
                          NULL_TO_EMPTY (s[2]))
#else
void
usage (const char **s)
{
  printf("%s%s%s%s%s",
    NULL_TO_EMPTY (s[0]),
    s[0]?s[1]?"\n  ":"\n":"",
    NULL_TO_EMPTY (s[1]),
    s[1]?"\n":"",
    NULL_TO_EMPTY (s[2]));
}
#define UCON64_USAGE usage
#endif


void
ucon64_usage (int argc, char *argv[])
{
  int c = 0, single = 0;

#ifdef  ANSI_COLOR
#define ANSI_COLOR_MSG "  " OPTION_LONG_S "ncol        disable ANSI colors in output\n"
#endif

#ifdef  __MSDOS__
#define HEXDUMP_MSG "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex " OPTION_LONG_S "rom=ROM|more\"\n"
#else
#define HEXDUMP_MSG "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex " OPTION_LONG_S "rom=ROM|less\"\n"       // less is more ;-)
#endif

#ifdef  __MSDOS__
#define GOOD_EXAMPLE  "                  Example: %s " OPTION_LONG_S "rrom " OPTION_LONG_S "good C:\\MAME_ROMS\\\n"
#else
#define GOOD_EXAMPLE  "                  Example: %s " OPTION_LONG_S "rrom " OPTION_LONG_S "good /home/joe/mame/\n"
#endif

  printf (
    "Usage: %s [OPTION]... [" OPTION_LONG_S "rom=][ROM]... [[" OPTION_LONG_S "file=]FILE]" /* [-o=OUTPUT_PATH] */ "\n\n"
    "  " OPTION_LONG_S "nbak        prevents backup files (*.BAK)\n"
    ANSI_COLOR_MSG
    "  " OPTION_LONG_S "hdn=N       force ROM has backup unit/emulator header with N Bytes size\n"
    "  " OPTION_LONG_S "hd          same as " OPTION_LONG_S "hdn=512\n"
    "                  most backup units use a header with 512 Bytes size\n"
    "  " OPTION_LONG_S "nhd         force ROM has no backup unit/emulator header\n"
    "  " OPTION_LONG_S "int         force ROM is interleaved (2143)\n"
    "  " OPTION_LONG_S "nint        force ROM is not interleaved (1234)\n"
    "  " OPTION_LONG_S "dint        convert ROM to (non-)interleaved format (1234 <-> 2143)\n"
    "                  this differs from the SNES & NES " OPTION_LONG_S "dint option\n"
    "  " OPTION_LONG_S "ns          force ROM is not split\n"
    "  " OPTION_S "e           emulate/run ROM (see %s for more)\n"
    "  " OPTION_LONG_S "crc         show CRC32 value of ROM; this will also force calculation for\n"
    "                  files bigger than %d Bytes (%.4f Mb)\n"

    "  " OPTION_LONG_S "ls          generate ROM list for all ROMs; " OPTION_LONG_S "rom=DIRECTORY\n"
    "  " OPTION_LONG_S "lsv         like " OPTION_LONG_S "ls but more verbose; " OPTION_LONG_S "rom=DIRECTORY\n"

/*
    "  " OPTION_LONG_S "rl          rename all files in DIRECTORY to lowercase; " OPTION_LONG_S "rom=DIRECTORY\n"
    "  " OPTION_LONG_S "ru          rename all files in DIRECTORY to uppercase; " OPTION_LONG_S "rom=DIRECTORY\n"
*/
    HEXDUMP_MSG
    "  " OPTION_LONG_S "find        find string in ROM; " OPTION_LONG_S "file=STRING (wildcard: '?')\n"
    "  " OPTION_S "c           compare ROMs for differencies; " OPTION_LONG_S "file=OTHER_ROM\n"
    "  " OPTION_LONG_S "cs          compare ROMs for similarities; " OPTION_LONG_S "file=OTHER_ROM\n"
    "  " OPTION_LONG_S "help        display this help and exit\n"
    "  " OPTION_LONG_S "version     output version information and exit\n"
    "  " OPTION_S "q           be quiet (don't show ROM info)\n"
//    "  " OPTION_LONG_S "qq          be even more quiet\n"
    "\n",
    argv[0], ucon64.configfile, MAXROMSIZE, TOMBIT_F (MAXROMSIZE));

  printf ("Padding\n"
    "  " OPTION_LONG_S "stpn=N      strip N Bytes from ROM beginning\n"
    "  " OPTION_LONG_S "stp         same as " OPTION_LONG_S "stpn=512\n"
    "                  most backup units use a header with 512 Bytes size\n"
    "  " OPTION_LONG_S "insn=N      insert N Bytes (0x00) before ROM\n"
    "  " OPTION_LONG_S "ins         same as " OPTION_LONG_S "insn=512\n"
    "                  most backup units use a header with 512 Bytes size\n"
    "  " OPTION_LONG_S "ispad       check if ROM is padded\n"
    "  " OPTION_S "p, " OPTION_LONG_S "pad    pad ROM to full Mb\n"
    "  " OPTION_LONG_S "padn=N      pad ROM to N Bytes (put Bytes with value 0x00 after end)\n"
    "  " OPTION_LONG_S "strip       strip Bytes from end of ROM; " OPTION_LONG_S "file=VALUE\n"
    "\n");

//  if (ucon64.dat_enabled)
    printf ("DATabase (support for DAT files)\n"
      "  " OPTION_LONG_S "dbs         search ROM in DATabase by CRC32; " OPTION_LONG_S "rom=0xCRC32\n"
      "  " OPTION_LONG_S "db          DATabase statistics\n"
      "  " OPTION_LONG_S "dbv         like " OPTION_LONG_S "db but more verbose\n"
      "  " OPTION_LONG_S "lsd         generate ROM list for all ROMs using DATabase info; " OPTION_LONG_S "rom=DIR\n"
      "  " OPTION_LONG_S "rrom        rename all ROMs in DIRECTORY to their internal names; " OPTION_LONG_S "rom=DIR\n"
      "                  with " OPTION_LONG_S "good you will use DATabase instead of internal names\n"
      "                  and sort the ROMs into subdirs (DAT files: %s)\n"
      GOOD_EXAMPLE
      "                  Only ROMs of these consoles use to have internal names:\n"
      "                  %s,\n"
      "                  %s,\n"
      "                  %s,\n"
      "                  %s, and %s\n"
      "  " OPTION_LONG_S "rr83        like " OPTION_LONG_S "rrom but with 8.3 filenames; " OPTION_LONG_S "rom=DIRECTORY\n\n",
      ucon64.configdir,
      argv[0],
      snes_usage[0],
      genesis_usage[0],
      gameboy_usage[0],
      gba_usage[0],
      n64_usage[0]);

  printf ("Patching\n");

  UCON64_USAGE (bsl_usage);
  UCON64_USAGE (ips_usage);
  UCON64_USAGE (aps_usage);
  UCON64_USAGE (pal4u_usage);
  UCON64_USAGE (ppf_usage);
  UCON64_USAGE (xps_usage);
  UCON64_USAGE (gg_usage);

  printf ("                  supported are:\n"
    "                  %s,\n"
    "                  %s,\n"
//    "                  %s,\n"
    "                  %s,\n"
    "                  %s\n",
    gameboy_usage[0], sms_usage[0],
//    genesis_usage[0],
    nes_usage[0], snes_usage[0]);

    printf (
      "\n"
      "All DISC-based consoles (using libdiscmage)\n");
  if (ucon64.discmage_enabled)
    printf (
      "  " OPTION_LONG_S "mksheet     generate TOC and CUE sheet files for IMAGE; " OPTION_LONG_S "rom=IMAGE\n"
//      "                  " OPTION_LONG_S "rom could also be an existing TOC or CUE file\n"
      "TODO: " OPTION_LONG_S "cdirip=Nrip/dump track N from DiscJuggler/CDI IMAGE; " OPTION_LONG_S "rom=CDI_IMAGE\n"
      "TODO: " OPTION_LONG_S "nrgrip=Nrip/dump track N from Nero/NRG IMAGE; " OPTION_LONG_S "rom=NRG_IMAGE\n"
      "TODO: " OPTION_LONG_S "rip     rip/dump file(s) from a track; " OPTION_LONG_S "rom=TRACK\n"
/*
    OPTION_LONG_S "file=SECTOR_SIZE\n"
      "TODO: " OPTION_LONG_S "iso     strip SECTOR_SIZE of any CD_IMAGE to MODE1/2048; " OPTION_LONG_S "rom=CD_IMAGE\n"
      "                  " OPTION_LONG_S "file=SECTOR_SIZE\n"
      "                  " OPTION_LONG_S "file=SECTOR_SIZE is optional, uCON64 will always try to\n"
      "                  detect the correct SECTOR_SIZE from the CD_IMAGE itself\n"
      "                  SECTOR_SIZE can be 2048, 2052, 2056, 2324, 2332, 2336, 2340,\n"
      "                  2352 (default), or custom values\n"
*/
      "\n");
  else
    printf ("                %s not found; support disabled\n\n", ucon64.discmage_path);

  optind = 0;
  single = 0;

  while (!single && (c = getopt_long_only (argc, argv, "", options, NULL)) != -1)
    {
//      if (single) break;

      switch (c)
        {
      case UCON64_GBA:
        UCON64_USAGE (gba_usage);
#ifdef  PARALLEL
        UCON64_USAGE (fal_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_N64:
        UCON64_USAGE (n64_usage);
#ifdef  PARALLEL
        UCON64_USAGE (doctor64_usage);
        UCON64_USAGE (doctor64jr_usage);
//        UCON64_USAGE (cd64_usage);
        UCON64_USAGE (dex_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_JAG:
        UCON64_USAGE (jaguar_usage);
        single = 1;
        break;

      case UCON64_SNES:
        UCON64_USAGE (snes_usage);
#ifdef  PARALLEL
        UCON64_USAGE (swc_usage);
        UCON64_USAGE (gd_usage);
//        UCON64_USAGE (fig_usage);
//        UCON64_USAGE (mgd_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_NG:
        UCON64_USAGE (neogeo_usage);
        single = 1;
        break;

      case UCON64_NGP:
        UCON64_USAGE (ngp_usage);
#ifdef  PARALLEL
//        UCON64_USAGE (fpl_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_GEN:
        UCON64_USAGE (genesis_usage);
#ifdef  PARALLEL
        UCON64_USAGE (smd_usage);
//        UCON64_USAGE (mgd_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_GB:
        UCON64_USAGE (gameboy_usage);
#ifdef  PARALLEL
        UCON64_USAGE (gbx_usage);
        UCON64_USAGE (mccl_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_LYNX:
        UCON64_USAGE (lynx_usage);
#ifdef  PARALLEL
        UCON64_USAGE (lynxit_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_PCE:
        UCON64_USAGE (pcengine_usage);
#ifdef  PARALLEL
//        UCON64_USAGE (mgd_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_SMS:
        UCON64_USAGE (sms_usage);
#ifdef  PARALLEL
//        UCON64_USAGE (smd_usage);
#endif // PARALLEL
        single = 1;
        break;

      case UCON64_NES:
        UCON64_USAGE (nes_usage);
        single = 1;
        break;

      case UCON64_SWAN:
        UCON64_USAGE (swan_usage);
        single = 1;
        break;

      case UCON64_DC:
        UCON64_USAGE (dc_usage);
        single = 1;
        break;

      case UCON64_PSX:
        UCON64_USAGE (psx_usage);
#ifdef  PARALLEL
        UCON64_USAGE (dex_usage);
#endif // PARALLEL
        single = 1;
        break;

#if 0
      case UCON64_GC:
      case UCON64_S16:
      case UCON64_ATA:
      case UCON64_COLECO:
      case UCON64_VBOY:
      case UCON64_VEC:
      case UCON64_INTELLI:
      case UCON64_PS2:
      case UCON64_SAT:
      case UCON64_3DO:
      case UCON64_CD32:
      case UCON64_CDI:
      case UCON64_XBOX:
      case UCON64_GP32:
        break;
#endif

      default:
        break;
      }
      printf ("\n");
    }

  if (!single)
    {
      UCON64_USAGE (dc_usage);
      printf("\n");

      UCON64_USAGE (psx_usage);
#ifdef  PARALLEL
      UCON64_USAGE (dex_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (gba_usage);
#ifdef  PARALLEL
      UCON64_USAGE (fal_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (n64_usage);
#ifdef  PARALLEL
      UCON64_USAGE (doctor64_usage);
      UCON64_USAGE (doctor64jr_usage);
//      UCON64_USAGE (cd64_usage);
      UCON64_USAGE (dex_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (snes_usage);
#ifdef  PARALLEL
      UCON64_USAGE (swc_usage);
      UCON64_USAGE (gd_usage);
//      UCON64_USAGE (fig_usage);
//      UCON64_USAGE (mgd_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (neogeo_usage);
      printf ("\n");

      UCON64_USAGE (genesis_usage);
#ifdef  PARALLEL
      UCON64_USAGE (smd_usage);
//      UCON64_USAGE (mgd_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (gameboy_usage);
#ifdef  PARALLEL
      UCON64_USAGE (gbx_usage);
      UCON64_USAGE (mccl_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (lynx_usage);
#ifdef  PARALLEL
      UCON64_USAGE (lynxit_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (pcengine_usage);
#ifdef  PARALLEL
//      UCON64_USAGE (mgd_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (sms_usage);
#ifdef  PARALLEL
//      UCON64_USAGE (smd_usage);
#endif // PARALLEL
      printf ("\n");

      UCON64_USAGE (nes_usage);
      printf ("\n");

      UCON64_USAGE (swan_usage);
      printf ("\n");

      UCON64_USAGE (jaguar_usage);
      printf ("\n");

      UCON64_USAGE (ngp_usage);
#ifdef  PARALLEL
//      UCON64_USAGE (fpl_usage);
#endif // PARALLEL
      printf ("\n");

#ifdef  SAMPLE
      UCON64_USAGE (sample_usage);
      printf ("\n");
#endif // SAMPLE
  }

#ifdef  PARALLEL
#define PARALLEL_MSG "NOTE: You only need to specify PORT if uCON64 doesn't detect the (right)\n" \
     "      parallel port. If that is the case give a hardware address. For example:\n" \
     "        ucon64 " OPTION_LONG_S "xswc \"rom.swc\" 0x378\n" \
     "      In order to connect a copier to a PC's parallel port you need a standard\n" \
     "      bidirectional parallel cable\n" \
     "\n"
#else
#define PARALLEL_MSG ""
#endif

#ifdef  __MSDOS__
#define MORE_MSG "     %s " OPTION_LONG_S "help|more (to see everything in more)\n"
#else
#define MORE_MSG "     %s " OPTION_LONG_S "help|less (to see everything in less)\n" // less is more ;-)
#endif

  printf (
     "DATabase: %d known ROMs (DAT files: %s)\n"
     "\n"
     PARALLEL_MSG
     "TIP: %s " OPTION_LONG_S "help " OPTION_LONG_S "snes (would show only SNES related help)\n"
     MORE_MSG
     "     give the force recognition switch a try if something went wrong\n"
     "\n"
     "Report problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net\n"
     "\n",
     ucon64_dat_total_entries (UCON64_UNKNOWN),
     ucon64.configdir,
     argv[0], argv[0]);
}


/*
_ __ ________________________________________________________________ __ _
                                                      ___
    .,,,,     .---._ Oo  .::::. .::::. :::   :::    __\__\
    ( oo)__   (¯oo) /..\ ::  :: ::  :: :::   :::    \ / Oo\o  (\(\
   /\_  \__) /\_  \/\_,/ ::  .. ::..:: ::'   ::'    _\\`--_/ o/oO \
   \__)_/   _\__)_/_/    :::::: :::::: ::....::.... \_ \  \  \.--'/
   /_/_/    \ /_/_//     `::::' ::  :: `:::::`:::::: /_/__/   /¯\ \___
 _(__)_)_,   (__)_/  .::::.                      ;::  |_|_    \_/_/\_/\
  o    o      (__)) ,:' `::::::::::::::::::::::::::' (__)_)___(_(_)  ¯¯
     ________  ________  _____ _____________________/   __/_  __/_________
    _\___   /__\___   /_/____/_\    __________     /    ___/  ______     /
   /    /    /    /    /     /  \      \/    /    /     /     /    /    /
  /    /    /         /     /          /    _____/_    /_    /_   _____/_
 /____/    /_________/     /·aBn/fAZ!/nB·_________/_____/_____/_________/
- -- /_____\--------/_____/------------------------------------------ -- -
4 Nodes USRobotics & Isdn Power     All Releases Since Day 0 Are Available
 SNES/Sega/Game Boy/GameGear/Ultra 64/PSX/Jaguar/Saturn/Engine/Lynx/NeoGeo
- -- ---------------------------------------------------------------- -- -
*/

/*
Nintendo Game Cube/Panasonic Gamecube Q
2001/2002 Nintendo http://www.nintendo.com
gc
Sega System 16(A/B)/Sega System 18/dual 68000
1987/19XX/19XX SEGA http://www.sega.com
s16
Atari VCS 2600(aka Stella)/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr
1977/1982/1984/1986 Atari
ata
ColecoVision
1982
coleco
Nintendo Virtual Boy
19XX Nintendo http://www.nintendo.com
vboy
Vectrex
1982
vec
Intellivision
1979 Mattel
intelli
GP32 Game System
2002 Gamepark http://www.gamepark.co.kr
gp32
Playstation 2
2000 Sony http://www.playstation.com
ps2
XBox
2001 Microsoft http://www.xbox.com
xbox
Saturn
1994 SEGA http://www.sega.com
sat
Real3DO
1993
3do
CD32
1993 Commodore
cd32
CD-i
1991
cdi

Vectrex (1982)
Colecovision (1982)
Interton VC4000 (~1980)
Intellivision (1979)
G7400+/Odyssey² (1978)
Channel F (1976)
Odyssey (Ralph Baer/USA/1972)
Virtual Boy
Real 3DO 1993 Panasonic/Goldstar/Philips?
Game.com ? Tiger
CD-i (1991) 1991
Vectrex 1982
Colecovision 1982
Interton VC4000 ~1980
Intellivision 1979
G7400+/Odyssey² 1978
Channel F 1976
Odyssey 1972 Ralph Baer

X-Box
Game Cube
Indrema
Nuon
GB Advance
Playstation 2
Dreamcast
Nintendo 64
Playstation
Virtual Boy
Saturn
Sega 32X
Jaguar
3DO
Sega CD
Philips CDI
Super Nintendo
Neo·Geo
Game Gear
Lynx
GameBoy
Turbo Grafx 16
Genesis
XE System
Master System
Atari 7800
Nintendo
Commodore 64
Coleco Vision
Atari 5200
Arcadia
Vectrex
Microvision
Adv. Vision
RDI Halcyon
Intellivision
Odyssey 2
Astrocade
Home Arcade
Atari 2600
RCA Studio 2
FC Channel F
Telstar
Atari Pong
PONG
Odyssey

gametz.com
gameaxe.com
sys2064.com
logiqx.com
romcenter.com
emuchina.net

Bandai announced that a new version of the system, the SwanCrystal, will debut in Japan this July.
*/
