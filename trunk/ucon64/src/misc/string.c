/*
string.c - some string functions

Copyright (c) 1999 - 2008                    NoisyB
Copyright (c) 2001 - 2004, 2015, 2017 - 2019 dbjh


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
#include <ctype.h>
#include "misc/string.h"
#ifdef  DEBUG
#include <stdio.h>
#endif


#if     !(defined _MSC_VER || defined __CYGWIN__ || defined __MSDOS__)
static inline char *
tofunc (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = (char) func (*p);

  return s;
}


char *
strupr (char *s)
{
  return tofunc (s, strlen (s), toupper);
}


char *
strlwr (char *s)
{
  return tofunc (s, strlen (s), tolower);
}
#endif


char *
strcasestr2 (const char *str, const char *search)
{
  if (!(*search))
    return (char *) str;

  return (char *) memmem2 (str, strlen (str), search, strlen (search), MEMMEM2_CASE);
}


char *
strtrimr (char *str)
{
  int i = strlen (str) - 1;

  while (isspace ((int) str[i]) && (i >= 0))
    str[i--] = 0;

  return str;
}


char *
strtriml (char *str)
{
  int i = 0, j;

  j = strlen (str) - 1;

  while (i <= j && isspace ((int) str[i]))
    i++;

  if (0 < i)
    strcpy (str, &str[i]);

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

  if (str && *str)
    for (; argc < max_args - 1 &&
           (argv[argc] = (char *) strtok (!argc ? str : NULL, separator_s)) != NULL;
         argc++)
      ;

#ifdef  DEBUG
  strarg_debug (argc, argv);
#endif

  return argc;
}


int
memcmp2 (const void *buffer, const void *search, size_t searchlen, unsigned int flags)
{
  size_t i = 0;
  const unsigned char *b = (const unsigned char *) buffer,
                      *s = (const unsigned char *) search;

  if (!flags)
    return memcmp (buffer, search, searchlen);

  if (flags & MEMMEM2_REL)
    {
      searchlen--;
      if (searchlen < 1)
        return -1;
    }

  for (i = 0; i < searchlen; i++)
    {
      if (flags & MEMMEM2_WCARD (0))
        if (*(s + i) == (flags & 0xff))
          continue;

      if (flags & MEMMEM2_REL)
        {
          if ((*(b + i) - *(b + i + 1)) != (*(s + i) - *(s + i + 1)))
            break;
        }
      else
        {
          if (flags & MEMMEM2_CASE && isalpha (*(s + i)))
            {
              if (tolower (*(b + i)) != tolower (*(s + i)))
                break;
            }
          else
            if (*(b + i) != *(s + i))
              break;
        }
    }

  return i == searchlen ? 0 : -1;
}


const void *
memmem2 (const void *buffer, size_t bufferlen,
         const void *search, size_t searchlen, unsigned int flags)
{
  if (bufferlen >= searchlen)
    {
      size_t i;

      for (i = 0; i <= bufferlen - searchlen; i++)
        if (!memcmp2 ((const unsigned char *) buffer + i, search, searchlen, flags))
          return (const unsigned char *) buffer + i;
    }

  return NULL;
}


#ifndef HAVE_STRNLEN
size_t
strnlen (const char *str, size_t maxlen)
{
  size_t n;

  for (n = 0; n < maxlen && str[n]; n++)
    ;
  return n;
}
#endif
