/*
mgd.h - Multi Game Doctor/Hunter support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
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
#ifndef MGD_H
#define MGD_H

extern const st_usage_t mgd_usage[];

/*
The MGD2 only accepts certain filenames, and these filenames
must be specified in an index file, "MULTI-GD", otherwise the
MGD2 will not recognize the file. In the case of multiple games
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
                                SF8XXXxB.078    SF8XXXxB
16M             2               SF16XXXA.078    SF16XXXA
                                SF16XXXB.078    SF16XXXB
20M             3               SF20XXXA.078    SF20XXXA
                                SF20XXXB.078    SF20XXXB
                                SF20XXXC.078    SF20XXXC
24M             3               SF24XXXA.078    SF24XXXA
                                SF24XXXB.078    SF24XXXB
                                SF24XXXC.078    SF24XXXC
32M             4               SF32XXXA.078    SF32XXXA
                                SF32XXXB.078    SF32XXXB
                                SF32XXXC.078    SF32XXXC
                                SF32XXXD.078    SF32XXXC

Mega Drive:

game size       # of files      names           MULTI-GD
================================================================
1M              1               MD1XXX.000      MD1XXX
2M              1               MD2XXX.000      MD2XXX
4M              1               MD4XXX.000      MD4XXX
8M              1               MD8XXX.008      MD8XXX
16M             2               MD16XXXA.018    MD16XXXA
                                MD16XXXB.018    MD16XXXB
20M             3               MD20XXXA.038    MD20XXXA
                                MD20XXXB.038    MD20XXXB
                                MD20XXXC.038    MD20XXXC
24M             3               MD24XXXA.038    MD24XXXA
                                MD24XXXB.038    MD24XXXB
                                MD24XXXC.038    MD24XXXC
32M             4               MD32XXXA.038    MD32XXXA
                                MD32XXXB.038    MD32XXXB
                                MD32XXXC.038    MD32XXXC
                                MD32XXXD.038    MD32XXXD

PC Engine:

game size       # of files      names           MULTI-GD
================================================================
1M              1               PC1XXX.040      PC1XXX
2M              1               PC2XXX.040      PC2XXX
4M              1               PC4XXX.048      PC4XXX
8M              1               PC8XXX.058      PC8XXX

Game Gear:

game size       # of files      names           MULTI-GD
================================================================
2M              1               GG2XXX.000 (040) GG2XXX
4M              1               GG4XXX.018 (048) GG4XXX


Contrary to popular belief the Game Doctor SF3/SF6/SF7 *does*
use a 512 byte header like the SWC, but it also accepts
headerless files.
A header is necessary when things like SRAM size must be made
known to the Game Doctor. The Game Doctor also uses specially
designed filenames to distinguish between multi files.

Usually, the filename is in the format of: SFXXYYYZ.078

Where SF means Super Famicom, XX refers to the size of the
image in Mbit. If the size is only one character (i.e. 2, 4 or
8 Mbit) then no leading "0" is inserted.

YYY refers to a catalogue number in Hong Kong shops
identifying the game title. (0 is Super Mario World, 1 is F-
Zero, etc). I was told that the Game Doctor copier produces a
random number when backing up games.

Z indicates a multi file. Like XX, if it isn't used it's
ignored.

A would indicate the first file, B the second, etc. I am told
078 is not needed, but is placed on the end of the filename by
systems in Asia.

e.g. The first 16 Mbit file of Donkey Kong Country (assuming it
is cat. no. 475) would look like: SF16475A.078
*/

#ifdef PARALLEL
#endif // PARALLEL

// the following four functions are used by non-transfer code in genesis.c
extern void mgd_interleave (unsigned char **buffer, int size);
extern void mgd_deinterleave (unsigned char **buffer, int data_size,
                              int buffer_size);
extern int fread_mgd (void *buffer, size_t size, size_t number, FILE *fh);
extern int q_fread_mgd (void *buffer, size_t start, size_t len,
                        const char *filename);
extern void mgd_make_name (const char *filename, const char *prefix, int size,
                           char *name);

#define MGD_HEADER_START 0
#define MGD_HEADER_LEN 512
#endif // MGD_H
