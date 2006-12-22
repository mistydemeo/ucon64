/*
hash.h - miscellaneous hash (and checksum) functions

Copyright (c) 1999 - 2006 NoisyB
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
#ifndef MISC_HASH_H
#define MISC_HASH_H
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif
#ifdef  __cplusplus
extern "C" {
#endif
#ifdef  HAVE_INTTYPES_H
#include <inttypes.h>
#else                                           // __MSDOS__, _WIN32 (VC++)
#include "itypes.h"
#endif
#ifdef  USE_CRC
#include "hash_crc.h"
#endif
#ifdef  USE_MD4
#include "hash_md4.h"                           // MD4_CTX
#endif
#ifdef  USE_MD5
#include "hash_md5.h"                           // s_md5_ctx_t
#endif
#ifdef  USE_SHA1
#include "hash_sha1.h"                          // s_sha1_ctx_t
#endif


#ifdef  USE_MD5
#define HASH_MD5   (1<<0)
#endif
#ifdef  USE_SHA1
#define HASH_SHA1  (1<<1)
#endif
#ifdef  USE_CRC
#define HASH_CRC32 (1<<2)
#define HASH_CRC16 (1<<3)
#endif
#ifdef  USE_MD4
#define HASH_MD4   (1<<4)
#endif


typedef struct
{
  int flags;

  char buf[128];

#ifdef  USE_CRC
  uint16_t crc16;
  uint32_t crc32;
#endif

  // md5
#ifdef  USE_MD5
  s_md5_ctx_t m_md5;
#endif

  // sha1
#ifdef  USE_SHA1
  s_sha1_ctx_t m_sha1;
#endif

  // md4
#ifdef  USE_MD4
  MD4_CTX m_md4;
#endif
} st_hash_t;


/*
  hash_open()    open st_hash_t context
  hash_close()   close st_hash_t context

  Flags
    HASH_MD4     calc (also) MD4 hash
    HASH_MD5     calc (also) MD5 hash
    HASH_SHA1    calc (also) SHA1 hash
    HASH_CRC16   calc (also) CRC16 crc
    HASH_CRC32   calc (also) CRC32 crc

  hash_update()  send buffer through hash algorithm

  hash_get_s()   return current hash code
                   flag can be f.e. HASH_MD5 _OR_ HASH_CRC32 _OR_ HASH_CRC16 _OR_ ...

  hash_get_crc16() return current crc16 as integer
  hash_get_crc32() return current crc32 as integer
*/
extern st_hash_t *hash_open (int flags);
extern int hash_close (st_hash_t *h);

extern st_hash_t *hash_update (st_hash_t *h, const unsigned char *buffer, unsigned int buffer_len);
extern const char *hash_get_s (st_hash_t *h, int flag);

#ifdef  USE_CRC
extern uint16_t hash_get_crc16 (st_hash_t *h);
extern uint32_t hash_get_crc32 (st_hash_t *h);
#endif


#ifdef  __cplusplus
}
#endif
#endif // MISC_HASH_H
