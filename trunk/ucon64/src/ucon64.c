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
#ifdef __linux__
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
#include "wswan/wswan.h"
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

static int ucon64_init (char *romfile, struct rom_ *rombuf);
static int ucon64_console_probe (struct rom_ *rombuf);
static int ucon64_nfo (struct rom_ *rombuf);

static int ucon64_e (struct rom_ *rombuf);
static int ucon64_ls (char *path, int mode);
static int ucon64_configfile (void);
static void ucon64_exit (void);
static void ucon64_usage (int argc, char *argv[]);

struct ucon64_ ucon64;

static struct option long_options[] = {
    {"1991", 0, 0, ucon64_1991},
    {"3do", 0, 0, ucon64_3DO},
    {"?", 0, 0, ucon64_HELP},
    {"a", 0, 0, ucon64_A},
    {"ata", 0, 0, ucon64_ATA},
    {"b", 0, 0, ucon64_B},
    {"b0", 0, 0, ucon64_B0},
    {"b1", 0, 0, ucon64_B1},
    {"bios", 0, 0, ucon64_BIOS},
    {"bot", 0, 0, ucon64_BOT},
    {"c", 0, 0, ucon64_C},
    {"cd32", 0, 0, ucon64_CD32},
    {"cdi", 0, 0, ucon64_CDI},
    {"chk", 0, 0, ucon64_CHK},
    {"col", 0, 0, ucon64_COL},
    {"coleco", 0, 0, ucon64_COLECO},
    {"crc", 0, 0, ucon64_CRC},
    {"crchd", 0, 0, ucon64_CRCHD},
    {"crp", 0, 0, ucon64_CRP},
    {"cs", 0, 0, ucon64_CS},
    {"db", 0, 0, ucon64_DB},
    {"dbs", 0, 0, ucon64_DBS},
    {"dbv", 0, 0, ucon64_DBV},
    {"dc", 0, 0, ucon64_DC},
    {"dint", 0, 0, ucon64_DINT},
    {"e", 0, 0, ucon64_E},
    {"f", 0, 0, ucon64_F},
//    {"fds", 0, 0, ucon64_FDS},
//    {"fdsl", 0, 0, ucon64_FDSL},
    {"ffe", 0, 0, ucon64_FFE},
    {"fig", 0, 0, ucon64_FIG},
    {"figs", 0, 0, ucon64_FIGS},
    {"file", 1, 0, ucon64_FILE},
    {"find", 0, 0, ucon64_FIND},
    {"frontend", 0, 0, ucon64_FRONTEND},
    {"gb", 0, 0, ucon64_GB},
    {"gba", 0, 0, ucon64_GBA},
    {"gbx", 0, 0, ucon64_GBX},
    {"gc", 0, 0, ucon64_GC},
    {"gd3", 0, 0, ucon64_GD3},
    {"gdf", 0, 0, ucon64_GDF},
    {"gen", 0, 0, ucon64_GEN},
    {"gg", 0, 0, ucon64_GG},
    {"ggd", 0, 0, ucon64_GGD},
    {"gge", 0, 0, ucon64_GGE},
    {"gp32", 0, 0, ucon64_GP32},
    {"h", 0, 0, ucon64_HELP},
    {"help", 0, 0, ucon64_HELP},
    {"hex", 0, 0, ucon64_HEX},
    {"i", 0, 0, ucon64_I},
    {"idppf", 0, 0, ucon64_IDPPF},
    {"ines", 0, 0, ucon64_INES},
    {"ineshd", 0, 0, ucon64_INESHD},
    {"ins", 0, 0, ucon64_INS},
    {"intelli", 0, 0, ucon64_INTELLI},
    {"ip", 0, 0, ucon64_IP},
    {"iso", 0, 0, ucon64_ISO},
    {"ispad", 0, 0, ucon64_ISPAD},
    {"j", 0, 0, ucon64_J},
    {"jag", 0, 0, ucon64_JAG},
    {"k", 0, 0, ucon64_K},
    {"l", 0, 0, ucon64_L},
    {"lnx", 0, 0, ucon64_LNX},
    {"logo", 0, 0, ucon64_LOGO},
    {"ls", 0, 0, ucon64_LS},
    {"lsv", 0, 0, ucon64_LSV},
    {"lynx", 0, 0, ucon64_LYNX},
    {"lyx", 0, 0, ucon64_LYX},
    {"mgd", 0, 0, ucon64_MGD},
//    {"mgh", 0, 0, ucon64_MGH},
    {"mka", 0, 0, ucon64_MKA},
    {"mkcue", 0, 0, ucon64_MKCUE},
    {"mki", 0, 0, ucon64_MKI},
    {"mkppf", 0, 0, ucon64_MKPPF},
    {"mktoc", 0, 0, ucon64_MKTOC},
    {"multi", 0, 0, ucon64_MULTI},
    {"multi1", 0, 0, ucon64_MULTI1},
    {"multi2", 0, 0, ucon64_MULTI2},
    {"mvs", 0, 0, ucon64_MVS},
    {"n", 0, 0, ucon64_N},
    {"n2", 0, 0, ucon64_N2},
    {"n2gb", 0, 0, ucon64_N2GB},
    {"n64", 0, 0, ucon64_N64},
    {"na", 0, 0, ucon64_NA},
    {"nbak", 0, 0, ucon64_NBAK},
    {"nes", 0, 0, ucon64_NES},
    {"ng", 0, 0, ucon64_NG},
    {"ngp", 0, 0, ucon64_NGP},
    {"nppf", 0, 0, ucon64_NPPF},
    {"nrot", 0, 0, ucon64_NROT},
    {"ns", 0, 0, ucon64_NS},
    {"p", 0, 0, ucon64_P},
    {"pad", 0, 0, ucon64_PAD},
    {"padhd", 0, 0, ucon64_PADHD},
    {"pas", 0, 0, ucon64_PAS},
    {"pce", 0, 0, ucon64_PCE},
    {"port", 1, 0, ucon64_PORT},
    {"ppf", 0, 0, ucon64_PPF},
    {"ps2", 0, 0, ucon64_PS2},
    {"psx", 0, 0, ucon64_PSX},
    {"ren", 0, 0, ucon64_REN},
    {"rl", 0, 0, ucon64_RL},
    {"rom", 1, 0, ucon64_ROM},
    {"rotl", 0, 0, ucon64_ROTL},
    {"rotr", 0, 0, ucon64_ROTR},
    {"ru", 0, 0, ucon64_RU},
    {"s", 0, 0, ucon64_S},
    {"s16", 0, 0, ucon64_S16},
    {"sam", 0, 0, ucon64_SAM},
    {"sat", 0, 0, ucon64_SAT},
    {"sgb", 0, 0, ucon64_SGB},
    {"smc", 0, 0, ucon64_SMC},
    {"smd", 0, 0, ucon64_SMD},
    {"smds", 0, 0, ucon64_SMDS},
    {"smg", 0, 0, ucon64_SMG},
    {"sms", 0, 0, ucon64_SMS},
    {"snes", 0, 0, ucon64_SNES},
    {"sram", 0, 0, ucon64_SRAM},
    {"ssc", 0, 0, ucon64_SSC},
    {"stp", 0, 0, ucon64_STP},
    {"strip", 0, 0, ucon64_STRIP},
    {"swan", 0, 0, ucon64_SWAN},
    {"swap", 0, 0, ucon64_SWAP},
    {"swc", 0, 0, ucon64_SWC},
    {"swcs", 0, 0, ucon64_SWCS},
    {"ufos", 0, 0, ucon64_UFOS},
    {"unif", 0, 0, ucon64_UNIF},
    {"usms", 0, 0, ucon64_USMS},
    {"v64", 0, 0, ucon64_V64},
    {"vboy", 0, 0, ucon64_VBOY},
    {"vec", 0, 0, ucon64_VEC},
    {"xbox", 0, 0, ucon64_XBOX},
#ifdef BACKUP_CD
    {"xcdrw", 0, 0, ucon64_XCDRW},
#endif // BACKUP_CD
#ifdef BACKUP
    {"xdjr", 0, 0, ucon64_XDJR},
    {"xfal", 0, 0, ucon64_XFAL},
    {"xfalb", 1, 0, ucon64_XFALB},
    {"xfalc", 1, 0, ucon64_XFALC},
    {"xfals", 0, 0, ucon64_XFALS},
    {"xgbx", 0, 0, ucon64_XGBX},
    {"xgbxb", 1, 0, ucon64_XGBXB},
    {"xgbxs", 0, 0, ucon64_XGBXS},
    {"xsmd", 0, 0, ucon64_XSMD},
    {"xsmds", 0, 0, ucon64_XSMDS},
    {"xswc", 0, 0, ucon64_XSWC},
    {"xswcs", 0, 0, ucon64_XSWCS},
    {"xv64", 0, 0, ucon64_XV64},
#endif // BACKUP
    {"z64", 0, 0, ucon64_Z64},
    {"hd", 1, 0, ucon64_HD},
    {"nhd", 0, 0, ucon64_NHD},
    {"int", 0, 0, ucon64_INT},
    {"int2", 0, 0, ucon64_INT2},
    {"nint", 0, 0, ucon64_NINT},
    {"hi", 0, 0, ucon64_HI},
    {"nhi", 0, 0, ucon64_NHI},
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
  int ucon64_argc, c = 0, result = 0;
  unsigned long padded;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], *ucon64_argv[128];
  int option_index = 0;
  struct stat puffer;
  struct rom_ rom;

  printf ("%s\n", ucon64_TITLE);
  printf ("Uses code from various people. See 'developers.html' for more!\n");
  printf ("This may be freely redistributed under the terms of the GNU Public License\n\n");

  memset (&ucon64, 0L, sizeof (struct ucon64_));
/*
  if these values are != -1 before <console>_init() then --hd=n, --nhd, etc.
  were used...
*/
  ucon64.buheader_len =
  ucon64.interleaved =
  ucon64.splitted =
  ucon64.snes_hirom = -1;

  ucon64_configfile ();

  ucon64.backup = ((!strcmp (getProperty (ucon64.configfile, "backups", buf2, "1"), "1")) ?
               1 : 0);

  ucon64.argc = argc;
  for (x = 0; x < argc; x++)ucon64.argv[x] = argv[x];

  if (argc < 2)
    {
       ucon64_usage (argc, argv);
       return 0;
    }

  ucon64_init (NULL, &rom);

/*
  getopt_long_only() - switches and overrides
*/
  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      switch (c)
        {
        case ucon64_FRONTEND:
          atexit (ucon64_exit);
          ucon64.frontend = 1;       // used by ucon64_gauge()
          break;

        case ucon64_NBAK:
          ucon64.backup = 0;
          break;

        case ucon64_NS:
          ucon64.splitted = 0;
          break;

        case ucon64_HD:
          ucon64.buheader_len = atoi (optarg);
        case ucon64_NHD:
//          rom.buheader_start = 0;
          strcpy (rom.copier, unknown_title);
          break;

        case ucon64_INT:
          ucon64.interleaved = 1;
          break;

        case ucon64_INT2:
          ucon64.interleaved = 2;
          break;

        case ucon64_NINT:
          ucon64.interleaved = 0;
          break;

        case ucon64_PORT:
          if(optarg)
            sscanf (optarg, "%x", &ucon64.parport);
          break;
        
        case ucon64_FILE:
          if(optarg)
            strcpy (ucon64.file, optarg);
          break;
        
        case ucon64_ROM:
          if(optarg)
            strcpy (rom.rom, optarg);
          break;

        case ucon64_A:
        case ucon64_B:
        case ucon64_C:
        case ucon64_CHK:
        case ucon64_CRC:
        case ucon64_CRCHD://obsolete only for compat.
        case ucon64_CS:
        case ucon64_DB:
        case ucon64_DBS:
        case ucon64_DBV:
        case ucon64_DINT://TODO replace --swap?
        case ucon64_SWAP:
        case ucon64_E:
        case ucon64_FIND:
        case ucon64_GG:
        case ucon64_GGD:
        case ucon64_GGE:
        case ucon64_HELP:
        case ucon64_HEX:
        case ucon64_I:
        case ucon64_IDPPF:
        case ucon64_INS:
        case ucon64_ISO:
        case ucon64_ISPAD:
        case ucon64_J:
        case ucon64_LS:
        case ucon64_LSV:
        case ucon64_MGD:
//        case ucon64_MGH:
        case ucon64_MKA:
        case ucon64_MKCUE:
        case ucon64_MKI:
        case ucon64_MKPPF:
        case ucon64_MKTOC:
        case ucon64_N:
        case ucon64_NA:
        case ucon64_NPPF:
        case ucon64_P:
        case ucon64_PAD:
        case ucon64_PADHD://obsolete only for compat.
        case ucon64_PPF:
        case ucon64_REN:
        case ucon64_RL:
        case ucon64_RU:
        case ucon64_S:
        case ucon64_SRAM:
        case ucon64_STP:
        case ucon64_STRIP:
#ifdef BACKUP_CD
        case ucon64_XCDRW:
#endif // BACKUP_CD
        case ucon64_SMD:
        case ucon64_SMDS:
#ifdef BACKUP
        case ucon64_XSMD:
        case ucon64_XSMDS:
#endif // BACKUP
          break;
  
        case ucon64_SWCS:
        case ucon64_FIGS:
        case ucon64_UFOS:
        case ucon64_COL:
          ucon64.show_nfo = 1;
        case ucon64_HI:
          ucon64.snes_hirom = 1;
        case ucon64_NHI:
          ucon64.snes_hirom = 0;
        case ucon64_SNES:
        case ucon64_F:
        case ucon64_K:
        case ucon64_L:
        case ucon64_GD3:
//        case ucon64_GDF://obsolete
        case ucon64_SMC:
        case ucon64_SWC:
        case ucon64_FIG:
#ifdef BACKUP
        case ucon64_XSWC:
        case ucon64_XSWCS:
#endif // BACKUP
          rom.console = ucon64_SNES;
          break;

        case ucon64_IP:
          rom.console = ucon64_DC;
          break;
  
        case ucon64_GB:
        case ucon64_SGB:
        case ucon64_GBX:
        case ucon64_N2GB:
        case ucon64_SSC:
#ifdef BACKUP
        case ucon64_XGBX:
        case ucon64_XGBXS:
        case ucon64_XGBXB:
#endif // BACKUP
          rom.console = ucon64_GB;
          break;

        case ucon64_SAM:
        case ucon64_MVS:
        case ucon64_BIOS:
          rom.console = ucon64_NEOGEO;
          break;
  
        case ucon64_ATA:
          rom.console = ucon64_ATARI;
          break;
  
        case ucon64_S16:
          rom.console = ucon64_SYSTEM16;
          break;
  
        case ucon64_COLECO:
          rom.console = ucon64_COLECO;
          break;
  
        case ucon64_VBOY:
          rom.console = ucon64_VIRTUALBOY;
          break;
  
        case ucon64_SWAN:
          rom.console = ucon64_WONDERSWAN;
          break;

        case ucon64_VEC:
          rom.console = ucon64_VECTREX;
          break;
  
        case ucon64_INTELLI:
          rom.console = ucon64_INTELLI;
          break;
  
        case ucon64_LYNX:
        case ucon64_B0:
        case ucon64_B1:
        case ucon64_ROTL:
        case ucon64_ROTR:
        case ucon64_NROT:
        case ucon64_LNX:
        case ucon64_LYX:
          rom.console = ucon64_LYNX;
          break;
  
        case ucon64_SMS:
          rom.console = ucon64_SMS;
          break;
  
        case ucon64_NGP:
          rom.console = ucon64_NEOGEOPOCKET;
          break;
  
        case ucon64_INES:
        case ucon64_INESHD:
        case ucon64_NES:
//        case ucon64_FDS:
//        case ucon64_FDSL:
//        case ucon64_PAS:
        case ucon64_FFE:
        case ucon64_UNIF:
          rom.console = ucon64_NES;
          break;
  
        case ucon64_SMG:
        case ucon64_PCE:
          rom.console = ucon64_PCE;
          break;
  
        case ucon64_JAG:
          rom.console = ucon64_JAGUAR;
          break;
  
        case ucon64_GEN:
        case ucon64_N2:
        case ucon64_1991:
          rom.console = ucon64_GENESIS;
          break;
  
        case ucon64_NG:
          rom.console = ucon64_NEOGEO;
          break;
  
#ifdef BACKUP
        case ucon64_XV64:
        case ucon64_XDJR:
#endif // BACKUP
        case ucon64_BOT:
        case ucon64_Z64:
        case ucon64_N64:
        case ucon64_USMS:
        case ucon64_V64:
          rom.console = ucon64_N64;
          break;
  
        case ucon64_MULTI:
        case ucon64_MULTI1:
        case ucon64_MULTI2:
          ucon64.show_nfo = 1;      // This gets rid of nonsense GBA info on a GBA multirom loader binary
#ifdef BACKUP
        case ucon64_XFAL:
        case ucon64_XFALS:
        case ucon64_XFALB:
        case ucon64_XFALC:
#endif // BACKUP
        case ucon64_LOGO:
        case ucon64_CRP:
        case ucon64_GBA:
          rom.console = ucon64_GBA;
          break;
  

        case ucon64_SAT:
          rom.console = ucon64_SATURN;
          break;
  
        case ucon64_PSX:
          rom.console = ucon64_PSX;
          break;
  
        case ucon64_PS2:
          rom.console = ucon64_PS2;
          break;

        case ucon64_CDI:
          rom.console = ucon64_CDI;
          break;
  
        case ucon64_CD32:
          rom.console = ucon64_CD32;
          break;
  
        case ucon64_3DO:
          rom.console = ucon64_REAL3DO;
          break;
  
        case ucon64_DC:
          rom.console = ucon64_DC;
          break;
  
        case ucon64_XBOX:
          rom.console = ucon64_XBOX;
          break;
  
        case ucon64_GC:
          rom.console = ucon64_GAMECUBE;
          break;

        case ucon64_GP32:
          rom.console = ucon64_GP32;
          break;

        case ucon64_GETOPT_ERROR:
        default:
          fprintf (STDERR, "Try '%s " OPTION_LONG_S "help' for more information.\n", argv[0]);
          return -1;
      }
    }

  if (optind < argc)
    strcpy(rom.rom, argv[optind++]);
  if (optind < argc)
    strcpy(ucon64.file, argv[optind++]);

  if (ucon64.file[0])// && !ucon64.parport)
      sscanf (ucon64.file, "%x", &ucon64.parport);
  ucon64.parport = ucon64_parport_probe (ucon64.parport);

  if (!access (rom.rom, F_OK|R_OK))
    {
      if (!stat (rom.rom, &puffer))
        if (S_ISREG (puffer.st_mode))  
          {
            if (ucon64_init (rom.rom, &rom) == -1) ucon64.show_nfo = 1;
#if 0
  int show_nfo;                 //show or skip info output for ROM
                                //values:
                                //0 show before processing of ROM (default)
                                //1 skip before and after processing of ROM
                                //2 show after processing of ROM
                                //3 show before and after processing of ROM
#endif
            if (!ucon64.show_nfo || ucon64.show_nfo == 3)
              ucon64_nfo (&rom);
          }
    }
/*
  getopt_long_only() - options
*/
  optind = option_index = 0;//TODO is there a better way to "reset"?

  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      switch (c)
        {
        case ucon64_HELP:
          ucon64_usage (argc, argv);
          return 0;

        case ucon64_CRCHD://obsolete only for compat.
          rom.buheader_len = unknown_HEADER_LEN;
        case ucon64_CRC:
          printf ("Checksum (CRC32): %08lx\n\n", fileCRC32 (rom.rom, rom.buheader_len));
          return 0;

        case ucon64_RL:
          return renlwr (rom.rom);
  
        case ucon64_RU:
          return renupr (rom.rom);
  
        case ucon64_HEX:
          return filehexdump (rom.rom, 0, quickftell (rom.rom));
  
        case ucon64_C:
          if (filefile (rom.rom, 0, ucon64.file, 0, FALSE) == -1)
            {
              printf ("ERROR: file not found/out of memory\n");
              return -1;
            }
          return 0;
  
        case ucon64_CS:
          if (filefile (rom.rom, 0, ucon64.file, 0, TRUE) == -1)
            {
              printf ("ERROR: file not found/out of memory\n");
              return -1;
            }
          return 0;
  
        case ucon64_FIND:
          x = 0;
          y = quickftell (rom.rom);
          while ((x =
                  filencmp2 (rom.rom, x, y, ucon64.file, strlen (ucon64.file),
                             '?')) != -1)
            {
              filehexdump (rom.rom, x, strlen (ucon64.file));
              x++;
              printf ("\n");
            }
          return 0;
  
        case ucon64_SWAP:
          result = fileswap (ucon64_fbackup (rom.rom), 0,
                           quickftell (rom.rom));
          printf ("Wrote output to %s\n", rom.rom);
          return result;
  
        case ucon64_PADHD:
          rom.buheader_len = unknown_HEADER_LEN;
        case ucon64_PAD:
          ucon64_fbackup (rom.rom);
          return filepad (rom.rom, rom.buheader_len, MBIT);
  
        case ucon64_ISPAD:
          if ((padded = filetestpad (rom.rom)) != -1)
            {
              if (!padded)
                printf ("Padded: No\n\n");
              else
                printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n\n", padded,
                        (float) padded / MBIT);
            }
          return 0;
  
        case ucon64_STRIP:
          ucon64_fbackup (rom.rom);
  
          return truncate (rom.rom, quickftell (rom.rom) - atol (ucon64.file));
  
        case ucon64_STP:
          strcpy (buf, rom.rom);
          newext (buf, ".BAK");
          remove (buf);           // try to remove or rename will fail
          rename (rom.rom, buf);
  
          return filecopy (buf, 512, quickftell (buf), rom.rom, "wb");
  
        case ucon64_INS:
          strcpy (buf, rom.rom);
          newext (buf, ".BAK");
          remove (buf);           // try to remove or rename will fail
          rename (rom.rom, buf);
  
          memset (buf2, 0, 512);
          quickfwrite (buf2, 0, 512, rom.rom, "wb");
  
          return filecopy (buf, 0, quickftell (buf), rom.rom, "ab");
  
        case ucon64_B:
          ucon64_fbackup (rom.rom);
  
          if ((result = bsl (rom.rom, ucon64.file)) != 0)
            printf ("ERROR: failed\n");
          return result;

        case ucon64_I:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = ucon64.file;
          ucon64_argv[2] = rom.rom;
          ucon64_argc = 3;
  
          ucon64_fbackup (rom.rom);
  
          ips_main (ucon64_argc, ucon64_argv);
          break;
  
        case ucon64_A:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = "-f";
          ucon64_argv[2] = rom.rom;
          ucon64_argv[3] = ucon64.file;
          ucon64_argc = 4;
  
          ucon64_fbackup (rom.rom);
  
          return n64aps_main (ucon64_argc, ucon64_argv);

        case ucon64_MKI:
          return cips (rom.rom, ucon64.file);
  
        case ucon64_MKA:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = "-d \"\"";
          ucon64_argv[2] = rom.rom;
          ucon64_argv[3] = ucon64.file;
          strcpy (buf, rom.rom);
          newext (buf, ".APS");
  
          ucon64_argv[4] = buf;
          ucon64_argc = 5;
  
          return n64caps_main (ucon64_argc, ucon64_argv);
  
        case ucon64_NA:
          memset (buf2, ' ', 50);
          strncpy (buf2, ucon64.file, strlen (ucon64.file));
          return quickfwrite (buf2, 7, 50, ucon64_fbackup (rom.rom), "r+b");
  
        case ucon64_PPF:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = rom.rom;
          ucon64_argv[2] = ucon64.file;
          ucon64_argc = 3;
  
          return applyppf_main (ucon64_argc, ucon64_argv);
  
        case ucon64_MKPPF:
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = rom.rom;
          ucon64_argv[2] = ucon64.file;
  
          strcpy (buf, ucon64.file);
          newext (buf, ".PPF");
  
          ucon64_argv[3] = buf;
          ucon64_argc = 4;
  
          return makeppf_main (ucon64_argc, ucon64_argv);

        case ucon64_NPPF:
          memset (buf2, ' ', 50);
          strncpy (buf2, ucon64.file, strlen (ucon64.file));
          return quickfwrite (buf2, 6, 50, ucon64_fbackup (rom.rom), "r+b");

        case ucon64_IDPPF:
          return addppfid (rom.rom);
  
        case ucon64_LS:
          return ucon64_ls (rom.rom, ucon64_LS);
  
        case ucon64_LSV:
          return ucon64_ls (rom.rom, ucon64_LSV);
  
        case ucon64_REN:
          return ucon64_ls (rom.rom, ucon64_REN);
  
        case ucon64_ISO:
          return bin2iso (rom.rom);
  
        case ucon64_MKTOC:
          return cdrw_mktoc (&rom);
  
        case ucon64_MKCUE:
          return cdrw_mkcue (&rom);

#ifdef BACKUP_CD
        case ucon64_XCDRW:
          switch (rom.console)
            {
            case ucon64_DC:      //Dreamcast NOTE: CDI
              return dc_xcdrw (&rom);
  
            default:
              return (!access (rom.rom, F_OK)) ? cdrw_write (&rom) :
                cdrw_read (&rom);
            }
#endif // BACKUP_CD
  
        case ucon64_DB:
          printf ("Database: %ld known ROMs in db.h (%+ld)\n\n"
                  "TIP: %s " OPTION_LONG_S "db " OPTION_LONG_S "nes would show only the number of known NES ROMs\n\n",
                  ucon64_dbsize (rom.console),
                  ucon64_dbsize (rom.console) - ucon64_DBSIZE,
                  argv[0]);
          break;

        case ucon64_DBS:
          ucon64_init (NULL, &rom);
          sscanf (rom.rom, "%lx", &rom.current_crc32);
          ucon64_dbsearch (&rom);
          ucon64_nfo (&rom);
//          ucon64_dbview (rom.console);
          printf ("TIP: %s "OPTION_LONG_S "dbs "OPTION_LONG_S "nes would search only for a NES ROM\n\n",
                  argv[0]);
  
          break;
  
        case ucon64_DBV:
          ucon64_dbview (rom.console);
  
          printf ("\nTIP: %s " OPTION_LONG_S "db " OPTION_LONG_S "nes would view only NES ROMs\n\n",
                  argv[0]);
          break;
  
        case ucon64_MULTI:
          return gbadvance_multi (&rom, 256 * MBIT);

        case ucon64_MULTI1:
          return gbadvance_multi (&rom, 64 * MBIT);

        case ucon64_MULTI2:
          return gbadvance_multi (&rom, 128 * MBIT);
  
        case ucon64_SWCS:
          return snes_swcs (&rom);
  
        case ucon64_FIGS:
          return snes_figs (&rom);
  
        case ucon64_UFOS:
          return snes_ufos (&rom);
  
        case ucon64_E:
          return ucon64_e (&rom);

        case ucon64_1991:
          return genesis_1991 (&rom);
  
        case ucon64_B0:
          return lynx_b0 (&rom);
  
        case ucon64_B1:
          return lynx_b1 (&rom);
  
        case ucon64_BIOS:
          return neogeo_bios (&rom);
  
        case ucon64_BOT:
          return nintendo64_bot (&rom);
  
        case ucon64_CHK:
          switch (rom.console)
            {
            case ucon64_GB:
              return gameboy_chk (&rom);
            case ucon64_GBA:
              return gbadvance_chk (&rom);
            case ucon64_GENESIS:
              return genesis_chk (&rom);
            case ucon64_N64:
              return nintendo64_chk (&rom);
            case ucon64_SNES:
              return snes_chk (&rom);
            case ucon64_WONDERSWAN:
              return wonderswan_chk (&rom);
            default:
//TODO error
              return -1;
            }
  
        case ucon64_COL:
          return snes_col (&rom);
  
        case ucon64_CRP:
          return gbadvance_crp (&rom);
  
        case ucon64_DINT:
          return snes_dint (&rom);
  
        case ucon64_F:
          switch (rom.console)
            {
            case ucon64_N64:
              return nintendo64_f (&rom);
            case ucon64_SNES:
              return snes_f (&rom);
            default:
//TODO error
              return -1;
            }
  
#if 0
        case ucon64_FDS:
          return nes_fds (&rom);

        case ucon64_FDSL:
          return nes_fdsl (&rom);
#endif
        case ucon64_FFE:
          return nes_ffe (&rom);
  
        case ucon64_FIG:
          return snes_fig (&rom);
  
        case ucon64_GBX:
          return gameboy_gbx (&rom);
  
        case ucon64_GD3:
          return snes_gd3 (&rom);
  
        case ucon64_GDF:
          return snes_gdf (&rom);
  
        case ucon64_GG:
          switch (rom.console)
            {
            case ucon64_GB:
              return gameboy_gg (&rom);
#if 0
            case ucon64_GENESIS:
              return
#endif              
            case ucon64_NES:
              return nes_gg (&rom);
            case ucon64_SNES:
              return snes_gg (&rom);
            default:
//TODO error
              return -1;
            }
  
        case ucon64_GGD:
          switch (rom.console)
            {
            case ucon64_GB:
              return gameboy_ggd (&rom);
            case ucon64_GENESIS:
              return genesis_ggd (&rom);
            case ucon64_NES:
              return nes_ggd (&rom);
            case ucon64_SNES:
              return snes_ggd (&rom);
            default:
//TODO error
              return -1;
            }
  
        case ucon64_GGE:
          switch (rom.console)
            {
            case ucon64_GB:
              return gameboy_gge (&rom);
            case ucon64_GENESIS:
              return genesis_gge (&rom);
            case ucon64_NES:
              return nes_gge (&rom);
            case ucon64_SNES:
              return snes_gge (&rom);
            default:
//TODO error
              return -1;
            }
  
        case ucon64_INES:
          return nes_ines (&rom);
  
        case ucon64_INESHD:
          return nes_ineshd (&rom);
  
#if 0
        case ucon64_IP:
          break;
#endif
  
        case ucon64_J:
          switch (rom.console)
            {
            case ucon64_GENESIS:
              return genesis_j (&rom);
            case ucon64_NES:
              return nes_j (&rom);
            case ucon64_SNES:
              return snes_j (&rom);
            default:
//TODO error
              return -1;
            }

        case ucon64_K:
          return snes_k (&rom);
  
        case ucon64_L:
          return snes_l (&rom);

        case ucon64_LNX:
          return lynx_lnx (&rom);
  
        case ucon64_LOGO:
          return gbadvance_logo (&rom);
  
        case ucon64_LYX:
          return lynx_lyx (&rom);
  
        case ucon64_MGD:
          switch (rom.console)
            {
            case ucon64_GB:
              return gameboy_mgd (&rom);
            case ucon64_GENESIS:
              return genesis_mgd (&rom);
            case ucon64_NEOGEO:
              return neogeo_mgd (&rom);
            case ucon64_SNES:
              return snes_mgd (&rom);
            case ucon64_PCE:
              return pcengine_mgd (&rom);
            default:
//TODO error
              return -1;
            }
  
        case ucon64_MVS:
          return neogeo_mvs (&rom);
  
        case ucon64_N:
          switch (rom.console)
            {
            case ucon64_GB:
              return gameboy_n (&rom);
            case ucon64_GBA:
              return gbadvance_n (&rom);
            case ucon64_GENESIS:
              return genesis_n (&rom);
            case ucon64_LYNX:
              return lynx_n (&rom);
            case ucon64_N64:
              return nintendo64_n (&rom);
#if 0
            case ucon64_NES:
              return nes_n (&rom);
#endif
            case ucon64_SNES:
              return snes_n (&rom);
            default:
//TODO error 
              return -1;
            }
  
        case ucon64_N2:
          return genesis_n2 (&rom);
  
        case ucon64_N2GB:
          return gameboy_n2gb (&rom);
  
        case ucon64_NROT:
          return lynx_nrot (&rom);

        case ucon64_P:
          switch (rom.console)
            {
            case ucon64_GENESIS:
              return genesis_p (&rom);
            case ucon64_N64:
              return nintendo64_p (&rom);
            case ucon64_SNES:
              return snes_p (&rom);
            default:
  //TODO error
              return -1;
            }
  
        case ucon64_PAS:
          return nes_pas (&rom);
  
        case ucon64_ROTL:
          return lynx_rotl (&rom);
  
        case ucon64_ROTR:
          return lynx_rotr (&rom);

        case ucon64_S:
          switch (rom.console)
            {
            case ucon64_GENESIS:
              return genesis_s (&rom);
            case ucon64_NEOGEO:
              return neogeo_s (&rom);
            case ucon64_NES:
              return nes_s (&rom);
            case ucon64_SNES:
              return snes_s (&rom);
            default:
  //TODO error
              return -1;
            }
  
        case ucon64_SAM:
          return neogeo_sam (&rom);
  
        case ucon64_SGB:
          return gameboy_sgb (&rom);

        case ucon64_SMC:
          return snes_smc (&rom);
  
        case ucon64_SMD:
          return genesis_smd (&rom);

        case ucon64_SMDS:
          return genesis_smds (&rom);
  
        case ucon64_SMG:
          return pcengine_smg (&rom);
  
        case ucon64_SRAM:
          switch (rom.console)
            {
            case ucon64_GBA:
              return gbadvance_sram (&rom);
            case ucon64_N64:
              return nintendo64_sram (&rom);
            default:
  //TODO error
              return -1;
            }
          
        case ucon64_SSC:
          return gameboy_ssc (&rom);
  
        case ucon64_SWC:
          return snes_swc (&rom);
  
        case ucon64_UNIF:
          return nes_unif (&rom);
  
        case ucon64_USMS:
          return nintendo64_usms (&rom);
  
        case ucon64_V64:
          return nintendo64_v64 (&rom);
  
#ifdef BACKUP
        case ucon64_XDJR:
          return nintendo64_xdjr (&rom);
  
        case ucon64_XFAL:
          return gbadvance_xfal (&rom, -1);

        case ucon64_XFALS:
          return gbadvance_xfals (&rom);

        case ucon64_XGBX:
          return gameboy_xgbx (&rom);
  
        case ucon64_XGBXS:
          return gameboy_xgbxs (&rom);
  
        case ucon64_XSMD:
          return genesis_xsmd (&rom);
  
        case ucon64_XSMDS:
          return genesis_xsmds (&rom);

        case ucon64_XSWC:
          return snes_xswc (&rom);
  
        case ucon64_XSWCS:
          return snes_xswcs (&rom);
  
        case ucon64_XV64:
          return nintendo64_xv64 (&rom);
#endif  // BACKUP
  
        case ucon64_Z64:
          return nintendo64_z64 (&rom);
#if 0  
        case ucon64_MGH:
          return snes_mgh (&rom);
#endif
#ifdef BACKUP
        case ucon64_XFALB:
          return gbadvance_xfalb (&rom, strtol (optarg, NULL, 10));

        case ucon64_XFALC:
          return gbadvance_xfal (&rom, strtol (optarg, NULL, 10));

        case ucon64_XGBXB:
          return gameboy_xgbxb (&rom, strtol (optarg, NULL, 10));
#endif // BACKUP
  
        case ucon64_ATA:
        case ucon64_S16:
        case ucon64_COLECO:
        case ucon64_VBOY:
        case ucon64_SWAN:
        case ucon64_VEC:
        case ucon64_INTELLI:
        case ucon64_LYNX:
        case ucon64_SMS:
        case ucon64_NGP:
        case ucon64_NES:
        case ucon64_PCE:
        case ucon64_JAG:
        case ucon64_GEN:
        case ucon64_NG:
        case ucon64_N64:
        case ucon64_SNES:
        case ucon64_GBA:
        case ucon64_GB:
        case ucon64_SAT:
        case ucon64_PSX:
        case ucon64_PS2:
        case ucon64_CDI:
        case ucon64_CD32:
        case ucon64_3DO:
        case ucon64_DC:
        case ucon64_XBOX:
        case ucon64_GC:
        case ucon64_GP32:
        case ucon64_NS:
        case ucon64_INT:
        case ucon64_NINT:
        case ucon64_HD:
        case ucon64_NHD:
        case ucon64_HI:
        case ucon64_NHI:
        case ucon64_FRONTEND:
        case ucon64_NBAK:
        case ucon64_PORT:
        case ucon64_FILE:
        case ucon64_ROM:
          break;

        case ucon64_GETOPT_ERROR:
        default:
//          fprintf (STDERR, "Try '%s " OPTION_LONG_S "help' for more information.\n", argv[0]);
//          return -1;
          break;
      }
    }

#if 0
  int show_nfo;                 //show or skip info output for ROM
                                //values:
                                //0 show before processing of ROM (default)
                                //1 skip before and after processing of ROM
                                //2 show after processing of ROM
                                //3 show before and after processing of ROM
#endif

  if (!access (rom.rom, F_OK|R_OK) && ucon64.show_nfo >= 2)
    {
      if (!stat (rom.rom, &puffer))
        if (S_ISREG (puffer.st_mode))  
          {
#if 0
  int show_nfo;                 //show or skip info output for ROM
                                //values:
                                //0 show before processing of ROM (default)
                                //1 skip before and after processing of ROM
                                //2 show after processing of ROM
                                //3 show before and after processing of ROM
#endif
/*
            if (rom.console == ucon64_UNKNOWN) 
              {
                if (ucon64_init (rom.rom, &rom) == -1) ucon64.show_nfo = 1;
              }
*/              
            if (!ucon64.show_nfo || ucon64.show_nfo == 3)
              ucon64_nfo (&rom);
          }
    }

  return 0;
}


int
ucon64_init (char *romfile, struct rom_ *rombuf)
{
  if (romfile == NULL)
//  flush struct rombuf_
    {
      memset (rombuf, 0L, sizeof (struct rom_));
      rombuf->console = ucon64_UNKNOWN;

      return 0;
    }

  strcpy(rombuf->rom, romfile);

  rombuf->bytes = quickftell (rombuf->rom);

  ucon64_console_probe(rombuf);

  if (rombuf->console == ucon64_UNKNOWN)
    {
       printf ("ERROR: could not auto detect the right ROM/console type\n"
               "TIP:   If this is a ROM you might try to force the recognition\n"
               "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n");
       return -1;
    }

  if (ucon64.buheader_len != -1)
    rombuf->buheader_len = ucon64.buheader_len;

  if (ucon64.interleaved != -1)
    rombuf->interleaved = ucon64.interleaved;
    
  if (ucon64.splitted != -1)
    rombuf->splitted = ucon64.splitted;

  if (ucon64.snes_hirom != -1)
    rombuf->snes_hirom = ucon64.snes_hirom;

  quickfread (rombuf->buheader, rombuf->buheader_start, rombuf->buheader_len,
              rombuf->rom);
//  quickfread(rombuf->header, rombuf->header_start, rombuf->header_len, rombuf->rom);

  rombuf->mbit = (rombuf->bytes - rombuf->buheader_len) / (float) MBIT;

  if (rombuf->bytes <= MAXROMSIZE)
    {
      rombuf->padded = filetestpad (rombuf->rom);
      rombuf->intro = ((rombuf->bytes - rombuf->buheader_len) > MBIT) ?
        ((rombuf->bytes - rombuf->buheader_len) % MBIT) : 0;

      rombuf->splitted = ucon64_testsplit (rombuf->rom);

      rombuf->current_crc32 = fileCRC32 (rombuf->rom, rombuf->buheader_len);

      ucon64_dbsearch (rombuf);
    }

  return 0;
}


int
ucon64_console_probe (struct rom_ *rombuf)
{
  switch (rombuf->console)
    {
      case ucon64_GB:
        gameboy_init (rombuf);
        break;

      case ucon64_GBA:
        gbadvance_init (rombuf);
        break;

      case ucon64_GENESIS:
        genesis_init (rombuf);
        break;

      case ucon64_N64:
        nintendo64_init (rombuf);
        break;

      case ucon64_SNES:
        snes_init (rombuf);
        break;

      case ucon64_SMS:
        sms_init (rombuf);
        break;

      case ucon64_JAGUAR:
        jaguar_init (rombuf);
        break;

      case ucon64_LYNX:
        lynx_init (rombuf);
        break;

      case ucon64_NEOGEO:
        neogeo_init (rombuf);
        break;

      case ucon64_NES:
        nes_init (rombuf);
        break;

      case ucon64_PCE:
        pcengine_init (rombuf);
        break;

      case ucon64_SYSTEM16:
        system16_init (rombuf);
        break;

      case ucon64_ATARI:
        atari_init (rombuf);
        break;

      case ucon64_NEOGEOPOCKET:
        neogeopocket_init (rombuf);
        break;

      case ucon64_VECTREX:
        vectrex_init (rombuf);
        break;

      case ucon64_VIRTUALBOY:
        virtualboy_init (rombuf);
        break;

      case ucon64_WONDERSWAN:
        wonderswan_init (rombuf);
        break;

      case ucon64_COLECO:
        coleco_init (rombuf);
        break;

      case ucon64_INTELLI:
        intelli_init (rombuf);
        break;

      case ucon64_PS2:
        ps2_init (rombuf);
        break;

      case ucon64_DC:
        dc_init (rombuf);
        break;

      case ucon64_SATURN:
        saturn_init (rombuf);
        break;

      case ucon64_CDI:
        cdi_init (rombuf);
        break;

      case ucon64_CD32:
        cd32_init (rombuf);
        break;

      case ucon64_PSX:
        psx_init (rombuf);
        break;

      case ucon64_GAMECUBE:
        gamecube_init (rombuf);
        break;

      case ucon64_XBOX:
        xbox_init (rombuf);
        break;

      case ucon64_GP32:
        gp32_init (rombuf);
        break;

      case ucon64_REAL3DO:
        real3do_init (rombuf);
        break;

      case ucon64_UNKNOWN:
        if(rombuf->bytes <= MAXROMSIZE)
          rombuf->console =
            (!genesis_init (rombuf)) ? ucon64_GENESIS ://TODO correct defines assigned?
            (!snes_init (rombuf)) ? ucon64_SNES :
            (!nintendo64_init (rombuf)) ? ucon64_N64 :
//            (!gameboy_init (rombuf)) ? ucon64_GB :
            (!gbadvance_init (rombuf)) ? ucon64_GBA :
            (!nes_init (rombuf)) ? ucon64_NES :
            (!jaguar_init (rombuf)) ? ucon64_JAGUAR :
#ifdef DB
            (!atari_init (rombuf)) ? ucon64_ATARI :
            (!lynx_init (rombuf)) ? ucon64_LYNX :
            (!pcengine_init (rombuf)) ? ucon64_PCE :
            (!neogeo_init (rombuf)) ? ucon64_NEOGEO :
            (!neogeopocket_init (rombuf)) ? ucon64_NEOGEOPOCKET :
            (!sms_init (rombuf)) ? ucon64_SMS :
            (!system16_init (rombuf)) ? ucon64_SYSTEM16 :
            (!virtualboy_init (rombuf)) ? ucon64_VIRTUALBOY :
            (!vectrex_init (rombuf)) ? ucon64_VECTREX :
            (!coleco_init (rombuf)) ? ucon64_COLECO :
            (!intelli_init (rombuf)) ? ucon64_INTELLI :
            (!wonderswan_init (rombuf)) ? ucon64_WONDERSWAN :
#endif // DB
            (!gamecube_init (rombuf)) ? ucon64_GAMECUBE :
            (!xbox_init (rombuf)) ? ucon64_XBOX :
            (!gp32_init (rombuf)) ? ucon64_GP32 :
            (!cd32_init (rombuf)) ? ucon64_CD32 :
            (!cdi_init (rombuf)) ? ucon64_CDI :
            (!dc_init (rombuf)) ? ucon64_DC :
            (!ps2_init (rombuf)) ? ucon64_PS2 :
            (!psx_init (rombuf)) ? ucon64_PSX :
            (!real3do_init (rombuf)) ? ucon64_REAL3DO :
            (!saturn_init (rombuf)) ? ucon64_SATURN : ucon64_UNKNOWN;

            return (rombuf->console == ucon64_UNKNOWN) ? -1 : 0;
          
        default:
            rombuf->console = ucon64_UNKNOWN;
            return -1;
        }
  return 0;
}


/*
    this is the now centralized nfo output for all kinds of ROMs
*/
int
ucon64_nfo (struct rom_ *rombuf)
{
  char buf[4096];
  int n;

  printf ("%s\n%s\n\n", rombuf->rom, rombuf->copier);

  if (rombuf->header_len)
    {
      strhexdump (rombuf->header, 0, rombuf->header_start + rombuf->buheader_len,
                  rombuf->header_len);
      printf ("\n");
    }

  // some ROMs have a name with control chars in it -> replace control chars
  for (n = 0; n < strlen (rombuf->name); n++)
    buf[n] = isprint ((int) rombuf->name[n]) ? rombuf->name[n] : '.';
  buf[n] = 0;                   // terminate string

  printf ("%s\n%s\n%s%s%s\n%s\n%ld bytes (%.4f Mb)\n\n", rombuf->title, buf,
//        rombuf->name2, (rombuf->name2[0]) ? "\n": "",
          "", "",
          rombuf->manufacturer,
          rombuf->country, rombuf->bytes - rombuf->buheader_len, rombuf->mbit);

  if (rombuf->bytes <= MAXROMSIZE) // if it is no CD image
    {
      if (!rombuf->padded)
        printf ("Padded: No\n");
      else if (rombuf->padded)
        printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n", rombuf->padded,
                (float) rombuf->padded / MBIT);

//    if (!rombuf->intro)
//      printf("Intro/Trainer: No\n");
//    else
      if (rombuf->intro)
        printf ("Intro/Trainer: Maybe, %ld Bytes\n", rombuf->intro);

      if (!rombuf->buheader_len)
        printf ("Backup Unit Header: No\n");    // printing this is handy for
      else if (rombuf->buheader_len)               //  SNES ROMs
        printf ("Backup Unit Header: Yes, %ld Bytes\n", rombuf->buheader_len);

//    if (!rombuf->splitted)
//      printf("Splitted: No\n");
//    else
      if (rombuf->splitted)
        printf ("Splitted: Yes, %d parts\n",
                rombuf->splitted);
    }
  if (rombuf->misc[0])
    printf ("%s\n", rombuf->misc);

  if (rombuf->has_internal_crc)
    {
      sprintf (buf,
               "Checksum: %%s, 0x%%0%dlx (calculated) %%s= 0x%%0%dlx (internal)\n",
               rombuf->internal_crc_len * 2, rombuf->internal_crc_len * 2);
      printf (buf,
              (rombuf->current_internal_crc ==
               rombuf->internal_crc) ? "ok" : "bad",
              rombuf->current_internal_crc,
              (rombuf->current_internal_crc == rombuf->internal_crc) ? "=" : "!",
              rombuf->internal_crc);

      if (rombuf->internal_crc2[0])
        printf ("%s\n", rombuf->internal_crc2);
    }

  if (rombuf->current_crc32 != 0)
    printf ("Checksum (CRC32): 0x%08lx\n", rombuf->current_crc32);

  if (rombuf->splitted)
    printf ("NOTE: to get the correct checksum the ROM must be joined\n");

  printf ("\n");

  return 0;
}


int ucon64_e (struct rom_ *rombuf)
{
  int result, x;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[4096];
  char *property;

  if (rombuf->console == ucon64_UNKNOWN)
    {
       printf ("ERROR: could not auto detect the right ROM/console type\n"
               "TIP:   If this is a ROM you might try to force the recognition\n"
               "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n");
       return -1;
    }
 
  x = 0;
  while (long_options[x].name)
    {
      if (long_options[x].val == rombuf->console)
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


int ucon64_ls (char *path, int mode)
{
  struct dirent *ep;
  struct stat puffer;
  struct rom_ rom;
  int single_file = 0;
  char current_dir[FILENAME_MAX];
  DIR *dp;
  char buf[MAXBUFSIZE];

//TODO dir or single file?

  if (stat (path, &puffer) == -1)
    getcwd (path, FILENAME_MAX);
  else if (S_ISDIR (puffer.st_mode) != TRUE)
    getcwd (path, FILENAME_MAX);

  if ((dp = opendir (path)) == NULL)
    return -1;

  getcwd (current_dir,FILENAME_MAX);
  chdir (path);

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
              ucon64_init (NULL, &rom);
              if (ucon64_init (ep->d_name, &rom) != -1)
                switch (mode)
                  {
                    case ucon64_LSV:
                      ucon64_nfo (&rom);
                      fflush (stdout);
                      break;
/*TODO renamer!
                    case ucon64_REN:
                      if (rom.console != ucon64_UNKNOWN)
//                        && rom.console != ucon64_KNOWN)
                        {
                          strcpy (buf, &rom.rom[findlast (rom.rom, ".") + 1]);
                          printf ("%s.%s\n", rom.name, buf);
                        }
                      break;
*/
                    default:
                    case ucon64_LS:
                      strftime (buf, 13, "%b %d %H:%M",
                                localtime (&puffer.st_mtime));
//                      printf ("%-31.31s %10d %s %s\n", rom.name,
//                              (int) puffer.st_size, buf, rom.rom);
                      printf ("%-31.31s %10d %s %s\n", str2filename(rom.name),
                              (int) puffer.st_size, buf, rom.rom);
                      fflush (stdout);
                      break;
                }
            }
#ifdef UCON64_LS_SAVE
        }
    }
#endif // UCON64_LS_SAVE
  closedir (dp);

  chdir (current_dir);
  
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
  , getchd (buf2, FILENAME_MAX));

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
      newext (buf2, ".OLD");

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

  printf ("USAGE: %s [OPTION(S)] [" OPTION_LONG_S "rom=]ROM [[" OPTION_LONG_S "file=]FILE]\n\n"
           "  " OPTION_LONG_S "nbak        prevents backup files (*.bak)\n"
           "  " OPTION_LONG_S "hd=n        force ROM has backup unit/emulator header with n Bytes size\n"
           "NOTE: most backup units use a header with 512 Bytes size\n"
           "  " OPTION_LONG_S "nhd         force ROM has no backup unit/emulator header\n"
           "  " OPTION_LONG_S "int         force ROM is interleaved (2143)\n"
           "  " OPTION_LONG_S "nint        force ROM is not interleaved (1234)\n"
           "  " OPTION_LONG_S "ns          force ROM is not splitted\n"
#ifdef	__MSDOS__
           "  " OPTION_S "e           emulate/run ROM (see %s for more)\n"
#else
           "  " OPTION_S "e           emulate/run ROM (see %s for more)\n"
#endif
           "  " OPTION_LONG_S "crc         show CRC32 value of ROM\n"
//obsolete since -hd and -nhd are global   "  " OPTION_LONG_S "crchd       show CRC32 value of ROM (regarding to +512 Bytes header)\n"
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
           "  " OPTION_LONG_S "swap        swap/(de)interleave ALL Bytes in ROM (1234<->2143)\n"
           "  " OPTION_LONG_S "ispad       check if ROM is padded\n"
           "  " OPTION_LONG_S "pad         pad ROM to full Mb\n"
//obsolete since -hd and -nhd are global  "  " OPTION_LONG_S "padhd       pad ROM to full Mb (regarding to +512 Bytes header)\n"
           "  " OPTION_LONG_S "stp         strip first 512 Bytes (possible header) from ROM\n"
           "  " OPTION_LONG_S "ins         insert 512 Bytes (0x00) before ROM\n"
           "  " OPTION_LONG_S "strip       strip Bytes from end of ROM; " OPTION_LONG_S "file=VALUE\n"
           , argv[0], ucon64.configfile);

  bsl_usage ();
  ips_usage ();
  aps_usage ();
  pal4u_usage ();
  ppf_usage ();
  xps_usage ();

  cdrw_usage ();

  printf ("\n");

  optind = option_index = 0;//TODO is there a better way to "reset"?

  single = 0;

  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      if (single) break;
      
      switch (c)
        {
      case ucon64_GBA:
        gbadvance_usage ();
        single = 1;
        break;
      case ucon64_N64:
        nintendo64_usage ();
        single = 1;
        break;
      case ucon64_JAG:
        jaguar_usage ();
        single = 1;
        break;
      case ucon64_SNES:
        snes_usage ();
        single = 1;
        break;
      case ucon64_NG:
        neogeo_usage ();
        single = 1;
        break;
      case ucon64_NGP:
        neogeopocket_usage ();
        single = 1;
        break;
      case ucon64_GEN:
        genesis_usage ();
        single = 1;
        break;
      case ucon64_GB:
        gameboy_usage ();
        single = 1;
        break;
      case ucon64_LYNX:
        lynx_usage ();
        single = 1;
        break;
      case ucon64_PCE:
        pcengine_usage ();
        single = 1;
        break;
      case ucon64_SMS:
        sms_usage ();
        single = 1;
        break;
      case ucon64_NES:
        nes_usage ();
        single = 1;
        break;
      case ucon64_S16:
        sys16_usage ();
        single = 1;
        break;
      case ucon64_ATA:
        atari_usage ();
        single = 1;
        break;
      case ucon64_COLECO:
        coleco_usage ();
        single = 1;
        break;
      case ucon64_VBOY:
        virtualboy_usage ();
        single = 1;
        break;
      case ucon64_SWAN:
        wonderswan_usage ();
        single = 1;
        break;
      case ucon64_VEC:
        vectrex_usage ();
        single = 1;
        break;
      case ucon64_INTELLI:
        intelli_usage ();
        single = 1;
        break;
      case ucon64_DC:
        dc_usage ();
        single = 1;
        break;
      case ucon64_PSX:
        psx_usage ();
        single = 1;
        break;
      case ucon64_PS2:
        ps2_usage ();
        single = 1;
        break;
      case ucon64_SAT:
        saturn_usage ();
        single = 1;
        break;
      case ucon64_3DO:
        real3do_usage ();
        single = 1;
        break;
      case ucon64_CD32:
        cd32_usage ();
        single = 1;
        break;
      case ucon64_CDI:
        cdi_usage ();
        single = 1;
        break;
      case ucon64_GC:
        gamecube_usage ();
        single = 1;
        break;
      case ucon64_XBOX:
        xbox_usage ();
        single = 1;
        break;
      case ucon64_GP32:
        gp32_usage ();
        single = 1;
        break;
  
      default:
        break;
      }
    }

  if (!single)
    {
      gamecube_usage ();
      dc_usage ();
      psx_usage ();
#if 0
      ps2_usage ();
      sat_usage ();
      3do_usage ();
      cd32_usage ();
      cdi_usage ();
#endif
      printf ("%s\n%s\n%s\n%s\n%s\n%s\n"
              "  " OPTION_LONG_S "xbox, " OPTION_LONG_S "ps2, " OPTION_LONG_S "sat, " OPTION_LONG_S "3do, " OPTION_LONG_S "cd32, " OPTION_LONG_S "cdi\n"
              "                force recognition; NEEDED\n"
//            "  " OPTION_LONG_S "iso         force image is ISO9660\n"
//            "  " OPTION_LONG_S "raw         force image is MODE2_RAW/BIN\n"
              "  " OPTION_LONG_S "iso         convert RAW/BIN to ISO9660; " OPTION_LONG_S "rom=RAW_IMAGE\n",
              xbox_title,
              ps2_title, saturn_title, real3do_title, cd32_title,
              cdi_title);

      ppf_usage ();
      xps_usage ();

      cdrw_usage ();

      printf ("\n");

      gbadvance_usage ();
      nintendo64_usage ();
      snes_usage ();
      neogeopocket_usage ();
      neogeo_usage ();
      genesis_usage ();
      gameboy_usage ();
      jaguar_usage ();
      lynx_usage ();
      pcengine_usage ();
      sms_usage ();
      nes_usage ();
      wonderswan_usage ();
#if 0
      sys16_usage ();
      atari_usage ();
      coleco_usage ();
      virtualboy_usage ();
      wonderswan_usage ();
      vectrex_usage ();
      intelli_usage ();
#endif

      printf ("%s\n%s\n%s\n%s\n%s\n%s\n%s\n"
              "  " OPTION_LONG_S "s16, " OPTION_LONG_S "ata, " OPTION_LONG_S "coleco, " OPTION_LONG_S "vboy, " OPTION_LONG_S "vec, " OPTION_LONG_S "intelli, " OPTION_LONG_S "gp32\n"
              "                force recognition"
#ifndef DB
              "; NEEDED"
#endif
              "\n"
              "  " OPTION_LONG_S "hd          force ROM has header (+512 Bytes)\n"
              "  " OPTION_LONG_S "nhd         force ROM has no header\n"
              "\n", system16_title,
              atari_title, coleco_title, virtualboy_title,
              vectrex_title, intelli_title, gp32_title);
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
     , ucon64_dbsize (ucon64_UNKNOWN)
     , ucon64_dbsize (ucon64_UNKNOWN) - ucon64_DBSIZE
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
