/*
ucon64_misc.c - miscellaneous functions for uCON64

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh
                  2001 Caz


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
#include <unistd.h>             // ioperm() (libc5)
#include <dirent.h>
#include <sys/stat.h>
#include "config.h"
#ifdef  BACKUP
#ifdef  __FreeBSD__
#include <machine/sysarch.h>
#elif   defined __linux__ // __FreeBSD__
#ifdef  __GLIBC__
#include <sys/io.h>             // ioperm() (glibc)
#endif // __GLIBC__
#elif   defined __MSDOS__ // __linux__
#include <pc.h>                 // inportb(), inportw()
#elif   defined __BEOS__ // __MSDOS__
#include <fcntl.h>
#endif // __BEOS__
#endif // BACKUP
#include "ucon64.h"
#include "misc.h"
#include "ucon64_misc.h"

#include "snes/snes.h"
#include "gb/gb.h"
#include "gba/gba.h"
#include "n64/n64.h"
#include "lynx/lynx.h"
#include "sms/sms.h"
#include "nes/nes.h"
#include "genesis/genesis.h"
#include "pce/pce.h"
#include "neogeo/neogeo.h"
#include "ngp/ngp.h"
#include "swan/swan.h"
#include "dc/dc.h"
#include "jaguar/jaguar.h"
#include "sample/sample.h"


const st_track_modes_t track_modes[] = {
  {"MODE1/2048", "MODE1"},
  {"MODE1/2352", "MODE1_RAW"},
  {"MODE2/2336", "MODE2"},
  {"MODE2/2352", "MODE2_RAW"},
  {NULL, NULL}
};


const char *ucon64_parport_error =
  "ERROR:  please check cables and connection\n"
  "        turn off/on the backup unit\n"
  "        split ROMs must be joined first\n"
  "        use " OPTION_LONG_S "file={3bc,378,278,...} to specify your port\n"
  "        set the port to SPP (Standard, Normal) mode in your bios\n"
  "        some backup units do not support EPP and ECP style parports\n"
  "        read the backup units manual\n";


const char *ucon64_console_error =
  "ERROR: could not auto detect the right ROM/IMAGE/console type\n"
  "TIP:   If this is a ROM or CD IMAGE you might try to force the recognition\n"
  "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n";

static char *ucon64_temp_file = NULL;

void
ucon64_wrote (const char *filename)
{
  printf ("Wrote output to: %s\n", filename);
}


#if     defined BACKUP && defined __BEOS__
static int ucon64_io_fd;
#endif

#define DETECT_MAX_CNT 1000



const char *unknown_usage[] =
  {
    "Unknown backup unit/Emulator",
    NULL
  };


const char *
ucon64_fbackup (char *move_name, const char *filename)
{
  if (!ucon64.backup)
    return filename;

  if (!access (filename, F_OK))
    {
      printf ("Writing backup of: %s\n", filename);//verbose
      fflush (stdout);
    }

  return filebackup (move_name, filename);
}


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
  ucon64_temp_file = 0;
  if (!access (dest, F_OK))
    {
      if (!strcmp (dest, ucon64.rom))
        {                                       // case 1
          if (ucon64.backup)
            {                                   // case 1a
              ucon64_fbackup (NULL, dest);
              setext (src, ".BAK");
            }                                   // must match with what filebackup() does
          else
            {                                   // case 1b
              ucon64.backup = 1;                // force ucon64_fbackup() to _rename_ file
              ucon64_fbackup (src, dest);       // arg 1 != NULL -> rename
              ucon64.backup = 0;
              ucon64_temp_file = src;
            }
        }
      else
        ucon64_fbackup (NULL, dest);            // case 2 (ucon64_fbackup() handles a & b)
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


size_t
filepad (const char *filename, long start, long unit)
/*
  pad file (if necessary) from start size_t size;
  ignore start bytes (if file has header or something)
  unit is size of block (example: MBIT)
*/
{
  size_t size = quickftell (filename) - start;

  if ((size % unit) != 0)
    {
      size /= unit;
      size += 1;
      size *= unit;

      truncate (filename, size + start);
    }
  return size;
}


long
filetestpad (const char *filename, st_rominfo_t *rominfo)
// test if EOF is padded (repeating bytes)
{
  long size, x;
  int y;
  char *buf;

  size = rominfo->file_size;

  if (!(buf = (char *) malloc ((size + 2) * sizeof (char))))
    return -1;

  quickfread (buf, 0, size, filename);

  y = buf[size - 1] & 0xff;
  x = size - 2;
  while (y == (buf[x] & 0xff))
    x--;

  free (buf);

  return (size - x - 1 == 1) ? 0 : size - x - 1;
}


#ifdef  BACKUP
#ifdef  __BEOS__
void
close_io_port (void)
{
  close (ucon64_io_fd);
}
#endif


#if     defined __unix__ || defined __BEOS__ // DJGPP (DOS) has outportX() & inportX()
unsigned char
inportb (unsigned short port)
{
#ifdef  __BEOS__
  st_ioport_t temp;

  temp.port = port;
  ioctl (ucon64_io_fd, 'r', &temp, 0);

  return temp.data8;
#else
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
#ifdef  __BEOS__
  st_ioport_t temp;

  temp.port = port;
  ioctl (ucon64_io_fd, 'r16', &temp, 0);

  return temp.data16;
#else
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
#ifdef  __BEOS__
  st_ioport_t temp;

  temp.port = port;
  temp.data8 = byte;
  ioctl (ucon64_io_fd, 'w', &temp, 0);
#else
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
#ifdef  __BEOS__
  st_ioport_t temp;

  temp.port = port;
  temp.data16 = word;
  ioctl (ucon64_io_fd, 'w16', &temp, 0);
#else
  __asm__ __volatile__
  ("outw %1, %0"
    :
    : "d" (port), "a" (word)
  );
#endif
}
#endif // defined __unix__ || defined __BEOS__


int
detect_parport (unsigned int port)
{
  int i;

#if     defined  __linux__
  if (ioperm (port, 1, 1) == -1)
    return -1;
#elif   defined __FreeBSD__
  if (i386_set_ioperm (port, 1, 1) == -1)
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

#if     defined  __linux__
  if (ioperm (port, 1, 0) == -1)
    return -1;
#elif   defined __FreeBSD__
  if (i386_set_ioperm (port, 1, 0) == -1)
    return -1;
#endif

  if (i >= DETECT_MAX_CNT)
    return 0;

  return 1;
}


unsigned int
parport_probe (unsigned int port)
// detect parallel port
{
  unsigned int parport_addresses[] = { 0x3bc, 0x378, 0x278 };
  int i;

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
#endif // __BEOS__

  if (port <= 3)
    {
      for (i = 0; i < 3; i++)
        {
          if (detect_parport (parport_addresses[i]) == 1)
            {
              port = parport_addresses[i];
              break;
            }
        }
      if (i >= 3)
        return 0;
    }
  else
    if ((port != parport_addresses[0]) &&
        (port != parport_addresses[1]) &&
        (port != parport_addresses[2]))
    return 0;

  if (port != 0)
    {
#if     defined  __linux__ || defined __FreeBSD__
#ifdef  __linux__
      if (ioperm (port, 3, 1) == -1)            // data, status & control
#else
      if (i386_set_ioperm (port, 3, 1) == -1)   // data, status & control
#endif // __linux__
        {
          fprintf (stderr,
                   "ERROR: Could not set port permissions for I/O ports 0x%x, 0x%x and 0x%x\n"
                   "       (This program needs root privileges for the requested action)\n",
                   port + PARPORT_DATA, port + PARPORT_STATUS, port + PARPORT_CONTROL);
          exit (1);                             // Don't return, if ioperm() fails port access
        }                                       //  causes core dump
#endif
      outportb (port + PARPORT_CONTROL,
                inportb (port + PARPORT_CONTROL) & 0x0f);
    }                                           // bit 4 = 0 -> IRQ disable for ACK, bit 5-7 unused

  return port;
}


unsigned int
ucon64_parport_probe (unsigned int port)
{
  if (!(port = parport_probe (port)))
    ;
/*
    fprintf (stderr, "ERROR: no parallel port 0x%s found\n\n", strupr (buf));
  else
    printf ("0x%x\n\n", port);
*/

#ifdef  __unix__
  /*
    Some code needs us to switch to the real uid and gid. However, other code
    needs access to I/O ports other than the standard printer port registers.
    We just do an iopl(3) and all code should be happy. Using iopl(3) enables
    users to run all code without being root (of course with the uCON64
    executable setuid root). Anyone a better idea?
  */
#if     defined  __linux__ || defined __FreeBSD__
#ifdef  __linux__
  if (iopl (3) == -1)
#else
  if (i386_iopl (3) == -1)
#endif
    {
      fprintf (stderr, "ERROR: Could not set the I/O privilege level to 3\n"
                       "       (This program needs root privileges for the requested action)\n");
      return 1;
    }
#endif // __linux__ || __FreeBSD__
#endif // __unix__
  return port;
}
#endif // BACKUP


int
ucon64_gauge (time_t init_time, long pos, long size)
{
  if (!ucon64.frontend)
    return gauge (init_time, pos, size);
  else
    {
      int percentage = 100 * pos/size;

      printf ("%u\n", percentage);
      fflush (stdout);
      return 0;
    }
}


int
ucon64_testsplit (const char *filename)
// test if ROM is split into parts
{
  signed int x;
  int parts = 0;
  char buf[FILENAME_MAX];
  int pos = 0;

  if (!strchr (filename, '.')) return 0;

  for (x = -1; x < 2; x += 2)
    {
      parts = 0;
      strcpy (buf, filename);
      pos = strrcspn (buf, ".") + x; /* x == -1 change char before '.'
                                        else x == 1 change char behind '.' */

      while (!access (buf, F_OK))buf[pos]--;  /* "rewind" */
      buf[pos]++;

      while (!access (buf, F_OK)) /* count split parts */
        {
          buf[pos]++;
          parts++;
        }

      if (parts > 1)
        return parts;
    }

  return 0;
}


int
ucon64_bin2iso (const char *image, int track_mode)
{
#ifdef TODO
#warning TODO nrg2iso and cdi2iso
#endif
  int seek_header, seek_ecc, sector_size;
  long i, size;
  char buf[MAXBUFSIZE];
  FILE *dest, *src;
  time_t starttime = time (NULL);


  switch (track_mode)
    {
      case MODE1_2048:
        fprintf (stderr, "ERROR: the images track mode is already MODE1/2048\n");
        return -1;

      case MODE1_2352:
        seek_header = 16;
        seek_ecc = 288;
        sector_size = 2352;
        break;

      case MODE2_2336:
#ifdef __MAC__ // macintosh
        seek_header = 0;
#else
        seek_header = 8;
#endif      
        seek_ecc = 280;
        sector_size = 2336;
        break;        

      case MODE2_2352:
#ifdef __MAC__ // macintosh
        seek_header = 16;
#else
        seek_header = 24;
#endif
        seek_ecc = 280;
        sector_size = 2352;
        break;

      default:
        fprintf (stderr, "ERROR: unknown/unsupported track mode");
        return -1;
    }

  strcpy (buf, FILENAME_ONLY(image));
  setext (buf, ".ISO");
  size = quickftell (image) / sector_size;

  if (!(src = fopen (image, "rb"))) return -1;
  if (!(dest = fopen (buf, "wb")))
    {
      fclose (src);
      return -1;
    }

  for (i = 0; i < size; i++)
    {
      fseek (src, seek_header, SEEK_CUR);
#ifdef __MAC__
      if (track_mode == MODE2_2336 || track_mode == MODE2_2352)
        {
          fread (buf, sizeof (char), 2056, src);
          fwrite (buf, sizeof (char), 2056, dest);
        }
      else
#endif
        {
          fread (buf, sizeof (char), 2048, src);
          fwrite (buf, sizeof (char), 2048, dest);
        }
      fseek (src, seek_ecc, SEEK_CUR);

      ucon64_gauge (starttime, i * sector_size, size * sector_size);
    }

  fclose (dest);
  fclose (src);

  return 0;
}


// seek_pvd() will search for valid PVD in sector 16 of source image

int
seek_pvd (int sector_size, int mode, const char *filename)
{
  const char PVD_STRING[8] = { 0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\x01" "CD001" "\x01" "\0";
#if 0
  const char SVD_STRING[8] = { 0x02, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\x02" "CD001" "\x01" "\0";
  const char VDT_STRING[8] = { 0xff, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\xFF" "CD001" "\x01" "\0";
#endif
  char buf[MAXBUFSIZE];
#if 0
  fseek (fsource, 0L, SEEK_SET);
  fseek (fsource, 16 * sector_size, SEEK_CUR);  // boot area

  if (sector_size == 2352)
    fseek (fsource, 16, SEEK_CUR);      // header
  if (mode == 2)
    fseek (fsource, 8, SEEK_CUR);       // subheader

  fread (buffer, 1, 8, fsource);
#endif
  quickfread (buf, 16 * sector_size
    + ((sector_size == 2352) ? 16 : 0) // header
    + ((mode == 2) ? 8 : 0)            // subheader
    ,8 , filename);

  if (!memcmp (PVD_STRING, buf, 8)
#if 0
      || !memcmp (SVD_STRING, buf, 8)
      || !memcmp (VDT_STRING, buf, 8)
#endif
    )
    return 1;

  return 0;
}


const char *
ucon64_rom_in_archive (DIR **dp, const char *archive, char *romname,
                       const char *configfile)
{
#if 0
  struct dirent *ep;
  struct stat fstate;
  char buf[FILENAME_MAX], cwd[FILENAME_MAX];

#ifdef UNZIP
  if (!stricmp (GETEXT (archive), ".zip"))
    {
//use libz

    }
#endif // UNZIP

#ifndef __MSDOS__ 
  getcwd (cwd, FILENAME_MAX);
  sprintf (buf, "%s" FILE_SEPARATOR_S "%s", cwd, archive);
//  strcpy (buf, archive);

  if (!(*dp = opendir2 (buf, configfile, NULL, NULL)))
    return archive;

  chdir (buf);

  while ((ep = readdir (*dp)))  //find rom in dp and return it as romname
    if (!stat (ep->d_name, &fstate))
      if (S_ISREG (fstate.st_mode))
        {
          sprintf (romname, "%s" FILE_SEPARATOR_S "%s", buf, ep->d_name);
          chdir (cwd);

          return romname;
        }

  chdir (cwd);
#endif
#endif
  return archive;
}


int
ucon64_trackmode_probe (const char *image)
// tries to figure out the used track mode of the cd image
{
#ifdef TODO
#warning TODO support image == /dev/<cdrom>
#endif
  int result = -1; 
#if 0
  const char PVD_STRING[8] = { 0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\x01" "CD001" "\x01" "\0";
  const char SVD_STRING[8] = { 0x02, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\x02" "CD001" "\x01" "\0";
  const char VDT_STRING[8] = { 0xff, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\xFF" "CD001" "\x01" "\0";
  const char SYNC_DATA[12] =
    { 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0 };
  const char SUB_HEADER[8] = { 0, 0, 0x08, 0, 0, 0, 0x08, 0 };
#endif  
  const char SYNC_HEADER[12] =
    { 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0 };
  char buf[MAXBUFSIZE];

  quickfread (buf, 0, 16, image);

#ifdef DEBUG
  memhexdump(buf, 16, 0);
#endif

  if (!memcmp (SYNC_HEADER, buf, 12))
    {
      switch (buf[15])
        {
          case 2:
            if (seek_pvd (2352, 2, image)) result = MODE2_2352;
            break;

          case 1:
            if (seek_pvd (2352, 1, image)) result = MODE1_2352;
            break;

          default:
            result = -1;
            break;
        }
      return result;
    }

  result = (seek_pvd (2048, 1, image)) ? MODE1_2048 :
           (seek_pvd (2352, 1, image)) ? MODE1_2352 :
           (seek_pvd (2336, 2, image)) ? MODE2_2336 :
           (seek_pvd (2352, 2, image)) ? MODE2_2352 :
#if 0
//mac
           (seek_pvd (2056, 2, image)) ? MODE2_2056 :
#endif
  (-1);
  
  return result;
}


int
ucon64_mktoc (st_rominfo_t *rominfo)
{
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE];
  FILE *fh;
  int result, fsize;

  result = ucon64_trackmode_probe (ucon64.rom);
  fsize = quickftell (ucon64.rom);

  sprintf (buf, "%s\n" "\n" "\n" "// Track 1\n"
           "TRACK %s\n"
           "NO COPY\n"
           "DATAFILE \"%s\" %d// length in bytes: %d\n",
           (!strnicmp (ucon64.file, "MODE1", 5) ? "CD_ROM" : "CD_ROM_XA"),
           (!strnicmp (ucon64.file, "MODE", 4) ? ucon64.file :
           (result != -1) ? track_modes[result].cdrdao : "MODE2_RAW"),
           ucon64.rom, fsize, fsize);
  printf ("%s\n", buf);

  strcpy (buf2, ucon64.rom);
  setext (buf2, ".TOC");

  if (!access (buf2, F_OK))
    ucon64_fbackup (NULL, buf2);

  if (!(fh = fopen (buf2, "wb")))
    return -1;
  result = fputs (buf, fh);
  fclose (fh);

  return result;
}


int
ucon64_mkcue (st_rominfo_t *rominfo)
{
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE];
  FILE *fh;
  int result;
  result = ucon64_trackmode_probe (ucon64.rom);

  sprintf (buf, "FILE \"%s\" BINARY\n"
           "  TRACK 01 %s\n"
           "    INDEX 01 00:00:00\n",
           ucon64.rom,
           (!stricmp (ucon64.file, "MODE1") || !stricmp (ucon64.file, "MODE2_FORM1")) ? "MODE1/2048" :
           (!stricmp (ucon64.file, "MODE1_RAW")) ? "MODE1/2352" :
           (!stricmp (ucon64.file, "MODE2") || !stricmp (ucon64.file, "MODE2_FORM_MIX")) ? "MODE2/2336" :
           (result != -1) ? track_modes[result].common : "MODE2/2352");

  printf ("%s\n", buf);

  strcpy (buf2, ucon64.rom);
  setext (buf2, ".CUE");

  if (!access (buf2, F_OK))
    ucon64_fbackup (NULL, buf2);

  if (!(fh = fopen (buf2, "wb")))
    return -1;
  result = fputs (buf, fh);
  fclose (fh);

  return result;
}


int ucon64_e (const char *romfile)
{
  int result, x;
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[MAXBUFSIZE];
  const char *property;

  x = 0;
  while (long_options[x].name)
    {
      if (long_options[x].val == ucon64.console)
      {
        sprintf (buf3, "emulate_%s", long_options[x].name);
        break;
      }
      x++;
    }

  if (access (ucon64.configfile, F_OK) != 0)
    {
      fprintf (stderr, "ERROR: %s does not exist\n", ucon64.configfile);
      return -1;
    }


  property = getProperty (ucon64.configfile, buf3, buf2, NULL);   // buf2 also contains property value
  if (property == NULL)
    {
      fprintf (stderr, "ERROR: could not find the correct settings (%s) in\n"
              "       %s\n"
              "TIP:   If the wrong console was detected you might try to force recognition\n"
              "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n",
              buf3, ucon64.configfile);
      return -1;
    }

  sprintf (buf, "%s %s", buf2, ucon64.rom);

  printf ("%s\n", buf);
  fflush (stdout);
  sync ();

  result = fsystem (stderr, buf);

#if 1
  // Snes9x (Linux) for example returns a non-zero value on a normal exit
  //  (3)...
  // under WinDOS, system() immediately returns with exit code 0 when
  //  starting a Windows executable (as if fork() was called) it also
  //  returns 0 when the exe could not be started
  if (result != 127 && result != -1 && result != 0)        // 127 && -1 are system() errors, rest are exit codes
    {
      fprintf (stderr, "ERROR: the Emulator returned an error code (%d)\n"
              "TIP:   If the wrong emulator was used you might try to force recognition\n"
              "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n",
              (int) result);
    }
#endif
  return result;
}


int
ucon64_ls_main (const char *filename, struct stat *fstate, int mode, int console)
{
  int result;
  char buf[MAXBUFSIZE];
  st_rominfo_t rominfo;

  ucon64.console = console;
  ucon64.rom = filename;
  ucon64.type = (quickftell (ucon64.rom) <= MAXROMSIZE) ? UCON64_ROM : UCON64_CD;
  ucon64_flush (&rominfo);

  result = ucon64_init (ucon64.rom, &rominfo);

  switch (mode)
    {
    case UCON64_LSV:
      if (!result)
        ucon64_nfo (&rominfo);
        break;

    case UCON64_RROM:
    case UCON64_RR83:
      if (ucon64.console != UCON64_UNKNOWN && !ucon64_testsplit (filename))
        {
          strcpy (buf, mkfile (strtrim (rominfo.name), '_'));
          if (!buf[0])
            strcpy (buf, mkfile (UCON64_UNKNOWN_S, '_'));
          if (mode == UCON64_RR83)
            buf[8] = 0;
          strcat (buf, GETEXT (ucon64.rom));
          if (mode == UCON64_RR83)
            buf[12] = 0;
          if (!strcmp (ucon64.rom, buf))
            break;
          printf ("Renaming %s to %s\n", ucon64.rom, buf);
          remove (buf);
          rename (ucon64.rom, buf);
        }
      break;

    case UCON64_LS:
    default:
      strftime (buf, 13, "%b %d %H:%M", localtime (&fstate->st_mtime));
      printf ("%-31.31s %10ld %s %s\n", mkprint(rominfo.name, ' '),
            (long) fstate->st_size, buf, ucon64.rom);
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
  char dir[FILENAME_MAX];
  char old_dir[FILENAME_MAX];
  DIR *dp;
  int console = ucon64.console;

  dir[0]=0;

  if (path)
    if (path[0])
      {
        if (!stat (path, &fstate))
          {
            if (S_ISREG (fstate.st_mode))
              return ucon64_ls_main (path, &fstate, mode, console);
          }
        strcpy (dir, path);
      }

  if (!dir[0])
    getcwd (dir, FILENAME_MAX);

  if ((dp = opendir (dir)) == NULL)
    return -1;

  getcwd (old_dir, FILENAME_MAX); // remember current dir
  chdir (dir);

  while ((ep = readdir (dp)))
    if (!stat (ep->d_name, &fstate))
      if (S_ISREG (fstate.st_mode))
        ucon64_ls_main (ep->d_name, &fstate, mode, console);

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
  char buf[256];
  char buf2[MAXBUFSIZE], *dirname;

  dirname = getenv2 ("HOME");
  sprintf (ucon64.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
  "ucon64.cfg"
#else
  ".ucon64rc"
#endif
  , dirname);
  free (dirname);

  if (access (ucon64.configfile, F_OK) != 0)
    {
      FILE *fh;

      printf ("WARNING: %s not found: creating...", ucon64.configfile);

      if (!(fh = fopen (ucon64.configfile, "wb")))
        {
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
                 "#\n"
                 "# parallel port\n"
                 "#\n"
                 "#parport=378\n"
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
                 , UCON64_VERSION);
          fclose (fh);
          printf ("OK\n\n");
        }
    }
  else if (strtol (getProperty (ucon64.configfile, "version", buf2, "0"), NULL, 10) < UCON64_VERSION)
    {
      strcpy (buf2, ucon64.configfile);
      setext (buf2, ".OLD");

      printf ("NOTE: updating config: old version will be renamed to %s...", buf2);

      filecopy (ucon64.configfile, 0, quickftell (ucon64.configfile), buf2, "wb");

      sprintf (buf, "%d", UCON64_VERSION);
      setProperty (ucon64.configfile, "version", buf);

#if 0
      setProperty (ucon64.configfile, "backups", "1");

#ifdef BACKUP
      setProperty (ucon64.configfile, "parport", "0x378");
#endif // BACKUP

      setProperty (ucon64.configfile, "emulate_gp32", "");
      setProperty (ucon64.configfile, "cdrw_read",
        getProperty (ucon64.configfile, "cdrw_raw_read", buf2, "cdrdao read-cd --read-raw --device 0,0,0 --driver generic-mmc-raw --datafile "));
      setProperty (ucon64.configfile, "cdrw_write",
        getProperty (ucon64.configfile, "cdrw_raw_write", buf2, "cdrdao write --device 0,0,0 --driver generic-mmc "));
#endif

      DELETEPROPERTY (ucon64.configfile, "cdrw_raw_read");
      DELETEPROPERTY (ucon64.configfile, "cdrw_raw_write");
      DELETEPROPERTY (ucon64.configfile, "cdrw_iso_read");
      DELETEPROPERTY (ucon64.configfile, "cdrw_iso_write");

      setProperty (ucon64.configfile, "lha_extract", "lha efi \"%s\"");
      setProperty (ucon64.configfile, "lzh_extract", "lha efi \"%s\"");
      setProperty (ucon64.configfile, "zip_extract", "unzip -xojC \"%s\"");
      setProperty (ucon64.configfile, "rar_extract", "unrar x \"%s\"");
      setProperty (ucon64.configfile, "ace_extract", "unace e \"%s\"");

      sync ();
      printf ("OK\n\n");
    }
  return 0;
}


int
toc2cue (char *filename)
{
#if 0
#include "util.h"
#include "Toc.h"

static const char *PRGNAME = NULL;
static int VERBOSE = 1;

//  message(0, "\nUsage: %s [-v #] { -V | input-toc-file output-cue-file}", PRGNAME);
#endif
#if 0
  char *tocFile, *cueFile;
  Toc *toc;

  PRGNAME = *argv;
  
  Msf start, end;
  const Track *trun;
  int trackNr;
  TrackIterator titr(toc);
  char *binFileName = NULL;
  int err = 0;

  // first make some consistency checks, surely not complete to identify
  // toc-files that can be correctly converted to cue files
  for (trun = titr.first(start, end), trackNr = 1;
       trun != NULL;
       trun = titr.next(start, end), trackNr++) {
    const SubTrack *strun;
    int stcount;
    TrackData::Type sttype1, sttype2;
    SubTrackIterator stitr(trun);

    switch (trun->type()) {
    case TrackData::MODE0:
    case TrackData::MODE2_FORM2:
      message(-2, "Cannot convert: track %d has unsupported mode.", trackNr);
      err = 1;
      break;
    default:
      break;
    }

    for (strun = stitr.first(), stcount = 0;
	 strun != NULL;
	 strun = stitr.next(), stcount++) {

      // store types of first two sub-tracks for later evaluation
      switch (stcount) {
      case 0:
	sttype1 = strun->TrackData::type();
	break;
      case 1:
	sttype2 = strun->TrackData::type();
	break;
      }

      // check if whole toc-file just references a single bin file
      if (strun->TrackData::type() == TrackData::DATAFILE) {
	if (binFileName == NULL) {
	  binFileName = strdupCC(strun->filename());
	}
	else {
	  if (strcmp(binFileName, strun->filename()) != 0) {
	    message(-2, "Cannot convert: toc-file references multiple data files.");
	    err = 1;
	  }
	}
      }
    }

    switch (stcount) {
    case 0:
      message(-2, "Cannot convert: track %d references no data file.",
	      trackNr);
      err = 1;
      break;

    case 1:
      if (sttype1 != TrackData::DATAFILE) {
	message(-2, "Cannot convert: track %d references no data file.",
	      trackNr);
	err = 1;
      }
      break;

    case 2:
      if (sttype1 != TrackData::ZERODATA || sttype2 != TrackData::DATAFILE) {
	message(-2, "Cannot convert: track %d has unsupported layout.",
		trackNr);
	err = 1;
      }
      break;

    default:
      message(-2, "Cannot convert: track %d has unsupported layout.", trackNr);
      err = 1;
      break;
    }
  }

  if (binFileName == NULL) {
    message(-2, "Cannot convert: toc-file references no data file.");
    err = 1;
  }

  if (err) {
    message(-2, "Cannot convert toc-file '%s' to a cue file.", tocFile);
    exit(1);
  }

  ofstream out(cueFile);

  if (!out) {
    message(-2, "Cannot open cue file \'%s\' for writing: %s", cueFile,
	    strerror(errno));
    exit(1);
  }

  out << "FILE \"" << binFileName << "\" BINARY" << endl;

  long offset = 0;

  for (trun = titr.first(start, end), trackNr = 1;
       trun != NULL;
       trun = titr.next(start, end), trackNr++) {
    out << "  TRACK ";
    out.form("%02d ", trackNr);

    switch (trun->type()) {
    case TrackData::AUDIO:
      out << "AUDIO";
      break;
    case TrackData::MODE1:
    case TrackData::MODE2_FORM1:
      out << "MODE1/2048";
      break;
    case TrackData::MODE2:
    case TrackData::MODE2_FORM_MIX:
      out << "MODE2/2336";
      break;
    case TrackData::MODE1_RAW:
      out << "MODE1/2352";
      break;
    case TrackData::MODE2_RAW:
      out << "MODE2/2352";
      break;
    default:
      break;
    }
    
    out << endl;

    const SubTrack *strun;
    SubTrackIterator stitr(trun);
    int pregap = 0;

    for (strun = stitr.first(); strun != NULL; strun = stitr.next()) {
      if (strun->TrackData::type() == TrackData::ZERODATA) {
	out << "    PREGAP " << trun->start().str() << endl;
	pregap = 1;
      }
      else {
	if (!pregap && trun->start().lba() != 0) {
	  out << "    INDEX 00 " << Msf(offset).str() << endl;
	  out << "    INDEX 01 " 
	      << Msf(offset + trun->start().lba()).str() << endl;
	}
	else {
	  out << "    INDEX 01 " << Msf(offset).str() << endl;
	}

	offset += trun->length().lba();

	if (pregap)
	  offset -= trun->start().lba();
      }
    }
  } 


  fprintf (stderr, "NOTE: the resulting cue file is only valid if the\n"
    "      toc-file was created with cdrdao using the commands 'read-toc'\n"
    "      or 'read-cd'. For manually created or edited toc-files the\n"
    "      cue file may not be correct. This program just checks for\n"
    "      the most obvious toc-file features that cannot be converted to\n"
    "      a cue file.\n"
    "      Furthermore, if the toc-file contains audio tracks the byte\n"
    "      order of the image file will be wrong which results in static\n"
    "      noise when the resulting cue file is used for recording\n"
    "      (even with cdrdao itself).");
#endif
  return 0;
}
