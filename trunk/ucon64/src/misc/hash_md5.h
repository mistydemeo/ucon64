/*
hash_md5.h - miscellaneous hash (and checksum) functions

Copyright (c) 1999 - 2004 NoisyB
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


MD5  - Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.
       License to copy and use this software is granted provided that
       it is identified as the "RSA Data Security, Inc. MD5 Message
       Digest Algorithm" in all material mentioning or referencing this
       software or this function.
*/
#ifndef MISC_HASH_MD5_H
#define MISC_HASH_MD5_H
#ifdef  __cplusplus
extern "C" {
#endif
#ifdef  HAVE_INTTYPES_H
#include <inttypes.h>
#else                                           // __MSDOS__, _WIN32 (VC++)
#include "itypes.h"
#endif


/*
  s_md5_ctx_t
  md5_init()   start md5
  md5_update() process data
  md5_final()  stop md5
*/
// data structure for MD5 (Message Digest) computation
typedef struct
{
  uint32_t i[2];                        // number of _bits_ handled mod 2^64
  uint32_t buf[4];                      // scratch buffer
  unsigned char in[64];                 // input buffer
  unsigned char digest[16];             // actual digest after md5_final call
} s_md5_ctx_t;

extern void md5_init (s_md5_ctx_t *mdContext, unsigned long pseudoRandomNumber);
extern void md5_update (s_md5_ctx_t *mdContext, unsigned char *inBuf, unsigned int inLen);
extern void md5_final (s_md5_ctx_t *mdContext);


#ifdef  __cplusplus
}
#endif
#endif // MISC_HASH_MD5_H
