/*
misc.c - miscellaneous functions

Copyright (c) 1999 - 2008              NoisyB
Copyright (c) 2001 - 2005, 2015 - 2019 dbjh
Copyright (c) 2002 - 2005              Jan-Erik Karlsson (Amiga code)


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
#include "config.h"                             // USE_ZLIB
#endif
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <ctype.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#include <direct.h>
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#include <io.h>
#pragma warning(pop)
#endif
#ifdef  HAVE_CLOCK_NANOSLEEP
#include <errno.h>                              // EINTR
#endif
#include <stdarg.h>                             // va_arg()
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>                             // usleep(), microseconds
#endif

#ifdef  __MSDOS__
#include <dos.h>                                // delay(), milliseconds
#elif   defined __BEOS__
#include <OS.h>                                 // snooze(), microseconds
#elif   defined AMIGA
#error Include directives are missing. Please add them and let us know.
#elif   defined _WIN32 || defined __CYGWIN__
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4255) // 'function' : no function prototype given: converting '()' to '(void)'
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <windows.h>                            // Sleep(), milliseconds
#ifdef  __CYGWIN__
#undef  _WIN32
#endif
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#endif

#include "misc/archive.h"
#include "misc/file.h"
#include "misc/itypes.h"
#include "misc/misc.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768


typedef struct st_func_node
{
  void (*func) (void);
  struct st_func_node *next;
} st_func_node_t;

int cm_verbose = 0;
static st_func_node_t func_list = { NULL, NULL };
static int func_list_locked = 0;


int
misc_digits (unsigned long v)
{
  int ret = 1;

  if ( v >= 100000000 ) { ret += 8; v /= 100000000; }
  if ( v >=     10000 ) { ret += 4; v /=     10000; }
  if ( v >=       100 ) { ret += 2; v /=       100; }
  if ( v >=        10 ) { ret += 1;                 }

  return ret;
}


int
bytes_per_second (time_t start_time, int nbytes)
{
  int curr = (int) difftime (time (NULL), start_time);

  if (curr < 1)
    curr = 1;                                   // "round up" to at least 1 sec (no division
                                                //  by zero below)
  return nbytes / curr;                         // # bytes/second (average transfer speed)
}


int
misc_percent (int pos, int len)
{
  if (len < 1)
    len = 1;

  return (int) ((((int64_t) 100) * pos) / len);
}


void
wait2 (int nmillis)
{
#ifdef  __MSDOS__
  delay (nmillis);
#elif   defined _WIN32
  Sleep (nmillis);
#elif   defined HAVE_CLOCK_NANOSLEEP            // see comment about usleep()
#if 0 // very inaccurate, but sleeping implementation
  struct timespec end;

  clock_gettime (CLOCK_MONOTONIC, &end);
  end.tv_sec += nmillis / 1000;
  end.tv_nsec += (nmillis % 1000) * 1000000;
  if (end.tv_nsec > 999999999)
    {
      end.tv_sec++;
      end.tv_nsec -= 1000000000;
    }

  while (clock_nanosleep (CLOCK_MONOTONIC, TIMER_ABSTIME, &end, NULL) == EINTR)
    ;
#else // busy-waiting, but much more accurate
  struct timespec end, current;

  clock_gettime (CLOCK_MONOTONIC, &end);
  end.tv_sec += nmillis / 1000;
  end.tv_nsec += (nmillis % 1000) * 1000000;
  if (end.tv_nsec > 999999999)
    {
      end.tv_sec++;
      end.tv_nsec -= 1000000000;
    }

  do
    {
      clock_gettime (CLOCK_MONOTONIC, &current);
      current.tv_sec -= end.tv_sec;
      current.tv_nsec -= end.tv_nsec;
      if (current.tv_nsec < 0)
        {
          current.tv_sec--;
          current.tv_nsec += 1000000000;
        }
    }
  while (current.tv_sec < 0);
#endif
#elif   defined __unix__ || defined __APPLE__   // Mac OS X actually
  // NOTE: Apparently usleep() was deprecated in POSIX.1-2001 and removed from
  //       POSIX.1-2008. clock_nanosleep() will likely give better results.
  usleep (nmillis * 1000);
#elif   defined __BEOS__
  snooze (nmillis * 1000);
#elif   defined AMIGA
  Delay (nmillis > 20 ? nmillis / 20 : 1);      // 50Hz clock, so interval is 20ms
#else
#ifdef  __GNUC__
#warning Please provide a wait2() implementation
#else
#pragma message ("Please provide a wait2() implementation")
#endif
  volatile int n;
  for (n = 0; n < nmillis * 65536; n++)
    ;
#endif
}


#if     (defined __unix__ && !defined __MSDOS__) || defined __APPLE__ || \
        defined __BEOS__ || defined _WIN32
void
microwait (int nmicros)
{
#ifdef  _WIN32
#if 0 // very inaccurate, but sleeping implementation
  HANDLE timer;
  LARGE_INTEGER interval;

  interval.QuadPart = -10 * nmicros;
  if ((timer = CreateWaitableTimer (NULL, TRUE, NULL)) != NULL)
    {
      if (SetWaitableTimer (timer, &interval, 0, NULL, NULL, FALSE))
        WaitForSingleObject (timer, INFINITE);
      CloseHandle (timer);
    }
#else // busy-waiting, but much more accurate
  LARGE_INTEGER frequency, start, end;

  if (QueryPerformanceFrequency (&frequency) && QueryPerformanceCounter (&start))
    while (QueryPerformanceCounter (&end) &&
           ((end.QuadPart - start.QuadPart) * 1000000) / frequency.QuadPart < nmicros)
      ;
#endif
#elif   defined HAVE_CLOCK_NANOSLEEP            // see comment about usleep()
#if 0 // very inaccurate, but sleeping implementation
  struct timespec end;

  clock_gettime (CLOCK_MONOTONIC, &end);
  end.tv_sec += nmicros / 1000000;
  end.tv_nsec += (nmicros % 1000000) * 1000;
  if (end.tv_nsec > 999999999)
    {
      end.tv_sec++;
      end.tv_nsec -= 1000000000;
    }

  while (clock_nanosleep (CLOCK_MONOTONIC, TIMER_ABSTIME, &end, NULL) == EINTR)
    ;
#else // busy-waiting, but much more accurate
  struct timespec end, current;

  clock_gettime (CLOCK_MONOTONIC, &end);
  end.tv_sec += nmicros / 1000000;
  end.tv_nsec += (nmicros % 1000000) * 1000;
  if (end.tv_nsec > 999999999)
    {
      end.tv_sec++;
      end.tv_nsec -= 1000000000;
    }

  do
    {
      clock_gettime (CLOCK_MONOTONIC, &current);
      current.tv_sec -= end.tv_sec;
      current.tv_nsec -= end.tv_nsec;
      if (current.tv_nsec < 0)
        {
          current.tv_sec--;
          current.tv_nsec += 1000000000;
        }
    }
  while (current.tv_sec < 0);
#endif
#elif   defined __unix__ || defined __APPLE__   // Mac OS X actually
  // NOTE: Apparently usleep() was deprecated in POSIX.1-2001 and removed from
  //       POSIX.1-2008. clock_nanosleep() will likely give better results.
  usleep (nmicros);
#elif   defined __BEOS__
  snooze (nmicros);
#endif
}
#endif


void
dumper (FILE *output, const void *buffer, size_t bufferlen, int virtual_start,
        unsigned int flags)
// Do NOT use DUMPER_PRINT in uCON64 code - dbjh
{
#define DUMPER_REPLACER ('.')
  size_t pos;
  char buf[17];
  const unsigned char *p = (const unsigned char *) buffer;

  memset (buf, 0, sizeof buf);
  for (pos = 0; pos < bufferlen; pos++, p++)
    if (flags & DUMPER_PRINT)
      {
//        fprintf (output, (flags & DUMPER_DEC_COUNT ? "%010d  " : "%08x  "),
//          (int) (pos + virtual_start));
        int c = isprint (*p) ||
#ifdef USE_ANSI_COLOR
                *p == 0x1b || // ESC
#endif
                isspace (*p) ? *p : DUMPER_REPLACER;
        fputc (c, output);
      }
    else if (flags & DUMPER_DUAL)
      {
        if (!(pos & 3))
          fprintf (output, (flags & DUMPER_DEC_COUNT ? "%010d  " : "%08x  "),
            (int) (pos + virtual_start));

        fprintf (output, "%02x  %08d  ",
                         *p,
                         ((*p >> 7) & 1) * 10000000 +
                         ((*p >> 6) & 1) * 1000000 +
                         ((*p >> 5) & 1) * 100000 +
                         ((*p >> 4) & 1) * 10000 +
                         ((*p >> 3) & 1) * 1000 +
                         ((*p >> 2) & 1) * 100 +
                         ((*p >> 1) & 1) * 10 +
                         (*p & 1));

        *(buf + (pos & 3)) = isprint (*p) ? *p : DUMPER_REPLACER;
        if (!((pos + 1) & 3))
          fprintf (output, "%s\n", buf);
      }
    else if (flags & DUMPER_CODE)
      {
        fprintf (output, "0x%02x, ", *p);

        if (!((pos + 1) & 7))
          fprintf (output, (flags & DUMPER_DEC_COUNT ? "// (%d) 0x%x\n" : "// 0x%x (%d)\n"),
            (int) (pos + virtual_start + 1),
            (int) (pos + virtual_start + 1));
      }
    else // if (flags & DUMPER_HEX) // default
      {
        if (!(pos & 15))
          fprintf (output, (flags & DUMPER_DEC_COUNT ? "%08d  " : "%08x  "),
            (int) (pos + virtual_start));

        fprintf (output, (pos + 1) & 3 ? "%02x " : "%02x  ", *p);

        *(buf + (pos & 15)) = isprint (*p) ? *p : DUMPER_REPLACER;
        if (!((pos + 1) & 15))
          fprintf (output, "%s\n", buf);
      }

  if (flags & DUMPER_PRINT)
    return;
  else if (flags & DUMPER_DUAL)
    {
      if (pos & 3)
        {
          *(buf + (pos & 3)) = '\0';
          fprintf (output, "%s\n", buf);
        }
    }
  else if (flags & DUMPER_CODE)
    return;
  else // if (flags & DUMPER_HEX) // default
    {
      if (pos & 15)
        {
          // align ASCII columns by inserting spaces
          int n;
          for (n = 16 - (pos % 16); n > 0; n--)
            {
              fputs ("   ", output);
              if (n % 4 == 0)
                fputc (' ', output);
            }
          if ((16 - (pos % 16)) % 4 != 0)
            fputc (' ', output);

          *(buf + (pos & 15)) = 0;
          fprintf (output, "%s\n", buf);
        }
    }
}


int
change_mem (char *buf, unsigned int bufsize, char *searchstr,
            unsigned int strsize, char wc, char esc, char *newstr,
            unsigned int newsize, int offset, ...)
// convenience wrapper for change_mem2()
{
  va_list argptr;
  unsigned int i, n_esc = 0, retval;
  st_cm_set_t *sets;

  for (i = 0; i < strsize; i++)
    if (searchstr[i] == esc)
      n_esc++;

  if ((sets = (st_cm_set_t *) malloc (n_esc * sizeof (st_cm_set_t))) == NULL)
    {
      fprintf (stderr, "ERROR: Not enough memory for buffer (%u bytes)\n",
               (unsigned int) (n_esc * sizeof (st_cm_set_t)));
      return -1;
    }
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


static int
match_found (char *buf, unsigned int bufsize, unsigned int strsize,
             char *newstr, unsigned int newsize, int offset,
             unsigned int bufpos, unsigned int strpos)
{
  if ((int) bufpos + offset >= 0 && bufpos + offset + newsize <= bufsize)
    {
      if (cm_verbose > 0)
        {
          printf ("Match, patching at pattern offset %d/0x%08x / buffer[%u/0x%08x]\n",
                  offset, offset, bufpos + offset, bufpos + offset);
          dumper (stdout, buf + bufpos - strpos, strsize,
                  bufpos - strpos, DUMPER_HEX);
        }
      memcpy (buf + bufpos + offset, newstr, newsize);
      return 1;
    }
  else
    {
      printf ("WARNING: The combination of buffer position (%u), offset (%d) and\n"
              "         replacement size (%u) would cause a buffer overflow -- ignoring\n"
              "         match\n", bufpos, offset, newsize);
      return 0;
    }
}


int
change_mem2 (char *buf, unsigned int bufsize, char *searchstr,
             unsigned int strsize, char wc, char esc, char *newstr,
             unsigned int newsize, int offset, st_cm_set_t *sets)
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
    change_mem2 (str, strlen (str), "f**bar", 6, '*', '!', "XXXXXXXX", 8, 2, NULL);
  finds and changes "foobar means..." into "foobar XXXXXXXX", while with uCON's
  algorithm it would not (but does the job good enough for patching SNES ROMs).

  One example of using sets:
    char str[] = "fu-bar     is the same as foobar    ";
    st_cm_set_t sets[] = {{"uo", 2}, {"o-", 2}};
    change_mem2 (str, strlen (str), "f!!", 3, '*', '!', "fighter", 7, 1, sets);
  This changes str into "fu-fighter is the same as foofighter".
*/
{
  char *set;
  unsigned int bufpos, strpos = 0, pos_1st_esc = (unsigned int) -1, setsize, i,
               n_wc, n_matches = 0, setindex = 0;

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
          if (pos_1st_esc == (unsigned int) -1)
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
              n_matches += match_found (buf, bufsize, strsize, newstr, newsize,
                                        offset, bufpos, strpos);
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
              n_matches += match_found (buf, bufsize, strsize, newstr, newsize,
                                        offset, bufpos, strpos);
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
          bufpos--;                             // current char has to be checked, but "for"
          continue;                             //  increments bufpos
        }

      // no escape char, no wildcard => normal character
      if (searchstr[strpos] == buf[bufpos])
        {
          if (strpos == strsize - 1)            // check if at end of searchstr
            {
              n_matches += match_found (buf, bufsize, strsize, newstr, newsize,
                                        offset, bufpos, strpos);
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
              bufpos--;                         // current char has to be checked, but "for"
              strpos = 0;                       //  increments bufpos
            }
        }
    }

  return n_matches;
}


int
build_cm_patterns (st_cm_pattern_t **patterns, const char *filename)
/*
  This function goes a bit over the top what memory allocation technique
  concerns, but at least it's stable.
  Note the important difference between (*patterns)[0].n_sets and
  patterns[0]->n_sets (not especially that member). I (dbjh) am too ashamed to
  tell how long it took me to finally realise that...
*/
{
  char src_name[FILENAME_MAX], line[MAXBUFSIZE], buffer[MAXBUFSIZE],
       *token, *last;
  unsigned int line_num = 0, n, currentsize1, requiredsize1, currentsize2,
               requiredsize2, currentsize3, requiredsize3;
  int n_codes = 0;
  FILE *srcfile;

  strcpy (src_name, filename);
  if (access (src_name, F_OK | R_OK))
    return -1;                                  // NOT an error, it's optional

  if ((srcfile = fopen (src_name, "r")) == NULL) // open in text mode
    {
      fprintf (stderr, "ERROR: Cannot open \"%s\" for reading\n", src_name);
      return -1;
    }

  *patterns = NULL;
  currentsize1 = requiredsize1 = 0;
  while (fgets (line, sizeof line, srcfile) != NULL)
    {
      char *ptr = line + strspn (line, "\t ");
      unsigned int n_sets = 0;

      line_num++;

      if (*ptr == '#' || *ptr == '\n' || *ptr == '\r')
        continue;
      if ((ptr = strpbrk (line, "\n\r#")) != NULL) // text after # is comment
        *ptr = '\0';

      requiredsize1 += sizeof (st_cm_pattern_t);
      if (requiredsize1 > currentsize1)
        {
          st_cm_pattern_t *old_patterns = *patterns;
          currentsize1 = requiredsize1 + 10 * sizeof (st_cm_pattern_t);
          if ((*patterns = (st_cm_pattern_t *) realloc (old_patterns, currentsize1)) == NULL)
            {
              fprintf (stderr, "ERROR: Not enough memory for buffer (%u bytes)\n", currentsize1);
              free (old_patterns);
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
      do
        {
          requiredsize2++;
          if (requiredsize2 > currentsize2)
            {
              char *old_search = (*patterns)[n_codes].search;
              currentsize2 = requiredsize2 + 10;
              if (((*patterns)[n_codes].search =
                   (char *) realloc (old_search, currentsize2)) == NULL)
                {
                  fprintf (stderr, "ERROR: Not enough memory for buffer (%u bytes)\n", currentsize2);
                  free (old_search);
                  free (*patterns);
                  *patterns = NULL;
                  return -1;
                }
            }
          (*patterns)[n_codes].search[n] = (unsigned char) strtol (token, NULL, 16);
          n++;
        }
      while ((token = strtok (NULL, " ")) != NULL);
      (*patterns)[n_codes].search_size = n;     // size in bytes

      strcpy (buffer, line);
      strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %u is invalid, no wildcard value is specified\n", line_num);
          continue;
        }
      (*patterns)[n_codes].wildcard = (char) strtol (token, NULL, 16);

      strcpy (buffer, line);
      strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %u is invalid, no escape value is specified\n", line_num);
          continue;
        }
      (*patterns)[n_codes].escape = (char) strtol (token, NULL, 16);

      strcpy (buffer, line);
      strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %u is invalid, no replacement is specified\n", line_num);
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
              char *old_replace = (*patterns)[n_codes].replace;
              currentsize2 = requiredsize2 + 10;
              if (((*patterns)[n_codes].replace =
                   (char *) realloc (old_replace, currentsize2)) == NULL)
                {
                  fprintf (stderr, "ERROR: Not enough memory for buffer (%u bytes)\n", currentsize2);
                  free (old_replace);
                  free ((*patterns)[n_codes].search);
                  free (*patterns);
                  *patterns = NULL;
                  return -1;
                }
            }
          (*patterns)[n_codes].replace[n] = (unsigned char) strtol (token, NULL, 16);
          n++;
        }
      while ((token = strtok (NULL, " ")) != NULL);
      (*patterns)[n_codes].replace_size = n;    // size in bytes

      strcpy (buffer, line);
      strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %u is invalid, no offset is specified\n", line_num);
          continue;
        }
      (*patterns)[n_codes].offset = strtol (token, NULL, 10); // yes, offset is decimal

      if (cm_verbose)
        {
          printf ("pattern:      %d\n"
                  "line:         %u\n"
                  "searchstring: ",
                  n_codes + 1, line_num);
          for (n = 0; n < (*patterns)[n_codes].search_size; n++)
            printf ("%02x ", (unsigned char) (*patterns)[n_codes].search[n]);
          printf ("(%u)\n"
                  "wildcard:     %02x\n"
                  "escape:       %02x\n"
                  "replacement:  ",
                  (*patterns)[n_codes].search_size,
                  (unsigned char) (*patterns)[n_codes].wildcard,
                  (unsigned char) (*patterns)[n_codes].escape);
          for (n = 0; n < (*patterns)[n_codes].replace_size; n++)
            printf ("%02x ", (unsigned char) (*patterns)[n_codes].replace[n]);
          printf ("(%u)\n"
                  "offset:       %d\n",
                  (*patterns)[n_codes].replace_size,
                  (*patterns)[n_codes].offset);
        }

      (*patterns)[n_codes].sets = NULL;
      currentsize2 = 0;
      requiredsize2 = 1;                        // for string terminator
      strcpy (buffer, line);
      strtok (last, ":");
      token = strtok (NULL, ":");
      last = token;
      while (token)
        {
          requiredsize2 += sizeof (st_cm_set_t);
          if (requiredsize2 > currentsize2)
            {
              st_cm_set_t *old_sets = (*patterns)[n_codes].sets;
              currentsize2 = requiredsize2 + 10 * sizeof (st_cm_set_t);
              if (((*patterns)[n_codes].sets =
                   (st_cm_set_t *) realloc (old_sets, currentsize2)) == NULL)
                {
                  fprintf (stderr, "ERROR: Not enough memory for buffer (%u bytes)\n", currentsize2);
                  free (old_sets);
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
                  char *old_data = (*patterns)[n_codes].sets[n_sets].data;
                  currentsize3 = requiredsize3 + 10;
                  if (((*patterns)[n_codes].sets[n_sets].data =
                       (char *) realloc (old_data, currentsize3)) == NULL)
                    {
                      fprintf (stderr, "ERROR: Not enough memory for buffer (%u bytes)\n", currentsize3);
                      free (old_data);
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
          while ((token = strtok (NULL, " ")) != NULL);
          (*patterns)[n_codes].sets[n_sets].size = n;

          if (cm_verbose)
            {
              fputs ("set:          ", stdout);
              for (n = 0; n < (*patterns)[n_codes].sets[n_sets].size; n++)
                printf ("%02x ", (unsigned char) (*patterns)[n_codes].sets[n_sets].data[n]);
              printf ("(%u)\n", (*patterns)[n_codes].sets[n_sets].size);
            }

          strcpy (buffer, line);
          strtok (last, ":");
          token = strtok (NULL, ":");
          last = token;

          n_sets++;
        }
      (*patterns)[n_codes].n_sets = n_sets;

      n_codes++;

      if (cm_verbose)
        fputc ('\n', stdout);
    }
  fclose (srcfile);
  return n_codes;
}


void
cleanup_cm_patterns (st_cm_pattern_t **patterns, int n_patterns)
{
  int n;

  for (n = 0; n < n_patterns; n++)
    {
      unsigned int m;

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


char *
getenv2 (const char *variable)
/*
  getenv() suitable for enviroments w/o HOME, TMP or TEMP variables.
  The caller should copy the returned string to its own memory, because this
  function will overwrite that memory on the next call.
  Note that this function never returns NULL.
*/
{
  char *tmp;
  static char value[MAXBUFSIZE];
  size_t len;
#ifdef  __MSDOS__
/*
  On DOS and Windows the environment variables are not stored in a case
  sensitive manner. The run-time system of DJGPP acts as if they are stored in
  upper case. Its getenv() however *is* case sensitive. We fix this by changing
  all characters of the search string (variable) to upper case.

  Note that in Cygwin's Bash environment variables *are* stored in a case
  sensitive manner.
*/
  char tmp2[MAXBUFSIZE];

  len = strlen (variable);
  if (len >= sizeof tmp2)
    len = sizeof tmp2 - 1;
  strncpy (tmp2, variable, len)[len] = '\0';
  variable = strupr (tmp2);                     // DON'T copy the string into variable
#endif                                          //  (variable itself is local)

  *value = '\0';

  if (!strcmp (variable, "HOME"))
    {
      if ((tmp = getenv ("UCON64_HOME")) != NULL)
        {
          len = strlen (tmp);
          if (len >= sizeof value)
            len = sizeof value - 1;
          strncpy (value, tmp, len)[len] = '\0';
        }
      else if ((tmp = getenv ("HOME")) != NULL)
        {
          len = strlen (tmp);
          if (len >= sizeof value)
            len = sizeof value - 1;
          strncpy (value, tmp, len)[len] = '\0';
        }
      else if ((tmp = getenv ("USERPROFILE")) != NULL)
        {
          len = strlen (tmp);
          if (len >= sizeof value)
            len = sizeof value - 1;
          strncpy (value, tmp, len)[len] = '\0';
        }
      else if ((tmp = getenv ("HOMEDRIVE")) != NULL)
        {
          char *p = getenv ("HOMEPATH");

          if (!p)
            p = DIR_SEPARATOR_S;
          len = strlen (tmp) + strlen (p);
          if (len >= sizeof value)
            len = sizeof value - 1;
          snprintf (value, len + 1, "%s%s", tmp, p);
          value[len] = '\0';
        }
      else
        /*
          Don't just use C:\\ on DOS, the user might not have write access
          there (Windows NT DOS-box). Besides, it would make uCON64 behave
          differently on DOS than on the other platforms.
          Returning the current directory when none of the above environment
          variables are set can be seen as a feature. A frontend could execute
          uCON64 with an environment without any of the environment variables
          set, so that the directory from where uCON64 starts will be used.
        */
        {
          char c;

          if (getcwd (value, FILENAME_MAX) != NULL)
            {
              c = (char) toupper ((int) *value);
              // if current dir is root dir strip problematic ending slash (DJGPP)
              if (c >= 'A' && c <= 'Z' &&
                  value[1] == ':' && value[2] == '/' && value[3] == '\0')
                value[2] = '\0';
            }
          else
            *value = '\0';
        }
    }
  else if ((tmp = getenv (variable)) != NULL)
    {
      len = strlen (tmp);
      if (len >= sizeof value)
        len = sizeof value - 1;
      strncpy (value, tmp, len)[len] = '\0';
    }
  else
    {
      if (!strcmp (variable, "TEMP") || !strcmp (variable, "TMP"))
        {
#if     defined __MSDOS__ || defined __CYGWIN__
          /*
            DJGPP and (yet another) Cygwin quirck
            A trailing backslash is used to check for a directory. Normally
            DJGPP's run-time system is able to handle forward slashes in paths,
            but access() won't differentiate between files and dirs if a
            forward slash is used. Cygwin's run-time system seems to handle
            paths with forward slashes quite different from paths with
            backslashes. This trick seems to work only if a backslash is used.
          */
          if (access ("\\tmp\\", R_OK | W_OK) == 0)
#else
          // trailing file separator to force it to be a directory
          if (access (DIR_SEPARATOR_S "tmp" DIR_SEPARATOR_S, R_OK | W_OK) == 0)
#endif
            strcpy (value, DIR_SEPARATOR_S "tmp");
          else
            if (getcwd (value, FILENAME_MAX) == NULL)
              *value = '\0';
        }
    }

#ifdef  __CYGWIN__
  /*
    Under certain circumstances Cygwin's run-time system returns "/" as value
    of HOME while that var has not been set. To specify a root dir a path like
    /cygdrive/<drive letter> or simply a drive letter should be used.
  */
  if (!strcmp (variable, "HOME") && !strcmp (value, "/"))
    if (getcwd (value, FILENAME_MAX) == NULL)
      *value = '\0';
#endif

  return value;
}


#if     (defined __unix__ && !defined __MSDOS__) || defined __APPLE__
int
drop_privileges (void)
{
  uid_t uid;
  gid_t gid;

#ifndef __CYGWIN__
  regain_privileges (); // the saved UID and GID must be changed too
#endif

  gid = getgid ();
  if (setgid (gid) == -1)
    {
      fprintf (stderr, "ERROR: Could not set group ID to %u\n", gid);
      return -1;
    }

  uid = getuid ();
  if (setuid (uid) == -1)
    {
      fprintf (stderr, "ERROR: Could not set user ID to %u\n", uid);
      return -1;
    }

  return 0;
}


int
drop_privileges_temp (void)
{
  uid_t uid;
  gid_t gid;

  gid = getgid ();
  if (setegid (gid) == -1)
    {
      fprintf (stderr, "ERROR: Could not set effective group ID to %u\n", gid);
      return -1;
    }

  uid = getuid ();
  if (seteuid (uid) == -1)
    {
      fprintf (stderr, "ERROR: Could not set effective user ID to %u\n", uid);
      return -1;
    }

  return 0;
}


int
regain_privileges (void)
{
  if (seteuid ((uid_t) 0) == -1)
    {
      puts ("WARNING: Could not set effective user ID to 0");
      return -1;
    }

  return 0;
}
#endif


int
register_func (void (*func) (void))
{
  st_func_node_t *new_node;

  if ((new_node = (st_func_node_t *) malloc (sizeof (st_func_node_t))) == NULL)
    return -1;

  new_node->func = func;
  new_node->next = func_list.next;
  func_list.next = new_node;
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
      return 0;
    }
  else
    return -1;
}


void
handle_registered_funcs (void)
{
  st_func_node_t *func_node = &func_list;

  func_list_locked = 1;
  while (func_node->next != NULL)
    {
      func_node = func_node->next;              // first node's func is NULL
      if (func_node->func != NULL)
        func_node->func ();
    }
  func_list_locked = 0;
}


#ifdef  AMIGA
// custom _popen() and _pclose(), because the standard ones (named popen() and
//  pclose()) are buggy
FILE *
_popen (const char *path, const char *mode)
{
  int fd;
  BPTR fh;
  long fhflags;
  char *apipe = malloc (strlen (path) + 7);
  if (!apipe)
    return NULL;

  sprintf (apipe, "PIPE:%08lx.%08lx", (ULONG) FindTask (NULL), (ULONG) time (NULL));

  if (*mode == 'w')
    fhflags = MODE_NEWFILE;
  else
    fhflags = MODE_OLDFILE;

  if (fh = Open (apipe, fhflags))
    {
      switch (SystemTags(path, SYS_Input, Input(), SYS_Output, fh, SYS_Asynch,
                TRUE, SYS_UserShell, TRUE, NP_CloseInput, FALSE, TAG_END))
        {
        case 0:
          return fopen (apipe, mode);
          break;
        case -1:
          Close (fh);
          return NULL;
          break;
        default:
          return NULL;
          break;
        }
    }
  return NULL;
}


int
_pclose (FILE *stream)
{
  if (stream)
    return fclose (stream);
  else
    return -1;
}
#endif                                          // AMIGA
