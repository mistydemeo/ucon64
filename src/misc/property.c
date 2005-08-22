/*
property.c - configfile handling

Copyright (c) 2004 NoisyB


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
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>                           // for struct stat
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include "file.h"                               // realpath2()
#include "property.h"
#include "misc.h"                               // getenv2()
#include "string.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768


char *
get_property_from_string (char *str, const char *propname, const char prop_sep,
                          char *value_s, const char comment_sep)
{
  char str_end[6], *p = NULL;

  p = strtriml (str);
  if (*p == comment_sep || *p == '\n' || *p == '\r')
    return NULL;                      // text after comment_sep is comment

  sprintf (str_end, "%c\r\n", comment_sep);
  if ((p = strpbrk (str, str_end)))   // strip *any* returns and comments
    *p = 0;

  p = strchr (str, prop_sep);
  if (p)
    {
      *p = 0;                           // note that this "cuts" _str_ ...
      p++;
    }
  strtriml (strtrimr (str));

  if (!stricmp (str, propname))        // ...because we do _not_ use strnicmp()
    {
      // if no divider was found the propname must be a bool config entry
      //  (present or not present)
      if (p)
        {
          strcpy (value_s, p);
          strtriml (strtrimr (value_s));
        }
      else
        strcpy (value_s, "1");
    }
  else
    value_s = NULL;

  return value_s;
}


char *
get_property (const char *filename, const char *propname,
              char *value_s, const char *def)
{
  char line[MAXBUFSIZE], *p = NULL;
  FILE *fh;
  int prop_found = 0;

  if ((fh = fopen (filename, "r")) != 0)        // opening the file in text mode
    {                                           //  avoids trouble under DOS
      while (fgets (line, sizeof line, fh) != NULL)
        if (get_property_from_string (line, propname, PROPERTY_SEPARATOR, value_s, PROPERTY_COMMENT))
          prop_found = 1;

      fclose (fh);
    }

  p = getenv2 (propname);
  if (*p == 0)                                  // getenv2() never returns NULL
    {
      if (!prop_found)
        {
          if (def)
            strcpy (value_s, def);
          else
            value_s = NULL;                     // value_s won't be changed
        }                                       //  after this func (=ok)
    }
  else
    strcpy (value_s, p);

  return value_s;
}


int
get_property_int (const char *filename, const char *propname)
{
  char value_s[MAXBUFSIZE];                     // MAXBUFSIZE is enough for a *very* large number
  int value = 0;                                //  and people who might not get the idea that
                                                //  get_property_int() is ONLY about numbers
  get_property (filename, propname, value_s, NULL);

  if (*value_s)
    switch (tolower (*value_s))
      {
      case '0':                                 // 0
      case 'n':                                 // [Nn]o
        return 0;
      }

  value = strtol (value_s, NULL, 10);
  return value ? value : 1;                     // if value_s was only text like 'Yes'
}                                               //  we'll return at least 1


char *
get_property_fname (const char *filename, const char *propname,
                    char *value_s, const char *def)
// get a filename from file with name filename, expand it and fix characters
{
  char tmp[FILENAME_MAX];

  get_property (filename, propname, tmp, def);
#ifdef  __CYGWIN__
  fix_character_set (tmp);
#endif
  return realpath2 (tmp, value_s);
}


int
set_property (const char *filename, const char *propname,
              const char *value_s, const char *comment_s)
{
  int found = 0, result = 0, file_size = 0;
  char line[MAXBUFSIZE], line2[MAXBUFSIZE], *str = NULL, *p = NULL,
       line_end[6];
  FILE *fh;
  struct stat fstate;

  if (stat (filename, &fstate) != 0)
    file_size = fstate.st_size;

  if (!(str = (char *) malloc (file_size + MAXBUFSIZE)))
    return -1;

  sprintf (line_end, "%c\r\n", PROPERTY_COMMENT);

  *str = 0;
  if ((fh = fopen (filename, "r")) != 0)        // opening the file in text mode
    {                                           //  avoids trouble under DOS
      // update existing properties
      while (fgets (line, sizeof line, fh) != NULL)
        {
          strcpy (line2, line);
          if ((p = strpbrk (line2, line_end)))
            *p = 0;                             // note that this "cuts" _line2_
          p = strchr (line2, PROPERTY_SEPARATOR);
          if (p)
            *p = 0;

          strtriml (strtrimr (line2));

          if (!stricmp (line2, propname))
            {
              found = 1;
              if (value_s)
                sprintf (line, "%s%c%s\n", propname, PROPERTY_SEPARATOR, value_s ? value_s : "1");
            }
          strcat (str, line);
        }
      fclose (fh);
    }

  // completely new properties are added at the bottom
  if (!found && value_s)
    {
      if (comment_s)
        {
          sprintf (strchr (str, 0), "%c\n%c ", PROPERTY_COMMENT, PROPERTY_COMMENT);

          for (p = strchr (str, 0); *comment_s; comment_s++)
            switch (*comment_s)
              {
              case '\r':
                break;
              case '\n':
                sprintf (strchr (str, 0), "\n%c ", PROPERTY_COMMENT);
                break;

              default:
                p = strchr (str, 0);
                *p = *comment_s;
                *(++p) = 0;
                break;
              }

          sprintf (strchr (str, 0), "\n%c\n", PROPERTY_COMMENT);
        }

      sprintf (line, "%s%c%s\n", propname, PROPERTY_SEPARATOR, value_s);
      strcat (str, line);
    }

  if ((fh = fopen (filename, "w")) == NULL)     // open in text mode
    return -1;
  result = fwrite (str, 1, strlen (str), fh);
  fclose (fh);

  return result;
}


int
set_property_array (const char *filename, const st_property_t *prop)
{
  int i = 0, result = 0;

  for (; prop[i].name; i++)
    {
      result = set_property (filename, prop[i].name, prop[i].value_s,
                             prop[i].comment_s);

      if (result == -1) // failed
        break;
    }

  return result;
}
