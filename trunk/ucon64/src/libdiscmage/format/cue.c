/*
cue.c - suppert of ISO/BIN Tracks with external cue files

written by 2002 - 2003 NoisyB (noisyb@gmx.net)


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
#ifdef  HAVE_CONFIG_H
#include "../config.h"
#endif
#include "../misc.h"
#include "../libdiscmage.h"
#include "../libdm_misc.h"
#include "format.h"
#ifdef  DJGPP
#include "../dxedll_priv.h"
#endif


const st_track_desc_t cue_desc[] = 
  {
    {DM_MODE1_2048, "MODE1/2048"}, // MODE2_FORM1
    {DM_MODE1_2352, "MODE1/2352"},
    {DM_MODE2_2336, "MODE2/2336"}, // MODE2_FORM_MIX
    {DM_MODE2_2352, "MODE2/2352"},
    {DM_AUDIO, "AUDIO"},
    {0, NULL}
  };


static const char *
cue_get_desc (int id)
{
  int x = 0;
  
  for (x = 0; cue_desc[x].desc; x++)
    if (id == cue_desc[x].id)
      return cue_desc[x].desc;
  return "";
}


#if 0
static struct cue_track_pos
{
  int track;
  unsigned short mode;
  unsigned short minute;
  unsigned short second;
  unsigned short frame;
}
cue_current_pos;


/* number of tracks on the cd */
static int nTracks = 0;
#endif


dm_image_t *
dm_cue_read (dm_image_t *image, const char *cue_cue)
{
  char buf[MAXBUFSIZE];
#if 0
  , *p = NULL;
  char inum[3];
  char min;
  char sec;
  char fps;
  int already_set = 0;
#endif  
  int t = 0;
  FILE *fh = NULL;
  int x = 0;
  
  if (!(fh = fopen (cue_cue, "rb")))
    return NULL; // cue_cue not found

  while (fgets (buf, MAXBUFSIZE, fh))
    {
      if (strstr (buf, " TRACK ")) // a new track entry
        {
          dm_track_t *track = (dm_track_t *) &image->track[t];

          track->sector_size = track->mode = 0;
            
          for (x = 0; track_probe[x].sector_size; x++)
            if (strstr (buf, cue_desc[x].desc))
              {
                dm_get_track_by_id (cue_desc[x].id, &track->mode, &track->sector_size);
                break;
              }       

          if (!track->sector_size)
            {
              fclose (fh);
              return !t ? NULL : image;
            }

          // get the track indexes
          while (fgets (buf, MAXBUFSIZE, fh))
            {
              if (strstr (buf, "TRACK "))
                break;
#if 0

      /* Track 0 or 1, take the first an get fill the values */
      if (strncmp (&buf[4], "INDEX ", 6) == 0)
        {
          /* check stuff here so if the answer is false the else stuff below won't be executed */
          strncpy (inum, &buf[10], 2);
          inum[2] = '\0';
          if ((already_set == 0) &&
              ((strcmp (inum, "00") == 0) || (strcmp (inum, "01") == 0)))
            {
              already_set = 1;

              min = ((buf[13] - '0') << 4) | (buf[14] - '0');
              sec = ((buf[16] - '0') << 4) | (buf[17] - '0');
              fps = ((buf[19] - '0') << 4) | (buf[20] - '0');

              track->minute = (((min >> 4) * 10) + (min & 0xf));
              track->second = (((sec >> 4) * 10) + (sec & 0xf));
              track->frame = (((fps >> 4) * 10) + (fps & 0xf));
            }
        }
      else if (strncmp (&buf[4], "PREGAP ", 7) == 0)
        {;                      /* ignore */
        }
      else if (strncmp (&buf[4], "FLAGS ", 6) == 0)
        {;                      /* ignore */
        }
#endif
        }
      }
    }
  return 0;
}


int
cue_init (dm_image_t *image)
{
  int t = 0;
  FILE *fh = NULL;
  char buf[FILENAME_MAX];
  
  strcpy (buf, image->fname);
      set_suffix (buf, ".CUE");
      if (!dm_cue_read (image, buf)) // read cue cue into dm_image_t
        {
          if (!(fh = fopen (image->fname, "rb"))) // no cue; try the image itself 
            return -1;
        }

#if 1
  image->sessions =
  image->tracks =
  image->session[0] = 1;
#endif

  for (t = 0; t < image->tracks; t++)
    {
      dm_track_t *track = (dm_track_t *) &image->track[t];
      
      if (!dm_track_init (track, fh))
        {
          track->track_len =
          track->total_len = q_fsize (image->fname) / track->sector_size;
        }
      else
        {
          fclose (fh);
          return !t ? (-1) : 0;
        }
    }

  image->desc = "ISO/BIN track with external cue file";

  fclose (fh);
  return 0;
}



/*
  NOTE: TOC -> CUE conversion
  the resulting cue file is only valid if the
  toc-file was created with cdrdao using the commands 'read-toc'
  or 'read-cd'. For manually created or edited toc-files the
  cue file may not be correct. This program just checks for
  the most obvious toc-file features that cannot be converted to
  a cue file.
  Furthermore, if the toc-file contains audio tracks the byte
  order of the image file will be wrong which results in static
  noise when the resulting cue file is used for recording
  (even with cdrdao itself).
*/
int
dm_cue_write (const dm_image_t *image)
{
  int result = (-1), t = 0;
        
  for (t = 0; t < image->tracks; t++)
    {
#if     FILENAME_MAX > MAXBUFSIZE
      char buf[FILENAME_MAX];
#else
      char buf[MAXBUFSIZE];
#endif
//      char buf2[MAXBUFSIZE];
      dm_track_t *track = (dm_track_t *) &image->track[t];
      FILE *fh = NULL;

      strcpy (buf, image->fname);
#if 0
      sprintf (buf2, "_%d.CUE", t);
      set_suffix (buf, buf2);
#else
      set_suffix (buf, ".CUE");
#endif

      if (!(fh = fopen (buf, "wb")))
        {
          result = -1;
          continue;
        }
      else result = 0;

      switch (track->mode)
        {
          case 0: // audio
            fprintf (fh,
                     "FILE \"%s\" WAVE\r\n"
                     "  TRACK %02d %s\r\n",
                     image->fname, t + 1,
                     cue_get_desc (track->id));

#if 0
            if (/* t + 1 > 1 && */
                track->pregap_len > 0)
              fprintf (fh, "    PREGAP 00:02:00\r\n");
#endif
            break;

          case 1: // iso
          fprintf (fh,
                   "FILE \"%s\" BINARY\r\n"
                   "  TRACK %02d %s\r\n",
                   image->fname, t + 1,
                   cue_get_desc (track->id));
            break;

          default: // bin
          fprintf (fh,
                   "FILE \"%s\" BINARY\r\n"
                   "  TRACK %02d %s\r\n",
                   image->fname, t + 1,
                   cue_get_desc (track->id));
            break;
        }

      fprintf (fh, "    INDEX 01 00:00:00\r\n");
#if 0
      if (/* discmage.pregap &&*/ track->mode && t + 1 == image->tracks)   // instead of saving pregap
        fprintf (fh, "  POSTGAP 00:02:00\r\n");
#endif
      fclose (fh);
    }

  return result;
}
