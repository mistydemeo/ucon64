/*
ucon64_misc.c - miscellaneous functions for uCON64

written by 1999 - 2003 NoisyB (noisyb@gmx.net)
           2001 - 2004 dbjh
                  2001 Caz
           2002 - 2003 Jan-Erik Karlsson (Amiga)


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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>                             // ioperm() (libc5)
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include "misc.h"
#include "ucon64.h"
#ifdef  USE_DISCMAGE
#include "ucon64_dm.h"
#endif
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
  "ERROR: Communication with backup unit failed\n"
  "TIP:   Check cables and connection\n"
  "       Turn the backup unit off and on\n"
//  "       Split ROMs must be joined first\n" // handled with WF_NO_SPLIT
  "       Use " OPTION_LONG_S "port={3bc, 378, 278, ...} to specify a parallel port address\n"
  "       Set the port to SPP (standard, normal) mode in your BIOS as some backup\n"
  "         units do not support EPP and ECP style parallel ports\n"
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

const st_getopt2_t unknown_usage[] =
  {
    {NULL, 0, 0, 0, NULL, "Unknown backup unit/emulator", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t gc_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Nintendo Game Cube/Panasonic Gamecube Q"
      /*"2001/2002 Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      "gc", 0, 0, UCON64_GC,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_GC)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t s16_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Sega System 16(A/B)/Sega System 18/dual 68000"
      /*"1987/19XX/19XX SEGA http://www.sega.com"*/,
      NULL
    },
    {
      "s16", 0, 0, UCON64_S16,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_S16)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t atari_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Atari VCS 2600(aka Stella)/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr"
      /*"1977/1982/1984/1986 Atari"*/,
      NULL
    },
    {
      "ata", 0, 0, UCON64_ATA,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_ATA)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t coleco_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "ColecoVision"/*"1982"*/,
      NULL
    },
    {
      "coleco", 0, 0, UCON64_COLECO,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_COLECO)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t vboy_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Nintendo Virtual Boy"/*"19XX Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      "vboy", 0, 0, UCON64_VBOY,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_VBOY)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t vectrex_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Vectrex"/*"1982"*/,
      NULL
    },
    {
      "vec", 0, 0, UCON64_VEC,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_VEC)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t intelli_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Intellivision"/*"1979 Mattel"*/,
      NULL
    },
    {
      "intelli", 0, 0, UCON64_INTELLI,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_INTELLI)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t gp32_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "GP32 Game System"/*"2002 Gamepark http://www.gamepark.co.kr"*/,
      NULL
    },
    {
      "gp32", 0, 0, UCON64_GP32,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_GP32)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t ps2_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Playstation 2"/*"2000 Sony http://www.playstation.com"*/,
      NULL
    },
    {
      "ps2", 0, 0, UCON64_PS2,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_PS2)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t xbox_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "XBox"/*"2001 Microsoft http://www.xbox.com"*/,
      NULL
    },
    {
      "xbox", 0, 0, UCON64_XBOX,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_XBOX)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t sat_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Saturn"/*"1994 SEGA http://www.sega.com"*/,
      NULL
    },
    {
      "sat", 0, 0, UCON64_SAT,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_SAT)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t real3do_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Real3DO"/*"1993 Panasonic/Goldstar/Philips"*/,
      NULL
    },
    {
      "3do", 0, 0, UCON64_3DO,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_3DO)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t cd32_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "CD32"/*"1993 Commodore"*/,
      NULL
    },
    {
      "cd32", 0, 0, UCON64_CD32,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_CD32)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t cdi_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "CD-i"/*"1991 Philips"*/,
      NULL
    },
    {
      "cdi", 0, 0, UCON64_CDI,
      NULL, "force recognition",
      (void *) (WF_SWITCH|UCON64_CDI)
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t vc4000_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Interton VC4000"/*"~1980"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t odyssey2_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "G7400+/Odyssey2"/*"1978"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t channelf_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "FC Channel F"/*"1976"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t odyssey_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Magnavox Odyssey"/*"1972 Ralph Baer (USA)"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t gamecom_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Game.com"/*"? Tiger"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t mame_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "M.A.M.E. (Multiple Arcade Machine Emulator)",
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

#if 0
Adv. Vision
Arcadia
Astrocade
Indrema
Microvision
N-Gage 2003 Nokia http://www.n-gage.com
Nuon
RCA Studio 2
RDI Halcyon
Telstar
XE System
#endif


int
unknown_init (st_rominfo_t *rominfo)
// init routine for all consoles missing in console/.
{
  ucon64.rominfo = rominfo;
  ucon64.dat = NULL;
#ifdef  USE_DISCMAGE
  ucon64.image = NULL;
#endif

  return 0;
}


const st_getopt2_t ucon64_options_usage[] = {
  {
    NULL, 0, 0, 0,
    NULL, "Options",
    NULL
  },
  {
    "o", 1, 0, UCON64_O,
    "DIRECTORY", "specify output directory",
    (void *) WF_SWITCH
  },
  {
    "nbak", 0, 0, UCON64_NBAK,
    NULL, "prevents backup files (*.BAK)",
    (void *) WF_SWITCH
  },
#ifdef  USE_ANSI_COLOR
  {
    "ncol", 0, 0, UCON64_NCOL,
    NULL, "disable ANSI colors in output",
    (void *) WF_SWITCH
  },
#endif
#if     defined USE_PARALLEL || defined USE_USB
  {
    "port", 1, 0, UCON64_PORT,
    "PORT", "specify "
#ifdef  USE_USB
      "USB"
#endif
#if     defined USE_PARALLEL && defined USE_USB
      " or "
#endif
#ifdef  USE_PARALLEL
      "parallel"
#endif
      " PORT={"
#ifdef  USE_USB
      "USB0, USB1, "
#endif
#ifdef  USE_PARALLEL
      "3bc, 378, 278, "
#endif
      "...}",
    (void *) WF_SWITCH
  },
#endif  // defined USE_PARALLEL || defined USE_USB

  {
    "hdn", 1, 0, UCON64_HDN,
    "N", "force ROM has backup unit/emulator header with size of N Bytes",
    (void *) WF_SWITCH
  },
  {
    "hd", 0, 0, UCON64_HD,
    NULL, "same as " OPTION_LONG_S "hdn=512\n"
    "most backup units use a header with a size of 512 Bytes",
    (void *) WF_SWITCH
  },
  {
    "nhd", 0, 0, UCON64_NHD,
    NULL, "force ROM has no backup unit/emulator header",
    (void *) WF_SWITCH
  },
  {
    "ns", 0, 0, UCON64_NS,
    NULL, "force ROM is not split",
    (void *) WF_SWITCH
  },
  {
    "e", 0, 0, UCON64_E,
#ifdef  __MSDOS__
    NULL, "emulate/run ROM (check ucon64.cfg for more)",
#else
    NULL, "emulate/run ROM (check .ucon64rc for more)",
#endif
    (void *) WF_DEFAULT
  },
  {
    "crc", 0, 0, UCON64_CRC,
    NULL, "show CRC32 value of ROM",
#if 0
    "; this will also force calculation for\n"
    "files bigger than %d Bytes (%.4f Mb)"
#endif
    (void *) (WF_INIT|WF_PROBE|WF_NOCRC32)
  },
  {
    "sha1", 0, 0, UCON64_SHA1,
    NULL, "show SHA1 value of ROM",
    (void *) (WF_INIT|WF_PROBE|WF_NOCRC32)
  },
  {
    "md5", 0, 0, UCON64_MD5,
    NULL, "show MD5 value of ROM",
    (void *) (WF_INIT|WF_PROBE|WF_NOCRC32)
  },
  {
    "ls", 0, 0, UCON64_LS,
    NULL, "generate ROM list for all recognized ROMs",
    (void *) (WF_INIT|WF_PROBE)
  },
  {
    "lsv", 0, 0, UCON64_LSV,
    NULL, "like " OPTION_LONG_S "ls but more verbose",
    (void *) (WF_INIT|WF_PROBE)
  },
  {
    "hex", 0, 0, UCON64_HEX,
#ifdef  __MSDOS__
    NULL, "show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex ...|more\"",
#else
    NULL, "show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex ...|less\"", // less is more ;-)
#endif
    NULL
  },
  {
    "find", 1, 0, UCON64_FIND,
    "STRING", "find STRING in ROM (wildcard: '?')",
    (void *) WF_INIT
  },
  {
    "c", 1, 0, UCON64_C,
    "FILE", "compare FILE with ROM for differences",
    NULL
  },
  {
    "cs", 1, 0, UCON64_CS,
    "FILE", "compare FILE with ROM for similarities",
    NULL
  },
  {
    "help", 0, 0, UCON64_HELP,
    NULL, "display this help and exit",
    (void *) WF_STOP
  },
  {
    "version", 0, 0, UCON64_VER,
    NULL, "output version information and exit",
    (void *) WF_STOP
  },
  {
    "q", 0, 0, UCON64_Q,
    NULL, "be quiet (don't show ROM info)",
    (void *) WF_SWITCH
  },
#if 0
  {
    "qq", 0, 0, UCON64_QQ,
    NULL, "be even more quiet",
    (void *) WF_SWITCH
  },
#endif
  {
    "v", 0, 0, UCON64_V,
    NULL, "be more verbose (show backup unit headers also)",
    (void *) WF_SWITCH
  },
  {NULL, 0, 0, 0, NULL, NULL, NULL}
};


const st_getopt2_t ucon64_options_without_usage[] = {
  {
    "crchd", 0, 0, UCON64_CRCHD,                // backward compat.
    NULL, NULL,
    (void *) (WF_INIT|WF_PROBE|WF_NOCRC32)
  },
  {
    "file", 1, 0, UCON64_FILE,                  // obsolete?
    NULL, NULL,
    (void *) WF_SWITCH
  },
  {
    "frontend", 0, 0, UCON64_FRONTEND,          // no usage?
    NULL, NULL,
    (void *) WF_SWITCH
  },
  {
    "?", 0, 0, UCON64_HELP,                     // same as --help
    NULL, NULL,
    (void *) WF_STOP
  },
  {
    "h", 0, 0, UCON64_HELP,                     // same as --help
    NULL, NULL,
    (void *) WF_STOP
  },
  {
    "id", 0, 0, UCON64_ID,                      // currently only used in snes.c
    NULL, NULL,
    (void *) WF_SWITCH
  },
  {
    "rom", 0, 0, UCON64_ROM,                    // obsolete?
    NULL, NULL,
    (void *) WF_SWITCH
  },
  {
    "83", 0, 0, UCON64_RR83,                    // is now "rr83"
    NULL, NULL,
    (void *) (WF_INIT|WF_PROBE|WF_NO_SPLIT)
  },
#if 0
  {
    "xcdrw", 0, 0, UCON64_XCDRW, // obsolete
    NULL, NULL,
    (void *) (WF_DEFAULT|WF_STOP|WF_NO_ROM)
  },
  {
    "cdmage", 1, 0, UCON64_CDMAGE, // obsolete
    NULL, NULL,
    (void *) WF_DEFAULT
  },
#endif
  // these consoles are (still) not supported
  {
    "3do", 0, 0, UCON64_3DO,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_3DO)
  },
  {
    "gp32", 0, 0, UCON64_GP32,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_GP32)
  },
  {
    "intelli", 0, 0, UCON64_INTELLI,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_INTELLI)
  },
  {
    "ps2", 0, 0, UCON64_PS2,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_PS2)
  },
  {
    "s16", 0, 0, UCON64_S16,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_S16)
  },
  {
    "sat", 0, 0, UCON64_SAT,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_SAT)
  },
  {
    "vboy", 0, 0, UCON64_VBOY,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_VBOY)
  },
  {
    "vec", 0, 0, UCON64_VEC,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_VEC)
  },
  {
    "xbox", 0, 0, UCON64_XBOX,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_XBOX)
  },
  {
    "coleco", 0, 0, UCON64_COLECO,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_COLECO)
  },
  {
    "gc", 0, 0, UCON64_GC,
    NULL, NULL,
    (void *) (WF_SWITCH|UCON64_GC)
  },
  {NULL, 0, 0, 0, NULL, NULL, NULL}
};


const st_getopt2_t ucon64_padding_usage[] = {
  {
    NULL, 0, 0, 0,
    NULL, "Padding",
    NULL
  },
  {
    "ispad", 0, 0, UCON64_ISPAD,
    NULL, "check if ROM is padded",
    (void *) (WF_INIT|WF_NO_SPLIT)
  },
  {
    "pad", 0, 0, UCON64_PAD,
    NULL, "pad ROM to next Mb",
    (void *) WF_DEFAULT
  },
  {
    "p", 0, 0, UCON64_P,
    NULL, "same as " OPTION_LONG_S "pad",
    (void *) WF_DEFAULT
  },
  {
    "padn", 1, 0, UCON64_PADN,
    "N", "pad ROM to N Bytes (put Bytes with value 0x00 after end)",
    (void *) WF_DEFAULT
  },
  {
    "strip", 1, 0, UCON64_STRIP,
    "N", "strip N Bytes from end of ROM",
    NULL
  },
  {
    "stpn", 1, 0, UCON64_STPN,
    "N", "strip N Bytes from start of ROM",
    NULL
  },
  {
    "stp", 0, 0, UCON64_STP,
    NULL, "same as " OPTION_LONG_S "stpn=512\n"
    "most backup units use a header with a size of 512 Bytes",
    NULL
  },
  {
    "insn", 1, 0, UCON64_INSN,
    "N", "insert N Bytes (0x00) before ROM",
    NULL
  },
  {
    "ins", 0, 0, UCON64_INS,
    NULL, "same as " OPTION_LONG_S "insn=512\n"
    "most backup units use a header with a size of 512 Bytes",
    NULL
  },
  {NULL, 0, 0, 0, NULL, NULL, NULL}
};


const st_getopt2_t ucon64_patching_usage[] = {
  {
    NULL, 0, 0, 0,
    NULL, "Patching",
    NULL
  },
  {
    "poke", 1, 0, UCON64_POKE,
    "OFF:V", "change byte at file offset OFF to value V (both in hexadecimal)",
    NULL
  },
  {
    "pattern", 1, 0, UCON64_PATTERN,
    "FILE", "change ROM based on patterns specified in FILE",
    (void *) (WF_INIT|WF_PROBE)
  },
  {
    "patch", 1, 0, UCON64_PATCH,
    "PATCH", "specify the PATCH for the following options\n"
    "use this option or uCON64 expects the last commandline\n"
    "argument to be the name of the PATCH file",
    (void *) WF_SWITCH
  },
  {NULL, 0, 0, 0, NULL, NULL, NULL}
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
      strcpy (fname, basename2 (requested_fname));
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
                      fputc ('\n', stdout);
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
static int
ucon64_configfile_update (void)
{
  char buf[MAXBUFSIZE];
  
  sprintf (buf, "%d", UCON64_CONFIG_VERSION);
  set_property (ucon64.configfile, "version",     buf, "uCON64 configuration");

  return 0;
}


typedef struct
{
  int id;
  const char *emu;
} st_emulate_t;


static int
ucon64_configfile_create (void)
{
  st_emulate_t emulate[] = {
    {UCON64_3DO,      ""},
    {UCON64_ATA,      ""},
    {UCON64_CD32,     ""},
    {UCON64_CDI,      ""},
    {UCON64_COLECO,   ""},
    {UCON64_DC,       ""},
    {UCON64_GB,       "vgb -sound -sync 50 -sgb -scale 2"},
    {UCON64_GBA,      "vgba -scale 2 -uperiod 6"},
    {UCON64_GC,       ""},
    {UCON64_GEN,      "dgen -f -S 2"},
    {UCON64_INTELLI,  ""},
    {UCON64_JAG,      ""},
    {UCON64_LYNX,     ""},
    {UCON64_MAME,     ""},
    {UCON64_N64,      ""},
    {UCON64_NES,      "tuxnes -E2 -rx11 -v -s/dev/dsp -R44100"},
    {UCON64_NG,       ""},
    {UCON64_NGP,      ""},
    {UCON64_PCE,      ""},
    {UCON64_PS2,      ""},
    {UCON64_PSX,      "pcsx"},
    {UCON64_S16,      ""},
    {UCON64_SAT,      ""},
    {UCON64_SMS,      ""},
    {UCON64_GAMEGEAR, ""},
    {UCON64_SNES,     "zsnes"},
    {UCON64_SWAN,     ""},
    {UCON64_VBOY,     ""},
    {UCON64_VEC,      ""},
    {UCON64_XBOX,     ""},
    {0, NULL}
  };
  int x = 0, y = 0;

  ucon64_configfile_update ();

  set_property (ucon64.configfile, "backups",     "1",
    "create backups of files? (1=yes; 0=no)\n"
    "before processing a ROM uCON64 will make a backup of it");

  set_property (ucon64.configfile, "ansi_color",  "1", "use ANSI colors in output? (1=yes; 0=no)");

#ifdef  USE_PPDEV
  set_property (ucon64.configfile, "parport_dev", "/dev/parport0",   "parallel port");
#elif   defined AMIGA
  set_property (ucon64.configfile, "parport_dev", "parallel.device", "parallel port");
  set_property (ucon64.configfile, "parport",     "0", NULL);
#else
  set_property (ucon64.configfile, "parport",     "378",             "parallel port");
#endif

  set_property (ucon64.configfile, "discmage_path",
#if     defined __MSDOS__
    "~\\discmage.dxe",                          // realpath2() expands the tilde
#elif   defined __CYGWIN__
    "~/discmage.dll",
#elif   defined _WIN32
    "~\\discmage.dll",
#elif   defined __APPLE__                       // Mac OS X actually
    "~/.ucon64/discmage.dylib",
#elif   defined __unix__ || defined __BEOS__
    "~/.ucon64/discmage.so",
#else
    "",
#endif
    "complete path to the discmage library for CD image support");

  set_property (ucon64.configfile, "ucon64_configdir",
#if     defined __MSDOS__ || defined __CYGWIN__ || defined _WIN32
    "~",                                        // realpath2() expands the tilde
#elif   defined __unix__ || defined __BEOS__ || defined __APPLE__ // Mac OS X actually
    "~/.ucon64",
#else
    "",
#endif
    "directory with additional config files");

  set_property (ucon64.configfile, "ucon64_datdir",
#if     defined __MSDOS__ || defined __CYGWIN__ || defined _WIN32
    "~",                                        // realpath2() expands the tilde
#elif   defined __unix__ || defined __BEOS__ || defined __APPLE__ // Mac OS X actually
    "~/.ucon64/dat",
#else
    "",
#endif
    "directory with DAT files");

  set_property (ucon64.configfile, "f2afirmware", "f2afirm.hex",  "F2A support files\n"
                                                                  "path to F2A USB firmware");
  set_property (ucon64.configfile, "iclientu",    "iclientu.bin", "path to GBA client binary (for USB code)");
  set_property (ucon64.configfile, "iclientp",    "iclientp.bin", "path to GBA client binary (for parallel port code)");
  set_property (ucon64.configfile, "ilogo",       "ilogo.bin",    "path to iLinker logo file");
  set_property (ucon64.configfile, "gbaloader",   "loader.bin",   "path to GBA multi-game loader");

  for (x = 0; emulate[x].emu; x++)
    for (y = 0; options[y].name; y++)
       if (emulate[x].id == options[y].val)
         {
           char buf[MAXBUFSIZE];

           sprintf (buf, "emulate_%s", options[y].name);

           set_property (ucon64.configfile, buf, emulate[x].emu, !x ?
             "emulate_<console shortcut>=<emulator with options>\n\n"
             "You can also use CRC32 values for ROM specific emulation options:\n\n"
             "emulate_0x<crc32>=<emulator with options>\n"
             "emulate_<crc32>=<emulator with options>": NULL);

           break;
         }

  return 0;
}


int
ucon64_configfile (void)
{
  char buf[MAXBUFSIZE], *dirname;
  int result = -1;

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
      fclose (fh);                              // we'll use set_property() from now

      result = ucon64_configfile_create ();

      if (!result)
        {
          sync ();
          printf ("OK\n\n");
        }
      else
        printf ("FAILED\n\n");
    }
  else if (get_property_int (ucon64.configfile, "version") < UCON64_CONFIG_VERSION)
    {
      strcpy (buf, ucon64.configfile);
      set_suffix (buf, ".OLD");

      printf ("NOTE: Updating config, old version will be renamed to %s...", buf);

      q_fcpy (ucon64.configfile, 0, q_fsize (ucon64.configfile), buf, "wb"); // "wb" is correct for copying

      result = ucon64_configfile_update ();

      if (!result)
        {
          sync ();
          printf ("OK\n\n");
        }
      else
        printf ("FAILED\n\n");
    }

  return result;
}


int
ucon64_rename (int mode)
{
  char buf[FILENAME_MAX + 1], buf2[FILENAME_MAX + 1], suffix[80], *p, *p2;
  int good_name;

  buf[0] = 0;
  strncpy (suffix, get_suffix (ucon64.rom), 80);
  suffix[80 - 1] = 0;                           // in case suffix is >= 80 chars

  switch (mode)
    {
    case UCON64_RROM:
      if (ucon64.rominfo)
        if (ucon64.rominfo->name)
          {
            strcpy (buf, ucon64.rominfo->name);
            strtrim (buf);
          }
      break;

    case UCON64_RENAME:                         // GoodXXXX style rename
      if (ucon64.dat)
        if (ucon64.dat->fname)
          {
            p = (char *) get_suffix (ucon64.dat->fname);
            strcpy (buf, ucon64.dat->fname);

            // get_suffix() never returns NULL
            if (p[0])
              if (strlen (p) < 5)
                if (!(stricmp (p, ".nes") &&    // NES
                      stricmp (p, ".fds") &&    // NES FDS
                      stricmp (p, ".gb") &&     // Game Boy
                      stricmp (p, ".gbc") &&    // Game Boy Color
                      stricmp (p, ".gba") &&    // Game Boy Advance
                      stricmp (p, ".smc") &&    // SNES
                      stricmp (p, ".sc") &&     // Sega Master System
                      stricmp (p, ".sg") &&     // Sega Master System
                      stricmp (p, ".sms") &&    // Sega Master System
                      stricmp (p, ".gg") &&     // Game Gear
//                    stricmp (p, ".smd") &&    // Genesis
                      stricmp (p, ".v64")))     // Nintendo 64
                  buf[strlen (buf) - strlen (p)] = 0;
          }
      break;

    default:
      return 0;                                 // invalid mode
    }

  if (!buf[0])
    return 0;

  if (ucon64.fname_len == UCON64_FORCE63)
    buf[63] = 0;
  else if (ucon64.fname_len == UCON64_RR83)
    buf[8] = 0;

  // replace chars the fs might not like
  strcpy (buf2, to_func (buf, strlen (buf), tofname));
  strcpy (buf, basename2 (ucon64.rom));

  p = (char *) get_suffix (buf);
  // Remove the suffix from buf (ucon64.rom). Note that this isn't fool-proof.
  //  However, this is the best solution, because several DAT files contain
  //  "canonical" file names with a suffix. That is a STUPID bug.
  if (p)
    buf[strlen (buf) - strlen (p)] = 0;

#ifdef  DEBUG
//  printf ("buf: \"%s\"; buf2: \"%s\"\n", buf, buf2);
#endif
  if (!strcmp (buf, buf2))
    // also process files with a correct name, so that -rename can be used to
    //  "weed" out good dumps when -o is used (like GoodXXXX without inplace
    //  command)
    good_name = 1;
  else
    {
      // Another test if the file already has a correct name. This is necessary
      //  for files without a "normal" suffix (e.g. ".smc"). Take for example a
      //  name like "Final Fantasy III (V1.1) (U) [!]".
      strcat (buf, suffix);
      if (!strcmp (buf, buf2))
        {
          good_name = 1;
          suffix[0] = 0;                        // discard "suffix" (part after period)
        }
      else
        good_name = 0;
    }

  // DON'T use set_suffix()! Consider file names (in the DAT file) like
  //  "Final Fantasy III (V1.1) (U) [!]". The suffix is ".1) (U) [!]"...
  strcat (buf2, suffix);

  if (ucon64.fname_len == UCON64_RR83)
    buf2[12] = 0;

  ucon64_output_fname (buf2, OF_FORCE_BASENAME | OF_FORCE_SUFFIX);

  p = basename2 (ucon64.rom);
  p2 = basename2 (buf2);

  if (one_file (ucon64.rom, buf2) && !strcmp (p, p2))
    {                                           // skip only if the letter case
      printf ("Skipping \"%s\"\n", p);          //  also matches (Windows...)
      return 0;
    }

  if (!good_name)
    /*
      Note that the previous statement causes whatever file is present in the
      dir specified with -o (or the current dir) to be overwritten. This seems
      bad, but is actually better than making a backup. It isn't so bad,
      because the file that gets overwritten is either the same as the file it
      is overwritten with or doesn't deserve its name.
      Without this statement repeating a rename action for already renamed
      files would result in a real mess. And I (dbjh) mean a *real* mess...
    */
    if (!access (buf2, F_OK))                   // a file with that name exists already?
      ucon64_file_handler (buf2, NULL, OF_FORCE_BASENAME | OF_FORCE_SUFFIX);

  if (!good_name)
    printf ("Renaming \"%s\" to \"%s\"\n", p, p2);
  else
    printf ("Moving \"%s\"\n", p);
#ifndef DEBUG
  rename2 (ucon64.rom, buf2);                   // rename2() must be used!
#endif
#ifdef  USE_ZLIB
  unzip_current_file_nr = 0x7fffffff - 1;       // dirty hack
#endif
  return 0;
}


int
ucon64_e (void)
{
  int result, x;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[MAXBUFSIZE];
  const char *property;

  if (access (ucon64.configfile, F_OK) != 0)
    {
      fprintf (stderr, "ERROR: %s does not exist\n", ucon64.configfile);
      return -1;
    }

  sprintf (buf3, "emulate_%08x", ucon64.crc32);

  property = get_property (ucon64.configfile, buf3, buf2, NULL); // buf2 also contains property value
  if (property == NULL)
    {
      sprintf (buf3, "emulate_0x%08x", ucon64.crc32);
      property = get_property (ucon64.configfile, buf3, buf2, NULL);
    }

  if (property == NULL)
    {
      for (x = 0; options[x].name; x++)
        if (options[x].val == ucon64.console)
          {
            sprintf (buf3, "emulate_%s", options[x].name);
            break;
          }
      property = get_property (ucon64.configfile, buf3, buf2, NULL);
    }

  if (property == NULL)
    {
      fprintf (stderr, "ERROR: Could not find the correct settings (%s) in\n"
              "       %s\n"
              "TIP:   If the wrong console was detected you might try to force recognition\n"
              "       The force recognition option for SNES would be " OPTION_LONG_S "snes\n",
              buf3, ucon64.configfile);
      return -1;
    }

  sprintf (buf, "%s \"%s\"", buf2, ucon64.rom);

  puts (buf);
  fflush (stdout);
  sync ();

  result = system (buf)
#ifndef __MSDOS__
           >> 8                                 // the exit code is coded in bits 8-15
#endif                                          //  (that is, under non-DOS)
           ;

#if 1
  // Snes9x (Linux) for example returns a non-zero value on a normal exit
  //  (3)...
  // under WinDOS, system() immediately returns with exit code 0 when
  //  starting a Windows executable (as if fork() was called) it also
  //  returns 0 when the exe could not be started
  if (result != 127 && result != -1 && result != 0)        // 127 && -1 are system() errors, rest are exit codes
    {
      fprintf (stderr, "ERROR: The emulator returned an error (?) code: %d\n"
               "TIP:   If the wrong emulator was used you might try to force recognition\n"
               "       The force recognition option for SNES would be " OPTION_LONG_S "snes\n",
               result);
    }
#endif
  return result;
}


#define PATTERN_BUFSIZE (64 * 1024)
/*
  In order for this function to be really useful for general purposes
  change_mem2() should be changed so that it will return detailed status
  information. Since we don't use it for general purposes, this has not a high
  priority. It will be updated as soon as there is a need.
  The thing that currently goes wrong is that offsets that fall outside the
  buffer (either positive or negative) won't result in a change. It will result
  in memory corruption...
*/
int
ucon64_pattern (st_rominfo_t *rominfo, const char *pattern_fname)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX],
       buffer[PATTERN_BUFSIZE];
  FILE *srcfile, *destfile;
  int bytesread = 0, n, n_found = 0, n_patterns, overlap = 0;
  st_cm_pattern_t *patterns = NULL;

  realpath2 (pattern_fname, src_name);
  // First try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    sprintf (src_name, "%s" FILE_SEPARATOR_S "%s", ucon64.configdir, pattern_fname);
  n_patterns = build_cm_patterns (&patterns, src_name, ucon64.quiet == -1 ? 1 : 0);
  if (n_patterns == 0)
    {
      fprintf (stderr, "ERROR: No patterns found in %s\n", src_name);
      cleanup_cm_patterns (&patterns, n_patterns);
      return -1;
    }
  else if (n_patterns < 0)
    {
      char *dir1 = dirname2 (pattern_fname), *dir2 = dirname2 (src_name);

      fprintf (stderr, "ERROR: Could not read from %s, not in %s nor in %s\n",
                       basename2 (pattern_fname), dir1, dir2);
      free (dir1);
      free (dir2);
      // when build_cm_patterns() returns -1, cleanup_cm_patterns() should not be called
      return -1;
    }

  printf ("Found %d pattern%s in %s\n", n_patterns, n_patterns != 1 ? "s" : "", src_name);

  for (n = 0; n < n_patterns; n++)
    {
      if (patterns[n].search_size > overlap)
        {
          overlap = patterns[n].search_size;
          if (overlap > PATTERN_BUFSIZE)
            {
              fprintf (stderr,
                       "ERROR: Pattern %d is too large, specify a shorter pattern\n",
                       n + 1);
              cleanup_cm_patterns (&patterns, n_patterns);
              return -1;
            }
        }

      if ((patterns[n].offset < 0 && patterns[n].offset <= -patterns[n].search_size) ||
           patterns[n].offset > 0)
        printf ("WARNING: The offset of pattern %d falls outside the search pattern.\n"
                "         This can cause problems with the current implementation of --pattern.\n"
                "         Please consider enlarging the search pattern.\n",
                n + 1);
    }
  overlap--;

  puts ("Searching for patterns...");

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }
  if (rominfo->buheader_len)                    // copy header (if present)
    {
      n = rominfo->buheader_len;
      while ((bytesread = fread (buffer, 1, MIN (n, PATTERN_BUFSIZE), srcfile)))
        {
          fwrite (buffer, 1, bytesread, destfile);
          n -= bytesread;
        }
    }

  n = fread (buffer, 1, overlap, srcfile);      // keep bytesread set to 0
  if (n < overlap)                              // DAMN special cases!
    {
      n_found += change_mem2 (buffer, n, patterns[n].search,
                              patterns[n].search_size, patterns[n].wildcard,
                              patterns[n].escape, patterns[n].replace,
                              patterns[n].replace_size, patterns[n].offset,
                              patterns[n].sets);
      fwrite (buffer, 1, n, destfile);
      n = -1;
    }
  else
    do
      {
        if (bytesread)                          // the code also works without this if
          {
            for (n = 0; n < n_patterns; n++)
              {
                int x = 1 - patterns[n].search_size;
                n_found += change_mem2 (buffer + overlap + x,
                                        bytesread + patterns[n].search_size - 1,
                                        patterns[n].search, patterns[n].search_size,
                                        patterns[n].wildcard, patterns[n].escape,
                                        patterns[n].replace, patterns[n].replace_size,
                                        patterns[n].offset, patterns[n].sets);
              }
            fwrite (buffer, 1, bytesread, destfile);
            memmove (buffer, buffer + bytesread, overlap);
          }
      }
    while ((bytesread = fread (buffer + overlap, 1, PATTERN_BUFSIZE - overlap, srcfile)));
  if (n != -1)
    fwrite (buffer, 1, overlap, destfile);

  fclose (srcfile);
  fclose (destfile);
  cleanup_cm_patterns (&patterns, n_patterns);

  printf ("Found %d pattern%s\n", n_found, n_found != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return n_found;
}
#undef PATTERN_BUFSIZE
