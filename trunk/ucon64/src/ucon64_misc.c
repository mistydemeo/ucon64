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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#if     defined __unix__ || defined __BEOS__ || defined AMIGA || HAVE_UNISTD_H
  #include <unistd.h>                           // ioperm() (libc5)
#endif
#ifdef  PARALLEL
  #ifdef  __FreeBSD__
    #include <machine/sysarch.h>
  #elif   defined __linux__
    #ifdef  __GLIBC__
      #include <sys/io.h>                       // ioperm() (glibc)
    #endif
  #elif   defined __BEOS__ || defined AMIGA
    #include <fcntl.h>
  #endif
#endif // PARALLEL

#include "ucon64.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64_misc.h"

#include "console/snes.h"
#include "console/gb.h"
#include "console/gba.h"
#include "console/n64.h"
#include "console/lynx.h"
#include "console/sms.h"
#include "console/nes.h"
#include "console/genesis.h"
#include "console/pce.h"
#include "console/neogeo.h"
#include "console/ngp.h"
#include "console/swan.h"
#include "console/dc.h"
#include "console/jaguar.h"


/*
  This is a string pool. gcc 2.9x generates something like this itself, but it
  seems gcc 3.x does not. By using a string pool the executable will be
  smaller than without it.
*/
const char *ucon64_msg[] = {
  "ERROR: Please check cables and connection\n"
  "       Turn the backup unit off and on\n"
  "       Split ROMs must be joined first\n"
  "       Use " OPTION_LONG_S "file={3bc, 378, 278, ...} to specify your port\n"
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
  NULL
};

const char *unknown_usage[] =
  {
    "Unknown backup unit/emulator",
    NULL,
    NULL,
    NULL
  };

const char *gc_usage[] =
  {
    "Nintendo Game Cube/Panasonic Gamecube Q\n",
    "2001/2002 Nintendo http://www.nintendo.com\n",
    "  " OPTION_LONG_S "gc     force recognition\n",
    NULL,
    NULL
  };

const char *s16_usage[] =
  {
    "Sega System 16(A/B)/Sega System 18/dual 68000\n"
    "1987/19XX/19XX SEGA http://www.sega.com\n",
    "  " OPTION_LONG_S "s16       force recognition\n",
    NULL,
    NULL
  };

const char *atari_usage[] =
  {
    "Atari VCS 2600(aka Stella)/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr\n",
    "1977/1982/1984/1986 Atari\n",
    "  " OPTION_LONG_S "ata         force recognition\n",
    NULL,
    NULL
  };

const char *coleco_usage[] =
  {
    "ColecoVision\n",
    "1982\n",
    "  " OPTION_LONG_S "coleco     force recognition\n",
    NULL,
    NULL
  };

const char *vboy_usage[] =
  {
    "Nintendo Virtual Boy\n",
    "19XX Nintendo http://www.nintendo.com\n",
    "  " OPTION_LONG_S "vboy       force recognition\n",
    NULL,
    NULL
  };

const char *vectrex_usage[] =
  {
    "Vectrex\n",
    "1982\n",
    "  " OPTION_LONG_S "vec         force recognition\n",
    NULL,
    NULL
  };

const char *intelli_usage[] =
  {
    "Intellivision\n",
    "1979 Mattel\n",
    "  " OPTION_LONG_S "intelli             force recognition\n",
    NULL,
    NULL
  };

const char *gp32_usage[] =
  {
    "GP32 Game System\n",
    "2002 Gamepark http://www.gamepark.co.kr\n",
    "  " OPTION_LONG_S "gp32        force recognition\n",
    NULL,
    NULL
  };

const char *ps2_usage[] =
  {
    "Playstation 2\n",
    "2000 Sony http://www.playstation.com\n",
    "  " OPTION_LONG_S "ps2      force recognition\n",
    NULL,
    NULL
  };

const char *xbox_usage[] =
  {
    "XBox\n",
    "2001 Microsoft http://www.xbox.com\n",
    "  " OPTION_LONG_S "xbox        force recognition\n",
    NULL,
    NULL
  };

const char *sat_usage[] =
  {
    "Saturn\n",
    "1994 SEGA http://www.sega.com\n",
    "  " OPTION_LONG_S "sat      force recognition\n",
    NULL,
    NULL
  };

const char *real3do_usage[] =
  {
    "Real3DO\n",
    "1993 Panasonic/Goldstar/Philips\n",
    "  " OPTION_LONG_S "3do        force recognition\n",
    NULL,
    NULL
  };

const char *cd32_usage[] =
  {
    "CD32\n",
    "1993 Commodore\n",
    "  " OPTION_LONG_S "cd32      force recognition\n",
    NULL,
    NULL
  };

const char *cdi_usage[] =
  {
    "CD-i",
    "1991 Philips",
    "  " OPTION_LONG_S "cdi       force recognition\n",
    NULL,
    NULL
  };

const char *vc4000_usage[] =
  {
    "Interton VC4000",
    "~1980",
    NULL,
    NULL,
    NULL
  };

const char *odyssey2_usage[] =
  {
    "G7400+/Odyssey²",
    "1978",
    NULL,
    NULL,
    NULL
  };

const char *channelf_usage[] =
  {
    "FC Channel F",
    "1976",
    NULL,
    NULL,
    NULL
  };

const char *odyssey_usage[] =
  {
    "Odyssey",
    "1972 Ralph Baer (USA)",
    NULL,
    NULL,
    NULL
  };

const char *gamecom_usage[] =
  {
    "Game.com",
    "? Tiger",
    NULL,
    NULL,
    NULL
  };

#if 0
Indrema
Nuon
Saturn
Sega 32X
Sega CD
Philips CDI
Super Nintendo
XE System
Arcadia
Microvision
Adv. Vision
RDI Halcyon
Astrocade
Home Arcade
RCA Studio 2
Telstar
Atari Pong
PONG

gametz.com
gameaxe.com
sys2064.com
logiqx.com
romcenter.com
emuchina.net
#endif


char *ucon64_temp_file = NULL;
int (*ucon64_testsplit_callback) (const char *filename) = 0;


// maker/publisher strings for SNES, GB, GBC and GBA games
const char *nintendo_maker[792] = {
  NULL, "Nintendo", "Rocket Games/Ajinomoto", "Imagineer-Zoom", "Gray Matter",
  "Zamuse", "Falcom", NULL, "Capcom", "Hot B Co.",
  "Jaleco", "Coconuts Japan", "Coconuts Japan/G.X.Media", "Micronet", "Technos",
  "Mebio Software", "Shouei System", "Starfish", NULL, "Mitsui Fudosan/Dentsu",
  NULL, "Warashi Inc.", NULL, NULL, NULL,
  "Game Village", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 0Z
  NULL, NULL, "Infocom", "Electronic Arts Japan", NULL,
  "Cobra Team", "Human/Field", "KOEI", "Hudson Soft", "S.C.P.",
  "Yanoman", NULL, "Tecmo Products", "Japan Glary Business", "Forum/OpenSystem",
  "Virgin Games (Japan)", "SMDE", NULL, NULL, "Daikokudenki",
  NULL, NULL, NULL, NULL, NULL,
  "Creatures Inc.", "TDK Deep Impresion", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 1Z
  "Destination Software/KSS", "Sunsoft/Tokai Engineering",
    "POW (Planning Office Wada)/VR 1 Japan", "Micro World", NULL,
  "San-X", "Enix", "Loriciel/Electro Brain", "Kemco Japan", "Seta",
  "Culture Brain", NULL, "Palsoft", "Visit Co., Ltd.", "Intec",
  "System Sacom", "Poppo", NULL, NULL, "Media Works",
  "NEC InterChannel", "Tam", "Gajin/Jordan", "Smilesoft", NULL,
  NULL, "Mediakite", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 2Z
  "Viacom", "Carrozzeria", "Dynamic", NULL, "Magifact",
  "Hect", "Codemasters", "Taito/GAGA Communications", "Capcom/Laguna",
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
  "Time Warner Interactive", NULL, NULL, "Playmates Interactive", NULL,
  NULL, "Disney Interactive", "Time Warner Interactive/Bitmasters", "Black Pearl", NULL,
  "Advanced Productions", NULL, NULL, "GT Interactive", "RARE",
  "Crave Entertainment",                        // 4Z
  "Absolute Entertainment", "Acclaim", "Activision", "American Sammy", "Take 2/GameTek",
  "Hi Tech", "LJN Ltd.", NULL, "Mattel", NULL,
  "Mindscape/Red Orb Entertainment", "Romstar", "Taxan", "Midway/Tradewest", NULL,
  "American Softworks", "Majesco Sales Inc.", "3DO", NULL, NULL,
  "Williams Entertainment Inc." /*"Hasbro"*/, "NewKidCo", "Telegames",
    "Metro3D/Majesco Sales Inc.", NULL,
  "Vatical Entertainment", "LEGO Media", NULL, "Xicat Interactive", "Cryo Interactive",
  NULL, NULL, "Red Storm Entertainment", "Microids", NULL,
  "Conspiracy/Swing",                           // 5Z
  "Titus", "Virgin Interactive", "Maxis", NULL, "LucasArts Entertainment",
  NULL, NULL, "Ocean", NULL, "Electronic Arts",
  NULL, "Laser Beam", NULL, NULL, "Elite Systems",
  "Electro Brain", "The Learning Company", "BBC", NULL, "Software 2000",
  NULL, "BAM! Entertainment", "Electro Brain" /*"Studio 3"*/, NULL, NULL,
  NULL, "Classified Games", NULL, "TDK Mediactive", NULL,
  "DreamCatcher", "JoWood Produtions", "SEGA", "Wannado Edition",
    "LSP (Light & Shadow Prod.)",
  "ITE Media",                                  // 6Z
  "Infogrames", "Interplay", "JVC (US)", "Parker Brothers", NULL,
  "Sales Curve/Storm/SCI", NULL, NULL, "LucasArts Entertainment/THQ Software", "Accolade",
  "Triffix Entertainment", NULL, "Microprose Software",
    "Universal Interactive/Sierra/Simon & Schuster", NULL,
  "Kemco", "Rage Software", NULL, NULL, NULL,
  "BVM", "Simon & Schuster Interactive", "Asmik Ace Entertainment Inc./AIA",
    "Empire Interactive", NULL,
  NULL, "Jester Interactive", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 7Z
  "Misawa", "Teichiku", "Namco Ltd.", "LOZC", "KOEI",
  NULL, "Tokuma Shoten Intermedia", "Tsukuda Original", "DATAM-Polystar", NULL,
  NULL, "Bulletproof Software", "Vic Tokai Inc.", NULL, "Character Soft",
  "I'Max", "Saurus", NULL, NULL, "General Entertainment",
  NULL, NULL, "I'Max", "Success", NULL,
  "SEGA Japan", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 8Z
  "Takara", "Chun Soft", "Video System/McO'River", "BEC", NULL,
  "Varie", "Yonezawa/S'pal", "Kaneko", NULL, "Victor Interactive Software/Pack in Video",
  "Nichibutsu/Nihon Bussan", "Tecmo", "Imagineer", NULL, NULL,
  "Nova", "Den'Z", "Bottom Up", "Tecmo", "TGL (Technical Group Laboratory)",
  NULL, "Hasbro Japan", NULL, "Marvelous Entertainment", NULL,
  "Keynet Inc.", "Hands-On Entertainment", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // 9Z
  "Telenet", "Hori", NULL, NULL, "Konami",
  "K.Amusement Leasing Co.", "Kawada", "Takara", NULL, "Technos Japan Corp.",
  "JVC (Europe/Japan)/Victor Musical Indutries", NULL, "Toei Animation", "Toho", NULL,
  "Namco", "Media Rings Corporation", "J-Wing", NULL, "Pioneer LDC",
  "KID", "Mediafactory", NULL, NULL, NULL,
  "Infogrames Hudson", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // AZ
  "Acclaim Japan", "ASCII/Nexoft", "Bandai", NULL, "Enix",
  NULL, "HAL Laboratory", "SNK", NULL, "Pony Canyon Hanbai",
  "Culture Brain", "Sunsoft", "Toshiba EMI", "Sony Imagesoft", NULL,
  "Sammy", "Magical", "Visco", NULL, "Compile",
  NULL, "MTO Inc.", NULL, "Sunrise Interactive", NULL,
  "Global A Entertainment", "Fuuki", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // BZ
  "Taito", NULL, "Kemco", "Square", "Tokuma Shoten",
  "Data East", "Tonkin House", NULL, "KOEI", NULL,
  "Konami/Ultra/Palcom", "NTVIC/VAP", "Use Co., Ltd.", "Meldac", "Pony Canyon (J)/FCI (U)",
  "Angel/Sotsu Agency/Sunrise", "Yumedia/Aroma Co., Ltd.", NULL, NULL, "Boss",
  "Axela/Crea-Tech", "Sekaibunka-Sha/Sumire kobo/Marigul Management Inc.",
    "Konami Computer Entertainment Osaka", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // CZ
  "Taito/Disco", "Sofel", "Quest", "Sigma", "Ask Kodansha",
  NULL, "Naxat", "Copya System", "Capcom Co., Ltd.", "Banpresto",
  "TOMY", "Acclaim/LJN Japan", NULL, "NCS", "Human Entertainment",
  "Altron", "Jaleco", "Gaps Inc.", NULL, NULL,
  NULL, NULL, NULL, "Elf", NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // DZ
  "Jaleco", NULL, "Yutaka", "Varie", "T&ESoft",
  "Epoch", NULL, "Athena", "Asmik", "Natsume",
  "King Records", "Atlus", "Epic/Sony Records (J)", NULL,
    "IGS (Information Global Service)",
  NULL, "Chatnoir", "Right Stuff", NULL, NULL,
  NULL, "Spike", "Konami Computer Entertainment Tokyo", "Alphadream Corporation", NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // EZ
  "A Wave", "Motown Software", "Left Field Entertainment", "Extreme Ent. Grp.",
    "TecMagik",
  NULL, NULL, NULL, NULL, "Cybersoft",
  NULL, "Psygnosis", NULL, NULL, "Davidson/Western Tech.",
  "Hudson Soft", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // FZ
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // GZ
  NULL, NULL, NULL, NULL, "Konami",
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, "JVC", NULL, NULL,
  NULL, NULL, "Namco Ltd.", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, "Sachen",
  NULL,                                         // HZ
  NULL, "ASCII", NULL, NULL, "Enix",
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, "Yojigen", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // IZ
  NULL, NULL, NULL, "Square", NULL,
  "Data East", NULL, NULL, "Falcom/KOEI", NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // JZ
  NULL, NULL, "Quest", NULL, NULL,
  NULL, NULL, NULL, NULL, "Banpresto",
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  "NCS", "Human Entertainment", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL,                                         // KZ
  NULL, NULL, NULL, NULL, NULL,
  "Epoch Co., Ltd.", NULL, NULL, NULL, "Natsume",
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL};                                        // LZ


#if     defined PARALLEL && (defined __BEOS__ || defined AMIGA)
typedef struct st_ioport
{
  unsigned int port;
  unsigned char data8;
  unsigned short data16;
} st_ioport_t;

static int ucon64_io_fd;
#endif


void
handle_existing_file (const char *dest, char *src)
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
*/
{
  struct stat src_info, dest_info;

  ucon64_temp_file = NULL;
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
          return;
        }

      // Check if src and dest are the same file based on the inode and device info,
      //  not the filenames
      stat (src, &src_info);
      if (src_info.st_dev == dest_info.st_dev && src_info.st_ino == dest_info.st_ino)
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
    }
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
unsigned int
ucon64_filefile (const char *filename1, int start1, const char *filename2, int start2,
                 int similar)
{
  int base, fsize1, fsize2, len, chunksize1, chunksize2, readok = 1,
      bytesread1, bytesread2, bytesleft1, bytesleft2;
#ifdef  FILEFILE_LARGE_BUF
  int bufsize = 1024 * 1024;
  unsigned char *buf1, *buf2;
#else
  int bufsize = MAXBUFSIZE;
  unsigned char buf1[MAXBUFSIZE], buf2[MAXBUFSIZE];
#endif
  FILE *file1, *file2;

  if (!strcmp (filename1, filename2))
    return 0;
  if (access (filename1, R_OK) != 0 || access (filename2, R_OK) != 0)
    return -1;

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
  return 0;
}


int
ucon64_pad (const char *filename, int start, int size)
/*
  Pad file (if necessary) to start + size bytes;
  Ignore start bytes (if file has header or something)
*/
{
  FILE *file;
  int oldsize = q_fsize (filename) - start, sizeleft;
  unsigned char padbuffer[MAXBUFSIZE];
  struct stat fstate;

  // now we can also "pad" to smaller sizes
  if (oldsize > size)
    {
      stat (filename, &fstate);
      if (chmod (filename, fstate.st_mode | S_IWUSR))
        {
          fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename); // msg is not a typo
          exit (1);
        }
      truncate (filename, size + start);
    }
  else if (oldsize < size)
    {
      // don't use truncate() to enlarge files, because the result is undefined (by POSIX)
      if ((file = fopen (filename, "ab")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
          exit (1);
        }
      sizeleft = size - oldsize;
      memset (padbuffer, 0, MAXBUFSIZE);
      while (sizeleft >= MAXBUFSIZE)
        {
          fwrite (padbuffer, 1, MAXBUFSIZE, file);
          sizeleft -= MAXBUFSIZE;
        }
      if (sizeleft)
        fwrite (padbuffer, 1, sizeleft, file);
      fclose (file);
    }
  return size;
}


#if 1
int
ucon64_testpad (const char *filename, st_rominfo_t *rominfo)
// test if EOF is padded (repeating bytes)
{
  int pos = ucon64.file_size - 1;
  int buf_pos = pos % MAXBUFSIZE;
  int c = q_fgetc (filename, pos);
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

  return ucon64.file_size;                    // the whole file is "padded"
}
#else
int
ucon64_testpad (const char *filename, st_rominfo_t *rominfo)
// test if EOF is padded (repeating bytes)
{
  int size = ucon64.file_size;
  int pos = ucon64.file_size - 2;
  int c = q_fgetc (filename, ucon64.file_size - 1);
  unsigned char *buf;

  if (!(buf = (unsigned char *) malloc ((size + 2) * sizeof (unsigned char))))
    return -1;

  q_fread (buf, 0, size, filename);

  while (c == buf[pos])
    pos--;

  free (buf);

  size -= (pos + 1);
  return size > 1 ? size : 0;
}
#endif


#ifdef  PARALLEL
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
#elif   defined __i386__
  unsigned char byte;

  __asm__ __volatile__
  ("inb %1, %0"
    : "=a" (byte)
    : "d" (port)
  );

  return byte;
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
#elif   defined __i386__
  unsigned short word;

  __asm__ __volatile__
  ("inw %1, %0"
    : "=a" (word)
    : "d" (port)
  );

  return word;
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
#elif   defined __i386__
  __asm__ __volatile__
  ("outb %1, %0"
    :
    : "d" (port), "a" (byte)
  );
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
#elif   defined __i386__
  __asm__ __volatile__
  ("outw %1, %0"
    :
    : "d" (port), "a" (word)
  );
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

  outportb (port, 0xaa);
  for (i = 0; i < DETECT_MAX_CNT; i++)
    if (inportb (port) == 0xaa)
      break;

  if (i < DETECT_MAX_CNT)
    {
      outportb (port, 0x55);
      for (i = 0; i < DETECT_MAX_CNT; i++)
        if (inportb (port) == 0x55)
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
#ifdef  __i386__                                // 0x3bc, 0x378, 0x278
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
      if (i386_set_ioperm (port, 3, 1) == -1)   // data, status & control
#else
      if (ioperm (port, 3, 1) == -1)            // data, status & control
#endif
        {
          fprintf (stderr,
                   "ERROR: Could not set port permissions for I/O ports 0x%x, 0x%x and 0x%x\n"
                   "       (This program needs root privileges for the requested action)\n",
                   port + PARPORT_DATA, port + PARPORT_STATUS, port + PARPORT_CONTROL);
          exit (1);                             // Don't return, if ioperm() fails port access
        }                                       //  causes core dump
#endif // __linux__ || __FreeBSD__

  outportb (port + PARPORT_CONTROL, inportb (port + PARPORT_CONTROL) & 0x0f);
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
      int percentage = (100LL * pos) / size;

      printf ("%u\n", percentage);
      fflush (stdout);
      return 0;
    }
}


int
ucon64_testsplit (const char *filename)
// test if ROM is split into parts based on the name of files
{
  int x, parts = 0;
  char buf[FILENAME_MAX], *p = NULL;

  if (!strchr (filename, '.'))
    return 0;

  for (x = -1; x < 2; x += 2)
    {
      parts = 0;
      strcpy (buf, filename);
      p = strrchr (buf, '.') + x;               // if x == -1 change char before '.'
                                                // else if x == 1 change char behind '.'

      if (buf > p ||                            // filename starts with a period
          p - buf > strlen (buf) - 1)           // filename ends with a period
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


int
ucon64_e (const char *romfile)
{
  int result, x;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[MAXBUFSIZE];
  const char *property;

  x = 0;
  while (options[x].name)
    {
      if (options[x].val == ucon64.console)
      {
        sprintf (buf3, "emulate_%s", options[x].name);
        break;
      }
      x++;
    }

  if (access (ucon64.configfile, F_OK) != 0)
    {
      fprintf (stderr, "ERROR: %s does not exist\n", ucon64.configfile);
      return -1;
    }

  property = get_property (ucon64.configfile, buf3, buf2, NULL);   // buf2 also contains property value
  if (property == NULL)
    {
      fprintf (stderr, "ERROR: Could not find the correct settings (%s) in\n"
              "       %s\n"
              "TIP:   If the wrong console was detected you might try to force recognition\n"
              "       The force recognition option for SNES would be " OPTION_LONG_S "snes\n",
              buf3, ucon64.configfile);
      return -1;
    }

  sprintf (buf, "%s %s", buf2, ucon64.rom);

  printf ("%s\n", buf);
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


static int
ucon64_ls_main (const char *filename, struct stat *fstate, int mode, int console)
{
  int result;
  char buf[MAXBUFSIZE];
  st_rominfo_t rominfo;

//  ucon64.dat_enabled = 0;
  ucon64.console = console;
  ucon64.rom = filename;
  ucon64_flush (&rominfo);
  result = ucon64_init (ucon64.rom, &rominfo);
  ucon64.type = (ucon64.file_size <= MAXROMSIZE) ? UCON64_ROM : UCON64_DISC;

  switch (mode)
    {
    case UCON64_LSV:
      if (!result)
        ucon64_nfo (&rominfo);
      break;

    case UCON64_LSD:
      if (ucon64.crc32)
        {
          printf ("%s\n", ucon64.rom);
          printf ("Checksum (CRC32): 0x%08x\n", ucon64.crc32);
          ucon64_dat_nfo (ucon64_dat);
          printf ("\n");
          ucon64_flush (&rominfo);
        }
      break;

    case UCON64_RROM:
    case UCON64_RR83:
      if (ucon64.console != UCON64_UNKNOWN && !ucon64_testsplit (filename))
        {
          char buf2[FILENAME_MAX];

          if (ucon64.good_enabled)
            strcpy (buf, ucon64_dat ? ucon64_dat->fname : "");
          else
            strcpy (buf, ucon64_dat ? ucon64_dat->fname : rominfo.name);
          
          strcpy (buf2, strtrim (buf));

          if (buf2[0])
            {
              strcpy (buf, to_func (buf2, strlen (buf2), tofname)); // replace chars the fs might not like
              if (mode == UCON64_RR83)
                buf[8] = 0;
              strcat (buf, getext (ucon64.rom));
              if (mode == UCON64_RR83)
                buf[12] = 0;
              if (!strcmp (ucon64.rom, buf))
                {
                  printf ("Found %s\n", ucon64.rom);
                  return 0;
                }
              printf ("Renaming %s to %s\n", ucon64.rom, buf);
              remove (buf);
              rename (ucon64.rom, buf);
            }
        }
      break;

    case UCON64_LS:
    default:
      strftime (buf, 13, "%b %d %H:%M", localtime (&fstate->st_mtime));
      printf ("%-31.31s %10d %s %s\n", to_func (rominfo.name, strlen (rominfo.name), toprint2),
              ucon64.file_size, buf, ucon64.rom);
      fflush (stdout);
      break;
    }
  return 0;
}


int
ucon64_ls (const char *path, int mode)
{
  struct dirent *ep;
  struct stat fstate;
  char dir[FILENAME_MAX], old_dir[FILENAME_MAX];
  DIR *dp;
  int console = ucon64.console;

  dir[0] = 0;

  if (path)
    if (path[0])
      {
        if (!stat (path, &fstate))
          {
            if (S_ISREG (fstate.st_mode))
              {
#ifdef  HAVE_ZLIB_H
                int n = unzip_get_number_entries (path), retval = 0;
                if (n != -1)
                  {
                    for (unzip_current_file_nr = 0; unzip_current_file_nr < n;
                         unzip_current_file_nr++)
                      retval = ucon64_ls_main (path, &fstate, mode, console);
                    unzip_current_file_nr = 0;
                  }
                else
                  retval = ucon64_ls_main (path, &fstate, mode, console);
                return retval;
#else
                return ucon64_ls_main (path, &fstate, mode, console);
#endif
              }
          }
        strcpy (dir, path);
      }

  if (!dir[0])
    getcwd (dir, FILENAME_MAX);

  if ((dp = opendir (dir)) == NULL)
    return -1;

  getcwd (old_dir, FILENAME_MAX);               // remember current dir
  chdir (dir);

  while ((ep = readdir (dp)))
    if (!stat (ep->d_name, &fstate))
      if (S_ISREG (fstate.st_mode))
        {
#ifdef  HAVE_ZLIB_H
          int n = unzip_get_number_entries (ep->d_name);
          if (n != -1)
            {
              for (unzip_current_file_nr = 0; unzip_current_file_nr < n;
                   unzip_current_file_nr++)
                ucon64_ls_main (ep->d_name, &fstate, mode, console);
              unzip_current_file_nr = 0;
            }
          else
#endif
          ucon64_ls_main (ep->d_name, &fstate, mode, console);
        }

  closedir (dp);
  chdir (old_dir);

  return 0;
}


/*
  configfile handling
*/
int
ucon64_configfile (void)
{
  char buf[256], buf2[MAXBUFSIZE], *dirname;

  dirname = getenv2 ("HOME");
  sprintf (ucon64.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
    "ucon64.cfg"
#else
    ".ucon64rc"
#endif
    , dirname);
#ifdef  DJGPP
  // this is DJGPP specific - not necessary, but causes less confusion
  change_string ("/", 1, 0, 0, FILE_SEPARATOR_S, 1, ucon64.configfile,
    strlen (ucon64.configfile), 0);
#endif

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
                 "discmage_path=discmage.dxe\n"
                 "configdir=\n"
#elif   defined __CYGWIN__
                 "discmage_path=discmage.dll\n"
                 "configdir=\n"
#elif   defined __unix__ || defined __BEOS__
                 "discmage_path=~/.ucon64/discmage.so\n"
                 "configdir=~/.ucon64\n"
#endif
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
  else if (strtol (get_property (ucon64.configfile, "version", buf2, "0"), NULL, 10) < UCON64_CONFIG_VERSION)
    {
      strcpy (buf2, ucon64.configfile);
      setext (buf2, ".OLD");

      printf ("NOTE: updating config: old version will be renamed to %s...", buf2);

      q_fcpy (ucon64.configfile, 0, q_fsize (ucon64.configfile), buf2, "wb"); // "wb" is correct for copying

      sprintf (buf, "%d", UCON64_CONFIG_VERSION);
      set_property (ucon64.configfile, "version", buf);

      set_property (ucon64.configfile, "ansi_color", "1");

      set_property (ucon64.configfile, "discmage_path",
#if     defined __MSDOS__
        "discmage.dxe"
#elif   defined __CYGWIN__
        "discmage.dll"
#elif   defined __unix__ || defined __BEOS__
        "~/.ucon64/discmage.so"
#else
        ""
#endif
      );

      set_property (ucon64.configfile, "configdir",
#if     defined __MSDOS__
        ""
#elif   defined __CYGWIN__
        ""
#elif   defined __unix__ || defined __BEOS__
        "~/.ucon64"
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
