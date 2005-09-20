/*
filter.h - Simple filter framework for any file, stream or data
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
#ifndef MISC_FILTER_H 
#define MISC_FILTER_H 
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <time.h>    // time_t
#ifdef  HAVE_LIMITS_H
#include <limits.h>  // ARG_MAX
#endif  // HAVE_LIMITS_H

#ifndef ARG_MAX
#define ARG_MAX 128
#endif

#if     128 < ARG_MAX
#define FILTER_MAX ARG_MAX
#else
#define FILTER_MAX 128
#endif

#define FILTER_MAX_OBJ (FILTER_MAX*8)


/*
  st_filter_t
  Single filter struct
  THIS is the only struct that you will have to init in your code
*/
typedef struct st_filter_t
{
  int id;
  const char *id_s;       // very plain description (suggestion: max. 10 chars, acronyms)
  const void *magic;      // optional (could be file suffix)
  int magic_len;          // optional
//  unsigned long flags;    // examples: MODE, READ, DEC, WRITE, ENC, IN, OUT, DEMUX
//  char fork;              // run filter in child process? (FALSE or TRUE)
//  void *object;           // optional

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
  st_filter_object_t
  every filter can read/write objects
TODO: make FILTER_MAX_OBJ with dynamic
*/
typedef struct
{
  void *key;                     // malloc'ed key
  unsigned short key_len;        // key length
      
  void *o;                       // malloc'ed object
  unsigned long o_len;           // object length
} st_filter_object_t;


/*
  st_filter_chain_t
  The filter chain struct that will be init'ed and returned by filter_init_filters*()
*/
typedef struct st_filter_chain_t
{
  // ALL filters
  st_filter_t *all[FILTER_MAX]; // ALL unsorted filters (init and quit use this)

  // SET filters
  int set[FILTER_MAX];          // currently used filters (ignored by init and quit)
                                //   set by filter_set_chain() (default: ALL filters (but unsorted))
                                //   id's of filters in all[] in the order set by filter_set_chain()
  time_t start_time[FILTER_MAX]; // start times of all set filters (SET filters only)
  int result[FILTER_MAX]; // results (SET filters only) (-1 == failed, 0 == OK, >0 == skipped)
  st_filter_object_t o[FILTER_MAX_OBJ]; // objects for ALL filters

  int pos;               // # of current filter (SET filters only)
  int total;             // # of all set filters (SET filters only)
  int inited;             // filter were init()'ed (ALL filters)
  int op;                 // (private) current filter operation FILTER_OPEN, FILTER_CLOSE, ...
  char verbose;           // print verbose messages
  char debug;             // print debug messages
} st_filter_chain_t;


/*
  filter_malloc_chain()  turn filter array into a filter chain (use only once per process)
  filter_set_chain()     (de-)activate/(re-)sort needed filters in chain by an array of their id's
                           (use only once per file/data/stream)
                           filter_set_chain(..., NULL) will activate ALL filters (but unsorted!)

                           NOTE: a nice aspect of filter_set_chain() is that it can be
                           called from inside a filter of the actual filter chain and therefore
                           change the rest of the filter chain while running :-)
                           A demuxer (for example) could become itself(!) the first filter in a
                           chain, demux and (de-)activate/(re-)sort the rest of the chain
                           according to the demuxed type of input
                           Or a path (or url) could be parsed by the first filter the second
                           filter opens the descriptor from the file or the url the third filter
                           caches (if from url) the fourth filter demuxes and (de-)activates/(re-)sorts
                           all following filters according to the input type... etc...
  filter_get_chain()     returns the current filter chain as int array with filter id's
  filter_st_filter_chain_t_sanity_check()
                         try this for some transparency and to solve problems
  filter_free_chain()    free filter chain (use only once per process)

  filter_init()    wrapper for st_filter_t->init (use only once per process)
                     IMPORTANT: it does NOT(!) make a difference if you call this before or after
                     filter_set_chain(); always ALL filters with an init() will be init()'ed 
                     (or quit()'ed) no matter what filter_set_chain() did
  filter_quit()    wrapper for st_filter_t->quit (use only once per process)

  filter_open()    wrapper for st_filter_t->open (use only once per file/data/stream)
  filter_close()   wrapper for st_filter_t->close (use only once per file/data/stream)

  filter_read()    wrapper for st_filter_t->read (use only once per part of file/data/stream)
  filter_write()   wrapper for st_filter_t->write (use only once per part of file/data/stream)

  filter_seek()    wrapper for st_filter_t->seek (use only once per part of file/data/stream)
  filter_ctrl()    wrapper for st_filter_t->ctrl (supposed to be used as often as necessary)

  NOTE: all wrappers will pass that (void *) argument to st_filter_t->(* func)()


  filter_object_*() functions

  are meant to be called from inside a filter
  they are useful to avoid any (static) global variables in filters that might appear
  more than just once in a filter chain

  filter_object_write() writes an object into current-filter-exclusive cache identified by key
  filter_object_read()  writes an object into current-filter-exclusive cache identified by key
  filter_object_read_by_id()  like filter_object_read() but uses integer as key
  filter_object_write_by_id() like filter_object_write() but uses integer as key

  NOTE: objects with always be overwritten by new objects that have the same id
  NOTE2: objects that are written during init/quit will be stored without the
         information of their position in the filter chain (because it SHOULDN'T make sence
         to have duplicates of objects that were created during init because init (and quit)
         SHOULD be called only once per process)
*/
extern st_filter_chain_t *filter_malloc_chain (const st_filter_t **);
extern int filter_set_chain (st_filter_chain_t *fc, const int *filter_id);
//extern const int *filter_get_chain (st_filter_chain_t *fc);
extern void filter_st_filter_chain_t_sanity_check (st_filter_chain_t *fc);
extern void filter_free_chain (st_filter_chain_t *);


extern int filter_init (st_filter_chain_t *fc, void *o);
extern int filter_quit (st_filter_chain_t *fc, void *o);

extern int filter_open (st_filter_chain_t *fc, void *o);
extern int filter_close (st_filter_chain_t *fc, void *o);

extern int filter_read (st_filter_chain_t *fc, void *o);
extern int filter_write (st_filter_chain_t *fc, void *o);
extern int filter_seek (st_filter_chain_t *fc, void *o);
extern int filter_ctrl (st_filter_chain_t *fc, void *o);


extern int filter_object_read (st_filter_chain_t *fc,
                               void *key,
                               unsigned short key_len,
                               void *o,
                               unsigned long o_len);
extern int filter_object_write (st_filter_chain_t *fc,
                                void *key,
                                unsigned short key_len,
                                void *o,
                                unsigned long o_len);

extern int filter_object_read_by_id (st_filter_chain_t *fc,
                                     int o_id,
                                     void *o,
                                     unsigned long o_len);
extern int filter_object_write_by_id (st_filter_chain_t *fc,
                                      int o_id,
                                      void *o,
                                      unsigned long o_len);


/*
  filter_get_total()           get # of all set filters (SET filters only)
  filter_get_pos()             get # of current filter (SET filters only)
  filter_get_pos_by_id()       get # of current filter (filters with id only)
  filter_get_id()              get id of filter in pos (SET filters only)
  filter_get_id_s()            get id_s of filter in pos (SET filters only)
  filter_get_result()          get last result (SET filters only) (-1 == failed, 0 == OK, >0 == skipped)
  filter_get_start_time()      get start time of the 1st filter (SET filters only)

  filter_get_filter_by_id()    get st_filter_t by id (ALL filters)
  filter_get_filter_by_pos()   get st_filter_t by pos (ALL filters)
  filter_get_filter_by_magic() get st_filter_t by magic (ALL filters)

  filter_get_all_id_s_in_array()
                               get comma separated id_s as a string (from st_filter_t array)
  filter_get_all_id_s_in_chain()
                               get comma separated id_s as a string (ALL filters)
*/
extern int filter_get_total (const st_filter_chain_t *fc);
extern int filter_get_pos (const st_filter_chain_t *fc);
extern int filter_get_pos_by_id (const st_filter_chain_t *fc, int id);
extern int filter_get_id (const st_filter_chain_t *fc, int pos);
extern const char * filter_get_id_s (const st_filter_chain_t *fc, int pos);
//extern int filter_get_op (const st_filter_chain_t *fc);
extern int filter_get_result (const st_filter_chain_t *fc, int pos);
//extern time_t filter_get_start_time (const st_filter_chain_t *fc);

extern const st_filter_t *filter_get_filter_by_id (const st_filter_chain_t *fc, int id);
extern const st_filter_t *filter_get_filter_by_pos (const st_filter_chain_t *fc, int pos);
extern const st_filter_t *filter_get_filter_by_magic (const st_filter_chain_t *fc,
                                                      const unsigned char *magic, int magic_len);

extern const char * filter_get_all_id_s_in_array (const st_filter_t **f);
//extern const char * filter_get_all_id_s_in_chain (const st_filter_chain_t *fc);


#endif // MISC_FILTER_H
