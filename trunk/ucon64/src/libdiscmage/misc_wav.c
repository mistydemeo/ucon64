/*
misc_wav.c

Copyright (c) 2004 NoisyB


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <stdio.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include <stdlib.h>
#include <math.h>
#include "misc.h"
#include "misc_wav.h"
#ifndef M_PI
#define M_PI 3.1415926535
#endif
// TODO: replace ceil(), floor() and sin()


#if 0
unsigned char wav_header[3][80] = {
  {
    'R', 'I', 'F', 'F', 0x80, 0x80, 0x80, 0x80,    // RIFF TAG
    'W', 'A', 'V', 'E',                            // WAV TAG
    'f', 'm', 't', ' ', 0x10, 0, 0, 0,             // FMT TAG
    1, 0,                                          // format (WAVE_FORMAT_PCM)
    1, 0,                                          // CHANNELS
    0x22, 0x56, 0, 0,                              // SamplesPerSec
    0x22, 0x56, 0, 0,                              // BytesPerSec
    1, 0,                                          // Block align
    8, 0,                                          // Bits per sample
    'd', 'a', 't', 'a', 0, 0, 0, 0, '\0'           // DATA TAG
  },
  {
    'R', 'I', 'F', 'F', 0x80, 0x80, 0x80, 0x80,    // RIFF TAG
    'W', 'A', 'V', 'E',                            // WAV TAG
    'f', 'm', 't', ' ', 0x10, 0, 0, 0,             // FMT TAG
    1, 0,                                          // format (WAVE_FORMAT_PCM)
    1, 0,                                          // CHANNELS
    0x44, 0xac, 0, 0,                              // SamplesPerSec
    0x44, 0xac, 0, 0,                              // BytesPerSec
    1, 0,                                          // Block align
    8, 0,                                          // Bits per sample
    'd', 'a', 't', 'a', 0, 0, 0, 0, '\0'           // DATA TAG
  },
  {
    'R', 'I', 'F', 'F', 0x80, 0x80, 0x80, 0x80,    // RIFF TAG
    'W', 'A', 'V', 'E',                            // WAV TAG
    'f', 'm', 't', ' ', 0x10, 0, 0, 0,             // FMT TAG
    1, 0,                                          // format (WAVE_FORMAT_PCM)
    2, 0,                                          // CHANNELS
    0x44, 0xac, 0, 0,                              // SamplesPerSec
    0x10, 0xb1, 2, 0,                              // BytesPerSec
    4, 0,                                          // Block align
    0x10, 0,                                       // Bits per sample
    'd', 'a', 't', 'a', 0, 0, 0, 0, '\0'           // DATA TAG
  }
};
#endif


typedef struct
{
  uint8_t magic[4];       // 'RIFF'
  uint32_t total_length;  // length of file minus the 8 byte riff header

  uint8_t type[4];        // 'WAVE'

  uint8_t fmt[4];         // 'fmt '
  uint32_t header_length; // length of format chunk minus 8 byte header
  uint16_t format;        // identifies WAVE_FORMAT_PCM
  uint16_t channels;
  uint32_t freq;          // samples per second per channel
  uint32_t bytespersecond;
  uint16_t blockalign;    // basic block size
  uint16_t bitspersample;

  // PCM formats then go straight to the data chunk
  uint8_t data[4];        // 'data'
  uint32_t data_length;   // length of data chunk minus 8 byte header
} st_wav_header_t;


int
misc_wav_write_header (FILE *fh, int channels, int freq,
                       int bytespersecond, int blockalign,
                       int bitspersample, int data_length)
{
  st_wav_header_t wav_header;
  memset (&wav_header, 0, sizeof (st_wav_header_t));

  memcpy (wav_header.magic,   "RIFF", 4);
  wav_header.total_length =   me2le_32 (data_length + sizeof (st_wav_header_t) - 8);
  memcpy (wav_header.type,    "WAVE", 4);
  memcpy (wav_header.fmt,     "fmt ", 4);
  wav_header.header_length =  me2le_32 (16); // always 16
  wav_header.format =         me2le_16 (1); // WAVE_FORMAT_PCM == default
  wav_header.channels =       (uint16_t) me2le_16 (channels);
  wav_header.freq =           me2le_32 (freq);
  wav_header.bytespersecond = me2le_32 (bytespersecond);
  wav_header.blockalign =     (uint16_t) me2le_16 (blockalign);
  wav_header.bitspersample =  (uint16_t) me2le_16 (bitspersample);
  memcpy (wav_header.data,    "data", 4);
  wav_header.data_length =    me2le_32 (data_length);

  return fwrite (&wav_header, 1, sizeof (st_wav_header_t), fh);
}


void
misc_wav_generator (unsigned char *bit, int bit_length, float volume, int wav_type)
{
  int i;

#ifndef  USE_LIBMATH
  (void) wav_type;
#else
  if (wav_type == SQUARE_WAVE)
#endif // USE_LIBMATH
    {
      int half_bit_length = bit_length / 2;
      unsigned char vol_byte = (unsigned char) (0xfc * volume);
      for (i = 0; i < half_bit_length; i++)
        bit[i] = vol_byte;

      if (bit_length & 1)                       // Is bit_length odd?
        bit[i++] = 0x80;

      vol_byte = (unsigned char) (0x06 * volume);
      for (; i < bit_length; i++)
        bit[i] = vol_byte;
    }
#ifdef  USE_LIBMATH
  else // SINE_WAV
    {
      double v = volume;
      for (i = 0; i < bit_length; i++)
        bit[i] = (unsigned char)
                   ((sin (((2 * M_PI) / bit_length) * i) * v + 1) * 128);
    }
#endif // USE_LIBMATH
}
