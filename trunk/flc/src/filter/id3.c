/*
id3.c - mp3 ID3 support for flc

Copyright (c) 2004 by NoisyB

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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "misc/misc.h"
#include "misc/file.h"
#include "misc/filter.h"
#include "flc.h"
#include "flc_defines.h"
#include "id3.h"

#define EMPTY_TO_UNKNOWN_S(str) (isalpha (*str) ? (char *)str : "?")


/*
  This gives info about the id3 tag found in the last part 
  of the mp3 file. 
*/  
typedef struct
{
  unsigned char magic[3];
  unsigned char title[30];
  unsigned char artist[30];
  unsigned char album[30];
  unsigned char year[4];
  unsigned char comment[30];
  unsigned char genre;
} id3v1_tag_info_t;


typedef struct
{
//TODO
} id3v2_tag_info_t;


typedef struct
{
  int id;
  const char *genre;
} st_genre_t;


static const st_genre_t id3v1_genre[] =
{
  {0,   "Blues"},
  {1,   "Classic Rock"},
  {2,   "Country"},
  {3,   "Dance"},
  {4,   "Disco"},
  {5,   "Funk"},
  {6,   "Grunge"},
  {7,   "Hip-Hop"},
  {8,   "Jazz"},
  {9,   "Metal"},
  {10,  "New Age"},
  {11,  "Oldies"},
  {12,  "Other"},
  {13,  "Pop"},
  {14,  "R&B"},
  {15,  "Rap"},
  {16,  "Reggae"},
  {17,  "Rock"},
  {18,  "Techno"},
  {19,  "Industrial"},
  {20,  "Alternative"},
  {21,  "Ska"},
  {22,  "Death Metal"},
  {23,  "Pranks"},
  {24,  "Soundtrack"},
  {25,  "Euro-Techno"},
  {26,  "Ambient"},
  {27,  "Trip-Hop"},
  {28,  "Vocal"},
  {29,  "Jazz+Funk"},
  {30,  "Fusion"},
  {31,  "Trance"},
  {32,  "Classical"},
  {33,  "Instrumental"},
  {34,  "Acid"},
  {35,  "House"},
  {36,  "Game"},
  {37,  "Sound Clip"},
  {38,  "Gospel"},
  {39,  "Noise"},
  {40,  "Alternative Rock"},
  {41,  "Bass"},
  {43,  "Punk"},
  {44,  "Space"},
  {45,  "Meditative"},
  {46,  "Instrumental Pop"},
  {47,  "Instrumental Rock"},
  {48,  "Ethnic"},
  {49,  "Gothic"},
  {50,  "Darkwave"},
  {51,  "Techno-Industrial"},
  {52,  "Electronic"},
  {53,  "Pop-Folk"},
  {54,  "Eurodance"},
  {55,  "Dream"},
  {56,  "Southern Rock"},
  {57,  "Comedy"},
  {58,  "Cult"},
  {59,  "Gangsta"},
  {60,  "Top 40"},
  {61,  "Christian Rap"},
  {62,  "Pop/Funk"},
  {63,  "Jungle"},
  {64,  "Native US"},
  {65,  "Cabaret"},
  {66,  "New Wave"},
  {67,  "Psychadelic"},
  {68,  "Rave"},
  {69,  "Showtunes"},
  {70,  "Trailer"},
  {71,  "Lo-Fi"},
  {72,  "Tribal"},
  {73,  "Acid Punk"},
  {74,  "Acid Jazz"},
  {75,  "Polka"},
  {76,  "Retro"},
  {77,  "Musical"},
  {78,  "Rock & Roll"},
  {79,  "Hard Rock"},
  {80,  "Folk"},
  {81,  "Folk-Rock"},
  {82,  "National Folk"},
  {83,  "Swing"},
  {84,  "Fast Fusion"},
  {85,  "Bebob"},
  {86,  "Latin"},
  {87,  "Revival"},
  {88,  "Celtic"},
  {89,  "Bluegrass"},
  {90,  "Avantgarde"},
  {91,  "Gothic Rock"},
  {92,  "Progressive Rock"},
  {93,  "Psychedelic Rock"},
  {94,  "Symphonic Rock"},
  {95,  "Slow Rock"},
  {96,  "Big Band"},
  {97,  "Chorus"},
  {98,  "Easy Listening"},
  {99,  "Acoustic"},
  {100, "Humour"},
  {101, "Speech"},
  {102, "Chanson"},
  {103, "Opera"},
  {104, "Chamber Music"},
  {105, "Sonata"},
  {106, "Symphony"},
  {107, "Booty Bass"},
  {108, "Primus"},
  {109, "Porn Groove"},
  {110, "Satire"},
  {111, "Slow Jam"},
  {112, "Club"},
  {113, "Tango"},
  {114, "Samba"},
  {115, "Folklore"},
  {116, "Ballad"},
  {117, "Power Ballad"},
  {118, "Rhytmic Soul"},
  {119, "Freestyle"},
  {120, "Duet"},
  {121, "Punk Rock"},
  {122, "Drum Solo"},
  {123, "Acapella"},
  {124, "Euro-House"},
  {125, "Dance Hall"},
  {126, "Goa"},
  {127, "Drum & Bass"},
  {128, "Club-House"},
  {129, "Hardcore"},
  {130, "Terror"},
  {131, "Indie"},
  {132, "BritPop"},
  {133, "Negerpunk"},
  {134, "Polsk Punk"},
  {135, "Beat"},
  {136, "Christian Gangsta"},
  {137, "Heavy Metal"},
  {138, "Black Metal"},
  {139, "Crossover"},
  {140, "Contemporary C"},
  {141, "Christian Rock"},
  {142, "Merengue"},
  {143, "Salsa"},
  {144, "Thrash Metal"},
  {145, "Anime"},
  {146, "JPop"},
  {147, "SynthPop"},
  {148, "Unknown"},
  {0,   NULL},
};


int
id3_open (st_flc_t *flc)
{
  id3v1_tag_info_t id3v1;
//  id3v2_tag_info_t id3v2; // id3v2 sucks
  int x = 0;
  FILE *fh = NULL, *tmp = NULL;

  if (!(tmp = fopen (flc->dstfile, "wb")))
    return -1;
        
  if (!(fh = fopen (flc->srcfile, "rb")))
    {
      fclose (tmp);
      return -1;
    }
                              
  fseek (fh, -sizeof (id3v1_tag_info_t), SEEK_END);
  fread (&id3v1, 1, sizeof (id3v1_tag_info_t), fh);
  fclose (fh);

  if (id3v1.magic[0] != 'T' ||
      id3v1.magic[1] != 'A' ||
      id3v1.magic[2] != 'G') return -1;

  fprintf (tmp, "Title  : %-30.30s", EMPTY_TO_UNKNOWN_S (id3v1.title));
  fprintf (tmp, "Artist : %-30.30s", EMPTY_TO_UNKNOWN_S (id3v1.artist));
  fprintf (tmp, "Album  : %-30.30s", EMPTY_TO_UNKNOWN_S (id3v1.album));
  fprintf (tmp, "Year   : %-4.4s",   EMPTY_TO_UNKNOWN_S (id3v1.year));
  fprintf (tmp, "Comment: %-30.30s", EMPTY_TO_UNKNOWN_S (id3v1.comment));

  for (x = 0; id3v1_genre[x].genre; x++)
    if (id3v1_genre[x].id == id3v1.genre)
      {
        fprintf (tmp, "Genre  : %-30.30s\n ", id3v1_genre[x].genre);
        break;
      }

  fclose (tmp);

  return 0;
}


const st_filter_t id3_filter = {
  FLC_ID3,
  "id3 tags (mp3)",
  ".mp3",
  4,
  (int (*) (void *)) &id3_open,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};
