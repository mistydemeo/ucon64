/*
libdm_misc.c - libdiscmage miscellaneous

written by 2002 NoisyB (noisyb@gmx.net)


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
#include <config.h>
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#ifdef  DEBUG
#ifdef  __GNUC__
#warning DEBUG active
#else
#pragma message ("DEBUG active")
#endif
#endif
#include "misc.h"
#include "libdiscmage.h"
#include "libdm_misc.h"
#ifdef  DJGPP                                   // DXE's are specific to DJGPP
// It's important that this file is included _after_ the headers of code
//  external to the DXE!
#include "dxedll_priv.h"
#endif
#include "cdi.h"
#include "nero.h"
#include "sheets.h"

#ifndef CD_MINS
#define CD_MINS              74 /* max. minutes per CD, not really a limit */
#define CD_SECS              60 /* seconds per minute */
#define CD_FRAMES            75 /* frames per second */
#endif

static const char pvd_magic[] = {0x01, 'C', 'D', '0', '0', '1', 0x01, 0};
static const char svd_magic[] = {0x02, 'C', 'D', '0', '0', '1', 0x01, 0};
static const char vdt_magic[] = {(const char) 0xff, 'C', 'D', '0', '0', '1', 0x01, 0};


typedef struct
{
  int mode;
  int seek_header; // sync, head, sub
  int sector_size; // data
  int seek_ecc;    // EDC, zero, ECC, spare
} st_track_probe_t;


/*
 * A CD-ROM physical sector size is 2048, 2052, 2056, 2324, 2332, 2336, 
 * 2340, or 2352 bytes long.  

*         Sector types of the standard CD-ROM data formats:
 *
 * format   sector type               user data size (bytes)
 * -----------------------------------------------------------------------------
 *   1     (Red Book)    CD-DA          2352    (CD_FRAMESIZE_RAW)
 *   2     (Yellow Book) Mode1 Form1    2048    (CD_FRAMESIZE)
 *   3     (Yellow Book) Mode1 Form2    2336    (CD_FRAMESIZE_RAW0)
 *   4     (Green Book)  Mode2 Form1    2048    (CD_FRAMESIZE)
 *   5     (Green Book)  Mode2 Form2    2328    (2324+4 spare bytes)
 *
 *
 *       The layout of the standard CD-ROM data formats:
 * -----------------------------------------------------------------------------
 * - audio (red):                  | audio_sample_bytes |
 *                                 |        2352        |
 *
 * - data (yellow, mode1):         | sync - head - data - EDC - zero - ECC |
 *                                 |  12  -   4  - 2048 -  4  -   8  - 276 |
 *
 * - data (yellow, mode2):         | sync - head - data |
 *                                 |  12  -   4  - 2336 |
 *
 * - XA data (green, mode2 form1): | sync - head - sub - data - EDC - ECC |
 *                                 |  12  -   4  -  8  - 2048 -  4  - 276 |
 *
 * - XA data (green, mode2 form2): | sync - head - sub - data - Spare |
 *                                 |  12  -   4  -  8  - 2324 -  4    |
 *
 */

/*
  from CDmage
  
  m1_2048=iso
  m1_2352=tao
  m2_2336=mm2
  m2_2352=tao
  cf_2048=fcd
  a1_2352=pcm
  a2_2352=wav
  cg_2448=cdg
  ci_2336=mci
  ci_2352=cdi
  cv_2048=vcd

CDRWin cuesheet file (*.cue)|*.cue
M1/2048 track (*.iso)|*.iso
M1/2352 track (*.bin;*.tao;*.iso;*.img;*.bwi)|*.bin;*.tao;*.iso;*.img;*.bwi
M2/2336 track (*.mm2)|*.mm2
M2/2352 track (*.bin;*.tao;*.iso;*.img;*.bwi)|*.bin;*.tao;*.iso;*.img;*.bwi
VirtualCD uncompressed container file (*.fcd)|*.fcd
Audio track image file (*.pcm;*.bin;*.img;*.bwi)|*.pcm;*.bin;*img;*.bwi
Wave file 44.1KHz 16-bit stereo (*.wav)|*.wav
Virtual Drive uncomp. container file (*.vcd)|*.vcd
Nero Burning Rom image file (*.nrg)|*.nrg
CloneCD image control file (*.ccd)|*.ccd
Raw Image (*.iso;*.bin;*.tao;*.img;*.bwi;*.mm2)|*.iso;*.bin;*.tao;*.img;*.bwi;*.mm2
BlindWrite TOC file (*.bwt)|*.bwt
DiscJuggler CD image file (*.cdi)|*.cdi
Gear image files (*.rdb;*.md1;*.xa)|*.rdb;*.md1;*.xa
NTI CD image files (*.cdp;*.ncd)|*.cdp;*.ncd
Prassi Global-CD image file (*.gcd)|*.gcd
WinOnCD full CD image file (*.c2d)|*.c2d
Easy CD Creator image file (*.cif)|*.cif
*/

const st_track_probe_t track_probe[] = 
  {
    {1, 0,  2048, 0},   // MODE1/2048, MODE1
    {1, 16, 2352, 288}, // MODE1/2352, MODE1_RAW
    {2, 8,  2336, 280}, // MODE2/2336, MODE2
    {2, 24, 2352, 280}, // MODE2/2352, MODE2_RAW
#if 0
    {2, 24, 2324, 4},   // MODE2/2328, MODE2_FORM2
    {2, 0,  2340, 0},
    {2, 0,  2368, 0},
    {2, 0,  2448, 0},
    {2, 0,  2646, 0},
    {2, 0,  2647, 0},
    {2, 0,  2336, 280},  // MODE2/2336, Macintosh
    {2, 16, 2352, 280}, // MODE2/2352, Macintosh
    {2, 0,  2056, 0},  // MODE2/2056, Macintosh
#endif
    {0, 0,  2352, 0},  // AUDIO/2352, AUDIO
    {0, 0, 0, 0}
  };


const char *
dm_get_track_desc (int mode, int sector_size, int cue)
// if cue == TRUE return track_desc[x].cue
{
  typedef struct
    {
      int mode;
      int sector_size;
//      int multiple_sessions;
      const char *cue;
      const char *toc;
    } st_track_desc_t;

  int x = 0;
  static const st_track_desc_t track_desc[] = {
      {1, 2048, "MODE1/2048", "MODE1"}, // MODE2_FORM1
      {1, 2352, "MODE1/2352", "MODE1_RAW"},
      {2, 2336, "MODE2/2336", "MODE2"}, // MODE2_FORM_MIX
      {2, 2352, "MODE2/2352", "MODE2_RAW"},
      {0, 2352, "AUDIO",      "AUDIO"},
#if 0
      {0, 0,    NULL,         "MODE0"},
      {0, 0,    NULL,         "MODE2_FORM2"},
#endif
      {0, 0,    NULL,         NULL}
    };

  for (x = 0; track_desc[x].sector_size; x++)
    if (track_desc[x].mode == mode &&
        track_desc[x].sector_size == sector_size)
      return (cue ? track_desc[x].cue : track_desc[x].toc);

  return NULL;
}


static void (* dm_ext_gauge) (int, int);


const char *dm_msg[] = {
  "ERROR: %s has been deprecated\n",
  "ERROR: unknown/unsupported track mode\n",
  "ERROR: the images track mode is already MODE1/2048\n",
  NULL
};


#if 0
void
writewavheader (FILE * fdest, int track_length)
{
  wav_header_t wav_header;

  memset (&wav_header, 0, sizeof (wav_header_t));

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

  fwrite (&wav_header, 1, sizeof (wav_header_t), fdest);
}
#endif


int
dm_bcd_to_int (int b)
{
  return (b & 0x0F) + 10 * (((b) >> 4) & 0x0F);
}


int
dm_int_to_bcd (int i)
{
#if 1
  return i % 10 | ((i / 10) % 10) << 4;
#else
  return ((i / 10) * 16) + (i % 10);
#endif
}


int
dm_lba_to_msf (int lba, int *m0, int *s0, int *f0)
#if 1
{
  static int m, s, f;

  m = s = f = -1;

#ifdef	__follow_redbook__
  if (lba >= -150 && lba < 405000) /* lba <= 404849 */
#else
  if (lba >= -150)
#endif
    {
      m = (lba + 150) / CD_SECS / CD_FRAMES;
      s = (lba + 150 - m * CD_SECS * CD_FRAMES) / CD_FRAMES;
      f = (lba + 150 - m * CD_SECS * CD_FRAMES - s * CD_FRAMES);

    }
  else if (lba >= -45150 && lba <= -151)
    {
      m = (lba + 450150) / CD_SECS / CD_FRAMES;
      s = (lba + 450150 - m * CD_SECS * CD_FRAMES) / CD_FRAMES;
      f = (lba + 450150 - m * CD_SECS * CD_FRAMES - s * CD_FRAMES);
    }

  m0 = (int *) &m;
  s0 = (int *) &s;
  f0 = (int *) &f;

  if (lba > 404849 || m == -1 || s == -1 || f == -1) /* 404850 -> 404999: lead out */
    return FALSE;
  return TRUE;
}
#else
{
  static int m, s, f;

  m = s = f = -1;

  if (lba >= -150)
    {
      m = (lba + 150) / (CD_SECS * CD_FRAMES);
      lba -= m * CD_SECS * CD_FRAMES;
      s = (lba + 150) / CD_FRAMES;
      lba -= s * CD_FRAMES;
      f = (lba + 150);
    }
  else
    {
      m = (lba + 450150) / (CD_SECS * CD_FRAMES);
      lba -= m * CD_SECS * CD_FRAMES;
      s = (lba + 450150) / CD_FRAMES;
      lba -= s * CD_FRAMES;
      f = (lba + 450150);
    }

  m0 = (int *) &m;
  s0 = (int *) &s;
  f0 = (int *) &f;

  return TRUE;
}
#endif


int
dm_msf_to_lba (int m, int s, int f, int force_positive)
#if 1
{
  long ret = m * CD_SECS + s;

  ret *= CD_FRAMES;
  ret += f;
  if (m < 90 || force_positive)
    ret -= 150;
  else
    ret -= 450150;
  return ret;
}
#else
{
  return m * CD_SECS * CD_FRAMES + s * CD_FRAMES + f;
}
#endif


static void
dm_clean (dm_image_t *image)
{
  int x = 0;

  memset (image, 0, sizeof (dm_image_t));
  for (x = 0; x < DM_MAX_TRACKS; x++)
    image->track[x].iso_header_start = (-1);
}


int
dm_free (dm_image_t *image)
{
#if 0
  memset (image, 0, sizeof (dm_image_t));
#else
  free (image);
#endif
  image = NULL;
  return 0;
}


#if 0
static int
dirty_hack (const char *fname, int track_num)
// brute force sync_data[] callibration for unknown images
{
  FILE *fh;
  int32_t pos = 0;
  const char sync_data[] = {0, (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff, 0};
  char buf[MAXBUFSIZE];
  int size = q_fsize (fname);
  if (!(fh = fopen (fname, "rb")))
    return -1;

  (void) track_num;
  
  for (pos = 0; pos < size - 16; pos++)
    {
      fseek (fh, pos, SEEK_SET);
      fread (&buf, 16, 1, fh);
      if (!memcmp (sync_data, buf, 12))
        {
          printf ("%x %d\n", pos, pos);
          break;
        }
    }

  fclose (fh);
  
  return 0;
}
#endif


const dm_track_t *
dm_track_init (dm_track_t *track, FILE *fh)
{
  int x = 0, identified = 0;
  const char sync_data[] = {0, (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff,
                            (const char) 0xff, (const char) 0xff, 0};
  char value_s[32];

  fseek (fh, track->track_start, SEEK_SET);
#if 0
  fread (value_s, 1, 16, fh);
#else
// callibrate
  fseek (fh, -15, SEEK_CUR);
  for (x = 0; x < 64; x++)
    {
      if (fread (&value_s, 1, 16, fh) != 16)
        return NULL;
      fseek (fh, -16, SEEK_CUR);
      if (!memcmp (sync_data, value_s, 12))
        break;
      fseek (fh, 1, SEEK_CUR);
    }
#endif

  if (!memcmp (sync_data, value_s, 12))
    for (x = 0; track_probe[x].sector_size; x++)
      if (track_probe[x].mode == value_s[15])
        {
          // search for valid PVD in sector 16 of source image
          fseek (fh, (track_probe[x].sector_size * 16) +
                 track_probe[x].seek_header + track->track_start, SEEK_SET);
          fread (value_s, 1, 16, fh);

          if (!memcmp (pvd_magic, &value_s, 8))
            {
              identified = 1;
              break;
            }
        }

  // no sync_data found? probably MODE1/2048
  if (!identified)
    {
      x = 0;
      if (track_probe[x].sector_size != 2048)
        fprintf (stderr, "ERROR: dm_track_init()\n");

      fseek (fh, (track_probe[x].sector_size * 16) +
             track_probe[x].seek_header + track->track_start, SEEK_SET);
      fread (value_s, 1, 16, fh);

      if (!memcmp (pvd_magic, &value_s, 8))
        identified = 1;
    }

  if (!identified)
    {
      fprintf (stderr, "ERROR: could not find iso header of current track\n");
      return NULL;
    }

  track->sector_size = track_probe[x].sector_size;
  track->mode = track_probe[x].mode;
  track->seek_header = track_probe[x].seek_header;
  track->seek_ecc = track_probe[x].seek_ecc;
  track->iso_header_start = (track_probe[x].sector_size * 16) + track_probe[x].seek_header;
  track->desc = dm_get_track_desc (track->mode, track->sector_size, TRUE);

  return track;
}


dm_image_t *
dm_reopen (const char *fname, uint32_t flags, dm_image_t *image)
// recurses through all <image_type>_init functions to find correct image type
{
  typedef struct
    {
      int type;
      int (* func) (dm_image_t *);
    } st_probe_t;

  static st_probe_t probe[] =
    {
      {DM_SHEET, sheet_init},
      {DM_CDI, cdi_init},
#if 0
      {DM_NRG, nrg_init},
      {DM_CCD, ccd_init},
      {DM_UNKNOWN, NULL},
#endif
      {0, NULL}
    };
  int x = 0;
//  static dm_image_t image2;

#ifdef  DEBUG
  printf ("sizeof (dm_track_t) == %d\n", sizeof (dm_track_t));
  printf ("sizeof (dm_image_t) == %d\n", sizeof (dm_image_t));
  fflush (stdout);
#endif

//  dirty_hack (fname, 0);

  if (image)
    dm_free (image);

  if (access (fname, F_OK) != 0)
    return NULL;

  if (!image)
    image = (dm_image_t *) malloc (sizeof (dm_image_t));
//    image = (dm_image_t *) &image2;
  if (!image)
    return NULL;

  dm_clean (image);
  image->flags = flags;
  strcpy (image->fname, fname);
  for (x = 0; probe[x].type; x++)
    if (probe[x].func)
      {
        if (probe[x].func (image) != -1)
          {
            image->type = probe[x].type;
            break;
          }
        else
          {
            dm_clean (image);
            image->flags = flags;
            strcpy (image->fname, fname);
          }
      }

  if (!image->type) // unknown image
    return NULL;

  return image;
}


int
dm_fseek (FILE *fp, int track, int how)
{
 (void) fp;                                     // warning remover
 (void) track;                                  // warning remover
 (void) how;                                    // warning remover
 return 0;
}


dm_image_t *
dm_open (const char *fname, uint32_t flags)
{
  return dm_reopen (fname, flags, NULL);
}


int
dm_close (dm_image_t *image)
{
//  dm_clean (image);
  dm_free (image);
  return 0;
}


int
dm_bin2iso (const dm_image_t *image, int track_num)
{
  (void) image;
  (void) track_num;
#if 0
  dm_track_t *track = NULL;
  uint32_t i, size;
  char buf[MAXBUFSIZE];
  FILE *dest, *src;

  if (!image)
    return -1;

  track = (dm_track_t *) &image->track[track_num];

  if (track->mode == 1 && track->sector_size == 2048)
    {
      fprintf (stderr, dm_msg[ALREADY_2048]);
      return -1;
    }

  strcpy (buf, basename (image->fname));
  set_suffix (buf, ".ISO");

  if (!(dest = fopen (buf, "wb")))
    {
      fclose (src);
      return -1;
    }

  size = q_fsize (image->fname);
// TODO: float point exception

  for (i = 0; i < size / track->sector_size; i++)
    {
      if (dm_read (buf, track_num, i, image) == track->sector_size)
        fwrite (&buf[track->seek_header], 2048, 1, dest);
      else break; // truncated?

      if (!(i % 100) && dm_ext_gauge)
        dm_ext_gauge (i * track->sector_size, size);
    }

  if (dm_ext_gauge)
    dm_ext_gauge (i * track->sector_size, size * track->sector_size);

  fclose (dest);
  fclose (src);
#endif
  return 0;
}


int
dm_isofix (const dm_image_t * image, int start_lba, int track_num)
{
#define BOOTFILE_S "bootfile.bin"
#define HEADERFILE_S "header.iso"
  int32_t size_left, last_pos, i, size;
  dm_track_t *track = (dm_track_t *) &image->track[track_num];
  char buf[MAXBUFSIZE], buf2[FILENAME_MAX];
  FILE *dest = NULL, *src = NULL, *boot = NULL, *header = NULL;
  int mac = FALSE;
  const char sub_header[] = {0, 0, 0x08, 0, 0, 0, 0x08, 0};
  
  if (start_lba <= 0)
    {
      fprintf (stderr, "ERROR: Bad LBA value");
      return -1;
    }

  if (!track)
    return -1;
  
  mac = (track->sector_size == 2056 ? TRUE : FALSE);

  if (!(src = fopen (image->fname, "rb")))
    return -1;

  strcpy (buf2, basename (image->fname));
  set_suffix (buf2, ".FIX");
  if (!(dest = fopen (buf2, "wb")))
    {
      fclose (src);
      return -1;
    }

  // Saving boot area to file 'bootfile.bin'...
  if (!(boot = fopen (BOOTFILE_S, "wb")))
    {
      fclose (src);

      fclose (dest);
      remove (buf);

      return -1;
    }

  // Saving ISO header to file 'header.iso'...
  if (!(header = fopen (HEADERFILE_S, "wb")))
    {
      fclose (src);

      fclose (dest);
      remove (buf);

      fclose (boot);
      remove (BOOTFILE_S);

      return -1;
    }

  // save boot area
  for (i = 0; i < 16; i++)
    {
      fseek (src, track->seek_header, SEEK_CUR);
      fread (buf, 2048, 1, src);
      fseek (src, track->seek_ecc, SEEK_CUR);

      if (mac)
        fwrite (sub_header, 8, 1, dest);
      fwrite (buf, 2048, 1, dest);

      if (mac)
        fwrite (sub_header, 8, 1, boot);
      fwrite (buf, 2048, 1, boot);

      if (mac)
        fwrite (sub_header, 8, 1, header);
      fwrite (buf, 2048, 1, header);
    }
  fclose (boot); // boot area written

  // seek & copy pvd etc.
//  last_pos = ftell (src);   // start of pvd
  for (last_pos = ftell (src); memcmp (vdt_magic, buf, 8) != 0; last_pos = ftell (src))
    {
      fseek (src, track->seek_header, SEEK_CUR);
      fread (buf, 2048, 1, src);
      fseek (src, track->seek_ecc, SEEK_CUR);

      if (memcmp (pvd_magic, buf, 8) != 0 &&
          memcmp (svd_magic, buf, 8) != 0 &&
          memcmp (vdt_magic, buf, 8) != 0)
        {
          fprintf (stderr, "ERROR: Found unknown Volume Descriptor");
// which sector?
          return -1;
        }

      printf ("Found %s at sector %d\n",
        !memcmp (pvd_magic, buf, 8) ? "PVD" :
        !memcmp (svd_magic, buf, 8) ? "SVD" :
        !memcmp (vdt_magic, buf, 8) ? "VDT" : "unknown Volume Descriptor",
        last_pos / track->sector_size);

      if (mac)
        fwrite (sub_header, 8, 1, dest);
      fwrite (buf, 2048, 1, dest);

      if (mac)
        fwrite (sub_header, 8, 1, header);
      fwrite (buf, 2048, 1, header);
    }

  // add padding data to header file
  memset (&buf, 0, sizeof (buf));
  size_left = 300 - (last_pos / track->sector_size);
  for (i = 0; i < size_left; i++)
    {
      if (mac == TRUE)
        fwrite (sub_header, 8, 1, header);
      fwrite (buf, 2048, 1, header);
    }
  fclose (header);

  // add padding data to iso image
  if (last_pos > (int) (start_lba * track->sector_size))
    {
      fprintf (stderr, "ERROR: LBA value is too small\n"
               "       It should be at least %d for current ISO image (probably greater)",
               last_pos / track->sector_size);
      return -1;
    }

  if (start_lba < 11700)
    fprintf (stderr,
             "WARNING: LBA value should be greater or equal to 11700 for multisession\n"
             "         images\n");

  // adding padding data up to start LBA value...
  size_left = start_lba - (last_pos / track->sector_size);
  memset (&buf, 0, sizeof (buf));
  for (i = 0; i < size_left; i++)
    {
      if (mac)
        fwrite (sub_header, 8, 1, dest);
      if (!fwrite (buf, 2048, 1, dest))
        return -1;
    }

// append original iso image
  fseek (src, 0L, SEEK_SET);
  size = q_fsize (buf2);
  size_left = size / track->sector_size;
  for (i = 0; i < size_left; i++)
    {
      fseek (src, track->seek_header, SEEK_CUR);
      if (!fread (buf, 2048, 1, src))
        break;
      fseek (src, track->seek_ecc, SEEK_CUR);

      if (mac)
        fwrite (sub_header, 8, 1, dest);
      if (!fwrite (buf, 2048, 1, dest))
        return -1;
    }

  fclose (src);
  fclose (dest);

  return 0;
}


int
dm_disc_read (const dm_image_t *image)
{
#if 1
  fprintf (stderr, dm_msg[DEPRECATED], "dm_disc_read()");
  (void) image;                                 // warning remover
#else
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[MAXBUFSIZE];

  get_property (ucon64.configfile, "cdrw_raw_read", buf2,
    get_property (ucon64.configfile, "cdrw_read", buf2, "cdrdao read-cd --read-raw --device 0,0,0 --driver generic-mmc-raw --datafile"));

  strcpy (buf3, ucon64.rom);
  setext (buf3, ".TOC");
  sprintf (buf, "%s \"%s\" \"%s\"", buf2, ucon64.rom, buf3);

  printf ("%s\n", buf);
  fflush (stdout);
  sync ();

  return system (buf)
#ifndef __MSDOS__
      >> 8                                      // the exit code is coded in bits 8-15
#endif                                          //  (that is, under non-DOS)
  ;
#endif
  return 0;
}


int
dm_disc_write (const dm_image_t *image)
{
#if 1
  fprintf (stderr, dm_msg[DEPRECATED], "dm_disc_write()");
  (void) image;                                 // warning remover
#else
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE], buf3[MAXBUFSIZE];

  get_property (ucon64.configfile, "cdrw_raw_write", buf2,
    get_property (ucon64.configfile, "cdrw_write", buf2, "cdrdao write --eject --device 0,0,0 --driver generic-mmc"));

  strcpy (buf3, ucon64.rom);
  setext (buf3, ".TOC");
  if (access (buf3, F_OK) != 0 || !strncmp (ucon64.file, "MODE", 4))
    ucon64_mktoc (rominfo);

  sprintf (buf, "%s \"%s\"", buf2, buf3);

  printf ("%s\n", buf);
  fflush (stdout);
  sync ();

  return system (buf)
#ifndef __MSDOS__
      >> 8                                      // the exit code is coded in bits 8-15
#endif                                          //  (that is, under non-DOS)
  ;
#endif 
  return 0;
}


uint32_t
dm_get_version (void)
{
  static const uint32_t dm_version = LIB_VERSION (0, 0, 2);
  return dm_version;
}


void
dm_set_gauge (void (* gauge) (int, int))
{
  dm_ext_gauge = gauge;
}


int
dm_read (char *buffer, int track_num, int sector, const dm_image_t *image)
{
  dm_track_t *track = (dm_track_t *) &image->track[track_num];
  FILE *fh;
  
  if (!(fh = fopen (image->fname, "rb")))
    return 0;

  if (fseek (fh, track->track_start + (track->sector_size * sector), SEEK_SET) != 0)
    {
      fclose (fh);
      return 0;
    }
  
  if (fread (buffer, track->sector_size, 1, fh) != track->sector_size)
    {
      fclose (fh);
      return 0;
    }

  fclose (fh);
  return track->sector_size;
}


int
dm_write (const char *buffer, int track_num, int sector, const dm_image_t *image)
{
  (void) buffer;
  (void) track_num;
  (void) sector;
  (void) image;
  return 0;
}


FILE *
dm_fdopen (dm_image_t *image, int track_num, const char *mode)
{
  dm_track_t *track = (dm_track_t *) &image->track[track_num];
  FILE *fh;
  
  if (!(fh = fopen (image->fname, mode)))
    return NULL;

  if (!fseek (fh, track->track_start, SEEK_SET))
    return fh;

  fclose (fh);
  return NULL;
}
