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
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#ifdef _WIN32
//#include <windows.h>
//#include <shlobj.h>
#endif // _WIN32

#if     defined __unix__ || defined __BEOS__
#include <unistd.h>                             // ioperm() (libc5)
#endif

#include "config.h"
#ifdef  BACKUP
#ifdef  __FreeBSD__
#include <machine/sysarch.h>
#elif   defined __linux__
#ifdef  __GLIBC__
#include <sys/io.h>                             // ioperm() (glibc)
#endif
#elif   defined __BEOS__
#include <fcntl.h>
#endif
#endif // BACKUP

#include "ucon64.h"
#include "misc.h"
#include "quick_io.h"
#include "ucon64_misc.h"
#include "cdi.h"

#include "console/snes.h"
#include "console/gb.h"
#include "console/gba.h"
#include "console/n64.h"
#include "console/lynx.h"
#include "console/sms.h"
#include "console/nes.h"
#include "console/genesis.h"
#include "console/pce.h"
#include "console/neogeo.h"
#include "console/ngp.h"
#include "console/swan.h"
#include "console/dc.h"
#include "console/jaguar.h"
#include "console/other.h"


const st_track_modes_t track_modes[] = {
  {"MODE1/2048", "MODE1"},
  {"MODE1/2352", "MODE1_RAW"},
  {"MODE2/2336", "MODE2"},
  {"MODE2/2352", "MODE2_RAW"},
  {NULL, NULL}
};


static const char PVD_STRING[8] = { 0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\x01" "CD001" "\x01" "\0";
static const char SVD_STRING[8] = { 0x02, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\x02" "CD001" "\x01" "\0";
static const char VDT_STRING[8] = { 0xff, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };      //"\xFF" "CD001" "\x01" "\0";
static const char SYNC_DATA[12] =
  { 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0 };
static const char SUB_HEADER[8] = { 0, 0, 0x08, 0, 0, 0, 0x08, 0 };


const char *ucon64_parport_error =
  "ERROR: please check cables and connection\n"
  "       turn the backup unit off and on\n"
  "       split ROMs must be joined first\n"
  "       use " OPTION_LONG_S "file={3bc,378,278,...} to specify your port\n"
  "       set the port to SPP (Standard, Normal) mode in your BIOS\n"
  "       some backup units do not support EPP and ECP style parports\n"
  "       read the backup unit's manual\n";

const char *ucon64_console_error =
  "ERROR: could not auto detect the right ROM/IMAGE/console type\n"
  "TIP:   If this is a ROM or CD IMAGE you might try to force the recognition\n"
  "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n";

char *ucon64_temp_file = NULL;

void
ucon64_wrote (const char *filename)
{
  printf ("Wrote output to: %s\n", filename);
}


#if     defined BACKUP && defined __BEOS__
typedef struct st_ioport
{
  unsigned int port;
  unsigned char data8;
  unsigned short data16;
} st_ioport_t;

static int ucon64_io_fd;
#endif

const char *unknown_usage[] =
{
  "Unknown backup unit/Emulator",
  NULL,
  NULL,
  NULL
};


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
  struct stat src_info, dest_info;

  ucon64_temp_file = NULL;
  if (!access (dest, F_OK))
    {
      if (src == NULL)
        {
          if (ucon64.backup)
            printf ("Wrote backup to: %s\n", q_fbackup (dest, BAK_DUPE));
          return;
        }

#if 1
      // Check if src and dest are the same file based on the inode and device info,
      //  not the filenames
      stat (src, &src_info);
      stat (dest, &dest_info);
      if (src_info.st_dev == dest_info.st_dev && src_info.st_ino == dest_info.st_ino)
#else
      if (!strcmp (dest, src))
#endif
        {                                       // case 1
          if (ucon64.backup)
            {                                   // case 1a
              strcpy (src, q_fbackup (dest, BAK_DUPE));
              printf ("Wrote backup to: %s\n", src);
            }
          else
            {                                   // case 1b
              strcpy (src, q_fbackup (dest, BAK_MOVE));
              ucon64_temp_file = src;
#ifdef  DEBUG
              printf ("case 1b, src: %s\n", src);
              fflush (stdout);
#endif
            }
        }
      else
        {
          if (ucon64.backup)
            printf ("Wrote backup to: %s\n", q_fbackup (dest, BAK_DUPE));
#ifdef  DEBUG
          else
            {
              printf ("case 2b, src: %s\n", src);
              fflush (stdout);
            }
#endif
        }
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


int
ucon64_fhexdump (const char *filename, int start, int len)
{
  int pos, size = q_fsize (filename), value = 0,
      buf_size = MAXBUFSIZE - (MAXBUFSIZE % 16); // buf_size must be < MAXBUFSIZE && 16 * n
  char buf[MAXBUFSIZE];
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    return -1;

  if ((size - start) < len)
    len = size - start;

  fseek (fh, start, SEEK_SET);
  for (pos = 0; pos < len; pos += buf_size)
    {
      value = fread (buf, 1, MIN (len, buf_size), fh);
      mem_hexdump (buf, value, pos + start);
    }

  fclose (fh);

  return 0;
}


#define FILEFILE_LARGE_BUF
unsigned int
ucon64_filefile (const char *filename1, int start1, const char *filename2, int start2,
                 int similar)
{
  int base, fsize1, fsize2, len, chunksize1, chunksize2, readok = 1,
      bytesread1, bytesread2, bytesleft1, bytesleft2;
#ifdef  FILEFILE_LARGE_BUF
  int bufsize = 1024 * 1024;
  unsigned char *buf1, *buf2;
#else
  int bufsize = MAXBUFSIZE;
  unsigned char buf1[MAXBUFSIZE], buf2[MAXBUFSIZE];
#endif
  FILE *file1, *file2;

  if (!strcmp (filename1, filename2))
    return 0;
  if (access (filename1, R_OK) != 0 || access (filename2, R_OK) != 0)
    return -1;

  fsize1 = q_fsize (filename1);              // q_fsize() returns size in bytes
  fsize2 = q_fsize (filename2);
  if (fsize1 < start1 || fsize2 < start2)
    return -1;

#ifdef  FILEFILE_LARGE_BUF
  if (!(buf1 = (unsigned char *) malloc (bufsize)))
    return -1;

  if (!(buf2 = (unsigned char *) malloc (bufsize)))
    {
      free (buf1);
      return -1;
    }
#endif

  if (!(file1 = fopen (filename1, "rb")))
    {
#ifdef  FILEFILE_LARGE_BUF
      free (buf1);
      free (buf2);
#endif
      return -1;
    }
  if (!(file2 = fopen (filename2, "rb")))
    {
      fclose (file1);
#ifdef  FILEFILE_LARGE_BUF
      free (buf1);
      free (buf2);
#endif
      return -1;
    }

  fseek (file1, start1, SEEK_SET);
  fseek (file2, start2, SEEK_SET);
  bytesleft1 = fsize1;
  bytesread1 = 0;
  bytesleft2 = fsize2;
  bytesread2 = 0;

  while (bytesleft1 > 0 && bytesread1 < fsize2 && readok)
    {
      chunksize1 = fread (buf1, 1, bufsize, file1);
      if (chunksize1 == 0)
        readok = 0;
      else
        {
          bytesread1 += chunksize1;
          bytesleft1 -= chunksize1;
        }

      while (bytesleft2 > 0 && bytesread2 < bytesread1 && readok)
        {
          chunksize2 = fread (buf2, 1, chunksize1, file2);
          if (chunksize2 == 0)
            readok = 0;
          else
            {
              base = 0;
              while (base < chunksize2)
                {
                  if (similar == TRUE ?
                      buf1[base] == buf2[base] :
                      buf1[base] != buf2[base])
                    {
                      for (len = 0; base + len < chunksize2; len++)
                        if (similar == TRUE ?
                            buf1[base + len] != buf2[base + len] :
                            buf1[base + len] == buf2[base + len])
                          break;

                      printf ("%s:\n", filename1);
                      mem_hexdump (&buf1[base], len, start1 + base + bytesread2);
                      printf ("%s:\n", filename2);
                      mem_hexdump (&buf2[base], len, start2 + base + bytesread2);
                      printf ("\n");
                      base += len;
                    }
                  else
                    base++;
                }

              bytesread2 += chunksize2;
              bytesleft2 -= chunksize2;
            }
        }
    }

  fclose (file1);
  fclose (file2);
#ifdef  FILEFILE_LARGE_BUF
  free (buf1);
  free (buf2);
#endif
  return 0;
}


int
ucon64_pad (const char *filename, int start, int size)
/*
  Pad file (if necessary) to start + size bytes;
  Ignore start bytes (if file has header or something)
*/
{
  FILE *file;
  int oldsize = q_fsize (filename) - start, sizeleft;
  unsigned char padbuffer[MAXBUFSIZE];

  // now we can also "pad" to smaller sizes
  if (oldsize > size)
    truncate (filename, size + start);
  else if (oldsize < size)
    {
      // don't use truncate() to enlarge files, because the result is undefined (by POSIX)
      if ((file = fopen (filename, "ab")) == NULL)
        {
          fprintf (stderr, "ERROR: Can't open %s for writing\n", filename);
          exit (1);
        }
      sizeleft = size - oldsize;
      memset (padbuffer, 0, MAXBUFSIZE);
      while (sizeleft >= MAXBUFSIZE)
        {
          fwrite (padbuffer, 1, MAXBUFSIZE, file);
          sizeleft -= MAXBUFSIZE;
        }
      if (sizeleft)
        fwrite (padbuffer, 1, sizeleft, file);
      fclose (file);
    }
  return size;
}


#if 0
int
ucon64_testpad (const char *filename, st_rominfo_t *rominfo)
// test if EOF is padded (repeating bytes)
{
  int size = rominfo->file_size;
  int pos = rominfo->file_size - 2;
  int c = q_fgetc (filename, rominfo->file_size - 1);
  unsigned char *buf;

  if (!(buf = (unsigned char *) malloc ((size + 2) * sizeof (unsigned char))))
    return -1;

  q_fread (buf, 0, size, filename);

  while (c == buf[pos])
    pos--;

  free (buf);

  size -= (pos + 1);
  return size > 1 ? size : 0;
}
#else
int
ucon64_testpad (const char *filename, st_rominfo_t *rominfo)
// test if EOF is padded (repeating bytes)
{
  int pos = rominfo->file_size - 1;
  int buf_pos = pos % MAXBUFSIZE;
  int c = q_fgetc (filename, pos);
  unsigned char buf[MAXBUFSIZE];
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    return -1;

  for (pos -= buf_pos; !fseek (fh, pos, SEEK_SET) && pos > -1;
        pos -= MAXBUFSIZE, buf_pos = MAXBUFSIZE)
    {
      fread (buf, 1, buf_pos, fh);

      for (; buf_pos > 0; buf_pos--)
        if (buf[buf_pos - 1] != c)
          {
            fclose (fh);

            return rominfo->file_size - (pos + buf_pos) > 1 ?
              rominfo->file_size - (pos + buf_pos) : 0;
          }
    }

  fclose (fh);

  return rominfo->file_size;                    // the whole file is "padded"
}
#endif


#ifdef  BACKUP
#ifdef  __BEOS__
void
close_io_port (void)
{
  close (ucon64_io_fd);
}
#endif


unsigned char
inportb (unsigned short port)
{
#ifdef  __BEOS__
  st_ioport_t temp;

  temp.port = port;
  ioctl (ucon64_io_fd, 'r', &temp, 0);

  return temp.data8;
#elif   defined __i386__
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
#elif   defined __i386__
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
#elif   defined __i386__
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
#elif   defined __i386__
  __asm__ __volatile__
  ("outw %1, %0"
    :
    : "d" (port), "a" (word)
  );
#endif
}


#define DETECT_MAX_CNT 1000
static int
ucon64_parport_probe (unsigned int port)
{
  int i = 0;

#ifdef  __FreeBSD__
  if (i386_set_ioperm (port, 1, 1) == -1)
    return -1;
#elif   defined __linux__
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

#ifdef  __FreeBSD__
  if (i386_set_ioperm (port, 1, 0) == -1)
    return -1;
#elif   defined __linux__
  if (ioperm (port, 1, 0) == -1)
    return -1;
#endif

  if (i >= DETECT_MAX_CNT)
    return 0;

  return 1;
}


unsigned int
ucon64_parport_init (unsigned int port)
{
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
#endif
#ifdef  __i386__                                // 0x3bc, 0x378, 0x278
  if (!port)                                    // no port specified or forced?
    {
      unsigned int parport_addresses[] = { 0x3bc, 0x378, 0x278 };
      int x, found = 0;

      for (x = 0; x < 3; x++)
        if ((found = ucon64_parport_probe (parport_addresses[x])) == 1)
          {
            port = parport_addresses[x];
            break;
          }

      if (found != 1)
        {
          fprintf (stderr, "ERROR: Could not find a parallel port on your system\n"
                           "       Try " OPTION_LONG_S "port=PORT to specify it by hand\n\n");
          exit (1);
        }
    }
#endif // __i386__

#if     defined __linux__ || defined __FreeBSD__
#ifdef  __FreeBSD__
      if (i386_set_ioperm (port, 3, 1) == -1)   // data, status & control
#else
      if (ioperm (port, 3, 1) == -1)            // data, status & control
#endif
        {
          fprintf (stderr,
                   "ERROR: Could not set port permissions for I/O ports 0x%x, 0x%x and 0x%x\n"
                   "       (This program needs root privileges for the requested action)\n",
                   port + PARPORT_DATA, port + PARPORT_STATUS, port + PARPORT_CONTROL);
          exit (1);                             // Don't return, if ioperm() fails port access
        }                                       //  causes core dump
#endif // __linux__ || __FreeBSD__

  outportb (port + PARPORT_CONTROL,
    inportb (port + PARPORT_CONTROL) & 0x0f);   // bit 4 = 0 -> IRQ disable for
                                                //  ACK, bit 5-7 unused

#if     defined __linux__ || defined __FreeBSD__
  /*
    Some code needs us to switch to the real uid and gid. However, other code
    needs access to I/O ports other than the standard printer port registers.
    We just do an iopl(3) and all code should be happy. Using iopl(3) enables
    users to run all code without being root (of course with the uCON64
    executable setuid root). Anyone a better idea?
  */
#ifdef  __FreeBSD__
  if (i386_iopl (3) == -1)
#else
  if (iopl (3) == -1)
#endif
    {
      fprintf (stderr, "ERROR: Could not set the I/O privilege level to 3\n"
                       "       (This program needs root privileges for the requested action)\n");
      return 1;
    }
#endif // __linux__ || __FreeBSD__

  return port;
}
#endif // BACKUP


#if 0
const char *
ucon64_extract (const char *archive)
{
#ifndef __MSDOS__
  DIR *dp;
  struct dirent *ep;
  struct stat fstate;
  char buf[FILENAME_MAX], cwd[FILENAME_MAX];
  int result = 0;
  char temp[FILENAME_MAX];
  char path[FILENAME_MAX];
  char property_name[MAXBUFSIZE], buf2[MAXBUFSIZE];
  const char *property_format = NULL;

  if (!archive) return archive;
  if (!archive[0]) return archive;

  getcwd (cwd, FILENAME_MAX);
  sprintf (path, "%s" FILE_SEPARATOR_S "%s", cwd, basename2 (archive));

  sprintf (property_name, "%s_extract", &getext (archive)[1]);
  property_format = get_property (ucon64.configfile, strlwr (property_name), buf2, NULL);

  if (!property_format)
    return archive;

  sprintf (buf, property_format, path);

  if (!buf[0])
    return archive;

  tmpnam3 (temp, TYPE_DIR);
  chdir (temp);

#ifdef  DEBUG
  fprintf (stderr, "%s\n", temp);
#endif

#if 1
  result = system (buf)
#ifndef __MSDOS__
      >> 8                                      // the exit code is coded in bits 8-15
#endif                                          //  (that is, under non-DOS)
    ;
  sync ();
#else
  fprintf (stderr, "%s\n", buf);
  fflush (stdout);
#endif

  chdir (cwd);

  if ((dp = opendir (temp)) != NULL)
    while ((ep = readdir (dp)))
      {
        strcpy (ucon64.rom_in_archive, ep->d_name);
        sprintf (path, "%s" FILE_SEPARATOR_S "%s", temp, ucon64.rom_in_archive);

        if (!stat (path, &fstate))
          if (S_ISREG (fstate.st_mode) && fstate.st_size >= MBIT)
            {
#ifdef  DEBUG
              fprintf (stderr, "%s\n\n", path);
#endif
              q_fcpy (path, 0, q_fsize (path), ucon64.rom_in_archive, "wb");
              rmdir2 (temp);
              return ucon64.rom_in_archive;
            }
      }
  rmdir2 (temp);
#endif
  return archive;
}
#endif


int
ucon64_gauge (time_t init_time, int pos, int size)
{
  if (!ucon64.frontend)
    return gauge (init_time, pos, size);
  else
    {
      int percentage = (100LL * pos) / size;

      printf ("%u\n", percentage);
      fflush (stdout);
      return 0;
    }
}


int
ucon64_testsplit (const char *filename)
// test if ROM is split into parts based on the name of files
{
  int x, parts = 0;
  char buf[FILENAME_MAX], *p = NULL;

  if (!strchr (filename, '.'))
    return 0;

  for (x = -1; x < 2; x += 2)
    {
      parts = 0;
      strcpy (buf, filename);
      p = strrchr (buf, '.') + x;               // if x == -1 change char before '.'
                                                // else if x == 1 change char behind '.'

      if (buf > p ||                            // filename starts with a period
          p - buf > strlen (buf) - 1)           // filename ends with a period
        continue;

      while (!access (buf, F_OK))
        (*p)--;                                 // "rewind" (find the first part)
      (*p)++;

      while (!access (buf, F_OK))               // count split parts
        {
          (*p)++;
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
  int seek_header, seek_ecc, sector_size, i, size;
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
#ifdef  __MAC__ // macintosh
        seek_header = 0;
#else
        seek_header = 8;
#endif
        seek_ecc = 280;
        sector_size = 2336;
        break;

      case MODE2_2352:
#ifdef  __MAC__ // macintosh
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

  strcpy (buf, basename2(image));
  setext (buf, ".ISO");
  size = q_fsize (image) / sector_size;

  if (!(src = fopen (image, "rb")))
    return -1;
  if (!(dest = fopen (buf, "wb")))
    {
      fclose (src);
      return -1;
    }

  for (i = 0; i < size; i++)
    {
      fseek (src, seek_header, SEEK_CUR);
#ifdef  __MAC__
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

//#if 1
int
seek_pvd (int sector_size, int mode, const char *filename)
// will search for valid PVD in sector 16 of source image
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
  q_fread (buf, 16 * sector_size
    + (sector_size == 2352 ? 16 : 0) // header
    + (mode == 2 ? 8 : 0)            // subheader
    , 8, filename);

  if (!memcmp (PVD_STRING, buf, 8)
#if 0
      || !memcmp (SVD_STRING, buf, 8)
      || !memcmp (VDT_STRING, buf, 8)
#endif
    )
    return 1;

  return 0;
}
//#else
static int
seek_pvd2 (char *buffer, int sector_size, int mode, FILE * fsource)
// will search for valid PVD in sector 16 of source image
{
  fseek (fsource, 16 * sector_size, SEEK_SET);  // boot area

  if (sector_size == 2352)
    fseek (fsource, 16, SEEK_CUR);      // header

  if (mode == 2)
    fseek (fsource, 8, SEEK_CUR);       // subheader

  fread (buffer, 1, 8, fsource);

  if (!memcmp (PVD_STRING, buffer, 8))
    return 1;

  return 0;
}
//#endif


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

  q_fread (buf, 0, 16, image);

#ifdef DEBUG
  mem_hexdump(buf, 16, 0);
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
  fsize = q_fsize (ucon64.rom);

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


int 
ucon64_e (const char *romfile)
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

  property = get_property (ucon64.configfile, buf3, buf2, NULL);   // buf2 also contains property value
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
      fprintf (stderr, "ERROR: the Emulator returned an error code (%d)\n"
               "TIP:   If the wrong emulator was used you might try to force recognition\n"
               "       The force recognition option for Super Nintendo would be " OPTION_LONG_S "snes\n",
               result);
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
  ucon64_flush (&rominfo);
  result = ucon64_init (ucon64.rom, &rominfo);
  ucon64.type = (rominfo.file_size <= MAXROMSIZE) ? UCON64_ROM : UCON64_CD;

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
          strcat (buf, getext (ucon64.rom));
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
      printf ("%-31.31s %10d %s %s\n", mkprint (rominfo.name, ' '),
              rominfo.file_size, buf, ucon64.rom);
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
  char dir[FILENAME_MAX], old_dir[FILENAME_MAX];
  DIR *dp;
  int console = ucon64.console;

  dir[0] = 0;

  if (path)
    if (path[0])
      {
        if (!stat (path, &fstate))
          {
            if (S_ISREG (fstate.st_mode))
              {
#ifdef  ZLIB
                int n = unzip_get_number_entries (path), retval = 0;
                if (n != -1)
                  {
                    for (unzip_current_file_nr = 0; unzip_current_file_nr < n;
                         unzip_current_file_nr++)
                      retval = ucon64_ls_main (path, &fstate, mode, console);
                    unzip_current_file_nr = 0;
                  }
                else
                  retval = ucon64_ls_main (path, &fstate, mode, console);
                return retval;
#else
                return ucon64_ls_main (path, &fstate, mode, console);
#endif
              }
          }
        strcpy (dir, path);
      }

  if (!dir[0])
    getcwd (dir, FILENAME_MAX);

  if ((dp = opendir (dir)) == NULL)
    return -1;

  getcwd (old_dir, FILENAME_MAX);               // remember current dir
  chdir (dir);

  while ((ep = readdir (dp)))
    if (!stat (ep->d_name, &fstate))
      if (S_ISREG (fstate.st_mode))
        {
#ifdef  ZLIB
          int n = unzip_get_number_entries (ep->d_name);
          if (n != -1)
            {
              for (unzip_current_file_nr = 0; unzip_current_file_nr < n;
                   unzip_current_file_nr++)
                ucon64_ls_main (ep->d_name, &fstate, mode, console);
              unzip_current_file_nr = 0;
            }
          else
#endif
          ucon64_ls_main (ep->d_name, &fstate, mode, console);
        }

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
  char buf[256], buf2[MAXBUFSIZE], *dirname;

  dirname = getenv2 ("HOME");
  sprintf (ucon64.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
    "ucon64.cfg"
#else
    ".ucon64rc"
#endif
    , dirname);

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
#ifdef ANSI_COLOR
                 "# use ANSI colors in output? (1=yes; 0=no)\n"
                 "#\n"
                 "ansi_color=1\n"
#endif
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
#if 0
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
#endif
#ifdef  BACKUP_CD
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
                 , UCON64_CONFIG_VERSION);
          fclose (fh);
          printf ("OK\n\n");
        }
    }
  else if (strtol (get_property (ucon64.configfile, "version", buf2, "0"), NULL, 10) < UCON64_CONFIG_VERSION)
    {
      strcpy (buf2, ucon64.configfile);
      setext (buf2, ".OLD");

      printf ("NOTE: updating config: old version will be renamed to %s...", buf2);

      q_fcpy (ucon64.configfile, 0, q_fsize (ucon64.configfile), buf2, "wb");

      sprintf (buf, "%d", UCON64_CONFIG_VERSION);
      set_property (ucon64.configfile, "version", buf);

      set_property (ucon64.configfile, "ansi_color", "1");
#if 0

#ifdef BACKUP
      set_property (ucon64.configfile, "parport", "0x378");
#endif // BACKUP

      set_property (ucon64.configfile, "emulate_gp32", "");
      set_property (ucon64.configfile, "cdrw_read",
        get_property (ucon64.configfile, "cdrw_raw_read", buf2, "cdrdao read-cd --read-raw --device 0,0,0 --driver generic-mmc-raw --datafile "));
      set_property (ucon64.configfile, "cdrw_write",
        get_property (ucon64.configfile, "cdrw_raw_write", buf2, "cdrdao write --device 0,0,0 --driver generic-mmc "));
#endif

      DELETE_PROPERTY (ucon64.configfile, "cdrw_raw_read");
      DELETE_PROPERTY (ucon64.configfile, "cdrw_raw_write");
      DELETE_PROPERTY (ucon64.configfile, "cdrw_iso_read");
      DELETE_PROPERTY (ucon64.configfile, "cdrw_iso_write");

#if 0
      set_property (ucon64.configfile, "lha_extract", "lha efi \"%s\"");
      set_property (ucon64.configfile, "lzh_extract", "lha efi \"%s\"");
      set_property (ucon64.configfile, "zip_extract", "unzip -xojC \"%s\"");
      set_property (ucon64.configfile, "rar_extract", "unrar x \"%s\"");
      set_property (ucon64.configfile, "ace_extract", "unace e \"%s\"");
#endif

      sync ();
      printf ("OK\n\n");
    }
  return 0;
}


int
ucon64_toc2cue (const char *toc_file)
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
         strun = stitr.next(), stcount++)
      {

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
        if (binFileName == NULL)
          binFileName = strdupCC(strun->filename());
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
        message(-2, "Cannot convert: track %d references no data file.", trackNr);
        err = 1;
      }
      break;

    case 2:
      if (sttype1 != TrackData::ZERODATA || sttype2 != TrackData::DATAFILE) {
        message(-2, "Cannot convert: track %d has unsupported layout.", trackNr);
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
    message(-2, "Cannot open cue file \'%s\' for writing: %s", cueFile, strerror(errno));
    exit(1);
  }

  out << "FILE \"" << binFileName << "\" BINARY" << endl;

  int offset = 0;

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


#if 0
void
frame2msf (unsigned int i, struct cdrom_msf *msf)
{
  msf->cdmsf_min0 = i / CD_SECS / CD_FRAMES;
  msf->cdmsf_sec0 = (i / CD_FRAMES) % CD_SECS;
  msf->cdmsf_frame0 = i % CD_FRAMES;
  msf->cdmsf_min1 = (i + 1) / CD_SECS / CD_FRAMES;
  msf->cdmsf_sec1 = ((i + 1) / CD_FRAMES) % CD_SECS;
  msf->cdmsf_frame1 = (i + 1) % CD_FRAMES;
}


unsigned int
msf2frame (struct cdrom_msf0 *msf)
{
  return (msf->minute * CD_SECS * CD_FRAMES +
          msf->second * CD_FRAMES + msf->frame);
}
#endif


#if 0
int
read_raw_frame (int fd, int lba, unsigned char *buf)
{
  struct cdrom_msf *msf = (struct cdrom_msf *) buf;
  int rc;

//  msf = (struct cdrom_msf *) buf;
  msf->cdmsf_min0 = (lba + CD_MSF_OFFSET) / CD_FRAMES / CD_SECS;
  msf->cdmsf_sec0 = (lba + CD_MSF_OFFSET) / CD_FRAMES % CD_SECS;
  msf->cdmsf_frame0 = (lba + CD_MSF_OFFSET) % CD_FRAMES;

  if ((rc = ioctl (fd, CDROMREADMODE2, buf)) == -1)
    fprintf (stderr, "ERROR: ioctl CDROMREADMODE2\n");

  return rc;
}
#endif


static void
write_wav_header (FILE * fdest, int track_length) 
{
  st_wav_header_t wav;

  strcpy (wav.magic, "RIFF");
  strcpy (wav.type, "WAVE");
  strcpy (wav.fmt, "fmt ");
  strcpy (wav.data, "data");
  
  wav.header_length = 16;
  wav.format = 1;
  wav.channels = 2;
  wav.samplerate = 44100;
  wav.bitrate = 176400;
  wav.blockalign = 4;
  wav.bitspersample = 16;

  wav.data_length = track_length * 2352;
  wav.total_length = wav.data_length + 8 + 16 + 12;

#ifdef DEBUG
  fprintf (stderr, "%d\n", sizeof (st_wav_header_t));
  fflush (stderr);
#endif  
    
  fwrite (&wav, sizeof (st_wav_header_t), 1, fdest);
}


struct buffer_s
{
  FILE * file;
  char *ptr;
  int index;
  int size;
};


static int
BufWrite (char *data, int data_size, struct buffer_s *buffer)
{
  int write_length;
  if (data_size > (buffer->size + (buffer->size - buffer->index - 1)))
    return 0;                   // unimplemented
  if (buffer->index + data_size < buffer->size) // 1 menos
    {
      memcpy ((buffer->ptr + buffer->index), data, data_size);
      buffer->index += data_size;
    }
  else
    {
      write_length = buffer->size - buffer->index;
      memcpy ((buffer->ptr + buffer->index), data, write_length);
      fwrite (buffer->ptr, buffer->size, 1, buffer->file);
      memcpy (buffer->ptr, data + write_length, data_size - write_length);
      buffer->index = data_size - write_length;
    }
  return 1;
}


static int
BufWriteFlush (struct buffer_s *buffer)
{
  fwrite (buffer->ptr, buffer->index, 1, buffer->file);
  buffer->index = 0;
  return 1;
}


static int
BufRead (char *data, int data_size, struct buffer_s *buffer, int filesize)
{
  int read_length, max_length, pos;
  if (data_size > (buffer->size + (buffer->size - buffer->index - 1)))
    return 0;                   // unimplemented
  if (filesize == 0)            // no cuenta
    {
      max_length = buffer->size;
    }
  else
    {
      pos = ftell (buffer->file);
      if (pos > filesize)
        max_length = 0;
      else
        max_length =
          ((pos + buffer->size) > filesize) ? (filesize - pos) : buffer->size;
    }
  if (buffer->index == 0)
    {
      fread (buffer->ptr, max_length, 1, buffer->file);
    }
  if (buffer->index + data_size <= buffer->size)
    {
      memcpy (data, buffer->ptr + buffer->index, data_size);
      buffer->index += data_size;
      if (buffer->index >= buffer->size)
        buffer->index = 0;
    }
  else
    {
      read_length = buffer->size - buffer->index;
      memcpy (data, buffer->ptr + buffer->index, read_length);
      fread (buffer->ptr, max_length, 1, buffer->file);
      memcpy (data + read_length, buffer->ptr, data_size - read_length);
      buffer->index = data_size - read_length;
    }
  return 1;
}


char *global_read_buffer_ptr;
char *global_write_buffer_ptr;

#define DEFAULT_FORMAT   0
#define ISO_FORMAT       1
#define BIN_FORMAT       2
  
#define SHOW_INTERVAL 2000
  
#define READ_BUF_SIZE  1024*1024
#define WRITE_BUF_SIZE 1024*1024

typedef struct st_opts
{
  char cutfirst;
  char cutall;
  char convert;
  char fulldata;
  char rawaudio;
  char swap;
  char pregap;
} st_opts_t;

typedef struct st_flags
{
  char do_cut;
  char do_convert;
//  char create_cuesheet;
  char save_as_iso;
} st_flags_t;

static const char STR_TDISC_CUE_FILENAME[] = "tdisc.cue";
static const char STR_TDISCN_CUE_FILENAME[] = "tdisc%d.cue";
static const char STR_TAUDIO_RAW_FILENAME[] = "taudio%02d.raw";
static const char STR_TAUDIO_WAV_FILENAME[] = "taudio%02d.wav";
static const char STR_TDATA_ISO_FILENAME[] = "tdata%02d.iso";
static const char STR_TDATA_BIN_FILENAME[] = "tdata%02d.bin";


static void
save_cue_sheet (FILE * fcuesheet, st_image_t * image, st_track_t * track,
              st_opts_t * opts, st_flags_t * flags)
{
  char track_format_string[10];

  strcpy (track_format_string, opts->swap ? "MOTOROLA" : "BINARY");

  if (track->mode == 0)

    {
      if (opts->rawaudio)
        fprintf (fcuesheet, "FILE TAUDIO%02d.RAW %s\r\n"
                 "  TRACK %02d AUDIO\r\n", track->global_current_track,
                 track_format_string, track->number);

      else
        fprintf (fcuesheet, "FILE TAUDIO%02d.WAV WAVE\r\n"
                 "  TRACK %02d AUDIO\r\n", track->global_current_track,
                 track->number);
      if (track->global_current_track > 1 && !opts->pregap 
          && track->pregap_length > 0)
        fprintf (fcuesheet, "    PREGAP 00:02:00\r\n");

    }
  else
    {
      if (flags->save_as_iso)
        fprintf (fcuesheet, "FILE TDATA%02d.ISO BINARY\r\n"
                 "  TRACK %02d MODE%d/2048\r\n",
                 track->global_current_track, track->number, track->mode);

      else
        fprintf (fcuesheet, "FILE TDATA%02d.BIN BINARY\r\n"
                 "  TRACK %02d MODE%d/%d\r\n", track->global_current_track,
                 track->number, track->mode, track->sector_size);
    }

  fprintf (fcuesheet, "    INDEX 01 00:00:00\r\n");
  if (opts->pregap && track->mode != 0 && image->remaining_tracks > 1)  // instead of saving pregap
    fprintf (fcuesheet, "  POSTGAP 00:02:00\r\n");
}


static int
sector_read (char *buffer, int sector_size, int mode, FILE * fsource)
// will put user data into buffer no matter the source format
{
  int status;

  if (sector_size == 2352)
    fseek (fsource, 16, SEEK_CUR);      // header

  if (mode == 2)
    fseek (fsource, 8, SEEK_CUR);       // subheader

  status = fread (buffer, 2048, 1, fsource);

  if (sector_size >= 2336)
    {
      fseek (fsource, 280, SEEK_CUR);

      if (mode == 1)
        fseek (fsource, 8, SEEK_CUR);
    }

  return status;
}


static void
save_track (FILE * fh, st_image_t * image, st_track_t * track, st_opts_t * opts,
           st_flags_t * flags) 
{
  unsigned int i;
  int track_length;
  unsigned int header_length = 0;
  int all_fine;
  char buffer[2352], imagename[13];
  static time_t start_time = 0;
  FILE *fdest;
  struct buffer_s read_buffer;
  struct buffer_s write_buffer;

  if (!start_time) start_time = time (0);

  fseek (fh, track->position, SEEK_SET);

  if (track->mode == 0)
        sprintf (imagename, opts->rawaudio ? STR_TAUDIO_RAW_FILENAME :
                  STR_TAUDIO_WAV_FILENAME,
                  track->global_current_track);
  else
      sprintf (imagename, flags->save_as_iso ? STR_TDATA_ISO_FILENAME :
                STR_TDATA_BIN_FILENAME,
                track->global_current_track);

  if (!(fdest = fopen (imagename, "wb")))
    {
//    error
      return;
    }
    
  read_buffer.file = fh;
  read_buffer.size = READ_BUF_SIZE;
  read_buffer.index = 0;
  read_buffer.ptr = global_read_buffer_ptr;

  write_buffer.file = fdest;
  write_buffer.size = WRITE_BUF_SIZE;
  write_buffer.index = 0;
  write_buffer.ptr = global_write_buffer_ptr;

  fseek (fh, track->pregap_length * track->sector_size, SEEK_CUR);       // always skip pregap

  if (flags->do_cut != 0) printf ("[cut: %d] ", flags->do_cut);

  track_length = track->length - flags->do_cut;       // para no modificar valor original

  if (opts->pregap && track->mode == 0 && image->remaining_tracks > 1) // quick hack to save next track pregap (audio tracks only)
    track_length += track->pregap_length;       // if this isn't last track in current session

//  printf (flags->do_convert ? "[ISO]\n" : "\n");

  if (flags->do_convert)
    {
      if (track->mode == 2)
        {
          switch (track->sector_size)
            {
            case 2352:
              header_length = 24;
              break;
            case 2336:
              header_length = 8;
              break;
            default:
              header_length = 0;
            }
        }
      else
        {
          switch (track->sector_size)
            {
            case 2352:
              header_length = 16;
              break;
            case 2048:
            default:
              header_length = 0;
            }
        }
    }

  if (track->mode == 0 && !opts->rawaudio)
    write_wav_header (fdest, track_length);

  for (i = 0; i < track_length; i++)
    {
      if (!(i % 128))
//        show_counter (i, track_length, image->length, ftell (fh));
        ucon64_gauge(start_time, ftell (fh), image->length);

      BufRead (buffer, track->sector_size, &read_buffer, image->length);

      if (track->mode == 0 && opts->swap)
        mem_swap (buffer, track->sector_size);

      all_fine = flags->do_convert ? 
                   BufWrite (buffer + header_length, 2048, &write_buffer) :
                   BufWrite (buffer, track->sector_size, &write_buffer);

      if (!all_fine)
        {
          fprintf (stderr, "ERROR: %s\n", imagename);
          return;
        }
      
    }
  
//  if (flags->do_cut)
//    fseek (fh, flags->do_cut * track->sector_size, SEEK_CUR);
    
  fseek (fh, track->position, SEEK_SET);
  
//  fseek(fh, track->pregap_length * track->sector_size, SEEK_CUR);
//  fseek(fh, track->length * track->sector_size, SEEK_CUR);
  fseek (fh, track->total_length * track->sector_size, SEEK_CUR);
  
  BufWriteFlush (&write_buffer);
  fflush (fdest);
  fclose (fdest);
}


int
ucon64_cdirip (const char *imagename)
{
  char cuesheetname[13];
  FILE *fh = NULL, *fcuesheet = NULL;
  st_image_t image;
  st_track_t track;
  st_opts_t opts;
  st_flags_t flags;

  if (!(global_read_buffer_ptr = (char *) malloc (READ_BUF_SIZE)))
    return -1;

  if (!(global_write_buffer_ptr = (char *) malloc (WRITE_BUF_SIZE)))
    {
      free (global_read_buffer_ptr);
      return -1;
    }

  if (!(fh = fopen (ucon64.rom, "rb")))
    return -1;

  memset (&image, 0, sizeof (st_image_t));
  memset (&track, 0, sizeof (st_track_t));

#if 0
  image.global_current_session =
  track.global_current_track =
  track.position = 0;
#endif
  cdi_init (fh, &image, ucon64.rom);

  printf ("DiscJuggler/CDI version: ");

  switch (image.version)
    {
      case CDI_V2:
        printf ("2.x\n");
        break;
        
      case CDI_V3:
        printf ("3.x\n");
        break;
        
#if 0
      case CDI_V4:
        printf ("4.x\n");
        break;
#endif        

      default:
        printf ("%s (not supported)\n", UCON64_UNKNOWN_S);
        fclose (fh);
        return -1;
    }
  printf ("\n");

  memset (&opts, 0, sizeof (st_opts_t));
  memset (&flags, 0, sizeof (st_flags_t));

  opts.convert = ISO_FORMAT;    // Linux only!
#if 0
  opts.convert = ISO_FORMAT;    // iso
  opts.convert = BIN_FORMAT;    // bin
  opts.rawaudio = TRUE;         // raw
  opts.cutfirst = TRUE;         // cut
  opts.cutall = TRUE;           // cutall
  opts.fulldata = TRUE;         // full
  opts.swap = TRUE;             // swap
  opts.showspeed = TRUE;        // speed
  opts.pregap = TRUE;           // pregap data will be saved

  opts.cutall = TRUE;           // cdrecord
  opts.convert = ISO_FORMAT;    // cdrecord

  opts.cutall = TRUE;           // winoncd
  opts.convert = ISO_FORMAT;    // winoncd
  opts.rawaudio = TRUE;         // winoncd

  opts.cutall = TRUE;           // fireburner
  opts.convert = BIN_FORMAT;    // fireburner
#endif

  cdi_get_sessions (fh, &image);

  if (image.sessions == 0)
    {
      fprintf (stderr, "ERROR: Bad format: Could not find header\n");
      fclose (fh);
      return -1;
    }

// rip

  for (image.remaining_sessions = image.sessions;
       image.remaining_sessions > 0; image.remaining_sessions--)
    {
      image.global_current_session++;

      cdi_get_tracks (fh, &image);
      image.header_position = ftell (fh);

      if (!image.tracks)
        {
//           printf ("Open session\n");
          cdi_skip_next_session (fh, &image);
          continue;
        }

      if (image.global_current_session == 1)
        {
          if (ask_type (fh, image.header_position) == 2)
            {
              if (opts.convert != ISO_FORMAT)
                opts.convert = BIN_FORMAT;
            }
        }

// Create cuesheet
      if (image.global_current_session == 1)
        sprintf (cuesheetname, STR_TDISC_CUE_FILENAME);
      else
        sprintf (cuesheetname, STR_TDISCN_CUE_FILENAME,
                 image.global_current_session);

      if (!(fcuesheet = fopen (cuesheetname, "wb")))
        {
          fprintf (stderr, "ERROR: could not create cue sheet\n");
//              fclose (fh);
//              return -1;
        }

// rip tracks from seesion

      for (image.remaining_tracks = image.tracks;
           image.remaining_tracks > 0; image.remaining_tracks--)
        {
          track.global_current_track++;
          track.number = image.tracks - image.remaining_tracks + 1;
          cdi_read_track (fh, &image, &track);
          image.header_position = ftell (fh);

         printf ("Session: %d/%d\n",
                  image.global_current_session, image.sessions);

          printf ("Track: %d/%d\n", track.global_current_track, image.tracks);
          printf ("Mode: %s/%d\n", track.mode == 0 ? "Audio" :
                  track.mode == 1 ? "Mode1" : "Mode2",
                  track.sector_size);

          if (opts.pregap)
            printf ("Pregap: %d\n", track.pregap_length);

          printf ("Size: %d (%d Bytes)\n"
                  "LBA: %d\n\n",
                  track.length, track.length * track.sector_size ,track.start_lba);

          if (track.pregap_length != 150)
            fprintf (stderr,
                     "WARNING: This track seems to have a non-standard pregap (%d Bytes)\n",
                     track.pregap_length);

          if (track.length < 0 && opts.pregap == FALSE)
            {
              fprintf (stderr, "ERROR: Negative track size found\n"
//                           "       You must extract image with /pregap option"
                );
              return -1;
            }

// Decidir si cortar
          if (!opts.fulldata && track.mode != 0
              && image.global_current_session == 1 && image.sessions > 1)
            flags.do_cut = 2;

          else if (!(track.mode != 0 && opts.fulldata))
            {
              flags.do_cut = (opts.cutall ? 2 : 0) +
                ((opts.cutfirst && track.global_current_track == 1) ? 2 : 0);
            }
          else
            flags.do_cut = 0;

// Decidir si convertir
          if (track.mode != 0 && track.sector_size != 2048)
            switch (opts.convert)
              {
              case BIN_FORMAT:
                flags.do_convert = FALSE;
                break;

              case ISO_FORMAT:
                flags.do_convert = TRUE;
                break;

              case DEFAULT_FORMAT:
              default:
                flags.do_convert =
                  (track.mode == 1 || image.global_current_session > 1) ?
                    TRUE : FALSE;
              }
          else
            flags.do_convert = FALSE;

          flags.save_as_iso =
            (track.sector_size == 2048 ||
             (track.mode != 0 && flags.do_convert)) ? TRUE : FALSE;

// Guardar la pista
          if (track.total_length < track.length + track.pregap_length)
            {
              fprintf (stderr, "SKIPPING: This track seems truncated\n");
              fseek (fh, track.position + track.total_length, SEEK_SET);
              track.position = ftell (fh);
            }
          else
            {
#if 1
              save_track (fh, &image, &track, &opts, &flags);
              track.position = ftell (fh);
              printf ("\n\n");
//TODO toc?
              if (!(track.mode == 2 && flags.do_convert))
                save_cue_sheet (fcuesheet, &image, &track, &opts, &flags);
#endif
            }
          fseek (fh, image.header_position, SEEK_SET);
        }
      fclose (fcuesheet);

      cdi_skip_next_session (fh, &image);
    }

  free (global_write_buffer_ptr);
  free (global_read_buffer_ptr);

  fclose (fh);
  return 0;
}


int
ucon64_cdi2nero (const char *image)
{
  return 0;
}


int
ucon64_isofix (const char *image)
/*
  ISO start LBA fixing routine
                                                    
  This tool will take an ISO image with PVD pointing 
  to bad DR offset and add padding data so actual DR 
  gets located in right absolute address.            
                                                    
  Original boot area, PVD, SVD and VDT are copied to 
  the start of new, fixed ISO image.                 
                                                    
  Supported input image formats are: 2048, 2336,     
  2352 and 2056 bytes per sector. All of them are    
  converted to 2048 bytes per sector when writing    
  excluding 2056 format which is needed by Mac users.
*/
{
  int sector_size = 2048, mode = 1;
  int image_length, remaining_length, last_pos, i;
  
  
  static time_t start_time = 0;

  char destfname[256];
  char string[256];
  char buffer[4096];

  int last_vd = FALSE;
  int extractbootonly = FALSE;
  int extractheaderonly = FALSE;
  int macformat = FALSE;
  int isoformat = FALSE;
  int start_lba = strtol (ucon64.file, NULL, 10); // !!!!!

  FILE *fsource, *fdest = NULL, *fboot = NULL, *fheader = NULL;

  if (!start_time) start_time = time (0);
  if (!start_lba) start_lba = 0; // ????

  strcpy (destfname, "fixed.iso");

  extractbootonly = TRUE; // boot
  extractheaderonly = TRUE; // header
  macformat = TRUE; // mac
  isoformat = TRUE; // iso

  strcpy (string, ucon64.rom);

  if (!(fsource = fopen (string, "rb")))
    return -1;

  fseek (fsource, 0L, SEEK_END);
  image_length = ftell (fsource);
  fseek (fsource, 0L, SEEK_SET);

// detect format

  fread (buffer, 1, 16, fsource);
  if (!memcmp (SYNC_DATA, buffer, 12))  // raw (2352)
    {
      sector_size = 2352;
      switch (buffer[15])
        {
        case 2:
          mode = 2;
          break;
        case 1:
          mode = 1;
          break;
        default:
          {
            printf ("Unsupported track mode (%d)", buffer[15]);
            return -1;
          }
        }
      if (seek_pvd2 (buffer, 2352, mode, fsource) == 0)
        {
          printf ("Could not find PVD!\n");
          return -1;
        }
    }
  else if (seek_pvd2 (buffer, 2048, 1, fsource))
    {
      sector_size = 2048;
      mode = 1;
    }
  else if (seek_pvd2 (buffer, 2336, 2, fsource))
    {
      sector_size = 2336;
      mode = 2;
    }
  else if (seek_pvd2 (buffer, 2056, 2, fsource))
    {
      sector_size = 2056;
      mode = 2;
      macformat = TRUE;
    }
  else
    {
      fprintf (stderr, "ERROR: Could not find PVD\n");
      return -1;
    }

  if (isoformat == TRUE)
    macformat = FALSE;

  printf ("sector size = %d, mode = %d\n", sector_size, mode);

// detect format end

  if (extractbootonly == FALSE && extractheaderonly == FALSE)
    {
  if (start_lba <= 0)
    {
      fprintf (stderr, "ERROR: Bad LBA value");
      return -1;
    }


      printf ("Creating destination file '%s'...\n", destfname);

      if (!(fdest = fopen (destfname, "wb")))
        return -1;
    }

  if (extractheaderonly == FALSE || (extractheaderonly == TRUE && extractbootonly == TRUE))
    {
      printf ("Saving boot area to file 'bootfile.bin'...\n");
      fboot = fopen ("bootfile.bin", "wb");
    }
  if (extractbootonly == FALSE || (extractheaderonly == TRUE && extractbootonly == TRUE))
    {
      printf ("Saving ISO header to file 'header.iso'...\n");
      fheader = fopen ("header.iso", "wb");
    }

// save boot area

  fseek (fsource, 0L, SEEK_SET);
  for (i = 0; i < 16; i++)
    {
      sector_read (buffer, sector_size, mode, fsource);
      if (extractbootonly == FALSE && extractheaderonly == FALSE)
        {
          if (macformat == TRUE)
            fwrite (SUB_HEADER, 8, 1, fdest);
          fwrite (buffer, 2048, 1, fdest);
        }
      if (extractheaderonly == FALSE
          || (extractheaderonly == TRUE && extractbootonly == TRUE))
        {
          if (macformat == TRUE)
            fwrite (SUB_HEADER, 8, 1, fboot);
          fwrite (buffer, 2048, 1, fboot);
        }
      if (extractbootonly == FALSE
          || (extractheaderonly == TRUE && extractbootonly == TRUE))
        {
          if (macformat == TRUE)
            fwrite (SUB_HEADER, 8, 1, fheader);
          fwrite (buffer, 2048, 1, fheader);
        }
    }

  if (extractheaderonly == FALSE || (extractheaderonly == TRUE && extractbootonly == TRUE))
    fclose (fboot);
  if (extractbootonly == TRUE && extractheaderonly == FALSE)
    return 0;                   // boot saved, exit

// seek & copy pvd etc.

  last_pos = ftell (fsource);   // start of pvd

  do
    {
      sector_read (buffer, sector_size, mode, fsource);

      if (!memcmp (PVD_STRING, buffer, 8))
        {
          printf ("Found PVD at sector %d\n", last_pos / sector_size);
        }
      else if (!memcmp (SVD_STRING, buffer, 8))
        {
          printf ("Found SVD at sector %d\n", last_pos / sector_size);
        }
      else if (!memcmp (VDT_STRING, buffer, 8))
        {
          printf ("Found VDT at sector %d\n", last_pos / sector_size);
          last_vd = TRUE;
        }
      else
        {
          fprintf (stderr, "ERROR: Found unknown Volume Descriptor");
          return -1;
        }

      if (extractbootonly == FALSE && extractheaderonly == FALSE)
        {
          if (macformat == TRUE)
            fwrite (SUB_HEADER, 8, 1, fdest);
          fwrite (buffer, 2048, 1, fdest);
        }

      if (macformat == TRUE)
        fwrite (SUB_HEADER, 8, 1, fheader);
      fwrite (buffer, 2048, 1, fheader);
      last_pos = ftell (fsource);
    }
  while (last_vd == FALSE);

// add padding data to header file

  memset (&buffer, 0, sizeof (buffer));

  remaining_length = 300 - (last_pos / sector_size);

  for (i = 0; i < remaining_length; i++)
    {
      if (macformat == TRUE)
        fwrite (SUB_HEADER, 8, 1, fheader);
      fwrite (buffer, 2048, 1, fheader);
    }

  fclose (fheader);

  if (extractheaderonly == TRUE)
    return 0; // header saved

// add padding data to iso image

  if (last_pos > start_lba * sector_size)
    {
      fprintf (stderr, "ERROR: LBA value is too small\n"
                       "       It should be at least %d for current ISO image (probably greater)",
               last_pos / sector_size);
      return -1;
    }

  if (start_lba < 11700)
    fprintf (stderr, "WARNING: LBA value should be greater or equal to 11700 for multisession\n"
             "         images\n");

  printf ("Adding padding data up to start LBA value...");

  remaining_length = start_lba - (last_pos / sector_size);

  for (i = 0; i < remaining_length; i++)
    {
      if (!(i % 512))
        ucon64_gauge (start_time, i, remaining_length);

      if (macformat == TRUE)
        fwrite (SUB_HEADER, 8, 1, fdest);

      if (!fwrite (buffer, 2048, 1, fdest))
        return -1;
    }

// append original iso image

  fseek (fsource, 0L, SEEK_SET);

  remaining_length = image_length / sector_size;

  for (i = 0; i < remaining_length; i++)
    {
      if (!(i % 512))
        ucon64_gauge (start_time, i, remaining_length);

      if (!sector_read (buffer, sector_size, mode, fsource))
        return -1;

      if (macformat == TRUE)
        fwrite (SUB_HEADER, 8, 1, fdest);

      if (!fwrite (buffer, 2048, 1, fdest))
        return -1;
    }

  fclose (fsource);
  fclose (fdest);

  return 0;
}


#if 0
int
ucon64_readiso (const char *image)
{
  return 0;
}
#endif
