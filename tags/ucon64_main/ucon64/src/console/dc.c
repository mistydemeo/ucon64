/*
dc.c - Dreamcast support for uCON64

written by 2001 NoisyB (noisyb@gmx.net)


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
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "dc.h"
#if 0

static int dc_ip(char *dev,char *name)
#endif


const st_usage_t dc_usage[] = {
    {NULL, NULL, "Dreamcast"},
    {NULL, NULL, "1998 SEGA http://www.sega.com"},
    {"dc", NULL, "force recognition"},
//TODO    {"ip", "FILE", "extract ip.bin FILE from IMAGE; " OPTION_LONG_S "rom=IMAGE"},
    {NULL, NULL, NULL}
  };


int
dc_init (st_rominfo_t *rominfo)
{
//  dc_ip0000_header_t header;
  int result = -1;

//  printf ("%d\n", sizeof (dc_ip0000_header_t));

  rominfo->console_usage = dc_usage;
//  rominfo->copier_usage = cdrw_usage;

  return result;
}


#define NUM_FIELDS 11

struct field;

int check_areasym(char *, struct field *);

struct field {
  char *name;
  int pos;
  int len;
  int (*extra_check) (char *, struct field *);
} fields[NUM_FIELDS] = {
  { "Hardware ID", 0x0, 0x10, NULL },
  { "Maker ID", 0x10, 0x10, NULL },
  { "Device Info", 0x20, 0x10, NULL },
  { "Area Symbols", 0x30, 0x8, check_areasym },
  { "Peripherals", 0x38, 0x8, NULL },
  { "Product No", 0x40, 0xa, NULL },
  { "Version", 0x4a, 0x6, NULL },
  { "Release Date", 0x50, 0x10, NULL },
  { "Boot Filename", 0x60, 0x10, NULL },
  { "SW Maker Name", 0x70, 0x10, NULL },
  { "Game Title", 0x80, 0x80, NULL },
};

int filled_in[NUM_FIELDS];

int
check_areasym (char *ptr, struct field *f)
{
  int i, a = 0;
  for (i = 0; i < f->len; i++)
    switch (ptr[i])
      {
      case 'J':
        a |= (1<<0);
        break;
      case 'U':
        a |= (1<<1);
        break;
      case 'E':
        a |= (1<<2);
        break;
      case ' ':
        break;
      default:
        fprintf (stderr, "Unknown area symbol '%c'.\n", ptr[i]);
        return 0;
      }
  for (i = 0; i < f->len; i++)
    if ((a & (1<<i)) == 0)
      ptr[i] = ' ';
    else
      ptr[i] = "JUE"[i];
  return 1;
}


void
trim (char *str)
{
  int l = strlen(str);
  while (l > 0 && (str[l-1] == '\r' || str[l-1] == '\n' ||
         str[l-1] == ' ' || str[l-1] == '\t'))
    str[--l] = 0;
}


int
parse_input (FILE *fh, char *ip)
{
  static char buf[80];
  int i;
  memset (filled_in, 0, sizeof (filled_in));
  while (fgets (buf, sizeof (buf), fh))
    {
      char *p;
      trim (buf);
      if (*buf)
        {
          if ((p = strchr (buf, ':')))
            {
              *p++ = '\0';
              trim (buf);
              for (i = 0; i < NUM_FIELDS; i++)
                if (!strcmp (buf, fields[i].name))
                  break;
              if (i >= NUM_FIELDS)
                {
                  fprintf (stderr, "Unknown field \"%s\".\n", buf);
                  return 0;
                }
              memset (ip + fields[i].pos, ' ', fields[i].len);
              while (*p == ' ' || *p == '\t')
                p++;
              if ((int) strlen (p) > fields[i].len)
                {
                  fprintf (stderr, "Data for field \"%s\" is too long.\n",
                           fields[i].name);
                  return 0;
                }
              memcpy (ip + fields[i].pos, p, strlen (p));
              if (fields[i].extra_check != NULL &&
                  !(*fields[i].extra_check) (ip + fields[i].pos, &fields[i]))
                return 0;
              filled_in[i] = 1;
            }
          else
            {
              fprintf (stderr, "Missing : on line.\n");
              return 0;
            }
        }
    }

  for (i = 0; i < NUM_FIELDS; i++)
    if (!filled_in[i])
      {
        fprintf (stderr, "Missing value for \"%s\".\n", fields[i].name);
        return 0;
      }

  return 1;
}


int
calcCRC (const unsigned char *buf, int size)
{
  int i, c, n = 0xffff;
  for (i = 0; i < size; i++)
    {
      n ^= (buf[i] << 8);
      for (c = 0; c < 8; c++)
        if (n & 0x8000)
          n = (n << 1) ^ 4129;
        else
          n = (n << 1);
    }
  return n & 0xffff;
}


void
update_crc (char *ip)
{
  int n = calcCRC ((unsigned char *) (ip + 0x40), 16);
  char buf[5];
  sprintf (buf, "%04X", n);
  if (memcmp (buf, ip + 0x20, 4))
    {
      printf ("Setting CRC to %s (was %.4s)\n", buf, ip + 0x20);
      memcpy (ip + 0x20, buf, 4);
    }
}


int
dc_mkip (const char *ip_file) // make ip
{
/* in
Hardware ID   : SEGA SEGAKATANA
Maker ID      : SEGA ENTERPRISES
Device Info   : 0000 CD-ROM1/1
Area Symbols  : JUE
Peripherals   : E000F10
Product No    : T0000
Version       : V1.000
Release Date  : 20000627
Boot Filename : 1ST_READ.BIN
SW Maker Name : YOUR NAME HERE
Game Title    : TITLE OF THE SOFTWARE
*/
  FILE *fh;
  char ip[0x8000], dest_name[FILENAME_MAX], *in = NULL; // TODO: something that sets this var :-)

  if (!(fh = fopen (in, "rb")))
    return -1;
  if (!parse_input (fh, ip))
    return -1;

  fclose (fh);
  update_crc (ip);

  strcpy (dest_name, ip_file);
  ucon64_file_handler (dest_name, NULL, 0);
  q_fwrite (ip, 0, 0x8000, dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}



/*
 How to Make an 11700 to 11702 LBA Patch
 by xeal

 Note: This guide explains how to make a patch that others can use with the ppf software made by paradox. It is expected that
 the patch-maker already has some basic hex editing skills. I personally recommend either Hexworkshop or HEdit.

 If you find any errors or you need clarification on something, mail me.

 The ppf software is at www.paradogs.com, cdirip/isofix are at DeXT's site, cdirip.freeyellow.com

 1. Extract your cdi from the rars.
 2. Use cdirip on the cdi to produce the raw files/iso file (write down the second session LBA, you'll need it later)
 3. use isofix on the iso file.
 4. make a new folder
 5. use isobuster to extract all the files to this folder from the new iso you made with isofix (open as a creator iso image)
 6. use mkisofs: mkisofs -C 0,xxxxx -l -o tmp.iso //c/game/
 note: replace xxxxx with whatever LBA your second session needs to be
 note: replace //c/game/ with whatever the path to the folder you isobusted to is
 7. wait for mkisofs to start showing a percent completed. Hit ctrl+c to stop it. No need to continue, we only want the header.
 8. open the new iso (tmp.iso) you just made with mkisofs in a hex editor.
 9. go to address B84A, write down the 8 hex digits you see. (example: 0000 2DE5)
 10. convert the new second session LBA to hex (example: 11702 = 2DB6)
 11. subtract this from the value at B84A (step 4.) (example: 2DE5 - 2DB6 = 2F)
 12. multiply this value by 800 (example: 800 * 2F = 17800)
 13. subtract 8000 from this value (example: 17800 - 8000 = F800)
 14. go to address 8000 in your mkisofs iso (tmp.iso)
 15. select a block that is the size in bytes from step 8. (example, select F800 bytes starting at 8000)
 16. copy this and paste into a new file. you now have a new, fixed header file
 17. go back to the original iso you got from cdirip. Go to address 8000, select the same size block from step 10, paste your
 fixed header over top this. (Do not insert, REPLACE. You have to keep the same iso size.)
 18. search for the text string cd001 in this iso.
 19. Take the Hex value of the original second session LBA (11700 = 2DB4) Add A6 to it (2DB4 + A6 = 2E5A). Reverse the
 order of the bytes. (2E 5A would now be 5A 2E, one byte is two hex values)
 20. Look behind the cd001 string that you found for this value (if you're not sure you've found the right spot, shortly after the
 cd001 text you should also see some text that includes "gdfs"). If you see it, take the LBA for the second session that you
 used with mkisofs, convert to hex, add A6 to it, and reverse the bytes, just like step 14 but with your new lba (11702).
 Replace this over top the value you had found before the text string cd001. Search your iso for other incidents of this as well
 (usually there's only one, but some games have exceptions) (basically for 11700 to 11702 games, you change 5A2E to 5C2E).
 21. You should now have an altered iso for the second session of a self-boot dreamcast game.

 At this point you should burn your new iso to see if it works.
 If it does you might as well proceed to make the patch.

 1. Name your new altered iso nData02.iso (or whatever you wish)
 2. Use cdirip again to make an original TData02.iso.
 3. Have the 2 isos in the same folder as makeppf.exe
 4. From the command line, type: makeppf TData02.iso nData02.iso patch.ppf
 5. it will ask for some descriptions, then start making a patch.
 6. when it's finished you will have a patch.ppf, and you should probably test it out or find someone else to try it.

 Make an info file, put it with the patch in a zip file, and send it to me to put on the site :)
*/


/*
<title>Dreamcast Programming - Bootable CD-Rs</title>

The Dreamcast firmware allows booting from a normal CD-R (CD-RW won't work
though), provided that it has the right structure.  This page gives detailed
instructions on how to create a bootable CD-R using <a href="http://www.fokus.gmd.de/research/cc/glone/employees/joerg.schilling/private/cdrecord.html">cdrecord</a> (people running Wintendo can allegedly find binaries <a href="ftp://ftp.fokus.gmd.de/pub/unix/cdrecord/alpha/win32/">here</a>).
The example commandlines assume a CD-R drive with SCSI id 6, you should of
course adjust them according to your configuration.  They also tend to assume you are
running some kind of UNIX.  Please don't mail me asking how to create empty files etc under DOS.
I don't know.  Ask someone who uses DOS.

Overall structure

For a CD-R to be bootable on the Dreamcast, it should have two sessions.
The first should contain only a normal audio track.  It doesn't matter
what kind of audio you actually put there, silence is fine.  (It has been
suggested that a data track could also be used for the first session.
I haven't tried this myself though.)  The second
session should contain a CD/XA data track (mode 2 form 1).  This data track
should contain a regular ISO9660 file system, and in the first 16 sectors a
correct bootstrap (IP.BIN).  How to create a correct IP.BIN is described
<a href="ip.bin.html">elsewhere</a>.

Burning the audio track

First you have to burn the audio session.  You can use any audio you like,
but the simplest option is just to create 4 seconds (the minimum track length)
of silence, like so:
<pre>
  dd if=/dev/zero bs=2352 count=300 of=audio.raw
</pre>

Next, insert a blank CD and burn the audio track.  Make sure to leave the
disc open for further sessions, the <tt>-multi</tt> option to <tt>cdrecord</tt>
takes care of that.

  cdrecord dev=0,6,0 -multi -audio audio.raw

Creating the ISO image

Now that the audio track has been burned, it is possible to create the
ISO filesystem image.  The reason that it can't be done earlier is that
on a multisession disc the sector numbers in the image have to be offset
with a number depending on the sessions burned before.  To find out this
number, run

  cdrecord dev=0,6,0 -msinfo

with the disc still in the drive.  You should get two comma separated
numbers (for example <tt>0,11700</tt>).  Remember these numbers.
Now create the ISO image with mkisofs.  If you want to create an image
containing only the file <tt>1ST_READ.BIN</tt> for example, then run

  mkisofs -l -C <i>x,y</i> -o tmp.iso 1ST_READ.BIN

where <i>x,y</i> is the pair of numbers you got with <tt>-msinfo</tt> earlier.
Make sure you get them correctly, or the image won't work.

Adding the bootstrap

The first 16 sectors of an ISO9660 filesystem are blank, to leave room
for bootstraps.  This is where IP.BIN (32768 bytes) goes.  Replace the
first 16 sectors of your image with the appropriate IP.BIN bootstrap:
<pre>
  ( cat IP.BIN ; dd if=tmp.iso bs=2048 skip=16 ) &gt; data.raw
</pre>
<!-- Nope, that didn't work.  Stupid mkisofs doesn't allow -G with -C.
Alternately, the IP.BIN can be added directly when running mkisofs:

<pre>
  mkisofs -l -C <i>x,y</i> -G IP.BIN -o data.raw 1ST_READ.BIN
</pre>
-->

Burning the data track

Finally, you're ready to burn the second session, completing the disc.
This track should be burned as CD/XA with form 1 sectors (2048 bytes
per sector).  Use the <tt>-xa1</tt> option to <tt>cdrecord</tt>:

  cdrecord dev=0,6,0 -multi -xa1 data.raw

When <tt>cdrecord</tt> completes, the disc is ready.  Eject it and try it
out in your Dreamcast.
*/
