/*
console.c - console support for uCON64

Copyright (c) 2006 NoisyB


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
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/itypes.h"
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/file.h"
#include "misc/property.h"
#include "misc/misc.h"
#include "ucon64_misc.h"
#include "backup/backup.h"
#include "console.h"


const st_getopt2_t unknown_console_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Unknown console"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };


int
unknown_console_init (st_ucon64_nfo_t *rominfo)
{
  ucon64.nfo = rominfo;
  ucon64.dat = NULL;

  return 0;
}


const st_getopt2_t gc_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Nintendo Game Cube/Panasonic Gamecube Q"
//      "2001/2002 Nintendo http://www.nintendo.com"
    },
    {
      UCON64_GC_S, 0, 0, UCON64_GC,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t s16_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Sega System 16(A/B)/Sega System 18/dual 68000"
//      "1987/19XX/19XX SEGA http://www.sega.com"
    },
    {
      UCON64_S16_S, 0, 0, UCON64_S16,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t vectrex_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Vectrex"
//      "1982"
    },
    {
      UCON64_VEC_S, 0, 0, UCON64_VEC,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t intelli_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Intellivision"
//      "1979 Mattel"
    },
    {
      UCON64_INTELLI_S, 0, 0, UCON64_INTELLI,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t gp32_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "GP32 Game System/GPX2 - F100"
//      "2001 Gamepark http://www.gamepark.co.kr"
    },
    {
      UCON64_GP32_S, 0, 0, UCON64_GP32,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t ps2_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Playstation 2"
//      "2000 Sony http://www.playstation.com"
    },
    {
      UCON64_PS2_S, 0, 0, UCON64_PS2,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t xbox_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "XBox"
//      "2001 Microsoft http://www.xbox.com"
    },
    {
      UCON64_XBOX_S, 0, 0, UCON64_XBOX,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t sat_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Saturn"
//      "1994 SEGA http://www.sega.com"
    },
    {
      UCON64_SAT_S, 0, 0, UCON64_SAT,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t real3do_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Real3DO"
//      "1993 Panasonic/Goldstar/Philips"
    },
    {
      UCON64_3DO_S, 0, 0, UCON64_3DO,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t cd32_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "CD32"
//      "1993 Commodore"
    },
    {
      UCON64_CD32_S, 0, 0, UCON64_CD32,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t cdi_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "CD-i"
//      "1991 Philips"
    },
    {
      UCON64_CDI_S, 0, 0, UCON64_CDI,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t psx_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Playstation (One)"/*"1994/(2000) Sony http://www.playstation.com"*/
    },
    {
      UCON64_PSX_S, 0, 0, UCON64_PSX,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
};

const st_getopt2_t neogeo_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Neo Geo/Neo Geo CD(Z)/MVS"/*"1990/1994 SNK http://www.neogeo.co.jp"*/
    },
    {
      UCON64_NG_S, 0, 0, UCON64_NG,
      NULL, "force recognition"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t vc4000_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Interton VC4000"
      /*"~1980"*/
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t odyssey2_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "G7400+/Odyssey2"
      /*"1978"*/
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t channelf_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "FC Channel F"
      /*"1976"*/
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t odyssey_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Magnavox Odyssey"
      /*"1972 Ralph Baer (USA)"*/
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t gamecom_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Game.com"
      /*"1997 Tiger Electronics"*/
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };

const st_getopt2_t arcade_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Arcade Machines/M.A.M.E. (Multiple Arcade Machine Emulator)"
    },
    {NULL, 0, 0, 0, NULL, NULL}
  };
#if 0
Nintendo Revolution 2006
XBox 360
PS3
Microvision (Handheld)/1979 MB
Supervision/1991 Hartung
Pokemon Mini/200X Nintendo http://www.nintendo.com
N-Gage/2003 Nokia http://www.n-gage.com
PSP (Playstation Portable)/2005 Sony http://www.playstation.com
Adv. Vision
Arcadia
Astrocade
Indrema
Nuon
RCA Studio 2
RDI Halcyon
Telstar
XE System
#endif


typedef struct
{
  int console;
  int (*demux) (st_ucon64_nfo_t *);
  uint32_t flags;
  int (*open) (st_ucon64_nfo_t *);
} st_console_filter_t;


// auto recognition
#define AUTO 1

st_console_filter_t console_filter[] =
{
#warning split *_init into *_demux and *_open
  {UCON64_GBA, gba_init, AUTO, gba_init},
  {UCON64_N64, n64_init, AUTO, n64_init},
  {UCON64_GEN, genesis_init, AUTO, genesis_init},
  {UCON64_LYNX, lynx_init, AUTO, lynx_init},
  {UCON64_GB, gb_init, AUTO, gb_init},
  {UCON64_SMS, sms_init, AUTO, sms_init},
  {UCON64_COLECO, coleco_init, AUTO, coleco_init},
  {UCON64_SNES, snes_init, AUTO, snes_init},
  {UCON64_NES, nes_init, AUTO, nes_init},
  {UCON64_NGP, ngp_init, AUTO, ngp_init},
  {UCON64_SWAN, swan_init, AUTO, swan_init},
  {UCON64_JAG, jaguar_init, AUTO, jaguar_init},
  {UCON64_ATA, atari_init, AUTO, atari_init},
  {UCON64_NDS, nds_init, AUTO, nds_init},
  {UCON64_VBOY, vboy_init, 0, vboy_init},
  {UCON64_PCE, pce_init, 0, pce_init},
  {UCON64_SWAN, swan_init, 0, swan_init},
  {UCON64_DC, dc_init, 0, dc_init},
  {UCON64_UNKNOWN, unknown_console_init, 0, unknown_console_init},
  {0, NULL, 0}
};


int
ucon64_console_demux (const char *fname)
{
  int x = 0;

  ucon64.fname = fname;

  if (ucon64.console != UCON64_UNKNOWN)         // force recognition option was used
    return ucon64.console;

  for (; console_filter[x].console; x++)
    if (console_filter[x].flags & AUTO)
      {
        st_ucon64_nfo_t nfo;

        if (!console_filter[x].demux (&nfo))
          return console_filter[x].console;
      }

  return UCON64_UNKNOWN;
}


st_ucon64_nfo_t *
ucon64_console_open (const char *fname, int console)
{
  static st_ucon64_nfo_t nfo;
  struct stat fstate;
  int x = 0;

  // a ROM (file)?
  if (!fname)
    return NULL;

  if (!fname[0])
    return NULL;

  if (access (fname, F_OK | R_OK) != 0)
    return NULL;

  if (stat (fname, &fstate) == -1)
    return NULL;

  if (S_ISREG (fstate.st_mode) != TRUE)
    return NULL;

#if 0
  if (!fstate.st_size)
    return NULL;
#endif

  if (fsizeof (fname) < 0)
    return NULL;

  ucon64.fname = fname;

  memset (&nfo, 0, sizeof (st_ucon64_nfo_t));

  nfo.data_size = UCON64_UNKNOWN;

  // Overrides from st_ucon64_t
  if (ucon64.backup_header_len != UCON64_UNKNOWN)
    nfo.backup_header_len = ucon64.backup_header_len;

  if (ucon64.interleaved != UCON64_UNKNOWN)
    nfo.interleaved = ucon64.interleaved;

  if (console == UCON64_UNKNOWN)
    console = ucon64_console_demux (fname);

  for (x = 0; console_filter[x].console; x++)
    if (console_filter[x].console == console)
      if (!console_filter[x].open (&nfo))
        return &nfo;

  return NULL;
}


int
ucon64_console_close (st_ucon64_nfo_t * nfo)
{
  nfo = NULL;

  return 0;
}


int
ucon64_e (st_ucon64_nfo_t *rominfo)
{
  (void) rominfo;
  int result = 0, x = 0;
  char buf[MAXBUFSIZE], name[MAXBUFSIZE];
  const char *value_p = NULL;
  typedef struct
  {
    int id;
    const char *s;
  } st_strings_t;
  st_strings_t s[] = {
    {UCON64_3DO,      "emulate_" UCON64_3DO_S},
    {UCON64_ATA,      "emulate_" UCON64_ATA_S},
    {UCON64_CD32,     "emulate_" UCON64_CD32_S},
    {UCON64_CDI,      "emulate_" UCON64_CDI_S},
    {UCON64_COLECO,   "emulate_" UCON64_COLECO_S},
    {UCON64_DC,       "emulate_" UCON64_DC_S},
    {UCON64_GB,       "emulate_" UCON64_GB_S},
    {UCON64_GBA,      "emulate_" UCON64_GBA_S},
    {UCON64_GC,       "emulate_" UCON64_GC_S},
    {UCON64_GEN,      "emulate_" UCON64_GEN_S},
    {UCON64_GP32,     "emulate_" UCON64_GP32_S},
    {UCON64_INTELLI,  "emulate_" UCON64_INTELLI_S},
    {UCON64_JAG,      "emulate_" UCON64_JAG_S},
    {UCON64_LYNX,     "emulate_" UCON64_LYNX_S},
    {UCON64_ARCADE,   "emulate_" UCON64_ARCADE_S},
    {UCON64_N64,      "emulate_" UCON64_N64_S},
    {UCON64_NDS,      "emulate_" UCON64_NDS_S},
    {UCON64_NES,      "emulate_" UCON64_NES_S},
    {UCON64_NG,       "emulate_" UCON64_NG_S},
    {UCON64_NGP,      "emulate_" UCON64_NGP_S},
    {UCON64_PCE,      "emulate_" UCON64_PCE_S},
    {UCON64_PS2,      "emulate_" UCON64_PS2_S},
    {UCON64_PSX,      "emulate_" UCON64_PSX_S},
    {UCON64_S16,      "emulate_" UCON64_S16_S},
    {UCON64_SAT,      "emulate_" UCON64_SAT_S},
    {UCON64_SMS,      "emulate_" UCON64_SMS_S},
    {UCON64_GAMEGEAR, "emulate_" UCON64_GAMEGEAR_S},
    {UCON64_SNES,     "emulate_" UCON64_SNES_S},
    {UCON64_SWAN,     "emulate_" UCON64_SWAN_S},
    {UCON64_VBOY,     "emulate_" UCON64_VBOY_S},
    {UCON64_VEC,      "emulate_" UCON64_VEC_S},
    {UCON64_XBOX,     "emulate_" UCON64_XBOX_S},
    {0,               NULL}
  };

  if (access (ucon64.configfile, F_OK) != 0)
    {
      fprintf (stderr, "ERROR: %s does not exist\n", ucon64.configfile);
      return -1;
    }

  sprintf (name, "emulate_%08x", ucon64.crc32); // look for emulate_<crc32>
  value_p = get_property (ucon64.configfile, name, PROPERTY_MODE_TEXT);

  if (value_p == NULL)
    {
      sprintf (name, "emulate_0x%08x", ucon64.crc32); // look for emulate_0x<crc32>
      value_p = get_property (ucon64.configfile, name, PROPERTY_MODE_TEXT);
    }

  if (value_p == NULL)
    for (x = 0; s[x].s; x++)
      if (s[x].id == ucon64.console)
        {
          value_p = get_property (ucon64.configfile, s[x].s, PROPERTY_MODE_TEXT);
          break;
        }

  if (value_p == NULL)
    {
      fprintf (stderr, "ERROR: Could not find the correct settings (%s) in\n"
               "       %s\n"
               "TIP:   If the wrong console was detected you might try to force recognition\n"
               "       The force recognition option for SNES would be " OPTION_LONG_S "snes\n",
               name, ucon64.configfile);
      return -1;
    }

  sprintf (buf, "%s \"%s\"", value_p, ucon64.fname);

  puts (buf);
  fflush (stdout);

  result = system (buf)
#if     !(defined __MSDOS__ || defined _WIN32)
           >> 8                                 // the exit code is coded in bits 8-15
#endif                                          //  (does not apply to DJGPP, MinGW & VC++)
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
ucon64_rom_nfo (const st_ucon64_nfo_t *nfo)
{
  unsigned int padded = ucon64_testpad (ucon64.fname);
  unsigned int intro = 0;
  unsigned long size = fsizeof (ucon64.fname);
  int value = 0, split = 0;
  char buf[MAXBUFSIZE];
  const char *maker = "", *country = "";

  if (size - nfo->backup_header_len > MBIT)
    intro = (size - nfo->backup_header_len) % MBIT;

  if (ucon64.split == UCON64_UNKNOWN)
    split = ucon64_testsplit (ucon64.fname, NULL);
  else
    split = ucon64.split;

  // backup unit header
  if (nfo->backup_header && nfo->backup_header_len && nfo->backup_header_len != UNKNOWN_BACKUP_HEADER_LEN)
    {
      dumper (stdout, nfo->backup_header, nfo->backup_header_len, nfo->backup_header_start, 0);
      fputc ('\n', stdout);
    }
  else
    if (nfo->backup_header_len && ucon64.quiet < 0)
      {
        ucon64_dump (stdout, ucon64.fname, nfo->backup_header_start, nfo->backup_header_len, 0);
        fputc ('\n', stdout);
      }

  // backup unit type?
  if (nfo->backup_usage != NULL)
    {
      puts (nfo->backup_usage);
      fputc ('\n', stdout);
    }

  // ROM header
  if (nfo->header && nfo->header_len)
    {
      dumper (stdout, nfo->header, nfo->header_len,
        nfo->header_start + nfo->backup_header_len, 0);
      fputc ('\n', stdout);
    }

  // console type
  if (nfo->console_usage != NULL)
    puts (nfo->console_usage);

  // name, maker, country and size
  if (nfo->name)
    {
      strcpy (buf, nfo->name);
      // some ROMs have a name with control chars in it -> replace control chars
      to_func (buf, MIN (strlen (buf), MAXBUFSIZE), toprint);
    }
  else
    *buf = 0;

  if (nfo->data_size != UCON64_UNKNOWN)
    value = nfo->data_size;
  else
    value = size - nfo->backup_header_len;

  if (nfo->maker)
    maker = nfo->maker;

  if (nfo->country)
    country = nfo->country;

  printf ("%s\n"
          "%s\n"
          "%s\n"
          "%d Bytes (%.4f Mb)\n\n",
          buf, 
          maker,
          country,
          value,
          TOMBIT_F (value));

  // padded?
  if (padded)
    printf ("Padded: Maybe, %d Bytes (%.4f Mb)\n", padded, TOMBIT_F (padded));
  else
    puts ("Padded: No");

  // intro, trainer?
  // nes.c determines itself whether or not there is a trainer
  if (intro && ucon64.console != UCON64_NES)
    printf ("Intro/Trainer: Maybe, %d Bytes\n", intro);

  // interleaved?
  if (nfo->interleaved != UCON64_UNKNOWN)
    // printing this is handy for SNES, N64 & Genesis ROMs, but maybe
    //  nonsense for others
    printf ("Interleaved/Swapped: %s\n",
      nfo->interleaved ?
        (nfo->interleaved > 1 ? "Yes (2)" : "Yes") :
        "No");

  // backup unit header?
  if (nfo->backup_header_len)
    printf ("Backup unit/emulator header: Yes, %d Bytes\n",
      nfo->backup_header_len);
  else
    puts ("Backup unit/emulator header: No");   // printing No is handy for SNES ROMs

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
  if (nfo->misc[0])
    {
      strcpy (buf, nfo->misc);
      printf ("%s\n", to_func (buf, strlen (buf), toprint));
    }

  // internal checksums?
  if (nfo->has_internal_crc)
    {
      char *fstr;

      // the internal checksum of GBA ROMS stores only the checksum of the
      //  internal header
      if (ucon64.console != UCON64_GBA)
        fstr = "Checksum: %%s, 0x%%0%dlx (calculated) %%c= 0x%%0%dlx (internal)\n";
      else
        fstr = "Header checksum: %%s, 0x%%0%dlx (calculated) %%c= 0x%%0%dlx (internal)\n";

      sprintf (buf, fstr,
        nfo->internal_crc_len * 2, nfo->internal_crc_len * 2);
#ifdef  USE_ANSI_COLOR
      printf (buf,
        ucon64.ansi_color ?
          ((nfo->current_internal_crc == nfo->internal_crc) ?
            "\x1b[01;32mOk\x1b[0m" : "\x1b[01;31mBad\x1b[0m")
          :
          ((nfo->current_internal_crc == nfo->internal_crc) ? "Ok" : "Bad"),
        nfo->current_internal_crc,
        (nfo->current_internal_crc == nfo->internal_crc) ? '=' : '!',
        nfo->internal_crc);
#else
      printf (buf,
        (nfo->current_internal_crc == nfo->internal_crc) ? "Ok" : "Bad",
        nfo->current_internal_crc,
        (nfo->current_internal_crc == nfo->internal_crc) ? '=' : '!',
        nfo->internal_crc);
#endif

      if (nfo->internal_crc2[0])
        printf ("%s\n", nfo->internal_crc2);
    }

  fflush (stdout);
}

