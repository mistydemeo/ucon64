/*
uCON64 1.9.5 by NoisyB (noisyb@gmx.net)
uCON64 is a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It supports N64, PSX, JAG, SNES, NG,
SEGA, GB, LYNX, PCE, SMS, GG, C64, NES, ATARI and backup units.
Copyright (C) 1999/2000/2001 NoisyB (noisyb@gmx.net)

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

/*
first i want to thank SiGMA SEVEN! who was my mentor and teached me how to
write programs in C
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/perm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "include.h"

#define ucon64_UNKNOWN		-1
#define ucon64_GB		1
#define ucon64_GENESIS		2
#define ucon64_SMS		3
#define ucon64_JAGUAR		4
#define ucon64_LYNX		5
#define ucon64_N64		6
#define ucon64_NEOGEO		7
#define ucon64_NES		8
#define ucon64_PCE		9
#define ucon64_PSX		10
#define ucon64_SNES		11
#define ucon64_SATURN		12
#define ucon64_DC		13
#define ucon64_CD32		14
#define ucon64_CDI		15
#define ucon64_REAL3DO		16
#define ucon64_ATARI		17
#define ucon64_SYSTEM16		18
#define ucon64_NEOGEOPOCKET	19
#define ucon64_KNOWN		9999

#define MBIT	131072

#define ucon64_VERSION "1.9.5"
#define ucon64_TITLE "uCON64 1.9.5 GNU/Linux 1999/2000/2001 by NoisyB (noisyb@gmx.net)"

#define ucon64_NAME	0
#define ucon64_ROM	1
#define ucon64_FILE	2
#define ucon64_name() (getarg(argc,argv,ucon64_NAME))
#define ucon64_rom() (getarg(argc,argv,ucon64_ROM))
#define ucon64_file() (getarg(argc,argv,ucon64_FILE))

int ucon64_backup(char *name);

int ucon64_argc;
char *ucon64_argv[128];

#include "stolen/nes/fam2fds.h"
#include "stolen/nes/fdslist.h"
#include "stolen/n64aps.h"
#include "stolen/n64caps.h"
#include "stolen/cips.h"
#include "stolen/ips.h"
#include "stolen/applyppf.h"
#include "stolen/makeppf.h"
#include "stolen/cdinfo.h"
#include "stolen/psx/xadump.h"
#include "stolen/psx/xa2wav.h"

#include "usage.h"
//#include "unzip.h"
#include "gamecodes.h"
#include "transfer/transfer.h"
#include "snes.h"
#include "gb.h"
#include "jaguar.h"
#include "psx.h"
#include "n64.h"
#include "lynx.h"
#include "sms.h"
#include "nes.h"
#include "genesis.h"
#include "pce.h"
#include "bsl.h"
#include "xps.h"
#include "neogeo.h"
#include "sys16.h"
#include "atari.h"
#include "dc.h"
#include "ngp.h"

int main(int argc,char *argv[])
{
register long x;
char buf[4096],buf2[4096];
long console=ucon64_UNKNOWN;
FILE *fh;

printf("%s\n",ucon64_TITLE);
printf("This may be freely redistributed under the terms of the GNU Public License\n\n");

if(	argc<2 ||
	argcmp(argc,argv,"-h") ||
	argcmp(argc,argv,"--help") ||
	argcmp(argc,argv,"-?")
)
{
	usage_main(argc,argv);
	return(0);
}



if(argcmp(argc,argv,"-db")||argcmp(argc,argv,"-dbv"))
{
	atari_main(argc,argv);
//	commodore_main(argc,argv);
//	gameboy_main(argc,argv);
//	segamegadrive_main(argc,argv);
	jaguar_main(argc,argv);
	lynx_main(argc,argv);
//	nintendo64_main(argc,argv);
	neogeo_main(argc,argv);
	nes_main(argc,argv);
	pcengine_main(argc,argv);
	sms_main(argc,argv);
//	supernintendo_main(argc,argv);
	system16_main(argc,argv);

	printf("\n");
	return(0);
}

if(!ucon64_rom()[0])
{
	usage_main(argc,argv);
	return(0);
}

if(argcmp(argc,argv,"-rn"))
{
	n_renlwr(ucon64_rom());
	return(0);
}

if(argcmp(argc,argv,"-hex"))
{
	n_hexdump(ucon64_rom(),0,n_size(ucon64_rom()));
	return(0);
}

/*
if(argcmp(argc,argv,"-ls"))
{
	mkfid(ucon64_rom());
	return(0);
}
*/

if(argcmp(argc,argv,"-c"))
{
	if(n_cmp(ucon64_rom(),0,ucon64_file(),0,n_cmp_DIFF)!=0)
		printf("ERROR: file not found/out of memory\n");
	return(0);
}

if(argcmp(argc,argv,"-cs"))
{
	if(n_cmp(ucon64_rom(),0,ucon64_file(),0,n_cmp_SIMI)!=0)
                       printf("ERROR: file not found/out of memory\n");
	return(0);
}

if(argcmp(argc,argv,"-find"))
{
	x=0;
	while((x=n_strstr(ucon64_rom(),ucon64_file(),x,strlen(ucon64_file())))!=-1)
	{
		n_hexdump(ucon64_rom(),x,strlen(ucon64_file()));
		x++;
		printf("\n");
	}
	return(0);
}

if(argcmp(argc,argv,"-cdrom"))
{
//	n_cdrom_nfo(ucon64_rom(),ucon64_rom());
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_rom();
	ucon64_argc=2;

	cdinfo_main(ucon64_argc,ucon64_argv);
	return(0);
}



if(argcmp(argc,argv,"-b2i"))
{
	bin2iso(ucon64_rom());
	return(0);
}

if(argcmp(argc,argv,"-swap"))
{
/*
	strcpy(buf,romname);
	buf[strcspn(buf,".")+1]=0;
	strcat(buf,isstrlwr(buf)?"rom":"ROM");

	n_copy(romname,0,n_size(romname),buf,"wb");
//	if(!swapped)fswap(buf,0,(n_size(buf)-hsize));
	n_swap(buf,0,(n_size(buf)-hsize));


*/

	n_swap(ucon64_rom(),0,n_size(ucon64_rom()));
	return(0);
}

if(argcmp(argc,argv,"-pad"))
{
	n_pad(ucon64_rom(),0,"ab");
	return(0);
}

if(argcmp(argc,argv,"-ispad"))
{
	unsigned long padded;
	
	if((padded=n_pad(ucon64_rom(),0,"rb"))!=-1)
	{
		if(!padded)printf("Padded: No\n");
		else printf("Padded: Maybe, %ld Bytes (%.4f Mb)\n",padded,(float)padded/MBIT);
	}
	printf("\n");return(0);
}

if(argcmp(argc,argv,"-stp"))
{
	strcpy(buf,ucon64_rom());
	buf[strcspn(buf,".")+1]=0;
	strcat(buf,isstrlwr(buf)?"tmp":"TMP");
	rename(ucon64_rom(),buf);
	n_copy(buf,512,n_size(buf),ucon64_rom(),"wb");

	remove(buf);
	return(0);
}
if(argcmp(argc,argv,"-ins"))
{
	strcpy(buf,ucon64_rom());
	buf[strcspn(buf,".")+1]=0;
	strcat(buf,isstrlwr(buf)?"tmp":"TMP");
	rename(ucon64_rom(),buf);

	if((fh=fopen(ucon64_rom(),"wb"))!=0)
	{
		memset(buf2,0x00,512);
		fwrite(buf2,512,1,fh);
		fclose(fh);
	}
	n_copy(buf,0,n_size(buf),ucon64_rom(),"ab");
	remove(buf);
	return(0);
}

if(argcmp(argc,argv,"-b"))
{
	if(bsl(ucon64_rom(),ucon64_file())!=0)printf("ERROR: failed\n");
	return(0);
}

if(argcmp(argc,argv,"-i"))
{
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_file();
	ucon64_argv[2]=ucon64_rom();
	ucon64_argc=3;

	if(ips_main(ucon64_argc,ucon64_argv));
	return(0);
}

if(argcmp(argc,argv,"-a"))
{
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]="-f";
	ucon64_argv[2]=ucon64_rom();
	ucon64_argv[3]=ucon64_file();
	ucon64_argc=4;

	if(n64aps_main(ucon64_argc,ucon64_argv));
	return(0);
}

if(argcmp(argc,argv,"-ppf"))
{
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_rom();
	ucon64_argv[2]=ucon64_file();
	ucon64_argc=3;

	if(applyppf_main(ucon64_argc,ucon64_argv));
	return(0);
}

if(argcmp(argc,argv,"-mki"))
{
	cips(ucon64_rom(),ucon64_file());
/*
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_file();
	ucon64_argv[2]=ucon64_rom();
	ucon64_argc=3;

	if(cips_main(ucon64_argc,ucon64_argv));
*/
	return(0);
}

if(argcmp(argc,argv,"-mka"))
{
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]="-d \"\"";
	ucon64_argv[2]=ucon64_rom();
	ucon64_argv[3]=ucon64_file();
	ucon64_argv[4]="test";
	ucon64_argc=5;

	if(n64caps_main(ucon64_argc,ucon64_argv));
	return(0);
}

if(argcmp(argc,argv,"-na"))
{
	strcpy(buf2,ucon64_file());
strcat(buf2,"\
                                                            ");
                                                            
                n_strncpy(ucon64_rom(),buf2,7,50);
                
		return(0);
}

if(argcmp(argc,argv,"-mkppf"))
{
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_rom();
	ucon64_argv[2]=ucon64_file();
	ucon64_argv[3]="test";
	ucon64_argc=4;

	if(makeppf_main(ucon64_argc,ucon64_argv));
	return(0);
}

if(argcmp(argc,argv,"-nppf"))
{
	strcpy(buf2,ucon64_file());
strcat(buf2,"\
                                                            ");
                                                            
               n_strncpy(ucon64_rom(),buf2,6,50);
                
	return(0);
}











//find out which console type

if(argcmp(argc,argv,"-ata"))console=ucon64_ATARI;
if(argcmp(argc,argv,"-s16"))console=ucon64_SYSTEM16;
if(argcmp(argc,argv,"-n64"))console=ucon64_N64;
if(argcmp(argc,argv,"-psx"))console=ucon64_PSX;
if(argcmp(argc,argv,"-psx2"))console=ucon64_PSX;
if(argcmp(argc,argv,"-lynx"))console=ucon64_LYNX;
if(argcmp(argc,argv,"-jag"))console=ucon64_JAGUAR;
if(argcmp(argc,argv,"-sms"))console=ucon64_SMS;
if(argcmp(argc,argv,"-snes"))console=ucon64_SNES;
if(argcmp(argc,argv,"-gen"))console=ucon64_GENESIS;
if(argcmp(argc,argv,"-gb"))console=ucon64_GB;
if(argcmp(argc,argv,"-ng"))console=ucon64_NEOGEO;
if(argcmp(argc,argv,"-ngp"))console=ucon64_NEOGEOPOCKET;
if(argcmp(argc,argv,"-nes"))console=ucon64_NES;
if(argcmp(argc,argv,"-pce"))console=ucon64_PCE;
if(argcmp(argc,argv,"-sat"))console=ucon64_SATURN;
if(argcmp(argc,argv,"-dc"))console=ucon64_DC;
if(argcmp(argc,argv,"-cdi"))console=ucon64_CDI;
if(argcmp(argc,argv,"-cd32"))console=ucon64_CD32;
if(argcmp(argc,argv,"-3do"))console=ucon64_REAL3DO;

if(argcmp(argc,argv,"-col"))console=ucon64_SNES;
if(argcmp(argc,argv,"-ip"))console=ucon64_DC;
if(argcmp(argc,argv,"-sam"))console=ucon64_NEOGEO;
if(argcmp(argc,argv,"-stp"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-ins"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-cdrom"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-b2i"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-hex"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-xv64"))console=ucon64_N64;
if(argcmp(argc,argv,"-bot"))console=ucon64_N64;

strcpy(buf,ucon64_rom());
if(!strcmp(strupr(&buf[strlen(buf)-4]),".FDS")&&(n_size(ucon64_rom())%65500)==0)
	console=ucon64_NES;



if(console==ucon64_UNKNOWN)
{
//the same but automatic

	if(supernintendo_probe(argc,argv)!=-1)console=ucon64_SNES;
	else if(segamegadrive_probe(argc,argv)!=-1)console=ucon64_GENESIS;
	else if(nintendo64_probe(argc,argv)!=-1)console=ucon64_N64;
	else if(nes_probe(argc,argv)!=-1)console=ucon64_NES;
	else if(gameboy_probe(argc,argv)!=-1)console=ucon64_GB;
	else if(jaguar_probe(argc,argv)!=-1)console=ucon64_JAGUAR;
	else if(pcengine_probe(argc,argv)!=-1)console=ucon64_PCE;
	else console=ucon64_UNKNOWN;
}

//here could be global overrides


//options which depend on a special console system


switch(console)
{

case ucon64_GB:
	gameboy_main(argc,argv);
break;
case ucon64_GENESIS:
	segamegadrive_main(argc,argv);
break;
case ucon64_SMS:
	sms_main(argc,argv);
break;
case ucon64_JAGUAR:
	jaguar_main(argc,argv);
break;
case ucon64_LYNX:
	lynx_main(argc,argv);
break;
case ucon64_N64:
	nintendo64_main(argc,argv);
break;
case ucon64_NEOGEO:
	neogeo_main(argc,argv);
break;
case ucon64_NES:
	nes_main(argc,argv);
break;
case ucon64_PCE:
	pcengine_main(argc,argv);
break;
case ucon64_PSX:
	playstation_main(argc,argv);
break;
case ucon64_DC:
	dreamcast_main(argc,argv);
break;
case ucon64_SYSTEM16:
	system16_main(argc,argv);
break;
case ucon64_ATARI:
	atari_main(argc,argv);
break;
case ucon64_SNES:
	supernintendo_main(argc,argv);
break;
case ucon64_NEOGEOPOCKET:
	neogeopocket_main(argc,argv);
break;
case ucon64_UNKNOWN:
default:
	n_hexdump(ucon64_rom(),0,n_hexdump_EOF);
	printf("\nUnknown (try to force: -snes, -gen, ...)\n\n");
//	usage_main(argc,argv);
break;
}

return(0);
}


/*
_ __ ________________________________________________________________ __ _
                                                      ___
    .,,,,     .---._ Oo  .::::. .::::. :::   :::    __\__\
    ( oo)__   (¯oo) /..\ ::  :: ::  :: :::   :::    \ / Oo\o  (\(\
   /\_  \__) /\_  \/\_,/ ::  .. ::..:: ::'   ::'    _\\`--_/ o/oO \
   \__)_/   _\__)_/_/    :::::: :::::: ::....::.... \_ \  \  \.--'/
   /_/_/    \ /_/_//     `::::' ::  :: `:::::`:::::: /_/__/   /¯\ \___
 _(__)_)_,   (__)_/  .::::.                      ;::  |_|_    \_/_/\_/\
  o    o      (__)) ,:' `::::::::::::::::::::::::::' (__)_)___(_(_)  ¯¯
     ________  ________  _____ _____________________/   __/_  __/_________
    _\___   /__\___   /_/____/_\    __________     /    ___/  ______     /
   /    /    /    /    /     /  \      \/    /    /     /     /    /    /
  /    /    /         /     /          /    _____/_    /_    /_   _____/_
 /____/    /_________/     /·aBn/fAZ!/nB·_________/_____/_____/_________/
- -- /_____\--------/_____/------------------------------------------ -- -
4 Nodes USRobotics & Isdn Power     All Releases Since Day 0 Are Available
 Snes/Sega/GameBoy/GameGear/Ultra 64/PSX/Jaguar/Saturn/Engine/Lynx/NeoGeo
- -- ---------------------------------------------------------------- -- -
*/
