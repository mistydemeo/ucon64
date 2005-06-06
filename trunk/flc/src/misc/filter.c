/*
filter.c - Simple filter and cache framework for any file, stream or data
           processing application
                      
written by 2005 NoisyB (noisyb@gmx.net)


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


static int
filter_result (st_filter_chain_t *f, const char *operation_s)
{
  if (f->debug)
    if (operation_s)
      {
        fprintf (stderr, "\nfilter: %s->%s()\n", f->active[f->pos]->id_s, operation_s);
        fflush (stderr);
      }

  if (f->result[f->pos] == -1)
    {
      if (f->verbose)
        fprintf (stderr, "ERROR: could not %s %s filter (failed)\n", operation_s, f->active[f->pos]->id_s);
      return -1;
    }

  if (f->result[f->pos] > 0)
    if (f->verbose)
      fprintf (stderr, "ERROR: could not %s %s filter (skipped; result: %d)\n", operation_s, f->active[f->pos]->id_s, f->result[f->pos]);

  return 0;
}


void
filter_st_filter_chain_t_sanity_check (st_filter_chain_t *f)
{
#ifndef DEBUG
  (void) f;
#else
  int x = 0;
  int y = 0;
  
  for (; f->all[x]; x++)
    {
      printf ("filter: %d\n", x);
      printf ("filter->id: %d\n", f->all[x]->id);
      printf ("filter->id_s: %s\n", f->all[x]->id_s);
      printf ("filter->magic: %s\n", f->all[x]->magic ? "!NULL" : "NULL");
      printf ("filter->magic_len: %d\n", f->all[x]->magic_len);
      printf ("filter->init(): %s\n", f->all[x]->init ? "Yes" : "No");
      printf ("filter->quit(): %s\n", f->all[x]->quit ? "Yes" : "No");
      for (y = 0; f->active[y]; y++)
        if (f->all[x]->id == f->active[y]->id)
          break;
      printf ("filter->active: %d\n", f->all[x]->id == f->active[y]->id ? 1 : 0);
      printf ("\n");
    }
#endif
}


st_filter_chain_t *
filter_malloc_chain (const st_filter_t **filter)
{
#ifdef  DEBUG
  int x = 0;
#endif
  int total = 0;
  st_filter_chain_t *f = NULL;

  if (!(f = (st_filter_chain_t *) malloc (sizeof (st_filter_chain_t))))
    return NULL;

  memset (f, 0, sizeof (st_filter_chain_t));

  for (total = 0; total < FILTER_MAX && filter[total]; total++);

#if 0
  if (!(f->all = (st_filter_t **) malloc (total * sizeof (st_filter_t *))))
    {
      free (f);
      return NULL;
    }
#endif

  for (f->total = 0; f->total < total; f->total++)
    if ((f->all[f->total] = (st_filter_t *) malloc (sizeof (st_filter_t))))
      {
        memcpy (f->all[f->total], filter[f->total], sizeof (st_filter_t));
        f->active[f->total] = f->all[f->total]; // by default ALL total are active
      }
    else
      {
        filter_free_chain (f);
        return NULL;
      }

//  f->all = filter;
//  f->object = object;
//  f->start_time = time (0);
//  f->verbose = verbose;
//#define DEBUG
#ifndef  DEBUG
  f->verbose = 0;
  f->debug = 0;
#else
  f->verbose = 1;
  f->debug = 1;
#endif
#ifdef  DEBUG
#undef  DEBUG
#endif

#ifdef  DEBUG
  for (x = 0; x < f->total; x++)
    printf ("%d %s\n", f->active[x]->id, f->active[x]->id_s);
#endif

  return f;
}


void
filter_free_chain (st_filter_chain_t *f)
{
  while (f->total > 0)
    {
      free (f->all[f->total - 1]);
//      f->all[f->total - 1] = NULL;
      f->total--;
    }
  free (f);
//  f = NULL;
}


int
filter_set_active (st_filter_chain_t *f, const int *id)
{
  st_filter_t *filter = NULL;
#ifdef  DEBUG
  int x = 0;
#endif

  memset (f->active, 0, sizeof (st_filter_t *) * FILTER_MAX);
    
  // enable filters
  if (!id) // ALL filters active
    {
      for (f->total = 0; f->total < FILTER_MAX && f->all[f->total]; f->total++)
        f->active[f->total] = f->all[f->total];
    }
  else
    {
      int total = 0;

      for (f->total = 0; f->total < FILTER_MAX && id[f->total]; f->total++)
        if ((filter = (st_filter_t *) filter_get_filter_by_id (f, id[f->total])))
          f->active[total++] = filter;

      f->total = total;
    }

#ifdef  DEBUG
  for (x = 0; f->active[x]; x++)
    printf ("%d %s\n", f->active[x]->id, f->active[x]->id_s);
  fflush (stdout);
#endif
  
  return 0;
}


int
filter_init (st_filter_chain_t *f, void *object)
{
  for (f->pos = 0; f->all[f->pos]; f->pos++)
    if (f->all[f->pos]->init)
      {
//        f->result[f->pos] = f->all[f->pos]->init (object);
        f->all[f->pos]->init (object);

//        if (filter_result (f, "init") == -1)
//          return -1;
      }

  f->inited = 1;

  return 0;
}


// quit filters reverse
int
filter_quit (st_filter_chain_t *f, void *object)
{
  int total = 0;

  if (!f->inited)
    return 0;

  for (; f->all[total]; total++);

  for (f->pos = total; f->pos-- > 0;)
    if (f->all[f->pos]->quit) \
      {
//        f->result[f->pos] = f->all[f->pos]->quit (object);
        f->all[f->pos]->quit (object);

//        if (filter_result (f, "quit") == -1)
//          return -1;
      }

  return 0;
}


// init() and quit() were special cases; the rest is stereo-type


#define FILTER_TEMPLATE(operation, operation_s) int \
filter_##operation (st_filter_chain_t *f, void *object) \
{ \
  for (f->pos = 0; f->pos < f->total; f->pos++) \
    if (f->active[f->pos]->operation) \
      { \
        f->result[f->pos] = f->active[f->pos]->operation (object); \
\
        if (filter_result (f, operation_s) == -1) \
          return -1; \
      } \
\
  return 0; \
}


#define FILTER_TEMPLATE_REV(operation, operation_s) int \
filter_##operation (st_filter_chain_t *f, void *object) \
{ \
  for (f->pos = f->total; f->pos-- > 0;) \
    if (f->active[f->pos]->operation) \
      { \
        f->result[f->pos] = f->active[f->pos]->operation (object); \
\
        if (filter_result (f, operation_s) == -1) \
          return -1; \
      } \
\
  return 0; \
}


FILTER_TEMPLATE(open, "open")


// close filters reverse
FILTER_TEMPLATE_REV(close, "close")


FILTER_TEMPLATE(read, "read")
//FILTER_TEMPLATE(read, NULL)


FILTER_TEMPLATE(write, "write")
//FILTER_TEMPLATE(write, NULL)


FILTER_TEMPLATE(ctrl, "ctrl")


FILTER_TEMPLATE(seek, "seek")


int
filter_get_total (const st_filter_chain_t *f)
{
#if 0
  int total = 0

  for (; f->active[total]; total++);

  return total;
#else
  return f->total;
#endif
}


int
filter_get_pos (const st_filter_chain_t *f)
{
  return f->pos;
}


int
filter_get_id (const st_filter_chain_t *f, int pos)
{
  return f->active[pos]->id;
}


const char *
filter_get_id_s (const st_filter_chain_t *f, int pos)
{
  return f->active[pos]->id_s;
}


int
filter_get_result (const st_filter_chain_t *f, int pos)
{
  return f->result[pos];
}


time_t
filter_get_start_time (const st_filter_chain_t *f)
{
  return f->start_time;
}


const st_filter_t *
filter_get_filter_by_id (const st_filter_chain_t *f, int id)
{
  int x = 0;

  for (; f->all[x]; x++)
    if (f->all[x]->id == id)
      return f->all[x];

  return NULL;
}


const st_filter_t *
filter_get_filter_by_pos (const st_filter_chain_t *f, int pos)
{
  return f->all[pos];
}


const st_filter_t *
filter_get_filter_by_magic (const st_filter_chain_t *f, const unsigned char *magic, int magic_len)
{
  int x = 0;

  if (magic && magic_len > 0)
    for (; f->all[x]; x++)
      if (f->all[x]->magic && f->all[x]->magic_len > 0)
        if (!memcmp (f->all[x]->magic, magic, magic_len) ||
            memmem2 (f->all[x]->magic, f->all[x]->magic_len, magic, magic_len, 0))
          return f->all[x];

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
filter_get_all_id_s_in_chain (const st_filter_chain_t *f)
{
//  return filter_get_all_id_s_in_array (&f->all);
  return "";
}





#if 0








static void *
shmem_alloc (int size)
{
  return malloc (size);
}


void
shmem_free (void *p, int size)
{
  (void) size;
  free (p);
}











// Initial draft of my new cache system...
// Note it runs in 2 processes (using fork()), but doesn't requires locking!!
// TODO: seeking, data consistency checking

#define READ_USLEEP_TIME 10000
#define FILL_USLEEP_TIME 50000
#define PREFILL_SLEEP_TIME 200
#define STREAM_BUFFER_SIZE 2048




//extern int mp_input_check_interrupt (int time);



typedef struct
{
  // constats:
  unsigned char *buffer;        // base pointer of the allocated buffer memory
  int buffer_size;              // size of the allocated buffer memory
  int sector_size;              // size of a single sector (2048/2324)
  int back_size;                // we should keep back_size amount of old bytes for backward seek
  int fill_limit;               // we should fill buffer only if space>=fill_limit
  int prefill;                  // we should fill min prefill bytes if cache gets empty

  // filler's pointers:
  int eof;
  unsigned long min_filepos;            // buffer contain only a part of the file, from min-max pos
  unsigned long max_filepos;
  unsigned long offset;                 // filepos <-> bufferpos  offset value (filepos of the buffer's first byte)

  // reader's pointers:
  unsigned long read_filepos;

} cache_vars_t;

//static int min_fill = 0;
//int cache_fill_status = 0;
static int fp = 0;
static unsigned int buf_pos, buf_len;
static unsigned long pos;
static int eof;
static unsigned int cache_pid;

static cache_vars_t *cache_data;
static unsigned char buffer[STREAM_BUFFER_SIZE];


#if 0
void
cache_stats (cache_vars_t * s)
{
  int newb = s->max_filepos - s->read_filepos;  // new bytes in the buffer

  printf ("0x%06X  [0x%06X]  0x%06X   ",
    (int) s->min_filepos,
    (int) s->read_filepos,
    (int) s->max_filepos);

  printf ("%3d %%  (%3d%%)\n",
    100 * newb / s->buffer_size,
    100 * min_fill / s->buffer_size);
}
#endif


int
cache_read (cache_vars_t * s, unsigned char *buf, int size)
{
  int total = 0;

  while (size > 0)
    {
      int pos, newb, len;

      printf("CACHE2_READ: 0x%X <= 0x%X <= 0x%X  \n",s->min_filepos,s->read_filepos,s->max_filepos);

      if (s->read_filepos >= s->max_filepos ||
          s->read_filepos < s->min_filepos)
        {
          // eof?
          if (s->eof)
            break;
          // waiting for buffer fill...
          wait2 (READ_USLEEP_TIME / 1000);
          continue;             // try again...
        }

      newb = s->max_filepos - s->read_filepos;  // new bytes in the buffer
//      if (newb < min_fill)
//        min_fill = newb;        // statistics...

     printf("*** newb: %d bytes ***\n",newb);

      pos = s->read_filepos - s->offset;
      if (pos < 0)
        pos += s->buffer_size;
      else if (pos >= s->buffer_size)
        pos -= s->buffer_size;

      if (newb > s->buffer_size - pos)
        newb = s->buffer_size - pos;    // handle wrap...
      if (newb > size)
        newb = size;

      // check:
      if (s->read_filepos < s->min_filepos)
        fprintf (stderr, "Ehh. s->read_filepos<s->min_filepos !!! Report bug...\n");

      // len=write(mem,newb)
      //printf("Buffer read: %d bytes\n",newb);
      memcpy (buf, &s->buffer[pos], newb);
      buf += newb;
      len = newb;
      // ...

      s->read_filepos += len;
      size -= len;
      total += len;

    }
//  cache_fill_status = 100 * (s->max_filepos - s->read_filepos) / s->buffer_size;

  return total;
}


int
cache_fill (cache_vars_t * s)
{
  int back, back2, newb, space, len, posi;

  if (s->read_filepos < s->min_filepos ||
      s->read_filepos > s->max_filepos)
    {
      // seek...
      fprintf (stderr, "Out of boundaries... seeking to 0x%X  \n", s->read_filepos);

      // streaming: drop cache contents only if seeking backward or too much fwd:
      if (s->read_filepos < s->min_filepos || s->read_filepos >= s->max_filepos + s->buffer_size)
        {
          s->offset = s->min_filepos = s->max_filepos = s->read_filepos;     // drop cache content :(
          if(eof)
            {
              pos=0;
              eof=0;
            }
  
          fprintf (stderr, "Seek to new pos: 0x%X  \n",
             (int) lseek (fp, s->read_filepos, SEEK_SET));
        }
    }

  // calc number of back-bytes:
  back = MAX (s->read_filepos - s->min_filepos, 0); // strange...
  back = MIN (back, s->back_size);

  // calc number of new bytes:
  newb = MAX (s->max_filepos - s->read_filepos, 0); // strange...

  // calc free buffer space:
  space = s->buffer_size - (newb + back);

  // calc bufferpos:
  posi = s->max_filepos - s->offset;
  if (posi >= s->buffer_size)
    posi -= s->buffer_size;      // wrap-around

  if (space < s->fill_limit)
    {
      printf("Buffer is full (%d bytes free, limit: %d)\n",space,s->fill_limit);
      return 0;                 // no fill...
    }

  printf("### read=0x%X  back=%d  newb=%d  space=%d  pos=%d\n",s->read_filepos,back,newb,space,posi);

  // reduce space if needed:
  space = MIN (space, s->buffer_size - posi);
  space = MIN (space, 4 * s->sector_size);

  // back+newb+space <= buffer_size
  back2 = s->buffer_size - (space + newb);      // max back size

  s->min_filepos = MAX (s->min_filepos, (s->read_filepos - back2));

  printf("Buffer fill: %d bytes of %d\n",space,s->buffer_size);

  len = read (fp, &s->buffer[posi], space);

  if (!len)
    s->eof = 1;

  s->max_filepos += len;
  if (posi + len >= s->buffer_size)
    {
      // wrap...
      s->offset += s->buffer_size;
    }

  return len;
}


cache_vars_t *
cache_open (int size, int sector)
{
  int num;
  cache_vars_t *s = shmem_alloc (sizeof (cache_vars_t));

  memset (s, 0, sizeof (cache_vars_t));

  num = size / sector;
  s->buffer_size = num * sector;
  s->sector_size = sector;
  s->buffer = shmem_alloc (s->buffer_size);
  s->fill_limit = 8 * sector;
  s->back_size = size / 2;
  s->prefill = size / 20;       // default: 5%

  return s;
}


void
cache_close (void)
{
  if (!cache_pid)
    return;
  kill (cache_pid, SIGKILL);
  waitpid (cache_pid, NULL, 0);

  if (!cache_data)
    return;

  shmem_free (cache_data->buffer, cache_data->buffer_size);
  shmem_free (cache_data, sizeof (cache_vars_t));
}


int
stream_enable_cache (int size, int min, int prefill)
{
  int ss = STREAM_BUFFER_SIZE;

  if (fp < 0)
    {
      // The stream has no 'fd' behind it, so is non-cacheable
      fprintf (stderr, "\rThis stream is non-cacheable\n");
      return 1;
    }

  if (size < 32 * 1024)
    size = 32 * 1024;           // 32kb min
  cache_data = cache_open (size, ss);
  cache_data->prefill = size * prefill;

  if ((cache_pid = fork ()))
    {
      // wait until cache is filled at least prefill_init %
      fprintf (stderr, "CACHE_PRE_INIT: %d [%d] %d  pre:%d  eof:%d  \n",
        cache_data->min_filepos,
        cache_data->read_filepos,
        cache_data->max_filepos,
        min,
        cache_data->eof);

      while (cache_data->read_filepos < cache_data->min_filepos ||
             cache_data->max_filepos - cache_data->read_filepos < min)
        {
          fprintf (stderr, "\rCache fill: %5.2f%% (%d bytes)    ",
            100.0 * (float) (cache_data->max_filepos - cache_data->read_filepos) / (float) (cache_data->buffer_size),
            cache_data->max_filepos - cache_data->read_filepos);

          if (cache_data->eof)
            break;              // file is smaller than prefill size

//          if (mp_input_check_interrupt (PREFILL_SLEEP_TIME))
//            return 0;
        }
      return 1;                 // parent exits
    }

  // child fills cache
  while (TRUE)
    {
      if (!cache_fill (cache_data))
        {
//          usec_sleep (FILL_USLEEP_TIME);        // idle
          wait2 (FILL_USLEEP_TIME / 1000);
        }
//       cache_stats(cache_data);
    }
}


int
cache_stream_fill_buffer (void)
{
  int len;
  if (eof)
    {
      buf_pos = buf_len = 0;
      return 0;
    }

  if (!cache_pid)
    return read (fp, buffer, STREAM_BUFFER_SIZE);
    

//  cache_stats(cache_data);

  if (pos != cache_data->read_filepos)
    fprintf (stderr, "!!! read_filepos differs!!! report this bug...\n");

  len = cache_read (cache_data, buffer, cache_data->sector_size);

  //printf("cache_stream_fill_buffer->read -> %d\n",len);

  if (len <= 0)
    {
      eof = 1;
      buf_pos = buf_len = 0;
      return 0;
    }
  buf_pos = 0;
  buf_len = len;
  pos += len;
//  printf("[%d]",len);fflush(stdout);
  return len;

}


int
cache_stream_seek_long (unsigned long posi)
{
  unsigned long newpos;

  fprintf (stderr,
           "CACHE2_SEEK: 0x%X <= 0x%X (0x%X) <= 0x%X  \n", cache_data->min_filepos,
           (int) posi, cache_data->read_filepos, cache_data->max_filepos);

  newpos = posi / cache_data->sector_size;
  newpos *= cache_data->sector_size;     // align
  pos = cache_data->read_filepos = newpos;
  cache_data->eof = 0;                   // !!!!!!!

  cache_stream_fill_buffer ();

  posi -= newpos;
  if (posi >= 0 && posi <= buf_len)
    {
      buf_pos = posi;    // byte position in sector
      return 1;
    }

//  stream->buf_pos=stream->buf_len=0;
//  return 1;

  fprintf (stderr,
           "cache_stream_seek: WARNING! Can't seek to 0x%llX !\n",
#ifdef _LARGEFILE_SOURCE
           (long long) (posi + newpos)
#else
           (posi + newpos)
#endif
           );
  return 0;
}





#endif











st_filter_cache_t *
filter_cache_open (int buffers, unsigned long buffer_size, int cache_type)
{
  (void) cache_type;
#warning build-in pipe(), streamed_pip(), etc..
  st_filter_cache_t *fc = NULL;

  if (!(fc = (st_filter_cache_t *) malloc (sizeof (st_filter_cache_t))))
    return NULL;

  memset (fc, 0, sizeof (st_filter_cache_t));

  if (!(fc->buffer = (unsigned char **) malloc (buffers * sizeof (unsigned char *))))
    {
      free (fc);
      return NULL;
    }

  for (fc->buffers = 0; fc->buffers < buffers; fc->buffers++)
    if (!(fc->buffer[fc->buffers] = (unsigned char *) malloc (buffer_size)))
      {
        filter_cache_close (fc);
        return NULL;
      }

  fc->buffer_size = buffer_size;

  return fc;
}


int
filter_cache_close (st_filter_cache_t *fc)
{
  while (fc->buffers > 0)
    {
      free (fc->buffer[fc->buffers - 1]);
      fc->buffers--;
    }
  free (fc->buffer);
  free (fc);
  
  return 0;
}


int
filter_cache_write (st_filter_cache_t *fc, unsigned char *buffer, unsigned long buffer_len)
{
  int len = 0;
  unsigned long x;

  while (buffer_len > 0)
    {
      if (fc->full == fc->buffers)
        {
          wait2 (50);
          continue; // break?
        }

      x = MIN (fc->buffer_size - fc->write_pos, buffer_len);

      // read
      memcpy (fc->buffer[fc->write_buf] + fc->write_pos, buffer + len, x);

      if (!fc->write_pos)
        fc->full++;

      len += x;
      buffer_len -= x;
      fc->len += x;
      fc->write_pos += x;

      // block is full, find next
      if (fc->write_pos >= fc->buffer_size)
        {
          fc->write_buf = (fc->write_buf + 1) % fc->buffers;
//          fc->full++;
          fc->write_pos = 0;
        }
    }
  return len;
}


int
filter_cache_read (st_filter_cache_t *fc, unsigned char *buffer, unsigned long buffer_len)
{
  int len = 0;
  unsigned long x;

  while (buffer_len > 0)
    {
      if (!fc->len) // EOF
        break;

      x = MIN (fc->buffer_size - fc->read_pos, buffer_len);
      x = MIN (x, fc->len);

      // write
      memcpy (buffer + len, fc->buffer[fc->read_buf] + fc->read_pos, x);

      len += x;
      buffer_len -= x;
      fc->len -= x;
      fc->read_pos += x;

      // block is empty, find next
      if (fc->read_pos >= fc->buffer_size)
        {
          fc->read_buf = (fc->read_buf + 1) % fc->buffers;
          fc->full--;
          fc->read_pos = 0;
        }
    }
  return len;
}


int
filter_cache_read_cb (st_filter_cache_t *fc, int (*write_func) (unsigned char *, unsigned long), unsigned long buffer_len)
{
  int len = 0;
  unsigned long x;

  while (buffer_len > 0)
    {
      if (!fc->len) // EOF
        break;

      x = MIN (fc->buffer_size - fc->read_pos, buffer_len);
      x = MIN (x, fc->len);

      write_func (fc->buffer[fc->read_buf] + fc->read_pos, x);

      len += x;
      buffer_len -= x;
      fc->len -= x;
      fc->read_pos += x;

      // block is empty, find next
      if (fc->read_pos >= fc->buffer_size)
        {
          fc->read_buf = (fc->read_buf + 1) % fc->buffers;
          fc->full--;
          fc->read_pos = 0;
        }
    }
  return len;
}


void
filter_cache_pause (st_filter_cache_t *fc)
{
  fc->pause = fc->pause ? 0 : 1;
}
