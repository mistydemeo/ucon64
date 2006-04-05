/*
filter.h - Simple filter framework for any file, stream or data
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
  int magic_len;          // optional (if(magic_len == -1) strlen(magic) will be used)
//  unsigned long flags;    // examples: MODE, READ, DEC, WRITE, ENC, IN, OUT, DEMUX
//  char fork;              // run filter in child process? (FALSE or TRUE)
//  void *object;           // optional

  // return values (results) should be:
  // -1 == failed (will stop everything)
  //  0 == OK
  // >0 == skipped (that single filter will be ignored)
  int (* demux) (void *);

  int (* open) (void *);
  int (* close) (void *);

  int (* read) (void *);  // also: encode
  int (* write) (void *); // also: decode

  int (* seek) (void *);
  int (* ctrl) (void *);

  int (* init) (void *);
  int (* quit) (void *);
} st_filter_t;


/*
  st_filter_chain_t
  The filter chain struct that will be init'ed and returned by filter_init_filters*()
*/
typedef struct st_filter_chain_t
{
  // ALL filters variables
  st_filter_t *all[FILTER_MAX]; // ALL unsorted filters (init and quit use this)
  int all_total;                // # of ALL filters
//  int all_pos;                  // # of current SET filter in ALL filter array
  int inited[FILTER_MAX];       // id of filters which were init()'ed

  // SET filters variables
  int set[FILTER_MAX];          // currently used filters (ignored by init and quit)
                                //   set by filter_set_chain() (default: ALL filters (but unsorted))
                                //   id's of filters in all[] in the order set by filter_set_chain()
  int total;                    // # of SET filters
  int pos;                      // # of current SET filter
  time_t start_time[FILTER_MAX]; // start time of first filter
  int result[FILTER_MAX];       // result from last filter (-1 == failed, 0 == OK, >0 == skipped)

  // other variables
  int op;                       // current filter operation FILTER_OPEN, FILTER_CLOSE, ...
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

  filter_demux()   wrapper for st_filter_t->demux (use always)

  filter_init()    wrapper for st_filter_t->init (use only once per process)
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
extern int filter_set_chain (st_filter_chain_t *fc, const int *filter_id);
//extern const int *filter_get_chain (st_filter_chain_t *fc);
extern void filter_st_filter_chain_t_sanity_check (st_filter_chain_t *fc);
extern void filter_free_chain (st_filter_chain_t *);


extern int filter_demux (st_filter_chain_t *fc, void *o);

extern int filter_init (st_filter_chain_t *fc, void *o, const int *filter_id);
extern int filter_quit (st_filter_chain_t *fc, void *o);

extern int filter_open (st_filter_chain_t *fc, void *o);
extern int filter_close (st_filter_chain_t *fc, void *o);

extern int filter_read (st_filter_chain_t *fc, void *o);
extern int filter_write (st_filter_chain_t *fc, void *o);
extern int filter_seek (st_filter_chain_t *fc, void *o);
extern int filter_ctrl (st_filter_chain_t *fc, void *o);


/*
  ALL filters functions

  filter_get_filter_total      get # of ALL filters
  filter_get_filter_by_id()    get st_filter_t by id
  filter_get_filter_by_pos()   get st_filter_t by pos
  filter_get_filter_by_magic() get st_filter_t by magic
  filter_get_all_id_s_in_array()
                               get comma separated id_s as a string
  filter_get_all_id_s_in_chain()
                               get comma separated id_s as a string


  SET filters functions

  filter_get_total()           get # of SET filters
  filter_get_pos()             get # of current filter
  filter_get_pos_by_id()       get # of current filter
  filter_get_id()              get id of filter in pos
  filter_get_id_s()            get id_s of filter in pos
  filter_get_result()          get last result (-1 == failed, 0 == OK, >0 == skipped)
  filter_get_start_time()      get start time of the 1st filter


  Other filter functions

  filter_get_key()             get (generate) a unique key for get/set read/write an object
                                 from inside a filter
                                 subkey is optional and used if more than one object is
                                 get/set read/write from inside a filter
                                 keys are made of the position, filter id and (optional) a
                                 subkey divided by ':'s
  filter_generate_key()        manually generate a key that will be identical to what
                               filter_get_key() would return
  NOTE: objects with always be overwritten by new objects that have the same id
  NOTE2: objects that are written during init/quit will be stored without the
         information of their position in the filter chain (because it SHOULDN'T make sence
         to have duplicates of objects that were created during init because init (and quit)
         SHOULD be called only once per process)
*/
extern int filter_get_filter_total (const st_filter_chain_t *fc);
extern const st_filter_t *filter_get_filter_by_id (const st_filter_chain_t *fc, int id);
extern const st_filter_t *filter_get_filter_by_pos (const st_filter_chain_t *fc, int pos);
extern const st_filter_t *filter_get_filter_by_magic (const st_filter_chain_t *fc,
                                                      const unsigned char *magic, int magic_len);
extern const char *filter_get_all_id_s_in_array (const st_filter_t **f);

//extern const char *filter_get_all_id_s_in_chain (const st_filter_chain_t *fc);


extern int filter_get_total (const st_filter_chain_t *fc);
extern int filter_get_pos (const st_filter_chain_t *fc);
extern int filter_get_id (const st_filter_chain_t *fc, int pos);
extern const char *filter_get_id_s (const st_filter_chain_t *fc, int pos);

//extern int filter_get_pos_by_id (const st_filter_chain_t *fc, int id);
//extern int filter_get_op (const st_filter_chain_t *fc);
//extern int filter_get_result (const st_filter_chain_t *fc, int pos);
//extern time_t filter_get_start_time (const st_filter_chain_t *fc);


extern char *filter_get_key (st_filter_chain_t *fc, int *subkey);
extern char *filter_generate_key (int *pos, int *id, int *subkey);


#endif // MISC_FILTER_H
