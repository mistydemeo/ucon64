/*
ucon64_misc.c - miscellaneous functions for uCON64

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2003 dbjh
                  2001 Caz
                  2002 Jan-Erik Karlsson (Amiga)


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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <sys/stat.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>                             // ioperm() (libc5)
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef  PARALLEL
#ifdef  __FreeBSD__
#include <machine/sysarch.h>
#elif   defined __linux__ && defined __GLIBC__
#include <sys/io.h>                             // ioperm() (glibc)
#elif   defined __BEOS__ || defined AMIGA
#include <fcntl.h>
#elif   defined _WIN32
#include <conio.h>                              // inp{w}() & outp{w}()
#include "dlopen.h"
#elif   defined __CYGWIN__
#include <windows.h>                            // definition of WINAPI
#undef  _WIN32
#include "dlopen.h"
#endif
#endif // PARALLEL

#include "misc.h"
#include "ucon64.h"
#ifdef  DISCMAGE
#include "ucon64_dm.h"
#endif
#include "quick_io.h"
#include "ucon64_misc.h"
#include "console/console.h"
#include "backup/backup.h"
#include "patch/patch.h"


/*
  This is a string pool. gcc 2.9x generates something like this itself, but it
  seems gcc 3.x does not. By using a string pool the executable will be
  smaller than without it.
  It's also handy in order to be consistent with messages.
*/
const char *ucon64_msg[] = {
  "ERROR: Please check cables and connection\n"
  "       Turn the backup unit off and on\n"
  "       Split ROMs must be joined first\n"
  "       Use " OPTION_LONG_S "port={3bc, 378, 278, ...} to specify your port\n"
  "       Set the port to SPP (Standard, Normal) mode in your BIOS\n"
  "       Some backup units do not support EPP and ECP style parports\n"
  "       Read the backup unit's manual\n",

  "ERROR: Could not auto detect the right ROM/IMAGE/console type\n"
  "TIP:   If this is a ROM or CD IMAGE you might try to force the recognition\n"
  "       The force recognition option for SNES would be " OPTION_LONG_S "snes\n",

  "Wrote output to: %s\n",
  "ERROR: Can't open \"%s\" for reading\n",
  "ERROR: Can't open \"%s\" for writing\n",
  "ERROR: Can't read from \"%s\"\n",
  "ERROR: Can't write to \"%s\"\n",
  "ERROR: Not enough memory for buffer (%d bytes)\n",
  "ERROR: Not enough memory for ROM buffer (%d bytes)\n",
  "ERROR: Not enough memory for file buffer (%d bytes)\n",
  "DAT info: No ROM with 0x%08x as checksum found\n",
  "WARNING: Support for DAT files is disabled, because \"ucon64_datdir\" (either\n"
  "         in the configuration file or the environment) points to an incorrect\n"
  "         directory. Read the FAQ for more information.\n",
  "Reading config file %s\n",
  "NOTE: %s not found or too old, support for discmage disabled\n",
  NULL
};

const st_usage_t unknown_usage[] =
  {
    {NULL, NULL, "Unknown backup unit/emulator"},
    {NULL, NULL, NULL}
  };

const st_usage_t gc_usage[] =
  {
    {NULL, NULL, "Nintendo Game Cube/Panasonic Gamecube Q\n"},
    {NULL, NULL, "2001/2002 Nintendo http://www.nintendo.com\n"},
    {"gc", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t s16_usage[] =
  {
    {NULL, NULL, "Sega System 16(A/B)/Sega System 18/dual 68000\n"},
    {NULL, NULL, "1987/19XX/19XX SEGA http://www.sega.com\n"},
    {"s16", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t atari_usage[] =
  {
    {NULL, NULL, "Atari VCS 2600(aka Stella)/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr\n"},
    {NULL, NULL, "1977/1982/1984/1986 Atari\n"},
    {"ata", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t coleco_usage[] =
  {
    {NULL, NULL, "ColecoVision\n"},
    {NULL, NULL, "1982\n"},
    {"coleco", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t vboy_usage[] =
  {
    {NULL, NULL, "Nintendo Virtual Boy\n"},
    {NULL, NULL, "19XX Nintendo http://www.nintendo.com\n"},
    {"vboy", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t vectrex_usage[] =
  {
    {NULL, NULL, "Vectrex\n"},
    {NULL, NULL, "1982\n"},
    {"vec", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t intelli_usage[] =
  {
    {NULL, NULL, "Intellivision\n"},
    {NULL, NULL, "1979 Mattel\n"},
    {"intelli", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t gp32_usage[] =
  {
    {NULL, NULL, "GP32 Game System\n"},
    {NULL, NULL, "2002 Gamepark http://www.gamepark.co.kr\n"},
    {"gp32", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t ps2_usage[] =
  {
    {NULL, NULL, "Playstation 2\n"},
    {NULL, NULL, "2000 Sony http://www.playstation.com\n"},
    {"ps2", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t xbox_usage[] =
  {
    {NULL, NULL, "XBox\n"},
    {NULL, NULL, "2001 Microsoft http://www.xbox.com\n"},
    {"xbox", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t sat_usage[] =
  {
    {NULL, NULL, "Saturn\n"},
    {NULL, NULL, "1994 SEGA http://www.sega.com\n"},
    {"sat", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t real3do_usage[] =
  {
    {NULL, NULL, "Real3DO\n"},
    {NULL, NULL, "1993 Panasonic/Goldstar/Philips\n"},
    {"3do", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t cd32_usage[] =
  {
    {NULL, NULL, "CD32\n"},
    {NULL, NULL, "1993 Commodore\n"},
    {"cd32", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t cdi_usage[] =
  {
    {NULL, NULL, "CD-i"},
    {NULL, NULL, "1991 Philips"},
    {"cdi", NULL, "force recognition\n"},
    {NULL, NULL, NULL}
  };

const st_usage_t vc4000_usage[] =
  {
    {NULL, NULL, "Interton VC4000"},
    {NULL, NULL, "~1980"},
    {NULL, NULL, NULL}
  };

const st_usage_t odyssey2_usage[] =
  {
    {NULL, NULL, "G7400+/Odyssey2"},
    {NULL, NULL, "1978"},
    {NULL, NULL, NULL}
  };

const st_usage_t channelf_usage[] =
  {
    {NULL, NULL, "FC Channel F"},
    {NULL, NULL, "1976"},
    {NULL, NULL, NULL}
  };

const st_usage_t odyssey_usage[] =
  {
    {NULL, NULL, "Magnavox Odyssey"},
    {NULL, NULL, "1972 Ralph Baer (USA)"},
    {NULL, NULL, NULL}
  };

const st_usage_t gamecom_usage[] =
  {
    {NULL, NULL, "Game.com"},
    {NULL, NULL, "? Tiger"},
    {NULL, NULL, NULL}
  };

const st_usage_t mame_usage[] =
  {
    {NULL, NULL, "M.A.M.E. (Multiple Arcade Machine Emulator)"},
    {NULL, NULL, NULL}
  };

#if 0
Adv. Vision
Arcadia
Astrocade
Atari 2600
Atari 5200
Atari 7800
Atari Pong
CD-i (1991) 1991
Channel F (1976)
Coleco Vision
Colecovision (1982)
Dreamcast
FC Channel F
G7400+/Odyssey² (1978)
GameBoy
Game.com ? Tiger
Game Cube
Game Gear
Indrema
Intellivision (1979)
Interton VC4000 (~1980)
Microvision
N-Gage (Handheld) 2003 Nokia http://www.n-gage.com
Nuon
Odyssey (Ralph Baer/USA/1972)
Philips CDI
PONG
RCA Studio 2
RDI Halcyon
Real 3DO 1993 Panasonic/Goldstar/Philips?
Sega 32X
Sega CD
Telstar
Turbo Grafx 16
Vectrex (1982)
Virtual Boy
X-Box
XE System

emuchina.net
gameaxe.com
gametz.com
logiqx.com
romcenter.com
sys2064.com
#endif


int
unknown_init (st_rominfo_t *rominfo)
// init routine for all consoles missing in console/.
{
  ucon64.rominfo = rominfo;
  ucon64.dat = NULL;
#ifdef  DISCMAGE
  ucon64.image = NULL;
#endif

  return 0;
}


const st_usage_t ucon64_options_usage[] = {
  {NULL, NULL, "Options"},
  {"o", "DIRECTORY", "specify output directory"},
  {"nbak", NULL, "prevents backup files (*.BAK)"},
#ifdef  ANSI_COLOR
  {"ncol", NULL, "disable ANSI colors in output"},
#endif
#ifdef  PARALLEL
  {"port", "PORT", "specify parallel PORT={3bc, 378, 278, ...}"},
#endif
  {"hdn", "N", "force ROM has backup unit/emulator header with N Bytes size"},
  {"hd", NULL, "same as " OPTION_LONG_S "hdn=512\n"
                   "most backup units use a header with 512 Bytes size"},
  {"nhd", NULL, "force ROM has no backup unit/emulator header"},
  {"int", NULL, "force ROM is interleaved (2143)"},
  {"nint", NULL, "force ROM is not interleaved (1234)"},
  {"dint", NULL, "convert ROM to (non-)interleaved format (1234 <-> 2143)\n"
             "this differs from the SNES & NES " OPTION_LONG_S "dint option"},
  {"ns", NULL, "force ROM is not split"},
#ifdef  __MSDOS__
  {"e", NULL, "emulate/run ROM (check ucon64.cfg for more)"},
#else
  {"e", NULL, "emulate/run ROM (check .ucon64rc for more)"},
#endif
  {"crc", NULL, "show CRC32 value of ROM"  //; this will also force calculation for\n"
             /* "files bigger than %d Bytes (%.4f Mb)" */},
  {"ls", NULL, "generate ROM list for all recognized ROMs"},
  {"lsv", NULL, "like " OPTION_LONG_S "ls but more verbose"},
#ifdef  __MSDOS__
  {"hex", NULL, "show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex ...|more\""},
#else
  {"hex", NULL, "show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex ...|less\""}, // less is more ;-)
#endif
  {"find", "STRING", "find STRING in ROM (wildcard: '?')"},
  {"c", "FILE", "compare FILE with ROM for differences"},
  {"cs" ,"FILE", "compare FILE with ROM for similarities"},
  {"help", NULL, "display this help and exit"},
  {"version", NULL, "output version information and exit"},
  {"q", NULL, "be quiet (don't show ROM info)"},
//  {"qq", NULL, "be even more quiet"},
  {"v", NULL, "be more verbose (show backup unit headers also)"},
  {NULL, NULL, NULL}
};


const st_usage_t ucon64_padding_usage[] = {
  {NULL, NULL, "Padding"},
  {"ispad", NULL, "check if ROM is padded"},
  {"pad", NULL, "pad ROM to next Mb"},
  {"p", NULL, "same as " OPTION_LONG_S "pad"},
  {"padn", "N", "pad ROM to N Bytes (put Bytes with value 0x00 after end)"},
  {"strip", "N", "strip N Bytes from end of ROM"},
  {"stpn", "N", "strip N Bytes from ROM beginning"},
  {"stp", NULL, "same as " OPTION_LONG_S "stpn=512\n"
            "most backup units use a header with 512 Bytes size"},
  {"insn", "N", "insert N Bytes (0x00) before ROM"},
  {"ins", NULL, "same as " OPTION_LONG_S "insn=512\n"
             "most backup units use a header with 512 Bytes size"},
  {NULL, NULL, NULL}
};


const st_usage_t ucon64_patching_usage[] = {
  {NULL, NULL, "Patching"},
  {"poke", "OFF:V", "change byte at file offset OFF to value V (both in hexadecimal)"},
  {"patch", "PATCH", "specify the PATCH for the following options\n"
                    "use this option or uCON64 expects the last commandline\n"
                    "argument to be the name of the PATCH file"},
  {NULL, NULL, NULL}
};


const st_ucon64_wf_t *
ucon64_get_wf (const int option)
{
  int x = 0;

  for (x = 0; ucon64_wf[x].option != 0; x++)
    if (ucon64_wf[x].option == option)
      return (st_ucon64_wf_t *) &ucon64_wf[x];

  return NULL;
}


// -xgd3 handles split files. Only SNES and Genesis files can be detected as
//  being split. Split NES files (Pasofami format) are detected by nes_init().
const st_ucon64_wf_t ucon64_wf[] = {
//  {option, console, usage, flags},
/*
  these options "know" the console
*/
  {UCON64_1991, UCON64_GEN, genesis_usage,     WF_DEFAULT},
  {UCON64_B0, UCON64_LYNX, lynx_usage,         WF_DEFAULT},
  {UCON64_B1, UCON64_LYNX, lynx_usage,         WF_DEFAULT},
  {UCON64_BIOS, UCON64_NG, neogeo_usage,       WF_DEFAULT},
  {UCON64_BOT, UCON64_N64, n64_usage,          WF_DEFAULT},
  {UCON64_CRP, UCON64_GBA, gba_usage,          WF_DEFAULT},
  {UCON64_COL, UCON64_SNES, snes_usage,        WF_NO_ROM},
  {UCON64_DBUH, UCON64_SNES, snes_usage,       WF_DEFAULT},
  {UCON64_DMIRR, UCON64_SNES, snes_usage,      WF_DEFAULT},
  {UCON64_F, UCON64_SNES, snes_usage,          WF_DEFAULT},
  {UCON64_FDS, UCON64_NES, nes_usage,          WF_DEFAULT},
  {UCON64_FDSL, UCON64_NES, nes_usage,         WF_DEFAULT},
  {UCON64_FFE, UCON64_NES, nes_usage,          WF_DEFAULT},
  {UCON64_FIG, UCON64_SNES, snes_usage,        WF_DEFAULT|WF_NO_SPLIT},
  {UCON64_FIGS, UCON64_SNES, snes_usage,       0},
  {UCON64_GBX, UCON64_GB, gameboy_usage,       WF_DEFAULT},
  {UCON64_GD3, UCON64_SNES, snes_usage,        WF_DEFAULT|WF_NO_SPLIT},
  {UCON64_INES, UCON64_NES, nes_usage,         WF_DEFAULT},
  {UCON64_INESHD, UCON64_NES, nes_usage,       WF_DEFAULT},
//  {UCON64_IP, UCON64_DC, dc_usage,             WF_DEFAULT},
  {UCON64_K, UCON64_SNES, snes_usage,          WF_DEFAULT},
  {UCON64_L, UCON64_SNES, snes_usage,          WF_DEFAULT},
  {UCON64_LNX, UCON64_LYNX, lynx_usage,        WF_DEFAULT},
  {UCON64_LOGO, UCON64_GBA, gba_usage,         WF_DEFAULT},
  {UCON64_LSRAM, UCON64_N64, n64_usage,        WF_INIT|WF_PROBE},
  {UCON64_LYX, UCON64_LYNX, lynx_usage,        WF_DEFAULT},
  {UCON64_MULTI, UCON64_GBA, gba_usage,        WF_STOP},
//  {UCON64_MVS, UCON64_NG, neogeo_usage,        WF_DEFAULT},
  {UCON64_N2, UCON64_GEN, genesis_usage,       WF_DEFAULT},
  {UCON64_N2GB, UCON64_GB, gameboy_usage,      WF_DEFAULT},
  {UCON64_NROT, UCON64_LYNX, lynx_usage,       WF_DEFAULT},
  {UCON64_PASOFAMI, UCON64_NES, nes_usage,     WF_DEFAULT},
  {UCON64_ROTL, UCON64_LYNX, lynx_usage,       WF_DEFAULT},
  {UCON64_ROTR, UCON64_LYNX, lynx_usage,       WF_DEFAULT},
  {UCON64_SAM, UCON64_NG, neogeo_usage,        WF_DEFAULT},
  {UCON64_SGB, UCON64_GB, gameboy_usage,       WF_DEFAULT},
  {UCON64_SMC, UCON64_SNES, snes_usage,        WF_DEFAULT|WF_NO_SPLIT},
  {UCON64_SMG, UCON64_PCE, pcengine_usage,     WF_DEFAULT},
  {UCON64_SRAM, UCON64_GBA, gba_usage,         0},
  {UCON64_SSC, UCON64_GB, gameboy_usage,       WF_DEFAULT},
  {UCON64_SWC, UCON64_SNES, snes_usage,        WF_DEFAULT|WF_NO_SPLIT},
  {UCON64_SWCS, UCON64_SNES, snes_usage,       0},
  {UCON64_UFO, UCON64_SNES, snes_usage,        WF_DEFAULT|WF_NO_SPLIT},
  {UCON64_UFOS, UCON64_SNES, snes_usage,       0},
  {UCON64_UNIF, UCON64_NES, nes_usage,         WF_DEFAULT},
  {UCON64_USMS, UCON64_N64, n64_usage,         WF_DEFAULT},
  {UCON64_V64, UCON64_N64, n64_usage,          WF_DEFAULT},
#ifdef  PARALLEL
  // We have to add |WF_NO_ROM to the copier options workflow parameter in
  //  order to support dumping of cartridges or copier SRAM.
  {UCON64_XDEX, UCON64_N64, dex_usage,         WF_DEFAULT|WF_STOP|WF_NO_ROM},
  {UCON64_XDJR, UCON64_N64, doctor64jr_usage,  WF_DEFAULT|WF_STOP|WF_NO_ROM},
  {UCON64_XFAL, UCON64_GBA, fal_usage,         WF_DEFAULT|WF_STOP|WF_NO_ROM},
  {UCON64_XFALB, UCON64_GBA, fal_usage,        WF_STOP|WF_NO_ROM},
  {UCON64_XFALC, UCON64_GBA, fal_usage,        WF_STOP|WF_NO_ROM},
  {UCON64_XFALMULTI, UCON64_GBA, fal_usage,    WF_DEFAULT|WF_STOP}, // send only
  {UCON64_XFALS, UCON64_GBA, fal_usage,        WF_STOP|WF_NO_ROM},
  {UCON64_XFIG, UCON64_SNES, fig_usage,        WF_DEFAULT|WF_STOP|WF_NO_SPLIT|WF_NO_ROM},
  {UCON64_XFIGS, UCON64_SNES, fig_usage,       WF_STOP|WF_NO_ROM},
  {UCON64_XGBX, UCON64_GB, gbx_usage,          WF_DEFAULT|WF_STOP|WF_NO_ROM},
  {UCON64_XGBXB, UCON64_GB, gbx_usage,         WF_STOP|WF_NO_ROM},
  {UCON64_XGBXS, UCON64_GB, gbx_usage,         WF_STOP|WF_NO_ROM},
  {UCON64_XGD3, UCON64_SNES, gd_usage,         WF_DEFAULT|WF_STOP|WF_NO_ROM}, // supports split files
  {UCON64_XLIT, UCON64_GB, lynxit_usage,       WF_STOP|WF_NO_ROM},
  {UCON64_XMCCL, UCON64_LYNX, mccl_usage,      WF_DEFAULT|WF_STOP|WF_NO_ROM},
#if 0
  {UCON64_XSMD, UCON64_UNKNOWN, smd_usage,     WF_DEFAULT|WF_STOP|WF_NO_SPLIT|WF_NO_ROM},
  {UCON64_XSMDS, UCON64_UNKNOWN, smd_usage,    WF_STOP|WF_NO_ROM},
#else
  {UCON64_XSMD, UCON64_GEN, smd_usage,         WF_DEFAULT|WF_STOP|WF_NO_SPLIT|WF_NO_ROM},
  {UCON64_XSMDS, UCON64_GEN, smd_usage,        WF_STOP|WF_NO_ROM},
#endif
  {UCON64_XSWC, UCON64_SNES, swc_usage,        WF_DEFAULT|WF_STOP|WF_NO_SPLIT|WF_NO_ROM},
  {UCON64_XSWC2, UCON64_SNES, swc_usage,       WF_DEFAULT|WF_STOP|WF_NO_SPLIT|WF_NO_ROM},
  {UCON64_XSWCS, UCON64_SNES, swc_usage,       WF_STOP|WF_NO_ROM},
  {UCON64_XV64, UCON64_N64, doctor64_usage,    WF_DEFAULT|WF_STOP|WF_NO_ROM},
#endif
  {UCON64_Z64, UCON64_N64, n64_usage,          WF_DEFAULT},
/*
  these options do not (need to) know the console or work for more than one
*/
#ifdef  DISCMAGE
  {UCON64_BIN2ISO, UCON64_UNKNOWN, libdm_usage, WF_DEFAULT},
  {UCON64_MKSHEET, UCON64_UNKNOWN, libdm_usage, WF_DEFAULT},
  {UCON64_MKTOC, UCON64_UNKNOWN, libdm_usage,  WF_DEFAULT},
  {UCON64_ISOFIX, UCON64_UNKNOWN, libdm_usage, WF_DEFAULT},
  {UCON64_MKCUE, UCON64_UNKNOWN, libdm_usage,  WF_DEFAULT},
  {UCON64_RIP, UCON64_UNKNOWN, libdm_usage,    WF_DEFAULT},
  {UCON64_XCDRW, UCON64_UNKNOWN, NULL,  WF_DEFAULT|WF_STOP|WF_NO_ROM},
  {UCON64_CDMAGE, UCON64_UNKNOWN, NULL,  WF_DEFAULT},
#endif

  {UCON64_HELP, UCON64_UNKNOWN, NULL,          WF_STOP},
  {UCON64_A, UCON64_UNKNOWN, aps_usage,        WF_STOP},
  {UCON64_B, UCON64_UNKNOWN, bsl_usage,        WF_STOP},
  {UCON64_C, UCON64_UNKNOWN, ucon64_options_usage, 0},
  {UCON64_CHK, UCON64_UNKNOWN, NULL,           WF_DEFAULT},
  {UCON64_CRC, UCON64_UNKNOWN, ucon64_options_usage, WF_INIT|WF_PROBE},
  {UCON64_CRCHD, UCON64_UNKNOWN, NULL,         WF_INIT|WF_PROBE},
  {UCON64_CS, UCON64_UNKNOWN, ucon64_options_usage, 0},
  {UCON64_DB, UCON64_UNKNOWN, ucon64_dat_usage, WF_NO_ROM},
  {UCON64_DBS, UCON64_UNKNOWN, ucon64_dat_usage, WF_NO_ROM},
  {UCON64_DBV, UCON64_UNKNOWN, ucon64_dat_usage, WF_NO_ROM},
  {UCON64_DINT, UCON64_UNKNOWN, ucon64_options_usage, WF_INIT|WF_PROBE|WF_NO_SPLIT},
  {UCON64_E, UCON64_UNKNOWN, ucon64_options_usage, WF_DEFAULT},
  {UCON64_FIND, UCON64_UNKNOWN, ucon64_options_usage, WF_INIT},
  {UCON64_GG, UCON64_UNKNOWN, gg_usage,        WF_INIT|WF_PROBE},
  {UCON64_GGD, UCON64_UNKNOWN, gg_usage,       WF_INIT|WF_PROBE|WF_NO_ROM},
  {UCON64_GGE, UCON64_UNKNOWN, gg_usage,       WF_INIT|WF_PROBE|WF_NO_ROM},
  {UCON64_HEX, UCON64_UNKNOWN, ucon64_options_usage, 0},
  {UCON64_I, UCON64_UNKNOWN, ips_usage,        WF_STOP},
  {UCON64_IDPPF, UCON64_UNKNOWN, ppf_usage,    0},
  {UCON64_INS, UCON64_UNKNOWN, ucon64_padding_usage, 0},
  {UCON64_INSN, UCON64_UNKNOWN, ucon64_padding_usage, 0},
  {UCON64_ISPAD, UCON64_UNKNOWN, ucon64_padding_usage, WF_INIT|WF_NO_SPLIT},
  {UCON64_J, UCON64_UNKNOWN, NULL,             WF_INIT|WF_PROBE},
  {UCON64_LS, UCON64_UNKNOWN, ucon64_options_usage, WF_INIT|WF_PROBE},
  {UCON64_LSD, UCON64_UNKNOWN, ucon64_dat_usage, WF_INIT|WF_PROBE},
  {UCON64_LSV, UCON64_UNKNOWN, ucon64_options_usage, WF_INIT|WF_PROBE},
  {UCON64_MGD, UCON64_UNKNOWN, NULL,           WF_DEFAULT|WF_NO_SPLIT},
//  {UCON64_MGH, UCON64_UNKNOWN, ucon64_options_usage, WF_DEFAULT},
  {UCON64_MKA, UCON64_UNKNOWN, aps_usage,      WF_STOP},
  {UCON64_MKDAT, UCON64_UNKNOWN, ucon64_dat_usage, WF_INIT|WF_PROBE},
  {UCON64_MKI, UCON64_UNKNOWN, ips_usage,      WF_STOP},
  {UCON64_MKPPF, UCON64_UNKNOWN, ppf_usage,    WF_STOP},
  {UCON64_N, UCON64_UNKNOWN, NULL,             WF_DEFAULT},
  {UCON64_NA, UCON64_UNKNOWN, aps_usage,       0},
  {UCON64_NPPF, UCON64_UNKNOWN, ppf_usage,     0},
  {UCON64_P, UCON64_UNKNOWN, ucon64_padding_usage, WF_DEFAULT},
  {UCON64_PAD, UCON64_UNKNOWN, ucon64_padding_usage, WF_DEFAULT},
  {UCON64_PADHD, UCON64_UNKNOWN, NULL,         WF_DEFAULT},
  {UCON64_PADN, UCON64_UNKNOWN, ucon64_padding_usage, WF_DEFAULT},
  {UCON64_POKE, UCON64_UNKNOWN, ucon64_patching_usage, 0},
  {UCON64_PPF, UCON64_UNKNOWN, ppf_usage,      WF_STOP},
  {UCON64_RENAME, UCON64_UNKNOWN, ucon64_dat_usage, WF_INIT|WF_PROBE|WF_NO_SPLIT},
  {UCON64_RROM, UCON64_UNKNOWN, ucon64_dat_usage, WF_INIT|WF_PROBE|WF_NO_SPLIT},
  {UCON64_RR83, UCON64_UNKNOWN, NULL,          WF_INIT|WF_PROBE|WF_NO_SPLIT},
  {UCON64_RL, UCON64_UNKNOWN, NULL,            0},
  {UCON64_RU, UCON64_UNKNOWN, NULL,            0},
  {UCON64_S, UCON64_UNKNOWN, NULL,             WF_DEFAULT|WF_NO_SPLIT},
  {UCON64_SCAN, UCON64_UNKNOWN, ucon64_dat_usage, WF_INIT|WF_PROBE|WF_NO_SPLIT},
  {UCON64_SMD, UCON64_UNKNOWN, NULL,           WF_DEFAULT|WF_NO_SPLIT}, // Genesis AND Sega Master System
  {UCON64_SMDS, UCON64_UNKNOWN, NULL,          0}, // Genesis AND Sega Master System
  {UCON64_STP, UCON64_UNKNOWN, ucon64_padding_usage, 0},
  {UCON64_STPN, UCON64_UNKNOWN, ucon64_padding_usage, 0},
  {UCON64_STRIP, UCON64_UNKNOWN, ucon64_padding_usage, 0},
  {UCON64_SWAP, UCON64_UNKNOWN, NULL,          WF_INIT|WF_PROBE},
  {UCON64_VER, UCON64_UNKNOWN, ucon64_options_usage, WF_STOP},
/*
  force recognition switches
*/
#ifdef  DISCMAGE
  {UCON64_DISC, UCON64_UNKNOWN, libdm_usage,   WF_SWITCH},
#endif

  {UCON64_3DO, UCON64_3DO, real3do_usage,      WF_SWITCH},
  {UCON64_ATA, UCON64_ATA, atari_usage,        WF_SWITCH},
//  {UCON64_CD32, UCON64_CD32, cd32_usage,       WF_SWITCH},
//  {UCON64_CDI, UCON64_CDI, cdi_usage,          WF_SWITCH},
  {UCON64_COLECO, UCON64_COLECO, coleco_usage, WF_SWITCH},
  {UCON64_DC, UCON64_DC, dc_usage,             WF_SWITCH},
  {UCON64_GB, UCON64_GB, gameboy_usage,        WF_SWITCH},
  {UCON64_GBA, UCON64_GBA, gba_usage,          WF_SWITCH},
  {UCON64_GC, UCON64_GC, gc_usage,             WF_SWITCH},
  {UCON64_GEN, UCON64_GEN, genesis_usage,      WF_SWITCH},
  {UCON64_GP32, UCON64_GP32, gp32_usage,       WF_SWITCH},
  {UCON64_INTELLI, UCON64_INTELLI, intelli_usage, WF_SWITCH},
  {UCON64_JAG, UCON64_JAG, jaguar_usage,       WF_SWITCH},
  {UCON64_LYNX, UCON64_LYNX, lynx_usage,       WF_SWITCH},
  {UCON64_N64, UCON64_N64, n64_usage,          WF_SWITCH},
  {UCON64_NES, UCON64_NES, nes_usage,          WF_SWITCH},
  {UCON64_NG, UCON64_NG, neogeo_usage,         WF_SWITCH},
  {UCON64_NGP, UCON64_NGP, ngp_usage,          WF_SWITCH},
  {UCON64_PCE, UCON64_PCE, pcengine_usage,     WF_SWITCH},
  {UCON64_PS2, UCON64_PS2, ps2_usage,          WF_SWITCH},
  {UCON64_PSX, UCON64_PSX, psx_usage,          WF_SWITCH},
  {UCON64_S16, UCON64_S16, s16_usage,          WF_SWITCH},
  {UCON64_SAT, UCON64_SAT, sat_usage,          WF_SWITCH},
  {UCON64_SMS, UCON64_SMS, sms_usage,          WF_SWITCH},
  {UCON64_SNES, UCON64_SNES, snes_usage,       WF_SWITCH},
  {UCON64_SWAN, UCON64_SWAN, swan_usage,       WF_SWITCH},
  {UCON64_VBOY, UCON64_VBOY, vboy_usage,       WF_SWITCH},
  {UCON64_VEC, UCON64_VEC, vectrex_usage,      WF_SWITCH},
  {UCON64_XBOX, UCON64_XBOX, xbox_usage,       WF_SWITCH},
/*
  other switches
*/
  {UCON64_83, UCON64_UNKNOWN, ucon64_dat_usage, WF_SWITCH},
  {UCON64_BAT, UCON64_NES, nes_usage,          WF_SWITCH},
  {UCON64_BS, UCON64_SNES, snes_usage,         WF_SWITCH},
  {UCON64_CMNT, UCON64_NES, nes_usage,         WF_SWITCH},
  {UCON64_CTRL, UCON64_UNKNOWN, NULL,          WF_SWITCH}, // for NES & SNES
  {UCON64_CTRL2, UCON64_SNES, snes_usage,      WF_SWITCH},
  {UCON64_DUMPINFO, UCON64_NES, nes_usage,     WF_SWITCH},
  {UCON64_EROM, UCON64_SNES, snes_usage,       WF_SWITCH},
  {UCON64_FILE, UCON64_UNKNOWN, NULL,          WF_SWITCH},
  {UCON64_FORCE63, UCON64_UNKNOWN, ucon64_dat_usage, WF_SWITCH},
  {UCON64_FRONTEND, UCON64_UNKNOWN, NULL,      WF_SWITCH},
//  {UCON64_GOOD, UCON64_UNKNOWN, ucon64_dat_usage, WF_SWITCH},
  {UCON64_HD, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_HDN, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_HI, UCON64_SNES, snes_usage,         WF_SWITCH},
  {UCON64_INT, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_INT2, UCON64_SNES, snes_usage, WF_SWITCH},
  {UCON64_MAPR, UCON64_NES, nes_usage,         WF_SWITCH},
  {UCON64_MIRR, UCON64_NES, nes_usage,         WF_SWITCH},
  {UCON64_NBAK, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_NBAT, UCON64_NES, nes_usage,         WF_SWITCH},
  {UCON64_NBS, UCON64_SNES, snes_usage,        WF_SWITCH},
#ifdef  ANSI_COLOR
  {UCON64_NCOL, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
#endif
  {UCON64_NHD, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_NHI, UCON64_SNES, snes_usage,        WF_SWITCH},
  {UCON64_NINT, UCON64_UNKNOWN, ucon64_options_usage,WF_SWITCH},
  {UCON64_NS, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_NTSC, UCON64_NES, nes_usage,         WF_SWITCH},
  {UCON64_NVRAM, UCON64_NES, nes_usage,        WF_SWITCH},
  {UCON64_O, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_PAL, UCON64_NES, nes_usage,          WF_SWITCH},
  {UCON64_PATCH, UCON64_UNKNOWN, ucon64_patching_usage, WF_SWITCH},
#ifdef  PARALLEL
  {UCON64_PORT, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
#endif
  {UCON64_Q, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_QQ, UCON64_UNKNOWN, NULL,            WF_SWITCH},
  {UCON64_ROM, UCON64_UNKNOWN, NULL,           WF_SWITCH},
  {UCON64_SSIZE, UCON64_SNES, snes_usage,      WF_SWITCH},
  {UCON64_V, UCON64_UNKNOWN, ucon64_options_usage, WF_SWITCH},
  {UCON64_VRAM, UCON64_NES, nes_usage,         WF_SWITCH},
#ifdef  PARALLEL
  {UCON64_XFALM, UCON64_GBA, fal_usage,        WF_SWITCH},
#endif
#ifdef  GUI
  {UCON64_GUI, UCON64_UNKNOWN, libng_usage,  WF_DEFAULT},
#endif  
  {0, 0, NULL, 0}
};


char *ucon64_temp_file = NULL;
int (*ucon64_testsplit_callback) (const char *filename) = NULL;


// _publisher_ strings for SNES, GB, GBC and GBA games
const char *nintendo_maker[NINTENDO_MAKER_LEN] = {
  NULL, "Nintendo", "Rocket Games/Ajinomoto", "Imagineer-Zoom", "Gray Matter",
  "Zamuse", "Falcom", NULL, "Capcom", "Hot B Co.",
  "Jaleco", "Coconuts Japan", "Coconuts Japan/G.X.Media",
    "Micronet", "Technos",
  "Mebio Software", "Shouei System", "Starfish", NULL, "Mitsui Fudosan/Dentsu",
  NULL, "Warashi Inc.", NULL, "Nowpro", NULL,
  "Game Village", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 0Z
  NULL, "Starfish", "Infocom", "Electronic Arts Japan", NULL,
  "Cobra Team", "Human/Field", "KOEI", "Hudson Soft", "S.C.P./Game Village",
  "Yanoman", NULL, "Tecmo Products", "Japan Glary Business", "Forum/OpenSystem",
  "Virgin Games (Japan)", "SMDE", NULL, NULL, "Daikokudenki",
  NULL, NULL, NULL, NULL, NULL,
  "Creatures Inc.", "TDK Deep Impresion", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 1Z
  "Destination Software/KSS", "Sunsoft/Tokai Engineering",
    "POW (Planning Office Wada)/VR 1 Japan", "Micro World", NULL,
  "San-X", "Enix", "Loriciel/Electro Brain", "Kemco Japan", "Seta Co., Ltd.",
  "Culture Brain", NULL, "Palsoft", "Visit Co., Ltd.", "Intec",
  "System Sacom", "Poppo", "Ubisoft Japan", NULL, "Media Works",
  "NEC InterChannel", "Tam", "Gajin/Jordan", "Smilesoft", NULL,
  NULL, "Mediakite", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 2Z
  "Viacom", "Carrozzeria", "Dynamic", NULL, "Magifact",
  "Hect", "Codemasters", "Taito/GAGA Communications", "Laguna",
    "Telstar Fun & Games/Event/Taito",
  NULL, "Arcade Zone Ltd.", "Entertainment International/Empire Software", "Loriciel",
    "Gremlin Graphics",
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 3Z
  "Seika Corp.", "UBI SOFT Entertainment Software", "Sunsoft US", NULL, "Life Fitness",
  NULL, "System 3", "Spectrum Holobyte", NULL, "IREM",
  NULL, "Raya Systems", "Renovation Products", "Malibu Games", NULL,
  "Eidos/U.S. Gold", "Playmates Interactive", NULL, NULL, "Fox Interactive",
  "Time Warner Interactive", NULL, NULL, NULL, NULL,
  NULL, "Disney Interactive", NULL, "Black Pearl", NULL,
  "Advanced Productions", NULL, NULL, "GT Interactive", "RARE",
  "Crave Entertainment",                        // 4Z
  "Absolute Entertainment", "Acclaim", "Activision", "American Sammy", "Take 2/GameTek",
  "Hi Tech", "LJN Ltd.", NULL, "Mattel", NULL,
  "Mindscape/Red Orb Entertainment", "Romstar", "Taxan", "Midway/Tradewest", NULL,
  "American Softworks Corp.", "Majesco Sales Inc.", "3DO", NULL, NULL,
  "Hasbro", "NewKidCo", "Telegames", "Metro3D", NULL,
  "Vatical Entertainment", "LEGO Media", NULL, "Xicat Interactive", "Cryo Interactive",
  NULL, NULL, "Red Storm Entertainment", "Microids", NULL,
  "Conspiracy/Swing",                           // 5Z
  "Titus", "Virgin Interactive", "Maxis", NULL, "LucasArts Entertainment",
  NULL, NULL, "Ocean", NULL, "Electronic Arts",
  NULL, "Laser Beam", NULL, NULL, "Elite Systems",
  "Electro Brain", "The Learning Company", "BBC", NULL, "Software 2000",
  NULL, "BAM! Entertainment", "Studio 3", NULL, NULL,
  NULL, "Classified Games", NULL, "TDK Mediactive", NULL,
  "DreamCatcher", "JoWood Produtions", "SEGA", "Wannado Edition",
    "LSP (Light & Shadow Prod.)",
  "ITE Media",                                  // 6Z
  "Infogrames", "Interplay", "JVC (US)", "Parker Brothers", NULL,
  "SCI (Sales Curve Interactive)/Storm", NULL, NULL, "THQ Software", "Accolade Inc.",
  "Triffix Entertainment", NULL, "Microprose Software",
    "Universal Interactive/Sierra/Simon & Schuster", NULL,
  "Kemco", "Rage Software", "Encore", NULL, "Zoo",
  "BVM", "Simon & Schuster Interactive", "Asmik Ace Entertainment Inc./AIA",
    "Empire Interactive", NULL,
  NULL, "Jester Interactive", NULL, NULL, "Scholastic",
  "Ignition Entertainment", NULL, "Stadlbauer", NULL, NULL,
  NULL,                                         // 7Z
  "Misawa", "Teichiku", "Namco Ltd.", "LOZC", "KOEI",
  NULL, "Tokuma Shoten Intermedia", "Tsukuda Original", "DATAM-Polystar", NULL,
  NULL, "Bulletproof Software", "Vic Tokai Inc.", NULL, "Character Soft",
  "I'Max", "Saurus", NULL, NULL, "General Entertainment",
  NULL, NULL, "I'Max", "Success", NULL,
  "SEGA Japan", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 8Z
  "Takara", "Chun Soft", "Video System Co., Ltd./McO'River", "BEC", NULL,
  "Varie", "Yonezawa/S'pal", "Kaneko", NULL, "Victor Interactive Software/Pack in Video",
  "Nichibutsu/Nihon Bussan", "Tecmo", "Imagineer", NULL, NULL,
  "Nova", "Den'Z", "Bottom Up", NULL, "TGL (Technical Group Laboratory)",
  NULL, "Hasbro Japan", NULL, "Marvelous Entertainment", NULL,
  "Keynet Inc.", "Hands-On Entertainment", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 9Z
  "Telenet", "Hori", NULL, NULL, "Konami",
  "K.Amusement Leasing Co.", "Kawada", "Takara", NULL, "Technos Japan Corp.",
  "JVC (Europe/Japan)/Victor Musical Industries", NULL, "Toei Animation", "Toho", NULL,
  "Namco", "Media Rings Corp.", "J-Wing", NULL, "Pioneer LDC",
  "KID", "Mediafactory", NULL, NULL, NULL,
  "Infogrames Hudson", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // AZ
  "Acclaim Japan", "ASCII Co./Nexoft" /*/Activision*/, "Bandai", NULL, "Enix",
  NULL, "HAL Laboratory/Halken", "SNK", NULL, "Pony Canyon Hanbai",
  "Culture Brain", "Sunsoft", "Toshiba EMI", "Sony Imagesoft", NULL,
  "Sammy", "Magical", "Visco", NULL, "Compile",
  NULL, "MTO Inc.", NULL, "Sunrise Interactive", NULL,
  "Global A Entertainment", "Fuuki", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // BZ
  "Taito", NULL, "Kemco", "Square", "Tokuma Shoten",
  "Data East", "Tonkin House", NULL, "KOEI", NULL,
  "Konami/Ultra/Palcom", "NTVIC/VAP", "Use Co., Ltd.", "Meldac",
    "Pony Canyon (Japan)/FCI (US)",
  "Angel/Sotsu Agency/Sunrise", "Yumedia/Aroma Co., Ltd.", NULL, NULL, "Boss",
  "Axela/Crea-Tech", "Sekaibunka-Sha/Sumire kobo/Marigul Management Inc.",
    "Konami Computer Entertainment Osaka", NULL, NULL,
  "Enterbrain", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // CZ
  "Taito/Disco", "Sofel", "Quest Corp.", "Sigma", "Ask Kodansha",
  NULL, "Naxat", "Copya System", "Capcom Co., Ltd.", "Banpresto",
  "TOMY", "Acclaim/LJN Japan", NULL, "NCS", "Human Entertainment",
  "Altron", "Jaleco", "Gaps Inc.", NULL, NULL,
  NULL, NULL, NULL, "Elf", NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // DZ
  "Jaleco", NULL, "Yutaka", "Varie", "T&ESoft",
  "Epoch Co., Ltd.", NULL, "Athena", "Asmik", "Natsume",
  "King Records", "Atlus", "Epic/Sony Records (Japan)", NULL,
    "IGS (Information Global Service)",
  NULL, "Chatnoir", "Right Stuff", NULL, NULL,
  NULL, "Spike", "Konami Computer Entertainment Tokyo", "Alphadream Corp.", NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // EZ
  "A Wave", "Motown Software", "Left Field Entertainment", "Extreme Ent. Grp.",
    "TecMagik",
  NULL, NULL, NULL, NULL, "Cybersoft",
  NULL, "Psygnosis", NULL, NULL, "Davidson/Western Tech.",
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // FZ
  NULL, "PCCW Japan", NULL, NULL, "KiKi Co. Ltd.",
  "Open Sesame Inc.", "Sims", "Broccoli", "Avex", NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // GZ
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // HZ
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, "Yojigen", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL};                                        // IZ


#if     defined PARALLEL && (defined __BEOS__ || defined AMIGA)
typedef struct st_ioport
{
  unsigned int port;
  unsigned char data8;
  unsigned short data16;
} st_ioport_t;

static int ucon64_io_fd;
#endif


int
ucon64_file_handler (char *dest, char *src, int flags)
/*
  We have to handle the following cases (for example -swc and rom.swc exists):
  1) ucon64 -swc rom.swc
    a) with backup creation enabled
       Create backup of rom.swc
       postcondition: src == name of backup
    b) with backup creation disabled
       Create temporary backup of rom.swc by renaming rom.swc
       postcondition: src == name of backup
  2) ucon64 -swc rom.fig
    a) with backup creation enabled
       Create backup of rom.swc
       postcondition: src == rom.fig
    b) with backup creation disabled
       Do nothing
       postcondition: src == rom.fig

  This function returns 1 if dest existed (in the directory specified with -o).
  Otherwise it returns 0;
*/
{
  struct stat dest_info;

  ucon64_output_fname (dest, flags);            // call this function unconditionally

#if 0
  // ucon64_temp_file will be reset in remove_temp_file()
  ucon64_temp_file = NULL;
#endif
  if (!access (dest, F_OK))
    {
      stat (dest, &dest_info);
      // *Trying* to make dest writable here avoids having to change all code
      //  that might (try to) operate on a read-only file
      chmod (dest, dest_info.st_mode | S_IWUSR);

      if (src == NULL)
        {
          if (ucon64.backup)
            printf ("Wrote backup to: %s\n", q_fbackup (dest, BAK_DUPE));
          return 1;
        }

      if (one_file (src, dest))
        {                                       // case 1
          if (ucon64.backup)
            {                                   // case 1a
              strcpy (src, q_fbackup (dest, BAK_DUPE));
              printf ("Wrote backup to: %s\n", src);
            }
          else
            {                                   // case 1b
              strcpy (src, q_fbackup (dest, BAK_MOVE));
              ucon64_temp_file = src;
            }
        }
      else
        {                                       // case 2
          if (ucon64.backup)                    // case 2a
            printf ("Wrote backup to: %s\n", q_fbackup (dest, BAK_DUPE));
        }
      return 1;
    }
  return 0;
}


void
remove_temp_file (void)
{
  if (ucon64_temp_file)
    {
      printf ("Removing: %s\n", ucon64_temp_file);
      remove (ucon64_temp_file);
      ucon64_temp_file = NULL;
    }
}


char *
ucon64_output_fname (char *requested_fname, int flags)
{
  char suffix[80], fname[FILENAME_MAX];

  // We have to make a copy, because get_suffix() returns a pointer to a
  //  location in the original string
  strncpy (suffix, get_suffix (requested_fname), 80);
  suffix[80 - 1] = 0;                           // in case suffix is >= 80 chars

  // OF_FORCE_BASENAME is necessary for options like -gd3. Of course that
  //  code should handle archives and come up with unique filenames for
  //  archives with more than one file.
  if (!ucon64.fname_arch[0] || (flags & OF_FORCE_BASENAME))
    {
      strcpy (fname, basename (requested_fname));
      sprintf (requested_fname, "%s%s", ucon64.output_path, fname);
    }
  else                                          // an archive (for now: zip file)
    sprintf (requested_fname, "%s%s", ucon64.output_path, ucon64.fname_arch);

  /*
    Keep the requested suffix, but only if it isn't ".zip" or ".gz". This
    because we currently don't write to zip or gzip files. Otherwise the output
    file would have the suffix ".zip" or ".gz" while it isn't a zip or gzip
    file. uCON64 handles such files correctly, because it looks at the file
    data itself, but many programs don't.
    If the flag OF_FORCE_SUFFIX was used we keep the suffix, even if it's
    ".zip" or ".gz". Now ucon64_output_fname() can be used when renaming/moving
    files.
  */
  if (!(flags & OF_FORCE_SUFFIX) &&
      !(stricmp (suffix, ".zip") && stricmp (suffix, ".gz")))
    strcpy (suffix, ".tmp");
  set_suffix_i (requested_fname, suffix);       // use set_suffix_i(), because
                                                //  the suffix must keep its case
  return requested_fname;
}


int
ucon64_fhexdump (const char *filename, int start, int len)
{
  int pos, size = q_fsize (filename), value = 0,
      buf_size = MAXBUFSIZE - (MAXBUFSIZE % 16); // buf_size must be < MAXBUFSIZE && 16 * n
  char buf[MAXBUFSIZE];
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    return -1;

  if ((size - start) < len)
    len = size - start;

  fseek (fh, start, SEEK_SET);
  for (pos = 0; pos < len; pos += buf_size)
    {
      value = fread (buf, 1, MIN (len, buf_size), fh);
      mem_hexdump (buf, value, pos + start);
    }

  fclose (fh);

  return 0;
}


#define FILEFILE_LARGE_BUF
// When verifying if the code produces the same output when FILEFILE_LARGE_BUF
//  is defined as when it's not, be sure to use the same buffer size
int
ucon64_filefile (const char *filename1, int start1, const char *filename2,
                 int start2, int similar)
{
  int base, fsize1, fsize2, len, chunksize1, chunksize2, readok = 1,
      bytesread1, bytesread2, bytesleft1, bytesleft2, n_bytes = 0;
#ifdef  FILEFILE_LARGE_BUF
  int bufsize = 1024 * 1024;
  unsigned char *buf1, *buf2;
#else
  int bufsize = MAXBUFSIZE;
  unsigned char buf1[MAXBUFSIZE], buf2[MAXBUFSIZE];
#endif
  FILE *file1, *file2;

  if (one_file (filename1, filename2))
    return -2;

  fsize1 = q_fsize (filename1);                 // q_fsize() returns size in bytes
  fsize2 = q_fsize (filename2);
  if (fsize1 < start1 || fsize2 < start2)
    return -1;

#ifdef  FILEFILE_LARGE_BUF
  if (!(buf1 = (unsigned char *) malloc (bufsize)))
    return -1;

  if (!(buf2 = (unsigned char *) malloc (bufsize)))
    {
      free (buf1);
      return -1;
    }
#endif

  if (!(file1 = fopen (filename1, "rb")))
    {
#ifdef  FILEFILE_LARGE_BUF
      free (buf1);
      free (buf2);
#endif
      return -1;
    }
  if (!(file2 = fopen (filename2, "rb")))
    {
      fclose (file1);
#ifdef  FILEFILE_LARGE_BUF
      free (buf1);
      free (buf2);
#endif
      return -1;
    }

  fseek (file1, start1, SEEK_SET);
  fseek (file2, start2, SEEK_SET);
  bytesleft1 = fsize1;
  bytesread1 = 0;
  bytesleft2 = fsize2;
  bytesread2 = 0;

  while (bytesleft1 > 0 && bytesread1 < fsize2 && readok)
    {
      chunksize1 = fread (buf1, 1, bufsize, file1);
      if (chunksize1 == 0)
        readok = 0;
      else
        {
          bytesread1 += chunksize1;
          bytesleft1 -= chunksize1;
        }

      while (bytesleft2 > 0 && bytesread2 < bytesread1 && readok)
        {
          chunksize2 = fread (buf2, 1, chunksize1, file2);
          if (chunksize2 == 0)
            readok = 0;
          else
            {
              base = 0;
              while (base < chunksize2)
                {
                  if (similar == TRUE ?
                      buf1[base] == buf2[base] :
                      buf1[base] != buf2[base])
                    {
                      for (len = 0; base + len < chunksize2; len++)
                        if (similar == TRUE ?
                            buf1[base + len] != buf2[base + len] :
                            buf1[base + len] == buf2[base + len])
                          break;

                      printf ("%s:\n", filename1);
                      mem_hexdump (&buf1[base], len, start1 + base + bytesread2);
                      printf ("%s:\n", filename2);
                      mem_hexdump (&buf2[base], len, start2 + base + bytesread2);
                      printf ("\n");
                      base += len;
                      n_bytes += len;
                    }
                  else
                    base++;
                }

              bytesread2 += chunksize2;
              bytesleft2 -= chunksize2;
            }
        }
    }

  fclose (file1);
  fclose (file2);
#ifdef  FILEFILE_LARGE_BUF
  free (buf1);
  free (buf2);
#endif
  return n_bytes;
}


#if 1
int
ucon64_testpad (const char *filename)
/*
  Test if EOF is padded (repeated byte values)
  This (new) version is not efficient for uncompressed files, but *much* more
  efficient for compressed files. For example (a bad case), on a Celeron 850
  just viewing info about a zipped dump of Mario Party (U) takes more than 3
  minutes when the old version of ucon64_testpad() is used. A gzipped dump
  can take more than 6 minutes. With this version it takes about 9 seconds for
  the zipped dump and 12 seconds for the gzipped dump.
*/
{
  int c = 0, blocksize, i, n = 0, start_n;
  unsigned char buffer[MAXBUFSIZE];
  FILE *file = fopen (filename, "rb");

  if (!file)
    return -1;

  while ((blocksize = fread (buffer, 1, MAXBUFSIZE, file)))
    {
      if (buffer[blocksize - 1] != c)
        {
          c = buffer[blocksize - 1];
          n = 0;
        }
      start_n = n;
      for (i = blocksize - 1; i >= 0; i--)
        {
          if (buffer[i] != c)
            {
              n -= start_n;
              break;
            }
          else
            {
              /*
                A file is either padded with 2 or more bytes or it isn't
                padded at all. It can't be detected that a file is padded with
                1 byte.
              */
              if (i == blocksize - 2)
                n += 2;
              else if (i < blocksize - 2)
                n++;
              // NOT else, because i == blocksize - 1 must initially be skipped
            }
        }
    }

  fclose (file);
  return n;
}
#else
int
ucon64_testpad (const char *filename)
// test if EOF is padded (repeating bytes)
{
  int pos = ucon64.file_size - 1, buf_pos = pos % MAXBUFSIZE,
      c = q_fgetc (filename, pos);
  unsigned char buf[MAXBUFSIZE];
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    return -1;

  for (pos -= buf_pos; !fseek (fh, pos, SEEK_SET) && pos > -1;
       pos -= MAXBUFSIZE, buf_pos = MAXBUFSIZE)
    {
      fread (buf, 1, buf_pos, fh);

      for (; buf_pos > 0; buf_pos--)
        if (buf[buf_pos - 1] != c)
          {
            fclose (fh);

            return ucon64.file_size - (pos + buf_pos) > 1 ?
              ucon64.file_size - (pos + buf_pos) : 0;
          }
    }

  fclose (fh);

  return ucon64.file_size;                      // the whole file is "padded"
}
#endif


#ifdef  PARALLEL

#ifdef  __i386__                                // GCC && x86
inline static unsigned char
i386_input_byte (unsigned short port)
{
  unsigned char byte;
  __asm__ __volatile__
  ("inb %1, %0"
    : "=a" (byte)
    : "d" (port)
  );
  return byte;
}


inline static unsigned short
i386_input_word (unsigned short port)
{
  unsigned short word;
  __asm__ __volatile__
  ("inw %1, %0"
    : "=a" (word)
    : "d" (port)
  );
  return word;
}


inline static void
i386_output_byte (unsigned short port, unsigned char byte)
{
  __asm__ __volatile__
  ("outb %1, %0"
    :
    : "d" (port), "a" (byte)
  );
}


inline static void
i386_output_word (unsigned short port, unsigned short word)
{
  __asm__ __volatile__
  ("outw %1, %0"
    :
    : "d" (port), "a" (word)
  );
}
#endif // __i386__


#if     defined _WIN32 || defined __CYGWIN__
void *io_driver;

// inpout32.dll only has I/O functions for byte-sized I/O
unsigned char (__stdcall *Inp32) (unsigned short) = NULL;
void (__stdcall *Outp32) (unsigned short, unsigned char) = NULL;

unsigned char inpout32_input_byte (unsigned short port) { return Inp32 (port); }
void inpout32_output_byte (unsigned short port, unsigned char byte) { Outp32 (port, byte); }

// io.dll has more functions then the ones we refer to here, but we don't need them
char (WINAPI *PortIn) (short int) = NULL;
short int (WINAPI *PortWordIn) (short int) = NULL;
void (WINAPI *PortOut) (short int, char) = NULL;
void (WINAPI *PortWordOut) (short int, short int) = NULL;
short int (WINAPI *IsDriverInstalled) () = NULL;

unsigned char io_input_byte (unsigned short port) { return PortIn (port); }
unsigned short io_input_word (unsigned short port) { return PortWordIn (port); }
void io_output_byte (unsigned short port, unsigned char byte) { PortOut (port, byte); }
void io_output_word (unsigned short port, unsigned short word) { PortWordOut (port, word); }

#if     defined __CYGWIN__ || defined __MINGW32__
// default to functions which are always available (but which generate an
//  exception without a "driver" such as UserPort)
unsigned char (*input_byte) (unsigned short) = i386_input_byte;
unsigned short (*input_word) (unsigned short) = i386_input_word;
void (*output_byte) (unsigned short, unsigned char) = i386_output_byte;
void (*output_word) (unsigned short, unsigned short) = i386_output_word;

#elif   defined _WIN32
// The following four functions are needed because inp{w} and outp{w} seem to be macros
unsigned char inp_func (unsigned short port) { return (unsigned char) inp (port); }
unsigned short inpw_func (unsigned short port) { return inpw (port); }
void outp_func (unsigned short port, unsigned char byte) { outp (port, byte); }
void outpw_func (unsigned short port, unsigned short word) { outpw (port, word); }

// default to functions which are always available (but which generate an
//  exception without a "driver" such as UserPort)
unsigned char (*input_byte) (unsigned short) = inp_func;
unsigned short (*input_word) (unsigned short) = inpw_func;
void (*output_byte) (unsigned short, unsigned char) = outp_func;
void (*output_word) (unsigned short, unsigned short) = outpw_func;

#endif
#endif // defined _WIN32 || defined __CYGWIN__


#if     defined __BEOS__ || defined AMIGA
void
close_io_port (void)
{
  close (ucon64_io_fd);
}
#endif


unsigned char
inportb (unsigned short port)
{
#if     defined __BEOS__ || defined AMIGA
  st_ioport_t temp;

  temp.port = port;
  ioctl (ucon64_io_fd, 'r', &temp, 0);

  return temp.data8;
#elif   defined _WIN32 || defined __CYGWIN__
  return input_byte (port);
#elif   defined __i386__
  return i386_input_byte (port);
#endif
}


unsigned short
inportw (unsigned short port)
{
#if     defined __BEOS__ || defined AMIGA
  st_ioport_t temp;

  temp.port = port;
  ioctl (ucon64_io_fd, 'r16', &temp, 0);

  return temp.data16;
#elif   defined _WIN32 || defined __CYGWIN__
  return input_word (port);
#elif   defined __i386__
  return i386_input_word (port);
#endif
}


void
outportb (unsigned short port, unsigned char byte)
{
#if     defined __BEOS__ || defined AMIGA
  st_ioport_t temp;

  temp.port = port;
  temp.data8 = byte;
  ioctl (ucon64_io_fd, 'w', &temp, 0);
#elif   defined _WIN32 || defined __CYGWIN__
  output_byte (port, byte);
#elif   defined __i386__
  i386_output_byte (port, byte);
#endif
}


void
outportw (unsigned short port, unsigned short word)
{
#if     defined __BEOS__ || defined AMIGA
  st_ioport_t temp;

  temp.port = port;
  temp.data16 = word;
  ioctl (ucon64_io_fd, 'w16', &temp, 0);
#elif   defined _WIN32 || defined __CYGWIN__
  output_word (port, word);
#elif   defined __i386__
  i386_output_word (port, word);
#endif
}


#define DETECT_MAX_CNT 1000
static int
ucon64_parport_probe (unsigned int port)
{
  int i = 0;

#ifdef  __FreeBSD__
  if (i386_set_ioperm (port, 1, 1) == -1)
    return -1;
#elif   defined __linux__
  if (ioperm (port, 1, 1) == -1)
    return -1;
#endif

  outportb ((unsigned short) port, 0xaa);
  for (i = 0; i < DETECT_MAX_CNT; i++)
    if (inportb ((unsigned short) port) == 0xaa)
      break;

  if (i < DETECT_MAX_CNT)
    {
      outportb ((unsigned short) port, 0x55);
      for (i = 0; i < DETECT_MAX_CNT; i++)
        if (inportb ((unsigned short) port) == 0x55)
          break;
    }

#ifdef  __FreeBSD__
  if (i386_set_ioperm (port, 1, 0) == -1)
    return -1;
#elif   defined __linux__
  if (ioperm (port, 1, 0) == -1)
    return -1;
#endif

  if (i >= DETECT_MAX_CNT)
    return 0;

  return 1;
}


unsigned int
ucon64_parport_init (unsigned int port)
{
#ifdef  __BEOS__
  ucon64_io_fd = open ("/dev/misc/ioport", O_RDWR | O_NONBLOCK);
  if (ucon64_io_fd == -1)
    {
      ucon64_io_fd = open ("/dev/misc/parnew", O_RDWR | O_NONBLOCK);
      if (ucon64_io_fd == -1)
        {
          fprintf (stderr, "ERROR: Could not open I/O port device (no driver)\n"
                           "       You can download the latest ioport driver from\n"
                           "       http://www.infernal.currantbun.com or http://ucon64.sourceforge.net\n");
          exit (1);
        }
      else
        {                                       // print warning, but continue
          printf ("WARNING: Support for the driver parnew is deprecated. Future versions of uCON64\n"
                  "         might not support this driver. You can download the latest ioport\n"
                  "         driver from http://www.infernal.currantbun.com or\n"
                  "         http://ucon64.sourceforge.net\n\n");
        }
    }

  if (atexit (close_io_port) == -1)
    {
      close (ucon64_io_fd);
      fprintf (stderr, "ERROR: Could not register function with atexit()\n");
      exit (1);
    }
#endif
#ifdef  AMIGA
  ucon64_io_fd = open ("PAR:", O_RDWR | O_NONBLOCK);
  if (ucon64_io_fd == -1)
    {
      fprintf (stderr, "ERROR: Could not open parallel port\n");
      exit (1);
    }
  if (atexit (close_io_port) == -1)
    {
      close (ucon64_io_fd);
      fprintf (stderr, "ERROR: Could not register function with atexit()\n");
      exit(1);
    }
#endif
#if     defined __i386__ || defined _WIN32

#if     defined _WIN32 || defined __CYGWIN__
  /*
    We support the I/O port drivers inpout32.dll and io.dll, because using them
    is way easier than using UserPort or GiveIO. The drivers are also more
    reliable and seem to enable access to all I/O ports (at least it's *very*
    easy to crash Windows XP ;-) The only downside to inpout32.dll is that it's
    almost two times slower than UserPort...
  */
  char fname[FILENAME_MAX];
  int driver_found = 0;
  
  sprintf (fname, "%s" FILE_SEPARATOR_S "%s", ucon64.configdir, "io.dll");
#if 0 // We must not do this for Cygwin or access() won't "find" the file
  change_mem (fname, strlen (fname), "/", 1, 0, 0, "\\", 1, 0);
#endif
  if (access (fname, F_OK) == 0)
    {
      io_driver = open_module (fname);

      IsDriverInstalled = 
#ifdef  __cplusplus // this is really nice: gcc wants something else than g++...
                          (short int (WINAPI *) ())
#endif
                          get_symbol (io_driver, "IsDriverInstalled");
      if (IsDriverInstalled ())
        {
          driver_found = 1;
          printf ("Using %s\n", fname);

          PortIn = 
#ifdef  __cplusplus
                   (char (WINAPI *) (short int))
#endif
                   get_symbol (io_driver, "PortIn");
          PortWordIn = 
#ifdef  __cplusplus
                       (short int (WINAPI *) (short int))
#endif
                       get_symbol (io_driver, "PortWordIn");
          PortOut =
#ifdef  __cplusplus
                    (void (WINAPI *) (short int, char))
#endif
                    get_symbol (io_driver, "PortOut");
          PortWordOut =
#ifdef  __cplusplus
                        (void (WINAPI *) (short int, short int))
#endif
                        get_symbol (io_driver, "PortWordOut");
          input_byte = io_input_byte;
          input_word = io_input_word;
          output_byte = io_output_byte;
          output_word = io_output_word;
        }
    }

  if (!driver_found)
    {
      sprintf (fname, "%s" FILE_SEPARATOR_S "%s", ucon64.configdir, "inpout32.dll");
      if (access (fname, F_OK) == 0)
        {
          driver_found = 1;
          printf ("Using %s\n", fname);
          io_driver = open_module (fname);
          Inp32 = 
#ifdef  __cplusplus
                  (unsigned char (__stdcall *) (unsigned short))
#endif
                  get_symbol (io_driver, "Inp32");
          Outp32 =
#ifdef  __cplusplus
                   (void (__stdcall *) (unsigned short, unsigned char))
#endif
                   get_symbol (io_driver, "Out32");
          // note that inport_word and output_word keep their default value...
          input_byte = inpout32_input_byte;
          output_byte = inpout32_output_byte;
        }
    }
#endif // _WIN32 || __CYGWIN__

  if (!port)                                    // no port specified or forced?
    {
      unsigned int parport_addresses[] = { 0x3bc, 0x378, 0x278 };
      int x, found = 0;

      for (x = 0; x < 3; x++)
        if ((found = ucon64_parport_probe (parport_addresses[x])) == 1)
          {
            port = parport_addresses[x];
            break;
          }

      if (found != 1)
        {
          fprintf (stderr, "ERROR: Could not find a parallel port on your system\n"
                           "       Try " OPTION_LONG_S "port=PORT to specify it by hand\n\n");
          exit (1);
        }
    }
#endif // __i386__

#if     defined __linux__ || defined __FreeBSD__
#ifdef  __FreeBSD__
  if (i386_set_ioperm (port, 3, 1) == -1)       // data, status & control
#else
  if (ioperm (port, 3, 1) == -1)                // data, status & control
#endif
    {
      fprintf (stderr,
              "ERROR: Could not set port permissions for I/O ports 0x%x, 0x%x and 0x%x\n"
              "       (This program needs root privileges for the requested action)\n",
              port + PARPORT_DATA, port + PARPORT_STATUS, port + PARPORT_CONTROL);
      exit (1);                                 // Don't return, if ioperm() fails port access
    }                                           //  causes core dump
#endif // __linux__ || __FreeBSD__

  outportb ((unsigned short) (port + PARPORT_CONTROL), 
            (unsigned char) (inportb ((unsigned short) (port + PARPORT_CONTROL)) & 0x0f));
  // bit 4 = 0 -> IRQ disable for ACK, bit 5-7 unused

#ifdef  __linux__
  /*
    Some code needs us to switch to the real uid and gid. However, other code
    needs access to I/O ports other than the standard printer port registers.
    We just do an iopl(3) and all code should be happy. Using iopl(3) enables
    users to run all code without being root (of course with the uCON64
    executable setuid root). Anyone a better idea?
  */
  if (iopl (3) == -1)
    {
      fprintf (stderr, "ERROR: Could not set the I/O privilege level to 3\n"
                       "       (This program needs root privileges for the requested action)\n");
      return 1;
    }
#endif // __linux__

  return port;
}
#endif // PARALLEL


int
ucon64_gauge (time_t init_time, int pos, int size)
{
  if (!ucon64.frontend)
    return gauge (init_time, pos, size);
  else
    {
      int percentage = (int) ((((int64_t) 100) * pos) / size);

      printf ("%u\n", percentage);
      fflush (stdout);
      return 0;
    }
}


int
ucon64_testsplit (const char *filename)
// test if ROM is split into parts based on the name of files
{
  int x, parts = 0, l;
  char buf[FILENAME_MAX], *p = NULL;

  for (x = -1; x < 2; x += 2)
    {
      parts = 0;
      strcpy (buf, filename);
      p = strrchr (buf, '.');
      l = strlen (buf);

      if (p == NULL)                            // filename doesn't contain a period
        p = buf + l - 1;
      else
        p += x;                                 // if x == -1 change char before '.'
                                                //  else if x == 1 change char after '.'
      if (buf > p ||                            // filename starts with '.' (x == -1)
          p - buf > l - 1)                      // filename ends with '.' (x == 1)
        continue;

      while (!access (buf, F_OK))
        (*p)--;                                 // "rewind" (find the first part)
      (*p)++;

      while (!access (buf, F_OK))               // count split parts
        {
          if (ucon64_testsplit_callback)
            ucon64_testsplit_callback (buf);
          (*p)++;
          parts++;
        }

      if (parts > 1)
        return parts;
    }

  return 0;
}


// configfile handling
int
ucon64_configfile (void)
{
  char buf[MAXBUFSIZE], *dirname;

  dirname = getenv2 ("UCON64_HOME");
  if (!dirname[0])
    dirname = getenv2 ("HOME");
  sprintf (ucon64.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
    "ucon64.cfg"
#else
    ".ucon64rc"
#endif
    , dirname);
#ifdef  DJGPP
  // this is DJGPP specific - not necessary, but prevents confusion
  change_mem (ucon64.configfile, strlen (ucon64.configfile), "/", 1, 0, 0, "\\", 1, 0);
#endif

//  if (!access (ucon64.configfile, F_OK))
//    fprintf (stderr, ucon64_msg[READ_CONFIG_FILE], ucon64.configfile);

  if (access (ucon64.configfile, F_OK) != 0)
    {
      FILE *fh;

      printf ("WARNING: %s not found: creating...", ucon64.configfile);

      if (!(fh = fopen (ucon64.configfile, "w"))) // opening the file in text mode
        {                                         //  avoids trouble under DOS
          printf ("FAILED\n\n");
          return -1;
        }
      else
        {
          fprintf (fh, "# uCON64 config\n"
                 "#\n"
                 "version=%d\n"
                 "#\n"
                 "# create backups of files? (1=yes; 0=no)\n"
//                 "# before processing a ROM uCON64 will make a backup of it\n"
                 "#\n"
                 "backups=1\n"
#ifdef  ANSI_COLOR
                 "#\n"
                 "# use ANSI colors in output? (1=yes; 0=no)\n"
                 "#\n"
                 "ansi_color=1\n"
#endif
                 "#\n"
                 "# parallel port\n"
                 "#\n"
                 "#parport=378\n"
                 "#\n"
#if     defined __MSDOS__
                 "discmage_path=~\\discmage.dxe\n" // realpath2() expands the tilde
                 "netgui_path=~\\netgui.dxe\n"
                 "ucon64_configdir=~\n"
                 "ucon64_datdir=~\n"
                 "ucon64_skindir=~\n"
#elif   defined __CYGWIN__
                 "discmage_path=~/discmage.dll\n"
                 "netgui_path=~/netgui.dll\n"
                 "ucon64_configdir=~\n"
                 "ucon64_datdir=~\n"
                 "ucon64_skindir=~\n"
#elif   defined _WIN32
                 "discmage_path=~\\discmage.dll\n"
                 "netgui_path=~/netgui.dll\n"
                 "ucon64_configdir=~\n"
                 "ucon64_datdir=~\n"
                 "ucon64_skindir=~\n"
#elif   defined __unix__ || defined __BEOS__
                 "discmage_path=~/.ucon64/discmage.so\n"
                 "netgui_path=~/.ucon64/netgui.so\n"
                 "ucon64_configdir=~/.ucon64\n"
                 "ucon64_datdir=~/.ucon64/dat\n"
                 "ucon64_skindir=~/.ucon64/skin\n"
#endif
                 "#\n"
                 "# emulate_<console shortcut>=<emulator with options>\n"
                 "#\n"
                 "# You can also use CRC32 values for ROM specific emulation options:\n"
                 "#\n"
                 "# emulate_0x<crc32>=<emulator with options>\n"
                 "# emulate_<crc32>=<emulator with options>\n"
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
#if 0
#ifndef __MSDOS__
                 "#\n"
                 "# LHA support\n"
                 "#\n"
                 "lha_extract=lha efi \"%%s\"\n"
                 "#\n"
                 "# LZH support\n"
                 "#\n"
                 "lzh_extract=lha efi \"%%s\"\n"
                 "#\n"
                 "# ZIP support\n"
                 "#\n"
                 "zip_extract=unzip -xojC \"%%s\"\n"
                 "#\n"
                 "# RAR support\n"
                 "#\n"
                 "rar_extract=unrar x \"%%s\"\n"
                 "#\n"
                 "# ACE support\n"
                 "#\n"
                 "ace_extract=unace e \"%%s\"\n"
#endif
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
                 , UCON64_CONFIG_VERSION);
          fclose (fh);
          printf ("OK\n\n");
        }
    }
  else if (strtol (get_property (ucon64.configfile, "version", buf, "0"), NULL, 10) < UCON64_CONFIG_VERSION)
    {
      strcpy (buf, ucon64.configfile);
      set_suffix (buf, ".OLD");

      printf ("NOTE: Updating config, old version will be renamed to %s...", buf);

      q_fcpy (ucon64.configfile, 0, q_fsize (ucon64.configfile), buf, "wb"); // "wb" is correct for copying

      sprintf (buf, "%d", UCON64_CONFIG_VERSION);
      set_property (ucon64.configfile, "version", buf);

      set_property (ucon64.configfile, "ansi_color", "1");

      set_property (ucon64.configfile, "discmage_path",
#if     defined __MSDOS__
        "~\\discmage.dxe"                       // realpath2() expands the tilde
#elif   defined __CYGWIN__
        "~/discmage.dll"
#elif   defined _WIN32
        "~\\discmage.dll"
#elif   defined __unix__ || defined __BEOS__
        "~/.ucon64/discmage.so"
#else
        ""
#endif
      );

      set_property (ucon64.configfile, "netgui_path",
#if     defined __MSDOS__
        "~\\netgui.dxe"                       // realpath2() expands the tilde
#elif   defined __CYGWIN__
        "~/netgui.dll"
#elif   defined _WIN32
        "~\\netgui.dll"
#elif   defined __unix__ || defined __BEOS__
        "~/.ucon64/netgui.so"
#else
        ""
#endif
      );

      set_property (ucon64.configfile, "ucon64_configdir",
#if     defined __MSDOS__
        "~"                                     // realpath2() expands the tilde
#elif   defined __CYGWIN__
        "~"
#elif   defined _WIN32
        "~"
#elif   defined __unix__ || defined __BEOS__
        "~/.ucon64"
#else
        ""
#endif
      );

      set_property (ucon64.configfile, "ucon64_datdir",
#if     defined __MSDOS__
        "~"                                     // realpath2() expands the tilde
#elif   defined __CYGWIN__
        "~"
#elif   defined _WIN32
        "~"
#elif   defined __unix__ || defined __BEOS__
        "~/.ucon64/dat"
#else
        ""
#endif
      );

      set_property (ucon64.configfile, "ucon64_skindir",
#if     defined __MSDOS__
        "~"                                     // realpath2() expands the tilde
#elif   defined __CYGWIN__
        "~"
#elif   defined _WIN32
        "~"
#elif   defined __unix__ || defined __BEOS__
        "~/.ucon64/skin"
#else
        ""
#endif
      );

#if 0
      DELETE_PROPERTY (ucon64.configfile, "cdrw_read");
      DELETE_PROPERTY (ucon64.configfile, "cdrw_write");

      set_property (ucon64.configfile, "lha_extract", "lha efi \"%s\"");
      set_property (ucon64.configfile, "lzh_extract", "lha efi \"%s\"");
      set_property (ucon64.configfile, "zip_extract", "unzip -xojC \"%s\"");
      set_property (ucon64.configfile, "rar_extract", "unrar x \"%s\"");
      set_property (ucon64.configfile, "ace_extract", "unace e \"%s\"");
#endif

      sync ();
      printf ("OK\n\n");
    }
  return 0;
}
