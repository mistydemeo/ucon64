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
#ifdef  _WIN32
#include <windows.h>
#endif

#ifdef  DEBUG
#ifdef  __GNUC__
#warning DEBUG active
#else
#pragma message ("DEBUG active")
#endif
#endif
#include "misc.h"
#include "getopt.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "ucon64_lib.h"
#include "ucon64_opts.h"
#include "console/console.h"
#include "patch/patch.h"
#include "backup/backup.h"


static void ucon64_exit (void);
static int ucon64_execute_options (void);
static void ucon64_rom_nfo (const st_rominfo_t *rominfo);
static st_rominfo_t *ucon64_probe (st_rominfo_t *rominfo);
static int ucon64_rom_handling (void);
static void ucon64_render_usage (const st_usage_t *usage);
static int ucon64_process_rom (char *fname);


st_ucon64_t ucon64;                             // containes ptr to image, dat and rominfo

static const char *ucon64_title = "uCON64 " UCON64_VERSION_S " " CURRENT_OS_S " 1999-2003";

typedef struct
{
  int val; // option
//    const
  char *optarg; // option argument
} st_args_t;

static st_args_t arg[UCON64_MAX_ARGS];

const struct option options[] = {
    {"1991", 0, 0, UCON64_1991},
    {"3do", 0, 0, UCON64_3DO},
    {"83", 0, 0, UCON64_83},
    {"?", 0, 0, UCON64_HELP},
    {"a", 0, 0, UCON64_A},
    {"ata", 0, 0, UCON64_ATA},
    {"b", 0, 0, UCON64_B},
    {"b0", 1, 0, UCON64_B0},
    {"b1", 1, 0, UCON64_B1},
    {"bat", 0, 0, UCON64_BAT},
    {"bin2iso", 1, 0, UCON64_BIN2ISO},
    {"bios", 1, 0, UCON64_BIOS},
    {"bot", 1, 0, UCON64_BOT},
    {"bs", 0, 0, UCON64_BS},
    {"c", 1, 0, UCON64_C},
//    {"cd32", 0, 0, UCON64_CD32},
//    {"cdi", 0, 0, UCON64_CDI},
    {"chk", 0, 0, UCON64_CHK},
    {"cmnt", 1, 0, UCON64_CMNT},                // will be active only if UNIF_REVISION > 7
    {"col", 1, 0, UCON64_COL},
    {"coleco", 0, 0, UCON64_COLECO},
    {"crc", 0, 0, UCON64_CRC},
    {"crchd", 0, 0, UCON64_CRCHD},
    {"crp", 1, 0, UCON64_CRP},
    {"cs", 1, 0, UCON64_CS},
    {"ctrl", 1, 0, UCON64_CTRL},
    {"ctrl2", 1, 0, UCON64_CTRL2},
    {"db", 0, 0, UCON64_DB},
    {"dbs", 1, 0, UCON64_DBS},
    {"dbuh", 0, 0, UCON64_DBUH},
    {"dbv", 0, 0, UCON64_DBV},
    {"dc", 0, 0, UCON64_DC},
    {"dint", 0, 0, UCON64_DINT},
    {"disc", 0, 0, UCON64_DISC},
    {"dumpinfo", 1, 0, UCON64_DUMPINFO},
    {"e", 0, 0, UCON64_E},
    {"f", 0, 0, UCON64_F},
    {"fds", 0, 0, UCON64_FDS},
    {"fdsl", 0, 0, UCON64_FDSL},
    {"ffe", 0, 0, UCON64_FFE},
    {"fig", 0, 0, UCON64_FIG},
    {"figs", 0, 0, UCON64_FIGS},
    {"file", 1, 0, UCON64_FILE},
    {"find", 1, 0, UCON64_FIND},
    {"force63", 0, 0, UCON64_FORCE63},
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
//    {"good", 0, 0, UCON64_GOOD},
    {"h", 0, 0, UCON64_HELP},
    {"hd", 0, 0, UCON64_HD},
    {"hdn", 1, 0, UCON64_HDN},
    {"help", 0, 0, UCON64_HELP},
    {"hex", 0, 0, UCON64_HEX},
    {"hi", 0, 0, UCON64_HI},
    {"i", 0, 0, UCON64_I},
    {"idppf", 1, 0, UCON64_IDPPF},
    {"ines", 0, 0, UCON64_INES},
    {"ineshd", 0, 0, UCON64_INESHD},
    {"ins", 0, 0, UCON64_INS},
    {"insn", 1, 0, UCON64_INSN},
    {"int", 0, 0, UCON64_INT},
    {"int2", 0, 0, UCON64_INT2},
    {"intelli", 0, 0, UCON64_INTELLI},
//    {"ip", 0, 0, UCON64_IP},
    {"isofix", 1, 0, UCON64_ISOFIX},
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
    {"mapr", 1, 0, UCON64_MAPR},
    {"mgd", 0, 0, UCON64_MGD},
//    {"mgh", 0, 0, UCON64_MGH},
    {"mirr", 1, 0, UCON64_MIRR},
    {"mka", 1, 0, UCON64_MKA},
    {"mkcue", 0, 0, UCON64_MKCUE},
    {"mki", 1, 0, UCON64_MKI},
    {"mkppf", 1, 0, UCON64_MKPPF},
    {"mksheet", 0, 0, UCON64_MKSHEET},
    {"mktoc", 0, 0, UCON64_MKTOC},
    {"multi", 1, 0, UCON64_MULTI},
//    {"mvs", 0, 0, UCON64_MVS},
    {"n", 1, 0, UCON64_N},
    {"n2", 1, 0, UCON64_N2},
    {"n2gb", 1, 0, UCON64_N2GB},
    {"n64", 0, 0, UCON64_N64},
    {"na", 1, 0, UCON64_NA},
    {"nbak", 0, 0, UCON64_NBAK},
    {"nbat", 0, 0, UCON64_NBAT},
    {"nbs", 0, 0, UCON64_NBS},
#ifdef  ANSI_COLOR
    {"ncol", 0, 0, UCON64_NCOL},
#endif
    {"nes", 0, 0, UCON64_NES},
    {"nhd", 0, 0, UCON64_NHD},
    {"nhi", 0, 0, UCON64_NHI},
    {"nint", 0, 0, UCON64_NINT},
    {"ng", 0, 0, UCON64_NG},
    {"ngp", 0, 0, UCON64_NGP},
    {"nppf", 1, 0, UCON64_NPPF},
    {"nrot", 0, 0, UCON64_NROT},
    {"ns", 0, 0, UCON64_NS},
    {"ntsc", 0, 0, UCON64_NTSC},
    {"nvram", 0, 0, UCON64_NVRAM},
    {"o", 1, 0, UCON64_O},
    {"p", 0, 0, UCON64_P},
    {"pad", 0, 0, UCON64_PAD},
    {"padhd", 0, 0, UCON64_PADHD},
    {"padn", 1, 0, UCON64_PADN},
    {"pal", 0, 0, UCON64_PAL},
    {"pasofami", 0, 0, UCON64_PASOFAMI},
    {"patch", 1, 0, UCON64_PATCH},
    {"pce", 0, 0, UCON64_PCE},
    {"poke", 1, 0, UCON64_POKE},
#ifdef  PARALLEL
    {"port", 1, 0, UCON64_PORT},
#endif
    {"ppf", 0, 0, UCON64_PPF},
    {"ps2", 0, 0, UCON64_PS2},
    {"psx", 0, 0, UCON64_PSX},
    {"q", 0, 0, UCON64_Q},
    {"qq", 0, 0, UCON64_QQ},
    {"rename", 0, 0, UCON64_RENAME},
    {"rip", 1, 0, UCON64_RIP},
    {"rr83", 0, 0, UCON64_RR83},
    {"rrom", 0, 0, UCON64_RROM},
    {"rl", 0, 0, UCON64_RL},
    // yes, in reality --rom doesn't take an argument... it's just a dummy to
    //  make the usage easier to understand and might be gone soon...
    {"rom", 0, 0, UCON64_ROM},
    {"rotl", 0, 0, UCON64_ROTL},
    {"rotr", 0, 0, UCON64_ROTR},
    {"ru", 0, 0, UCON64_RU},
    {"s", 0, 0, UCON64_S},
    {"s16", 0, 0, UCON64_S16},
    {"sam", 1, 0, UCON64_SAM},
    {"sat", 0, 0, UCON64_SAT},
    {"scan", 0, 0, UCON64_SCAN},
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
    {"ufos", 0, 0, UCON64_UFOS},
    {"unif", 0, 0, UCON64_UNIF},
    {"usms", 1, 0, UCON64_USMS},
    {"v", 0, 0, UCON64_V},
    {"v64", 0, 0, UCON64_V64},
    {"vboy", 0, 0, UCON64_VBOY},
    {"vec", 0, 0, UCON64_VEC},
    {"version", 0, 0, UCON64_VER},
    {"vram", 0, 0, UCON64_VRAM},
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
    {"xfalm", 0, 0, UCON64_XFALM},
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
    {0, 0, 0, 0}
  };


static st_rominfo_t *
ucon64_rom_flush (st_rominfo_t * rominfo)
{
  if (rominfo)
    memset (rominfo, 0L, sizeof (st_rominfo_t));

  ucon64.rominfo = NULL;
  ucon64.crc32 = ucon64.fcrc32 = 0;             // yes, this belongs here
  rominfo->data_size = UCON64_UNKNOWN;

  return rominfo;
}


static const struct option *
ucon64_get_opt (const int option)
{
  int x = 0;

  for (x = 0; options[x].val != 0; x++)
    if (options[x].val == option)
      return (struct option *) &options[x];

  return NULL;
}


#ifdef  DEBUG
static void
ucon64_runtime_debug (void)
{
  int x = 0, y = 0;
  char buf[MAXBUFSIZE];

  // how many consoles does uCON64 support?
  for (x = 0; ucon64_wf[x].option; x++)
    if (ucon64_wf[x].option == ucon64_wf[x].console)
      y++;

  fprintf (stderr, "DEBUG: %d consoles found\n", y);

  // sanity check at runtime
  // Does ucon64_wf cover all getopt() options and vice versa?
  for (x = 0; options[x].val; x++)
    if (!ucon64_get_wf (options[x].val)) // compare options with workflow
      {
        fprintf (stderr, "DEBUG: Sanity check failed (option \"%s\" in (struct option *) options)\n", options[x].name);
//        exit (1);
      }

  // How many option do we have?
  printf ("DEBUG: Total options: %d\n", x);
  printf ("DEBUG: UCON64_MAX_ARGS == %d, %s \n", UCON64_MAX_ARGS,
    (x < UCON64_MAX_ARGS ? "good" : "\nERROR:   too small; must be larger than options"));

  // the other way
  for (x = 0; ucon64_wf[x].option; x++)
    if (!ucon64_get_opt (ucon64_wf[x].option)) // compare workflow with options
      {
        strcpy (buf, (ucon64_get_opt (ucon64_wf[x].option))->name);
        fprintf (stderr, "DEBUG: Sanity check failed (option \"%s\" in ucon64_wf)\n", buf);
        exit (1);
      }

  // Any duplicates in ucon64_wf?
  for (x = 0; ucon64_wf[x].option; x++)
    {
      for (y = 0; ucon64_wf[y].option; y++)
        if (ucon64_wf[x].option == ucon64_wf[y].option && x != y)
          break;

      if (ucon64_wf[x].option == ucon64_wf[y].option && x != y)
        {
          strcpy (buf, (ucon64_get_opt (ucon64_wf[x].option))->name);
          fprintf (stderr, "DEBUG: Sanity check failed (option \"%s\" in ucon64_wf is a dupe)\n", buf);
//          exit (1);
        }
    }

  // Check for wrong usage assignments in ucon64_wf
  for (x = 0; ucon64_wf[x].option; x++)
    {
      strcpy (buf, (ucon64_get_opt (ucon64_wf[x].option))->name);
      if (ucon64_wf[x].usage)
        {
          const st_usage_t *p = ucon64_wf[x].usage;

          for (y = 0; p[y].option_s || p[y].desc; y++)
            if (p[y].option_s)
              if (!stricmp (buf, p[y].option_s))
                break;

          if (p[y].option_s)
            if (stricmp (buf, p[y].option_s) != 0 || (!p[y].option_s && !p[y].desc))
              {
                fprintf (stderr, "DEBUG: Wrong usage assigned (option \"%s\" in ucon64_wf)\n", buf);
//                exit (1);
              }

            if (!p[y].option_s && !p[y].desc)
              {
                fprintf (stderr, "DEBUG: Wrong usage assigned (option \"%s\" in ucon64_wf)\n", buf);
//                exit (1);
              }
        }
      else
        printf ("DEBUG: No usage assigned (option \"%s\" in ucon64_wf)\n", buf);
    }

  printf ("DEBUG: Sanity check finished\n");
}
#endif  // DEBUG


void
ucon64_exit (void)
{
  if (ucon64.discmage_enabled)
    if (ucon64.image)
      libdm_close (ucon64.image);
  handle_registered_funcs ();
  fflush (stdout);
}


int
main (int argc, char **argv)
{
  int x = 0, rom_index = 0, c = 0;
#if (FILENAME_MAX < MAXBUFSIZE)
  static char buf[MAXBUFSIZE];
#else
  static char buf[FILENAME_MAX];
#endif
  struct stat fstate;

  printf ("%s\n"
    "Uses code from various people. See 'developers.html' for more!\n"
    "This may be freely redistributed under the terms of the GNU Public License\n\n",
    ucon64_title);

#ifdef  DEBUG
  ucon64_runtime_debug ();
#endif

  if (atexit (ucon64_exit) == -1)
    {
      fprintf (stderr, "ERROR: Could not register function with atexit()\n");
      exit (1);
    }

  // flush st_ucon64_t
  memset (&ucon64, 0, sizeof (st_ucon64_t));

  ucon64.rom =
  ucon64.file =
  ucon64.mapr =
  ucon64.comment = "";

  ucon64.flags = WF_DEFAULT;

  ucon64.fname_arch[0] = 0;

  ucon64.argc = argc;
  ucon64.argv = argv;                           // must be set prior to calling
                                                //  ucon64_load_discmage() (for DOS)
#ifdef  __unix__
  // We need to modify the umask, because the configfile is made while we are
  //  still running in root mode. Maybe 0 is even better (in case root did
  //  `chmod +s').
  umask (002);
#endif
  ucon64_configfile ();

#ifdef  ANSI_COLOR
  // ANSI colors?
  ucon64.ansi_color = get_property_int (ucon64.configfile, "ansi_color", '=');
  // the conditional call to ansi_init() has to be done *after* the check for
  //  the switch -ncol
#endif

  // parallel port?
  // Use "0" to force probing if the config file doesn't contain a parport line
  sscanf (get_property (ucon64.configfile, "parport", buf, "0"), "%x", &ucon64.parport);

  // make backups?
  ucon64.backup = get_property_int (ucon64.configfile, "backups", '=');

  // $HOME/.ucon64/ ?
  strcpy (ucon64.configdir, get_property (ucon64.configfile, "configdir", buf, ""));
#ifdef  __CYGWIN__
  strcpy (ucon64.configdir, fix_character_set (ucon64.configdir));
#endif
  strcpy (buf, ucon64.configdir);
  realpath2 (buf, ucon64.configdir);

  // DAT file handling
  ucon64.dat_enabled = 0;
  strcpy (ucon64.datdir, get_property (ucon64.configfile, "datdir", buf, ""));
#ifdef  __CYGWIN__
  strcpy (ucon64.datdir, fix_character_set (ucon64.datdir));
#endif
  strcpy (buf, ucon64.datdir);
  realpath2 (buf, ucon64.datdir);

  // we use ucon64.datdir as path to the dats
  if (!access (ucon64.datdir,
  // !W_OK doesn't mean that files can't be written to dir for Win32 exe's
#if     !defined __CYGWIN__ && !defined _WIN32
                              W_OK |
#endif
                              R_OK | X_OK))
    if (!stat (ucon64.datdir, &fstate))
      if (S_ISDIR (fstate.st_mode))
        ucon64.dat_enabled = 1;

  if (!ucon64.dat_enabled)
    if (!access (ucon64.configdir,
#if     !defined __CYGWIN__ && !defined _WIN32
                                   W_OK |
#endif
                                   R_OK | X_OK))
      if (!stat (ucon64.configdir, &fstate))
        if (S_ISDIR (fstate.st_mode))
          {
//            fprintf (stderr, "Please move your DAT files from %s to %s\n\n", ucon64.configdir, ucon64.datdir);
            strcpy (ucon64.datdir, ucon64.configdir); // use .ucon64/ instead of .ucon64/dat/
            ucon64.dat_enabled = 1;
          }

  if (ucon64.dat_enabled)
    ucon64_dat_indexer ();  // update cache (index) files if necessary

  // load libdiscmage
  ucon64.discmage_enabled = ucon64_load_discmage ();

  // ucon64.dat_enabled and ucon64.discmage_enabled can affect the usage output
  if (argc < 2)
    {
      ucon64_usage (argc, argv);
      return 0;
    }

  // getopt() is utilized to make uCON64 handle/parse cmdlines in a sane
  //  and expected way
  x = optind = 0;
  memset (&arg, 0, sizeof (st_args_t) * UCON64_MAX_ARGS);
  while ((c = getopt_long_only (argc, argv, "", options, NULL)) != -1)
    {
      if (c == '?') // getopt() returns 0x3f ('?') when an unknown option was given
        {
          fprintf (stderr,
               "Try '%s " OPTION_LONG_S "help' for more information.\n",
               argv[0]);
          exit (1);
        }

      if (x < UCON64_MAX_ARGS)
        {
          arg[x].val = c;
          arg[x++].optarg = (optarg ? optarg : NULL);
        }
      else
        // this shouldn't happen
        exit (1);
    }

#ifdef  DEBUG
  for (x = 0; arg[x].val; x++)
    printf ("%d %s\n\n", arg[x].val, arg[x].optarg ? arg[x].optarg : "(null)");
#endif

  rom_index = optind;                           // save index of first file
  if (rom_index == argc)
    ucon64_execute_options();
  else
    for (; rom_index < argc; rom_index++)
      {
        int result = 0;
        char buf2[FILENAME_MAX];
#ifndef _WIN32
        struct dirent *ep;
        DIR *dp;
#else
        char search_pattern[FILENAME_MAX];
        WIN32_FIND_DATA find_data;
        HANDLE dp;
#endif

        realpath2 (argv[rom_index], buf);
        if (stat (buf, &fstate) != -1)
          {
            if (S_ISREG (fstate.st_mode))
              result = ucon64_process_rom (buf);
            else if (S_ISDIR (fstate.st_mode))  // a dir?
              {
                char *p;
#if     defined __MSDOS__ || defined _WIN32 || defined __CYGWIN__
                /*
                  Note that this code doesn't make much sense for Cygwin,
                  because at least the version I use (1.3.6, dbjh) doesn't
                  support current directories for drives.
                */
                c = toupper (buf[0]);
                if (buf[strlen (buf) - 1] == FILE_SEPARATOR ||
                    (c >= 'A' && c <= 'Z' && buf[1] == ':' && buf[2] == 0))
#else
                if (buf[strlen (buf) - 1] == FILE_SEPARATOR)
#endif
                  p = "";
                else
                  p = FILE_SEPARATOR_S;

#ifndef _WIN32
                if ((dp = opendir (buf)))
                  {
                    while ((ep = readdir (dp)))
                      {
                        sprintf (buf2, "%s%s%s", buf, p, ep->d_name);
                        if (stat (buf2, &fstate) != -1)
                          if (S_ISREG (fstate.st_mode))
                            {
                              result = ucon64_process_rom (buf2);
                              if (result == 1)
                                break;
                            }
                      }
                    closedir (dp);
                  }
#else
                sprintf (search_pattern, "%s%s*", buf, p);
                if ((dp = FindFirstFile (search_pattern, &find_data)) != INVALID_HANDLE_VALUE)
                  {
                    do
                      {
                        sprintf (buf2, "%s%s%s", buf, p, find_data.cFileName);
                        if (stat (buf2, &fstate) != -1)
                          if (S_ISREG (fstate.st_mode))
                            {
                              result = ucon64_process_rom (buf2);
                              if (result == 1)
                                break;
                            }
                      }
                    while (FindNextFile (dp, &find_data));
                    FindClose (dp);
                  }
#endif
              }
            else
              result = ucon64_process_rom (buf);
          }
        else
          result = ucon64_process_rom (buf);

        if (result == 1)
          break;
      }

  return 0;
}


int
ucon64_process_rom (char *fname)
{
#ifdef  HAVE_ZLIB_H
  int n_entries = unzip_get_number_entries (fname);
  if (n_entries != -1)                          // it's a zip file
    {
      for (unzip_current_file_nr = 0; unzip_current_file_nr < n_entries;
           unzip_current_file_nr++)
        {
          ucon64_fname_arch (fname);
          ucon64.rom = fname;

          ucon64_execute_options();

          if (ucon64.flags & WF_STOP)
            break;
        }
      unzip_current_file_nr = 0;
      ucon64.fname_arch[0] = 0;

      if (ucon64.flags & WF_STOP)
        return 1;
    }
  else
#endif
    {
      ucon64.rom = fname;

      ucon64_execute_options();
      if (ucon64.flags & WF_STOP)
        return 1;
    }

  return 0;
}


int
ucon64_execute_options (void)
/*
  Execute all options for a single file.
  Please, if you experience problems then try your luck with the flags
  in ucon64_misc.c/ucon64_wf[] before changing things here or in
  ucon64_rom_handling()
*/
{
  int c = 0, result = 0, x = 0, opts = 0;
  static int first_call = 1;                    // first call to this function
  const st_ucon64_wf_t *wf = NULL;

  ucon64.console = UCON64_UNKNOWN;
  ucon64.dat = NULL;
  ucon64.image = NULL;
  ucon64.rominfo = NULL;

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

  ucon64.file_size =
  ucon64.crc32 =
  ucon64.fcrc32 = 0;

  // switches
  for (x = 0; arg[x].val; x++)
    {
      if ((wf = ucon64_get_wf (arg[x].val)))    // get workflow for that option
        {
          if (wf->console != UCON64_UNKNOWN)
            ucon64.console = wf->console;
          ucon64.flags = wf->flags;
        }

//      if (wf->flags & WF_SWITCH)
        ucon64_switches (arg[x].val, arg[x].optarg);
    }
#ifdef  ANSI_COLOR
  if (ucon64.ansi_color && first_call)
    ucon64.ansi_color = ansi_init ();
#endif

#ifdef  PARALLEL
  /*
    The copier options need root privileges for ucon64_parport_init()
    We can't use ucon64.flags & WF_PAR to detect whether a (parallel port)
    copier option has been specified, because another switch might've been
    specified after -port.
  */
  if (ucon64_parport_needed)
    ucon64.parport = ucon64_parport_init (ucon64.parport);
#endif
#if     defined __unix__ && !defined __MSDOS__
  if (first_call)
    drop_privileges ();                         // now we can drop privileges
#endif
  first_call = 0;
  
  for (x = 0; arg[x].val; x++)
    {
      if ((wf = ucon64_get_wf (arg[x].val)))    // get workflow for that option
        if (wf->flags & WF_SWITCH)
          continue;

      if (wf)
        {
          if (ucon64.console == UCON64_UNKNOWN)
            ucon64.console = wf->console;
          ucon64.flags = wf->flags;
        }

      opts++;

      // WF_NO_SPLIT WF_INIT, WF_PROBE, CRC32, DATabase and WF_NFO
      result = ucon64_rom_handling ();

      if (result == -1) // no rom, but WF_NO_ROM
        return -1;

      if (ucon64_options (arg[x].val, arg[x].optarg) == -1)
        {
          const struct option *option = ucon64_get_opt (c);
//          const st_usage_t *p = (ucon64_get_wf (c))->usage;
          const char *opt = option ? option->name : NULL;

          fprintf (stderr, "ERROR: %s%s encountered a problem\n",
                           opt ? (!opt[1] ? OPTION_S : OPTION_LONG_S) : "",
                           opt ? opt : "uCON64");

//          if (p)
//            ucon64_render_usage (p);

          fprintf (stderr, "       Is the option you used available for the current console system?\n"
                           "       Please report bugs to noisyb@gmx.net\n\n");

          return -1;
        }

#if 0
      // WF_NFO_AFTER?!
      if (!result && (ucon64.flags & WF_NFO_AFTER) && ucon64.quiet < 1)
        ucon64_rom_handling ();
#endif

      /*
        "stop" options:
        - -multi (and -xfalmulti) takes more than one file as argument, but
          should be executed only once.
        - stop after sending one ROM to a copier ("multizip")
        - stop after applying a patch so that the patch file won't be
          interpreted as ROM
      */
      if (ucon64.flags & WF_STOP)
        break;
    }

  if (!opts) // no options => just display ROM info
    {
      ucon64.flags = WF_DEFAULT;
      // WF_NO_SPLIT WF_INIT, WF_PROBE, CRC32, DATabase and WF_NFO
      if (ucon64_rom_handling () == -1)
        return -1; // no rom, but WF_NO_ROM
    }

  fflush (stdout);

  return 0;
}


int
ucon64_rom_handling (void)
{
  int no_rom = 0;
  static st_rominfo_t rominfo;
  struct stat fstate;

  ucon64_rom_flush (&rominfo);

  // a ROM (file)?
  if (!ucon64.rom)
    no_rom = 1;
  else if (!ucon64.rom[0])
    no_rom = 1;
  else if (access (ucon64.rom, F_OK | R_OK) == -1 && (!(ucon64.flags & WF_NO_ROM)))
    {
      fprintf (stderr, "ERROR: Could not open %s\n", ucon64.rom);
      no_rom = 1;
    }
  else if (stat (ucon64.rom, &fstate) == -1)
    no_rom = 1;
  else if (S_ISREG (fstate.st_mode) != TRUE)
    no_rom = 1;
#if 0
  // printing an error message for files of 0 bytes only confuses people
  else if (!fstate.st_size)
    no_rom = 1;
#endif

  if (no_rom)
    {
      if (!(ucon64.flags & WF_NO_ROM))
        {
          fprintf (stderr, "ERROR: This option requires a file argument (ROM/image/SRAM file/directory)\n");
          return -1;
        }
      return 0;
    }

  // the next statement is important and should be executed as soon as
  //  possible (and sensible) in this function
  ucon64.file_size = q_fsize (ucon64.rom);

  // Does the option allow split ROMs?
  if (ucon64.flags & WF_NO_SPLIT)
    if ((UCON64_ISSET (ucon64.split)) ? ucon64.split : ucon64_testsplit (ucon64.rom))
      {
        fprintf (stderr, "ERROR: %s seems to be split. You have to join it first\n", basename2 (ucon64.rom));
        return -1;
      }

  if (!(ucon64.flags & WF_INIT))
    return 0;

  // "walk through" <console>_init()
  if (ucon64.flags & WF_PROBE)
    {
      ucon64.rominfo = ucon64_probe (&rominfo); // returns console type

      if (ucon64.rominfo)
        {
          // restore any overrides from st_ucon64_t
          if (UCON64_ISSET (ucon64.buheader_len))
            rominfo.buheader_len = ucon64.buheader_len;

          if (UCON64_ISSET (ucon64.snes_hirom))
            rominfo.snes_hirom = ucon64.snes_hirom;

          if (UCON64_ISSET (ucon64.interleaved))
            rominfo.interleaved = ucon64.interleaved;

//          ucon64.rominfo = (st_rominfo_t *) &rominfo;
        }

      // check for disc image only if ucon64_probe() failed or --disc was used
      if (ucon64.discmage_enabled)
//        if (!ucon64.rominfo || ucon64.force_disc)
        if (ucon64.force_disc)
          ucon64.image = libdm_reopen (ucon64.rom, DM_RDONLY, ucon64.image);
    }
//end of WF_PROBE


  /*
    CRC32

    Calculating the CRC32 checksum for the ROM data of a UNIF file (NES)
    shouldn't be done with q_fcrc32(). nes_init() uses crc32().
    The CRC32 checksum is used to search in the DAT files, but at the time
    of this writing (Februari the 7th 2003) all DAT files contain checksums
    of files in only one format. This matters for SNES and Genesis ROMs in
    interleaved format and Nintendo 64 ROMs in non-interleaved format. The
    corresponding initialization functions calculate the CRC32 checksum of
    the data in the format of which the checksum is stored in the DAT
    files. For these "problematic" files, their "real" checksum is stored
    in ucon64.fcrc32.
  */
  switch (ucon64.console)
    {
      case UCON64_NES:
        break;

      default:
        if (ucon64.crc32 == 0)
          if (!ucon64.force_disc) // NOT for disc images
            if (!(ucon64.flags & WF_NOCRC32) || ucon64.file_size < MAXROMSIZE)
              ucon64.crc32 = q_fcrc32 (ucon64.rom, ucon64.rominfo ? ucon64.rominfo->buheader_len : 0);
        break;
    }


  // DATabase
  ucon64.dat = NULL;
  if (ucon64.crc32 != 0 && ucon64.dat_enabled)
    {
      ucon64.dat = ucon64_dat_search (ucon64.crc32, NULL);

      if (ucon64.dat)
        switch (ucon64.console)
          {
            case UCON64_SNES:
            case UCON64_GEN:
            case UCON64_GB:
            case UCON64_GBA:
            case UCON64_N64:
              // These ROMs have internal headers with name, country, maker, etc.
              break;

            default:
              // Use dat instead of ucon64.dat_enabled in case the index
              //  file could not be created/opened -> no segmentation fault
              if (ucon64.dat && ucon64.rominfo && ucon64.console == UCON64_NES &&
                  (nes_get_file_type () == UNIF ||
                   nes_get_file_type () == INES ||
                   nes_get_file_type () == PASOFAMI ||
                   nes_get_file_type () == FDS))
                strcpy ((char *) ucon64.rominfo->name, NULL_TO_EMPTY (ucon64.dat->name));
              break;
          }
    }

  // display info
  if ((ucon64.flags & WF_NFO || ucon64.flags & WF_NFO_AFTER) && ucon64.quiet < 1)
    ucon64_nfo ();

  return 0;
}


st_rominfo_t *
ucon64_probe (st_rominfo_t * rominfo)
{
  typedef struct
    {
      int console;
      int (*init) (st_rominfo_t *);
      uint32_t flags;
    } st_probe_t;

// auto recognition
#define AUTO (1)

  int x = 0;
  st_probe_t probe[] =
    {
      {UCON64_GBA, gba_init, AUTO},
      {UCON64_N64, n64_init, AUTO},
      {UCON64_GEN, genesis_init, AUTO},
      {UCON64_LYNX, lynx_init, AUTO},
      {UCON64_GB, gameboy_init, AUTO},
      {UCON64_SNES, snes_init, AUTO},
      {UCON64_NES, nes_init, AUTO},
      {UCON64_NGP, ngp_init, AUTO},
      {UCON64_SWAN, swan_init, AUTO},
      {UCON64_JAG, jaguar_init, AUTO},
      {UCON64_SMS, sms_init, 0},
      {UCON64_NG, neogeo_init, 0},
      {UCON64_PCE, pcengine_init, 0},
      {UCON64_SWAN, swan_init, 0},
      {UCON64_DC, dc_init, 0},
      {UCON64_PSX, psx_init, 0},
#if 0
      {UCON64_GC, NULL, 0},
      {UCON64_GP32, NULL, 0},
      {UCON64_COLECO, NULL, 0},
      {UCON64_INTELLI, NULL, 0},
      {UCON64_S16, NULL, 0},
      {UCON64_ATA, NULL, 0},
      {UCON64_VEC, NULL, 0},
      {UCON64_VBOY, NULL, 0},
#endif
      {UCON64_UNKNOWN, unknown_init, 0},
      {0, NULL, 0}
    };

  if (ucon64.console != UCON64_UNKNOWN) //  force recognition option was used
    {
      for (x = 0; probe[x].console != 0; x++)
        if (probe[x].console == ucon64.console)
          {
            ucon64_rom_flush (rominfo);

            probe[x].init (rominfo);

            return rominfo;
          }
    }
  else if (ucon64.file_size < MAXROMSIZE) // give auto_recognition a try
    {
      for (x = 0; probe[x].console != 0; x++)
        if (probe[x].flags & AUTO)
          {
            ucon64_rom_flush (rominfo);

            if (!probe[x].init (rominfo))
              {
                ucon64.console = probe[x].console;
                return rominfo;
              }
          }
    }
  
  return NULL;
}


int
ucon64_nfo (void)
{
  printf ("%s\n", ucon64.rom);
  if (ucon64.fname_arch[0])
    printf ("  (%s)\n", ucon64.fname_arch);
  fputc ('\n', stdout);

  if (ucon64.console == UCON64_UNKNOWN && !ucon64.image)
    {
      fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
      printf ("\n");
    }

  if (ucon64.rominfo && ucon64.console != UCON64_UNKNOWN && !ucon64.force_disc)
    ucon64_rom_nfo (ucon64.rominfo);
  
  if (ucon64.discmage_enabled)
    if (ucon64.image)
      libdm_nfo (ucon64.image);

  // Use ucon64.fcrc32 for SNES & Genesis interleaved/N64 non-interleaved
  printf ("Checksum (CRC32): 0x%08x\n", ucon64.fcrc32 ? ucon64.fcrc32 : ucon64.crc32);

  // The check for the size of the file is made, so that uCON64 won't display a
  //  (nonsense) DAT info line when dumping a ROM (file doesn't exist, so
  //  ucon64.file_size is 0).
  if (ucon64.file_size > 0 && ucon64.dat_enabled)
    if (ucon64.dat)
      ucon64_dat_nfo (ucon64.dat, 1);

  printf ("\n");

  return 0;
}


void
ucon64_rom_nfo (const st_rominfo_t *rominfo)
{
  unsigned int padded = ucon64_testpad (ucon64.rom),
    intro = ((ucon64.file_size - rominfo->buheader_len) > MBIT) ?
      ((ucon64.file_size - rominfo->buheader_len) % MBIT) : 0;
  int x, split = (UCON64_ISSET (ucon64.split)) ? ucon64.split :
           ucon64_testsplit (ucon64.rom);
  char buf[MAXBUFSIZE];

  // backup unit header

  if (rominfo->buheader && rominfo->buheader_len && rominfo->buheader_len != UNKNOWN_HEADER_LEN)
    {
      mem_hexdump (rominfo->buheader, rominfo->buheader_len, rominfo->buheader_start);
      printf ("\n");
    }
  else
    if (rominfo->buheader_len && ucon64.quiet < 0)
      {
        ucon64_fhexdump (ucon64.rom, rominfo->buheader_start, rominfo->buheader_len);
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


void
ucon64_render_usage (const st_usage_t *usage)
{
  // TODO: speed up
  int x, pos = 0;
  char buf[MAXBUFSIZE];

#ifdef  DEBUG
  // look for malformed usages
  for (x = 0; usage[x].option_s || usage[x].optarg || usage[x].desc; x++)
    fprintf (stderr, "{\"%s\", \"%s\", \"%s\"},\n",
      usage[x].option_s,
      usage[x].optarg,
      usage[x].desc);
#endif

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

              // adjust tabs for OPTION_S and OPTION_LONG_S here not in the
              //  OPTION_S definition
              sprintf (buf,
                        (len == 1 ?
                         ("   " OPTION_S "%s%c") :
                         ("  " OPTION_LONG_S "%s%c")),
                         usage[x].option_s,
                         (usage[x].optarg ? OPTARG : '\0'));

              if (usage[x].optarg)
                strcat (buf, usage[x].optarg);

              strcat (buf, " ");

              if (strlen (buf) < 16)
                {
                  strcat (buf, "                             ");
                  buf[16] = 0;
                }
              printf (buf);
            }

          if (usage[x].desc)
            {
#if 1
              for (pos = 0; usage[x].desc[pos]; pos++)
                {
                  printf ("%c", usage[x].desc[pos]); // TODO: speed this up

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


void
ucon64_usage (int argc, char *argv[])
{
  typedef struct
    {
      int console;
      const st_usage_t *usage[6];
    } st_usage_array_t;

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
    {UCON64_SMS, {sms_usage, 0, 0, 0, 0, 0}},
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
  int x = 0, c = 0, single = 0;
  char *name_exe = basename (argv[0]), *name_discmage;
  (void) argc;                                  // warning remover

#ifdef  HAVE_ZLIB_H
#define USAGE_S "Usage: %s [OPTION]... [ROM|IMAGE|SRAM|FILE|DIR|ARCHIVE]...\n\n"
#else
#define USAGE_S "Usage: %s [OPTION]... [ROM|IMAGE|SRAM|FILE|DIR]...\n\n"
#endif
  printf (USAGE_S, name_exe);

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

  if (ucon64.discmage_enabled)
    {
      ucon64_render_usage (libdm_usage);
      printf ("\n");
    }

  // getopt()?
  for (c = 0; arg[c].val; c++)
    for (x = 0; usage_array[x].console != 0; x++)
      if (usage_array[x].console == arg[c].val)
        {
          int y = 0;
          for (; usage_array[x].usage[y]; y++)
            ucon64_render_usage (usage_array[x].usage[y]);
           single = 1; // we show only the usage for the specified console(s)

          printf ("\n");
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

  printf (
     "DATabase: %d known ROMs (DAT files: %s)\n\n",
       ucon64_dat_total_entries (),
       ucon64.datdir);
          
  name_discmage =
#ifdef  DLOPEN
    ucon64.discmage_path;
#else
#if     defined __MSDOS__
    "discmage.dxe";
#elif   defined __CYGWIN__ || defined _WIN32
    "discmage.dll";
#elif   defined __unix__ || defined __BEOS__
    "libdiscmage.so";
#else
    "unknown";
#endif
#endif

  if (!ucon64.discmage_enabled)
    {
      printf (ucon64_msg[NO_LIB], name_discmage);
      printf ("\n");
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
     PARALLEL_MSG
     "TIP: %s " OPTION_LONG_S "help " OPTION_LONG_S "snes (would show only SNES related help)\n"
     MORE_MSG
     "     Give the force recognition switch a try if something went wrong\n"
     "\n"
     "Please report any problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net\n"
     "\n",
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
