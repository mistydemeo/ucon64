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

#include "ucon64.h"

//#include "unzip.h"
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

#ifdef  CD
#include "cd32/cd32.h"
#include "ps2/ps2.h"
#include "saturn/saturn.h"
#include "cdi/cdi.h"
#include "psx/psx.h"
#include "dc/dc.h"
#include "real3do/real3do.h"
#include "gamecube/gamecube.h"
#include "xbox/xbox.h"

#include "patch/ppf.h"
#include "patch/xps.h"
#include "patch/pal4u.h"

#include "backup/cdrw.h"
#endif

#ifdef	BACKUP
#include "backup/fig.h"
#include "backup/swc.h"
#include "backup/unknown_bu.h"
#include "backup/unknown_bu512.h"
#endif

#include "patch/aps.h"
#include "patch/ips.h"
#include "patch/bsl.h"

FILE *frontend_file;
struct ucon64_ rom;

void
ucon64_exit (void)
{
  printf ("+++EOF");
  fflush (stdout);
  if (frontend_file != stdout)
    {
      fclose (frontend_file);
      remove (FRONTEND_FILENAME);
    }
}

int
main (int argc, char *argv[])
{
  long x, y = 0;
  int ucon64_argc, skip_init_nfo = 0;
  struct dirent *ep;
  struct stat puffer;
  DIR *dp;
  char buf[MAXBUFSIZE], buf2[FILENAME_MAX], buf3[4096], *ucon64_argv[128];
  char *forceargs[] = {
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
    "-xbox"
  };
#if     defined BACKUP && defined __UNIX__
  uid_t uid;
  gid_t gid;
#endif
  struct ucon64_ rom;


  ucon64_flush (argc, argv, &rom);
  if (!strlen (rom.rom))
    getcwd (rom.rom, sizeof (rom.rom));

  printf ("%s\n", ucon64_TITLE);
  printf ("Uses code from various people. See 'developers.html' for more!\n");
  printf ("This may be freely redistributed under the terms of the GNU Public License\n\n");

  if (argc < 2 ||
      argcmp (argc, argv, "-h") ||
      argcmp (argc, argv, "-help") ||
      argcmp (argc, argv, "-?"))
    {
      ucon64_usage (argc, argv);
      return (0);
    }

#ifdef  BACKUP
  if (rom.file[0])
    {
//      strcpy(buf, rom.file);
      sscanf (rom.file, "%x", &rom.parport);
    }

  if (!(rom.parport = parport_probe (rom.parport)))
    ;
/*
    printf ("ERROR: no parallel port 0x%s found\n\n", strupr (buf));
  else
    printf ("0x%x\n\n", rom.parport);
*/

#ifdef  __UNIX__
/*
  Some code needs us to switch to the real uid and gid. However, other code needs access to
  I/O ports other than the standard printer port registers. We just do an iopl(3) and all
  code should be happy. Using iopl(3) enables users to run all code without being root (of
  course with the uCON64 executable setuid root). Anyone a better idea?
*/
#ifdef  __linux__
  if (iopl (3) == -1)
    {
      fprintf (stderr, "Could not set the I/O privilege level to 3\n"
                       "(This program needs root privileges)\n");
      return (1);
    }
#endif

  // now we can drop privileges
  uid = getuid ();
  if (setuid (uid) == -1)
    {
      fprintf (stderr, "Could not set uid\n");
      return (1);
    }
  gid = getgid ();                              // This shouldn't be necessary if `make install'
  if (setgid (gid) == -1)                       //  was used, but just in case (root did `chmod +s')
    {
      fprintf (stderr, "Could not set gid\n");
      return (1);
    }
#endif // __UNIX__
#endif // BACKUP

/*
    support for frontends
*/

  frontend_file = stdout;
  if (argcmp (argc, argv, "-frontend"))
    {
/*
  this must disappear before 1.9.8
*/
      atexit (ucon64_exit);
#ifndef __BEOS__ // keep behaviour like 1.9.7 under BeOS until BeOS frontend is updated
      // if we can't create a file to write the parport_gauge() status to, set
      // frontend to 0, so we won't accidentily write to frontend_file
      if ((frontend_file = fopen (FRONTEND_FILENAME, "wt")) == NULL)
        frontend_file = stdout;
#endif
    }


/*
   configfile handling
*/
#ifdef  __DOS__
  sprintf (buf, "%s%cucon64.cfg", getchd (buf2, FILENAME_MAX), FILE_SEPARATOR);
//  strcpy (buf, "ucon64.cfg");
#else
  sprintf (buf, "%s%c.ucon64rc", getenv ("HOME"), FILE_SEPARATOR);
#endif

  if (access (buf, F_OK) == -1)
    printf ("WARNING: %s not found: creating...", buf);
  else if (strcmp (getProperty (buf, "version", buf2, ""), "198") != 0)
    {
/*
      strcpy (buf2, buf);

      newext (buf2, ".OLD");

      printf ("NOTE: updating config: old version will be renamed to %s...", buf2);

      filecopy (buf, 0, quickftell(buf), buf2, "wb");

      setProperty(buf,"version","198");

      setProperty(buf,"cdrw_read",
         getProperty (buf, "cdrw_raw_read", buf2, "cdrdao read-cd --read-raw --device 0,0,0 --driver generic-mmc-raw --datafile "));
      setProperty(buf,"cdrw_write",
         getProperty (buf, "cdrw_raw_write", buf2, "cdrdao write --device 0,0,0 --driver generic-mmc "));

      deleteProperty(buf,"cdrw_raw_read");
      deleteProperty(buf,"cdrw_raw_write");
      deleteProperty(buf,"cdrw_iso_read");
      deleteProperty(buf,"cdrw_iso_write");

      sync ();

      printf ("OK\n\n");
*/
      printf("ERROR: old config file (<1.9.8) found (%s), please remove it\n", buf);

      return 0;
    }

  if (access (buf, F_OK) == -1)
    {
      FILE *fh;

      if (!(fh = fopen (buf, "wb")))
        {
          printf ("FAILED\n\n");

//    return -1;
        }
      else
        {
          fputs ("# uCON64 config\n"
                 "#\n"
                 "version=198\n"
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
                 "emulate_swan=\n" "emulate_coleco=\n" "emulate_intelli=\n"
#ifdef CD
                 "emulate_psx=pcsx\n"
                 "emulate_ps2=\n"
                 "emulate_sat=\n"
                 "emulate_dc=\n"
                 "emulate_cd32=\n"
                 "emulate_cdi=\n"
                 "emulate_3do=\n"
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

//      return 0;
    }

/*
// TODO shell modus
  if (argcmp (argc, argv, "-sh"))
    {
      for (;;)
        printf ("ucon64>");
      return (0);
    }
*/

  if (argcmp (argc, argv, "-crc"))
    {
      printf ("Checksum: %08lx\n\n", fileCRC32 (rom.rom, 0));
      return (0);
    }

  if (argcmp (argc, argv, "-crchd"))
    {
      printf ("Checksum: %08lx\n\n", fileCRC32 (rom.rom, 512));
      return (0);
    }

  if (argcmp (argc, argv, "-rl"))
    {
      renlwr (rom.rom);
      return (0);
    }

  if (argcmp (argc, argv, "-ru"))
    {
      renupr (rom.rom);
      return (0);
    }

  if (argcmp (argc, argv, "-hex"))
    {
      filehexdump (rom.rom, 0, quickftell (rom.rom));
      return (0);
    }

  if (argcmp (argc, argv, "-c"))
    {
      if (filefile (rom.rom, 0, rom.file, 0, FALSE) == -1)
        printf ("ERROR: file not found/out of memory\n");
      return (0);
    }

  if (argcmp (argc, argv, "-cs"))
    {
      if (filefile (rom.rom, 0, rom.file, 0, TRUE) == -1)
        printf ("ERROR: file not found/out of memory\n");
      return (0);
    }

  if (argcmp (argc, argv, "-find"))
    {
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
      return (0);
    }

  if (argcmp (argc, argv, "-swap"))
    {
      fileswap (filebackup (rom.rom), 0, quickftell (rom.rom));
      return (0);
    }

  if (argcmp (argc, argv, "-pad"))
    {
      filepad (rom.rom, 0, MBIT);
      return (0);
    }

  if (argcmp (argc, argv, "-padhd"))
    {
      filepad (rom.rom, 512, MBIT);
      return (0);
    }

  if (argcmp (argc, argv, "-ispad"))
    {
      unsigned long padded;

      if ((padded = filetestpad (rom.rom)) != -1)
        {
          if (!padded)
            printf ("Padded: No\n");
          else
            printf ("Padded: Maybe, %ld Bytes (%.4f Mb)\n", padded,
                    (float) padded / MBIT);
        }
      printf ("\n");
      return (0);
    }

  if (argcmp (argc, argv, "-strip"))
    {
      truncate (rom.rom, quickftell (rom.rom) - atol (rom.file));
      return (0);
    }

  if (argcmp (argc, argv, "-stp"))
    {
      strcpy (buf, filebackup (rom.rom));
      newext (buf, ".TMP");
      rename (rom.rom, buf);

      filecopy (buf, 512, quickftell (buf), rom.rom, "wb");
      remove (buf);
      return (0);
    }

  if (argcmp (argc, argv, "-ins"))
    {
      strcpy (buf, filebackup (rom.rom));
      newext (buf, ".TMP");
      rename (rom.rom, buf);

      memset (buf2, 0, 512);
      quickfwrite (buf2, 0, 512, rom.rom, "wb");

      filecopy (buf, 0, quickftell (buf), rom.rom, "ab");

      remove (buf);
      return (0);
    }

  if (argcmp (argc, argv, "-b"))
    {
      if (bsl (rom.rom, rom.file) != 0)
        printf ("ERROR: failed\n");
      return (0);
    }

  if (argcmp (argc, argv, "-i"))
    {
      ucon64_argv[0] = "ucon64";
      ucon64_argv[1] = rom.file;
      ucon64_argv[2] = rom.rom;
      ucon64_argc = 3;

      ips_main (ucon64_argc, ucon64_argv);

      return (0);
    }

  if (argcmp (argc, argv, "-a"))
    {
      ucon64_argv[0] = "ucon64";
      ucon64_argv[1] = "-f";
      ucon64_argv[2] = rom.rom;
      ucon64_argv[3] = rom.file;
      ucon64_argc = 4;

      n64aps_main (ucon64_argc, ucon64_argv);
      return (0);
    }


  if (argcmp (argc, argv, "-mki"))
    {
      cips (rom.rom, rom.file);
      return (0);
    }

  if (argcmp (argc, argv, "-mka"))
    {
      ucon64_argv[0] = "ucon64";
      ucon64_argv[1] = "-d \"\"";
      ucon64_argv[2] = rom.rom;
      ucon64_argv[3] = rom.file;
      strcpy (buf, rom.rom);
      newext (buf, ".APS");

      ucon64_argv[4] = buf;
      ucon64_argc = 5;

      n64caps_main (ucon64_argc, ucon64_argv);
      return (0);
    }

  if (argcmp (argc, argv, "-na"))
    {
      memset (buf2, ' ', 50);
      strncpy (buf2, rom.file, strlen (rom.file));
      quickfwrite (buf2, 7, 50, filebackup (rom.rom), "r+b");

      return (0);
    }

#ifdef  CD

  if (argcmp (argc, argv, "-ppf"))
    {
      ucon64_argv[0] = "ucon64";
      ucon64_argv[1] = rom.rom;
      ucon64_argv[2] = rom.file;
      ucon64_argc = 3;

      applyppf_main (ucon64_argc, ucon64_argv);
      return (0);
    }

  if (argcmp (argc, argv, "-mkppf"))
    {
      ucon64_argv[0] = "ucon64";
      ucon64_argv[1] = rom.rom;
      ucon64_argv[2] = rom.file;

      strcpy (buf, rom.file);
      newext (buf, ".PPF");

      ucon64_argv[3] = buf;
      ucon64_argc = 4;

      makeppf_main (ucon64_argc, ucon64_argv);
      return (0);
    }

  if (argcmp (argc, argv, "-nppf"))
    {
      memset (buf2, ' ', 50);
      strncpy (buf2, rom.file, strlen (rom.file));
      quickfwrite (buf2, 6, 50, filebackup (rom.rom), "r+b");

      return (0);
    }

  if (argcmp (argc, argv, "-idppf"))
    {
      addppfid (argc, argv);
      return (0);
    }

#endif

  if (argcmp (argc, argv, "-ls") || argcmp (argc, argv, "-lsv"))
//    || argcmp (argc, argv, "-rrom") || argcmp(argc,argv, "-rr83")
    {
      char current_dir[FILENAME_MAX];

      if (access (rom.rom, R_OK) == -1 || (dp = opendir (rom.rom)) == NULL)
        return (-1);

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

                  if (argcmp (argc, argv, "-ls"))
                    {
                      strftime (buf, 13, "%b %d %H:%M",
                                localtime (&puffer.st_mtime));
                      printf ("%-31.31s %10d %s %s\n", rom.name,
                              (int) puffer.st_size, buf, rom.rom);
                    }
                  else if (argcmp (argc, argv, "-lsv"))
                    ucon64_nfo (&rom);
/*        
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

      return (0);
    }

  rom.console =
    (argcmp (argc, argv, "-ata")) ? ucon64_ATARI :
    (argcmp (argc, argv, "-s16")) ? ucon64_SYSTEM16 :
    (argcmp (argc, argv, "-coleco")) ? ucon64_COLECO :
    (argcmp (argc, argv, "-vboy")) ? ucon64_VIRTUALBOY :
    (argcmp (argc, argv, "-swan")) ? ucon64_WONDERSWAN :
    (argcmp (argc, argv, "-vec")) ? ucon64_VECTREX :
    (argcmp (argc, argv, "-intelli")) ? ucon64_INTELLI :
    (argcmp (argc, argv, "-lynx")) ? ucon64_LYNX :
    (argcmp (argc, argv, "-sms")) ? ucon64_SMS :
    (argcmp (argc, argv, "-ngp")) ? ucon64_NEOGEOPOCKET :
    (argcmp (argc, argv, "-nes")) ? ucon64_NES :
    (argcmp (argc, argv, "-pce")) ? ucon64_PCE :
    (argcmp (argc, argv, "-jag")) ? ucon64_JAGUAR :
    (argcmp (argc, argv, "-gen")) ? ucon64_GENESIS :
    (argcmp (argc, argv, "-ng")) ? ucon64_NEOGEO :
    (argcmp (argc, argv, "-n64")) ? ucon64_N64 :
    (argcmp (argc, argv, "-snes")) ? ucon64_SNES :
    (argcmp (argc, argv, "-gba")) ? ucon64_GBA :
    (argcmp (argc, argv, "-gb")) ? ucon64_GB :
#ifdef	CD
    (argcmp (argc, argv, "-sat")) ? ucon64_SATURN :
    (argcmp (argc, argv, "-psx")) ? ucon64_PSX :
    (argcmp (argc, argv, "-ps2")) ? ucon64_PS2 :
    (argcmp (argc, argv, "-cdi")) ? ucon64_CDI :
    (argcmp (argc, argv, "-cd32")) ? ucon64_CD32 :
    (argcmp (argc, argv, "-3do")) ? ucon64_REAL3DO :
    (argcmp (argc, argv, "-dc")) ? ucon64_DC :
    (argcmp (argc, argv, "-xbox")) ? ucon64_XBOX :
    (argcmp (argc, argv, "-gc")) ? ucon64_GAMECUBE :
    (argcmp (argc, argv, "-ip")) ? ucon64_DC :
#endif
    (argcmp (argc, argv, "-col")) ? ucon64_SNES :
    (argcmp (argc, argv, "-n2gb")) ? ucon64_GB :
    (argcmp (argc, argv, "-sam")) ? ucon64_NEOGEO : ucon64_UNKNOWN;


/*
    database functions (optionally console dependend)
*/
  if (argcmp (argc, argv, "-db"))
    {
      printf ("Database: %ld known ROMs in ucon64_db.c (%+ld)\n\n",
              ucon64_dbsize (rom.console),
              ucon64_dbsize (rom.console) - ucon64_DBSIZE);

      printf
        ("TIP: %s -db -nes would show only the number of known NES ROMs\n\n",
         getarg (argc, argv, ucon64_NAME));
      return (0);
    }

  if (argcmp (argc, argv, "-dbs"))
    {
      ucon64_flush (argc, argv, &rom);

      sscanf (rom.rom, "%lx", &rom.current_crc32);

      ucon64_dbsearch (&rom);

      ucon64_nfo (&rom);

      ucon64_dbview (rom.console);

      printf ("TIP: %s -dbs -nes would search only for a NES ROM\n\n",
              getarg (argc, argv, ucon64_NAME));

      return (0);
    }

  if (argcmp (argc, argv, "-dbv"))
    {
      ucon64_dbview (rom.console);

      printf ("\nTIP: %s -db -nes would view only NES ROMs\n\n",
              getarg (argc, argv, ucon64_NAME));
      return (0);
    }

  if (argcmp (argc, argv, "-multi") ||          // This gets rid of nonsense GBA info
      argcmp (argc, argv, "-multi1") ||         //  on a GBA multirom loader binary
      argcmp (argc, argv, "-multi2") ||
      argcmp (argc, argv, "-swcs") ||
      argcmp (argc, argv, "-figs") ||
      argcmp (argc, argv, "-ufos"))
    {
      skip_init_nfo = 1;
    }

  /*
    Do the following outside the if that checks for existence of rom.rom
    (necessary for headerless SRAM files, which can't be detected without
    looking at the command line options)
  */
  rom.console =
    (argcmp (argc, argv, "-xdjr") ||
     argcmp (argc, argv, "-xv64")) ? ucon64_N64 :
     (argcmp (argc, argv, "-xgbx") ||
     argncmp (argc, argv, "-xgbxb", 6) ||
     argcmp (argc, argv, "-xgbxs")) ? ucon64_GB :
     (argcmp (argc, argv, "-xswc") ||
     argcmp (argc, argv, "-xswcs") ||
     argcmp (argc, argv, "-swcs") ||
     argcmp (argc, argv, "-figs") ||
     argcmp (argc, argv, "-ufos")) ? ucon64_SNES :
     (argcmp (argc, argv, "-multi") ||
     argcmp (argc, argv, "-multi1") ||
     argcmp (argc, argv, "-multi2") ||
     argcmp (argc, argv, "-xfal") ||
     argncmp (argc, argv, "-xfalc", 6) ||
     argncmp (argc, argv, "-xfalb", 6) ||
     argcmp (argc, argv, "-xfals")) ? ucon64_GBA : rom.console;
  if (!access (rom.rom, F_OK) && !skip_init_nfo)
    {
      ucon64_init (&rom);
      if (rom.console != ucon64_UNKNOWN)
        ucon64_nfo (&rom);
    }

  if (argcmp (argc, argv, "-e"))
    {
      char *property;
#ifdef	__DOS__
      strcpy (buf, "ucon64.cfg");
#else
      sprintf (buf, "%s%c.ucon64rc", getenv ("HOME"), FILE_SEPARATOR);
#endif

      if (rom.console != ucon64_UNKNOWN /* && rom.console != ucon64_KNOWN */ )
        sprintf (buf3, "emulate_%s", &forceargs[rom.console][1]);
      else
        {
          printf ("ERROR: could not auto detect the right ROM/console type\n"
                  "TIP:   If this is a ROM you might try to force the recognition\n"
                  "       The force recognition option for Super Nintendo would be -snes\n");
          return (-1);
        }

      if (access (buf, F_OK) == -1)
        {
          printf ("ERROR: %s does not exist\n", buf);
          return (-1);
        }

      property = getProperty (buf, buf3, buf2, NULL);   // buf2 also contains property value
      if (property == NULL)
        {
          printf ("ERROR: could not find the correct settings (%s) in\n"
                  "       %s\n"
                  "TIP:   If the wrong console was detected you might try to force recognition\n"
                  "       The force recognition option for Super Nintendo would be -snes\n",
                  buf3, buf);
          return (-1);
        }

      sprintf (buf, "%s %s", buf2, rom.file);
      for (x = 0; x < argc; x++)
        {
          if (strdcmp (argv[x], "-e")
              && strdcmp (argv[x], getarg (argc, argv, ucon64_NAME))
              && strdcmp (argv[x], rom.file))
            {
              sprintf (buf2, ((!strdcmp (argv[x], rom.rom)) ? " \"%s\"" :       /*" %s" */
                              ""), argv[x]);
              strcat (buf, buf2);
            }
        }

      printf ("%s\n", buf);
      fflush (stdout);
      sync ();

      x = system (buf);
#ifndef __DOS__
      x >>= 8;                  // the exit code is coded in bits 8-15
#endif                          //  (that is, under Linux & BeOS)

#if 1
      // Snes9x (Linux) for example returns a non-zero value on a normal exit (3)...

      // under WinDOS, system() immediately returns with exit code 0 when starting
      //  a Windows executable (as if fork() was called) it also returns 0 when the
      //  exe could not be started
      if (x != 127 && x != -1 && x != 0)        // 127 && -1 are system() errors, rest are exit codes
        {
          printf ("ERROR: the Emulator returned an error code (%d)\n"
                  "TIP:   If the wrong emulator was used you might try to force recognition\n"
                  "       The force recognition option for Super Nintendo would be -snes\n",
                  (int) x);
          return (x);
        }
#endif

      return (0);
    }

  switch (rom.console)
    {
    case ucon64_GB:
      return ((argcmp (argc, argv, "-chk")) ? gameboy_chk (&rom) :
              (argcmp (argc, argv, "-gbx")) ? gameboy_gbx (&rom) :
              (argcmp (argc, argv, "-gg")) ? gameboy_gg (&rom) :
              (argcmp (argc, argv, "-ggd")) ? gameboy_ggd (&rom) :
              (argcmp (argc, argv, "-gge")) ? gameboy_gge (&rom) :
              (argcmp (argc, argv, "-mgd")) ? gameboy_mgd (&rom) :
              (argcmp (argc, argv, "-n")) ? gameboy_n (&rom) :
              (argcmp (argc, argv, "-sgb")) ? gameboy_sgb (&rom) :
              (argcmp (argc, argv, "-ssc")) ? gameboy_ssc (&rom) :
              (argcmp (argc, argv, "-n2gb")) ? gameboy_n2gb (&rom) :
#ifdef  BACKUP
              (argcmp (argc, argv, "-xgbx")) ? gameboy_xgbx (&rom) :
              (argncmp (argc, argv, "-xgbxb", 6)) ? gameboy_xgbxb (&rom) :
              (argcmp (argc, argv, "-xgbxs")) ? gameboy_xgbxs (&rom) :
#endif
              0);
      break;

    case ucon64_GBA:
      return ((argcmp (argc, argv, "-chk")) ? gbadvance_chk (&rom) :
              (argcmp (argc, argv, "-crp")) ? gbadvance_crp (&rom) :
              (argcmp (argc, argv, "-logo")) ? gbadvance_logo (&rom) :
              (argcmp (argc, argv, "-n")) ? gbadvance_n (&rom) :
              (argcmp (argc, argv, "-sram")) ? gbadvance_sram (&rom) :
              (argcmp (argc, argv, "-multi")) ? gbadvance_multi (&rom, 256 * MBIT) :
              (argcmp (argc, argv, "-multi1")) ? gbadvance_multi (&rom, 64 * MBIT) :
              (argcmp (argc, argv, "-multi2")) ? gbadvance_multi (&rom, 128 * MBIT) :
#ifdef  BACKUP
              (argcmp (argc, argv, "-xfal")) ? gbadvance_xfal (&rom) :
              (argncmp (argc, argv, "-xfalc", 6)) ? gbadvance_xfal (&rom) :
              (argncmp (argc, argv, "-xfalb", 6)) ? gbadvance_xfalb (&rom) :
              (argcmp (argc, argv, "-xfals")) ? gbadvance_xfals (&rom) :
#endif
              0);
      break;

    case ucon64_GENESIS:
      return ((argcmp (argc, argv, "-1991")) ? genesis_1991 (&rom) :
              (argcmp (argc, argv, "-chk")) ? genesis_chk (&rom) :
              (argcmp (argc, argv, "-ggd")) ? genesis_ggd (&rom) :
              (argcmp (argc, argv, "-gge")) ? genesis_gge (&rom) :
              (argcmp (argc, argv, "-j")) ? genesis_j (&rom) :
              (argcmp (argc, argv, "-mgd")) ? genesis_mgd (&rom) :
              (argcmp (argc, argv, "-n")) ? genesis_n (&rom) :
              (argcmp (argc, argv, "-n2")) ? genesis_n2 (&rom) :
              (argcmp (argc, argv, "-p")) ? genesis_p (&rom) :
              (argcmp (argc, argv, "-s")) ? genesis_s (&rom) :
              (argcmp (argc, argv, "-smd")) ? genesis_smd (&rom) :
              (argcmp (argc, argv, "-smds")) ? genesis_smds (&rom) :
#ifdef	BACKUP
              (argcmp (argc, argv, "-xsmd")) ? genesis_xsmd (&rom) :
              (argcmp (argc, argv, "-xsmds")) ? genesis_xsmds (&rom) :
#endif
#ifdef	CD
              (argcmp (argc, argv, "-mktoc")) ? genesis_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? genesis_xcdrw (&rom) :
#endif
              0);
      break;

    case ucon64_LYNX:
      return ((argcmp (argc, argv, "-b0")) ? lynx_b0 (&rom) :
              (argcmp (argc, argv, "-b1")) ? lynx_b1 (&rom) :
              (argcmp (argc, argv, "-lnx")) ? lynx_lnx (&rom) :
              (argcmp (argc, argv, "-lyx")) ? lynx_lyx (&rom) :
              (argcmp (argc, argv, "-n")) ? lynx_n (&rom) :
              (argcmp (argc, argv, "-nrot")) ? lynx_nrot (&rom) :
              (argcmp (argc, argv, "-rotl")) ? lynx_rotl (&rom) :
              (argcmp (argc, argv, "-rotr")) ? lynx_rotr (&rom) : 0);
      break;

    case ucon64_N64:
      return ((argcmp (argc, argv, "-bot")) ? nintendo64_bot (&rom) :
              (argcmp (argc, argv, "-chk")) ? nintendo64_chk (&rom) :
              (argcmp (argc, argv, "-f")) ? nintendo64_f (&rom) :
              (argcmp (argc, argv, "-n")) ? nintendo64_n (&rom) :
              (argcmp (argc, argv, "-p")) ? nintendo64_p (&rom) :
              (argcmp (argc, argv, "-sram")) ? nintendo64_sram (&rom) :
              (argcmp (argc, argv, "-swap")) ? nintendo64_swap (&rom) :
              (argcmp (argc, argv, "-usms")) ? nintendo64_usms (&rom) :
              (argcmp (argc, argv, "-v64")) ? nintendo64_v64 (&rom) :
              (argcmp (argc, argv, "-z64")) ? nintendo64_z64 (&rom) :
#ifdef	BACKUP
              (argcmp (argc, argv, "-xdjr")) ? nintendo64_xdjr (&rom) :
              (argcmp (argc, argv, "-xv64")) ? nintendo64_xv64 (&rom) :
#endif
              0);
      break;

    case ucon64_NEOGEO:
      return ((argcmp (argc, argv, "-bios")) ? neogeo_bios (&rom) :
              (argcmp (argc, argv, "-mgd")) ? neogeo_mgd (&rom) :
              (argcmp (argc, argv, "-mvs")) ? neogeo_mvs (&rom) :
              (argcmp (argc, argv, "-s")) ? neogeo_s (&rom) :
              (argcmp (argc, argv, "-sam")) ? neogeo_sam (&rom) :
#ifdef	CD
              (argcmp (argc, argv, "-mktoc")) ? neogeo_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? neogeo_xcdrw (&rom) :
#endif
              0);
      break;

    case ucon64_NES:
      return ((argcmp (argc, argv, "-fds")) ? nes_fds (&rom) :
              (argcmp (argc, argv, "-fdsl")) ? nes_fdsl (&rom) :
              (argcmp (argc, argv, "-ffe")) ? nes_ffe (&rom) :
              (argcmp (argc, argv, "-gg")) ? nes_gg (&rom) :
              (argcmp (argc, argv, "-ggd")) ? nes_ggd (&rom) :
              (argcmp (argc, argv, "-gge")) ? nes_gge (&rom) :
              (argcmp (argc, argv, "-ines")) ? nes_ines (&rom) :
              (argcmp (argc, argv, "-ineshd")) ? nes_ineshd (&rom) :
              (argcmp (argc, argv, "-j")) ? nes_j (&rom) :
              (argcmp (argc, argv, "-pas")) ? nes_pas (&rom) :
              (argcmp (argc, argv, "-unif")) ? nes_unif (&rom) :
              (argcmp (argc, argv, "-s")) ? nes_s (&rom) : 0);
      break;

    case ucon64_SNES:
      return ((argcmp (argc, argv, "-chk")) ? snes_chk (&rom) :
              (argcmp (argc, argv, "-col")) ? snes_col (&rom) :
              (argcmp (argc, argv, "-dint")) ? snes_dint (&rom) :
              (argcmp (argc, argv, "-f")) ? snes_f (&rom) :
              (argcmp (argc, argv, "-fig")) ? snes_fig (&rom) :
              (argcmp (argc, argv, "-figs")) ? snes_figs (&rom) :
              (argcmp (argc, argv, "-gd3")) ? snes_gd3 (&rom) :
              (argcmp (argc, argv, "-gdf")) ? snes_gdf (&rom) :
              (argcmp (argc, argv, "-gg")) ? snes_gg (&rom) :
              (argcmp (argc, argv, "-ggd")) ? snes_ggd (&rom) :
              (argcmp (argc, argv, "-gge")) ? snes_gge (&rom) :
              (argcmp (argc, argv, "-j")) ? snes_j (&rom) :
              (argcmp (argc, argv, "-k")) ? snes_k (&rom) :
              (argcmp (argc, argv, "-l")) ? snes_l (&rom) :
              (argcmp (argc, argv, "-mgd")) ? snes_mgd (&rom) :
//    (argcmp(argc,argv,"-mgh")) ? snes_mgh(&rom) :
              (argcmp (argc, argv, "-n")) ? snes_n (&rom) :
              (argcmp (argc, argv, "-p")) ? snes_p (&rom) :
              (argcmp (argc, argv, "-s")) ? snes_s (&rom) :
              (argcmp (argc, argv, "-smc")) ? snes_smc (&rom) :
              (argcmp (argc, argv, "-swc")) ? snes_swc (&rom) :
              (argcmp (argc, argv, "-swcs")) ? snes_swcs (&rom) :
              (argcmp (argc, argv, "-ufos")) ? snes_ufos (&rom) :
#ifdef	BACKUP
              (argcmp (argc, argv, "-xswc")) ? snes_xswc (&rom) :
              (argcmp (argc, argv, "-xswcs")) ? snes_xswcs (&rom) :
#endif
              0);
      break;

    case ucon64_PCE:
      return ((argcmp (argc, argv, "-mgd")) ? pcengine_mgd (&rom) :
              (argcmp (argc, argv, "-smg")) ? pcengine_smg (&rom) :
#ifdef	CD
              (argcmp (argc, argv, "-mktoc")) ? pcengine_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? pcengine_xcdrw (&rom) :
#endif
              0);
      break;

    case ucon64_JAGUAR:
      return (
#ifdef	CD
               (argcmp (argc, argv, "-mktoc")) ? jaguar_mktoc (&rom) :
               (argcmp (argc, argv, "-xcdrw")) ? jaguar_xcdrw (&rom) :
#endif
               0);
      break;

    case ucon64_SYSTEM16:
    case ucon64_ATARI:
    case ucon64_SMS:
    case ucon64_NEOGEOPOCKET:
    case ucon64_VECTREX:
    case ucon64_VIRTUALBOY:
    case ucon64_WONDERSWAN:
    case ucon64_COLECO:
    case ucon64_INTELLI:
      break;

#ifdef CD
    case ucon64_DC:
      return ((argcmp (argc, argv, "-ip")) ?
              /* ip0000(char *dev,char *name) */ 0 :
              (argcmp (argc, argv, "-iso")) ? /* cdi2iso(rom.rom) */ :
/*  backup */
              (argcmp (argc, argv, "-mktoc")) ? dc_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? dc_xcdrw (&rom) : 0);
      break;

    case ucon64_PSX:
      return ((argcmp (argc, argv, "-iso")) ? raw2iso (rom.rom) :
/*  backup */
              (argcmp (argc, argv, "-mktoc")) ? psx_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? psx_xcdrw (&rom) : 0);
      break;

    case ucon64_PS2:
      return ((argcmp (argc, argv, "-iso")) ? raw2iso (rom.rom) :
/*  backup */
              (argcmp (argc, argv, "-mktoc")) ? ps2_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? ps2_xcdrw (&rom) : 0);
      break;

    case ucon64_SATURN:
      return ((argcmp (argc, argv, "-iso")) ? raw2iso (rom.rom) :
/*  backup */
              (argcmp (argc, argv, "-mktoc")) ? saturn_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? saturn_xcdrw (&rom) : 0);
      break;

    case ucon64_CDI:
      return ((argcmp (argc, argv, "-iso")) ? raw2iso (rom.rom) :
/*  backup */
              (argcmp (argc, argv, "-mktoc")) ? cdi_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? cdi_xcdrw (&rom) : 0);
      break;

    case ucon64_CD32:
      return ((argcmp (argc, argv, "-iso")) ? raw2iso (rom.rom) :
/*  backup */
              (argcmp (argc, argv, "-mktoc")) ? cd32_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? cd32_xcdrw (&rom) : 0);
      break;

    case ucon64_REAL3DO:
      return ((argcmp (argc, argv, "-iso")) ? raw2iso (rom.rom) :
/*  backup */
              (argcmp (argc, argv, "-mktoc")) ? real3do_mktoc (&rom) :
              (argcmp (argc, argv, "-xcdrw")) ? real3do_xcdrw (&rom) : 0);
      break;
#endif /* CD */


    case ucon64_UNKNOWN:
    default:
      if (!access (rom.rom, F_OK) || argcmp (argc, argv, "-xsmd") ||    //the SMD made backups for Genesis and Sega Master System
          argcmp (argc, argv, "-xsmds")
#ifdef CD
          || argcmp (argc, argv, "-mktoc") ||   //take image for which cd-based console?
          argcmp (argc, argv, "-xcdrw")
#endif
        )
        {
//    filehexdump(rom.rom,0,512);//show possible header or maybe the internal rom header
          printf
            ("\nERROR: Unknown ROM: %s not found (in internal database)\n"
             "TIP:   If this is a ROM you might try to force the recognition\n"
             "       The force recognition option for Super Nintendo would be -snes\n"
             "       This is also needed for backup units which support more than one\n"
             "       console system\n"
//      "       if you compiled from the sources you can add it to ucon64_db.c and\n"
//      "       recompile\n"
             , rom.rom);
          return (-1);
        }
      else
        return (ucon64_usage (argc, argv));
      break;
    }


  return (0);
}


int
ucon64_init (struct ucon64_ *rom)
{
  long bytes = 0;
  int other_console = 0;

  bytes = quickftell (rom->rom);

  // call testsplit() before any <CONSOLE>_init() function to provide for a
  // possible override in those functions
  rom->splitted[0] = testsplit (rom->rom);
  if (argcmp (rom->argc, rom->argv, "-ns"))
    rom->splitted[0] = 0;

  if (rom->console != ucon64_UNKNOWN)
    {
      if (bytes <= MAXROMSIZE)
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
// ROMs for the following consoles can be only detected by their CRC32
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
            default:
              other_console = 1;
            }
        }
      // now check if it's one of the CD based consoles
      switch (rom->console)
        {
#ifdef	CD
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
#endif
        default:
          if (other_console)
            rom->console = ucon64_UNKNOWN;
        }
    }

  if (rom->console == ucon64_UNKNOWN && bytes <= MAXROMSIZE)
    if ((snes_init (rom) == -1) &&
        (genesis_init (rom) == -1) &&
        (nintendo64_init (rom) == -1) &&
        (gameboy_init (rom) == -1) &&
        (gbadvance_init (rom) == -1) &&
        (atari_init (rom) == -1) &&
        (lynx_init (rom) == -1) &&
        (nes_init (rom) == -1) &&
        (jaguar_init (rom) == -1) &&
        (pcengine_init (rom) == -1) &&
        (neogeo_init (rom) == -1) &&
        (neogeopocket_init (rom) == -1) &&
        (sms_init (rom) == -1) &&
        (system16_init (rom) == -1) &&
        (virtualboy_init (rom) == -1) &&
        (vectrex_init (rom) == -1) &&
        (coleco_init (rom) == -1) &&
        (intelli_init (rom) == -1) &&
        (wonderswan_init (rom) == -1))
      rom->console = ucon64_UNKNOWN;

  /*
    Calculate the CRC32 after rom->buheader_len has been initialized.
    In this way we can call fileCRC32() with the right value for `start'
    so that we disregard the header (if there is one).
    Under BeOS fileCRC32() is EXTREMELY slow (at least for me, dbjh),
    so we definitely don't want to call it twice.
  */
  if (bytes <= MAXROMSIZE &&
      rom->console != ucon64_PS2 &&
      rom->console != ucon64_DC &&
      rom->console != ucon64_SATURN &&
      rom->console != ucon64_CDI &&
      rom->console != ucon64_CD32 &&
      rom->console != ucon64_PSX &&
      rom->console != ucon64_GAMECUBE &&
      rom->console != ucon64_XBOX)
    rom->current_crc32 = fileCRC32 (rom->rom, rom->buheader_len);

  if (rom->console == ucon64_UNKNOWN)
    return -1;

  quickfread (rom->buheader, rom->buheader_start, rom->buheader_len,
              rom->rom);
//  quickfread(rom->header, rom->header_start, rom->header_len, rom->rom);

  rom->bytes = quickftell (rom->rom);
  rom->mbit = (rom->bytes - rom->buheader_len) / (float) MBIT;

#ifdef	CD
  if (rom->console == ucon64_PS2 ||
      rom->console == ucon64_PSX ||
      rom->console == ucon64_DC ||
      rom->console == ucon64_SATURN ||
      rom->console == ucon64_CDI ||
      rom->console == ucon64_CD32 ||
      rom->console == ucon64_REAL3DO ||
      rom->console == ucon64_XBOX ||
      rom->console == ucon64_GAMECUBE ||
      rom->bytes > MAXROMSIZE)
    {
/*
  this doesn't work really..

    strcat(rom->misc,"\nTrackmode: ");
    strcat(rom->misc,
      (!(rom->bytes%2048)) ? "MODE1, MODE2_FORM1 (2048 Bytes)" :
      (!(rom->bytes%2324)) ? "MODE2_FORM2 (2324 Bytes)" :
      (!(rom->bytes%2336)) ? "MODE2 or MODE2_FORM_MIX (2336 Bytes)" :
      (!(rom->bytes%2352)) ? "AUDIO, MODE1_RAW or MODE2_RAW (2352 Bytes)" :
                             "Unknown"
    );
*/
      return (0);
    }
#endif

  rom->padded = filetestpad (rom->rom);
  rom->intro = ((rom->bytes - rom->buheader_len) > MBIT) ?
    ((rom->bytes - rom->buheader_len) % MBIT) : 0;

  return (0);
}


int
ucon64_usage (int argc, char *argv[])
{
  printf ("USAGE: %s [OPTION(S)] ROM [FILE]\n\n",
          getarg (argc, argv, ucon64_NAME));

  printf (
         /*
           "TODO: $ROM could also be the name of a *.ZIP archive\n"
           "      it will automatically find and extract the ROM\n"
           "\n"
         */
//         "TODO:  -sh      use uCON64 in shell modus\n"
#ifdef	__DOS__
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
#ifdef	__DOS__
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
           "  -strip        strip Bytes from end of ROM; $FILE=VALUE\n");

  bsl_usage (argc, argv);
  ips_usage (argc, argv);
  aps_usage (argc, argv);
#ifdef	CD
  pal4u_usage (argc, argv);
  ppf_usage (argc, argv);
  xps_usage (argc, argv);
#endif

  printf ("\n");

  if (argcmp (argc, argv, "-gba"))
    gbadvance_usage (argc, argv);
  else if (argcmp (argc, argv, "-n64"))
    nintendo64_usage (argc, argv);
  else if (argcmp (argc, argv, "-jag"))
    jaguar_usage (argc, argv);
  else if (argcmp (argc, argv, "-snes"))
    snes_usage (argc, argv);
  else if (argcmp (argc, argv, "-ng"))
    neogeo_usage (argc, argv);
  else if (argcmp (argc, argv, "-ngp"))
    neogeopocket_usage (argc, argv);
  else if (argcmp (argc, argv, "-gen"))
    genesis_usage (argc, argv);
  else if (argcmp (argc, argv, "-gb"))
    gameboy_usage (argc, argv);
  else if (argcmp (argc, argv, "-lynx"))
    lynx_usage (argc, argv);
  else if (argcmp (argc, argv, "-pce"))
    pcengine_usage (argc, argv);
  else if (argcmp (argc, argv, "-sms"))
    sms_usage (argc, argv);
  else if (argcmp (argc, argv, "-nes"))
    nes_usage (argc, argv);
  else if (argcmp (argc, argv, "-s16"))
    sys16_usage (argc, argv);
  else if (argcmp (argc, argv, "-ata"))
    atari_usage (argc, argv);
  else if (argcmp (argc, argv, "-coleco"))
    coleco_usage (argc, argv);
  else if (argcmp (argc, argv, "-vboy"))
    virtualboy_usage (argc, argv);
  else if (argcmp (argc, argv, "-swan"))
    wonderswan_usage (argc, argv);
  else if (argcmp (argc, argv, "-vec"))
    vectrex_usage (argc, argv);
  else if (argcmp (argc, argv, "-intelli"))
    intelli_usage (argc, argv);
#ifdef	CD
  else if (argcmp (argc, argv, "-dc"))
    dc_usage (argc, argv);
  else if (argcmp (argc, argv, "-psx"))
    psx_usage (argc, argv);
  else if (argcmp (argc, argv, "-ps2"))
    ps2_usage (argc, argv);
  else if (argcmp (argc, argv, "-sat"))
    saturn_usage (argc, argv);
  else if (argcmp (argc, argv, "-3do"))
    real3do_usage (argc, argv);
  else if (argcmp (argc, argv, "-cd32"))
    cd32_usage (argc, argv);
  else if (argcmp (argc, argv, "-cdi"))
    cdi_usage (argc, argv);
//  else if(argcmp(argc,argv,"-gc"))gamecube_usage(argc,argv);
  else if (argcmp (argc, argv, "-xbox"))
    xbox_usage (argc, argv);
#endif
  else
    {
#ifdef CD
//  gamecube_usage(argc,argv);
      dc_usage (argc, argv);
      psx_usage (argc, argv);
/*
  ps2_usage(argc,argv);
  sat_usage(argc,argv);
  3do_usage(argc,argv);
  cd32_usage(argc,argv);
  cdi_usage(argc,argv);
*/
      printf ("%s\n%s\n%s\n%s\n%s\n"
//            "  -xbox, -ps2, -sat, -3do, -cd32, -cdi\n"
              "  -ps2, -sat, -3do, -cd32, -cdi\n"
              "                force recognition; NEEDED\n"
//            "  -iso          force image is ISO9660\n"
//            "  -raw          force image is MODE2_RAW/BIN\n"
              "  *             show info (default); ONLY $ROM=RAW_IMAGE\n"
              "  -iso          convert RAW/BIN to ISO9660; $ROM=RAW_IMAGE\n",
//            xbox_TITLE,
              ps2_TITLE, saturn_TITLE, real3do_TITLE, cd32_TITLE,
              cdi_TITLE);

      ppf_usage (argc, argv);
      xps_usage (argc, argv);

      cdrw_usage (argc, argv);

      printf ("\n");
#endif

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
      sys16_usage(argc,argv);
      atari_usage(argc,argv);
      coleco_usage(argc,argv);
      virtualboy_usage(argc,argv);
      wonderswan_usage(argc,argv);
      vectrex_usage(argc,argv);
      intelli_usage(argc,argv);
*/

      printf ("%s\n%s\n%s\n%s\n%s\n%s\n"
              "  -s16, -ata, -coleco, -vboy, -vec, -intelli\n"
              "                force recognition"
#ifndef DB
              "; NEEDED"
#endif
              "\n"
              "  -hd           force ROM has header (+512 Bytes)\n"
              "  -nhd          force ROM has no header\n"
              "  *             show info (default)\n\n", system16_TITLE,
              atari_TITLE, coleco_TITLE, virtualboy_TITLE,
              vectrex_TITLE, intelli_TITLE);

    }

  printf ("Database: %ld known ROMs in ucon64_db.c (%+ld)\n\n",
          ucon64_dbsize (ucon64_UNKNOWN),
          ucon64_dbsize (ucon64_UNKNOWN) - ucon64_DBSIZE);

  printf
    ("TIP: %s -help -snes (would show only Super Nintendo related help)\n"
#ifdef	__DOS__
     "     %s -help|more (to see everything in more)\n"
#else
     "     %s -help|less (to see everything in less)\n" // less is better ;-)
#endif
     "     give the force recognition option a try if something went wrong\n"
     "\n"
     "Report problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net\n"
     "\n", getarg (argc, argv, ucon64_NAME), getarg (argc, argv, ucon64_NAME));

  return (0);
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

// noisyb: in case you don't wan't the "Backup Unit Header: No", don't print
// "Backup Unit Header:" when there is a header, because that makes people like me (dbjh)
// search for "Backup Unit Header: No" when there is no header
      if (!rom->buheader_len)
        printf ("Backup Unit Header: No\n");
      else if (rom->buheader_len)
        printf ("Backup Unit Header: Yes, %ld Bytes\n"
                /* (use -nhd to override)\n" */ ,
                rom->buheader_len);

//    if (!rom->splitted[0])
//      printf("Splitted: No\n");
//    else
      if (rom->splitted[0])
        printf ("Splitted: Yes, %d parts (recommended: use -j to join)\n",
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
  if (rom->splitted[0])
    printf ("NOTE: to get the correct checksum the ROM must be joined\n");

  if (rom->current_crc32 != 0)
    printf ("Checksum (CRC32): 0x%08lx\n", rom->current_crc32);

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

  return (0);
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
