/*
codec.c - miscellaneous codec (and compression) functions

Copyright (c) 2006 NoisyB


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
#include "config.h"
#endif
#include "codec.h"


int
codec_encode_file (const char *src, const char *dst, int flag)
{
  (void) src;
  (void) dst;
  (void) flag;
  return 0;
}


int
codec_decode_file (const char *src, const char *dst, int flag)
{
  (void) src;
  (void) dst;
  (void) flag;
  return 0;
}


int codec_encode_mem (const unsigned char *src, unsigned int src_len,
                       const unsigned char *dst, unsigned int dst_len, int flag)
{
  (void) src;
  (void) src_len;
  (void) dst;
  (void) dst_len;
  (void) flag;
  return 0;
}


int
codec_decode_mem (const unsigned char *src, unsigned int src_len, 
                  const unsigned char *dst, unsigned int dst_len, int flag)
{
  (void) src;
  (void) src_len;
  (void) dst;
  (void) dst_len;
  (void) flag;
  return 0;
}


#ifdef  TEST
int
main (int argc, char **argv)
{
  return 0;
}
#endif
