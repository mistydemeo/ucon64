/*
gbx.h - GameBoy Xchanger support for uCON64

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
#include "../ucon64.h"

#define gbx_TITLE "GameBoy Xchanger"

int gbx_usage(int argc,char *argv[]);


int send_file(char *fname, int (*progress)(int), void (status)(char *status));
int xchanger_status(void);
int backup_cart(char *fname, int (*progress)(int), void (status)(char *status));
void init_xchanger(void);


int gbx_read(	char *filename
			,unsigned int parport
);
                        
int gbx_write(	char *filename
			,long start
			,long len
			,unsigned int parport
);
                                                                                                