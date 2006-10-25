/*
hash_sha1.h - miscellaneous hash (and checksum) functions

Copyright (c) 1999 - 2004 NoisyB
Copyright (c) 2001 - 2004 dbjh

sha1 - Copyright (c) 2002, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.


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
#ifndef MISC_HASH_SHA1_H
#define MISC_HASH_SHA1_H
#ifdef  __cplusplus
extern "C" {
#endif
#ifdef  HAVE_INTTYPES_H
#include <inttypes.h>
#else                                           // __MSDOS__, _WIN32 (VC++)
#include "itypes.h"
#endif


/*
  s_sha1_ctx_t
  sha1_begin() start sha1
  sha1()       process data
  sha1_end()   stop sha1
*/
typedef struct
{
  uint32_t count[2];
  uint32_t hash[5];
  uint32_t wbuf[16];
} s_sha1_ctx_t;

extern void sha1_begin (s_sha1_ctx_t ctx[1]);
extern void sha1 (s_sha1_ctx_t ctx[1], const unsigned char data[], unsigned int len);
extern void sha1_end (unsigned char hval[], s_sha1_ctx_t ctx[1]);


#ifdef  __cplusplus
}
#endif
#endif // MISC_HASH_SHA1_H
