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
#include "backup/unknown_bu.h"
#include "backup/unknown_bu512.h"
#include "backup/cdrw.h"

static int ucon64_usage (int argc, char *argv[]);
static int ucon64_init (struct ucon64_ *rom);
static int ucon64_nfo (struct ucon64_ *rom);
static int ucon64_flush (int argc, char *argv[], struct ucon64_ *rom);
static void ucon64_exit (void);
static int ucon64_ls (int verbose);
static int ucon64_e (void);
static int ucon64_configfile (void);
static int ucon64_parport_probe (void); 

struct ucon64_ rom;

static struct option long_options[] = {
    {"1991", 0, 0, 168},
    {"3do", 0, 0, 252},
    {"?", 0, 0, 'h'},
    {"a", 0, 0, 'a'},
    {"ata", 0, 0, 233},
    {"b", 0, 0, 'b'},
    {"b0", 0, 0, 169},
    {"b1", 0, 0, 170},
    {"bios", 0, 0, 171},
    {"bot", 0, 0, 172},
    {"c", 0, 0, 'c'},
    {"cd32", 0, 0, 251},
    {"cdi", 0, 0, 250},
    {"chk", 0, 0, 173},
    {"col", 0, 0, 174},
    {"coleco", 0, 0, 235},
    {"crc", 0, 0, 131},
    {"crchd", 0, 0, 133},
    {"crp", 0, 0, 175},
    {"cs", 0, 0, 137},
    {"db", 0, 0, 159},
    {"dbs", 0, 0, 160},
    {"dbv", 0, 0, 161},
    {"dc", 0, 0, 253},
    {"dint", 0, 0, 176},
    {"e", 0, 0, 'e'},
    {"f", 0, 0, 'f'},
//    {"fds", 0, 0, 177},
//    {"fdsl", 0, 0, 178},
    {"ffe", 0, 0, 179},
    {"fig", 0, 0, 180},
    {"figs", 0, 0, 166},
    {"find", 0, 0, 138},
    {"frontend", 0, 0, 130},
    {"gb", 0, 0, 246},
    {"gba", 0, 0, 245},
    {"gbx", 0, 0, 182},
    {"gc", 0, 0, 255},
    {"gd3", 0, 0, 183},
    {"gdf", 0, 0, 184},
    {"gen", 0, 0, 191},
    {"gg", 0, 0, 185},
    {"ggd", 0, 0, 186},
    {"gge", 0, 0, 187},
    {"gp32", 0, 0, 129},
    {"h", 0, 0, 'h'},
    {"help", 0, 0, 'h'},
    {"hex", 0, 0, 136},
    {"i", 0, 0, 'i'},
    {"idppf", 0, 0, 152},
    {"ines", 0, 0, 188},
    {"ineshd", 0, 0, 189},
    {"ins", 0, 0, 145},
    {"intelli", 0, 0, 239},
    {"ip", 0, 0, 190},
    {"iso", 0, 0, 155},
    {"ispad", 0, 0, 142},
    {"j", 0, 0, 'j'},
    {"jag", 0, 0, 181},
    {"k", 0, 0, 'k'},
    {"l", 0, 0, 'l'},
    {"lnx", 0, 0, 192},
    {"logo", 0, 0, 193},
    {"ls", 0, 0, 153},
    {"lsv", 0, 0, 154},
    {"lynx", 0, 0, 240},
    {"lyx", 0, 0, 194},
    {"mgd", 0, 0, 195},
//    {"mgh", 0, 0, 229},
    {"mka", 0, 0, 147},
    {"mkcue", 0, 0, 157},
    {"mki", 0, 0, 146},
    {"mkppf", 0, 0, 150},
    {"mktoc", 0, 0, 156},
    {"multi", 0, 0, 162},
    {"multi1", 0, 0, 163},
    {"multi2", 0, 0, 164},
    {"mvs", 0, 0, 196},
    {"n", 0, 0, 'n'},
    {"n2", 0, 0, 197},
    {"n2gb", 0, 0, 198},
    {"n64", 0, 0, 213},
    {"na", 0, 0, 148},
    {"nbak", 0, 0, 132},
    {"nes", 0, 0, 243},
    {"ng", 0, 0, 211},
    {"ngp", 0, 0, 242},
    {"nppf", 0, 0, 151},
    {"nrot", 0, 0, 199},
    {"ns", 0, 0, 256},
    {"p", 0, 0, 'p'},
    {"pad", 0, 0, 140},
    {"padhd", 0, 0, 141},
    {"pas", 0, 0, 200},
    {"pce", 0, 0, 244},
    {"ppf", 0, 0, 149},
    {"ps2", 0, 0, 249},
    {"psx", 0, 0, 248},
    {"rl", 0, 0, 134},
    {"rotl", 0, 0, 201},
    {"rotr", 0, 0, 202},
    {"ru", 0, 0, 135},
    {"s", 0, 0, 's'},
    {"s16", 0, 0, 234},
    {"sam", 0, 0, 203},
    {"sat", 0, 0, 247},
    {"sgb", 0, 0, 204},
    {"smc", 0, 0, 205},
    {"smd", 0, 0, 206},
    {"smds", 0, 0, 207},
    {"smg", 0, 0, 208},
    {"sms", 0, 0, 241},
    {"snes", 0, 0, 214},
    {"sram", 0, 0, 209},
    {"ssc", 0, 0, 210},
    {"stp", 0, 0, 144},
    {"strip", 0, 0, 143},
    {"swan", 0, 0, 237},
    {"swap", 0, 0, 139},
    {"swc", 0, 0, 212},
    {"swcs", 0, 0, 165},
    {"ufos", 0, 0, 167},
    {"unif", 0, 0, 215},
    {"usms", 0, 0, 216},
    {"v64", 0, 0, 217},
    {"vboy", 0, 0, 236},
    {"vec", 0, 0, 238},
    {"xbox", 0, 0, 254},
#ifdef BACKUP_CD
    {"xcdrw", 0, 0, 158},
#endif // BACKUP_CD
#ifdef BACKUP
    {"xdjr", 0, 0, 218},
    {"xfal", 0, 0, 219},
    {"xfalb", 1, 0, 230},
    {"xfalc", 1, 0, 231},
    {"xfals", 0, 0, 220},
    {"xgbx", 0, 0, 221},
    {"xgbxb", 1, 0, 232},
    {"xgbxs", 0, 0, 222},
    {"xsmd", 0, 0, 223},
    {"xsmds", 0, 0, 224},
    {"xswc", 0, 0, 225},
    {"xswcs", 0, 0, 226},
    {"xv64", 0, 0, 227},
#endif // BACKUP
    {"z64", 0, 0, 228},
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
  int ucon64_argc, skip_init_nfo = 0, c = 0, result = 0;
  unsigned long padded;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], *ucon64_argv[128];
  int option_index = 0;

  printf ("%s\n", ucon64_TITLE);
  printf ("Uses code from various people. See 'developers.html' for more!\n");
  printf ("This may be freely redistributed under the terms of the GNU Public License\n\n");


  ucon64_configfile ();

  rom.backup =     (
      (!strcmp (getProperty (rom.config_file, "backups", buf2, "1"), "1")) ? 1 : 0
    );

  ucon64_parport_probe ();

//TODO do the rom.rom and rom.file checking first! ucon64_e() etc. needs this

  if (argc < 2)
    {
       ucon64_usage (argc, argv);
       return 0;
    }

  memset (&rom, 0, sizeof (struct ucon64_));
  rom.console = ucon64_UNKNOWN;
  ucon64_flush (argc, argv, &rom);

  if (!strlen (rom.rom))//no $ROM? then just use current working dir
    getcwd (rom.rom, sizeof (rom.rom));

  if (!access (rom.rom, F_OK|R_OK))
    {
      ucon64_init (&rom);
    }

  /*
    getopt_long_only()
  */
  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      switch (c)
        {
        case 'h':                //help
          return ucon64_usage (argc, argv);
  
        case 130:                //frontend
          atexit (ucon64_exit);
          rom.frontend = 1;       // used by ucon64_gauge()
          break;
  
        case 131:                //crc
          printf ("Checksum (CRC32): %08lx\n\n", fileCRC32 (rom.rom, 0));
          return 0;
  
        case 132:                //nbak
          rom.backup = 0;
          break;
  
        case 133:                //crchd
          printf ("Checksum (CRC32): %08lx\n\n", fileCRC32 (rom.rom, 512));
          return 0;
  
        case 134:                //rl
          return renlwr (rom.rom);
  
        case 135:                //ru
          return renupr (rom.rom);
  
        case 136:                //hex
          return filehexdump (rom.rom, 0, quickftell (rom.rom));
  
        case 'c':
          if (filefile (rom.rom, 0, rom.file, 0, FALSE) == -1)
            printf ("ERROR: file not found/out of memory\n");
          return 0;
  
        case 137:                //cs
          if (filefile (rom.rom, 0, rom.file, 0, TRUE) == -1)
            printf ("ERROR: file not found/out of memory\n");
          return 0;
  
        case 138:                //find
          x = 0;
          y = quickftell (rom.rom);
          while ((x =
                  filencmp2 (rom.rom, x, y, rom.file, strlen (rom.file),
                             '?')) != -1)
            {
              filehexdump (rom.rom, x, strlen (rom.file));
              x++;
              printf ("\n");
            }
          return 0;
  
        case 139:                //swap
          result = fileswap (ucon64_fbackup (&rom, rom.rom), 0,
                           quickftell (rom.rom));
          printf ("Wrote output to %s\n", rom.rom);
          return result;
  
        case 140:                //pad
          ucon64_fbackup (&rom, rom.rom);
  
          return filepad (rom.rom, 0, MBIT);
  
        case 141:                //padhd
          ucon64_fbackup (&rom, rom.rom);
  
          return filepad (rom.rom, 512, MBIT);
  
        case 142:                //ispad
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
  
        case 143:                //strip
          ucon64_fbackup (&rom, rom.rom);
  
          return truncate (rom.rom, quickftell (rom.rom) - atol (rom.file));
  
        case 144:                //stp
          strcpy (buf, rom.rom);
          newext (buf, ".BAK");
          remove (buf);           // try to remove or rename will fail
          rename (rom.rom, buf);
  
          return filecopy (buf, 512, quickftell (buf), rom.rom, "wb");
  
        case 145:                //ins
          strcpy (buf, rom.rom);
          newext (buf, ".BAK");
          remove (buf);           // try to remove or rename will fail
          rename (rom.rom, buf);
  
          memset (buf2, 0, 512);
          quickfwrite (buf2, 0, 512, rom.rom, "wb");
  
          return filecopy (buf, 0, quickftell (buf), rom.rom, "ab");
  
        case 'b':
          ucon64_fbackup (&rom, rom.rom);
  
          if ((result = bsl (rom.rom, rom.file)) != 0)
            printf ("ERROR: failed\n");
          return result;
  
        case 'i':
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = rom.file;
          ucon64_argv[2] = rom.rom;
          ucon64_argc = 3;
  
          ucon64_fbackup (&rom, rom.rom);
  
          ips_main (ucon64_argc, ucon64_argv);
          break;
  
        case 'a':
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = "-f";
          ucon64_argv[2] = rom.rom;
          ucon64_argv[3] = rom.file;
          ucon64_argc = 4;
  
          ucon64_fbackup (&rom, rom.rom);
  
          return n64aps_main (ucon64_argc, ucon64_argv);
  
        case 146:                //mki
          return cips (rom.rom, rom.file);
  
        case 147:                //mka
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = "-d \"\"";
          ucon64_argv[2] = rom.rom;
          ucon64_argv[3] = rom.file;
          strcpy (buf, rom.rom);
          newext (buf, ".APS");
  
          ucon64_argv[4] = buf;
          ucon64_argc = 5;
  
          return n64caps_main (ucon64_argc, ucon64_argv);
  
        case 148:                //na
          memset (buf2, ' ', 50);
          strncpy (buf2, rom.file, strlen (rom.file));
          return quickfwrite (buf2, 7, 50, ucon64_fbackup (&rom, rom.rom), "r+b");
  
        case 149:                //ppf
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = rom.rom;
          ucon64_argv[2] = rom.file;
          ucon64_argc = 3;
  
          return applyppf_main (ucon64_argc, ucon64_argv);
  
        case 150:                //mkppf
          ucon64_argv[0] = "ucon64";
          ucon64_argv[1] = rom.rom;
          ucon64_argv[2] = rom.file;
  
          strcpy (buf, rom.file);
          newext (buf, ".PPF");
  
          ucon64_argv[3] = buf;
          ucon64_argc = 4;
  
          return makeppf_main (ucon64_argc, ucon64_argv);
  
        case 151:                //nppf
          memset (buf2, ' ', 50);
          strncpy (buf2, rom.file, strlen (rom.file));
          return quickfwrite (buf2, 6, 50, ucon64_fbackup (&rom, rom.rom), "r+b");
  
        case 152:                //idppf
          return addppfid (argc, argv);
  
        case 153:                //ls
          return ucon64_ls (0);
  
        case 154:                //lsv
          return ucon64_ls (1);
  
        case 155:                //iso
          return bin2iso (rom.rom);
  
        case 156:                //mktoc
          return cdrw_mktoc (&rom);
  
        case 157:                //mkcue
          return cdrw_mkcue (&rom);
  
#ifdef BACKUP_CD
        case 158:                //xcdrw
          switch (rom.console)
            {
            case ucon64_DC:      //Dreamcast NOTE: CDI
              return dc_xcdrw (&rom);
  
            default:
              return (!access (rom.rom, F_OK)) ? cdrw_write (&rom) :
                cdrw_read (&rom);
            }
#endif // BACKUP_CD
  
        case 159:                //db
          printf ("Database: %ld known ROMs in ucon64_db.c (%+ld)\n\n",
                  ucon64_dbsize (rom.console),
                  ucon64_dbsize (rom.console) - ucon64_DBSIZE);
  
          printf
            ("TIP: %s -db -nes would show only the number of known NES ROMs\n\n",
             getarg (argc, argv, ucon64_NAME));
          break;
  
        case 160:                //dbs
          ucon64_flush (argc, argv, &rom);
          sscanf (rom.rom, "%lx", &rom.current_crc32);
          ucon64_dbsearch (&rom);
          ucon64_nfo (&rom);
//          ucon64_dbview (rom.console);
          printf ("TIP: %s -dbs -nes would search only for a NES ROM\n\n",
                  getarg (argc, argv, ucon64_NAME));
  
          break;
  
        case 161:                //dbv
          ucon64_dbview (rom.console);
  
          printf ("\nTIP: %s -db -nes would view only NES ROMs\n\n",
                  getarg (argc, argv, ucon64_NAME));
          break;
  
        case 162:                //multi
          rom.console = ucon64_GBA;
          skip_init_nfo = 1;      // This gets rid of nonsense GBA info on a GBA multirom loader binary
          return gbadvance_multi (&rom, 256 * MBIT);
  
        case 163:                //multi1
          rom.console = ucon64_GBA;
          skip_init_nfo = 1;      // This gets rid of nonsense GBA info on a GBA multirom loader binary
          return gbadvance_multi (&rom, 64 * MBIT);
  
        case 164:                //multi2
          rom.console = ucon64_GBA;
          skip_init_nfo = 1;      // This gets rid of nonsense GBA info on a GBA multirom loader binary
          return gbadvance_multi (&rom, 128 * MBIT);
  
        case 165:                //swcs
          rom.console = ucon64_SNES;
          skip_init_nfo = 1;
          return snes_swcs (&rom);
  
        case 166:                //figs
          skip_init_nfo = 1;
          rom.console = ucon64_SNES;
          return snes_figs (&rom);
  
        case 167:                //ufos 
          skip_init_nfo = 1;
          rom.console = ucon64_SNES;
          return snes_ufos (&rom);
  
        case 'e':
          return ucon64_e ();
  
        case 168:                //1991
          return genesis_1991 (&rom);
  
        case 169:                //b0
          return lynx_b0 (&rom);
  
        case 170:                //b1
          return lynx_b1 (&rom);
  
        case 171:                //bios
          return neogeo_bios (&rom);
  
        case 172:                //bot
          return nintendo64_bot (&rom);
  
        case 173:                //chk
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
  
        case 174:                //col
//          rom.console = ucon64_SNES;
          return snes_col (&rom);
  
        case 175:                //crp
          return gbadvance_crp (&rom);
  
        case 176:                //dint
          return snes_dint (&rom);
  
        case 'f':
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
        case 177:                //fds
          return nes_fds (&rom);
*/
/*
        case 178:                //fdsl
          return nes_fdsl (&rom);
*/
        case 179:                //ffe
          return nes_ffe (&rom);
  
        case 180:                //fig
          return snes_fig (&rom);
  
        case 182:                //gbx
          return gameboy_gbx (&rom);
  
        case 183:                //gd3
          return snes_gd3 (&rom);
  
        case 184:                //gdf
          return snes_gdf (&rom);
  
        case 185:                //gg
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
  
        case 186:                //ggd
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
  
        case 187:                //gge
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
  
        case 188:                //ines
          return nes_ines (&rom);
  
        case 189:                //ineshd
          return nes_ineshd (&rom);
  
/*
        case 190:                //ip
          rom.console = ucon64_DC;
//TODO dreamcast ip.bin extract
          break;
*/
  
        case 'j':
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
  
        case 'k':
          return snes_k (&rom);
  
        case 'l':
          return snes_l (&rom);
  
        case 192:                //lnx
          return lynx_lnx (&rom);
  
        case 193:                //logo
          return gbadvance_logo (&rom);
  
        case 194:                //lyx
          return lynx_lyx (&rom);
  
        case 195:                //mgd
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
  
        case 196:                //mvs
          return neogeo_mvs (&rom);
  
        case 'n':
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
  
        case 197:                //n2
          return genesis_n2 (&rom);
  
        case 198:                //n2gb
          rom.console = ucon64_GB;
          return gameboy_n2gb (&rom);
  
        case 199:                //nrot
          return lynx_nrot (&rom);
  
        case 'p':
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
  
        case 200:                //pas
          return nes_pas (&rom);
  
        case 201:                //rotl
          return lynx_rotl (&rom);
  
        case 202:                //rotr
          return lynx_rotr (&rom);
  
        case 's':
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
  
        case 203:                //sam
          rom.console = ucon64_NEOGEO;
          return neogeo_sam (&rom);
  
        case 204:                //sgb
          return gameboy_sgb (&rom);
  
        case 205:                //smc
          return snes_smc (&rom);
  
        case 206:                //smd
          return genesis_smd (&rom);
  
        case 207:                //smds
          return genesis_smds (&rom);
  
        case 208:                //smg
          return pcengine_smg (&rom);
  
        case 209:                //sram
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
          
        case 210:                //ssc
          return gameboy_ssc (&rom);
  
        case 212:                //swc
          return snes_swc (&rom);
  
        case 215:                //unif
          return nes_unif (&rom);
  
        case 216:                //usms
          return nintendo64_usms (&rom);
  
        case 217:                //v64
          return nintendo64_v64 (&rom);
  
#ifdef BACKUP
        case 218:                //xdjr
          rom.console = ucon64_N64;
          return nintendo64_xdjr (&rom);
  
        case 219:                //xfal
          rom.console = ucon64_GBA;
          return gbadvance_xfal (&rom);
  
        case 220:                //xfals
          rom.console = ucon64_GBA;
          return gbadvance_xfals (&rom);
  
        case 221:                //xgbx
          rom.console = ucon64_GB;
          return gameboy_xgbx (&rom);
  
        case 222:                //xgbxs
          rom.console = ucon64_GB;
          return gameboy_xgbxs (&rom);
  
        case 223:                //xsmd
  //        rom.console = ucon64_GENESIS;//could be SMS too
          return genesis_xsmd (&rom);
  
        case 224:                //xsmds
  //        rom.console = ucon64_GENESIS;//could be SMS too
          return genesis_xsmds (&rom);
  
        case 225:                //xswc
          rom.console = ucon64_SNES;
          return snes_xswc (&rom);
  
        case 226:                //xswcs
          rom.console = ucon64_SNES;
          return snes_xswcs (&rom);
  
        case 227:                //xv64
          rom.console = ucon64_N64;
          return nintendo64_xv64 (&rom);
#endif  // BACKUP
  
        case 228:                //z64
          return nintendo64_z64 (&rom);
  
  /*      case 229:                //mgh
          return snes_mgh (&rom);
  */
#ifdef BACKUP
        case 230:                //xfalb
          rom.console = ucon64_GBA;
          return gbadvance_xfalb (&rom);
  
        case 231:                //xfalc
          rom.console = ucon64_GBA;
          return gbadvance_xfal (&rom);
  
        case 232:                //xgbxb
          rom.console = ucon64_GB;
          return gameboy_xgbxb (&rom);
#endif // BACKUP
  
        case 233:                //ata
          rom.console = ucon64_ATARI;
          break;
  
        case 234:                //s16
          rom.console = ucon64_SYSTEM16;
          break;
  
        case 235:                //coleco
          rom.console = ucon64_COLECO;
          break;
  
        case 236:                //vboy
          rom.console = ucon64_VIRTUALBOY;
          break;
  
        case 237:                //swan
          rom.console = ucon64_WONDERSWAN;
          break;
  
        case 238:                //vec
          rom.console = ucon64_VECTREX;
          break;
  
        case 239:                //intelli
          rom.console = ucon64_INTELLI;
          break;
  
        case 240:                //lynx
          rom.console = ucon64_LYNX;
          break;
  
        case 241:                //sms
          rom.console = ucon64_SMS;
          break;
  
        case 242:                //ngp
          rom.console = ucon64_NEOGEOPOCKET;
          break;
  
        case 243:                //nes
          rom.console = ucon64_NES;
          break;
  
        case 244:                //pce
          rom.console = ucon64_PCE;
          break;
  
        case 181:                //jag
          rom.console = ucon64_JAGUAR;
          break;
  
        case 191:                //gen
          rom.console = ucon64_GENESIS;
          break;
  
        case 211:                //ng
          rom.console = ucon64_NEOGEO;
          break;
  
        case 213:                //n64
          rom.console = ucon64_N64;
          break;
  
        case 214:                //snes
          rom.console = ucon64_SNES;
          break;
  
        case 245:                //gba
          rom.console = ucon64_GBA;
          break;
  
        case 246:                //gb
          rom.console = ucon64_GB;
          break;
  
        case 247:                //sat
          rom.console = ucon64_SATURN;
          break;
  
        case 248:                //psx
          rom.console = ucon64_PSX;
          break;
  
        case 249:                //ps2
          rom.console = ucon64_PS2;
          break;
  
        case 250:                //cdi
          rom.console = ucon64_CDI;
          break;
  
        case 251:                //cd32
          rom.console = ucon64_CD32;
          break;
  
        case 252:                //3do
          rom.console = ucon64_REAL3DO;
          break;
  
        case 253:                //dc
          rom.console = ucon64_DC;
          break;
  
        case 254:                //xbox
          rom.console = ucon64_XBOX;
          break;
  
        case 255:                //gc
          rom.console = ucon64_GAMECUBE;
          break;
  
        case 129:                //gp32
          rom.console = ucon64_GP32;
          break;
  
        case 256:                //ns
          rom.splitted[0] = 0;
          break;
  
        default:
          fprintf (STDERR, "Try '%s --help' for more information.\n", argv[0]);
          return -1;
      }
    }


  if (!access (rom.rom, F_OK))
    {
      if(!skip_init_nfo)
      {
//        ucon64_init (&rom);
        if (rom.console != ucon64_UNKNOWN)
          ucon64_nfo (&rom);
      }
    }
  else
    {
      printf (
        "\nERROR: Unknown ROM: %s not found (in internal database)\n"
        "TIP:   If this is a ROM you might try to force the recognition\n"
        "       The force recognition option for Super Nintendo would be -snes\n"
        "       This is also needed for backup units which support more than one\n"
        "       console system\n"
        , rom.rom
        );
      return -1;
    }

  return 0;
}


int
ucon64_init (struct ucon64_ *rom)
{
  rom->bytes = quickftell (rom->rom);

  // call testsplit() before any <CONSOLE>_init() function to provide for a
  // possible override in those functions
  rom->splitted[0] = (argcmp (rom->argc, rom->argv, "-ns")) ? 0 :
      testsplit (rom->rom);

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

        default:
          break;
        }
    }
  else if(rom->bytes <= MAXROMSIZE)
    {
      if ((snes_init (rom) == -1) &&
          (genesis_init (rom) == -1) &&
          (nintendo64_init (rom) == -1) &&
          (gameboy_init (rom) == -1) &&
          (gbadvance_init (rom) == -1) &&
          (nes_init (rom) == -1) &&
          (jaguar_init (rom) == -1)
#ifdef DB
                                &&
  //the following consoles will only be detected if the ROM is present in the DB
          (atari_init (rom) == -1) &&
          (lynx_init (rom) == -1) &&
          (pcengine_init (rom) == -1) &&
          (neogeo_init (rom) == -1) &&
          (neogeopocket_init (rom) == -1) &&
          (sms_init (rom) == -1) &&
          (system16_init (rom) == -1) &&
          (virtualboy_init (rom) == -1) &&
          (vectrex_init (rom) == -1) &&
          (coleco_init (rom) == -1) &&
          (intelli_init (rom) == -1) &&
          (wonderswan_init (rom) == -1) 
#endif // DB
/*
  //detection for the these consoles is not implemented yet
                                      &&
          (gamecube_init (rom) == -1) &&
          (xbox_init (rom) == -1) &&
          (gp32_init (rom) == -1) &&
          (cd32_init (rom) == -1) &&
          (cdi_init (rom) == -1) &&
          (dc_init (rom) == -1) &&
          (ps2_init (rom) == -1) &&
          (psx_init (rom) == -1) &&
          (real3do_init (rom) == -1) &&
          (saturn_init (rom) == -1) &&
          (cd32_init (rom) == -1)
*/
          )
        rom->console = ucon64_UNKNOWN;
      }
    
  quickfread (rom->buheader, rom->buheader_start, rom->buheader_len,
              rom->rom);
//  quickfread(rom->header, rom->header_start, rom->header_len, rom->rom);

//  rom->bytes = quickftell (rom->rom);
  rom->mbit = (rom->bytes - rom->buheader_len) / (float) MBIT;


  /*
    Calculate the CRC32 after rom->buheader_len has been initialized.
    In this way we can call fileCRC32() with the right value for `start'
    so that we disregard the header (if there is one).
  */
  switch (rom->console)
    {
      case ucon64_PS2:
      case ucon64_PSX:
      case ucon64_DC:
      case ucon64_SATURN:
      case ucon64_CDI:
      case ucon64_CD32:
      case ucon64_REAL3DO:
      case ucon64_XBOX:
      case ucon64_GAMECUBE:
//the following stuff is not relevant for potential CD images
        if (rom->bytes > MAXROMSIZE)
          return 0;
      
      default:
        rom->current_crc32 = (rom->bytes <= MAXROMSIZE) ? 
          fileCRC32 (rom->rom, rom->buheader_len) : 0;
    }

  rom->padded = filetestpad (rom->rom);
  rom->intro = ((rom->bytes - rom->buheader_len) > MBIT) ?
    ((rom->bytes - rom->buheader_len) % MBIT) : 0;

  return 0;
}


int
ucon64_usage (int argc, char *argv[])
{
  int c = 0;
  int option_index = 0;
  int single = 0;

  printf ("USAGE: %s [OPTION(S)] ROM [FILE]\n\n"
         /*
           "TODO: $ROM could also be the name of a *.ZIP archive\n"
           "      it will automatically find and extract the ROM\n"
           "\n"
         */
           "  -nbak         prevents backup files (*.bak)\n"
#ifdef	__MSDOS__
           "  -e            emulate/run ROM (see ucon64.cfg for more)\n"
#else
           "  -e            emulate/run ROM (see $HOME/.ucon64rc for more)\n"
#endif
           "  -crc          show CRC32 value of ROM\n"
           "  -crchd        show CRC32 value of ROM (regarding to +512 Bytes header)\n"
           "  -dbs          search ROM database (all entries) by CRC32; $ROM=0xCRC32\n"
           "  -db           ROM database statistics (# of entries)\n"
           "  -dbv          view ROM database (all entries)\n"
           "  -ls           generate ROM list for all ROMs; $ROM=DIRECTORY\n"
           "  -lsv          like -ls but more verbose; $ROM=DIRECTORY\n"
//         "TODO:  -rrom    rename all ROMs in DIRECTORY to their internal names; $ROM=DIR\n"
//         "TODO:  -rr83    like -rrom but with 8.3 filenames; $ROM=DIR\n"
//         "                this is often used by people who loose control of their ROMs\n"
           "  -rl           rename all files in DIRECTORY to lowercase; $ROM=DIRECTORY\n"
           "  -ru           rename all files in DIRECTORY to uppercase; $ROM=DIRECTORY\n"
#ifdef	__MSDOS__
           "  -hex          show ROM as hexdump; use \"ucon64 -hex $ROM|more\"\n"
#else
           "  -hex          show ROM as hexdump; use \"ucon64 -hex $ROM|less\"\n"       // less is better ;-)
#endif
           "  -find         find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)\n"
           "  -c            compare ROMs for differencies; $FILE=OTHER_ROM\n"
           "  -cs           compare ROMs for similarities; $FILE=OTHER_ROM\n"
           "  -swap         swap/(de)interleave ALL Bytes in ROM (1234<->2143)\n"
           "  -ispad        check if ROM is padded\n"
           "  -pad          pad ROM to full Mb\n"
           "  -padhd        pad ROM to full Mb (regarding to +512 Bytes header)\n"
           "  -stp          strip first 512 Bytes (possible header) from ROM\n"
           "  -ins          insert 512 Bytes (0x00) before ROM\n"
           "  -strip        strip Bytes from end of ROM; $FILE=VALUE\n"
           ,getarg (argc, argv, ucon64_NAME));

  bsl_usage (argc, argv);
  ips_usage (argc, argv);
  aps_usage (argc, argv);

  pal4u_usage (argc, argv);
  ppf_usage (argc, argv);
  xps_usage (argc, argv);

  cdrw_usage (argc, argv);

  printf ("\n");

  while ((c = getopt_long_only (argc, argv, "", long_options, &option_index)) != -1)
    {
      single = 1;
      
      switch (c)
        {
      case 245://gba
        gbadvance_usage (argc, argv);
        break;
      case 213://n64
        nintendo64_usage (argc, argv);
        break;
      case 181://jag
        jaguar_usage (argc, argv);
        break;
      case 214://snes
        snes_usage (argc, argv);
        break;
      case 211://ng
        neogeo_usage (argc, argv);
        break;
      case 242://ngp
        neogeopocket_usage (argc, argv);
        break;
      case 191://gen
        genesis_usage (argc, argv);
        break;
      case 246://gb
        gameboy_usage (argc, argv);
        break;
      case 240://lynx
        lynx_usage (argc, argv);
        break;
      case 244://pce
        pcengine_usage (argc, argv);
        break;
      case 241://sms
        sms_usage (argc, argv);
        break;
      case 243://nes
        nes_usage (argc, argv);
        break;
      case 234://s16
        sys16_usage (argc, argv);
        break;
      case 233://ata
        atari_usage (argc, argv);
        break;
      case 235://coleco
        coleco_usage (argc, argv);
        break;
      case 236://vboy
        virtualboy_usage (argc, argv);
        break;
      case 237://swan
        wonderswan_usage (argc, argv);
        break;
      case 238://vec
        vectrex_usage (argc, argv);
        break;
      case 239://intelli
        intelli_usage (argc, argv);
        break;
      case 253://dc
        dc_usage (argc, argv);
        break;
      case 248://psx
        psx_usage (argc, argv);
        break;
      case 249://ps2
        ps2_usage (argc, argv);
        break;
      case 247://sat
        saturn_usage (argc, argv);
        break;
      case 252://3do
        real3do_usage (argc, argv);
        break;
      case 251://cd32
        cd32_usage (argc, argv);
        break;
      case 250://cdi
        cdi_usage (argc, argv);
        break;
      case 255://gc
        gamecube_usage (argc, argv);
        break;
      case 254://xbox
        xbox_usage (argc, argv);
        break;
      case 129://gp32
        gp32_usage (argc, argv);
        break;
  
      default:
        single = 0;
        break;
      }
    }

  if (!single)
    {
      gamecube_usage (argc, argv);
      dc_usage (argc, argv);
      psx_usage (argc, argv);
/*
      ps2_usage(argc,argv);
      sat_usage(argc,argv);
      3do_usage(argc,argv);
      cd32_usage(argc,argv);
      cdi_usage(argc,argv);
*/
      printf ("%s\n%s\n%s\n%s\n%s\n%s\n"
              "  -xbox, -ps2, -sat, -3do, -cd32, -cdi\n"
              "                force recognition; NEEDED\n"
//            "  -iso          force image is ISO9660\n"
//            "  -raw          force image is MODE2_RAW/BIN\n"
              "  *             show info (default); ONLY $ROM=RAW_IMAGE\n"
              "  -iso          convert RAW/BIN to ISO9660; $ROM=RAW_IMAGE\n",
              xbox_TITLE,
              ps2_TITLE, saturn_TITLE, real3do_TITLE, cd32_TITLE,
              cdi_TITLE);

      ppf_usage (argc, argv);
      xps_usage (argc, argv);

      cdrw_usage (argc, argv);

      printf ("\n");

      gbadvance_usage (argc, argv);
      nintendo64_usage (argc, argv);
      snes_usage (argc, argv);
      neogeopocket_usage (argc, argv);
      neogeo_usage (argc, argv);
      genesis_usage (argc, argv);
      gameboy_usage (argc, argv);
      jaguar_usage (argc, argv);
      lynx_usage (argc, argv);
      pcengine_usage (argc, argv);
      sms_usage (argc, argv);
      nes_usage (argc, argv);
      wonderswan_usage (argc, argv);
/*
      sys16_usage (argc, argv);
      atari_usage (argc, argv);
      coleco_usage (argc, argv);
      virtualboy_usage (argc, argv);
      wonderswan_usage (argc, argv);
      vectrex_usage (argc, argv);
      intelli_usage (argc, argv);
*/

      printf ("%s\n%s\n%s\n%s\n%s\n%s\n%s\n"
              "  -s16, -ata, -coleco, -vboy, -vec, -intelli, -gp32\n"
              "                force recognition"
#ifndef DB
              "; NEEDED"
#endif
              "\n"
              "  -hd           force ROM has header (+512 Bytes)\n"
              "  -nhd          force ROM has no header\n"
              "  *             show info (default)\n\n", system16_TITLE,
              atari_TITLE, coleco_TITLE, virtualboy_TITLE,
              vectrex_TITLE, intelli_TITLE, gp32_TITLE);
    }

  printf ("Database: %ld known ROMs in ucon64_db.c (%+ld)\n\n"
     "TIP: %s -help -snes (would show only Super Nintendo related help)\n"
#ifdef	__MSDOS__
     "     %s -help|more (to see everything in more)\n"
#else
     "     %s -help|less (to see everything in less)\n" // less is better ;-)
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

gametz.com
gameaxe.com
sys2064.com
logiqx.com
romcenter.com
emuchina.net
*/
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
        printf ("Backup Unit Header: Yes, %ld Bytes\n"
                /* (use -nhd to override)\n" */ ,
                rom->buheader_len);

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
               rom->internal_crc) ? "ok" : "bad (use -chk to fix)",
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


/*
    flush the ucon64 struct with default values
*/
int
ucon64_flush (int argc, char *argv[], struct ucon64_ *rom)
{
  long x = 0;

  rom->argc = argc;
//  for( x = 0 ; x < argc ; x++ )strcpy(rom->argv[x],argv[x]);
  for (x = 0; x < argc; x++)
    rom->argv[x] = argv[x];

  rom->name[0] = 0;
//  rom->name2[0]=0;

  strcpy (rom->rom, getarg (argc, argv, ucon64_ROM));
  strcpy (rom->file, getarg (argc, argv, ucon64_FILE));

  rom->parport = 0x378;

  rom->title[0] = 0;
  strcpy (rom->copier, "?");

  rom->mbit = 0;
//  rom->bytes=0;
  rom->bytes = quickftell (rom->rom);

  rom->console = ucon64_UNKNOWN;        //integer for the console system

  rom->interleaved = 0;
  for (x = 0; x < sizeof (rom->splitted) / sizeof (rom->splitted[0]); x++)
    rom->splitted[x] = 0;
  rom->padded = 0;
  rom->intro = 0;

  rom->db_crc32 = 0;
  rom->current_crc32 = 0;       //standard current_crc32 checksum of the rom

  rom->has_internal_crc = 0;
  rom->current_internal_crc = 0;        //current custom crc
  rom->internal_crc = 0;        //custom crc (if not current_crc32)
  rom->internal_crc_start = 0;
  rom->internal_crc_len = 0;    //size in bytes
//  rom->has_internal_inverse_crc=0;
//  rom->internal_inverse_crc=0;
  rom->internal_crc2[0] = 0;
  rom->internal_crc2_start = 0;
  rom->internal_crc2_len = 0;   //size in bytes

  memset (rom->buheader, 0, sizeof (rom->buheader));
  rom->buheader_start = 0;
  rom->buheader_len = 0;        //header of backup unit

  memset (rom->header, 0, sizeof (rom->header));
  rom->header_start = 0;
  rom->header_len = 0;          //header of rom itself (if there is one)

  strcpy (rom->name, "?");
  rom->name_start = 0;
  rom->name_len = 0;

  strcpy (rom->manufacturer, "Unknown Manufacturer");
  rom->manufacturer_start = 0;
  rom->manufacturer_len = 0;

  strcpy (rom->country, "Unknown Country");
  rom->country_start = 0;
  rom->country_len = 0;

  rom->misc[0] = 0;

  return 0;
}

int ucon64_e(void)
    {
  long x;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[4096],
       *forceargs[] = {
    "",
    "-gb",
    "-gen",
    "-sms",
    "-jag",
    "-lynx",
    "-n64",
    "-ng",
    "-nes",
    "-pce",
    "-psx",
    "-ps2",
    "-snes",
    "-sat",
    "-dc",
    "-cd32",
    "-cdi",
    "-3do",
    "-ata",
    "-s16",
    "-ngp",
    "-gba",
    "-vec",
    "-vboy",
    "-swan",
    "-coleco",
    "-intelli",
    "-gc",
    "-xbox",
    "-gp32"
  };
      char *property;

      if (rom.console != ucon64_UNKNOWN /* && rom.console != ucon64_KNOWN */ )
        sprintf (buf3, "emulate_%s", &forceargs[rom.console][1]);
      else
        {
          printf ("ERROR: could not auto detect the right ROM/console type\n"
                  "TIP:   If this is a ROM you might try to force the recognition\n"
                  "       The force recognition option for Super Nintendo would be -snes\n");
          return -1;
        }

      if (access (rom.config_file, F_OK) != 0)
        {
          printf ("ERROR: %s does not exist\n", rom.config_file);
          return -1;
        }

      property = getProperty (rom.config_file, buf3, buf2, NULL);   // buf2 also contains property value
      if (property == NULL)
        {
          printf ("ERROR: could not find the correct settings (%s) in\n"
                  "       %s\n"
                  "TIP:   If the wrong console was detected you might try to force recognition\n"
                  "       The force recognition option for Super Nintendo would be -snes\n",
                  buf3, rom.config_file);
          return -1;
        }

      sprintf (buf, "%s %s", buf2, rom.file);
/*      for (x = 0; x < argc; x++)
        {
          if (strdcmp (argv[x], "-e")
              && strdcmp (argv[x], getarg (argc, argv, ucon64_NAME))
              && strdcmp (argv[x], rom.file))
            {
              sprintf (buf2, ((!strdcmp (argv[x], rom.rom)) ? " \"%s\"" :
                              ""), argv[x]);
              strcat (buf, buf2);
            }
        }
*/
      printf ("%s\n", buf);
      fflush (stdout);
      sync ();

      x = system (buf);
#ifndef __MSDOS__
      x >>= 8;                  // the exit code is coded in bits 8-15
#endif                          //  (that is, under Unix & BeOS)

#if 1
      // Snes9x (Linux) for example returns a non-zero value on a normal exit
      //  (3)...
      // under WinDOS, system() immediately returns with exit code 0 when
      //  starting a Windows executable (as if fork() was called) it also
      //  returns 0 when the exe could not be started
      if (x != 127 && x != -1 && x != 0)        // 127 && -1 are system() errors, rest are exit codes
        {
          printf ("ERROR: the Emulator returned an error code (%d)\n"
                  "TIP:   If the wrong emulator was used you might try to force recognition\n"
                  "       The force recognition option for Super Nintendo would be -snes\n",
                  (int) x);
          return x;
        }
#endif

      return 0;
    }

int ucon64_ls(int verbose)
{
  int ucon64_argc;
  struct dirent *ep;
  struct stat puffer;
  DIR *dp;
  char current_dir[FILENAME_MAX];
  char buf[MAXBUFSIZE], *ucon64_argv[128];

        if (access (rom.rom, R_OK) != 0 || (dp = opendir (rom.rom)) == NULL)
          return -1;

        getcwd (current_dir, FILENAME_MAX);
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
  sprintf (rom.config_file, "%s" FILE_SEPARATOR_S
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

  if (access (rom.config_file, F_OK) != 0)
    {
      FILE *fh;

      printf ("WARNING: %s not found: creating...", rom.config_file);

      if (!(fh = fopen (rom.config_file, "wb")))
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
  else if (strcmp (getProperty (rom.config_file, "version", buf2, "198"), "198") != 0)
    {
      strcpy (buf2, rom.config_file);
      newext (buf2, ".OLD");

      printf ("NOTE: updating config: old version will be renamed to %s...", buf2);

      filecopy (rom.config_file, 0, quickftell (rom.config_file), buf2, "wb");

      setProperty (rom.config_file, "version", "198");

      setProperty (rom.config_file, "backups", "1");

      setProperty (rom.config_file, "emulate_gp32", "");

      setProperty (rom.config_file, "cdrw_read",
        getProperty (rom.config_file, "cdrw_raw_read", buf2, "cdrdao read-cd --read-raw --device 0,0,0 --driver generic-mmc-raw --datafile "));
      setProperty (rom.config_file, "cdrw_write",
        getProperty (rom.config_file, "cdrw_raw_write", buf2, "cdrdao write --device 0,0,0 --driver generic-mmc "));

      deleteProperty (rom.config_file, "cdrw_raw_read");
      deleteProperty (rom.config_file, "cdrw_raw_write");
      deleteProperty (rom.config_file, "cdrw_iso_read");
      deleteProperty (rom.config_file, "cdrw_iso_write");

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
  if (rom.file[0])
    sscanf (rom.file, "%x", &rom.parport);

  if (!(rom.parport = parport_probe (rom.parport)))
    ;
/*
    printf ("ERROR: no parallel port 0x%s found\n\n", strupr (buf));
  else
    printf ("0x%x\n\n", rom.parport);
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
