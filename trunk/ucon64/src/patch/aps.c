/*
aps.c - APS support for uCON64

written by 1998 Silo / BlackBag
	   1999 - 2001 NoisyB (noisyb@gmx.net)

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
#include "../ucon64.h"

/* Apply an APS (Advanced Patch System) File for N64 Images
 * (C)1998 Silo / BlackBag
 *
 * Version 1.2 981217
 */

#ifndef n64aps_TRUE
#define n64aps_TRUE  1
#define n64aps_FALSE !n64aps_TRUE
#endif

#define n64aps_MESSAGE   "\nN64APS v1.2 (BETA) Build 981217\n"
#define n64aps_COPYRIGHT "(C)1998 Silo/BlackBag (Silo@BlackBag.org)\n\n"

#define n64aps_BUFFERSIZE 255

char n64aps_Magic[] = "APS10";
#define n64aps_MagicLength 5

#define n64aps_TYPE_N64 1
unsigned char n64aps_PatchType = n64aps_TYPE_N64;

#define n64aps_DESCRIPTION_LEN 50

#define n64aps_ENCODINGMETHOD 0 // Very Simplistic Method

unsigned char n64aps_EncodingMethod = n64aps_ENCODINGMETHOD;

FILE *n64aps_APSFile;
FILE *n64aps_ORGFile;
FILE *n64aps_NEWFile;
int n64aps_Quiet = n64aps_FALSE;

void
n64aps_syntax (void)
{
/*	printf ("%s", n64aps_MESSAGE);
	printf ("%s", n64aps_COPYRIGHT);
	printf ("N64APS <options> <Original File> <APS File>\n");
    printf (" -f                 : Force Patching Over Incorrect Image\n");
	printf (" -q                 : Quiet Mode\n");
*/ fflush (stdout);
}

int
n64aps_CheckFile (char *Filename, char *mode)
{
  FILE *fp;

  fp = fopen (Filename, mode);
  if (fp == NULL)
    return (n64aps_FALSE);
  else
    {
      fclose (fp);
      if (mode[0] == 'w')
        unlink (Filename);
      return (n64aps_TRUE);
    }
}

void
ReadStdHeader ()
{
  char an64aps_Magic[n64aps_MagicLength];
  char Description[n64aps_DESCRIPTION_LEN + 1];

  fread (an64aps_Magic, 1, n64aps_MagicLength, n64aps_APSFile);
  if (strncmp (an64aps_Magic, n64aps_Magic, n64aps_MagicLength) != 0)
    {
      printf ("Not a Valid Patch File\n");
      fclose (n64aps_ORGFile);
      fclose (n64aps_APSFile);
      exit (1);
    }
  fread (&n64aps_PatchType, sizeof (n64aps_PatchType), 1, n64aps_APSFile);
  if (n64aps_PatchType != 1)    // N64 Patch
    {
      printf ("Unable to Process Patch File\n");
      fclose (n64aps_ORGFile);
      fclose (n64aps_APSFile);
      exit (1);
    }
  fread (&n64aps_EncodingMethod, sizeof (n64aps_EncodingMethod), 1,
         n64aps_APSFile);
  if (n64aps_EncodingMethod != 0)       // Simple Encoding
    {
      printf ("Unknown or New Encoding Method\n");
      fclose (n64aps_ORGFile);
      fclose (n64aps_APSFile);
      exit (1);
    }

  fread (Description, 1, n64aps_DESCRIPTION_LEN, n64aps_APSFile);
  Description[n64aps_DESCRIPTION_LEN] = 0;
  if (!n64aps_Quiet)
    {
      printf ("Description : %s\n", Description);
      fflush (stdout);
    }
}

void
ReadN64Header (int Force)
{
  unsigned long n64aps_MagicTest;
  unsigned char Buffer[8];
  unsigned char APSBuffer[8];
  int c;
  unsigned char CartID[2], Temp;
  unsigned char Teritory, APSTeritory;
  unsigned char APSFormat;


  fseek (n64aps_ORGFile, 0, SEEK_SET);
  fread (&n64aps_MagicTest, sizeof (n64aps_MagicTest), 1, n64aps_ORGFile);
  fread (Buffer, 1, 1, n64aps_APSFile);
  APSFormat = Buffer[0];

  if (((n64aps_MagicTest == 0x12408037) && (Buffer[0] == 1)) || ((n64aps_MagicTest != 0x12408037 && (Buffer[0] == 0)))) // 0 for Doctor Format, 1 for Everything Else
    {
      printf ("Image is in the wrong format\n");
      fclose (n64aps_ORGFile);
      fclose (n64aps_APSFile);
      exit (1);
    }

  fseek (n64aps_ORGFile, 60, SEEK_SET); // Cart ID
  fread (CartID, 1, 2, n64aps_ORGFile);
  fread (Buffer, 1, 2, n64aps_APSFile);
  if (n64aps_MagicTest == 0x12408037)   // Doc
    {
      Temp = CartID[0];
      CartID[0] = CartID[1];
      CartID[1] = Temp;
    }
  if ((Buffer[0] != CartID[0]) || (Buffer[1] != CartID[1]))
    {
      printf ("Not the Same Image\n");
      fclose (n64aps_ORGFile);
      fclose (n64aps_APSFile);
      exit (1);
    }

  if (n64aps_MagicTest == 0x12408037)
    fseek (n64aps_ORGFile, 63, SEEK_SET);       // Teritory
  else
    fseek (n64aps_ORGFile, 62, SEEK_SET);

  fread (&Teritory, sizeof (Teritory), 1, n64aps_ORGFile);
  fread (&APSTeritory, sizeof (APSTeritory), 1, n64aps_APSFile);
  if (Teritory != APSTeritory)
    {
      printf ("Wrong Country\n");
      if (!Force)
        {
          fclose (n64aps_ORGFile);
          fclose (n64aps_APSFile);
          exit (1);
        }
    }

  fseek (n64aps_ORGFile, 0x10, SEEK_SET);       // CRC Header Position
  fread (Buffer, 1, 8, n64aps_ORGFile);
  fread (APSBuffer, 1, 8, n64aps_APSFile);

  if (n64aps_MagicTest == 0x12408037)   // Doc
    {
      Temp = Buffer[0];
      Buffer[0] = Buffer[1];
      Buffer[1] = Temp;
      Temp = Buffer[2];
      Buffer[2] = Buffer[3];
      Buffer[3] = Temp;
      Temp = Buffer[4];
      Buffer[4] = Buffer[5];
      Buffer[5] = Temp;
      Temp = Buffer[6];
      Buffer[6] = Buffer[7];
      Buffer[7] = Temp;
    }

  if ((APSBuffer[0] != Buffer[0]) || (APSBuffer[1] != Buffer[1]) ||
      (APSBuffer[2] != Buffer[2]) || (APSBuffer[3] != Buffer[3]) ||
      (APSBuffer[4] != Buffer[4]) || (APSBuffer[5] != Buffer[5]) ||
      (APSBuffer[6] != Buffer[6]) || (APSBuffer[7] != Buffer[7]))
    {
      if (!n64aps_Quiet)
        {
          printf ("Incorrect Image\n");
          fflush (stdout);
        }
      if (!Force)
        {
          fclose (n64aps_ORGFile);
          fclose (n64aps_APSFile);
          exit (1);
        }
    }

  fseek (n64aps_ORGFile, 0, SEEK_SET);

  c = fgetc (n64aps_APSFile);
  c = fgetc (n64aps_APSFile);
  c = fgetc (n64aps_APSFile);
  c = fgetc (n64aps_APSFile);
  c = fgetc (n64aps_APSFile);
}


void
ReadSizeHeader (char *File1)
{
  long OrigSize;
  long APSOrigSize;
  unsigned char t;
  long i;

  fseek (n64aps_ORGFile, 0, SEEK_END);

  OrigSize = ftell (n64aps_ORGFile);

  fread (&APSOrigSize, sizeof (APSOrigSize), 1, n64aps_APSFile);

  if (OrigSize != APSOrigSize)  // Do File Resize
    {
      if (APSOrigSize < OrigSize)
        {
          int x;
          fclose (n64aps_ORGFile);
          x = open (File1, O_WRONLY);
          if (ftruncate (x, APSOrigSize) != 0)
            {
              printf ("Trunacte Failed\n");
            }
          close (x);
          n64aps_ORGFile = fopen (File1, "rb+");
        }
      else
        {
          t = 0;
          for (i = 0; i < (APSOrigSize - OrigSize); i++)
            fputc (t, n64aps_ORGFile);
        }
    }

  fseek (n64aps_ORGFile, 0, SEEK_SET);
}


void
ReadPatch ()
{
  int APSReadLen;
  int Finished = n64aps_FALSE;
  unsigned char Buffer[256];
  long Offset;
  unsigned char Size;

  while (!Finished)
    {
      APSReadLen = fread (&Offset, sizeof (Offset), 1, n64aps_APSFile);
      if (APSReadLen == 0)
        {
          Finished = n64aps_TRUE;
        }
      else
        {
          fread (&Size, sizeof (Size), 1, n64aps_APSFile);
          if (Size != 0)
            {
              fread (Buffer, 1, Size, n64aps_APSFile);
              if ((fseek (n64aps_ORGFile, Offset, SEEK_SET)) != 0)
                {
                  printf ("Seek Failed\n");
                  fflush (stdout);
                  exit (1);
                }
              fwrite (Buffer, 1, Size, n64aps_ORGFile);
            }
          else
            {
              unsigned char data;
              unsigned char len;
              int i;

              fread (&data, sizeof (data), 1, n64aps_APSFile);
              fread (&len, sizeof (data), 1, n64aps_APSFile);

              if ((fseek (n64aps_ORGFile, Offset, SEEK_SET)) != 0)
                {
                  printf ("Seek Failed\n");
                  fflush (stdout);
                  exit (1);
                }

              for (i = 0; i < len; i++)
                fputc (data, n64aps_ORGFile);
            }
        }
    }
}


int
n64aps_main (int argc, char *argv[])
{
  char File1[256];
  char File2[256];
  int Force = n64aps_TRUE;


  strcpy (File1, argv[2]);
  strcpy (File2, argv[3]);

  if (!n64aps_CheckFile (File1, "rb+"))
    return (1);
  if (!n64aps_CheckFile (File2, "rb"))
    return (1);

  n64aps_ORGFile = fopen (File1, "rb+");
  n64aps_APSFile = fopen (File2, "rb");

  if (!n64aps_Quiet)
    {
//              printf ("%s", n64aps_MESSAGE);
//              printf ("%s", n64aps_COPYRIGHT);
      fflush (stdout);
    }

  ReadStdHeader ();

  ReadN64Header (Force);
  ReadSizeHeader (File1);

  ReadPatch ();

//      fclose (n64aps_NEWFile);
  fclose (n64aps_ORGFile);
  fclose (n64aps_APSFile);

  return (0);
}

/* Create APS (Advanced Patch System) for N64 Images
 * (C)1998 Silo / BlackBag
 *
 * Version 1.2 981217
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef n64caps_TRUE
#define n64caps_TRUE  1
#define n64caps_FALSE !n64caps_TRUE
#endif

#define n64caps_MESSAGE   "\nN64CAPS v1.2 (BETA) Build 981217\n"
#define n64caps_COPYRIGHT "(C)1998 Silo/BlackBag (Silo@BlackBag.org)\n\n"

#define n64caps_BUFFERSIZE 255

char n64caps_Magic[] = "APS10";
#define n64caps_MagicLength 5

#define n64caps_TYPE_N64 1
unsigned char n64caps_PatchType = n64caps_TYPE_N64;

#define n64caps_DESCRIPTION_LEN 50

#define n64caps_ENCODINGMETHOD 0        // Very Simplistic Method

unsigned char n64caps_EncodingMethod = n64caps_ENCODINGMETHOD;

FILE *n64caps_APSFile;
FILE *n64caps_ORGFile;
FILE *n64caps_NEWFile;
int n64caps_Quiet = n64caps_FALSE;
int n64caps_ChangeFound;

void
n64caps_syntax (void)
{
/*	printf ("%s", n64caps_MESSAGE);
	printf ("%s", n64caps_COPYRIGHT);
	printf ("N64CAPS <options> <Original File> <Modified File> <Output APS File>\n");
	printf (" -d %c<Image Title>%c : Description\n", 34, 34);
	printf (" -q                 : Quiet Mode\n");
*/ fflush (stdout);
}

int
n64caps_CheckFile (char *Filename, char *mode, int Image)
{
  FILE *fp;
  unsigned long n64caps_MagicTest;

  fp = fopen (Filename, mode);
  if (fp == NULL)
    return (n64caps_FALSE);
  else
    {
      if (Image)
        {
          fread (&n64caps_MagicTest, sizeof (n64caps_MagicTest), 1, fp);

          fclose (fp);
          if (mode[0] == 'w')
            unlink (Filename);

          if (n64caps_MagicTest != 0x12408037 && n64caps_MagicTest != 0x40123780)       // Invalid Image
            {
              if (!n64caps_Quiet)
                {
                  printf ("%s is an Invalid Image\n", Filename);
                  fflush (stdout);
                }
              return (n64caps_FALSE);
            }
          return (n64caps_TRUE);
        }
    }
  return (n64caps_TRUE);
}

void
WriteStdHeader (char *Desc)
{
  char Description[n64caps_DESCRIPTION_LEN];

  fwrite (n64caps_Magic, 1, n64caps_MagicLength, n64caps_APSFile);
  fwrite (&n64caps_PatchType, sizeof (n64caps_PatchType), 1, n64caps_APSFile);
  fwrite (&n64caps_EncodingMethod, sizeof (n64caps_EncodingMethod), 1,
          n64caps_APSFile);

  memset (Description, ' ', n64caps_DESCRIPTION_LEN);
  strcpy (Description, Desc);
  Description[strlen (Desc)] = ' ';

  fwrite (Description, 1, n64caps_DESCRIPTION_LEN, n64caps_APSFile);

}

void
WriteN64Header ()
{
  unsigned long n64caps_MagicTest;
  unsigned char Buffer[8];
  unsigned char Teritory;
  unsigned char CartID[2], Temp;

  fread (&n64caps_MagicTest, sizeof (n64caps_MagicTest), 1, n64caps_ORGFile);

  if (n64caps_MagicTest == 0x12408037)  // 0 for Doctor Format, 1 for Everything Else
    fputc (0, n64caps_APSFile);
  else
    fputc (1, n64caps_APSFile);


  fseek (n64caps_ORGFile, 60, SEEK_SET);
  fread (CartID, 1, 2, n64caps_ORGFile);
  if (n64caps_MagicTest == 0x12408037)  // Doc
    {
      Temp = CartID[0];
      CartID[0] = CartID[1];
      CartID[1] = Temp;
    }
  fwrite (CartID, 1, 2, n64caps_APSFile);

  if (n64caps_MagicTest == 0x12408037)  // Doc
    fseek (n64caps_ORGFile, 63, SEEK_SET);
  else
    fseek (n64caps_ORGFile, 62, SEEK_SET);
  fread (&Teritory, sizeof (Teritory), 1, n64caps_ORGFile);
  fwrite (&Teritory, sizeof (Teritory), 1, n64caps_APSFile);

  fseek (n64caps_ORGFile, 0x10, SEEK_SET);      // CRC Header Position
  fread (Buffer, 1, 8, n64caps_ORGFile);

  if (n64caps_MagicTest == 0x12408037)  // Doc
    {
      Temp = Buffer[0];
      Buffer[0] = Buffer[1];
      Buffer[1] = Temp;
      Temp = Buffer[2];
      Buffer[2] = Buffer[3];
      Buffer[3] = Temp;
      Temp = Buffer[4];
      Buffer[4] = Buffer[5];
      Buffer[5] = Temp;
      Temp = Buffer[6];
      Buffer[6] = Buffer[7];
      Buffer[7] = Temp;
    }

  fwrite (Buffer, 1, 8, n64caps_APSFile);

  fseek (n64caps_ORGFile, 0, SEEK_SET);

  fputc (0, n64caps_APSFile);   // PAD
  fputc (0, n64caps_APSFile);   // PAD
  fputc (0, n64caps_APSFile);   // PAD
  fputc (0, n64caps_APSFile);   // PAD
  fputc (0, n64caps_APSFile);   // PAD
}


void
WriteSizeHeader ()
{
  long NEWSize, ORGSize;

  fseek (n64caps_NEWFile, 0, SEEK_END);
  fseek (n64caps_ORGFile, 0, SEEK_END);

  NEWSize = ftell (n64caps_NEWFile);
  ORGSize = ftell (n64caps_ORGFile);

  fwrite (&NEWSize, sizeof (NEWSize), 1, n64caps_APSFile);

  if (ORGSize != NEWSize)
    n64caps_ChangeFound = n64caps_TRUE;

  fseek (n64caps_NEWFile, 0, SEEK_SET);
  fseek (n64caps_ORGFile, 0, SEEK_SET);
}


void
WritePatch ()
{
  long ORGReadLen;
  long NEWReadLen;
  int Finished = n64caps_FALSE;
  unsigned char ORGBuffer[n64caps_BUFFERSIZE];
  unsigned char NEWBuffer[n64caps_BUFFERSIZE];
  long FilePos;
  int i;
  long ChangedStart = 0;
  long ChangedOffset = 0;
  int ChangedLen = 0;
  int State;

  fseek (n64caps_ORGFile, 0, SEEK_SET);
  fseek (n64caps_NEWFile, 0, SEEK_SET);

  FilePos = 0;

  while (!Finished)
    {
      ORGReadLen = fread (ORGBuffer, 1, n64caps_BUFFERSIZE, n64caps_ORGFile);
      NEWReadLen = fread (NEWBuffer, 1, n64caps_BUFFERSIZE, n64caps_NEWFile);

      if (ORGReadLen != NEWReadLen)
        {
          int a;

          for (a = ORGReadLen; a < NEWReadLen; a++)
            ORGBuffer[a] = 0;
        }

      i = 0;
      State = 0;

      if (NEWReadLen != 0)
        {
          while (i < NEWReadLen)
            {
              switch (State)
                {
                case 0:
                  if (NEWBuffer[i] != ORGBuffer[i])
                    {
                      State = 1;
                      ChangedStart = FilePos + i;
                      ChangedOffset = i;
                      ChangedLen = 1;
                      n64caps_ChangeFound = n64caps_TRUE;
                    }
                  i++;
                  break;
                case 1:
                  if (NEWBuffer[i] != ORGBuffer[i])
                    {
                      ChangedLen++;
                      i++;
                    }
                  else
                    State = 2;
                  break;
                case 2:
                  fwrite (&ChangedStart, sizeof (ChangedStart), 1,
                          n64caps_APSFile);
                  fputc ((ChangedLen & 0xff), n64caps_APSFile);
                  fwrite (NEWBuffer + (ChangedOffset), 1, ChangedLen,
                          n64caps_APSFile);
                  State = 0;
                  break;
                }
            }
        }

      if (State != 0)
        {
          if (ChangedLen == 0)
            ChangedLen = 255;
          fwrite (&ChangedStart, sizeof (ChangedStart), 1, n64caps_APSFile);
          fputc ((ChangedLen & 0xff), n64caps_APSFile);
          fwrite (NEWBuffer + (ChangedOffset), 1, ChangedLen,
                  n64caps_APSFile);
        }

      if (NEWReadLen == 0)
        Finished = n64caps_TRUE;
      FilePos += NEWReadLen;
    }
}


int
n64caps_main (int argc, char *argv[])
{
  char File1[256];
  char File2[256];
  char OutFile[256];
  char Description[81];

  n64caps_ChangeFound = n64caps_FALSE;

  Description[0] = 0;


  strcpy (File1, argv[2]);
  strcpy (File2, argv[3]);
  strcpy (OutFile, argv[4]);

  if (!n64caps_CheckFile (File1, "rb", n64caps_TRUE))
    return (1);
  if (!n64caps_CheckFile (File2, "rb", n64caps_TRUE))
    return (1);
  if (!n64caps_CheckFile (OutFile, "wb", n64caps_FALSE))
    return (1);

  n64caps_APSFile = fopen (OutFile, "wb");
  n64caps_ORGFile = fopen (File1, "rb");
  n64caps_NEWFile = fopen (File2, "rb");

  if (!n64caps_Quiet)
    {
//              printf ("%s", n64caps_MESSAGE);
//              printf ("%s", n64caps_COPYRIGHT);
      printf ("Writing Headers...");
      fflush (stdout);
    }

  WriteStdHeader (Description);

  WriteN64Header ();

  WriteSizeHeader ();

  if (!n64caps_Quiet)
    {
      printf ("Done\nFinding Changes...");
      fflush (stdout);
    }

  WritePatch ();

  if (!n64caps_Quiet)
    {
      printf ("Done\n");
      fflush (stdout);
    }

  fclose (n64caps_NEWFile);
  fclose (n64caps_ORGFile);
  fclose (n64caps_APSFile);

  if (!n64caps_Quiet && !n64caps_ChangeFound)
    {
      printf ("No Changes Found\n");
      fflush (stdout);
      unlink (OutFile);
    }

  return (0);
}



int
aps_usage (int argc, char *argv[])
{
  printf ("\
  -a		apply APS patch (<=1.2); $FILE=PATCHFILE\n\
  -mka		create APS patch; $FILE=CHANGED_ROM\n\
  -na		change APS description; $ROM=PATCHFILE $FILE=DESCRIPTION\n\
");
  return (0);
}
