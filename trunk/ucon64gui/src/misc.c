/*
misc.c - miscellaneous functions

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
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>                             // va_arg()
#include <limits.h>                             // opendir2()
#include <sys/stat.h>
#ifdef  __CYGWIN__                              // under Cygwin (gcc for Windows) we
#define USE_POLL                                //  need poll() for kbhit(). poll()
#include <sys/poll.h>                           //  is available on Linux, not on
#endif                                          //  BeOS. DOS already has kbhit()

#ifdef __MSDOS__
#include <conio.h>                              // getch()
#include <pc.h>                                 // kbhit()
#endif

#if     defined __unix__ || defined __BEOS__
#include <termios.h>

typedef struct termios tty_t;
#endif

#include "config.h"                             // ZLIB
#include "misc.h"

#ifdef  ANSI_COLOR
#ifdef  DJGPP
#include <dpmi.h>                               // needed for __dpmi_int() by ansi_init()
#endif
#endif

#ifdef NETWORK
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

extern int errno;
static st_func_node_t func_list = { NULL, NULL };
static int func_list_locked = 0;

#if     defined __unix__ || defined __BEOS__
static void set_tty (tty_t param);
#endif

#ifdef __CYGWIN__
static char *cygwin_fix (char *value);
#endif

static int misc_ansi_color = 0;


#if 0                                           // currently not used
/*
  crc16 routine
*/
unsigned short
mem_crc16 (unsigned int size, unsigned short crc16, const void *buf)
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


#ifndef ZLIB
/*
  crc32 routines

  replacement for zlib's crc32() stuff
*/
#define CRC32_POLYNOMIAL     0xEDB88320L

static unsigned int crc32_table[256];
static int crc32_table_built = 0;

void
build_crc32_table ()
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
mem_crc32 (unsigned int size, unsigned int crc32, const void *buffer)
// zlib: crc32 (crc, buffer, size);
{
  unsigned char *p;
  unsigned int temp1, temp2;

  if (!crc32_table_built)
    build_crc32_table ();

  crc32 ^= 0xffffffffL;
  p = (unsigned char *) buffer;
  while (size-- != 0)
    {
      temp1 = (crc32 >> 8) & 0x00ffffffL;
      temp2 = crc32_table[((int) crc32 ^ *p++) & 0xff];
      crc32 = temp1 ^ temp2;
    }
  return crc32 ^ 0xffffffffL;
}
#endif


#ifdef  ANSI_COLOR
int
ansi_init (void)
{
  int result = isatty (STDOUT_FILENO);

  if (result)
    {
#ifdef  DJGPP
// Don't use __MSDOS__, because __dpmi_regs and __dpmi_int are DJGPP specific
      __dpmi_regs reg;

      reg.x.ax = 0x1a00;                        // DOS 4.0+ ANSI.SYS installation check
      __dpmi_int (0x2f, &reg);
      if (reg.h.al != 0xff)                     // AL == 0xff if installed
        result = 0;
#endif
    }

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
#endif // ANSI_COLOR


#if 0
/*
  Copy into a malloced string
*/
void
strmcpy (char **t, const char *s)
{
  if (*t != NULL)
    free (*t);

  *t = malloc (strlen (s) + 1);
  if (*t == NULL)
    {
      perror ("strmcpy");
      exit (1);
    }

  (void) strcpy (*t, s);
}
#endif


/*
  like isprint() but for strings
*/
int
areprint (const char *str, int size)
{
  while (size > 0)
    {
      if (!isprint ((int) str[--size]))
        return FALSE;//0
    }
  return TRUE;//1
}


char *
mkprint (char *str, const char replacement)
{
  char *str1 = str;

  while (*str)
    {
      switch (*str)
        {
          case '\n':
            break;

          case '\x1b':                          // escape
            if (misc_ansi_color)
              break;

          default:
            if (iscntrl ((int) *str))
              *str = replacement;
            break;
        }
      str++;
    }

  return str1;
}


char *
mkfile (char *str, const char replacement)
{
  int pos = 0;
  char *str1 = str;

  while (*str)
    {
      if (pos == FILENAME_MAX)
        {
          str = 0;
          break;
        }

      switch (*str)
        {
          case '.':
          case '-':
            break;

          case ' ':
            *str = replacement;
            break;

          default:
            if (!isalnum ((int) *str))
              *str = replacement;
            break;
        }
      str++;
      pos++;
    }

  return str1;
}


#if 0
/*
  Description

  stricmp() and strnicmp() functions lexicographically compare the nullterminated
  strings s1 and s2 case independent

  Return Values

  The strimp() and strnicmp() return an integer greater than, equal to, or less
  than 0, according as the string s1 is greater than, equal to, or less than the
  string s2. The comparison is done using unsigned characters, so that '\\200'
  is greater than '\\0'.

  The strnicmp() compares not more than n characters.
*/
#define TOLOWER(a) (((a > 'A') && (a < 'Z')) ? ((a) + 0x20) : (a))
#define TOUPPER(a) (((a > 'a') && (a < 'z')) ? ((a) - 0x20) : (a))

int
strnicmp (const char *s1, const char *s2, size_t n)
{
  if (!strcmp (s1, s2))
    return 0;

  for (;n ;n--)
    if (TOLOWER (s1[n]) != TOLOWER (s2[n])) return -1;

  return 0;
}


int
stricmp (const char *s1, const char *s2)
{
  size_t l1 = strlen (s1), l2 = strlen (s2);

  return strnicmp (s1, s2, (l1 > l2) ? l1 : l2);
}
#endif


int
findlwr (const char *str)
// searches the string for ANY lowercase char
{
  const char *p = str;

//TODO filenames which have only numbers

  while (*p)
    {
      if (islower ((int) *p))
        return TRUE;
      p++;
    }

  return FALSE;
}


int
strrcspn (const char *str, const char *str2)
/*
  same as strcspn() but looks for the LAST appearance of str2
  strrcspn (".1234.6789",".") == 5
*/
{
  int len = strlen (str) - 1, len2 = strlen (str2);

  while (len > -1)
    if (!strncmp (&str[len], str2, len2))
      return len;
    else
      len--;

  return strlen (str);
}


char *
strupr (char *str)
{
  char *p = str;

  while (*p)
    {
      *p = toupper (*p);
      p++;
    }

  return str;
}


char *
strlwr (char *str)
{
  char *p = str;

  while (*p)
    {
      *p = tolower (*p);
      p++;
    }

  return str;
}


char *
setext (char *filename, const char *ext)
{
  char ext2[FILENAME_MAX];
  int pos = ((strchr (&filename[1], FILE_SEPARATOR) == NULL) ||
         (strrcspn (filename, ".") > (strrcspn (filename, FILE_SEPARATOR_S) + 1 ))) ?
    strrcspn (filename, ".") : strlen (filename);

  filename[pos] = 0;

  strcpy (ext2, ext);
  strcat (filename, findlwr (filename_only (filename)) ? strlwr (ext2) : strupr (ext2));

  return filename;
}


const char *
getext (const char *filename)
{
  const char *p = NULL;

  if (!(p = strrchr (filename, FILE_SEPARATOR)))
    p = filename;
  if (!(p = strrchr (p, '.')))
    p = "";

  return p;
}


char *
stpblk (char *str)
{
  while (*str == '\t' || *str == 32)
    str++;
  return str;
}


char *
strtrim (char *str)
{
  long x = strlen (str) - 1;

  while (x && str[x] == 32)
    x--;
  str[x + 1] = 0;

  return stpblk (str);
}


char *
stplcr (char *str)
{
  char *p = str;

  while (*p != 0x00 && *p != 0x0a && *p != 0x0d)
    p++;
  *p = 0;

  return str;
}


int
memwcmp (const void *add, const void *add_with_wildcards, size_t n, int wildcard)
{
  const unsigned char *a = add, *a_w = add_with_wildcards;

  while (n)
    {
      if (/* *a != wildcard &&*/ *a_w != wildcard && *a != *a_w)
        return -1;

      a++;
      a_w++;
      n--;
    }

  return 0;
}


void *
memswap (void *add, size_t n)
{
  size_t pos = 0;
  unsigned char *a = add, c;

  while (pos + 1 < n)
    {
      c = a[pos];
      a[pos] = a[pos + 1];
      a[pos + 1] = c;

      pos += 2;
    }

  return add;
}


void
mem_hexdump (const void *mem, size_t n, long virtual_start)
{
  size_t x;
  char buf[MAXBUFSIZE];
  const unsigned char *a = mem;

  buf [0] = 0;
  for (x = 0; x < n; x++)
    {
      if (!(x % 16))
        printf ("%s%s%08lx  ", x ? buf : "",
                               x ? "\n" : "",
                               x + virtual_start);
      printf ("%02x %s", ((char) a[x]) & 0xff,
                         !((x + 1) % 4) ? " ": "");
      sprintf (&buf[x % 16], "%c", isprint ((int) a[x]) ? a[x] : '.');
    }
  printf ("%s\n", buf);
}


int
renlwr (const char *dir)
{
  struct dirent *ep;
  struct stat fstate;
  DIR *dp;
  char buf[MAXBUFSIZE];

  if (access (dir, R_OK) != 0 || (dp = opendir (dir)) == NULL)
    {
      errno = ENOENT;
      return -1;
    }

  chdir (dir);

  while ((ep = readdir (dp)) != 0)
    {
      if (!stat (ep->d_name, &fstate))
        {
//              if(S_ISREG(fstate.st_mode)==1)
          {
            strcpy (buf, ep->d_name);
            rename (ep->d_name, strlwr (buf));
          }
        }
    }
  (void) closedir (dp);
  return 0;
}


int
file_size (const char *filename)
// If ZLIB is defined this function is very slow. Please avoid to use it much.
{
#ifndef  ZLIB
  struct stat fstate;

  if (!stat (filename, &fstate))
    return fstate.st_size;

  errno = ENOENT;
  return -1;
#else
  FILE *file1;
  gzFile file2;
  unsigned char magic[3] = { 0 };
  int size = 0;
  struct stat fstate;

#undef  fopen
#undef  fread
#undef  fclose
  if ((file1 = fopen (filename, "rb")) == NULL)
    {
      errno = ENOENT;
      return -1;
    }
  fread (magic, 1, sizeof (magic), file1);
  fclose (file1);
#define fopen                           fopen2
#define fclose                          fclose2
#define fread                           fread2
  // ID1, ID2 and CM. gzip uses Compression Method 8
  if (magic[0] != 0x1f || magic[1] != 0x8b || magic[2] != 0x08)
    {
      stat (filename, &fstate);
      return fstate.st_size;
    }

  // it appears to be a gzipped file
  if ((file2 = gzopen (filename, "rb")) == NULL)
    {
      errno = ENOENT;
      return -1;
    }

#if 1
  // This is not much faster than the other method
  while (!gzeof (file2))
    gzseek (file2, 1024 * 1024, SEEK_CUR);
  size = gztell (file2);
#else
  // Is there a more efficient way to determine the uncompressed size?
  while ((bytesread = gzread (file2, buf, MAXBUFSIZE)) > 0)
    size += bytesread;
#endif

  gzclose (file2);
  return size;
#endif
}


unsigned int
file_crc32 (const char *filename, int start)
{
  unsigned int count, crc = 0;                  // don't name it crc32 to avoid
  unsigned char buffer[512];                    //  name clash with zlib's func
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    return -1;

  fseek (fh, start, SEEK_SET);

  while ((count = fread (buffer, 1, 512, fh)))
    crc = mem_crc32 (count, crc, buffer);       // zlib: crc32 (crc32, buffer, count);

  fclose (fh);

  return crc;
}


long
filencmp (const char *filename, long start, long len, const char *search,
          long searchlen, int wildcard)
{
  int seg_len = 0 /* segment length */, seg_pos = 0, // position in segment
      size = file_size (filename);
  char buf[MAXBUFSIZE];

  if (searchlen >= MAXBUFSIZE)
    return -1;

  if (access (filename, R_OK) != 0)
    {
      errno = ENOENT;
      return -1;
    }

  if ((size - start) < len)
    len = size - start;

  while (1)
    {
      seg_len = (len >= MAXBUFSIZE) ? MAXBUFSIZE : len;

      if (!quick_fread (buf, start, seg_len, filename))
        break;

      for (seg_pos = 0; seg_pos < seg_len; seg_pos++)
        if (!memwcmp (&buf[seg_pos], search, searchlen, wildcard))
          return start + seg_pos;

      start += seg_len;
      len -= seg_len;
    }
  return -1;
}


#if 0
long
filencmp (const char *filename, long start, long len, const char *search, long searchlen, const char wildcard)
// searchlen is length of *search in bytes
{
  int x, y, size = file_size (filename);
  char *buf;

  if (access (filename, R_OK) != 0)
    {
      errno = ENOENT;
      return -1;
    }

  x = 0;
  if (!(buf = (char *) malloc ((size - start + 2) * sizeof (char))))
    {
      errno = ENOMEM;
      return -1;
    }

  if (!quick_fread (buf, start, size, filename))
    {
      errno = ENOMEM;
      free (buf);
      return -1;
    }

  while ((x + searchlen + start) < size)
    {
      for (y = 0; y < searchlen; y++)
        if (search[y] != wildcard)
          {
            if (buf[x + y] != search[y])
              break;
            if (y == searchlen - 1)
              {
                free (buf);
                return x + start;
              }
          }
      x++;
    }
  free (buf);
  return -1;
}
#endif


size_t
quick_fread (void *dest, size_t start, size_t len, const char *src)
{
  size_t result = 0;
  FILE *fh;

  if ((fh = fopen (src, "rb")) == NULL)
    {
      errno = ENOENT;
      return -1;
    }

  fseek (fh, start, SEEK_SET);
  result = fread (dest, 1, len, fh);
  fclose (fh);

  return result;
}


size_t
quick_fwrite (const void *src, size_t start, size_t len, const char *dest, const char *mode)
{
  size_t result = 0;
  FILE *fh;

  if ((fh = fopen (dest, mode)) == NULL)
    {
      errno = ENOENT;
      return -1;
    }

  fseek (fh, start, SEEK_SET);
  result = fwrite (src, 1, len, fh);
  fclose (fh);

  return result;
}


int
quick_fgetc (const char *filename, long pos)
{
  int c;
  FILE *fh = fopen (filename, "rb");

  if ((!fh) || fseek (fh, pos, SEEK_SET) != 0)
    {
      errno = ENOENT;
      return -1;
    }

  c = fgetc (fh);
  fclose (fh);

  return c;
}


int
quick_fputc (const char *filename, long pos, int c, const char *mode)
{
  int result;
  FILE *fh = fopen (filename, mode);

  if ((!fh) || fseek (fh, pos, SEEK_SET) != 0)
    {
      errno = ENOENT;
      return -1;
    }

  result = fputc (c, fh);
  fclose (fh);

  return result;
}


int
file_hexdump (const char *filename, long start, long len)
{
  int x, size = file_size (filename), dump;
  char buf[MAXBUFSIZE];
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    {
      errno = ENOENT;
      return -1;
    }
  if ((size - start) < len)
    len = size - start;

  fseek (fh, start, SEEK_SET);
  for (x = 0; x < len; x++)
    {
      if (!(x % 16))
        printf ("%s%s%08lx  ", x?buf:"", x?"\n":"", x + start);

      dump = fgetc (fh);
      printf ("%02x %s", dump & 0xff, !((x + 1) % 4) ? " " : "");
      sprintf (&buf[x % 16], "%c", isprint (dump) ? ((char) dump) : '.');
    }
  printf ("%s\n", buf);
  fclose (fh);

  return 0;
}


int
filecopy (const char *src, long start, long len, const char *dest, const char *mode)
{
  int seg_len = 0, size = file_size (src);
  char buf[MAXBUFSIZE];
  FILE *fh, *fh2;

  if (!strcmp (dest, src))
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

  if ((size - start) < len)
    len = size - start;

  fseek (fh, start, SEEK_SET);
  fseek (fh2, 0, SEEK_END);

  while (1)
    {
      seg_len = (len >= MAXBUFSIZE) ? MAXBUFSIZE : len;

      if (!fread (buf, seg_len, 1, fh))
        break;
      fwrite (buf, seg_len, 1, fh2);

      len -= seg_len;
    }

  fclose (fh);
  fclose (fh2);

  sync ();
  return 0;
}


#if 0
const char *
file_backup (char *filename, long mode)
/*
  We have to handle the following cases (for example -swc and rom.swc exists):
  1) ucon64 -swc rom.swc

    a) backup = 1
       Create backup of rom.swc
       postcondition: src == name of backup

    b) backup = 0
       Create temporary backup of rom.swc by renaming rom.swc
       postcondition: src == name of backup

  2) ucon64 -swc rom.fig

    a) backup = 1
       Create backup of rom.swc
       postcondition: src == rom.fig

    b) backup = 0
       Do nothing
       postcondition: src == rom.fig
*/
{
  char buf[FILENAME_MAX];

  if (access (filename, F_OK) == -1)
    return dest;                                // does not exist == no backup necessary

  strcpy (buf, filename);
  setext (buf, ".BAK");

  if (!access (buf, F_OK))
    remove (buf);

  if (src)
    rename (filename, buf);                     // keep file attributes like date, etc.
  else
    return filename;

  filecopy (src, 0, file_size (src), dest, "wb");
  sync ();

  return src;
}
#endif


/*
  Return type is not const char *, because it may return move_name, which is
  not a pointer to constant characters.
*/
char *
file_backup (char *move_name, const char *filename)
{
  char buf[FILENAME_MAX];

  if (access (filename, R_OK) != 0)
    return (char *) filename;

  strcpy (buf, filename);
  setext (buf, ".BAK");
  if (strcmp (filename, buf))
    {
      remove (buf);                             // try to remove or rename will fail
      rename (filename, buf);                   // keep file attributes like date, etc.
    }

  if (move_name == NULL)
    {
      filecopy (buf, 0, file_size (buf), filename, "wb");
      sync ();
      return (char *) filename;
    }

  strcpy (move_name, buf);
  return move_name;
}


const char *
filename_only (const char *str)
{
  const char *str2;

  if (!(str2 = strrchr (str, FILE_SEPARATOR)))
    str2 = str;                                 // strip path if it is a filename
  else
    str2++;

  return str2;
}


#if 0
unsigned long
filefile (const char *filename, long start, const char *filename2, long start2, int similar)
{
  int fsize = file_size (filename), fsize2 = file_size (filename2), pos = 0,
      base, len, chunksize, chunksize2, readok = 1,
      bytesread, bytesread2, bytesleft, bytesleft2;
  unsigned char buf[MAXBUFSIZE], buf2[MAXBUFSIZE];
  FILE *file, *file2;

  if (!strcmp (filename, filename2))
    return 0;

  if (access (filename, R_OK) != 0 || access (filename2, R_OK) != 0)
    {
      errno = ENOENT;
      return -1;

    }

  if (fsize < start || fsize2 < start2)
    return -1;

  if (!(file = fopen (filename, "rb")))
    {
      errno = ENOENT;
      return -1;
    }

  if (!(file2 = fopen (filename2, "rb")))
    {
      errno = ENOENT;
      fclose (file);
      return -1;
    }

  fseek (file, start, SEEK_SET);
  fseek (file2, start2, SEEK_SET);

  while (fread (buf, 1, MAXBUFSIZE, file) && fread (buf2, 1, MAXBUFSIZE, file2))
    {
      for (pos = 0; pos < MAXBUFSIZE; pos++)
        {
          if ((similar == FALSE && buf[pos] != buf2[pos]) ||
              (similar == TRUE && buf[pos] == buf2[pos]))
            {
              len = 0;
              while ((similar == TRUE) ?
                     (buf[pos + len] == buf2[pos + len]) :
                     (buf[pos + len] != buf2[pos + len]))
                {
                  len++;
                  if (pos + len >= MAXBUFSIZE)
                    break;
                }

              printf ("%s:\n", filename);
              mem_hexdump (&buf[pos], len, start + pos + bytesread2);
              printf ("%s:\n", filename2);
              mem_hexdump (&buf2[pos], len, start2 + pos + bytesread2);
              printf ("\n");
              pos += len;
            }
        }

    }

  fclose (file);
  fclose (file2);

  return 0;
}
#endif


unsigned long
filefile (const char *filename1, long start1, const char *filename2, long start2, int similar)
{
  int base, fsize1, fsize2, len, chunksize1, chunksize2, readok = 1,
      bytesread1, bytesread2, bytesleft1, bytesleft2, bufsize = 1024 * 1024;
  unsigned char *buf1, *buf2;
  FILE *file1, *file2;

  if (!strcmp (filename1, filename2))
    return 0;
  if (access (filename1, R_OK) != 0 || access (filename2, R_OK) != 0)
    {
      errno = ENOENT;
      return -1;
    }

  fsize1 = file_size (filename1);              // file_size() returns size in bytes
  fsize2 = file_size (filename2);
  if (fsize1 < start1 || fsize2 < start2)
    return -1;

  if (!(buf1 = (unsigned char *) malloc (bufsize)))
    {
      errno = ENOMEM;
      return -1;
    }

  if (!(buf2 = (unsigned char *) malloc (bufsize)))
    {
      errno = ENOMEM;
      free (buf1);
      return -1;
    }

  if (!(file1 = fopen (filename1, "rb")))
    {
      errno = ENOENT;
      free (buf1);
      free (buf2);
      return -1;
    }
  if (!(file2 = fopen (filename2, "rb")))
    {
      errno = ENOENT;
      fclose (file1);
      free (buf1);
      free (buf2);
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
                  if ((similar == FALSE && buf1[base] != buf2[base]) ||
                      (similar == TRUE && buf1[base] == buf2[base]))
                    {
                      len = 0;
                      while ((similar == TRUE) ?
                             (buf1[base + len] == buf2[base + len]) :
                             (buf1[base + len] != buf2[base + len]))
                        {
                          len++;
                          if (base + len >= chunksize2)
                            break;
                        }

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
  free (buf1);
  free (buf2);
  return 0;
}


int
file_replace (const char *filename, long start, const char *search, long slen,
             const char *replace, long rlen)
{
  int size = file_size (filename);

  if (access (filename, R_OK) != 0)
    {
      errno = ENOENT;
      return -1;
    }

  for (;;)
    {
      if ((start = filencmp (filename, start, size, search, slen, -1)) == -1)
        return 0;
      file_hexdump (filename, start, slen);
      quick_fwrite (replace, start, rlen, filename, "r+b");
      file_hexdump (filename, start, rlen);
      printf ("\n");
      start++;
    }
}


void
change_string (char *searchstr, int strsize, char wc, char esc,
               char *end, int endsize, char *buf, int bufsize, int offset,
               ...)
/*
  Search for all occurrences of string searchstr in buf and replace endsize bytes in buf
  by copying string end to the end of the found searchstring in buf plus offset.
  If searchstr contains wildcard characters wc, then n wildcard characters in searchstr
  match any n characters in buf.
  If searchstr contains escape characters esc, change_string() must be called with two
  extra arguments for each escape character, set, which must be a string (char *) and
  setsize, which must be an int. searchstr matches for an escape character if one of the
  characters in set matches.
  Note that searchstr is not necessarily a C string; it may contain one or more zero bytes
  as strsize indicates the length.
  offset is the relative offset from the last character in searchstring and may have a
  negative value.
  This function was written to patch SNES ROM dumps. It does basically the same as the old
  uCON does, with one exception, the line with:
    bufpos -= nwc;

  As stated in the comment, this causes the search to restart at the first wildcard
  character of the sequence of wildcards that was most recently skipped if the current
  character in buf didn't match the current character in searchstr. This makes
  change_string() behave a bit more intuitive. For example
    char str[] = "f foobar means...";
    change_string("f**bar", 6, '*', '!', "XXXXXXXX", 8, str, strlen(str), 2);
  finds and changes "foobar means..." into "foobar XXXXXXXX", while with uCON's algorithm
  it would not (but does the job good enough for patching SNES ROMs).

  One example of using sets:
    char str[] = "fu-bar     is the same as foobar    ";
    change_string("f!!", 3, '*', '!', "fighter", 7, str, strlen(str), 1,
                  "uo", 2, "o-", 2);
  This changes str into "fu-fighter is the same as foofighter".
*/
{
  va_list argptr;
  char *set;
  int bufpos, strpos = 0, pos_1st_esc = -1, setsize, i, nwc;

  va_start (argptr, offset);

  for (bufpos = 0; bufpos < bufsize; bufpos++)
    {
      if (strpos == 0 && searchstr[0] != esc && searchstr[0] != wc)
        while (bufpos < bufsize && searchstr[0] != buf[bufpos])
          bufpos++;

      // handle escape character in searchstr
      while (searchstr[strpos] == esc && bufpos < bufsize)
        {
          if (strpos == pos_1st_esc)
            va_start (argptr, offset);          // reset argument pointer
          if (pos_1st_esc == -1)
            pos_1st_esc = strpos;

          set = va_arg (argptr, char *);        // get next set of characters
          setsize = va_arg (argptr, int);       // get set size
          i = 0;
          // see if buf[bufpos] matches with any character in current set
          while (i < setsize && buf[bufpos] != set[i])
            i++;
          if (i == setsize)
            break;                              // buf[bufpos] didn't match with any char

          if (strpos == strsize - 1)            // check if we are at the end of searchstr
            {
              memcpy (buf + bufpos + offset, end, endsize);
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
      nwc = 0;
      while (searchstr[strpos] == wc && bufpos < bufsize)
        {
          if (strpos == strsize - 1)            // check if at end of searchstr
            {
              memcpy (buf + bufpos + offset, end, endsize);
              break;
            }

          strpos++;
          bufpos++;
          nwc++;
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
              memcpy (buf + bufpos + offset, end, endsize);
              strpos = 0;
            }
          else
            strpos++;
        }
      else
        {
          strpos = 0;
          bufpos -= nwc;                        // scan the most recent wildcards too if
        }                                       //  the character didn't match
    }

  va_end (argptr);
}


int
fileswap (const char *filename, long start, long len)
{
  int seg_len = 0, size = file_size (filename);
  FILE *fh;
  char buf[MAXBUFSIZE];

  if (!(fh = fopen (filename, "r+b")))
    {
      errno = ENOENT;
      return -1;
    }

  if ((size - start) < len)
    len = size - start;

  fseek (fh, start, SEEK_SET);

  while (1)
    {
      seg_len = (len >= MAXBUFSIZE) ? MAXBUFSIZE : len;

      if (!fread (buf, 1, seg_len, fh))
        break;

      memswap (buf, seg_len);

      fseek (fh, -(seg_len), SEEK_CUR);
      fwrite (buf, 1, seg_len, fh);

      len -= seg_len;
    }

  fclose (fh);
  sync ();

  return 0;
}


int
gauge (time_t init_time, int pos, int size)
{
#define GAUGE_LENGTH 24LL
  int curr, bps, left, p, percentage;
  char progress[MAXBUFSIZE];
#ifdef  ANSI_COLOR
  const char *ansi_gauge = {
    "\x1b[37m-"
    "\x1b[37m-"
    "\x1b[37m-"

    "\x1b[36m-"

    "\x1b[37m-"
    "\x1b[37m-"

    "\x1b[36m-"
    "\x1b[36m-"

    "\x1b[37m-"

    "\x1b[36m-"
    "\x1b[36m-"
    "\x1b[36m-"
    "\x1b[36m-"
    "\x1b[36m-"
    "\x1b[36m-"

    "\x1b[37m-"

    "\x1b[36m-"
    "\x1b[36m-"

    "\x1b[37m-"
    "\x1b[37m-"

    "\x1b[36m-"

    "\x1b[37m-"
    "\x1b[37m-"
    "\x1b[37m-"};
#endif

  if (pos > size)
    return -1;

  if ((curr = time (0) - init_time) == 0)
    curr = 1;                                   // `round up' to at least 1 sec (no division
                                                //  by zero below)
  bps = pos / curr;                             // # bytes/second (average transfer speed)
  left = size - pos;
  left /= bps ? bps : 1;

  p = (GAUGE_LENGTH * pos) / size;
  progress[0] = 0;
  strncat (progress, "========================", p);

#ifdef  ANSI_COLOR
  if (misc_ansi_color)
    {
      progress[p] = 0;
      if (p < GAUGE_LENGTH)
        strcat(progress, &ansi_gauge[p * 6]);
    }
  else
#endif
    strncat (&progress[p], "------------------------", GAUGE_LENGTH - p);

  percentage = (100LL * pos) / size;            // DON'T make precision worse,
                                                //  by shifting pos or size
#ifdef ANSI_COLOR
  if (misc_ansi_color)
    printf ("\r%10d Bytes \x1b[01;34m[\x1b[35m%s\x1b[34m]\x1b[0m %d%%, BPS=%d, ", pos, progress, percentage, bps);
  else
#endif
    printf ("\r%10d Bytes [%s] %d%%, BPS=%d, ", pos, progress, percentage, bps);

  if (pos == size)
    printf ("TOTAL=%03d:%02d", curr / 60, curr % 60); // DON'T print a newline
  else                                                //  -> gauge can be cleared
    printf ("ETA=%03d:%02d   ", left / 60, left % 60);

  fflush (stdout);

  return 0;
}


#ifdef __CYGWIN__
/*
  Weird problem with combination Cygwin uCON64 exe and cmd.exe (Bash is ok):
  When a string with "e (e with diaeresis, one character) is read from an
  environment variable, the character isn't the right character for accessing
  the file system. We fix this.
  TODO: fix the same problem for other non-ASCII characters (> 127).
*/
char *
cygwin_fix (char *value)
{
  int l = strlen (value);

  change_string ("\x89", 1, 0, 0, "\xeb", 1, value, l, 0); // e diaeresis
  change_string ("\x84", 1, 0, 0, "\xe4", 1, value, l, 0); // a diaeresis
  change_string ("\x8b", 1, 0, 0, "\xef", 1, value, l, 0); // i diaeresis
  change_string ("\x94", 1, 0, 0, "\xf6", 1, value, l, 0); // o diaeresis
  change_string ("\x81", 1, 0, 0, "\xfc", 1, value, l, 0); // u diaeresis

  return value;
}
#endif


/*
  getenv() suitable for enviroments w/o HOME, TMP or TEMP variables
  The caller is responsible for freeing the memory of string. Note that
  this function should NEVER return NULL.
*/
char *
getenv2 (const char *variable)
{
  char *tmp, *value;

  value = (char *) malloc (FILENAME_MAX);       // ALWAYS use value, so that the
  value[0] = 0;                                 //  caller can always use free()

  if ((tmp = getenv (variable)) != NULL)
    strcpy (value, tmp);
  else
    {
      if (!strcmp (variable, "HOME"))
        {
          if ((tmp = getenv (variable)) != NULL)
            strcpy (value, tmp);
          else if ((tmp = getenv ("USERPROFILE")) != NULL)
            strcpy (value, tmp);
          else if ((tmp = getenv ("HOMEDRIVE")) != NULL)
            {
              strcpy (value, tmp);
              strcat (value, getenv ("HOMEPATH"));
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
              c = toupper (value[0]);
              // if current dir is root dir strip problematic ending slash (DJGPP)
              if (c >= 'A' && c <= 'Z' &&
                  value[1] == ':' && value[2] == '/' && value[3] == 0)
                value[2] = 0;
            }
         }

      if (!strcmp (variable, "TEMP") || !strcmp (variable, "TMP"))
        {
          if ((tmp = getenv (variable)) != NULL)
            strcpy (value, tmp);
          else
            getcwd (value, FILENAME_MAX);
        }
    }

#ifdef __CYGWIN__
  return cygwin_fix (value);
#else
  return value;
#endif
}


const char *
get_property (const char *filename, const char *propname, char *buffer, const char *def)
{
  char buf[MAXBUFSIZE], *ptr;
  FILE *fh;

  if ((fh = fopen (filename, "rb")) != 0)
    {
      while (fgets (buf, sizeof buf, fh) != NULL)
        {
          stplcr (buf);
          if (stpblk (buf)[0] == '#')
            continue;

          buf[strcspn (buf, "#")] = 0;          // comment at end of a line

          if (!strncmp (buf, propname, strlen (propname)))
            {
              ptr = strchr (buf, '=');
              ptr++;
              strcpy (buffer, stpblk (ptr));

              fclose (fh);
              return buffer;
            }
        }
      fclose (fh);
    }

  ptr = getenv2 (propname);
  if (ptr[0] == 0)                              // getenv2() never returns NULL
    {
      if (def)
        strcpy (buffer, def);
      else
        buffer = (char *) def;                  // buffer won't be changed
    }                                           //  after this func (=ok)
  else
    strcpy (buffer, ptr);
  free (ptr);
  return buffer;
}


int
set_property (const char *filename, const char *propname, const char *value)
{
  int found = 0;
  char buf[MAXBUFSIZE], *buf2;
  FILE *fh;

  if (!(buf2 = (char *) malloc ((file_size (filename) + MAXBUFSIZE) * sizeof (char))))
    {
      errno = ENOMEM;
      return -1;
    }

  buf2[0] = 0;

  if ((fh = fopen (filename, "rb")) != 0)
    {
      while (fgets (buf, sizeof buf, fh) != NULL)
        {
          if (!strncmp (buf, propname, strlen (propname)))
            {
              found = 1;
              if (value == NULL)
                continue;
              sprintf (buf, "%s=%s\n", propname, value);
            }
          strcat (buf2, buf);
        }
      fclose (fh);
    }

    if (!found && value != NULL)
      {
        sprintf (buf, "%s=%s\n", propname, value);
        strcat (buf2, buf);
      }

    quick_fwrite (buf2, 0, strlen (buf2), filename, "wb");

    return 0;
}


#if 0
static int
my_system (const char *command)
{
  int pid, status;

  if (command == 0)
    return 1;
  pid = fork ();
  if (pid == -1)
    return -1;
  if (pid == 0)
    {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = (char *) command;
      argv[3] = 0;
      execve ("/bin/sh", argv, environ);
      exit (127);
    }
  do
    {
      if (waitpid (pid, &status, 0) == -1)
        {
          if (errno != EINTR)
            return -1;
        }
      else
        return status;
    }
  while (1);
}
#endif


int
fsystem (FILE *output, const char *cmdline)
{
  int result = -1;
  FILE *fh;
  char buf[MAXBUFSIZE];

  if (output == stdout)
    return system (cmdline)
#ifndef __MSDOS__
      >> 8                                      // the exit code is coded in bits 8-15
#endif                                          //  (that is, under non-DOS)
/*
  // Snes9x (Linux) for example returns a non-zero value on a normal exit (3)...

  // under WinDOS, system() immediately returns with exit code 0 when starting
  //  a Windows executable (as if a fork() happened) it also returns 0 when the
  //  exe could not be started
*/
    ;
  if (!(fh = popen (cmdline, "r"))) 
    return -1;

  while (fgets (buf, MAXBUFSIZE, fh) != NULL)
    if (output)
      fprintf (output, buf);

  result = pclose (fh);
  if (output)
    fsync (fileno (output));

  return result;
}


int
rmdir_R (const char *path)
{
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
          if (strcmp (ep->d_name, "..") != 0 &&
            strcmp (ep->d_name, ".") != 0) rmdir_R (ep->d_name);
        }
      else
        remove (ep->d_name);
    }

  closedir (dp);
  chdir (cwd);

  return rmdir (path);
}


char *
tmpnam2 (char *temp) // tmpnam() replacement
{
  char *p = getenv2 ("TEMP");
  temp[0] = 0;
 
  while (!temp[0] || !access (temp, F_OK))
    sprintf (temp, "%s%s%08x.tmp", NULL_TO_EMPTY (p),
      (p?FILE_SEPARATOR_S:""), RANDOM (0x10000000, 0xffffffff));

  return temp;
}


typedef struct
{
  DIR *dir;
  char temp[FILENAME_MAX]; //  if temp == NULL then archive_or_dir was in fact a dir
} DIR2;

static DIR2 misc_dir2[FOPEN_MAX];


DIR *
opendir2 (char *archive_or_dir, const char *config_file, const char *property_format, const char *filename)
{
  int pos = 0;
  struct stat fstate;
//  char temp[FILENAME_MAX];
  char *temp, property_name[MAXBUFSIZE], buf[MAXBUFSIZE], buf2[MAXBUFSIZE],
       cwd[FILENAME_MAX];

  if (stat (archive_or_dir, &fstate) == -1)
    return NULL;

  if (S_ISDIR (fstate.st_mode))
    return opendir (archive_or_dir);

  for (pos = 0; pos < FOPEN_MAX; pos++)
    if (!misc_dir2[pos].dir)
      break;
  if (misc_dir2[pos].dir)
    return NULL; // FOPEN_MAX has been reached

  sprintf (property_name, (property_format ? property_format : "%s_extract"), &getext (archive_or_dir)[1]);
  sprintf (buf, NULL_TO_EMPTY (get_property (config_file, strlwr (property_name), buf2, NULL)), archive_or_dir);

  if (!buf[0])
    return NULL;

  if (filename)
    {
      strcat (buf, " ");
      strcat (buf, filename);
    }

  temp = archive_or_dir;

  tmpnam2 (temp);
  if (mkdir (temp, S_IRUSR|S_IWUSR) == -1)
    return NULL;

  getcwd (cwd, FILENAME_MAX);
  chdir (temp);

  fsystem (stderr, buf);
  sync ();

  chdir (cwd);

  misc_dir2[pos].dir = opendir (temp);
  strcpy (misc_dir2[pos].temp, temp);

  return misc_dir2[pos].dir;
}


int closedir2 (DIR *dir)
{
  int pos;

  for (pos = 0; pos < FOPEN_MAX; pos++)
    if (misc_dir2[pos].dir == dir)
      {
        if (misc_dir2[pos].temp[0])
          rmdir_R (misc_dir2[pos].temp);

        misc_dir2[pos].dir = NULL;
        misc_dir2[pos].temp[0] = 0;
        break;
      }

  return closedir (dir);
}


#if 0
char *
html_parser (const char *filename, char *buffer)
{
  int tag = 0;
  char c = 0, buf[2];
  FILE *fh;

  if (!(fh = fopen (filename, "rb")))
    {
      errno = ENOENT;
      return buffer;
    }

  buffer[0] = 0;
  while ((c = fgetc (fh)) != EOF)
    {
      switch (c)
        {
          case '<':
            tag = 1;
            break;
            
          case '>':
            tag = 0;
            break;
            
          default:
            if (!tag) switch (c)
              {            
//                case '\n':
//                  strcat (buffer, "<BR>");
//                  break;
               
                default:
                  sprintf (buf, "%c", c);
                  strcat (buffer, buf);
                  break;
              }
    
            if (tag) switch (c)
              {
                default:
                  break;
              }

            break;
        }
    }

  fclose (fh);

  return buffer;
}
#endif


st_url_t *
url_parser (st_url_t *url_p, const char *url)
{
/*
  possible (tested) url's
  
  host
  host:80
  host/path/index.html
  host:80/path/index.html
  host?query&query2
  host:80?query&query2
  host/path/index.html?query&query2
  host:80/path/index.html?query&query2
  
  all of these w/ or w/o 'protocol://' in front
*/
  char buf[MAXBUFSIZE];
  char *p = NULL;
  int x = 0, c = 0;
    
  memset (url_p, 0, sizeof (st_url_t));

/*
  protocol
*/
  strcpy (buf, url);
  if ((p = strstr (buf, "://")) != NULL)
    {
      *p = 0;
      strcpy (url_p->protocol, buf);
    }

#ifdef DEBUG
  fprintf (stderr, "%s\n", url_p->protocol);
  fflush (stderr);
#endif // DEBUG  

/*
  host
*/
  strcpy (buf, url);

  if ((p = strstr (buf, "://")) != NULL) p += 3;
  else p = buf;
  
  p[strcspn (p, ":/?")] = 0;

  strcpy (url_p->host, p);
  

#ifdef DEBUG
  fprintf (stderr, "%s\n", url_p->host);
  fflush (stderr);
#endif // DEBUG  

/*
  port
*/
  strcpy (buf, url);

  if (strchr (buf, ':'))
    {
      p = buf;
      while (strchr (p, ':') != NULL)
        {
          p = strchr (p, ':');
          p++;
        }
    }
  else p = NULL;
  
  if (p)
    {
      p[strspn (p, "0123456789")] = 0;

      url_p->port = strtol (p, NULL, 10);
    }

  if (!url_p->port)
    {
      if (!stricmp (url_p->protocol, "http")) url_p->port = 80;
      else if (!stricmp (url_p->protocol, "ftp")) url_p->port = 21;
    }

#ifdef DEBUG
  fprintf (stderr, "%d\n", url_p->port);
  fflush (stderr);
#endif // DEBUG  

/*
  path
*/
  strcpy (buf, url);
  p = strstr (buf, url_p->host) + strlen (url_p->host);

  if ((p = strchr (p, '/')) != NULL)
    {
      if (strchr (p, '&') || strchr (p, '?')) p[strcspn (p, "?&")] = 0;
      strcpy (url_p->path, p);
    }

#ifdef DEBUG
  fprintf (stderr, "%s\n", url_p->path);
  fflush (stderr);
#endif // DEBUG  

/*
  query
*/
  strcpy (buf, url);
  if ((p = strchr (buf, '?')) != NULL)
    strcpy (url_p->query, p);
    
#ifdef DEBUG
  fprintf (stderr, "%s\n", url_p->query);
  fflush (stderr);
#endif // DEBUG  

/*
  cmdline
*/
  if (url_p->path[0]) /* argv[0] */
    strcpy (url_p->cmd, url_p->path);

  if (url_p->query[0])
    {
      strcat (url_p->cmd, " ");

      for (x = 0; x < strlen (url_p->query); x++)
        {
          p = NULL;
          switch (url_p->query[x])
            {
              case '?':
                p = "";
                break;
                
              case '&':
              case '+':
                p = " ";
                break;

              case '%':
                sscanf (&url_p->query[x+1], "%02x", &c);
                sprintf (buf, "%c", c);
                x += 2;
                break;

              default:
                sprintf (buf, "%c", url_p->query[x]);
                break;
            }

            strcat (url_p->cmd, p?p:buf);
         }
    }

#ifdef DEBUG
  fprintf (stderr, "%s\n", url_p->cmd);
  fflush (stderr);
#endif // DEBUG  

/*
  args
*/
  if (url_p->cmd[0])
    {
      strcpy (buf, url_p->cmd);

      while ((url_p->argv[url_p->argc] = strtok (!url_p->argc?buf:NULL, " ")) && url_p->argc < (ARGS_MAX - 1))
        url_p->argc++;
    }

#ifdef DEBUG
  for (x = 0; x < url_p->argc; x++)
    fprintf (stderr, "%d:%s\n", x, url_p->argv[x]);
  fflush (stderr);

/*
  again
*/
  fprintf (stderr, "----\n%s\n", url_p->protocol);
  fprintf (stderr, "%s\n", url_p->host);
  fprintf (stderr, "%d\n", url_p->port);
  fprintf (stderr, "%s\n", url_p->path);
  fprintf (stderr, "%s\n", url_p->query);
  fprintf (stderr, "%s\n", url_p->cmd);
  for (x = 0; x < url_p->argc; x++)
    fprintf (stderr, "%d:%s\n", x, url_p->argv[x]);
  fflush (stderr);
#endif // DEBUG

  return url_p;
}


#if     defined __unix__ || defined __BEOS__
static int oldtty_set = 0, stdin_tty = 1;       // 1 => stdin is a tty, 0 => it's not
static tty_t oldtty, newtty;


void
set_tty (tty_t param)
{
  if (stdin_tty && tcsetattr (STDIN_FILENO, TCSANOW, &param) == -1)
    {
      fprintf (stderr, "ERROR: Could not set tty parameters\n");
      exit (100);
    }
}


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

  set_tty (newtty);
}


void
deinit_conio (void)
{
  if (oldtty_set)
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &oldtty);
}


#if     defined __CYGWIN__ && !defined USE_POLL
#warning kbhit() does not work properly in Cygwin executable if USE_POLL is not defined
#endif
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
  set_tty(tmptty);                              //  Cygwin (define USE_POLL)

  if ((ch = fgetc (stdin)) != EOF)
    {
      key_pressed = 1;
      ungetc (ch, stdin);
    }
  else
    key_pressed = 0;

  set_tty (newtty);

  return key_pressed;
#endif
}
#endif                                          // __unix__ || __BEOS__


#ifdef  __unix__
int
drop_privileges (void)
{
  uid_t uid;
  gid_t gid;

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


#ifdef  ZLIB
st_map_t *
map_create (int n_elements)
{
  st_map_t *map;
  int size = sizeof (st_map_t) + n_elements * sizeof (st_map_element_t);

  if ((map = (st_map_t *) malloc (size)) == NULL)
    {
      fprintf (stderr, "ERROR: Not enough memory for buffer (%d bytes)\n", size);
      exit (1);
    }
  map->data = (st_map_element_t *) (((unsigned char *) map) + sizeof (st_map_t));
  memset (map->data, MAP_FREE_KEY, n_elements * sizeof (st_map_element_t));
  map->size = n_elements;
  return map;
}


void
map_copy (st_map_t *dest, st_map_t *src)
{
  memcpy (dest->data, src->data, src->size * sizeof (st_map_element_t));
}


st_map_t *
map_put (st_map_t *map, void *key, void *object)
{
  int n = 0;

  // I (dbjh) don't use a more efficient but also more complex technique,
  //  because we will probably only have small maps.
  while (n < map->size && map->data[n].key != MAP_FREE_KEY)
    n++;

  if (n == map->size)                           // current map is full
    {
      int new_size = map->size + 20;
      st_map_t *map2;

      map2 = map_create (new_size);
      map_copy (map2, map);
      free (map);
      map = map2;
    }

  map->data[n].key = key;
  map->data[n].object = object;

  return map;
}


void *
map_get (st_map_t *map, void *key)
{
  int n = 0;

  while (n < map->size && map->data[n].key != key)
    n++;

  if (n == map->size)
    return NULL;

  return map->data[n].object;
}


void
map_del (st_map_t *map, void *key)
{
  int n = 0;

  while (n < map->size && map->data[n].key != key)
    n++;

  if (n < map->size)
    map->data[n].key = MAP_FREE_KEY;
}


void
map_dump (st_map_t *map)
{
  int n = 0;

  while (n < map->size)
    {
      printf ("%p -> %p\n", map->data[n].key, map->data[n].object);
      n++;
    }
}


/*
  The zlib functions gzwrite() and gzputc() write compressed data if the file
  was opened with a "w" in the mode string, but we want to control whether we
  write compressed data. Note that for the mode string "w+", zlib ignores the
  '+'. zlib does the same for "r+".
  Currently we never write compressed data.
*/
st_map_t *fh_map = NULL;                        // associative array: file handle -> file mode

typedef enum { FM_NORMAL, FM_ZLIB, FM_UNDEF } fmode2_t;

typedef struct st_finfo
{
  fmode2_t fmode;
  int compressed;
} st_finfo_t;

static st_finfo_t finfo_list[4] = { {FM_NORMAL, 0},
                                    {FM_NORMAL, 1},     // should never be used
                                    {FM_ZLIB, 0},
                                    {FM_ZLIB, 1} };


static st_finfo_t *
get_finfo (FILE *file)
{
  st_finfo_t *finfo = (st_finfo_t *) map_get (fh_map, file);

  if (finfo == NULL)
    {
      fprintf (stderr, "\nINTERNAL ERROR: File pointer was not present in map (%p)\n", file);
      map_dump (fh_map);
      exit (1);
    }
  return finfo;
}


static fmode2_t
get_fmode (FILE *file)
{
  return get_finfo (file)->fmode;
}


FILE *
fopen2 (const char *filename, const char *mode)
{
#undef  fopen
  int n, len = strlen (mode), read = 0, compressed = 0;
  fmode2_t fmode = FM_UNDEF;
  st_finfo_t *finfo;
  FILE *file;

//  printf ("opening %s", filename);
  if (fh_map == NULL)
    {
      fh_map = map_create (20);                 // 20 simultaneous open files
      map_put (fh_map, stdin, &finfo_list[0]);  //  should be enough to start with
      map_put (fh_map, stdout, &finfo_list[0]);
      map_put (fh_map, stderr, &finfo_list[0]);
    }

  for (n = 0; n < len; n++)
    {
      switch (mode[n])
      {
      case 'r':
        read = 1;
      case 'f':
      case 'h':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        fmode = FM_ZLIB;
        break;
      case 'w':
      case 'a':
        fmode = FM_NORMAL;
        break;
      case '+':
        if (fmode == FM_UNDEF)
          fmode = FM_NORMAL;
        break;
      }
    }

  if (fmode == FM_UNDEF)
    return NULL;

  if (read)
    {
#undef  fread
#undef  fclose
      unsigned char magic[3] = { 0 };
      FILE *fh;

      // TODO: check if mode is valid for fopen(), i.e., no 'f', 'h' or number
      if ((fh = fopen (filename, mode)) != NULL)
        {
          fread (magic, sizeof (magic), 1, fh);
          if (magic[0] == 0x1f && magic[1] == 0x8b && magic[2] == 0x08)
            compressed = 1;     // ID1, ID2 and CM. gzip uses Compression Method 8
          else
            /*
              Files that are opened with mode "r+" will probably be written to.
              zlib doesn't support mode "r+", so we have to use FM_NORMAL.
              Mode "r" doesn't require FM_NORMAL and FM_ZLIB works, but we
              shouldn't introduce needless overhead.
            */
            fmode = FM_NORMAL;
          fclose (fh);
        }
#define fread   fread2
#define fclose  fclose2
    }

  if (fmode == FM_NORMAL)
    file = fopen (filename, mode);
  else
    file = gzopen (filename, mode);

  if (file == NULL)
    return NULL;

  finfo = &finfo_list[fmode * 2 + compressed];
  fh_map = map_put (fh_map, file, finfo);

/*
  printf (", ptr = %p, mode = %s, fmode = %s\n", file, mode,
    fmode == FM_NORMAL ? "FM_NORMAL" :
      (fmode == FM_ZLIB ? "FM_ZLIB" : "FM_UNDEF"));
  map_dump (fh_map);
*/
  return file;
#define fopen   fopen2
}


int
fclose2 (FILE *file)
{
#undef  fclose
  fmode2_t fmode = get_fmode (file);

  map_del (fh_map, file);
  if (fmode == FM_NORMAL)
    return fclose (file);
  else if (fmode == FM_ZLIB)
    return gzclose (file);
  else
    return EOF;
#define fclose  fclose2
}


int
fseek2 (FILE *file, long offset, int mode)
{
#undef  fseek
  st_finfo_t *finfo = get_finfo (file);

/*
//  if (fmode != FM_NORMAL)
  printf ("fmode = %s\n", finfo->fmode == FM_NORMAL ? "FM_NORMAL" :
                            (finfo->fmode == FM_ZLIB ? "FM_ZLIB" : "FM_UNDEF"));
*/
  if (finfo->fmode == FM_NORMAL)
    return fseek (file, offset, mode);
  else
    {
      /*
        FUCKING zlib documentation! It took me around 4 hours of debugging time
        to find out that the doc is wrong! From the doc:
          gzrewind(file) is equivalent to (int)gzseek(file, 0L, SEEK_SET)
        That is not true for uncompressed files. gzrewind() doesn't change the
        file pointer for uncompressed files in the ports I tested (zlib 1.1.3,
        DJGPP, Cygwin & GNU/Linux). It clears the EOF indicator.
      */
      if (!finfo->compressed)
        gzrewind (file);
      gzseek (file, offset, mode);
      return gzeof (file);
    }
#define fseek   fseek2
}


size_t
fread2 (void *buffer, size_t size, size_t number, FILE *file)
{
#undef  fread
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fread (buffer, size, number, file);
  else
    return gzread (file, buffer, number * size);
#define fread   fread2
}


int
fgetc2 (FILE *file)
{
#undef  fgetc
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fgetc (file);
  else
    return gzgetc (file);
#define fgetc   fgetc2
}


char *
fgets2 (char *buffer, int maxlength, FILE *file)
{
#undef  fgets
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fgets (buffer, maxlength, file);
  else if (fmode == FM_ZLIB)
    return gzgets (file, buffer, maxlength);
  else
    return NULL;
#define fgets   fgets2
}


int
feof2 (FILE *file)
{
#undef  feof
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return feof (file);
  else
    return gzeof (file);
#define feof    feof2
}


size_t
fwrite2 (const void *buffer, size_t size, size_t number, FILE *file)
{
#undef  fwrite
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fwrite (buffer, size, number, file);
  else
    return gzwrite (file, (void *) buffer, number * size);
#define fwrite  fwrite2
}


int
fputc2 (int character, FILE *file)
{
#undef  fputc
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fputc (character, file);
  else
    return gzputc (file, character);
#define fputc   fputc2
}
#endif // ZLIB


unsigned short int
bswap_16 (unsigned short int x)
{
  unsigned char *ptr = (unsigned char *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[1];
  ptr[1] = tmp;
  return x;
}


unsigned int
bswap_32 (unsigned int x)
{
  unsigned char *ptr = (unsigned char *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[3];
  ptr[3] = tmp;
  tmp = ptr[1];
  ptr[1] = ptr[2];
  ptr[2] = tmp;
  return x;
}


unsigned long long int
bswap_64 (unsigned long long int x)
{
  unsigned char *ptr = (unsigned char *) &x, tmp;
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


#ifdef NETWORK
int
is_address_multicast (unsigned long address)
{
  if ((address & 255) >= 224 && (address & 255) <= 239)
    return 1;
  return 0;
}

#ifndef USE_IPV6

/* =================== IPV4 ================== */

int
tcp_open (char *address, int port)
{
  struct sockaddr_in stAddr;
  struct hostent *host;
  int sock;
  struct linger l;

  memset (&stAddr, 0, sizeof (stAddr));
  stAddr.sin_family = AF_INET;
  stAddr.sin_port = htons (port);

  if ((host = gethostbyname (address)) == NULL)
    return 0;

  stAddr.sin_addr = *((struct in_addr *) host->h_addr_list[0]);

  if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    return 0;

  l.l_onoff = 1;
  l.l_linger = 5;
  if (setsockopt (sock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof (l)) < 0)
    return 0;

  if (connect (sock, (struct sockaddr *) &stAddr, sizeof (stAddr)) < 0)
    return 0;

  return sock;
}


int
udp_open (char *address, int port)
{
  int enable = 1L;
  struct sockaddr_in stAddr;
  struct sockaddr_in stLclAddr;
  struct ip_mreq stMreq;
  struct hostent *host;
  int sock;

  stAddr.sin_family = AF_INET;
  stAddr.sin_port = htons (port);

  if ((host = gethostbyname (address)) == NULL)
    return 0;

  stAddr.sin_addr = *((struct in_addr *) host->h_addr_list[0]);

  /* Create a UDP socket */
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    return 0;

  /* Allow multiple instance of the client to share the same address and port */
  if (setsockopt
      (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &enable,
       sizeof (unsigned long int)) < 0)
    return 0;

  /* If the address is multicast, register to the multicast group */
  if (is_address_multicast (stAddr.sin_addr.s_addr))
    {
      /* Bind the socket to port */
      stLclAddr.sin_family = AF_INET;
      stLclAddr.sin_addr.s_addr = htonl (INADDR_ANY);
      stLclAddr.sin_port = stAddr.sin_port;
      if (bind (sock, (struct sockaddr *) &stLclAddr, sizeof (stLclAddr)) < 0)
        return 0;

      /* Register to a multicast address */
      stMreq.imr_multiaddr.s_addr = stAddr.sin_addr.s_addr;
      stMreq.imr_interface.s_addr = INADDR_ANY;
      if (setsockopt
          (sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &stMreq,
           sizeof (stMreq)) < 0)
        return 0;
    }
  else
    {
      /* Bind the socket to port */
      stLclAddr.sin_family = AF_INET;
      stLclAddr.sin_addr.s_addr = htonl (INADDR_ANY);
      stLclAddr.sin_port = htons (0);
      if (bind (sock, (struct sockaddr *) &stLclAddr, sizeof (stLclAddr)) < 0)
        return 0;
    }

  return sock;
}

#else

/* =================== IPV6 ================== */

int
tcp_open (char *address, int port)
{
  struct addrinfo hints, *res, *res_tmp;
  int sock;
  struct linger l;
  char ipstring[MAX_HOSTNAME];
  char portstring[MAX_HOSTNAME];

  sprintf (portstring, "%d", port);

  memset (&hints, 0, sizeof (hints));
  /*
   * hints.ai_protocol  = 0;
   * hints.ai_addrlen   = 0;
   * hints.ai_canonname = NULL;
   * hints.ai_addr      = NULL;
   * hints.ai_next      = NULL;
   */
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;



  if (getaddrinfo (address, portstring, &hints, &res) != 0)
    return 0;

  for (res_tmp = res; res_tmp != NULL; res_tmp = res_tmp->ai_next)
    {
      if ((res_tmp->ai_family != AF_INET) && (res_tmp->ai_family != AF_INET6))
        continue;
      if ((sock = socket (res_tmp->ai_family, res_tmp->ai_socktype,
                          res_tmp->ai_protocol)) == -1)
        {
          sock = 0;
          continue;
        }

      l.l_onoff = 1;
      l.l_linger = 5;
      if (setsockopt (sock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof (l)) <
          0)
        {
          /* return 0; */
          sock = 0;
          continue;
        }

      if (connect (sock, res_tmp->ai_addr, res_tmp->ai_addrlen) == -1)
        {
          close (sock);
          sock = 0;
          continue;
        }

      ipstring[0] = 0;
      getnameinfo (res_tmp->ai_addr, res_tmp->ai_addrlen, ipstring,
                   sizeof (ipstring), NULL, 0, NI_NUMERICHOST);

      if (ipstring == NULL)
        sock = 0;

      break;
    }

  freeaddrinfo (res);
  return sock;
}


int
udp_open (char *address, int port)
{
  int enable = 1L;
  struct addrinfo hints, *res, *res_tmp;
  int sock;
  struct linger l;
  char ipstring[MAX_HOSTNAME];
  char portstring[MAX_HOSTNAME];
  struct ipv6_mreq imr6;
  struct ip_mreq imr;

  sprintf (portstring, "%d", port);

  memset (&hints, 0, sizeof (hints));
  /*
   * hints.ai_protocol  = 0;
   * hints.ai_addrlen   = 0;
   * hints.ai_canonname = NULL;
   * hints.ai_addr      = NULL;
   * hints.ai_next      = NULL;
   */
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if (getaddrinfo (address, portstring, &hints, &res) != 0)
    return 0;

  for (res_tmp = res; res_tmp != NULL; res_tmp = res_tmp->ai_next)
    {
      if ((res_tmp->ai_family != AF_INET) && (res_tmp->ai_family != AF_INET6))
        continue;
      if ((sock = socket (res_tmp->ai_family, res_tmp->ai_socktype,
                          res_tmp->ai_protocol)) < 0)
        {
          sock = 0;
          continue;
        }
      /* Allow multiple instance of the client to share the same address and port */
      if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
                      (char *) &enable, sizeof (unsigned long int)) < 0)
        {
          sock = 0;
          continue;
        }

      /* If the address is multicast, register to the multicast group */
      if ((res_tmp->ai_family == AF_INET6) &&
          IN6_IS_ADDR_MULTICAST (&
                                 (((struct sockaddr_in6 *) res_tmp->ai_addr)->
                                  sin6_addr)))
        {
          /* Bind the socket to port */
          if (bind (sock, res_tmp->ai_addr, res_tmp->ai_addrlen) < 0)
            {
              close (sock);
              sock = 0;
              continue;
            }

          imr6.ipv6mr_multiaddr =
            ((struct sockaddr_in6 *) res_tmp->ai_addr)->sin6_addr;
          imr6.ipv6mr_interface = INADDR_ANY;
          if (setsockopt (sock,
                          IPPROTO_IPV6,
                          IPV6_ADD_MEMBERSHIP,
                          (char *) &imr6, sizeof (struct ipv6_mreq)) < 0)
            return 0;
        }
      else if ((res_tmp->ai_family == AF_INET) &&
               IN_MULTICAST (ntohl ((((struct sockaddr_in *) res_tmp->
                                      ai_addr)->sin_addr.s_addr))))
        {
          /* Bind the socket to port */
          if (bind (sock, res_tmp->ai_addr, res_tmp->ai_addrlen) < 0)
            {
              close (sock);
              sock = 0;
              continue;
            }

          imr.imr_multiaddr =
            ((struct sockaddr_in *) res_tmp->ai_addr)->sin_addr;
          imr.imr_interface.s_addr = INADDR_ANY;

          if (setsockopt (sock,
                          IPPROTO_IP,
                          IP_ADD_MEMBERSHIP,
                          (char *) &imr, sizeof (struct ip_mreq)) < 0)
            return 0;
        }
      else
        {
          /* Bind the socket to port */
          if (bind (sock, res_tmp->ai_addr, res_tmp->ai_addrlen) < 0)
            {
              close (sock);
              sock = 0;
              continue;
            }
        }
    }

  freeaddrinfo (res);
  return sock;
}

#endif

static int
ftp_get_reply (int tcp_sock)
{
  int i;
  char c;
  char answer[1024];

  do
    {
      /* Read a line */
      for (i = 0, c = 0; i < 1024 && c != '\n'; i++)
        {
          read (tcp_sock, &c, sizeof (char));
          answer[i] = c;
        }
      answer[i] = 0;
      fprintf (stderr, "%s", answer + 4);
    }
  while (answer[3] == '-');

  answer[3] = 0;

  return atoi (answer);
}


int network_open (char *url)
{
  int fd = 0;
  char *p = NULL;
  char host[MAX_HOSTNAME];
//  int port = url_to_host_and_port (host, url);
  int port = 0;
  char http_request[PATH_MAX];
  char filename[PATH_MAX];
  switch (port)
    {
      case 21:
        if (!(fd = tcp_open (host, port))) return -1;
        return fd;
      
      case 80:
      case 8080:
        if (!(fd = tcp_open (host, port))) return -1;
      
        p = strchr (url, '/');

        if (p) while (strchr (p, '/'))
          {
            p = strchr (p, '/');
            p++;
          }
printf ("%s\n",p);
fflush (stdout);  

        strcpy (filename, p);

      /* Send HTTP GET request
         Please don't use a Agent know by shoutcast (Lynx, Mozilla) seems to
         be reconized and print a html page and not the stream */
        snprintf (http_request, sizeof (http_request), "GET /%s HTTP/1.0\r\n"
//          "User-Agent: Mozilla/2.0 (Win95; I)\r\n"
          "Pragma: no-cache\r\n" "Host: %s\r\n" "Accept: */*\r\n" "\r\n",
          filename, host);

        send (fd, http_request, strlen (http_request), 0);

        return fd;
      
      default:
//        if (!(fd = udp_open (host, port))) return -1;
        if (!(fd = tcp_open (host, port))) return -1;
        return fd;
    }

  return -1;
}

#if 0

int
http_read (void *dest, size_t start, size_t len, const char *url)
{
  char *host;
  int port;
  char *request;
  int fd;
  char *p;
  char http_request[PATH_MAX];
  char filename[PATH_MAX];
  char c;

  if (!(fd = network_open (url))) return -1;

  p = strchr (arg, '/');
  if (p)
    {
  while (strchr (p, '/')){ p = strchr (p, '/'); p++;}

  strcpy (filename, p);
  printf ("%s\n", filename);
  fflush (stdout);
}
}





int
ftp_open (char *arg)
{
  char *host;
  int port;
  char *dir;
  char *file;
  int tcp_sock;
  int data_sock;
  char ftp_request[PATH_MAX];
  struct sockaddr_in stLclAddr;
  socklen_t namelen;
  int i;

  if ((dir = strchr (arg, '/')) == NULL)
    return 0;
  *dir++ = 0;
  if ((file = strrchr (dir, '/')) == NULL)
    {
      file = dir;
      dir = NULL;
    }
  else
    *file++ = 0;

  /* Open a TCP socket */
  if (!(tcp_sock = network_open (url))) return -1;

  /* Send FTP USER and PASS request */
  ftp_get_reply (tcp_sock);

  sprintf (ftp_request, "USER anonymous\r\n");
  send (tcp_sock, ftp_request, strlen (ftp_request), 0);

  if (ftp_get_reply (tcp_sock) != 331)
    return 0;

  sprintf (ftp_request, "PASS smpeguser@\r\n");
  send (tcp_sock, ftp_request, strlen (ftp_request), 0);

  if (ftp_get_reply (tcp_sock) != 230)
    return 0;

  sprintf (ftp_request, "TYPE I\r\n");
  send (tcp_sock, ftp_request, strlen (ftp_request), 0);

  if (ftp_get_reply (tcp_sock) != 200)
    return 0;

  if (dir != NULL)
    {
      snprintf (ftp_request, sizeof (ftp_request), "CWD %s\r\n", dir);
      send (tcp_sock, ftp_request, strlen (ftp_request), 0);

      if (ftp_get_reply (tcp_sock) != 250)
        return 0;
    }

  /* Get interface address */
  namelen = sizeof (stLclAddr);
  if (getsockname (tcp_sock, (struct sockaddr *) &stLclAddr, &namelen) < 0)
    return 0;

  /* Open data socket */
  if ((data_sock = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    return 0;

  stLclAddr.sin_family = AF_INET;

  /* Get the first free port */
  for (i = 0; i < 0xC000; i++)
    {
      stLclAddr.sin_port = htons (0x4000 + i);
      if (bind (data_sock, (struct sockaddr *) &stLclAddr, sizeof (stLclAddr))
          >= 0)
        break;
    }
  port = 0x4000 + i;

  if (listen (data_sock, 1) < 0)
    return 0;

  i = ntohl (stLclAddr.sin_addr.s_addr);
  sprintf (ftp_request, "PORT %d,%d,%d,%d,%d,%d\r\n",
           (i >> 24) & 0xFF, (i >> 16) & 0xFF,
           (i >> 8) & 0xFF, i & 0xFF, (port >> 8) & 0xFF, port & 0xFF);
  send (tcp_sock, ftp_request, strlen (ftp_request), 0);
  if (ftp_get_reply (tcp_sock) != 200)
    return 0;

  snprintf (ftp_request, sizeof (ftp_request), "RETR %s\r\n", file);
  send (tcp_sock, ftp_request, strlen (ftp_request), 0);
  if (ftp_get_reply (tcp_sock) != 150)
    return 0;

  return accept (data_sock, NULL, NULL);
}
#endif
#endif // NETWORK
