/*
misc_wav.c

written by 2004 NoisyB (noisyb@gmx.net)


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
#include <stdio.h>
#include "misc.h"

void
writewavheader (FILE *fh, int track_length)
{
  typedef struct
    {
      uint8_t magic[4];
      uint32_t total_length;
      uint8_t type[4];
      uint8_t fmt[4];
      uint32_t header_length;
      uint16_t format;
      uint16_t channels;
      uint32_t samplerate;
      uint32_t bitrate;
      uint16_t blockalign;
      uint16_t bitspersample;
      uint8_t data[4];
      uint32_t data_length;
    } st_wav_header_t;

  st_wav_header_t wav_header;
  memset (&wav_header, 0, sizeof (st_wav_header_t));

  strcpy ((char *) wav_header.magic, "RIFF");
  strcpy ((char *) wav_header.type, "WAVE");
  strcpy ((char *) wav_header.fmt, "fmt ");
  strcpy ((char *) wav_header.data, "data");

  wav_header.header_length = me2le_32 (16);
  wav_header.format = me2le_16 (1);
  wav_header.channels = me2le_16 (2);
  wav_header.samplerate = me2le_32 (44100);
  wav_header.bitrate = me2le_32 (176400);
  wav_header.blockalign = me2le_16 (4);
  wav_header.bitspersample = me2le_16 (16);

  wav_header.data_length = me2le_32 (track_length * 2352);
  wav_header.total_length = me2le_32 (wav_header.data_length + 8 + 16 + 12);

  fwrite (&wav_header, 1, sizeof (st_wav_header_t), fh);
}
