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
strcasestr2 (const char *str, const char *search)
{
  if (!(*search))
    return (char *) str;

  return (char *) memmem2 (str, strlen (str), search, strlen (search), MEMMEM2_CASE);
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
        {
          char *s = str;
          while (*p)
            *s++ = *p++;
          *s = 0;
        }
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


#define STRTRIM_S(f)  if (left) \
    { \
      char *p = f (str, left); \
 \
      if (p) \
        { \
          char *s = str; \
          p = p + strlen (left); \
          while (*p) \
            *s++ = *p++; \
          *s = 0; \
        } \
    } \
 \
  if (right) \
    if (strlen (str) >= strlen (right)) \
      { \
        char *p = strchr (str, 0) - strlen (right); \
 \
        while (p - str  >= 0 && !f (p, right)) \
          p--; \
 \
        *p = 0; \
      }


char *
strtrim_s (char *str, const char *left, const char *right)
{
  STRTRIM_S(strstr)
  return str;
}


char *
stritrim_s (char *str, const char *left, const char *right)
{
  STRTRIM_S(stristr)
  return str;
}


char *
strins (char *dest, int dest_replace_len, const char *ins)
{
  char *bak = strdup (dest + dest_replace_len);

  strcpy (dest, ins);
  strcat (dest, bak);

  free (bak);

  return dest;
}


#if 0
char *
strcode (char *str)
{
 char *p = strdup (str);

 if (p)
   for (; *p; p++)
     switch (*p)
       {
         case ' ':
         case '~':
         case '%':
         case '|':
         case '\\':
         case '&':
         case ';':
         case '?':
         case '!':
         case '*':
         case '[':
         case ']':
         case '{':
         case '}':
         case '(':
         case ')':
         case '<':
         case '>':
       }
 if (!Selected) Selected = " ";
 Selected.replace(" ", "\\ ");
 Selected.replace("~", "\\~");
 Selected.replace("%", "\\%");
 Selected.replace("|", "\\|");
 Selected.replace("'", "\\'");
 Selected.replace("&", "\\&");
 Selected.replace(";", "\\;");
 Selected.replace("?", "\\?");
 Selected.replace("!", "\\!");
 Selected.replace("*", "\\*");
 Selected.replace("[", "\\[");
 Selected.replace("]", "\\]");
 Selected.replace("{", "\\{");
 Selected.replace("}", "\\}");
 Selected.replace("(", "\\(");
 Selected.replace(")", "\\)");
 Selected.replace("<", "\\<");
 Selected.replace(">", "\\>");
 return Selected;

  free (p);

  return str;
}
#endif


char *
string_code (char *d, const char *s)
{
  char *p = d;

  *p = 0;
  for (; *s; s++)
    switch (*s)
      {
      case '\n':
        strcat (p, "\\n\"\n  \"");
        break;

      case '\"':
        strcat (p, "\\\"");
        break;

      default:
        p = strchr (p, 0);
        *p = *s;
        *(++p) = 0;
      }

  return d;
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
#define SINGLE_WC(f)  (f & 0xff)
#define MULTI_WC(f) ((f >> 8) & 0xff)
  size_t i = 0, j = 0;
  const unsigned char *b = (const unsigned char *) buffer,
                      *s = (const unsigned char *) search;


#ifdef  DEBUG
  if (flags & MEMMEM2_WCARD (0, 0))
    {
      printf ("single_wc: %c\n", SINGLE_WC (flags));
      printf ("multi_wc: %c\n", MULTI_WC (flags));
    }
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
#if 0
      if (flags & MEMMEM2_WCARD (0, 0))
#else
      if (flags & MEMMEM2_WCARD (0))
#endif
        {
          if (*(s + i) == SINGLE_WC (flags))
            continue;
#if 0
          if (*(s + i) == MULTI_WC (flags))
            for (; j < strlen ((char *) b); j++)
              if (*(s + i + 1) == *(b + j))
                break;
#endif
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
  const char *b = "123(123.32.21.44)214";
  const char *s = "(xxx.xx.xx.xx)";

//  const char *p = memmem2 (b, strlen (b), s, strlen (s), MEMCMP2_WCARD('*', 'x'));
  const char *p = memmem2 (b, strlen (b), s, strlen (s), MEMCMP2_WCARD('x'));
  printf ("%s\n", p);

  strcpy (buf, "1234567890");
  strins (buf + 2, 6, "abc");
  printf ("%s\n", buf);
  
  return 0;
}
#endif
