/*
hash_sha1.c - miscellaneous hash (and checksum) functions

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
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "bswap.h"
#include "hash_sha1.h"
#include "defines.h"


#ifdef  WORDS_BIGENDIAN
#undef  WORDS_BIGENDIAN
#endif


#if     defined _LIBC || defined __GLIBC__
  #include <endian.h>
  #if __BYTE_ORDER == __BIG_ENDIAN
    #define WORDS_BIGENDIAN 1
  #endif
#elif   defined AMIGA || defined __sparc__ || defined __BIG_ENDIAN__ || \
        defined __APPLE__
  #define WORDS_BIGENDIAN 1
#endif


#define ROTL32(x,n) (((x) << (n)) | ((x) >> (32 - (n))))

#define SHA1_BLOCK_SIZE  64
#define SHA1_DIGEST_SIZE 20
#define SHA2_GOOD         0
#define SHA2_BAD          1
#define SHA1_MASK (SHA1_BLOCK_SIZE - 1)


void
sha1_compile (s_sha1_ctx_t ctx[1])
{
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define PARITY(x,y,z) ((x) ^ (y) ^ (z))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

  uint32_t w[80], i, a, b, c, d, e, t;

  /*
    note that words are compiled from the buffer into 32-bit
    words in big-endian order so an order reversal is needed
    here on little endian machines
  */
  for (i = 0; i < SHA1_BLOCK_SIZE / 4; ++i)
    w[i] = me2be_32 (ctx->wbuf[i]);

  for (i = SHA1_BLOCK_SIZE / 4; i < 80; ++i)
    w[i] = ROTL32 (w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

  a = ctx->hash[0];
  b = ctx->hash[1];
  c = ctx->hash[2];
  d = ctx->hash[3];
  e = ctx->hash[4];

  for (i = 0; i < 80; i++)
    {
      t = a;
      a = ROTL32 (a, 5) + e + w[i];
      if (i < 20)
        a += CH (b, c, d) + 0x5a827999;
      else if (i < 40)
        a += PARITY (b, c, d) + 0x6ed9eba1;
      else if (i < 60)
        a += MAJ (b, c, d) + 0x8f1bbcdc;
      else if (i < 80)
        a += PARITY (b, c, d) + 0xca62c1d6;
      e = d;
      d = c;
      c = ROTL32 (b, 30);
      b = t;
    }

  ctx->hash[0] += a;
  ctx->hash[1] += b;
  ctx->hash[2] += c;
  ctx->hash[3] += d;
  ctx->hash[4] += e;
}


void
sha1_begin (s_sha1_ctx_t ctx[1])
{
  ctx->count[0] = ctx->count[1] = 0;
  ctx->hash[0] = 0x67452301;
  ctx->hash[1] = 0xefcdab89;
  ctx->hash[2] = 0x98badcfe;
  ctx->hash[3] = 0x10325476;
  ctx->hash[4] = 0xc3d2e1f0;
}


void
sha1 (s_sha1_ctx_t ctx[1], const unsigned char data[], unsigned int len)
{
  uint32_t pos = (uint32_t) (ctx->count[0] & SHA1_MASK),
           space = SHA1_BLOCK_SIZE - pos;
  const unsigned char *sp = data;

  if ((ctx->count[0] += len) < len)
    ++(ctx->count[1]);

  while (len >= space)                  // transfer whole blocks while possible
    {
      memcpy (((unsigned char *) ctx->wbuf) + pos, sp, space);
      sp += space;
      len -= space;
      space = SHA1_BLOCK_SIZE;
      pos = 0;
      sha1_compile (ctx);
    }

  memcpy (((unsigned char *) ctx->wbuf) + pos, sp, len);
}


void
sha1_end (unsigned char hval[], s_sha1_ctx_t ctx[1])
{
#ifdef  WORDS_BIGENDIAN
  const uint32_t mask[4] = { 0x00000000, 0xff000000, 0xffff0000, 0xffffff00 };
  const uint32_t bits[4] = { 0x80000000, 0x00800000, 0x00008000, 0x00000080 };
#else
  const uint32_t mask[4] = { 0x00000000, 0x000000ff, 0x0000ffff, 0x00ffffff };
  const uint32_t bits[4] = { 0x00000080, 0x00008000, 0x00800000, 0x80000000 };
#endif
  uint32_t i = (uint32_t) (ctx->count[0] & SHA1_MASK);

  /*
    mask out the rest of any partial 32-bit word and then set
    the next byte to 0x80. On big-endian machines any bytes in
    the buffer will be at the top end of 32 bit words, on little
    endian machines they will be at the bottom. Hence the AND
    and OR masks above are reversed for little endian systems
    Note that we can always add the first padding byte at this
    because the buffer always contains at least one empty slot
  */
  ctx->wbuf[i >> 2] = (ctx->wbuf[i >> 2] & mask[i & 3]) | bits[i & 3];

  /*
    we need 9 or more empty positions, one for the padding byte
    (above) and eight for the length count.  If there is not
    enough space pad and empty the buffer
  */
  if (i > SHA1_BLOCK_SIZE - 9)
    {
      if (i < 60)
        ctx->wbuf[15] = 0;
      sha1_compile (ctx);
      i = 0;
    }
  else                                  // compute a word index for the empty buffer positions
    i = (i >> 2) + 1;

  while (i < 14)                        // and zero pad all but last two positions
    ctx->wbuf[i++] = 0;

  // assemble the eight byte counter in big-endian format
  ctx->wbuf[14] = me2be_32 ((ctx->count[1] << 3) | (ctx->count[0] >> 29));
  ctx->wbuf[15] = me2be_32 (ctx->count[0] << 3);

  sha1_compile (ctx);

  // extract the hash value as bytes in case the hash buffer is
  // misaligned for 32-bit words
  for (i = 0; i < SHA1_DIGEST_SIZE; ++i)
    hval[i] = (unsigned char) (ctx->hash[i >> 2] >> 8 * (~i & 3));
}
