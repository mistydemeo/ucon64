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


#warning build filters with these
#define UCON64_FILTER_BUILD(id,d,o,c,r,w) FILTER_BUILD(&NULL,id,d,o,c,r,w,&NULL,&NULL,&NULL,&NULL)


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


#warning merge all *_tmp_* funcs into the funcs they wrap
static int
ucon64_tmp_help (st_ucon64_t *p)
{
printf ("SHIT");
fflush (stdout);

  int x = 0;
    /*
      Many tools ignore other options if --help has been specified. We do the
      same (compare with GNU tools).
    */
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
        }
      ucon64_usage (ucon64.argc, ucon64.argv, x);

  return 0;
}


static int
ucon64_tmp_ver (st_ucon64_t *p)
{
    /*
      It's also common to exit after displaying version information.
      On some configurations printf is a macro (Red Hat Linux 6.2 + GCC 3.2),
      so we can't use preprocessor directives in the argument list.
    */
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
  return 0;
}


static int
ucon64_tmp_crc (st_ucon64_t *p)
{
  unsigned int checksum = 0;
#warning ucon64.nfo?
  ucon64_chksum (NULL, NULL, &checksum, p->fname,
                 ucon64.nfo ? ucon64.nfo->backup_header_len : ucon64.backup_header_len);
  return 0;
}


static int
ucon64_tmp_crchd (st_ucon64_t *p)                          // deprecated
{
  unsigned int checksum = 0;
  ucon64_chksum (NULL, NULL, &checksum, p->fname, UNKNOWN_BACKUP_HEADER_LEN);
  return 0;
}


static int
ucon64_tmp_sha1 (st_ucon64_t *p)
{
  char buf[MAXBUFSIZE];

#warning buf?
#warning ucon64.nfo?
  ucon64_chksum (buf, NULL, NULL, p->fname,
                 ucon64.nfo ? ucon64.nfo->backup_header_len : ucon64.backup_header_len);
  return 0;
}


static int
ucon64_tmp_md5 (st_ucon64_t *p)
{
  char buf[MAXBUFSIZE];

#warning buf?
#warning ucon64.nfo?
  ucon64_chksum (NULL, buf, NULL, p->fname,
                 ucon64.nfo ? ucon64.nfo->backup_header_len : ucon64.backup_header_len);
  return 0;
}


static int
ucon64_tmp_hex (st_ucon64_t *p)
{
  ucon64_dump (stdout, p->fname,
               p->optarg ? MAX (strtol2 (p->optarg, NULL), 0) : 0,
               fsizeof (p->fname), 0);
  return 0;
}


static int
ucon64_tmp_bits (st_ucon64_t *p)
{
  ucon64_dump (stdout, p->fname,
               p->optarg ? MAX (strtol2 (p->optarg, NULL), 0) : 0,
               fsizeof (p->fname), DUMPER_BIT);
  return 0;
}


static int
ucon64_tmp_code (st_ucon64_t *p)
{
  ucon64_dump (stdout, p->fname,
               p->optarg ? MAX (strtol2 (p->optarg, NULL), 0) : 0,
               fsizeof (p->fname), DUMPER_CODE);
  return 0;
}


static int
ucon64_tmp_print (st_ucon64_t *p)
{
  ucon64_dump (stdout, p->fname,
               p->optarg ? MAX (strtol2 (p->optarg, NULL), 0) : 0,
               fsizeof (p->fname), DUMPER_TEXT);
  return 0;
}


static int
ucon64_tmp_c (st_ucon64_t *p)
{
  ucon64_filefile (p->optarg, 0, p->fname, 0, FALSE);
  return 0;
}


static int
ucon64_tmp_cs (st_ucon64_t *p)
{
  ucon64_filefile (p->optarg, 0, p->fname, 0, TRUE);
  return 0;
}


static int
ucon64_tmp_find (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, fsizeof (p->fname), p->optarg,
               strlen (p->optarg), MEMCMP2_WCARD ('?'), 0);
  return 0;
}


static int
ucon64_tmp_findr (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, fsizeof (p->fname), p->optarg,
               strlen (p->optarg), MEMCMP2_REL, 0);
  return 0;
}


static int
ucon64_tmp_findi (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, fsizeof (p->fname), p->optarg,
               strlen (p->optarg), MEMCMP2_WCARD ('?') | MEMCMP2_CASE, 0);
  return 0;
}


static int
ucon64_tmp_hfind (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, fsizeof (p->fname), p->optarg,
               strlen (p->optarg), MEMCMP2_WCARD ('?'), 1);
  return 0;
}


static int
ucon64_tmp_hfindr (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, fsizeof (p->fname), p->optarg,
               strlen (p->optarg), MEMCMP2_REL, 1);
  return 0;
}


static int
ucon64_tmp_dfind (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, fsizeof (p->fname), p->optarg,
               strlen (p->optarg), MEMCMP2_WCARD ('?'), 1);
  return 0;
}


static int
ucon64_tmp_dfindr (st_ucon64_t *p)
{
  ucon64_find (p->fname, 0, fsizeof (p->fname), p->optarg,
               strlen (p->optarg), MEMCMP2_REL, 1);
  return 0;
}


static int
ucon64_tmp_pad (st_ucon64_t *p)
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

  fcopy (src_name, 0, fsizeof (p->fname), dest_name, "wb");
  if (truncate2 (dest_name, fsizeof (p->fname) + (MBIT - ((fsizeof (p->fname) - value) % MBIT))) == -1)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name); // msg is not a typo
      exit (1);
    }

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}


static int
ucon64_tmp_padhd (st_ucon64_t *p)                         // deprecated
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

  fcopy (src_name, 0, fsizeof (p->fname), dest_name, "wb");
  if (truncate2 (dest_name, fsizeof (p->fname) + (MBIT - ((fsizeof (p->fname) - value) % MBIT))) == -1)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name); // msg is not a typo
      exit (1);
    }

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  return 0;
}


static int
ucon64_tmp_p (st_ucon64_t *p)
{
  ucon64_tmp_pad (p);
  return 0;
}


static int
ucon64_tmp_padn (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);

  fcopy (src_name, 0, fsizeof (p->fname), dest_name, "wb");
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
ucon64_tmp_ispad (st_ucon64_t *p)
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
ucon64_tmp_strip (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, 0, fsizeof (p->fname) - strtol (p->optarg, NULL, 10),
    dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_tmp_stp (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, 512, fsizeof (p->fname), dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_tmp_stpn (st_ucon64_t *p)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  if (p->fname)
    {
      strcpy (src_name, p->fname);
      strcpy (dest_name, p->fname);
    }

  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, strtol (p->optarg, NULL, 10), fsizeof (p->fname), dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_tmp_ins (st_ucon64_t *p)
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
  fcopy (src_name, 0, fsizeof (p->fname), dest_name, "ab");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_tmp_insn (st_ucon64_t *p)
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
  fcopy (src_name, 0, fsizeof (p->fname), dest_name, "ab");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
ucon64_tmp_a (st_ucon64_t *p)
{
  aps_apply (p->fname, p->fname_optarg);
  return 0;
}


static int
ucon64_tmp_b (st_ucon64_t *p)
{
  bsl_apply (p->fname, p->fname_optarg);
  return 0;
}


static int
ucon64_tmp_i (st_ucon64_t *p)
{
  ips_apply (p->fname, p->fname_optarg);
  return 0;
}


static int
ucon64_tmp_ppf (st_ucon64_t *p)
{
  ppf_apply (p->fname, p->fname_optarg);
  return 0;
}


static int
ucon64_tmp_mka (st_ucon64_t *p)
{
  aps_create (p->optarg, p->fname);          // original, modified
  return 0;
}


static int
ucon64_tmp_mki (st_ucon64_t *p)
{
  ips_create (p->optarg, p->fname);          // original, modified
  return 0;
}


static int
ucon64_tmp_mkppf (st_ucon64_t *p)
{
  ppf_create (p->optarg, p->fname);          // original, modified
  return 0;
}


static int
ucon64_tmp_na (st_ucon64_t *p)
{
  aps_set_desc (p->fname, p->optarg);
  return 0;
}


static int
ucon64_tmp_nppf (st_ucon64_t *p)
{
  ppf_set_desc (p->fname, p->optarg);
  return 0;
}


static int
ucon64_tmp_idppf (st_ucon64_t *p)
{
  ppf_set_fid (p->fname, p->optarg);
  return 0;
}


static int
ucon64_tmp_lsd (st_ucon64_t *p)
{
  if (p->dat_enabled)
    {
      if (p->crc32)
        {
          fputs (basename2 (p->fname), stdout);
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
ucon64_tmp_scan (st_ucon64_t *p)
{
  ucon64_tmp_lsd (p);
  return 0;
}


static int
ucon64_tmp_lsv (st_ucon64_t *p)
{
#warning huh?
//  if (p->nfo)
//    ucon64_nfo ();
  return 0;
}


static int
ucon64_tmp_ls (st_ucon64_t *p)
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
                fsizeof (p->fname), buf, basename2 (p->fname));
        fputc ('\n', stdout);
      }
  return 0;
}


static int
ucon64_tmp_rdat (st_ucon64_t *p)
{
  ucon64_rename (UCON64_RDAT);
  return 0;
}


static int
ucon64_tmp_rrom (st_ucon64_t *p)
{
  ucon64_rename (UCON64_RROM);
  return 0;
}


static int
ucon64_tmp_r83 (st_ucon64_t *p)
{
  ucon64_rename (UCON64_R83);
  return 0;
}


static int
ucon64_tmp_rjoliet (st_ucon64_t *p)
{
  ucon64_rename (UCON64_RJOLIET);
  return 0;
}


static int
ucon64_tmp_rl (st_ucon64_t *p)
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
ucon64_tmp_ru (st_ucon64_t *p)
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
ucon64_tmp_dbv (st_ucon64_t *p)
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
ucon64_tmp_db (st_ucon64_t *p)
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
  else return ucon64_tmp_dbv (p);
}


static int
ucon64_tmp_dbs (st_ucon64_t *p)
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
ucon64_tmp_mkdat (st_ucon64_t *p)
{
  ucon64_create_dat (p->optarg, p->fname, p->nfo ? p->nfo->backup_header_len : 0);
  return 0;
}


static int
ucon64_tmp_multi (st_ucon64_t *p)
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
ucon64_tmp_e (st_ucon64_t *p)
{
  ucon64_e ();
  return 0;
}


static int
ucon64_tmp_1991 (st_ucon64_t *p)
{
  genesis_1991 (p->nfo);
  return 0;
}


static int
ucon64_tmp_b0 (st_ucon64_t *p)
{
  lynx_b0 (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_b1 (st_ucon64_t *p)
{
  lynx_b1 (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_bin (st_ucon64_t *p)
{
  genesis_bin (p->nfo);
  return 0;
}


static int
ucon64_tmp_bot (st_ucon64_t *p)
{
  n64_bot (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_chk (st_ucon64_t *p)
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
ucon64_tmp_col (st_ucon64_t *p)
{
  snes_col (p->optarg);
  return 0;
}


static int
ucon64_tmp_crp (st_ucon64_t *p)
{
  gba_crp (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_dbuh (st_ucon64_t *p)
{
  snes_backup_header_info (p->nfo);
  return 0;
}


static int
ucon64_tmp_dint (st_ucon64_t *p)
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
ucon64_tmp_swap (st_ucon64_t *p)
{
  ucon64_tmp_dint (p);
  return 0;
}


static int
ucon64_tmp_swap2 (st_ucon64_t *p)
{
  // --swap2 is currently used only for Nintendo 64
  n64_swap2 (p->nfo);
  return 0;
}


static int
ucon64_tmp_dmirr (st_ucon64_t *p)
{
  snes_demirror (p->nfo);
  return 0;
}


static int
ucon64_tmp_dnsrt (st_ucon64_t *p)
{
  snes_densrt (p->nfo);
  return 0;
}


static int
ucon64_tmp_f (st_ucon64_t *p)
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
ucon64_tmp_fds (st_ucon64_t *p)
{
  nes_fds ();
  return 0;
}


static int
ucon64_tmp_fdsl (st_ucon64_t *p)
{
  nes_fdsl (p->nfo, NULL);
  return 0;
}


static int
ucon64_tmp_ffe (st_ucon64_t *p)
{
  nes_ffe (p->nfo);
  return 0;
}


static int
ucon64_tmp_fig (st_ucon64_t *p)
{
  snes_fig (p->nfo);
  return 0;
}


static int
ucon64_tmp_figs (st_ucon64_t *p)
{
  snes_figs (p->nfo);
  return 0;
}


static int
ucon64_tmp_gbx (st_ucon64_t *p)
{
  gb_gbx (p->nfo);
  return 0;
}


static int
ucon64_tmp_gd3 (st_ucon64_t *p)
{
  snes_gd3 (p->nfo);
  return 0;
}


static int
ucon64_tmp_gd3s (st_ucon64_t *p)
{
  snes_gd3s (p->nfo);
  return 0;
}


static int
ucon64_tmp_gg (st_ucon64_t *p)
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
ucon64_tmp_ggd (st_ucon64_t *p)
{
  gg_display (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_gge (st_ucon64_t *p)
{
  gg_display (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_ines (st_ucon64_t *p)
{
  nes_ines ();
  return 0;
}


static int
ucon64_tmp_ineshd (st_ucon64_t *p)
{
  nes_ineshd (p->nfo);
  return 0;
}


#if 0
static int
ucon64_tmp_ip (st_ucon64_t *p)
{
  return 0;
}


static int
ucon64_tmp_vms (st_ucon64_t *p)
{
  return 0;
}
#endif


static int
ucon64_tmp_parse (st_ucon64_t *p)
{
  dc_parse (p->optarg);
  return 0;
}


static int
ucon64_tmp_mkip (st_ucon64_t *p)
{
  dc_mkip ();
  return 0;
}


static int
ucon64_tmp_j (st_ucon64_t *p)
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
ucon64_tmp_k (st_ucon64_t *p)
{
  snes_k (p->nfo);
  return 0;
}


static int
ucon64_tmp_l (st_ucon64_t *p)
{
  snes_l (p->nfo);
  return 0;
}


static int
ucon64_tmp_lnx (st_ucon64_t *p)
{
  lynx_lnx (p->nfo);
  return 0;
}


static int
ucon64_tmp_logo (st_ucon64_t *p)
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
ucon64_tmp_lsram (st_ucon64_t *p)
{
  n64_sram (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_lyx (st_ucon64_t *p)
{
  lynx_lyx (p->nfo);
  return 0;
}


static int
ucon64_tmp_mgd (st_ucon64_t *p)
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
ucon64_tmp_mgdgg (st_ucon64_t *p)
{
  sms_mgd (p->nfo, UCON64_GAMEGEAR);
  return 0;
}


static int
ucon64_tmp_msg (st_ucon64_t *p)
{
  pce_msg (p->nfo);
  return 0;
}


static int
ucon64_tmp_n (st_ucon64_t *p)
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
ucon64_tmp_n2 (st_ucon64_t *p)
{
  genesis_n2 (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_n2gb (st_ucon64_t *p)
{
  gb_n2gb (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_nrot (st_ucon64_t *p)
{
  lynx_nrot (p->nfo);
  return 0;
}


static int
ucon64_tmp_pasofami (st_ucon64_t *p)
{
  nes_pasofami ();
  return 0;
}


static int
ucon64_tmp_pattern (st_ucon64_t *p)
{
  ucon64_pattern (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_poke (st_ucon64_t *p)
{
  patch_poke (&ucon64);
  return 0;
}


static int
ucon64_tmp_rotl (st_ucon64_t *p)
{
  lynx_rotl (p->nfo);
  return 0;
}


static int
ucon64_tmp_rotr (st_ucon64_t *p)
{
  lynx_rotr (p->nfo);
  return 0;
}


static int
ucon64_tmp_s (st_ucon64_t *p)
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
ucon64_tmp_scr (st_ucon64_t *p)
{
  dc_scramble ();
  return 0;
}


static int
ucon64_tmp_sgb (st_ucon64_t *p)
{
  gb_sgb (p->nfo);
  return 0;
}


#ifdef  HAVE_MATH_H
static int
ucon64_tmp_cc2 (st_ucon64_t *p)
{
  printf (ucon64_msg[UNTESTED]);
  atari_cc2 (p->fname, p->optarg ? strtol (p->optarg, NULL, 10) : 0);
  return 0;
}
#endif


static int
ucon64_tmp_smc (st_ucon64_t *p)
{
  snes_smc (p->nfo);
  return 0;
}


static int
ucon64_tmp_smd (st_ucon64_t *p)
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
ucon64_tmp_smds (st_ucon64_t *p)
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
ucon64_tmp_sram (st_ucon64_t *p)
{
  gba_sram ();
  return 0;
}


static int
ucon64_tmp_sc (st_ucon64_t *p)
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
ucon64_tmp_ssc (st_ucon64_t *p)
{
  gb_ssc (p->nfo);
  return 0;
}


static int
ucon64_tmp_swc (st_ucon64_t *p)
{
  snes_swc (p->nfo);
  return 0;
}


static int
ucon64_tmp_swcs (st_ucon64_t *p)
{
  snes_swcs (p->nfo);
  return 0;
}


static int
ucon64_tmp_ufo (st_ucon64_t *p)
{
  snes_ufo (p->nfo);
  return 0;
}


static int
ucon64_tmp_ufos (st_ucon64_t *p)
{
  snes_ufos (p->nfo);
  return 0;
}


static int
ucon64_tmp_unif (st_ucon64_t *p)
{
  nes_unif ();
  return 0;
}


static int
ucon64_tmp_unscr (st_ucon64_t *p)
{
  dc_unscramble ();
  return 0;
}


static int
ucon64_tmp_usms (st_ucon64_t *p)
{
  n64_usms (p->nfo, p->optarg);
  return 0;
}


static int
ucon64_tmp_v64 (st_ucon64_t *p)
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
ucon64_tmp_xcd64 (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_rom (p->fname, 64);
  else
    cd64_write_rom (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xcd64c (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  cd64_read_rom (p->fname, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xcd64b (st_ucon64_t *p)
{
  cd64_write_bootemu (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xcd64s (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_sram (p->fname);
  else
    cd64_write_sram (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xcd64f (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_flashram (p->fname);
  else
    cd64_write_flashram (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xcd64e (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    cd64_read_eeprom (p->fname);
  else
    cd64_write_eeprom (p->fname);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xcd64m (st_ucon64_t *p)
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
ucon64_tmp_xreset (st_ucon64_t *p)
{
  parport_print_info ();
  fputs ("Resetting parallel port...", stdout);
  outportb ((unsigned short) (p->parport + PARPORT_DATA), 0);
  outportb ((unsigned short) (p->parport + PARPORT_CONTROL), 0);
  puts ("done");
  return 0;
}


static int
ucon64_tmp_xcmc (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  cmc_read_rom (p->fname, p->parport, p->io_mode); // p->io_mode contains speed value
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xcmct (st_ucon64_t *p)
{
  cmc_test (strtol (p->optarg, NULL, 10), p->parport, p->io_mode);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xdex (st_ucon64_t *p)
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
ucon64_tmp_xdjr (st_ucon64_t *p)
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
ucon64_tmp_xfal (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    fal_read_rom (p->fname, p->parport, 32);
  else
    fal_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xfalmulti (st_ucon64_t *p)
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
      fal_write_rom (src_name, p->parport);
    }

  unregister_func (remove_temp_file);
  remove_temp_file ();
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xfalc (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  fal_read_rom (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xfals (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    fal_read_sram (p->fname, p->parport, UCON64_UNKNOWN);
  else
    fal_write_sram (p->fname, p->parport, UCON64_UNKNOWN);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xfalb (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    fal_read_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  else
    fal_write_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xfig (st_ucon64_t *p)
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
ucon64_tmp_xfigs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    fig_read_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    fig_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xfigc (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cart SRAM contents
    fig_read_cart_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    fig_write_cart_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xgbx (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump cartridge/flash card
    gbx_read_rom (p->fname, p->parport);
  else                                      // file exists -> send it to the programmer
    gbx_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xgbxs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gbx_read_sram (p->fname, p->parport, -1);
  else
    gbx_write_sram (p->fname, p->parport, -1);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xgbxb (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gbx_read_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  else
    gbx_write_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xgd3 (st_ucon64_t *p)
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
ucon64_tmp_xgd3s (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    gd3_read_sram (p->fname, p->parport); // dumping is not yet supported
  else                                      // file exists -> restore SRAM
    gd3_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xgd3r (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gd3_read_saver (p->fname, p->parport);
  else
    gd3_write_saver (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xgd6 (st_ucon64_t *p)
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
ucon64_tmp_xgd6s (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gd6_read_sram (p->fname, p->parport);
  else
    gd6_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xgd6r (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    gd6_read_saver (p->fname, p->parport);
  else
    gd6_write_saver (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xgg (st_ucon64_t *p)
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
ucon64_tmp_xggs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    smsgg_read_sram (p->fname, p->parport, -1);
  else
    smsgg_write_sram (p->fname, p->parport, -1);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xggb (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    smsgg_read_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  else
    smsgg_write_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xlit (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  lynxit_read_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xmccl (st_ucon64_t *p)
{
  printf (ucon64_msg[UNTESTED]);
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  mccl_read (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xmcd (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  mcd_read_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xmd (st_ucon64_t *p)
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
ucon64_tmp_xmds (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    md_read_sram (p->fname, p->parport, -1);
  else                                      // file exists -> restore SRAM
    md_write_sram (p->fname, p->parport, -1);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xmdb (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    md_read_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  else
    md_write_sram (p->fname, p->parport, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xmsg (st_ucon64_t *p)
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
ucon64_tmp_xpce (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    pce_read_rom (p->fname, p->parport, 32 * MBIT);
  else
    pce_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xpl (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    pl_read_rom (p->fname, p->parport);
  else
    pl_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xpli (st_ucon64_t *p)
{
  pl_info (p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xsf (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump flash card
    sf_read_rom (p->fname, p->parport, 64 * MBIT);
  else                                      // file exists -> send it to the Super Flash
    sf_write_rom (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xsfs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    sf_read_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    sf_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xsmc (st_ucon64_t *p)
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
ucon64_tmp_xsmcr (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    smc_read_rts (p->fname, p->parport);
  else
    smc_write_rts (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xsmd (st_ucon64_t *p)
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
ucon64_tmp_xsmds (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    smd_read_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    smd_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xswc2 (st_ucon64_t *p)
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
ucon64_tmp_xswc (st_ucon64_t *p)
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
ucon64_tmp_xswcs (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    swc_read_sram (p->fname, p->parport);
  else                                      // file exists -> restore SRAM
    swc_write_sram (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xswcc (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)       // file does not exist -> dump SRAM contents
    swc_read_cart_sram (p->fname, p->parport, p->io_mode);
  else                                      // file exists -> restore SRAM
    swc_write_cart_sram (p->fname, p->parport, p->io_mode);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xswcr (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    swc_read_rts (p->fname, p->parport);
  else
    swc_write_rts (p->fname, p->parport);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xv64 (st_ucon64_t *p)
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
                        fsizeof (p->fname), p->parport);
    }
  fputc ('\n', stdout);
  return 0;
}
#endif // USE_PARALLEL


#if     defined USE_PARALLEL || defined USE_USB
static int
ucon64_tmp_xf2a (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    f2a_read_rom (p->fname, 32);
  else
    f2a_write_rom (p->fname, UCON64_UNKNOWN);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xf2amulti (st_ucon64_t *p)
{
  f2a_write_rom (NULL, strtol (p->optarg, NULL, 10) * MBIT);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xf2ac (st_ucon64_t *p)
{
  if (!access (p->fname, F_OK) && p->backup)
    printf ("Wrote backup to: %s\n", mkbak (p->fname, BAK_MOVE));
  f2a_read_rom (p->fname, strtol (p->optarg, NULL, 10));
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xf2as (st_ucon64_t *p)
{
  if (access (p->fname, F_OK) != 0)
    f2a_read_sram (p->fname, UCON64_UNKNOWN);
  else
    f2a_write_sram (p->fname, UCON64_UNKNOWN);
  fputc ('\n', stdout);
  return 0;
}


static int
ucon64_tmp_xf2ab (st_ucon64_t *p)
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
ucon64_tmp_z64 (st_ucon64_t *p)
{
  n64_z64 (p->nfo);
  return 0;
}


st_ucon64_filter_t ucon64_filter[] = {
  {
    UCON64_CRCHD,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM,
    ucon64_tmp_crchd
  },
  {
    UCON64_CRC,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM,
    ucon64_tmp_crc
  },
  {
    UCON64_SHA1,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM,
    ucon64_tmp_sha1
  },
  {
    UCON64_MD5,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM,
    ucon64_tmp_md5
  },
  {
    UCON64_HEX,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_hex
  },
  {
    UCON64_BITS,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_bits
  },
  {
    UCON64_CODE,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_code
  },
  {
    UCON64_PRINT,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_print
  },
  {
    UCON64_C,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_c
  },
  {
    UCON64_CS,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_cs
  },
  {
    UCON64_FIND,
    0,
    WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_find
  },
  {
    UCON64_FINDR,
    0,
    WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_findr
  },
  {
    UCON64_FINDI,
    0,
    WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_findi
  },
  {
    UCON64_HFIND,
    0,
    WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_hfind
  },
  {
    UCON64_HFINDR,
    0,
    WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_hfindr
  },
  {
    UCON64_DFIND,
    0,
    WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_dfind
  },
  {
    UCON64_DFINDR,
    0,
    WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_dfindr
  },
  {
    UCON64_PADHD,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_padhd
  },
  {
    UCON64_P,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_p
  },
  {
    UCON64_PAD,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_pad
  },
  {
    UCON64_PADN,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_padn
  },
  {
    UCON64_ISPAD,
    0,
    WF_OPEN|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_ispad
  },
  {
    UCON64_STRIP,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_strip
  },
  {
    UCON64_STP,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_stp
  },
  {
    UCON64_STPN,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_stpn
  },
  {
    UCON64_INS,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_ins
  },
  {
    UCON64_INSN,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_insn
  },
  {
    UCON64_A,
    0,
    0,
    ucon64_tmp_a
  },
  {
    UCON64_B,
    0,
    0,
    ucon64_tmp_b
  },
  {
    UCON64_I,
    0,
    0,
    ucon64_tmp_i
  },
  {
    UCON64_PPF,
    0,
    0,
    ucon64_tmp_ppf
  },
  {
    UCON64_MKA,
    0,
    0,
    ucon64_tmp_mka
  },
  {
    UCON64_MKI,
    0,
    0,
    ucon64_tmp_mki
  },
  {
    UCON64_MKPPF,
    0,
    0,
    ucon64_tmp_mkppf
  },
  {
    UCON64_NA,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_na
  },
  {
    UCON64_NPPF,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_nppf
  },
  {
    UCON64_IDPPF,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_idppf
  },
  {
    UCON64_SCAN,
    0,
    WF_DEMUX|WF_OPEN|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_scan
  },
  {
    UCON64_LSD,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_lsd
  },
  {
    UCON64_LSV,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_lsv
  },
  {
    UCON64_LS,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_ls
  },
  {
    UCON64_RDAT,
    0,
    WF_DEMUX|WF_OPEN|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_rdat
  },
  {
    UCON64_RROM,
    0,
    WF_DEMUX|WF_OPEN|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_rrom
  },
  {
    UCON64_R83,
    0,
    0,
    ucon64_tmp_r83
  },
  {
    UCON64_RJOLIET,
    0,
    0,
    ucon64_tmp_rjoliet
  },
  {
    UCON64_RL,
    0,
    0,
    ucon64_tmp_rl
  },
  {
    UCON64_RU,
    0,
    0,
    ucon64_tmp_ru
  },
  {
    UCON64_DB,
    0,
    WF_NEEDS_CRC32,
    ucon64_tmp_db
  },
  {
    UCON64_DBV,
    0,
    WF_NEEDS_CRC32,
    ucon64_tmp_dbv
  },
  {
    UCON64_DBS,
    0,
    WF_NEEDS_CRC32,
    ucon64_tmp_dbs
  },
  {
    UCON64_MKDAT,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_mkdat
  },
  {
    UCON64_MULTI,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_multi
  },
  {
    UCON64_E,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_e
  },
  {
    UCON64_1991,
    UCON64_GEN,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_1991
  },
  {
    UCON64_B0,
    UCON64_LYNX,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_b0
  },
  {
    UCON64_B1,
    UCON64_LYNX,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_b1
  },
  {
    UCON64_BIN,
    UCON64_GEN,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_bin
  },
  {
    UCON64_BOT,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_bot
  },
  {
    UCON64_CHK,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_chk
  },
  {
    UCON64_COL,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_col
  },
  {
    UCON64_CRP,
    UCON64_GBA,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_crp
  },
  {
    UCON64_DBUH,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_dbuh
  },
  {
    UCON64_SWAP,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_swap
  },
  {
    UCON64_DINT,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_dint
  },
  {
    UCON64_SWAP2,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_swap2
  },
  {
    UCON64_DMIRR,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_dmirr
  },
  {
    UCON64_DNSRT,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_dnsrt
  },
  {
    UCON64_F,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_f
  },
  {
    UCON64_FDS,
    UCON64_NES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_fds
  },
  {
    UCON64_FDSL,
    UCON64_NES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_fdsl
  },
  {
    UCON64_FFE,
    UCON64_NES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_ffe
  },
  {
    UCON64_FIG,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_fig
  },
  {
    UCON64_FIGS,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_figs
  },
  {
    UCON64_GBX,
    UCON64_GB,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_gbx
  },
  {
    UCON64_GD3,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_gd3
  },
  {
    UCON64_GD3S,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_gd3s
  },
  {
    UCON64_GG,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_gg
  },
  {
    UCON64_GGD,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_CRC32,
    ucon64_tmp_ggd
  },
  {
    UCON64_GGE,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_CRC32,
    ucon64_tmp_gge
  },
  {
    UCON64_INES,
    UCON64_NES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_ines
  },
  {
    UCON64_INESHD,
    UCON64_NES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_ineshd
  },
#if 0
  {
    UCON64_IP,
    0,
    0,
    ucon64_tmp_ip
  },
  {
    UCON64_VMS,
    0,
    0,
    ucon64_tmp_vms
  },
#endif
  {
    UCON64_PARSE,
    UCON64_DC,
    WF_NEEDS_CRC32,
    ucon64_tmp_parse
  },
  {
    UCON64_MKIP,
    UCON64_DC,
    WF_NEEDS_CRC32,
    ucon64_tmp_mkip
  },
  {
    UCON64_J,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_j
  },
  {
    UCON64_K,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_k
  },
  {
    UCON64_L,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_l
  },
  {
    UCON64_LNX,
    UCON64_LYNX,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_lnx
  },
  {
    UCON64_LOGO,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_logo
  },
  {
    UCON64_LSRAM,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_lsram
  },
  {
    UCON64_LYX,
    UCON64_LYNX,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_lyx
  },
  {
    UCON64_MGD,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_mgd
  },
  {
    UCON64_MGDGG,
    UCON64_SMS,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_mgdgg
  },
  {
    UCON64_MSG,
    UCON64_PCE,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_msg
  },
  {
    UCON64_N,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_n
  },
  {
    UCON64_N2,
    UCON64_GEN,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_n2
  },
  {
    UCON64_N2GB,
    UCON64_GB,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_n2gb
  },
  {
    UCON64_NROT,
    UCON64_LYNX,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_nrot
  },
  {
    UCON64_PASOFAMI,
    UCON64_NES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_pasofami
  },
  {
    UCON64_PATTERN,
    0,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_pattern
  },
  {
    UCON64_POKE,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_poke
  },
  {
    UCON64_ROTL,
    UCON64_LYNX,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_rotl
  },
  {
    UCON64_ROTR,
    UCON64_LYNX,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_rotr
  },
  {
    UCON64_S,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_s
  },
  {
    UCON64_SCR,
    UCON64_DC,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_scr
  },
  {
    UCON64_SGB,
    UCON64_GB,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_sgb
  },
#ifdef  HAVE_MATH_H
  {
    UCON64_CC2,
    UCON64_ATA,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_cc2
  },
#endif
  {
    UCON64_SMC,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_smc
  },
  {
    UCON64_SMD,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_smd
  },
  {
    UCON64_SMDS,
    UCON64_UNKNOWN,
    0,
    ucon64_tmp_smds
  },
  {
    UCON64_SRAM,
    UCON64_GBA,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_sram
  },
  {
    UCON64_SC,
    UCON64_GBA,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_sc
  },
  {
    UCON64_SSC,
    UCON64_GB,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_ssc
  },
  {
    UCON64_SWC,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_swc
  },
  {
    UCON64_SWCS,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_swcs
  },
  {
    UCON64_UFO,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_ufo
  },
  {
    UCON64_UFOS,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_ufos
  },
  {
    UCON64_UNIF,
    UCON64_NES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_unif
  },
  {
    UCON64_UNSCR,
    UCON64_DC,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_unscr
  },
  {
    UCON64_USMS,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_usms
  },
  {
    UCON64_V64,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_v64
  },
#ifdef  USE_PARALLEL
#ifdef  USE_LIBCD64
  {
    UCON64_XCD64,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xcd64
  },
  {
    UCON64_XCD64C,
    UCON64_N64,
    0,
    ucon64_tmp_xcd64c
  },
  {
    UCON64_XCD64B,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_xcd64b
  },
  {
    UCON64_XCD64S,
    UCON64_N64,
    WF_NEEDS_CRC32,
    ucon64_tmp_xcd64s
  },
  {
    UCON64_XCD64F,
    UCON64_N64,
    WF_NEEDS_CRC32,
    ucon64_tmp_xcd64f
  },
  {
    UCON64_XCD64E,
    UCON64_N64,
    WF_NEEDS_CRC32,
    ucon64_tmp_xcd64e
  },
  {
    UCON64_XCD64M,
    UCON64_N64,
    WF_NEEDS_CRC32,
    ucon64_tmp_xcd64m
  },
#endif
  {
    UCON64_XRESET,
    0,
    WF_NEEDS_CRC32,
    ucon64_tmp_xreset
  },
  {
    UCON64_XCMC,
    UCON64_GEN,
    WF_NEEDS_CRC32,
    ucon64_tmp_xcmc
  },
  {
    UCON64_XCMCT,
    UCON64_GEN,
    WF_NEEDS_CRC32,
    ucon64_tmp_xcmct
  },
  {
    UCON64_XDEX,
    0,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xdex
  },
  {
    UCON64_XDJR,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xdjr
  },
  {
    UCON64_XFAL,
    UCON64_GBA,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xfal
  },
  {
    UCON64_XFALMULTI,
    UCON64_GBA,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_xfalmulti
  },
  {
    UCON64_XFALC,
    UCON64_GBA,
    WF_NEEDS_CRC32,
    ucon64_tmp_xfalc
  },
  {
    UCON64_XFALS,
    UCON64_GBA,
    WF_NEEDS_CRC32,
    ucon64_tmp_xfals
  },
  {
    UCON64_XFALB,
    UCON64_GBA,
    WF_NEEDS_CRC32,
    ucon64_tmp_xfalb
  },
  {
    UCON64_XFIG,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xfig
  },
  {
    UCON64_XFIGS,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xfigs
  },
  {
    UCON64_XFIGC,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xfigc
  },
  {
    UCON64_XGBX,
    UCON64_GB,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xgbx
  },
  {
    UCON64_XGBXS,
    UCON64_GB,
    WF_NEEDS_CRC32,
    ucon64_tmp_xgbxs
  },
  {
    UCON64_XGBXB,
    UCON64_GB,
    WF_NEEDS_CRC32,
    ucon64_tmp_xgbxb
  },
  {
    UCON64_XGD3,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xgd3
  },
  {
    UCON64_XGD3S,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xgd3s
  },
  {
    UCON64_XGD3R,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xgd3r
  },
  {
    UCON64_XGD6,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xgd6
  },
  {
    UCON64_XGD6S,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xgd6s
  },
  {
    UCON64_XGD6R,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xgd6r
  },
  {
    UCON64_XGG,
    UCON64_SMS,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xgg
  },
  {
    UCON64_XGGS,
    UCON64_SMS,
    WF_NEEDS_CRC32,
    ucon64_tmp_xggs
  },
  {
    UCON64_XGGB,
    UCON64_SMS,
    WF_NEEDS_CRC32,
    ucon64_tmp_xggb
  },
  {
    UCON64_XLIT,
    UCON64_LYNX,
    WF_NEEDS_CRC32,
    ucon64_tmp_xlit
  },
  {
    UCON64_XMCCL,
    UCON64_GB,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xmccl
  },
  {
    UCON64_XMCD,
    UCON64_GEN,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xmcd
  },
  {
    UCON64_XMD,
    UCON64_GEN,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xmd
  },
  {
    UCON64_XMDS,
    UCON64_GEN,
    WF_NEEDS_CRC32,
    ucon64_tmp_xmds
  },
  {
    UCON64_XMDB,
    UCON64_GEN,
    WF_NEEDS_CRC32,
    ucon64_tmp_xmdb
  },
  {
    UCON64_XMSG,
    UCON64_PCE,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xmsg
  },
  {
    UCON64_XPCE,
    UCON64_PCE,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xpce
  },
  {
    UCON64_XPL,
    UCON64_NGP,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xpl
  },
  {
    UCON64_XPLI,
    UCON64_NGP,
    WF_NEEDS_CRC32,
    ucon64_tmp_xpli
  },
  {
    UCON64_XSF,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xsf
  },
  {
    UCON64_XSFS,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xsfs
  },
  {
    UCON64_XSMC,
    UCON64_NES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_xsmc
  },
  {
    UCON64_XSMCR,
    UCON64_NES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xsmcr
  },
  {
    UCON64_XSMD,
    UCON64_GEN,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xsmd
  },
  {
    UCON64_XSMDS,
    UCON64_GEN,
    WF_NEEDS_CRC32,
    ucon64_tmp_xsmds
  },
  {
    UCON64_XSWC,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xswc
  },
  {
    UCON64_XSWC2,
    UCON64_SNES,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NO_SPLIT|WF_NEEDS_CRC32,
    ucon64_tmp_xswc2
  },
  {
    UCON64_XSWCS,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xswcs
  },
  {
    UCON64_XSWCC,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xswcc
  },
  {
    UCON64_XSWCR,
    UCON64_SNES,
    WF_NEEDS_CRC32,
    ucon64_tmp_xswcr
  },
  {
    UCON64_XV64,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xv64
  },
#endif // USE_PARALLEL
#if     defined USE_PARALLEL || defined USE_USB
  {
    UCON64_XF2A,
    UCON64_GBA,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_CRC32,
    ucon64_tmp_xf2a
  },
  {
    UCON64_XF2AMULTI,
    UCON64_GBA,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_xf2amulti
  },
  {
    UCON64_XF2AC,
    UCON64_GBA,
    WF_NEEDS_CRC32,
    ucon64_tmp_xf2ac
  },
  {
    UCON64_XF2AS,
    UCON64_GBA,
    WF_NEEDS_CRC32,
    ucon64_tmp_xf2as
  },
  {
    UCON64_XF2AB,
    UCON64_GBA,
    WF_NEEDS_CRC32,
    ucon64_tmp_xf2ab
  },
#endif // USE_PARALLEL || USE_USB
  {
    UCON64_Z64,
    UCON64_N64,
    WF_DEMUX|WF_OPEN|WF_EXIT|WF_NEEDS_ROM|WF_NEEDS_CRC32,
    ucon64_tmp_z64
  },
  {
    UCON64_HELP,
    0,
    WF_EXIT,
    ucon64_tmp_help
  },
  {
    UCON64_VER,
    0,
    WF_EXIT,
    ucon64_tmp_ver
  },
/*
  switches

  switches are not console specific and have no flags
  the function is always ucon64_switches()
*/
#warning switches are not console specific and have no flags?
  {UCON64_BAT, 0, 0, NULL},
  {UCON64_BS, 0, 0, NULL},
  {UCON64_CMNT, 0, 0, NULL},
  {UCON64_CTRL2, 0, 0, NULL},
  {UCON64_CTRL, 0, 0, NULL},
  {UCON64_DUMPINFO, 0, 0, NULL},
  {UCON64_EROM, 0, 0, NULL},
  {UCON64_FILE, 0, 0, NULL},  // deprecated
  {UCON64_FRONTEND, 0, 0, NULL},
  {UCON64_HD, 0, 0, NULL},
  {UCON64_HDN, 0, 0, NULL},
  {UCON64_HI, 0, 0, NULL},
  {UCON64_ID, 0, 0, NULL},
  {UCON64_IDNUM, 0, 0, NULL},
  {UCON64_INT2, 0, 0, NULL},
  {UCON64_INT, 0, 0, NULL},
  {UCON64_MAPR, 0, 0, NULL},
  {UCON64_MIRR, 0, 0, NULL},
  {UCON64_NBAK, 0, 0, NULL},
  {UCON64_NBAT, 0, 0, NULL},
  {UCON64_NBS, 0, 0, NULL},
  {UCON64_NCOL, 0, 0, NULL},
  {UCON64_NHD, 0, 0, NULL},
  {UCON64_NHI, 0, 0, NULL},
  {UCON64_NINT, 0, 0, NULL},
  {UCON64_NS, 0, 0, NULL},
  {UCON64_NTSC, 0, 0, NULL},
  {UCON64_NVRAM, 0, 0, NULL},
  {UCON64_O, 0, 0, NULL},
  {UCON64_PAL, 0, 0, NULL},
  {UCON64_PATCH, 0, 0, NULL},
  {UCON64_PORT, 0, 0, NULL},
  {UCON64_Q, 0, 0, NULL},
//  {UCON64_QQ, 0, 0, NULL},   // reserved
  {UCON64_R, 0, 0, NULL},
  {UCON64_REGION, 0, 0, NULL},
  {UCON64_ROM, 0, 0, NULL}, // deprecated
  {UCON64_SSIZE, 0, 0, NULL},
//  {UCON64_SWP, 0, 0, NULL},  // deprecated
  {UCON64_V, 0, 0, NULL},
  {UCON64_VRAM, 0, 0, NULL},
  {UCON64_XCD64P, 0, 0, NULL},
  {UCON64_XCMCM, 0, 0, NULL},
  {UCON64_XFALM, 0, 0, NULL},
  {UCON64_XGBXM, 0, 0, NULL},
  {UCON64_XPLM, 0, 0, NULL},
  {UCON64_XSWC_IO, 0, 0, NULL},

//  {UCON64_3DO, 0, 0, NULL},
  {UCON64_ATA, 0, 0, NULL},
//  {UCON64_CD32, 0, 0, NULL},
//  {UCON64_CDI, 0, 0, NULL},
  {UCON64_COLECO, 0, 0, NULL},
  {UCON64_DC, 0, 0, NULL},
  {UCON64_GB, 0, 0, NULL},
  {UCON64_GBA, 0, 0, NULL},
//  {UCON64_GC, 0, 0, NULL},
  {UCON64_GEN, 0, 0, NULL},
//  {UCON64_GP32, 0, 0, NULL},
//  {UCON64_INTELLI, 0, 0, NULL},
  {UCON64_JAG, 0, 0, NULL},
  {UCON64_LYNX, 0, 0, NULL},
//  {UCON64_ARCADE, 0, 0, NULL},
  {UCON64_N64, 0, 0, NULL},
  {UCON64_NDS, 0, 0, NULL},
  {UCON64_NES, 0, 0, NULL},
//  {UCON64_NG, 0, 0, NULL},
  {UCON64_NGP, 0, 0, NULL},
  {UCON64_PCE, 0, 0, NULL},
//  {UCON64_PS2, 0, 0, NULL},
//  {UCON64_PSX, 0, 0, NULL},
//  {UCON64_S16, 0, 0, NULL},
//  {UCON64_SAT, 0, 0, NULL},
  {UCON64_SMS, 0, 0, NULL},
  {UCON64_GAMEGEAR, 0, 0, NULL},
  {UCON64_SNES, 0, 0, NULL},
  {UCON64_SWAN, 0, 0, NULL},
  {UCON64_VBOY, 0, 0, NULL},
//  {UCON64_VEC, 0, 0, NULL},
//  {UCON64_XBOX, 0, 0, NULL},
  {0, 0, 0, NULL}
};
