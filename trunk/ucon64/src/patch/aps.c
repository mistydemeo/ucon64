/*
aps.c - APS support for uCON64

written by        1998 Silo / BlackBag
           1999 - 2001 NoisyB (noisyb@gmx.net)
           2002 - 2003 dbjh


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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "aps.h"


#define N64APS_DESCRIPTION_LEN 50
#define N64APS_BUFFERSIZE 255
#define N64APS_MAGICLENGTH 5

const st_usage_t aps_usage[] =
  {
    {"a", NULL, "apply APS PATCH to ROM (APS<=v1.2)"},
    {"mka", "ORG_ROM", "create APS patch; " OPTION_LONG_S "rom" OPTARG_S "MOD_ROM"},
    {"na", "DESC", "change APS single line DESCRIPTION"},
    {NULL, NULL, NULL}
  };

char n64aps_magic[] = "APS10";
unsigned char n64aps_patchtype = 1, n64aps_encodingmethod = 0;
FILE *n64aps_apsfile, *n64aps_orgfile, *n64aps_modfile;
int n64aps_changefound;


static void
readstdheader (void)
{
  char magic[N64APS_MAGICLENGTH], description[N64APS_DESCRIPTION_LEN + 1];

  fread (magic, 1, N64APS_MAGICLENGTH, n64aps_apsfile);
  if (strncmp (magic, n64aps_magic, N64APS_MAGICLENGTH) != 0)
    {
      fprintf (stderr, "ERROR: Not a valid APS file\n");
      fclose (n64aps_modfile);
      fclose (n64aps_apsfile);
      exit (1);
    }
  n64aps_patchtype = fgetc (n64aps_apsfile);
  if (n64aps_patchtype != 1)                    // N64 patch
    {
      fprintf (stderr, "ERROR: Unable to process patch file\n");
      fclose (n64aps_modfile);
      fclose (n64aps_apsfile);
      exit (1);
    }
  n64aps_encodingmethod = fgetc (n64aps_apsfile);
  if (n64aps_encodingmethod != 0)               // simple encoding
    {
      fprintf (stderr, "ERROR: Unknown or new encoding method\n");
      fclose (n64aps_modfile);
      fclose (n64aps_apsfile);
      exit (1);
    }

  memset (description, ' ', N64APS_DESCRIPTION_LEN);
  fread (description, 1, N64APS_DESCRIPTION_LEN, n64aps_apsfile);
  description[N64APS_DESCRIPTION_LEN] = 0;
  printf ("Description: %s\n", description);
}


static void
readN64header ()
{
  unsigned int n64aps_magictest;
  unsigned char buffer[8], APSbuffer[8], cartid[2], temp, teritory, APSteritory;

  fseek (n64aps_modfile, 0, SEEK_SET);
  fread (&n64aps_magictest, 4, 1, n64aps_modfile);
#ifdef  WORDS_BIGENDIAN
  n64aps_magictest = bswap_32 (n64aps_magictest);
#endif
  buffer[0] = fgetc (n64aps_apsfile);           // APS format
  if (((n64aps_magictest == 0x12408037) && (buffer[0] == 1)) ||
      ((n64aps_magictest != 0x12408037 && (buffer[0] == 0))))
      // 0 for Doctor format, 1 for everything else
    {
      fprintf (stderr, "ERROR: Image is in the wrong format\n");
      fclose (n64aps_modfile);
      fclose (n64aps_apsfile);
      exit (1);
    }

  fseek (n64aps_modfile, 60, SEEK_SET);         // cart id
  fread (cartid, 1, 2, n64aps_modfile);
  fread (buffer, 1, 2, n64aps_apsfile);
  if (n64aps_magictest == 0x12408037)
    {
      temp = cartid[0];
      cartid[0] = cartid[1];
      cartid[1] = temp;
    }
  if ((buffer[0] != cartid[0]) || (buffer[1] != cartid[1]))
    {
      fprintf (stderr, "ERROR: This patch does not belong to this image\n");
      fclose (n64aps_modfile);
      fclose (n64aps_apsfile);
      exit (1);
    }

  if (n64aps_magictest == 0x12408037)
    fseek (n64aps_modfile, 63, SEEK_SET);       // teritory
  else
    fseek (n64aps_modfile, 62, SEEK_SET);

  teritory = fgetc (n64aps_modfile);
  APSteritory = fgetc (n64aps_apsfile);
  if (teritory != APSteritory)
    {
      printf ("WARNING: Wrong country\n");
#if 0
      if (!force)
        {
          fclose (n64aps_modfile);
          fclose (n64aps_apsfile);
          exit (1);
        }
#endif
    }

  fseek (n64aps_modfile, 16, SEEK_SET);       // CRC header position
  fread (buffer, 1, 8, n64aps_modfile);
  fread (APSbuffer, 1, 8, n64aps_apsfile);
  if (n64aps_magictest == 0x12408037)
    {
      temp = buffer[0];
      buffer[0] = buffer[1];
      buffer[1] = temp;
      temp = buffer[2];
      buffer[2] = buffer[3];
      buffer[3] = temp;
      temp = buffer[4];
      buffer[4] = buffer[5];
      buffer[5] = temp;
      temp = buffer[6];
      buffer[6] = buffer[7];
      buffer[7] = temp;
    }
  if (memcmp (APSbuffer, buffer, 8))
    {
      printf ("WARNING: Incorrect image\n");
#if 0
      if (!force)
        {
          fclose (n64aps_modfile);
          fclose (n64aps_apsfile);
          exit (1);
        }
#endif
    }

  fseek (n64aps_apsfile, 5, SEEK_CUR);
  fseek (n64aps_modfile, 0, SEEK_SET);
}


static void
readsizeheader (int modsize)
{
  int orgsize, i;

  fread (&orgsize, 4, 1, n64aps_apsfile);
#ifdef  WORDS_BIGENDIAN
  orgsize = bswap_32 (orgsize);
#endif
  if (modsize != orgsize)                       // resize file
    {
      if (orgsize < modsize)
        {
          if (ftruncate (fileno (n64aps_modfile), orgsize) != 0)
            fprintf (stderr, "ERROR: Truncate failed\n");
          fflush (n64aps_modfile);
        }
      else
        {
          fseek (n64aps_modfile, 0, SEEK_END);
          for (i = 0; i < (orgsize - modsize); i++)
            fputc (0, n64aps_modfile);
        }
    }
//  fseek (n64aps_modfile, 0, SEEK_SET);
}


static void
readpatch (void)
{
  int APSreadlen, offset;
  unsigned char buffer[N64APS_BUFFERSIZE], size;

  while ((APSreadlen = fread (&offset, 1, 4, n64aps_apsfile)))
    {
#ifdef  WORDS_BIGENDIAN
      offset = bswap_32 (offset);
#endif
      if ((size = fgetc (n64aps_apsfile)))
        {
          fread (buffer, 1, size, n64aps_apsfile);
          if ((fseek (n64aps_modfile, offset, SEEK_SET)) != 0)
            {
              fprintf (stderr, "ERROR: Seek failed\n");
              exit (1);
            }
          fwrite (buffer, 1, size, n64aps_modfile);
        }
      else // apply an RLE block
        {
          unsigned char data, len;
          int i;

          data = fgetc (n64aps_apsfile),
          len = fgetc (n64aps_apsfile);

          if ((fseek (n64aps_modfile, offset, SEEK_SET)) != 0)
            {
              fprintf (stderr, "ERROR: Seek failed\n");
              exit (1);
            }
          for (i = 0; i < len; i++)
            fputc (data, n64aps_modfile);
        }
    }
}


// based on source code (version 1.2 981217) by Silo / BlackBag
int
aps_apply (const char *modname, const char *apsname)
{
  ucon64_file_handler (modname, NULL, 0);

  if ((n64aps_modfile = fopen (modname, "rb+")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], modname);
      return -1;
    }
  if ((n64aps_apsfile = fopen (apsname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], apsname);
      return -1;
    }

  readstdheader ();
  readN64header ();
  readsizeheader (q_fsize (modname));

  readpatch ();

  fclose (n64aps_modfile);
  fclose (n64aps_apsfile);
  printf (ucon64_msg[WROTE], modname);

  return 0;
}


static int
n64caps_checkfile (FILE *file, const char *filename)
{
  unsigned int n64aps_magictest;

  fread (&n64aps_magictest, 4, 1, file);
#ifdef  WORDS_BIGENDIAN
  n64aps_magictest = bswap_32 (n64aps_magictest);
#endif

  if (n64aps_magictest != 0x12408037 && n64aps_magictest != 0x40123780)
    {
      fprintf (stderr, "ERROR: %s is an invalid N64 image\n", filename);
      return FALSE;
    }
  else
    return TRUE;
}


static void
writestdheader (void)
{
  char description[N64APS_DESCRIPTION_LEN];

  fwrite (n64aps_magic, 1, N64APS_MAGICLENGTH, n64aps_apsfile);
  fputc (n64aps_patchtype, n64aps_apsfile);
  fputc (n64aps_encodingmethod, n64aps_apsfile);

  memset (description, ' ', N64APS_DESCRIPTION_LEN);
  fwrite (description, 1, N64APS_DESCRIPTION_LEN, n64aps_apsfile);
}


static void
writeN64header ()
{
  unsigned int n64aps_magictest;
  unsigned char buffer[8], teritory, cartid[2], temp;

  fread (&n64aps_magictest, 4, 1, n64aps_orgfile);
#ifdef  WORDS_BIGENDIAN
  n64aps_magictest = bswap_32 (n64aps_magictest);
#endif

  if (n64aps_magictest == 0x12408037)           // 0 for Doctor format, 1 for everything else
    fputc (0, n64aps_apsfile);
  else
    fputc (1, n64aps_apsfile);

  fseek (n64aps_orgfile, 60, SEEK_SET);
  fread (cartid, 1, 2, n64aps_orgfile);
  if (n64aps_magictest == 0x12408037)
    {
      temp = cartid[0];
      cartid[0] = cartid[1];
      cartid[1] = temp;
    }
  fwrite (cartid, 1, 2, n64aps_apsfile);

  if (n64aps_magictest == 0x12408037)
    fseek (n64aps_orgfile, 63, SEEK_SET);
  else
    fseek (n64aps_orgfile, 62, SEEK_SET);
  teritory = fgetc (n64aps_orgfile);
  fputc (teritory, n64aps_apsfile);

  fseek (n64aps_orgfile, 0x10, SEEK_SET);       // CRC header position
  fread (buffer, 1, 8, n64aps_orgfile);

  if (n64aps_magictest == 0x12408037)
    {
      temp = buffer[0];
      buffer[0] = buffer[1];
      buffer[1] = temp;
      temp = buffer[2];
      buffer[2] = buffer[3];
      buffer[3] = temp;
      temp = buffer[4];
      buffer[4] = buffer[5];
      buffer[5] = temp;
      temp = buffer[6];
      buffer[6] = buffer[7];
      buffer[7] = temp;
    }

  fwrite (buffer, 1, 8, n64aps_apsfile);
  fputc (0, n64aps_apsfile);   // pad
  fputc (0, n64aps_apsfile);
  fputc (0, n64aps_apsfile);
  fputc (0, n64aps_apsfile);
  fputc (0, n64aps_apsfile);

  fseek (n64aps_orgfile, 0, SEEK_SET);
}


static void
writesizeheader (int orgsize, int newsize)
{
  if (orgsize != newsize)
    n64aps_changefound = TRUE;

#ifdef  WORDS_BIGENDIAN
  newsize = bswap_32 (newsize);
#endif
  fwrite (&newsize, 4, 1, n64aps_apsfile);
}


static void
writepatch (void)
// currently RLE is not supported
{
  int orgreadlen, newreadlen, filepos, changedstart = 0, changedoffset = 0,
      i, changedlen = 0, changefound = 0;
  unsigned char orgbuffer[N64APS_BUFFERSIZE], newbuffer[N64APS_BUFFERSIZE];

  fseek (n64aps_orgfile, 0, SEEK_SET);
  fseek (n64aps_modfile, 0, SEEK_SET);

  filepos = 0;
  while ((newreadlen = fread (newbuffer, 1, N64APS_BUFFERSIZE, n64aps_modfile)))
    {
      orgreadlen = fread (orgbuffer, 1, N64APS_BUFFERSIZE, n64aps_orgfile);
      for (i = orgreadlen; i < newreadlen; i++)
        orgbuffer[i] = 0;

      for (i = 0; i < newreadlen; i++)
        {
          if (newbuffer[i] != orgbuffer[i])
            {
              if (!changefound)
                {
                  changedstart = filepos + i;
                  changedoffset = i;
                  changedlen = 0;
                  changefound = TRUE;
                  n64aps_changefound = TRUE;
                }
              changedlen++;
            }
          else if (changefound)
            {
#ifdef  WORDS_BIGENDIAN
              changedstart = bswap_32 (changedstart);
#endif
              fwrite (&changedstart, 4, 1, n64aps_apsfile);
              fputc (changedlen, n64aps_apsfile);
              fwrite (newbuffer + changedoffset, 1, changedlen, n64aps_apsfile);
              changefound = FALSE;
            }
        }

      if (changefound)
        {
#ifdef  WORDS_BIGENDIAN
          changedstart = bswap_32 (changedstart);
#endif
          fwrite (&changedstart, 4, 1, n64aps_apsfile);
          fputc (changedlen, n64aps_apsfile);
          fwrite (newbuffer + changedoffset, 1, changedlen, n64aps_apsfile);
          changefound = FALSE;
        }

      filepos += newreadlen;
    }
}


// based on source code (version 1.2 981217) by Silo / BlackBag
int
aps_create (const char *orgname, const char *modname)
{
  char apsname[FILENAME_MAX];

  if ((n64aps_orgfile = fopen (orgname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], orgname);
      return -1;
    }
  if ((n64aps_modfile = fopen (modname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], modname);
      fclose (n64aps_orgfile);
      return -1;
    }
  strcpy (apsname, orgname);
  set_suffix (apsname, ".APS");
  ucon64_file_handler (apsname, NULL, 0);
  if ((n64aps_apsfile = fopen (apsname, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], apsname);
      fclose (n64aps_modfile);
      fclose (n64aps_orgfile);
      return -1;
    }

  if (!n64caps_checkfile (n64aps_orgfile, orgname) ||
      !n64caps_checkfile (n64aps_modfile, modname))
    {
      fclose (n64aps_orgfile);
      fclose (n64aps_modfile);
      return -1;
    }

  n64aps_changefound = FALSE;

  writestdheader ();
  writeN64header ();
  writesizeheader (q_fsize (orgname), q_fsize (modname));

  printf ("Finding changes...");
  fflush (stdout);
  writepatch ();
  printf (" Done\n");

  fclose (n64aps_modfile);
  fclose (n64aps_orgfile);
  fclose (n64aps_apsfile);

  if (!n64aps_changefound)
    {
      printf ("No changes found\n");
      remove (apsname);
    }
  else
    printf (ucon64_msg[WROTE], apsname);

  return 0;
}


int
aps_set_desc (const char *apsname, const char *description)
{
  char desc[50];

  memset (desc, ' ', 50);
  strncpy (desc, description, strlen (description));
  ucon64_file_handler (apsname, NULL, 0);
  q_fwrite (desc, 7, 50, apsname, "r+b");
  printf (ucon64_msg[WROTE], apsname);

  return 0;
}
