/*
misc.c - miscellaneous functions

written by 1999 - 2002 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh
                  2002 Jan-Erik Karlsson (Amiga)


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
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>                             // va_arg()
#include <sys/stat.h>

#ifdef  __unix__
#include <unistd.h>
#endif

#ifdef  __CYGWIN__                              // under Cygwin (gcc for Windows) we
#define USE_POLL                                //  need poll() for kbhit(). poll()
#include <sys/poll.h>                           //  is available on Linux, not on
#endif                                          //  BeOS. DOS already has kbhit()

#ifdef  __MSDOS__
#include <dos.h>                                // delay(), milliseconds
#elif   defined __unix__
#include <unistd.h>                             // usleep(), microseconds
#elif   defined __BEOS__
#include <OS.h>                                 // snooze(), microseconds
#endif

#if     (defined __unix__ || defined __BEOS__ || defined AMIGA) && !defined __MSDOS__
#include <termios.h>
typedef struct termios tty_t;
#endif
#include "config.h"                             // ZLIB
#include "misc.h"

#ifdef  ZLIB
#include <zlib.h>
#include "unzip.h"
#endif

#if     defined ANSI_COLOR && defined DJGPP
#include <dpmi.h>                               // needed for __dpmi_int() by ansi_init()
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

#if     (defined __unix__ || defined __BEOS__ || defined AMIGA) && !defined __MSDOS__
static void set_tty (tty_t *param);
#endif

#ifdef  __CYGWIN__
static char *cygwin_fix (char *value);
#endif

void deinit_conio(void);

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
#define CRC32_POLYNOMIAL     0xedb88320L

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


char *
mkprint (char *str, const unsigned char replacement)
{
  char *p = str;

  for (; *p; p++)
    switch (*p)
      {
      case '\n':
        break;

      case '\x1b':                              // escape
        if (misc_ansi_color)
          break;

      default:
        if (iscntrl ((int) *p))
          *p = replacement;
        break;
      }

  return str;
}


char *
mkfile (char *str, const unsigned char replacement)
{
  int pos = 0;
  char *p = str;

  for (; *p && pos < FILENAME_MAX; p++, pos++)
    switch (*p)
      {
      case '.':
      case '-':
      case ' ':
        break;

      default:
        if (!isalnum ((int) *p))
          *p = replacement;
        break;
      }

  *p = 0;

  return str;
}


int
areprint (const char *str, int size)
// like isprint() but for strings
{
  while (size > 0)
    if (!isprint ((int) str[--size]))
      return FALSE;

  return TRUE;
}


int
areupper (const char *str)
// searches the string for ANY lowercase char
{
  for (; *str; str++)
    if (islower ((int) *str))
      return FALSE;

  return TRUE;
}


char *
strupr (char *str)
{
  char *p = str;

  for (; *p; p++)
    *p = toupper (*p);

  return str;
}


char *
strlwr (char *str)
{
  char *p = str;

  for (; *p; p++)
    *p = tolower (*p);

  return str;
}


char *
setext (char *filename, const char *ext)
{
  char ext2[FILENAME_MAX],
       *p = basename2 (filename) ? basename2 (filename) : filename,
       *p2 = NULL;

  if ((p2 = strrchr (p, '.')))
    if (strcmp (p2 ,p) != 0) // some files start with '.'
      *p2 = 0;

  strcpy (ext2, ext);
  strcat (filename, areupper (basename2 (filename)) ? strupr (ext2) : strlwr (ext2));

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
strtrim (char *str)
{
  int x = strlen (str);

  while (x && isspace ((int) str[--x]))
    ;

  str[++x] = 0;

  return str + strspn (str, "\t ");
}


int
memwcmp (const void *add, const void *add_with_wildcards, uint32_t n, int wildcard)
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


#if 0
void *
mem_swap (void *add, uint32_t bit, uint32_t n)
{
  uint32_t pos = 0;
  uint32_t increment = bit / 8;
  unsigned char *a = add, c;

  for (; pos + 1 < n; pos += 2)
    {
      c = a[pos];
      a[pos] = a[pos + 1];
      a[pos + 1] = c;
    }

  return add;
}
#else
void *
mem_swap (void *add, uint32_t n)
{
  uint32_t pos = 0;
  unsigned char *a = add, c;

  for (; pos + 1 < n; pos += 2)
    {
      c = a[pos];
      a[pos] = a[pos + 1];
      a[pos + 1] = c;
    }

  return add;
}
#endif


void
mem_hexdump (const void *mem, uint32_t n, int virtual_start)
//hexdump something
{
  uint32_t pos;
  char buf[MAXBUFSIZE];
  const unsigned char *p = mem;

  buf [0] = 0;
  for (pos = 0; pos < n; pos++, p++)
    {
      if (!(pos % 16))
        printf ("%s%s%08x  ", pos ? buf : "",
                               pos ? "\n" : "",
                               (int) pos + virtual_start);
      printf ("%02x %s", *p, !((pos + 1) % 4) ? " ": "");
#if 1
      sprintf (buf + (pos % 16), "%c", isprint (*p) ? *p : '.');
#else
      *(buf + (pos % 16)) = isprint (*p) ? *p : '.';
      *(buf + (pos % 16) + 1) = 0;
#endif
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
//              if (S_ISREG (fstate.st_mode))
          {
            strcpy (buf, ep->d_name);
            rename (ep->d_name, strlwr (buf));
          }
        }
    }
  (void) closedir (dp);
  return 0;
}


char *
basename2 (const char *str)
// GNU basename() clone
{
  char *p = strrchr (str, FILE_SEPARATOR);

  return p ? p + 1 : (char *) str;
}


void
change_string (char *searchstr, int strsize, char wc, char esc,
               char *newstr, int newsize, char *buf, int bufsize, int offset,
               ...)
/*
  Search for all occurrences of string searchstr in buf and replace newsize
  bytes in buf by copying string newstr to the end of the found search string
  in buf plus offset.
  If searchstr contains wildcard characters wc, then n wildcard characters in
  searchstr match any n characters in buf.
  If searchstr contains escape characters esc, change_string() must be called
  with two extra arguments for each escape character, set, which must be a
  string (char *) and setsize, which must be an int. searchstr matches for an
  escape character if one of the characters in set matches.
  Note that searchstr is not necessarily a C string; it may contain one or more
  zero bytes as strsize indicates the length.
  offset is the relative offset from the last character in searchstring and may
  have a negative value.
  This function was written to patch SNES ROM dumps. It does basically the same
  as the old uCON does, with one exception, the line with:
    bufpos -= nwc;

  As stated in the comment, this causes the search to restart at the first
  wildcard character of the sequence of wildcards that was most recently
  skipped if the current character in buf didn't match the current character
  in searchstr. This makes change_string() behave a bit more intuitive. For
  example
    char str[] = "f foobar means...";
    change_string ("f**bar", 6, '*', '!', "XXXXXXXX", 8, str, strlen (str), 2);
  finds and changes "foobar means..." into "foobar XXXXXXXX", while with uCON's
  algorithm it would not (but does the job good enough for patching SNES ROMs).

  One example of using sets:
    char str[] = "fu-bar     is the same as foobar    ";
    change_string ("f!!", 3, '*', '!', "fighter", 7, str, strlen (str), 1,
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
              memcpy (buf + bufpos + offset, newstr, newsize);
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
              memcpy (buf + bufpos + offset, newstr, newsize);
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
              memcpy (buf + bufpos + offset, newstr, newsize);
              strpos = 0;
            }
          else
            strpos++;
        }
      else
        {
          bufpos -= nwc;                        // scan the most recent wildcards too if
          if (strpos > 0)                       //  the character didn't match
            {
              bufpos--;                         // current char has to be checked, but `for'
              strpos = 0;                       //  increments bufpos
            }
        }
    }

  va_end (argptr);
}


int
gauge (time_t init_time, int pos, int size)
{
#define GAUGE_LENGTH 24LL
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

  p = (GAUGE_LENGTH * pos) / size;
  progress[0] = 0;
  strncat (progress, "========================", p);

#ifdef  ANSI_COLOR
  if (misc_ansi_color)
    {
      progress[p] = 0;
      if (p < GAUGE_LENGTH)
        strcat(progress, "\x1b[31;41m");
    }
#endif

  strncat (&progress[p], "------------------------", GAUGE_LENGTH - p);

  percentage = (100LL * pos) / size;

    printf (
#ifdef ANSI_COLOR
    misc_ansi_color ? "\r%10d Bytes [\x1b[32;42m%s\x1b[0m] %d%%, BPS=%d, " :
#endif
    "\r%10d Bytes [%s] %d%%, BPS=%d, ", pos, progress, percentage, bps);

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
  getenv() suitable for enviroments w/o HOME, TMP or TEMP variables.
  The caller should copy the returned string to it's own memory, because this
  function will overwrite that memory on the next call.
  Note that this function never returns NULL.
*/
char *
getenv2 (const char *variable)
{
#undef getenv
  char *tmp;
  static char value[FILENAME_MAX];

  value[0] = 0;

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
          else if (access ("/tmp/", R_OK | W_OK) == 0)  // trailing slash to force
            strcpy (value, "/tmp");                     //  it to be a directory
          else
            getcwd (value, FILENAME_MAX);
        }
    }

#ifdef __CYGWIN__
  /*
    Under certain circumstances Cygwin's runtime system returns "/" as value of HOME
    while that var has not been set. To specify the root dir a drive letter should be
    used.
  */
  if (!strcmp (variable, "HOME") && !strcmp (value, "/"))
    getcwd (value, FILENAME_MAX);

  return cygwin_fix (value);
#else
  return value;
#endif
#define getenv getenv2
}


const char *
get_property (const char *filename, const char *propname, char *buffer, const char *def)
{
  char buf[MAXBUFSIZE], *p = NULL;
  FILE *fh;

  if ((fh = fopen (filename, "rb")) != 0)
    {
      while (fgets (buf, sizeof buf, fh) != NULL)
        {
          if ((p = strpbrk (buf, "\x0a\x0d")))  // strip any returns
             *p = 0;

          if (*(buf + strspn (buf, "\t ")) == '#')
            continue;

          *(buf + strcspn (buf, "#")) = 0;      // comment at end of a line

          if (!strnicmp (buf, propname, strlen (propname)))
            {
              p = strchr (buf, '=');
              p++;
              strcpy (buffer, p + strspn (p, "\t "));

              fclose (fh);
              return buffer;
            }
        }
      fclose (fh);
    }

  p = getenv2 (propname);
  if (p[0] == 0)                                // getenv2() never returns NULL
    {
      if (def)
        strcpy (buffer, def);
      else
        buffer = (char *) def;                  // buffer won't be changed
    }                                           //  after this func (=ok)
  else
    strcpy (buffer, p);
  return buffer;
}


int
set_property (const char *filename, const char *propname, const char *value)
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

  buf2[0] = 0;

  if ((fh = fopen (filename, "rb")) != 0)
    {
      while (fgets (buf, sizeof buf, fh) != NULL)
        {
          if (!strncasecmp (buf, propname, strlen (propname)))
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

  if ((fh = fopen (filename, "wb")) == NULL)
    return -1;

  result = fwrite (buf2, 1, strlen (buf2), fh);
  fclose (fh);

//  q_fwrite (buf2, 0, strlen (buf2), filename, "wb");

  return result;
}


int
rmdir2 (const char *path)
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
          if (strcmp (ep->d_name, "..") != 0 && strcmp (ep->d_name, ".") != 0)
            rmdir2 (ep->d_name);
        }
      else
        remove (ep->d_name);
    }

  closedir (dp);
  chdir (cwd);

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

  temp[0] = 0;
  while (!temp[0] || !access (temp, F_OK))      // must work for files AND dirs
    sprintf (temp, "%s%s%08x.tmp", p, FILE_SEPARATOR_S, rand());

  return temp;
}


char *
tmpnam3 (char *temp, int type)
// tmpnam() clone
{
  FILE *fh;
  tmpnam2 (temp);

// create file or dir in the same moment to prevent double usage
  switch (type)
    {
      case TYPE_DIR:
        mkdir (temp, S_IRUSR|S_IWUSR);
        break;

      case TYPE_FILE:
      default: // a file is the default
        if ((fh = fopen (temp, "wb+")) != 0)
          fclose (fh);
        break;
    }

  return temp;
}


#if     defined __unix__ || defined __BEOS__ || defined AMIGA
#ifndef __MSDOS__
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
#endif


void
init_conio (void)
{
#ifndef __MSDOS__
/*
  This code compiles with DJGPP, but is not neccesary. Our kbhit() conflicts
  with DJGPP's one, so it won't be used for that function. Perhaps it works
  for making getchar() behave like getch(), but that's a bit pointless.
*/
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
#endif
}


void
deinit_conio (void)
{
#ifndef __MSDOS__
  if (oldtty_set)
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &oldtty);
#endif
}


#if     defined __CYGWIN__ && !defined USE_POLL
#warning kbhit() does not work properly in Cygwin executable if USE_POLL is not defined
#endif
#ifndef __MSDOS__                               // this kbhit() conflicts with DJGPP's one
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
  set_tty(&tmptty);                             //  Cygwin (define USE_POLL)

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
#endif                                          // !__MSDOS__
#endif                                          // __unix__ || __BEOS__


#if     defined __unix__ && !defined __MSDOS__
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
  Currently we never write compressed data. That is, no code fopen()s files
  with a mode string containing 'f', 'h' or a number, but writing compressed
  output does work.
*/
st_map_t *fh_map = NULL;                        // associative array: file handle -> file mode

typedef enum { FM_NORMAL, FM_GZIP, FM_ZIP, FM_UNDEF } fmode2_t;

typedef struct st_finfo
{
  fmode2_t fmode;
  int compressed;
} st_finfo_t;

static st_finfo_t finfo_list[6] = { {FM_NORMAL, 0},
                                    {FM_NORMAL, 1},     // should never be used
                                    {FM_GZIP, 0},
                                    {FM_GZIP, 1},
                                    {FM_ZIP, 0},        // should never be used
                                    {FM_ZIP, 1} };

int unzip_current_file_nr = 0;


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


int
unzip_get_number_entries (const char *filename)
{
  FILE *file;
  unsigned char magic[4] = { 0 };

#undef  fopen
#undef  fread
#undef  fclose
  if ((file = fopen (filename, "rb")) == NULL)
    {
      errno = ENOENT;
      return -1;
    }
  fread (magic, 1, sizeof (magic), file);
  fclose (file);
#define fopen   fopen2
#define fclose  fclose2
#define fread   fread2

  if (magic[0] == 'P' && magic[1] == 'K' && magic[2] == 0x03 && magic[3] == 0x04)
    {
      unz_global_info info;

      file = unzOpen (filename);
      unzGetGlobalInfo (file, &info);
      unzClose (file);
      return info.number_entry;
    }
  else
    return -1;
}


int
unzip_goto_file (unzFile file, int file_index)
{
  int retval = unzGoToFirstFile (file), n = 0;

  if (file_index > 0)
    while (n < file_index)
      {
        retval = unzGoToNextFile (file);
        n++;
      }
  return retval;
}


static void
unzip_seek_helper (FILE *file, int offset)
{
  char buffer[MAXBUFSIZE];
  int n, pos = unztell (file);                  // returns ftell() of the "current file"

  if (pos == offset)
    return;
  else if (pos > offset)
    {
      unzCloseCurrentFile (file);
      unzip_goto_file (file, unzip_current_file_nr);
      unzOpenCurrentFile (file);
      pos = 0;
    }
  n = offset - pos;
  while (n > 0 && !unzeof (file))
    n -= unzReadCurrentFile (file, buffer, n > MAXBUFSIZE ? MAXBUFSIZE : n);
}


FILE *
fopen2 (const char *filename, const char *mode)
{
#undef  fopen
  int n, len = strlen (mode), read = 0, compressed = 0;
  fmode2_t fmode = FM_UNDEF;
  st_finfo_t *finfo;
  FILE *file = NULL;

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
        break;
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
        fmode = FM_GZIP;
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

  if (read)
    {
#undef  fread
#undef  fclose
      unsigned char magic[4] = { 0 };
      FILE *fh;

      // TODO: check if mode is valid for fopen(), i.e., no 'f', 'h' or number
      if ((fh = fopen (filename, mode)) != NULL)
        {
          fread (magic, sizeof (magic), 1, fh);
          if (magic[0] == 0x1f && magic[1] == 0x8b && magic[2] == 0x08)
            {                           // ID1, ID2 and CM. gzip uses Compression Method 8
              fmode = FM_GZIP;
              compressed = 1;
            }
          else if (magic[0] == 'P' && magic[1] == 'K' &&
                   magic[2] == 0x03 && magic[3] == 0x04)
            {
              fmode = FM_ZIP;
              compressed = 1;
            }
          else
            /*
              Files that are opened with mode "r+" will probably be written to.
              zlib doesn't support mode "r+", so we have to use FM_NORMAL.
              Mode "r" doesn't require FM_NORMAL and FM_GZIP works, but we
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
  else if (fmode == FM_GZIP)
    file = gzopen (filename, mode);
  else if (fmode == FM_ZIP)
    {
      file = unzOpen (filename);
      if (file != NULL)
        {
          unzip_goto_file (file, unzip_current_file_nr);
          unzOpenCurrentFile (file);
        }
    }

  if (file == NULL)
    return NULL;

  finfo = &finfo_list[fmode * 2 + compressed];
  fh_map = map_put (fh_map, file, finfo);

/*
  printf (", ptr = %p, mode = %s, fmode = %s\n", file, mode,
    fmode == FM_NORMAL ? "FM_NORMAL" :
      (fmode == FM_GZIP ? "FM_GZIP" :
        (fmode == FM_ZIP ? "FM_ZIP" : "FM_UNDEF")));
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
  else if (fmode == FM_GZIP)
    return gzclose (file);
  else if (fmode == FM_ZIP)
    {
      unzCloseCurrentFile (file);
      return unzClose (file);
    }
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
                            (finfo->fmode == FM_GZIP ? "FM_GZIP" :
                              (finfo->fmode == FM_ZIP ? "FM_ZIP" : "FM_UNDEF")));
*/

  if (finfo->fmode == FM_NORMAL)
    return fseek (file, offset, mode);
  else if (finfo->fmode == FM_GZIP)
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
  else if (finfo->fmode == FM_ZIP)
    {
      int base;
      if (mode != SEEK_SET && mode != SEEK_CUR && mode != SEEK_END)
        {
          errno = EINVAL;
          return -1;
        }
      if (mode == SEEK_SET)
        base = 0;
      else if (mode == SEEK_CUR)
        base = unztell (file);
      else // mode == SEEK_END
        {
          unz_file_info info;

          unzip_goto_file (file, unzip_current_file_nr);
          unzGetCurrentFileInfo (file, &info, NULL, 0, NULL, 0, NULL, 0);
          base = info.uncompressed_size;
        }
      unzip_seek_helper (file, base + offset);
      return unzeof (file);
    }
  return -1;
#define fseek   fseek2
}


size_t
fread2 (void *buffer, size_t size, size_t number, FILE *file)
{
#undef  fread
  fmode2_t fmode = get_fmode (file);

  if (size == 0 || number == 0)
    return 0;

  if (fmode == FM_NORMAL)
    return fread (buffer, size, number, file);
  else if (fmode == FM_GZIP)
    {
      int n = gzread (file, buffer, number * size);
      return n / size;
    }
  else if (fmode == FM_ZIP)
    {
      int n = unzReadCurrentFile (file, buffer, number * size);
      return n / size;
    }
  return 0;
#define fread   fread2
}


int
fgetc2 (FILE *file)
{
#undef  fgetc
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fgetc (file);
  else if (fmode == FM_GZIP)
    return gzgetc (file);
  else if (fmode == FM_ZIP)
    {
      char c;
      int retval = unzReadCurrentFile (file, &c, 1);
      return retval <= 0 ? EOF : c & 0xff;      // avoid sign bit extension
    }
  else
    return EOF;
#define fgetc   fgetc2
}


char *
fgets2 (char *buffer, int maxlength, FILE *file)
{
#undef  fgets
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fgets (buffer, maxlength, file);
  else if (fmode == FM_GZIP)
    {
      char *retval = gzgets (file, buffer, maxlength);
      return retval == Z_NULL ? NULL : retval;
    }
  else if (fmode == FM_ZIP)
    {
      int n = 0, c = 0;
      while (n < maxlength - 1 && (c = fgetc (file)) != EOF)
        {
          buffer[n] = c;                        // '\n' must also be stored in buffer
          n++;
          if (c == '\n')
            {
              buffer[n] = 0;
              break;
            }
        }
      if (n >= maxlength - 1 || c == EOF)
        buffer[n] = 0;
      return n > 0 ? buffer : NULL;
    }
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
  else if (fmode == FM_GZIP)
    return gzeof (file);
  else if (fmode == FM_ZIP)
    return unzeof (file);                       // returns feof() of the "current file"
  else
    return -1;
#define feof    feof2
}


size_t
fwrite2 (const void *buffer, size_t size, size_t number, FILE *file)
{
#undef  fwrite
  fmode2_t fmode = get_fmode (file);

  if (size == 0 || number == 0)
    return 0;

  if (fmode == FM_NORMAL)
    return fwrite (buffer, size, number, file);
  else if (fmode == FM_GZIP)
    {
      int n = gzwrite (file, (void *) buffer, number * size);
      return n / size;
    }
  else
    return 0;                                   // writing to zip files is not supported
#define fwrite  fwrite2
}


int
fputc2 (int character, FILE *file)
{
#undef  fputc
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fputc (character, file);
  else if (fmode == FM_GZIP)
    return gzputc (file, character);
  else
    return EOF;                                 // writing to zip files is not supported
#define fputc   fputc2
}
#endif // ZLIB


unsigned short int
bswap_16 (unsigned short int x)
{
#if 1
  unsigned char *ptr = (unsigned char *) &x, tmp;
  tmp = ptr[0];
  ptr[0] = ptr[1];
  ptr[1] = tmp;
  return x;
#else
  return (((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8);
#endif
}


unsigned int
bswap_32 (unsigned int x)
{
#if 1
  unsigned char *ptr = (unsigned char *) &x, tmp;
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


void
wait2 (int nmillis)
{
#ifdef  __MSDOS__
  delay (nmillis);
#elif   defined __unix__ || defined AMIGA
  usleep (nmillis * 1000);
#elif   defined __BEOS__
  snooze (nmillis * 1000);
#endif
}
