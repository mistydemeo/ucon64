/*
filter.c - Simple filter framework for any file, stream or data
           processing application
                      
written by 2005 NoisyB


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
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>  // waitpid()
#include "misc/string.h"
#include "misc/misc.h"
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


#define FILTER_DEBUG(fc,s) if(fc->debug){printf("\n%s\n\n", s);fflush(stdout);}
#define FILTER_VERBOSE(fc,s) if(fc->verbose){printf("\n%s\n\n", s);fflush(stdout);}


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


int
filter_set_chain (st_filter_chain_t *fc, const int *id)
{
  memset (&fc->set, 0, sizeof (int) * FILTER_MAX);

  // enable filters
  if (!id) // set ALL filters (default)
    {
      for (fc->total = 0; fc->total < FILTER_MAX && fc->all[fc->total]; fc->total++)
        fc->set[fc->total] = fc->all[fc->total]->id;
    }
  else
    {
      for (fc->total = 0; fc->total < FILTER_MAX && id[fc->total]; fc->total++)
        fc->set[fc->total] = id[fc->total];
    }

//filter_st_filter_chain_t_sanity_check (fc);

#if 1
  fc->verbose = 0;
  fc->debug = 0;
#else
  fc->verbose = 1;
  fc->debug = 1;
#endif

  return 0;
}


st_filter_chain_t *
filter_malloc_chain (/* int max_filter_chains, int max_filters, int max_objects, */const st_filter_t **filter)
{
  int total = 0;
  st_filter_chain_t *fc = NULL;
  int id_chain[FILTER_MAX];

  if (!(fc = (st_filter_chain_t *) malloc (sizeof (st_filter_chain_t))))
    return NULL;

  memset (fc, 0, sizeof (st_filter_chain_t));

  for (total = 0; total < FILTER_MAX && filter[total]; total++);

#if 0
  if (!(fc->all = (st_filter_t **) malloc (total * sizeof (st_filter_t *))))
    {
      free (fc);
      return NULL;
    }
#endif

  // malloc chain
  for (fc->total = 0; fc->total < total; fc->total++)
    if ((fc->all[fc->total] = (st_filter_t *) malloc (sizeof (st_filter_t))))
      {
        memcpy (fc->all[fc->total], filter[fc->total], sizeof (st_filter_t));
        id_chain[fc->total] = fc->all[fc->total]->id; // by default ALL total are set
      }
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
  while (fc->total > 0)
    {
      free (fc->all[fc->total - 1]);
//      fc->all[fc->total - 1] = NULL;
      fc->total--;
    }
  free (fc);
//  fc = NULL;
}


static int
filter_func (st_filter_chain_t *fc, void *o, int operation, const char *operation_s)
{
  const st_filter_t *f = NULL;

  if (fc->debug)
    if (operation_s)
      {
        fprintf (stderr, "\nfilter: %s->%s()\n", filter_get_filter_by_id (fc, fc->set[fc->pos])->id_s, operation_s);
        fflush (stderr);
      }

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
      if (fc->verbose)
        fprintf (stderr, "ERROR: could not %s %s filter (failed)\n", operation_s, filter_get_filter_by_id (fc, fc->set[fc->pos])->id_s);
      return -1;
    }

  if (fc->result[fc->pos] > 0)
    if (fc->verbose)
      fprintf (stderr, "ERROR: could not %s %s filter (skipped; result: %d)\n", operation_s, filter_get_filter_by_id (fc, fc->set[fc->pos])->id_s, fc->result[fc->pos]);

  return 0;
}


static int
filter_func_ctrl (st_filter_chain_t *fc, void *o, int operation, const char *operation_s)
{
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

#if 0
FILTER_TEMPLATE(init, FILTER_INIT, "init")


FILTER_TEMPLATE(quit, FILTER_QUIT, "quit")
#else
int
filter_init (st_filter_chain_t *fc, void *o, const int *id)
{
  fc->op = FILTER_INIT;

#if 1
  if (!id) // init ALL filters (default)
    {
      for (fc->pos = 0; fc->pos < FILTER_MAX && fc->all[fc->pos]; fc->pos++)
        {
          const st_filter_t *f = filter_get_filter_by_id (fc, fc->all[fc->pos]->id);
                
          if (f)
            if (f->init)
              fc->result[fc->pos] = f->init (o);

          fc->inited[fc->pos] = 1;
        }
    }
  else
    {
      for (fc->pos = 0; fc->pos < FILTER_MAX && id[fc->pos]; fc->pos++)
        {
          const st_filter_t *f = filter_get_filter_by_id (fc, id[fc->pos]);

          if (f)
            if (f->init)
              fc->result[fc->pos] = f->init (o);

          fc->inited[fc->pos] = 1;
        }
    }
#endif


#if 0
  // init _always_ ALL filters?
  for (fc->pos = 0; fc->all[fc->pos]; fc->pos++)
    if (fc->all[fc->pos]->init)
      {
        FILTER_DEBUG(fc, fc->all[fc->pos]->id_s);

        fc->result[fc->pos] = fc->all[fc->pos]->init (o);
        fc->inited[fc->pos] = 1;
      }
#endif


#if 0
  // init only the needed filters?
  for (fc->pos = 0; fc->set[fc->pos]; fc->pos++)
    {
      const st_filter_t *f = filter_get_filter_by_id (fc, fc->set[fc->pos]);

      FILTER_DEBUG(fc, f->id_s);

      if (f)
        if (f->init)
          fc->result[fc->pos] = f->init (o);
      fc->inited[fc->pos] = 1;
    }                  
#endif

  return 0;
}


int
filter_quit (st_filter_chain_t *fc, void *o)
{
  int total = 0;
  
  fc->op = FILTER_QUIT;
  
  for (; fc->all[total]; total++);

  for (fc->pos = total; fc->pos-- > 0;) // reverse
    if (fc->inited[fc->pos])
      if (fc->all[fc->pos]->quit)
        {
//          FILTER_DEBUG(fc, fc->all[fc->pos]->id_s);

          fc->all[fc->pos]->quit (o);
        }

  return 0;
}
#endif


int
filter_object_write (st_filter_chain_t *fc,
                     void *key,
                     unsigned short key_len,
                     void *o,
                     unsigned long o_len)
{
  int x = 0;
  char key_temp[MAXBUFSIZE];

  // default key
  if (!key || !key_len)
    {
      sprintf (key_temp, "%d:%d", fc->pos, fc->set[fc->pos]);
      key = key_temp;
      key_len = strlen (key_temp);
    }

#ifdef  DEBUG
  printf ("filter_cache_write(): %s %d %s %ld\n", (char *) key, key_len, o, o_len);
  fflush (stdout); 
#endif

  for (x = 0; x < FILTER_MAX_OBJ; x++)
  {
#ifdef  DEBUG
    printf ("filter_cache_write().2: %d %s %d %s %d\n", x, (char *) fc->o[x].key, fc->o[x].key_len, key, key_len);
    fflush (stdout); 
#endif

    if ((fc->o[x].key && !memcmp (fc->o[x].key, key, key_len) && fc->o[x].key_len == key_len && fc->o[x].key) ||
        !fc->o[x].key)
      {
        if (!fc->o[x].key)
          if (!(fc->o[x].key = malloc (key_len)))
            { 
              memset (&fc->o[x], 0, sizeof (st_filter_object_t));
              return -1;
            }
        
        memcpy (fc->o[x].key, key, key_len);
        fc->o[x].key_len = key_len;

        if (fc->o[x].o)
          free (fc->o[x].o);

        if (!(fc->o[x].o = malloc (o_len)))
          {
            free (fc->o[x].key);
            memset (&fc->o[x], 0, sizeof (st_filter_object_t));
            return -1;
          }

        memcpy (fc->o[x].o, o, o_len);
        fc->o[x].o_len = o_len;

            
        return 0;
      }
}
  return -1;
}


int
filter_object_read (st_filter_chain_t *fc,
                    void *key,
                    unsigned short key_len,
                    void *o,
                    unsigned long o_len)
{
  int x = 0;
  char key_temp[MAXBUFSIZE];
  
  // default key
  if (!key || !key_len)
    {
      sprintf (key_temp, "%d:%d", fc->pos, fc->set[fc->pos]);
      key = key_temp;
      key_len = strlen (key_temp);
    }

  for (; x < FILTER_MAX_OBJ && fc->o[x].key; x++)
    {
#ifdef  DEBUG
      printf ("filter_cache_read(): %s %d == %s %d\n", (char *) fc->o[x].key, fc->o[x].key_len, key, key_len);
      fflush (stdout);
#endif

      if (fc->o[x].key_len == key_len)
        if (!memcmp (fc->o[x].key, key, key_len))
          {
            memcpy (o, fc->o[x].o, MIN (o_len, fc->o[x].o_len));

#ifdef  DEBUG
            printf ("filter_cache_read().2->object: %s", (char *) o);
            fflush (stdout);
#endif            
            return 0;
          }
    }

  return -1;
}


int
filter_object_read_by_id (st_filter_chain_t *fc,
                          int o_id,
                          void *o,
                          unsigned long o_len)
{
  char key[MAXBUFSIZE];
  int pos = filter_get_pos (fc);
  int id = filter_get_id (fc, pos);

  switch (fc->op)
    {
      case FILTER_INIT:
      case FILTER_QUIT:
        sprintf (key, "%d:%d", filter_get_id (fc, pos), o_id);
        break;
        
      default:
        sprintf (key, "%d:%d:%d", filter_get_pos_by_id (fc, id), id, o_id);
    }

  return filter_object_read (fc, key, strlen (key) + 1, o, o_len);
}


int
filter_object_write_by_id (st_filter_chain_t *fc,
                           int o_id,
                           void *o,
                           unsigned long o_len)
{
  char key[MAXBUFSIZE];
  int pos = filter_get_pos (fc);
  int id = filter_get_id (fc, pos);

  switch (fc->op)
    {
      case FILTER_INIT:
      case FILTER_QUIT:
        sprintf (key, "%d:%d", filter_get_id (fc, pos), o_id);
        break;
        
      default:
        sprintf (key, "%d:%d:%d", filter_get_pos_by_id (fc, id), id, o_id);
    }

  return filter_object_write (fc, key, strlen (key) + 1, o, o_len);
}


int
filter_get_total (const st_filter_chain_t *fc)
{
#if 0
  int total = 0

  for (; fc->set[total]; total++);

  return total;
#else
  return fc->total;
#endif
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
  return filter_get_filter_by_id (fc, fc->set[pos])->id_s;
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
  return fc->all[fc->set[pos]];
}


const st_filter_t *
filter_get_filter_by_magic (const st_filter_chain_t *fc, const unsigned char *magic, int magic_len)
{
  int x = 0;

  if (magic && magic_len > 0)
    for (; fc->all[x]; x++)
      if (fc->all[x]->magic && fc->all[x]->magic_len > 0)
        if (!memcmp (fc->all[x]->magic, magic, magic_len) ||
            memmem2 (fc->all[x]->magic, fc->all[x]->magic_len, magic, magic_len, 0))
          return fc->all[x];

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
