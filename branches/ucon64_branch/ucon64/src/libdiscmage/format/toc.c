/*
toc.c - suppert of ISO/BIN Tracks with external toc files

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

#if 0
#define SIZERAW 2352
#define SIZEISO_MODE1 2048
#define SIZEISO_MODE2_RAW 2352
#define SIZEISO_MODE2_FORM1 2048
#define SIZEISO_MODE2_FORM2 2336
#define AUDIO 0
#define MODE1 1
#define MODE2 2
#define MODE1_2352 10
#define MODE2_2352 20
#define MODE1_2048 30
#define MODE2_2336 40
#define UNKNOWN -1
#endif


dm_image_t *
dm_toc_read (dm_image_t *image, const char *toc_toc)
{
  (void) image;
  (void) toc_toc;

#if 0
//  Toc *toc;
//  Msf start, end;
  const Track *trun;
  int t = 0;
  char *binFileName = NULL;
  int32_t err = 0;

  dm_clean (image);

  // first make some consistency checks, surely not complete to identify
  // toc-files that can be correctly converted to cue files
  for (image->tracks = 0;; image->tracks++)
    {
//      const SubTrack *strun;
      int32_t stcount;
//      TrackData::Type sttype1, sttype2;
//      SubTrackIterator stitr (trun);

// test toc file

// supported mode?
#if 0
      switch (trun->type ())
        {
        case TrackData::MODE0:
        case TrackData::MODE2_FORM2:
          fprintf (stderr, "ERROR: Cannot convert: track %d has unsupported mode.",
                   trackNr);
          err = 1;
          break;
        default:
          break;
        }

      for (strun = stitr.first (), stcount = 0;
           strun != NULL; strun = stitr.next (), stcount++)
        {

          // store types of first two sub-tracks for later evaluation
          switch (stcount)
            {
            case 0:
              sttype1 = strun->TrackData::type ();
              break;
            case 1:
              sttype2 = strun->TrackData::type ();
              break;
            }
#endif
  // check if whole toc-file just references a single bin file
//                      message (-2,
//                               "Cannot convert: toc-file references multiple data files.");

      switch (stcount)
        {
        case 0:
          fprintf (stderr, "ERROR: Cannot convert: track %d references no data file.",
                   trackNr);
          err = 1;
          break;

        case 1:
          if (sttype1 != TrackData::DATAFILE)
            {
              message (-2,
                       "Cannot convert: track %d references no data file.",
                       trackNr);
              err = 1;
            }
          break;

        case 2:
          if (sttype1 != TrackData::ZERODATA
              || sttype2 != TrackData::DATAFILE)
            {
              fprintf (stderr, "ERROR: Cannot convert: track %d has unsupported layout.",
                       trackNr);
              err = 1;
            }
          break;

        default:
          fprintf (stderr, "ERROR: Cannot convert: track %d has unsupported layout.",
                   trackNr);
          err = 1;
          break;
        }
    }

  if (binFileName == NULL)
    {
      fprintf (stderr, "ERROR: Cannot convert: toc-file references no data file.");
      err = 1;
    }

  if (err)
    return NULL;

// read the toc file into dm_image_t


  int32_t offset = 0;
  for (trun = titr.first (start, end), trackNr = 1;
       trun != NULL; trun = titr.next (start, end), trackNr++)
    {
      out << "  TRACK ";
      out.form ("%02d ", trackNr);

      switch (trun->type ())
        {
        case TrackData::AUDIO:
          out << "AUDIO";
          break;
        case TrackData::MODE1:
        case TrackData::MODE2_FORM1:
          out << "MODE1/2048";
          break;
        case TrackData::MODE2:
        case TrackData::MODE2_FORM_MIX:
          out << "MODE2/2336";
          break;
        case TrackData::MODE1_RAW:
          out << "MODE1/2352";
          break;
        case TrackData::MODE2_RAW:
          out << "MODE2/2352";
          break;
        default:
          break;
        }

      out << endl;

      const SubTrack *strun;
      SubTrackIterator stitr (trun);
      int32_t pregap = 0;

      for (strun = stitr.first (); strun != NULL; strun = stitr.next ())
        {
          if (strun->TrackData::type () == TrackData::ZERODATA)
            {
              out << "    PREGAP " << trun->start ().str () << endl;
              pregap = 1;
            }
          else
            {
              if (!pregap && trun->start ().lba () != 0)
                {
                  out << "    INDEX 00 " << Msf (offset).str () << endl;
                  out << "    INDEX 01 "
                    << Msf (offset + trun->start ().lba ()).str () << endl;
                }
              else
                {
                  out << "    INDEX 01 " << Msf (offset).str () << endl;
                }

              offset += trun->length ().lba ();

              if (pregap)
                offset -= trun->start ().lba ();
            }
        }
    }


  return 0;
}


#endif
  return 0;
}


int
toc_init (dm_image_t *image)
{
  int t = 0;
  FILE *fh = NULL;
  char buf[FILENAME_MAX];
  
  strcpy (buf, image->fname);
  set_suffix (buf, ".TOC");
  if (!dm_toc_read (image, buf)) // read toc toc into dm_image_t
    {
          if (!(fh = fopen (image->fname, "rb"))) // no toc; try the image itself 
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
      
      if (!format_track_init (track, fh))
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

  image->desc = "ISO/BIN track with external toc file";

  fclose (fh);
  return 0;
}


int
dm_toc_write (const dm_image_t *image)
{
  int result = (-1), t = 0;
  
  for (t = 0; t < image->tracks; t++)
    {
      char buf[MAXBUFSIZE];
//      char buf2[MAXBUFSIZE];
      dm_track_t *track = (dm_track_t *) &image->track[t];
      FILE *fh = NULL;

      strcpy (buf, image->fname);
#if 0
      sprintf (buf2, "_%d.TOC", t);
      set_suffix (buf, buf2);
#else
      set_suffix (buf, ".TOC");
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
                     "AUDIO\n"
                     "\n"
                     "TRACK %s\n"
//                     "NO COPY\n"
                     "DATAFILE \"%s\" %u// length in bytes: %u\n",
                     dm_get_track_desc (track->mode, track->sector_size, FALSE),
                     basename2 (image->fname),
		     (unsigned int) (track->total_len * track->sector_size),
                     (unsigned int) (track->total_len * track->sector_size));
              break;

          case 1: // mode1
            fprintf (fh, 
                     "CD_ROM\n"
                     "\n"
                     "TRACK %s\n"
//                     "NO COPY\n"
                     "DATAFILE \"%s\" %u// length in bytes: %u\n",
                     dm_get_track_desc (track->mode, track->sector_size, FALSE),
                     basename2 (image->fname),
		     (unsigned int) (track->total_len * track->sector_size),
                     (unsigned int) (track->total_len * track->sector_size));
            break;
            
          default: // mode2
            fprintf (fh, 
                     "CD_ROM_XA\n"
                     "\n"
                     "TRACK %s\n"
//                     "NO COPY\n"
                     "DATAFILE \"%s\" %u// length in bytes: %u\n",
                     dm_get_track_desc (track->mode, track->sector_size, FALSE),
                     basename2 (image->fname),
		     (unsigned int) (track->total_len * track->sector_size),
                     (unsigned int) (track->total_len * track->sector_size));
            break;
        }

      fclose (fh);
    }
    
  return result;
}
