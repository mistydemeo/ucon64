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

#include "ucon64.h"

#include "aps.h"
#include "ips.h"
#include "ppf.h"
#include "usage.h"
//#include "unzip.h"
#include "snes/snes.h"
#include "gb/gb.h"
#include "jaguar/jaguar.h"
#include "n64/n64.h"
#include "lynx/lynx.h"
#include "sms/sms.h"
#include "nes/nes.h"
#include "genesis/genesis.h"
#include "pce/pce.h"
#include "bsl.h"
#include "xps.h"
#include "neogeo/neogeo.h"
#include "sys16/sys16.h"
#include "atari/atari.h"
#include "ngp/ngp.h"


int ucon64_probe(int argc,char *argv[]);

int main(int argc,char *argv[])
{
register long x;
char buf[4096],buf2[4096];
long console=ucon64_UNKNOWN;
FILE *fh;
int ucon64_argc;
char *ucon64_argv[128];

printf("%s\n",ucon64_TITLE);
printf("Uses code from various people. See 'DEVELOPERS' for more!\n");
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
//	genesis_main(argc,argv);
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

if(argcmp(argc,argv,"-rl"))
{
	rencase(ucon64_rom(),"lwr");
	return(0);
}

if(argcmp(argc,argv,"-ru"))
{
	rencase(ucon64_rom(),"upr");
	return(0);
}

if(argcmp(argc,argv,"-hex"))
{
	hexdump(ucon64_rom(),0,quickftell(ucon64_rom()));
	return(0);
}

if(argcmp(argc,argv,"-ls"))
{
//TODO read dir and then while file ucon64_probe und <con>_nfo

//	mkfid(ucon64_rom());

	return(0);
}

if(argcmp(argc,argv,"-c"))
{
//	if(n_cmp(ucon64_rom(),0,ucon64_file(),0,filecmp_DIFF)!=0)
		printf("ERROR: file not found/out of memory\n");
	return(0);
}

if(argcmp(argc,argv,"-cs"))
{
//	if(n_cmp(ucon64_rom(),0,ucon64_file(),0,filecmp_SIMI)!=0)
                       printf("ERROR: file not found/out of memory\n");
	return(0);
}

if(argcmp(argc,argv,"-find"))
{
	x=0;
	while((x=filegrep(ucon64_rom(),ucon64_file(),x,strlen(ucon64_file())))!=-1)
	{
		hexdump(ucon64_rom(),x,strlen(ucon64_file()));
		x++;
		printf("\n");
	}
	return(0);
}

/*
if(argcmp(argc,argv,"-cdrom"))
{
//	cdromnfo(ucon64_rom(),ucon64_rom());
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_rom();
	ucon64_argc=2;

//	cdinfo_main(ucon64_argc,ucon64_argv);
	return(0);
}
*/

if(argcmp(argc,argv,"-swap"))
{
/*
	strcpy(buf,romname);
	buf[strcspn(buf,".")+1]=0;
	strcat(buf,findlwr(buf)?"rom":"ROM");

	filecopy(romname,0,quickftell(romname),buf,"wb");
//	if(!swapped)fswap(buf,0,(quickftell(buf)-hsize));
	fileswap(buf,0,(quickftell(buf)-hsize));


*/

	fileswap(ucon64_rom(),0,quickftell(ucon64_rom()));
	return(0);
}

if(argcmp(argc,argv,"-pad"))
{
	filepad(ucon64_rom(),0,"ab");
	return(0);
}

if(argcmp(argc,argv,"-ispad"))
{
	unsigned long padded;
	
	if((padded=filepad(ucon64_rom(),0,"rb"))!=-1)
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
	strcat(buf,findlwr(buf)?"tmp":"TMP");
	rename(ucon64_rom(),buf);
	filecopy(buf,512,quickftell(buf),ucon64_rom(),"wb");

	remove(buf);
	return(0);
}
if(argcmp(argc,argv,"-ins"))
{
	strcpy(buf,ucon64_rom());
	buf[strcspn(buf,".")+1]=0;
	strcat(buf,findlwr(buf)?"tmp":"TMP");
	rename(ucon64_rom(),buf);

	if((fh=fopen(ucon64_rom(),"wb"))!=0)
	{
		memset(buf2,0x00,512);
		fwrite(buf2,512,1,fh);
		fclose(fh);
	}
	filecopy(buf,0,quickftell(buf),ucon64_rom(),"ab");
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
//	cips_main(ucon64_rom(),ucon64_file());
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
                                                            
                quickfwrite(buf2,7,50,filebackup(ucon64_rom()));
                
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
                                                            
               quickfwrite(buf2,6,50,filebackup(ucon64_rom()));
                
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
if(argcmp(argc,argv,"-n2gb"))console=ucon64_GB;

strcpy(buf,ucon64_rom());
if(!strcmp(strupr(&buf[strlen(buf)-4]),".FDS")&&(quickftell(ucon64_rom())%65500)==0)
	console=ucon64_NES;



if(console==ucon64_UNKNOWN)
{
//the same but automatic
	console=ucon64_probe(argc,argv);
}

//here could be global overrides


//options which depend on a special console system


switch(console)
{

case ucon64_GB:
	gameboy_main(argc,argv);
break;
case ucon64_GENESIS:
	genesis_main(argc,argv);
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
/*
case ucon64_PSX:
	playstation_main(argc,argv);
break;
*/
/*
case ucon64_DC:
	dreamcast_main(argc,argv);
break;
*/
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
	hexdump(ucon64_rom(),0,hexdump_EOF);
	printf("\nERROR: unknown file %s\n       USE -<CONSOLE> force recognition OPTION\n\n",ucon64_rom());
//	usage_main(argc,argv);
break;
}

return(0);
}


int ucon64_probe(int argc,char *argv[])
{
	int console=ucon64_UNKNOWN;
	
	if(supernintendo_probe(argc,argv)!=-1)console=ucon64_SNES;
	else if(genesis_probe(argc,argv)!=-1)console=ucon64_GENESIS;
	else if(nintendo64_probe(argc,argv)!=-1)console=ucon64_N64;
	else if(nes_probe(argc,argv)!=-1)console=ucon64_NES;
	else if(gameboy_probe(argc,argv)!=-1)console=ucon64_GB;
	else if(jaguar_probe(argc,argv)!=-1)console=ucon64_JAGUAR;
	else if(pcengine_probe(argc,argv)!=-1)console=ucon64_PCE;
	else console=ucon64_UNKNOWN;

	return(console);
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
