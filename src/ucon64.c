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
#include "misc/defines.h"
#ifdef  USE_PARALLEL
#include "misc/parallel.h"
#endif
#include "misc/itypes.h"
#include "misc/bswap.h"
#include "misc/misc.h"
#include "misc/property.h"
#include "misc/hash.h"
#include "misc/file.h"
#include "misc/getopt2.h"
#include "misc/string.h"
#include "misc/term.h"
#include "misc/codec.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ucon64_opts.h"
#include "ucon64_dat.h"
#include "console/console.h"
#include "patch/patch.h"
#include "backup/backup.h"
#ifdef  TEST_UCON64
#include "misc/test.h"
#endif  // TEST_UCON64


#ifdef  TEST_UCON64
static void ucon64_test (void);
#endif
static int ucon64_switches (st_ucon64_t *p);
static int (*ucon64_func) (st_ucon64_t *) = NULL;


st_ucon64_t ucon64; 


#ifdef  AMIGA
unsigned long __stacksize = 102400;             // doesn't work on PPC? is StormC specific?
//unsigned long __stack = 102400;               // for SAS/C, DICE, GCC etc.?
char vers[] = "$VER: uCON64 "UCON64_VERSION_S" "CURRENT_OS_S" ("__DATE__") ("__TIME__")";
#endif
static uint32_t ucon64_flags = 0;
static int ucon64_files = 0;
static const char *ucon64_fname[UCON64_MAX_ARGS];
static st_getopt2_t options[UCON64_MAX_ARGS];
static const st_getopt2_t lf[] =
  {
    {NULL, 0, 0, 0, NULL, ""},
    {NULL, 0, 0, 0, NULL, NULL}
  },
  *option[] =
  {
    ucon64_options_usage,
    lf,
    ucon64_padding_usage,
    lf,
    ucon64_dat_usage,
    lf,
    patch_usage,
    bsl_usage,
    ips_usage,
    aps_usage,
    ppf_usage,
    gg_usage,
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
    genesis_usage,
#ifdef  USE_PARALLEL
    smd_usage,
    mdpro_usage,
    mcd_usage,
    cmc_usage,
  //  mgd_usage,
#endif
    lf,
    gb_usage,
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
    pce_usage,
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
    dc_usage,
    lf,
#ifdef  USE_PARALLEL
    dex_usage,
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


//#ifdef  DEBUG
static void
ucon64_sanity_check (void)
{
  // ucon64 sanity check
  {
    int i, j, f;

    for (i = 0; ucon64_filter[i].option; i++)
      {
         f = 0;
         for (j = 0; options[j].name || options[j].help; j++)
          if (options[j].val)
            if (ucon64_filter[i].option == options[j].val)
              f++;

         if (f != 1)
           printf ("option: %d was found %d times in options[]\n", ucon64_filter[i].option, f);
      }

    for (j = 0; options[j].name || options[j].help; j++)
      if (options[j].val)
      {
         f = 0;
         for (i = 0; ucon64_filter[i].option; i++)
          if (ucon64_filter[i].option == options[j].val)
            f++;

         if (f != 1)
           printf ("option: %d was found %d times in ucon64_filter[]\n", options[j].val, f);
      }
  }
}
//#endif


void
ucon64_exit (void)
{
  handle_registered_funcs ();
  fflush (stdout);
}


static int
ucon64_set_fname (const char *fname)
{
  if (ucon64_files < UCON64_MAX_ARGS)
    ucon64_fname[ucon64_files++] = fname;

  return 0;
}


static int
ucon64_opts (int c)
{
  static int x = 0;
  int i = 0;

  if (c == '?') // unknown option
    {
      fprintf (stderr, "Try '%s " OPTION_LONG_S "help' for more information.\n", ucon64.argv[0]);
      exit (1);
    }

  if (x >= UCON64_MAX_ARGS)
    return 0;

  for (i = 0; ucon64_filter[i].option; i++)
    if (ucon64_filter[i].option == c)
      {
        ucon64.option = ucon64_filter[i].option;
        ucon64.optarg = optarg;

        ucon64_switches (&ucon64);

        if (!ucon64_func && ucon64_filter[i].func)
          {
//            ucon64.option = ucon64_filter[i].option;
//            ucon64.optarg = optarg;

            if (ucon64.console == UCON64_UNKNOWN)
              ucon64.console = ucon64_filter[i].console;
            ucon64_flags = ucon64_filter[i].flags;

            ucon64_func = ucon64_filter[i].func;
          }
      }

  x++;

  return 0;
}


int
main (int argc, char **argv)
{
  int result = 0;
  int i = 0, x = 0, y = 0, c = 0;
  const char *p = NULL;
  struct stat fstate;
  struct option long_options[UCON64_MAX_ARGS];
  const st_property_t props[] =
    {
      {
        "backups", "1",
        "create backups of files? (1=yes; 0=no)\n"
        "before processing a ROM uCON64 will make a backup of it"
      },
      {
        "ansi_color", "1",
        "use ANSI colors in output? (1=yes; 0=no)"
      },
#ifdef  USE_PPDEV
      {
        "parport_dev", "/dev/parport0",
        "parallel port"
      },
#elif   defined AMIGA
      {
        "parport_dev", "parallel.device",
        "parallel port"
      },
      {
        "parport", "0",
        NULL
      },
#else
      {
        "parport", "378",
        "parallel port"
      },
#endif
#ifdef  USE_USB
#endif
      {
        "ucon64_configdir",
        PROPERTY_MODE_DIR ("ucon64"),
        "directory with additional config files"
      },
      {
        "ucon64_datdir",
        PROPERTY_MODE_DIR ("ucon64/dat"),
        "directory with DAT files"
      },
      {
        "f2afirmware", "f2afirm.hex",
        "F2A support files\n"
        "path to F2A USB firmware"
      },
      {
        "iclientu", "iclientu.bin",
        "path to GBA client binary (for USB code)"
      },
      {
        "iclientp", "iclientp.bin",
        "path to GBA client binary (for parallel port code)"
      },
      {
        "ilogo", "ilogo.bin",
        "path to iLinker logo file"
      },
      {
        "gbaloader", "loader.bin",
        "path to GBA multi-game loader"
      },
      {
        "gbaloader_sc", "sc_menu.bin",
        "path to GBA multi-game loader (Super Card)"
      },
      {
        "genpal_txt", "genpal.txt",
        "path to textfile with Genesis PAL protection search-and-defeat \"codes\" (patterns)"
      },
      {
        "mdntsc_txt", "mdntsc.txt",
        "path to textfile with Mega Drive NTSC protection search-and-defeat \"codes\" (patterns)"
      },
      {
        "snescopy_txt", "snescopy.txt",
        "path to textfile with SNES protection search-and-defeat \"codes\" (patterns)"
      },
      {
        "snesntsc_txt", "snesntsc.txt",
        "path to textfile with SNES NTSC protection search-and-defeat \"codes\" (patterns)"
      },
      {
        "snespal_txt", "snespal.txt",
        "path to textfile with SNES PAL protection search-and-defeat \"codes\" (patterns)"
      },
      {
        "snesslow_txt", "snesslow.txt",
        "path to textfile with SNES SLOWROM protection search-and-defeat \"codes\" (patterns)"
      },
      {
        "emulate_" UCON64_3DO_S,      "",
        "emulate_<console shortcut>=<emulator with options>\n\n"
        "Example: \"emulate_snes=snes9x -fs\"\n\n"
        "You can also use CRC32 values for ROM specific emulation options:\n\n"
        "emulate_0x<crc32>=<emulator with options>\n"
        "emulate_<crc32>=<emulator with options>"
      },
      {"emulate_" UCON64_ATA_S,      "", NULL},
      {"emulate_" UCON64_CD32_S,     "", NULL},
      {"emulate_" UCON64_CDI_S,      "", NULL},
      {"emulate_" UCON64_COLECO_S,   "", NULL},
      {"emulate_" UCON64_DC_S,       "", NULL},
      {"emulate_" UCON64_GB_S,       "vgb -sound -sync 50 -sgb -scale 2", NULL},
      {"emulate_" UCON64_GBA_S,      "vgba -scale 2 -uperiod 6", NULL},
      {"emulate_" UCON64_GC_S,       "", NULL},
      {"emulate_" UCON64_GEN_S,      "dgen -f -S 2", NULL},
      {"emulate_" UCON64_INTELLI_S,  "", NULL},
      {"emulate_" UCON64_JAG_S,      "", NULL},
      {"emulate_" UCON64_LYNX_S,     "", NULL},
      {"emulate_" UCON64_ARCADE_S,   "", NULL},
      {"emulate_" UCON64_N64_S,      "", NULL},
      {"emulate_" UCON64_NES_S,      "tuxnes -E2 -rx11 -v -s/dev/dsp -R44100", NULL},
      {"emulate_" UCON64_NG_S,       "", NULL},
      {"emulate_" UCON64_NGP_S,      "", NULL},
      {"emulate_" UCON64_PCE_S,      "", NULL},
      {"emulate_" UCON64_PS2_S,      "", NULL},
      {"emulate_" UCON64_PSX_S,      "pcsx", NULL},
      {"emulate_" UCON64_S16_S,      "", NULL},
      {"emulate_" UCON64_SAT_S,      "", NULL},
      {"emulate_" UCON64_SMS_S,      "", NULL},
      {"emulate_" UCON64_GAMEGEAR_S, "", NULL},
      {"emulate_" UCON64_SNES_S,     "snes9x -tr -sc -hires -dfr -r 7 -is -joymap1 2 3 5 0 4 7 6 1", NULL},
      {"emulate_" UCON64_SWAN_S,     "", NULL},
      {"emulate_" UCON64_VBOY_S,     "", NULL},
      {"emulate_" UCON64_VEC_S,      "", NULL},
      {"emulate_" UCON64_XBOX_S,     "", NULL},
      {NULL, NULL, NULL}
    };

#ifdef  TEST_UCON64
  if (argc == 1)
    ucon64_test ();
#else
  // turn (st_getopt2_t **) into (st_getopt2_t *)
  memset (&options, 0, sizeof (st_getopt2_t) * UCON64_MAX_ARGS);
  for (c = x = 0; option[x]; x++)
    for (y = 0; option[x][y].name || option[x][y].help; y++)
      if (c < UCON64_MAX_ARGS)
        {
          memcpy (&options[c], &option[x][y], sizeof (st_getopt2_t));
          c++;
        }

#ifdef  DEBUG
  getopt2_sanity_check (option); // check (st_getopt2_t *) options consistency
#endif
//#ifdef  DEBUG
  ucon64_sanity_check ();
//#endif

  printf ("uCON64 " UCON64_VERSION_S " " CURRENT_OS_S " 1999-2006\n"
    "Uses code from various people. See 'developers.html' for more!\n"
    "This may be freely redistributed under the terms of the GNU Public License\n\n");
#endif

  atexit (ucon64_exit);

  // defaults
  memset (&ucon64_fname, 0, sizeof (const char *) * UCON64_MAX_ARGS);
  memset (&ucon64, 0, sizeof (st_ucon64_t));

  ucon64.argc = argc;
  ucon64.argv = argv;

  ucon64.fname =
  ucon64.mapr =
  ucon64.comment = "";
  ucon64.patch = ucon64.argv[ucon64.argc - 1];

#warning ucon64.backup_header_len can be (-1) (UCON64_UNKNOWN)
  ucon64.console = UCON64_UNKNOWN;

  ucon64.backup_header_len =
  ucon64.battery =
  ucon64.bs_dump =
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

  // read/parse configfile
#ifdef  __unix__
  // We need to modify the umask, because the configfile is made while we are
  //  still running in root mode. Maybe 0 is even better (in case root did
  //  `chmod +s').
  umask (002);
#endif
  realpath2 (PROPERTY_HOME_RC ("ucon64"), ucon64.configfile);

  result = property_check (ucon64.configfile, UCON64_CONFIG_VERSION, 1);
  if (result == 1) // update needed
    result = set_property_array (ucon64.configfile, props);
  if (result == -1) // property_check() or update failed
    return -1;

  // ANSI colors?
#ifdef  USE_ANSI_COLOR
  ucon64.ansi_color = get_property_int (ucon64.configfile, "ansi_color");
  // the conditional call to ansi_init() has to be done *after* the check for
  //  the switch --ncol
#endif

  // parallel port?
#warning default?
#if     defined USE_PPDEV || defined AMIGA
  if ((p = get_property (ucon64.configfile, "parport_dev", PROPERTY_MODE_FILENAME)))
#ifdef  USE_PPDEV
    strncpy (ucon64.parport_dev, "/dev/parport0", FILENAME_MAX)[FILENAME_MAX - 1] = 0;
#elif   defined AMIGA
    strncpy (ucon64.parport_dev, "parallel.device", FILENAME_MAX)[FILENAME_MAX - 1] = 0;
#endif
#endif

  if ((p = get_property (ucon64.configfile, "parport", PROPERTY_MODE_TEXT)))
    sscanf (p, "%x", &ucon64.parport);
  else
    // use UCON64_UNKNOWN to force probing if the config file doesn't contain
    //  a parport line
    ucon64.parport = UCON64_UNKNOWN;

  // make backups?
  ucon64.backup = get_property_int (ucon64.configfile, "backups");

  // $HOME/.ucon64/ ?
  if ((p = get_property (ucon64.configfile, "ucon64_configdir", PROPERTY_MODE_FILENAME)))
    strncpy (ucon64.configdir, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  // DAT file handling
  if ((p = get_property (ucon64.configfile, "ucon64_datdir", PROPERTY_MODE_FILENAME)))
    strncpy (ucon64.datdir, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  if (argc < 2)
    {
      ucon64_usage (argc, argv, USAGE_VIEW_SHORT);
      return 0;
    }

  // turn st_getopt2_t into struct option
  getopt2_long_only (long_options, options, UCON64_MAX_ARGS);

  // getopt() is utilized to make uCON64 handle/parse cmdlines in a sane
  //  and expected way
//  optind = 0;
  while ((c = getopt_long_only (argc, argv, "", long_options, NULL)) != -1)
    ucon64_opts (c);

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
    ucon64_flags & WF_PAR to detect whether a (parallel port) copier option has
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

  if (optind != argc)                   // files were specified
    {
      int flags = GETFILE_FILES_ONLY;

      if (ucon64.recursive)
        flags |= GETFILE_RECURSIVE;
      else 
        {
          /*
            Check if one of the parameters is a directory and if so, set the
            flag GETFILE_RECURSIVE_ONCE. This flag makes uCON64 behave
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
                  flags |= GETFILE_RECURSIVE_ONCE;
                  break;
                }
        }

      getfile (argc, argv, ucon64_set_fname, flags);
    }

  if (!ucon64_files)
    {
      if (ucon64_func)
        return ucon64_func (&ucon64);
      return 0;
    }

  for (i = 0; ucon64_fname[i]; i++)
    {
      ucon64.fname = ucon64_fname[i];
      ucon64.split = UCON64_UNKNOWN;
      ucon64.crc32 = ucon64.fcrc32 = 0;
      ucon64_flags = WF_DEMUX|WF_OPEN|WF_NFO;

#warning TODO: unzip into a temp dir

      if (ucon64.console == UCON64_UNKNOWN)
        if (ucon64_flags & WF_DEMUX)
          ucon64.console = ucon64_console_demux (ucon64.fname);

      if (ucon64_flags & WF_OPEN)
        ucon64.nfo = ucon64_console_open (ucon64.fname, ucon64.console);
    
      if (!ucon64.nfo)
        if (ucon64_flags & WF_NEEDS_ROM)
          {
            fputs ("ERROR: This option requires a file argument (ROM/image/SRAM file/directory)\n", stderr);
            return -1;
          }
    
      // Does the option allow split ROMs?
      if (ucon64_flags & WF_NO_SPLIT)
        switch (ucon64.console)
          {
            /*
              Test for split files only if the console type knows about split files at
              all. However we only know the console type after probing.
            */
            case UCON64_NES:
            case UCON64_SNES:
            case UCON64_GEN:
            case UCON64_NG:
              if (ucon64.split == UCON64_UNKNOWN)
                ucon64.split = ucon64_testsplit (ucon64.fname, NULL);
              if (ucon64.split)
                {
                  fprintf (stderr, "ERROR: %s seems to be split. You have to join it first\n",
                           basename2 (ucon64.fname));
                  return -1;
                }
              break;
          }
    
      // CRC32 calculation
      if (ucon64_flags & WF_NEEDS_CRC32)
        {
#warning fcrc32 == raw_crc32?
          ucon64_chksum (NULL, NULL, &ucon64.fcrc32, ucon64.fname,
                         ucon64.nfo ? ucon64.nfo->backup_header_len : ucon64.backup_header_len);
          ucon64_chksum (NULL, NULL, &ucon64.crc32, ucon64.fname, ucon64.nfo->backup_header_len);
        }
    
      // DATabase
      if (ucon64.dat_enabled)
        if (ucon64.crc32)
          ucon64.dat = ucon64_dat_search (ucon64.crc32, NULL);
    
      // display info
      if ((ucon64_flags & WF_NFO) && ucon64.quiet < 1)
        {
          printf ("%s\n", ucon64.fname);
        
          if (ucon64.nfo)
            {
              ucon64_rom_nfo (ucon64.nfo);
        
              if (ucon64.crc32)
                printf ("ROM checksum (CRC32): 0x%08x\n", ucon64.crc32);
        
              if (ucon64.dat)
                ucon64_dat_nfo (ucon64.dat, 1);
            }
          else
            fprintf (stderr, "%s\n", ucon64_msg[CONSOLE_ERROR]);

          printf ("File checksum (CRC32): 0x%08x\n", ucon64.fcrc32);

          fputc ('\n', stdout);
        }
        
      if (ucon64_func)
        if (ucon64_func (&ucon64) == -1)
          {
            const st_getopt2_t *p = getopt2_get_index_by_val (options, c);
            const char *opt = p ? p->name : NULL;
        
            fprintf (stderr, "ERROR: %s%s encountered a problem\n"
                             "       Is the option you used available for the current console system?\n"
                             "       Please report bugs to ucon64-main@lists.sf.net or http://ucon64.sf.net\n\n",
                             opt ? (!opt[1] ? OPTION_S : OPTION_LONG_S) : "",
                             opt ? opt : "uCON64");
        
            return -1;
          }

#warning TODO ucon64_dat_close()
//      if (ucon64.dat)
//        ucon64_dat_close (ucon64.dat);

      ucon64_console_close (ucon64.nfo);

      if (ucon64_flags & WF_EXIT)
        return 0; // do not process any other ROMs
    }

  return 0;
}


int
ucon64_switches (st_ucon64_t *p)
{
  switch (p->option)
    {
    case UCON64_FRONTEND:
      ucon64.frontend = 1;                      // used by (for example) ucon64_gauge()
      break;

    case UCON64_NBAK:
      ucon64.backup = 0;
      break;

    case UCON64_R:
      ucon64.recursive = 1;
      break;

#ifdef  USE_ANSI_COLOR
    case UCON64_NCOL:
      ucon64.ansi_color = 0;
      break;
#endif

    case UCON64_NS:
      ucon64.split = 0;
      break;

    case UCON64_HD:
      ucon64.backup_header_len = MIN (UNKNOWN_BACKUP_HEADER_LEN, fsizeof (ucon64.fname));
      break;

    case UCON64_HDN:
      ucon64.backup_header_len = MIN (strtol (p->optarg, NULL, 10), fsizeof (ucon64.fname));
      break;

    case UCON64_NHD:
      ucon64.backup_header_len = 0;
      break;

    case UCON64_SWP:                            // deprecated
    case UCON64_INT:
      ucon64.interleaved = 1;
      break;

    case UCON64_INT2:
      ucon64.interleaved = 2;
      break;

    case UCON64_NSWP:                           // deprecated
    case UCON64_NINT:
      ucon64.interleaved = 0;
      break;

    case UCON64_PATCH:
      ucon64.patch = p->optarg;
      break;

    case UCON64_PORT:
#ifdef  USE_USB
      if (!strnicmp (p->optarg, "usb", 3))
        {
          if (strlen (p->optarg) >= 4)
            ucon64.usbport = strtol (p->optarg + 3, NULL, 10) + 1; // usb0 => ucon64.usbport = 1
          else                                  // we automatically detect the
            ucon64.usbport = 1;                 //  USB port in the F2A code

          /*
            We don't want to make uCON64 behave different if --port=USB{n} is
            specified *after* a transfer option (instead of before one), so we
            have to reset ucon64.parport_needed here.
          */
          ucon64.parport_needed = 0;
        }
      else
#endif
        ucon64.parport = strtol (p->optarg, NULL, 16);
      break;

#ifdef  USE_PARALLEL
    /*
      We detect the presence of these options here so that we can drop
      privileges ASAP.
      Note that the libcd64 options are not listed here. We cannot drop
      privileges before libcd64 is initialised (after cd64_t.devopen() has been
      called).
    */
    case UCON64_XCMC:
    case UCON64_XCMCT:
    case UCON64_XDEX:
    case UCON64_XDJR:
    case UCON64_XF2A:                           // could be for USB version
    case UCON64_XF2AMULTI:                      // idem
    case UCON64_XF2AC:                          // idem
    case UCON64_XF2AS:                          // idem
    case UCON64_XF2AB:                          // idem
    case UCON64_XFAL:
    case UCON64_XFALMULTI:
    case UCON64_XFALC:
    case UCON64_XFALS:
    case UCON64_XFALB:
    case UCON64_XFIG:
    case UCON64_XFIGS:
    case UCON64_XFIGC:
    case UCON64_XGBX:
    case UCON64_XGBXS:
    case UCON64_XGBXB:
    case UCON64_XGD3:
    case UCON64_XGD3R:
    case UCON64_XGD3S:
    case UCON64_XGD6:
    case UCON64_XGD6R:
    case UCON64_XGD6S:
    case UCON64_XGG:
    case UCON64_XGGS:
    case UCON64_XGGB:
    case UCON64_XLIT:
    case UCON64_XMCCL:
    case UCON64_XMCD:
    case UCON64_XMD:
    case UCON64_XMDS:
    case UCON64_XMDB:
    case UCON64_XMSG:
    case UCON64_XPCE:
    case UCON64_XPL:
    case UCON64_XPLI:
    case UCON64_XRESET:
    case UCON64_XSF:
    case UCON64_XSFS:
    case UCON64_XSMC:
    case UCON64_XSMCR:
    case UCON64_XSMD:
    case UCON64_XSMDS:
    case UCON64_XSWC:
    case UCON64_XSWC2:
    case UCON64_XSWCR:
    case UCON64_XSWCS:
    case UCON64_XSWCC:
    case UCON64_XV64:
#ifdef  USE_USB
      if (!ucon64.usbport)                      // no pport I/O if F2A option and USB F2A
#endif
      ucon64.parport_needed = 1;
      /*
        We want to make this possible:
          1.) ucon64 <transfer option> <rom>
          2.) ucon64 <transfer option> <rom> --port=<parallel port address>
        The above works "automatically". The following type of command used to
        be possible, but has been deprecated:
          3.) ucon64 <transfer option> <rom> <parallel port address>
        It has been removed, because it caused problems when specifying additional
        switches without specifying the parallel port address. For example:
          ucon64 -xfal -xfalm <rom>
        This would be interpreted as:
          ucon64 -xfal -xfalm <rom as file> <rom as parallel port address>
        If <rom> has a name that starts with a number an I/O port associated
        with that number will be accessed which might well have unwanted
        results. We cannot check for valid I/O port numbers, because the I/O
        port of the parallel port can be mapped to almost any 16-bit number.
      */
#if 0
      if (ucon64.parport == UCON64_UNKNOWN)
        if (ucon64.argc >= 4)
          if (access (ucon64.argv[ucon64.argc - 1], F_OK))
            // Yes, we don't get here if ucon64.argv[ucon64.argc - 1] is [0x]278,
            //  [0x]378 or [0x]3bc and a file with the same name (path) exists.
            ucon64.parport = strtol (ucon64.argv[ucon64.argc - 1], NULL, 16);
#endif
      break;

#ifdef  USE_LIBCD64
    case UCON64_XCD64:
    case UCON64_XCD64B:
    case UCON64_XCD64C:
    case UCON64_XCD64E:
    case UCON64_XCD64F:
    case UCON64_XCD64M:
    case UCON64_XCD64S:
      // We don't really need the parallel port. We just have to make sure that
      //  privileges aren't dropped.
      ucon64.parport_needed = 2;
      break;

    case UCON64_XCD64P:
      ucon64.io_mode = strtol (p->optarg, NULL, 10);
      break;
#endif

    case UCON64_XCMCM:
      ucon64.io_mode = strtol (p->optarg, NULL, 10);
      break;

    case UCON64_XFALM:
    case UCON64_XGBXM:
    case UCON64_XPLM:
      ucon64.parport_mode = UCON64_EPP;
      break;

    case UCON64_XSWC_IO:
      ucon64.io_mode = strtol (p->optarg, NULL, 16);

      if (ucon64.io_mode & SWC_IO_ALT_ROM_SIZE)
        puts ("WARNING: I/O mode not yet implemented");
#if 0 // all these constants are defined by default
      if (ucon64.io_mode & (SWC_IO_SPC7110 | SWC_IO_SDD1 | SWC_IO_SA1 | SWC_IO_MMX2))
        puts ("WARNING: Be sure to compile swc.c with the appropriate constants defined");
#endif

      if (ucon64.io_mode > SWC_IO_MAX)
        {
          printf ("WARNING: Invalid value for MODE (0x%x), using 0\n", ucon64.io_mode);
          ucon64.io_mode = 0;
        }
      else
        {
          printf ("I/O mode: 0x%03x", ucon64.io_mode);
          if (ucon64.io_mode)
            {
              char flagstr[100];

              flagstr[0] = 0;
              if (ucon64.io_mode & SWC_IO_FORCE_32MBIT)
                strcat (flagstr, "force 32 Mbit dump, ");
              if (ucon64.io_mode & SWC_IO_ALT_ROM_SIZE)
                strcat (flagstr, "alternative ROM size method, ");
              if (ucon64.io_mode & SWC_IO_SUPER_FX)
                strcat (flagstr, "Super FX, ");
              if (ucon64.io_mode & SWC_IO_SDD1)
                strcat (flagstr, "S-DD1, ");
              if (ucon64.io_mode & SWC_IO_SA1)
                strcat (flagstr, "SA-1, ");
              if (ucon64.io_mode & SWC_IO_SPC7110)
                strcat (flagstr, "SPC7110, ");
              if (ucon64.io_mode & SWC_IO_DX2_TRICK)
                strcat (flagstr, "DX2 trick, ");
              if (ucon64.io_mode & SWC_IO_MMX2)
                strcat (flagstr, "Mega Man X 2, ");
              if (ucon64.io_mode & SWC_IO_DUMP_BIOS)
                strcat (flagstr, "dump BIOS, ");

              if (flagstr[0])
                flagstr[strlen (flagstr) - 2] = 0;
              printf (" (%s)", flagstr);
            }
          fputc ('\n', stdout);
        }
      break;
#endif // USE_PARALLEL

    case UCON64_O:
      {
        struct stat fstate;
        int dir = 0;

        if (!stat (p->optarg, &fstate))
          if (S_ISDIR (fstate.st_mode))
            {
              strcpy (ucon64.output_path, p->optarg);
              if (ucon64.output_path[strlen (ucon64.output_path) - 1] != FILE_SEPARATOR)
                strcat (ucon64.output_path, FILE_SEPARATOR_S);
              dir = 1;
            }

        if (!dir)
          puts ("WARNING: Argument for -o must be a directory. Using current directory instead");
      }
      break;

    case UCON64_NHI:
      ucon64.snes_hirom = 0;
      break;

    case UCON64_HI:
      ucon64.snes_hirom = SNES_HIROM;
      break;

    case UCON64_EROM:
      ucon64.snes_header_base = SNES_EROM;
      break;

    case UCON64_BS:
      ucon64.bs_dump = 1;
      break;

    case UCON64_NBS:
      ucon64.bs_dump = 0;
      break;

    case UCON64_CTRL:
      if (ucon64.controller != UCON64_UNKNOWN)
        ucon64.controller |= 1 << strtol (p->optarg, NULL, 10);
      else
        ucon64.controller = 1 << strtol (p->optarg, NULL, 10);
      break;

    case UCON64_CTRL2:
      if (ucon64.controller2 != UCON64_UNKNOWN)
        ucon64.controller2 |= 1 << strtol (p->optarg, NULL, 10);
      else
        ucon64.controller2 = 1 << strtol (p->optarg, NULL, 10);
      break;

    case UCON64_NTSC:
      if (ucon64.tv_standard == UCON64_UNKNOWN)
        ucon64.tv_standard = 0;
      else if (ucon64.tv_standard == 1)
        ucon64.tv_standard = 2;                 // code for NTSC/PAL (NES UNIF/iNES)
      break;

    case UCON64_PAL:
      if (ucon64.tv_standard == UCON64_UNKNOWN)
        ucon64.tv_standard = 1;
      else if (ucon64.tv_standard == 0)
        ucon64.tv_standard = 2;                 // code for NTSC/PAL (NES UNIF/iNES)
      break;

    case UCON64_BAT:
      ucon64.battery = 1;
      break;

    case UCON64_NBAT:
      ucon64.battery = 0;
      break;

    case UCON64_VRAM:
      ucon64.vram = 1;
      break;

    case UCON64_NVRAM:
      ucon64.vram = 0;
      break;

    case UCON64_MIRR:
      ucon64.mirror = strtol (p->optarg, NULL, 10);
      break;

    case UCON64_MAPR:
      ucon64.mapr = p->optarg;                     // pass the _string_, it can be a
      break;                                    //  board name

    case UCON64_CMNT:
      ucon64.comment = p->optarg;
      break;

    case UCON64_DUMPINFO:
      ucon64.use_dump_info = 1;
      ucon64.dump_info = p->optarg;
      break;

    case UCON64_Q:
    case UCON64_QQ:                             // for now -qq is equivalent to -q
      ucon64.quiet = 1;
      break;

    case UCON64_V:
      ucon64.quiet = -1;
      break;

    case UCON64_SSIZE:
      ucon64.part_size = strtol (p->optarg, NULL, 10) * MBIT;
      break;

    case UCON64_ID:
      ucon64.id = -2;                           // just a value other than
      break;                                    //  UCON64_UNKNOWN and smaller than 0

    case UCON64_IDNUM:
      ucon64.id = strtol (p->optarg, NULL, 10);
      if (ucon64.id < 0)
        ucon64.id = 0;
      else if (ucon64.id > 999)
        {
          fprintf (stderr, "ERROR: NUM must be smaller than 999\n");
          exit (1);
        }
      break;

    case UCON64_REGION:
      if (p->optarg[1] == 0 && toupper (p->optarg[0]) == 'X') // be insensitive to case
        ucon64.region = 256;
      else
        ucon64.region = strtol (p->optarg, NULL, 10);
      break;

    default:
      break;
    }

  return 0;
}


void
ucon64_usage (int argc, char *argv[], int view)
{
  (void) argc;                                  // warning remover
  int x = 0, y = 0, c = 0, single = 0;
  const char *name_exe = basename2 (argv[0]);

#ifdef  USE_ZLIB
  printf ("Usage: %s [OPTION]... [ROM|IMAGE|SRAM|FILE|DIR|ARCHIVE]...\n\n", name_exe);
#else
  printf ("Usage: %s [OPTION]... [ROM|IMAGE|SRAM|FILE|DIR]...\n\n", name_exe);
#endif

  // single usage
#if 0
  for (x = 0; options[x]; x++)
    for (y = 0; ucon64_filter[y]; y++)
      if (options[x].val == ucon64_filter[y].option &&
          ucon64_filter[y].console == ucon64.console)
        


  for (y = 0; option[y]; y++)
    for (c = 0; option[y][c].name || option[y][c].help; c++)
      if (option[y][c].object)
        if (((st_ucon64_obj_t *) option[y][c].object)->console == arg[x].console)
          {
                getopt2_usage (option[y]);
                single = 1;
                break;
          }
#endif

  if (!single)
    switch (view)
      {
        case USAGE_VIEW_LONG:
          getopt2_usage (options);
          break;

        case USAGE_VIEW_PAD:
          getopt2_usage (ucon64_padding_usage);
          break;

        case USAGE_VIEW_DAT:
          getopt2_usage (ucon64_dat_usage);
          break;

        case USAGE_VIEW_PATCH:
          getopt2_usage (patch_usage);
          getopt2_usage (bsl_usage);
          getopt2_usage (ips_usage);
          getopt2_usage (aps_usage);
          getopt2_usage (ppf_usage);
          getopt2_usage (gg_usage);
          break;

        case USAGE_VIEW_BACKUP:
//          getopt2_usage (cc2_usage);
//          getopt2_usage (cd64_usage);
          getopt2_usage (cmc_usage);
          getopt2_usage (dex_usage);
          getopt2_usage (doctor64_usage);
          getopt2_usage (doctor64jr_usage);
          getopt2_usage (f2a_usage);
          getopt2_usage (fal_usage);
          getopt2_usage (fig_usage);
          getopt2_usage (gbx_usage);
          getopt2_usage (gd_usage);
//          getopt2_usage (interceptor_usage);
          getopt2_usage (lynxit_usage);
          getopt2_usage (mccl_usage);
          getopt2_usage (mcd_usage);
          getopt2_usage (mdpro_usage);
//          getopt2_usage (mgd_usage);
          getopt2_usage (msg_usage);
//          getopt2_usage (nfc_usage);
          getopt2_usage (pcepro_usage);
          getopt2_usage (pl_usage);
//          getopt2_usage (sc_usage);
          getopt2_usage (sflash_usage);
          getopt2_usage (smc_usage);
          getopt2_usage (smd_usage);
          getopt2_usage (smsggpro_usage);
//          getopt2_usage (spsc_usage);
//          getopt2_usage (ssc_usage);
          getopt2_usage (swc_usage);
//          getopt2_usage (ufo_usage);
//          getopt2_usage (yoko_usage);
//          getopt2_usage (z64_usage);
          break;

        case USAGE_VIEW_SHORT:
        default:
          getopt2_usage (ucon64_options_usage);
      }

  fputc ('\n', stdout);

  printf ("DATabase: %d known ROMs (DAT files: %s)\n\n",
          ucon64_dat_total_entries (), ucon64.datdir);

  puts ("Please report problems, fixes or ideas to ucon64-main@lists.sf.net or visit\n"
        "http://ucon64.sourceforge.net\n");
}


#ifdef  TEST_UCON64
void
ucon64_test (void)
{
  st_test_t t[] =
    {
      {UCON64_1991,	"ucon64 -1991 /tmp/test/test.smd;"
                        "ucon64 -gen test.smd;"
                        "rm test.smd", 0xadc940f4},
      {UCON64_A,	"ucon64 -a", TEST_TODO},
      {UCON64_ATA,	"ucon64 -ata /tmp/test/test.64k", 0x4b3a37d0},
      {UCON64_B,	"ucon64 -b", TEST_TODO},
      {UCON64_B0,	"ucon64 -b0 64 /tmp/test/test.lnx;"
                        "ucon64 test.lnx;"
                        "rm test.lnx", 0xde32e069},
      {UCON64_B1,	"ucon64 -b1 64 /tmp/test/test.lnx;"
                        "ucon64 test.lnx;"
                        "rm test.lnx", 0x6b648320},
      {UCON64_BAT,	"ucon64 -bat /tmp/test/test.nes", TEST_BUG},
      {UCON64_BIN,	"ucon64 -bin", TEST_TODO},
      {UCON64_BIOS,	"ucon64 -bios", TEST_TODO},
      {UCON64_BOT,	"ucon64 -bot=test.bot /tmp/test/test.v64;"
                        "ucon64 -crc test.bot;"
                        "rm test.bot", 1},
      {UCON64_BS,	"ucon64 -bs /tmp/test/test.smc", 0x18910ac6},
      {UCON64_C,	"ucon64 -c /tmp/test/test.txt /tmp/test/12345678.abc", 0x2284888d},
#if 0
      {UCON64_CC2,      "ucon64 -cc2 /tmp/test/test.16k;"
                        "ucon64 -crc test.wav;" // crc should be 0xc5cdd20f
                        "rm test.wav", 3},
#endif
      {UCON64_CHK,	"ucon64 -chk /tmp/test/test.smc;"
                        "ucon64 test.smc;"
                        "rm test.smc", 0x3fa1e89a},
      {UCON64_CMNT,	"ucon64 -cmnt", TEST_TODO},
      {UCON64_CODE,	"ucon64 -code /tmp/test/test.txt", TEST_BUG},
      {UCON64_COL,	"ucon64 -col 0xff00", 0xd4f45031},
      {UCON64_COLECO,	"ucon64 -coleco /tmp/test/test.1mb", 0x2fb8741c},
      {UCON64_CRC,	"ucon64 -crc /tmp/test/test.2kb", 0xd17fda4a},
      {UCON64_CRP,	"ucon64 -crp", TEST_TODO},
      {UCON64_CS,	"ucon64 -cs /tmp/test/test.txt /tmp/test/12345678.abc", 0xd6e61833},
      {UCON64_CTRL,	"ucon64 -ctrl", TEST_TODO},
      {UCON64_CTRL2,	"ucon64 -ctrl2", TEST_TODO},
      {UCON64_DB,	"ucon64 -db", TEST_TODO},
      {UCON64_DBS,	"ucon64 -dbs", TEST_TODO},
      {UCON64_DBUH,	"ucon64 -dbuh", TEST_TODO},
      {UCON64_DBV,	"ucon64 -dbv", TEST_TODO},
      {UCON64_DC,	"ucon64 -dc /tmp/test/test.1mb", 0x14c9d369},
      {UCON64_DFIND,	"ucon64 -dfind \"97 98 99 100\" /tmp/test/test.txt", 0xd7aed3fd},
      {UCON64_DFINDR,	"ucon64 -dfindr \"1 2 3 4\" /tmp/test/test.txt", 0x88a6f737},
      {UCON64_DINT,	"ucon64 -dint /tmp/test/test.txt;"
                        "ucon64 -crc test.txt;"
                        "rm test.txt", 0xe60a7df5},
      {UCON64_DMIRR,	"ucon64 -dmirr", TEST_TODO},
      {UCON64_DNSRT,	"ucon64 -dnsrt", TEST_TODO},
      {UCON64_BITS,	"ucon64 -bits", TEST_TODO},
      {UCON64_DUMPINFO,	"ucon64 -dumpinfo", TEST_TODO},
      {UCON64_E,	"ucon64 -e", TEST_TODO},
      {UCON64_EROM,	"ucon64 -erom", TEST_TODO},
      {UCON64_F,	"ucon64 -f", TEST_TODO},
      {UCON64_FDS,	"ucon64 -fds", TEST_TODO},
      {UCON64_FDSL,	"ucon64 -fdsl", TEST_TODO},
      {UCON64_FFE,	"ucon64 -ffe", TEST_TODO},
      {UCON64_FIG,	"ucon64 -fig", TEST_TODO},
      {UCON64_FIGS,	"ucon64 -figs", TEST_TODO},
      {UCON64_FIND,	"ucon64 -find \"abcd\" /tmp/test/test.txt", 0xd7aed3fd},
      {UCON64_FINDI,	"ucon64 -findi \"ABcD\" /tmp/test/test.txt", 0x10e913cd},
      {UCON64_FINDR,	"ucon64 -findr \"1234\" /tmp/test/test.txt", 0x1b1284d8},
      {UCON64_GB,	"ucon64 -gb /tmp/test/test.1mb", 0xf050caa1},
      {UCON64_GBA,	"ucon64 -gba /tmp/test/test.1mb", 0x5253861d},
      {UCON64_GBX,	"ucon64 -gbx", TEST_TODO},
      {UCON64_GD3,	"ucon64 -gd3", TEST_TODO},
      {UCON64_GD3S,	"ucon64 -gd3s", TEST_TODO},
      {UCON64_GEN,	"ucon64 -gen /tmp/test/test.1mb", 0xaa1f503c},
      {UCON64_GG,	"ucon64 -gg", TEST_TODO},
      {UCON64_GGD,	"ucon64 -ggd", TEST_TODO},
      {UCON64_GGE,	"ucon64 -gge", TEST_TODO},
      {UCON64_HD,	"ucon64 -snes -hd /tmp/test/test.1mb", 0x9ea45865},
      {UCON64_HDN,	"ucon64 -snes -hdn=1024 /tmp/test/test.1mb", 0x37812a26},
      {UCON64_HEX,	"ucon64 -hex /tmp/test/test.txt", 0x90d0b764},
      {UCON64_HFIND,	"ucon64 -hfind \"? 68 ?? 6a\" /tmp/test/test.txt", 0xdfa06028},
      {UCON64_HFINDR,	"ucon64 -hfindr \"01 02 03 04\" /tmp/test/test.txt", 0x88a6f737},
      {UCON64_HI,	"ucon64 -snes -hi /tmp/test/test.1mb", 0x086266b1},
      {UCON64_I,	"ucon64 -i", TEST_TODO},
      {UCON64_IDNUM,	"ucon64 -idnum", TEST_TODO},
      {UCON64_IDPPF,	"ucon64 -idppf", TEST_TODO},
      {UCON64_INES,	"ucon64 -ines", TEST_TODO},
      {UCON64_INESHD,	"ucon64 -ineshd", TEST_TODO},
      {UCON64_INS,      "ucon64 -ins /tmp/test/test.txt;"
                        "ucon64 -crc test.txt 2>&1;"
                        "rm test.txt", 0xa87abae3},
      {UCON64_INSN,     "ucon64 -insn=512 /tmp/test/test.txt;"
                        "ucon64 -crc test.txt 2>&1;"
                        "rm test.txt", 0xa87abae3},
      {UCON64_INT,	"ucon64 -int /tmp/test/test.z64", 0x1d23f41c},
      {UCON64_INT2,	"ucon64 -int2 /tmp/test/test.v64", 1},
      {UCON64_ISPAD,	"ucon64 -ispad /tmp/test/test.2kb", 0xa5e38fbd},
      {UCON64_J,	"ucon64 -j", TEST_TODO},
      {UCON64_JAG,	"ucon64 -jag /tmp/test/test.1mb", 0x2d1f3594},
      {UCON64_K,	"ucon64 -k", TEST_TODO},
      {UCON64_L,	"ucon64 -l", TEST_TODO},
      {UCON64_LNX,	"ucon64 -lnx", TEST_TODO},
      {UCON64_LOGO,	"ucon64 -logo /tmp/test/test.gba", TEST_TODO},
      {UCON64_LS,	"ucon64 -ls /tmp/test/*", TEST_BUG},
      {UCON64_LSD,	"ucon64 -lsd /tmp/test/*", TEST_BUG},
      {UCON64_LSRAM,	"ucon64 -lsram", TEST_TODO},
      {UCON64_LSV,	"ucon64 -lsv", TEST_TODO},
      {UCON64_LYNX,	"ucon64 -lynx /tmp/test/test.1mb", 0x78e02858},
      {UCON64_LYX,	"ucon64 -lyx", TEST_TODO},
      {UCON64_MAPR,	"ucon64 -mapr", TEST_TODO},
      {UCON64_MD5,	"ucon64 -md5", TEST_TODO},
      {UCON64_MGD,	"ucon64 -mgd", TEST_TODO},
      {UCON64_MGDGG,	"ucon64 -mgdgg", TEST_TODO},
      {UCON64_MIRR,	"ucon64 -mirr", TEST_TODO},
      {UCON64_MKA,	"ucon64 -mka", TEST_TODO},
      {UCON64_MKDAT,	"ucon64 -mkdat", TEST_TODO},
      {UCON64_MKI,	"ucon64 -mki=/tmp/test/test.txt /tmp/test/test2.txt;"
                        "ucon64 -crc test2.ips;"
                        "rm test2.ips", 0xe2b26d35},
      {UCON64_MKIP,	"ucon64 -mkip", TEST_TODO},
      {UCON64_MKPPF,	"ucon64 -mkppf", TEST_TODO},
      {UCON64_MSG,	"ucon64 -msg", TEST_TODO},
      {UCON64_MULTI,	"ucon64 -multi", TEST_TODO},
      {UCON64_N,	"ucon64 -n", TEST_TODO},
      {UCON64_N2,	"ucon64 -n2", TEST_TODO},
      {UCON64_N2GB,	"ucon64 -n2gb", TEST_TODO},
      {UCON64_N64,	"ucon64 -n64 /tmp/test/test.1mb", 0x5eedaf08},
      {UCON64_NA,	"ucon64 -na", TEST_TODO},
      {UCON64_NBAT,	"ucon64 -nbat", TEST_TODO},
      {UCON64_NBS,	"ucon64 -nbs", TEST_TODO},
      {UCON64_NCOL,	"ucon64 -ncol -snes /tmp/test/test.1mb", 0xf3091231},
      {UCON64_NDS,	"ucon64 -nds", TEST_TODO},
      {UCON64_NES,	"ucon64 -nes", TEST_TODO},
      {UCON64_NG,	"ucon64 -ng", TEST_TODO},
      {UCON64_NGP,	"ucon64 -ngp", TEST_TODO},
      {UCON64_NHD,	"ucon64 -nhd /tmp/test/test.smc", 0x1a5cc5d4},
      {UCON64_NHI,	"ucon64 -snes -nhi /tmp/test/test.1mb", 0xf3091231},
      {UCON64_NINT,	"ucon64 -nint", TEST_TODO},
      {UCON64_NPPF,	"ucon64 -nppf", TEST_TODO},
      {UCON64_NROT,	"ucon64 -nrot", TEST_TODO},
      {UCON64_NS,	"ucon64 -ns", TEST_TODO},
      {UCON64_NTSC,	"ucon64 -ntsc", TEST_TODO},
      {UCON64_NVRAM,	"ucon64 -nvram", TEST_TODO},
      {UCON64_P,	"ucon64 -p", TEST_TODO},
      {UCON64_PAD,	"ucon64 -pad", TEST_TODO},
      {UCON64_PADN,	"ucon64 -padn", TEST_TODO},
      {UCON64_PAL,	"ucon64 -pal", TEST_TODO},
      {UCON64_PARSE,	"ucon64 -parse", TEST_TODO},
      {UCON64_PASOFAMI,	"ucon64 -pasofami", TEST_TODO},
      {UCON64_PATCH,	"ucon64 -patch", TEST_TODO},
      {UCON64_PATTERN,	"ucon64 -pattern", TEST_TODO},
      {UCON64_PCE,	"ucon64 -pce", TEST_TODO},
      {UCON64_POKE,	"ucon64 -poke", TEST_TODO},
      {UCON64_PPF,	"ucon64 -ppf", TEST_TODO},
      {UCON64_PRINT,	"ucon64 -print /tmp/test/test.txt", 0x5c4acd52},
      {UCON64_PSX,	"ucon64 -psx /tmp/test/test.1mb", 0x79b34e40},
      {UCON64_R83,      "cp /tmp/test/1234567890.abcd .;"
                        "ucon64 -r83 1234567890.abcd;"
                        "rm 12345eeb.abc", 0x1f791880},
      {UCON64_RDAT,	"ucon64 -rdat", TEST_TODO},
      {UCON64_REGION,	"ucon64 -region", TEST_TODO},
      {UCON64_RJOLIET,  "cp /tmp/test/1234567890123456789012345678901234567890123456789012345678901234567890.abcd .;"
                        "ucon64 -rjoliet 1234567890123456789012345678901234567890123456789012345678901234567890.abcd;"
                        "rm 123456789012345678901234567890123456789012345678901234566f5.abcd", 0xd5cfab05},
      {UCON64_RL,       "cp /tmp/test/12345678.ABC .;"
                        "ucon64 -rl 12345678.ABC;"
                        "rm 12345678.abc", 0x48934d06},
      {UCON64_ROTL,	"ucon64 -rotl", TEST_TODO},
      {UCON64_ROTR,	"ucon64 -rotr", TEST_TODO},
      {UCON64_RROM,	"cp /tmp/test/test.smc .;"
                        "ucon64 -rrom test.smc;"
                        "rm \"Mode 7 interactive de.smc\"", 0xaaa714b6},
      {UCON64_RU,       "cp /tmp/test/12345678.abc .;"
                        "ucon64 -ru 12345678.abc;"
                        "rm 12345678.ABC", 0x0c9f305f},
      {UCON64_S,	"ucon64 -s", TEST_TODO},
      {UCON64_SAM,	"ucon64 -sam", TEST_TODO},
      {UCON64_SC,	"ucon64 -sc", TEST_TODO},
      {UCON64_SCAN,	"ucon64 -scan", TEST_TODO},
      {UCON64_SCR,	"ucon64 -scr", TEST_TODO},
      {UCON64_SGB,	"ucon64 -sgb", TEST_TODO},
      {UCON64_SHA1,	"ucon64 -sha1 /tmp/test/test.txt", 0x65608105},
      {UCON64_SMC,	"ucon64 -smc", TEST_TODO},
      {UCON64_SMD,	"ucon64 -smd", TEST_TODO},
      {UCON64_SMDS,	"ucon64 -smds", TEST_TODO},
      {UCON64_SMS,	"ucon64 -sms /tmp/test/test.1mb", 0x73996f1d},
      {UCON64_SNES,	"ucon64 -snes /tmp/test/test.1mb", 0xf3091231},
      {UCON64_SRAM,	"ucon64 -sram", TEST_TODO},
      {UCON64_SSC,	"ucon64 -ssc", TEST_TODO},
      {UCON64_SSIZE,	"ucon64 -ssize", TEST_TODO},
      {UCON64_STP,	"ucon64 -stp /tmp/test/test.64k;"
                        "ucon64 -crc test.64k;"
                        "rm test.64k", 0xe1a4cd85},
      {UCON64_STPN,	"ucon64 -stpn=512 /tmp/test/test.64k;"
                        "ucon64 -crc test.64k;"
                        "rm test.64k", 0xe1a4cd85},
      {UCON64_STRIP,	"ucon64 -strip=512 /tmp/test/test.64k;"
                        "ucon64 -crc test.64k;"
                        "rm test.64k", 0xe1a4cd85},
      {UCON64_SWAN,	"ucon64 -swan /tmp/test/test.1mb", 0x8e07f287},
      {UCON64_SWAP,	"ucon64 -swap /tmp/test/test.txt;"
                        "ucon64 -crc test.txt;"
                        "rm test.txt", 0xe60a7df5},
      {UCON64_SWAP2,	"ucon64 -swap2 /tmp/test/test.txt;"
                        "ucon64 -crc test.txt;"
                        "rm test.txt", 0xd5f4368d},
      {UCON64_SWC,	"ucon64 -swc /tmp/test/test.1mb;"
                        "ucon64 test.swc;"
                        "rm test.swc", 0xa9c3730f},
      {UCON64_SWCS,	"ucon64 -swcs", TEST_TODO},
      {UCON64_UFO,	"ucon64 -ufo", TEST_TODO},
      {UCON64_UFOS,	"ucon64 -ufos", TEST_TODO},
      {UCON64_UNIF,	"ucon64 -unif /tmp/test/test.nes", TEST_BUG},
      {UCON64_UNSCR,	"ucon64 -unscr", TEST_TODO},
      {UCON64_USMS,	"ucon64 -usms", TEST_TODO},
      {UCON64_V64,	"ucon64 -v64 /tmp/test/test.z64;"
                        "ucon64 test.v64;"
                        "rm test.v64", 1},
      {UCON64_VBOY,	"ucon64 -vboy /tmp/test/test.1mb", 0x87661ae8},
      {UCON64_VRAM,	"ucon64 -vram", TEST_TODO},
      {UCON64_Z64,	"ucon64 -z64 /tmp/test/test.v64;"
                        "ucon64 test.z64;"
                        "rm test.z64", 1},
TEST_BREAK
      {UCON64_VER,	"ucon64 -version", 0},  // NO TEST: changes always
      {UCON64_V,	"ucon64 -v", 0},        // NO TEST: verbose switch
      {UCON64_Q,	"ucon64 -q", 0},        // NO TEST: quiet switch
      {UCON64_HELP,	"ucon64 -help", 0},     // NO TEST: usage changes always
      {UCON64_R,	"ucon64 -r", 0},        // NO TEST: recursion
      {UCON64_O,        "ucon64 -o", 0},        // NO TEST: output
      {UCON64_NBAK,	"ucon64 -nbak", 0},     // NO TEST: no backup

      {UCON64_ROM,      "ucon64 -rom", 0},      // NO TEST: hidden option or deprecated
      {UCON64_3DO,	"ucon64 -3do", 0},      // NO TEST: hidden option or deprecated
      {UCON64_CRCHD,	"ucon64 -crchd /tmp/test/test.2kb", 0x707bbaf1}, // NO TEST: hidden option or deprecated
      {UCON64_FILE,	"ucon64 -file", 0},     // NO TEST: hidden option or deprecated
      {UCON64_FRONTEND,	"ucon64 -frontend", 0}, // NO TEST: hidden option or deprecated
      {UCON64_GC,	"ucon64 -gc", 0},       // NO TEST: hidden option or deprecated
      {UCON64_GP32,	"ucon64 -gp32", 0},     // NO TEST: hidden option or deprecated
      {UCON64_ID,	"ucon64 -id", 0},       // NO TEST: hidden option or deprecated
      {UCON64_INTELLI,	"ucon64 -intelli", 0},  // NO TEST: hidden option or deprecated
      {UCON64_PS2,	"ucon64 -ps2", 0},      // NO TEST: hidden option or deprecated
      {UCON64_S16,	"ucon64 -s16", 0},      // NO TEST: hidden option or deprecated
      {UCON64_SAT,	"ucon64 -sat", 0},      // NO TEST: hidden option or deprecated
      {UCON64_VEC,	"ucon64 -vec", 0},      // NO TEST: hidden option or deprecated
      {UCON64_XBOX,	"ucon64 -xbox", 0},     // NO TEST: hidden option or deprecated

      {UCON64_PORT,     "ucon64 -port", 0},     // NO TEST: transfer code
      {UCON64_XCMC,	"ucon64 -xcmc", 0},     // NO TEST: transfer code
      {UCON64_XCMCM,	"ucon64 -xcmcm", 0},    // NO TEST: transfer code
      {UCON64_XCMCT,	"ucon64 -xcmct", 0},    // NO TEST: transfer code
      {UCON64_XDEX,	"ucon64 -xdex", 0},     // NO TEST: transfer code
      {UCON64_XDJR,	"ucon64 -xdjr", 0},     // NO TEST: transfer code
      {UCON64_XF2A,	"ucon64 -xf2a", 0},     // NO TEST: transfer code
      {UCON64_XF2AB,	"ucon64 -xf2ab", 0},    // NO TEST: transfer code
      {UCON64_XF2AC,	"ucon64 -xf2ac", 0},    // NO TEST: transfer code
      {UCON64_XF2AMULTI,	"ucon64 -xf2amulti", 0}, // NO TEST: transfer code
      {UCON64_XF2AS,	"ucon64 -xf2as", 0},    // NO TEST: transfer code
      {UCON64_XFAL,	"ucon64 -xfal", 0},     // NO TEST: transfer code
      {UCON64_XFALB,	"ucon64 -xfalb", 0},    // NO TEST: transfer code
      {UCON64_XFALC,	"ucon64 -xfalc", 0},    // NO TEST: transfer code
      {UCON64_XFALM,	"ucon64 -xfalm", 0},    // NO TEST: transfer code
      {UCON64_XFALMULTI,	"ucon64 -xfalmulti", 0}, // NO TEST: transfer code
      {UCON64_XFALS,	"ucon64 -xfals", 0},    // NO TEST: transfer code
      {UCON64_XFIG,	"ucon64 -xfig", 0},     // NO TEST: transfer code
      {UCON64_XFIGC,	"ucon64 -xfigc", 0},    // NO TEST: transfer code
      {UCON64_XFIGS,	"ucon64 -xfigs", 0},    // NO TEST: transfer code
      {UCON64_XGBX,	"ucon64 -xgbx", 0},     // NO TEST: transfer code
      {UCON64_XGBXB,	"ucon64 -xgbxb", 0},    // NO TEST: transfer code
      {UCON64_XGBXM,	"ucon64 -xgbxm", 0},    // NO TEST: transfer code
      {UCON64_XGBXS,	"ucon64 -xgbxs", 0},    // NO TEST: transfer code
      {UCON64_XGD3,	"ucon64 -xgd3", 0},     // NO TEST: transfer code
      {UCON64_XGD3R,	"ucon64 -xgd3r", 0},    // NO TEST: transfer code
      {UCON64_XGD3S,	"ucon64 -xgd3s", 0},    // NO TEST: transfer code
      {UCON64_XGD6,	"ucon64 -xgd6", 0},     // NO TEST: transfer code
      {UCON64_XGD6R,	"ucon64 -xgd6r", 0},    // NO TEST: transfer code
      {UCON64_XGD6S,	"ucon64 -xgd6s", 0},    // NO TEST: transfer code
      {UCON64_XGG,	"ucon64 -xgg", 0},      // NO TEST: transfer code
      {UCON64_XGGB,	"ucon64 -xggb", 0},     // NO TEST: transfer code
      {UCON64_XGGS,	"ucon64 -xggs", 0},     // NO TEST: transfer code
      {UCON64_XLIT,	"ucon64 -xlit", 0},     // NO TEST: transfer code
      {UCON64_XMCCL,	"ucon64 -xmccl", 0},    // NO TEST: transfer code
      {UCON64_XMCD,	"ucon64 -xmcd", 0},     // NO TEST: transfer code
      {UCON64_XMD,	"ucon64 -xmd", 0},      // NO TEST: transfer code
      {UCON64_XMDB,	"ucon64 -xmdb", 0},     // NO TEST: transfer code
      {UCON64_XMDS,	"ucon64 -xmds", 0},     // NO TEST: transfer code
      {UCON64_XMSG,	"ucon64 -xmsg", 0},     // NO TEST: transfer code
      {UCON64_XPCE,	"ucon64 -xpce", 0},     // NO TEST: transfer code
      {UCON64_XPL,	"ucon64 -xpl", 0},      // NO TEST: transfer code
      {UCON64_XPLI,	"ucon64 -xpli", 0},     // NO TEST: transfer code
      {UCON64_XPLM,	"ucon64 -xplm", 0},     // NO TEST: transfer code
      {UCON64_XRESET,	"ucon64 -xreset", 0},   // NO TEST: transfer code
      {UCON64_XSF,	"ucon64 -xsf", 0},      // NO TEST: transfer code
      {UCON64_XSFS,	"ucon64 -xsfs", 0},     // NO TEST: transfer code
      {UCON64_XSMC,	"ucon64 -xsmc", 0},     // NO TEST: transfer code
      {UCON64_XSMCR,	"ucon64 -xsmcr", 0},    // NO TEST: transfer code
      {UCON64_XSMD,	"ucon64 -xsmd", 0},     // NO TEST: transfer code
      {UCON64_XSMDS,	"ucon64 -xsmds", 0},    // NO TEST: transfer code
      {UCON64_XSWC,	"ucon64 -xswc", 0},     // NO TEST: transfer code
      {UCON64_XSWC_IO,	"ucon64 -xswc-io", 0},  // NO TEST: transfer code
      {UCON64_XSWC2,	"ucon64 -xswc2", 0},    // NO TEST: transfer code
      {UCON64_XSWCC,	"ucon64 -xswcc", 0},    // NO TEST: transfer code
      {UCON64_XSWCR,	"ucon64 -xswcr", 0},    // NO TEST: transfer code
      {UCON64_XSWCS,	"ucon64 -xswcs", 0},    // NO TEST: transfer code
      {UCON64_XV64,	"ucon64 -xv64", 0},     // NO TEST: transfer code

      {0, NULL, 0}
    };
//  int x = 0;
//  unsigned int crc = 0;
//  char buf[MAXBUFSIZE], fname[FILENAME_MAX];

#ifdef  DEBUG
//#if 1
  // this is why no external script is used for testing
  {
    int c = 0, y = 0;
  
    // convert (st_getopt2_t **) to (st_getopt2_t *)
    memset (&options, 0, sizeof (st_getopt2_t) * UCON64_MAX_ARGS);
    for (c = x = 0; option[x]; x++)
      for (y = 0; option[x][y].name || option[x][y].help; y++)
        if (c < UCON64_MAX_ARGS)
          {
            memcpy (&options[c], &option[x][y], sizeof (st_getopt2_t));
            c++;
          }

    // do we test ALL options?
    for (x = 0; options[x].name || options[x].help; x++)
      if (options[x].val)
        {
          int found = 0;
          for (y = 0; test[y].val; y++)
            if (options[x].val == test[y].val)
              {
                found = 1;
                break;
              }
            
          if (!found)
            printf ("option: %4d \"%s\" will NOT be tested\n", options[x].val, options[x].name);
        }
  }              
#endif

  test (t);
}
#endif
