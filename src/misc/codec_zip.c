/*
codec_zip.c - g(un)zip and unzip support

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
#include "config.h"                             // USE_ZLIB
#endif
#ifdef  USE_ZLIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <zlib.h>
#ifdef  HAVE_BYTESWAP_H
#include <byteswap.h>
#else
#include "bswap.h"
#endif
#include "codec.h"
#include "codec_zip.h"
#include "misc.h"
#include "map.h"
#include "unzip.h"
#if     defined DJGPP && defined DLL
#include "dxedll_priv.h"
#endif


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768


//extern int errno;


/*
  The zlib functions gzwrite() and gzputc() write compressed data if the file
  was opened with a "w" in the mode string, but we want to control whether we
  write compressed data. Note that for the mode string "w+", zlib ignores the
  '+'. zlib does the same for "r+".
  Currently we never write compressed data. That is, no code fopen()s files
  with a mode string containing 'f', 'h' or a number, but writing compressed
  output does work.
*/
st_map_t *fh_map = NULL;                        // associative array: file handle -> file mode

#if 0
typedef enum { FM_NORMAL, FM_GZIP, FM_ZIP, FM_UNDEF } fmode2_t;

typedef struct st_finfo
{
  fmode2_t fmode;
  int compressed;
} st_finfo_t;

static st_finfo_t finfo_list[6] = { {FM_NORMAL, 0},
                                    {FM_NORMAL, 1},     // should never be used
                                    {FM_GZIP, 0},
                                    {FM_GZIP, 1},
                                    {FM_ZIP, 0},        // should never be used
                                    {FM_ZIP, 1} };
#endif
int unzip_current_file_nr = 0;


#if 0
static void
init_fh_map (void)
{
  fh_map = map_create (20);                     // 20 simultaneous open files
  map_put (fh_map, stdin, &finfo_list[0]);      //  should be enough to start with
  map_put (fh_map, stdout, &finfo_list[0]);
  map_put (fh_map, stderr, &finfo_list[0]);
}


static st_finfo_t *
get_finfo (FILE *file)
{
  st_finfo_t *finfo;

  if (fh_map == NULL)
    init_fh_map ();
  if ((finfo = (st_finfo_t *) map_get (fh_map, file)) == NULL)
    {
      fprintf (stderr, "\nINTERNAL ERROR: File pointer was not present in map (%p)\n", file);
      map_dump (fh_map);
      exit (1);
    }
  return finfo;
}


static fmode2_t
get_fmode (FILE *file)
{
  return get_finfo (file)->fmode;
}
#endif


int
unzip_get_number_entries (const char *filename)
{
  FILE *file;
  unsigned char magic[4] = { 0 };

  if ((file = fopen (filename, "rb")) == NULL)
    {
      errno = ENOENT;
      return -1;
    }
  fread (magic, 1, sizeof (magic), file);
  fclose (file);

  if (magic[0] == 'P' && magic[1] == 'K' && magic[2] == 0x03 && magic[3] == 0x04)
    {
      unz_global_info info;

      file = (FILE *) unzOpen (filename);
      unzGetGlobalInfo (file, &info);
      unzClose (file);
      return info.number_entry;
    }
  else
    return -1;
}


int
unzip_goto_file (unzFile file, int file_index)
{
  int retval = unzGoToFirstFile (file), n = 0;

  if (file_index > 0)
    while (n < file_index)
      {
        retval = unzGoToNextFile (file);
        n++;
      }
  return retval;
}


#if 0
static int
unzip_seek_helper (FILE *file, int offset)
{
  char buffer[MAXBUFSIZE];
  int n, tmp, pos = unztell (file);             // returns ftell() of the "current file"

  if (pos == offset)
    return 0;
  else if (pos > offset)
    {
      unzCloseCurrentFile (file);
      unzip_goto_file (file, unzip_current_file_nr);
      unzOpenCurrentFile (file);
      pos = 0;
    }
  n = offset - pos;
  while (n > 0 && !unzeof (file))
    {
      tmp = unzReadCurrentFile (file, buffer, n > MAXBUFSIZE ? MAXBUFSIZE : n);
      if (tmp < 0)
        return -1;
      n -= tmp;
    }
  return n > 0 ? -1 : 0;
}
#endif


#define CHUNK 16384

int
unzip_inflate (FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


#endif // USE_ZLIB
