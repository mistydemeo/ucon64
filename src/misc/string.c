/*
string.c - some string functions

Copyright (c) 1999 - 2004 NoisyB
Copyright (c) 2001 - 2004 dbjh


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
#include "config.h"
#endif
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "string.h"


#if 0
static int
is_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  /*
    Casting to unsigned char * is necessary to avoid differences between the
    different compilers' run-time environments. At least for isprint(). Without
    the cast the isprint() of (older versions of) DJGPP, MinGW, Cygwin and
    Visual C++ returns nonzero values for ASCII characters > 126.
  */
  for (; len >= 0; p++, len--)
    if (!func (*(unsigned char *) p))
      return 0;

  return 1;
}


static char *
to_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = func (*p);

  return s;
}
#endif


/*
unsigned char *
strutf8 (const char *s)
{
  static unsigned char *d = NULL;
  unsigned char *p = NULL;

  if (d)
    free (d);

  d = malloc ((strlen (s) + 1) * 3);

  if (!d)
    return NULL;

  p = d;

//  strcpy (p, s);
//  strrep (p, "\r\n", "\n");

  for (; *s; s++, p++)
    {
//      if (*s == '\r')
//        continue;

      if (*s < 128)
        *p = *s;
      else if (*s > 127 && *s < 2048)
        {
          *p = ((*s) >> 6) | 192;
          p++;
          *p = ((*s) & 63) | 128;
        }
      else
        {
          *p = ((*s) >> 12) | 224;
          p++;
          *p = (((*s) >> 6) & 63) | 128;
          p++;
          *p = ((*s) & 63) | 128;
        }
    }

  return d;
}


char *
utf8str (const unsigned char *s)
{
  static char *d = NULL;
  char *p = NULL;

  if (d)
    free (d);

  d = malloc (strlen (s));

  if (!d)
    return NULL;

  p = d;

  for (; *s; s++, p++)
    {
      if (*s < 128)
        *p = *s;
      else if (*s > 191 && *s < 224)
        {
          *p = ((*s) << 6) | ((*(s + 1)) & 63);
          s++;
        }
      else
        {
          *p = (((*s) & 15) << 12) | (((*(s + 1)) & 63) << 6) | ((*(s + 1)) & 63);
          s += 2;
        }
    }

  return d;
}
*/


char *
strupr (char *s)
{
  char *p = s;

  for (; *p; p++)
    *p = toupper (*p);

  return s;
}


char *
strlwr (char *s)
{
  char *p = s;

  for (; *p; p++)
    *p = tolower (*p);

  return s;
}


char *
strmove (char *to, char *from)
{
  return memmove (to, from, strlen (from) + 1); // + 1 because of termination
}


char *
strins (char *dest, const char *ins)
{
  strmove (dest + strlen (ins), dest);
  memcpy (dest, ins, strlen (ins));

  return dest;
}


char *
strcasestr2 (const char *str, const char *search)
{
  if (!(*search))
    return (char *) str;

  return (char *) memmem2 (str, strlen (str), search, strlen (search), MEMMEM2_CASE);
}


char *
strrstr (char *str, const char *search)
{
  unsigned int search_len = strlen (search);
  char *p = NULL;

  if (strlen (str) < search_len)
    return NULL;

  p = strchr (str, 0) - search_len;

  for (;; p--)
    {
      if (!strncmp (p, search, search_len))
        return p;

      if (p == str)
        return NULL;
    }

  return NULL;
}


char *
strristr (char *str, const char *search)
{
  unsigned int search_len = strlen (search);
  char *p = NULL;

  if (strlen (str) < search_len)
    return NULL;

  p = strchr (str, 0) - search_len;

  for (;; p--)
    {
      if (!strnicmp (p, search, search_len))
        return p;

      if (p == str)
        return NULL;
    }

  return NULL;
}


char *
strtrim (char *str, int (*left) (int), int (*right) (int))
{
  if (left)
    {
      char *p = str;

      while (*p && left ((int) *p))
        p++;

      if (p - str)
        strmove (str, p);
    }

  if (right)
    {
      char *p = strchr (str, 0);

      while ((p - 1) - str && right ((int) *(p - 1)))
        p--;

      *p = 0;
    }

  return str;
}


char *
strtriml (char *str)
{
  return strtrim (str, isspace, NULL);
}


char *
strtrimr (char *str)
{
  return strtrim (str, NULL, isspace);
}


char *
strtrim_s (char *str, const char *left, const char *right)
{
  if (left)
    {
      char *p = strstr (str, left);

      if (p)
        strmove (str, p + strlen (left));
    }

  if (right)
    {
      char *p = strrstr (str, right);

      if (p)
        *p = 0;
    }

  return str;
}


char *
stritrim_s (char *str, const char *left, const char *right)
{
  if (left)
    {
      char *p = stristr (str, left);

      if (p)
        strmove (str, p + strlen (left));
    }

  if (right)
    {
      char *p = strristr (str, right);

      if (p)
        *p = 0;
    }

  return str;
}


char *
strrep (char *str, const char *orig, const char *rep)
{
  int o_len = strlen (orig);
  int r_len = strlen (rep);
  char *p = str;

//  if (r_len)
    while ((p = strstr (p, orig)))
      {
        strmove (p + r_len, p + o_len);
        memcpy (p, rep, r_len);
        p += r_len;
      }

  return str;
}


char *
strcode (char *str)
{
  strrep (str, "~", "\\~");
  strrep (str, "%", "\\%");
  strrep (str, "|", "\\|");
  strrep (str, "'", "\\'");
  strrep (str, "&", "\\&");
  strrep (str, ";", "\\;");
  strrep (str, "?", "\\?");
  strrep (str, "!", "\\!");
  strrep (str, "*", "\\*");
  strrep (str, "[", "\\[");
  strrep (str, "]", "\\]");
  strrep (str, "{", "\\{");
  strrep (str, "}", "\\}");
  strrep (str, "(", "\\(");
  strrep (str, ")", "\\)");
  strrep (str, "<", "\\<");
  strrep (str, ">", "\\>");

  return str;
}


char *
strhtml (char *str)
{
  strrep (str, "<", "&lt;");
  strrep (str, ">", "&gt;");

  return str;
}


#ifdef  DEBUG
static int
strarg_debug (int argc, char **argv)
{
  int pos;
  fprintf (stderr, "argc:     %d\n", argc);
  for (pos = 0; pos < argc; pos++)
    fprintf (stderr, "argv[%d]:  %s\n", pos, argv[pos]);

  fflush (stderr);

  return 0;
}
#endif


int
strarg (char **argv, char *str, const char *separator_s, int max_args)
{
  int argc = 0;

  if (str)
    if (*str)
      for (; (argv[argc] = (char *) strtok (!argc ? str : NULL, separator_s)) &&
           (argc < (max_args - 1)); argc++)
        ;

#ifdef  DEBUG
  strarg_debug (argc, argv);
#endif

  return argc;
}


int
memcmp2 (const void *buffer, const void *search, size_t searchlen, unsigned int flags)
{
#define WILDCARD(f)  (f & 0xff)
  size_t i = 0, j = 0;
  const unsigned char *b = (const unsigned char *) buffer,
                      *s = (const unsigned char *) search;

#ifdef  DEBUG
  if (flags & MEMMEM2_WCARD (0))
    printf ("wildcard: %c\n", WILDCARD (flags));
#endif

  if (!flags)
    return memcmp (buffer, search, searchlen);

  if (flags & MEMMEM2_REL)
    {
      searchlen--;
      if (searchlen < 1)
        return -1;
    }

  for (i = j = 0; i < searchlen; i++, j++)
    {
      if (flags & MEMMEM2_WCARD (0))
        {
          if (*(s + i) == WILDCARD (flags))
            continue;
        }

      if (flags & MEMMEM2_REL)
        {
          if ((*(b + j) - *(b + j + 1)) != (*(s + i) - *(s + i + 1)))
            break;
        }
      else
        {
          if (flags & MEMMEM2_CASE && isalpha (*(s + i)))
            {
              if (tolower (*(b + j)) != tolower (*(s + i)))
                break;
            }
          else
            if (*(b + j) != *(s + i))
              break;
        }
    }

  return i == searchlen ? 0 : -1;
}


const void *
memmem2 (const void *buffer, size_t bufferlen,
         const void *search, size_t searchlen, unsigned int flags)
{
  size_t i;

  if (bufferlen >= searchlen)
    for (i = 0; i <= bufferlen - searchlen; i++)
      if (!memcmp2 ((const unsigned char *) buffer + i, search, searchlen, flags))
        return (const unsigned char *) buffer + i;

  return NULL;
}


#if 0
//#ifdef  TEST
int
main (int argc, char **argv)
{
#define MAXBUFSIZE 32768
  char buf[MAXBUFSIZE];
#if 0
  const char *b = "123(123.32.21.44)214";
  const char *s = "(xxx.xx.xx.xx)";

//  const char *p = memmem2 (b, strlen (b), s, strlen (s), MEMCMP2_WCARD('*', 'x'));
  const char *p = memmem2 (b, strlen (b), s, strlen (s), MEMCMP2_WCARD('x'));
  printf ("%s\n", p);

  strcpy (buf, "1234567890");
  strins (buf + 2, 6, "abc");
  printf ("%s\n", buf);
#else
  strcpy (buf, "12434akjgkjh56453fdsg");
  stritrim_s (buf, "sg", "xx");
  printf (buf);
#endif  
  return 0;
}
#endif
