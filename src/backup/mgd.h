/*
mgd.h - Multi Game Doctor/Hunter support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)


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
#ifndef MGD_H
#define MGD_H

extern const char *mgd_usage[];

/*
The MGD2 only accepts certain filenames, and these filenames
must be specified in an index file, "MULTI-GD", otherwise the
MGD2 will not recognize the file.  In the case of multiple games
being stored in a single disk, simply enter its corresponding
MULTI-GD index into the "MULTI-GD" file.

Super Famicom:

game size       # of files      names           MULTI-GD
================================================================
4M              1               SF4XXX.048      SF4XXX
4M              2               SF4XXXxA.078    SF4XXXxA
                                SF4XXXxB.078    SF4XXXxB
8M              1               SF8XXX.058      SF8XXX
                2               SF8XXXxA.078    SF8XXXxA
                                SF8XXXxB.078    SF8xxxxB
16M             2               SF16XXXA.078    SF16XXXA
                                SF16XXXB.078    SF16XXXB
24M             3               SF24XXXA.078    SF24XXXA
                                SF24XXXB.078    SF24XXXB
                                SF24XXXC.078    SF24XXXC

Mega Drive:

game size       # of files      names           MUTLI-GD
================================================================
1M              1               MD1XXX.000      MD1XXX
2M              1               MD2XXX.000      MD2XXX
4M              1               MD4XXX.000      MD4XXX
8M              1               MD8XXX.008      MD8XXX
16M             2               MD16XXXA.018    MD16XXXA
                                MD16XXXB.018    MD16XXXB
24M             3               MD24XXXA.038    MD24XXXA
                                MD24XXXB.038    MD24XXXB
                                MD24XXXC.038    MD24XXXC
32M             4               MD32XXXA.038    MD32XXXA
                                MD32XXXB.038    MD32XXXB
                                MD32XXXC.038    MD32XXXC
                                MD32XXXD.038    MD32XXXD

PC Engine:
game size       # of files      names           MUTLI-GD
================================================================
1M              1               PC1XXX.040      PC1XXX
2M              1               PC2XXX.040      PC2XXX
4M              1               PC4XXX.048      PC4XXX
8M              1               PC8XXX.058      PC8XXX


The Game Doctor does not use a 512 byte header like the SWC,
instead it uses specially designed filenames to distinguish
between multi files. I'm not sure if it used the filename for
information about the size of the image though.
<p>
Usually, the filename is in the format of: SFXXYYYZ.078
<p>
Where SF means Super Famicon, XX refers to the size of the
image in Mbit. If the size is only one character (i.e. 2, 4 or
8 Mbit) then no leading "0" is inserted.
<p>
YYY refers to a catalogue number in Hong Kong shops
identifying the game title. (0 is Super Mario World, 1 is F-
Zero, etc). I was told that the Game Doctor copier produces a
random number when backing up games.
<p>
Z indicates a multi file. Like XX, if it isn't used it's
ignored.
<p>
A would indicate the first file, B the second, etc. I am told
078 is not needed, but is placed on the end of the filename by
systems in Asia.
<p>
e.g. The first 16Mbit file of Donkey Kong Country (assuming it
  is cat. no. 475) would look like:  SF16475A.078


Offset
Size
Section
   Description
   H100
  16 bytes
  File Identifier
 'SEGA MEGA DRIVE' or 'SEGA GENESIS'
   H110
  16 bytes
  Copyright notice
 See Below
   H120
  48 bytes
  Domestic game name
 Original Name [48 bytes]
   H150
  48 bytes
  Overseas game name
 Worldwide Name [48 bytes]
   H180
  14 bytes
  Product Code
 'PT XXXXXXXXXXX'// PT=Product type X=Product Code after '-' is the version number
   H18E
  02 bytes
  Checksum
 Checksum of ROM
   H190
  16 bytes
  I/O Support
 Can contain up to 16. Unused need to be filled with spaces. See Below
   H1A0
  08 bytes
  ROM capacity
 Start Address [4 bytes; most cases 0] End Address [4 bytes]
   H1A8
  08 bytes
  RAM capacity
 Start Address [4 bytes] End Address [4 bytes]
   H1B0
  12 bytes
  External RAM
 Unknown
   H1BC
  12 bytes
  MODEM Support
 If not supported filled with spaces otherwise 'MOxxxxyy.z' //x=Company Code y=MODEM NO. z=Version
   H1C8
  40 bytes
  MEMO
 Seems just to be reserved for Memos.
   H1F0
  03 bytes
  Countries
 Can contain up to 3. Unused need to be filled with spaces. See Below


   Copyright Notice

Standard Format: (C)XXXX yyyy.mmm //XXXX=Company Code
2 Char Code Format: (C)T-XX yyyy.mmm //XX=Company Code
3 Char Code Format: (C)T-XXX yyyy.mmm //XXX=Company Code
Wrong Copyrights found:

The year is written as '199X' or '19XX', or doen't include the millenium and the century.
The company name is '00' or 'XX'
Some companies that use a number for company code overwrite the hyphen, not the space.
Some companies don't include the '(C)' in the beginning and others include just their name; some just include the the year
Some copyrights have the year and month separated by ',','/', '-', space, null character (H00) or no separator at all.
Wrong Abbreviations for Months= APL, 08 and SEPT.
*/

#ifdef BACKUP
#endif // BACKUP

#define MGD_HEADER_START 0
#define MGD_HEADER_LEN 0
#endif // MGD_H
