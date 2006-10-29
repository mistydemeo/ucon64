/*
codec.h - miscellaneous codec (and compression) functions

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
#ifndef MISC_CODEC_H
#define MISC_CODEC_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  __cplusplus
extern "C" {
#endif
#ifdef  USE_BASE32
#include "codec_base32.h"
#endif
#ifdef  USE_BASE64
#include "codec_base64.h"
#endif
#ifdef  USE_ZLIB
#include "codec_zip.h"
#endif
#ifdef  USE_RAR
#include "codec_rar.h"
#endif


#ifdef  USE_BASE32
#define CODEC_BASE32 1
#endif
#ifdef  USE_BASE64
#define CODEC_BASE64 2
#endif
#ifdef  USE_7BIT
#define CODEC_7BIT   3
#endif
#ifdef  USE_ZIP
#define CODEC_ZIP    4
#endif
#ifdef  USE_RAR
#define CODEC_RAR    5
#endif


#if 0
typedef struct
{
  int flag;
} st_codec_t;
#endif


/*
  Flags
    CODEC_BASE32
    CODEC_BASE64
    CODEC_7BIT
    CODEC_ZIP
    CODEC_RAR

  codec_encode_file()  encode a src file or directory to a dst file
  codec_decode_file()  decode a src file to a dst file or directory
  
  codec_encode_mem()   encode src to dst mem
  codec_decode_mem()   decode src to dst mem
*/
extern int codec_encode_file (const char *src, const char *dst, int flag);
extern int codec_decode_file (const char *src, const char *dst, int flag);
#define codec_encode_dir codec_encode_file
#define codec_decode_dir codec_decode_file


extern int codec_encode_mem (const unsigned char *src, unsigned int src_len,
                             const unsigned char *dst, unsigned int dst_len, int flag);
extern int codec_decode_mem (const unsigned char *src, unsigned int src_len, 
                             const unsigned char *dst, unsigned int dst_len, int flag);


#ifdef  __cplusplus
}
#endif
#endif // MISC_CODEC_H
