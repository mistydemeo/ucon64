/*
filter.h - Simple filter and cache framework for any file, stream or data
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
#ifndef MISC_FILTER_H 
#define MISC_FILTER_H 
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <time.h>    // time_t
#ifdef  HAVE_LIMITS_H
#include <limits.h>  // ARG_MAX
#if     128 < ARG_MAX
#define FILTER_MAX ARG_MAX
#endif
#endif


// TODO: make FILTER_MAX dynamic (by removing it)
#ifndef FILTER_MAX
#define FILTER_MAX 128
#endif


/*
  st_filter_t
  Single filter struct
  THIS is the only struct that you will have to init in your code
*/
typedef struct
{
  int id;
  const char *id_s;       // very plain description (suggestion: max. 10 chars, acronyms)
  const void *magic;      // optional (could be file suffix)
  int magic_len;          // optional

  // return values (results) should be:
  // -1 == failed (will stop everything)
  //  0 == OK
  // >0 == skipped (that single filter will be ignored)
  int (* open) (void *);
  int (* close) (void *);

  int (* read) (void *);  // also: demux
  int (* write) (void *); // also: decode (or encode)

  int (* seek) (void *);
  int (* ctrl) (void *);

  int (* init) (void *);
  int (* quit) (void *);
} st_filter_t;


/*
  st_filter_chain_t
  The filter chain struct that will be init'ed and returned by filter_init_filters*()
*/
typedef struct
{
  st_filter_t *all[FILTER_MAX]; // ALL unsorted filters (init and quit use this)

  st_filter_t *active[FILTER_MAX]; // currently used filters (ignored by init and quit)
                                   //   set by filter_set_active() (default: ALL filters (but unsorted))
                                   //   points to filters in all[] in the order set by filter_set_active()
  int pos;               // # of current filter (active filters only)
  int total;             // # of all active filters (active filters only)
#if 1
  time_t start_time;     // start time of 1st filter (active filters only)
#else
  time_t start_time[FILTER_MAX]; // start times of all active filters (active filters only)
#endif
  int result[FILTER_MAX]; // results (active filters only) (-1 == failed, 0 == OK, >0 == skipped)
  int inited;             // filter were init()'ed (ALL filters)
  char verbose;           // print verbose messages
  char debug;             // print debug messages
} st_filter_chain_t;


/*
  filter_malloc_chain()  turn filter array into a filter chain (use only once per process)
  filter_free_chain()    free filter chain (use only once per process)

  filter_st_filter_chain_t_sanity_check()
                         try this for some transparency and to solve problems

  filter_set_active()    (de-)activate/(re-)sort needed filters in chain by an array of their id's
                           (use only once per file/data/stream)
                           filter_set_active(..., NULL) will activate ALL filters (but unsorted!)

                           NOTE: a nice aspect of filter_set_active() is that it can be
                           called from inside a filter of the actual filter chain and therefore
                           change the rest of the filter chain while running :-)
                           A demuxer (for example) could become itself(!) the first filter in a
                           chain, demux and (de-)activate/(re-)sort the rest of the chain
                           according to the demuxed type of input
                           Or a path (or url) could be parsed by the first filter the second
                           filter opens the descriptor from the file or the url the third filter
                           caches (if from url) the fourth filter demuxes and (de-)activates/(re-)sorts
                           all following filters according to the input type... etc...

  filter_init()    wrapper for st_filter_t->init (use only once per process)
                     IMPORTANT: it does NOT(!) make a difference if you call this before or after
                     filter_set_active(); always ALL filters with init() will be init()'ed
  filter_quit()    wrapper for st_filter_t->quit (use only once per process)

  filter_open()    wrapper for st_filter_t->open (use only once per file/data/stream)
  filter_close()   wrapper for st_filter_t->close (use only once per file/data/stream)

  filter_read()    wrapper for st_filter_t->read (use only once per part of file/data/stream)
  filter_write()   wrapper for st_filter_t->write (use only once per part of file/data/stream)
  filter_seek()    wrapper for st_filter_t->seek (use only once per part of file/data/stream)

  filter_ctrl()    wrapper for st_filter_t->ctrl (supposed to be used as often as necessary)

  NOTE: all wrappers will pass that (void *) argument to st_filter_t->(* func)()
*/
extern st_filter_chain_t *filter_malloc_chain (const st_filter_t **);
extern void filter_free_chain (st_filter_chain_t *);

//#ifdef  DEBUG
extern void filter_st_filter_chain_t_sanity_check (st_filter_chain_t *f);
//#endif
extern int filter_set_active (st_filter_chain_t *f, const int *id);

extern int filter_init (st_filter_chain_t *f, void *o);
extern int filter_quit (st_filter_chain_t *f, void *o);

extern int filter_open (st_filter_chain_t *f, void *o);
extern int filter_close (st_filter_chain_t *f, void *o);

extern int filter_read (st_filter_chain_t *f, void *o);
extern int filter_write (st_filter_chain_t *f, void *o);
extern int filter_seek (st_filter_chain_t *f, void *o);
extern int filter_ctrl (st_filter_chain_t *f, void *o);


/*
  filter_get_total()           get # of all active filters (active filters only)
  filter_get_pos()             get # of current filter (active filters only)
  filter_get_id()              get id of filter in pos (active filters only)
  filter_get_id_s()            get id_s of filter in pos (active filters only)
  filter_get_result()          get last result (active filters only) (-1 == failed, 0 == OK, >0 == skipped)
  filter_get_start_time()      get start time of the 1st filter (active filters only)

  filter_get_filter_by_id()    get st_filter_t by id (ALL filters)
  filter_get_filter_by_pos()   get st_filter_t by pos (ALL filters)
  filter_get_filter_by_magic() get st_filter_t by magic (ALL filters)

  filter_get_all_id_s_in_array()
                               get comma separated id_s as a string (from st_filter_t array)
  filter_get_all_id_s_in_chain()
                               get comma separated id_s as a string (ALL filters)
*/
extern int filter_get_total (const st_filter_chain_t *f);
extern int filter_get_pos (const st_filter_chain_t *f);
extern int filter_get_id (const st_filter_chain_t *f, int pos);
extern const char * filter_get_id_s (const st_filter_chain_t *f, int pos);
extern int filter_get_result (const st_filter_chain_t *f, int pos);
//extern time_t filter_get_start_time (const st_filter_chain_t *f);

extern const st_filter_t * filter_get_filter_by_id (const st_filter_chain_t *f, int id);
extern const st_filter_t * filter_get_filter_by_pos (const st_filter_chain_t *f, int pos);
extern const st_filter_t * filter_get_filter_by_magic (const st_filter_chain_t *f, const unsigned char *magic, int magic_len);

extern const char * filter_get_all_id_s_in_array (const st_filter_t **f);
extern const char * filter_get_all_id_s_in_chain (const st_filter_chain_t *f);


/*
  Cache framework
    Operations to avoid latency problems between filter_a and filter_b
    using a dynamical, build-in ring buffer, or pipe(), or stream_pipe(),
    or whatever seems best for a cache with the size of (buffers * buffer_size)
    (see filter_cache_open())

  st_filter_cache_t      cache framework struct

  filter_cache_open()    open cache
                        you set the # of buffers and the buffer_size

  filter_cache_read()    read from shared buffer
  filter_cache_read_cb() read from shared buffer to write_func
  filter_cache_write()   write to shared buffer
  filter_cache_close()   close and free cache
                       forces break (stop) and exit() of reading side of cache

  filter_cache_pause()   pause read and write side of cache
  filter_cache_unpause() unpause read and write side of cache

  Cache types
  CACHE_AUTO          select cache type automatically depending on size and other
                        factors

  CACHE_PIPE          use pipe() as cache
  CACHE_STREAM_PIPE   use stream_pipe() as cache
  CACHE_FIFO          use fifo() as cache

  Cache types that use a ring-buffer architecture to prevent expansion
  CACHE_RB_SHM        use shared memory as cache
  CACHE_RB_TMP        use a tmp file as cache
                        (with a cache of maybe ~100MB you could play mp3 on a i386 ;-)
  CACHE_RB_MALLOC     use malloc'd memory as cache
  CACHE_RB_MAP        use mapped memory as cache


  The cache framework can transport data between two filters in
  different ways to avoid latency/buffer-underrun problems

  normal:     linear transfer w/o fork or cache
                (data)-(process)->(data)
  
  callback:   same as normal but the filter writes the output to a callback function
                instead (for example) to (void *) o->data
                (data)-(process)->callback

  forked (a): fork() (every! call)
                parent: read from child() and continue until user-exit()-process
                child: write to OUT cache and exit()
                (data)->fork->child-(data)-(OUT cache)->parent

  forked (b): fork() (every! call)
                parent: write to OUT cache and exit()
                child: read from parent() and continue until user-exit()-process
                (data)->fork->parent-(data)-(OUT cache)->child

  forked (c): fork() (only once)
                parent: write to IN cache and read from OUT cache until
                         user-exit()-process
                child: read from IN cache, process and write to OUT cache until
                         user-exit()-process
                (data)->fork->parent-(data)-(IN cache)->child-(process)-(data)-(OUT cache)->parent
*/
enum
{
  CACHE_AUTO = 0,
  CACHE_RB_MALLOC,
  CACHE_RB_SHM,
  CACHE_RB_TMP,
  CACHE_RB_MAP,
  CACHE_PIPE,
  CACHE_STREAM_PIPE,
  CACHE_FIFO
};


typedef struct
{
  int buffers;                   // # of buffers with buffer_size
  unsigned long buffer_size;     // size of a single buffer
  unsigned char **buffer;        // malloc'ed buffer array

  int pause;                     // cache is paused (1==yes or 0==no)
  int full;                      // # of full buffers
  unsigned long len;             // (Bytes) total buffered bytes in all buffers

  unsigned int read_buf;         // current read buffer ("sector")
  unsigned int write_buf;        // current write buffer ("sector")

  unsigned long read_pos;        // (Bytes) current pos in current read buffer
  unsigned long write_pos;       // (Bytes) current pos in current write buffer
} st_filter_cache_t;


extern st_filter_cache_t * filter_cache_open (int buffers, unsigned long buffer_size, int cache_type);
extern int filter_cache_write (st_filter_cache_t *fc, unsigned char *buffer, unsigned long buffer_len);
extern int filter_cache_read (st_filter_cache_t *fc, unsigned char *buffer, unsigned long buffer_len);
extern int filter_cache_read_cb (st_filter_cache_t *fc, int (*write_func) (unsigned char *, unsigned long),
  unsigned long buffer_len);
extern int filter_cache_close (st_filter_cache_t *fc);

extern void filter_cache_pause (st_filter_cache_t *fc);
extern void filter_cache_unpause (st_filter_cache_t *fc);


#endif // MISC_FILTER_H
