/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

written by 1999 - 2003 NoisyB (noisyb@gmx.net)
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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#ifdef  __linux__
#include <sys/io.h>
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
static int ucon64_process_rom (const char *fname, int console, int show_nfo);
static int ucon64_execute_options (void);
static void ucon64_rom_nfo (const st_rominfo_t *rominfo);
static int ucon64_discmage (void);

st_ucon64_t ucon64;
static st_rominfo_t rom;
static st_ucon64_dat_t dat;
st_ucon64_dat_t *ucon64_dat = NULL;
dm_image_t *image = NULL;
static const char *ucon64_title = "uCON64 " UCON64_VERSION_S " " CURRENT_OS_S " 1999-2003";
static int ucon64_fsize = 0, ucon64_option = 0;
const struct option options[] =   {
    {"1991", 0, 0, UCON64_1991},
    {"3do", 0, 0, UCON64_3DO},
    {"?", 0, 0, UCON64_HELP},
    {"a", 0, 0, UCON64_A},
    {"ata", 0, 0, UCON64_ATA},
    {"b", 0, 0, UCON64_B},
    {"b0", 1, 0, UCON64_B0},
    {"b1", 1, 0, UCON64_B1},
    {"bios", 1, 0, UCON64_BIOS},
    {"bot", 1, 0, UCON64_BOT},
    {"c", 1, 0, UCON64_C},
//    {"cd32", 0, 0, UCON64_CD32},
//    {"cdi", 0, 0, UCON64_CDI},
    {"cdirip", 0, 0, UCON64_CDIRIP},
    {"chk", 0, 0, UCON64_CHK},
    {"col", 1, 0, UCON64_COL},
    {"coleco", 0, 0, UCON64_COLECO},
    {"crc", 0, 0, UCON64_CRC},
    {"crchd", 0, 0, UCON64_CRCHD},
    {"crp", 1, 0, UCON64_CRP},
    {"cs", 1, 0, UCON64_CS},
    {"db", 0, 0, UCON64_DB},
    {"dbs", 1, 0, UCON64_DBS},
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
    {"find", 1, 0, UCON64_FIND},
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
    {"idppf", 1, 0, UCON64_IDPPF},
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
    {"lsram", 1, 0, UCON64_LSRAM},
    {"lsv", 0, 0, UCON64_LSV},
    {"lynx", 0, 0, UCON64_LYNX},
    {"lyx", 0, 0, UCON64_LYX},
    {"mgd", 0, 0, UCON64_MGD},
//    {"mgh", 0, 0, UCON64_MGH},
    {"mka", 1, 0, UCON64_MKA},
    {"mkcue", 0, 0, UCON64_MKCUE},
    {"mki", 1, 0, UCON64_MKI},
    {"mkppf", 1, 0, UCON64_MKPPF},
    {"mksheet", 0, 0, UCON64_MKSHEET},
    {"mktoc", 0, 0, UCON64_MKTOC},
    {"multi", 1, 0, UCON64_MULTI},
    {"mvs", 0, 0, UCON64_MVS},
    {"n", 1, 0, UCON64_N},
    {"n2", 1, 0, UCON64_N2},
    {"n2gb", 1, 0, UCON64_N2GB},
    {"n64", 0, 0, UCON64_N64},
    {"na", 0, 0, UCON64_NA},
    {"nbak", 0, 0, UCON64_NBAK},
    {"ncol", 0, 0, UCON64_NCOL},
    {"nrgrip", 0, 0, UCON64_NRGRIP},
    {"nes", 0, 0, UCON64_NES},
    {"ng", 0, 0, UCON64_NG},
    {"ngp", 0, 0, UCON64_NGP},
    {"nppf", 1, 0, UCON64_NPPF},
    {"nrot", 0, 0, UCON64_NROT},
    {"ns", 0, 0, UCON64_NS},
    {"o", 1, 0, UCON64_O},
    {"p", 0, 0, UCON64_P},
    {"pad", 0, 0, UCON64_PAD},
    {"padhd", 0, 0, UCON64_PADHD},
    {"padn", 1, 0, UCON64_PADN},
    {"pasofami", 0, 0, UCON64_PASOFAMI},
    {"patch", 1, 0, UCON64_PATCH},
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
    {"sam", 1, 0, UCON64_SAM},
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
    {"strip", 1, 0, UCON64_STRIP},
    {"swan", 0, 0, UCON64_SWAN},
    {"swap", 0, 0, UCON64_SWAP},
    {"swc", 0, 0, UCON64_SWC},
    {"swcs", 0, 0, UCON64_SWCS},
#ifdef  DEBUG
    {"test", 0, 0, UCON64_TEST},
#endif // DEBUG
    {"ufos", 0, 0, UCON64_UFOS},
    {"unif", 0, 0, UCON64_UNIF},
    {"usms", 1, 0, UCON64_USMS},
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
    {"dumpinfo", 1, 0, UCON64_DUMPINFO},
    {"version", 0, 0, UCON64_VER},
    {0, 0, 0, 0}
  };

#ifdef  DLOPEN
void *libdm;

#define dm_get_version dm_get_version_ptr
uint32_t (*dm_get_version_ptr) (void) = NULL;

#define dm_get_usage dm_get_usage_ptr
st_dm_usage_t *(*dm_get_usage_ptr) (void) = NULL;

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
  int x = 0, c = 0, console, show_nfo, rom_index;
  char buf[MAXBUFSIZE], *ptr = NULL;
  struct stat fstate;

  printf ("%s\n"
    "Uses code from various people. See 'developers.html' for more!\n"
    "This may be freely redistributed under the terms of the GNU Public License\n\n",
    ucon64_title);

  if (atexit (ucon64_exit) == -1)
    {
      fprintf (stderr, "ERROR: Could not register function with atexit()\n");
      exit (1);
    }

  memset (&ucon64, 0, sizeof (st_ucon64_t));

#ifdef  __unix__
  // We need to modify the umask, because the configfile is made while we are
  //  still running in root mode. Maybe 0 is even better (in case root did
  //  `chmod +s').
  umask (002);
#endif

  ucon64.argc = argc;
  ucon64.argv = argv;                           // must be set prior to calling
                                                //  ucon64_discmage()
  ucon64_configfile ();
  ucon64.discmage_enabled = ucon64_discmage ();

  ucon64.show_nfo = TRUE;

#ifdef  ANSI_COLOR
  ucon64.ansi_color = get_property_int (ucon64.configfile, "ansi_color", '=');
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

  ucon64.good_enabled = 0; // --good
  ucon64.fname_arch[0] = 0;

  ucon64.rom =
  ucon64.file =
  ucon64.mapr =
  ucon64.comment = "";

#if 0
  // It's not necessary to fill ucon64.output_path with the absolute path to
  //  the current working directory. If the path to the cwd is long setting
  //  ucon64.output_path makes the output confusing.
  getcwd (ucon64.output_path, FILENAME_MAX);    // default output path
  if (OFFSET (ucon64.output_path, strlen (ucon64.output_path) - 1) != FILE_SEPARATOR)
    strcat (ucon64.output_path, FILE_SEPARATOR_S);
#endif

  strcpy (ucon64.configdir, get_property (ucon64.configfile, "configdir", buf, ""));
#ifdef  __CYGWIN__
  strcpy (ucon64.configdir, cygwin_fix (ucon64.configdir));
#endif
  strcpy (buf, ucon64.configdir);
  realpath2 (buf, ucon64.configdir);

  ucon64.dat_enabled = 0;
  if (!access (ucon64.configdir, R_OK | W_OK | X_OK))
    if (!stat (ucon64.configdir, &fstate))
      if (S_ISDIR (fstate.st_mode))
        ucon64.dat_enabled = 1;

//  if (!access (ucon64.configfile, F_OK))
//    fprintf (stdout, ucon64_msg[READ_CONFIG_FILE], ucon64.configfile);

  // Use "0" to force probing if the config file doesn't contain a parport line
  sscanf (get_property (ucon64.configfile, "parport", buf, "0"), "%x", &ucon64.parport);

  ucon64.backup = get_property_int (ucon64.configfile, "backups", '=');

  if (argc < 2)
    {
      ucon64_usage (argc, argv);
      return 0;
    }

#ifdef  ANSI_COLOR
  if (ucon64.ansi_color)
    ucon64.ansi_color = ansi_init ();
#endif

  if (ucon64.dat_enabled)
    ucon64_dat_indexer ();                      // update cache (index) files if necessary

  ucon64_flush (&rom);

  ucon64.show_nfo = TRUE;
  while ((c = getopt_long_only (argc, argv, "", options, NULL)) != -1)
    {
      for (x = 0; ucon64_wf[x].option != 0 && ucon64_wf[x].console != 0; x++)
        if (ucon64_wf[x].option == c)
          {
            // a specific console takes precedence over unknown
            if (ucon64_wf[x].console != UCON64_UNKNOWN)
              ucon64.console = ucon64_wf[x].console;
            // FALSE takes precedence over TRUE
            if ((ucon64_wf[x].flags & WF_SHOW_NFO) == 0)
              ucon64.show_nfo = FALSE;
            break;
          }

#include "switches.c"
    }

  if (!strlen (ucon64.rom) && optind < argc)
    ucon64.rom = argv[optind];

  rom_index = optind;                           // save index of first file

  // no ROM or FILE argument (for example -dbv)
  if (rom_index == argc)
    ucon64_execute_options ();

  console = ucon64.console;
  show_nfo = ucon64.show_nfo;
  for (; rom_index < argc; rom_index++)
    {
#ifdef  HAVE_ZLIB_H
      int n_entries;
#endif

#if 0 // TODO: detect nonsense arguments. Using access() requires users to
      //       always specify -rom or -port, which is annoying.
      if (access (argv[rom_index], F_OK) != 0)
        continue;
#endif

#ifdef  HAVE_ZLIB_H
      n_entries = unzip_get_number_entries (argv[rom_index]);
      if (n_entries != -1)                      // it's a zip file
        {
          int stop = 0;

          for (unzip_current_file_nr = 0; unzip_current_file_nr < n_entries;
               unzip_current_file_nr++)
            {
              ucon64_fname_arch (argv[rom_index]);
              if ((stop = ucon64_process_rom (argv[rom_index], console, show_nfo)))
                break;
            }
          unzip_current_file_nr = 0;
          ucon64.fname_arch[0] = 0;
          if (stop)
            break;
        }
      else
#endif
      if (ucon64_process_rom (argv[rom_index], console, show_nfo))
        break;
    }

  return 0;
}


int
ucon64_discmage (void)
{
  uint32_t version;
#ifdef  DLOPEN
  char buf[MAXBUFSIZE];

  strcpy (ucon64.discmage_path, get_property (ucon64.configfile, "discmage_path", buf, ""));
#ifdef  __CYGWIN__
  strcpy (ucon64.discmage_path, cygwin_fix (ucon64.discmage_path));
#endif
  strcpy (buf, ucon64.discmage_path);
  realpath2 (buf, ucon64.discmage_path);

  // if ucon64.discmage_path points to an existing file then load it
  if (!access (ucon64.discmage_path, F_OK))
    {
      libdm = open_module (ucon64.discmage_path);

      dm_get_version = get_symbol (libdm, "dm_get_version");
      version = dm_get_version ();
      if (version < LIB_VERSION (0, 0, 2))
        {
          fprintf (stderr,
                   "ERROR: Your libdiscmage is too old (%d.%d.%d)\n"
                   "       You need at least version %d.%d.%d for this version of uCON64\n\n",
                   version >> 16, (version >> 8) & 0xff, version & 0xff,
                   0, 0, 2);
          return 0;
        }
      else
        {
          dm_get_usage = get_symbol (libdm, "dm_get_usage");

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

          return 1;
        }
    }
  else
    return 0;
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
    char *p = dirname (ucon64.argv[0]);

    sprintf (djimport_path, "%s"FILE_SEPARATOR_S"%s", p, "discmage.dxe");
    free (p);
  }
#endif // DJGPP
  version = dm_get_version ();
  if (version < LIB_VERSION (0, 0, 2))
    {
      fprintf (stderr,
               "ERROR: Your libdiscmage is too old (%d.%d.%d)\n"
               "       You need at least version %d.%d.%d for this version of uCON64\n\n",
               version >> 16, (version >> 8) & 0xff, version & 0xff,
               0, 0, 2);
      return 0;
    }
  else
    return 1;
#endif // #ifdef DLOPEN
}


#ifdef  HAVE_ZLIB_H
void
ucon64_fname_arch (const char *fname)
{
  unzFile file = unzOpen (fname);
  unzip_goto_file (file, unzip_current_file_nr);
  unzGetCurrentFileInfo (file, NULL, ucon64.fname_arch, FILENAME_MAX,
                         NULL, 0, NULL, 0);
  unzClose (file);
}
#endif


int
ucon64_process_rom (const char *fname, int console, int show_nfo)
{
  int special_option;

  ucon64.rom = fname;
  ucon64.console = console;
  ucon64.show_nfo = show_nfo;

  ucon64_init (ucon64.rom, &rom);
  if (ucon64.show_nfo == TRUE)
    ucon64_nfo (&rom);
  ucon64.show_nfo = FALSE;

  special_option = ucon64_execute_options ();

  if (ucon64.show_nfo == TRUE)
    {
      ucon64_init (ucon64.rom, &rom);
      ucon64_nfo (&rom);
    }

  return special_option;
}


int
ucon64_execute_options (void)
// execute all options for a single file
{
  int ucon64_argc, c, result = 0, value = 0, special_option = 0, x = 0;
  unsigned int padded;
  char buf[MAXBUFSIZE], src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  const char *ucon64_argv[128];

  optind = 0;                                   // start with first option
  while ((ucon64_option = c =
            getopt_long_only (ucon64.argc, ucon64.argv, "", options, NULL)) != -1)
    {
#include "options.c"

      /*
        "special" options:
        - -multi (and -xfalmulti) takes more than one file as argument, but
          should be executed only once.
        - stop after sending one ROM to a copier ("multizip")
        - stop after applying a patch so that the patch file won't be
          interpreted as ROM
      */
      for (x = 0; ucon64_wf[x].option != 0; x++)
        if (ucon64_wf[x].option == ucon64_option)
          {
            if (ucon64_wf[x].flags & WF_SPECIAL_OPT)
              special_option = 1;
            break;
          }
    }
  return special_option;
}


st_rominfo_t *
ucon64_flush (st_rominfo_t *rominfo)
{
  memset (rominfo, 0L, sizeof (st_rominfo_t));
  rominfo->data_size = UCON64_UNKNOWN;
  ucon64.file_size = ucon64_fsize;
  // We *have* to reset these in a central place or else in *every* init function
  ucon64.crc32 = ucon64.fcrc32 = 0;
#if 0
  rominfo->maker = rominfo->country = "";
  rominfo->console_usage = rominfo->copier_usage = NULL;
#endif

  // restore the overrides from st_ucon64_t
  if (UCON64_ISSET (ucon64.buheader_len))
    rominfo->buheader_len = ucon64.buheader_len;

  if (UCON64_ISSET (ucon64.snes_hirom))
    rominfo->snes_hirom = ucon64.snes_hirom;

  if (UCON64_ISSET (ucon64.interleaved))
    rominfo->interleaved = ucon64.interleaved;

  return rominfo;
}


static int
unknown_init (st_rominfo_t *rominfo)
{
  ucon64_flush (rominfo);
  return 0;
}


static int
disc_init (st_rominfo_t *rominfo)
{
// TODO: dm_init()
  return -1;
}


int
ucon64_console_probe (st_rominfo_t *rominfo)
{
  typedef struct
    {
      int console;
      int (*init) (st_rominfo_t *);
      uint8_t auto_recognition;
    } st_probe_t;

  int x = 0;
  st_probe_t probe[] =
    {
      {UCON64_GBA, gba_init, 1},
      {UCON64_N64, n64_init, 1},
      {UCON64_GENESIS, genesis_init, 1},
      {UCON64_LYNX, lynx_init, 1},
      {UCON64_GB, gameboy_init, 1},
      {UCON64_SNES, snes_init, 1},
      {UCON64_NES, nes_init, 1},
      {UCON64_NEOGEOPOCKET, ngp_init, 1},
      {UCON64_SWAN, swan_init, 1},
      {UCON64_JAGUAR, jaguar_init, 1},
      {UCON64_SMS, sms_init, 0},
      {UCON64_NEOGEO, neogeo_init, 0},
      {UCON64_PCE, pcengine_init, 0},
      {UCON64_WONDERSWAN, swan_init, 0},
      {UCON64_DC, dc_init, 0},
      {UCON64_PSX, psx_init, 0},
      {UCON64_SATURN, disc_init, 0},
      {UCON64_CDI, disc_init, 0},
      {UCON64_CD32, disc_init, 0},
      {UCON64_REAL3DO, disc_init, 0},
      {UCON64_PS2, disc_init, 0},
      {UCON64_XBOX, disc_init, 0},
#if 0
      {UCON64_GAMECUBE, NULL, 0},
      {UCON64_GP32, NULL, 0},
      {UCON64_COLECO, NULL, 0},
      {UCON64_INTELLI, NULL, 0},
      {UCON64_SYSTEM16, NULL, 0},
      {UCON64_ATARI, NULL, 0},
      {UCON64_VECTREX, NULL, 0},
      {UCON64_VIRTUALBOY, NULL, 0},
#endif
      {UCON64_UNKNOWN, unknown_init, 0},
      {0, NULL}
    };

  if (ucon64.console != UCON64_UNKNOWN) //  force recognition option was used
    {
      for (x = 0; probe[x].console != 0; x++)
        if (probe[x].console == ucon64.console)
          {
            ucon64_flush (rominfo);
            probe[x].init (rominfo);
            break;
          }
    }
  else // give auto_recognition a try
    {
      if (UCON64_TYPE_ISROM (ucon64.type))      // TODO: still needed?
        for (x = 0; probe[x].console != 0; x++)
          if (probe[x].auto_recognition)
            {
              ucon64_flush (rominfo);
              if (!probe[x].init (rominfo))
                {
                  ucon64.console = probe[x].console;
                  break;
                }
            }
    }

  return (ucon64.console == UCON64_UNKNOWN) ? (-1) : 0;
}


int
ucon64_init (const char *romfile, st_rominfo_t *rominfo)
{
  int result = -1;
  struct stat fstate;

  if (access (romfile, F_OK | R_OK) == -1)
    return -1;
  if (stat (romfile, &fstate) == -1)
    return -1;
  if (S_ISREG (fstate.st_mode) != TRUE)
    return -1;

  ucon64_fsize = q_fsize (ucon64.rom);          // save size in ucon64_fsize
  ucon64.file_size = ucon64_fsize;

  // What media type? Do not calculate CRC32 checksum and stuff for DISC images (speed!)
  ucon64.type = (ucon64.file_size <= MAXROMSIZE) ? UCON64_TYPE_ROM : UCON64_TYPE_DISC;

  if (ucon64.discmage_enabled)
    {
#if 0
      image = dm_open (ucon64.rom);
      ucon64.type = (image ? UCON64_TYPE_DISC : UCON64_TYPE_ROM);
      dm_close (image);
#endif
      image = NULL;
    }

  if (UCON64_TYPE_ISROM (ucon64.type))
    {
      ucon64_flush (rominfo); // clear rominfo
      result = ucon64_console_probe (rominfo);

      /*
        Calculating the CRC32 checksum for the ROM data of a UNIF file (NES)
        shouldn't be done with q_fcrc32(). nes_init() uses mem_crc32().
        The CRC32 checksum is used to search in the DAT files, but at the time
        of this writing (Februari the 7th 2003) all DAT files contain checksums
        of files in only one format. This matters for SNES and Genesis ROMs in
        interleaved format and Nintendo 64 ROMs in non-interleaved format. The
        corresponding initialization functions calculate the CRC32 checksum of
        the data in the format of which the checksum is stored in the DAT
        files. For these "problematic" files, their "real" checksum is stored
        in ucon64.fcrc32.
      */
      if (ucon64.crc32 == 0)
//        if (ucon64.do_not_calc_crc != UCON64_UNKNOWN)
          if (ucon64.crc_big_files != UCON64_UNKNOWN ||
              UCON64_TYPE_ISROM (ucon64.type))
        ucon64.crc32 = q_fcrc32 (romfile, rominfo->buheader_len);

      if (ucon64.dat_enabled)
        {
          memset (&dat, 0, sizeof (st_ucon64_dat_t));
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
            // Use ucon64_dat instead of ucon64.dat_enabled in case the index
            //  file could not be created/opened -> no segmentation fault
            if (ucon64_dat && ucon64.console == UCON64_NES &&
                (nes_get_file_type () == UNIF ||
                 nes_get_file_type () == INES ||
                 nes_get_file_type () == PASOFAMI ||
                 nes_get_file_type () == FDS))
              strcpy ((char *) rominfo->name, NULL_TO_EMPTY (ucon64_dat->name));

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
  printf ("%s\n", ucon64.rom);
  if (ucon64.fname_arch[0])
    printf ("  (%s)\n", ucon64.fname_arch);
  fputc ('\n', stdout);

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

  if (ucon64.fcrc32)                    // SNES & Genesis interleaved/N64 non-interleaved
    printf ("Checksum (CRC32): 0x%08x\n", ucon64.fcrc32);
  else if (ucon64.crc32)
    printf ("Checksum (CRC32): 0x%08x\n", ucon64.crc32);

  if (ucon64.dat_enabled)
    ucon64_dat_nfo (ucon64_dat, 1);

  printf ("\n");

  return 0;
}


void
ucon64_rom_nfo (const st_rominfo_t *rominfo)
{
  unsigned int padded = ucon64_testpad (ucon64.rom, (st_rominfo_t *) rominfo),
    intro = ((ucon64.file_size - rominfo->buheader_len) > MBIT) ?
      ((ucon64.file_size - rominfo->buheader_len) % MBIT) : 0;
  int x, split = (UCON64_ISSET (ucon64.split)) ? ucon64.split :
           ucon64_testsplit (ucon64.rom);
  char buf[MAXBUFSIZE];

  // backup unit header
  if (rominfo->buheader && rominfo->buheader_len && rominfo->buheader_len != SWC_HEADER_LEN)
    {
      mem_hexdump (rominfo->buheader, rominfo->buheader_len, rominfo->buheader_start);
      printf ("\n");
    }

  // backup unit type?
  if (rominfo->copier_usage != NULL)
    {
      strcpy (buf, rominfo->copier_usage[0].desc);
      printf ("%s\n", to_func (buf, strlen (buf), toprint2));

#if 0
      if (rominfo->copier_usage[1].desc)
        {
          strcpy (buf, rominfo->copier_usage[1].desc);
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

  // console type
  if (rominfo->console_usage != NULL)
    {
      strcpy (buf, rominfo->console_usage[0].desc);
      printf ("%s\n", to_func (buf, strlen (buf), toprint2));

#if 0
      if (rominfo->console_usage[1].desc)
        {
          strcpy (buf, rominfo->console_usage[1].desc);
          printf ("  %s\n", to_func (buf, strlen (buf), toprint2));
        }
#endif
    }

  // name, maker, country and size
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
        printf ("NOTE: To get the correct checksum the ROM parts must be joined\n");
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
#ifdef  ANSI_COLOR
      printf (buf,
        ucon64.ansi_color ?
          ((rominfo->current_internal_crc == rominfo->internal_crc) ?
            "\x1b[01;32mOk\x1b[0m" : "\x1b[01;31mBad\x1b[0m")
          :
          ((rominfo->current_internal_crc == rominfo->internal_crc) ? "Ok" : "Bad"),
        rominfo->current_internal_crc,
        (rominfo->current_internal_crc == rominfo->internal_crc) ? "=" : "!",
        rominfo->internal_crc);
#else
      printf (buf,
        (rominfo->current_internal_crc == rominfo->internal_crc) ? "Ok" : "Bad",
        rominfo->current_internal_crc,
        (rominfo->current_internal_crc == rominfo->internal_crc) ? "=" : "!",
        rominfo->internal_crc);
#endif

      if (rominfo->internal_crc2[0])
        {
          strcpy (buf, rominfo->internal_crc2);
          printf ("%s\n", to_func (buf, strlen (buf), toprint2));
        }
    }

  fflush (stdout);
}


static void
ucon64_render_usage (const st_usage_t *usage)
{
// TODO speed up
  int x, pos = 0;
  char buf[MAXBUFSIZE];

  for (x = 0; usage[x].option_s || usage[x].desc; x++)
    {
      if (!usage[x].option_s) // title
        {
          if (!x) // do not show date, manufacturer, etc..
            printf ("%s\n", usage[x].desc);
        }
      else  // options
        {
          if (usage[x].option_s)
            {
              int len = strlen (usage[x].option_s);

// adjust tabs for OPTION_S and OPTION_LONG_S here not in the OPTION_S definition
              if (MAX (1, MIN (len, strcspn (usage[x].option_s, "= "))) == 1)
                printf ("   %s", OPTION_S);
              else
                printf ("  %s", OPTION_LONG_S);

              sprintf (buf, "%s                          ", usage[x].option_s);
              buf[len < 12 ? 12 : len + 1] = 0;

              printf ("%s", buf);
            }

          if (usage[x].desc)
            {
#if 1
              for (pos = 0; usage[x].desc[pos]; pos++)
                {
                  printf ("%c", usage[x].desc[pos]); // TODO speed this up

                  if (usage[x].desc[pos] == '\n')
                    printf ("                  ");
                }
#else
              for (pos = 0; ;)
                {
                  strncpy (buf, usage[x].desc[pos], strcspn (usage[x].desc[pos], '\n') + 1);
                  printf ("%s                  ", buf);
                }
#endif                
              printf ("\n");
            }
        }
    }
}


typedef struct
{
  int console;
  const st_usage_t *usage[6];
} st_usage_array_t;


void
ucon64_usage (int argc, char *argv[])
{
  int x = 0, c = 0, single = 0;
  st_usage_array_t usage_array[] = {
    {UCON64_DC, {dc_usage, 0, 0, 0, 0, 0}},
    {UCON64_PSX, {psx_usage,
#ifdef  PARALLEL
      dex_usage,
#else
      0,
#endif // PARALLEL
      0, 0, 0, 0}},
    {UCON64_GBA, {gba_usage,
#ifdef  PARALLEL
      fal_usage,
#else
      0,
#endif // PARALLEL
      0, 0, 0, 0}},
    {UCON64_N64, {n64_usage,
#ifdef  PARALLEL
      doctor64_usage,
      doctor64jr_usage,
//      cd64_usage,
      dex_usage,
#else
      0, 0, 0, 0,
#endif // PARALLEL
      0}},
    {UCON64_SNES, {snes_usage,
#ifdef  PARALLEL
      swc_usage,
      gd_usage,
//      fig_usage,
//      mgd_usage,
#else
      0, 0,
#endif // PARALLEL
      0, 0, 0}},
    {UCON64_NG, {neogeo_usage, 0, 0, 0, 0, 0}},
    {UCON64_GEN, {genesis_usage,
#ifdef  PARALLEL
        smd_usage,
//        mgd_usage,
#else
      0,
#endif // PARALLEL
      0, 0, 0, 0}},
    {UCON64_GB, {gameboy_usage,
#ifdef  PARALLEL
        gbx_usage,
        mccl_usage,
#else
      0, 0,
#endif // PARALLEL
      0, 0, 0}},  
    {UCON64_LYNX, {lynx_usage,
#ifdef  PARALLEL
      lynxit_usage,
#else
      0,
#endif // PARALLEL
      0, 0, 0, 0}},
    {UCON64_PCE, {pcengine_usage,
#ifdef  PARALLEL
//        mgd_usage,
#endif // PARALLEL
      0, 0, 0, 0}},
    {UCON64_SMS, {sms_usage,
#ifdef  PARALLEL
      smd_usage,
#else
      0,
#endif // PARALLEL
      0, 0, 0, 0}},
    {UCON64_NES, {nes_usage, 0, 0, 0, 0, 0}},
    {UCON64_SWAN, {swan_usage, 0, 0, 0, 0, 0}},
    {UCON64_JAG, {jaguar_usage, 0, 0, 0, 0, 0}},
    {UCON64_NGP, {ngp_usage,
#ifdef  PARALLEL
//        fpl_usage,
#endif // PARALLEL
      0, 0, 0, 0, 0}},
#if 0
    {UCON64_G, {nes_usage, 0, 0, 0, 0, 0}},
    {UCON64_S16, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_ATA, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_COLECO, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_VBOY, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_VEC, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_INTELLI, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_PS2, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_SAT, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_3DO, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_CD32, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_CDI, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_XBOX, {unknown_usage, 0, 0, 0, 0, 0}},
    {UCON64_GP32, {unknown_usage, 0, 0, 0, 0, 0}},
#endif
    {0, {0, 0, 0, 0, 0, 0}}
  };
  char *name_exe = basename (argv[0]), *name_discmage;

  printf (
    "Usage: %s [OPTION(S)]... [[" OPTION_LONG_S "rom=]ROM(S)]... " /* [-o=OUTPUT_PATH] */ "\n\n", name_exe);

  ucon64_render_usage (ucon64_options_usage);

  printf ("\n");

  ucon64_render_usage (ucon64_padding_usage);

  printf ("\n");

//  if (ucon64.dat_enabled)
    ucon64_render_usage (ucon64_dat_usage);

  printf ("\n");

  ucon64_render_usage (ucon64_patching_usage);

  ucon64_render_usage (bsl_usage);
  ucon64_render_usage (ips_usage);
  ucon64_render_usage (aps_usage);
  ucon64_render_usage (pal4u_usage);
  ucon64_render_usage (ppf_usage);
  ucon64_render_usage (xps_usage);
  ucon64_render_usage (gg_usage);

  printf ("                  supported are:\n"
    "                  %s,\n"
    "                  %s,\n"
//    "                  %s,\n"
    "                  %s,\n"
    "                  %s\n\n",
    gameboy_usage[0].desc, sms_usage[0].desc,
//    genesis_usage[0].desc,
    nes_usage[0].desc, snes_usage[0].desc);

  name_discmage = 
#ifdef  DLOPEN
    ucon64.discmage_path;
#else
#if     defined __MSDOS__
    "discmage.dxe";
#elif   defined __CYGWIN__
    "discmage.dll";
#elif   defined __unix__ || defined __BEOS__
    "libdiscmage.so";
#else
    "unknown";
#endif
#endif

  printf ("All DISC-based consoles (using libdiscmage)\n");
  if (!ucon64.discmage_enabled)
    printf ("                %s\n"
            "                  not found or too old, support disabled\n", name_discmage);
  else
    ucon64_render_usage ((const st_usage_t *) dm_get_usage ());
  printf ("\n");

  optind = 0;
  single = 0;

  while ((c = getopt_long_only (argc, argv, "", options, NULL)) != -1)
    {
      for (x = 0; usage_array[x].console != 0; x++)
        if (usage_array[x].console == c)
          {
            int y = 0;
            for (; usage_array[x].usage[y]; y++)
              ucon64_render_usage (usage_array[x].usage[y]);

            single = 1; // we show only the usage for the specified console(s)

            printf ("\n");
          }
    }

  if (!single)
    {
      for (x = 0; usage_array[x].console != 0; x++)
        {
          int y = 0;
          for (; usage_array[x].usage[y]; y++)
            ucon64_render_usage (usage_array[x].usage[y]);

          printf ("\n");
        }
  }

#undef  PARALLEL_MSG
#ifdef  PARALLEL
#define PARALLEL_MSG "NOTE: You only need to specify PORT if uCON64 doesn't detect the (right)\n" \
     "      parallel port. If that is the case give a hardware address. For example:\n" \
     "        ucon64 " OPTION_LONG_S "xswc \"rom.swc\" " OPTION_LONG_S "port=0x378\n" \
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
     "     Give the force recognition switch a try if something went wrong\n"
     "\n"
     "Please report any problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net\n"
     "\n",
     ucon64_dat_total_entries (UCON64_UNKNOWN),
     ucon64.configdir,
     name_exe, name_exe);
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
