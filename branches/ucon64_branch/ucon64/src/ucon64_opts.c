/*
ucon64_opts.c - switch()'es for all uCON64 options

written by 2002 - 2003 NoisyB (noisyb@gmx.net)
           2002 - 2003 dbjh


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
#include <sys/stat.h>
#include <ctype.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "ucon64.h"
#include "ucon64_defines.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "ucon64_lib.h"
#include "ucon64_opts.h"
#include "quick_io.h"
#include "libdiscmage/libdiscmage.h"
#ifdef  GUI
#include "libnetgui/libnetgui.h"
#endif
#include "console/console.h"
#include "patch/patch.h"
#include "backup/backup.h"


int ucon64_parport_needed = 0;


int
ucon64_switches (int c, const char *optarg)
{
  char *ptr = NULL, *ptr2 = NULL, buf[MAXBUFSIZE];
  char buf2[MAXBUFSIZE];
  int x = 0;

  /*
    Handle options or switches that cause other _options_ to be ignored except
    other options of the same class (so the order in which they were specified
    matters).
    We have to do this here (not in ucon64_options()) or else other options
    might be executed before these.
  */
  switch (c)
    {
    /*
      Many tools ignore other options if --help has been specified. We do the
      same (compare with GNU tools).
    */
    case UCON64_HELP:
      ucon64_usage (ucon64.argc, ucon64.argv);
      exit (0);

#ifdef  GUI
    case UCON64_GUI:
      if (ucon64.netgui_enabled)
        if (optarg)
          ucon64.netgui = libng_open (optarg, NG_SERVER);
      exit (0);
#endif
    /*
      It's also common to exit after displaying version information.
      On some configurations printf is a macro (Red Hat Linux 6.2 + GCC 3.2),
      so we can't use preprocessor directives in the argument list.
    */
    case UCON64_VER:
#ifdef  DLOPEN
#define DISCMAGE_STATUS_MSG "discmage DLL:                      %s\n"
#else
#define DISCMAGE_STATUS_MSG "discmage DLL:                      %s, dynamically linked\n"
#endif

#ifdef  DLOPEN
#define NETGUI_STATUS_MSG "netgui DLL:                        %s\n"
#else
#define NETGUI_STATUS_MSG "netgui DLL:                        %s, dynamically linked\n"
#endif

#ifdef  WORDS_BIGENDIAN
#define ENDIANESS_STATUS "big"
#else
#define ENDIANESS_STATUS "little"
#endif

#ifdef  DEBUG
#define DEBUG_STATUS "yes"
#else
#define DEBUG_STATUS "no"
#endif

#ifdef  PARALLEL
#define PARALLEL_STATUS "yes"
#else
#define PARALLEL_STATUS "no"
#endif

#ifdef  ANSI_COLOR
#define ANSI_COLOR_STATUS "yes"
#else
#define ANSI_COLOR_STATUS "no"
#endif

#ifdef  HAVE_ZLIB_H
#define ZLIB_STATUS "yes"
#else
#define ZLIB_STATUS "no"
#endif

      ptr =
#ifdef  DLOPEN
        ucon64.discmage_path;
#else
#if     defined __MSDOS__
        "discmage.dxe";
#elif   defined __CYGWIN__ || defined _WIN32
        "discmage.dll";
#elif   defined __unix__ || defined __BEOS__
        "libdiscmage.so";
#else
        "unknown";
#endif
#endif // DLOPEN
      if (ucon64.discmage_enabled)
        {
          x = libdm_get_version();
          sprintf (buf, "%d.%d.%d", x >> 16, x >> 8, x);
        }
      else
        strcpy (buf, "not available");

      ptr2 =
#ifdef  DLOPEN
        ucon64.netgui_path;
#else
#if     defined __MSDOS__
        "netgui.dxe";
#elif   defined __CYGWIN__ || defined _WIN32
        "netgui.dll";
#elif   defined __unix__ || defined __BEOS__
        "libnetgui.so";
#else
        "unknown";
#endif
#endif // DLOPEN
      if (ucon64.netgui_enabled)
        {
#ifdef  GUI
          x = libng_get_version();
#else
          x = 0;
#endif                    
          sprintf (buf2, "%d.%d.%d", x >> 16, x >> 8, x);
        }
      else
        strcpy (buf2, "not available");

      printf ("version:                           %s (%s)\n"
              "platform:                          %s\n"
              "endianess:                         %s\n"
              "debug:                             %s\n"
              "parallel port backup unit support: %s\n"
              "ANSI colors enabled:               %s\n"
              "gzip and zip support:              %s\n"
              "configuration file %s  %s\n"
              DISCMAGE_STATUS_MSG
              "discmage enabled:                  %s\n"
              "discmage version:                  %s\n"
              NETGUI_STATUS_MSG
              "netgui enabled:                    %s\n"
              "netgui version:                    %s\n"
              "configuration directory:           %s\n"
              "DAT file directory:                %s\n"
              "entries in DATabase:               %d\n"
              "DATabase enabled:                  %s\n",
              UCON64_VERSION_S, __DATE__,
              CURRENT_OS_S,
              ENDIANESS_STATUS,
              DEBUG_STATUS,
              PARALLEL_STATUS,
              ANSI_COLOR_STATUS,
              ZLIB_STATUS,
              // display the existence only for the config file (really helps solving problems)
              access (ucon64.configfile, F_OK) ? "(not present):" : "(present):    ", ucon64.configfile,
              ptr,
              ucon64.discmage_enabled ? "yes" : "no",
              buf,
              ptr2,
              ucon64.netgui_enabled ? "yes" : "no",
              buf2,
              ucon64.configdir,
              ucon64.datdir,
              ucon64_dat_total_entries (),
              ucon64.dat_enabled ? "yes" : "no"
      );
      exit (0);
      break;

    case UCON64_FRONTEND:
      ucon64.frontend = 1;                      // used by ucon64_gauge()
      break;

    case UCON64_CRC:
      ucon64.crc_big_files = 1;
      break;

    case UCON64_NBAK:
      ucon64.backup = 0;
      break;

#ifdef  ANSI_COLOR
    case UCON64_NCOL:
      ucon64.ansi_color = 0;
      break;
#endif

    case UCON64_RIP:
    case UCON64_MKTOC:
    case UCON64_MKCUE:
    case UCON64_MKSHEET:
    case UCON64_BIN2ISO:
    case UCON64_ISOFIX:
    case UCON64_XCDRW:
    case UCON64_DISC:
      ucon64.force_disc = 1;
      break;

    case UCON64_NS:
      ucon64.split = 0;
      break;

    case UCON64_HD:
      ucon64.buheader_len = UNKNOWN_HEADER_LEN;
      break;

    case UCON64_HDN:
      ucon64.buheader_len = strtol (optarg, NULL, 10);
      break;

    case UCON64_NHD:
      ucon64.buheader_len = 0;
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

    case UCON64_PORT:
      if (optarg)
        sscanf (optarg, "%x", &ucon64.parport);
      break;

#ifdef  PARALLEL
    // We detect the presence of these options here so that we can drop privileges ASAP
    case UCON64_XDEX:
    case UCON64_XDJR:
    case UCON64_XFAL:
    case UCON64_XFALMULTI:
    case UCON64_XFALC:
    case UCON64_XFALS:
    case UCON64_XFALB:
    case UCON64_XGBX:
    case UCON64_XGBXS:
    case UCON64_XGBXB:
    case UCON64_XGD3:
    case UCON64_XLIT:
    case UCON64_XMCCL:
    case UCON64_XSMD:
    case UCON64_XSMDS:
    case UCON64_XSWC:
    case UCON64_XSWC2:
    case UCON64_XSWCS:
    case UCON64_XV64:
      /*
        We want to make this possible:
          1.) ucon64 <transfer option> <rom>
          2.) ucon64 <transfer option> <rom> <parallel port address>
          3.) ucon64 <transfer option> <rom> --port=<parallel port address>
      */
      if (!ucon64.parport)
        if (ucon64.argc >= 4)
          if (access (ucon64.argv[ucon64.argc - 1], F_OK))
            // Yes, we don't get here if ucon64.argv[ucon64.argc - 1] is [0x]278,
            //  [0x]378 or [0x]3bc and a file with the same name (path) exists.
            ucon64.parport = strtol (ucon64.argv[ucon64.argc - 1], NULL, 16);
      ucon64_parport_needed = 1;
      break;

    case UCON64_XFALM:
      ucon64.parport_mode = UCON64_EPP;
      break;
#endif // PARALLEL

    case UCON64_PATCH:
#if 0 // falling through, so --patch is an alias for --file
      if (optarg)
        ucon64.file = optarg;
      break;
#endif
    case UCON64_FILE:
      if (optarg)
        ucon64.file = optarg;
      break;

    case UCON64_I:
    case UCON64_B:
    case UCON64_A:
    case UCON64_NA:
    case UCON64_PPF:
    case UCON64_NPPF:
    case UCON64_IDPPF:
      if (!ucon64.file || !ucon64.file[0])
        ucon64.file = ucon64.argv[ucon64.argc - 1];
      break;

    case UCON64_ROM:
#if 0
      if (optarg)
        ucon64.rom = optarg;
#endif
      break;

    case UCON64_O:
      if (optarg)
        {
          struct stat fstate;
          int dir = 0;

          if (!stat (optarg, &fstate))
            if (S_ISDIR (fstate.st_mode))
              {
                strcpy (ucon64.output_path, optarg);
                if (OFFSET (ucon64.output_path, strlen (ucon64.output_path) - 1)
                      != FILE_SEPARATOR)
                  strcat (ucon64.output_path, FILE_SEPARATOR_S);
                dir = 1;
              }

          if (!dir)
            printf ("WARNING: Argument for -o must be a directory\n"
                    "         Using current directory instead\n");
        }
      break;

    case UCON64_NHI:
      ucon64.snes_hirom = 0;
      break;

    case UCON64_HI:
      ucon64.snes_hirom = 1;
      break;

    case UCON64_BS:
      ucon64.bs_dump = 1;
      break;

    case UCON64_NBS:
      ucon64.bs_dump = 0;
      break;

    case UCON64_CTRL:
      if (UCON64_ISSET (ucon64.controller))
        ucon64.controller |= 1 << strtol (optarg, NULL, 10);
      else
        ucon64.controller = 1 << strtol (optarg, NULL, 10);
      break;

    case UCON64_CTRL2:
      if (UCON64_ISSET (ucon64.controller2))
        ucon64.controller2 |= 1 << strtol (optarg, NULL, 10);
      else
        ucon64.controller2 = 1 << strtol (optarg, NULL, 10);
      break;

    case UCON64_NTSC:
      ucon64.tv_standard = 0;
      break;

    case UCON64_PAL:
      ucon64.tv_standard = 1;
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
      ucon64.mirror = strtol (optarg, NULL, 10);
      break;

    case UCON64_MAPR:
      ucon64.mapr = optarg;                     // pass the _string_, it can be a
      break;                                    //  board name

    case UCON64_CMNT:
      ucon64.comment = optarg;
      break;

    case UCON64_DUMPINFO:
      if (optarg)
        {
          ucon64.use_dump_info = 1;
          ucon64.dump_info = optarg;
        }
      break;

    case UCON64_83:
    case UCON64_RR83:
      ucon64.fname_len = UCON64_83;
      break;

    case UCON64_FORCE63:
      ucon64.fname_len = UCON64_FORCE63;
      break;

    case UCON64_Q:
    case UCON64_QQ:                             // for now -qq is equivalent to -q
      ucon64.quiet = 1;
      break;

    case UCON64_V:
      ucon64.quiet = -1;
      break;

    case UCON64_SSIZE:
      ucon64.part_size = strtol (optarg, NULL, 10) * MBIT;
      break;

    default:
      break;
    }

  return 0;
}


static int
ucon64_rename (int mode)
{
  char buf[FILENAME_MAX + 1], buf2[FILENAME_MAX + 1], suffix[80], *p = NULL;
  int good_name;

  buf[0] = 0;
  strncpy (suffix, get_suffix (ucon64.rom), 80);
  suffix[80 - 1] = 0;                           // in case suffix is >= 80 chars

  switch (mode)
    {
      case UCON64_RROM:
        if (ucon64.rominfo)
          if (ucon64.rominfo->name)
            strcpy (buf, strtrim (ucon64.rominfo->name));
        break;

      case UCON64_RENAME:                       // GoodXXXX style rename
        if (ucon64.dat)
          if (ucon64.dat->fname)
            {
              p = (char *) get_suffix (ucon64.dat->fname);
              strcpy (buf, ucon64.dat->fname);

              // get_suffix() never returns NULL
              if (p[0])
                if (strlen (p) < 5)
                  if (!(stricmp (p, ".nes") &&  // NES
                        stricmp (p, ".fds") &&  // NES FDS
//                        stricmp (p, ".smd") &&  // Genesis
                        stricmp (p, ".gb") &&   // Game Boy
                        stricmp (p, ".gbc") &&  // Game Boy Color
                        stricmp (p, ".gba") &&  // Game Boy Advance
                        stricmp (p, ".smc") &&  // SNES
                        stricmp (p, ".v64")))   // Nintendo 64
                    buf[strlen (buf) - strlen (p)] = 0;
            }
        break;

      default:
        return 0;                               // invalid mode
    }

  if (!buf[0])
    return 0;

  if (ucon64.fname_len == UCON64_FORCE63)
    buf[63] = 0;
  else if (ucon64.fname_len == UCON64_83)
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

  if (ucon64.fname_len == UCON64_83)
    buf2[12] = 0;

  ucon64_output_fname (buf2, OF_FORCE_BASENAME | OF_FORCE_SUFFIX);

  if (one_file (ucon64.rom, buf2))
    {
      printf ("Skipping \"%s\"\n", basename (ucon64.rom));
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
      ucon64_file_handler (buf2, NULL, OF_FORCE_BASENAME);

  if (!good_name)
    printf ("Renaming \"%s\" to \"%s\"\n", basename2 (ucon64.rom), basename2 (buf2));
  else
    printf ("Moving \"%s\"\n", basename2 (ucon64.rom));
#ifndef DEBUG
  rename (ucon64.rom, buf2);
#endif
#ifdef  HAVE_ZLIB_H
  unzip_current_file_nr = 0x7fffffff - 1;       // dirty hack, will be removed later
#endif
  return 0;
}


static int
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


int
ucon64_options (int c, const char *optarg)
{
#ifdef  PARALLEL
  int enableRTS = -1;                           // for UCON64_XSWC & UCON64_XSWC2
#endif
  int value = 0, x = 0, result = 0, padded;
  char buf[MAXBUFSIZE], src_name[FILENAME_MAX], dest_name[FILENAME_MAX],
       *p = NULL;
  struct stat fstate;

  if (ucon64.rom)
    {
      strcpy (src_name, ucon64.rom);
      strcpy (dest_name, ucon64.rom);
    }

  switch (c)
    {
    case UCON64_CRCHD:                          // deprecated
      value = UNKNOWN_HEADER_LEN;
    case UCON64_CRC:
      if (!value)
        value = ucon64.rominfo ? ucon64.rominfo->buheader_len : ucon64.buheader_len;
      printf ("%s", basename2 (ucon64.rom));
      if (ucon64.fname_arch[0])
        printf (" (%s)\n", basename2 (ucon64.fname_arch));
      else
        fputc ('\n', stdout);
      printf ("Checksum (CRC32): 0x%08x\n\n", q_fcrc32 (ucon64.rom, value));
      break;

    case UCON64_RL:
      strcpy (buf, ucon64.rom);
      strlwr (basename2 (buf));
      rename (ucon64.rom, buf);
      strcpy ((char *) ucon64.rom, buf);
      break;

    case UCON64_RU:
      strcpy (buf, ucon64.rom);
      strupr (basename2 (buf));
      rename (ucon64.rom, buf);
      strcpy ((char *) ucon64.rom, buf);
      break;

    case UCON64_HEX:
      ucon64_fhexdump (ucon64.rom, 0, ucon64.file_size);
      break;

    case UCON64_C:
    case UCON64_CS:
      result = -1;
      x = (c == UCON64_C ? FALSE : TRUE);
      if (optarg)
        result = ucon64_filefile (optarg, 0, ucon64.rom, 0, x);

      if (result == -1)
        {
          fprintf (stderr, "ERROR: File not found/out of memory\n");
          return -1;                            // it's logical to stop for this file
        }
      else if (result == -2)
        printf ("%s and %s refer to one file\n", optarg, ucon64.rom);
      else if (result >= 0)
        printf ("Found %d %s\n", result, x ? (result == 1 ? "similarity" : "similarities") :
                                             (result == 1 ? "difference" : "differences"));
      break;

    case UCON64_FIND:
      if (optarg)
        printf ("Searching: \"%s\"\n\n", optarg); // TODO: display "b?a" as "b" "a"
      else
        break; // empty search string

      while ((value = q_fncmp (ucon64.rom, value, ucon64.file_size, optarg,
                              strlen (optarg), '?')) != -1)
        {
          ucon64_fhexdump (ucon64.rom, value, strlen (optarg) + 16);
          printf ("\n");                        // + 16 gives a bit of context
          value++;
        }
      break;

    case UCON64_PADHD:                          // deprecated
      value = UNKNOWN_HEADER_LEN;
    case UCON64_P:
    case UCON64_PAD:
      if (!value && ucon64.rominfo)
        value = ucon64.rominfo->buheader_len;
      ucon64_file_handler (dest_name, src_name, 0);

      q_fcpy (src_name, 0, ucon64.file_size, dest_name, "wb");
      if (truncate2 (dest_name, ucon64.file_size + (MBIT - ((ucon64.file_size - value) % MBIT))) == -1)
        {
          fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name); // msg is not a typo
          exit (1);
        }

      printf (ucon64_msg[WROTE], dest_name);
      remove_temp_file ();
      break;

    case UCON64_PADN:
      ucon64_file_handler (dest_name, src_name, 0);

      q_fcpy (src_name, 0, ucon64.file_size, dest_name, "wb");
      if (truncate2 (dest_name, strtol (optarg, NULL, 10) +
            (ucon64.rominfo ? ucon64.rominfo->buheader_len : 0)) == -1)
        {
          fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name); // msg is not a typo
          exit (1);
        }

      printf (ucon64_msg[WROTE], dest_name);
      remove_temp_file ();
      break;

    case UCON64_ISPAD:
      if ((padded = ucon64_testpad (ucon64.rom)) != -1)
        {
          if (!padded)
            printf ("Padded: No\n\n");
          else
            printf ("Padded: Maybe, %d Bytes (%.4f Mb)\n\n", padded,
                    (float) padded / MBIT);
        }
      break;

    case UCON64_STRIP:
      ucon64_file_handler (dest_name, src_name, 0);
      q_fcpy (src_name, 0, ucon64.file_size - strtol (optarg, NULL, 10),
        dest_name, "wb");
      printf (ucon64_msg[WROTE], dest_name);
      remove_temp_file ();
      break;

    case UCON64_STP:
      ucon64_file_handler (dest_name, src_name, 0);
      q_fcpy (src_name, 512, ucon64.file_size, dest_name, "wb");
      printf (ucon64_msg[WROTE], dest_name);
      remove_temp_file ();
      break;

    case UCON64_STPN:
      ucon64_file_handler (dest_name, src_name, 0);
      q_fcpy (src_name, strtol (optarg, NULL, 10), ucon64.file_size, dest_name, "wb");
      printf (ucon64_msg[WROTE], dest_name);
      remove_temp_file ();
      break;

    case UCON64_INS:
      ucon64_file_handler (dest_name, src_name, 0);
      memset (buf, 0, 512);
      q_fwrite (buf, 0, 512, dest_name, "wb");
      q_fcpy (src_name, 0, ucon64.file_size, dest_name, "ab");
      printf (ucon64_msg[WROTE], dest_name);
      remove_temp_file ();
      break;

    case UCON64_INSN:
      ucon64_file_handler (dest_name, src_name, 0);
      value = strtol (optarg, NULL, 10);
      if (value <= MAXBUFSIZE)
        {
          memset (buf, 0, value);
          q_fwrite (buf, 0, value, dest_name, "wb");
        }
      else
        {
          int bytesleft = value, bytestowrite;
          memset (buf, 0, MAXBUFSIZE);
          while (bytesleft > 0)
            {
              bytestowrite = bytesleft <= MAXBUFSIZE ? bytesleft : MAXBUFSIZE;
              q_fwrite (buf, 0, bytestowrite, dest_name,
                bytesleft == value ? "wb" : "ab"); // we have to use "wb" for
              bytesleft -= bytestowrite;           //  the first iteration
            }
        }
      q_fcpy (src_name, 0, ucon64.file_size, dest_name, "ab");
      printf (ucon64_msg[WROTE], dest_name);
      remove_temp_file ();
      break;

    case UCON64_A:
      aps_apply (ucon64.rom, ucon64.file);
      break;

    case UCON64_B:
      bsl_apply (ucon64.rom, ucon64.file);
      break;

    case UCON64_I:
      ips_apply (ucon64.rom, ucon64.file);
      break;

    case UCON64_PPF:
      ppf_apply (ucon64.rom, ucon64.file);
      break;

    case UCON64_MKA:
      aps_create (optarg, ucon64.rom);          // original, modified
      break;

    case UCON64_MKI:
      ips_create (optarg, ucon64.rom);          // original, modified
      break;

    case UCON64_MKPPF:
      ppf_create (optarg, ucon64.rom);          // original, modified
      break;

    case UCON64_NA:
      aps_set_desc (ucon64.rom, optarg);
      break;

    case UCON64_NPPF:
      ppf_set_desc (ucon64.rom, optarg);
      break;

    case UCON64_IDPPF:
      ppf_set_fid (ucon64.rom, optarg);
      break;

    case UCON64_SCAN:
    case UCON64_LSD:
      if (ucon64.dat_enabled)
        {
          if (ucon64.crc32)
            {
              printf ("%s", basename2 (ucon64.rom));
              if (ucon64.fname_arch[0])
                printf (" (%s)\n", basename2 (ucon64.fname_arch));
              else
                fputc ('\n', stdout);
              // Use ucon64.fcrc32 for SNES & Genesis interleaved/N64 non-interleaved
              printf ("Checksum (CRC32): 0x%08x\n", ucon64.fcrc32 ?
                      ucon64.fcrc32 : ucon64.crc32);
              ucon64_dat_nfo (ucon64.dat, 1);
              printf ("\n");
            }
        }
      else
        printf (ucon64_msg[DAT_NOT_ENABLED]);
      break;

    case UCON64_LSV:
      if (ucon64.rominfo)
        ucon64_nfo ();
      break;

    case UCON64_LS:
      if (ucon64.rominfo)
        p = ucon64.rominfo->name;

      if (ucon64.dat)
        {
          if (!p)
            p = ucon64.dat->name;
          else if (!p[0])
            p = ucon64.dat->name;
        }

      if (p)
        if (p[0])
          {
            if (stat (ucon64.rom, &fstate) != 0)
              break;
            strftime (buf, 13, "%b %d %Y", localtime (&fstate.st_mtime));
            printf ("%-31.31s ", to_func (p, strlen (p), toprint2));

            printf ("%10d %s %s", ucon64.file_size, buf, basename2 (ucon64.rom));
            if (ucon64.fname_arch[0])
              printf (" (%s)\n", basename2 (ucon64.fname_arch));
            else
              fputc ('\n', stdout);
          }
      break;

    case UCON64_RENAME:
      ucon64_rename (UCON64_RENAME);
      break;

    case UCON64_RR83:
      ucon64.fname_len = UCON64_83;
    case UCON64_RROM:
      ucon64_rename (UCON64_RROM);
      break;

    case UCON64_BIN2ISO:
    case UCON64_ISOFIX:
    case UCON64_RIP:
      if (ucon64.discmage_enabled)
        {
          uint32_t flags = 0;

          switch (c)
            {
              case UCON64_BIN2ISO:
                flags |= DM_2048; // DM_RDONLY|DM_2048 read sectors and convert to 2048 Bytes
                break;

              case UCON64_ISOFIX:
                flags |= DM_FIX; // DM_RDONLY|DM_FIX read sectors and fix (if needed/possbile)
                break;
            }

          ucon64.image = libdm_reopen (ucon64.rom, DM_RDONLY, ucon64.image);
          if (ucon64.image)
            {
              int track = strtol (optarg, NULL, 10);
              if (track < 1)
                track = 1;

              libdm_set_gauge ((void (*)(int, int)) &libdm_gauge);
              libdm_rip (ucon64.image, track, flags);
              printf ("\n");
            }
        }
      else
        printf (ucon64_msg[NO_LIB], ucon64.discmage_path);
      break;

    case UCON64_MKTOC:
    case UCON64_MKCUE:
    case UCON64_MKSHEET:
      if (ucon64.discmage_enabled)
        {
          if (ucon64.image)
            {
              if (c == UCON64_MKTOC || c == UCON64_MKSHEET)
                {
                  if (!libdm_toc_write (ucon64.image))
                    printf (ucon64_msg[WROTE], "toc sheet");
                  else
                    fprintf (stderr, "ERROR: Could not generate toc sheet\n");
                }

              if (c == UCON64_MKCUE || c == UCON64_MKSHEET)
                {
                  if (!libdm_cue_write (ucon64.image))
                    printf (ucon64_msg[WROTE], "cue sheet");
                  else
                    fprintf (stderr, "ERROR: Could not generate cue sheet\n");
                }
            }
        }
      else
        printf (ucon64_msg[NO_LIB], ucon64.discmage_path);
      break;

    case UCON64_XCDRW:
      if (ucon64.discmage_enabled)
        {
//          libdm_set_gauge ((void *) &libdm_gauge);
          if (!access (ucon64.rom, F_OK))
            libdm_disc_write (ucon64.image);
          else
            libdm_disc_read (ucon64.image);
        }
      else
        printf (ucon64_msg[NO_LIB], ucon64.discmage_path);
      break;

    case UCON64_DB:
      if (ucon64.quiet > -1) // -db + -v == -dbv
        {
          if (ucon64.dat_enabled)
            {
              ucon64_dat_view (ucon64.console, 0);
              printf ("TIP: %s " OPTION_LONG_S "db " OPTION_LONG_S "nes"
                      " would show only information about known NES ROMs\n\n",
                      basename (ucon64.argv[0]));
            }
          else
            printf (ucon64_msg[DAT_NOT_ENABLED]);
          break;
        }

    case UCON64_DBV:
      if (ucon64.dat_enabled)
        {
          ucon64_dat_view (ucon64.console, 1);
          printf ("TIP: %s " OPTION_LONG_S "dbv " OPTION_LONG_S "nes"
                  " would show only information about known NES ROMs\n\n",
                  basename (ucon64.argv[0]));
        }
      else
        printf (ucon64_msg[DAT_NOT_ENABLED]);
      break;

    case UCON64_DBS:
      if (ucon64.dat_enabled)
        {
          ucon64.crc32 = 0;
          sscanf (optarg, "%x", &ucon64.crc32);

          if (!(ucon64.dat = ucon64_dat_search (ucon64.crc32, NULL)))
            {
              printf (ucon64_msg[DAT_NOT_FOUND], ucon64.crc32);
              printf ("TIP: Be sure to install the right DAT files in %s\n", ucon64.datdir);
            }
          else
            {
              ucon64_dat_nfo (ucon64.dat, 1);
              printf ("\n"
                      "TIP: %s " OPTION_LONG_S "dbs" OPTARG_S "0x%08x " OPTION_LONG_S
                      "nes would search only for a NES ROM\n\n",
                      basename (ucon64.argv[0]), ucon64.crc32);
            }
        }
      else
        printf (ucon64_msg[DAT_NOT_ENABLED]);
      break;

    case UCON64_MULTI:
      gba_multi (strtol (optarg, NULL, 10) * MBIT, NULL);
      break;

    case UCON64_SWCS:
      snes_swcs ();
      break;

    case UCON64_FIGS:
      snes_figs ();
      break;

    case UCON64_UFOS:
      snes_ufos ();
      break;

    case UCON64_E:
      ucon64_e ();
      break;

    case UCON64_1991:
      genesis_1991 (ucon64.rominfo);
      break;

    case UCON64_B0:
      lynx_b0 (ucon64.rominfo, optarg);
      break;

    case UCON64_B1:
      lynx_b1 (ucon64.rominfo, optarg);
      break;

    case UCON64_BIOS:
      if (optarg)
        neogeo_bios (optarg);
      break;

    case UCON64_BOT:
      n64_bot (ucon64.rominfo, optarg);
      break;

    case UCON64_CHK:
      switch (ucon64.console)
        {
        case UCON64_GB:
          gameboy_chk (ucon64.rominfo);
          break;
        case UCON64_GBA:
          gba_chk (ucon64.rominfo);
          break;
        case UCON64_GEN:
          genesis_chk (ucon64.rominfo);
          break;
        case UCON64_N64:
          n64_chk (ucon64.rominfo);
          break;
        case UCON64_SNES:
          snes_chk (ucon64.rominfo);
          break;
        case UCON64_SWAN:
          swan_chk (ucon64.rominfo);
          break;
        default:
// The next msg has already been printed
//          fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
          return -1;
        }
      break;

    case UCON64_COL:
      if (optarg)
        snes_col (optarg);
      break;

    case UCON64_CRP:
      gba_crp (ucon64.rominfo, optarg);
      break;

    case UCON64_DBUH:
      snes_buheader_info (ucon64.rominfo);
      break;

    case UCON64_SWAP:                           // deprecated
    case UCON64_DINT:
      switch (ucon64.console)
        {
        case UCON64_SNES:
          snes_dint (ucon64.rominfo);
          break;
        case UCON64_NES:
          nes_dint ();
          break;
        default:
          puts ("Converting to deinterleaved format...");
          ucon64_file_handler (dest_name, NULL, 0);
          q_fcpy (src_name, 0, ucon64.file_size, dest_name, "wb");
          q_fswap (dest_name, 0, ucon64.file_size);
          printf (ucon64_msg[WROTE], dest_name);
          break;
        }
      break;

    case UCON64_F:
      switch (ucon64.console)
        {
        case UCON64_N64:
          n64_f (ucon64.rominfo);
          break;
        case UCON64_SNES:
          snes_f (ucon64.rominfo);
          break;
        default:
// The next msg has already been printed
//          fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
          return -1;
        }
      break;

    case UCON64_FDS:
      nes_fds ();
      break;

    case UCON64_FDSL:
      nes_fdsl (ucon64.rominfo, NULL);
      break;

    case UCON64_FFE:
      nes_ffe (ucon64.rominfo);
      break;

    case UCON64_FIG:
      snes_fig (ucon64.rominfo);
      break;

    case UCON64_GBX:
      gameboy_gbx (ucon64.rominfo);
      break;

    case UCON64_GD3:
      snes_gd3 (ucon64.rominfo);
      break;

    case UCON64_GG:
      switch (ucon64.console)
        {
        case UCON64_GB:
        case UCON64_SMS:
        case UCON64_GEN:
        case UCON64_NES:
        case UCON64_SNES:
          gg_apply (ucon64.rominfo, optarg);
          break;
        default:
          fprintf (stderr, "ERROR: Can not apply Game Genie code for this ROM/console\n");
// The next msg has already been printed
//          fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
          return -1;
        }
      break;

    case UCON64_GGD:
      gg_display (ucon64.rominfo, optarg);
      break;

    case UCON64_GGE:
      gg_display (ucon64.rominfo, optarg);
      break;

    case UCON64_INES:
      nes_ines ();
      break;

    case UCON64_INESHD:
      nes_ineshd (ucon64.rominfo);
      break;

#if 0
        case UCON64_IP:
          break;
#endif

    case UCON64_J:
      switch (ucon64.console)
        {
        case UCON64_GEN:
          genesis_j (ucon64.rominfo);
          break;
        case UCON64_NES:
          nes_j (NULL);
          break;
        case UCON64_SNES:
          snes_j (ucon64.rominfo);
          break;
        default:
// The next msg has already been printed
//          fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
          return -1;
        }
      break;

    case UCON64_K:
      snes_k (ucon64.rominfo);
      break;

    case UCON64_L:
      snes_l (ucon64.rominfo);
      break;

    case UCON64_LNX:
      lynx_lnx (ucon64.rominfo);
      break;

    case UCON64_LOGO:
      gba_logo (ucon64.rominfo);
      break;

    case UCON64_LYX:
      lynx_lyx (ucon64.rominfo);
      break;

    case UCON64_MGD:
      switch (ucon64.console)
        {
        case UCON64_GB:
          gameboy_mgd (ucon64.rominfo);
          break;
        case UCON64_GEN:
          genesis_mgd (ucon64.rominfo);
          break;
        case UCON64_NG:
          neogeo_mgd ();
          break;
        case UCON64_SNES:
          snes_mgd (ucon64.rominfo);
          break;
        case UCON64_PCE:
          pcengine_mgd (ucon64.rominfo);
          break;
        default:
// The next msg has already been printed
//          fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
          return -1;
        }
      break;

    case UCON64_N:
//      if (strlen (optarg) == 0)
//        break;
      switch (ucon64.console)
        {
        case UCON64_GB:
          gameboy_n (ucon64.rominfo, optarg);
          break;
        case UCON64_GBA:
          gba_n (ucon64.rominfo, optarg);
          break;
        case UCON64_GEN:
          genesis_n (ucon64.rominfo, optarg);
          break;
        case UCON64_LYNX:
          lynx_n (ucon64.rominfo, optarg);
          break;
        case UCON64_N64:
          n64_n (ucon64.rominfo, optarg);
          break;
        case UCON64_NES:
          nes_n (optarg);
          break;
        case UCON64_SNES:
          snes_n (ucon64.rominfo, optarg);
          break;
        default:
// The next msg has already been printed
//          fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
          return -1;
        }
      break;

    case UCON64_N2:
//      if (strlen (optarg) == 0)
//        break;
      genesis_n2 (ucon64.rominfo, optarg);
      break;

    case UCON64_N2GB:
      gameboy_n2gb (optarg);
      break;

    case UCON64_NROT:
      lynx_nrot (ucon64.rominfo);
      break;

    case UCON64_PASOFAMI:
      nes_pasofami ();
      break;

    case UCON64_POKE:
      ucon64_file_handler (dest_name, src_name, 0);
      q_fcpy (src_name, 0, ucon64.file_size, dest_name, "wb");

      sscanf (optarg, "%x:%x", &x, &value);
      if (x >= ucon64.file_size)
        {
          fprintf (stderr, "ERROR: Offset 0x%x is too large\n", x);
          remove (dest_name);
          break;
        }
      printf ("\n");
      buf[0] = q_fgetc (dest_name, x);
      mem_hexdump (buf, 1, x);

      q_fputc (dest_name, x, value, "r+b");

      buf[0] = value;
      mem_hexdump (buf, 1, x);
      printf ("\n");

      printf (ucon64_msg[WROTE], dest_name);
      remove_temp_file ();
      break;

    case UCON64_ROTL:
      lynx_rotl (ucon64.rominfo);
      break;

    case UCON64_ROTR:
      lynx_rotr (ucon64.rominfo);
      break;

    case UCON64_S:
      switch (ucon64.console)
        {
        case UCON64_GEN:
          genesis_s (ucon64.rominfo);
          break;
        case UCON64_NG:
          neogeo_s ();
          break;
        case UCON64_NES:
          nes_s ();
          break;
        case UCON64_SNES:
          snes_s (ucon64.rominfo);
          break;
        default:
// The next msg has already been printed
//          fprintf (stderr, ucon64_msg[CONSOLE_ERROR]);
          return -1;
        }
      break;

    case UCON64_SAM:
      if (optarg)
        neogeo_sam (optarg);
      break;

    case UCON64_SGB:
      gameboy_sgb (ucon64.rominfo);
      break;

    case UCON64_SMC:
      snes_smc (ucon64.rominfo);
      break;

    case UCON64_SMD:
      genesis_smd (ucon64.rominfo);
      break;

    case UCON64_SMDS:
      genesis_smds ();
      break;

    case UCON64_SMG:
      pcengine_smg (ucon64.rominfo);
      break;

    case UCON64_SRAM:
      gba_sram ();
      break;

    case UCON64_LSRAM:
      n64_sram (ucon64.rominfo, optarg);
      break;

    case UCON64_SSC:
      gameboy_ssc (ucon64.rominfo);
      break;

    case UCON64_SWC:
      snes_swc (ucon64.rominfo);
      break;

    case UCON64_UNIF:
      nes_unif ();
      break;

    case UCON64_USMS:
      n64_usms (ucon64.rominfo, optarg);
      break;

    case UCON64_V64:
      n64_v64 (ucon64.rominfo);
      break;

#ifdef  PARALLEL
    /*
      It doesn't make sense to continue after executing a (send) backup option
      ("multizip"). Don't return, but use break instead. ucon64_execute_options()
      checks if an option was used that should stop uCON64.
    */
    case UCON64_XDEX:
      if (!access (ucon64.rom, F_OK))
        {
          if (dex_write_block (ucon64.rom, strtol (optarg, NULL, 10), ucon64.parport) != 0)
            fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
        }
      else
        {
          if (dex_read_block (ucon64.rom, strtol (optarg, NULL, 10), ucon64.parport) != 0)
            fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
        }
      printf ("\n");
      break;

    case UCON64_XDJR:
      if (!access (ucon64.rom, F_OK))
        {
          if (doctor64jr_write (ucon64.rom, ucon64.parport) != 0)
            fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
        }
      else
        {
          if (doctor64jr_read (ucon64.rom, ucon64.parport) != 0)
            fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
        }
      printf ("\n");
      break;

    case UCON64_XGD3:
      if (access (ucon64.rom, F_OK) != 0)       // file does not exist -> dump cartridge
        gd_read_rom (ucon64.rom, ucon64.parport); // dumping is not yet supported
      else
        {
          if (!ucon64.rominfo->buheader_len)
            fprintf (stderr,
                    "ERROR: This ROM has no header. Convert to a Game Doctor compatible format.\n");
          else
            gd_write_rom (ucon64.rom, ucon64.parport, ucon64.rominfo); // file exists -> send it to the copier
        }
      printf ("\n");
      break;

    case UCON64_XSMD:
      if (access (ucon64.rom, F_OK) != 0)       // file does not exist -> dump cartridge
        smd_read_rom (ucon64.rom, ucon64.parport);
      else                                      // file exists -> send it to the copier
        {
          if (!ucon64.rominfo->buheader_len)
            fprintf (stderr,
                    "ERROR: This ROM has no header. Convert to an SMD compatible format.\n");
          else if (!ucon64.rominfo->interleaved)
            fprintf (stderr,
                    "ERROR: This ROM doesn't seem to be interleaved but the SMD only supports\n"
                    "       interleaved ROMs. Convert to an SMD compatible format.\n");
          else
            smd_write_rom (ucon64.rom, ucon64.parport);
        }
      printf ("\n");
      break;

    case UCON64_XSMDS:
      if (access (ucon64.rom, F_OK) != 0)       // file does not exist -> dump SRAM contents
        smd_read_sram (ucon64.rom, ucon64.parport);
      else                                      // file exists -> restore SRAM
        smd_write_sram (ucon64.rom, ucon64.parport);
      printf ("\n");
      break;

    case UCON64_XSWC:
      enableRTS = 0;                            // falling through
    case UCON64_XSWC2:
      if (access (ucon64.rom, F_OK) != 0)       // file does not exist -> dump cartridge
        swc_read_rom (ucon64.rom, ucon64.parport);
      else
        {
          if (!ucon64.rominfo->buheader_len)
            fprintf (stderr,
                    "ERROR: This ROM has no header. Convert to an SWC compatible format.\n");
          else if (ucon64.rominfo->interleaved)
            fprintf (stderr,
                    "ERROR: This ROM seems to be interleaved but the SWC doesn't support\n"
                    "       interleaved ROMs. Convert to an SWC compatible format.\n");
          else
            {
              if (enableRTS != 0)
                enableRTS = 1;
              // file exists -> send it to the copier
              swc_write_rom (ucon64.rom, ucon64.parport, enableRTS);
            }
        }
      printf ("\n");
      break;

    case UCON64_XSWCS:
      if (access (ucon64.rom, F_OK) != 0)       // file does not exist -> dump SRAM contents
        swc_read_sram (ucon64.rom, ucon64.parport);
      else
        swc_write_sram (ucon64.rom, ucon64.parport); // file exists -> restore SRAM
      printf ("\n");
      break;

    case UCON64_XV64:
      if (!access (ucon64.rom, F_OK))
        {
          if (doctor64_write (ucon64.rom, ucon64.rominfo->buheader_len,
                              ucon64.file_size, ucon64.parport) != 0)
            fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
        }
      else
        {
          if (doctor64_read (ucon64.rom, ucon64.parport) != 0)
            fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
        }
      printf ("\n");
      break;

    case UCON64_XLIT:
#if 0
//   write does not exist for the current lynxit interface
      if (!access (ucon64.rom, F_OK))
        {
          if (lynxit_write_rom (ucon64.rom, ucon64.parport) != 0)
            fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
        }
      else
        {
#endif
          if (lynxit_read_rom (ucon64.rom, ucon64.parport) != 0)
            fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
//        }
      printf ("\n");
      break;

    case UCON64_XFAL:
      if (access (ucon64.rom, F_OK) != 0)
        fal_read_rom (ucon64.rom, ucon64.parport, UCON64_UNKNOWN);
      else
        fal_write_rom (ucon64.rom, ucon64.parport);
      printf ("\n");
      break;

    case UCON64_XFALMULTI:
      tmpnam2 (src_name);
      ucon64_temp_file = src_name;
      register_func (remove_temp_file);
      gba_multi (strtol (optarg, NULL, 10) * MBIT, src_name);
      fal_write_rom (src_name, ucon64.parport);
      unregister_func (remove_temp_file);
      remove_temp_file ();
      printf ("\n");
      break;

    case UCON64_XFALC:
      fal_read_rom (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
      printf ("\n");
      break;

    case UCON64_XFALS:
      if (access (ucon64.rom, F_OK) != 0)
        fal_read_sram (ucon64.rom, ucon64.parport, UCON64_UNKNOWN);
      else
        fal_write_sram (ucon64.rom, ucon64.parport, UCON64_UNKNOWN);
      printf ("\n");
      break;

    case UCON64_XFALB:
      if (access (ucon64.rom, F_OK) != 0)
        fal_read_sram (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
      else
        fal_write_sram (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
      printf ("\n");
      break;

    case UCON64_XMCCL:
      mccl_read (ucon64.rom, ucon64.parport);
      printf ("\n");
      break;

    case UCON64_XGBX:
      if (access (ucon64.rom, F_OK) != 0)       // file does not exist -> dump cartridge
        gbx_read_rom (ucon64.rom, ucon64.parport);
      else                                      // file exists -> send it to the copier
        gbx_write_rom (ucon64.rom, ucon64.parport);
      printf ("\n");
      break;

    case UCON64_XGBXS:
      if (access (ucon64.rom, F_OK) != 0)
        gbx_read_sram (ucon64.rom, ucon64.parport, -1);
      else
        gbx_write_sram (ucon64.rom, ucon64.parport, -1);
      printf ("\n");
      break;

    case UCON64_XGBXB:
      if (access (ucon64.rom, F_OK) != 0)
        gbx_read_sram (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
      else
        gbx_write_sram (ucon64.rom, ucon64.parport, strtol (optarg, NULL, 10));
      printf ("\n");
      break;

#endif // PARALLEL

    case UCON64_Z64:
      n64_z64 (ucon64.rominfo);
      break;

      default:
      break;
    }

  return 0;
}
