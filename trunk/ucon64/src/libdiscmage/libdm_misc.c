/*
libdiscmage.c - libdiscmage

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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include "misc.h"
#include "libdiscmage.h"
#ifdef  DJGPP                                   // DXE's are specific to DJGPP
// It's important that this file is included _after_ the headers of code
//  external to the DXE!
#include "dxedll_priv.h"
#endif
#include "cdi.h"
#include "nero.h"

int
fsize (const char *filename)
{
  struct stat fstate;

  if (!stat (filename, &fstate))
    return fstate.st_size;

  return -1;
}


typedef struct
{
  unsigned char magic[4];
  uint32_t total_length;
  unsigned char type[4];
  unsigned char fmt[4];
  uint32_t header_length;
  unsigned short format;
  unsigned short channels;
  uint32_t samplerate;
  uint32_t bitrate;
  unsigned short blockalign;
  unsigned short bitspersample;
  unsigned char data[4];
  uint32_t data_length;
} wav_header_t;



#define ISODCL(from, to) (to - from + 1)
typedef struct st_iso_header
{
  char type[ISODCL (1, 1)];   /* 711 */
  char id[ISODCL (2, 6)];
  char version[ISODCL (7, 7)];        /* 711 */
  char unused1[ISODCL (8, 8)];
  char system_id[ISODCL (9, 40)];     /* achars */
  char volume_id[ISODCL (41, 72)];    /* dchars */
  char unused2[ISODCL (73, 80)];
  char volume_space_size[ISODCL (81, 88)];    /* 733 */
  char unused3[ISODCL (89, 120)];
  char volume_set_size[ISODCL (121, 124)];    /* 723 */
  char volume_sequence_number[ISODCL (125, 128)];     /* 723 */
  char logical_block_size[ISODCL (129, 132)]; /* 723 */
  char path_table_size[ISODCL (133, 140)];    /* 733 */
  char type_l_path_table[ISODCL (141, 144)];  /* 731 */
  char opt_type_l_path_table[ISODCL (145, 148)];      /* 731 */
  char type_m_path_table[ISODCL (149, 152)];  /* 732 */
  char opt_type_m_path_table[ISODCL (153, 156)];      /* 732 */
  char root_directory_record[ISODCL (157, 190)];      /* 9.1 */
  char volume_set_id[ISODCL (191, 318)];      /* dchars */
  char publisher_id[ISODCL (319, 446)];       /* achars */
  char preparer_id[ISODCL (447, 574)];        /* achars */
  char application_id[ISODCL (575, 702)];     /* achars */
  char copyright_file_id[ISODCL (703, 739)];  /* 7.5 dchars */
  char abstract_file_id[ISODCL (740, 776)];   /* 7.5 dchars */
  char bibliographic_file_id[ISODCL (777, 813)];      /* 7.5 dchars */
  char creation_date[ISODCL (814, 830)];      /* 8.4.26.1 */
  char modification_date[ISODCL (831, 847)];  /* 8.4.26.1 */
  char expiration_date[ISODCL (848, 864)];    /* 8.4.26.1 */
  char effective_date[ISODCL (865, 881)];     /* 8.4.26.1 */
  char file_structure_version[ISODCL (882, 882)];     /* 711 */
  char unused4[ISODCL (883, 883)];
  char application_data[ISODCL (884, 1395)];
  char unused5[ISODCL (1396, 2048)];
} st_iso_header_t;

#define ISO_HEADER_START 0
#define ISO_HEADER_LEN (sizeof (st_iso_header_t))


#define DEFAULT_FORMAT   0
#define ISO_FORMAT       1
#define BIN_FORMAT       2
#define CDI_FORMAT       3
#define NRG_FORMAT       4
#define CCD_FORMAT       5

#define READ_BUF_SIZE  (1024*1024)
#define WRITE_BUF_SIZE (1024*1024)


void gauge_dummy (uint32_t pos, uint32_t total)
{
}

//static void (gauge *) (int,int) = gauge_dummy;





static const char pvd[8] = { 0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };     //"\x01" "CD001" "\x01" "\0";
static const char svd[8] = { 0x02, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };     //"\x02" "CD001" "\x01" "\0";
static const char vdt[8] = { 0xff, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0 };     //"\xFF" "CD001" "\x01" "\0";

static const char sub_header[8] = { 0, 0, 0x08, 0, 0, 0, 0x08, 0 };

static const char sync_data[12] =
  { 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0 };

#if 0
  const st_track_modes_t track_modes[] = {
#if 0
    {, 512, , "/512", ""},
    {, 1024, , "/1024", ""},
#endif
    {1, 2048, 0, 0, "MODE1/2048", "MODE1"},
    {1, 2352, 16, 288, "MODE1/2352", "MODE1_RAW"},
    {2, 2336, 8, 280, "MODE2/2336", "MODE2"},
    {2, 2352, 24, 280, "MODE2/2352", "MODE2_RAW"},
#if 0
    {2, 2056, 0, 0, "MODE2/2056 (Macintosh)", ""},      // Macintosh
    {2, 2336, 0, 280, "MODE2/2336 (Macintosh)", ""},    // Macintosh
    {, 2340, , "/2340", ""},
    {2, 2352, 16, 280, "MODE2/2352 (Macintosh)", ""},   // Macintosh
    {, 2368, , "/2368", ""},
    {, 2448, , "/2448", ""},
    {, 2646, , "/2646", ""},
    {, 2647, , "/2647", ""},
#endif
    {0, 0, 0, 0, NULL, NULL}
  };
#endif




void
writewavheader (FILE * fdest, uint32_t track_length)
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
int32_t
to_bcd (int32_t value)
//binary coded decimal
{
  int32_t a, b;
  a = (value / 10) * 16;
  b = value % 10;
  return (a + b);
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
lba_to_msf (uint32_t lba, dm_msf_t *mp)
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
lba_to_msf (uint32_t lba, struct cdrom_msf *msf)
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
uint32_t
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
int32_t
msf2lba (struct cdrom_msf *msf)
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


static int32_t
seek_pvd (int32_t sector_size, int32_t mode, const char *filename)
// will search for valid PVD in sector 16 of source image
{
  uint32_t start = sector_size * 16;
  char buf[10];
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    return 1;

  if (sector_size == 2352)
    start += 16;                // header
  if (mode == 2)
    start += 8;                 // subheader

  fseek (fh, start, SEEK_SET);
  fread (buf, 1, 8, fh);
  fclose (fh);

  if (!memcmp (pvd, buf, 8))
    return 1;

  return 0;
}


dm_image_t *
dm_open (const char *image_filename)
// recurses through all <image_type>_init functions to find correct image type
{
  char buf[32];
  dm_image_t *image;
  FILE *fh = fopen (image_filename, "rb");
  if (!fh)
    return NULL;

  image = (dm_image_t *) malloc (sizeof (dm_image_t));
  if (!image)
    {
      return NULL;
    }
    
  memset (image, 0, sizeof (dm_image_t));

  strcpy (image->filename, image_filename);

  fread (buf, 1, 16, fh);
  fclose (fh);


  image->type =
    !cdi_init (image) ? CDI_FORMAT : DEFAULT_FORMAT;
#if 0
    !nrg_open (image) ? NRG_FORMAT : 
    !iso_init (image) ? ISO_FORMAT :
    !ccd_open (image) ? CCD_FORMAT :
    !bin_open (image) ? BIN_FORMAT : DEFAULT_FORMAT
#endif

#if 0
    if (!memcmp (sync_data, buf, 12))
    {
      switch (buf[15])
        {
        case 2:
          if (seek_pvd (2336, 2, image_filename))
            {
              image->sector_size = 2336;
              track->mode = 2;
#ifdef  __MAC__
              image->seek_header = 0;
#else
              image->seek_header = 8;
#endif
              image->seek_ecc = 280;
            }
          else if (seek_pvd (2352, 2, image_filename))
            {
              image->sector_size = 2352;
#ifdef  __MAC__
              image->seek_header = 16;
#else
              image->seek_header = 24;
#endif
              image->seek_ecc = 280;
            }
          else if (seek_pvd (2056, 2, image_filename))
            {
              image->sector_size = 2056;
            }
          track->mode = 2;
          break;

        case 1:
          if (seek_pvd (2352, 1, image_filename))
            {
              image->sector_size = 2352;
              image->seek_header = 16;
              image->seek_ecc = 288;
            }
          else if (seek_pvd (2048, 1, image_filename))
            {
              image->sector_size = 2048;
            }
          track->mode = 1;
          break;

        default:
          image->sector_size = 2048;
          track->mode = 1;
          break;
        }
    }
#endif
  return image;
}


int dm_close (dm_image_t *image)
{
  free (image);
  return 0;
}



static int32_t
sector_read (char *buffer, int32_t sector_size, int32_t mode, FILE * fsource)
// will put user data into buffer no matter the source libdiscmage
{
  int32_t status;

  if (sector_size == 2352)
    fseek (fsource, 16, SEEK_CUR);      // header

  if (mode == 2)
    fseek (fsource, 8, SEEK_CUR);       // subheader

  status = fread (buffer, 2048, 1, fsource);

  if (sector_size >= 2336)
    {
      fseek (fsource, 280, SEEK_CUR);

      if (mode == 1)
        fseek (fsource, 8, SEEK_CUR);
    }

  return status;
}



int32_t
dm_nrgrip (dm_image_t *image)
{
  return 0;
}


int32_t
dm_rip (dm_image_t *image)
{
  return 0;
}


int32_t
dm_bin2iso (dm_image_t * image)
{
  dm_track_t *track = image->track;
  int32_t seek_header = 0, seek_ecc = 0, sector_size = 0, i, size;
  char buf[MAXBUFSIZE];
  FILE *dest, *src;
//  time_t starttime = time (NULL);

  if (track->mode == 1)
    switch (track->sector_size)
      {
      case 2048:
        fprintf (stderr,
                 "ERROR: the images track mode is already MODE1/2048\n");
        return -1;

      case 2352:
        seek_header = 16;
        seek_ecc = 288;
        sector_size = 2352;
        break;

      default:
        break;
      }
  else if (track->mode == 2)
    switch (track->sector_size)
      {
      case 2336:
#ifdef  __MAC__                 // macintosh
        seek_header = 0;
#else
        seek_header = 8;
#endif
        seek_ecc = 280;
        sector_size = 2336;
        break;

      case 2352:
#ifdef  __MAC__                 // macintosh
        seek_header = 16;
#else
        seek_header = 24;
#endif
        seek_ecc = 280;
        sector_size = 2352;
        break;

      default:
        fprintf (stderr, "ERROR: unknown/unsupported track mode");
        return -1;
      }

  strcpy (buf, basename (image->filename));
#if 0
  setext (buf, ".ISO");
#else
  strcat (buf, ".ISO");
#endif
  size = fsize (image->filename) / sector_size;

  if (!(src = fopen (image->filename, "rb")))
    return -1;
  if (!(dest = fopen (buf, "wb")))
    {
      fclose (src);
      return -1;
    }

  for (i = 0; i < size; i++)
    {
      fseek (src, seek_header, SEEK_CUR);
#ifdef  __MAC__
      if (track_mode == MODE2_2336 || track_mode == MODE2_2352)
        {
          fread (buf, 1, 2056, src);
          fwrite (buf, 1, 2056, dest);
        }
      else
#endif
        {
          fread (buf, 1, 2048, src);
          fwrite (buf, 1, 2048, dest);
        }
      fseek (src, seek_ecc, SEEK_CUR);

//      gauge (starttime, i * sector_size, size * sector_size);
    }

  fclose (dest);
  fclose (src);

  return 0;
}




int32_t
dm_cdi2nero (dm_image_t * image)
{
  return 0;
}


int32_t
dm_isofix (dm_image_t * image)
/*
  ISO start LBA fixing routine

  This tool will take an ISO image with PVD point32_ting
  to bad DR offset and add padding data so actual DR
  gets located in right absolute address.

  Original boot area, PVD, SVD and VDT are copied to
  the start of new, fixed ISO image.

  Supported input image libdiscmages are: 2048, 2336,
  2352 and 2056 bytes per sector. All of them are
  converted to 2048 bytes per sector when writing
  excluding 2056 libdiscmage which is needed by Mac users.
*/
{
  int32_t sector_size = 2048, mode = 1;
  int32_t image_length, remaining_length, last_pos, i;


  static time_t start_time = 0;

  char destfname[256];
  char string[256];
  char buffer[4096];

  int32_t last_vd = FALSE;
  int32_t extractbootonly = FALSE;
  int32_t extractheaderonly = FALSE;
  int32_t maclibdiscmage = FALSE;
  int32_t isolibdiscmage = FALSE;
  int32_t start_lba = strtol (image->filename, NULL, 10);     // !!!!!

  FILE *fsource, *fdest = NULL, *fboot = NULL, *fheader = NULL;

  if (!start_time)
    start_time = time (0);
  if (!start_lba)
    start_lba = 0;              // ????

  strcpy (destfname, "fixed.iso");

  extractbootonly = TRUE;       // boot
  extractheaderonly = TRUE;     // header
  maclibdiscmage = TRUE;        // mac
  isolibdiscmage = TRUE;        // iso

  strcpy (string, image->filename);

  if (!(fsource = fopen (string, "rb")))
    return -1;

  fseek (fsource, 0L, SEEK_END);
  image_length = ftell (fsource);
  fseek (fsource, 0L, SEEK_SET);

// detect libdiscmage

  fread (buffer, 1, 16, fsource);
  if (!memcmp (sync_data, buffer, 12))  // raw (2352)
    {
      sector_size = 2352;
      switch (buffer[15])
        {
        case 2:
          mode = 2;
          break;
        case 1:
          mode = 1;
          break;
        default:
          {
            printf ("Unsupported track mode (%d)", buffer[15]);
            return -1;
          }
        }
      if (seek_pvd (2352, mode, image->filename) == 0)
        {
          printf ("Could not find PVD!\n");
          return -1;
        }
    }
  else if (seek_pvd (2048, 1, image->filename))
    {
      sector_size = 2048;
      mode = 1;
    }
  else if (seek_pvd (2336, 2, image->filename))
    {
      sector_size = 2336;
      mode = 2;
    }
  else if (seek_pvd (2056, 2, image->filename))
    {
      sector_size = 2056;
      mode = 2;
      maclibdiscmage = TRUE;
    }
  else
    {
      fprintf (stderr, "ERROR: Could not find PVD\n");
      return -1;
    }

  if (isolibdiscmage == TRUE)
    maclibdiscmage = FALSE;

  printf ("sector size = %d, mode = %d\n", sector_size, mode);

// detect libdiscmage end

  if (extractbootonly == FALSE && extractheaderonly == FALSE)
    {
      if (start_lba <= 0)
        {
          fprintf (stderr, "ERROR: Bad LBA value");
          return -1;
        }


      printf ("Creating destination file '%s'...\n", destfname);

      if (!(fdest = fopen (destfname, "wb")))
        return -1;
    }

  if (extractheaderonly == FALSE
      || (extractheaderonly == TRUE && extractbootonly == TRUE))
    {
      printf ("Saving boot area to file 'bootfile.bin'...\n");
      fboot = fopen ("bootfile.bin", "wb");
    }
  if (extractbootonly == FALSE
      || (extractheaderonly == TRUE && extractbootonly == TRUE))
    {
      printf ("Saving ISO header to file 'header.iso'...\n");
      fheader = fopen ("header.iso", "wb");
    }

// save boot area

  fseek (fsource, 0L, SEEK_SET);
  for (i = 0; i < 16; i++)
    {
      sector_read (buffer, sector_size, mode, fsource);
      if (extractbootonly == FALSE && extractheaderonly == FALSE)
        {
          if (maclibdiscmage == TRUE)
            fwrite (sub_header, 8, 1, fdest);
          fwrite (buffer, 2048, 1, fdest);
        }
      if (extractheaderonly == FALSE
          || (extractheaderonly == TRUE && extractbootonly == TRUE))
        {
          if (maclibdiscmage == TRUE)
            fwrite (sub_header, 8, 1, fboot);
          fwrite (buffer, 2048, 1, fboot);
        }
      if (extractbootonly == FALSE
          || (extractheaderonly == TRUE && extractbootonly == TRUE))
        {
          if (maclibdiscmage == TRUE)
            fwrite (sub_header, 8, 1, fheader);
          fwrite (buffer, 2048, 1, fheader);
        }
    }

  if (extractheaderonly == FALSE
      || (extractheaderonly == TRUE && extractbootonly == TRUE))
    fclose (fboot);
  if (extractbootonly == TRUE && extractheaderonly == FALSE)
    return 0;                   // boot saved, exit

// seek & copy pvd etc.

  last_pos = ftell (fsource);   // start of pvd

  do
    {
      sector_read (buffer, sector_size, mode, fsource);

      if (!memcmp (pvd, buffer, 8))
        {
          printf ("Found PVD at sector %d\n", last_pos / sector_size);
        }
      else if (!memcmp (svd, buffer, 8))
        {
          printf ("Found SVD at sector %d\n", last_pos / sector_size);
        }
      else if (!memcmp (vdt, buffer, 8))
        {
          printf ("Found VDT at sector %d\n", last_pos / sector_size);
          last_vd = TRUE;
        }
      else
        {
          fprintf (stderr, "ERROR: Found unknown Volume Descriptor");
          return -1;
        }

      if (extractbootonly == FALSE && extractheaderonly == FALSE)
        {
          if (maclibdiscmage == TRUE)
            fwrite (sub_header, 8, 1, fdest);
          fwrite (buffer, 2048, 1, fdest);
        }

      if (maclibdiscmage == TRUE)
        fwrite (sub_header, 8, 1, fheader);
      fwrite (buffer, 2048, 1, fheader);
      last_pos = ftell (fsource);
    }
  while (last_vd == FALSE);

// add padding data to header file

  memset (&buffer, 0, sizeof (buffer));

  remaining_length = 300 - (last_pos / sector_size);

  for (i = 0; i < remaining_length; i++)
    {
      if (maclibdiscmage == TRUE)
        fwrite (sub_header, 8, 1, fheader);
      fwrite (buffer, 2048, 1, fheader);
    }

  fclose (fheader);

  if (extractheaderonly == TRUE)
    return 0;                   // header saved

// add padding data to iso image

  if (last_pos > start_lba * sector_size)
    {
      fprintf (stderr, "ERROR: LBA value is too small\n"
               "       It should be at least %d for current ISO image (probably greater)",
               last_pos / sector_size);
      return -1;
    }

  if (start_lba < 11700)
    fprintf (stderr,
             "WARNING: LBA value should be greater or equal to 11700 for multisession\n"
             "         images\n");

  printf ("Adding padding data up to start LBA value...");

  remaining_length = start_lba - (last_pos / sector_size);

  for (i = 0; i < remaining_length; i++)
    {
      if (!(i % 512))
//        gauge (start_time, i, remaining_length);

      if (maclibdiscmage == TRUE)
        fwrite (sub_header, 8, 1, fdest);

      if (!fwrite (buffer, 2048, 1, fdest))
        return -1;
    }

// append original iso image

  fseek (fsource, 0L, SEEK_SET);

  remaining_length = image_length / sector_size;

  for (i = 0; i < remaining_length; i++)
    {
      if (!(i % 512))
//        gauge (start_time, i, remaining_length);

      if (!sector_read (buffer, sector_size, mode, fsource))
        return -1;

      if (maclibdiscmage == TRUE)
        fwrite (sub_header, 8, 1, fdest);

      if (!fwrite (buffer, 2048, 1, fdest))
        return -1;
    }

  fclose (fsource);
  fclose (fdest);

  return 0;
}


int
dm_disc_read (dm_image_t *image)
{
  fprintf (stderr, "INFO: deprecated for portability reasons\n"
                   "      Use the libdiscmage scripts in contrib/ instead\n");
  return 0;
}


int
dm_disc_write (dm_image_t *image)
{
  dm_disc_read (image);
  return 0;
}
