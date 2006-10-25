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
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef  USE_ZLIB
#include <zlib.h>
#include "unzip.h"
#endif
#include "hash_crc.h"
#include "defines.h"



#define CRC16_POLYNOMIAL 0xa001
#define CRC32_POLYNOMIAL 0xedb88320


void
init_crc_table (void *table, unsigned int polynomial)
// works for crc16 and crc32
{
  unsigned int crc, i, j;

  for (i = 0; i < 256; i++)
    {
      crc = i;
      for (j = 8; j > 0; j--)
        if (crc & 1)
          crc = (crc >> 1) ^ polynomial;
        else
          crc >>= 1;

      if (polynomial == CRC32_POLYNOMIAL)
        ((unsigned int *) table)[i] = crc;
      else
        ((unsigned short *) table)[i] = (unsigned short) crc;
    }
}


static unsigned short crc16_table[256] = {0};


unsigned short
crc16 (unsigned short crc, const void *buffer, unsigned int size)
{
  unsigned char *p = (unsigned char *) buffer;

  if (!(*crc16_table))
    {
      init_crc_table (&crc16_table, CRC16_POLYNOMIAL);
    }

  crc = ~crc;
  while (size--)
    crc = (crc >> 8) ^ crc16_table[(crc ^ *p++) & 0xff];
  return ~crc;
}


// CRC32
#ifndef USE_ZLIB


static unsigned int crc32_table[256] = {0};


unsigned int
crc32 (unsigned int crc, const void *buffer, unsigned int size)
{
  unsigned char *p = (unsigned char *) buffer;

  if (!(*crc32_table))
    {
      init_crc_table (&crc32_table, CRC32_POLYNOMIAL);
    }

  crc = ~crc;
  while (size--)
    crc = (crc >> 8) ^ crc32_table[(crc ^ *p++) & 0xff];
  return ~crc;
}
#endif
