/*
hash.c - miscellaneous hash (and checksum) functions

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
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "itypes.h"
#include "defines.h"
#include "misc.h"
#include "hash.h"
#ifdef  USE_CRC
#include "hash_crc.h"
#endif
#ifdef  USE_SHA1
#include "hash_sha1.h"
#endif
#ifdef  USE_MD5
#include "hash_md5.h"
#endif
#ifdef  USE_MD4
#include "hash_md4.h"
#endif


st_hash_t *
hash_open (int flags)
{
  st_hash_t *h = malloc (sizeof (st_hash_t));
  int inited = 0;

  if (!h)
    return NULL;

  memset (h, 0, sizeof (st_hash_t));

  h->flags = flags;

#ifdef  USE_MD4
  if (h->flags & HASH_MD4)
    {
      MD4Init (&h->m_md4);
      inited = 1;
    }
#endif

#ifdef  USE_MD5
  if (h->flags & HASH_MD5)
    {
      md5_init (&h->m_md5, 0);
      inited = 1;
    }
#endif

#ifdef  USE_SHA1
  if (h->flags & HASH_SHA1)
    {
      sha1_begin (&h->m_sha1);
      inited = 1;
    }
#endif

#ifdef  USE_CRC
  if (h->flags & HASH_CRC32)
    {
      h->crc32 = 0;
      inited = 1;
    }

  if (h->flags & HASH_CRC16)
    {
      h->crc16 = 0;
      inited = 1;
    }
#endif

  if (inited)
    return h;

  free (h);
  h = NULL;

  return NULL;
}


int
hash_close (st_hash_t *h)
{
  free (h);
  h = NULL;

  return 0;
}


st_hash_t *
hash_update (st_hash_t *h, const unsigned char *buffer, unsigned int buffer_len)
{
#ifdef  USE_MD4
  if (h->flags & HASH_MD4)
    MD4Update (&h->m_md4, (unsigned char *) buffer, buffer_len);
#endif

#ifdef  USE_MD5
  if (h->flags & HASH_MD5)
    md5_update (&h->m_md5, (unsigned char *) buffer, buffer_len);
#endif

#ifdef  USE_SHA1
  if (h->flags & HASH_SHA1)
    sha1 (&h->m_sha1, buffer, buffer_len);
#endif

#ifdef  USE_CRC
  if (h->flags & HASH_CRC32)
    h->crc32 = crc32 (h->crc32, buffer, buffer_len);

  if (h->flags & HASH_CRC16)
    h->crc16 = crc16 (h->crc16, buffer, buffer_len);
#endif

  return h;
}


#ifdef  USE_CRC
uint16_t
hash_get_crc16 (st_hash_t *h)
{
  return h->crc16;
}


uint32_t
hash_get_crc32 (st_hash_t *h)
{
  return h->crc32;
}
#endif


const char *
hash_get_s (st_hash_t *h, int flag)
{
  int i = 0;
#if      (defined USE_MD4 || defined USE_SHA1)
  unsigned char tmp[32];
#endif

  if (!(h->flags & flag))
    return NULL;

  switch (flag)
    {
#ifdef  USE_MD4
      case HASH_MD4:
        MD4Final (tmp, &h->m_md4);
        *h->buf = 0;
        for (i = 0; i < 16; i++)
          sprintf (strchr (h->buf, 0), "%02x", tmp[i]);
        return h->buf;
#endif

#ifdef  USE_MD5
      case HASH_MD5:
        md5_final (&h->m_md5);
        *h->buf = 0;
        for (i = 0; i < 16; i++)
          sprintf (strchr (h->buf, 0), "%02x", h->m_md5.digest[i]);
        return h->buf;
#endif

#ifdef  USE_SHA1
      case HASH_SHA1:
        sha1_end (tmp, &h->m_sha1);
        *h->buf = 0;
        for (i = 0; i < 20; i++)
          sprintf (strchr (h->buf, 0), "%02x", tmp[i] & 0xff);
        return h->buf;
#endif

#ifdef  USE_CRC
      case HASH_CRC32:
        sprintf (h->buf, "%08x", h->crc32);
        return h->buf;

      case HASH_CRC16:
        sprintf (h->buf, "%04x", h->crc16);
        return h->buf;
#endif 
    }

  return NULL;
}


#ifdef  TEST
int
main (int argc, char **argv)
{
  st_hash_t *h = hash_init (HASH_SHA1|HASH_MD5|HASH_CRC32|HASH_CRC16|HASH_MD4);
//  st_hash_t *h = hash_init (HASH_MD4);

  h = hash_update (h, (const unsigned char *) "test", 4);
 
#if 1
  printf ("crc16: %x\n", hash_get_crc16 (h));
  printf ("crc32: %x\n", hash_get_crc32 (h));
  printf ("sha1: %s\n", hash_get_s (h, HASH_SHA1));
  printf ("md5: %s\n", hash_get_s (h, HASH_MD5));
  printf ("md4: %s\n", hash_get_s (h, HASH_MD4));
#endif

  h = hash_init (HASH_SHA1|HASH_MD5|HASH_CRC32|HASH_CRC16|HASH_MD4);

  h = hash_update (h, (const unsigned char *) "test2", 5);

#if 1
  printf ("crc16: %x\n", hash_get_crc16 (h));
  printf ("crc32: %x\n", hash_get_crc32 (h));
  printf ("sha1: %s\n", hash_get_s (h, HASH_SHA1));
  printf ("md5: %s\n", hash_get_s (h, HASH_MD5));
  printf ("md4: %s\n", hash_get_s (h, HASH_MD4));
#endif

  h = hash_init (HASH_MD4);
  h = hash_update (h, "", 0);
  printf ("md4: %s\n", hash_get_s (h, HASH_MD4));

  return 0;
}
#endif
