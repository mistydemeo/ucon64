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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
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

static const char pvd_magic[] = {0x01, 'C', 'D', '0', '0', '1', 0x01, 0};
static const char svd_magic[] = {0x02, 'C', 'D', '0', '0', '1', 0x01, 0};
static const char vdt_magic[] = {0xff, 'C', 'D', '0', '0', '1', 0x01, 0};

static void (* ext_gauge) (int, int);

const char *dm_msg[] = {
  "ERROR: %s has been deprecated\n",
  "ERROR: unknown/unsupported track mode\n",
  "ERROR: the images track mode is already MODE1/2048\n",
  NULL
};


int
fsize (const char *fname)
{
  struct stat fstate;

  if (!stat (fname, &fstate))
    return fstate.st_size;

  return -1;
}


void
writewavheader (FILE * fdest, int track_length)
{
  wav_header_t wav_header;

  memset (&wav_header, 0, sizeof (wav_header_t));

  strcpy (wav_header.magic, "RIFF");
  strcpy (wav_header.type, "WAVE");
  strcpy (wav_header.fmt, "fmt ");
  strcpy (wav_header.data, "data");

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


int
from_bcd (int b)
{
  return (b & 0x0F) + 10 * (((b) >> 4) & 0x0F);
}


#if 1
int
to_bcd (int i)
{
  return i % 10 | ((i / 10) % 10) << 4;
}
#else
int
to_bcd (int value)
{
  int a, b;
  a = (value / 10) * 16;
  b = value % 10;
  return a + b;
}
#endif


/*
  LBA libdiscmage represents the logical block address for the CD-ROM absolute
  address field or for the offset from the beginning of the current track
  expressed as a number of logical blocks in a CD-ROM track relative
  address field.
  MSF libdiscmage represents the physical address written on CD-ROM discs,
  expressed as a sector count relative to either the beginning of the
  medium or the beginning of the current track.
*/
#if 1
int
lba_to_msf (int lba, dm_msf_t *mp)
{
  int m;
  int s;
  int f;

#ifdef	__follow_redbook__
  if (lba >= -150 && lba < 405000)
    {                           /* lba <= 404849 */
#else
  if (lba >= -150)
    {
#endif
      m = (lba + 150) / 60 / 75;
      s = (lba + 150 - m * 60 * 75) / 75;
      f = (lba + 150 - m * 60 * 75 - s * 75);

    }
  else if (lba >= -45150 && lba <= -151)
    {
      m = (lba + 450150) / 60 / 75;
      s = (lba + 450150 - m * 60 * 75) / 75;
      f = (lba + 450150 - m * 60 * 75 - s * 75);

    }
  else
    {
      mp->cdmsf_min0 = -1;
      mp->cdmsf_sec0 = -1;
      mp->cdmsf_frame0 = -1;

      return (FALSE);
    }
  mp->cdmsf_min0 = m;
  mp->cdmsf_sec0 = s;
  mp->cdmsf_frame0 = f;

  if (lba > 404849)             /* 404850 -> 404999: lead out */
    return (FALSE);
  return (TRUE);
}
#else
int
lba_to_msf (int lba, struct cdrom_msf *msf)
{
  if (lba >= -CD_MSF_OFFSET)
    {
      msf->cdmsf_min0 = (lba + CD_MSF_OFFSET) / (CD_SECS * CD_FRAMES);
      lba -= (msf->cdmsf_min0) * CD_SECS * CD_FRAMES;
      msf->cdmsf_sec0 = (lba + CD_MSF_OFFSET) / CD_FRAMES;
      lba -= (msf->cdmsf_sec0) * CD_FRAMES;
      msf->cdmsf_frame0 = (lba + CD_MSF_OFFSET);
    }
  else
    {
      msf->cdmsf_min0 = (lba + 450150) / (CD_SECS * CD_FRAMES);
      lba -= (msf->cdmsf_min0) * CD_SECS * CD_FRAMES;
      msf->cdmsf_sec0 = (lba + 450150) / CD_FRAMES;
      lba -= (msf->cdmsf_sec0) * CD_FRAMES;
      msf->cdmsf_frame0 = (lba + 450150);
    }
}
#endif


#if 1
int
msf_to_lba (int m, int s, int f, int force_positive)
{
  long ret = m * 60 + s;

  ret *= 75;
  ret += f;
  if (m < 90 || force_positive)
    ret -= 150;
  else
    ret -= 450150;
  return (ret);
}
#else
int
msf_to_lba (struct cdrom_msf *msf)
{
  return (msf->cdmsf_min0 * CD_SECS * CD_FRAMES +
          msf->cdmsf_sec0 * CD_FRAMES + msf->cdmsf_frame0);
}
#endif


#if 0
int32_t
read_raw_frame (int32_t fd, int32_t lba, unsigned char *buf)
{
  struct cdrom_msf *msf = (struct cdrom_msf *) buf;
  int32_t rc;

//  msf = (struct cdrom_msf *) buf;
#if 1
  n2msf (lba + CD_MSF_OFFSET, msf);
#else
  msf->cdmsf_min0 = (lba + CD_MSF_OFFSET) / CD_FRAMES / CD_SECS;
  msf->cdmsf_sec0 = (lba + CD_MSF_OFFSET) / CD_FRAMES % CD_SECS;
  msf->cdmsf_frame0 = (lba + CD_MSF_OFFSET) % CD_FRAMES;
#endif

  if ((rc = ioctl (fd, CDROMREADMODE2, buf)) == -1)
    fprintf (stderr, "ERROR: ioctl CDROMREADMODE2\n");

  return rc;
}
#endif


static dm_track_t *
dm_track_init (dm_track_t *track, FILE *fh)
{
  typedef struct
  {
    int mode;
    int sector_size;
    int seek_header;
    int seek_ecc;
  } st_probe_t;

  const st_probe_t probe[] = 
    {
      {1, 2048, 0, 0},    // MODE1_2048, MODE1
      {1, 2352, 16, 288}, // MODE1_2352, MODE1_RAW
      {2, 2336, 8, 280},  // MODE2_2336, MODE2
      {2, 2352, 24, 280}, // MODE2_2352, MODE2_RAW
#if 0
      {2, 2340, , },
      {2, 2368, , },
      {2, 2448, , },
      {2, 2646, , },
      {2, 2647, , },
      {2, 2336, 0, 280},  // MODE2/2336, Macintosh
      {2, 2352, 16, 280}, // MODE2/2352, Macintosh
      {2, 2056, 0, 0},  // MODE2/2056, Macintosh
#endif
      {0, 0, 0, 0}
    };
  const char sync_data[] = {0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0};
  char buf[32];
  int x = 0, identified = 0;

//TODO?: callibration?
  fread (buf, 1, 16, fh);
  if (!memcmp (sync_data, buf, 12))
    for (x = 0; probe[x].mode; x++)
      if (probe[x].mode == buf[15])
        {
            // search for valid PVD in sector 16 of source image
            fseek (fh, (probe[x].sector_size * 16) + probe[x].seek_header, SEEK_SET);
            fread (buf, 1, 8, fh);

            if (!memcmp (pvd_magic, buf, 8))
              {
                identified = 1;
                break;
              }
        }

  if (!identified)
    return NULL;

  track->sector_size = probe[x].sector_size;
  track->mode = probe[x].mode;
  track->seek_header = probe[x].seek_header;
  track->seek_ecc = probe[x].seek_ecc;

  return track;
}


dm_image_t *
dm_reopen (const char *fname, dm_image_t *image_p)
// recurses through all <image_type>_init functions to find correct image type
{
  typedef struct
    {
      int type;
      int (* func) (dm_image_t *);
      char *desc;
    } st_probe_t;

  static st_probe_t probe[] = 
    {
      {DM_CDI, cdi_init, "CDI (DiscJuggler) Image"},
#if 0
      {DM_ISO, iso_init, "ISO Image"},
      {DM_BIN, bin_init, "BIN Image"},
      {DM_NRG, nrg_init, "NRG (Nero) Image"}, // nero
      {DM_CCD, ccd_init, "CCD (CloneCD) Image"},
      {DM_UNKNOWN, NULL, "Unknown Image"},
#endif
      {0, NULL, NULL}
    };
  static dm_image_t image; //TODO: malloc
  static dm_track_t track; //TODO: malloc
  int x = 0, identified = 0;
  FILE *fh;
  
  memset (&image, 0, sizeof (dm_image_t));
  strcpy (image.fname, fname);

  for (x = 0; probe[x].type; x++)
    if (probe[x].func)
      if (probe[x].func (&image))
        {
          identified = 1;
          break;
        }

  if (!identified)
    return NULL;

  image.type = probe[x].type;
  image.desc = probe[x].desc;

  if (!(fh = fopen (fname, "rb")))
    return NULL;

//  for (x = 0; x < image.tracks; x++)
    {
//      dm_fseek (fh, x, SEEK_SET);
      dm_track_init (&track, fh);
      mem_hexdump (&track, sizeof (dm_track_t), 0);
      image.track = (dm_track_t *) &track;
    }

  fclose (fh);
  return &image;
}


int
dm_fseek (FILE *fp, int track, int how)
{
 return 0;
}


dm_image_t *
dm_open (const char *fname)
{
  return dm_reopen (fname, NULL);
}


int
dm_close (dm_image_t *image)
{
//  fclose (image->fh);
//  free (image);
  return 0;
}


int32_t
dm_rip (dm_image_t *image)
{
  return 0;
}


int
dm_bin2iso (dm_image_t *image)
{
  dm_track_t *track = NULL;
  uint32_t i, size, netto_size = 0;
  char buf[MAXBUFSIZE];
  FILE *dest, *src;
  
  if (!image)
    return -1;

  track = image->track;
    
  if (track->mode == 1 && track->sector_size == 2048)
    {
      fprintf (stderr, dm_msg[ALREADY_2048]);
      return -1;
    }

  if (!(src = fopen (image->fname, "rb")))
    return -1;

  strcpy (buf, basename (image->fname));
  set_suffix (buf, ".ISO");

  if (!(dest = fopen (buf, "wb")))
    {
      fclose (src);
      return -1;
    }

  size = fsize (image->fname);
// TODO: float point exception
  size /= track->sector_size;

  netto_size = track->sector_size - (track->seek_header + track->seek_ecc);

  for (i = 0; i < size; i++)
    {
      fseek (src, track->seek_header, SEEK_CUR);

      fread (buf, 1, netto_size, src);
      fwrite (buf, 1, netto_size, dest);

      fseek (src, track->seek_ecc, SEEK_CUR);

      if (!(i % 100) && ext_gauge)
        ext_gauge (i * track->sector_size, size * track->sector_size);
    }

  if (ext_gauge)
    ext_gauge (i * track->sector_size, size * track->sector_size);

  fclose (dest);
  fclose (src);

  return 0;
}


int32_t
dm_isofix (dm_image_t * image, int start_lba)
{
#define BOOTFILE_S "bootfile.bin"
#define HEADERFILE_S "header.iso"
  int32_t size_left, last_pos;
  dm_track_t *track = NULL;
  uint32_t i, size;
  char buf[MAXBUFSIZE], buf2[FILENAME_MAX];
  FILE *dest = NULL, *src = NULL, *boot = NULL, *header = NULL;
  int mac = FALSE;
  const char sub_header[] = {0, 0, 0x08, 0, 0, 0, 0x08, 0};
  
  if (start_lba <= 0)
    {
      fprintf (stderr, "ERROR: Bad LBA value");
      return -1;
    }

  if (!image)
    return -1;

  track = image->track;
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
  if (last_pos > start_lba * track->sector_size)
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
  size = fsize (buf2);
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
dm_disc_read (dm_image_t *image)
{
#if 1
  fprintf (stderr, dm_msg[DEPRECATED], "dm_disc_read()");
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
dm_disc_write (dm_image_t *image)
{
#if 1
  fprintf (stderr, dm_msg[DEPRECATED], "dm_disc_write()");
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
  ext_gauge = gauge;
}
