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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
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
#include "n64/n64.h"
#include "lynx/lynx.h"
#include "sms/sms.h"
#include "nes/nes.h"
#include "genesis/genesis.h"
#include "pce/pce.h"
#include "neogeo/neogeo.h"
#include "ngp/ngp.h"
#include "swan/swan.h"
#include "dc/dc.h"
#include "jaguar/jaguar.h"
#include "sample/sample.h"

#include "patch/ppf.h"
#include "patch/xps.h"
#include "patch/pal4u.h"
#include "patch/aps.h"
#include "patch/ips.h"
#include "patch/bsl.h"
#include "patch/gg.h"

#include "backup/fig.h"
#include "backup/swc.h"
#include "backup/cdrw.h"
#include "backup/doctor64jr.h"
#include "backup/doctor64.h"
#include "backup/smd.h"
#include "backup/fal.h"
#include "backup/gbx.h"
#include "backup/cd64.h"
#include "backup/dex.h"
#include "backup/smc.h"
#include "backup/fpl.h"
#include "backup/mgd.h"

static void ucon64_exit (void);
static void usage (const char **usage);
static void ucon64_usage (int argc, char *argv[]);

st_ucon64_t ucon64;

static const char *ucon64_title = "uCON64 " UCON64_VERSION_S " " UCON64_OS 
                              " 1999-2002 by (various)";

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
main (int argc, char **argv)
{
  long x, y = 0;
  int ucon64_argc;
  int c = 0, result = 0;
  unsigned long padded;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE];
  const char *ucon64_argv[128];
  int option_index = 0;
  st_rominfo_t rom;

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
  ucon64.snes_hirom = 
  ucon64.console = UCON64_UNKNOWN;

  ucon64.rom =
  ucon64.file = "";

  sscanf (getProperty (ucon64.configfile, "parport", buf2, "0x378"), "%x", &ucon64.parport);

  ucon64.backup = ((!strcmp (getProperty (ucon64.configfile, "backups", buf2, "1"), "1")) ?
               1 : 0);

  if (argc < 2)
    {
       ucon64_usage (argc, argv);
       return 0;
    }

  ucon64.argc = argc;
  ucon64.argv = argv;

  ucon64_flush (&rom);

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
          rom.copier_usage = unknown_usage;
          break;

        case UCON64_HDN:
          if (strtol (optarg, NULL, 10) > MAXBUFSIZE)
            fprintf (stderr, "ERROR: BYTES > %d does not work\n\n", MAXBUFSIZE);
          else
            ucon64.buheader_len = strtol (optarg, NULL, 10);
          rom.copier_usage = unknown_usage;
          break;

        case UCON64_NHD:
          ucon64.buheader_len = 0;
          rom.copier_usage = unknown_usage;
          break;

        case UCON64_SWP://deprecated
        case UCON64_INT:
          ucon64.interleaved = 1;
          break;

        case UCON64_INT2:
          ucon64.interleaved = 2;
          break;

        case UCON64_NSWP://deprecated
        case UCON64_NINT:
          ucon64.interleaved = 0;
          break;

        case UCON64_PORT:
#ifdef BACKUP
          if (optarg)
            sscanf (optarg, "%x", &ucon64.parport);
#endif // BACKUP            
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
        case UCON64_CS:
        case UCON64_DB:
        case UCON64_DBS:
        case UCON64_DBV:
        case UCON64_DINT:
        case UCON64_SWAP:
        case UCON64_E:
          break;
          
        case UCON64_CRC:
        case UCON64_CRCHD://deprecated
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
          ucon64.console = UCON64_SNES;
          break;

        case UCON64_HI:
          ucon64.snes_hirom = 1;
          ucon64.console = UCON64_SNES;
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
          ucon64.console = UCON64_SNES;
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
          ucon64.console = UCON64_GB;
          break;

        case UCON64_ATA:
          ucon64.console = UCON64_ATARI;
          break;
  
        case UCON64_S16:
          ucon64.console = UCON64_SYSTEM16;
          break;

        case UCON64_COLECO:
          ucon64.console = UCON64_COLECO;
          break;
  
        case UCON64_VBOY:
          ucon64.console = UCON64_VIRTUALBOY;
          break;
  
        case UCON64_SWAN:
          ucon64.console = UCON64_WONDERSWAN;
          break;

        case UCON64_VEC:
          ucon64.console = UCON64_VECTREX;
          break;

        case UCON64_INTELLI:
          ucon64.console = UCON64_INTELLI;
          break;
  
        case UCON64_LYNX:
        case UCON64_B0:
        case UCON64_B1:
        case UCON64_ROTL:
        case UCON64_ROTR:
        case UCON64_NROT:
        case UCON64_LNX:
        case UCON64_LYX:
          ucon64.console = UCON64_LYNX;
          break;
  
        case UCON64_SMS:
          ucon64.console = UCON64_SMS;
          break;

        case UCON64_NGP:
          ucon64.console = UCON64_NEOGEOPOCKET;
          break;

        case UCON64_INES:
        case UCON64_INESHD:
        case UCON64_NES:
//        case UCON64_FDS:
//        case UCON64_FDSL:
//        case UCON64_PAS:
        case UCON64_FFE:
        case UCON64_UNIF:
          ucon64.console = UCON64_NES;
          break;
  
        case UCON64_SMG:
        case UCON64_PCE:
          ucon64.console = UCON64_PCE;
          break;
  
        case UCON64_JAG:
          ucon64.console = UCON64_JAGUAR;
          break;
  
        case UCON64_GEN:
        case UCON64_N2:
        case UCON64_1991:
          ucon64.console = UCON64_GENESIS;
          break;
  
        case UCON64_NG:
        case UCON64_SAM:
        case UCON64_MVS:
        case UCON64_BIOS:
          ucon64.console = UCON64_NEOGEO;
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
          ucon64.console = UCON64_N64;
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
          ucon64.console = UCON64_GBA;
          break;

        case UCON64_SAT:
          ucon64.console = UCON64_SATURN;
          break;
  
        case UCON64_PSX:
          ucon64.console = UCON64_PSX;
          break;
  
        case UCON64_PS2:
          ucon64.console = UCON64_PS2;
          break;

        case UCON64_CDI:
          ucon64.console = UCON64_CDI;
          break;

        case UCON64_CD32:
          ucon64.console = UCON64_CD32;
          break;
  
        case UCON64_3DO:
          ucon64.console = UCON64_REAL3DO;
          break;

        case UCON64_IP:
        case UCON64_DC:
          ucon64.console = UCON64_DC;
          break;

        case UCON64_XBOX:
          ucon64.console = UCON64_XBOX;
          break;

        case UCON64_GC:
          ucon64.console = UCON64_GAMECUBE;
          break;

        case UCON64_GP32:
          ucon64.console = UCON64_GP32;
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
          printf ("Checksum (CRC32): 0x%08lx\n\n", fileCRC32 (ucon64.rom, rom.buheader_len));
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
              fprintf (stderr, "ERROR: file not found/out of memory\n");
              return -1;
            }
          return 0;

        case UCON64_CS:
          if (filefile (ucon64.rom, 0, ucon64.file, 0, TRUE) == -1)
            {
              fprintf (stderr, "ERROR: file not found/out of memory\n");
              return -1;
            }
          return 0;
  
        case UCON64_FIND:
          x = 0;
          y = quickftell (ucon64.rom);
          while ((x = filencmp2 (ucon64.rom, x, y, ucon64.file, 
            strlen (ucon64.file), '?')) != -1)
            {
              filehexdump (ucon64.rom, x, strlen (ucon64.file));
              x++;
              printf ("\n");
            }
          return 0;

        case UCON64_PADHD://deprecated
          rom.buheader_len = UNKNOWN_HEADER_LEN;
        case UCON64_P:
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
          return truncate (ucon64_fbackup (ucon64.rom),
            quickftell (ucon64.rom) - atol (ucon64.file));

        case UCON64_STP:
          strcpy (buf, ucon64.rom);
          setext (buf, ".BAK");
          remove (buf);           // try to remove or rename will fail
          printf ("Writing backup of: %s\n", ucon64.rom);
          fflush (stdout);
          rename (ucon64.rom, buf);
          return filecopy (buf, 512, quickftell (buf), ucon64.rom, "wb");
  
        case UCON64_INS:
          strcpy (buf, ucon64.rom);
          setext (buf, ".BAK");
          remove (buf);           // try to remove or rename will fail
          printf ("Writing backup of: %s\n", ucon64.rom);
          fflush (stdout);
          rename (ucon64.rom, buf);
          memset (buf2, 0, 512);
          quickfwrite (buf2, 0, 512, ucon64.rom, "wb");
          return filecopy (buf, 0, quickftell (buf), ucon64.rom, "ab");
  
        case UCON64_B:
          if ((result = bsl (ucon64_fbackup (ucon64.rom), ucon64.file)) != 0)
            fprintf (stderr, "ERROR: failed\n");
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
          return ucon64_ls (ucon64.rom, UCON64_LS);
  
        case UCON64_LSV:
          return ucon64_ls (ucon64.rom, UCON64_LSV);
  
        case UCON64_REN:
          return ucon64_ls (ucon64.rom, UCON64_REN);
  
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
          switch (ucon64.console)
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
                  ucon64_dbsize (ucon64.console),
                  ucon64_dbsize (ucon64.console) - UCON64_DBSIZE,
                  argv[0]);
          break;

        case UCON64_DBS:
          ucon64_flush (&rom);
          sscanf (ucon64.rom, "%lx", &rom.current_crc32);
          ucon64_dbsearch (&rom);
          ucon64_nfo (&rom);
//          ucon64_dbview (ucon64.console);
          printf ("TIP: %s "OPTION_LONG_S "dbs "OPTION_LONG_S "nes would search only for a NES ROM\n\n",
                  argv[0]);
  
          break;
  
        case UCON64_DBV:
          ucon64_dbview (ucon64.console);
  
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
          return ucon64_e (ucon64.rom);

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
          switch (ucon64.console)
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
          switch (ucon64.console)
            {
            case UCON64_SNES:
              return snes_dint (&rom);

            default:
              result = fileswap (ucon64_fbackup (ucon64.rom), 0,
                           quickftell (ucon64.rom));
              printf ("Wrote output to: %s\n", ucon64.rom);
              return result;
            }
  
        case UCON64_F:
          switch (ucon64.console)
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
#if 0
        case UCON64_GG:
          switch (ucon64.console)
            {
            case UCON64_GB:
              return gameboy_gg (&rom);
//            case UCON64_GENESIS:
//              return
            case UCON64_NES:
              return nes_gg (&rom);
            case UCON64_SNES:
              return snes_gg (&rom);
            default:
//TODO error
              return -1;
            }
#endif  

        case UCON64_GGD:
          return gg_ggd ();
  
        case UCON64_GGE:
          return gg_gge ();

        case UCON64_INES:
          return nes_ines (&rom);
  
        case UCON64_INESHD:
          return nes_ineshd (&rom);
  
#if 0
        case UCON64_IP:
          break;
#endif
  
        case UCON64_J:
          switch (ucon64.console)
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
          switch (ucon64.console)
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
          switch (ucon64.console)
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

        case UCON64_PAS:
          return nes_pas (&rom);
  
        case UCON64_ROTL:
          return lynx_rotl (&rom);
  
        case UCON64_ROTR:
          return lynx_rotr (&rom);

        case UCON64_S:
          switch (ucon64.console)
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
          switch (ucon64.console)
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
          if (!access (ucon64.rom, F_OK))
            {
              if (doctor64jr_write (ucon64.rom, rom.buheader_len,
                  quickftell (ucon64.rom), ucon64.parport) != 0)
                {
                  fprintf (stderr, ucon64_parport_error);
                  return -1;
                }
            }
          else
            {
              if (doctor64jr_read (ucon64.rom, ucon64.parport) != 0)
                {
                  fprintf (stderr, ucon64_parport_error);
                  return -1;
                }
            }
          printf ("\n");
          return 0;
  
        case UCON64_XSMD:
          if (access (ucon64.rom, F_OK) != 0)     // file does not exist -> dump cartridge
            smd_read_rom (ucon64.rom, ucon64.parport);
          else                          // file exists -> send it to the copier
            {
              if (!rom.buheader_len)
                {
                  fprintf (stderr, "ERROR: this ROM has no header. Convert to an SMD compatible format.\n");
                  return -1;
                }
              smd_write_rom (ucon64.rom, ucon64.parport);
            }
          printf ("\n");
          return 0;

        case UCON64_XSMDS:
          if (access (ucon64.rom, F_OK) != 0)     // file does not exist -> dump SRAM contents
            smd_read_sram (ucon64.rom, ucon64.parport);
          else                          // file exists -> restore SRAM
            smd_write_sram (ucon64.rom, ucon64.parport);
          printf ("\n");
          return 0;

        case UCON64_XSWC:
          if (access (ucon64.rom, F_OK) != 0) // file does not exist -> dump cartridge
            swc_read_rom (ucon64.rom, ucon64.parport);
          else
            {
              if (!rom.buheader_len)
                {
                  fprintf (stderr, "ERROR: this ROM has no header. Convert to an SWC compatible format.\n");
                  return -1;
                }

              if (rom.interleaved)
                {
                  fprintf (stderr, "ERROR: this ROM seems to be interleaved but the SWC doesn't support\n"
                    "       interleaved ROMs\n");
                  return -1;
                }
              swc_write_rom (ucon64.rom, ucon64.parport); // file exists -> send it to the copier
            }
          return 0;
  
        case UCON64_XSWCS:
          if (access (ucon64.rom, F_OK) != 0)         // file does not exist -> dump SRAM contents
            {
              result = swc_read_sram (ucon64.rom, ucon64.parport);
            }
          else
            {
              result = swc_write_sram (ucon64.rom, ucon64.parport);// file exists -> restore SRAM
            }
          printf ("\n");
          return result;
  
        case UCON64_XV64:
          if (!access (ucon64.rom, F_OK))
            {
              if (doctor64_write (ucon64.rom, rom.buheader_len,
                  quickftell (ucon64.rom), ucon64.parport) != 0)
                {
                  fprintf (stderr, ucon64_parport_error);
                  return -1;
                }
            }
          else
            {
              if (doctor64_read (ucon64.rom, ucon64.parport) != 0)
                {
                  fprintf (stderr, ucon64_parport_error);
                  return -1;
                }
            }
          return 0;

        case UCON64_XFALB:
          if (access (ucon64.rom, F_OK) != 0)
            fal_read_sram (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
          else
            fal_write_sram (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
          printf ("\n");
          return 0;

        case UCON64_XFALC:
          if (access (ucon64.rom, F_OK) != 0)
            fal_read_rom (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
          else
            fal_write_rom (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
          printf ("\n");
          return 0;

        case UCON64_XFAL:
          if (access (ucon64.rom, F_OK) != 0)
            fal_read_rom (ucon64.rom, ucon64.parport, -1);
          else
            fal_write_rom (ucon64.rom, ucon64.parport, -1);
          printf ("\n");
          return 0;

        case UCON64_XFALS:
          if (access (ucon64.rom, F_OK) != 0)
            fal_read_sram (ucon64.rom, ucon64.parport, -1);
          else
            fal_write_sram (ucon64.rom, ucon64.parport, -1);
          printf ("\n");
          return 0;

        case UCON64_XGBX:
          if (access (ucon64.rom, F_OK) != 0)     // file does not exist -> dump cartridge
            gbx_read_rom (ucon64.rom, ucon64.parport);
          else                          // file exists -> send it to the copier
            gbx_write_rom (ucon64.rom, ucon64.parport);
          printf ("\n");
          return 0;

        case UCON64_XGBXS:
          if (access (ucon64.rom, F_OK) != 0)
            gbx_read_sram (ucon64.rom, ucon64.parport, -1);
          else
            gbx_write_sram (ucon64.rom, ucon64.parport, -1);
          printf ("\n");
          return 0;

        case UCON64_XGBXB:
          if (access (ucon64.rom, F_OK) != 0)
            gbx_read_sram (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
          else
            gbx_write_sram (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
          printf ("\n");
          return 0;
#endif // BACKUP
  
        case UCON64_Z64:
          return n64_z64 (&rom);
#if 0
        case UCON64_MGH:
          return snes_mgh (&rom);
#endif
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


st_rominfo_t *
ucon64_flush (st_rominfo_t *rominfo)
{
  memset (rominfo, 0L, sizeof (st_rominfo_t));
#if 0
  rominfo->maker = rominfo->country = "";
  rominfo->console_usage = rominfo->copier_usage = NULL;
#endif

  return rominfo;
}


int
ucon64_console_probe (st_rominfo_t *rominfo)
{
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

      case UCON64_SATURN:
      case UCON64_CDI:
      case UCON64_CD32:
      case UCON64_PSX:
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
#ifdef SAMPLE
        sample_init (rominfo);
#endif // SAMPLE        
        break;

      case UCON64_UNKNOWN:
        if (UCON64_TYPE_ISROM (ucon64.type))
          ucon64.console =
#ifdef CONSOLE_PROBE
            (!nes_init (ucon64_flush (rominfo))) ? UCON64_NES :
            (!gba_init (ucon64_flush (rominfo))) ? UCON64_GBA :
            (!n64_init (ucon64_flush (rominfo))) ? UCON64_N64 :
            (!snes_init (ucon64_flush (rominfo))) ? UCON64_SNES :
            (!genesis_init (ucon64_flush (rominfo))) ? UCON64_GENESIS :
            (!lynx_init (ucon64_flush (rominfo))) ? UCON64_LYNX :
            (!gameboy_init (ucon64_flush (rominfo))) ? UCON64_GB :
            (!ngp_init (ucon64_flush (rominfo))) ? UCON64_NEOGEOPOCKET :
            (!swan_init (ucon64_flush (rominfo))) ? UCON64_WONDERSWAN :
            (!jaguar_init (ucon64_flush (rominfo))) ? UCON64_JAGUAR :
#endif // CONSOLE_PROBE
                UCON64_UNKNOWN;

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
  struct stat puffer;
  long size;

  if (access (romfile, F_OK | R_OK) == -1) return result;
  if (!stat (romfile, &puffer) == -1) return result;
  if (S_ISREG (puffer.st_mode) != TRUE) return result;

  size = quickftell (romfile);

/*
  currently the media type is determined by its size
*/
  ucon64.type = (size <= MAXROMSIZE) ? UCON64_ROM : UCON64_CD;
        
  ucon64_flush (rominfo);

  result = ucon64_console_probe (rominfo);

  if (UCON64_TYPE_ISROM (ucon64.type))
    {
#if 0
      rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
        ucon64.buheader_len : rominfo->buheader_len;
#endif

      rominfo->current_crc32 =
        fileCRC32 (romfile, rominfo->buheader_len);

      if (ucon64.console == UCON64_UNKNOWN)    // don't call if console type is already
        ucon64_dbsearch (rominfo);               //  known (destroys rominfo fields)
    }
  else if (UCON64_TYPE_ISCD (ucon64.type))
    {
      st_iso_header_t iso_header;

//      ucon64_flush (rominfo);

      result = 0;

      quickfread (&iso_header, ISO_HEADER_START + 
          UCON64_ISSET (ucon64.buheader_len) ?
          ucon64.buheader_len :
          CDRW_HEADER_START(ucon64_trackmode_probe (romfile)), ISO_HEADER_LEN, romfile
        );
      rominfo->header_start = ISO_HEADER_START;
      rominfo->header_len = ISO_HEADER_LEN;
      rominfo->header = &iso_header;

//CD internal name
      strcpy (rominfo->name, iso_header.volume_id);

//CD maker
      rominfo->maker = iso_header.publisher_id;

//misc stuff
      sprintf (rominfo->misc, "Track Mode: %s\n", track_modes[ucon64_trackmode_probe (romfile)]);

//      rominfo->console_usage = 

      rominfo->copier_usage = cdrw_usage;
    }

  return result;
}


int
ucon64_nfo (const st_rominfo_t *rominfo)
{
  char buf[MAXBUFSIZE];
  int size = quickftell (ucon64.rom);

  printf ("%s\n\n", ucon64.rom);

#if 0
  if (rominfo->buheader_len)
    {
      strhexdump (rominfo->buheader, 0, rominfo->buheader_start,
                  rominfo->buheader_len);
      printf ("\n");
    }
#endif

  if (rominfo->copier_usage != NULL)
    {
      strcpy (buf, rominfo->copier_usage[0]);
      printf ("%s\n", mkprint (buf, '.'));

      if (rominfo->copier_usage[1])
        {
          strcpy (buf, rominfo->copier_usage[1]);
          printf ("  %s\n\n", mkprint (buf, '.'));
        }
    }

  if (rominfo->header_len)
    {
      strhexdump (rominfo->header, 0, rominfo->header_start + rominfo->buheader_len,
                  rominfo->header_len);
      printf ("\n");
    }

  if (rominfo->console_usage != NULL)
    {
      strcpy (buf, rominfo->console_usage[0]);
      printf ("%s\n", mkprint (buf, '.'));

      if (rominfo->console_usage[1])
        {
          strcpy (buf, rominfo->console_usage[1]);
          printf ("  %s\n", mkprint (buf, '.'));
        }
    }

  strcpy (buf, rominfo->name);
  printf ("%s\n%s\n%s\n%ld Bytes (%.4f Mb)\n\n", mkprint (buf, '.'),  // some ROMs have a name with control chars in it -> replace control chars
          NULLtoEmpty (rominfo->maker),
          NULLtoEmpty (rominfo->country),
          quickftell (ucon64.rom) - rominfo->buheader_len,
          TOMBIT_F (quickftell (ucon64.rom) - rominfo->buheader_len));

  if (UCON64_TYPE_ISCD (ucon64.type))
    {
//   toc header,track mode, etc.
    }
  else if (UCON64_TYPE_ISROM (ucon64.type))
    {


      unsigned long padded = filetestpad (ucon64.rom);
      unsigned long intro = ((size - rominfo->buheader_len) > MBIT) ?
        ((size - rominfo->buheader_len) % MBIT) : 0;
      int splitted = (UCON64_ISSET (ucon64.splitted)) ? ucon64.splitted :
        ucon64_testsplit (ucon64.rom);

      if (!padded)
        printf ("Padded: No\n");
      else if (padded)
        printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n", padded,
                TOMBIT_F (padded));

      if (intro)
        printf ("Intro/Trainer: Maybe, %ld Bytes\n", intro);

      printf ("Interleaved/Swapped: %s\n", (rominfo->interleaved) ? "Yes" : "No");

      if (rominfo->buheader_len)
        printf ("Backup unit/Emulator header: Yes, %ld Bytes\n",
          rominfo->buheader_len);

      if (splitted)
        printf ("Splitted: Yes, %d parts\n"
          "NOTE: to get the correct checksum the ROM must be joined\n",
          splitted);
    }

  if (rominfo->misc[0])
    {
      strcpy (buf, rominfo->misc);
      printf ("%s\n", mkprint (buf, '.'));
    }

  if (UCON64_TYPE_ISROM (ucon64.type))
    {
      if (rominfo->has_internal_crc)
        {
          sprintf (buf,
            "Checksum: %%s, 0x%%0%dlx (calculated) %%s= 0x%%0%dlx (internal)\n",
            rominfo->internal_crc_len * 2, rominfo->internal_crc_len * 2);
    
          printf (buf,
            (rominfo->current_internal_crc == rominfo->internal_crc) ? "ok" : "bad",
            rominfo->current_internal_crc,
            (rominfo->current_internal_crc == rominfo->internal_crc) ? "=" : "!",
            rominfo->internal_crc);
    
          if (rominfo->internal_crc2[0])
            {
              strcpy (buf, rominfo->internal_crc2);
              printf ("%s\n", mkprint (buf, '.'));
            }
        }
    
      if (rominfo->current_crc32)
        printf ("Checksum (CRC32): 0x%08lx\n", rominfo->current_crc32);
    }

  printf ("\n");

  if (ucon64.console == UCON64_UNKNOWN)
    {
       printf (
           "ERROR: could not auto detect the right ROM/IMAGE/console type\n"
           "TIP:   If this is a ROM or CD IMAGE you might try to force the recognition\n"
           "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n"
         );
    }

  fflush (stdout);

  return 0;
}


void
usage (const char **usage)
{
  if (usage[0] != NULL)
    printf ("%s\n" , usage[0]);
  if (usage[1] != NULL)
    printf ("  %s\n", usage[1]);
  if (usage[2] != NULL)
    printf ("%s", usage[2]);
}


void
ucon64_usage (int argc, char *argv[])
{
  int c = 0;
  int option_index = 0;
  int single = 0;

  printf (
    "Usage: %s [OPTION]... [" OPTION_LONG_S "rom=]ROM [[" OPTION_LONG_S "file=]FILE]\n\n"
    "  " OPTION_LONG_S "nbak        prevents backup files (*.BAK)\n"
    "  " OPTION_LONG_S "hdn=BYTES   force ROM has backup unit/emulator header with BYTES size\n"
    "  " OPTION_LONG_S "hd          same as " OPTION_LONG_S "hdn=512\n"
//    "TODO: " OPTION_LONG_S "hdn=TYPE     force ROM has TYPE backup unit/emulator header:\n"
//    "                  FFE, SMC, FIG, SWC, SMD, SMG, SSC or LNX\n"
    "                  most backup units use a header with 512 Bytes size\n"
    "  " OPTION_LONG_S "nhd         force ROM has no backup unit/emulator header\n"
    "  " OPTION_LONG_S "stp         strip 512 Bytes (header) from ROM beginning\n"
    "  " OPTION_LONG_S "ins         insert 512 Bytes (empty header) before ROM\n"
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
#if 0
    "TODO:  " OPTION_LONG_S "rrom   rename all ROMs in DIRECTORY to their internal names; " OPTION_LONG_S "rom=DIR\n"
    "TODO:  " OPTION_LONG_S "rr83   like " OPTION_LONG_S "rrom but with 8.3 filenames; " OPTION_LONG_S "rom=DIR\n"
    "               this is often used by people who loose control of their ROMs\n"
#endif
#if 0
    "  " OPTION_LONG_S "rl          rename all files in DIRECTORY to lowercase; " OPTION_LONG_S "rom=DIRECTORY\n"
    "  " OPTION_LONG_S "ru          rename all files in DIRECTORY to uppercase; " OPTION_LONG_S "rom=DIRECTORY\n"
#endif
#ifdef	__MSDOS__
    "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex " OPTION_LONG_S "rom=ROM|more\"\n"
#else
    "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex " OPTION_LONG_S "rom=ROM|less\"\n"       // less is better ;-)
#endif
    "  " OPTION_LONG_S "find        find string in ROM; " OPTION_LONG_S "file=STRING (wildcard: '?')\n"
    "  " OPTION_S "c           compare ROMs for differencies; " OPTION_LONG_S "file=OTHER_ROM\n"
    "  " OPTION_LONG_S "cs          compare ROMs for similarities; " OPTION_LONG_S "file=OTHER_ROM\n"
    "  " OPTION_LONG_S "ispad       check if ROM is padded\n"
    "  " OPTION_S "p, " OPTION_LONG_S "pad    pad ROM to full Mb\n"
    , argv[0], ucon64.configfile
  );

  usage (bsl_usage);
  usage (ips_usage);
  usage (aps_usage);
  usage (pal4u_usage);
  usage (ppf_usage);
  usage (xps_usage);
  usage (gg_usage);
  
  printf ("                  supported are:\n"
    "                  %s,\n"
    "                  %s,\n"
    "                  %s,\n"
    "                  %s,\n"
    "                  and %s\n",
    gameboy_usage[0], sms_usage[0], genesis_usage[0], nes_usage[0], snes_usage[0]);
  
//  usage (cdrw_usage);
  printf (cdrw_usage[2]);

  printf (
    "  " OPTION_LONG_S "help        display this help and exit\n"
    "  " OPTION_LONG_S "version     output version information and exit\n"
//    "  " OPTION_LONG_S "quiet       don't show output\n"
    "\n"
  );

  optind = option_index = 0;//TODO is there a better way to "reset"?

  single = 0;

  while (!single && (c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
//      if (single) break;
      
      switch (c)
        {
      case UCON64_GBA:
        usage (gba_usage);
#ifdef BACKUP
        usage (fal_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_N64:
        usage (n64_usage);
#ifdef BACKUP
        usage (doctor64_usage);
        usage (doctor64jr_usage);
//        usage (cd64_usage);
//        usage (dex_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_JAG:
        usage (jaguar_usage);
        single = 1;
        break;

      case UCON64_SNES:
        usage (snes_usage);
#ifdef BACKUP
        usage (swc_usage);
//        usage (fig_usage);
//        usage (smc_usage);
//        usage (mgd_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_NG:
        usage (neogeo_usage);
        single = 1;
        break;

      case UCON64_NGP:
        usage (ngp_usage);
#ifdef BACKUP
//        usage (fpl_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_GEN:
        usage (genesis_usage);
#ifdef BACKUP
        usage (smd_usage);
//        usage (mgd_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_GB:
        usage (gameboy_usage);
#ifdef BACKUP
        usage (gbx_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_LYNX:
        usage (lynx_usage);
        single = 1;
        break;

      case UCON64_PCE:
        usage (pcengine_usage);
#ifdef BACKUP
//        usage (mgd_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_SMS:
        usage (sms_usage);
#ifdef BACKUP
        usage (smd_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_NES:
        usage (nes_usage);
        single = 1;
        break;

      case UCON64_SWAN:
        usage (swan_usage);
        single = 1;
        break;

      case UCON64_DC:
        usage (dc_usage);
        single = 1;
        break;

      case UCON64_GC:
      case UCON64_S16:
      case UCON64_ATA:
      case UCON64_COLECO:
      case UCON64_VBOY:
      case UCON64_VEC:
      case UCON64_INTELLI:
      case UCON64_PSX:
      case UCON64_PS2:
      case UCON64_SAT:
      case UCON64_3DO:
      case UCON64_CD32:
      case UCON64_CDI:
      case UCON64_XBOX:
      case UCON64_GP32:
#ifdef SAMPLE
        usage (sample_usage);
        single = 1;
#endif // SAMPLE        
        break;

      default:
        break;
      }
      printf ("\n");
    }

  if (!single)
    {
      usage (dc_usage);
      printf("\n");
      
      usage (gba_usage);
#ifdef BACKUP
      usage (fal_usage);
#endif // BACKUP
      printf ("\n");

      usage (n64_usage);
#ifdef BACKUP
      usage (doctor64_usage);
      usage (doctor64jr_usage);
//      usage (cd64_usage);
//      usage (dex_usage);
#endif // BACKUP
      printf ("\n");

      usage (snes_usage);
#ifdef BACKUP
      usage (swc_usage);
//      usage (fig_usage);
//      usage (smc_usage);
//      usage (mgd_usage);
#endif // BACKUP
      printf ("\n");

      usage (neogeo_usage);
      printf ("\n");
      
      usage (genesis_usage);
#ifdef BACKUP
      usage (smd_usage);
//      usage (mgd_usage);
#endif // BACKUP
      printf ("\n");

      usage (gameboy_usage);
#ifdef BACKUP
      usage (gbx_usage);
#endif // BACKUP
      printf ("\n");

      usage (lynx_usage);
      printf ("\n");

      usage (pcengine_usage);
#ifdef BACKUP
//      usage (mgd_usage);
#endif // BACKUP
      printf ("\n");

      usage (sms_usage);
#ifdef BACKUP
      usage (smd_usage);
#endif // BACKUP
      printf ("\n");

      usage (nes_usage);
      printf ("\n");

      usage (swan_usage);
      printf ("\n");

      usage (jaguar_usage);
      printf ("\n");

      usage (ngp_usage);
#ifdef BACKUP
//      usage (fpl_usage);
#endif // BACKUP
      printf ("\n");

#ifdef SAMPLE
      usage (sample_usage);
      printf ("\n");
#endif // SAMPLE
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
