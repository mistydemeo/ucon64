/*
dex.h - DexDrive support for uCON64

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
#ifndef DEX_H
#define DEX_H
#include "../ucon64.h"

#define dex_TITLE "DexDrive\nInterAct http://www.dexdrive.de"

int dex_usage (int argc, char *argv[]);
#define dex_HEADER_START 0
#define dex_HEADER_LEN 0


char *read_block (int block_num, char *data);
int write_block (int block_num, char *data);
char *read_frame (int frame, char *data);
int write_frame (int frame, char *data);
int get_perm ();

#endif /* DEX_H */
