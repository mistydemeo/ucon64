/*
mgd.c - Multi Game Doctor/Hunter support for uCON64

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
#include "mgd.h"

int mgd_usage(int argc,char *argv[])
{
if(argcmp(argc,argv,"-help"))printf("\n%s\n",mgd_TITLE);


printf("TODO:  -xmgd    send/receive ROM to/from Multi Game*/MGD2/MGH/RAW; $FILE=PORT\n\
		receives automatically when $ROM does not exist\n\
");

if(argcmp(argc,argv,"-help"))
{
//TODO more info like technical info about cabeling and stuff for the copier

}
	return(0);
}
