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
#ifndef UCON64_H
#define UCON64_H

#include "getopt.h"                      // for struct option


#define UCON64_YES 1
#define UCON64_NO 0

#define UCON64_UNKNOWN_S "Unknown"
#ifdef __GNUC__
#define NULL_TO_UNKNOWN_S(str) ((str)?:UCON64_UNKNOWN_S)
#else
#define NULL_TO_UNKNOWN_S(str) ((str)?(str):UCON64_UNKNOWN_S)
#endif // __GNUC__

//#define UCON64_KNOWN -1
#define UCON64_UNKNOWN -1

#define UCON64_OPTION 1000
#define UCON64_CONSOLE 2000

#define UCON64_GETOPT_ERROR ('?')        //getopt() returns 0x3f when a unknown option was given

#define UCON64_1991 (UCON64_OPTION + 1)
#define UCON64_3DO (UCON64_OPTION + 2)
#define UCON64_A (UCON64_OPTION + 3)
#define UCON64_ATA (UCON64_OPTION + 4)
#define UCON64_B (UCON64_OPTION + 6)
#define UCON64_B0 (UCON64_OPTION + 7)
#define UCON64_B1 (UCON64_OPTION + 8)
#define UCON64_BIOS (UCON64_OPTION + 9)
#define UCON64_BOT (UCON64_OPTION + 10)
#define UCON64_C (UCON64_OPTION + 11)
#define UCON64_CD (UCON64_OPTION + 12)
#define UCON64_CHK (UCON64_OPTION + 14)
#define UCON64_COL (UCON64_OPTION + 15)
#define UCON64_CRC (UCON64_OPTION + 17)
#define UCON64_CRCHD (UCON64_OPTION + 18)
#define UCON64_CRP (UCON64_OPTION + 19)
#define UCON64_CS (UCON64_OPTION + 20)
#define UCON64_DB (UCON64_OPTION + 21)
#define UCON64_DBS (UCON64_OPTION + 22)
#define UCON64_DBV (UCON64_OPTION + 23)
#define UCON64_DINT (UCON64_OPTION + 25)
#define UCON64_E (UCON64_OPTION + 26)
#define UCON64_F (UCON64_OPTION + 27)
#define UCON64_FDS (UCON64_OPTION + 28)
#define UCON64_FDSL (UCON64_OPTION + 29)
#define UCON64_FFE (UCON64_OPTION + 30)
#define UCON64_FIG (UCON64_OPTION + 31)
#define UCON64_FIGS (UCON64_OPTION + 32)
#define UCON64_FILE (UCON64_OPTION + 33)
#define UCON64_FIND (UCON64_OPTION + 34)
#define UCON64_FRONTEND (UCON64_OPTION + 35)
#define UCON64_GBX (UCON64_OPTION + 39)
#define UCON64_GC (UCON64_OPTION + 40)
#define UCON64_GD3 (UCON64_OPTION + 41)
#define UCON64_GDF (UCON64_OPTION + 42)
#define UCON64_GEN (UCON64_OPTION + 43)
#define UCON64_GG (UCON64_OPTION + 45)
#define UCON64_GGD (UCON64_OPTION + 46)
#define UCON64_GGE (UCON64_OPTION + 47)
#define UCON64_GP32 (UCON64_OPTION + 48)
#define UCON64_HD (UCON64_OPTION + 50)
#define UCON64_HDN (UCON64_OPTION + 51)
#define UCON64_HELP (UCON64_OPTION + 52)
#define UCON64_HEX (UCON64_OPTION + 53)
#define UCON64_HI (UCON64_OPTION + 54)
#define UCON64_I (UCON64_OPTION + 55)
#define UCON64_IDPPF (UCON64_OPTION + 56)
#define UCON64_INES (UCON64_OPTION + 57)
#define UCON64_INESHD (UCON64_OPTION + 58)
#define UCON64_INS (UCON64_OPTION + 59)
#define UCON64_INSN (UCON64_OPTION + 37)
#define UCON64_INT (UCON64_OPTION + 60)
#define UCON64_INT2 (UCON64_OPTION + 61)
#define UCON64_IP (UCON64_OPTION + 63)
#define UCON64_ISO (UCON64_OPTION + 64)
#define UCON64_ISPAD (UCON64_OPTION + 65)
#define UCON64_J (UCON64_OPTION + 66)
#define UCON64_JAG (UCON64_OPTION + 67)
#define UCON64_K (UCON64_OPTION + 69)
#define UCON64_L (UCON64_OPTION + 70)
#define UCON64_LNX (UCON64_OPTION + 71)
#define UCON64_LOGO (UCON64_OPTION + 72)
#define UCON64_LS (UCON64_OPTION + 73)
#define UCON64_LSV (UCON64_OPTION + 74)
#define UCON64_LYX (UCON64_OPTION + 76)
#define UCON64_MGD (UCON64_OPTION + 77)
#define UCON64_MGH (UCON64_OPTION + 78)
#define UCON64_MKA (UCON64_OPTION + 79)
#define UCON64_MKCUE (UCON64_OPTION + 80)
#define UCON64_MKI (UCON64_OPTION + 81)
#define UCON64_MKPPF (UCON64_OPTION + 82)
#define UCON64_MKTOC (UCON64_OPTION + 83)
#define UCON64_MULTI (UCON64_OPTION + 84)
#define UCON64_MULTI1 (UCON64_OPTION + 85)
#define UCON64_MULTI2 (UCON64_OPTION + 86)
#define UCON64_MVS (UCON64_OPTION + 87)
#define UCON64_N (UCON64_OPTION + 88)
#define UCON64_N2 (UCON64_OPTION + 89)
#define UCON64_N2GB (UCON64_OPTION + 90)
#define UCON64_NA (UCON64_OPTION + 92)
#define UCON64_NBAK (UCON64_OPTION + 93)
#define UCON64_NG (UCON64_OPTION + 97)
#define UCON64_NGP (UCON64_OPTION + 98)
#define UCON64_NHD (UCON64_OPTION + 99)
#define UCON64_NHI (UCON64_OPTION + 100)
#define UCON64_NINT (UCON64_OPTION + 101)
#define UCON64_NPPF (UCON64_OPTION + 103)
#define UCON64_NROT (UCON64_OPTION + 104)
#define UCON64_NS (UCON64_OPTION + 105)
#define UCON64_NSWP (UCON64_OPTION + 106)
#define UCON64_P (UCON64_OPTION + 107)
#define UCON64_PAD (UCON64_OPTION + 108)
#define UCON64_PADHD (UCON64_OPTION + 109)
#define UCON64_PAS (UCON64_OPTION + 110)
#define UCON64_PASOFAMI (UCON64_OPTION + 111)
#define UCON64_PORT (UCON64_OPTION + 112)
#define UCON64_PPF (UCON64_OPTION + 113)
#define UCON64_RROM (UCON64_OPTION + 116)
#define UCON64_RR83 (UCON64_OPTION + 117)
#define UCON64_RL (UCON64_OPTION + 118)
#define UCON64_ROM (UCON64_OPTION + 119)
#define UCON64_ROTL (UCON64_OPTION + 120)
#define UCON64_ROTR (UCON64_OPTION + 121)
#define UCON64_RU (UCON64_OPTION + 122)
#define UCON64_S (UCON64_OPTION + 123)
#define UCON64_S16 (UCON64_OPTION + 124)
#define UCON64_SAM (UCON64_OPTION + 125)
#define UCON64_SAT (UCON64_OPTION + 126)
#define UCON64_SGB (UCON64_OPTION + 128)
#define UCON64_SMC (UCON64_OPTION + 129)
#define UCON64_SMD (UCON64_OPTION + 130)
#define UCON64_SMDS (UCON64_OPTION + 131)
#define UCON64_SMG (UCON64_OPTION + 132)
#define UCON64_SRAM (UCON64_OPTION + 135)
#define UCON64_SSC (UCON64_OPTION + 136)
#define UCON64_STP (UCON64_OPTION + 137)
#define UCON64_STPN (UCON64_OPTION + 38)
#define UCON64_STRIP (UCON64_OPTION + 138)
#define UCON64_SWAN (UCON64_OPTION + 139)
#define UCON64_SWAP (UCON64_OPTION + 140)
#define UCON64_SWC (UCON64_OPTION + 141)
#define UCON64_SWCS (UCON64_OPTION + 142)
#define UCON64_SWP (UCON64_OPTION + 143)
#define UCON64_TM (UCON64_OPTION + 145)
#define UCON64_UFOS (UCON64_OPTION + 146)
#define UCON64_UNIF (UCON64_OPTION + 147)
#define UCON64_USMS (UCON64_OPTION + 148)
#define UCON64_V64 (UCON64_OPTION + 149)
#define UCON64_VBOY (UCON64_OPTION + 150)
#define UCON64_VEC (UCON64_OPTION + 151)
#define UCON64_VERSION (UCON64_OPTION + 153)
#define UCON64_XCDRW (UCON64_OPTION + 157)
#define UCON64_XDJR (UCON64_OPTION + 158)
#define UCON64_XFAL (UCON64_OPTION + 159)
#define UCON64_XFALB (UCON64_OPTION + 160)
#define UCON64_XFALC (UCON64_OPTION + 161)
#define UCON64_XFALS (UCON64_OPTION + 162)
#define UCON64_XGBX (UCON64_OPTION + 163)
#define UCON64_XGBXB (UCON64_OPTION + 164)
#define UCON64_XGBXS (UCON64_OPTION + 165)
#define UCON64_XMCCL (UCON64_OPTION + 36)
#define UCON64_XSMD (UCON64_OPTION + 166)
#define UCON64_XSMDS (UCON64_OPTION + 167)
#define UCON64_XSWC (UCON64_OPTION + 168)
#define UCON64_XSWCS (UCON64_OPTION + 169)
#define UCON64_XV64 (UCON64_OPTION + 170)
#define UCON64_Z64 (UCON64_OPTION + 171)
#define UCON64_BS (UCON64_OPTION + 172)
#define UCON64_NBS (UCON64_OPTION + 173)
#define UCON64_MULTI3 (UCON64_OPTION + 174)
#define UCON64_TEST (UCON64_OPTION + 175)
#define UCON64_CTRL (UCON64_OPTION + 176)
#define UCON64_NTSC (UCON64_OPTION + 177)
#define UCON64_PAL (UCON64_OPTION + 178)
#define UCON64_BAT (UCON64_OPTION + 179)
#define UCON64_NBAT (UCON64_OPTION + 180)
#define UCON64_VRAM (UCON64_OPTION + 181)
#define UCON64_NVRAM (UCON64_OPTION + 182)
#define UCON64_MIRR (UCON64_OPTION + 183)
#define UCON64_MAPR (UCON64_OPTION + 184)
#define UCON64_DUMPINFO (UCON64_OPTION + 185)

#define UCON64_ATARI UCON64_ATA
#define UCON64_CD32 (UCON64_CONSOLE + 2)
#define UCON64_CDI (UCON64_CONSOLE + 3)
#define UCON64_COLECO (UCON64_CONSOLE + 4)
#define UCON64_DC (UCON64_CONSOLE + 5)
#define UCON64_GAMECUBE UCON64_GC
#define UCON64_GB (UCON64_CONSOLE + 7)
#define UCON64_GBA (UCON64_CONSOLE + 8)
#define UCON64_GENESIS UCON64_GEN
#define UCON64_INTELLI (UCON64_CONSOLE + 10)
#define UCON64_JAGUAR UCON64_JAG
#define UCON64_LYNX (UCON64_CONSOLE + 12)
#define UCON64_N64 (UCON64_CONSOLE + 13)
#define UCON64_NEOGEO UCON64_NG
#define UCON64_NEOGEOPOCKET UCON64_NGP
#define UCON64_NES (UCON64_CONSOLE + 16)
#define UCON64_PCE (UCON64_CONSOLE + 17)
#define UCON64_PS2 (UCON64_CONSOLE + 18)
#define UCON64_PSX (UCON64_CONSOLE + 19)
#define UCON64_REAL3DO UCON64_3DO
#define UCON64_SATURN UCON64_SAT
#define UCON64_SMS (UCON64_CONSOLE + 22)
#define UCON64_SNES (UCON64_CONSOLE + 23)
#define UCON64_SYSTEM16 UCON64_S16
#define UCON64_VECTREX UCON64_VEC
#define UCON64_VIRTUALBOY UCON64_VBOY
#define UCON64_WONDERSWAN UCON64_SWAN
#define UCON64_XBOX (UCON64_CONSOLE + 28)

#define UCON64_VERSION_S "1.9.8beta4"

#define MBIT	131072
#define TOMBIT(x) ((long)(x) / MBIT)
#define TOMBIT_F(x) ((float)(x) / MBIT)

#define MAXROMSIZE ((512 + 1) * MBIT)

/*
  this struct holds only workflow relevant informations
*/
typedef struct st_ucon64
{
#ifdef TODO
//#warning TODO get rid of argc and argv here
#endif // TODO
  int argc;
  char **argv;

  const char *rom;                              // rom (cmdline) with path

//  DIR2_t *dp;                                   // ptr to tempdir with archive contents
  char rom_in_archive[FILENAME_MAX];            // filename holder if the rom comes from an archive
                                                // (const char *)rom will point then to this

  const char *file;                             // file (cmdline) with path

  unsigned int parport;                         // parallel port address
  int parport_mode;                             // parallel port mode: ECP, EPP, SPP, other
  char configfile[FILENAME_MAX];                // path and name of the config file

  int backup;                                   // flag if backups files should be created
  int frontend;                                 // flag if uCON64 was started by a frontend

  int show_nfo;                                 // show or skip info output for ROM

#define UCON64_ISSET(x) (x != UCON64_UNKNOWN)
  long buheader_len;                            // length of backup unit header 0 == no bu hdr
  int split;                                    // rom is split
  int snes_hirom;                               // Super Nintendo ROM is HiROM
  int bs_dump;                                  // SNES "ROM" is a Broadcast Satellaview dump
  int fal_size;                                 // Flash Advance Linker cart size
  int controller;                               // NES UNIF
  int tv_standard;                              // NES UNIF
  int battery;                                  // NES UNIF/iNES/Pasofami
  int vram;                                     // NES UNIF
  int mirror;                                   // NES UNIF/iNES/Pasofami
  const char *mapr;                             // NES UNIF board name or iNES mapper number
  int use_dump_info;                            // NES UNIF
  int interleaved;                              // rom is interleaved (swapped)
  int console;                                  // the detected console system
  int do_not_calc_crc;                          // disable crc calc. to speed up --ls,--lsv, etc.

#define UCON64_TYPE_ISROM(x) (x == UCON64_ROM)
#define UCON64_TYPE_ISCD(x) (x == UCON64_CD)
  int type;                                     // rom type ROM or CD image
} st_ucon64_t;

extern st_ucon64_t ucon64;

typedef struct st_rominfo
{
  const char **console_usage;                   // console system usage
  const char **copier_usage;                    // backup unit usage

  int interleaved;                              // ROM is interleaved (swapped)
  int snes_hirom;                               // Super Nintendo ROM is HiROM
  int data_size;                                // ROM data size without "red tape"

  long buheader_start;                          // start of backup unit header (mostly 0)
  long buheader_len;                            // length of backup unit header 0 == no bu hdr
  const void *buheader;                         // (possible) header of backup unit

  long header_start;                            // start of internal ROM header
  long header_len;                              // length of internal ROM header 0 == no hdr
  const void *header;                           // (possible) internal ROM header

  char name[MAXBUFSIZE];                        // internal ROM name
  const char *maker;                            // maker name of the ROM
  const char *country;                          // country name of the ROM
  char misc[MAXBUFSIZE];                        // some miscellaneous information
                                                //  about the ROM in one single string
  unsigned long current_crc32;                  // current crc32 value of ROM
  unsigned long db_crc32;                       // crc32 value of ROM in internal database

  int has_internal_crc;                         // ROM has internal CRC (SNES, Mega Drive, Gameboy)
  unsigned long current_internal_crc;           // calculated CRC

  unsigned long internal_crc;                   // internal CRC
  long internal_crc_start;                      // start of internal CRC in ROM header
  int internal_crc_len;                         // length (in bytes) of internal CRC in ROM header

  char internal_crc2[MAXBUFSIZE];               // 2nd or inverse internal CRC
  long internal_crc2_start;                     // start of 2nd/inverse internal CRC
  int internal_crc2_len;                        // length (in bytes) of 2nd/inverse internal CRC
} st_rominfo_t;

extern const struct option long_options[];

extern int ucon64_nfo (const st_rominfo_t *);
extern int ucon64_init (const char *romfile, st_rominfo_t *);
extern st_rominfo_t *ucon64_flush (st_rominfo_t *);
extern int ucon64_console_probe (st_rominfo_t *);

#endif // #ifndef UCON64_H
