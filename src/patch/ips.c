/*
ips.c - IPS support for uCON64

written by ???? - ???? madman
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "misc.h"
#include "ucon64.h"
#include "ips.h"

void
ips_usage (void)
{
  printf ("  " OPTION_S "i           apply IPS patch (<=1.2); " OPTION_LONG_S "file=PATCHFILE\n"
           "  "OPTION_LONG_S "mki         create IPS patch; " OPTION_LONG_S "file=CHANGED_ROM\n");
}


/*

  IPS v1.0 for UNIX by madman

*/
int
ips_main (int argc, const char *argv[])
{
  FILE *ipsfile, *patchee;
  char patch[5];
  char byte;
  int done = 0, add2, add1, add0, len2, len1, len0;
  unsigned long offset, length, i;

//   printf("IPS Patcher v1.0 for UNIX by madman\n\n");

  if (argc < 3)
    {
//      printf ("Usage: %s <patchfile> <file_to_patch>\n", argv[0]);
      exit (0);
    }

  ipsfile = fopen (argv[1], "rb");
  if (ipsfile == NULL)
    {
      printf ("Could not open the file %s\n", argv[1]);
      exit (0);
    }

  patchee = fopen (argv[2], "rb+");
  if (patchee == NULL)
    {
      printf ("Could not open the file %s\n", argv[2]);
      exit (0);
    }

  fgets (patch, 6, ipsfile);
  if (strcmp (patch, "PATCH") != 0)
    {                           //make sure its a valid IPS
      printf ("Invalid IPS File!\n");
      exit (0);
    }

  printf ("Applying IPS Patch...\n");
  while (done == 0)
    {
      fread (&byte, 1, 1, ipsfile);
      if ((add2 = byte) < 0)
        add2 += 256;
      fread (&byte, 1, 1, ipsfile);
      if ((add1 = byte) < 0)
        add1 += 256;
      fread (&byte, 1, 1, ipsfile);
      if ((add0 = byte) < 0)
        add0 += 256;
      offset = ((add2 * 256 * 256) + (add1 * 256) + add0);
      if (offset == 4542278)
        {
          done = 1;
          break;
        }
      fseek (patchee, offset, SEEK_SET);
      fread (&byte, 1, 1, ipsfile);
      if ((len1 = byte) < 0)
        len1 += 256;
      fread (&byte, 1, 1, ipsfile);
      if ((len0 = byte) < 0)
        len0 += 256;
      length = ((len1 * 256) + len0);

      if (length == 0)
        {                       //code for RLE compressed block
          fread (&byte, 1, 1, ipsfile);
          if ((len1 = byte) < 0)
            len1 += 256;
          fread (&byte, 1, 1, ipsfile);
          if ((len0 = byte) < 0)
            len0 += 256;
          length = ((len1 * 256) + len0);
          fread (&byte, 1, 1, ipsfile);
          for (i = 0; i < length; i++)
            fwrite (&byte, 1, 1, patchee);
        }
      else
        {                       //non compressed
          for (i = 0; i < length; i++)
            {
              fread (&byte, 1, 1, ipsfile);
              fwrite (&byte, 1, 1, patchee);
            }
        }
    }

  fread (&byte, 1, 1, ipsfile);
  if (!feof (ipsfile))
    {                           //IPS2 stuff     
      if ((len2 = byte) < 0)
        len2 += 256;
      fread (&byte, 1, 1, ipsfile);
      if ((len1 = byte) < 0)
        len1 += 256;
      fread (&byte, 1, 1, ipsfile);
      if ((len0 = byte) < 0)
        len0 += 256;
      length = ((len2 * 256 * 256) + (len1 * 256) + len0);
      truncate (argv[2], length);
      printf ("File truncated to %ld MBit\n", (length / 1048576) * 8);
    }

  printf ("Patching complete!\n\n"
          "NOTE: sometimes you have to add/strip a 512 bytes header when you patch a ROM\n"
          "      This means you must convert for example a Super Nintendo ROM with -swc\n"
          "      or -mgd or the patch will not work\n");
  fclose (patchee);
  fclose (ipsfile);
  return 0;
}

/*
#define ips_TOP 4542278 //tales of phantasia had 6291968 (incl 512bytes header)

int ips(const char *name, const char *option2)
{
	FILE *ipsfile, *patchee;
	char patch[5];
	char byte;
	int done=0,  add2, add1, add0, len2, len1, len0;
	unsigned long offset, length, i;

	if(!(ipsfile=fopen(option2,"rb")))return(-1);
	if(!(patchee=fopen(name,"r+b")))return(-1);

	fgets(patch,6,ipsfile);
	if(strcmp(patch,"PATCH")!=0) return(-1);

	n_hexdump(option2,0,5);
	printf("\n");
	printf("IPS/International Patch Standard\n");
	printf("%ld (%.4f Mb)\n",n_size(option2),(float)n_size(option2)/MBIT);
	printf("\n");

	printf("Internal Size: %.4f Mb\n",(float)(n_size(option2)-8)/MBIT);
	printf("Version: 1.%s\n","?");
	printf("\n");
   

	while(done==0) 
	{
		fread(&byte, 1, 1, ipsfile);
		if ((add2=byte)<0) add2+=256;

		fread(&byte, 1, 1, ipsfile);
		if ((add1=byte)<0) add1+=256;

		fread(&byte, 1, 1, ipsfile);
		if ((add0=byte)<0) add0+=256;

		offset=((add2*256*256)+(add1*256)+add0);

		if(offset>=ips_TOP)
		{
			done=1;
			break;
		}

		fseek(patchee, offset, SEEK_SET);
		fread(&byte, 1, 1, ipsfile);
		if ((len1=byte)<0) len1+=256;
		fread(&byte, 1, 1, ipsfile);
		if ((len0=byte)<0) len0+=256;
		length=((len1*256)+len0);
    
		if (length==0) 
		{
//code for RLE compressed block
			fread(&byte, 1, 1, ipsfile);
			if((len1=byte)<0) len1+=256;
			fread(&byte, 1, 1, ipsfile);
			if ((len0=byte)<0) len0+=256;
			length=((len1*256)+len0);
			fread(&byte, 1, 1, ipsfile);
			for (i=0; i<length; i++) 
			fwrite(&byte, 1, 1, patchee);
		}
		else 
		{
//non compressed
	 		for (i=0; i<length; i++) 
	 		{
	    			fread(&byte, 1, 1, ipsfile);
	    			fwrite(&byte, 1, 1, patchee);
		 	}
	      	}
	}
	fread(&byte, 1, 1, ipsfile);

	if(!feof(ipsfile))
	{  
//IPS2 stuff     
		if ((len2=byte)<0) len2+=256;
	      	fread(&byte, 1, 1, ipsfile);
	      	if ((len1=byte)<0) len1+=256;
	      	fread(&byte, 1, 1, ipsfile);
	      	if ((len0=byte)<0) len0+=256;
	      	length=((len2*256*256)+(len1*256)+len0);
	      	truncate (name, length);
	      	printf("File truncated to %lu Mb\n", (length/1048576)*8);
	}
   
	fclose(patchee);
	fclose(ipsfile);
	return(0);
}
*/



int
cips (const char *name, const char *option2)
{
  FILE *fp, *fp2, *fp3;
//      FILE *ORGFile, *NEWFile;
  int done = 0, diffdone = 0;
  unsigned long filepos = 0, add0, add1, add2;
  unsigned long diffcount = 0;
  char infp, infp2;
  char buf[65537];

  if (!(fp = fopen (name, "rb")))
    {
      return -1;
    }

  if (!(fp2 = fopen (option2, "rb")))
    {
      fclose (fp);
      return -1;
    }

  strcpy (buf, name);
  setext (buf, ".IPS");

  if (!(fp3 = fopen (buf, "wb")))
    {
      fclose (fp);
      fclose (fp2);
      return -1;
    }

  fprintf (fp3, "PATCH");

  while (!done)
    {
      /* Grab each character from fp and fp2, compare, keep pos */
      infp = fgetc (fp);
      infp2 = fgetc (fp2);
      if (infp != infp2)
        {

          /* Save current pos in patch file */
          /* Go through a loop until infp is equal to infp2 */
          if (filepos < 256)
            {
              add2 = add1 = 0;
              add0 = filepos;
              fprintf (fp3, "%c%c%c", 0, 0, (char) filepos);
            }
          else if (filepos < 65536)
            {
              add2 = 0;
              add1 = filepos / 256;
              add0 = filepos % 256;
              fprintf (fp3, "%c%c%c", 0, (char) (filepos / 256),
                       (char) (filepos % 256));
            }
          else if (filepos < 16711680)
            {
              add2 = filepos / 65536;
              add1 = (filepos % 65536) / 256;
              add0 = ((filepos % 65536) % 256);
              fprintf (fp3, "%c%c%c", (char) (filepos / 65536),
                       (char) ((filepos % 65536) / 256),
                       (char) ((filepos % 65536) % 256));
            }

          diffcount = 0;
          diffdone = 0;
          while (!diffdone)
            {
              buf[diffcount++] = infp2;
              infp = fgetc (fp);
              infp2 = fgetc (fp2);
              if (feof (fp2))
                {
                  diffdone = 1;
                  continue;
                }
              filepos++;
              if ((!feof (fp) && (infp == infp2)) || (diffcount >= 65535))
                {
                  diffdone = 1;
                  continue;
                }
            }
          fprintf (fp3, "%c%c",
                   (char) ((diffcount > 256) ? (diffcount / 256) : 0),
                   (char) ((diffcount >
                            256) ? (diffcount % 256) : diffcount));
          fwrite (buf, diffcount, 1, fp3);
        }
      if (feof (fp2))
        done = 1;
      filepos++;
    }
  fprintf (fp3, "EOF");
  fclose (fp3);
  fclose (fp2);
  fclose (fp);
  return 0;
}



