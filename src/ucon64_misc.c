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
#include "ucon64_misc.h"

#define MAXBUFSIZE 32768
#define DETECT_MAX_CNT 1000
#define CRC32_POLYNOMIAL     0xEDB88320L

unsigned long CRCTable[256];

#if     defined BACKUP && defined __BEOS__
static int ucon64_io_fd;
#endif

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
fileCRC32 (char *filename, long start)
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

char *
ucon64_fbackup (struct ucon64_ *rom, char *filename)
{
  if(!rom->backup) return (filename);

  printf ("Writing backup of: %s\n\n", filename);
  fflush (stdout);
  return filebackup (filename);
}

size_t
filepad (char *filename, long start, long unit)
/*
  pad file (if necessary) from start  size_t size;
  ignore start bytes (if file has header or something)
  unit is size of block (example: MBIT)
*/
{
  size_t size = quickftell (filename) - start;

/*
  if (!(size%unit))
    return size;

  size = (size_t) size/unit;
  size = size + 1;
  size = size*unit;

  truncate(filename, size + start);
*/

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
filetestpad (char *filename)
// test if EOF is padded (repeating bytes) beginning from start
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
/*
  if (y != (buf[x+1]&0xff))
    x++;
*/
  free (buf);

  return (size - x - 1 == 1) ? 0 : size - x - 1;
}

#ifdef  BACKUP
#if     defined __UNIX__ || defined __BEOS__ // DJGPP (DOS) has outportX() & inportX()
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
#endif // defined __UNIX__ || defined __BEOS__

int
detect_parport (unsigned int port)
{
  int i;

#ifdef  __linux__
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

#ifdef  __linux__
  if (ioperm (port, 1, 0) == -1)
    return -1;
#endif

  if (i >= DETECT_MAX_CNT)
    return 0;

  return 1;
}

#ifdef  __BEOS__
static void
close_io_port (void)
{
  close (ucon64_io_fd);
}
#endif

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
#ifdef  __linux__
      if (ioperm (port, 3, 1) == -1)            // data, status & control
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

/*
  checkout the definition of parport_gauge() in ucon64_misc.h
*/
int
fparport_gauge (FILE *output, time_t init_time, long pos, long size)
{
  long cps;
  int p, percentage;
  time_t curr, left;
  char progress[24 + 1];

  percentage = 100 * pos / size;
  if (output != stdout)
    {
      fprintf (output, "%u\n", percentage);
      fflush (output);
    }
  else
    {
      if ((curr = time (0) - init_time) == 0)
        curr = 1;                               // `round up' to at least 1 sec (no division
      if (pos > size)                           //  by zero below)
        return -1;

      cps = pos / curr;                         // # bytes/second (average transfer speed)
      left = (size - pos) / cps;

      p = 24 * pos / size;
      progress[0] = 0;
      strncat (progress, "========================", p);
      strncat (&progress[p], "------------------------", 24 - p);

      printf ("\r%10lu Bytes [%s] %u%%, CPS=%lu, ",
              pos, progress, percentage, (unsigned long) cps);

      if (pos == size)
        printf ("TOTAL=%03ld:%02ld", (long) curr / 60, (long) curr % 60); // DON'T print a newline
      else                                                                //  -> gauge can be cleared
        printf ("ETA=%03ld:%02ld   ", (long) left / 60, (long) left % 60);

      fflush (stdout);
    }

  return 0;
}
#endif // BACKUP

int
testsplit (char *filename)
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
trackmode (long imagesize)
// tries to figure out the used track mode of the cd image
{
  return ((!(imagesize % 2048)) ? 2048 :        // MODE1, MODE2_FORM1
          (!(imagesize % 2324)) ? 2324 :        // MODE2_FORM2
          (!(imagesize % 2336)) ? 2336 :        // MODE2, MODE2_FORM_MIX
          (!(imagesize % 2352)) ? 2352 :        // AUDIO, MODE1_RAW, MODE2_RAW
          -1                    // unknown
    );
}


int
raw2iso (char *filename)
// convert MODE1_RAW, MODE2_RAW, MODE2 and MODE2_FORM_MIX to ISO9660
{
#if 0
  int seek_header, seek_ecc, sector_size;
  long i, source_length;
  char buf[2352], destfilename[4096];
  const char SYNC_HEADER[12] =
    { 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0 };
  FILE *fdest, *fsource;

  strcpy (destfilename, filename);
  newext (destfilename, ".ISO");

  fsource = fopen (filename, "rb");
//  fdest = fopen(filebackup(destfilename),"wb");
  fdest = fopen (destfilename, "wb");

  fread (buf, sizeof (char), 16, fsource);
#endif
  return 0;
}
