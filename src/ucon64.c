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

static int ucon64_usage (int argc, char *argv[]);
static int ucon64_init (struct ucon64_ *rom);
static int ucon64_nfo (struct ucon64_ *rom);
static int ucon64_flush (int argc, char *argv[], struct ucon64_ *rom);
static void ucon64_exit (void);
static int ucon64_ls (int verbose);
static int ucon64_e (struct ucon64_ *rom);
static int ucon64_configfile (void);
static int ucon64_parport_probe (void); 

struct ucon64_ rom;

struct ucon64__ ucon64;

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
    {"ppf", 0, 0, ucon64_PPF},
    {"ps2", 0, 0, ucon64_PS2},
    {"psx", 0, 0, ucon64_PSX},
    {"rl", 0, 0, ucon64_RL},
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

  printf ("%s\n", ucon64_TITLE);
  printf ("Uses code from various people. See 'developers.html' for more!\n");
  printf ("This may be freely redistributed under the terms of the GNU Public License\n\n");

  if (argc < 2)
    {
       ucon64_usage (argc, argv);
       return 0;
    }

  ucon64_configfile ();

  ucon64.backup = ((!strcmp (getProperty (ucon64.configfile, "backups", buf2, "1"), "1")) ?
               1 : 0);


  ucon64.argc = argc;
  for (x = 0; x < argc; x++)ucon64.argv[x] = argv[x];

  ucon64_flush (argc, argv, &rom);

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

        case ucon64_A:
        case ucon64_B:
        case ucon64_BIOS:
        case ucon64_C:
        case ucon64_CHK:
        case ucon64_CRC:
        case ucon64_CRCHD:
        case ucon64_CRP:
        case ucon64_CS:
        case ucon64_DB:
        case ucon64_DBS:
        case ucon64_DBV:
        case ucon64_DINT:
        case ucon64_E:
        case ucon64_F:
//        case ucon64_FDS:
//        case ucon64_FDSL:
        case ucon64_FFE:
        case ucon64_FIG:
        case ucon64_FIND:
        case ucon64_GD3:
        case ucon64_GDF:
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
        case ucon64_K:
        case ucon64_L:
        case ucon64_LS:
        case ucon64_LSV:
        case ucon64_MGD:
//        case ucon64_MGH:
        case ucon64_MKA:
        case ucon64_MKCUE:
        case ucon64_MKI:
        case ucon64_MKPPF:
        case ucon64_MKTOC:
        case ucon64_MVS:
        case ucon64_N:
        case ucon64_N2:
        case ucon64_NA:
        case ucon64_NPPF:
        case ucon64_NROT:
        case ucon64_P:
        case ucon64_PAD:
        case ucon64_PADHD:
        case ucon64_PAS:
        case ucon64_PPF:
        case ucon64_RL:
        case ucon64_RU:
        case ucon64_S:
        case ucon64_SGB:
        case ucon64_SMC:
        case ucon64_SMD:
        case ucon64_SMDS:
        case ucon64_SMG:
        case ucon64_SRAM:
        case ucon64_SSC:
        case ucon64_STP:
        case ucon64_STRIP:
        case ucon64_SWAP:
        case ucon64_SWC:
        case ucon64_UNIF:
        case ucon64_USMS:
        case ucon64_V64:
#ifdef BACKUP_CD
        case ucon64_XCDRW:
#endif // BACKUP_CD
        case ucon64_Z64:
          break;
  
        case ucon64_SWCS:
        case ucon64_FIGS:
        case ucon64_UFOS:
        case ucon64_COL:
          ucon64.show_nfo = 1;
        case ucon64_HI:
          rom.snes_hirom = 1;
        case ucon64_NHI:
          rom.snes_hirom = 0;
        case ucon64_SNES:
#ifdef BACKUP
        case ucon64_XSWC:
        case ucon64_XSWCS:
#endif // BACKUP
          rom.console = ucon64_SNES;
          break;

        case ucon64_IP:
          rom.console = ucon64_DC;
          break;
  
#ifdef BACKUP
        case ucon64_XGBX:
        case ucon64_XGBXS:
        case ucon64_XGBXB:
#endif // BACKUP
        case ucon64_GB:
        case ucon64_GBX:
        case ucon64_N2GB:
          rom.console = ucon64_GB;
          break;

        case ucon64_SAM:
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
        case ucon64_LNX:
        case ucon64_LYX:
          rom.console = ucon64_LYNX;
          break;
  
#ifdef BACKUP
//        case ucon64_XSMD://could be GENESIS too
//        case ucon64_XSMDS:
#endif // BACKUP
        case ucon64_SMS:
          rom.console = ucon64_SMS;
          break;
  
        case ucon64_NGP:
          rom.console = ucon64_NEOGEOPOCKET;
          break;
  
        case ucon64_INES:
        case ucon64_INESHD:
        case ucon64_NES:
          rom.console = ucon64_NES;
          break;
  
        case ucon64_PCE:
          rom.console = ucon64_PCE;
          break;
  
        case ucon64_JAG:
          rom.console = ucon64_JAGUAR;
          break;
  
#ifdef BACKUP
//        case ucon64_XSMD://could be SMS too
//        case ucon64_XSMDS:
#endif // BACKUP
        case ucon64_GEN:
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
        case ucon64_N64:
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

        case ucon64_NS:
          rom.splitted[0] = 0;
          break;

        case ucon64_HD:
          rom.buheader_start = unknown_bu512_HEADER_START;
          rom.buheader_len = unknown_bu512_HEADER_LEN;
          strcpy (rom.copier, unknown_bu512_title);
          break;

        case ucon64_NHD:
          rom.buheader_start = unknown_bu_HEADER_START;
          rom.buheader_len = unknown_bu_HEADER_LEN;
          strcpy (rom.copier, unknown_bu_title);
          break;

        case ucon64_INT:
          rom.interleaved = 1;
          break;

        case ucon64_INT2:
          rom.interleaved = 2;
          break;

        case ucon64_NINT:
          rom.interleaved = 0;
          break;

        default:
          fprintf (STDERR, "Try '%s " OPTION_LONG_S "help' for more information.\n", argv[0]);
          return -1;
      }
    }


  if (optind < argc)
    strcpy(rom.rom, argv[optind++]);
  if (optind < argc)
    strcpy(ucon64.file, argv[optind++]);
                                    
  ucon64.parport = atoi (ucon64.file);
  ucon64_parport_probe ();

  if (!access (rom.rom, F_OK|R_OK))
    {
      if (!stat (rom.rom, &puffer))
        if (S_ISREG (puffer.st_mode))  
          {
            ucon64_init (&rom);
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
          return ucon64_usage (argc, argv);

        case ucon64_CRC:
          printf ("Checksum (CRC32): %08lx\n\n", fileCRC32 (rom.rom, 0));
          return 0;

        case ucon64_CRCHD:
          printf ("Checksum (CRC32): %08lx\n\n", fileCRC32 (rom.rom, 512));
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
  
        case ucon64_PAD:
          ucon64_fbackup (rom.rom);
  
          return filepad (rom.rom, 0, MBIT);
  
        case ucon64_PADHD:
          ucon64_fbackup (rom.rom);
          return filepad (rom.rom, 512, MBIT);
  
        case ucon64_ISPAD:
          if ((padded = filetestpad (rom.rom)) != -1)
            {
              if (!padded)
                printf ("Padded: No\n");
              else
                printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n", padded,
                        (float) padded / MBIT);
            }
          printf ("\n");
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
          return addppfid (argc, argv);
  
        case ucon64_LS:
          return ucon64_ls (0);
  
        case ucon64_LSV:
          return ucon64_ls (1);
  
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
                  getarg (argc, argv, ucon64_NAME));
          break;

        case ucon64_DBS:
          ucon64_flush (argc, argv, &rom);
          sscanf (rom.rom, "%lx", &rom.current_crc32);
          ucon64_dbsearch (&rom);
          ucon64_nfo (&rom);
//          ucon64_dbview (rom.console);
          printf ("TIP: %s "OPTION_LONG_S "dbs "OPTION_LONG_S "nes would search only for a NES ROM\n\n",
                  getarg (argc, argv, ucon64_NAME));
  
          break;
  
        case ucon64_DBV:
          ucon64_dbview (rom.console);
  
          printf ("\nTIP: %s " OPTION_LONG_S "db " OPTION_LONG_S "nes would view only NES ROMs\n\n",
                  getarg (argc, argv, ucon64_NAME));
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
  
/*
        case ucon64_FDS:
          return nes_fds (&rom);
*/
/*
        case ucon64_FDSL:
          return nes_fdsl (&rom);
*/
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
//      case ucon64_GENESIS:
//        return
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
  
/*
        case ucon64_IP:
          break;
*/
  
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
/*
            case ucon64_NES:
              return nes_n (&rom);
*/
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
  
  /*      case ucon64_MGH:
          return snes_mgh (&rom);
  */
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
        case ucon64_FRONTEND:
        case ucon64_NBAK:
          break;

        default:
          fprintf (STDERR, "Try '%s " OPTION_LONG_S "help' for more information.\n", argv[0]);
          return -1;
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
//          if (rom.console == ucon64_UNKNOWN) ucon64_init (&rom);
            if (!ucon64.show_nfo || ucon64.show_nfo == 3)
              ucon64_nfo (&rom);
          }
    }

  return 0;
}


/*
    flush the ucon64 struct with default values
*/
int
ucon64_flush (int argc, char *argv[], struct ucon64_ *rom)
{
  memset (rom, 0L, sizeof (struct ucon64_));

  strcpy (rom->rom, getarg (argc, argv, ucon64_ROM));
  rom->bytes = quickftell (rom->rom);
  rom->splitted[0] = ucon64_testsplit (rom->rom);

  rom->console = ucon64_UNKNOWN;        //integer for the console system

  strcpy (rom->name, "?");
  strcpy (rom->manufacturer, "Unknown Manufacturer");
  strcpy (rom->country, "Unknown Country");

  return 0;
}

int
ucon64_init (struct ucon64_ *rom)
{
  rom->bytes = quickftell (rom->rom);

  rom->current_crc32 = (rom->bytes <= MAXROMSIZE) ? 
    fileCRC32 (rom->rom, rom->buheader_len) : 0;

  quickfread (rom->buheader, rom->buheader_start, rom->buheader_len,
              rom->rom);
//  quickfread(rom->header, rom->header_start, rom->header_len, rom->rom);

//  rom->bytes = quickftell (rom->rom);
  rom->mbit = (rom->bytes - rom->buheader_len) / (float) MBIT;

  if (rom->bytes <= MAXROMSIZE)
    {
      rom->padded = filetestpad (rom->rom);
      rom->intro = ((rom->bytes - rom->buheader_len) > MBIT) ?
        ((rom->bytes - rom->buheader_len) % MBIT) : 0;
    }

  if (rom->console != ucon64_UNKNOWN)
    {
      switch (rom->console)
        {
        case ucon64_GB:
          gameboy_init (rom);
          break;

        case ucon64_GBA:
          gbadvance_init (rom);
          break;

        case ucon64_GENESIS:
          genesis_init (rom);
          break;

        case ucon64_N64:
          nintendo64_init (rom);
          break;

        case ucon64_SNES:
          snes_init (rom);
          break;

        case ucon64_SMS:
          sms_init (rom);
          break;

        case ucon64_JAGUAR:
          jaguar_init (rom);
          break;

        case ucon64_LYNX:
          lynx_init (rom);
          break;

        case ucon64_NEOGEO:
          neogeo_init (rom);
          break;

        case ucon64_NES:
          nes_init (rom);
          break;

        case ucon64_PCE:
          pcengine_init (rom);
          break;

        case ucon64_SYSTEM16:
          system16_init (rom);
          break;

        case ucon64_ATARI:
          atari_init (rom);
          break;

        case ucon64_NEOGEOPOCKET:
          neogeopocket_init (rom);
          break;

        case ucon64_VECTREX:
          vectrex_init (rom);
          break;

        case ucon64_VIRTUALBOY:
          virtualboy_init (rom);
          break;

        case ucon64_WONDERSWAN:
          wonderswan_init (rom);
          break;

        case ucon64_COLECO:
          coleco_init (rom);
          break;

        case ucon64_INTELLI:
          intelli_init (rom);
          break;

        case ucon64_PS2:
          ps2_init (rom);
          break;

        case ucon64_DC:
          dc_init (rom);
          break;

        case ucon64_SATURN:
          saturn_init (rom);
          break;

        case ucon64_CDI:
          cdi_init (rom);
          break;

        case ucon64_CD32:
          cd32_init (rom);
          break;

        case ucon64_PSX:
          psx_init (rom);
          break;

        case ucon64_GAMECUBE:
          gamecube_init (rom);
          break;

        case ucon64_XBOX:
          xbox_init (rom);
          break;

        case ucon64_UNKNOWN:
          if(rom->bytes <= MAXROMSIZE)
            rom->console =
              (!(snes_init (rom))) ? ucon64_SNES :
              (!(genesis_init (rom))) ? ucon64_GENESIS :
              (!(nintendo64_init (rom))) ? ucon64_N64 :
              (!(gameboy_init (rom))) ? ucon64_GB :
              (!(gbadvance_init (rom))) ? ucon64_GBA :
              (!(nes_init (rom))) ? ucon64_NES :
              (!(jaguar_init (rom))) ? ucon64_JAG :
#ifdef DB
              (!(atari_init (rom))) ? ucon64_ATARI :
              (!(lynx_init (rom))) ? ucon64_LYNX :
              (!(pcengine_init (rom))) ? ucon64_PCE :
              (!(neogeo_init (rom))) ? ucon64_NEOGEO :
              (!(neogeopocket_init (rom))) ? ucon64_NGP :
              (!(sms_init (rom))) ? ucon64_SMS :
              (!(system16_init (rom))) ? ucon64_SYSTEM16 :
              (!(virtualboy_init (rom))) ? ucon64_VBOY :
              (!(vectrex_init (rom))) ? ucon64_VECTREX :
              (!(coleco_init (rom))) ? ucon64_COLECO :
              (!(intelli_init (rom))) ? ucon64_INTELLI :
              (!(wonderswan_init (rom))) ? ucon64_WONDERSWAN :
#endif // DB
          //detection for the these consoles is not implemented yet
              (!(gamecube_init (rom))) ? ucon64_GAMECUBE :
              (!(xbox_init (rom))) ? ucon64_XBOX :
              (!(gp32_init (rom))) ? ucon64_GP32 :
              (!(cd32_init (rom))) ? ucon64_CD32 :
              (!(cdi_init (rom))) ? ucon64_CDI :
              (!(dc_init (rom))) ? ucon64_DC :
              (!(ps2_init (rom))) ? ucon64_PS2 :
              (!(psx_init (rom))) ? ucon64_PSX :
              (!(real3do_init (rom))) ? ucon64_REAL3DO :
              (!(saturn_init (rom))) ? ucon64_SAT : ucon64_UNKNOWN;
          break;
          
        default:
            rom->console = ucon64_UNKNOWN;
          break;
        }
      }

  return (rom->console == ucon64_UNKNOWN) ? -1 : 0;
}



/*
    this is the now centralized nfo output for all kinds of ROMs
*/
int
ucon64_nfo (struct ucon64_ *rom)
{
  char buf[4096];
  int n;

  printf ("%s\n%s\n\n", rom->rom, rom->copier);

  if (rom->header_len)
    {
      strhexdump (rom->header, 0, rom->header_start + rom->buheader_len,
                  rom->header_len);
      printf ("\n");
    }

  // some ROMs have a name with control chars in it -> replace control chars
  for (n = 0; n < strlen (rom->name); n++)
    buf[n] = isprint ((int) rom->name[n]) ? rom->name[n] : '.';
  buf[n] = 0;                   // terminate string

  printf ("%s\n%s\n%s%s%s\n%s\n%ld bytes (%.4f Mb)\n\n", rom->title, buf,
//        rom->name2, (rom->name2[0]) ? "\n": "",
          "", "",
          rom->manufacturer,
          rom->country, rom->bytes - rom->buheader_len, rom->mbit);

  if (rom->bytes <= MAXROMSIZE) // if it is no CD image
    {
      if (!rom->padded)
        printf ("Padded: No\n");
      else if (rom->padded)
        printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n", rom->padded,
                (float) rom->padded / MBIT);

//    if (!rom->intro)
//      printf("Intro/Trainer: No\n");
//    else
      if (rom->intro)
        printf ("Intro/Trainer: Maybe, %ld Bytes\n", rom->intro);

      if (!rom->buheader_len)
        printf ("Backup Unit Header: No\n");    // printing this is handy for
      else if (rom->buheader_len)               //  SNES ROMs
        printf ("Backup Unit Header: Yes, %ld Bytes\n", rom->buheader_len);

//    if (!rom->splitted[0])
//      printf("Splitted: No\n");
//    else
      if (rom->splitted[0])
        printf ("Splitted: Yes, %d parts\n",
                rom->splitted[0]);
    }
  if (rom->misc[0])
    printf ("%s\n", rom->misc);

  if (rom->has_internal_crc)
    {
      sprintf (buf,
               "Checksum: %%s, 0x%%0%dlx (calculated) %%s= 0x%%0%dlx (internal)\n",
               rom->internal_crc_len * 2, rom->internal_crc_len * 2);
      printf (buf,
              (rom->current_internal_crc ==
               rom->internal_crc) ? "ok" : "bad",
              rom->current_internal_crc,
              (rom->current_internal_crc == rom->internal_crc) ? "=" : "!",
              rom->internal_crc);

      if (rom->internal_crc2[0])
        printf ("%s\n", rom->internal_crc2);
    }

  if (rom->current_crc32 != 0)
    printf ("Checksum (CRC32): 0x%08lx\n", rom->current_crc32);

  if (rom->splitted[0])
    printf ("NOTE: to get the correct checksum the ROM must be joined\n");

  printf ("\n");

  return 0;
}


int ucon64_e(struct ucon64_ *rom)
{
  int result, x;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[4096];
  char *property;

  if (rom->console == ucon64_UNKNOWN)
    {
       printf ("ERROR: could not auto detect the right ROM/console type\n"
               "TIP:   If this is a ROM you might try to force the recognition\n"
               "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n");
       return -1;
    }
 
  x = 0;
  while (long_options[x].name)
    {
      if (long_options[x].val == rom->console)
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

int ucon64_ls (int verbose)
{
  int ucon64_argc;
  struct dirent *ep;
  struct stat puffer;
  char current_dir[FILENAME_MAX];
  DIR *dp;
  char buf[MAXBUFSIZE], *ucon64_argv[128];

  if (stat (rom.rom, &puffer) == -1)
    getcwd (rom.rom, FILENAME_MAX);
  else if (S_ISDIR (puffer.st_mode) != TRUE)
    getcwd (rom.rom, FILENAME_MAX);

  if ((dp = opendir (rom.rom)) == NULL)
    return -1;

  getcwd (current_dir,FILENAME_MAX);
  chdir (rom.rom);

  while ((ep = readdir (dp)) != 0)
    {
      if (!stat (ep->d_name, &puffer))
        {
          if (S_ISREG (puffer.st_mode))
            {
              ucon64_argv[0] = "ucon64";
              ucon64_argv[1] = ep->d_name;
              ucon64_argc = 2;

              ucon64_flush (ucon64_argc, ucon64_argv, &rom);
              strcpy (rom.rom, ep->d_name);
              ucon64_init (&rom);

              if (verbose == 0)
                {
                  strftime (buf, 13, "%b %d %H:%M",
                            localtime (&puffer.st_mtime));
                  printf ("%-31.31s %10d %s %s\n", rom.name,
                          (int) puffer.st_size, buf, rom.rom);
                }
              else if (verbose == 1)
                ucon64_nfo (&rom);
/*TODO renamer!
              else if (argcmp (argc, argv, "-rrom") &&
                       rom.console != ucon64_UNKNOWN)
                       // && rom.console != ucon64_KNOWN)
                {
                  strcpy (buf, &rom.rom[findlast (rom.rom, ".") + 1]);
                  printf ("%s.%s\n", rom.name, buf);
                }
*/
              fflush (stdout);
            }
        }
    }
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

int
ucon64_parport_probe(void)
{
#if     defined BACKUP && defined __unix__
  uid_t uid;
  gid_t gid;
#endif
/*
  parallel port probing and handling
*/
#ifdef  BACKUP
  if (ucon64.file[0])
    sscanf (ucon64.file, "%x", &ucon64.parport);

  if (!(ucon64.parport = parport_probe (ucon64.parport)))
    ;
/*
    printf ("ERROR: no parallel port 0x%s found\n\n", strupr (buf));
  else
    printf ("0x%x\n\n", ucon64.parport);
*/

#ifdef  __unix__
  /*
    Some code needs us to switch to the real uid and gid. However, other code
    needs access to I/O ports other than the standard printer port registers.
    We just do an iopl(3) and all code should be happy. Using iopl(3) enables
    users to run all code without being root (of course with the uCON64
    executable setuid root). Anyone a better idea?
  */
#ifdef  __linux__
  if (iopl (3) == -1)
    {
      fprintf (stderr, "Could not set the I/O privilege level to 3\n"
                       "(This program needs root privileges)\n");
      return 1;
    }
#endif

  // now we can drop privileges
  uid = getuid ();
  if (setuid (uid) == -1)
    {
      fprintf (stderr, "Could not set uid\n");
      return 1;
    }
  gid = getgid ();                              // This shouldn't be necessary
  if (setgid (gid) == -1)                       //  if `make install' was
    {                                           //  used, but just in case
      fprintf (stderr, "Could not set gid\n");  //  (root did `chmod +s')
      return 1;
    }
#endif // __unix__
#endif // BACKUP
  return 0;
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



int
ucon64_usage (int argc, char *argv[])
{
  int c = 0;
  int option_index = 0;
  int single = 0;

  printf ("USAGE: %s [OPTION(S)] ROM [FILE]\n\n"
           "  " OPTION_LONG_S "nbak        prevents backup files (*.bak)\n"
#ifdef	__MSDOS__
           "  " OPTION_S "e           emulate/run ROM (see ucon64.cfg for more)\n"
#else
           "  " OPTION_S "e           emulate/run ROM (see $HOME/.ucon64rc for more)\n"
#endif
           "  " OPTION_LONG_S "crc         show CRC32 value of ROM\n"
           "  " OPTION_LONG_S "crchd       show CRC32 value of ROM (regarding to +512 Bytes header)\n"
           "  " OPTION_LONG_S "dbs         search ROM database (all entries) by CRC32; $ROM=0xCRC32\n"
           "  " OPTION_LONG_S "db          ROM database statistics (# of entries)\n"
           "  " OPTION_LONG_S "dbv         view ROM database (all entries)\n"
           "  " OPTION_LONG_S "ls          generate ROM list for all ROMs; $ROM=DIRECTORY\n"
           "  " OPTION_LONG_S "lsv         like " OPTION_LONG_S "ls but more verbose; $ROM=DIRECTORY\n"
//         "TODO:  " OPTION_LONG_S "rrom   rename all ROMs in DIRECTORY to their internal names; $ROM=DIR\n"
//         "TODO:  " OPTION_LONG_S "rr83   like " OPTION_LONG_S "rrom but with 8.3 filenames; $ROM=DIR\n"
//         "               this is often used by people who loose control of their ROMs\n"
           "  " OPTION_LONG_S "rl          rename all files in DIRECTORY to lowercase; $ROM=DIRECTORY\n"
           "  " OPTION_LONG_S "ru          rename all files in DIRECTORY to uppercase; $ROM=DIRECTORY\n"
#ifdef	__MSDOS__
           "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex $ROM|more\"\n"
#else
           "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex $ROM|less\"\n"       // less is better ;-)
#endif
           "  " OPTION_LONG_S "find        find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)\n"
           "  " OPTION_S "c           compare ROMs for differencies; $FILE=OTHER_ROM\n"
           "  " OPTION_LONG_S "cs          compare ROMs for similarities; $FILE=OTHER_ROM\n"
           "  " OPTION_LONG_S "swap        swap/(de)interleave ALL Bytes in ROM (1234<->2143)\n"
           "  " OPTION_LONG_S "ispad       check if ROM is padded\n"
           "  " OPTION_LONG_S "pad         pad ROM to full Mb\n"
           "  " OPTION_LONG_S "padhd       pad ROM to full Mb (regarding to +512 Bytes header)\n"
           "  " OPTION_LONG_S "stp         strip first 512 Bytes (possible header) from ROM\n"
           "  " OPTION_LONG_S "ins         insert 512 Bytes (0x00) before ROM\n"
           "  " OPTION_LONG_S "strip       strip Bytes from end of ROM; $FILE=VALUE\n"
           ,getarg (argc, argv, ucon64_NAME));

  bsl_usage ();
  ips_usage ();
  aps_usage ();
  pal4u_usage ();
  ppf_usage ();
  xps_usage ();

  cdrw_usage ();

  printf ("\n");

  optind = option_index = 0;//TODO is there a better way to "reset"?

  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      single = 1;
      
      switch (c)
        {
      case ucon64_GBA:
        gbadvance_usage ();
        break;
      case ucon64_N64:
        nintendo64_usage ();
        break;
      case ucon64_JAG:
        jaguar_usage ();
        break;
      case ucon64_SNES:
        snes_usage ();
        break;
      case ucon64_NG:
        neogeo_usage ();
        break;
      case ucon64_NGP:
        neogeopocket_usage ();
        break;
      case ucon64_GEN:
        genesis_usage ();
        break;
      case ucon64_GB:
        gameboy_usage ();
        break;
      case ucon64_LYNX:
        lynx_usage ();
        break;
      case ucon64_PCE:
        pcengine_usage ();
        break;
      case ucon64_SMS:
        sms_usage ();
        break;
      case ucon64_NES:
        nes_usage ();
        break;
      case ucon64_S16:
        sys16_usage ();
        break;
      case ucon64_ATA:
        atari_usage ();
        break;
      case ucon64_COLECO:
        coleco_usage ();
        break;
      case ucon64_VBOY:
        virtualboy_usage ();
        break;
      case ucon64_SWAN:
        wonderswan_usage ();
        break;
      case ucon64_VEC:
        vectrex_usage ();
        break;
      case ucon64_INTELLI:
        intelli_usage ();
        break;
      case ucon64_DC:
        dc_usage ();
        break;
      case ucon64_PSX:
        psx_usage ();
        break;
      case ucon64_PS2:
        ps2_usage ();
        break;
      case ucon64_SAT:
        saturn_usage ();
        break;
      case ucon64_3DO:
        real3do_usage ();
        break;
      case ucon64_CD32:
        cd32_usage ();
        break;
      case ucon64_CDI:
        cdi_usage ();
        break;
      case ucon64_GC:
        gamecube_usage ();
        break;
      case ucon64_XBOX:
        xbox_usage ();
        break;
      case ucon64_GP32:
        gp32_usage ();
        break;
  
      default:
        single = 0;
        break;
      }
    }

  if (!single)
    {
      gamecube_usage ();
      dc_usage ();
      psx_usage ();
/*
      ps2_usage ();
      sat_usage ();
      3do_usage ();
      cd32_usage ();
      cdi_usage ();
*/
      printf ("%s\n%s\n%s\n%s\n%s\n%s\n"
              "  " OPTION_LONG_S "xbox, " OPTION_LONG_S "ps2, " OPTION_LONG_S "sat, " OPTION_LONG_S "3do, " OPTION_LONG_S "cd32, " OPTION_LONG_S "cdi\n"
              "                force recognition; NEEDED\n"
//            "  " OPTION_LONG_S "iso         force image is ISO9660\n"
//            "  " OPTION_LONG_S "raw         force image is MODE2_RAW/BIN\n"
              "  " OPTION_LONG_S "iso         convert RAW/BIN to ISO9660; $ROM=RAW_IMAGE\n",
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
/*
      sys16_usage ();
      atari_usage ();
      coleco_usage ();
      virtualboy_usage ();
      wonderswan_usage ();
      vectrex_usage ();
      intelli_usage ();
*/

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
     , getarg (argc, argv, ucon64_NAME), getarg (argc, argv, ucon64_NAME)
   );

  return 0;
/*
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
     Super
    Nintendo
    Neo·Geo
   Game Gear
      Lynx
    GameBoy
  Turbo Grafx
       16
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
*/
}
