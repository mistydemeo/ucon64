/*
chksum.h - miscellaneous checksum functions

Copyright (c) 1999 - 2004 NoisyB <noisyb@gmx.net>
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


MD5  - Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.
       License to copy and use this software is granted provided that
       it is identified as the "RSA Data Security, Inc. MD5 Message
       Digest Algorithm" in all material mentioning or referencing this
       software or this function.
*/
#ifndef MISC_CHKSUM_H
#define MISC_CHKSUM_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#ifdef  USE_ZLIB
#include "archive.h"
#endif                                          // USE_ZLIB

#ifdef __sun
#ifdef __SVR4
#define __solaris__
#endif
#endif

#ifdef  HAVE_INTTYPES_H
#include <inttypes.h>
#else                                           // __MSDOS__, _WIN32 (VC++)
#ifndef OWN_INTTYPES
#define OWN_INTTYPES                            // signal that these are defined
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
#ifndef _WIN32
typedef unsigned long long int uint64_t;
#else
typedef unsigned __int64 uint64_t;
#endif
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
#ifndef _WIN32
typedef signed long long int int64_t;
#else
typedef signed __int64 int64_t;
#endif
#endif                                          // OWN_INTTYPES
#endif

#if     (!defined TRUE || !defined FALSE)
#define FALSE 0
#define TRUE (!FALSE)
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#define LIB_VERSION(ver, rel, seq) (((ver) << 16) | ((rel) << 8) | (seq))
#define NULL_TO_EMPTY(str) ((str) ? (str) : (""))
//#define RANDOM(min, max) ((rand () % (max - min)) + min)
#define OFFSET(a, offset) ((((unsigned char *)&(a))+(offset))[0])

#ifdef  WORDS_BIGENDIAN
#undef WORDS_BIGENDIAN
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


#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE


typedef struct
{
  int crc32;
  char sha1[30];
  char md5[30];
} st_q_fchk_t;


/*
  crc16()      calculate the crc16 of buffer for size bytes
  crc32()      calculate the crc32 of buffer for size bytes
*/
extern unsigned short crc16 (unsigned short crc, const void *buffer, unsigned int size);
#ifndef  USE_ZLIB
// use zlib's crc32() if USE_ZLIB is defined...
#define crc32(C, B, S) crc32_2(C, B, S)
// ... but make it possible to link against a library that uses zlib while this
//  code does not use it
extern unsigned int crc32_2 (unsigned int crc, const void *buffer, unsigned int size);
#endif


/*
  q_fcrc32()   calculate the crc32 of filename from start
  q_fsha1()    calculate the sha1 of filename from start and store in buf as string
  q_fmd5()     calculate the md5 of filename from start and store in buf as string

  q_chk()      calculate everything at once
*/
extern int q_fcrc32 (const char *filename, int start);
extern int q_fsha1 (char *buf, const char *filename, int start);
extern int q_fmd5 (char *buf, const char *filename, int start);

extern int q_fchk (st_q_fchk_t *chksums, const char *filename, int start);

#ifdef  __cplusplus
}
#endif

#endif // MISC_CHKSUM_H
