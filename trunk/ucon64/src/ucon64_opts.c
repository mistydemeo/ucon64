/*
ucon64_opts.c - switch()'es for all uCON64 options

Copyright (c) 2002 - 2005 NoisyB
Copyright (c) 2002 - 2005 dbjh
Copyright (c) 2005        Jan-Erik Karlsson (Amiga)


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
#include <time.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/itypes.h"
#include "misc/misc.h"
#include "misc/getopt2.h"
#include "misc/file.h"
#include "misc/string.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ucon64_dat.h"
#include "ucon64_opts.h"
#include "console/console.h"
#include "patch/patch.h"
#include "backup/backup.h"
#include "misc/hash.h"
#ifdef  USE_PARALLEL
#include "misc/parallel.h"
#endif


static long int
strtol2 (const char *str, char **tail)
{
  long int i;
#warning make more intelligent (hex)
  return (i = strtol (str, tail, 10)) ? i : strtol (str, tail, 16);
}


static char *
to_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = func (*p);

  return s;
}


static int
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


int
ucon64_switches (st_ucon64_t *p)
{
  int x = 0;

  /*
    Handle options or switches that cause other _options_ to be ignored except
    other options of the same class (so the order in which they were specified
    matters).
    We have to do this here (not in ucon64_options()) or else other options
    might be executed before these.
  */
  switch (p->option)
    {
    /*
      Many tools ignore other options if --help has been specified. We do the
      same (compare with GNU tools).
    */
    case UCON64_HELP:
      x = USAGE_VIEW_LONG;
      if (p->optarg)
        {
          if (!strcmp (p->optarg, "pad"))
            x = USAGE_VIEW_PAD;
          else if (!strcmp (p->optarg, "dat"))
            x = USAGE_VIEW_DAT;
          else if (!strcmp (p->optarg, "patch"))
            x = USAGE_VIEW_PATCH;
          else if (!strcmp (p->optarg, "backup"))
            x = USAGE_VIEW_BACKUP;
          else if (!strcmp (p->optarg, "disc"))
            x = USAGE_VIEW_DISC;
        }
      ucon64_usage (ucon64.argc, ucon64.argv, x);
      exit (0);

    /*
      It's also common to exit after displaying version information.
      On some configurations printf is a macro (Red Hat Linux 6.2 + GCC 3.2),
      so we can't use preprocessor directives in the argument list.
    */
    case UCON64_VER:
      printf ("version:                           %s (%s)\n"
              "platform:                          %s\n",
              UCON64_VERSION_S, __DATE__,
              CURRENT_OS_S);

#ifdef  WORDS_BIGENDIAN
      puts ("endianess:                         big");
#else
      puts ("endianess:                         little");
#endif

#ifdef  DEBUG
      puts ("debug:                             yes");
#else
      puts ("debug:                             no");
#endif

#ifdef  USE_PARALLEL
#ifdef  USE_PPDEV
      puts ("parallel port backup unit support: yes (ppdev)");
#else
      puts ("parallel port backup unit support: yes");
#endif // USE_PPDEV
#else
      puts ("parallel port backup unit support: no");
#endif

#ifdef  USE_USB
      puts ("USB port backup unit support:      yes");
#else
      puts ("USB port backup unit support:      no");
#endif


#ifdef  USE_ANSI_COLOR
      puts ("ANSI colors enabled:               yes");
#else
      puts ("ANSI colors enabled:               no");
#endif

#ifdef  USE_ZLIB
      puts ("gzip and zip support:              yes");
#else
      puts ("gzip and zip support:              no");
#endif

      printf ("configuration file %s  %s\n",
              // display the existence only for the config file (really helps solving problems)
              access (ucon64.configfile, F_OK) ? "(not present):" : "(present):    ",
              ucon64.configfile);

      printf ("configuration directory:           %s\n"
              "DAT file directory:                %s\n"
              "entries in DATabase:               %d\n"
              "DATabase enabled:                  %s\n",
              ucon64.configdir,
              ucon64.datdir,
              ucon64_dat_total_entries (),
              ucon64.dat_enabled ? "yes" : "no");
      exit (0);
      break;

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
      ucon64.backup_header_len = UNKNOWN_BACKUP_HEADER_LEN;
      break;

    case UCON64_HDN:
      ucon64.backup_header_len = strtol (p->optarg, NULL, 10);
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

    case UCON64_PATCH: // --patch and --file are the same
    case UCON64_FILE:
      ucon64.file = p->optarg;
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

#if 0
    case UCON64_ROM:
      if (p->optarg)
        ucon64.fname = p->optarg;
      break;
#endif

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


#warning merge these funcs as much as possible with the funcs they call
static int
ucon64_opt_crc (st_ucon64_t *p)
{
  unsigned int checksum = 0;
  ucon64_chksum (NULL, NULL, &checksum, p->fname, 0);
  return 0;
}


static int
ucon64_opt_crchd (st_ucon64_t *p)                          // deprecated
{
  unsigned int checksum = 0;
  ucon64_chksum (NULL, NULL, &checksum, p->fname, UNKNOWN_BACKUP_HEADER_LEN);
  return 0;
}


static int
ucon64_opt_sha1 (st_ucon64_t *p)
{
  char buf[MAXBUFSIZE];

#warning buf?
  ucon64_chksum (buf, NULL, NULL, p->fname, 0);
  return 0;
}


static int
ucon64_opt_md5 (st_ucon64_t *p)
{
  char buf[MAXBUFSIZE];

#warning buf?
  ucon64_chksum (NULL, buf, NULL, p->fname, 0);
  return 0;
}


static int
ucon64_opt_hex (st_ucon64_t *p)
{
  ucon64_dump (stdout, p->fname,
               p->optarg ? MAX (strtol2 (p->optarg, NULL), 0) : 0,
               p->file_size, DUMPER_HEX);
  return 0;
}


static int
ucon64_opt_bits (st_ucon64_t *p)
{
  ucon64_dump (stdout, p->fname,
               p->optarg ? MAX (strtol2 (p->optarg, NULL), 0) : 0,
               p->file_size, DUMPER_BIT);
  return 0;
}


static int
ucon64_opt_code (st_ucon64_t *p)
{
  ucon64_dump (stdout, p->fname,
               p->optarg ? MAX (strtol2 (p->optarg, NULL), 0) : 0,
               p->file_size, DUMPER_CODE);
  return 0;
}


static int
ucon64_opt_print (st_ucon64_t *p)
{
  ucon64_dump (stdout, p->fname,
               p->optarg ? MAX (strtol2 (p->optarg, NULL), 0) : 0,
               p->file_size, DUMPER_TEXT);
  return 0;
}


static int
ucon64_opt_c (st_ucon64_t *p)
{
  ucon64_filefile (p->optarg, 0, p->fname, 0, FALSE);
  return 0;
}


static int
ucon64_opt_cs (st_ucon64_t *p)
{
  ucon64_filefile (p->optarg, 0, p->fname, 0, TRUE);
  return 0;
}


static int
ucon64_opt_find (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, p->file_size, p->optarg,
               strlen (p->optarg), MEMCMP2_WCARD ('?'), 0);
  return 0;
}


static int
ucon64_opt_findr (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, p->file_size, p->optarg,
               strlen (p->optarg), MEMCMP2_REL, 0);
  return 0;
}


static int
ucon64_opt_findi (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, p->file_size, p->optarg,
               strlen (p->optarg), MEMCMP2_WCARD ('?') | MEMCMP2_CASE, 0);
  return 0;
}


static int
ucon64_opt_hfind (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, p->file_size, p->optarg,
               strlen (p->optarg), MEMCMP2_WCARD ('?'), 1);
  return 0;
}


static int
ucon64_opt_hfindr (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, p->file_size, p->optarg,
               strlen (p->optarg), MEMCMP2_REL, 1);
  return 0;
}


static int
ucon64_opt_dfind (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, p->file_size, p->optarg,
               strlen (p->optarg), MEMCMP2_WCARD ('?'), 1);
  return 0;
}


static int
ucon64_opt_dfindr (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, p->file_size, p->optarg,
               strlen (p->optarg), MEMCMP2_REL, 1);
  return 0;
}


static int
ucon64_opt_pad (st_ucon64_t *p)
{
  int value = 0;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  if (p->nfo)
    value = p->nfo->backup_header_len;
  ucon64_file_handler (dest_name, src_name, 0);

  fcopy (src_name, 0, p->file_size, dest_name, "wb");
  if (truncate2 (dest_name, p->file_size + (MBIT - ((p->file_size - value) % MBIT))) == -1)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name); // msg is not a typo
      exit (1);
    }

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}


static int
ucon64_opt_padhd (st_ucon64_t *p)                         // deprecated
{
  int value = UNKNOWN_BACKUP_HEADER_LEN;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  if (p->nfo)
    value = p->nfo->backup_header_len;
  ucon64_file_handler (dest_name, src_name, 0);

  fcopy (src_name, 0, p->file_size, dest_name, "wb");
  if (truncate2 (dest_name, p->file_size + (MBIT - ((p->file_size - value) % MBIT))) == -1)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name); // msg is not a typo
      exit (1);
    }

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}


static int
ucon64_opt_p (st_ucon64_t *p)
{
  ucon64_opt_pad (p);
  return 0;
}


static int
ucon64_opt_padn (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);

  fcopy (src_name, 0, p->file_size, dest_name, "wb");
  if (truncate2 (dest_name, strtol (p->optarg, NULL, 10) +
        (p->nfo ? p->nfo->backup_header_len : 0)) == -1)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name); // msg is not a typo
      exit (1);
    }

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_opt_ispad (st_ucon64_t *p)
{
  int padded = 0;

  if ((padded = ucon64_testpad (p->fname)) != -1)
    {
      if (!padded)
        puts ("Padded: No\n");
      else
        printf ("Padded: Maybe, %d Bytes (%.4f Mb)\n\n", padded,
                (float) padded / MBIT);
    }
  return 0;
}


static int
ucon64_opt_strip (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, 0, p->file_size - strtol (p->optarg, NULL, 10),
    dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_opt_stp (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, 512, p->file_size, dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_opt_stpn (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, strtol (p->optarg, NULL, 10), p->file_size, dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_opt_ins (st_ucon64_t *p)
{
  char buf[MAXBUFSIZE];
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  memset (buf, 0, 512);
  ucon64_fwrite (buf, 0, 512, dest_name, "wb");
  fcopy (src_name, 0, p->file_size, dest_name, "ab");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_opt_insn (st_ucon64_t *p)
{
  int value = strtol (p->optarg, NULL, 10);
  char buf[MAXBUFSIZE];
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  if (value <= MAXBUFSIZE)
    {
      memset (buf, 0, value);
      ucon64_fwrite (buf, 0, value, dest_name, "wb");
    }
  else
    {
      int bytesleft = value, bytestowrite;
      memset (buf, 0, MAXBUFSIZE);
      while (bytesleft > 0)
        {
          bytestowrite = bytesleft <= MAXBUFSIZE ? bytesleft : MAXBUFSIZE;
          ucon64_fwrite (buf, 0, bytestowrite, dest_name,
            bytesleft == value ? "wb" : "ab"); // we have to use "wb" for
          bytesleft -= bytestowrite;           //  the first iteration
        }
    }
  fcopy (src_name, 0, p->file_size, dest_name, "ab");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_opt_a (st_ucon64_t *p)
{
  aps_apply (p->fname, p->file);
  return 0;
}


static int
ucon64_opt_b (st_ucon64_t *p)
{
  bsl_apply (p->fname, p->file);
  return 0;
}


static int
ucon64_opt_i (st_ucon64_t *p)
{
  ips_apply (p->fname, p->file);
  return 0;
}


static int
ucon64_opt_ppf (st_ucon64_t *p)
{
  ppf_apply (p->fname, p->file);
  return 0;
}


static int
ucon64_opt_mka (st_ucon64_t *p)
{
  aps_create (p->optarg, p->fname);          // original, modified
  return 0;
}


static int
ucon64_opt_mki (st_ucon64_t *p)
{
  ips_create (p->optarg, p->fname);          // original, modified
  return 0;
}


static int
ucon64_opt_mkppf (st_ucon64_t *p)
{
  ppf_create (p->optarg, p->fname);          // original, modified
  return 0;
}


static int
ucon64_opt_na (st_ucon64_t *p)
{
  aps_set_desc (p->fname, p->optarg);
  return 0;
}


static int
ucon64_opt_nppf (st_ucon64_t *p)
{
  ppf_set_desc (p->fname, p->optarg);
  return 0;
}


static int
ucon64_opt_idppf (st_ucon64_t *p)
{
  ppf_set_fid (p->fname, p->optarg);
  return 0;
}


static int
ucon64_opt_lsd (st_ucon64_t *p)
{
  if (p->dat_enabled)
    {
      if (p->crc32)
        {
          fputs (basename2 (p->fname), stdout);
          if (p->fname_arch[0])
            printf (" (%s)\n", basename2 (p->fname_arch));
          else
            fputc ('\n', stdout);
          // Use p->fcrc32 for SNES & Genesis interleaved/N64 non-interleaved
          printf ("Checksum (CRC32): 0x%08x\n", p->fcrc32 ?
                  p->fcrc32 : p->crc32);
          ucon64_dat_nfo ((st_ucon64_dat_t *) p->dat, 1);
          fputc ('\n', stdout);
        }
    }
  else
    printf (ucon64_msg[DAT_NOT_ENABLED]);
  return 0;
}


static int
ucon64_opt_scan (st_ucon64_t *p)
{
  ucon64_opt_lsd (p);
  return 0;
}


static int
ucon64_opt_lsv (st_ucon64_t *p)
{
  if (p->nfo)
    ucon64_nfo ();
  return 0;
}


static int
ucon64_opt_ls (st_ucon64_t *p)
{
  char buf[MAXBUFSIZE];
  struct stat fstate;
  char *ptr = NULL;

  if (p->nfo)
    ptr = p->nfo->name;

  if (p->dat)
    {
      if (!ptr)
        ptr = ((st_ucon64_dat_t *) p->dat)->name;
      else if (!ptr[0])
        ptr = ((st_ucon64_dat_t *) p->dat)->name;
    }

  if (ptr)
    if (ptr[0])
      {
        if (stat (p->fname, &fstate) != 0)
          return 0;
        strftime (buf, 13, "%b %d %Y", localtime (&fstate.st_mtime));
        printf ("%-31.31s %10d %s %s", to_func (ptr, strlen (ptr), toprint),
                p->file_size, buf, basename2 (p->fname));
        if (p->fname_arch[0])
          printf (" (%s)\n", basename2 (p->fname_arch));
        else
          fputc ('\n', stdout);
      }
  return 0;
}


static int
ucon64_opt_rdat (st_ucon64_t *p)
{
  ucon64_rename (UCON64_RDAT);
  return 0;
}


static int
ucon64_opt_rrom (st_ucon64_t *p)
{
  ucon64_rename (UCON64_RROM);
  return 0;
}


static int
ucon64_opt_r83 (st_ucon64_t *p)
{
  ucon64_rename (UCON64_R83);
  return 0;
}


static int
ucon64_opt_rjoliet (st_ucon64_t *p)
{
  ucon64_rename (UCON64_RJOLIET);
  return 0;
}


static int
ucon64_opt_rl (st_ucon64_t *p)
{
  char rename_buf[FILENAME_MAX];
#ifdef  AMIGA
  char *ptr = NULL;
  char tmpbuf[FILENAME_MAX];

  ptr = basename2 (tmpnam3 (tmpbuf, 0));
  rename2 (p->fname, ptr);
#endif
  strcpy (rename_buf, basename2 (p->fname));
  printf ("Renaming \"%s\" to ", rename_buf);
  strlwr (rename_buf);
  ucon64_output_fname (rename_buf, OF_FORCE_BASENAME | OF_FORCE_SUFFIX);
  printf ("\"%s\"\n", rename_buf);
#ifdef  AMIGA
  rename2 (ptr, rename_buf);
#else
  rename2 (p->fname, rename_buf);
#endif
  p->fname = (const char *) rename_buf;
  return 0;
}


static int
ucon64_opt_ru (st_ucon64_t *p)
{
  char rename_buf[FILENAME_MAX];
#ifdef  AMIGA
  char *ptr = NULL;
  char tmpbuf[FILENAME_MAX];

  ptr = basename2 (tmpnam3 (tmpbuf, 0));
  rename2 (p->fname, ptr);
#endif
  strcpy (rename_buf, basename2 (p->fname));
  printf ("Renaming \"%s\" to ", rename_buf);
  strupr (rename_buf);
  ucon64_output_fname (rename_buf, OF_FORCE_BASENAME | OF_FORCE_SUFFIX);
  printf ("\"%s\"\n", rename_buf);
#ifdef  AMIGA
  rename2 (ptr, rename_buf);
#else
  rename2 (p->fname, rename_buf);
#endif
  p->fname = (const char *) rename_buf;
  return 0;
}


static int
ucon64_opt_dbv (st_ucon64_t *p)
{
  if (p->dat_enabled)
    {
      ucon64_dat_view (p->console, 1);
      printf ("TIP: %s " OPTION_LONG_S "dbv " OPTION_LONG_S "nes"
              " would show only information about known NES ROMs\n\n",
              basename2 (p->argv[0]));
    }
  else
    fputs (ucon64_msg[DAT_NOT_ENABLED], stdout);
  return 0;
}


static int
ucon64_opt_db (st_ucon64_t *p)
{
  if (p->quiet > -1)
    {
      if (p->dat_enabled)
        {
          ucon64_dat_view (p->console, 0);
          printf ("TIP: %s " OPTION_LONG_S "db " OPTION_LONG_S "nes"
                  " would show only information about known NES ROMs\n\n",
                  basename2 (p->argv[0]));
        }
      else
        fputs (ucon64_msg[DAT_NOT_ENABLED], stdout);
      return 0;
    }
  else return ucon64_opt_dbv (p);
}


static int
ucon64_opt_dbs (st_ucon64_t *p)
{
  if (p->dat_enabled)
    {
      p->crc32 = 0;
      sscanf (p->optarg, "%x", &p->crc32);

      if (!(p->dat = (st_ucon64_dat_t *) ucon64_dat_search (p->crc32, NULL)))
        {
          printf (ucon64_msg[DAT_NOT_FOUND], p->crc32);
          printf ("TIP: Be sure to install the right DAT files in %s\n", p->datdir);
        }
      else
        {
          ucon64_dat_nfo ((st_ucon64_dat_t *) p->dat, 1);
          printf ("\n"
                  "TIP: %s " OPTION_LONG_S "dbs" OPTARG_S "0x%08x " OPTION_LONG_S
                  "nes would search only for a NES ROM\n\n",
                  basename2 (p->argv[0]), p->crc32);
        }
    }
  else
    fputs (ucon64_msg[DAT_NOT_ENABLED], stdout);
  return 0;
}


static int
ucon64_opt_mkdat (st_ucon64_t *p)
{
  ucon64_create_dat (p->optarg, p->fname, p->nfo ? p->nfo->backup_header_len : 0);
  return 0;
}


static int
ucon64_opt_multi (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GBA:
        gba_multi (strtol (p->optarg, NULL, 10) * MBIT, NULL);
        return 0;
      case UCON64_GEN: 
        genesis_multi (strtol (p->optarg, NULL, 10) * MBIT, NULL);
        return 0;
      case UCON64_PCE:
        pce_multi (strtol (p->optarg, NULL, 10) * MBIT, NULL);
        return 0;
      case UCON64_SMS:                        // Sega Master System *and* Game Gear
        sms_multi (strtol (p->optarg, NULL, 10) * MBIT, NULL);
        return 0;
      case UCON64_SNES:
        snes_multi (strtol (p->optarg, NULL, 10) * MBIT, NULL);
        return 0;
    }
  return -1;
}


static int
ucon64_opt_e (st_ucon64_t *p)
{
  ucon64_e ();
  return 0;
}


static int
ucon64_opt_1991 (st_ucon64_t *p)
{
  genesis_1991 (p->nfo);
  return 0;
}


static int
ucon64_opt_b0 (st_ucon64_t *p)
{
  lynx_b0 (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_b1 (st_ucon64_t *p)
{
  lynx_b1 (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_bin (st_ucon64_t *p)
{
  genesis_bin (p->nfo);
  return 0;
}


static int
ucon64_opt_bot (st_ucon64_t *p)
{
  n64_bot (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_chk (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GB:
        gb_chk (p->nfo);
        return 0;

      case UCON64_GBA:
        gba_chk (p->nfo);
        return 0;

      case UCON64_GEN:
        genesis_chk (p->nfo);
        return 0;

      case UCON64_N64:
        n64_chk (p->nfo);
        return 0;

      case UCON64_SMS:
        sms_chk (p->nfo);
        return 0;

      case UCON64_SNES: 
        snes_chk (p->nfo);
        return 0;

      case UCON64_SWAN:
        swan_chk (p->nfo);
        return 0;

      case UCON64_NDS:
        nds_chk (p->nfo);
        return 0;
    }

  return -1;
}


static int
ucon64_opt_col (st_ucon64_t *p)
{
  snes_col (p->optarg);
  return 0;
}


static int
ucon64_opt_crp (st_ucon64_t *p)
{
  gba_crp (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_dbuh (st_ucon64_t *p)
{
  snes_backup_header_info (p->nfo);
  return 0;
}


static int
ucon64_opt_dint (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_NES:
        nes_dint ();
        return 0;

      case UCON64_PCE:
        pce_swap (p->nfo);
        return 0;

      case UCON64_SNES:
        snes_dint (p->nfo);
        return 0;

      case UCON64_N64:
        n64_swap (p->nfo);
        return 0;
    }

  return -1;
}


static int
ucon64_opt_swap (st_ucon64_t *p)
{
  ucon64_opt_dint (p);
  return 0;
}


static int
ucon64_opt_swap2 (st_ucon64_t *p)
{
  // --swap2 is currently used only for Nintendo 64
  n64_swap2 (p->nfo);
  return 0;
}


static int
ucon64_opt_dmirr (st_ucon64_t *p)
{
  snes_demirror (p->nfo);
  return 0;
}


static int
ucon64_opt_dnsrt (st_ucon64_t *p)
{
  snes_densrt (p->nfo);
  return 0;
}


static int
ucon64_opt_f (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GEN:
        genesis_f (p->nfo);
        return 0;

      case UCON64_PCE:
        pce_f (p->nfo);
        return 0;

      case UCON64_SNES:
        snes_f (p->nfo);
        return 0;
    }
  return -1;
}


static int
ucon64_opt_fds (st_ucon64_t *p)
{
  nes_fds ();
  return 0;
}


static int
ucon64_opt_fdsl (st_ucon64_t *p)
{
  nes_fdsl (p->nfo, NULL);
  return 0;
}


static int
ucon64_opt_ffe (st_ucon64_t *p)
{
  nes_ffe (p->nfo);
  return 0;
}


static int
ucon64_opt_fig (st_ucon64_t *p)
{
  snes_fig (p->nfo);
  return 0;
}


static int
ucon64_opt_figs (st_ucon64_t *p)
{
  snes_figs (p->nfo);
  return 0;
}


static int
ucon64_opt_gbx (st_ucon64_t *p)
{
  gb_gbx (p->nfo);
  return 0;
}


static int
ucon64_opt_gd3 (st_ucon64_t *p)
{
  snes_gd3 (p->nfo);
  return 0;
}


static int
ucon64_opt_gd3s (st_ucon64_t *p)
{
  snes_gd3s (p->nfo);
  return 0;
}


static int
ucon64_opt_gg (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GB:
      case UCON64_GEN:
      case UCON64_NES: 
      case UCON64_SMS:
      case UCON64_SNES:
        gg_apply (p->nfo, p->optarg);
        return 0;
    }

  fputs ("ERROR: Cannot apply Game Genie code for this ROM/console\n", stderr);
  return -1;
}


static int
ucon64_opt_ggd (st_ucon64_t *p)
{
  gg_display (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_gge (st_ucon64_t *p)
{
  gg_display (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_ines (st_ucon64_t *p)
{
  nes_ines ();
  return 0;
}


static int
ucon64_opt_ineshd (st_ucon64_t *p)
{
  nes_ineshd (p->nfo);
  return 0;
}


#if 0
static int
ucon64_opt_ip (st_ucon64_t *p)
{
  return 0;
}
#endif


static int
ucon64_opt_vms (st_ucon64_t *p)
{
  return 0;
}


static int
ucon64_opt_parse (st_ucon64_t *p)
{
  dc_parse (p->optarg);
  return 0;
}


static int
ucon64_opt_mkip (st_ucon64_t *p)
{
  dc_mkip ();
  return 0;
}


static int
ucon64_opt_j (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GEN:
        genesis_j (p->nfo);
        return 0;

      case UCON64_NES:
        nes_j (NULL);
        return 0;

      case UCON64_SNES:
        snes_j (p->nfo);
        return 0;
    }

  return -1;
}


static int
ucon64_opt_k (st_ucon64_t *p)
{
  snes_k (p->nfo);
  return 0;
}


static int
ucon64_opt_l (st_ucon64_t *p)
{
  snes_l (p->nfo);
  return 0;
}


static int
ucon64_opt_lnx (st_ucon64_t *p)
{
  lynx_lnx (p->nfo);
  return 0;
}


static int
ucon64_opt_logo (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GB:
        gb_logo (p->nfo);
        return 0;
  
      case UCON64_GBA:
        gba_logo (p->nfo);
        return 0;
  
      case UCON64_NDS:
        nds_logo (p->nfo);
        return 0;
    }

  return -1;
}


static int
ucon64_opt_lsram (st_ucon64_t *p)
{
  n64_sram (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_lyx (st_ucon64_t *p)
{
  lynx_lyx (p->nfo);
  return 0;
}


static int
ucon64_opt_mgd (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GB:
        gb_mgd (p->nfo);
        return 0;

      case UCON64_GEN:
        genesis_mgd (p->nfo);
        return 0;

      case UCON64_PCE:
        pce_mgd (p->nfo);
        return 0;

      case UCON64_SMS:
        sms_mgd (p->nfo, UCON64_SMS);
        return 0;

      case UCON64_SNES:
        snes_mgd (p->nfo);
        return 0;
    }

  return -1;
}


static int
ucon64_opt_mgdgg (st_ucon64_t *p)
{
  sms_mgd (p->nfo, UCON64_GAMEGEAR);
  return 0;
}


static int
ucon64_opt_msg (st_ucon64_t *p)
{
  pce_msg (p->nfo);
  return 0;
}


static int
ucon64_opt_n (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GB:
        gb_n (p->nfo, p->optarg);
        return 0;

      case UCON64_GBA:
        gba_n (p->nfo, p->optarg);
        return 0;

      case UCON64_GEN:
        genesis_n (p->nfo, p->optarg);
        return 0;

      case UCON64_LYNX:
        lynx_n (p->nfo, p->optarg);
        return 0;

      case UCON64_N64:
        n64_n (p->nfo, p->optarg);
        return 0;

      case UCON64_NES:
        nes_n (p->optarg);
        return 0;

      case UCON64_SNES:
        snes_n (p->nfo, p->optarg);
        return 0;

      case UCON64_NDS:
        nds_n (p->nfo, p->optarg);
        return 0;
    }

  return -1;
}


static int
ucon64_opt_n2 (st_ucon64_t *p)
{
  genesis_n2 (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_n2gb (st_ucon64_t *p)
{
  gb_n2gb (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_nrot (st_ucon64_t *p)
{
  lynx_nrot (p->nfo);
  return 0;
}


static int
ucon64_opt_pasofami (st_ucon64_t *p)
{
  nes_pasofami ();
  return 0;
}


static int
ucon64_opt_pattern (st_ucon64_t *p)
{
  ucon64_pattern (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_poke (st_ucon64_t *p)
{
  patch_poke (&ucon64);
  return 0;
}


static int
ucon64_opt_rotl (st_ucon64_t *p)
{
  lynx_rotl (p->nfo);
  return 0;
}


static int
ucon64_opt_rotr (st_ucon64_t *p)
{
  lynx_rotr (p->nfo);
  return 0;
}


static int
ucon64_opt_s (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GEN:
        genesis_s (p->nfo);
        return 0;

      case UCON64_NES:
        nes_s ();
        return 0;

      case UCON64_SNES:
        snes_s (p->nfo);
        return 0;
    }

  return -1;
}


static int
ucon64_opt_scr (st_ucon64_t *p)
{
  dc_scramble ();
  return 0;
}


static int
ucon64_opt_sgb (st_ucon64_t *p)
{
  gb_sgb (p->nfo);
  return 0;
}


#ifdef  HAVE_MATH_H
static int
ucon64_opt_cc2 (st_ucon64_t *p)
{
  printf (ucon64_msg[UNTESTED]);
  atari_cc2 (p->fname, p->optarg ? strtol (p->optarg, NULL, 10) : 0);
  return 0;
}
#endif


static int
ucon64_opt_smc (st_ucon64_t *p)
{
  snes_smc (p->nfo);
  return 0;
}


static int
ucon64_opt_smd (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GEN:
        genesis_smd (p->nfo);
        return 0;

      case UCON64_SMS:
        sms_smd (p->nfo);
        return 0;
    }

  return -1;
}


static int
ucon64_opt_smds (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_GEN:
        genesis_smds ();
        return 0;

      case UCON64_SMS:
        sms_smds ();
        return 0;
    }

  return -1;
}


static int
ucon64_opt_sram (st_ucon64_t *p)
{
  gba_sram ();
  return 0;
}


static int
ucon64_opt_sc (st_ucon64_t *p)
{
  switch (p->console)
    {
      case UCON64_SMS:
        sms_sc ();
        return 0;

      case UCON64_GB:
        gb_sc ();
        return 0;

      case UCON64_GBA:
        gba_sc ();
        return 0;

      case UCON64_NES:
        nes_sc ();
        return 0;

      case UCON64_NDS:
        nds_sc ();
        return 0;
    }

  return -1;
}


static int
ucon64_opt_ssc (st_ucon64_t *p)
{
  gb_ssc (p->nfo);
  return 0;
}


static int
ucon64_opt_swc (st_ucon64_t *p)
{
  snes_swc (p->nfo);
  return 0;
}


static int
ucon64_opt_swcs (st_ucon64_t *p)
{
  snes_swcs (p->nfo);
  return 0;
}


static int
ucon64_opt_ufo (st_ucon64_t *p)
{
  snes_ufo (p->nfo);
  return 0;
}


static int
ucon64_opt_ufos (st_ucon64_t *p)
{
  snes_ufos (p->nfo);
  return 0;
}


static int
ucon64_opt_unif (st_ucon64_t *p)
{
  nes_unif ();
  return 0;
}


static int
ucon64_opt_unscr (st_ucon64_t *p)
{
  dc_unscramble ();
  return 0;
}


static int
ucon64_opt_usms (st_ucon64_t *p)
{
  n64_usms (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_opt_v64 (st_ucon64_t *p)
{
  n64_v64 (p->nfo);
  return 0;
}

#ifdef  USE_PARALLEL
/*
  It doesn't make sense to continue after executing a (send) backup option
  ("multizip"). Don't return, but use break instead. ucon64_execute_options()
  checks if an option was used that should stop uCON64.
*/
#ifdef  USE_LIBCD64
static int
ucon64_opt_xcd64 (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_rom (p->fname, 64);
  else
    cd64_write_rom (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xcd64c (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  cd64_read_rom (p->fname, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xcd64b (st_ucon64_t *p)
{
  cd64_write_bootemu (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xcd64s (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_sram (p->fname);
  else
    cd64_write_sram (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xcd64f (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_flashram (p->fname);
  else
    cd64_write_flashram (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xcd64e (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_eeprom (p->fname);
  else
    cd64_write_eeprom (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xcd64m (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_mempack (p->fname, strtol (p->optarg, NULL, 10));
  else
    cd64_write_mempack (p->fname, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}
#endif


static int
ucon64_opt_xreset (st_ucon64_t *p)
{
  parport_print_info ();
  fputs ("Resetting parallel port...", stdout);
  outportb ((unsigned short) (p->parport + PARPORT_DATA), 0);
  outportb ((unsigned short) (p->parport + PARPORT_CONTROL), 0);
  puts ("done");
  return 0;
}


static int
ucon64_opt_xcmc (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  cmc_read_rom (p->fname, p->parport, p->io_mode); // p->io_mode contains speed value
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xcmct (st_ucon64_t *p)
{
  cmc_test (strtol (p->optarg, NULL, 10), p->parport, p->io_mode);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xdex (st_ucon64_t *p)
{
  printf (ucon64_msg[UNTESTED]);
  if (access (p->fname, F_OK) != 0)
    dex_read_block (p->fname, strtol (p->optarg, NULL, 10), p->parport);
  else
    dex_write_block (p->fname, strtol (p->optarg, NULL, 10), p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xdjr (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    doctor64jr_read (p->fname, p->parport);
  else
    {
      if (!p->nfo->interleaved)
        fputs ("ERROR: This ROM doesn't seem to be interleaved but the Doctor V64 Junior only\n"
               "       supports interleaved ROMs. Convert to a Doctor V64 compatible format\n",
               stderr);
      else
        doctor64jr_write (p->fname, p->parport);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xfal (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    fal_read_rom (p->fname, p->parport, 32);
  else
    fal_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xfalmulti (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  tmpnam3 (src_name, 0);
  p->temp_file = src_name;
  register_func (remove_temp_file);
  // gba_multi() calls ucon64_file_handler() so the directory part will be
  //  stripped from src_name. The directory should be used though.
  if (!p->output_path[0])
    {
      dirname2 (src_name, p->output_path);
      if (p->output_path[strlen (p->output_path) - 1] != FILE_SEPARATOR)
        strcat (p->output_path, FILE_SEPARATOR_S);
    }
  if (gba_multi (strtol (p->optarg, NULL, 10) * MBIT, src_name) == 0)
    { // Don't try to start a transfer if there was a problem
      fputc ('\n', stdout);
      p->file_size = fsizeof (src_name);
      fal_write_rom (src_name, p->parport);
    }

  unregister_func (remove_temp_file);
  remove_temp_file ();
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xfalc (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  fal_read_rom (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xfals (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    fal_read_sram (p->fname, p->parport, UCON64_UNKNOWN);
  else
    fal_write_sram (p->fname, p->parport, UCON64_UNKNOWN);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xfalb (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    fal_read_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  else
    fal_write_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xfig (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cartridge
    fig_read_rom (p->fname, p->parport);
  else
    {
      if (!p->nfo->backup_header_len)
        fputs ("ERROR: This ROM has no header. Convert to a FIG compatible format\n",
               stderr);
      else if (p->nfo->interleaved)
        fputs ("ERROR: This ROM seems to be interleaved but the FIG doesn't support\n"
               "       interleaved ROMs. Convert to a FIG compatible format\n",
               stderr);
      else // file exists -> send it to the copier
        fig_write_rom (p->fname, p->parport);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xfigs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    fig_read_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    fig_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xfigc (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cart SRAM contents
    fig_read_cart_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    fig_write_cart_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgbx (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cartridge/flash card
    gbx_read_rom (p->fname, p->parport);
  else                                      // file exists -> send it to the programmer
    gbx_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgbxs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gbx_read_sram (p->fname, p->parport, -1);
  else
    gbx_write_sram (p->fname, p->parport, -1);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgbxb (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gbx_read_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  else
    gbx_write_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgd3 (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cartridge
    gd3_read_rom (p->fname, p->parport); // dumping is not yet supported
  else
    {
      if (!p->nfo->backup_header_len)
        fputs ("ERROR: This ROM has no header. Convert to a Game Doctor compatible format\n",
               stderr);
      else                                  // file exists -> send it to the copier
        gd3_write_rom (p->fname, p->parport, p->nfo);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgd3s (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    gd3_read_sram (p->fname, p->parport); // dumping is not yet supported
  else                                      // file exists -> restore SRAM
    gd3_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgd3r (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gd3_read_saver (p->fname, p->parport);
  else
    gd3_write_saver (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgd6 (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gd6_read_rom (p->fname, p->parport); // dumping is not yet supported
  else
    {
      if (!p->nfo->backup_header_len)
        fputs ("ERROR: This ROM has no header. Convert to a Game Doctor compatible format\n",
               stderr);
      else
        gd6_write_rom (p->fname, p->parport, p->nfo);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgd6s (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gd6_read_sram (p->fname, p->parport);
  else
    gd6_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgd6r (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gd6_read_saver (p->fname, p->parport);
  else
    gd6_write_saver (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xgg (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    smsgg_read_rom (p->fname, p->parport, 32 * MBIT);
  else
    {
      if (p->nfo->backup_header_len)
        fputs ("ERROR: This ROM has a header. Remove it with -stp or -mgd\n",
               stderr);
      else if (p->nfo->interleaved)
        fputs ("ERROR: This ROM seems to be interleaved, but uCON64 doesn't support\n"
               "       sending interleaved ROMs to the SMS-PRO/GG-PRO. Convert ROM with -mgd\n",
               stderr);
      else
        smsgg_write_rom (p->fname, p->parport);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xggs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    smsgg_read_sram (p->fname, p->parport, -1);
  else
    smsgg_write_sram (p->fname, p->parport, -1);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xggb (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    smsgg_read_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  else
    smsgg_write_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xlit (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  lynxit_read_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xmccl (st_ucon64_t *p)
{
  printf (ucon64_msg[UNTESTED]);
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  mccl_read (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xmcd (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  mcd_read_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xmd (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump flash card
    md_read_rom (p->fname, p->parport, 64 * MBIT); // reads 32 Mbit if Sharp card
  else                                      // file exists -> send it to the MD-PRO
    {
      if (p->nfo->backup_header_len)     // binary with header is possible
        fputs ("ERROR: This ROM has a header. Remove it with -stp or -bin\n",
               stderr);
      else if (genesis_get_file_type () != BIN)
        fputs ("ERROR: This ROM is not in binary/BIN/RAW format. uCON64 only supports sending\n"
               "       binary files to the MD-PRO. Convert ROM with -bin\n",
               stderr);
      else
        md_write_rom (p->fname, p->parport);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xmds (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    md_read_sram (p->fname, p->parport, -1);
  else                                      // file exists -> restore SRAM
    md_write_sram (p->fname, p->parport, -1);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xmdb (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    md_read_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  else
    md_write_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xmsg (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    msg_read_rom (p->fname, p->parport);
  else
    {
      if (!p->nfo->backup_header_len)
        fputs ("ERROR: This ROM has no header. Convert to an MSG compatible format\n",
               stderr);
      else if (p->nfo->interleaved)
        fputs ("ERROR: This ROM seems to be bit-swapped but the MSG doesn't support\n"
               "       bit-swapped ROMs. Convert to an MSG compatible format\n",
               stderr);
      else
        msg_write_rom (p->fname, p->parport);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xpce (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    pce_read_rom (p->fname, p->parport, 32 * MBIT);
  else
    pce_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xpl (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    pl_read_rom (p->fname, p->parport);
  else
    pl_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xpli (st_ucon64_t *p)
{
  pl_info (p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xsf (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump flash card
    sf_read_rom (p->fname, p->parport, 64 * MBIT);
  else                                      // file exists -> send it to the Super Flash
    sf_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xsfs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    sf_read_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    sf_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xsmc (st_ucon64_t *p) // we don't use WF_NO_ROM => no need to check for file
{
  if (!p->nfo->backup_header_len)
    fputs ("ERROR: This ROM has no header. Convert to an SMC compatible format with -ffe\n",
           stderr);
  else
    smc_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xsmcr (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    smc_read_rts (p->fname, p->parport);
  else
    smc_write_rts (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xsmd (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cartridge
    smd_read_rom (p->fname, p->parport);
  else                                      // file exists -> send it to the copier
    {
      if (!p->nfo->backup_header_len)
        fputs ("ERROR: This ROM has no header. Convert to an SMD compatible format\n",
               stderr);
      else if (!p->nfo->interleaved)
        fputs ("ERROR: This ROM doesn't seem to be interleaved but the SMD only supports\n"
               "       interleaved ROMs. Convert to an SMD compatible format\n",
               stderr);
      else
        smd_write_rom (p->fname, p->parport);
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xsmds (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    smd_read_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    smd_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xswc2 (st_ucon64_t *p)
{
  int enableRTS = -1;

  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cartridge
    swc_read_rom (p->fname, p->parport, p->io_mode);
  else
    {
      if (!p->nfo->backup_header_len)
        fputs ("ERROR: This ROM has no header. Convert to an SWC compatible format\n",
               stderr);
      else if (p->nfo->interleaved)
        fputs ("ERROR: This ROM seems to be interleaved but the SWC doesn't support\n"
               "       interleaved ROMs. Convert to an SWC compatible format\n",
               stderr);
      else
        {
          if (enableRTS != 0)
            enableRTS = 1;
          // file exists -> send it to the copier
          swc_write_rom (p->fname, p->parport, enableRTS);
        }
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xswc (st_ucon64_t *p)
{
  int enableRTS = 0;

  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cartridge
    swc_read_rom (p->fname, p->parport, p->io_mode);
  else
    {
      if (!p->nfo->backup_header_len)
        fputs ("ERROR: This ROM has no header. Convert to an SWC compatible format\n",
               stderr);
      else if (p->nfo->interleaved)
        fputs ("ERROR: This ROM seems to be interleaved but the SWC doesn't support\n"
               "       interleaved ROMs. Convert to an SWC compatible format\n",
               stderr);
      else
        {
          if (enableRTS != 0)
            enableRTS = 1;
          // file exists -> send it to the copier
          swc_write_rom (p->fname, p->parport, enableRTS);
        }
    }
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xswcs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    swc_read_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    swc_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xswcc (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    swc_read_cart_sram (p->fname, p->parport, p->io_mode);
  else                                      // file exists -> restore SRAM
    swc_write_cart_sram (p->fname, p->parport, p->io_mode);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xswcr (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    swc_read_rts (p->fname, p->parport);
  else
    swc_write_rts (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xv64 (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    doctor64_read (p->fname, p->parport);
  else
    {
      if (!p->nfo->interleaved)
        fputs ("ERROR: This ROM doesn't seem to be interleaved but the Doctor V64 only\n"
               "       supports interleaved ROMs. Convert to a Doctor V64 compatible format\n",
               stderr);
      else
        doctor64_write (p->fname, p->nfo->backup_header_len,
                        p->file_size, p->parport);
    }
  fputc ('\n', stdout);
  return 0;
}
#endif // USE_PARALLEL


#if     defined USE_PARALLEL || defined USE_USB
static int
ucon64_opt_xf2a (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    f2a_read_rom (p->fname, 32);
  else
    f2a_write_rom (p->fname, UCON64_UNKNOWN);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xf2amulti (st_ucon64_t *p)
{
  f2a_write_rom (NULL, strtol (p->optarg, NULL, 10) * MBIT);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xf2ac (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  f2a_read_rom (p->fname, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xf2as (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    f2a_read_sram (p->fname, UCON64_UNKNOWN);
  else
    f2a_write_sram (p->fname, UCON64_UNKNOWN);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_opt_xf2ab (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    f2a_read_sram (p->fname, strtol (p->optarg, NULL, 10));
  else
    f2a_write_sram (p->fname, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}
#endif // USE_PARALLEL || USE_USB


static int
ucon64_opt_z64 (st_ucon64_t *p)
{
  n64_z64 (p->nfo);
  return 0;
}


st_ucon64_opts_t ucon64_opts[] = {
  {UCON64_PAD, 0, 0, ucon64_opt_pad},
  {UCON64_CRCHD, 0, 0, ucon64_opt_crchd},
  {UCON64_CRC, 0, 0, ucon64_opt_crc},
  {UCON64_SHA1, 0, 0, ucon64_opt_sha1},
  {UCON64_MD5, 0, 0, ucon64_opt_md5},
  {UCON64_HEX, 0, 0, ucon64_opt_hex},
  {UCON64_BITS, 0, 0, ucon64_opt_bits},
  {UCON64_CODE, 0, 0, ucon64_opt_code},
  {UCON64_PRINT, 0, 0, ucon64_opt_print},
  {UCON64_C, 0, 0, ucon64_opt_c},
  {UCON64_CS, 0, 0, ucon64_opt_cs},
  {UCON64_FIND, 0, 0, ucon64_opt_find},
  {UCON64_FINDR, 0, 0, ucon64_opt_findr},
  {UCON64_FINDI, 0, 0, ucon64_opt_findi},
  {UCON64_HFIND, 0, 0, ucon64_opt_hfind},
  {UCON64_HFINDR, 0, 0, ucon64_opt_hfindr},
  {UCON64_DFIND, 0, 0, ucon64_opt_dfind},
  {UCON64_DFINDR, 0, 0, ucon64_opt_dfindr},
  {UCON64_PADHD, 0, 0, ucon64_opt_padhd},
  {UCON64_P, 0, 0, ucon64_opt_p},
  {UCON64_PAD, 0, 0, ucon64_opt_pad},
  {UCON64_PADN, 0, 0, ucon64_opt_padn},
  {UCON64_ISPAD, 0, 0, ucon64_opt_ispad},
  {UCON64_STRIP, 0, 0, ucon64_opt_strip},
  {UCON64_STP, 0, 0, ucon64_opt_stp},
  {UCON64_STPN, 0, 0, ucon64_opt_stpn},
  {UCON64_INS, 0, 0, ucon64_opt_ins},
  {UCON64_INSN, 0, 0, ucon64_opt_insn},
  {UCON64_A, 0, 0, ucon64_opt_a},
  {UCON64_B, 0, 0, ucon64_opt_b},
  {UCON64_I, 0, 0, ucon64_opt_i},
  {UCON64_PPF, 0, 0, ucon64_opt_ppf},
  {UCON64_MKA, 0, 0, ucon64_opt_mka},
  {UCON64_MKI, 0, 0, ucon64_opt_mki},
  {UCON64_MKPPF, 0, 0, ucon64_opt_mkppf},
  {UCON64_NA, 0, 0, ucon64_opt_na},
  {UCON64_NPPF, 0, 0, ucon64_opt_nppf},
  {UCON64_IDPPF, 0, 0, ucon64_opt_idppf},
  {UCON64_SCAN, 0, 0, ucon64_opt_scan},
  {UCON64_LSD, 0, 0, ucon64_opt_lsd},
  {UCON64_LSV, 0, 0, ucon64_opt_lsv},
  {UCON64_LS, 0, 0, ucon64_opt_ls},
  {UCON64_RDAT, 0, 0, ucon64_opt_rdat},
  {UCON64_RROM, 0, 0, ucon64_opt_rrom},
  {UCON64_R83, 0, 0, ucon64_opt_r83},
  {UCON64_RJOLIET, 0, 0, ucon64_opt_rjoliet},
  {UCON64_RL, 0, 0, ucon64_opt_rl},
  {UCON64_RU, 0, 0, ucon64_opt_ru},
  {UCON64_DB, 0, 0, ucon64_opt_db},
  {UCON64_DBV, 0, 0, ucon64_opt_dbv},
  {UCON64_DBS, 0, 0, ucon64_opt_dbs},
  {UCON64_MKDAT, 0, 0, ucon64_opt_mkdat},
  {UCON64_MULTI, 0, 0, ucon64_opt_multi},
  {UCON64_E, 0, 0, ucon64_opt_e},
  {UCON64_1991, 0, 0, ucon64_opt_1991},
  {UCON64_B0, 0, 0, ucon64_opt_b0},
  {UCON64_B1, 0, 0, ucon64_opt_b1},
  {UCON64_BIN, 0, 0, ucon64_opt_bin},
  {UCON64_BOT, 0, 0, ucon64_opt_bot},
  {UCON64_CHK, 0, 0, ucon64_opt_chk},
  {UCON64_COL, 0, 0, ucon64_opt_col},
  {UCON64_CRP, 0, 0, ucon64_opt_crp},
  {UCON64_DBUH, 0, 0, ucon64_opt_dbuh},
  {UCON64_SWAP, 0, 0, ucon64_opt_swap},
  {UCON64_DINT, 0, 0, ucon64_opt_dint},
  {UCON64_SWAP2, 0, 0, ucon64_opt_swap2},
  {UCON64_DMIRR, 0, 0, ucon64_opt_dmirr},
  {UCON64_DNSRT, 0, 0, ucon64_opt_dnsrt},
  {UCON64_F, 0, 0, ucon64_opt_f},
  {UCON64_FDS, 0, 0, ucon64_opt_fds},
  {UCON64_FDSL, 0, 0, ucon64_opt_fdsl},
  {UCON64_FFE, 0, 0, ucon64_opt_ffe},
  {UCON64_FIG, 0, 0, ucon64_opt_fig},
  {UCON64_FIGS, 0, 0, ucon64_opt_figs},
  {UCON64_GBX, 0, 0, ucon64_opt_gbx},
  {UCON64_GD3, 0, 0, ucon64_opt_gd3},
  {UCON64_GD3S, 0, 0, ucon64_opt_gd3s},
  {UCON64_GG, 0, 0, ucon64_opt_gg},
  {UCON64_GGD, 0, 0, ucon64_opt_ggd},
  {UCON64_GGE, 0, 0, ucon64_opt_gge},
  {UCON64_INES, 0, 0, ucon64_opt_ines},
  {UCON64_INESHD, 0, 0, ucon64_opt_ineshd},
//  {UCON64_IP, 0, 0, ucon64_opt_ip},
  {UCON64_VMS, 0, 0, ucon64_opt_vms},
  {UCON64_PARSE, 0, 0, ucon64_opt_parse},
  {UCON64_MKIP, 0, 0, ucon64_opt_mkip},
  {UCON64_J, 0, 0, ucon64_opt_j},
  {UCON64_K, 0, 0, ucon64_opt_k},
  {UCON64_L, 0, 0, ucon64_opt_l},
  {UCON64_LNX, 0, 0, ucon64_opt_lnx},
  {UCON64_LOGO, 0, 0, ucon64_opt_logo},
  {UCON64_LSRAM, 0, 0, ucon64_opt_lsram},
  {UCON64_LYX, 0, 0, ucon64_opt_lyx},
  {UCON64_MGD, 0, 0, ucon64_opt_mgd},
  {UCON64_MGDGG, 0, 0, ucon64_opt_mgdgg},
  {UCON64_MSG, 0, 0, ucon64_opt_msg},
  {UCON64_N, 0, 0, ucon64_opt_n},
  {UCON64_N2, 0, 0, ucon64_opt_n2},
  {UCON64_N2GB, 0, 0, ucon64_opt_n2gb},
  {UCON64_NROT, 0, 0, ucon64_opt_nrot},
  {UCON64_PASOFAMI, 0, 0, ucon64_opt_pasofami},
  {UCON64_PATTERN, 0, 0, ucon64_opt_pattern},
  {UCON64_POKE, 0, 0, ucon64_opt_poke},
  {UCON64_ROTL, 0, 0, ucon64_opt_rotl},
  {UCON64_ROTR, 0, 0, ucon64_opt_rotr},
  {UCON64_S, 0, 0, ucon64_opt_s},
  {UCON64_SCR, 0, 0, ucon64_opt_scr},
  {UCON64_SGB, 0, 0, ucon64_opt_sgb},
#ifdef  HAVE_MATH_H
  {UCON64_CC2, 0, 0, ucon64_opt_cc2},
#endif
  {UCON64_SMC, 0, 0, ucon64_opt_smc},
  {UCON64_SMD, 0, 0, ucon64_opt_smd},
  {UCON64_SMDS, 0, 0, ucon64_opt_smds},
  {UCON64_SRAM, 0, 0, ucon64_opt_sram},
  {UCON64_SC, 0, 0, ucon64_opt_sc},
  {UCON64_SSC, 0, 0, ucon64_opt_ssc},
  {UCON64_SWC, 0, 0, ucon64_opt_swc},
  {UCON64_SWCS, 0, 0, ucon64_opt_swcs},
  {UCON64_UFO, 0, 0, ucon64_opt_ufo},
  {UCON64_UFOS, 0, 0, ucon64_opt_ufos},
  {UCON64_UNIF, 0, 0, ucon64_opt_unif},
  {UCON64_UNSCR, 0, 0, ucon64_opt_unscr},
  {UCON64_USMS, 0, 0, ucon64_opt_usms},
  {UCON64_V64, 0, 0, ucon64_opt_v64},
#ifdef  USE_PARALLEL
#ifdef  USE_LIBCD64
  {UCON64_XCD64, 0, 0, ucon64_opt_xcd64},
  {UCON64_XCD64C, 0, 0, ucon64_opt_xcd64c},
  {UCON64_XCD64B, 0, 0, ucon64_opt_xcd64b},
  {UCON64_XCD64S, 0, 0, ucon64_opt_xcd64s},
  {UCON64_XCD64F, 0, 0, ucon64_opt_xcd64f},
  {UCON64_XCD64E, 0, 0, ucon64_opt_xcd64e},
  {UCON64_XCD64M, 0, 0, ucon64_opt_xcd64m},
#endif
  {UCON64_XRESET, 0, 0, ucon64_opt_xreset},
  {UCON64_XCMC, 0, 0, ucon64_opt_xcmc},
  {UCON64_XCMCT, 0, 0, ucon64_opt_xcmct},
  {UCON64_XDEX, 0, 0, ucon64_opt_xdex},
  {UCON64_XDJR, 0, 0, ucon64_opt_xdjr},
  {UCON64_XFAL, 0, 0, ucon64_opt_xfal},
  {UCON64_XFALMULTI, 0, 0, ucon64_opt_xfalmulti},
  {UCON64_XFALC, 0, 0, ucon64_opt_xfalc},
  {UCON64_XFALS, 0, 0, ucon64_opt_xfals},
  {UCON64_XFALB, 0, 0, ucon64_opt_xfalb},
  {UCON64_XFIG, 0, 0, ucon64_opt_xfig},
  {UCON64_XFIGS, 0, 0, ucon64_opt_xfigs},
  {UCON64_XFIGC, 0, 0, ucon64_opt_xfigc},
  {UCON64_XGBX, 0, 0, ucon64_opt_xgbx},
  {UCON64_XGBXS, 0, 0, ucon64_opt_xgbxs},
  {UCON64_XGBXB, 0, 0, ucon64_opt_xgbxb},
  {UCON64_XGD3, 0, 0, ucon64_opt_xgd3},
  {UCON64_XGD3S, 0, 0, ucon64_opt_xgd3s},
  {UCON64_XGD3R, 0, 0, ucon64_opt_xgd3r},
  {UCON64_XGD6, 0, 0, ucon64_opt_xgd6},
  {UCON64_XGD6S, 0, 0, ucon64_opt_xgd6s},
  {UCON64_XGD6R, 0, 0, ucon64_opt_xgd6r},
  {UCON64_XGG, 0, 0, ucon64_opt_xgg},
  {UCON64_XGGS, 0, 0, ucon64_opt_xggs},
  {UCON64_XGGB, 0, 0, ucon64_opt_xggb},
  {UCON64_XLIT, 0, 0, ucon64_opt_xlit},
  {UCON64_XMCCL, 0, 0, ucon64_opt_xmccl},
  {UCON64_XMCD, 0, 0, ucon64_opt_xmcd},
  {UCON64_XMD, 0, 0, ucon64_opt_xmd},
  {UCON64_XMDS, 0, 0, ucon64_opt_xmds},
  {UCON64_XMDB, 0, 0, ucon64_opt_xmdb},
  {UCON64_XMSG, 0, 0, ucon64_opt_xmsg},
  {UCON64_XPCE, 0, 0, ucon64_opt_xpce},
  {UCON64_XPL, 0, 0, ucon64_opt_xpl},
  {UCON64_XPLI, 0, 0, ucon64_opt_xpli},
  {UCON64_XSF, 0, 0, ucon64_opt_xsf},
  {UCON64_XSFS, 0, 0, ucon64_opt_xsfs},
  {UCON64_XSMC, 0, 0, ucon64_opt_xsmc},
  {UCON64_XSMCR, 0, 0, ucon64_opt_xsmcr},
  {UCON64_XSMD, 0, 0, ucon64_opt_xsmd},
  {UCON64_XSMDS, 0, 0, ucon64_opt_xsmds},
  {UCON64_XSWC, 0, 0, ucon64_opt_xswc},
  {UCON64_XSWC2, 0, 0, ucon64_opt_xswc2},
  {UCON64_XSWCS, 0, 0, ucon64_opt_xswcs},
  {UCON64_XSWCC, 0, 0, ucon64_opt_xswcc},
  {UCON64_XSWCR, 0, 0, ucon64_opt_xswcr},
  {UCON64_XV64, 0, 0, ucon64_opt_xv64},
#endif // USE_PARALLEL
#if     defined USE_PARALLEL || defined USE_USB
  {UCON64_XF2A, 0, 0, ucon64_opt_xf2a},
  {UCON64_XF2AMULTI, 0, 0, ucon64_opt_xf2amulti},
  {UCON64_XF2AC, 0, 0, ucon64_opt_xf2ac},
  {UCON64_XF2AS, 0, 0, ucon64_opt_xf2as},
  {UCON64_XF2AB, 0, 0, ucon64_opt_xf2ab},
#endif // USE_PARALLEL || USE_USB
  {UCON64_Z64, 0, 0, ucon64_opt_z64},
  {0, 0, 0, NULL}
};
