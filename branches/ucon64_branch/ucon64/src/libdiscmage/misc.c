/*
misc.c - miscellaneous functions

written by 1999 - 2004 NoisyB (noisyb@gmx.net)
           2001 - 2004 dbjh
           2002 - 2003 Jan-Erik Karlsson (Amiga)


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
#include "config.h"                             // HAVE_ZLIB_H
#endif
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>                             // va_arg()
#include <sys/stat.h>                           // for S_IFLNK

#ifdef  __MSDOS__
#include <dos.h>                                // delay(), milliseconds
#elif   defined __unix__
#include <unistd.h>                             // usleep(), microseconds
#elif   defined __BEOS__
#include <OS.h>                                 // snooze(), microseconds
// Include OS.h before misc.h, because OS.h includes StorageDefs.h which
//  includes param.h which unconditionally defines MIN and MAX.
#elif   defined AMIGA
#include <unistd.h>
#include <fcntl.h>
#include <dos/dos.h>
#include <dos/var.h>
#include <libraries/lowlevel.h>                 // GetKey()
#include <proto/dos.h>
#include <proto/lowlevel.h>
#elif   defined _WIN32
#include <windows.h>                            // Sleep(), milliseconds
#endif

#ifdef  HAVE_ZLIB_H
#include "miscz.h"
#endif
#include "misc.h"

#ifdef  __CYGWIN__                              // under Cygwin (gcc for Windows) we
#define USE_POLL                                //  need poll() for kbhit(). poll()
#include <sys/poll.h>                           //  is available on Linux, not on
#endif                                          //  BeOS. DOS already has kbhit()

#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined __APPLE__                       // Mac OS X actually
#include <termios.h>
typedef struct termios tty_t;
#endif


#ifdef  DJGPP
#include <dpmi.h>                               // needed for __dpmi_int() by ansi_init()
#ifdef  DLL
#include "dxedll_priv.h"
#endif
#endif

extern int errno;

typedef struct st_func_node
{
  void (*func) (void);
  struct st_func_node *next;
} st_func_node_t;

static st_func_node_t func_list = { NULL, NULL };
static int func_list_locked = 0;
static int misc_ansi_color = 0;

#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined __APPLE__                       // Mac OS X actually
static void set_tty (tty_t *param);
#endif


#if 0                                           // currently not used
/*
  crc16 routine
*/
unsigned short
crc16 (unsigned short crc16, const void *buf, unsigned int size)
{
  static const short crc_table[] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
  };
  unsigned char *p = (unsigned char *) buf;

  while (size)
    {
      crc16 = ((crc16 >> 8) & 0xff) ^ crc_table[(crc16 ^ *p++) & 0xff];
      size--;
    }
  return crc16 /* & 0xffff */;
}
#endif


#ifndef HAVE_ZLIB_H
/*
  crc32 routines
*/
#define CRC32_POLYNOMIAL     0xedb88320

static unsigned int crc32_table[256];
static int crc32_table_built = 0;

void
build_crc32_table (void)
{
  unsigned int crc32, i, j;

  for (i = 0; i <= 255; i++)
    {
      crc32 = i;
      for (j = 8; j > 0; j--)
        {
          if (crc32 & 1)
            crc32 = (crc32 >> 1) ^ CRC32_POLYNOMIAL;
          else
            crc32 >>= 1;
        }
      crc32_table[i] = crc32;
    }
  crc32_table_built = 1;
}


unsigned int
crc32 (unsigned int crc32, const void *buffer, unsigned int size)
{
  unsigned char *p;

  if (!crc32_table_built)
    build_crc32_table ();

  crc32 ^= 0xffffffff;
  p = (unsigned char *) buffer;
  while (size-- != 0)
    crc32 = (crc32 >> 8) ^ crc32_table[(crc32 ^ *p++) & 0xff];
  return crc32 ^ 0xffffffff;
}


int
q_fsize (const char *filename)
{
  struct stat fstate;

  if (!stat (filename, &fstate))
    return fstate.st_size;

  errno = ENOENT;
  return -1;
}
#endif


#if     defined _WIN32 && defined ANSI_COLOR
int
vprintf2 (const char *format, va_list argptr)
// Cheap hack to get the Visual C++ and MinGW ports support "ANSI colors".
//  Cheap, because it only supports the ANSI escape sequences uCON64 uses.
{
#undef  printf
#undef  fprintf
  int n_chars = 0, n_ctrl = 0, n_print, done = 0;
  char output[MAXBUFSIZE], *ptr, *ptr2;
  HANDLE stdout_handle;
  CONSOLE_SCREEN_BUFFER_INFO info;
  WORD org_attr, new_attr = 0;

  n_chars = vsprintf (output, format, argptr);
  if (n_chars > MAXBUFSIZE)
    {
      fprintf (stderr, "INTERNAL ERROR: Output buffer in vprintf2() is too small (%d bytes).\n"
                       "                %d bytes were needed. Please send a bug report\n",
               MAXBUFSIZE, n_chars);
      exit (1);
    }

  if ((ptr = strchr (output, 0x1b)) == NULL)
    fputs (output, stdout);
  else
    {
      stdout_handle = GetStdHandle (STD_OUTPUT_HANDLE);
      GetConsoleScreenBufferInfo (stdout_handle, &info);
      org_attr = info.wAttributes;

      if (ptr > output)
        {
          *ptr = 0;
          fputs (output, stdout);
          *ptr = 0x1b;
        }
      while (!done)
        {
          if (memcmp (ptr, "\x1b[0m", 4) == 0)
            {
              new_attr = org_attr;
              n_ctrl = 4;
            }
          else if (memcmp (ptr, "\x1b[01;31m", 8) == 0)
            {
              new_attr = FOREGROUND_INTENSITY | FOREGROUND_RED;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[01;32m", 8) == 0)
            {
              new_attr = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[01;33m", 8) == 0) // bright yellow
            {
              new_attr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[31;41m", 8) == 0)
            {
              new_attr = FOREGROUND_RED | BACKGROUND_RED;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[32;42m", 8) == 0)
            {
              new_attr = FOREGROUND_GREEN | BACKGROUND_GREEN;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[30;41m", 8) == 0) // 30 = foreground black
            {
              new_attr = BACKGROUND_RED;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[30;42m", 8) == 0)
            {
              new_attr = BACKGROUND_GREEN;
              n_ctrl = 8;
            }
          else if (*ptr == 0x1b)
            {
              new_attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
              SetConsoleTextAttribute (stdout_handle, new_attr);
              printf ("\n"
                      "INTERNAL WARNING: vprintf2() encountered an unsupported ANSI escape sequence\n"
                      "                  Please send a bug report\n");
              n_ctrl = 0;
            }
          SetConsoleTextAttribute (stdout_handle, new_attr);

          ptr2 = strchr (ptr + 1, 0x1b);
          if (ptr2)
            n_print = ptr2 - ptr;
          else
            {
              n_print = strlen (ptr);
              done = 1;
            }

          ptr[n_print] = 0;
          ptr += n_ctrl;
          fputs (ptr, stdout);
          (ptr - n_ctrl)[n_print] = 0x1b;
          ptr = ptr2;
        }
    }
  return n_chars;
#define printf  printf2
#define fprintf fprintf2
}


int
printf2 (const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = vprintf2 (format, argptr);
  va_end (argptr);
  return n_chars;
}


int
fprintf2 (FILE *file, const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  if (file != stdout)
    n_chars = vfprintf (file, format, argptr);
  else
    n_chars = vprintf2 (format, argptr);
  va_end (argptr);
  return n_chars;
}
#endif // defined _WIN32 && defined ANSI_COLOR


int
ansi_init (void)
{
  int result = isatty (STDOUT_FILENO);

#ifdef  DJGPP
  if (result)
    {
      // Don't use __MSDOS__, because __dpmi_regs and __dpmi_int are DJGPP specific
      __dpmi_regs reg;

      reg.x.ax = 0x1a00;                        // DOS 4.0+ ANSI.SYS installation check
      __dpmi_int (0x2f, &reg);
      if (reg.h.al != 0xff)                     // AL == 0xff if installed
        result = 0;
    }
#endif

  misc_ansi_color = result;

  return result;
}


#if 0                                           // currently not used
char *
ansi_strip (char *str)
{
  int ansi = 0;
  char *p = str, *s = str;

  for (; *p; p++)
    switch (*p)
      {
        case '\x1b':                            // escape
          ansi = 1;
          break;

        case 'm':
          if (ansi)
            {
              ansi = 0;
              break;
            }

        default:
          if (!ansi)
            {
              *s = *p;
              s++;
            }
          break;
      }
  *s = 0;

  return str;
}
#endif


int
isfname (int c)
{
  if (isalnum (c))
    return TRUE;

  switch (c)
    {
      // characters that are also allowed in filenames
      case '.':
      case ',':
      case '\'':
      case '+':
      case '-':
      case ' ':
      case '(':
      case ')':
      case '[':
      case ']':
      case '!':
      case '&':
        return TRUE;
    }

  return FALSE;
}


int
isprint2 (int c)
{
  if (isprint (c))
    return TRUE;

  switch (c)
    {
      // characters that also work with printf
      case '\x1b':
        return (misc_ansi_color) ? TRUE : FALSE;

      case '\n':
        return TRUE;
    }

  return FALSE;
}


int
tofname (int c)
{
  return isfname (c) ? c : '_';
}


int
toprint2 (int c)
{
  return isprint2 (c) ? c : '.';
}


int
is_func (char *s, int size, int (*func) (int))
{
  char *p = s;

  /*
    Casting to unsigned char * is necessary to avoid differences between the
    different compilers' run-time environments. At least for isprint(). Without
    the cast the isprint() of (older versions of) DJGPP, MinGW, Cygwin and
    Visual C++ returns nonzero values for ASCII characters > 126.
  */
  for (; size >= 0; p++, size--)
    if (!func (*(unsigned char *) p))
      return FALSE;

  return TRUE;
}


char *
to_func (char *s, int size, int (*func) (int))
{
  char *p = s;

  for (; size > 0; p++, size--)
    *p = func (*p);

  return s;
}


char *
strcasestr2 (const char *str, const char *search)
{
  char *p = (char *) str;
  int len = strlen (search);

  if (!len)
    return p;

  for (; *p; p++)
    if (!strnicmp (p, search, len))
      return p;

  return NULL;
}


char *
set_suffix (char *filename, const char *suffix)
{
  char suffix2[FILENAME_MAX], *p, *p2;

  if (!(p = basename (filename)))
    p = filename;
  if ((p2 = strrchr (p, '.')))
    if (p2 != p)                                // files can start with '.'
      *p2 = 0;

  strcpy (suffix2, suffix);
  strcat (filename, is_func (p, strlen (p), isupper) ? strupr (suffix2) : strlwr (suffix2));

  return filename;
}


char *
set_suffix_i (char *filename, const char *suffix)
{
  char *p, *p2;

  if (!(p = basename (filename)))
    p = filename;
  if ((p2 = strrchr (p, '.')))
    if (p2 != p)                                // files can start with '.'
      *p2 = 0;

  strcat (filename, suffix);

  return filename;
}


const char *
get_suffix (const char *filename)
// Note that get_suffix() never returns NULL. Other code relies on that!
{
  char *p, *p2;

  if (!(p = basename (filename)))
    p = (char *) filename;
  if (!(p2 = strrchr (p, '.')))
    p2 = "";
  if (p2 == p)
    p2 = "";                                    // files can start with '.'; be
                                                //  consistent with set_suffix[_i]()
  return p2;
}


static int
strtrimr (char *str)
/*
  Removes all trailing blanks from a string.
  Blanks are defined with isspace (blank, tab, newline, return, formfeed,
  vertical tab = 0x09 - 0x0D + 0x20)
*/
{
  int i, j;

  j = i = strlen (str) - 1;

  while (isspace ((int) str[i]) && (i >= 0))
    str[i--] = 0;

  return j - i;
}


static int
strtriml (char *str)
/*
  Removes all leading blanks from a string.
  Blanks are defined with isspace (blank, tab, newline, return, formfeed,
  vertical tab = 0x09 - 0x0D + 0x20)
*/
{
  int i = 0, j;

  j = strlen (str) - 1;

  while (isspace ((int) str[i]) && (i <= j))
    i++;

  if (0 < i)
    strcpy (str, &str[i]);

  return i;
}


char *
strtrim (char *str)
/*
  Removes all leading and trailing blanks in a string.
  Blanks are defined with isspace (blank, tab, newline, return, formfeed,
  vertical tab = 0x09 - 0x0D + 0x20)
*/
{
  strtrimr (str);
  strtriml (str);

  return str;
}


int
memwcmp (const void *buffer, const void *search, uint32_t searchlen, int wildcard)
{
  uint32_t n;

  for (n = 0; n < searchlen; n++)
    if (((uint8_t *) search)[n] != wildcard &&
        ((uint8_t *) buffer)[n] != ((uint8_t *) search)[n])
      return -1;

  return 0;
}


void *
mem_search (const void *buffer, uint32_t buflen,
            const void *search, uint32_t searchlen)
{
  int32_t n;

  for (n = 0; n <= (int32_t) (buflen - searchlen); n++)
    if (memcmp ((uint8_t *) buffer + n, search, searchlen) == 0)
      return (uint8_t *) buffer + n;

  return 0;
}


void *
mem_swap_b (void *buffer, uint32_t n)
{
  uint8_t *a = (uint8_t *) buffer, byte;

  for (; n > 1; n -= 2)
    {
      byte = *a;
      *a = *(a + 1);
      *(a + 1) = byte;
      a += 2;
    }

  return buffer;
}


void *
mem_swap_w (void *buffer, uint32_t n)
{
  uint16_t *a = (uint16_t *) buffer, word;

  n >>= 1;                                      // # words = # bytes / 2
  for (; n > 1; n -= 2)
    {
      word = *a;
      *a = *(a + 1);
      *(a + 1) = word;
      a += 2;
    }

  return buffer;
}


#ifdef  DEBUG
static void
mem_hexdump_code (const void *buffer, uint32_t n, int virtual_start)
// hexdump something into C code (for development)
{
  uint32_t pos;
  const unsigned char *p = (const unsigned char *) buffer;

  for (pos = 0; pos < n; pos++, p++)
    {
      printf ("0x%02x, ", *p);

      if (!((pos + 1) & 7))
        fprintf (stdout, "// 0x%x (%d)\n", pos + virtual_start + 1, pos + virtual_start + 1);
    }
}
#endif


void
mem_hexdump (const void *buffer, uint32_t n, int virtual_start)
// hexdump something
{
#ifdef  DEBUG
  mem_hexdump_code (buffer, n, virtual_start);
#else
  uint32_t pos;
  char buf[17];
  const unsigned char *p = (const unsigned char *) buffer;

  buf[16] = 0;
  for (pos = 0; pos < n; pos++, p++)
    {
      if (!(pos & 15))
        printf ("%08x  ", (unsigned int) (pos + virtual_start));
      printf ((pos + 1) & 3 ? "%02x " : "%02x  ", *p);

      *(buf + (pos & 15)) = isprint (*p) ? *p : '.';
      if (!((pos + 1) & 15))
        puts (buf);
    }
  if (pos & 15)
    {
      *(buf + (pos & 15)) = 0;
      puts (buf);
    }
#endif
}


#if 0                                           // currently not used
int
mkdir2 (const char *name)
// create a directory and check its permissions
{
  struct stat *st = NULL;

  if (stat (name, st) == -1)
    {
      if (errno != ENOENT)
        {
          fprintf (stderr, "stat %s", name);
          return -1;
        }
      if (mkdir (name, 0700) == -1)
        {
          fprintf (stderr, "mkdir %s", name);
          return -1;
        }
      if (stat (name, st) == -1)
        {
          fprintf (stderr, "stat %s", name);
          return -1;
        }
    }

  if (!S_ISDIR (st->st_mode))
    {
      fprintf (stderr, "%s is not a directory\n", name);
      return -1;
    }
  if (st->st_uid != getuid ())
    {
      fprintf (stderr, "%s is not owned by you\n", name);
      return -1;
    }
  if (st->st_mode & 077)
    {
      fprintf (stderr, "%s must not be accessible by other users\n", name);
      return -1;
    }

  return 0;
}
#endif


char *
basename2 (const char *path)
// basename() clone (differs from Linux's basename())
{
  char *p1;
#if     defined DJGPP || defined __CYGWIN__
  char *p2;
#endif

  if (path == NULL)
    return NULL;

#if     defined DJGPP || defined __CYGWIN__
  // Yes, DJGPP, not __MSDOS__, because DJGPP's basename() behaves the same
  // Cygwin has no basename()
  p1 = strrchr (path, '/');
  p2 = strrchr (path, '\\');
  if (p2 > p1)                                  // use the last separator in path
    p1 = p2;
#else
  p1 = strrchr (path, FILE_SEPARATOR);
#endif
#if     defined DJGPP || defined __CYGWIN__ || defined _WIN32
  if (p1 == NULL)                               // no slash, perhaps a drive?
    p1 = strrchr (path, ':');
#endif

  return p1 ? p1 + 1 : (char *) path;
}


char *
dirname2 (const char *path)
// dirname() clone (differs from Linux's dirname())
{
  char *p1, *dir;
#if     defined DJGPP || defined __CYGWIN__
  char *p2;
#endif

  if (path == NULL)
    return NULL;
  // real dirname() uses malloc() so we do too
  // +2: +1 for string terminator +1 if path is "<drive>:"
  if ((dir = (char *) malloc (strlen (path) + 2)) == NULL)
    return NULL;

  strcpy (dir, path);
#if     defined DJGPP || defined __CYGWIN__
  // Yes, DJGPP, not __MSDOS__, because DJGPP's dirname() behaves the same
  // Cygwin has no dirname()
  p1 = strrchr (dir, '/');
  p2 = strrchr (dir, '\\');
  if (p2 > p1)                                  // use the last separator in path
    p1 = p2;
#else
  p1 = strrchr (dir, FILE_SEPARATOR);
#endif
#if     defined DJGPP || defined __CYGWIN__ || defined _WIN32
  if (p1 == NULL)                               // no slash, perhaps a drive?
    {
      if ((p1 = strrchr (dir, ':')))
        {
          p1[1] = '.';
          p1 += 2;
        }
    }
#endif

  while (p1 > dir &&                            // find first of last separators (we have to strip trailing ones)
#if     defined DJGPP || defined __CYGWIN__
         ((*(p1 - 1) == '/' && (*p1 == '/' || *p1 == '\\'))
          ||
          (*(p1 - 1) == '\\' && (*p1 == '\\' || *p1 == '/'))))
#else
         (*(p1 - 1) == FILE_SEPARATOR && *p1 == FILE_SEPARATOR))
#endif
    p1--;

  if (p1 == dir)
    p1++;                                       // don't overwrite single separator (root dir)
#if     defined DJGPP || defined __CYGWIN__ || defined _WIN32
  else if (p1 > dir)
    if (*(p1 - 1) == ':')
      p1++;                                     // we must not overwrite the last separator if
#endif                                          //  it was directly preceded by a drive letter

  if (p1)
    *p1 = 0;                                    // terminate string (overwrite the separator)
  else
    {
      dir[0] = '.';
      dir[1] = 0;
    }

  return dir;
}


#ifndef HAVE_REALPATH
#undef realpath
char *
realpath (const char *path, char *full_path)
{
#if     defined __unix__ || defined __BEOS__ || defined __MSDOS__
/*
  Keep the "defined _WIN32"'s in this code in case GetFullPathName() turns out
  to have some unexpected problems. This code works for Visual C++, but it
  doesn't return the same paths as GetFullPathName() does. Most notably,
  GetFullPathName() expands <drive letter>:. to the current directory of
  <drive letter>: while this code doesn't.
*/
#define MAX_READLINKS 32
  char copy_path[FILENAME_MAX], got_path[FILENAME_MAX], *new_path = got_path,
       *max_path;
#if     defined __MSDOS__ || defined _WIN32 || defined __CYGWIN__
  char c;
#endif
#ifdef  S_IFLNK
  char link_path[FILENAME_MAX];
  int readlinks = 0;
#endif
  int n;

  // Make a copy of the source path since we may need to modify it
  n = strlen (path);
  if (n >= FILENAME_MAX - 2)
    return NULL;
  else if (n == 0)
    return NULL;

  strcpy (copy_path, path);
  path = copy_path;
  max_path = copy_path + FILENAME_MAX - 2;
#if     defined __MSDOS__ || defined _WIN32 || defined __CYGWIN__
  c = toupper (*path);
  if (c >= 'A' && c <= 'Z' && path[1] == ':')
    {
      *new_path++ = *path++;
      *new_path++ = *path++;
      if (*path == FILE_SEPARATOR)
        *new_path++ = *path++;
    }
  else
#endif
  if (*path != FILE_SEPARATOR)
    {
      getcwd (new_path, FILENAME_MAX - 1);
#ifdef  DJGPP
      // DJGPP's getcwd() returns a path with forward slashes
      {
        int l = strlen (new_path);
        for (n = 0; n < l; n++)
          if (new_path[n] == '/')
            new_path[n] = FILE_SEPARATOR;
      }
#endif
      new_path += strlen (new_path);
      if (*(new_path - 1) != FILE_SEPARATOR)
        *new_path++ = FILE_SEPARATOR;
    }
  else
    {
      *new_path++ = FILE_SEPARATOR;
      path++;
    }

  // Expand each (back)slash-separated pathname component
  while (*path != 0)
    {
      // Ignore stray FILE_SEPARATOR
      if (*path == FILE_SEPARATOR)
        {
          path++;
          continue;
        }
      if (*path == '.')
        {
          // Ignore "."
          if (path[1] == 0 || path[1] == FILE_SEPARATOR)
            {
              path++;
              continue;
            }
          if (path[1] == '.')
            {
              if (path[2] == 0 || path[2] == FILE_SEPARATOR)
                {
                  path += 2;
                  // Ignore ".." at root
                  if (new_path == got_path + 1)
                    continue;
                  // Handle ".." by backing up
                  while (*((--new_path) - 1) != FILE_SEPARATOR)
                    ;
                  continue;
                }
            }
        }
      // Safely copy the next pathname component
      while (*path != 0 && *path != FILE_SEPARATOR)
        {
          if (path > max_path)
            return NULL;

          *new_path++ = *path++;
        }
#ifdef  S_IFLNK
      // Protect against infinite loops
      if (readlinks++ > MAX_READLINKS)
        return NULL;

      // See if latest pathname component is a symlink
      *new_path = 0;
      n = readlink (got_path, link_path, FILENAME_MAX - 1);
      if (n < 0)
        {
          // EINVAL means the file exists but isn't a symlink
          if (errno != EINVAL
#ifdef  __BEOS__
              // Make this function work for a mounted ext2 fs ("/:")
              && errno != B_NAME_TOO_LONG
#endif
             )
            {
              // Make sure it's null terminated
              *new_path = 0;
              strcpy (full_path, got_path);
              return NULL;
            }
        }
      else
        {
          // Note: readlink() doesn't add the null byte
          link_path[n] = 0;
          if (*link_path == FILE_SEPARATOR)
            // Start over for an absolute symlink
            new_path = got_path;
          else
            // Otherwise back up over this component
            while (*(--new_path) != FILE_SEPARATOR)
              ;
          if (strlen (path) + n >= FILENAME_MAX - 2)
            return NULL;
          // Insert symlink contents into path
          strcat (link_path, path);
          strcpy (copy_path, link_path);
          path = copy_path;
        }
#endif // S_IFLNK
      *new_path++ = FILE_SEPARATOR;
    }
  // Delete trailing slash but don't whomp a lone slash
  if (new_path != got_path + 1 && *(new_path - 1) == FILE_SEPARATOR)
    {
#if     defined __MSDOS__ || defined _WIN32 || defined __CYGWIN__
      if (new_path >= got_path + 3)
        {
          if (*(new_path - 2) == ':')
            {
              c = toupper (*(new_path - 3));
              if (!(c >= 'A' && c <= 'Z'))
                new_path--;
            }
          else
            new_path--;
        }
      else
        new_path--;
#else
      new_path--;
#endif
    }
  // Make sure it's null terminated
  *new_path = 0;
  strcpy (full_path, got_path);

  return full_path;
#elif   defined _WIN32
  char *p, c;
  int n;

  if (GetFullPathName (path, FILENAME_MAX, full_path, &p) == 0)
    return NULL;

  c = toupper (full_path[0]);
  n = strlen (full_path) - 1;
  // Remove trailing separator if full_path is not the root dir of a drive,
  //  because Visual C++'s runtime system is *really* stupid
  if (full_path[n] == FILE_SEPARATOR &&
      !(c >= 'A' && c <= 'Z' && full_path[1] == ':' && full_path[3] == 0)) // && full_path[2] == FILE_SEPARATOR
    full_path[n] = 0;

  return full_path;
#elif   defined AMIGA
  strcpy (full_path, path);
  return full_path;
#endif
}
#endif


char *
realpath2 (const char *path, char *full_path)
// enhanced realpath() which returns the absolute path of a file
{
  char path1[FILENAME_MAX];
  const char *path2;

  if (path[0] == '~')
    {
      if (path[1] == FILE_SEPARATOR
#ifdef  __CYGWIN__
          || path[1] == '\\'
#endif
         )
        sprintf (path1, "%s"FILE_SEPARATOR_S"%s", getenv2 ("HOME"), &path[2]);
      else if (path[1] == 0)
        strcpy (path1, getenv2 ("HOME"));
      path2 = path1;
    }
  else
    path2 = path;

  return realpath (path2, full_path);
}


int
one_file (const char *filename1, const char *filename2)
// returns 1 if filename1 and filename2 refer to one file, 0 if not (or error)
{
#ifndef _WIN32
  struct stat finfo1, finfo2;

  /*
    Not the name, but the combination inode & device identify a file.
    Note that stat() doesn't need any access rights except search rights for
    the directories in the path to the file.
  */
  if (stat (filename1, &finfo1) != 0)
    return 0;
  if (stat (filename2, &finfo2) != 0)
    return 0;
  if (finfo1.st_dev == finfo2.st_dev && finfo1.st_ino == finfo2.st_ino)
    return 1;
  else
    return 0;
#else
  HANDLE file1, file2;
  BY_HANDLE_FILE_INFORMATION finfo1, finfo2;

  file1 = CreateFile (filename1, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file1 == INVALID_HANDLE_VALUE)
    return 0;
  file2 = CreateFile (filename2, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file2 == INVALID_HANDLE_VALUE)
    {
      CloseHandle (file1);
      return 0;
    }
  GetFileInformationByHandle (file1, &finfo1);
  GetFileInformationByHandle (file2, &finfo2);
  CloseHandle (file1);
  CloseHandle (file2);
  if (finfo1.dwVolumeSerialNumber == finfo2.dwVolumeSerialNumber &&
      (finfo1.nFileIndexHigh << 16 | finfo1.nFileIndexLow) ==
      (finfo2.nFileIndexHigh << 16 | finfo2.nFileIndexLow))
    return 1;
  else
    return 0;
#endif
}


int
one_filesystem (const char *filename1, const char *filename2)
// returns 1 if filename1 and filename2 reside on one file system, 0 if not
//  (or an error occurred)
{
#ifndef _WIN32
  struct stat finfo1, finfo2;

  if (stat (filename1, &finfo1) != 0)
    return 0;
  if (stat (filename2, &finfo2) != 0)
    return 0;
  if (finfo1.st_dev == finfo2.st_dev)
    return 1;
  else
    return 0;
#else
  HANDLE file1, file2;
  BY_HANDLE_FILE_INFORMATION finfo1, finfo2;

  file1 = CreateFile (filename1, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file1 == INVALID_HANDLE_VALUE)
    return 0;
  file2 = CreateFile (filename2, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file2 == INVALID_HANDLE_VALUE)
    {
      CloseHandle (file1);
      return 0;
    }
  GetFileInformationByHandle (file1, &finfo1);
  GetFileInformationByHandle (file2, &finfo2);
  CloseHandle (file1);
  CloseHandle (file2);
  if (finfo1.dwVolumeSerialNumber == finfo2.dwVolumeSerialNumber)
    return 1;
  else
    return 0;
#endif
}


int
rename2 (const char *oldname, const char *newname)
{
  int retval;
  char *dir1 = dirname2 (oldname), *dir2 = dirname2 (newname);

  // We should use dirname{2}() in case oldname or newname doesn't exist yet
  if (one_filesystem (dir1, dir2))
    retval = rename (oldname, newname);
  else
    {
      // don't remove unless the file can be copied
      retval = q_rfcpy (oldname, newname);
      if (retval == 0)
        remove (oldname);
    }

  free (dir1);
  free (dir2);
  return retval;
}


int
truncate2 (const char *filename, int new_size)
{
  int size = q_fsize (filename);
  struct stat fstate;

  stat (filename, &fstate);
  if (chmod (filename, fstate.st_mode | S_IWUSR))
    return -1;

  if (size < new_size)
    {
      FILE *file;
      unsigned char padbuffer[MAXBUFSIZE];
      int n_bytes;

      if ((file = fopen (filename, "ab")) == NULL)
        return -1;

      memset (padbuffer, 0, MAXBUFSIZE);

      while (size < new_size)
        {
          n_bytes = new_size - size > MAXBUFSIZE ? MAXBUFSIZE : new_size - size;
          fwrite (padbuffer, 1, n_bytes, file);
          size += n_bytes;
        }

      fclose (file);
    }
  else
    truncate (filename, new_size);

  return 0;                                     // success
}


int
change_mem (char *buf, int bufsize, char *searchstr, int strsize,
            char wc, char esc, char *newstr, int newsize, int offset, ...)
// convenience wrapper for change_mem2()
{
  va_list argptr;
  int i, n_esc = 0, retval;
  st_cm_set_t *sets;

  va_start (argptr, offset);
  for (i = 0; i < strsize; i++)
    if (searchstr[i] == esc)
      n_esc++;

  sets = (st_cm_set_t *) malloc (n_esc * sizeof (st_cm_set_t));
  va_start (argptr, offset);
  for (i = 0; i < n_esc; i++)
    {
      sets[i].data = va_arg (argptr, char *);   // get next set of characters
      sets[i].size = va_arg (argptr, int);      // get set size
    }
  va_end (argptr);
  retval = change_mem2 (buf, bufsize, searchstr, strsize, wc, esc, newstr,
                        newsize, offset, sets);
  free (sets);
  return retval;
}


int
change_mem2 (char *buf, int bufsize, char *searchstr, int strsize, char wc,
             char esc, char *newstr, int newsize, int offset, st_cm_set_t *sets)
/*
  Search for all occurrences of string searchstr in buf and replace newsize
  bytes in buf by copying string newstr to the end of the found search string
  in buf plus offset.
  If searchstr contains wildcard characters wc, then n wildcard characters in
  searchstr match any n characters in buf.
  If searchstr contains escape characters esc, sets must point to an array of
  sets. sets must contain as many elements as there are escape characters in
  searchstr. searchstr matches for an escape character if one of the characters
  in sets[i]->data matches.
  Note that searchstr is not necessarily a C string; it may contain one or more
  zero bytes as strsize indicates the length.
  offset is the relative offset from the last character in searchstring and may
  have a negative value.
  The return value is the number of times a match was found.
  This function was written to patch SNES ROM dumps. It does basically the same
  as the old uCON does, with one exception, the line with:
    bufpos -= n_wc;

  As stated in the comment, this causes the search to restart at the first
  wildcard character of the sequence of wildcards that was most recently
  skipped if the current character in buf didn't match the current character
  in searchstr. This makes change_mem() behave a bit more intuitive. For
  example
    char str[] = "f foobar means...";
    change_mem (str, strlen (str), "f**bar", 6, '*', '!', "XXXXXXXX", 8, 2, NULL);
  finds and changes "foobar means..." into "foobar XXXXXXXX", while with uCON's
  algorithm it would not (but does the job good enough for patching SNES ROMs).

  One example of using sets:
    char str[] = "fu-bar     is the same as foobar    ";
    st_cm_set_t sets[] = {{"o-", 2}, {"uo", 2}};
    change_mem (str, strlen (str), "f!!", 3, '*', '!', "fighter", 7, 1, sets);
  This changes str into "fu-fighter is the same as foofighter".
*/
{
  char *set;
  int bufpos, strpos = 0, pos_1st_esc = -1, setsize, i, n_wc, n_matches = 0,
      setindex = 0;

  for (bufpos = 0; bufpos < bufsize; bufpos++)
    {
      if (strpos == 0 && searchstr[0] != esc && searchstr[0] != wc)
        while (bufpos < bufsize && searchstr[0] != buf[bufpos])
          bufpos++;

      // handle escape character in searchstr
      while (searchstr[strpos] == esc && bufpos < bufsize)
        {
          if (strpos == pos_1st_esc)
            setindex = 0;                       // reset argument pointer
          if (pos_1st_esc == -1)
            pos_1st_esc = strpos;

          set = sets[setindex].data;            // get next set of characters
          setsize = sets[setindex].size;        // get set size
          setindex++;
          i = 0;
          // see if buf[bufpos] matches with any character in current set
          while (i < setsize && buf[bufpos] != set[i])
            i++;
          if (i == setsize)
            break;                              // buf[bufpos] didn't match with any char

          if (strpos == strsize - 1)            // check if we are at the end of searchstr
            {
              memcpy (buf + bufpos + offset, newstr, newsize);
              n_matches++;
              break;
            }

          strpos++;
          bufpos++;
        }
      if (searchstr[strpos] == esc)
        {
          strpos = 0;
          continue;
        }

      // skip wildcards in searchstr
      n_wc = 0;
      while (searchstr[strpos] == wc && bufpos < bufsize)
        {
          if (strpos == strsize - 1)            // check if at end of searchstr
            {
              memcpy (buf + bufpos + offset, newstr, newsize);
              n_matches++;
              break;
            }

          strpos++;
          bufpos++;
          n_wc++;
        }
      if (bufpos == bufsize)
        break;
      if (searchstr[strpos] == wc)
        {
          strpos = 0;
          continue;
        }

      if (searchstr[strpos] == esc)
        {
          bufpos--;                             // current char has to be checked, but `for'
          continue;                             //  increments bufpos
        }

      // no escape char, no wildcard -> normal character
      if (searchstr[strpos] == buf[bufpos])
        {
          if (strpos == strsize - 1)            // check if at end of searchstr
            {
              memcpy (buf + bufpos + offset, newstr, newsize);
              n_matches++;
              strpos = 0;
            }
          else
            strpos++;
        }
      else
        {
          bufpos -= n_wc;                       // scan the most recent wildcards too if
          if (strpos > 0)                       //  the character didn't match
            {
              bufpos--;                         // current char has to be checked, but `for'
              strpos = 0;                       //  increments bufpos
            }
        }
    }

  return n_matches;
}


#if     defined UCON64 && !defined DLL
// I think this function belongs here, because it is related to change_mem()
// Be a man and accept that the world is not perfect.
#include "ucon64_misc.h"

int
build_cm_patterns (st_cm_pattern_t **patterns, const char *filename, char *fullfilename)
/*
  This function goes a bit over the top what memory allocation technique
  concerns, but at least it's stable.
  Note the important difference between (*patterns)[0].n_sets and
  patterns[0]->n_sets (not especially that member). I (dbjh) am too ashamed too
  tell how long it took me to finally realise that...
*/
{
  char src_name[FILENAME_MAX], line[MAXBUFSIZE], buffer[MAXBUFSIZE],
       *token, *last, *ptr;
  int line_num = 0, n_sets, n_codes = 0, n, currentsize1, requiredsize1,
      currentsize2, requiredsize2, currentsize3, requiredsize3;
  FILE *srcfile;

  realpath2 (filename, src_name);
  // First try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    sprintf (src_name, "%s" FILE_SEPARATOR_S "%s", ucon64.configdir, filename);
  strcpy (fullfilename, src_name);
  if (access (src_name, F_OK | R_OK))
    return -1;                                  // NOT an error, it's optional

  if ((srcfile = fopen (src_name, "r")) == NULL) // open in text mode
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }

  *patterns = NULL;
  currentsize1 = requiredsize1 = 0;
  while (fgets (line, sizeof line, srcfile) != NULL)
    {
      line_num++;
      n_sets = 0;

      ptr = line + strspn (line, "\t ");
      if (*ptr == '#' || *ptr == '\n' || *ptr == '\r')
        continue;
      if ((ptr = strpbrk (line, "\n\r#")))      // text after # is comment
        *ptr = 0;

      requiredsize1 += sizeof (st_cm_pattern_t);
      if (requiredsize1 > currentsize1)
        {
          currentsize1 = requiredsize1 + 10 * sizeof (st_cm_pattern_t);
          if (!(*patterns = (st_cm_pattern_t *) realloc (*patterns, currentsize1)))
            {
              fprintf (stderr, ucon64_msg[BUFFER_ERROR], currentsize1);
              return -1;
            }
        }

      (*patterns)[n_codes].search = NULL;
      currentsize2 = 0;
      requiredsize2 = 1;                        // for string terminator
      n = 0;
      strcpy (buffer, line);
      token = strtok (buffer, ":");
      token = strtok (token, " ");
//      printf ("token: \"%s\"\n", token);
      last = token;
      // token is never NULL here (yes, tested with empty files and such)
      do
        {
          requiredsize2++;
          if (requiredsize2 > currentsize2)
            {
              currentsize2 = requiredsize2 + 10;
              if (!((*patterns)[n_codes].search =
                   (char *) realloc ((*patterns)[n_codes].search, currentsize2)))
                {
                  fprintf (stderr, ucon64_msg[BUFFER_ERROR], currentsize2);
                  free (*patterns);
                  *patterns = NULL;
                  return -1;
                }
            }
          (*patterns)[n_codes].search[n] = (unsigned char) strtol (token, NULL, 16);
          n++;
        }
      while ((token = strtok (NULL, " ")));
      (*patterns)[n_codes].search_size = n;     // size in bytes

      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %d is invalid, no wildcard value is specified\n",
                  line_num);
          continue;
        }
      (*patterns)[n_codes].wildcard = (char) strtol (token, NULL, 16);

      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %d is invalid, no escape value is specified\n",
                  line_num);
          continue;
        }
      (*patterns)[n_codes].escape = (char) strtol (token, NULL, 16);

      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %d is invalid, no replacement is specified\n", line_num);
          continue;
        }
      (*patterns)[n_codes].replace = NULL;
      currentsize2 = 0;
      requiredsize2 = 1;                        // for string terminator
      n = 0;
      do
        {
          requiredsize2++;
          if (requiredsize2 > currentsize2)
            {
              currentsize2 = requiredsize2 + 10;
              if (!((*patterns)[n_codes].replace =
                   (char *) realloc ((*patterns)[n_codes].replace, currentsize2)))
                {
                  fprintf (stderr, ucon64_msg[BUFFER_ERROR], currentsize2);
                  free ((*patterns)[n_codes].search);
                  free (*patterns);
                  *patterns = NULL;
                  return -1;
                }
            }
          (*patterns)[n_codes].replace[n] = (unsigned char) strtol (token, NULL, 16);
          n++;
        }
      while ((token = strtok (NULL, " ")));
      (*patterns)[n_codes].replace_size = n;  // size in bytes

      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %d is invalid, no offset is specified\n", line_num);
          continue;
        }
      (*patterns)[n_codes].offset = strtol (token, NULL, 10); // yes, offset is decimal

      if (ucon64.quiet == -1)
        {
          printf ("\n"
                  "line:         %d\n"
                  "searchstring: ",
                  line_num);
          for (n = 0; n < (*patterns)[n_codes].search_size; n++)
            printf ("%02x ", (unsigned char) (*patterns)[n_codes].search[n]);
          printf ("(%d)\n"
                  "wildcard:     %02x\n"
                  "escape:       %02x\n"
                  "replacement:  ",
                  (*patterns)[n_codes].search_size,
                  (unsigned char) (*patterns)[n_codes].wildcard,
                  (unsigned char) (*patterns)[n_codes].escape);
          for (n = 0; n < (*patterns)[n_codes].replace_size; n++)
            printf ("%02x ", (unsigned char) (*patterns)[n_codes].replace[n]);
          printf ("(%d)\n"
                  "offset:       %d\n",
                  (*patterns)[n_codes].replace_size,
                  (*patterns)[n_codes].offset);
        }

      (*patterns)[n_codes].sets = NULL;
      currentsize2 = 0;
      requiredsize2 = 1;                        // for string terminator
      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      last = token;
      while (token)
        {
          requiredsize2 += sizeof (st_cm_set_t);
          if (requiredsize2 > currentsize2)
            {
              currentsize2 = requiredsize2 + 10 * sizeof (st_cm_set_t);
              if (!((*patterns)[n_codes].sets = (st_cm_set_t *)
                    realloc ((*patterns)[n_codes].sets, currentsize2)))
                {
                  fprintf (stderr, ucon64_msg[BUFFER_ERROR], currentsize2);
                  free ((*patterns)[n_codes].replace);
                  free ((*patterns)[n_codes].search);
                  free (*patterns);
                  *patterns = NULL;
                  return -1;
                }
            }

          (*patterns)[n_codes].sets[n_sets].data = NULL;
          currentsize3 = 0;
          requiredsize3 = 1;                    // for string terminator
          n = 0;
          token = strtok (token, " ");
          do
            {
              requiredsize3++;
              if (requiredsize3 > currentsize3)
                {
                  currentsize3 = requiredsize3 + 10;
                  if (!((*patterns)[n_codes].sets[n_sets].data = (char *)
                        realloc ((*patterns)[n_codes].sets[n_sets].data, currentsize3)))
                    {
                      fprintf (stderr, ucon64_msg[BUFFER_ERROR], currentsize3);
                      free ((*patterns)[n_codes].sets);
                      free ((*patterns)[n_codes].replace);
                      free ((*patterns)[n_codes].search);
                      free (*patterns);
                      *patterns = NULL;
                      return -1;
                    }
                }
              (*patterns)[n_codes].sets[n_sets].data[n] =
                (unsigned char) strtol (token, NULL, 16);
              n++;
            }
          while ((token = strtok (NULL, " ")));
          (*patterns)[n_codes].sets[n_sets].size = n;

          if (ucon64.quiet == -1)
            {
              printf ("set:          ");
              for (n = 0; n < (*patterns)[n_codes].sets[n_sets].size; n++)
                printf ("%02x ", (unsigned char) (*patterns)[n_codes].sets[n_sets].data[n]);
              printf ("(%d)\n", (*patterns)[n_codes].sets[n_sets].size);
            }

          strcpy (buffer, line);
          token = strtok (last, ":");
          token = strtok (NULL, ":");
          last = token;

          n_sets++;
        }
      (*patterns)[n_codes].n_sets = n_sets;

      n_codes++;
    }
  fclose (srcfile);
  return n_codes;
}
#endif


void
cleanup_cm_patterns (st_cm_pattern_t **patterns, int n_patterns)
{
  int n, m;
  for (n = 0; n < n_patterns; n++)
    {
      free ((*patterns)[n].search);
      (*patterns)[n].search = NULL;
      free ((*patterns)[n].replace);
      (*patterns)[n].replace = NULL;
      for (m = 0; m < (*patterns)[n].n_sets; m++)
        {
          free ((*patterns)[n].sets[m].data);
          (*patterns)[n].sets[m].data = NULL;
        }
      free ((*patterns)[n].sets);
      (*patterns)[n].sets = NULL;
    }
  free (*patterns);
  *patterns = NULL;
}


int
gauge (time_t init_time, int pos, int size)
{
#define GAUGE_LENGTH ((int64_t) 24)

  int curr, bps, left, p, percentage;
  char progress[MAXBUFSIZE];

  if (pos > size)
    return -1;

  if ((curr = time (0) - init_time) == 0)
    curr = 1;                                   // `round up' to at least 1 sec (no division
                                                //  by zero below)
  bps = pos / curr;                             // # bytes/second (average transfer speed)
  left = size - pos;
  left /= bps ? bps : 1;

  p = (int) ((GAUGE_LENGTH * pos) / size);
  *progress = 0;
  strncat (progress, "========================", p);

  if (misc_ansi_color)
    {
      progress[p] = 0;
      if (p < GAUGE_LENGTH)
        strcat (progress, "\x1b[31;41m");
    }

  strncat (&progress[p], "------------------------", (int) (GAUGE_LENGTH - p));

  percentage = (int) ((((int64_t) 100) * pos) / size);

  printf (
    misc_ansi_color ? "\r%10d Bytes [\x1b[32;42m%s\x1b[0m] %d%%, BPS=%d, " :
      "\r%10d Bytes [%s] %d%%, BPS=%d, ", pos, progress, percentage, bps);

  if (pos == size)
    printf ("TOTAL=%03d:%02d", curr / 60, curr % 60); // DON'T print a newline
  else                                                //  -> gauge can be cleared
    printf ("ETA=%03d:%02d   ", left / 60, left % 60);

  fflush (stdout);

  return 0;
}


#ifdef  __CYGWIN__
/*
  Weird problem with combination Cygwin uCON64 exe and cmd.exe (Bash is ok):
  When a string with "e (e with diaeresis, one character) is read from an
  environment variable, the character isn't the right character for accessing
  the file system. We fix this.
  TODO: fix the same problem for other non-ASCII characters (> 127).
*/
char *
fix_character_set (char *str)
{
  int n, l = strlen (str);
  unsigned char *ptr = (unsigned char *) str;

  for (n = 0; n < l; n++)
    {
      if (ptr[n] == 0x89)                       // e diaeresis
        ptr[n] = 0xeb;
      else if (ptr[n] == 0x84)                  // a diaeresis
        ptr[n] = 0xe4;
      else if (ptr[n] == 0x8b)                  // i diaeresis
        ptr[n] = 0xef;
      else if (ptr[n] == 0x94)                  // o diaeresis
        ptr[n] = 0xf6;
      else if (ptr[n] == 0x81)                  // u diaeresis
        ptr[n] = 0xfc;
    }

  return str;
}
#endif


/*
  getenv() suitable for enviroments w/o HOME, TMP or TEMP variables.
  The caller should copy the returned string to it's own memory, because this
  function will overwrite that memory on the next call.
  Note that this function never returns NULL.
*/
char *
getenv2 (const char *variable)
{
  char *tmp;
  static char value[MAXBUFSIZE];
#if     defined __CYGWIN__ || defined __MSDOS__
/*
  Under DOS and Windows the environment variables are not stored in a case
  sensitive manner. The runtime systems of DJGPP and Cygwin act as if they are
  stored in upper case. Their getenv() however *is* case sensitive. We fix this
  by changing all characters of the search string (variable) to upper case.

  Note that under Cygwin's Bash environment variables *are* stored in a case
  sensitive manner.
*/
  char tmp2[MAXBUFSIZE];

  strcpy (tmp2, variable);
  variable = strupr (tmp2);                     // DON'T copy the string into variable
#endif                                          //  (variable itself is local)

  *value = 0;

  if ((tmp = getenv (variable)) != NULL)
    strcpy (value, tmp);
  else
    {
      if (!strcmp (variable, "HOME"))
        {
          if ((tmp = getenv ("USERPROFILE")) != NULL)
            strcpy (value, tmp);
          else if ((tmp = getenv ("HOMEDRIVE")) != NULL)
            {
              strcpy (value, tmp);
              tmp = getenv ("HOMEPATH");
              strcat (value, tmp ? tmp : FILE_SEPARATOR_S);
            }
          else
            /*
              Don't just use C:\\ under DOS, the user might not have write access
              there (Windows NT DOS-box). Besides, it would make uCON64 behave
              differently on DOS than on the other platforms.
              Returning the current directory when none of the above environment
              variables are set can be seen as a feature. A frontend could execute
              uCON64 with an environment without any of the environment variables
              set, so that the directory from where uCON64 starts will be used.
            */
            {
              char c;
              getcwd (value, FILENAME_MAX);
              c = toupper (*value);
              // if current dir is root dir strip problematic ending slash (DJGPP)
              if (c >= 'A' && c <= 'Z' &&
                  value[1] == ':' && value[2] == '/' && value[3] == 0)
                value[2] = 0;
            }
         }

      if (!strcmp (variable, "TEMP") || !strcmp (variable, "TMP"))
        {
#if     defined __MSDOS__ || defined __CYGWIN__
          /*
            DJGPP and (yet another) Cygwin quirck
            A trailing backslash is used to check for a directory. Normally
            DJGPP's runtime system is able to handle forward slashes in paths,
            but access() won't differentiate between files and dirs if a
            forward slash is used. Cygwin's runtime system seems to handle
            paths with forward slashes quite different from paths with
            backslashes. This trick seems to work only if a backslash is used.
          */
          if (access ("\\tmp\\", R_OK | W_OK) == 0)
#else
          // trailing file separator to force it to be a directory
          if (access (FILE_SEPARATOR_S"tmp"FILE_SEPARATOR_S, R_OK | W_OK) == 0)
#endif
            strcpy (value, FILE_SEPARATOR_S"tmp");
          else
            getcwd (value, FILENAME_MAX);
        }
    }

#ifdef  __CYGWIN__
  /*
    Under certain circumstances Cygwin's runtime system returns "/" as value of
    HOME while that var has not been set. To specify a root dir a path like
    /cygdrive/<drive letter> or simply a drive letter should be used.
  */
  if (!strcmp (variable, "HOME") && !strcmp (value, "/"))
    getcwd (value, FILENAME_MAX);

  return fix_character_set (value);
#else
  return value;
#endif
}


static const char *
get_property2 (const char *filename, const char *propname, char divider, char *buffer, const char *def)
// divider is the 1st char after propname ('=', ':', etc..)
{
  char line[MAXBUFSIZE], *p = NULL;
  FILE *fh;
  int prop_found = 0, i;

  if ((fh = fopen (filename, "r")) != 0)        // opening the file in text mode
    {                                           //  avoids trouble under DOS
      while (fgets (line, sizeof line, fh) != NULL)
        {
          p = line + strspn (line, "\t ");
          if (*p == '#' || *p == '\n' || *p == '\r')
            continue;                           // text after # is comment
          if ((p = strpbrk (line, "\n\r#")))    // strip *any* returns
            *p = 0;

          if (!strnicmp (line, propname, strlen (propname)))
            {
              p = strchr (line, divider);
              if (p)                    // if no divider was found the propname must be
                {                       //  a bool config entry (present or not present)
                  p++;
                  strcpy (buffer, p + strspn (p, "\t "));
                  // strip trailing whitespace
                  for (i = strlen (buffer) - 1;
                       i >= 0 && (buffer[i] == '\t' || buffer[i] == ' ');
                       i--)
                    buffer[i] = 0;

                }
              prop_found = 1;
              break;                            // an environment variable
            }                                   //  might override this
        }
      fclose (fh);
    }

  p = getenv2 (propname);
  if (*p == 0)                                  // getenv2() never returns NULL
    {
      if (!prop_found)
        {
          if (def)
            strcpy (buffer, def);
          else
            buffer = NULL;                      // buffer won't be changed
        }                                       //  after this func (=ok)
    }
  else
    strcpy (buffer, p);
  return buffer;
}


const char *
get_property (const char *filename, const char *propname, char *buffer, const char *def)
{
  return get_property2 (filename, propname, '=', buffer, def);
}


int32_t
get_property_int (const char *filename, const char *propname, char divider)
{
  char buf[MAXBUFSIZE];
  int32_t value = 0;

  get_property2 (filename, propname, divider, buf, NULL);

  if (buf[0])
    switch (tolower (buf[0]))
      {
        case '0': // 0
        case 'n': // [Nn]o
          return 0;
      }

  value = strtol (buf, NULL, 10);
  return value ? value : 1;                     // if buf was only text like 'Yes'
}                                               //  we'll return at least 1


static int
set_property2 (const char *filename, const char *propname, char divider, const char *value)
{
  int found = 0, result = 0, file_size = 0;
  char buf[MAXBUFSIZE], *buf2;
  FILE *fh;
  struct stat fstate;

  if (stat (filename, &fstate) != 0)
    file_size = fstate.st_size;

  if (!(buf2 = (char *) malloc ((file_size + MAXBUFSIZE) * sizeof (char))))
    {
      errno = ENOMEM;
      return -1;
    }

  *buf2 = 0;

  if ((fh = fopen (filename, "r")) != 0)        // opening the file in text mode
    {                                           //  avoids trouble under DOS
      while (fgets (buf, sizeof buf, fh) != NULL)
        {
          if (!strnicmp (buf, propname, strlen (propname)))
            {
              found = 1;
              if (value == NULL)
                continue;

              sprintf (buf, "%s%c%s\n", propname, divider, value);
            }
          strcat (buf2, buf);
        }
      fclose (fh);
    }

  if (!found && value != NULL)
    {
      sprintf (buf, "%s%c%s\n", propname, divider, value);
      strcat (buf2, buf);
    }

  if ((fh = fopen (filename, "w")) == NULL)     // open in text mode
    return -1;

  result = fwrite (buf2, 1, strlen (buf2), fh);
  fclose (fh);

//  q_fwrite (buf2, 0, strlen (buf2), filename, "wb");

  return result;
}


int
set_property (const char *filename, const char *propname, const char *value)
{
  return set_property2 (filename, propname, '=', value);
}


int
rmdir2 (const char *path)
{
#if 0
  char cwd[FILENAME_MAX];
  struct dirent *ep;
  struct stat fstate;
  DIR *dp;

  if (!(dp = opendir (path)))
    return -1;

  getcwd (cwd, FILENAME_MAX);
  chdir (path);

  while ((ep = readdir (dp)) != NULL)
    {
      if (stat (ep->d_name, &fstate) == -1)
        return -1;

      if (S_ISDIR (fstate.st_mode))
        {
          if (strcmp (ep->d_name, "..") != 0 && strcmp (ep->d_name, ".") != 0)
            rmdir2 (ep->d_name);
        }
      else
        remove (ep->d_name);
    }

  closedir (dp);
  chdir (cwd);

#endif
  return rmdir (path);
}


char *
tmpnam2 (char *temp)
// tmpnam() clone
{
  char *p = getenv2 ("TEMP");
  static time_t init = 0;

  if (!init)
    {
      init = time (0);
      srand (init);
    }

  *temp = 0;
  while (!(*temp) || !access (temp, F_OK))      // must work for files AND dirs
    sprintf (temp, "%s%s%08x.tmp", p, FILE_SEPARATOR_S, rand());

  return temp;
}


#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined __APPLE__                       // Mac OS X actually
static int oldtty_set = 0, stdin_tty = 1;       // 1 => stdin is a tty, 0 => it's not
static tty_t oldtty, newtty;


void
set_tty (tty_t *param)
{
  if (stdin_tty && tcsetattr (STDIN_FILENO, TCSANOW, param) == -1)
    {
      fprintf (stderr, "ERROR: Could not set tty parameters\n");
      exit (100);
    }
}


/*
  This code compiles with DJGPP, but is not neccesary. Our kbhit() conflicts
  with DJGPP's one, so it won't be used for that function. Perhaps it works
  for making getchar() behave like getch(), but that's a bit pointless.
*/
void
init_conio (void)
{
  if (!isatty (STDIN_FILENO))
    {
      stdin_tty = 0;
      return;                                   // rest is nonsense if not a tty
    }

  if (tcgetattr (STDIN_FILENO, &oldtty) == -1)
    {
      fprintf (stderr, "ERROR: Could not get tty parameters\n");
      exit (101);
    }
  oldtty_set = 1;

  if (register_func (deinit_conio) == -1)
    {
      fprintf (stderr, "ERROR: Could not register function with register_func()\n");
      exit (102);
    }

  newtty = oldtty;
  newtty.c_lflag &= ~(ICANON | ECHO);
  newtty.c_lflag |= ISIG;
  newtty.c_cc[VMIN] = 1;                        // if VMIN != 0, read calls
  newtty.c_cc[VTIME] = 0;                       //  block (wait for input)

  set_tty (&newtty);
}


void
deinit_conio (void)
{
  if (oldtty_set)
    {
      tcsetattr (STDIN_FILENO, TCSAFLUSH, &oldtty);
      oldtty_set = 0;
    }
}


#if     defined __CYGWIN__ && !defined USE_POLL
#warning kbhit() does not work properly in Cygwin executable if USE_POLL is not defined
#endif
// this kbhit() conflicts with DJGPP's one
int
kbhit (void)
{
#ifdef  USE_POLL
  struct pollfd fd;

  fd.fd = STDIN_FILENO;
  fd.events = POLLIN;
  fd.revents = 0;

  return poll (&fd, 1, 0) > 0;
#else
  tty_t tmptty = newtty;
  int ch, key_pressed;

  tmptty.c_cc[VMIN] = 0;                        // doesn't work as expected under
  set_tty (&tmptty);                            //  Cygwin (define USE_POLL)

  if ((ch = fgetc (stdin)) != EOF)
    {
      key_pressed = 1;
      ungetc (ch, stdin);
    }
  else
    key_pressed = 0;

  set_tty (&newtty);

  return key_pressed;
#endif
}
#elif   defined AMIGA                           // (__unix__ && !__MSDOS__) ||
int                                             //  __BEOS__ ||__APPLE__
kbhit (void)
{
  return GetKey () != 0xff ? 1 : 0;
}


int
getch (void)
{
  BPTR con_fileh;
  int temp;

  con_fileh = Input ();
  // put the console into RAW mode which makes getchar() behave like getch()?
  if (con_fileh)
    SetMode (con_fileh, 1);
  temp = getchar ();
  // put the console out of RAW mode (might make sense)
  if (con_fileh)
    SetMode (con_fileh, 0);

  return temp;
}
#endif                                          // AMIGA


#if     defined __unix__ && !defined __MSDOS__
int
drop_privileges (void)
{
  uid_t uid;
  gid_t gid;

  uid = getuid ();
  if (setuid (uid) == -1)
    {
      fprintf (stderr, "ERROR: Could not set uid\n");
      return 1;
    }
  gid = getgid ();                              // This shouldn't be necessary
  if (setgid (gid) == -1)                       //  if `make install' was
    {                                           //  used, but just in case
      fprintf (stderr, "ERROR: Could not set gid\n"); //  (root did `chmod +s')
      return 1;
    }

  return 0;
}
#endif


int
register_func (void (*func) (void))
{
  st_func_node_t *func_node = &func_list, *new_node;

  while (func_node->next != NULL)
    func_node = func_node->next;

  if ((new_node = (st_func_node_t *) malloc (sizeof (st_func_node_t))) == NULL)
    return -1;

  new_node->func = func;
  new_node->next = NULL;
  func_node->next = new_node;
  return 0;
}


int
unregister_func (void (*func) (void))
{
  st_func_node_t *func_node = &func_list, *prev_node = &func_list;

  while (func_node->next != NULL && func_node->func != func)
    {
      prev_node = func_node;
      func_node = func_node->next;
    }
  if (func_node->func != func)
    return -1;

  if (!func_list_locked)
    {
      prev_node->next = func_node->next;
      free (func_node);
    }
  return 0;
}


void
handle_registered_funcs (void)
{
  st_func_node_t *func_node = &func_list;

  func_list_locked = 1;
  while (func_node->next != NULL)
    {
      func_node = func_node->next;              // first node contains no valid address
      if (func_node->func != NULL)
        func_node->func ();
    }
  func_list_locked = 0;
}


#ifndef HAVE_BYTESWAP_H
uint16_t
bswap_16 (uint16_t x)
{
#if 1
  uint8_t *ptr = (uint8_t *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[1];
  ptr[1] = tmp;
  return x;
#else
  return (((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8);
#endif
}


uint32_t
bswap_32 (uint32_t x)
{
#if 1
  uint8_t *ptr = (uint8_t *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[3];
  ptr[3] = tmp;
  tmp = ptr[1];
  ptr[1] = ptr[2];
  ptr[2] = tmp;
  return x;
#else
  return ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
    (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24));
#endif
}


uint64_t
bswap_64 (uint64_t x)
{
  uint8_t *ptr = (uint8_t *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[7];
  ptr[7] = tmp;
  tmp = ptr[1];
  ptr[1] = ptr[6];
  ptr[6] = tmp;
  tmp = ptr[2];
  ptr[2] = ptr[5];
  ptr[5] = tmp;
  tmp = ptr[3];
  ptr[3] = ptr[4];
  ptr[4] = tmp;
  return x;
}
#endif // #ifndef HAVE_BYTESWAP_H


void
wait2 (int nmillis)
{
#ifdef  __MSDOS__
  delay (nmillis);
#elif   defined __unix__ || defined __APPLE__   // Mac OS X actually
  usleep (nmillis * 1000);
#elif   defined __BEOS__
  snooze (nmillis * 1000);
#elif   defined AMIGA
  Delay (nmillis * 1000);
#elif   defined _WIN32
  Sleep (nmillis);
#else
#ifdef  __GNUC__
#warning Please provide a wait2() implementation
#else
#pragma message ("Please provide a wait2() implementation")
#endif
  int n;
  for (n = 0; n < nmillis * 65536; n++)
    ;
#endif
}


char *
q_fbackup (const char *filename, int mode)
{
  static char buf[FILENAME_MAX];

  if (access (filename, R_OK) != 0)
    return (char *) filename;

  strcpy (buf, filename);
  set_suffix (buf, ".BAK");
  if (strcmp (filename, buf) != 0)
    {
      remove (buf);                             // *try* to remove or rename will fail
      if (rename (filename, buf))               // keep file attributes like date, etc.
        {
          fprintf (stderr, "ERROR: Can't rename \"%s\" to \"%s\"\n", filename, buf);
          exit (1);
        }
    }
  else // handle the case where filename has the suffix ".BAK".
    {
      char *dir = dirname2 (filename), buf2[FILENAME_MAX];

      if (dir == NULL)
        {
          fprintf (stderr, "INTERNAL ERROR: dirname2() returned NULL\n");
          exit (1);
        }
      strcpy (buf, dir);
      if (buf[0] != 0)
        if (buf[strlen (buf) - 1] != FILE_SEPARATOR)
          strcat (buf, FILE_SEPARATOR_S);

      strcat (buf, basename (tmpnam2 (buf2)));
      if (rename (filename, buf))
        {
          fprintf (stderr, "ERROR: Can't rename \"%s\" to \"%s\"\n", filename, buf);
          exit (1);
        }
      free (dir);
    }

  switch (mode)
    {
      case BAK_MOVE:
        return buf;

      case BAK_DUPE:
      default:
        if (q_fcpy (buf, 0, q_fsize (buf), filename, "wb"))
          {
            fprintf (stderr, "ERROR: Can't open \"%s\" for writing\n", filename);
            exit (1);
          }
        sync ();
        return buf;
    }
}


int
q_fcrc32 (const char *filename, int start)
{
  unsigned int n, crc = 0;                      // don't name it crc32 to avoid
  unsigned char buffer[MAXBUFSIZE];             //  name clash with zlib's func
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    return -1;

  fseek (fh, start, SEEK_SET);

  while ((n = fread (buffer, 1, MAXBUFSIZE, fh)))
    crc = crc32 (crc, buffer, n);

  fclose (fh);

  return crc;
}


int
q_fcpy (const char *src, int start, int len, const char *dest, const char *mode)
{
  int seg_len;
  char buf[MAXBUFSIZE];
  FILE *fh, *fh2;

  if (one_file (dest, src))                     // other code depends on this
    return -1;                                  //  behaviour!

  if (!(fh = fopen (src, "rb")))
    {
      errno = ENOENT;
      return -1;
    }
  if (!(fh2 = fopen (dest, mode)))
    {
      errno = ENOENT;
      fclose (fh);
      return -1;
    }

  fseek (fh, start, SEEK_SET);
  fseek (fh2, 0, SEEK_END);

  for (; len > 0; len -= seg_len)
    {
      if (!(seg_len = fread (buf, 1, MIN (len, MAXBUFSIZE), fh)))
        break;
      fwrite (buf, 1, seg_len, fh2);
    }

  fclose (fh);
  fclose (fh2);
  sync ();
  return 0;
}


int
q_rfcpy (const char *src, const char *dest)
// Raw file copy function. Raw, because it will copy the file data as it is,
//  unlike q_fcpy()
{
#ifdef  HAVE_ZLIB_H
#undef  fopen
#undef  fread
#undef  fwrite
#undef  fclose
#endif
  FILE *fh, *fh2;
  int seg_len;
  char buf[MAXBUFSIZE];

  if (one_file (dest, src))
    return -1;

  if (!(fh = fopen (src, "rb")))
    return -1;
  if (!(fh2 = fopen (dest, "wb")))
    {
      fclose (fh);
      return -1;
    }
  while ((seg_len = fread (buf, 1, MAXBUFSIZE, fh)))
    fwrite (buf, 1, seg_len, fh2);

  fclose (fh);
  fclose (fh2);
  return 0;
#ifdef  HAVE_ZLIB_H
#define fopen   fopen2
#define fread   fread2
#define fwrite  fwrite2
#define fclose  fclose2
#endif
}


int
q_fswap (const char *filename, int start, int len, swap_t type)
{
  int seg_len;
  FILE *fh;
  char buf[MAXBUFSIZE];
  struct stat fstate;

  // First (try to) change the file mode or we won't be able to write to it if
  //  it's a read-only file.
  stat (filename, &fstate);
  if (chmod (filename, fstate.st_mode | S_IWUSR))
    {
      errno = EACCES;
      return -1;
    }

  if (!(fh = fopen (filename, "r+b")))
    {
      errno = ENOENT;
      return -1;
    }

  fseek (fh, start, SEEK_SET);

  for (; len > 0; len -= seg_len)
    {
      if (!(seg_len = fread (buf, 1, MIN (len, MAXBUFSIZE), fh)))
        break;
      if (type == SWAP_BYTE)
        mem_swap_b (buf, seg_len);
      else // SWAP_WORD
        mem_swap_w (buf, seg_len);
      fseek (fh, -seg_len, SEEK_CUR);
      fwrite (buf, 1, seg_len, fh);
      /*
        This appears to be a bug in DJGPP and Solaris. Without an extra call to
        fseek() a part of the file won't be swapped (DJGPP: after 8 MB, Solaris:
        after 12 MB).
      */
      fseek (fh, 0, SEEK_CUR);
    }

  fclose (fh);
  sync ();
  return 0;
}


int
q_fncmp (const char *filename, int start, int len, const char *search,
         int searchlen, int wildcard)
{
#define BUFSIZE 8192
  char buf[BUFSIZE];
  FILE *fh;
  int seglen, maxsearchlen, searchpos, filepos = 0, matchlen = 0;

  if (!(fh = fopen (filename, "rb")))
    {
      errno = ENOENT;
      return -1;
    }
  fseek (fh, start, SEEK_SET);
  filepos = start;

  while ((seglen = fread (buf, 1, BUFSIZE + filepos > start + len ?
                            start + len - filepos : BUFSIZE, fh)))
    {
      maxsearchlen = searchlen - matchlen;
      for (searchpos = 0; searchpos <= seglen; searchpos++)
        {
          if (searchpos + maxsearchlen >= seglen)
            maxsearchlen = seglen - searchpos;
          if (!memwcmp (buf + searchpos, search + matchlen, maxsearchlen, wildcard))
            {
              if (matchlen + maxsearchlen < searchlen)
                {
                  matchlen += maxsearchlen;
                  break;
                }
              else
                {
                  fclose (fh);
                  return filepos + searchpos - matchlen;
                }
            }
          else
            matchlen = 0;
        }
      filepos += seglen;
    }

  fclose (fh);
  return -1;
}


#if 0
int
process_file (const char *src, int start, int len, const char *dest, const char *mode, int (*func) (char *, int))
{
  int seg_len;
  char buf[MAXBUFSIZE];
  FILE *fh, *fh2;

  if (one_file (dest, src))
    return -1;

  if (!(fh = fopen (src, "rb")))
    {
      errno = ENOENT;
      return -1;
    }
  if (!(fh2 = fopen (dest, mode)))
    {
      errno = ENOENT;
      fclose (fh);
      return -1;
    }

  fseek (fh, start, SEEK_SET);
  fseek (fh2, 0, SEEK_END);

  for (; len > 0; len -= seg_len)
    {
      if (!(seg_len = fread (buf, 1, MIN (len, MAXBUFSIZE), fh)))
        break;
      func (buf, seg_len);
      fwrite (buf, 1, seg_len, fh2);
    }

  fclose (fh);
  fclose (fh2);
  sync ();
  return 0;
}
#endif


int
argz_extract2 (char **argv, char *str, const char *separator_s, int max_args)
{
// TODO: replace with argz_extract()
#ifdef  DEBUG
  int pos = 0;
#endif
  int argc = 0;

  if (!str)
    return 0;
  if (!str[0])
    return 0;

  for (; (argv[argc] = (char *) strtok (!argc?str:NULL, separator_s)) &&
    argc < (max_args - 1); argc++);

#ifdef  DEBUG
  fprintf (stderr, "argc:     %d\n", argc);
  for (pos = 0; pos < argc; pos++)
    fprintf (stderr, "argv[%d]:  %s\n", pos, argv[pos]);

  fflush (stderr);
#endif

  return argc;
}


#ifdef  _WIN32
int
truncate (const char *path, off_t size)
{
  int retval;
  HANDLE file = CreateFile (path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return -1;

  SetFilePointer (file, size, 0, FILE_BEGIN);
  retval = SetEndOfFile (file);                 // returns nonzero on success
  CloseHandle (file);

  return retval ? 0 : -1;                       // truncate() returns zero on success
}


int
sync (void)
{
  _commit (fileno (stdout));
  _commit (fileno (stderr));
  fflush (NULL);                                // flushes all streams opened for output
  return 0;
}


#if     defined __MINGW32__ && defined DLL
// Ugly hack in order to fix something in zlib (yep, it's that bad)
FILE *
fdopen (int fd, const char *mode)
{
  return _fdopen (fd, mode);
}
#endif


#elif   defined AMIGA                           // _WIN32
int
truncate (const char *path, off_t size)
{
  return 0;
}


int
chmod (const char *path, mode_t mode)
{
  if (!SetProtection ((STRPTR) path,
                      ((mode & S_IRUSR ? 0 : FIBF_READ) |
                       (mode & S_IWUSR ? 0 : FIBF_WRITE | FIBF_DELETE) |
                       (mode & S_IXUSR ? 0 : FIBF_EXECUTE) |
                       (mode & S_IRGRP ? FIBF_GRP_READ : 0) |
                       (mode & S_IWGRP ? FIBF_GRP_WRITE | FIBF_GRP_DELETE : 0) |
                       (mode & S_IXGRP ? FIBF_GRP_EXECUTE : 0) |
                       (mode & S_IROTH ? FIBF_OTR_READ : 0) |
                       (mode & S_IWOTH ? FIBF_OTR_WRITE | FIBF_OTR_DELETE : 0) |
                       (mode & S_IXOTH ? FIBF_OTR_EXECUTE : 0))))
    return -1;
  else
    return 0;
}


void
sync (void)
{
}


int
readlink (const char *path, char *buf, int bufsize)
{
  // always return -1 as if anything passed to it isn't a soft link
  return -1;
}


// custom _popen() and _pclose(), because the standard ones (named popen() and
//  pclose()) are buggy
FILE *
_popen (const char *path, const char *mode)
{
  int fd;
  BPTR fh;
  long fdflags, fhflags;
  char *apipe = malloc (strlen (path) + 7);

  if (!apipe)
    return NULL;

  strcpy (apipe, "APIPE:");
  strcat (apipe, path);

  if (*mode == 'w')
    {
      fdflags = 2;
      fhflags = MODE_NEWFILE;
    }
  else
    {
      fdflags = 1;
      fhflags = MODE_OLDFILE;
    }

  if (!(fh = Open (apipe, fhflags)))
    return NULL;

  return fdopen (fd, mode);
}


int
_pclose (FILE *stream)
{
  return fclose (stream);
}
#endif                                          // AMIGA
