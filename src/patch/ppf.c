/*
ppf.c - Playstation Patch Format support for uCON64

written by ???? - ???? Icarus/Paradox
                  2001 NoisyB (noisyb@gmx.net)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"


const char *ppf_usage[] =
{
    NULL,
    NULL,
    "  " OPTION_LONG_S "ppf=PATCH   apply PPF PATCH against IMAGE (PPF version <= 2.0); " OPTION_LONG_S "rom=IMAGE\n"
    "  " OPTION_LONG_S "mkppf=IMAGE create PPF patch; " OPTION_LONG_S "rom=ORIGINAL_IMAGE\n"
    "  " OPTION_LONG_S "nppf=DESC   change PPF single line DESCription; " OPTION_LONG_S "rom=PATCH\n"
    "  " OPTION_LONG_S "idppf=FILE_ID.DIZ change FILE_ID.DIZ of PPF PATCH (PPF v2.0); " OPTION_LONG_S "rom=PATCH\n",
    NULL
};

/*

.-----------------------------------------------------------------.
| PLAYSTATION PATCH FILE VERSION 2.0 FILE-STRUCTURE FOR DEVELOPERS|
'-----------------------------------------------------------------'

1. The PPF 2.0 Header:

@START_PPF20HEADER
.----------+--------+---------------------------------------------.
| POSITION |  SIZE  |              E X P L A N A T I O N          |
+----------|--------|---------------------------------------------+
| 00-04    |   05   | PPF-Magic: "PPF20"                          |
+----------|--------|---------------------------------------------+
| 05       |   01   | Encoding Method:                            |
|          |        | - If $00 then it is a PPF 1.0 Patch         |
|          |        | - If $01 then it is a PPF 2.0 Patch         |
+----------|--------|---------------------------------------------+
| 06-55    |   50   | Patch Description                           |
+----------|--------|---------------------------------------------+
| 56-59    |   04   | Size of the file (e.g. CDRWin binfile) this |
|          |        | patch was made of. Used for Identification  |
+----------|--------|---------------------------------------------+
| 60-1083  | 1024   | this is a binary block of 1024 byte taken   |
|          |        | from position $9320 of the file (e.g. CDRWin|
|          |        | binfile) this patch was made of. Used for   |
|          |        | identification.                             |
+----------|--------|---------------------------------------------+
| 1084-X   |   XX   | The Patch itself.. see below for structure! |
'----------+--------+---------------------------------------------'
@END_PPF20HEADER - TOTAL HEADER-SIZE = 1084 BYTE.


2. The PPF 2.0 Patch Itself (Encoding Method #1)

@START_PPF20PATCH
FORMAT : xxxx,y,zzzz

         xxxx   = 4 byte file offset.

         y      = Number of bytes that will be changed.

         zzzz   = New data to be written ('y' number of bytes).

Example
~~~~~~~

Starting from File Offset 0x0015F9D0 replace 3 bytes with 01,02,03
D0 F9 15 00 03 01 02 03

Be careful! Watch the endian format! If you own an Amiga and want
to do a PPF2-Patcher for Amiga don't forget to swap the endian-format
of the OFFSET to avoid seek errors!

@END_PPF20PATCH


3. The PPF 2.0 Fileid area

@START_FILEID

The fileid area is used to store additional patch information of
the PPF 2.0 file. I implemented this following the AMIGA standard
of adding a fileid to e.g. .txt files. You can add a FILE_ID to a
PPF 2.0 patch by using the tool 'PPFdiz.exe' or "PPF-O-MATIC2"
included in this package. You don't have to add a FILE_ID to your
PPF 2.0 patch. It's only for your pleasure! :)

For developers: a file_id area begins with @BEGIN_FILE_ID.DIZ and
ends with @END_FILE_ID.DIZ (Amiga BBS standard).
Between @BEGIN_FILE_ID.DIZ and @END_FILE_ID.DIZ you will find
the File_Id and followed after @END_FILE_ID.DIZ you will find an
Integer (4 byte long) with the length of the FILE_ID.DIZ!

A File_ID.diz file cannot be greater than 3072 Bytes.

If you do a PPF 2.0 Applier be sure to check for an existing FILE
ID AREA, because it is located after the PATCH DATA!

@END_FILEID
*/

/*
 * MakePPF v2.0 Sourcecode by Icarus/Paradox
 * enter "gcc makeppf.c" on linux/unix to compile!
 *
 * Feel free to optimize speed or something!
 *
 */
int
makeppf_main (int argc, const char *argv[])
{
  FILE *originalbin, *patchedbin, *ppffile, *fileid = NULL;
  char desc[52], block[1025], fileidbuf[3073], enc = 1,
       obuf[512], pbuf[512], cbuf[512];
  unsigned char n_changes;
  int i, z, a, x, y, osize, psize, fsize, seekpos = 0, pos;

//  printf("MakePPF v2.0 Linux/Unix by Icarus/Paradox\n");
  if (argc == 1 || argc < 4)
    {
//      printf("Usage: MakePPF <Original Bin> <Patched Bin> <ppffile> [file_id.diz]\n");
      return -1;
    }

  // Open all necessary files
  originalbin = fopen (argv[1], "rb");
  if (originalbin == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], argv[1]);
      return -1;
    }
  patchedbin = fopen (argv[2], "rb");
  if (patchedbin == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], argv[2]);
      fclose (originalbin);
      return -1;
    }
  osize = q_fsize (argv[1]);
  psize = q_fsize (argv[2]);
  if (osize != psize)
    {
      fprintf (stderr, "ERROR: Filesize does not match\n");
      fclose (originalbin);
      fclose (patchedbin);
      return -1;
    }
  if (argc >= 5)
    {
      fileid = fopen (argv[4], "rb");
      if (fileid == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], argv[4]);
          fclose (patchedbin);
          fclose (originalbin);
          return -1;
        }
    }
  ppffile = fopen (argv[3], "wb+");
  if (ppffile == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], argv[3]);
      if (argc >= 5)
        fclose (fileid);
      fclose (patchedbin);
      fclose (originalbin);
      return -1;
    }

  for (i = 0; i < 50; i++)
    desc[i] = 0x20;

  // creating PPF2.0 header
  printf ("Creating PPF2.0 header data...\n");
  fwrite ("PPF20", 5, 1, ppffile);              // Magic (PPF20)
  fwrite (&enc, 1, 1, ppffile);                 // Enc.Method (0x01)
  fwrite (desc, 50, 1, ppffile);                // Description line
#ifdef  WORDS_BIGENDIAN
  i = bswap_32 (osize);
  fwrite (&i, 4, 1, ppffile);
#else
  fwrite (&osize, 4, 1, ppffile);               // BINfile size
#endif
  fseek (originalbin, 0x9320, SEEK_SET);
  fread (block, 1024, 1, originalbin);
  fwrite (block, 1024, 1, ppffile);             // 1024 byte block
  printf ("Done\n");

  printf ("Writing patch data, please wait...\n");
  // Finding changes
  z = (osize / 255);
  a = osize - (z * 255);
  do
    {
      fseek (originalbin, seekpos, SEEK_SET);
      fseek (patchedbin, seekpos, SEEK_SET);
      fread (obuf, 255, 1, originalbin);
      fread (pbuf, 255, 1, patchedbin);
      x = 0;
      pos = 0;
      do
        {
          if (obuf[x] != pbuf[x])
            {
              pos = seekpos + x;
              y = 0;
              n_changes = 0;
              do
                {
                  cbuf[y] = pbuf[x];
                  n_changes++;
                  x++;
                  y++;
                }
              while (x != 255 && obuf[x] != pbuf[x]);
#ifdef  WORDS_BIGENDIAN
              pos = bswap_32 (pos);
#endif
              fwrite (&pos, 4, 1, ppffile);
              fwrite (&n_changes, 1, 1, ppffile);
              fwrite (cbuf, n_changes, 1, ppffile);
            }
          else
            x++;
        }
      while (x != 255);

      seekpos += 255;
      z--;
    }
  while (z != 0);

  if (a != 0)
    {
      fseek (originalbin, seekpos, SEEK_SET);
      fseek (patchedbin, seekpos, SEEK_SET);
      fread (obuf, 255, 1, originalbin);
      fread (pbuf, 255, 1, patchedbin);
      x = 0;
      pos = 0;
      do
        {
          if (obuf[x] != pbuf[x])
            {
              pos = seekpos + x;
              y = 0;
              n_changes = 0;
              do
                {
                  cbuf[y] = pbuf[x];
                  n_changes++;
                  x++;
                  y++;
                }
              while (x != a && obuf[x] != pbuf[x]);
#ifdef  WORDS_BIGENDIAN
              pos = bswap_32 (pos);
#endif
              fwrite (&pos, 4, 1, ppffile);
              fwrite (&n_changes, 1, 1, ppffile);
              fwrite (cbuf, n_changes, 1, ppffile);
            }
          else
            x++;
        }
      while (x != a);
    }
  printf ("Done\n");

  // Was a file_id.diz argument present?
  if (argc >= 5)
    {
      printf ("Adding file_id.diz...\n");
      fsize = q_fsize (argv[4]);
      if (fsize > 3072)
        fsize = 3072;                           // File id only up to 3072 bytes!
      fread (fileidbuf, fsize, 1, fileid);
      fwrite ("@BEGIN_FILE_ID.DIZ", 18, 1, ppffile);
      fwrite (fileidbuf, fsize, 1, ppffile);
      fwrite ("@END_FILE_ID.DIZ", 16, 1, ppffile);
#ifdef  WORDS_BIGENDIAN
      fsize = bswap_32 (fsize);                 // Write filesize in little-endian format
#endif
      fwrite (&fsize, 4, 1, ppffile);
      printf ("Done\n");
    }

  fclose (ppffile);                             // Thats it!
  fclose (originalbin);
  fclose (patchedbin);
  return 0;
}


/*
 * ApplyPPF v2.0 for Linux/Unix. Coded by Icarus/Paradox 2k
 * If you want to compile applyppf just enter "gcc applyppf.c"
 * that's it but i think the Linux users know :)
 *
 * This one applies both, PPF1.0 and PPF2.0 patches.
 *
 * Sorry for the bad code i had no time for some cleanup.. but
 * it works 100%! Byebye!
 *
 * Btw feel free to use this in your own projects etc of course!
 */
int
applyppf_main (int argc, const char *argv[])
{
  FILE *binfile, *ppffile;
  char buffer[5], method, in, desc[50], diz[3072],
       ppfmem[512], ppfblock[1025], binblock[1025];
  unsigned char n_changes;
  int len, dizlen = 0, binlen, count, seekpos, pos, ppfsize;

//        printf("ApplyPPF v2.0 for Linux/Unix (c) Icarus/Paradox\n");
  if (argc == 1)
    {
#if 0
      printf("Usage: ApplyPPF <Binfile> <PPF-File>\n");
#endif
      return -1;
    }

  // Open the bin and ppf file
  binfile = fopen (argv[1], "rb+");
  if (binfile == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], argv[1]);
      return -1;
    }

  ppffile = fopen (argv[2], "rb");
  if (ppffile == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], argv[2]);
      return -1;
    }

  // Is it a PPF File?
  fread (buffer, 3, 1, ppffile);
  if (strncmp ("PPF", buffer, 3))
    {
      fprintf (stderr, "ERROR: %s is not a valid PPF file\n", argv[2]);
      fclose (ppffile);
      fclose (binfile);
      return -1;
    }

  ppfsize = q_fsize (argv[2]);

  // What encoding Method? PPF1.0 or PPF2.0?
  fseek (ppffile, 5, SEEK_SET);
  fread (&method, 1, 1, ppffile);

  switch (method)
    {
    case 0:
      // Show PPF-Patchinformation.
      // This is a PPF 1.0 Patch!
      fseek (ppffile, 6, SEEK_SET);             // Read Desc.line
      fread (desc, 50, 1, ppffile);
      printf ("\nFilename       : %s\n", argv[2]);
      printf ("Enc. Method    : %d (PPF1.0)\n", method);
      printf ("Description    : %s\n", desc);
      printf ("File_id.diz    : no\n\n");

      // Calculate the count for patching the image later
      // Easy calculation on a PPF1.0 Patch!
      count = ppfsize;
      count -= 56;
      seekpos = 56;
      printf ("Patching...\n");
      break;
    case 1:
      // Show PPF-Patchinformation.
      // This is a PPF 2.0 Patch!
      fseek (ppffile, 6, SEEK_SET);
      fread (desc, 50, 1, ppffile);
      printf ("\nFilename       : %s\n", argv[2]);
      printf ("Enc. Method    : %d (PPF2.0)\n", method);
      printf ("Description    : %s\n", desc);

      fseek (ppffile, ppfsize - 8, SEEK_SET);
      fread (buffer, 4, 1, ppffile);

      // Is there a File id?
      if (strncmp (".DIZ", buffer, 4))
        printf ("File_id.diz    : no\n\n");
      else
        {
          printf ("File_id.diz    : yes, showing...\n");
          fread (&dizlen, 4, 1, ppffile);
#ifdef  WORDS_BIGENDIAN
          dizlen = bswap_32 (dizlen);           // file_id.diz size is in little-endian format
#endif
          fseek (ppffile, ppfsize - dizlen - 20, SEEK_SET);
          fread (diz, dizlen, 1, ppffile);
          diz[dizlen - 7] = '\0';
          printf ("%s\n", diz);
        }
      // Do the file size check
      fseek (ppffile, 56, SEEK_SET);
      fread (&len, 4, 1, ppffile);
#ifdef  WORDS_BIGENDIAN
      len = bswap_32 (len);                     // filesize is stored in little-endian format
#endif
      binlen = q_fsize (argv[1]);
      if (len != binlen)
        {
          fprintf (stderr, "ERROR: The size of the image is not %d Bytes\n", len);
          fclose (ppffile);
          fclose (binfile);
          return -1;
        }

      // Do the binary block check
      fseek (ppffile, 60, SEEK_SET);
      fread (ppfblock, 1024, 1, ppffile);
      fseek (binfile, 0x9320, SEEK_SET);
      fread (binblock, 1024, 1, binfile);
      in = memcmp (ppfblock, binblock, 1024);
      if (in != 0)
        {
          fprintf (stderr, "ERROR: This patch does not belong to this image\n");
          fclose (ppffile);
          fclose (binfile);
          return -1;
        }

      // Calculate the count for patching the image later
      count = ppfsize;
      if (dizlen == 0)
        {
          count -= 1084;
          seekpos = 1084;
        }
      else
        {
          count -= 1084;
          count -= 34 + 4;
          count -= dizlen;
          seekpos = 1084;
        }
      printf ("Patching...\n");
      break;
    default:
      // Enc. Method wasn't 0 or 1
      fprintf (stderr, "ERROR: Unknown encoding method! Check for updates\n");
      fclose (ppffile);
      fclose (binfile);
      return -1;
    }

  // Patch the image
  do
    {
      fseek (ppffile, seekpos, SEEK_SET);       // Seek to patch data entry
      fread (&pos, 4, 1, ppffile);              // Get POS for binfile
#ifdef  WORDS_BIGENDIAN
      pos = bswap_32 (pos);
#endif

      fread (&n_changes, 1, 1, ppffile);        // How many byte do we have to write?
      fread (ppfmem, n_changes, 1, ppffile);    // And this is WHAT we have to write
      fseek (binfile, pos, SEEK_SET);           // Go to the right position in the BINfile
      fwrite (ppfmem, n_changes, 1, binfile);   // Write n_changes bytes to that pos from our ppfmem
      seekpos = seekpos + 5 + n_changes;        // Calculate next patch entry!
      count = count - 5 - n_changes;            // Have we reached the end of the PPFfile?
    }
  while (count != 0);                           // if not -> loop

  printf ("Done\n");
  fclose (ppffile);
  fclose (binfile);
  return 0;
}


int
ppf_add_fid (const char *filename, const char *fid)
{
  long fsize, pos = 0;
  char fileidbuf[3072], buf[3072 + 34 + 1];

  printf ("Adding file_id.diz...\n");
  fsize = q_fsize (fid);
  q_fread (fileidbuf, 0, (fsize > 3071) ? 3071 : fsize, fid);
  fileidbuf[(fsize > 3071) ? 3071 : fsize] = 0;
  sprintf (buf, "@BEGIN_FILE_ID.DIZ%s@END_FILE_ID.DIZ", fileidbuf);

  pos = q_fncmp (filename, 0, fsize, "@BEGIN_FILE_ID.DIZ", 18, -1);
  if (pos == -1)
    pos = fsize;
  truncate (filename, pos);

  q_fwrite (buf, pos, strlen (buf), filename, "r+b");
#ifdef  WORDS_BIGENDIAN
  fsize = bswap_32 (fsize);                     // Write filesize in little-endian format
#endif
  q_fwrite (&fsize, fsize, 4, filename, "r+b");
  printf ("Done\n");
  return 0;
}
