/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

Copyright (c) 1999 - 2005 NoisyB
Copyright (c) 2001 - 2005 dbjh


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
  First I want to thank SiGMA SEVEN! who was my mentor and taught me how to
  write programs in C.
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
#ifdef  USE_PARALLEL
#include "misc/parallel.h"
#endif
#include "misc/bswap.h"
#include "misc/misc.h"
#include "misc/property.h"
#include "misc/chksum.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"
#include "misc/string.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ucon64_opts.h"
#include "ucon64_dat.h"
#include "console/console.h"
#include "patch/patch.h"
#include "backup/backup.h"


static void ucon64_exit (void);
static int ucon64_execute_options (void);
static void ucon64_rom_nfo (const st_rominfo_t *rominfo);
static st_rominfo_t *ucon64_probe (st_rominfo_t *rominfo);
static int ucon64_rom_handling (void);
static int ucon64_process_rom (const char *fname);


st_ucon64_t ucon64;                             // containes ptr to image, dat and rominfo

static const char *ucon64_title = "uCON64 " UCON64_VERSION_S " " CURRENT_OS_S " 1999-2005";

#ifdef  AMIGA
unsigned long __stacksize = 102400;             // doesn't work on PPC? is StormC specific?
//unsigned long __stack = 102400;               // for SAS/C, DICE, GCC etc.?
char vers[] = "$VER: uCON64 "UCON64_VERSION_S" "CURRENT_OS_S" ("__DATE__") ("__TIME__")";
#endif

typedef struct
{
  int val;         // (st_getopt2_t->val)
//  const
    char *optarg;  // option argument
  int console;     // the console (st_getopt2_t->object)
  int flags;       // workflow flags (st_getopt2_t->object)
} st_args_t;

static st_args_t arg[UCON64_MAX_ARGS];

static st_getopt2_t options[UCON64_MAX_ARGS];
static const st_getopt2_t lf[] =
  {
    {NULL, 0, 0, 0, NULL, "", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  *option[] =
  {
    ucon64_options_usage,
    ucon64_options_without_usage,
    lf,
    ucon64_padding_usage,
    lf,
    ucon64_dat_usage,
    lf,
    ucon64_patching_usage,
    bsl_usage,
    ips_usage,
    aps_usage,
    ppf_usage,
    gg_usage,
    lf,
#ifdef  USE_DISCMAGE
    libdm_usage,
    lf,
#endif
    dc_usage,
    lf,
    psx_usage,
#ifdef  USE_PARALLEL
    dex_usage,
#endif
    lf,
    gba_usage,
#if     defined USE_PARALLEL || defined USE_USB
    // f2a_usage has to come before fal_usage in case only USE_USB is defined
    //  (no support for parallel port)
    f2a_usage,
#ifdef  USE_PARALLEL
    fal_usage,
#endif
#endif // USE_PARALLEL || USE_USB
#if 0
    // this backup unit uses CF cards for transfer
    sc_usage,
#endif
    lf,
    nds_usage,
    lf,
    n64_usage,
#ifdef  USE_PARALLEL
    doctor64_usage,
    doctor64jr_usage,
#ifdef  USE_LIBCD64
    cd64_usage,
#endif
    dex_usage,
#endif // USE_PARALLEL
    lf,
    snes_usage,
#ifdef  USE_PARALLEL
    swc_usage,
    gd_usage,
    fig_usage,
    sflash_usage,
  //  mgd_usage,
#endif
    lf,
    neogeo_usage,
    lf,
    genesis_usage,
#ifdef  USE_PARALLEL
    smd_usage,
    mdpro_usage,
    mcd_usage,
    cmc_usage,
  //  mgd_usage,
#endif
    lf,
    gameboy_usage,
#ifdef  USE_PARALLEL
    gbx_usage,
    mccl_usage,
#endif
    lf,
    lynx_usage,
#ifdef  USE_PARALLEL
    lynxit_usage,
#endif
    lf,
    pcengine_usage,
#ifdef  USE_PARALLEL
    msg_usage,
    pcepro_usage,
  //  mgd_usage,
#endif
    lf,
    nes_usage,
#ifdef  USE_PARALLEL
    smc_usage,
#endif
    lf,
    sms_usage,
#ifdef  USE_PARALLEL
    smsggpro_usage,
#endif
    lf,
    swan_usage,
    lf,
    jaguar_usage,
    lf,
    ngp_usage,
#ifdef  USE_PARALLEL
    pl_usage,
#endif
    lf,
    atari_usage,
#if 0
// these backup units use audio to transfer ROMs
    cc2_usage,
    spsc_usage,
#endif
    lf,
    coleco_usage,
    lf,
    vboy_usage,
    NULL
  };


static st_rominfo_t *
ucon64_rom_flush (st_rominfo_t * rominfo)
{
  if (rominfo)
    memset (rominfo, 0, sizeof (st_rominfo_t));

  ucon64.rominfo = NULL;
  ucon64.crc32 = ucon64.fcrc32 = 0;             // yes, this belongs here
  rominfo->data_size = UCON64_UNKNOWN;

  return rominfo;
}


#ifdef  TEST
typedef struct
{
  int option;

  const char *cmdline;
  uint32_t crc32;    // crc32 of cmdline's output
} st_ucon64_test_t;


void
ucon64_test (void)
{
  st_ucon64_test_t test[] =
    {
#if 0
      {UCON64_1991,	"ucon64 -1991", 0},
      {UCON64_3DO,	"ucon64 -3do", 0},
      {UCON64_83,	"ucon64 -83", 0},
      {UCON64_A,	"ucon64 -a", 0},
      {UCON64_ATA,	"ucon64 -ata", 0},
      {UCON64_B,	"ucon64 -b", 0},
      {UCON64_B0,	"ucon64 -b0", 0},
      {UCON64_B1,	"ucon64 -b1", 0},
      {UCON64_BAT,	"ucon64 -bat", 0},
      {UCON64_BIN,	"ucon64 -bin", 0},
      {UCON64_BIN2ISO,	"ucon64 -bin2iso", 0},
      {UCON64_BIOS,	"ucon64 -bios", 0},
      {UCON64_BOT,	"ucon64 -bot", 0},
      {UCON64_BS,	"ucon64 -bs", 0},
      {UCON64_C,	"ucon64 -c", 0},
      {UCON64_CHK,	"ucon64 -chk", 0},
      {UCON64_CMNT,	"ucon64 -cmnt", 0},
      {UCON64_CODE,	"ucon64 -code", 0},
      {UCON64_COL,	"ucon64 -col", 0},
      {UCON64_COLECO,	"ucon64 -coleco", 0},
      {UCON64_CRC,	"ucon64 -crc", 0},
      {UCON64_CRCHD,	"ucon64 -crchd", 0},
      {UCON64_CRP,	"ucon64 -crp", 0},
      {UCON64_CS,	"ucon64 -cs", 0},
      {UCON64_CTRL,	"ucon64 -ctrl", 0},
      {UCON64_CTRL2,	"ucon64 -ctrl2", 0},
      {UCON64_DB,	"ucon64 -db", 0},
      {UCON64_DBS,	"ucon64 -dbs", 0},
      {UCON64_DBUH,	"ucon64 -dbuh", 0},
      {UCON64_DBV,	"ucon64 -dbv", 0},
      {UCON64_DC,	"ucon64 -dc", 0},
      {UCON64_DFIND,	"ucon64 -dfind", 0},
      {UCON64_DFINDR,	"ucon64 -dfindr", 0},
      {UCON64_DINT,	"ucon64 -dint", 0},
      {UCON64_DISC,	"ucon64 -disc", 0},
      {UCON64_DMIRR,	"ucon64 -dmirr", 0},
      {UCON64_DNSRT,	"ucon64 -dnsrt", 0},
      {UCON64_DUAL,	"ucon64 -dual", 0},
      {UCON64_DUMPINFO,	"ucon64 -dumpinfo", 0},
      {UCON64_E,	"ucon64 -e", 0},
      {UCON64_EROM,	"ucon64 -erom", 0},
      {UCON64_F,	"ucon64 -f", 0},
      {UCON64_FDS,	"ucon64 -fds", 0},
      {UCON64_FDSL,	"ucon64 -fdsl", 0},
      {UCON64_FFE,	"ucon64 -ffe", 0},
      {UCON64_FIG,	"ucon64 -fig", 0},
      {UCON64_FIGS,	"ucon64 -figs", 0},
      {UCON64_FILE,	"ucon64 -file", 0},
      {UCON64_FIND,	"ucon64 -find", 0},
      {UCON64_FINDI,	"ucon64 -findi", 0},
      {UCON64_FINDR,	"ucon64 -findr", 0},
      {UCON64_FORCE63,	"ucon64 -force63", 0},
      {UCON64_FRONTEND,	"ucon64 -frontend", 0},
      {UCON64_GB,	"ucon64 -gb", 0},
      {UCON64_GBA,	"ucon64 -gba", 0},
      {UCON64_GBX,	"ucon64 -gbx", 0},
      {UCON64_GC,	"ucon64 -gc", 0},
      {UCON64_GD3,	"ucon64 -gd3", 0},
      {UCON64_GD3S,	"ucon64 -gd3s", 0},
      {UCON64_GEN,	"ucon64 -gen", 0},
      {UCON64_GG,	"ucon64 -gg", 0},
      {UCON64_GGD,	"ucon64 -ggd", 0},
      {UCON64_GGE,	"ucon64 -gge", 0},
      {UCON64_GP32,	"ucon64 -gp32", 0},
      {UCON64_H,	"ucon64 -h", 0},
      {UCON64_HD,	"ucon64 -hd", 0},
      {UCON64_HDN,	"ucon64 -hdn", 0},
#endif
      {UCON64_HELP,	"ucon64 -help", 0x9576194e},
      {UCON64_HEX,	"ucon64 -hex test/test.txt", 0x9732f50c},
      {UCON64_HFIND,	"ucon64 -hfind \"? 68 ?? 6a\" test/test.txt", 0x6c43021b},
      {UCON64_HFINDR,	"ucon64 -hfindr \"01 02 03 04\" test/test.txt", 0},
#if 0
      {UCON64_HI,	"ucon64 -hi", 0},
      {UCON64_I,	"ucon64 -i", 0},
      {UCON64_ID,	"ucon64 -id", 0},
      {UCON64_IDNUM,	"ucon64 -idnum", 0},
      {UCON64_IDPPF,	"ucon64 -idppf", 0},
      {UCON64_INES,	"ucon64 -ines", 0},
      {UCON64_INESHD,	"ucon64 -ineshd", 0},
      {UCON64_INS,	"ucon64 -ins", 0},
      {UCON64_INSN,	"ucon64 -insn", 0},
      {UCON64_INT,	"ucon64 -int", 0},
      {UCON64_INT2,	"ucon64 -int2", 0},
      {UCON64_INTELLI,	"ucon64 -intelli", 0},
      {UCON64_ISOFIX,	"ucon64 -isofix", 0},
      {UCON64_ISPAD,	"ucon64 -ispad", 0},
      {UCON64_J,	"ucon64 -j", 0},
      {UCON64_JAG,	"ucon64 -jag", 0},
      {UCON64_K,	"ucon64 -k", 0},
      {UCON64_L,	"ucon64 -l", 0},
      {UCON64_LNX,	"ucon64 -lnx", 0},
      {UCON64_LOGO,	"ucon64 -logo", 0},
      {UCON64_LS,	"ucon64 -ls", 0},
      {UCON64_LSD,	"ucon64 -lsd", 0},
      {UCON64_LSRAM,	"ucon64 -lsram", 0},
      {UCON64_LSV,	"ucon64 -lsv", 0},
      {UCON64_LYNX,	"ucon64 -lynx", 0},
      {UCON64_LYX,	"ucon64 -lyx", 0},
      {UCON64_MAPR,	"ucon64 -mapr", 0},
      {UCON64_MD5,	"ucon64 -md5", 0},
      {UCON64_MGD,	"ucon64 -mgd", 0},
      {UCON64_MGDGG,	"ucon64 -mgdgg", 0},
      {UCON64_MIRR,	"ucon64 -mirr", 0},
      {UCON64_MKA,	"ucon64 -mka", 0},
      {UCON64_MKCUE,	"ucon64 -mkcue", 0},
      {UCON64_MKDAT,	"ucon64 -mkdat", 0},
      {UCON64_MKI,	"ucon64 -mki", 0},
      {UCON64_MKIP,	"ucon64 -mkip", 0},
      {UCON64_MKPPF,	"ucon64 -mkppf", 0},
      {UCON64_MKSHEET,	"ucon64 -mksheet", 0},
      {UCON64_MKTOC,	"ucon64 -mktoc", 0},
      {UCON64_MSG,	"ucon64 -msg", 0},
      {UCON64_MULTI,	"ucon64 -multi", 0},
      {UCON64_N,	"ucon64 -n", 0},
      {UCON64_N2,	"ucon64 -n2", 0},
      {UCON64_N2GB,	"ucon64 -n2gb", 0},
      {UCON64_N64,	"ucon64 -n64", 0},
      {UCON64_NA,	"ucon64 -na", 0},
      {UCON64_NBAK,	"ucon64 -nbak", 0},
      {UCON64_NBAT,	"ucon64 -nbat", 0},
      {UCON64_NBS,	"ucon64 -nbs", 0},
      {UCON64_NCOL,	"ucon64 -ncol", 0},
      {UCON64_NDS,	"ucon64 -nds", 0},
      {UCON64_NES,	"ucon64 -nes", 0},
      {UCON64_NG,	"ucon64 -ng", 0},
      {UCON64_NGP,	"ucon64 -ngp", 0},
      {UCON64_NHD,	"ucon64 -nhd", 0},
      {UCON64_NHI,	"ucon64 -nhi", 0},
      {UCON64_NINT,	"ucon64 -nint", 0},
      {UCON64_NPPF,	"ucon64 -nppf", 0},
      {UCON64_NROT,	"ucon64 -nrot", 0},
      {UCON64_NS,	"ucon64 -ns", 0},
      {UCON64_NTSC,	"ucon64 -ntsc", 0},
      {UCON64_NVRAM,	"ucon64 -nvram", 0},
      {UCON64_O,	"ucon64 -o", 0},
      {UCON64_P,	"ucon64 -p", 0},
      {UCON64_PAD,	"ucon64 -pad", 0},
      {UCON64_PADN,	"ucon64 -padn", 0},
      {UCON64_PAL,	"ucon64 -pal", 0},
      {UCON64_PARSE,	"ucon64 -parse", 0},
      {UCON64_PASOFAMI,	"ucon64 -pasofami", 0},
      {UCON64_PATCH,	"ucon64 -patch", 0},
      {UCON64_PATTERN,	"ucon64 -pattern", 0},
      {UCON64_PCE,	"ucon64 -pce", 0},
      {UCON64_POKE,	"ucon64 -poke", 0},
      {UCON64_PORT,	"ucon64 -port", 0},
      {UCON64_PPF,	"ucon64 -ppf", 0},
      {UCON64_PRINT,	"ucon64 -print", 0},
      {UCON64_PS2,	"ucon64 -ps2", 0},
      {UCON64_PSX,	"ucon64 -psx", 0},
      {UCON64_Q,	"ucon64 -q", 0},
      {UCON64_R,	"ucon64 -r", 0},
      {UCON64_R83,	"ucon64 -r83", 0},
      {UCON64_RDAT,	"ucon64 -rdat", 0},
      {UCON64_REGION,	"ucon64 -region", 0},
      {UCON64_RENAME,	"ucon64 -rename", 0},
      {UCON64_RIP,	"ucon64 -rip", 0},
      {UCON64_RJOLIET,	"ucon64 -rjoliet", 0},
      {UCON64_RL,	"ucon64 -rl", 0},
      {UCON64_ROM,	"ucon64 -rom", 0},
      {UCON64_ROTL,	"ucon64 -rotl", 0},
      {UCON64_ROTR,	"ucon64 -rotr", 0},
      {UCON64_RR83,	"ucon64 -rr83", 0},
      {UCON64_RROM,	"ucon64 -rrom", 0},
      {UCON64_RU,	"ucon64 -ru", 0},
      {UCON64_S,	"ucon64 -s", 0},
      {UCON64_S16,	"ucon64 -s16", 0},
      {UCON64_SAM,	"ucon64 -sam", 0},
      {UCON64_SAT,	"ucon64 -sat", 0},
      {UCON64_SC,	"ucon64 -sc", 0},
      {UCON64_SCAN,	"ucon64 -scan", 0},
      {UCON64_SCR,	"ucon64 -scr", 0},
      {UCON64_SGB,	"ucon64 -sgb", 0},
      {UCON64_SHA1,	"ucon64 -sha1", 0},
      {UCON64_SMC,	"ucon64 -smc", 0},
      {UCON64_SMD,	"ucon64 -smd", 0},
      {UCON64_SMDS,	"ucon64 -smds", 0},
      {UCON64_SMS,	"ucon64 -sms", 0},
      {UCON64_SNES,	"ucon64 -snes", 0},
      {UCON64_SRAM,	"ucon64 -sram", 0},
      {UCON64_SSC,	"ucon64 -ssc", 0},
      {UCON64_SSIZE,	"ucon64 -ssize", 0},
      {UCON64_STP,	"ucon64 -stp", 0},
      {UCON64_STPN,	"ucon64 -stpn", 0},
      {UCON64_STRIP,	"ucon64 -strip", 0},
      {UCON64_SWAN,	"ucon64 -swan", 0},
      {UCON64_SWAP,	"ucon64 -swap", 0},
      {UCON64_SWAP2,	"ucon64 -swap2", 0},
      {UCON64_SWC,	"ucon64 -swc", 0},
      {UCON64_SWCS,	"ucon64 -swcs", 0},
      {UCON64_UFO,	"ucon64 -ufo", 0},
      {UCON64_UFOS,	"ucon64 -ufos", 0},
      {UCON64_UNIF,	"ucon64 -unif", 0},
      {UCON64_UNSCR,	"ucon64 -unscr", 0},
      {UCON64_USMS,	"ucon64 -usms", 0},
      {UCON64_V,	"ucon64 -v", 0},
      {UCON64_V64,	"ucon64 -v64", 0},
      {UCON64_VBOY,	"ucon64 -vboy", 0},
      {UCON64_VEC,	"ucon64 -vec", 0},
      {UCON64_VERSION,	"ucon64 -version", 0},
      {UCON64_VRAM,	"ucon64 -vram", 0},
      {UCON64_XBOX,	"ucon64 -xbox", 0},
      {UCON64_XCMC,	"ucon64 -xcmc", 0},
      {UCON64_XCMCM,	"ucon64 -xcmcm", 0},
      {UCON64_XCMCT,	"ucon64 -xcmct", 0},
      {UCON64_XDEX,	"ucon64 -xdex", 0},
      {UCON64_XDJR,	"ucon64 -xdjr", 0},
      {UCON64_XF2A,	"ucon64 -xf2a", 0},
      {UCON64_XF2AB,	"ucon64 -xf2ab", 0},
      {UCON64_XF2AC,	"ucon64 -xf2ac", 0},
      {UCON64_XF2AMULTI,	"ucon64 -xf2amulti", 0},
      {UCON64_XF2AS,	"ucon64 -xf2as", 0},
      {UCON64_XFAL,	"ucon64 -xfal", 0},
      {UCON64_XFALB,	"ucon64 -xfalb", 0},
      {UCON64_XFALC,	"ucon64 -xfalc", 0},
      {UCON64_XFALM,	"ucon64 -xfalm", 0},
      {UCON64_XFALMULTI,	"ucon64 -xfalmulti", 0},
      {UCON64_XFALS,	"ucon64 -xfals", 0},
      {UCON64_XFIG,	"ucon64 -xfig", 0},
      {UCON64_XFIGC,	"ucon64 -xfigc", 0},
      {UCON64_XFIGS,	"ucon64 -xfigs", 0},
      {UCON64_XGBX,	"ucon64 -xgbx", 0},
      {UCON64_XGBXB,	"ucon64 -xgbxb", 0},
      {UCON64_XGBXM,	"ucon64 -xgbxm", 0},
      {UCON64_XGBXS,	"ucon64 -xgbxs", 0},
      {UCON64_XGD3,	"ucon64 -xgd3", 0},
      {UCON64_XGD3R,	"ucon64 -xgd3r", 0},
      {UCON64_XGD3S,	"ucon64 -xgd3s", 0},
      {UCON64_XGD6,	"ucon64 -xgd6", 0},
      {UCON64_XGD6R,	"ucon64 -xgd6r", 0},
      {UCON64_XGD6S,	"ucon64 -xgd6s", 0},
      {UCON64_XGG,	"ucon64 -xgg", 0},
      {UCON64_XGGB,	"ucon64 -xggb", 0},
      {UCON64_XGGS,	"ucon64 -xggs", 0},
      {UCON64_XLIT,	"ucon64 -xlit", 0},
      {UCON64_XMCCL,	"ucon64 -xmccl", 0},
      {UCON64_XMCD,	"ucon64 -xmcd", 0},
      {UCON64_XMD,	"ucon64 -xmd", 0},
      {UCON64_XMDB,	"ucon64 -xmdb", 0},
      {UCON64_XMDS,	"ucon64 -xmds", 0},
      {UCON64_XMSG,	"ucon64 -xmsg", 0},
      {UCON64_XPCE,	"ucon64 -xpce", 0},
      {UCON64_XPL,	"ucon64 -xpl", 0},
      {UCON64_XPLI,	"ucon64 -xpli", 0},
      {UCON64_XPLM,	"ucon64 -xplm", 0},
      {UCON64_XRESET,	"ucon64 -xreset", 0},
      {UCON64_XSF,	"ucon64 -xsf", 0},
      {UCON64_XSFS,	"ucon64 -xsfs", 0},
      {UCON64_XSMC,	"ucon64 -xsmc", 0},
      {UCON64_XSMCR,	"ucon64 -xsmcr", 0},
      {UCON64_XSMD,	"ucon64 -xsmd", 0},
      {UCON64_XSMDS,	"ucon64 -xsmds", 0},
      {UCON64_XSWC,	"ucon64 -xswc", 0},
      {UCON64_XSWC-IO,	"ucon64 -xswc-io", 0},
      {UCON64_XSWC2,	"ucon64 -xswc2", 0},
      {UCON64_XSWCC,	"ucon64 -xswcc", 0},
      {UCON64_XSWCR,	"ucon64 -xswcr", 0},
      {UCON64_XSWCS,	"ucon64 -xswcs", 0},
      {UCON64_XV64,	"ucon64 -xv64", 0},
      {UCON64_Z64,	"ucon64 -z64", 0},
#endif
      {0, NULL, 0}
    };
  int x = 0;
  unsigned int crc;
  char buf[MAXBUFSIZE];

  for (; test[x].option; x++)
    {
      FILE *in, *out;

      // NO testing?
      if (!test[x].cmdline)
        continue;
        
      sprintf (buf, "%s 2>&1", test[x].cmdline);
      if (!(in = popen (buf, "r")))
        {
          fprintf (stderr, "ERROR: cmdline \"%s\"\n", test[x].cmdline);
          continue;
        }

      sprintf (buf, "%d-output.txt", test[x].option);
      out = fopen (buf, "w");
      
      crc = 0;
      while ((fgets (buf, MAXBUFSIZE, in)))
        {
          crc = crc32 (crc, (const void *) &buf, strlen (buf));
          fputs (buf, out);
        }

      printf ("option: %d crc: 0x%08x calc: 0x%08x status: %s\n",
        test[x].option,
        test[x].crc32,
        crc,
        test[x].crc32 == crc ? "OK" : "BUG?");
      
      fclose (in);
      fclose (out);
    }
  exit (0);
}
#endif


#ifdef  DEBUG
void
ucon64_runtime_debug_output (st_getopt2_t *p)
{
  printf ("{\"%s\", %d, 0, %d, \"%s\", \"%s\", %d}, // console: %d workflow: %d\n",
    p->name,
    p->has_arg,
    p->val,
    p->arg_name,
    p->help ? "usage" : p->help, // i (nb) mean it
//    p->help,
    0,
    p->object ? ((st_ucon64_obj_t *) p->object)->console : 0,
    p->object ? ((st_ucon64_obj_t *) p->object)->flags : 0);
}


static void
ucon64_runtime_debug (void)
{
  int x = 0, y = 0, c = 0;
  (void) x;
  (void) y;
  (void) c;

#if 0
  // how many options (incl. dupes) do we have?
  for (x = y = 0; options[x].name || options[x].help; x++)
    if (options[x].name)
      y++;
  printf ("DEBUG: Total options (with dupes): %d\n", y);
  printf ("DEBUG: UCON64_MAX_ARGS == %d, %s\n", UCON64_MAX_ARGS,
    (y < UCON64_MAX_ARGS ? "good" : "\nERROR: too small; must be larger than options"));
#endif

#if 0
  // list all options as a single st_getopt2_t array
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // how many consoles does uCON64 support?
  for (x = y = 0; options[x].name || options[x].help; x++)
    if (options[x].name && options[x].object)
      if (options[x].val == ((st_ucon64_obj_t *) options[x].object)->console)
        ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options without an object (allowed)
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name && !options[x].object)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options without a console (allowed)
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name && !((st_ucon64_obj_t *) options[x].object)->console)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options without a workflow (allowed)
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name && !((st_ucon64_obj_t *) options[x].object)->flags)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options without a val (NOT allowed)
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name && !options[x].val)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options with has_arg but without arg_name AND/OR usage
  // hidden options without arg_name AND usage are allowed
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name &&
        ((!options[x].has_arg && options[x].arg_name) ||
         (options[x].has_arg && !options[x].arg_name) ||
         !options[x].help))
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find dupe (NOT a problem) options that have different values for val,
  // flag, and/or object (NOT allowed)
  // getopt1() will always use the 1st option in the array
  // (st_getopt2_t *)->arg_name and (st_getopt2_t *)->help can be as
  // different as you like
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name)
      for (y = 0; options[y].name || options[y].help; y++)
        if (options[y].name && x != y) // IS option
          if (!strcmp (options[y].name, options[x].name))
            if (options[y].has_arg != options[x].has_arg || // (NOT allowed)
                options[y].flag != options[x].flag || // (NOT allowed)
                options[y].val != options[x].val || // (NOT allowed)
//                options[y].arg_name != options[x].arg_name || // (allowed)
//                options[y].help != options[x].help || // (allowed)
                ((st_ucon64_obj_t *) options[y].object)->console != ((st_ucon64_obj_t *) options[x].object)->console // (NOT allowed)
                ((st_ucon64_obj_t *) options[x].object)->flags != ((st_ucon64_obj_t *) options[x].object)->flags) // (NOT allowed)
              {
                fputs ("ERROR: different dupe options found\n  ", stdout);
                ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
                fputs ("  ", stdout);
                ucon64_runtime_debug_output ((st_getopt2_t *) &options[y]);
                fputs ("\n\n", stdout);
              }
#endif
  puts ("DEBUG: Sanity check finished");
  fflush (stdout);
}
#endif  // DEBUG


void
ucon64_exit (void)
{
#ifdef  USE_DISCMAGE
  if (ucon64.discmage_enabled)
    if (ucon64.image)
      dm_close ((dm_image_t *) ucon64.image);
#endif

  handle_registered_funcs ();
  fflush (stdout);
}


int
main (int argc, char **argv)
{
  int x = 0, y = 0, c = 0;
#if (FILENAME_MAX < MAXBUFSIZE)
  static char buf[MAXBUFSIZE];
#else
  static char buf[FILENAME_MAX];
#endif
  struct stat fstate;
  struct option long_options[UCON64_MAX_ARGS];

#ifdef  TEST
  if (argc == 1)
    ucon64_test ();
#endif

  printf ("%s\n"
    "Uses code from various people. See 'developers.html' for more!\n"
    "This may be freely redistributed under the terms of the GNU Public License\n\n",
    ucon64_title);

  if (atexit (ucon64_exit) == -1)
    {
      fputs ("ERROR: Could not register function with atexit()\n", stderr);
      exit (1);
    }

  // flush st_ucon64_t
  memset (&ucon64, 0, sizeof (st_ucon64_t));

  // these members of ucon64 (except rom and fname_arch) don't change per file
  ucon64.argc = argc;
  ucon64.argv = argv;                           // must be set prior to calling
                                                //  ucon64_load_discmage() (for DOS)
  ucon64.rom =
  ucon64.file =
  ucon64.mapr =
  ucon64.comment = "";

  ucon64.fname_arch[0] = 0;

  ucon64.recursive =
  ucon64.parport_needed =
  ucon64.io_mode = 0;

  ucon64.battery =
  ucon64.bs_dump =
  ucon64.buheader_len =
  ucon64.console =
  ucon64.controller =
  ucon64.controller2 =
  ucon64.do_not_calc_crc =
  ucon64.id =
  ucon64.interleaved =
  ucon64.mirror =
  ucon64.part_size =
  ucon64.region =
  ucon64.snes_header_base =
  ucon64.snes_hirom =
  ucon64.tv_standard =
  ucon64.use_dump_info =
  ucon64.vram = UCON64_UNKNOWN;

  ucon64.flags = WF_DEFAULT;


  // convert (st_getopt2_t **) to (st_getopt2_t *)
  memset (&options, 0, sizeof (st_getopt2_t) * UCON64_MAX_ARGS);
  for (c = x = 0; option[x]; x++)
    for (y = 0; option[x][y].name || option[x][y].help; y++)
      if (c < UCON64_MAX_ARGS)
        {
          memcpy (&options[c], &option[x][y], sizeof (st_getopt2_t));
          c++;
        }
  ucon64.options = options;

#ifdef  DEBUG
  ucon64_runtime_debug (); // check (st_getopt2_t *) options consistency
#endif

#ifdef  __unix__
  // We need to modify the umask, because the configfile is made while we are
  //  still running in root mode. Maybe 0 is even better (in case root did
  //  `chmod +s').
  umask (002);
#endif
  ucon64_configfile ();

#ifdef  USE_ANSI_COLOR
  // ANSI colors?
  ucon64.ansi_color = get_property_int (ucon64.configfile, "ansi_color");
  // the conditional call to ansi_init() has to be done *after* the check for
  //  the switch --ncol
#endif

  // parallel port?
#ifdef  USE_PPDEV
  get_property (ucon64.configfile, "parport_dev", ucon64.parport_dev, "/dev/parport0");
#elif   defined AMIGA
  get_property (ucon64.configfile, "parport_dev", ucon64.parport_dev, "parallel.device");
#endif
  // use -1 (UCON64_UNKNOWN) to force probing if the config file doesn't contain
  //  a parport line
  sscanf (get_property (ucon64.configfile, "parport", buf, "-1"), "%x", &ucon64.parport);

  // make backups?
  ucon64.backup = get_property_int (ucon64.configfile, "backups");

  // $HOME/.ucon64/ ?
  get_property_fname (ucon64.configfile, "ucon64_configdir", ucon64.configdir, "");

  // DAT file handling
  ucon64.dat_enabled = 0;
  get_property_fname (ucon64.configfile, "ucon64_datdir", ucon64.datdir, "");

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

  if (argc < 2)
    {
      ucon64_usage (argc, argv);
      return 0;
    }


  // turn st_getopt2_t into struct option
  getopt2_long_only (long_options, options, UCON64_MAX_ARGS);

  // getopt() is utilized to make uCON64 handle/parse cmdlines in a sane
  //  and expected way
  x = optind = 0;
  memset (&arg, 0, sizeof (st_args_t) * UCON64_MAX_ARGS);
  while ((c = getopt_long_only (argc, argv, "", long_options, NULL)) != -1)
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
          const st_ucon64_obj_t *p = (st_ucon64_obj_t *) getopt2_get_index_by_val (options, c)->object;

          arg[x].console = UCON64_UNKNOWN; // default

          if (p)
            {
              arg[x].flags = p->flags;
              if (p->console)
                arg[x].console = p->console;
            }

          arg[x].val = c;
          arg[x++].optarg = (optarg ? optarg : NULL);
        }
      else
        // this shouldn't happen
        exit (1);
    }

#ifdef  DEBUG
  for (x = 0; arg[x].val; x++)
    printf ("%d %s %d %d\n\n",
      arg[x].val,
      arg[x].optarg ? arg[x].optarg : "(null)",
      arg[x].flags,
      arg[x].console);
#endif


#ifdef  USE_DISCMAGE
  // load libdiscmage (should be done before handling the switches (--ver))
  ucon64.discmage_enabled = ucon64_load_discmage ();
#endif

  // switches
  for (x = 0; arg[x].val; x++)
    {
      if (arg[x].console != UCON64_UNKNOWN)
        ucon64.console = arg[x].console;
      if (arg[x].flags)
        ucon64.flags = arg[x].flags;
      if (arg[x].val)
        ucon64.option = arg[x].val;
      ucon64.optarg = arg[x].optarg;

//      if (ucon64.flags & WF_SWITCH)
        ucon64_switches (&ucon64);
    }

#ifdef  USE_ANSI_COLOR
  if (ucon64.ansi_color)
    ucon64.ansi_color = ansi_init ();
#endif

  /*
    Call ucon64_dat_indexer() after handling the switches and after calling
    ansi_init() so that the progress bar is displayed correctly (colour/no
    colour).
  */
  if (ucon64.dat_enabled)
    ucon64_dat_indexer ();              // update cache (index) files if necessary

#ifdef  USE_PARALLEL
  /*
    The copier options need root privileges for parport_open(). We can't use
    ucon64.flags & WF_PAR to detect whether a (parallel port) copier option has
    been specified, because another switch might've been specified after -port.
  */
  if (ucon64.parport_needed == 1)
    ucon64.parport = parport_open (ucon64.parport);
#endif
#if     defined __unix__ && !defined __MSDOS__
  /*
    We can drop privileges after we have set up parallel port access. We cannot
    drop privileges if the user wants to communicate with the USB version of the
    F2A.
    SECURITY WARNING: We stay in root mode if the user specified an F2A option!
    We could of course drop privileges which requires the user to run uCON64 as
    root (not setuid root), but we want to be user friendly. Besides, doing
    things as root is bad anyway (from a security viewpoint).
  */
  if (ucon64.parport_needed != 2
#ifdef  USE_USB
      && !ucon64.usbport
#endif
     )
    drop_privileges ();
#endif // __unix__ && !__MSDOS__

  if (optind == argc)                   // no file was specified (e.g. --db)
    ucon64_execute_options();
  else
    {
      int flags = GETOPT2_FILE_FILES_ONLY;
      if (ucon64.recursive)
        flags |= GETOPT2_FILE_RECURSIVE;
      else 
        {
          /*
            Check if one of the parameters is a directory and if so, set the
            flag GETOPT2_FILE_RECURSIVE_ONCE. This flag makes uCON64 behave
            like version 2.0.0, i.e., specifying a directory is equivalent to
            specifying all files in that directory. In commands:
              ucon64 file dir1 dir2
            is equivalent to:
              ucon64 file dir1\* dir2\*
            Once the flag is set it is not necessary to check the remaining
            parameters.

            TODO: Find a solution for the fact that the stat() implementation
                  of MinGW and VC++ don't accept a path with an ending slash.
          */
          int i = optind;
          for (; i < argc; i++)
            if (!stat (argv[i], &fstate))
              if (S_ISDIR (fstate.st_mode))
                {
                  flags |= GETOPT2_FILE_RECURSIVE_ONCE;
                  break;
                }
        }
      getopt2_file (argc, argv, ucon64_process_rom, flags);
    }

  return 0;
}


int
ucon64_process_rom (const char *fname)
{
#ifdef  USE_ZLIB
  int n_entries;
#endif

  // Try to get file status information only if the file exists. We have to
  //  accept non-existing files for the dump options.
  if (access (fname, F_OK) == 0)
    {
      struct stat fstate;
      if (stat (fname, &fstate) == -1)
        return 0;
      if (!S_ISREG (fstate.st_mode))
        return 0;
    }

#ifdef  USE_ZLIB
  n_entries = unzip_get_number_entries (fname);
  if (n_entries != -1)                          // it's a zip file
    {
      for (unzip_current_file_nr = 0; unzip_current_file_nr < n_entries;
           unzip_current_file_nr++)
        {
          ucon64_fname_arch (fname);
          /*
            There seems to be no other way to detect directories in ZIP files
            than by looking at the file name. Paths in ZIP files should contain
            forward slashes. ucon64_fname_arch() changes forward slashes into
            backslashes (FILE_SEPARATORs) when uCON64 is compiled with Visual
            C++ or MinGW so that basename2() always produces a correct base
            name. So, if the entry in the ZIP file is a directory
            ucon64.fname_arch will be an empty string.
          */
          if (ucon64.fname_arch[0] == 0)
            continue;

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
  Please, if you experience problems then try your luck with the flags in
  ucon64_misc.c/ucon64_wf[] before changing things here or in
  ucon64_rom_handling().
*/
{
  int c = 0, result = 0, x = 0, opts = 0;

  // these members of ucon64 can change per file
  ucon64.dat = NULL;
#ifdef  USE_DISCMAGE
  ucon64.image = NULL;
#endif
  ucon64.rominfo = NULL;

  ucon64.split = UCON64_UNKNOWN;

  ucon64.file_size =
  ucon64.crc32 =
  ucon64.fcrc32 = 0;

  for (x = 0; arg[x].val; x++)
    if (!(arg[x].flags & WF_SWITCH))
      {
        if (ucon64.console == UCON64_UNKNOWN)
          ucon64.console = arg[x].console;
        ucon64.flags = arg[x].flags;
        ucon64.option = arg[x].val;
        ucon64.optarg = arg[x].optarg;

        opts++;

        // WF_NO_SPLIT, WF_INIT, WF_PROBE, CRC32, DATabase and WF_NFO
        result = ucon64_rom_handling ();

        if (result == -1) // no rom, but WF_NO_ROM
          return -1;

        if (ucon64_options (&ucon64) == -1)
          {
            const st_getopt2_t *p = getopt2_get_index_by_val (options, c);
            const char *opt = p ? p->name : NULL;

            fprintf (stderr, "ERROR: %s%s encountered a problem\n",
                             opt ? (!opt[1] ? OPTION_S : OPTION_LONG_S) : "",
                             opt ? opt : "uCON64");

//            if (p)
//              getopt2_usage (p);

            fputs ("       Is the option you used available for the current console system?\n"
                   "       Please report bugs to ucon64-main@lists.sf.net or http://ucon64.sf.net\n\n",
                   stderr);

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
  // printing the no_rom error message for files of 0 bytes only confuses people
  else if (!fstate.st_size)
    no_rom = 1;
#endif

  if (no_rom)
    {
      if (!(ucon64.flags & WF_NO_ROM))
        {
          fputs ("ERROR: This option requires a file argument (ROM/image/SRAM file/directory)\n", stderr);
          return -1;
        }
      return 0;
    }

  // The next statement is important and should be executed as soon as
  //  possible (and sensible) in this function
  if ((ucon64.file_size = fsizeof (ucon64.rom)) < 0)
    {
      fprintf (stderr, "ERROR: Could not determine size of %s\n", ucon64.rom);
      return -1;
    }
  // We have to do this here, because we don't know the file size until now
  if (ucon64.buheader_len > ucon64.file_size)
    {
      fprintf (stderr,
               "ERROR: A backup unit header length was specified that is larger than the file\n"
               "       size (%d > %d)\n", ucon64.buheader_len, ucon64.file_size);
      return -1;
    }

  if (!(ucon64.flags & WF_INIT))
    return 0;

  // Try to find the correct console by analysing the ROM
  if (ucon64.flags & WF_PROBE)
    {
      if (ucon64.rominfo)
        {
          // Restore any overrides from st_ucon64_t
          // We have to do this *before* calling ucon64_probe(), *not* afterwards
          if (UCON64_ISSET (ucon64.buheader_len))
            rominfo.buheader_len = ucon64.buheader_len;

          if (UCON64_ISSET (ucon64.interleaved))
            rominfo.interleaved = ucon64.interleaved;

//          ucon64.rominfo = (st_rominfo_t *) &rominfo;
        }
      ucon64.rominfo = ucon64_probe (&rominfo); // determines console type

#ifdef  USE_DISCMAGE
      // check for disc image only if ucon64_probe() failed or --disc was used
      if (ucon64.discmage_enabled)
//        if (!ucon64.rominfo || ucon64.force_disc)
        if (ucon64.force_disc)
          ucon64.image = dm_reopen (ucon64.rom, 0, (dm_image_t *) ucon64.image);
#endif
    }
  // end of WF_PROBE

  // Does the option allow split ROMs?
  if (ucon64.flags & WF_NO_SPLIT)
    /*
      Test for split files only if the console type knows about split files at
      all. However we only know the console type after probing.
    */
    if (ucon64.console == UCON64_NES || ucon64.console == UCON64_SNES ||
        ucon64.console == UCON64_GEN || ucon64.console == UCON64_NG)
      if ((UCON64_ISSET (ucon64.split)) ? ucon64.split : ucon64_testsplit (ucon64.rom))
        {
          fprintf (stderr, "ERROR: %s seems to be split. You have to join it first\n",
                   basename2 (ucon64.rom));
          return -1;
        }


  /*
    CRC32

    Calculating the CRC32 checksum for the ROM data of a UNIF file (NES)
    shouldn't be done with ucon64_fcrc32(). nes_init() uses crc32().
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
    if (!ucon64.force_disc) // NOT for disc images
      if (!(ucon64.flags & WF_NO_CRC32) && ucon64.file_size <= MAXROMSIZE)
        ucon64_chksum (NULL, NULL, &ucon64.crc32, ucon64.rom, ucon64.rominfo ? ucon64.rominfo->buheader_len : 0);


  // DATabase
  ucon64.dat = NULL;
  if (ucon64.crc32 != 0 && ucon64.dat_enabled)
    {
      ucon64.dat = ucon64_dat_search (ucon64.crc32, NULL);
      if (ucon64.dat)
        {
          // detected file size must match DAT file size
          int size = ucon64.rominfo ?
                       UCON64_ISSET (ucon64.rominfo->data_size) ?
                         ucon64.rominfo->data_size :
                         ucon64.file_size - ucon64.rominfo->buheader_len :
                       ucon64.file_size;
          if ((int) (((st_ucon64_dat_t *) ucon64.dat)->fsize) != size)
            ucon64.dat = NULL;
        }

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
            // Use ucon64.dat instead of ucon64.dat_enabled in case the index
            //  file could not be created/opened -> no segmentation fault
            if (ucon64.dat && ucon64.rominfo)
              {
                if (!ucon64.rominfo->name[0])
                  strcpy (ucon64.rominfo->name, NULL_TO_EMPTY (((st_ucon64_dat_t *) ucon64.dat)->name));
                else if (ucon64.console == UCON64_NES)
                  { // override the three-character FDS or FAM name
                    int t = nes_get_file_type ();
                    if (t == FDS || t == FAM)
                      strcpy (ucon64.rominfo->name, NULL_TO_EMPTY (((st_ucon64_dat_t *) ucon64.dat)->name));
                  }

                if (!ucon64.rominfo->country)
                  ucon64.rominfo->country = NULL_TO_EMPTY (((st_ucon64_dat_t *) ucon64.dat)->country);
              }
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
#define AUTO 1

  int x = 0;
  st_probe_t probe[] =
    {
      /*
        The order of the init functions is important. snes_init() must be
        called before nes_init(), but after gameboy_init() and sms_init().
        sms_init() must be called before snes_init(), but after genesis_init().
        There may be more dependencies, so don't change the order unless you
        can verify it won't break anything.
      */
      {UCON64_GBA, gba_init, AUTO},
      {UCON64_N64, n64_init, AUTO},
      {UCON64_GEN, genesis_init, AUTO},
      {UCON64_LYNX, lynx_init, AUTO},
      {UCON64_GB, gameboy_init, AUTO},
      {UCON64_SMS, sms_init, AUTO},
      {UCON64_COLECO, coleco_init, AUTO},
      {UCON64_SNES, snes_init, AUTO},
      {UCON64_NES, nes_init, AUTO},
      {UCON64_NGP, ngp_init, AUTO},
      {UCON64_SWAN, swan_init, AUTO},
      {UCON64_JAG, jaguar_init, AUTO},
      {UCON64_ATA, atari_init, AUTO},
      {UCON64_NDS, nds_init, AUTO},
      {UCON64_VBOY, vboy_init, 0},
      {UCON64_PCE, pcengine_init, 0}, // AUTO still works with non-PCE files
      {UCON64_NG, neogeo_init, 0},
      {UCON64_SWAN, swan_init, 0},
      {UCON64_DC, dc_init, 0},
      {UCON64_PSX, psx_init, 0},
#if 0
      {UCON64_GC, NULL, 0},
      {UCON64_GP32, NULL, 0},
      {UCON64_INTELLI, NULL, 0},
      {UCON64_S16, NULL, 0},
      {UCON64_VEC, NULL, 0},
#endif
      {UCON64_UNKNOWN, unknown_init, 0},
      {0, NULL, 0}
    };

  if (ucon64.console != UCON64_UNKNOWN)         // force recognition option was used
    {
      for (x = 0; probe[x].console != 0; x++)
        if (probe[x].console == ucon64.console)
          {
            ucon64_rom_flush (rominfo);

            probe[x].init (rominfo);

            return rominfo;
          }
    }
  else if (ucon64.file_size <= MAXROMSIZE)      // give auto recognition a try
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
  puts (ucon64.rom);
  if (ucon64.fname_arch[0])
    printf ("  (%s)\n", ucon64.fname_arch);
  fputc ('\n', stdout);
#ifdef  USE_DISCMAGE
  if (ucon64.console == UCON64_UNKNOWN && !ucon64.image)
#else
  if (ucon64.console == UCON64_UNKNOWN)
#endif
    fprintf (stderr, "%s\n", ucon64_msg[CONSOLE_ERROR]);

  if (ucon64.rominfo && ucon64.console != UCON64_UNKNOWN && !ucon64.force_disc)
    ucon64_rom_nfo (ucon64.rominfo);

#ifdef  USE_DISCMAGE
  if (ucon64.discmage_enabled)
    if (ucon64.image)
      {
        dm_nfo ((dm_image_t *) ucon64.image, ucon64.quiet < 0 ? 1 : 0,
#ifdef  USE_ANSI_COLOR
                ucon64.ansi_color ? 1 :
#endif
                                    0);
        fputc ('\n', stdout);

        return 0; // no crc calc. for disc images and therefore no DAT entry either
      }
#endif
  // Use ucon64.fcrc32 for SNES, Genesis & SMS interleaved/N64 non-interleaved
  if (ucon64.fcrc32 && ucon64.crc32)
    printf ("Search checksum (CRC32): 0x%08x\n"
            "Data checksum (CRC32): 0x%08x\n", ucon64.crc32, ucon64.fcrc32);
  else if (ucon64.fcrc32 || ucon64.crc32)
    printf ("Checksum (CRC32): 0x%08x\n", ucon64.fcrc32 ? ucon64.fcrc32 : ucon64.crc32);

  // The check for the size of the file is made, so that uCON64 won't display a
  //  (nonsense) DAT info line when dumping a ROM (file doesn't exist, so
  //  ucon64.file_size is 0).
  if (ucon64.file_size > 0 && ucon64.dat_enabled)
    if (ucon64.dat)
      ucon64_dat_nfo ((st_ucon64_dat_t *) ucon64.dat, 1);

  fputc ('\n', stdout);

  return 0;
}


static inline char *
to_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = func (*p);

  return s;
}


static inline int
toprint (int c)
{
  if (isprint (c))
    return c;

  // characters that also work with printf()
#ifdef  USE_ANSI_COLOR
  if (c == '\x1b')
    return ucon64.ansi_color ? c : '.';
#endif

  return strchr ("\t\n\r", c) ? c : '.';
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
      dumper (stdout, rominfo->buheader, rominfo->buheader_len, rominfo->buheader_start, DUMPER_HEX);
      fputc ('\n', stdout);
    }
  else
    if (rominfo->buheader_len && ucon64.quiet < 0)
      {
        ucon64_dump (stdout, ucon64.rom, rominfo->buheader_start, rominfo->buheader_len, DUMPER_HEX);
        fputc ('\n', stdout);
      }

  // backup unit type?
  if (rominfo->copier_usage != NULL)
    {
      puts (rominfo->copier_usage);
      fputc ('\n', stdout);
    }

  // ROM header
  if (rominfo->header && rominfo->header_len)
    {
      dumper (stdout, rominfo->header, rominfo->header_len,
        rominfo->header_start + rominfo->buheader_len, DUMPER_HEX);
      fputc ('\n', stdout);
    }

  // console type
  if (rominfo->console_usage != NULL)
    puts (rominfo->console_usage);

  // name, maker, country and size
  strcpy (buf, NULL_TO_EMPTY (rominfo->name));
  x = UCON64_ISSET (rominfo->data_size) ?
    rominfo->data_size :
    ucon64.file_size - rominfo->buheader_len;
  printf ("%s\n%s\n%s\n%d Bytes (%.4f Mb)\n\n",
          // some ROMs have a name with control chars in it -> replace control chars
          to_func (buf, strlen (buf), toprint),
          NULL_TO_EMPTY (rominfo->maker),
          NULL_TO_EMPTY (rominfo->country),
          x,
          TOMBIT_F (x));

  // padded?
  if (!padded)
    puts ("Padded: No");
  else
    printf ("Padded: Maybe, %d Bytes (%.4f Mb)\n", padded, TOMBIT_F (padded));

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
        (rominfo->interleaved > 1 ? "Yes (2)" : "Yes") :
        "No");

  // backup unit header?
  if (rominfo->buheader_len)
    printf ("Backup unit/emulator header: Yes, %d Bytes\n",
      rominfo->buheader_len);
  else
// for NoisyB: <read only mode ON>
    puts ("Backup unit/emulator header: No");   // printing No is handy for SNES ROMs
// for NoisyB: <read only mode OFF>

  // split?
  if (split)
    {
      printf ("Split: Yes, %d part%s\n", split, (split != 1) ? "s" : "");
      // nes.c calculates the correct checksum for split ROMs (=Pasofami
      // format), so there is no need to join the files
      if (ucon64.console != UCON64_NES)
        puts ("NOTE: To get the correct checksum the ROM parts must be joined");
    }

  // miscellaneous info
  if (rominfo->misc[0])
    {
      strcpy (buf, rominfo->misc);
      printf ("%s\n", to_func (buf, strlen (buf), toprint));
    }

  // internal checksums?
  if (rominfo->has_internal_crc)
    {
      char *fstr;

      // the internal checksum of GBA ROMS stores only the checksum of the
      //  internal header
      if (ucon64.console != UCON64_GBA)
        fstr = "Checksum: %%s, 0x%%0%dlx (calculated) %%c= 0x%%0%dlx (internal)\n";
      else
        fstr = "Header checksum: %%s, 0x%%0%dlx (calculated) %%c= 0x%%0%dlx (internal)\n";

      sprintf (buf, fstr,
        rominfo->internal_crc_len * 2, rominfo->internal_crc_len * 2);
#ifdef  USE_ANSI_COLOR
      printf (buf,
        ucon64.ansi_color ?
          ((rominfo->current_internal_crc == rominfo->internal_crc) ?
            "\x1b[01;32mOk\x1b[0m" : "\x1b[01;31mBad\x1b[0m")
          :
          ((rominfo->current_internal_crc == rominfo->internal_crc) ? "Ok" : "Bad"),
        rominfo->current_internal_crc,
        (rominfo->current_internal_crc == rominfo->internal_crc) ? '=' : '!',
        rominfo->internal_crc);
#else
      printf (buf,
        (rominfo->current_internal_crc == rominfo->internal_crc) ? "Ok" : "Bad",
        rominfo->current_internal_crc,
        (rominfo->current_internal_crc == rominfo->internal_crc) ? '=' : '!',
        rominfo->internal_crc);
#endif

      if (rominfo->internal_crc2[0])
        printf ("%s\n", rominfo->internal_crc2);
    }

  fflush (stdout);
}


#ifdef  USE_ZLIB
void
ucon64_fname_arch (const char *fname)
{
  char name[FILENAME_MAX];

  unzFile file = unzOpen (fname);
  unzip_goto_file (file, unzip_current_file_nr);
  unzGetCurrentFileInfo (file, NULL, name, FILENAME_MAX, NULL, 0, NULL, 0);
  unzClose (file);
#if     defined _WIN32 || defined __MSDOS__
  {
    int n, l = strlen (name);
    for (n = 0; n < l; n++)
      if (name[n] == '/')
        name[n] = FILE_SEPARATOR;
  }
#endif
  strncpy (ucon64.fname_arch, basename2 (name), FILENAME_MAX)[FILENAME_MAX - 1] = 0;
}
#endif


void
ucon64_usage (int argc, char *argv[])
{
  int x = 0, y = 0, c = 0, single = 0;
  const char *name_exe = basename2 (argv[0]);
#ifdef  USE_DISCMAGE
  char *name_discmage;
#endif
  (void) argc;                                  // warning remover

#ifdef  USE_ZLIB
  printf ("Usage: %s [OPTION]... [ROM|IMAGE|SRAM|FILE|DIR|ARCHIVE]...\n\n", name_exe);
#else
  printf ("Usage: %s [OPTION]... [ROM|IMAGE|SRAM|FILE|DIR]...\n\n", name_exe);
#endif

  // single usage
  for (x = 0; arg[x].val; x++)
    if (arg[x].console) // IS console
      for (y = 0; option[y]; y++)
        for (c = 0; option[y][c].name || option[y][c].help; c++)
          if (option[y][c].object)
            if (((st_ucon64_obj_t *) option[y][c].object)->console == arg[x].console)
              {
                getopt2_usage (option[y]);
                single = 1;
                break;
              }

  if (!single)
    getopt2_usage (options);

  fputc ('\n', stdout);

  printf ("DATabase: %d known ROMs (DAT files: %s)\n\n",
          ucon64_dat_total_entries (), ucon64.datdir);

#ifdef  USE_DISCMAGE
  name_discmage =
#ifdef  DLOPEN
    ucon64.discmage_path;
#else
#if     defined __MSDOS__
    "discmage.dxe";
#elif   defined __CYGWIN__ || defined _WIN32
    "discmage.dll";
#elif   defined __APPLE__                       // Mac OS X actually
    "libdiscmage.dylib";
#elif   defined __unix__ || defined __BEOS__
    "libdiscmage.so";
#else
    "unknown";
#endif
#endif

  if (!ucon64.discmage_enabled)
    {
      printf (ucon64_msg[NO_LIB], name_discmage);
      fputc ('\n', stdout);
    }
#endif

#ifdef  USE_PARALLEL
  puts ("NOTE: You only need to specify PORT if uCON64 doesn't detect the (right)\n"
        "      parallel port. If that is the case give a hardware address. For example:\n"
        "        ucon64 " OPTION_LONG_S "xswc \"rom.swc\" " OPTION_LONG_S "port=0x378\n"
        "      In order to connect a copier to a PC's parallel port you need a standard\n"
        "      bidirectional parallel cable\n");
#endif

  printf ("TIP: %s " OPTION_LONG_S "help " OPTION_LONG_S "snes (would show only SNES related help)\n", name_exe);

#if     defined __MSDOS__ || defined _WIN32
  printf ("     %s " OPTION_LONG_S "help|more (to see everything in more)\n", name_exe);
#else
  printf ("     %s " OPTION_LONG_S "help|less (to see everything in less)\n", name_exe); // less is more ;-)
#endif

  puts ("     Give the force recognition switch a try if something went wrong\n"
        "\n"
        "Please report problems, fixes or ideas to ucon64-main@lists.sf.net or visit\n"
        "http://ucon64.sourceforge.net\n");
}
