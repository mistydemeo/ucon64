/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh


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
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef  __linux__
#include <sys/io.h>
#endif

#include "config.h"

#include "getopt.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "misc.h"

#include "snes/snes.h"
#include "gb/gb.h"
#include "gba/gba.h"
#include "jaguar/jaguar.h"
#include "n64/n64.h"
#include "lynx/lynx.h"
#include "sms/sms.h"
#include "nes/nes.h"
#include "genesis/genesis.h"
#include "pce/pce.h"
#include "neogeo/neogeo.h"
#include "sys16/sys16.h"
#include "atari/atari.h"
#include "ngp/ngp.h"
#include "coleco/coleco.h"
#include "vboy/vboy.h"
#include "vectrex/vectrex.h"
#include "swan/swan.h"
#include "intelli/intelli.h"
#include "cd32/cd32.h"
#include "ps2/ps2.h"
#include "saturn/saturn.h"
#include "cdi/cdi.h"
#include "psx/psx.h"
#include "dc/dc.h"
#include "real3do/real3do.h"
#include "gamecube/gamecube.h"
#include "xbox/xbox.h"
#include "gp32/gp32.h"

#include "patch/ppf.h"
#include "patch/xps.h"
#include "patch/pal4u.h"
#include "patch/aps.h"
#include "patch/ips.h"
#include "patch/bsl.h"

#include "backup/fig.h"
#include "backup/swc.h"
#include "backup/cdrw.h"

static int ucon64_nfo (const st_rom_t *rominfo);
static int ucon64_console_probe (st_rom_t *rominfo);
static int ucon64_e (const st_rom_t *rominfo);

static int ucon64_init (const char *romfile, st_rom_t *rominfo);

static int ucon64_ls (st_rom_t *rominfo, int mode);
static int ucon64_configfile (void);
static void ucon64_exit (void);
static void ucon64_usage (int argc, char *argv[]);

st_ucon64_t ucon64;

static const char *ucon64_title = "uCON64 " UCON64_VERSION_S " " UCON64_OS 
                              " 1999-2002 by (various)";

static const struct option long_options[] = {
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
    {"cd32", 0, 0, UCON64_CD32},
    {"cdi", 0, 0, UCON64_CDI},
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
    {"pas", 0, 0, UCON64_PAS},
    {"pce", 0, 0, UCON64_PCE},
    {"port", 1, 0, UCON64_PORT},
    {"ppf", 0, 0, UCON64_PPF},
    {"ps2", 0, 0, UCON64_PS2},
    {"psx", 0, 0, UCON64_PSX},
    {"ren", 0, 0, UCON64_REN},
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
    {"strip", 0, 0, UCON64_STRIP},
    {"swan", 0, 0, UCON64_SWAN},
    {"swap", 0, 0, UCON64_SWAP},
    {"swc", 0, 0, UCON64_SWC},
    {"swcs", 0, 0, UCON64_SWCS},
    {"tm", 0, 0, UCON64_TM},
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
    {"version", 0, 0, UCON64_VERSION},
    {0, 0, 0, 0}
  };


void
ucon64_exit (void)
{
  printf ("+++EOF");
  fflush (stdout);
}


int
main (int argc, char *argv[])
{
  long x, y = 0;
  int ucon64_argc;
  int c = 0, result = 0;
  unsigned long padded;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE];
  const char *ucon64_argv[128];
  int option_index = 0;
  st_rom_t rom;

  printf ("%s\n"
    "Uses code from various people. See 'developers.html' for more!\n"
    "This may be freely redistributed under the terms of the GNU Public License\n\n",
    ucon64_title);

  memset (&ucon64, 0L, sizeof (st_ucon64_t));

  ucon64_configfile ();

  ucon64.show_nfo = UCON64_SHOW_NFO_BEFORE;

  ucon64.buheader_len =
  ucon64.interleaved =
  ucon64.splitted =
  ucon64.snes_hirom = -1;

  ucon64.parport = 0x378;
#if 0
//TODO
  ucon64.parport = strtol (getProperty (ucon64.configfile, "parport", buf2, "0x378"), NULL, 10);
#endif

  ucon64.backup = ((!strcmp (getProperty (ucon64.configfile, "backups", buf2, "1"), "1")) ?
               1 : 0);

  if (argc < 2)
    {
       ucon64_usage (argc, argv);
       return 0;
    }

  ucon64.argc = argc;
  for (x = 0; x < argc; x++)
    ucon64.argv[x] = argv[x];

  ucon64_init (NULL, &rom);

/*
  getopt_long_only() - switches and overrides
*/
  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      switch (c)
        {
        case UCON64_FRONTEND:
          atexit (ucon64_exit);
          ucon64.frontend = 1;                  // used by ucon64_gauge()
          break;

        case UCON64_NBAK:
          ucon64.backup = 0;
          break;

        case UCON64_NS:
          ucon64.splitted = 0;
          break;

        case UCON64_HD:
          ucon64.buheader_len = UNKNOWN_HEADER_LEN;
          rom.copier = unknown_title;
          break;

        case UCON64_HDN:
          if (strtol (optarg, NULL, 10) > MAXBUFSIZE)
            printf ("ERROR: BYTES > %d does not work\n\n", MAXBUFSIZE);
          else
            ucon64.buheader_len = strtol (optarg, NULL, 10);
          rom.copier = unknown_title;
          break;

        case UCON64_NHD:
          ucon64.buheader_len = 0;
          rom.copier = unknown_title;
          break;

        case UCON64_SWP://deprecated
        case UCON64_INT:
#if 0
          switch (rom.console)
            {
              case UCON64_SNES:
              default:
#endif
                ucon64.interleaved = 1;
              break;
//            }

        case UCON64_INT2:
#if 0
          switch (rom.console)
            {
              case UCON64_SNES:
              default:
#endif
                ucon64.interleaved = 2;
                break;
//            }

        case UCON64_NSWP://deprecated
        case UCON64_NINT:
#if 0
          switch (rom.console)
            {
              case UCON64_SNES:
              default:
#endif
                ucon64.interleaved = 0;
                break;
//            }

        case UCON64_PORT:
          if (optarg)
            sscanf (optarg, "%x", &ucon64.parport);
          break;

        case UCON64_FILE:
          if (optarg)
            ucon64.file = optarg;
          break;

        case UCON64_ROM:
          if (optarg)
            ucon64.rom = optarg;
          break;

        case UCON64_A:
        case UCON64_B:
        case UCON64_C:
        case UCON64_CHK:
        case UCON64_CRC:
        case UCON64_CRCHD://deprecated
        case UCON64_CS:
        case UCON64_DB:
        case UCON64_DBS:
        case UCON64_DBV:
        case UCON64_DINT:
        case UCON64_SWAP:
        case UCON64_E:
        case UCON64_FIND:
        case UCON64_GG:
        case UCON64_GGD:
        case UCON64_GGE:
        case UCON64_HELP:
        case UCON64_HEX:
        case UCON64_I:
        case UCON64_IDPPF:
        case UCON64_INS:
        case UCON64_ISO:
        case UCON64_ISPAD:
        case UCON64_J:
        case UCON64_LS:
        case UCON64_LSV:
        case UCON64_MGD:
//        case UCON64_MGH:
        case UCON64_MKA:
        case UCON64_MKCUE:
        case UCON64_MKI:
        case UCON64_MKPPF:
        case UCON64_MKTOC:
        case UCON64_N:
        case UCON64_NA:
        case UCON64_NPPF:
        case UCON64_P:
        case UCON64_PAD:
        case UCON64_PADHD://deprecated
        case UCON64_PPF:
        case UCON64_REN:
        case UCON64_RL:
        case UCON64_RU:
        case UCON64_S:
        case UCON64_SRAM:
        case UCON64_STP:
        case UCON64_STRIP:
        case UCON64_TM:
#ifdef BACKUP_CD
        case UCON64_XCDRW:
#endif // BACKUP_CD
        case UCON64_SMD:
        case UCON64_SMDS:
#ifdef BACKUP
        case UCON64_XSMD:
        case UCON64_XSMDS:
#endif // BACKUP
        case UCON64_VERSION:
          ucon64.show_nfo = UCON64_SHOW_NFO_NEVER;
          break;
  
        case UCON64_NHI:
          ucon64.snes_hirom = 0;
          rom.console = UCON64_SNES;
          break;

        case UCON64_HI:
          ucon64.snes_hirom = 1;
          rom.console = UCON64_SNES;
          break;

        case UCON64_COL:
        case UCON64_SWCS:
        case UCON64_FIGS:
        case UCON64_UFOS:
#ifdef BACKUP
        case UCON64_XSWCS:
#endif // BACKUP
          ucon64.show_nfo = UCON64_SHOW_NFO_NEVER;
        case UCON64_SNES:
        case UCON64_F:
        case UCON64_K:
        case UCON64_L:
        case UCON64_GD3:
//        case UCON64_GDF:
        case UCON64_SMC:
        case UCON64_SWC:
        case UCON64_FIG:
#ifdef BACKUP
        case UCON64_XSWC:
#endif // BACKUP
          rom.console = UCON64_SNES;
          break;

        case UCON64_GB:
        case UCON64_SGB:
        case UCON64_GBX:
        case UCON64_N2GB:
        case UCON64_SSC:
#ifdef BACKUP
        case UCON64_XGBX:
        case UCON64_XGBXS:
        case UCON64_XGBXB:
#endif // BACKUP
          rom.console = UCON64_GB;
          break;

        case UCON64_ATA:
          rom.console = UCON64_ATARI;
          break;

        case UCON64_S16:
          rom.console = UCON64_SYSTEM16;
          break;

        case UCON64_COLECO:
          rom.console = UCON64_COLECO;
          break;
  
        case UCON64_VBOY:
          rom.console = UCON64_VIRTUALBOY;
          break;
  
        case UCON64_SWAN:
          rom.console = UCON64_WONDERSWAN;
          break;

        case UCON64_VEC:
          rom.console = UCON64_VECTREX;
          break;

        case UCON64_INTELLI:
          rom.console = UCON64_INTELLI;
          break;
  
        case UCON64_LYNX:
        case UCON64_B0:
        case UCON64_B1:
        case UCON64_ROTL:
        case UCON64_ROTR:
        case UCON64_NROT:
        case UCON64_LNX:
        case UCON64_LYX:
          rom.console = UCON64_LYNX;
          break;
  
        case UCON64_SMS:
          rom.console = UCON64_SMS;
          break;

        case UCON64_NGP:
          rom.console = UCON64_NEOGEOPOCKET;
          break;

        case UCON64_INES:
        case UCON64_INESHD:
        case UCON64_NES:
//        case UCON64_FDS:
//        case UCON64_FDSL:
//        case UCON64_PAS:
        case UCON64_FFE:
        case UCON64_UNIF:
          rom.console = UCON64_NES;
          break;

        case UCON64_SMG:
        case UCON64_PCE:
          rom.console = UCON64_PCE;
          break;

        case UCON64_JAG:
          rom.console = UCON64_JAGUAR;
          break;
  
        case UCON64_GEN:
        case UCON64_N2:
        case UCON64_1991:
          rom.console = UCON64_GENESIS;
          break;
  
        case UCON64_NG:
        case UCON64_SAM:
        case UCON64_MVS:
        case UCON64_BIOS:
          rom.console = UCON64_NEOGEO;
          break;
  
#ifdef BACKUP
        case UCON64_XV64:
        case UCON64_XDJR:
#endif // BACKUP
        case UCON64_BOT:
        case UCON64_Z64:
        case UCON64_N64:
        case UCON64_USMS:
        case UCON64_V64:
          rom.console = UCON64_N64;
          break;

        case UCON64_MULTI:
        case UCON64_MULTI1:
        case UCON64_MULTI2:
          ucon64.show_nfo = UCON64_SHOW_NFO_NEVER;
#ifdef BACKUP
        case UCON64_XFAL:
        case UCON64_XFALS:
        case UCON64_XFALB:
        case UCON64_XFALC:
#endif // BACKUP
        case UCON64_LOGO:
        case UCON64_CRP:
        case UCON64_GBA:
          rom.console = UCON64_GBA;
          break;

        case UCON64_SAT:
          rom.console = UCON64_SATURN;
          break;

        case UCON64_PSX:
          rom.console = UCON64_PSX;
          break;
  
        case UCON64_PS2:
          rom.console = UCON64_PS2;
          break;

        case UCON64_CDI:
          rom.console = UCON64_CDI;
          break;

        case UCON64_CD32:
          rom.console = UCON64_CD32;
          break;
  
        case UCON64_3DO:
          rom.console = UCON64_REAL3DO;
          break;

        case UCON64_IP:
        case UCON64_DC:
          rom.console = UCON64_DC;
          break;

        case UCON64_XBOX:
          rom.console = UCON64_XBOX;
          break;

        case UCON64_GC:
          rom.console = UCON64_GAMECUBE;
          break;

        case UCON64_GP32:
          rom.console = UCON64_GP32;
          break;

        case UCON64_GETOPT_ERROR:
        default:
          fprintf (stderr, "Try '%s " OPTION_LONG_S "help' for more information.\n", argv[0]);
          return -1;
      }
    }

  if (optind < argc)
    ucon64.rom = argv[optind++];
  if (optind < argc)
    ucon64.file = argv[optind++];

#ifdef BACKUP
  if (ucon64.file)
    sscanf (ucon64.file, "%x", &ucon64.parport);
  ucon64.parport = ucon64_parport_probe (ucon64.parport);
#endif

  if (!ucon64_init (ucon64.rom, &rom))
    switch (ucon64.show_nfo)
      {
         case UCON64_SHOW_NFO_BEFORE:
         case UCON64_SHOW_NFO_BEFORE_AND_AFTER:
           ucon64_nfo (&rom);
           break;

         case UCON64_SHOW_NFO_AFTER:
         case UCON64_SHOW_NFO_NEVER:
         default:
           break;
      }

/*
  getopt_long_only() - options
*/
  optind = option_index = 0;//TODO is there a better way to "reset"?

  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      switch (c)
        {
        case UCON64_HELP:
          ucon64_usage (argc, argv);
          return 0;

        case UCON64_CRCHD://deprecated
          rom.buheader_len = UNKNOWN_HEADER_LEN;
        case UCON64_CRC:
          printf ("Checksum (CRC32): %08lx\n\n", fileCRC32 (ucon64.rom, rom.buheader_len));
          return 0;

        case UCON64_RL:
          return renlwr (ucon64.rom);
  
        case UCON64_RU:
          return renupr (ucon64.rom);
  
        case UCON64_HEX:
          return filehexdump (ucon64.rom, 0, quickftell (ucon64.rom));

        case UCON64_C:
          if (filefile (ucon64.rom, 0, ucon64.file, 0, FALSE) == -1)
            {
              printf ("ERROR: file not found/out of memory\n");
              return -1;
            }
          return 0;

        case UCON64_CS:
          if (filefile (ucon64.rom, 0, ucon64.file, 0, TRUE) == -1)
            {
              printf ("ERROR: file not found/out of memory\n");
              return -1;
            }
          return 0;
  
        case UCON64_FIND:
          x = 0;
          y = quickftell (ucon64.rom);
          while ((x =
                  filencmp2 (ucon64.rom, x, y, ucon64.file, strlen (ucon64.file),
                             '?')) != -1)
            {
              filehexdump (ucon64.rom, x, strlen (ucon64.file));
              x++;
              printf ("\n");
            }
          return 0;


        case UCON64_PADHD://deprecated
          rom.buheader_len = UNKNOWN_HEADER_LEN;
        case UCON64_PAD:
          ucon64_fbackup (ucon64.rom);
          return filepad (ucon64.rom, rom.buheader_len, MBIT);
  
        case UCON64_ISPAD:
          if ((padded = filetestpad (ucon64.rom)) != -1)
            {
              if (!padded)
                printf ("Padded: No\n\n");
              else
                printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n\n", padded,
                        (float) padded / MBIT);
            }
          return 0;

        case UCON64_STRIP:
          ucon64_fbackup (ucon64.rom);

          return truncate (ucon64.rom, quickftell (ucon64.rom) - atol (ucon64.file));

        case UCON64_STP:
          strcpy (buf, ucon64.rom);
          setext (buf, ".BAK");
          remove (buf);           // try to remove or rename will fail
          rename (ucon64.rom, buf);
  
          return filecopy (buf, 512, quickftell (buf), ucon64.rom, "wb");
  
        case UCON64_INS:
          strcpy (buf, ucon64.rom);
          setext (buf, ".BAK");
          remove (buf);           // try to remove or rename will fail
          rename (ucon64.rom, buf);

          memset (buf2, 0, 512);
          quickfwrite (buf2, 0, 512, ucon64.rom, "wb");
  
          return filecopy (buf, 0, quickftell (buf), ucon64.rom, "ab");
  
        case UCON64_B:
          ucon64_fbackup (ucon64.rom);

          if ((result = bsl (ucon64.rom, ucon64.file)) != 0)
            printf ("ERROR: failed\n");
          return result;

        case UCON64_I:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = ucon64.file;
          ucon64_argv[2] = ucon64.rom;
          ucon64_argc = 3;
  
          ucon64_fbackup (ucon64.rom);

          ips_main (ucon64_argc, ucon64_argv);
          break;
  
        case UCON64_A:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = "-f";
          ucon64_argv[2] = ucon64.rom;
          ucon64_argv[3] = ucon64.file;
          ucon64_argc = 4;

          ucon64_fbackup (ucon64.rom);

          return n64aps_main (ucon64_argc, ucon64_argv);

        case UCON64_MKI:
          return cips (ucon64.rom, ucon64.file);

        case UCON64_MKA:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = "-d \"\"";
          ucon64_argv[2] = ucon64.rom;
          ucon64_argv[3] = ucon64.file;
          strcpy (buf, ucon64.rom);
          setext (buf, ".APS");
  
          ucon64_argv[4] = buf;
          ucon64_argc = 5;
  
          return n64caps_main (ucon64_argc, ucon64_argv);

        case UCON64_NA:
          memset (buf2, ' ', 50);
          strncpy (buf2, ucon64.file, strlen (ucon64.file));
          return quickfwrite (buf2, 7, 50, ucon64_fbackup (ucon64.rom), "r+b");

        case UCON64_PPF:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = ucon64.rom;
          ucon64_argv[2] = ucon64.file;
          ucon64_argc = 3;
  
          return applyppf_main (ucon64_argc, ucon64_argv);
  
        case UCON64_MKPPF:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = ucon64.rom;
          ucon64_argv[2] = ucon64.file;

          strcpy (buf, ucon64.file);
          setext (buf, ".PPF");

          ucon64_argv[3] = buf;
          ucon64_argc = 4;

          return makeppf_main (ucon64_argc, ucon64_argv);

        case UCON64_NPPF:
          memset (buf2, ' ', 50);
          strncpy (buf2, ucon64.file, strlen (ucon64.file));
          return quickfwrite (buf2, 6, 50, ucon64_fbackup (ucon64.rom), "r+b");

        case UCON64_IDPPF:
          return addppfid (ucon64.rom);

        case UCON64_LS:
          strcpy (rom.rom, ucon64.rom);
          return ucon64_ls (&rom, UCON64_LS);

        case UCON64_LSV:
          strcpy (rom.rom, ucon64.rom);
          return ucon64_ls (&rom, UCON64_LSV);

        case UCON64_REN:
          strcpy (rom.rom, ucon64.rom);
          return ucon64_ls (&rom, UCON64_REN);

        case UCON64_ISO:
          return ucon64_bin2iso (ucon64.rom, ucon64_trackmode_probe (ucon64.rom));

        case UCON64_TM:
          result = ucon64_trackmode_probe (ucon64.rom);
          
          printf ("Track mode: %s\n", (result != -1) ? track_modes[result] : "Unknown/Unsupported");
          return 0;

        case UCON64_MKTOC:
          return cdrw_mktoc (&rom);
  
        case UCON64_MKCUE:
          return cdrw_mkcue (&rom);

#ifdef BACKUP_CD
        case UCON64_XCDRW:
#if 0
          switch (rom.console)
            {
            case UCON64_DC:
              return dc_xcdrw (&rom);

            default:
#endif
              return (!access (ucon64.rom, F_OK)) ? cdrw_write (&rom) :
                cdrw_read (&rom);
//            }
#endif // BACKUP_CD
  
        case UCON64_DB:
          printf ("Database: %ld known ROMs in db.h (%+ld)\n\n"
                  "TIP: %s " OPTION_LONG_S "db " OPTION_LONG_S "nes would show only the number of known NES ROMs\n\n",
                  ucon64_dbsize (rom.console),
                  ucon64_dbsize (rom.console) - UCON64_DBSIZE,
                  argv[0]);
          break;

        case UCON64_DBS:
          ucon64_init (NULL, &rom);
          sscanf (ucon64.rom, "%lx", &rom.current_crc32);
          ucon64_dbsearch (&rom);
          ucon64_nfo (&rom);
//          ucon64_dbview (rom.console);
          printf ("TIP: %s "OPTION_LONG_S "dbs "OPTION_LONG_S "nes would search only for a NES ROM\n\n",
                  argv[0]);
  
          break;
  
        case UCON64_DBV:
          ucon64_dbview (rom.console);
  
          printf ("\nTIP: %s " OPTION_LONG_S "db " OPTION_LONG_S "nes would view only NES ROMs\n\n",
                  argv[0]);
          break;
  
        case UCON64_MULTI:
          return gba_multi (&rom, 256 * MBIT);

        case UCON64_MULTI1:
          return gba_multi (&rom, 64 * MBIT);

        case UCON64_MULTI2:
          return gba_multi (&rom, 128 * MBIT);
  
        case UCON64_SWCS:
          return snes_swcs (&rom);
  
        case UCON64_FIGS:
          return snes_figs (&rom);
  
        case UCON64_UFOS:
          return snes_ufos (&rom);
  
        case UCON64_E:
          return ucon64_e (&rom);

        case UCON64_1991:
          return genesis_1991 (&rom);
  
        case UCON64_B0:
          return lynx_b0 (&rom);
  
        case UCON64_B1:
          return lynx_b1 (&rom);
  
        case UCON64_BIOS:
          return neogeo_bios (&rom);
  
        case UCON64_BOT:
          return n64_bot (&rom);
  
        case UCON64_CHK:
          switch (rom.console)
            {
            case UCON64_GB:
              return gameboy_chk (&rom);
            case UCON64_GBA:
              return gba_chk (&rom);
            case UCON64_GENESIS:
              return genesis_chk (&rom);
            case UCON64_N64:
              return n64_chk (&rom);
            case UCON64_SNES:
              return snes_chk (&rom);
            case UCON64_WONDERSWAN:
              return swan_chk (&rom);
            default:
//TODO error
              return -1;
            }
  
        case UCON64_COL:
          return snes_col (&rom);
  
        case UCON64_CRP:
          return gba_crp (&rom);
  
        case UCON64_DINT:
        case UCON64_SWAP://deprecated
          switch (rom.console)
            {
            case UCON64_SNES:
              return snes_dint (&rom);

            default:
              result = fileswap (ucon64_fbackup (ucon64.rom), 0,
                           quickftell (ucon64.rom));
              printf ("Wrote output to %s\n", ucon64.rom);
              return result;
            }
  
        case UCON64_F:
          switch (rom.console)
            {
            case UCON64_N64:
              return n64_f (&rom);
            case UCON64_SNES:
              return snes_f (&rom);
            default:
//TODO error
              return -1;
            }
  
#if 0
        case UCON64_FDS:
          return nes_fds (&rom);

        case UCON64_FDSL:
          return nes_fdsl (&rom);
#endif
        case UCON64_FFE:
          return nes_ffe (&rom);
  
        case UCON64_FIG:
          return snes_fig (&rom);
  
        case UCON64_GBX:
          return gameboy_gbx (&rom);
  
        case UCON64_GD3:
          return snes_gd3 (&rom);
  
        case UCON64_GDF:
          return snes_gdf (&rom);
  
        case UCON64_GG:
          switch (rom.console)
            {
            case UCON64_GB:
              return gameboy_gg (&rom);
#if 0
            case UCON64_GENESIS:
              return
#endif              
            case UCON64_NES:
              return nes_gg (&rom);
            case UCON64_SNES:
              return snes_gg (&rom);
            default:
//TODO error
              return -1;
            }
  
        case UCON64_GGD:
          switch (rom.console)
            {
            case UCON64_GB:
              return gameboy_ggd (&rom);
            case UCON64_GENESIS:
              return genesis_ggd (&rom);
            case UCON64_NES:
              return nes_ggd (&rom);
            case UCON64_SNES:
              return snes_ggd (&rom);
            default:
//TODO error
              return -1;
            }
  
        case UCON64_GGE:
          switch (rom.console)
            {
            case UCON64_GB:
              return gameboy_gge (&rom);
            case UCON64_GENESIS:
              return genesis_gge (&rom);
            case UCON64_NES:
              return nes_gge (&rom);
            case UCON64_SNES:
              return snes_gge (&rom);
            default:
//TODO error
              return -1;
            }
  
        case UCON64_INES:
          return nes_ines (&rom);
  
        case UCON64_INESHD:
          return nes_ineshd (&rom);
  
#if 0
        case UCON64_IP:
          break;
#endif
  
        case UCON64_J:
          switch (rom.console)
            {
            case UCON64_GENESIS:
              return genesis_j (&rom);
            case UCON64_NES:
              return nes_j (&rom);
            case UCON64_SNES:
              return snes_j (&rom);
            default:
//TODO error
              return -1;
            }

        case UCON64_K:
          return snes_k (&rom);
  
        case UCON64_L:
          return snes_l (&rom);

        case UCON64_LNX:
          return lynx_lnx (&rom);
  
        case UCON64_LOGO:
          return gba_logo (&rom);
  
        case UCON64_LYX:
          return lynx_lyx (&rom);
  
        case UCON64_MGD:
          switch (rom.console)
            {
            case UCON64_GB:
              return gameboy_mgd (&rom);
            case UCON64_GENESIS:
              return genesis_mgd (&rom);
            case UCON64_NEOGEO:
              return neogeo_mgd (&rom);
            case UCON64_SNES:
              return snes_mgd (&rom);
            case UCON64_PCE:
              return pcengine_mgd (&rom);
            default:
//TODO error
              return -1;
            }
  
        case UCON64_MVS:
          return neogeo_mvs (&rom);
  
        case UCON64_N:
          switch (rom.console)
            {
            case UCON64_GB:
              return gameboy_n (&rom);
            case UCON64_GBA:
              return gba_n (&rom);
            case UCON64_GENESIS:
              return genesis_n (&rom);
            case UCON64_LYNX:
              return lynx_n (&rom);
            case UCON64_N64:
              return n64_n (&rom);
#if 0
            case UCON64_NES:
              return nes_n (&rom);
#endif
            case UCON64_SNES:
              return snes_n (&rom);
            default:
//TODO error 
              return -1;
            }
  
        case UCON64_N2:
          return genesis_n2 (&rom);
  
        case UCON64_N2GB:
          return gameboy_n2gb (&rom);
  
        case UCON64_NROT:
          return lynx_nrot (&rom);

        case UCON64_P:
          switch (rom.console)
            {
            case UCON64_GENESIS:
              return genesis_p (&rom);
            case UCON64_N64:
              return n64_p (&rom);
            case UCON64_SNES:
              return snes_p (&rom);
            default:
  //TODO error
              return -1;
            }
  
        case UCON64_PAS:
          return nes_pas (&rom);
  
        case UCON64_ROTL:
          return lynx_rotl (&rom);
  
        case UCON64_ROTR:
          return lynx_rotr (&rom);

        case UCON64_S:
          switch (rom.console)
            {
            case UCON64_GENESIS:
              return genesis_s (&rom);
            case UCON64_NEOGEO:
              return neogeo_s (&rom);
            case UCON64_NES:
              return nes_s (&rom);
            case UCON64_SNES:
              return snes_s (&rom);
            default:
  //TODO error
              return -1;
            }
  
        case UCON64_SAM:
          return neogeo_sam (&rom);
  
        case UCON64_SGB:
          return gameboy_sgb (&rom);

        case UCON64_SMC:
          return snes_smc (&rom);
  
        case UCON64_SMD:
          return genesis_smd (&rom);

        case UCON64_SMDS:
          return genesis_smds (&rom);
  
        case UCON64_SMG:
          return pcengine_smg (&rom);
  
        case UCON64_SRAM:
          switch (rom.console)
            {
            case UCON64_GBA:
              return gba_sram (&rom);
            case UCON64_N64:
              return n64_sram (&rom);
            default:
  //TODO error
              return -1;
            }
          
        case UCON64_SSC:
          return gameboy_ssc (&rom);
  
        case UCON64_SWC:
          return snes_swc (&rom);
  
        case UCON64_UNIF:
          return nes_unif (&rom);

        case UCON64_USMS:
          return n64_usms (&rom);
  
        case UCON64_V64:
          return n64_v64 (&rom);

#ifdef BACKUP
        case UCON64_XDJR:
          return n64_xdjr (&rom);
  
        case UCON64_XFAL:
          return gba_xfal (&rom, -1);

        case UCON64_XFALS:
          return gba_xfals (&rom);

        case UCON64_XGBX:
          return gameboy_xgbx (&rom);

        case UCON64_XGBXS:
          return gameboy_xgbxs (&rom);

        case UCON64_XSMD:
          return genesis_xsmd (&rom);

        case UCON64_XSMDS:
          return genesis_xsmds (&rom);

        case UCON64_XSWC:
          return snes_xswc (&rom);
  
        case UCON64_XSWCS:
          return snes_xswcs (&rom);
  
        case UCON64_XV64:
          return n64_xv64 (&rom);
#endif  // BACKUP
  
        case UCON64_Z64:
          return n64_z64 (&rom);
#if 0
        case UCON64_MGH:
          return snes_mgh (&rom);
#endif
#ifdef BACKUP
        case UCON64_XFALB:
          return gba_xfalb (&rom, strtol (optarg, NULL, 10));

        case UCON64_XFALC:
          return gba_xfal (&rom, strtol (optarg, NULL, 10));

        case UCON64_XGBXB:
          return gameboy_xgbxb (&rom, strtol (optarg, NULL, 10));
#endif // BACKUP
  
        case UCON64_ATA:
        case UCON64_S16:
        case UCON64_COLECO:
        case UCON64_VBOY:
        case UCON64_SWAN:
        case UCON64_VEC:
        case UCON64_INTELLI:
        case UCON64_LYNX:
        case UCON64_SMS:
        case UCON64_NGP:
        case UCON64_NES:
        case UCON64_PCE:
        case UCON64_JAG:
        case UCON64_GEN:
        case UCON64_NG:
        case UCON64_N64:
        case UCON64_SNES:
        case UCON64_GBA:
        case UCON64_GB:
        case UCON64_SAT:
        case UCON64_PSX:
        case UCON64_PS2:
        case UCON64_CDI:
        case UCON64_CD32:
        case UCON64_3DO:
        case UCON64_DC:
        case UCON64_XBOX:
        case UCON64_GC:
        case UCON64_GP32:
        case UCON64_NS:
        case UCON64_INT:
        case UCON64_NINT:
        case UCON64_HD:
        case UCON64_HDN:
        case UCON64_NHD:
        case UCON64_HI:
        case UCON64_NHI:
        case UCON64_SWP://deprecated
        case UCON64_NSWP://deprecated
        case UCON64_FRONTEND:
        case UCON64_NBAK:
        case UCON64_PORT:
        case UCON64_FILE:
        case UCON64_ROM:
          break;

        case UCON64_VERSION:
          printf ("%s\n", UCON64_VERSION_S);
          return 0;

        case UCON64_GETOPT_ERROR:
        default:
//          fprintf (stderr, "Try '%s " OPTION_LONG_S "help' for more information.\n", argv[0]);
//          return -1;
          break;
      }
    }

  switch (ucon64.show_nfo)
    {
       case UCON64_SHOW_NFO_BEFORE_AND_AFTER:
       case UCON64_SHOW_NFO_AFTER:
         if (!ucon64_init (ucon64.rom, &rom)) ucon64_nfo (&rom);
         break;
    
       case UCON64_SHOW_NFO_BEFORE:
       case UCON64_SHOW_NFO_NEVER:
       default:
         break;
    }

  return 0;
}


int
ucon64_init (const char *romfile, st_rom_t *rominfo)
{
  int result = 0;
  struct stat puffer;

  if (romfile == NULL)
//  flush rominfo
    {
      memset (rominfo, 0L, sizeof (st_rom_t));
      rominfo->console = UCON64_UNKNOWN;
      rominfo->maker = rominfo->country = rominfo->title = rominfo->copier = "";

      return 0;
    }

  if (access (romfile, F_OK | R_OK) == -1) return -1;

  if (!stat (romfile, &puffer) == -1) return -1;
  if (S_ISREG (puffer.st_mode) != TRUE) return -1;
        
  strcpy(rominfo->rom, romfile);

  rominfo->bytes = quickftell (rominfo->rom);

  // The next 4 if-statements MUST precede the call to ucon64_console_probe!
  if (ucon64.buheader_len != -1)
    rominfo->buheader_len = ucon64.buheader_len;

  if (ucon64.interleaved != -1)
    rominfo->interleaved = ucon64.interleaved;

  if (ucon64.splitted != -1)
    rominfo->splitted = ucon64.splitted;

  if (ucon64.snes_hirom != -1)
    rominfo->snes_hirom = ucon64.snes_hirom;

  result = ucon64_console_probe (rominfo);

  if (rominfo->console == UCON64_UNKNOWN && ucon64.show_nfo != UCON64_SHOW_NFO_NEVER)
    {
       printf ("ERROR: could not auto detect the right ROM/console type\n"
               "TIP:   If this is a ROM you might try to force the recognition\n"
               "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n");
       result = -1;
    }

  quickfread (rominfo->buheader, rominfo->buheader_start, rominfo->buheader_len,
              rominfo->rom);
//  quickfread(rominfo->header, rominfo->header_start, rominfo->header_len, rominfo->rom);

  rominfo->mbit = (rominfo->bytes - rominfo->buheader_len) / (float) MBIT;

  if (rominfo->bytes <= MAXROMSIZE)
    {
      rominfo->padded = filetestpad (rominfo->rom);
      rominfo->intro = ((rominfo->bytes - rominfo->buheader_len) > MBIT) ?
        ((rominfo->bytes - rominfo->buheader_len) % MBIT) : 0;

      rominfo->splitted = ucon64_testsplit (rominfo->rom);

      rominfo->current_crc32 = fileCRC32 (rominfo->rom, rominfo->buheader_len);

      if (rominfo->console == UCON64_UNKNOWN)    // don't call if console type is already
        ucon64_dbsearch (rominfo);               //  known (destroys rominfo fields)
    }

  return result;
}


int
ucon64_console_probe (st_rom_t *rominfo)
{
  switch (rominfo->console)
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

      case UCON64_JAGUAR:
        jaguar_init (rominfo);
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

      case UCON64_SYSTEM16:
        system16_init (rominfo);
        break;

      case UCON64_ATARI:
        atari_init (rominfo);
        break;

      case UCON64_NEOGEOPOCKET:
        ngp_init (rominfo);
        break;

      case UCON64_VECTREX:
        vectrex_init (rominfo);
        break;

      case UCON64_VIRTUALBOY:
        vboy_init (rominfo);
        break;

      case UCON64_WONDERSWAN:
        swan_init (rominfo);
        break;

      case UCON64_COLECO:
        coleco_init (rominfo);
        break;

      case UCON64_INTELLI:
        intelli_init (rominfo);
        break;

      case UCON64_PS2:
        ps2_init (rominfo);
        break;

      case UCON64_DC:
        dc_init (rominfo);
        break;

      case UCON64_SATURN:
        saturn_init (rominfo);
        break;

      case UCON64_CDI:
        cdi_init (rominfo);
        break;

      case UCON64_CD32:
        cd32_init (rominfo);
        break;

      case UCON64_PSX:
        psx_init (rominfo);
        break;

      case UCON64_GAMECUBE:
        gamecube_init (rominfo);
        break;

      case UCON64_XBOX:
        xbox_init (rominfo);
        break;

      case UCON64_GP32:
        gp32_init (rominfo);
        break;

      case UCON64_REAL3DO:
        real3do_init (rominfo);
        break;

      case UCON64_UNKNOWN:
        if (rominfo->bytes <= MAXROMSIZE)
          rominfo->console =
#ifdef CONSOLE_PROBE
/*
  these <console>_init() functions can detect the ROM and the correct console
  by significant bytes
*/
            (!nes_init (rominfo)) ? UCON64_NES :
            (!gameboy_init (rominfo)) ? UCON64_GB :
            (!gba_init (rominfo)) ? UCON64_GBA :
            (!snes_init (rominfo)) ? UCON64_SNES :
            (!n64_init (rominfo)) ? UCON64_N64 :
            (!genesis_init (rominfo)) ? UCON64_GENESIS :
            (!psx_init (rominfo)) ? UCON64_PSX ://TODO (rominfo->bytes <= MAXROMSIZE)
            (!jaguar_init (rominfo)) ? UCON64_JAGUAR :
            (!lynx_init (rominfo)) ? UCON64_LYNX :
#if 0
/*
  these <console>_init() still contain no auto-detection or probe code
*/
            (!atari_init (rominfo)) ? UCON64_ATARI :
            (!pcengine_init (rominfo)) ? UCON64_PCE :
            (!neogeo_init (rominfo)) ? UCON64_NEOGEO :
            (!ngp_init (rominfo)) ? UCON64_NEOGEOPOCKET :
            (!sms_init (rominfo)) ? UCON64_SMS :
            (!system16_init (rominfo)) ? UCON64_SYSTEM16 :
            (!vboy_init (rominfo)) ? UCON64_VIRTUALBOY :
            (!vectrex_init (rominfo)) ? UCON64_VECTREX :
            (!coleco_init (rominfo)) ? UCON64_COLECO :
            (!intelli_init (rominfo)) ? UCON64_INTELLI :
            (!swan_init (rominfo)) ? UCON64_WONDERSWAN :
            (!gamecube_init (rominfo)) ? UCON64_GAMECUBE :
            (!xbox_init (rominfo)) ? UCON64_XBOX :
            (!gp32_init (rominfo)) ? UCON64_GP32 :
            (!cd32_init (rominfo)) ? UCON64_CD32 :
            (!cdi_init (rominfo)) ? UCON64_CDI :
            (!dc_init (rominfo)) ? UCON64_DC :
            (!ps2_init (rominfo)) ? UCON64_PS2 :
            (!real3do_init (rominfo)) ? UCON64_REAL3DO :
            (!saturn_init (rominfo)) ? UCON64_SATURN :
#endif
#endif // CONSOLE_PROBE
                UCON64_UNKNOWN;
            return (rominfo->console == UCON64_UNKNOWN) ? -1 : 0;

        default:
            rominfo->console = UCON64_UNKNOWN;
            return -1;
        }
  return 0;
}


/*
    this is the now centralized nfo output for all kinds of ROMs
*/
int
ucon64_nfo (const st_rom_t *rominfo)
{
  char buf[4096];

  printf ("%s\n\n", rominfo->rom);

#if 0
  if (rominfo->buheader_len)
    {
      strhexdump (rominfo->buheader, 0, rominfo->buheader_start,
                  rominfo->buheader_len);
      printf ("\n");
    }
#endif

  printf ("%s\n\n", rominfo->copier);

  if (rominfo->header_len)
    {
      strhexdump (rominfo->header, 0, rominfo->header_start + rominfo->buheader_len,
                  rominfo->header_len);
      printf ("\n");
    }

  strcpy(buf, rominfo->name);
  mkprint (buf, '.');  // some ROMs have a name with control chars in it -> replace control chars

  printf ("%s\n%s\n%s%s%s\n%s\n%ld bytes (%.4f Mb)\n\n", rominfo->title, buf,
          "", "",
          rominfo->maker,
          rominfo->country, rominfo->bytes - rominfo->buheader_len, rominfo->mbit);

  if (rominfo->bytes <= MAXROMSIZE) // if it is no CD image
    {
      if (!rominfo->padded)
        printf ("Padded: No\n");
      else if (rominfo->padded)
        printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n", rominfo->padded,
                (float) rominfo->padded / MBIT);

//    if (!rominfo->intro)
//      printf("Intro/Trainer: No\n");
//    else
      if (rominfo->intro)
        printf ("Intro/Trainer: Maybe, %ld Bytes\n", rominfo->intro);

      if (!rominfo->buheader_len)
        printf ("Backup unit/Emulator header: No\n");    // printing this is handy for
      else if (rominfo->buheader_len)            //  SNES ROMs
        printf ("Backup unit/Emulator header: Yes, %ld Bytes\n", rominfo->buheader_len);

//    if (!rominfo->splitted)
//      printf("Splitted: No\n");
//    else
      if (rominfo->splitted)
        printf ("Splitted: Yes, %d parts\n",
                rominfo->splitted);
    }
  if (rominfo->misc[0])
    printf ("%s\n", rominfo->misc);

  if (rominfo->has_internal_crc)
    {
      sprintf (buf,
               "Checksum: %%s, 0x%%0%dlx (calculated) %%s= 0x%%0%dlx (internal)\n",
               rominfo->internal_crc_len * 2, rominfo->internal_crc_len * 2);
      printf (buf,
              (rominfo->current_internal_crc ==
               rominfo->internal_crc) ? "ok" : "bad",
              rominfo->current_internal_crc,
              (rominfo->current_internal_crc == rominfo->internal_crc) ? "=" : "!",
              rominfo->internal_crc);

      if (rominfo->internal_crc2[0])
        printf ("%s\n", rominfo->internal_crc2);
    }

  if (rominfo->current_crc32 != 0)
    printf ("Checksum (CRC32): 0x%08lx\n", rominfo->current_crc32);

  if (rominfo->splitted)
    printf ("NOTE: to get the correct checksum the ROM must be joined\n");

  printf ("\n");

  return 0;
}


int ucon64_e (const st_rom_t *rominfo)
{
  int result, x;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[4096];
  const char *property;

  if (rominfo->console == UCON64_UNKNOWN)
    {
       printf ("ERROR: could not auto detect the right ROM/console type\n"
               "TIP:   If this is a ROM you might try to force the recognition\n"
               "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n");
       return -1;
    }
 
  x = 0;
  while (long_options[x].name)
    {
      if (long_options[x].val == rominfo->console)
      {
        sprintf (buf3, "emulate_%s", long_options[x].name);
        break;
      }
      x++;
    }

  if (access (ucon64.configfile, F_OK) != 0)
    {
      printf ("ERROR: %s does not exist\n", ucon64.configfile);
      return -1;
    }

  property = getProperty (ucon64.configfile, buf3, buf2, NULL);   // buf2 also contains property value
  if (property == NULL)
    {
      printf ("ERROR: could not find the correct settings (%s) in\n"
              "       %s\n"
              "TIP:   If the wrong console was detected you might try to force recognition\n"
              "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n",
              buf3, ucon64.configfile);
      return -1;
    }

  sprintf (buf, "%s %s", buf2, ucon64.file);

  printf ("%s\n", buf);
  fflush (stdout);
  sync ();

  result = system (buf);
#ifndef __MSDOS__
  result >>= 8;                  // the exit code is coded in bits 8-15
#endif                          //  (that is, under Unix & BeOS)

#if 1
  // Snes9x (Linux) for example returns a non-zero value on a normal exit
  //  (3)...
  // under WinDOS, system() immediately returns with exit code 0 when
  //  starting a Windows executable (as if fork() was called) it also
  //  returns 0 when the exe could not be started
  if (result != 127 && result != -1 && result != 0)        // 127 && -1 are system() errors, rest are exit codes
    {
      printf ("ERROR: the Emulator returned an error code (%d)\n"
              "TIP:   If the wrong emulator was used you might try to force recognition\n"
              "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n",
              (int) result);
    }
#endif
  return result;
}


int ucon64_ls (st_rom_t *rominfo, int mode)
{
//  int single_file = 0;
  int forced_console;
  char dir[FILENAME_MAX], buf[MAXBUFSIZE];
  struct dirent *ep;
  struct stat puffer;
  DIR *dp;

//TODO dir or single file?

  strcpy (dir, rominfo->rom);

  if (stat (rominfo->rom, &puffer) == -1)
    getcwd (dir, FILENAME_MAX);
  else if (S_ISDIR (puffer.st_mode) != TRUE)
    getcwd (dir, FILENAME_MAX);

  if ((dp = opendir (dir)) == NULL)
    return -1;

  getcwd (dir, FILENAME_MAX);
  chdir (rominfo->rom);

  forced_console = rominfo->console;
#define UCON64_LS_SAVE
#ifdef UCON64_LS_SAVE
  while ((ep = readdir (dp)) != 0)
    {
      if (!stat (ep->d_name, &puffer))
        {
          if (S_ISREG (puffer.st_mode))
            {
#else
  while ((((ep = readdir (dp)) != 0) &&
          !stat (ep->d_name, &puffer)) &&
          S_ISREG (puffer.st_mode))
    {
#endif // UCON64_LS_SAVE
              // ucon64_init(NULL,...) sets rominfo->rom to UCON64_UNKNOWN, but
	      //  we have to remember a possible force recoginition option
              if (forced_console == UCON64_UNKNOWN)
                ucon64_init (NULL, rominfo);
              else
                rominfo->console = forced_console;
              if (ucon64_init (ep->d_name, rominfo) != -1)
                switch (mode)
                  {
                    case UCON64_LSV:
                      ucon64_nfo (rominfo);
                      fflush (stdout);
                      break;
/*TODO renamer!
                    case UCON64_REN:
                      if (rominfo->console != UCON64_UNKNOWN)
//                        && rominfo->console != UCON64_KNOWN)
                        {
                          strcpy (buf, &ucon64.rom[findlast (ucon64.rom, ".") + 1]);
                          printf ("%s.%s\n", rominfo->name, buf);
                        }
                      break;
*/
                    default:
                    case UCON64_LS:
                      strftime (buf, 13, "%b %d %H:%M",
                                localtime (&puffer.st_mtime));
//                      printf ("%-31.31s %10d %s %s\n", rom.name,
//                              (int) puffer.st_size, buf, ucon64.rom);
                      printf ("%-31.31s %10d %s %s\n", mkprint(rominfo->name, ' '),
                              (int) puffer.st_size, buf, ucon64.rom);
                      fflush (stdout);
                      break;
                }
            }
#ifdef UCON64_LS_SAVE
        }
    }
#endif // UCON64_LS_SAVE
  closedir (dp);

  chdir (dir);

  return 0;
}


int
ucon64_configfile (void)
{
  char buf2[MAXBUFSIZE];
/*
  configfile handling
*/
  sprintf (ucon64.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
  "ucon64.cfg"
#else
  /*
    Use getchd() here. If this code gets compiled under Windows the compiler
    has to be Cygwin. So, uCON64 will be a Win32 executable which will run only
    in an environment where long filenames are available and where files can
    have more than three characters as "extension". Under Bash HOME will be
    set, but most Windows people will propably run uCON64 under cmd or command
    where HOME is not set by default. Under Windows XP/2000 (/NT?) USERPROFILE
    and/or HOMEDRIVE+HOMEPATH will be set which getchd() will "detect".
  */
  ".ucon64rc"
#endif
  , ms_getenv ("HOME"));

  if (access (ucon64.configfile, F_OK) != 0)
    {
      FILE *fh;

      printf ("WARNING: %s not found: creating...", ucon64.configfile);

      if (!(fh = fopen (ucon64.configfile, "wb")))
        {
          printf ("FAILED\n\n");

//          return -1;
        }
      else
        {
          fputs ("# uCON64 config\n"
                 "#\n"
                 "version=198\n"
                 "#\n"
                 "# create backups of files? (1=yes; 0=no)\n"
//                 "# before processing a ROM uCON64 will make a backup of it\n"
                 "#\n"
                 "backups=1\n"
                 "#\n"
                 "# emulate_<console shortcut>=<emulator with options>\n"
                 "#\n"
                 "emulate_gb=vgb -sound -sync 50 -sgb -scale 2\n"
                 "emulate_gen=dgen -f -S 2\n"
                 "emulate_sms=\n"
                 "emulate_jag=\n"
                 "emulate_lynx=\n"
                 "emulate_n64=\n"
                 "emulate_ng=\n"
                 "emulate_nes=tuxnes -E2 -rx11 -v -s/dev/dsp -R44100\n"
                 "emulate_pce=\n"
                 "emulate_snes=snes9x -tr -fs -sc -hires -dfr -r 7 -is -j\n"
                 "emulate_ngp=\n"
                 "emulate_ata=\n"
                 "emulate_s16=\n"
                 "emulate_gba=vgba -scale 2 -uperiod 6\n"
                 "emulate_vec=\n"
                 "emulate_vboy=\n"
                 "emulate_swan=\n"
                 "emulate_coleco=\n"
                 "emulate_intelli=\n"
                 "emulate_psx=pcsx\n"
                 "emulate_ps2=\n"
                 "emulate_sat=\n"
                 "emulate_dc=\n"
                 "emulate_cd32=\n"
                 "emulate_cdi=\n"
                 "emulate_3do=\n"
                 "emulate_gp32=\n"
#ifdef BACKUP_CD
                 "#\n"
                 "# uCON64 can operate as frontend for CD burning software to make backups\n"
                 "# for CD-based consoles \n"
                 "#\n"
                 "# We suggest cdrdao (http://cdrdao.sourceforge.net) as burn engine for uCON64\n"
                 "# Make sure you check this configfile for the right settings\n"
                 "#\n"
                 "# --device [bus,id,lun] (cdrdao)\n"
                 "#\n"
                 "cdrw_read=cdrdao read-cd --read-raw --device 0,0,0 --driver generic-mmc-raw --datafile #bin and toc filenames are added by ucon64 at the end\n"
                 "cdrw_write=cdrdao write --device 0,0,0 --driver generic-mmc #toc filename is added by ucon64 at the end\n"
#endif
                 , fh);

          fclose (fh);
          printf ("OK\n\n");
        }
    }
  else if (strcmp (getProperty (ucon64.configfile, "version", buf2, "198"), "198") != 0)
    {
      strcpy (buf2, ucon64.configfile);
      setext (buf2, ".OLD");

      printf ("NOTE: updating config: old version will be renamed to %s...", buf2);

      filecopy (ucon64.configfile, 0, quickftell (ucon64.configfile), buf2, "wb");

      setProperty (ucon64.configfile, "version", "198");

      setProperty (ucon64.configfile, "backups", "1");

      setProperty (ucon64.configfile, "emulate_gp32", "");

      setProperty (ucon64.configfile, "cdrw_read",
        getProperty (ucon64.configfile, "cdrw_raw_read", buf2, "cdrdao read-cd --read-raw --device 0,0,0 --driver generic-mmc-raw --datafile "));
      setProperty (ucon64.configfile, "cdrw_write",
        getProperty (ucon64.configfile, "cdrw_raw_write", buf2, "cdrdao write --device 0,0,0 --driver generic-mmc "));

      deleteProperty (ucon64.configfile, "cdrw_raw_read");
      deleteProperty (ucon64.configfile, "cdrw_raw_write");
      deleteProperty (ucon64.configfile, "cdrw_iso_read");
      deleteProperty (ucon64.configfile, "cdrw_iso_write");

      sync ();
      printf ("OK\n\n");
    }
  return 0;
}


void
ucon64_usage (int argc, char *argv[])
{
  int c = 0;
  int option_index = 0;
  int single = 0;

  printf ("Usage: %s [OPTION]... [" OPTION_LONG_S "rom=]ROM [[" OPTION_LONG_S "file=]FILE]\n\n"
           "  " OPTION_LONG_S "nbak        prevents backup files (*.BAK)\n"
           "  " OPTION_LONG_S "hdn=BYTES   force ROM has backup unit/emulator header with BYTES size\n"
           "  " OPTION_LONG_S "hd          same as " OPTION_LONG_S "hdn=512\n"
//           "TODO: " OPTION_LONG_S "hdn=TYPE     force ROM has TYPE backup unit/emulator header:\n"
//           "                  FFE, SMC, FIG, SWC, SMD, SMG, SSC or LNX\n"
           "                  most backup units use a header with 512 Bytes size\n"
           "  " OPTION_LONG_S "nhd         force ROM has no backup unit/emulator header\n"
           "  " OPTION_LONG_S "stp         strip header from ROM; default size 512 Bytes\n"
           "  " OPTION_LONG_S "ins         insert empty header before ROM; default size 512 Bytes\n"
           "  " OPTION_LONG_S "strip       strip Bytes from end of ROM; " OPTION_LONG_S "file=VALUE\n"
           "  " OPTION_LONG_S "int         force ROM is interleaved (2143)\n"
           "  " OPTION_LONG_S "nint        force ROM is not interleaved (1234)\n"
           "  " OPTION_LONG_S "dint        convert ROM to (non-)interleaved format (1234 <-> 2143)\n"
           "                  this differs from the Super Nintendo " OPTION_LONG_S "dint option\n"
           "  " OPTION_LONG_S "ns          force ROM is not splitted\n"
#ifdef	__MSDOS__
           "  " OPTION_S "e           emulate/run ROM (see %s for more)\n"
#else
           "  " OPTION_S "e           emulate/run ROM (see %s for more)\n"
#endif
           "  " OPTION_LONG_S "crc         show CRC32 value of ROM\n"
           "  " OPTION_LONG_S "dbs         search ROM database (all entries) by CRC32; " OPTION_LONG_S "rom=0xCRC32\n"
           "  " OPTION_LONG_S "db          ROM database statistics (# of entries)\n"
           "  " OPTION_LONG_S "dbv         view ROM database (all entries)\n"
           "  " OPTION_LONG_S "ls          generate ROM list for all ROMs; " OPTION_LONG_S "rom=DIRECTORY\n"
           "  " OPTION_LONG_S "lsv         like " OPTION_LONG_S "ls but more verbose; " OPTION_LONG_S "rom=DIRECTORY\n"
//         "TODO:  " OPTION_LONG_S "rrom   rename all ROMs in DIRECTORY to their internal names; " OPTION_LONG_S "rom=DIR\n"
//         "TODO:  " OPTION_LONG_S "rr83   like " OPTION_LONG_S "rrom but with 8.3 filenames; " OPTION_LONG_S "rom=DIR\n"
//         "               this is often used by people who loose control of their ROMs\n"
           "  " OPTION_LONG_S "rl          rename all files in DIRECTORY to lowercase; " OPTION_LONG_S "rom=DIRECTORY\n"
           "  " OPTION_LONG_S "ru          rename all files in DIRECTORY to uppercase; " OPTION_LONG_S "rom=DIRECTORY\n"
#ifdef	__MSDOS__
           "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex " OPTION_LONG_S "rom=ROM|more\"\n"
#else
           "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex " OPTION_LONG_S "rom=ROM|less\"\n"       // less is better ;-)
#endif
           "  " OPTION_LONG_S "find        find string in ROM; " OPTION_LONG_S "file=STRING ('?'==wildcard for ONE char!)\n"
           "  " OPTION_S "c           compare ROMs for differencies; " OPTION_LONG_S "file=OTHER_ROM\n"
           "  " OPTION_LONG_S "cs          compare ROMs for similarities; " OPTION_LONG_S "file=OTHER_ROM\n"
           "  " OPTION_LONG_S "ispad       check if ROM is padded\n"
           "  " OPTION_LONG_S "pad         pad ROM to full Mb\n"
           , argv[0], ucon64.configfile);

  bsl_usage ();
  ips_usage ();
  aps_usage ();
  pal4u_usage ();
  ppf_usage ();
  xps_usage ();

  cdrw_usage ();

  printf (           "  " OPTION_LONG_S "help        display this help and exit\n"
           "  " OPTION_LONG_S "version     output version information and exit\n"
"\n");

  optind = option_index = 0;//TODO is there a better way to "reset"?

  single = 0;

  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      if (single) break;
      
      switch (c)
        {
      case UCON64_GBA:
        gba_usage ();
        single = 1;
        break;
      case UCON64_N64:
        n64_usage ();
        single = 1;
        break;
      case UCON64_JAG:
        jaguar_usage ();
        single = 1;
        break;
      case UCON64_SNES:
        snes_usage ();
        single = 1;
        break;
      case UCON64_NG:
        neogeo_usage ();
        single = 1;
        break;
      case UCON64_NGP:
        ngp_usage ();
        single = 1;
        break;
      case UCON64_GEN:
        genesis_usage ();
        single = 1;
        break;
      case UCON64_GB:
        gameboy_usage ();
        single = 1;
        break;
      case UCON64_LYNX:
        lynx_usage ();
        single = 1;
        break;
      case UCON64_PCE:
        pcengine_usage ();
        single = 1;
        break;
      case UCON64_SMS:
        sms_usage ();
        single = 1;
        break;
      case UCON64_NES:
        nes_usage ();
        single = 1;
        break;
      case UCON64_S16:
        sys16_usage ();
        single = 1;
        break;
      case UCON64_ATA:
        atari_usage ();
        single = 1;
        break;
      case UCON64_COLECO:
        coleco_usage ();
        single = 1;
        break;
      case UCON64_VBOY:
        vboy_usage ();
        single = 1;
        break;
      case UCON64_SWAN:
        swan_usage ();
        single = 1;
        break;
      case UCON64_VEC:
        vectrex_usage ();
        single = 1;
        break;
      case UCON64_INTELLI:
        intelli_usage ();
        single = 1;
        break;
      case UCON64_DC:
        dc_usage ();
        single = 1;
        break;
      case UCON64_PSX:
        psx_usage ();
        single = 1;
        break;
      case UCON64_PS2:
        ps2_usage ();
        single = 1;
        break;
      case UCON64_SAT:
        saturn_usage ();
        single = 1;
        break;
      case UCON64_3DO:
        real3do_usage ();
        single = 1;
        break;
      case UCON64_CD32:
        cd32_usage ();
        single = 1;
        break;
      case UCON64_CDI:
        cdi_usage ();
        single = 1;
        break;
      case UCON64_GC:
        gamecube_usage ();
        single = 1;
        break;
      case UCON64_XBOX:
        xbox_usage ();
        single = 1;
        break;
      case UCON64_GP32:
        gp32_usage ();
        single = 1;
        break;
  
      default:
        break;
      }
    }

  if (!single)
    {
      dc_usage ();
      gba_usage ();
      n64_usage ();
      snes_usage ();
      neogeo_usage ();
      genesis_usage ();
      gameboy_usage ();
      lynx_usage ();
      pcengine_usage ();
      sms_usage ();
      nes_usage ();
      swan_usage ();

#if 0
      psx_usage ();
      ps2_usage ();
      xbox_usage ();
      sat_usage ();
      3do_usage ();
      cd32_usage ();
      cdi_usage ();
#endif
#if 0
      sys16_usage ();
      atari_usage ();
      coleco_usage ();
      vboy_usage ();
      swan_usage ();
      vectrex_usage ();
      intelli_usage ();
      jaguar_usage ();
      ngp_usage ();
      gamecube_usage ();
#endif

      printf ("%s\n%s\n%s\n%s\n%s\n%s\n%s\n"
              "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n"
              "  " 
              OPTION_LONG_S "gc, "
              OPTION_LONG_S "jag, "
              OPTION_LONG_S "ngp, "
              OPTION_LONG_S "s16, "
              OPTION_LONG_S "ata, "
              OPTION_LONG_S "coleco, " 
              OPTION_LONG_S "vboy, "
              OPTION_LONG_S "vec, "
              OPTION_LONG_S "intelli, "
              OPTION_LONG_S "gp32\n"
              "                force recognition"
#ifndef DB
              "; NEEDED"
#endif
              "\n                  these consoles have no specific options\n"
              "                  see the miscellaneous options at the top for more\n"
              "  "
              OPTION_LONG_S "psx, " 
              OPTION_LONG_S "ps2, " 
              OPTION_LONG_S "xbox, " 
              OPTION_LONG_S "sat, " 
              OPTION_LONG_S "3do, " 
              OPTION_LONG_S "cd32, " 
              OPTION_LONG_S "cdi\n"
              "                force recognition; NEEDED\n"
              "                  these consoles have no specific options\n"
              "                  see the miscellaneous options at the top for more\n"
//              "                  uCON64 can make backups for most CD-based consoles\n"
              , gamecube_title,
              jaguar_title,
              ngp_title,
              system16_title,
              atari_title, coleco_title, vboy_title,
              vectrex_title, intelli_title, gp32_title,
              psx_title, ps2_title,
              xbox_title,
              saturn_title, real3do_title, cd32_title,
              cdi_title
              );

              cdrw_usage ();

              printf("\n");
    }

  printf ("Database: %ld known ROMs in db.h (%+ld)\n\n"
     "TIP: %s " OPTION_LONG_S "help " OPTION_LONG_S "snes (would show only Super Nintendo related help)\n"
#ifdef	__MSDOS__
     "     %s " OPTION_LONG_S "help|more (to see everything in more)\n"
#else
     "     %s " OPTION_LONG_S "help|less (to see everything in less)\n" // less is better ;-)
#endif
     "     give the force recognition option a try if something went wrong\n"
     "\n"
     "Report problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net\n"
     "\n"
     , ucon64_dbsize (UCON64_UNKNOWN)
     , ucon64_dbsize (UCON64_UNKNOWN) - UCON64_DBSIZE
     , argv[0], argv[0]
   );

#if 0
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
#endif
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
 Snes/Sega/GameBoy/GameGear/Ultra 64/PSX/Jaguar/Saturn/Engine/Lynx/NeoGeo
- -- ---------------------------------------------------------------- -- -
*/
