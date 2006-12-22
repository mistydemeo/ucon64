/*
hash_crc.c - miscellaneous hash (and checksum) functions

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
*/
#ifndef MISC_CRC_H
#define MISC_CRC_H
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif
#ifdef  __cplusplus
extern "C" {
#endif


/*
  crc16()  crc16

  crc32()  a crc32() clone (if no ZLIB is used)
             use zlib's crc32() if USE_ZLIB is defined...
             ... but make it possible to link against a library
             that uses zlib while this code does not use it

  crc32_wrap() wrapper for own or zlib's crc32()
*/
extern unsigned short crc16 (unsigned short crc, const void *buffer, unsigned int size);
#ifdef  USE_ZLIB
#include <zlib.h>    // crc32()
#else
extern unsigned int crc32 (unsigned int crc, const void *buffer, unsigned int size);
#endif
extern unsigned int crc32_wrap (unsigned int crc, const void *buffer, unsigned int size);


#ifdef  __cplusplus
}
#endif
#endif // MISC_CRC_H
