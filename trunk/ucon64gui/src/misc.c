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
extern int errno;
#include <time.h>
#include <stdarg.h>                             // va_arg()
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

//#include "config.h"
#include "misc.h"

static st_func_node_t func_list = { NULL, NULL };

#if     defined __unix__ || defined __BEOS__
static void deinit_conio (void);
static void set_tty (tty_t param);
#endif

#ifdef __CYGWIN__
static char *cygwin_fix (char *value);
#endif

static unsigned long crc_table[256];
static int crc_table_built = 0;

static void
build_crc_table (void)
{
  int i, j;
  unsigned long crc;
#define CRC32_POLYNOMIAL     0xEDB88320L

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
      crc_table[i] = crc;
    }
  crc_table_built = 1;
}


static unsigned long
calculate_buffer_crc (unsigned int size, unsigned long crc, void *buffer)
// zlib: crc32 (crc, buffer, size);
{
  unsigned char *p;
  unsigned long temp1, temp2;

  if (!crc_table_built)
    build_crc_table ();

  crc ^= 0xFFFFFFFFL;
  p = (unsigned char *) buffer;
  while (size-- != 0)
    {
      temp1 = (crc >> 8) & 0x00FFFFFFL;
      temp2 = crc_table[((int) crc ^ *p++) & 0xff];
      crc = temp1 ^ temp2;
    }
  return crc ^ 0xFFFFFFFFL;
}


static unsigned long
calculate_file_crc (FILE * file)
{
  unsigned long count, crc = 0;
  unsigned char buffer[512];

  while ((count = fread (buffer, 1, 512, file)))
    crc = calculate_buffer_crc (count, crc, buffer); // zlib: crc32 (crc, buffer, count);
  return crc;
}


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
int
strnicmp (const char *s1, const char *s2, size_t n)
{
  int result = 0;
  char *sb1, *sb2;

  if (!strcmp (s1, s2))
    return 0;

  if (!(sb1 = (char *) malloc (strlen(s1) * sizeof (char)))) return -1;

  if (!(sb2 = (char *) malloc (strlen(s2) * sizeof (char))))
    {
      free (sb1);
      return -1;
    }

  strcpy(sb1, s1);
  strcpy(sb2, s2);

  result = strncmp (strlwr(sb1), strlwr(sb2), n);

  free (sb1);
  free (sb2);

  return result;
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
  int len = strlen (str) - 1;
  int len2 = strlen (str2);

  while (len > -1)
    if (!strncmp (&str[len], str2, len2)) return len;
    else len--;
    
  return strlen (str);
}


char *
strupr (char *str)
{
  char *str1 = str;

  while (*str)
    {
      *str = toupper (*str);
      str++;
    }

  return str1;
}


char *
strlwr (char *str)
{
  char *str1 = str;

  while (*str)
    {
      *str = tolower (*str);
      str++;
    }

  return str1;
}


char *
setext (char *filename, const char *ext)
{
//  int pos = strrcspn (filename, ".");
  char ext2[FILENAME_MAX];
  int pos = ((strchr (&filename[1], FILE_SEPARATOR) == NULL) ||
         (strrcspn (filename, ".") > (strrcspn (filename, FILE_SEPARATOR_S) + 1 ))) ?
    strrcspn (filename, ".") : strlen (filename);

//  if (filename[pos - 1] != FILE_SEPARATOR) // some files might start with a dot (.)
  filename[pos] = 0;

  strcpy (ext2, ext);
  strcat (filename, findlwr (FILENAME_ONLY (filename)) ? strlwr (ext2) : strupr (ext2));

  return filename;
}


#ifndef __GNUC__
const char *
getext (const char *filename)
{
  char *p = NULL;

  if (!(p = strrchr (filename, FILE_SEPARATOR)))
    p = filename;
  if (!(p = strrchr (p, '.')))
    p = "";
  
  return p;
}
#endif


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
  register long x = strlen (str) - 1;

  while (x && str[x] == 32)
    x--;
  str[x+1]=0;

  return stpblk (str);
}


char *
stplcr (char *str)
{
  char *str2 = str;

  while (*str != 0x00 && *str != 0x0a && *str != 0x0d)
    str++;
  *str = 0;

  return str2;
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


unsigned long
memcrc32 (const void *add, size_t n)
{
  unsigned long crc = 0;

  return calculate_buffer_crc (n, crc, (unsigned char *) add);
}


void *
memswap (void *add, size_t n)
{
  register size_t pos = 0;
  unsigned char *a = add, c;

  while (pos + 1 < n)
    {
      c = (int) a[pos];
      a[pos] = a[pos + 1];
      a[pos + 1] = c;
      
      pos += 2;
    }

  return add;
}


void
memhexdump (const void *add, size_t n, long virtual_start)
{
  register size_t x;
  char buf[MAXBUFSIZE];
  const unsigned char *a = add;

  for (x = 0; x < n; x++)
    {
      if (!(x % 16))
        printf ("%s%s%08lx  ", x?buf:"", x?"\n":"", x + virtual_start);
      printf ("%02x %s", ((char)a[x]) & 0xff, !((x + 1) % 4)?" ":"");
      sprintf (&buf[x % 16], "%c", isprint ((char)a[x])?((char)a[x]):'.');
    }
  printf ("%s\n", buf);
}


int
renlwr (const char *dir, int lower)
{
  struct dirent *ep;
  struct stat puffer;
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
      if (!stat (ep->d_name, &puffer))
        {
//              if(S_ISREG(puffer.st_mode)==1)
          {
            strcpy (buf, ep->d_name);
            rename (ep->d_name,
                    lower ? strlwr (buf) : strupr (buf));
          }
        }
    }
  (void) closedir (dp);
  return 0;
}


long
quickftell (const char *filename)
{
  struct stat puffer;

  if (!stat (filename, &puffer))
    return ((long) puffer.st_size);

  errno = ENOENT;
  return -1;
}


unsigned long
file_crc32 (const char *filename, long start)
{
  unsigned long val;
  FILE *fh;

  build_crc_table ();

  if (!(fh = fopen (filename, "rb")))
    return -1;
  fseek (fh, start, SEEK_SET);
  val = calculate_file_crc (fh);
  fclose (fh);

  return val;
}


long
filencmp (const char *filename, long start, long len, const char *search,
  long searchlen, int wildcard)
{
  int seg_len = 0; // segment length
  int seg_pos = 0; // position in segment
  size_t size = quickftell (filename);
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

      if (!quickfread (buf, start, seg_len, filename))
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
  register long x, y;
  size_t size = quickftell (filename);
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

  if (!quickfread (buf, start, size, filename))
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
quickfread (const void *dest, size_t start, size_t len, const char *src)
{
  size_t result = 0;
  long size = quickftell (src);
  FILE *fh = fopen (src, "rb");

  if (!fh)
    {
      errno = ENOENT;
      return -1;
    }

#if 1
  if ((size - start) < len)
    {
      memset ((void *) dest, 0, len);
      len = size - start;
    }
#endif

  fseek (fh, start, SEEK_SET);
  result = fread ((void *) dest, 1, len, fh);
  fclose (fh);
#if 0
  dest[len]=0;
#endif
  return result;
}


size_t
quickfwrite (const void *src, size_t start, size_t len, const char *dest, const char *mode)
{
  size_t result = 0;
  FILE *fh = fopen (dest, mode);

  if (!fh)
    {
      errno = ENOENT;
      return -1;
    }
  fseek (fh, start, SEEK_SET);

  if (!src)//then write 0x00
    {
      char buf[MAXBUFSIZE];
      memset (buf, 0, MAXBUFSIZE);
      
      while (len >= MAXBUFSIZE)
        {
          result = fwrite (buf, 1, MAXBUFSIZE, fh);
          len -= MAXBUFSIZE;
        }

      if (len)
        result = fwrite (buf, 1, len, fh);
    }
  else
    result = fwrite (src, 1, len, fh);

  fclose (fh);
  return result;
}


int
quickfgetc (const char *filename, long pos)
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
quickfputc (const char *filename, long pos, int c, const char *mode)
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
filehexdump (const char *filename, long start, long len)
{
  register long x;
  long size = quickftell (filename);
  char buf[MAXBUFSIZE];
  int dump;
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
      printf ("%02x %s", dump & 0xff, !((x + 1) % 4)?" ":"");
      sprintf (&buf[x % 16], "%c", isprint ((char) dump) ? ((char) dump) : '.');
    }
  printf ("%s\n", buf);
  fclose (fh);

  return 0;
}


int
filecopy (const char *src, long start, long len, const char *dest, const char *mode)
{
  int seg_len = 0;
  long size = quickftell (src);
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


const char *
filebackup (char *move_name, const char *filename)
{
  char buf[FILENAME_MAX];

  if (access (filename, R_OK) != 0)
    return filename;

  strcpy (buf, filename);
#ifdef __MSDOS__
  setext (buf, ".BAK");
#else
  strcat (buf, (findlwr (FILENAME_ONLY (buf)) ? ".bak" : ".BAK"));
#endif
  if (strcmp (filename, buf))
    {
      remove (buf);                             // try to remove or rename will fail
      rename (filename, buf);                   // keep file attributes like date, etc.
    }

  if (move_name == NULL)
    {
      filecopy (buf, 0, quickftell (buf), filename, "wb");
      sync ();
      return filename;
    }

  strcpy (move_name, buf);
  return move_name;
}


#if 0
const char *
filenameonly (const char *str)
{
  const char *str2;

  if (!(str2 = strrchr (str, FILE_SEPARATOR)))
    str2 = str;                                 // strip path if it is a filename
  else
    str2++;

  return str2;
}
#endif


#if 0
unsigned long
filefile (const char *filename, long start, const char *filename2, long start2, int similar)
{
  int fsize = quickftell (filename);
  int fsize2 = quickftell (filename2);
  int pos = 0;
  int base, len, chunksize, chunksize2, readok = 1,
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
              memhexdump (&buf[pos], len, start + pos + bytesread2);
              printf ("%s:\n", filename2);
              memhexdump (&buf2[pos], len, start2 + pos + bytesread2);
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

  fsize1 = quickftell (filename1);              // quickftell() returns size in bytes
  fsize2 = quickftell (filename2);
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
                      memhexdump (&buf1[base], len, start1 + base + bytesread2);
                      printf ("%s:\n", filename2);
                      memhexdump (&buf2[base], len, start2 + base + bytesread2);
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
filereplace (const char *filename, long start, const char *search, long slen,
             const char *replace, long rlen)
{
  long size = quickftell (filename);

  if (access (filename, R_OK) != 0)
    {
      errno = ENOENT;
      return -1;
    }

  for (;;)
    {
      if ((start = filencmp (filename, start, size, search, slen, -1)) == -1)
        return 0;
      filehexdump (filename, start, slen);
      quickfwrite (replace, start, rlen, filename, "r+b");
      filehexdump (filename, start, rlen);
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
  int seg_len = 0;
  size_t size = quickftell (filename);
  FILE *fh;
  char buf[MAXBUFSIZE];

  if (!(fh = fopen (filename, "r+b")))
    {
      errno = ENOENT;
      return -1;
    }

  if ((size - start) < len) len = size - start;

  fseek (fh, start, SEEK_SET);

  while (1)
    {
      seg_len = (len >= MAXBUFSIZE) ? MAXBUFSIZE : len;
      
      if (!fread (buf, 1, seg_len, fh)) break;

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
gauge (time_t init_time, long pos, long size)
{
  long bps, value = 0;
  int p, percentage;
  time_t curr, left;
  char progress[24 + 1];
#if 0
  static time_t init_time = 0;
  if (!init_time)
    init_time = time (0);
#endif

#if 0
  if (pos > size)
    return -1;
#else
  if (pos > size)                               // When is this necessary?
    {                                           // DON'T fix other code's bugs
      value = pos;                              //  here!
      pos = size;
      size = value;
    }
#endif

  if ((curr = time (0) - init_time) == 0)
    curr = 1;                                   // `round up' to at least 1 sec (no division
                                                //  by zero below)

  bps = pos / curr;                             // # bytes/second (average transfer speed)
  left = size - pos;
#ifdef __GNUC__
  left /= bps ? : 1;
#else
  left /= bps ? bps : 1;
#endif // __GNUC__

  p = (24 * pos) / size;                        // DON'T make precision worse,
  progress[0] = 0;                              //  by shifting pos or size
  strncat (progress, "========================", p);
  strncat (&progress[p], "------------------------", 24 - p);

  percentage = (100 * (pos >> 10)) / (size >> 10);
  printf ("\r%10ld Bytes [%s] %d%%, BPS=%lu, ",
          pos, progress, percentage, (unsigned long) bps);

  if (pos == size)
    printf ("TOTAL=%03ld:%02ld", (long) curr / 60, (long) curr % 60); // DON'T print a newline
  else                                                                //  -> gauge can be cleared
    printf ("ETA=%03ld:%02ld   ", (long) left / 60, (long) left % 60);

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
  the caller is responsible for freeing memory of string
*/
char *
getenv2 (const char *variable)
{
  char *tmp, *dirname;

  dirname = (char *) malloc (FILENAME_MAX);     // ALWAYS use dirname, so that the
                                                //  caller can always use free()
  if ((tmp = getenv (variable)) != NULL)
    strcpy (dirname, tmp);
  else
    {
      if (!strcmp (variable, "HOME"))
        {
          if ((tmp = getenv (variable)) != NULL)
            strcpy (dirname, tmp);
          else if ((tmp = getenv ("USERPROFILE")) != NULL)
            strcpy (dirname, tmp);
          else if ((tmp = getenv ("HOMEDRIVE")) != NULL)
            {
              strcpy (dirname, tmp);
              strcat (dirname, getenv ("HOMEPATH"));
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
              getcwd (dirname, FILENAME_MAX);
              c = toupper (dirname[0]);
              // if current dir is root dir strip problematic ending slash (DJGPP)
              if (c >= 'A' && c <= 'Z' &&
                  dirname[1] == ':' && dirname[2] == '/' && dirname[3] == 0)
                dirname[2] = 0;
            }
         }

      if (!strcmp (variable, "TEMP") || !strcmp (variable, "TMP"))
        {
          if ((tmp = getenv (variable)) != NULL)
            strcpy (dirname, tmp);
          else
            getcwd (dirname, FILENAME_MAX);
        }
    }

#ifdef __CYGWIN__
  return cygwin_fix (dirname);
#else
  return dirname;
#endif
}


const char *
getProperty (const char *filename, const char *propname, char *buffer, const char *def)
{
  char buf[MAXBUFSIZE], *result;
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
              result = strchr (buf, '=');
              result++;
              strcpy (buffer, stpblk (result));

              fclose (fh);
              return buffer;
            }
        }
      fclose (fh);
    }

  return getenv2 (propname) ?
#ifdef __GNUC__
    getenv2 (propname)
#endif // __GNUC__
    : def;
}


int
setProperty (const char *filename, const char *propname, const char *value)
{
  int found=0;
  char buf[MAXBUFSIZE], *buf2;
  FILE *fh;

  if (!(buf2 = (char *) malloc ((quickftell(filename)
                                         + MAXBUFSIZE) * sizeof (char))))
    {
      errno = ENOMEM;
      return -1;
    }

  buf2[0]=0;

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

    quickfwrite (buf2, 0, strlen (buf2), filename, "wb");

    return 0;
}


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
#endif                                          //  (that is, under Linux & BeOS)
/*
  // Snes9x (Linux) for example returns a non-zero value on a normal exit (3)...

  // under WinDOS, system() immediately returns with exit code 0 when starting
  //  a Windows executable (as if a fork() happened) it also returns 0 when the
  //  exe could not be started
*/
    ;
  if (!(fh = popen (cmdline, "r"))) return -1;

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
  struct stat puffer;
  DIR *dp;

  if (!(dp = opendir (path))) return -1;

  getcwd (cwd, FILENAME_MAX);
  chdir (path);

  while ((ep = readdir (dp)) != NULL)
    {
      if (stat (ep->d_name, &puffer) == -1) return -1;

      if (S_ISDIR (puffer.st_mode))
        {
          if (strcmp (ep->d_name, "..") != 0 &&
            strcmp (ep->d_name, ".") != 0) rmdir_R (ep->d_name);
        }
      else remove (ep->d_name);
    }

  (void) closedir (dp);
  chdir (cwd);

  return rmdir (path);
}


char *
tmpnam2 (char *temp) // tmpnam() replacement
{
  temp[0] = 0;
 
  while (!temp[0] || !access (temp, F_OK))
    sprintf (temp, "%s" FILE_SEPARATOR_S "%08x.tmp", getenv2 ("TEMP"),
      RANDOM (0x10000000, 0xffffffff));

  return temp;
}


char *
html_parser (const char *filename, char *buffer)
{
  int tag = 0;
  char c = 0;
  char buf[2];
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


int
cmd2args (char **argv, const char *cmdline)
{
  char buf[MAXBUFSIZE];
  int argc = 0;
  
  if (cmdline)
    if (cmdline[0])
      {
  strcpy (buf, cmdline);

  while ((argv[argc] = strtok (!argc?buf:NULL, " ")) && argc < (ARGS_MAX - 1))
    argc++;
      }

  return argc;
}


char *
query2cmd (char *str, const char *uri, const char *query)
{
  register int x = 0;
  int c = 0, len = query?strlen(query):0;
  char buf[16], *p = NULL;
  
  if (!str) str = (char *) malloc ((strlen (uri) + len) * 2); //* 2 should be enough
  
  strcpy (str, uri);  //the uri is argv[0]

  if (!len) return str;

  strcat (str, " ");

  for (x = 0; x < len; x++)
    {
      p = NULL;
      switch (query[x])
        {
//          case '?':
          case '&':
          case '+':
            p = " ";
            break;

          case '%':
            sscanf (&query[x+1], "%02x", &c);
            sprintf (buf, "%c", c);
            x += 2;
            break;

          default:
            sprintf (buf, "%c", query[x]);
            break;
        }

        strcat (str, p?
#ifdef __GNUC__
        p
#endif // __GNUC__
        :buf);
     }

  return str;
}


char *
tag2cmd (char *str, const char *uri, const char *query)
{
  register int x = 0;
  int c = 0, len = query?strlen(query):0;
  char buf[16], *p = NULL;
  
  if (!str) str = (char *) malloc ((strlen (uri) + len) * 2); //* 2 should be enough

  strcpy (str, uri);  //the uri is argv[0]

  if (!len) return str;

  strcat (str, " ");

  for (x = 0; x < len; x++)
    {
      p = NULL;
      switch (query[x])
        {
//          case '?':
          case '&':
          case '+':
            p = " ";
            break;

          case '%':
            sscanf (&query[x+1], "%02x", &c);
            sprintf (buf, "%c", c);
            x += 2;
            break;

          default:
            sprintf (buf, "%c", query[x]);
            break;
        }

        strcat (str, p?
#ifdef __GNUC__
        p
#endif // __GNUC__
        :buf);
     }

  return str;
}


#if     defined __unix__ || defined __BEOS__
static int stdin_tty = 1;                       // 1 => stdin is a tty, 0 => it's not
static tty_t oldtty, newtty;


void
set_tty (tty_t param)
{
  if (stdin_tty && tcsetattr (STDIN_FILENO, TCSANOW, &param) == -1)
    {
      fprintf (stderr, "Could not set tty parameters\n");
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
      fprintf (stderr, "Could not get tty parameters\n");
      exit (101);
    }

  if (atexit (deinit_conio) == -1)
    {
      fprintf (stderr, "Could not register function with atexit()\n");
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
  set_tty(tmptty);                              //   Cygwin (define USE_POLL)

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
#endif


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

  prev_node->next = func_node->next;
  free (func_node);
  return 0;
}


void
handle_registered_funcs (void)
{
  st_func_node_t *func_node = &func_list;

  while (func_node->next != NULL)
    {
      func_node = func_node->next;              // first node contains no func
      if (func_node->func != NULL)
        func_node->func ();
    }
}


#ifdef _LIBC
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif
#endif


unsigned short bswap_16(unsigned short x)
{
#ifdef WORDS_BIGENDIAN
#ifndef __i386__
  return (((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8);
#else
  __asm("xchgb %b0,%h0"	:
        "=q" (x)	:
        "0" (x));
#endif // __i386__
#endif // WORDS_BIGENDIAN
  return x;
}


unsigned int bswap_32(unsigned int x)
{
#ifdef WORDS_BIGENDIAN
#ifndef __i386__
  return ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |
    (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24));
#else
#if __CPU__ > 386
 __asm("bswap	%0":
      "=r" (x)     :
#else
 __asm("xchgb	%b0,%h0\n"
      "	rorl	$16,%0\n"
      "	xchgb	%b0,%h0":
      "=q" (x)		:
#endif
      "0" (x));
#endif // __i386__
#endif // WORDS_BIGENDIAN
  return x;
}


unsigned long long int bswap_64(unsigned long long int x)
{
#ifdef WORDS_BIGENDIAN
#ifndef __i386__
  return
     (__extension__
      ({ union { __extension__ unsigned long long int __ll;
                 unsigned long int __l[2]; } __w, __r;
         __w.__ll = (x);
         __r.__l[0] = bswap_32 (__w.__l[1]);
         __r.__l[1] = bswap_32 (__w.__l[0]);
         __r.__ll; }));
#else
  register union { __extension__ unsigned long long int __ll;
          unsigned long int __l[2]; } __x;
  asm("xchgl	%0,%1":
      "=r"(__x.__l[0]),"=r"(__x.__l[1]):
      "0"(bswap_32((unsigned long)x)),"1"(bswap_32((unsigned long)(x>>32))));
  return __x.__ll;
#endif // __i386__
#else
  return x; 
#endif // WORDS_BIGENDIAN  
}
