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

const char *track_modes[] = {
  "MODE1/2048",
  "MODE1/2352",
  "MODE2/2336",
  "MODE2/2352"
};

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

const char *unknown_title = "Unknown backup unit/emulator";

char
hexDigit (int value)
{
  switch (toupper (value))
    {
    case 0:
      return '0';
    case 1:
      return '1';
    case 2:
      return '2';
    case 3:
      return '3';
    case 4:
      return '4';
    case 5:
      return '5';
    case 6:
      return '6';
    case 7:
      return '7';
    case 8:
      return '8';
    case 9:
      return '9';
    case 10:
      return 'A';
    case 11:
      return 'B';
    case 12:
      return 'C';
    case 13:
      return 'D';
    case 14:
      return 'E';
    case 15:
      return 'F';
    default:
      break;
    }
  return '?';
}

int
hexValue (char digit)
{
  switch (toupper (digit))
    {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'A':
      return 10;
    case 'B':
      return 11;
    case 'C':
      return 12;
    case 'D':
      return 13;
    case 'E':
      return 14;
    case 'F':
      return 15;
    default:
      break;
    }
  return 0;
}

int
hexByteValue (char x, char y)
{
  return (hexValue (x) << 4) + hexValue (y);
}

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
    printf ("ERROR: no parallel port 0x%s found\n\n", strupr (buf));
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
        printf ("ERROR: the images track mode is already MODE1/2048\n");
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
        printf ("ERROR: unknown/unsupported track mode");
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

          printf ("\n");
          strhexdump (buf, 0, 0, 16);
          printf ("\n");
  if (memcmp (SYNC_HEADER, buf, 12))
{printf("MODE2_2336");
    result = MODE2_2336;
 } else
    switch (buf[15])
      {
        case 2:
          result = MODE2_2352;
printf("MODE2_2352");
          break;

        case 1:
printf("MODE1_2352");
          result = MODE1_2352;
          break;

        case 0://TODO test this... at least we know MODE1_2048 has no sync headers
printf("MODE1_2048");
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
