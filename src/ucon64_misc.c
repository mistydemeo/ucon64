/*
ucon64_misc.c - miscellaneous functions for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
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


const char *track_modes[] = {
  "MODE1/2048",
  "MODE1/2352",
  "MODE2/2336",
  "MODE2/2352"
};

const char *ucon64_parport_error =
  "ERROR:  please check cables and connection\n"
  "        turn off/on the backup unit\n"
  "        splitted ROMs must be joined first\n"
  "        use " OPTION_LONG_S "file={3bc,378,278,...} to specify your port\n"
  "        set the port to SPP (Standard, Normal) mode in your bios\n";

#define MAXBUFSIZE 32768
#define DETECT_MAX_CNT 1000
#define CRC32_POLYNOMIAL     0xEDB88320L

static unsigned long CRCTable[256];

#if     defined BACKUP && defined __BEOS__
static int ucon64_io_fd;
#endif

static void BuildCRCTable ();
static unsigned long CalculateFileCRC (FILE * file);

#ifdef BACKUP
static unsigned int parport_probe (unsigned int parport);
static int detect_parport (unsigned int port);
#endif

const char *unknown_usage[] =
  {
    "Unknown backup unit/emulator",
    NULL
  };

void
BuildCRCTable ()
{
  int i, j;
  unsigned long crc;

  for (i = 0; i <= 255; i++)
    {
      crc = i;
      for (j = 8; j > 0; j--)
        {
          if (crc & 1)
            crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
          else
            crc >>= 1;
        }
      CRCTable[i] = crc;
    }
}

unsigned long
CalculateBufferCRC (unsigned int count, unsigned long crc, void *buffer)
{
  unsigned char *p;
  unsigned long temp1, temp2;

  p = (unsigned char *) buffer;
  while (count-- != 0)
    {
      temp1 = (crc >> 8) & 0x00FFFFFFL;
      temp2 = CRCTable[((int) crc ^ *p++) & 0xff];
      crc = temp1 ^ temp2;
    }
  return crc;
}

unsigned long
CalculateFileCRC (FILE * file)
{
  unsigned long crc;
  int count, i;
  unsigned char buffer[512];

  crc = 0xFFFFFFFFL;
  i = 0;
  while ((count = fread (buffer, 1, 512, file)))
    crc = CalculateBufferCRC (count, crc, buffer);

  return crc ^= 0xFFFFFFFFL;
}

unsigned long
fileCRC32 (const char *filename, long start)
{
  unsigned long val;
  FILE *fh;

  BuildCRCTable ();

  if (!(fh = fopen (filename, "rb")))
    return -1;
  fseek (fh, start, SEEK_SET);
  val = CalculateFileCRC (fh);
  fclose (fh);

  return val;
}

/*
  like zlib/crc32(); uCON64 has it's own crc calc. stuff
  this is just a wrapper
*/
/*
unsigned long
unif_crc32 (unsigned long dummy, unsigned char *prg_code, size_t size)
{
  unsigned long crc = 0;

  return CalculateBufferCRC ((unsigned int) size, crc, (void *) prg_code);
}
*/

const char *
ucon64_fbackup (const char *filename)
{
  if (!ucon64.backup)
    return filename;

  if (!access (filename, F_OK))
    {
      printf ("Writing backup of: %s\n", filename);
      fflush (stdout);
    }
  return filebackup (filename);
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
filetestpad (const char *filename)
// test if EOF is padded (repeating bytes)
{
  long size, x;
  int y;
  char *buf;

  size = quickftell (filename);

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
#if     defined __unix__ || defined __BEOS__ // DJGPP (DOS) has outportX() & inportX()
unsigned char
inportb (unsigned short port)
{
#ifdef  __BEOS__
  IO_Tuple temp;

  temp.Port = port;
  ioctl (ucon64_io_fd, DRV_READ_IO_8, &temp, 0);

  return temp.Data;
#else
  unsigned char byte;

  __asm__ __volatile__ ("inb %1, %0":"=a" (byte):"d" (port));

  return byte;
#endif
}

unsigned short
inportw (unsigned short port)
{
#ifdef  __BEOS__
  IO_Tuple temp;

  temp.Port = port;
  ioctl (ucon64_io_fd, DRV_READ_IO_16, &temp, 0);

  return temp.Data16;
#else
  unsigned short word;

  __asm__ __volatile__ ("inw %1, %0":"=a" (word):"d" (port));

  return word;
#endif
}

void
outportb (unsigned short port, unsigned char byte)
{
#ifdef  __BEOS__
  IO_Tuple temp;

  temp.Port = port;
  temp.Data = byte;
  ioctl (ucon64_io_fd, DRV_WRITE_IO_8, &temp, 0);
#else
  __asm__ __volatile__ ("outb %1, %0"::"d" (port), "a" (byte));
#endif
}

void
outportw (unsigned short port, unsigned short word)
{
#ifdef  __BEOS__
  IO_Tuple temp;

  temp.Port = port;
  temp.Data16 = word;
  ioctl (ucon64_io_fd, DRV_WRITE_IO_16, &temp, 0);
#else
  __asm__ __volatile__ ("outw %1, %0"::"d" (port), "a" (word));
#endif
}
#endif // defined __unix__ || defined __BEOS__

int
detect_parport (unsigned int port)
{
  int i;

#if     defined  __linux__ || defined __FreeBSD__
  if (
#ifdef  __linux__
      ioperm
#else
      i386_set_ioperm
#endif // __linux__
      (port, 1, 1) == -1)
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

#if     defined  __linux__ || defined __FreeBSD__
  if (
#ifdef  __linux__
      ioperm
#else
      i386_set_ioperm
#endif // __linux__
      (port, 1, 0) == -1)
    return -1;
#endif

  if (i >= DETECT_MAX_CNT)
    return 0;

  return 1;
}

#ifdef  __BEOS__
void
close_io_port (void)
{
  close (ucon64_io_fd);
}
#endif

unsigned int
ucon64_parport_probe (unsigned int port)
{
#ifdef BACKUP
#ifdef __unix__
  uid_t uid;
  gid_t gid;
#endif


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
  return port;
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
          fprintf (stderr,
                   "Could not open I/O port device (no driver)\n"
                   "You can download the latest ioport driver from\n"
                   "http://www.infernal.currantbun.com or http://ucon64.sourceforge.net\n");
          exit (1);
        }
      else
        {                                       // print warning, but continue
          fprintf (stderr,
                   "Support for the driver parnew is deprecated. Future versions of uCON64 might\n"
                   "not support this driver. You can download the latest ioport driver from\n"
                   "http://www.infernal.currantbun.com or http://ucon64.sourceforge.net\n\n");
        }
    }

  if (atexit (close_io_port) == -1)
    {
      close (ucon64_io_fd);
      fprintf (stderr, "Could not register function with atexit()\n");
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
  if (
#ifdef __linux__
      ioperm
#else
      i386_set_ioperm
#endif // __linux__
      (port, 3, 1) == -1)                       // data, status & control
        {
          fprintf (stderr,
                   "Could not set port permissions for I/O ports 0x%x, 0x%x and 0x%x\n"
                   "(This program needs root privileges)\n",
                   port + PARPORT_DATA, port + PARPORT_STATUS, port + PARPORT_CONTROL);
          exit (1);                             // Don't return, if ioperm() fails port access
        }                                       //  causes core dump
#endif
      outportb (port + PARPORT_CONTROL,
                inportb (port + PARPORT_CONTROL) & 0x0f);
    }                                           // bit 4 = 0 -> IRQ disable for ACK, bit 5-7 unused

  return port;
}

int
ucon64_gauge (time_t init_time, long pos, long size)
{
  if (!ucon64.frontend)
    return gauge (init_time, pos, size);
  else
    {
      int percentage;

      percentage = 100 * pos / size;
      printf ("%u\n", percentage);
      fflush (stdout);
      return 0;
    }
}
#endif // BACKUP

int
ucon64_testsplit (const char *filename)
// test if ROM is splitted into parts
{
  long x = 0;
  char buf[4096];

  strcpy (buf, filename);
  buf[findlast (buf, ".") - 1]++;
  while (!access (buf, F_OK) && strdcmp (buf, filename) != 0)
    {
      buf[findlast (buf, ".") - 1]++;
      x++;
    }

  if (x != 0)
    return x + 1;

  strcpy (buf, filename);
  buf[findlast (buf, ".") + 1]++;
  while (!access (buf, F_OK) && strdcmp (buf, filename) != 0)
    {
      buf[findlast (buf, ".") + 1]++;
      x++;
    }

  return (x != 0) ? (x + 1) : 0;
}

int
ucon64_bin2iso (const char *image, int track_mode)
{
  int seek_header, seek_ecc, sector_size;
  long i, size;
  char buf[MAXBUFSIZE];
  FILE *dest, *src;

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

  strcpy (buf, image);
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
    }

  fclose (dest);
  fclose (src);

  return 0;
}


int
ucon64_trackmode_probe (const char *image)
// tries to figure out the used track mode of the cd image
{
//TODO support image == "/dev/<cdrom>"
  int result = -1;
  const char SYNC_HEADER[12] =
    { 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0 };
  char buf[MAXBUFSIZE];

  quickfread (buf, 0, 16, image);

  if (memcmp (SYNC_HEADER, buf, 12))
{
    result = MODE2_2336;
 } else
    switch (buf[15])
      {
        case 2:
          result = MODE2_2352;
          break;

        case 1:
          result = MODE1_2352;
          break;

        case 0://TODO test this... at least we know MODE1_2048 has no sync headers
          result = MODE1_2048;
          break;
          
        default:
          printf ("\n");
          strhexdump (buf, 0, 0, 16);
          printf ("\n");
          break;
        }
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

  result = system (buf);
#ifndef __MSDOS__
  result >>= 8;                  // the exit code is coded in bits 8-15
#endif                          //  (that is, under Unix & BeOS)

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
ucon64_ls_main (const char *filename, struct stat *puffer, int mode)
{
  int result;
  char buf[MAXBUFSIZE];
  st_rominfo_t rominfo;

  ucon64.console = UCON64_UNKNOWN;
  ucon64.rom = filename;
  ucon64.type = (quickftell (ucon64.rom) <= MAXROMSIZE) ? UCON64_ROM : UCON64_CD;
  ucon64_flush (&rominfo);
  
  result = ucon64_console_probe (&rominfo);
    switch (mode)
      {
        case UCON64_LSV:
          if (!result) ucon64_nfo (&rominfo);
          break;
#if 0
//TODO renamer!
        case UCON64_REN:
          if (ucon64.console != UCON64_UNKNOWN)
//                        && ucon64.console != UCON64_KNOWN)
            {
              strcpy (buf, &ucon64.rom[findlast (ucon64.rom, ".") + 1]);
              printf ("%s.%s\n", rom.name, buf);
            }
          break;
#endif
        case UCON64_LS:
        default:
          strftime (buf, 13, "%b %d %H:%M", localtime (&puffer->st_mtime));
          printf ("%-31.31s %10ld %s %s\n", mkprint(rominfo.name, ' '),
            (long) puffer->st_size, buf, ucon64.rom);
          fflush (stdout);
          break;
    }
  return 0;
}

int ucon64_ls (const char *path, int mode)
{
  struct dirent *ep;
  struct stat puffer;
  char dir[FILENAME_MAX];
  char old_dir[FILENAME_MAX];
  DIR *dp;

  if (path)
    if (path[0])
    if (!stat (path, &puffer))
      if (S_ISREG (puffer.st_mode))
        return ucon64_ls_main (path, &puffer, mode);

  if (!path || !path[0])
    getcwd (dir, FILENAME_MAX);
  else
    strcpy (dir, path);
    
  if ((dp = opendir (dir)) == NULL)
    return -1;

  getcwd (old_dir,FILENAME_MAX); // remember current dir 
  chdir (dir);

  while ((ep = readdir (dp)))
    if (!stat (ep->d_name, &puffer))
      if (S_ISREG (puffer.st_mode))
        ucon64_ls_main (ep->d_name, &puffer, mode);

  closedir (dp);

  chdir (old_dir);

  return 0;
}


int
ucon64_configfile (void)
{
  char buf2[MAXBUFSIZE];
/*
  configfile handling
*/
  sprintf (ucon64.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
  "ucon64.cfg"
#else
  ".ucon64rc"
#endif
  , ms_getenv ("HOME"));

  if (access (ucon64.configfile, F_OK) != 0)
    {
      FILE *fh;

      printf ("WARNING: %s not found: creating...", ucon64.configfile);

      if (!(fh = fopen (ucon64.configfile, "wb")))
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
  else if (strcmp (getProperty (ucon64.configfile, "version", buf2, "198"), "198") != 0)
    {
      strcpy (buf2, ucon64.configfile);
      setext (buf2, ".OLD");

      printf ("NOTE: updating config: old version will be renamed to %s...", buf2);

      filecopy (ucon64.configfile, 0, quickftell (ucon64.configfile), buf2, "wb");

      setProperty (ucon64.configfile, "version", "198");

      setProperty (ucon64.configfile, "backups", "1");

#ifdef BACKUP
      setProperty (ucon64.configfile, "parport", "0x378");
#endif // BACKUP

      setProperty (ucon64.configfile, "emulate_gp32", "");

      setProperty (ucon64.configfile, "cdrw_read",
        getProperty (ucon64.configfile, "cdrw_raw_read", buf2, "cdrdao read-cd --read-raw --device 0,0,0 --driver generic-mmc-raw --datafile "));
      setProperty (ucon64.configfile, "cdrw_write",
        getProperty (ucon64.configfile, "cdrw_raw_write", buf2, "cdrdao write --device 0,0,0 --driver generic-mmc "));

      deleteProperty (ucon64.configfile, "cdrw_raw_read");
      deleteProperty (ucon64.configfile, "cdrw_raw_write");
      deleteProperty (ucon64.configfile, "cdrw_iso_read");
      deleteProperty (ucon64.configfile, "cdrw_iso_write");

      sync ();
      printf ("OK\n\n");
    }
  return 0;
}

