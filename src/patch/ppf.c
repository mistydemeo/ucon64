/*
ppf.c - Playstation Patch Format support for uCON64

written by ???? - ???? Icarus/Paradox
                  2001 NoisyB (noisyb@gmx.net)


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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "misc.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"

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

Be careful! watch the endian format!!! If you own an Amiga and want
to do a PPF2-Patcher for Amiga don't forget to swap the endian-format
of the OFFSET to avoid seek errors!

@END_PPF20PATCH


3. The PPF 2.0 Fileid area

@START_FILEID

The fileid area is used to store additional patch information of
the PPF 2.0 file. I implemented this following the AMIGA standard
of adding a fileid to e.g. .txt files. You can add a FILE_ID to a
PPF 2.0 patch by using the tool 'PPFdiz.exe' or "PPF-O-MATIC2"
included in this package. You dont have to add a FILE_ID to your
PPF 2.0 patch. It only for your pleasure! :)

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
makeppf_main (int argc, char *argv[])
{
#define null 0

  FILE *originalbin;
  FILE *patchedbin;
  FILE *ppffile;
  FILE *fileid = NULL;
  char desc[52];
  char block[1025];
  char fileidbuf[3073];
  char enc = 1;
  char obuf[512];
  char pbuf[512];
  char cbuf[512];
  unsigned char anz;            /* Hell YES! UNSIGNED CHAR! */
  int i, z, a, x, y, osize, psize, fsize, seekpos = 0, pos;


//        printf("MakePPF v2.0 Linux/Unix by Icarus/Paradox\n");
  if (argc == 1 || argc < 4)
    {
//        printf("Usage: MakePPF <Original Bin> <Patched Bin> <ppffile> [file_id.diz]\n");
      exit (0);
    }

  /* Open all neccessary files */
  originalbin = fopen (argv[1], "rb");
  if (originalbin == null)
    {
      printf ("File %s does not exist. (Original BIN)\n", argv[1]);
      exit (0);
    }
  patchedbin = fopen (argv[2], "rb");
  if (patchedbin == null)
    {
      printf ("File %s does not exist. (Patched BIN)\n", argv[2]);
      fclose (originalbin);
      exit (0);
    }
  fseek (originalbin, 0, SEEK_END);
  osize = ftell (originalbin);
  fseek (patchedbin, 0, SEEK_END);
  psize = ftell (patchedbin);
  if (osize != psize)
    {
      printf ("Error: Filesize does not match - byebye..\n");
      fclose (originalbin);
      fclose (patchedbin);
      exit (0);
    }
  if (argc >= 5)
    {
      fileid = fopen (argv[4], "rb");
      if (fileid == null)
        {
          printf ("File %s does not exist. (File_id.diz)\n", argv[4]);
          fclose (patchedbin);
          fclose (originalbin);
          exit (0);
        }
    }
  ppffile = fopen (argv[3], "wb+");
  if (ppffile == null)
    {
      printf ("Could not create file %s\n", argv[3]);
      if (argc >= 5)
        fclose (fileid);
      fclose (patchedbin);
      fclose (originalbin);
      exit (0);
    }

  for (i = 0; i < 50; i++)
    {
      desc[i] = 0x20;
    }

  /* creating PPF2.0 header */
  printf ("Creating PPF2.0 header data.. ");
  fwrite ("PPF20", 5, 1, ppffile);      /* Magic (PPF20) */
  fwrite (&enc, 1, 1, ppffile); /* Enc.Method (0x01) */
  fwrite (desc, 50, 1, ppffile);        /* Description line */
  fwrite (&osize, 4, 1, ppffile);       /* BINfile size */
  fseek (originalbin, 0x9320, SEEK_SET);
  fread (block, 1024, 1, originalbin);
  fwrite (block, 1024, 1, ppffile);     /* 1024 byte block */
  printf ("done!\n");

  printf ("Writing patchdata, please wait.. ");
  fflush (stdout);

  /* Finding changes.. i know it slow.. so feel free to optimize */
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
              anz = 0;
              do
                {
                  cbuf[y] = pbuf[x];
                  anz++;
                  x++;
                  y++;
                }
              while (x != 255 && obuf[x] != pbuf[x]);
              fwrite (&pos, 4, 1, ppffile);
              fwrite (&anz, 1, 1, ppffile);
              fwrite (cbuf, anz, 1, ppffile);
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
              anz = 0;
              do
                {
                  cbuf[y] = pbuf[x];
                  anz++;
                  x++;
                  y++;
                }
              while (x != a && obuf[x] != pbuf[x]);
              fwrite (&pos, 4, 1, ppffile);
              fwrite (&anz, 1, 1, ppffile);
              fwrite (cbuf, anz, 1, ppffile);
            }
          else
            x++;
        }
      while (x != a);
    }
  printf ("done!\n");

  /* was a file_id.diz argument present? */
  if (argc >= 5)
    {
      printf ("Adding file_id.diz .. ");
      fseek (fileid, 0, SEEK_END);
      fsize = ftell (fileid);
      fseek (fileid, 0, SEEK_SET);
      if (fsize > 3072)
        fsize = 3072;           /* File id only up to 3072 bytes! */
      fread (fileidbuf, fsize, 1, fileid);
      fwrite ("@BEGIN_FILE_ID.DIZ", 18, 1, ppffile);    /* Write the shit! */
      fwrite (fileidbuf, fsize, 1, ppffile);
      fwrite ("@END_FILE_ID.DIZ", 16, 1, ppffile);
      fwrite (&fsize, 4, 1, ppffile);
      printf ("done!\n");
    }

  fclose (ppffile);             /* Thats it! */
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
 * it works 100% ! Byebye!
 *
 * Btw feel free to use this in your own projects etc of course!!
 */


int
applyppf_main (int argc, char *argv[])
{
#define null 0

  FILE *binfile;
  FILE *ppffile;
  char buffer[5];
  char method, in;
  char desc[50];
  char diz[3072];
  int dizlen, binlen, dizyn, dizlensave = 0;
  char ppfmem[512];
  int count, seekpos, pos, anz;
  char ppfblock[1025];
  char binblock[1025];


//        printf("ApplyPPF v2.0 for Linux/Unix (c) Icarus/Paradox\n");
  if (argc == 1)
    {
//        printf("Usage: ApplyPPF <Binfile> <PPF-File>\n");
      exit (0);
    }

  /* Open the bin and ppf file */
  binfile = fopen (argv[1], "rb+");
  if (binfile == null)
    {
      printf ("File %s does not exist.\n", argv[1]);
      exit (0);
    }

  ppffile = fopen (argv[2], "rb");
  if (ppffile == null)
    {
      printf ("File %s does not exist.\n", argv[2]);
      exit (0);
    }

  /* Is it a PPF File ? */
  fread (buffer, 3, 1, ppffile);

  if (strncmp ("PPF", buffer, 3))
    {
      printf ("File %s is *NO* PPF file.\n", argv[2]);
      fclose (ppffile);
      fclose (binfile);
      exit (0);
    }

  /* What encoding Method? PPF1.0 or PPF2.0? */
  fseek (ppffile, 5, SEEK_SET);
  fread (&method, 1, 1, ppffile);

  switch (method)
    {
    case 0:
      /* Show PPF-Patchinformation. */
      /* This is a PPF 1.0 Patch! */
      fseek (ppffile, 6, SEEK_SET);     /* Read Desc.line */
      fread (desc, 50, 1, ppffile);
      printf ("\nFilename       : %s\n", argv[2]);
      printf ("Enc. Method    : %d (PPF1.0)\n", method);
      printf ("Description    : %s\n", desc);
      printf ("File_id.diz    : no\n\n");

      /* Calculate the count for patching the image later */
      /* Easy calculation on a PPF1.0 Patch! */
      fseek (ppffile, 0, SEEK_END);
      count = ftell (ppffile);
      count -= 56;
      seekpos = 56;
      printf ("Patching ... ");
      break;
    case 1:
      /* Show PPF-Patchinformation. */
      /* This is a PPF 2.0 Patch! */
      fseek (ppffile, 6, SEEK_SET);
      fread (desc, 50, 1, ppffile);
      printf ("\nFilename       : %s\n", argv[2]);
      printf ("Enc. Method    : %d (PPF2.0)\n", method);
      printf ("Description    : %s\n", desc);

      fseek (ppffile, -8, SEEK_END);
      fread (buffer, 4, 1, ppffile);

      /* Is there a File id ?! */
      if (strncmp (".DIZ", buffer, 4))
        {
          printf ("File_id.diz    : no\n\n");
          dizyn = 0;
        }
      else
        {
          printf ("File_id.diz    : yes, showing...\n");
          fread (&dizlen, 4, 1, ppffile);
          fseek (ppffile, -dizlen - 20, SEEK_END);
          fread (diz, dizlen, 1, ppffile);
          diz[dizlen - 7] = '\0';
          printf ("%s\n", diz);
          dizyn = 1;
          dizlensave = dizlen;
        }
      /* Do the BINfile size check! */
      fseek (ppffile, 56, SEEK_SET);
      fread (&dizlen, 4, 1, ppffile);
      fseek (binfile, 0, SEEK_END);
      binlen = ftell (binfile);
      if (dizlen != binlen)
        {
          printf ("ERROR: the size of the IMAGE is not %d Bytes\n", dizlen);
          fclose (ppffile);
          fclose (binfile);
          return -1;
        }

      /* do the Binaryblock check! this check is 100% secure! */
      fseek (ppffile, 60, SEEK_SET);
      fread (ppfblock, 1024, 1, ppffile);
      fseek (binfile, 0x9320, SEEK_SET);
      fread (binblock, 1024, 1, binfile);
      in = memcmp (ppfblock, binblock, 1024);
      if (in != 0)
        {
          printf ("ERROR: this patch does not belong to this IMAGE\n");
          fclose (ppffile);
          fclose (binfile);
          return -1;
        }

      /* Calculate the count for patching the image later */
      fseek (ppffile, 0, SEEK_END);
      count = ftell (ppffile);

      if (dizyn == 0)
        {
          count -= 1084;
          seekpos = 1084;
        }
      else
        {
          count -= 1084;
          count -= 38;
          count -= dizlensave;
          seekpos = 1084;
        }
      printf ("Patching ... ");
      fflush (stdout);
      break;
    default:

      /* Enc. Method wasnt 0 or 1 i bet you wont see this */
      printf ("Unknown Encodingmethod! - check for updates.\n");
      fclose (ppffile);
      fclose (binfile);
      exit (0);

    }

  /* Patch the Image */

  do
    {
      fseek (ppffile, seekpos, SEEK_SET);       /* seek to patchdataentry */
      fread (&pos, 4, 1, ppffile);      /* Get POS for binfile */

      fread (&anz, 1, 1, ppffile);      /* How many byte do we have to write? */
      fread (ppfmem, anz, 1, ppffile);  /* And this is WHAT we have to write */
      fseek (binfile, pos, SEEK_SET);   /* Go to the right position in the BINfile */
      fwrite (ppfmem, anz, 1, binfile); /* write 'anz' bytes to that pos from our ppfmem */
      seekpos = seekpos + 5 + anz;      /* calculate next patchentry! */
      count = count - 5 - anz;  /* have we reached the end of the PPFfile?? */
    }
  while (count != 0);           /* if not -> LOOOOOP! */

  printf ("DONE..\n");          /* byebye :) */
  fclose (ppffile);
  fclose (binfile);
  return 0;
}

#include "ucon64.h"
int
addppfid (int argc, char *argv[])
{
  long fsize;
  long pos = 0;

  char filename[4096];
  char fileidbuf[3072];
  char buf[4095];

  strcpy (filename, getarg (argc, argv, ucon64_ROM));

  printf ("Adding file_id.diz .. ");
  fsize = quickftell (filename);
  if (fsize > 3072)
    fsize = 3072;               /* File id only up to 3072 bytes! */
  quickfread (fileidbuf, 0, fsize, getarg (argc, argv, ucon64_FILE));
  fileidbuf[fsize] = 0;
  sprintf (buf, "@BEGIN_FILE_ID.DIZ%s@END_FILE_ID.DIZ", fileidbuf);

  pos =
    filencmp (filename, 0, quickftell (filename), "@BEGIN_FILE_ID.DIZ", 18);
  if (pos == -1)
    pos = quickftell (filename);
  truncate (filename, pos);

  quickfwrite (buf, pos, strlen (buf), filename, "r+b");

  quickfwrite (&fsize, quickftell (filename), 4, filename, "r+b");
  printf ("done!\n");
  return 0;
}


void
ppf_usage (void)
{

  printf
    ("  " OPTION_LONG_S "ppf         apply PPF patch (<=2.0); $ROM=RAW_IMAGE $FILE=PATCHFILE\n"
     "  " OPTION_LONG_S "mkppf       create PPF patch; $ROM=RAW_IMAGE $FILE=CHANGED_IMAGE\n"
     "  " OPTION_LONG_S "nppf        change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION\n"
     "  " OPTION_LONG_S "idppf       change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ\n"
);
}
