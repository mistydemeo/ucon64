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

/*
  enables probing in <console>_init(), it's a good idea to leave this defined
*/
#define CONSOLE_PROBE

enum
{
  ucon64_1991 = 1,
  ucon64_3DO,
  ucon64_A,
  ucon64_ATA,
  ucon64_ATARI,
  ucon64_B,
  ucon64_B0,
  ucon64_B1,
  ucon64_BIOS,
  ucon64_BOT,
  ucon64_C,
  ucon64_CD32,
  ucon64_CDI,
  ucon64_CHK,
  ucon64_COL,
  ucon64_COLECO,
  ucon64_CRC,
  ucon64_CRCHD,
  ucon64_CRP,
  ucon64_CS,
  ucon64_DB,
  ucon64_DBS,
  ucon64_DBV,
  ucon64_DC,
  ucon64_DINT,
  ucon64_E,
  ucon64_F,
  ucon64_FDS,
  ucon64_FDSL,
  ucon64_FFE,
  ucon64_FIG,
  ucon64_FIGS,
  ucon64_FILE, 
  ucon64_FIND,
  ucon64_FRONTEND,
  ucon64_GAMECUBE,
  ucon64_GB,
  ucon64_GBA,
  ucon64_GBX,
  ucon64_GC,
  ucon64_GD3,
  ucon64_GDF,
  ucon64_GEN,
  ucon64_GENESIS,
  ucon64_GETOPT_ERROR = '?',//getopt() returns 0x3f when a unknown option was given
  ucon64_GG,
  ucon64_GGD,
  ucon64_GGE,
  ucon64_GP32,
  ucon64_H,
  ucon64_HD,
  ucon64_HDN,
  ucon64_HELP,
  ucon64_HEX,
  ucon64_HI,
  ucon64_I,
  ucon64_IDPPF,
  ucon64_INES,
  ucon64_INESHD,
  ucon64_INS,
  ucon64_INT,
  ucon64_INT2,
  ucon64_INTELLI,
  ucon64_IP,
  ucon64_ISO,
  ucon64_ISPAD,
  ucon64_J,
  ucon64_JAG,
  ucon64_JAGUAR,
  ucon64_K,
  ucon64_L,
  ucon64_LNX,
  ucon64_LOGO,
  ucon64_LS,
  ucon64_LSV,
  ucon64_LYNX,
  ucon64_LYX,
  ucon64_MGD,
  ucon64_MGH,
  ucon64_MKA,
  ucon64_MKCUE,
  ucon64_MKI,
  ucon64_MKPPF,
  ucon64_MKTOC,
  ucon64_MULTI,
  ucon64_MULTI1,
  ucon64_MULTI2,
  ucon64_MVS,
  ucon64_N,
  ucon64_N2,
  ucon64_N2GB,
  ucon64_N64,
  ucon64_NA,
  ucon64_NBAK,
  ucon64_NEOGEO,
  ucon64_NEOGEOPOCKET,
  ucon64_NES,
  ucon64_NG,
  ucon64_NGP,
  ucon64_NHD,
  ucon64_NHI,
  ucon64_NINT,
  ucon64_NO,
  ucon64_NPPF,
  ucon64_NROT,
  ucon64_NS,
  ucon64_NSWP,
  ucon64_P,
  ucon64_PAD,
  ucon64_PADHD,
  ucon64_PAS,
  ucon64_PCE,
  ucon64_PORT,
  ucon64_PPF,
  ucon64_PS2,
  ucon64_PSX,
  ucon64_REAL3DO,
  ucon64_REN,
  ucon64_RL,
  ucon64_ROM,
  ucon64_ROTL,
  ucon64_ROTR,
  ucon64_RU,
  ucon64_S,
  ucon64_S16,
  ucon64_SAM,
  ucon64_SAT,
  ucon64_SATURN,
  ucon64_SGB,
  ucon64_SMC,
  ucon64_SMD,
  ucon64_SMDS,
  ucon64_SMG,
  ucon64_SMS,
  ucon64_SNES,
  ucon64_SRAM,
  ucon64_SSC,
  ucon64_STP,
  ucon64_STRIP,
  ucon64_SWAN,
  ucon64_SWAP,
  ucon64_SWC,
  ucon64_SWCS,
  ucon64_SWP,
  ucon64_SYSTEM16,
  ucon64_TM,
  ucon64_UFOS,
  ucon64_UNIF,
  ucon64_UNKNOWN,
  ucon64_USMS,
  ucon64_V64,
  ucon64_VBOY,
  ucon64_VEC,
  ucon64_VECTREX,
  ucon64_VERSION,
  ucon64_VIRTUALBOY,
  ucon64_WONDERSWAN,
  ucon64_XBOX,
  ucon64_XCDRW,
  ucon64_XDJR,
  ucon64_XFAL,
  ucon64_XFALB,
  ucon64_XFALC,
  ucon64_XFALS,
  ucon64_XGBX,
  ucon64_XGBXB,
  ucon64_XGBXS,
  ucon64_XSMD,
  ucon64_XSMDS,
  ucon64_XSWC,
  ucon64_XSWCS,
  ucon64_XV64,
  ucon64_YES,
  ucon64_Z64
};

#define UCON64_VERSION_S "1.9.8beta3"

#ifdef  __MSDOS__                               // __MSDOS__ must come before __unix__, because DJGPP defines both
  #define UCON64_OS "MSDOS"
#elif   defined __unix__
  #ifdef  __CYGWIN__
    #define UCON64_OS "Win32"
  #elif   defined __FreeBSD__
    #define UCON64_OS "Unix (FreeBSD)"
  #elif   defined __linux__
    #define UCON64_OS "Unix (Linux)"
  #elif   defined sun
    #define UCON64_OS "Unix (Solaris)"
  #else
    #define UCON64_OS "Unix"
  #endif
#elif   defined __BEOS__
  #define UCON64_OS "BeOS"
#else
  #define UCON64_OS ""
#endif

//extern const char *ucon64_title;

#define MBIT	131072
#define MAXROMSIZE ( ( 512+1 ) * MBIT )
#define MAXBUFSIZE 32768

/*
  this struct holds only workflow relevant informations
*/
typedef struct st_ucon64
{
//TODO get rid of argc and argv here
  int argc;
  char *argv[128];

  char rom[FILENAME_MAX];               //rom (cmdline) with path
  char file[FILENAME_MAX];              //file (cmdline) with path

  unsigned int parport;         //parallel port address
  int parport_mode;             //parallel port mode: ECP, EPP, SPP, other
  char configfile[FILENAME_MAX]; //path and name of the config file

  int backup;			//flag if backups files (*.bak) should be created
  int frontend;			//flag if uCON64 was started by a frontend

  int show_nfo;                 //show or skip info output for ROM
                                //values:
                                //0 show before processing of ROM (default)
                                //1 skip before and after processing of ROM
                                //2 show after processing of ROM
                                //3 show before and after processing of ROM

  long buheader_len;            //length of backup unit header (==0)?no bu head
  int splitted;                 //rom is splitted
  int snes_hirom;               //super nintendo ROM is a HiROM
  int interleaved;              //rom is interleaved (swapped)
} st_ucon64_t;

extern st_ucon64_t ucon64;

/*
  this struct holds only ROM relevant informations
*/
typedef struct st_rom
{
  char rom[FILENAME_MAX];               //rom (cmdline) with path

  long console;                 //integer for the detected console system
  const char *title;                  //console system name
  const char *copier;                 //name of backup unit
  unsigned long bytes;          //size in bytes
  float mbit;                   //size in mbit
  int interleaved;              //rom is interleaved (swapped)
  unsigned long padded;         //rom is padded
  unsigned long intro;          //rom has intro
  int splitted;                 //rom is splitted
  int snes_hirom;               //super nintendo ROM is a HiROM

  unsigned long current_crc32;  //current crc32 value of ROM
  unsigned long db_crc32;       //crc32 value of ROM in internal database

  int has_internal_crc;         //ROM has internal CRC (Super Nintendo, Mega Drive, Gameboy)
  unsigned long current_internal_crc;   //calculated CRC

  unsigned long internal_crc;   //internal CRC
  long internal_crc_start;      //start of internal CRC in ROM header
  int internal_crc_len;         //length (in bytes) of internal CRC in ROM header

  char internal_crc2[4096];     //2nd or inverse internal CRC
  long internal_crc2_start;     //start of 2nd/inverse internal CRC
  int internal_crc2_len;        //length (in bytes) of 2nd/inverse internal CRC

  unsigned char buheader[MAXBUFSIZE];  //(possible) header of backup unit
  long buheader_start;          //start of backup unit header (mostly 0)
  long buheader_len;            //length of backup unit header (==0)?no bu header

  unsigned char header[MAXBUFSIZE];     //(possible) internal ROM header
  long header_start;            //start of internal ROM header
  long header_len;              //length of internal ROM header (==0)?no header

  char name[4096];              //ROM name
  const char *maker;      //maker name of the ROM
  const char *country;           //country name of the ROM
  char misc[MAXBUFSIZE];        //some miscellaneous information about the ROM in one single string
} st_rom_t;

#endif // #ifndef UCON64_H
