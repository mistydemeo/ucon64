/*
    uCON64 1.9.4 by Noisy Belch/Gewalt! (noisyb@gmx.net)
    The ultimate console tool
    Copyright (C) 1999/2000/2001 Noisy Belch/Gewalt! (noisyb@gmx.net)

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
//#define LINUX

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

#include "noisette.h"

int isstrlwr(char *str)
{
//searches the string for ANY lowercase char
	char *str2;

	if(!(str2=strrchr(str,'/')))str2=str;
	else str2++;

	while(*str2)
	{                
		if(isalpha(*str2)&&islower(*str2))return(islower('a'));
		str2++;
	}
	return(islower('A'));
}

int isstrhex(char *code)
{
	while(*code)
	{
		if(!isxdigit(*code))return(islower('A'));
		code++;
	}
	return(islower('a'));
}


int mkfid(char name)
{
	return(0);
}

#define KNOWN 9999
#define UNKNOWN -1
#define GB 1
#define GENESIS 2
#define SMS 3
#define JAGUAR 4
#define LYNX 5
#define N64 6
#define NEOGEO 7
#define NES 8
#define PCE 9
#define PSX 10
#define SNES 11
#define SATURN 12
#define DC 13
#define CD32	14
#define CDI	15
#define REAL3DO	16
#define ATARI	17
#define SYSTEM16	18

#define MBIT 131072

char *arg;

char title(void)
{
#ifdef MSDOS
char *version="64 1.9.4 msdos 1999/2000/2001";
#endif
#ifdef LINUX
char *version="64 1.9.4 linux 1999/2000/2001";
#endif
	printf("uCON%s by Noisy Belch/Gewalt! (noisyb@gmx.net)\n",version);
	printf("This may be freely redistributed under the terms of the GNU Public License\n\n");
}

#include "usage.h"
//#include "unzip.h"
#include "gamecodes.h"
#include "transfer.h"
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
#include "aps.h"
#include "ips.h"
#include "ppf.h"
//#include "xps.h"
#include "neogeo.h"
#include "sys16.h"
#include "atari.h"
#include "dc.h"


int main(int argc, char* argv[])
{
register long x;
char buf[4096],buf2[4096],name[4096],file[4096];
long console=UNKNOWN;//used to buf2 the consoletype
//int backup=1;
char *romname={""};
FILE *fh;

//strcpy(arg,argv[0]);
arg=argv[0];
fgauge(START,0);
name[0]=file[0]=0;
console=UNKNOWN;
if(argc<2||(argv[1][0]=='-'&&argv[1][1]=='-'&&(argv[1][2]=='?'||argv[1][2]=='h')))
{
	title();
	usage(0);
}

for(x=1;x<argc;x++)if(argv[x][0]=='-')
{
//database options

	strcpy(buf,argv[x]);
	
        if(!strdcmp(buf,"-faq"))
        {
		printf("<pre><tt>\n");
		title();
        	usage(FAQ);
        }
        if(!strdcmp(buf,"-hh"))
        {
		printf("<pre><tt>\n");
		title();
        	usage(FAQ);
        }
                
	title();
        
        if(!strdcmp(buf,"-db"))
	{
	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"0");//original atari_db.h size
	atari(name,DBNFO,buf2);

	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"5196");//original nes_db.h size
	nes(name,DBNFO,buf2);

	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"515");//original pce_db.h size
	pcengine(name,DBNFO,buf2);

	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"0");//original jaguar_db.h size
	jaguar(name,DBNFO,buf2);

	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"0");//original sys16_db.h size
	system16(name,DBNFO,buf2);

	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"0");//original commodore_db.h size
//	commodore(name,DBNFO,buf2);

	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"0");//original neogeo_db.h size
	neogeo(name,DBNFO,buf2);

	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"324");//original sms_db.h size
	sms(name,DBNFO,buf2);

	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,"0");//original lynx_db.h size
	lynx(name,DBNFO,buf2);
	printf("\n");
		exit(0);
	}

	if(!strdcmp(buf,"-dbv"))
	{
//like -db but view also the contents of the db's
	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
		atari(name,DBNFO2,buf2);
		nes(name,DBNFO2,buf2);
		pcengine(name,DBNFO2,buf2);
		system16(name,DBNFO2,buf2);
//		commodore(name,DBNFO2,buf2);
		neogeo(name,DBNFO2,buf2);
		sms(name,DBNFO2,buf2);
		lynx(name,DBNFO2,buf2);
		printf("\n");
		exit(0);
	}

}

title();

for(x=1;x<argc;x++)if(argv[x][0]!='-')
{
	if(!name[0])
	{
		strcpy(buf,argv[x]);
		strcpy(name,buf);
	}
	else if(!file[0])
	{
		strcpy(buf,argv[x]);
		strcpy(file,buf);
	}
}





if(!name[0])usage(0);

for(x=1;x<argc;x++)if(argv[x][0]=='-')
{
	strcpy(buf,argv[x]);

//console override?

	if(!strdcmp(buf,"-ata"))console=ATARI;
	if(!strdcmp(buf,"-s16"))console=SYSTEM16;
	if(!strdcmp(buf,"-n64"))console=N64;
	if(!strdcmp(buf,"-psx"))console=PSX;
	if(!strdcmp(buf,"-psx2"))console=PSX;
	if(!strdcmp(buf,"-lynx"))console=LYNX;
	if(!strdcmp(buf,"-jag"))console=JAGUAR;
	if(!strdcmp(buf,"-sms"))console=SMS;
	if(!strdcmp(buf,"-snes"))console=SNES;
	if(!strdcmp(buf,"-gen"))console=GENESIS;
	if(!strdcmp(buf,"-gb"))console=GB;
	if(!strdcmp(buf,"-ng"))console=NEOGEO;
	if(!strdcmp(buf,"-nes"))console=NES;
	if(!strdcmp(buf,"-pce"))console=PCE;
	if(!strdcmp(buf,"-sat"))console=SATURN;
	if(!strdcmp(buf,"-dc"))console=DC;
	if(!strdcmp(buf,"-cdi"))console=CDI;
	if(!strdcmp(buf,"-cd32"))console=CD32;
	if(!strdcmp(buf,"-3do"))console=REAL3DO;

                        
	if(!strdcmp(buf,"-col"))console=SNES;
	if(!strdcmp(buf,"-ip"))console=DC;
	
	if(!strdcmp(buf,"-sam"))console=NEOGEO;

	if(!strdcmp(buf,"-stp"))console=KNOWN;
	if(!strdcmp(buf,"-ins"))console=KNOWN;
	if(!strdcmp(buf,"-cdrom"))console=KNOWN;
	if(!strdcmp(buf,"-b2i"))console=KNOWN;
	if(!strdcmp(buf,"-hex"))console=KNOWN;

	if(!strdcmp(buf,"-xv64"))console=N64;
	if(!strdcmp(buf,"-bot"))console=N64;
//...
}
strcpy(buf,name);
strupr(buf);
if(!strcmp(&buf[strlen(buf)-4],".FDS")&&(fsize(romname,0)%65500)==0)console=NES;







if(console==UNKNOWN)
{
//the same but automatic
	strcpy(buf2,"????????????????");//buf2
	buf2[16]=0;
	strcat(buf2,file);

	if(supernintendo(name,0,buf2)!=-1)console=SNES;
	else if(segamegadrive(name,0,buf2)!=-1)console=GENESIS;
	else if(nintendo64(name,0,buf2)!=-1)console=N64;
	else if(nes(name,0,buf2)!=-1)console=NES;
	else if(gameboy(name,0,buf2)!=-1)console=GB;
	else if(jaguar(name,0,buf2)!=-1)console=JAGUAR;
//	else if(pcengine(name,0,buf2)!=-1)console=PCE;
	else console=UNKNOWN;
}

//here could be global overrides


for(x=1;x<argc;x++)if(argv[x][0]=='-')
{
//options which dont depend on a special console system

	strcpy(buf,argv[x]);
	
	if(!strdcmp(buf,"-b2i")){bin2iso(name);exit(0);}
	if(!strdcmp(buf,"-cdrom")){fcdromdump(name,name);exit(0);}
	if(!strdcmp(buf,"-hex")){fhexdump(name,0,fsize(name,0));exit(0);}
//	if(!strdcmp(buf,"-id")){mkfid(name);exit(0);}
	if(!strdcmp(buf,"-rn")){fstrlwr(name);exit(0);}
	if(!strdcmp(buf,"-pad")){fpad(name,0,"ab");exit(0);}
	if(!strdcmp(buf,"-ispad"))
	{
unsigned long padded;
		if((padded=fpad(name,0,"rb"))!=-1)
		{
			if(!padded)printf("Padded: No\n");
			else printf("Padded: Maybe, %ld bytes (%.4f Mb)\n",padded,(float)padded/MBIT);
		}
		printf("\n");exit(0);
	}
	if(!strdcmp(buf,"-stp"))
	{
		strcpy(buf,name);
		buf[strcspn(buf,".")+1]=0;
		strcat(buf,isstrlwr(buf)?"tmp":"TMP");
		rename(name,buf);

		fcopy(buf,512,name,"wb");
		remove(buf);
		exit(0);
	}
	if(!strdcmp(buf,"-ins"))
	{
		strcpy(buf,name);
		buf[strcspn(buf,".")+1]=0;
		strcat(buf,isstrlwr(buf)?"tmp":"TMP");
		rename(name,buf);

		if((fh=fopen(name,"wb"))!=0)
		{
			memset(buf2,0x00,512);
			fwrite(buf2,512,1,fh);
			fclose(fh);

		}

		fcopy(buf,0,name,"ab");
		remove(buf);
		exit(0);
	}
	if(!strdcmp(buf,"-c"))
	{
		if(fcmp(name,0,file,0,DIFF)!=0)
			printf("ERROR: file not found/out of memory\n");
		exit(0);
	}
	if(!strdcmp(buf,"-cs"))
	{
		if(fcmp(name,0,file,0,SIMI)!=0)
                        printf("ERROR: file not found/out of memory\n");
		exit(0);
	}
//	if(!strdcmp(buf,"-bak"))backup=1;
	if(!strdcmp(buf,"-find"))
	{
		x=0;
		while((x=fstrstr(name,file,x,strlen(file)))!=-1)
		{
			fhexdump(name,x,strlen(file));
			x++;
			printf("\n");
		}
		exit(0);
	}
	if(!strdcmp(buf,"-swap")){fswap(name,0,fsize(name,0));exit(0);}
/*	if(!strdcmp(buf,"-b")){bsl(name,file);exit(0);}
	if(!strdcmp(buf,"-i")){ips(name,file);exit(0);}
*/	if(!strdcmp(buf,"-mka")){caps(name,file);exit(0);}
	if(!strdcmp(buf,"-mkppf")){cppf(name,file);exit(0);}
	if(!strdcmp(buf,"-mki")){cips(name,file);exit(0);}
	if(!strdcmp(buf,"-nppf"))
	{
		strcpy(buf2,file);
strcat(buf2,"\
                                                            ");
                                                            
                fstrncpy(name,buf2,6,50);
                
		exit(0);
	}
	if(!strdcmp(buf,"-na"))
	{
		strcpy(buf2,file);
strcat(buf2,"\
                                                            ");
                                                            
                fstrncpy(name,buf2,7,50);
                
		exit(0);
	}
}

//if(backup!=0)	fbackup(name);

if(console==GB)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
		if(!strdcmp(buf,"-hd"))buf2[0]='1';
		if(!strdcmp(buf,"-nhd"))buf2[0]='0';
	}
	buf2[16]=0;
	strcat(buf2,file);

	gameboy(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

		if(!strdcmp(buf,"-n"))gameboy(name,NAME,buf2);
		if(!strdcmp(buf,"-chk"))gameboy(name,CHK,buf2);
		if(!strdcmp(buf,"-mgd"))gameboy(name,MGD,buf2);
		if(!strdcmp(buf,"-ssc"))gameboy(name,SSC,buf2);
	}
}
if(console==GENESIS)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
		if(!strdcmp(buf,"-hd"))buf2[0]='1';
		if(!strdcmp(buf,"-nhd"))buf2[0]='0';
	}
	buf2[16]=0;
	strcat(buf2,file);


	segamegadrive(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

		if(!strdcmp(buf,"-smd"))segamegadrive(name,SMD,buf2);
		if(!strdcmp(buf,"-mgd"))segamegadrive(name,MGH,buf2);
		if(!strdcmp(buf,"-s"))segamegadrive(name,SPLIT,buf2);
		if(!strdcmp(buf,"-j"))segamegadrive(name,JOIN,buf2);
		if(!strdcmp(buf,"-p"))segamegadrive(name,PAD,buf2);
		if(!strdcmp(buf,"-n"))segamegadrive(name,NAME,buf2);
		if(!strdcmp(buf,"-n2"))segamegadrive(name,NAME2,buf2);
		if(!strdcmp(buf,"-chk"))segamegadrive(name,CHK,buf2);
		if(!strdcmp(buf,"-1991"))segamegadrive(name,SEGA1991,buf2);
	}
}
if(console==SMS)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
		if(!strdcmp(buf,"-hd"))buf2[0]='1';
		if(!strdcmp(buf,"-nhd"))buf2[0]='0';
	}
	buf2[16]=0;
	strcat(buf2,file);

	sms(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

		if(!strdcmp(buf,"-mgd"))sms(name,BIN,buf2);
		if(!strdcmp(buf,"-smd"))sms(name,FFE,buf2);
	}
}
if(console==JAGUAR)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
		if(!strdcmp(buf,"-hd"))buf2[0]='1';
		if(!strdcmp(buf,"-nhd"))buf2[0]='0';
	}
	buf2[16]=0;
	strcat(buf2,file);

	jaguar(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

//		if(!strdcmp(buf,"-"))
	}
}
if(console==LYNX)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
	}
	buf2[16]=0;
	strcat(buf2,file);

	lynx(name,NFO,buf2);
//	for(x=1;x<argc;x++)if(argv[x][0]=='-')
//	{
//		strcpy(buf,argv[x]);
//
//		if(!strdcmp(buf,"-"))
//	}
}
if(console==N64)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
		if(!strdcmp(buf,"-swp"))buf2[0]='1';
		if(!strdcmp(buf,"-nswp"))buf2[0]='0';
	}
	buf2[16]=0;
	strcat(buf2,file);

	nintendo64(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

		if(!strdcmp(buf,"-sram"))
		{
			if(nintendo64(name,SRAM,buf2)==-1)
printf("ERROR: check if ROM has 1310720 bytes and SRAMFILE has 512 bytes\n");
printf("       or the ROM is too short to calculate checksum\n");
		}
		if(!strdcmp(buf,"-v64"))nintendo64(name,V64,buf2);
		if(!strdcmp(buf,"-n"))nintendo64(name,NAME,buf2);
		if(!strdcmp(buf,"-f"))nintendo64(name,PNF,buf2);
		if(!strdcmp(buf,"-chk"))
		{
			if(nintendo64(name,CHK,buf2)==-1)printf("ERROR: the ROM is too short for checksum\n");
		}
		if(!strdcmp(buf,"-z64"))nintendo64(name,Z64,buf2);
		if(!strdcmp(buf,"-bot"))nintendo64(name,BOT,buf2);
		if(!strdcmp(buf,"-usms"))nintendo64(name,USMS,buf2);
		if(!strdcmp(buf,"-xv64"))
		{
			if(nintendo64(name,XV64,buf2)==-1)
{
//problems with the parallel port
	printf("\
ERROR:  please check cables and connection\n\
\tturn off/on the backup unit\n\
\tsplitted ROMs are not supported\n\
\tuse $FILE={3bc,378,278,...} to specify your port\n\
");
#ifdef LINUX
	printf("\
\tset the port to SPP (Standard, Normal) mode in your bios\n");
#endif
}

		}
		if(!strdcmp(buf,"-xvjr"))
		{
			if(nintendo64(name,XVJR,buf2)==-1)
{
//problems with the parallel port
	printf("\
ERROR:  please check cables and connection\n\
\tturn off/on the backup unit\n\
\tsplitted ROMs are not supported\n\
\tuse $FILE={3bc,378,278,...} to specify your port\n\
");
#ifdef LINUX
	printf("\
\tset the port to SPP (Standard, Normal) mode in your bios\n");
#endif
}

		}
	}
}
if(console==NEOGEO)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
	}
	buf2[16]=0;
	strcat(buf2,file);

	neogeo(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

		if(!strdcmp(buf,"-sam"))
		{
			if(neogeo(name,SAM,buf2)==-1)
			{
                        printf("ERROR: SAM header seems to be corrupt\n");
			}
		}
	}
}
if(console==NES)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
		if(!strdcmp(buf,"-hd"))buf2[0]='1';
		if(!strdcmp(buf,"-nhd"))buf2[0]='0';
	}
	buf2[16]=0;
	strcat(buf2,file);

	nes(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

                if(!strdcmp(buf,"-ffe"))nes(name,FFE,buf2);
		if(!strdcmp(buf,"-fds"))nes(name,FDS,buf2);
		if(!strdcmp(buf,"-ines"))nes(name,INES,buf2);
		if(!strdcmp(buf,"-ineshd"))nes(name,INESHD,buf2);
		if(!strdcmp(buf,"-pas")||!strdcmp(buf,"-s"))nes(name,PASO,buf2);
	}
}
if(console==PCE)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
		if(!strdcmp(buf,"-hd"))buf2[0]='1';
		if(!strdcmp(buf,"-nhd"))buf2[0]='0';
	}
	buf2[16]=0;
	strcat(buf2,file);

	pcengine(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

		if(!strdcmp(buf,"-smg"))pcengine(name,MG,buf2);
		if(!strdcmp(buf,"-mgd"))pcengine(name,BIN,buf2);
	}
}
if(console==PSX)
{
	playstation(name,NFO);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

//		if(!strdcmp(buf,"-"))
	}
}
if(console==DC)
{
	dreamcast(name,NFO);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

		if(!strdcmp(buf,"-ip"))dreamcast(name,IP);
	}
}
if(console==SYSTEM16)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
	}
	buf2[16]=0;
	strcat(buf2,file);

	system16(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

//		if(!strdcmp(buf,"-"))
	}
}
if(console==ATARI)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
	}
	buf2[16]=0;
	strcat(buf2,file);

	atari(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

//		if(!strdcmp(buf,"-"))
	}
}
if(console==SNES)
{
	strcpy(buf2,"????????????????");//buf2
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);
	
		if(!strdcmp(buf,"-hd"))buf2[0]='1';
		if(!strdcmp(buf,"-nhd"))buf2[0]='0';
		if(!strdcmp(buf,"-hi"))buf2[1]='1';
		if(!strdcmp(buf,"-nhi"))buf2[1]='0';
	}
	buf2[16]=0;
	strcat(buf2,file);


	supernintendo(name,NFO,buf2);
	for(x=1;x<argc;x++)if(argv[x][0]=='-')
	{
		strcpy(buf,argv[x]);

		if(!strdcmp(buf,"-smc"))supernintendo(name,SMC,buf2);
		if(!strdcmp(buf,"-fig"))supernintendo(name,FIG,buf2);
		if(!strdcmp(buf,"-swc"))supernintendo(name,SWC,buf2);
		if(!strdcmp(buf,"-mgd"))supernintendo(name,MGH,buf2);
		if(!strdcmp(buf,"-gd3"))supernintendo(name,GD3,buf2);
		if(!strdcmp(buf,"-gdf"))supernintendo(name,GDF,buf2);
		if(!strdcmp(buf,"-s"))supernintendo(name,SPLIT,buf2);
		if(!strdcmp(buf,"-j"))supernintendo(name,JOIN,buf2);
		if(!strdcmp(buf,"-p"))supernintendo(name,PAD,buf2);
		if(!strdcmp(buf,"-k"))supernintendo(name,CRK,buf2);
		if(!strdcmp(buf,"-f"))supernintendo(name,PNF,buf2);
		if(!strdcmp(buf,"-l"))supernintendo(name,SLW,buf2);
		if(!strdcmp(buf,"-n"))supernintendo(name,NAME,buf2);
		if(!strdcmp(buf,"-col"))supernintendo(name,COL,buf2);
		if(!strdcmp(buf,"-chk"))supernintendo(name,CHK,buf2);
		if(!strdcmp(buf,"-gg"))supernintendo(name,GAMEGENIE,buf2);
	}
}
if(console==UNKNOWN)
{
	fhexdump(name,0,fsize(name,0));
	printf("\nUnknown (try to force: -snes, -gen, ...)\n\n");
//	usage(0);
}






for(x=1;x<argc;x++)if(argv[x][0]=='-')
{
//options which dont depend on a special console system

	strcpy(buf,argv[x]);
	
/*	if(!strdcmp(buf,"-cdrom")){fcdromdump(name,name);exit(0);}
	if(!strdcmp(buf,"-hex")){fhexdump(name,0,fsize(name,0));exit(0);}*/
//	if(!strdcmp(buf,"-id")){mkfid(name);exit(0);}
/*	if(!strdcmp(buf,"-pad")){fpad(name,0,"ab");exit(0);}
	if(!strdcmp(buf,"-stp"))
	{
		strcpy(buf,name);
		buf[strcspn(buf,".")+1]=0;
		strcat(buf,isstrlwr(buf)?"tmp":"TMP");
		rename(name,buf);

		fcopy(buf,512,name,"wb");
		remove(buf);
		exit(0);
	}
	if(!strdcmp(buf,"-ins"))
	{
		strcpy(buf,name);
		strext(buf,".tmp");
		rename(name,buf);

		if((fh=fopen(name,"wb"))!=0)
		{
			memset(buf2,0x00,512);
			fwrite(buf2,512,1,fh);
			fclose(fh);

		}

		fcopy(buf,0,name,"ab");
		remove(buf);
		exit(0);
	}
	if(!strdcmp(buf,"-c"))
	{
		if(!fcmp(name,0,file,0,DIFF))
			printf("%s==%s\n\n",name,file);
		exit(0);
	}
	if(!strdcmp(buf,"-cs"))
	{
		if(!fcmp(name,0,file,0,SIMI))
			printf("%s==%s\n\n",name,file);
		exit(0);
	}
	if(!strdcmp(buf,"-bak"))backup=1;
	if(!strdcmp(buf,"-find"))
	{
		x=0;
		while((x=fstrstr(name,file,x,strlen(file)))!=-1)
		{
			fhexdump(name,x,strlen(file));
			x++;
			printf("\n");
		}
		exit(0);
	}
	if(!strdcmp(buf,"-swap")){fswap(name,0,fsize(name,0));exit(0);}
*/	if(!strdcmp(buf,"-b"))
	{
		if(bsl(name,file)!=0)printf("ERROR: failed\n");
		exit(0);
	}
	if(!strdcmp(buf,"-i"))
	{
		if(ips(name,file)!=0)printf("ERROR: failed\n");
		exit(0);
	}
	if(!strdcmp(buf,"-a"))
	{
		if(aps(name,file)!=0)printf("ERROR: failed\n");
		exit(0);
	}
	if(!strdcmp(buf,"-ppf"))
	{
		if(ppf(name,file)!=0)printf("ERROR: failed\n");
		exit(0);
	}
}






exit(0);
}
