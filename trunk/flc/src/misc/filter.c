/*
filter.c - Simple filter framework for any file, stream or data
           processing application
                      
Copyright (c) 2005 NoisyB


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
#include <stdlib.h>
#include "string.h"
#include "misc.h"
#include "filter.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif
#define MAXBUFSIZE 32768


#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif


//#define TEST
//#define TEST2

#ifdef  TEST
#define FILTER_TEST(fc,s) if(fc->debug){printf("\n%s\n\n", s);fflush(stdout);}
#endif


enum
{
  FILTER_DEMUX = 0,
  FILTER_OPEN,
  FILTER_CLOSE,
  FILTER_INIT,
  FILTER_QUIT,
  FILTER_READ,
  FILTER_WRITE,
  FILTER_SEEK,
  FILTER_CTRL
};


#ifdef  TEST
void
filter_st_filter_chain_t_sanity_check (st_filter_chain_t *fc)
{
  int x = 0;
#if 0
  int y = 0;

  for (; fc->all[x]; x++)
    {
      int set = 0;

      printf ("filter: %d\n", x);
      printf ("filter->id: %d\n", fc->all[x]->id);
      printf ("filter->id_s: %s\n", fc->all[x]->id_s);
      printf ("filter->magic: %s\n", fc->all[x]->magic ? "!NULL" : "NULL");
      printf ("filter->magic_len: %d\n", fc->all[x]->magic_len);
      printf ("filter->init(): %s\n", fc->all[x]->init ? "Yes" : "No");
      printf ("filter->quit(): %s\n", fc->all[x]->quit ? "Yes" : "No");

      for (y = 0; y < fc->total; y++)
        if (fc->all[x]->id == fc->set[y])
          {
            set = 1;
            break;
          }

      printf ("filter->set: %d\n", set);
      printf ("\n");
    }
#else
  {
    const st_filter_t *f = NULL;

    for (x = 0; fc->set[x]; x++)
      printf ("filter->set[%d]: %d %s\n", x, fc->set[x], (f = filter_get_filter_by_id (fc, fc->set[x])) ? f->id_s : "null");
    printf ("\n");
  }
#endif

  fflush (stdout);
}
#endif


int
filter_set_chain (st_filter_chain_t *fc, const int *id)
{
  memset (&fc->set, 0, sizeof (int) * FILTER_MAX);

  // set filters
  if (id)
    {
      for (fc->total = 0; fc->total < FILTER_MAX && id[fc->total]; fc->total++)
        fc->set[fc->total] = id[fc->total];
    }
  else
    {
      for (fc->total = 0; fc->total < FILTER_MAX && fc->all[fc->total]; fc->total++)
        fc->set[fc->total] = fc->all[fc->total]->id;
    }

#ifdef  TEST
  filter_st_filter_chain_t_sanity_check (fc);
#endif

  return 0;
}


st_filter_chain_t *
filter_malloc_chain (const st_filter_t **filter)
{
  int x = 0;
  st_filter_chain_t *fc = NULL;

  if (!(fc = (st_filter_chain_t *) malloc (sizeof (st_filter_chain_t))))
    return NULL;

  memset (fc, 0, sizeof (st_filter_chain_t));
  
  for (fc->all_total = 0; fc->all_total < FILTER_MAX && filter[fc->all_total]; fc->all_total++);

#if 0
  if (!(fc->all = (st_filter_t **) malloc (total * sizeof (st_filter_t *))))
    {
      free (fc);
      return NULL;
    }
#endif

  // malloc chain
  for (x = 0; x < fc->all_total; x++)
    if ((fc->all[x] = (st_filter_t *) malloc (sizeof (st_filter_t))))
      memcpy (fc->all[x], filter[x], sizeof (st_filter_t));
    else
      {
        filter_free_chain (fc);
        return NULL;
      }

  // default: set ALL filters
  if (filter_set_chain (fc, NULL) == -1)
    {
      filter_free_chain (fc);
      return NULL;
    }

  return fc;
}


void
filter_free_chain (st_filter_chain_t *fc)
{
  while (fc->all_total > 0)
    {
      free (fc->all[fc->all_total - 1]);
//      fc->all[fc->all_total - 1] = NULL;
      fc->all_total--;
    }
  free (fc);
//  fc = NULL;
}


static int
filter_func (st_filter_chain_t *fc, void *o, int operation, const char *operation_s)
{
  const st_filter_t *f = NULL;
#ifndef TEST
  (void) operation_s;
#endif

#ifdef  TEST
    if (operation_s)
      {
        fprintf (stderr, "\nfilter: %s->%s()\n", filter_get_filter_by_id (fc, fc->set[fc->pos])->id_s, operation_s);
        fflush (stderr);
      }
#endif

  if ((f = filter_get_filter_by_id (fc, fc->set[fc->pos])))
    switch (operation)
      {
        case FILTER_DEMUX:
          if (f->demux)
            fc->result[fc->pos] = f->demux (o);
          break;

        case FILTER_OPEN:
          if (f->open)
            fc->result[fc->pos] = f->open (o);
          break;

        case FILTER_CLOSE:
          if (f->close)
            fc->result[fc->pos] = f->close (o);
          break;

        case FILTER_READ:
          if (f->read)
            fc->result[fc->pos] = f->read (o);
          break;

        case FILTER_WRITE:
          if (f->write)
            fc->result[fc->pos] = f->write (o);
          break;

        case FILTER_SEEK:
          if (f->seek)
            fc->result[fc->pos] = f->seek (o);
          break;

        case FILTER_CTRL:
          if (f->ctrl)
            fc->result[fc->pos] = f->ctrl (o);
#if 0
          break;

        case FILTER_INIT:
          if (f->init)
            fc->result[fc->pos] = f->init (o);
          break;

        case FILTER_QUIT:
          if (f->quit)
            fc->result[fc->pos] = f->quit (o);
#endif
      }

  if (fc->result[fc->pos] == -1)
    {
#ifdef  TEST2
      fprintf (stderr, "ERROR: could not %s %s filter (failed)\n", operation_s, filter_get_filter_by_id (fc, fc->set[fc->pos])->id_s);
#endif
      return -1;
    }

#ifdef  TEST2
  if (fc->result[fc->pos] > 0)
    fprintf (stderr, "ERROR: could not %s %s filter (skipped; result: %d)\n", operation_s, filter_get_filter_by_id (fc, fc->set[fc->pos])->id_s, fc->result[fc->pos]);
#endif

  return 0;
}


static int
filter_func_ctrl (st_filter_chain_t *fc, void *o, int operation, const char *operation_s)
{
  // set current op
  fc->op = operation;

  switch (operation)
    {
      case FILTER_CLOSE:
        for (fc->pos = fc->total; fc->pos-- > 0;) // reverse
          if (filter_func (fc, o, operation, operation_s) == -1)
            return -1;
        break;
      
      default:
        for (fc->pos = 0; fc->pos < fc->total; fc->pos++)
          if (filter_func (fc, o, operation, operation_s) == -1)
            return -1;
    }

  return 0;
}


#define FILTER_TEMPLATE(operation_macro, operation, operation_s) int \
filter_##operation_macro (st_filter_chain_t *fc, void *o) \
{ \
  return filter_func_ctrl (fc, o, operation, operation_s); \
}


FILTER_TEMPLATE(demux, FILTER_DEMUX, "demux")


FILTER_TEMPLATE(open, FILTER_OPEN, "open")


FILTER_TEMPLATE(close, FILTER_CLOSE, "close")


FILTER_TEMPLATE(read, FILTER_READ, "read")


FILTER_TEMPLATE(write, FILTER_WRITE, "write")


FILTER_TEMPLATE(ctrl, FILTER_CTRL, "ctrl")


FILTER_TEMPLATE(seek, FILTER_SEEK, "seek")


static int
filter_was_inited (st_filter_chain_t *fc, int id)
{
  int x = 0;
  
  for (; fc->inited[x]; x++)
    if (fc->inited[x] == id)
      return 1;

  return 0;
}


static int
filter_inited (st_filter_chain_t *fc, void *o, int id)
{
  const st_filter_t *f = NULL;
  int x = 0;

  if (filter_was_inited (fc, id)) // don't init filters twice
    return 0;

  f = filter_get_filter_by_id (fc, id);
  if (f)
    if (f->init)
      fc->result[fc->pos] = f->init (o);

  for (; fc->inited[x]; x++) // get next free marker
    if (fc->inited[x] == id) // already marked as inited
      return fc->result[fc->pos];

  fc->inited[x] = id;

  return fc->result[fc->pos];
}


int
filter_init (st_filter_chain_t *fc, void *o, const int *id)
{
  int result = 0;

  // set current op
  fc->op = FILTER_INIT;

  if (id)
    {
      for (fc->pos = 0; fc->pos < FILTER_MAX && id[fc->pos]; fc->pos++)
        if (filter_inited (fc, o, id[fc->pos]) == -1) // TODO: skips?
          result = -1; 
    }
  else
    {
      for (fc->pos = 0; fc->pos < FILTER_MAX && fc->all[fc->pos]; fc->pos++)
        if (filter_inited (fc, o, fc->all[fc->pos]->id) == -1)
          result = -1;
    }

  return result;
}


int
filter_quit (st_filter_chain_t *fc, void *o)
{
  int x = 0;

  // set current op
  fc->op = FILTER_QUIT;
  
  for (x = fc->all_total; x-- > 0;) // quit filters in reverse order
    if (filter_was_inited (fc, fc->all[x]->id))
      if (fc->all[x]->quit)
        {
#ifdef  TEST
//          FILTER_TEST(fc, fc->all[x]->id_s);
#endif

          fc->all[x]->quit (o);
        }

  memset (&fc->inited, 0, sizeof (int) * FILTER_MAX);
  
  return 0; // always success
}


int
filter_get_filter_total (const st_filter_chain_t *fc)
{
  return fc->all_total;
}


const st_filter_t *
filter_get_filter_by_id (const st_filter_chain_t *fc, int id)
{
  int x = 0;

  for (; fc->all[x]; x++)
    if (fc->all[x]->id == id)
      return fc->all[x];

  return NULL;
}


const st_filter_t *
filter_get_filter_by_pos (const st_filter_chain_t *fc, int pos)
{
  int x = 0;

  for (; fc->all[x]; x++)
    if (fc->all[x]->id == fc->set[pos])
      return fc->all[x];

  return NULL;
}


const st_filter_t *
filter_get_filter_by_magic (const st_filter_chain_t *fc, const unsigned char *magic, int magic_len)
{
  int x = 0;

  if (magic && magic_len > 0)
    for (; fc->all[x]; x++)
      {
        int m_len = fc->all[x]->magic_len;

        if (m_len == -1)
          m_len = strlen (fc->all[x]->magic);

        if (fc->all[x]->magic && m_len > 0)
          if (!memcmp (fc->all[x]->magic, magic, magic_len) ||
              memmem2 (fc->all[x]->magic, m_len, magic, magic_len, 0))
          return fc->all[x];
      }
  return NULL;
}


const char *
filter_get_all_id_s_in_array (const st_filter_t **f)
{
  static char buf[MAXBUFSIZE];
  char *p = buf;
  int x = 0;

  *p = 0;
  for (; f[x]; x++)
    if (f[x]->id_s)
      {
        p = strchr (p, 0);
        sprintf (p, "%s, ", f[x]->id_s);
      }
    
  // remove last ','
  p = strchr (p, 0);
  if (p)
    *(p - 2)= 0;
          
  return buf;
}


const char *
filter_get_all_id_s_in_chain (const st_filter_chain_t *fc)
{
  (void) fc;
//  return filter_get_all_id_s_in_array (&fc->all);
  return "";
}


int
filter_get_total (const st_filter_chain_t *fc)
{
  return fc->total;
}


int
filter_get_pos (const st_filter_chain_t *fc)
{
  return fc->pos;
}


int
filter_get_pos_by_id (const st_filter_chain_t *fc, int id)
{
  int x = 0;
  int pos = 0;
  
  // a filter chain can have dupe id's when a filter is used more
  // than once
  for (; fc->set[x] && x < fc->pos; x++)
    if (fc->set[x] == id)
      pos++;

  return MAX (pos - 1, 0);
}


int
filter_get_id (const st_filter_chain_t *fc, int pos)
{
  return fc->set[pos];
}


const char *
filter_get_id_s (const st_filter_chain_t *fc, int pos)
{
  const st_filter_t *p = filter_get_filter_by_id (fc, fc->set[pos]);

  if (p)
    return p->id_s;
  return NULL;
}


int
filter_get_op (const st_filter_chain_t *fc)
{
  return fc->op;
}
  


int
filter_get_result (const st_filter_chain_t *fc, int pos)
{
  return fc->result[fc->set[pos]];
}


time_t
filter_get_start_time (const st_filter_chain_t *fc)
{
  return fc->start_time[filter_get_pos (fc)];
}


#define FILTER_KEY_SEP ":"
char *
filter_generate_key (int *pos, int *id, int *subkey)
{
  static char key_temp[MAXBUFSIZE];

  *key_temp = 0;
  
  if (pos)
    sprintf (key_temp, "%d", *pos);

  if (id)
    sprintf (strchr (key_temp, 0), "%s%d", pos ? FILTER_KEY_SEP : "", *id);

  if (subkey)
    sprintf (strchr (key_temp, 0), "%s%d", (pos || id) ? FILTER_KEY_SEP : "", *subkey);

  return key_temp;
}


#if 1
char *
filter_get_key (st_filter_chain_t *fc, int *subkey)
{
#if 0
  int pos = filter_get_pos_by_id (fc, fc->set[fc->pos]);
#else
  int pos = fc->pos;
#endif

  // subkey is optional and used if more than one object is
  // get/set read/write from inside a filter

  switch (fc->op)
    {
      // init() and quit() functions of filters share the same key
      // no matter how often these filters repeat in the filter chain
      // therefore the key doesn't include the position of the filter
      // in the filter chain
      case FILTER_INIT:
      case FILTER_QUIT:
        return filter_generate_key (NULL, &fc->set[fc->pos], subkey);

      default:
        return filter_generate_key (&pos, &fc->set[fc->pos], subkey);
    }
}
#else
char *
filter_get_key (st_filter_chain_t *fc, int *subkey)
{
  static char key_temp[MAXBUFSIZE];

  // subkey is optional and used if more than one object is
  // get/set read/write from inside a filter
  if (!subkey)
    {
      sprintf (key_temp, "%d" FILTER_KEY_SEP "%d", fc->pos, fc->set[fc->pos]);
//      sprintf (key_temp, "%d" FILTER_KEY_SEP "%d", filter_get_pos_by_id (fc, fc->set[fc->pos]), fc->set[fc->pos]);

      return key_temp;
    }

  switch (fc->op)
    {
      // init() and quit() functions of filters share the same key
      // no matter how often these filters repeat in the filter chain
      // therefore the key doesn't include the position of the filter
      // in the filter chain
      case FILTER_INIT:
      case FILTER_QUIT:
        break;

      default:
        sprintf (key_temp, "%d" FILTER_KEY_SEP, filter_get_pos_by_id (fc, fc->set[fc->pos]));
    }

  sprintf (strchr (key_temp, 0), "%d" FILTER_KEY_SEP "%d", fc->set[fc->pos], *subkey);

  return key_temp;
}
#endif
