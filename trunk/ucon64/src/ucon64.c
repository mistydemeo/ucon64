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
#include "backup/mccl.h"

static void ucon64_exit (void);
//static void usage (const char **usage);
static void ucon64_usage (int argc, char *argv[]);

st_ucon64_t ucon64;

static const char *ucon64_title = "uCON64 " UCON64_VERSION_S " " CURRENT_OS_S
#if 0
                             "/" CURRENT_ENDIAN_S
#endif
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
    {"pas", 0, 0, UCON64_PAS},
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
  int ucon64_argc;
  int c = 0, result = 0;
  long size = 0;
  int value = 0;
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

  ucon64.show_nfo = UCON64_YES;

  ucon64.type =
  ucon64.buheader_len =
  ucon64.interleaved =
  ucon64.split =
  ucon64.snes_hirom =
  ucon64.bs_dump =
  ucon64.fal_size =
  ucon64.console =
  ucon64.do_not_calc_crc = UCON64_UNKNOWN;

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

  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
#include "switches.c"
    }

  if (optind < argc)
    ucon64.rom = argv[optind++];
  if (optind < argc)
    ucon64.file = argv[optind++];

#ifdef BACKUP
  if (ucon64.file)
    sscanf (ucon64.file, "%x", &ucon64.parport);
#endif

  if (!ucon64_init (ucon64.rom, &rom))
    if (ucon64.show_nfo == UCON64_YES)
      ucon64_nfo (&rom);
  ucon64.show_nfo = UCON64_NO;

  optind = option_index = 0;

  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
#include "options.c"
    }

  if (ucon64.show_nfo == UCON64_YES)
    if (!ucon64_init (ucon64.rom, &rom))
      ucon64_nfo (&rom);

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
            (!genesis_init (ucon64_flush (rominfo))) ? UCON64_GENESIS :
            (!snes_init (ucon64_flush (rominfo))) ? UCON64_SNES :
            (!gameboy_init (ucon64_flush (rominfo))) ? UCON64_GB :
            (!lynx_init (ucon64_flush (rominfo))) ? UCON64_LYNX :
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

  if (access (romfile, F_OK | R_OK) == -1)
    return result;
  if (!stat (romfile, &puffer) == -1)
    return result;
  if (S_ISREG (puffer.st_mode) != TRUE)
    return result;

  size = quickftell (romfile);

/*
  currently the media type is determined by its size
*/
  if (ucon64.type == UCON64_UNKNOWN)
    ucon64.type = (size <= MAXROMSIZE) ? UCON64_ROM : UCON64_CD;

  ucon64_flush (rominfo);

  result = ucon64_console_probe (rominfo);

  if (UCON64_TYPE_ISROM (ucon64.type))
    {
#if 0
      rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
        ucon64.buheader_len : rominfo->buheader_len;
#endif

      rominfo->current_crc32 = fileCRC32 (romfile, rominfo->buheader_len);

#ifdef DB
      switch (ucon64.console)
        {
          case UCON64_SNES:
          case UCON64_GENESIS:
          case UCON64_GB:
          case UCON64_GBA:
          case UCON64_N64:
//These ROMs have internal headers with name, country, maker, etc.
            break;

          default:
            ucon64_dbsearch (rominfo);
            break;
        }
#endif // DB
    }
  else if (UCON64_TYPE_ISCD (ucon64.type))
    {
      st_iso_header_t iso_header;
      int value;

//      ucon64_flush (rominfo);

      result = 0;

      quickfread (&iso_header, ISO_HEADER_START +
          UCON64_ISSET (ucon64.buheader_len) ?
            ucon64.buheader_len :
            CDRW_HEADER_START (ucon64_trackmode_probe (romfile)),
              ISO_HEADER_LEN, romfile);
      rominfo->header_start = ISO_HEADER_START;
      rominfo->header_len = ISO_HEADER_LEN;
      rominfo->header = &iso_header;

//CD internal name
      strcpy (rominfo->name, iso_header.volume_id);

//CD maker
      rominfo->maker = iso_header.publisher_id;

//misc stuff
      value = ucon64_trackmode_probe (romfile);
      if (value == -1)
        strcpy (rominfo->misc, "Track Mode: Unknown (Maybe CDI or NRG?)\n");
      else
        sprintf (rominfo->misc, "Track Mode: %s (Cdrdao: %s)\n", track_modes[value].common, track_modes[value].cdrdao);

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

  if (rominfo->buheader && rominfo->buheader_len && rominfo->buheader_len != SMC_HEADER_LEN)
    {
      memhexdump (rominfo->buheader, rominfo->buheader_len, rominfo->buheader_start);
      printf ("\n");
    }

  if (rominfo->copier_usage != NULL)
    {
      strcpy (buf, rominfo->copier_usage[0]);
      printf ("%s\n", mkprint (buf, '.'));

      if (rominfo->copier_usage[1])
        {
          strcpy (buf, rominfo->copier_usage[1]);
          printf ("  %s\n", mkprint (buf, '.'));
        }
      printf ("\n");
    }

  if (rominfo->header && rominfo->header_len)
    {
      memhexdump (rominfo->header, rominfo->header_len,
        rominfo->header_start + rominfo->buheader_len);
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

  strcpy (buf, NULL_TO_EMPTY (rominfo->name));
  printf ("%s\n%s\n%s\n%ld Bytes (%.4f Mb)\n\n",
          // some ROMs have a name with control chars in it -> replace control chars
          mkprint (buf, '.'),
          NULL_TO_EMPTY (rominfo->maker),
          NULL_TO_EMPTY (rominfo->country),
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
      int split = (UCON64_ISSET (ucon64.split)) ? ucon64.split :
        ucon64_testsplit (ucon64.rom);

      if (!padded)
        printf ("Padded: No\n");
      else if (padded)
        printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n", padded,
                TOMBIT_F (padded));

      if (intro)
        printf ("Intro/Trainer: Maybe, %ld Bytes\n", intro);

      printf ("Interleaved/Swapped: %s\n",
        rominfo->interleaved ?
          (rominfo->interleaved > 1 ?
            "Yes (2)" :                         // printing this is handy for SNES ROMs
            "Yes") :
          "No");

      if (rominfo->buheader_len)
        printf ("Backup unit/Emulator header: Yes, %ld Bytes\n",
          rominfo->buheader_len);
      else
// for NoisyB: <read only mode ON>
        printf ("Backup unit/Emulator header: No\n"); // printing No is handy for SNES ROMs
// for NoisyB: <read only mode OFF>

      if (split)
        printf ("Split: Yes, %d parts\n"
          "NOTE: to get the correct checksum the ROM must be joined\n", split);
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

      printf ("\n");

      if (ucon64.console == UCON64_UNKNOWN)
        fprintf (stderr, ucon64_console_error);
    }

  fflush (stdout);

  return 0;
}


#if 1
#define UCON64_USAGE(s) printf("%s%s%s%s%s", \
                          NULL_TO_EMPTY (s[0]), \
                          s[0]?s[1]?"\n  ":"\n":"", \
                          NULL_TO_EMPTY (s[1]), \
                          s[1]?"\n":"", \
                          NULL_TO_EMPTY (s[2]))
#else
void
usage (const char **usage)
{
  if (usage[0])
    printf ("%s\n" , usage[0]);
  if (usage[1])
    printf ("  %s\n", usage[1]);

  printf (NULL_TO_EMPTY (usage[2]));
}
#define UCON64_USAGE usage
#endif

void
ucon64_usage (int argc, char *argv[])
{
  int c = 0;
  int option_index = 0;
  int single = 0;

  printf (
    "Usage: %s [OPTION]... [" OPTION_LONG_S "rom=]ROM [[" OPTION_LONG_S "file=]FILE]\n\n"
    "  " OPTION_LONG_S "nbak        prevents backup files (*.BAK)\n"
    "  " OPTION_LONG_S "hdn=N       force ROM has backup unit/emulator header with N Bytes size\n"
    "  " OPTION_LONG_S "hd          same as " OPTION_LONG_S "hdn=512\n"
    "                  most backup units use a header with 512 Bytes size\n"
    "  " OPTION_LONG_S "nhd         force ROM has no backup unit/emulator header\n"
    "  " OPTION_LONG_S "int         force ROM is interleaved (2143)\n"
    "  " OPTION_LONG_S "nint        force ROM is not interleaved (1234)\n"
    "  " OPTION_LONG_S "dint        convert ROM to (non-)interleaved format (1234 <-> 2143)\n"
    "                  this differs from the Super Nintendo " OPTION_LONG_S "dint option\n"
    "  " OPTION_LONG_S "ns          force ROM is not split\n"
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
    "  " OPTION_LONG_S "rrom        rename all ROMs in DIRECTORY to their internal names; " OPTION_LONG_S "rom=DIR\n"
    "                  this is often used by people who lose control of their ROMs\n"
    "  " OPTION_LONG_S "rr83        like " OPTION_LONG_S "rrom but with 8.3 filenames; " OPTION_LONG_S "rom=DIRECTORY\n"
#if 0
    "  " OPTION_LONG_S "rl          rename all files in DIRECTORY to lowercase; " OPTION_LONG_S "rom=DIRECTORY\n"
    "  " OPTION_LONG_S "ru          rename all files in DIRECTORY to uppercase; " OPTION_LONG_S "rom=DIRECTORY\n"
#endif
#ifdef	__MSDOS__
    "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex " OPTION_LONG_S "rom=ROM|more\"\n"
#else
    "  " OPTION_LONG_S "hex         show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex " OPTION_LONG_S "rom=ROM|less\"\n"       // less is more ;-)
#endif
    "  " OPTION_LONG_S "find        find string in ROM; " OPTION_LONG_S "file=STRING (wildcard: '?')\n"
    "  " OPTION_S "c           compare ROMs for differencies; " OPTION_LONG_S "file=OTHER_ROM\n"
    "  " OPTION_LONG_S "cs          compare ROMs for similarities; " OPTION_LONG_S "file=OTHER_ROM\n"
    "  " OPTION_LONG_S "stpn=N      strip N Bytes from ROM beginning\n"
    "  " OPTION_LONG_S "stp         same as " OPTION_LONG_S "stpn=512\n"
    "                  most backup units use a header with 512 Bytes size\n"
    "  " OPTION_LONG_S "insn=N      insert N Bytes (0x00) before ROM\n"
    "  " OPTION_LONG_S "ins         same as " OPTION_LONG_S "insn=512\n"
    "                  most backup units use a header with 512 Bytes size\n"
    "  " OPTION_LONG_S "ispad       check if ROM is padded\n"
    "  " OPTION_S "p, " OPTION_LONG_S "pad    pad ROM to full Mb\n"
    "  " OPTION_LONG_S "strip       strip Bytes from end of ROM; " OPTION_LONG_S "file=VALUE\n"
    , argv[0], ucon64.configfile
  );

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
    "                  %s,\n"
    "                  %s,\n"
    "                  and %s\n",
    gameboy_usage[0], sms_usage[0], genesis_usage[0], nes_usage[0], snes_usage[0]);

  printf (
    "  " OPTION_LONG_S "cd          force recognition (of CD IMAGES)\n"
    "                  this is the support for the most CD-based consoles\n"
    );

//  UCON64_USAGE (cdrw_usage);
  printf ("%s", cdrw_usage[2]);


  printf (
    "  " OPTION_LONG_S "mktoc       generate TOC file for Cdrdao; " OPTION_LONG_S "rom=CD_IMAGE " OPTION_LONG_S "file=TRACK_MODE\n"
    "  " OPTION_LONG_S "mkcue       generate CUE file; " OPTION_LONG_S "rom=CD_IMAGE " OPTION_LONG_S "file=TRACK_MODE\n"
//    "                TRACK_MODE='CD_DA'     (2352 Bytes; AUDIO)\n"
    "                  TRACK_MODE='MODE2_RAW' (2352 Bytes; default)\n"
    "                  TRACK_MODE='MODE1'     (2048 Bytes; standard ISO9660)\n"
    "                  TRACK_MODE='MODE1_RAW' (2352 Bytes)\n"
    "                  TRACK_MODE='MODE2'     (2336 Bytes)\n"
//    "                TRACK_MODE='MODE2_FORM1'    (2048 Bytes)\n"
//    "                TRACK_MODE='MODE2_FORM2'    (2324 Bytes)\n"
//    "                TRACK_MODE='MODE2_FORM_MIX' (2336 Bytes)\n"
    "                  " OPTION_LONG_S "file=TRACK_MODE is optional, uCON64 will always try to\n"
    "                  detect the correct TRACK_MODE from the CD_IMAGE itself\n"
#ifdef TODO
#warning TODO  --toc    convert CloneCD *.cue to cdrdao *.toc
#warning TODO  --cue    convert cdrdao *.toc to *.cue
#endif // TODO
//    "TODO:  " OPTION_LONG_S "toc    convert CloneCD *.cue to cdrdao *.toc\n"
//    "TODO:  " OPTION_LONG_S "cue    convert cdrdao *.toc to *.cue\n"
    "  " OPTION_LONG_S "iso         convert BIN/RAW CD_IMAGE to MODE1 (2048 Bytes); " OPTION_LONG_S "rom=CD_IMAGE\n"
    "                  this might be useful if you made a MODE2_RAW image of a\n"
    "                  MODE1 CD and want to "
#ifdef __unix__
    "mount or "
#endif
    "burn it as MODE1 (2048 Bytes)\n"
    "                  this does only work for MODE1_RAW and MODE2(_RAW) CD_IMAGEs\n"
    "  " OPTION_LONG_S "help        display this help and exit\n"
    "  " OPTION_LONG_S "version     output version information and exit\n"
//    "  " OPTION_LONG_S "quiet       don't show output\n"
#ifdef DEBUG
    "  " OPTION_LONG_S "test        run selftest (DEBUG)\n"
    "                  if you can read this uCON64 was compiled with active DEBUG\n"
    "                  this should only happen for development versions\n"
#endif // DEBUG
    "\n"
  );

  optind = option_index = 0;
#ifdef TODO
#warning TODO is there a better way to reset?
#endif // TODO

  single = 0;

  while (!single && (c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
//      if (single) break;

      switch (c)
        {
      case UCON64_GBA:
        UCON64_USAGE (gba_usage);
#ifdef BACKUP
        UCON64_USAGE (fal_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_N64:
        UCON64_USAGE (n64_usage);
#ifdef BACKUP
        UCON64_USAGE (doctor64_usage);
        UCON64_USAGE (doctor64jr_usage);
//        UCON64_USAGE (cd64_usage);
//        UCON64_USAGE (dex_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_JAG:
        UCON64_USAGE (jaguar_usage);
        single = 1;
        break;

      case UCON64_SNES:
        UCON64_USAGE (snes_usage);
#ifdef BACKUP
        UCON64_USAGE (swc_usage);
//        UCON64_USAGE (fig_usage);
//        UCON64_USAGE (smc_usage);
//        UCON64_USAGE (mgd_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_NG:
        UCON64_USAGE (neogeo_usage);
        single = 1;
        break;

      case UCON64_NGP:
        UCON64_USAGE (ngp_usage);
#ifdef BACKUP
//        UCON64_USAGE (fpl_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_GEN:
        UCON64_USAGE (genesis_usage);
#ifdef BACKUP
        UCON64_USAGE (smd_usage);
//        UCON64_USAGE (mgd_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_GB:
        UCON64_USAGE (gameboy_usage);
#ifdef BACKUP
        UCON64_USAGE (gbx_usage);
        UCON64_USAGE (mccl_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_LYNX:
        UCON64_USAGE (lynx_usage);
        single = 1;
        break;

      case UCON64_PCE:
        UCON64_USAGE (pcengine_usage);
#ifdef BACKUP
//        UCON64_USAGE (mgd_usage);
#endif // BACKUP
        single = 1;
        break;

      case UCON64_SMS:
        UCON64_USAGE (sms_usage);
#ifdef BACKUP
        UCON64_USAGE (smd_usage);
#endif // BACKUP
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
        UCON64_USAGE (sample_usage);
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
      UCON64_USAGE (dc_usage);
      printf("\n");

      UCON64_USAGE (gba_usage);
#ifdef BACKUP
      UCON64_USAGE (fal_usage);
#endif // BACKUP
      printf ("\n");

      UCON64_USAGE (n64_usage);
#ifdef BACKUP
      UCON64_USAGE (doctor64_usage);
      UCON64_USAGE (doctor64jr_usage);
//      UCON64_USAGE (cd64_usage);
//      UCON64_USAGE (dex_usage);
#endif // BACKUP
      printf ("\n");

      UCON64_USAGE (snes_usage);
#ifdef BACKUP
      UCON64_USAGE (swc_usage);
//      UCON64_USAGE (fig_usage);
//      UCON64_USAGE (smc_usage);
//      UCON64_USAGE (mgd_usage);
#endif // BACKUP
      printf ("\n");

      UCON64_USAGE (neogeo_usage);
      printf ("\n");

      UCON64_USAGE (genesis_usage);
#ifdef BACKUP
      UCON64_USAGE (smd_usage);
//      UCON64_USAGE (mgd_usage);
#endif // BACKUP
      printf ("\n");

      UCON64_USAGE (gameboy_usage);
#ifdef BACKUP
      UCON64_USAGE (gbx_usage);
      UCON64_USAGE (mccl_usage);
#endif // BACKUP
      printf ("\n");

      UCON64_USAGE (lynx_usage);
      printf ("\n");

      UCON64_USAGE (pcengine_usage);
#ifdef BACKUP
//      UCON64_USAGE (mgd_usage);
#endif // BACKUP
      printf ("\n");

      UCON64_USAGE (sms_usage);
#ifdef BACKUP
      UCON64_USAGE (smd_usage);
#endif // BACKUP
      printf ("\n");

      UCON64_USAGE (nes_usage);
      printf ("\n");

      UCON64_USAGE (swan_usage);
      printf ("\n");

      UCON64_USAGE (jaguar_usage);
      printf ("\n");

      UCON64_USAGE (ngp_usage);
#ifdef BACKUP
//      UCON64_USAGE (fpl_usage);
#endif // BACKUP
      printf ("\n");

#ifdef SAMPLE
      UCON64_USAGE (sample_usage);
      printf ("\n");
#endif // SAMPLE
  }

  printf (
#ifdef DB
     "Database: %ld known ROMs in db.h (%+ld)\n"
#endif // DB
     "\n"
     "TIP: %s " OPTION_LONG_S "help " OPTION_LONG_S "snes (would show only Super Nintendo related help)\n"
#ifdef	__MSDOS__
     "     %s " OPTION_LONG_S "help|more (to see everything in more)\n"
#else
     "     %s " OPTION_LONG_S "help|less (to see everything in less)\n" // less is more ;-)
#endif
     "     give the force recognition option a try if something went wrong\n"
     "\n"
     "Report problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net\n"
     "\n"
#ifdef DB
     , ucon64_dbsize (UCON64_UNKNOWN)
     , ucon64_dbsize (UCON64_UNKNOWN) - UCON64_DBSIZE
#endif // DB
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
