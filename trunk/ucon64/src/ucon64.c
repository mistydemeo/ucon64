/*
uCON64 1.9.6 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2001 dbjh


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

//#include "unzip.h"
#include "snes/snes.h"
#include "gb/gb.h"
#include "gba/gba.h"
#include "jaguar/jaguar.h"
#include "n64/n64.h"
#include "lynx/lynx.h"
#include "sms/sms.h"
#include "nes/nes.h"
#include "genesis/genesis.h"
#include "pce/pce.h"
#include "neogeo/neogeo.h"
#include "sys16/sys16.h"
#include "atari/atari.h"
#include "ngp/ngp.h"
#include "coleco/coleco.h"
#include "vboy/vboy.h"
#include "vectrex/vectrex.h"
#include "wswan/wswan.h"
#include "intelli/intelli.h"

#include "backup/fig.h"
#include "backup/swc.h"

#include "patch/aps.h"
#include "patch/ips.h"
#include "patch/ppf.h"
#include "patch/bsl.h"
#include "patch/xps.h"


#define MAXBUFSIZE 32768


int main(int argc,char *argv[])
{
/*if(	!argcmp(argc, argv, "-ls") &&
	!argcmp(argc, argv, "-lsv")
)*/
{
  printf("%s\n",ucon64_TITLE);
  printf("Uses code from various people. See 'DEVELOPERS' for more!\n");
  printf("This may be freely redistributed under the terms of the GNU Public License\n\n");
}
  if (argc<2 ||
      argcmp(argc, argv, "-h") ||
      argcmp(argc, argv, "--help") ||
      argcmp(argc, argv, "-?"))
  {
    ucon64_usage(argc,argv);
    return 0;
  }
/*
if(argcmp(argc, argv, "-sh"))
{
//TODO shell modus
	printf(
"                                                            ___\n"
"   .,,,,     .---._ Oo  .::::. ::  :: .::::: :::   :::    __\\__\\\n"
"   ( oo)__   ( oo) /..\\ ::  `' ::  :: ::     :::   :::    \\ / Oo\\o  (\\(\\\n"
"  /\\_  \\__) /\\_  \\/\\_,/ `::::. :::::: ::::.  ::'   ::'    _\\\\`--_/ o/oO \\\n"
"  \\__)_/   _\\__)_/_/    ..  :: ::  :: ::     ::....::.... \\_ \\  \\  \\.--'/\n"
"  /_/_/    \\ /_/_//     `::::' ::  :: `::::: `:::::`:::::: /_/__/   / \\ \\___\n"
"_(__)_)_,   (__)_/  .::::.                             ;::  |_|_    \\_/_/\\_/\\\n"
" o    o      (__)) ,:' `:::::::::::::::::::::::::::::::::' (__)_)   (_(_)\n"
"\n"
"\nShell modus active. Enter q to exit\n\n"
"BEFORE:           %s -help\n"
"NOW: (enter just) help\n\n"
,ucon64_name());

	for(;;)
	{
		
	}
	return(0);
}
*/
  return(ucon64_main(argc,argv));
}

int ucon64_main(int argc,char *argv[])
{
  long x, y = 0, console=ucon64_UNKNOWN;
  int ucon64_argc;
  struct dirent *ep;
  struct stat puffer;
  DIR *dp;
	struct ucon64_DB db;
  char buf[MAXBUFSIZE], buf2[4096], buf3[4096], *ucon64_argv[128], *forceargs[] =
  {
    "",
    "-gb",
    "-gen",
    "-sms",
    "-jag",
    "-lynx",
    "-n64",
    "-ng",
    "-nes",
    "-pce",
    "-psx",
    "-psx2",
    "-snes",
    "-sat",
    "-dc",
    "-cd32",
    "-cdi",
    "-3do",
    "-ata",
    "-s16",
    "-ngp",
    "-gba",
    "-vec",
    "-vboy",
    "-swan",
    "-coleco",
    "-int"
  };
#ifdef	__UNIX__
  uid_t uid;
  gid_t gid;
#endif


//find out which console type


if(argcmp(argc,argv,"-ata"))console=ucon64_ATARI;
if(argcmp(argc,argv,"-s16"))console=ucon64_SYSTEM16;
if(argcmp(argc,argv,"-n64"))console=ucon64_N64;
if(argcmp(argc,argv,"-psx"))console=ucon64_PSX;
if(argcmp(argc,argv,"-psx2"))console=ucon64_PSX2;
if(argcmp(argc,argv,"-lynx"))console=ucon64_LYNX;
if(argcmp(argc,argv,"-jag"))console=ucon64_JAGUAR;
if(argcmp(argc,argv,"-sms"))console=ucon64_SMS;
if(argcmp(argc,argv,"-snes"))console=ucon64_SNES;
if(argcmp(argc,argv,"-gen"))console=ucon64_GENESIS;
if(argcmp(argc,argv,"-gb"))console=ucon64_GB;
if(argcmp(argc,argv,"-gba"))console=ucon64_GBA;
if(argcmp(argc,argv,"-ng"))console=ucon64_NEOGEO;
if(argcmp(argc,argv,"-ngp"))console=ucon64_NEOGEOPOCKET;
if(argcmp(argc,argv,"-nes"))console=ucon64_NES;
if(argcmp(argc,argv,"-pce"))console=ucon64_PCE;
if(argcmp(argc,argv,"-sat"))console=ucon64_SATURN;
if(argcmp(argc,argv,"-dc"))console=ucon64_DC;
if(argcmp(argc,argv,"-cdi"))console=ucon64_CDI;
if(argcmp(argc,argv,"-cd32"))console=ucon64_CD32;
if(argcmp(argc,argv,"-3do"))console=ucon64_REAL3DO;
if(argcmp(argc,argv,"-coleco"))console=ucon64_COLECO;
if(argcmp(argc,argv,"-vboy"))console=ucon64_VIRTUALBOY;
if(argcmp(argc,argv,"-swan"))console=ucon64_WONDERSWAN;
if(argcmp(argc,argv,"-vec"))console=ucon64_VECTREX;
if(argcmp(argc,argv,"-int"))console=ucon64_INTELLI;

if(argcmp(argc,argv,"-col"))console=ucon64_SNES;
if(argcmp(argc,argv,"-ip"))console=ucon64_DC;
if(argcmp(argc,argv,"-sam"))console=ucon64_NEOGEO;
if(argcmp(argc,argv,"-stp"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-ins"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-cdrom"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-b2i"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-hex"))console=ucon64_KNOWN;
if(argcmp(argc,argv,"-xv64"))console=ucon64_N64;
if(argcmp(argc,argv,"-xdjr"))console=ucon64_N64;
if(argcmp(argc,argv,"-bot"))console=ucon64_N64;
if(argcmp(argc,argv,"-n2gb"))console=ucon64_GB;
if(argcmp(argc,argv,"-xfal"))console=ucon64_GBA;
if(argcmp(argc,argv,"-xswc"))console=ucon64_SNES;
if(argcmp(argc,argv,"-xswcs"))console=ucon64_SNES;
if(argcmp(argc,argv,"-swcs"))console=ucon64_SNES;


if(argcmp(argc,argv,"-db"))
{
	printf(
		"Database: %ld known ROMs in ucon64_db.c (%+ld)\n\n"
		,ucon64_dbsize(console)
		,ucon64_dbsize(console)-ucon64_DBSIZE
	);

	printf("TIP: %s -db -nes would show only the number of known NES ROMs\n\n",ucon64_name());
	return(0);
}

if(argcmp(argc,argv,"-dbs"))
{
	sscanf(ucon64_rom(), "%lx", &x);

	ucon64_dbsearch(	&db
				,console
				,x
	);

	printf("%s\n",db.name);
	printf("%s\n",db.maker);
	printf("%s\n",db.country);
	printf("%s\n\n",db.misc);

	printf("TIP: %s -dbs -nes would search only for a NES ROM\n\n",ucon64_name());

	return(0);
}

if(argcmp(argc,argv,"-dbv"))
{
	ucon64_dbview(console);

	printf("\nTIP: %s -db -nes would view only NES ROMs\n\n",ucon64_name());
	return(0);
}

  if (!strlen(ucon64_rom()))
  {
    ucon64_usage(argc,argv);
    return 0;
  }

  if (strlen( ucon64_file() ))
  {
    strcpy(buf, ucon64_file());
    sscanf(buf, "%x", &ucon64_parport);
  }

#ifdef BACKUP
  if (!(ucon64_parport = parport_probe(ucon64_parport)))
    ;
/*
    printf("ERROR: no parallel port 0x%s found\n\n",strupr(buf));
  else
    printf("0x%x\n\n",ucon64_parport);
*/

#ifdef __UNIX__
/*
  Some code needs us to switch to the real uid and gid. However, other code needs access to
  I/O ports other than the standard printer port registers. We just do an iopl(3) and all
  code should be happy. Using iopl(3) enables users to run all code without being root (of
  course with the ucon64 executable setuid root). Anyone a better idea?
*/
#ifdef __linux__
  if (iopl(3) == -1)
  {
    fprintf(stderr, "Could not set the I/O privilege level to 3\n"
                    "(This program needs root privileges)\n");
    exit(1);
  }
#endif

  uid = getuid();
  if (setuid(uid) == -1)
  {
    fprintf(stderr, "Could not set uid\n");
    exit(1);
  }
  gid = getgid();                               // This shouldn't be necessary if `make install'
  if (setgid(gid) == -1)                        //  was used, but just in case (root did `chmod +s')
  {
    fprintf(stderr, "Could not set gid\n");
    exit(1);
  }
#endif

#endif

  if (argcmp(argc, argv, "-crc"))
  {
    printf("Checksum: %08lx\n\n", fileCRC32(ucon64_rom(), 0));
    return 0;
  }

if(argcmp(argc,argv,"-crchd"))
{
	printf("Checksum: %08lx\n\n",fileCRC32(ucon64_rom(),512));
	return(0);
}


if(argcmp(argc,argv,"-rl"))
{
	renlwr(ucon64_rom());
	return(0);
}

if(argcmp(argc,argv,"-ru"))
{
	renupr(ucon64_rom());
	return(0);
}

if(argcmp(argc,argv,"-hex"))
{
	filehexdump(ucon64_rom(),0,quickftell(ucon64_rom()));
	return(0);
}

if(argcmp(argc,argv,"-ls"))
{
if(access( ucon64_rom() ,R_OK)==-1 ||
(dp=opendir( ucon64_rom() ))==NULL)return(-1);

chdir( ucon64_rom() );

while((ep=readdir(dp))!=0)
{
	if(!stat(ep->d_name,&puffer))
	{
		if(S_ISREG(puffer.st_mode)==1)
		{
			strftime(buf,13,"%b %d %H:%M",localtime(&puffer.st_mtime));

	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]="-ls";
	ucon64_argv[2]=ep->d_name;
	ucon64_argc=3;
			ucon64_probe(ucon64_argc,ucon64_argv);
			printf("%10ld %s %s\n",puffer.st_size,buf,ep->d_name);
		}
	}
}
(void)closedir(dp);

return(0);
}

if(argcmp(argc,argv,"-lsv"))
{
if(access( ucon64_rom() ,R_OK)==-1 ||
(dp=opendir( ucon64_rom() ))==NULL)return(-1);

chdir( ucon64_rom() );

while((ep=readdir(dp))!=0)
{
	if(!stat(ep->d_name,&puffer))
	{
		if(S_ISREG(puffer.st_mode)==1)
		{
			sprintf(buf,"%s %s",argv[0],ep->d_name);
			system(buf);
		}
	}
}
(void)closedir(dp);

	return(0);
}

if(argcmp(argc,argv,"-rrom"))
{
if(access( ucon64_rom() ,R_OK)==-1 ||
(dp=opendir( ucon64_rom() ))==NULL)return(-1);

chdir( ucon64_rom() );

while((ep=readdir(dp))!=0)
{
	if(!stat(ep->d_name,&puffer))
	{
		if(S_ISREG(puffer.st_mode)==1)
		{
			sprintf(buf,"%s %s",argv[0],ep->d_name);
			system(buf);
		}
	}
}
(void)closedir(dp);

	return(0);
}

if(argcmp(argc,argv,"-c"))
{
	if(filefile(ucon64_rom(),0,ucon64_file(),0,FALSE)==-1)
		printf("ERROR: file not found/out of memory\n");
	return(0);
}

if (argcmp(argc, argv, "-cs"))
{
  if (filefile(ucon64_rom(), 0, ucon64_file(), 0, TRUE) == -1)
    printf("ERROR: file not found/out of memory\n");
  return 0;
}

if(argcmp(argc,argv,"-find"))
{
	x=0;
	y=quickftell(ucon64_rom());
	while((x=filencmp2(ucon64_rom(),x,y,ucon64_file(),strlen(ucon64_file()),'?'))!=-1)
	{
		filehexdump(ucon64_rom(),x,strlen(ucon64_file()));
		x++;
		printf("\n");
	}
	return(0);
}

if(argcmp(argc,argv,"-swap"))
{
	fileswap(filebackup(ucon64_rom()),0,quickftell(ucon64_rom()));
	return(0);
}

if(argcmp(argc,argv,"-pad"))
{
	filepad(ucon64_rom(),0,MBIT);
	return(0);
}

if(argcmp(argc,argv,"-padhd"))
{
	filepad(ucon64_rom(),512,MBIT);
	return(0);
}

if(argcmp(argc,argv,"-ispad"))
{
	unsigned long padded;

	if((padded=filetestpad(ucon64_rom()))!=-1)
	{
		if(!padded)printf("Padded: No\n");
		else printf("Padded: Maybe, %ld Bytes (%.4f Mb)\n",padded,(float)padded/MBIT);
	}
	printf("\n");
	return(0);
}



if(argcmp(argc,argv,"-strip"))
{
	truncate(ucon64_rom(),quickftell(ucon64_rom())-atol(ucon64_file()));
	return(0);
}



if(argcmp(argc,argv,"-stp"))
{
	strcpy(buf,filebackup(ucon64_rom()));
	newext(buf,".TMP");
	rename(ucon64_rom(),buf);

	filecopy(buf,512,quickftell(buf),ucon64_rom(),"wb");
	remove(buf);
	return(0);
}

if(argcmp(argc,argv,"-ins"))
{
	strcpy(buf,filebackup(ucon64_rom()));
	newext(buf,".TMP");
	rename(ucon64_rom(),buf);

        memset(buf2,0,512);
	quickfwrite(buf2,0,512,ucon64_rom(),"wb");

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

	return(ips_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-a"))
{
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]="-f";
	ucon64_argv[2]=ucon64_rom();
	ucon64_argv[3]=ucon64_file();
	ucon64_argc=4;

	return(n64aps_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-ppf"))
{
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_rom();
	ucon64_argv[2]=ucon64_file();
	ucon64_argc=3;

	return(applyppf_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-mki"))
{
	cips(ucon64_rom(),ucon64_file());
/*
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_file();
	ucon64_argv[2]=ucon64_rom();
	ucon64_argc=3;

	return(cips_main(ucon64_argc,ucon64_argv));
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

	return(n64caps_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-na"))
{
	strcpy(buf2,ucon64_file());
	strcat(buf2
,"                                                            ");
                                                            
                quickfwrite(buf2,7,50,filebackup(ucon64_rom()),"r+b");
                
		return(0);
}

if(argcmp(argc,argv,"-mkppf"))
{
	ucon64_argv[0]=ucon64_name();
	ucon64_argv[1]=ucon64_rom();
	ucon64_argv[2]=ucon64_file();

	strcpy(buf,ucon64_file());
	newext(buf,".PPF");

	ucon64_argv[3]=buf;
	ucon64_argc=4;

	return(makeppf_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-nppf"))
{
	strcpy(buf2,ucon64_file());
	strcat(buf2
,"                                                            ");
                                                            
	quickfwrite(buf2,6,50,filebackup(ucon64_rom()),"r+b");
	return(0);
}


if(argcmp(argc,argv,"-idppf"))
{
	addppfid(argc,argv);
	return(0);
}


strcpy(buf,ucon64_rom());
if(!strcmp(strupr(&buf[strlen(buf)-4]),".FDS")&&(quickftell(ucon64_rom())%65500)==0)
	console=ucon64_NES;


if(console==ucon64_UNKNOWN)
{
//the same but automatic
        if(!access(ucon64_rom(),F_OK))
        	console=ucon64_probe(argc,argv);
}




//here could be global overrides


//options which depend on a special console system

switch(console)
{

case ucon64_GB:
	gameboy_main(argc,argv);
break;
case ucon64_GBA:
	gbadvance_main(argc,argv);
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
case ucon64_PSX2:
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
case ucon64_VECTREX:
	vectrex_main(argc,argv);
break;
case ucon64_VIRTUALBOY:
	virtualboy_main(argc,argv);
break;
case ucon64_WONDERSWAN:
	wonderswan_main(argc,argv);
break;
case ucon64_COLECO:
	coleco_main(argc,argv);
break;
case ucon64_INTELLI:
	intelli_main(argc,argv);
break;
case ucon64_UNKNOWN:
default:
	if(!access(ucon64_rom(),F_OK))
	{
		filehexdump(ucon64_rom(),0,512);//show possible header or maybe the internal rom header
		printf(
"\n"
"ERROR: unknown ROM: %s (not found in internal DB)\n"
"TIP:   if this is a ROM you can force recognition with the -<CONSOLE> option\n"
"       if you compiled from the sources you can add it to ucon64_db.c and\n"
"       recompile\n",ucon64_rom());
	}
	else ucon64_usage(argc,argv);
break;
}


if (argcmp(argc, argv, "-e"))
{
  char *property;
#ifdef __DOS__
  strcpy(buf, "ucon64.cfg");
#else 
  sprintf(buf, "%s%c.ucon64rc", getenv("HOME"), FILE_SEPARATOR);
#endif

  if (console != ucon64_UNKNOWN && console != ucon64_KNOWN)
    sprintf(buf3, "emulate_%s", &forceargs[console][1]);
  else
  {
    printf("ERROR: could not auto detect the right ROM/console type; please use the\n"
	   "\"-<CONSOLE> force recognition\" option next time\n");
    return -1;
  }

  if (access(buf, F_OK) == -1)
  {
    printf("ERROR: %s does not exist\n", buf);
    return -1;
  }

  property = getProperty(buf, buf3, buf2, NULL);        // buf2 also contains property value
  if (property == NULL)
  {
    printf("ERROR: could not find the correct settings (%s) in\n"
           "       %s\n"
           "       please fix that or use the \"-<CONSOLE> force recognition\" option next\n"
           "       time if the wrong ROM/console type was detected\n",
           buf3, buf);
    return -1;
  }

  sprintf(buf, "%s %s", buf2, ucon64_file());
  for (x = 0; x < argc; x++)
  {
    if (strdcmp(argv[x], "-e") && strdcmp(argv[x], ucon64_name()) &&
        strdcmp(argv[x], ucon64_file()))
    {
      sprintf(buf2, ((!strdcmp(argv[x], ucon64_rom())) ? " \"%s\"" : /*" %s"*/""), argv[x]);
      strcat(buf, buf2);
    }
  }

  printf("%s\n", buf);
  fflush(stdout);
  sync();

  x = system(buf);
#ifndef __DOS__
  x >>= 8;                                      // the exit code is coded in bits 8-15
#endif                                          //  (that is, under Linux & BeOS)

#if 1
  // snes9x (Linux) for example returns a non-zero values on a normal exit (3)...

  // under WinDOS, system() immediately returns with exit code 0 when starting
  //  a Windows executable (as if a fork() happened) it also returns 0 when the
  //  exe could not be started
  if (x != 127 && x != -1 && x != 0)    // 127 && -1 are system() errors, rest are exit codes
  {
    printf("ERROR: the Emulator returned an error code (%d)\n"
           "       maybe %s is corrupt...\n"
           "       or use the \"-<CONSOLE> force recognition\" option next time\n",
           (int) x, ucon64_rom());
    return x;
  }
#endif

  return 0;
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
	else if(lynx_probe(argc,argv)!=-1)console=ucon64_LYNX;
	else if(gameboy_probe(argc,argv)!=-1)console=ucon64_GB;
	else if(gbadvance_probe(argc,argv)!=-1)console=ucon64_GBA;
	else if(jaguar_probe(argc,argv)!=-1)console=ucon64_JAGUAR;
	else if(atari_probe(argc,argv)!=-1)console=ucon64_ATARI;
	else if(pcengine_probe(argc,argv)!=-1)console=ucon64_PCE;
	else if(coleco_probe(argc,argv)!=-1)console=ucon64_COLECO;
	else if(intelli_probe(argc,argv)!=-1)console=ucon64_INTELLI;
	else if(neogeo_probe(argc,argv)!=-1)console=ucon64_NEOGEO;
	else if(neogeopocket_probe(argc,argv)!=-1)console=ucon64_NEOGEOPOCKET;
	else if(sms_probe(argc,argv)!=-1)console=ucon64_SMS;
	else if(system16_probe(argc,argv)!=-1)console=ucon64_SYSTEM16;
	else if(virtualboy_probe(argc,argv)!=-1)console=ucon64_VIRTUALBOY;
	else if(vectrex_probe(argc,argv)!=-1)console=ucon64_VECTREX;
	else if(wonderswan_probe(argc,argv)!=-1)console=ucon64_WONDERSWAN;
	else console=ucon64_UNKNOWN;
	return(console);
}


int ucon64_usage(int argc,char *argv[])
{
	printf("USAGE: %s [OPTION(S)] ROM [FILE]\n"
	"\n"
,ucon64_name()
);

printf(/*"TODO: $ROM could also be the name of a *.ZIP archive\n"
	"      it will automatically find and extract the ROM\n"
	"\n"*/
	"TODO:  -sh	use uCON64 in shell modus\n"
	"  -e		emulate/run ROM (check INSTALL and $HOME/.ucon64rc for more)\n"
	"  -crc		show CRC32 value of ROM\n"
	"  -crchd	show CRC32 value of ROM (regarding to +512 Bytes header)\n"
	"  -dbs		search ROM database (all entries) by CRC32; $ROM=0xCRC32\n"
	"  -db		ROM database statistics (# of entries)\n"
	"  -dbv		view ROM database (all entries)\n"
//	"TODO:  -ls	generate ROM list for all ROMs; $ROM=DIRECTORY\n"
	"  -lsv		like -ls but more verbose; $ROM=DIRECTORY\n"
//	"TODO:  -rrom	rename all ROMs in DIRECTORY to their internal names; $ROM=DIR\n"
//	"		this is often used by people who loose control of their ROMs\n"
	"  -rl		rename all files in DIRECTORY to lowercase; $ROM=DIRECTORY\n"
	"  -ru		rename all files in DIRECTORY to uppercase; $ROM=DIRECTORY\n"
	"  -hex		show ROM as hexdump; use \"ucon64 -hex $ROM|less\"\n"
	"  -find		find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)\n"
	"  -c		compare ROMs for differencies; $FILE=OTHER_ROM\n"
	"  -cs		compare ROMs for similarities; $FILE=OTHER_ROM\n"
	"  -swap		swap/(de)interleave ALL Bytes in ROM (1234<->2143)\n"
	"  -ispad	check if ROM is padded\n"
	"  -pad		pad ROM to full Mb\n"
	"  -padhd	pad ROM to full Mb (regarding to +512 Bytes header)\n"
	"  -stp		strip first 512 Bytes (possible header) from ROM\n"
	"  -ins		insert 512 Bytes (0x00) before ROM\n"
	"  -strip	strip Bytes from end of ROM; $FILE=VALUE\n"
);

bsl_usage( argc, argv );
ips_usage( argc, argv );
aps_usage( argc, argv );
ppf_usage( argc, argv );
xps_usage( argc, argv );

printf("\n");


/*
if(argcmp(argc,argv,"-dc"))dreamcast_usage(argc,argv);
else */
if(argcmp(argc,argv,"-gba"))gbadvance_usage(argc,argv);
else if(argcmp(argc,argv,"-n64"))nintendo64_usage(argc,argv);
/*
else if(argcmp(argc,argv,"-psx") ||
	argcmp(argc,argv,"-psx2"))playstation_usage(argc,argv);
*/
else if(argcmp(argc,argv,"-jag"))jaguar_usage(argc,argv);
else if(argcmp(argc,argv,"-snes"))supernintendo_usage(argc,argv);
else if(argcmp(argc,argv,"-ng"))neogeo_usage(argc,argv);
else if(argcmp(argc,argv,"-ngp"))neogeopocket_usage(argc,argv);
else if(argcmp(argc,argv,"-gen"))genesis_usage(argc,argv);
else if(argcmp(argc,argv,"-gb"))gameboy_usage(argc,argv);
else if(argcmp(argc,argv,"-lynx"))lynx_usage(argc,argv);
else if(argcmp(argc,argv,"-pce"))pcengine_usage(argc,argv);
else if(argcmp(argc,argv,"-sms"))sms_usage(argc,argv);
//else if(argcmp(argc,argv,"-c64"))commodore_usage(argc,argv);
else if(argcmp(argc,argv,"-nes"))nes_usage(argc,argv);
else if(argcmp(argc,argv,"-s16"))sys16_usage(argc,argv);
else if(argcmp(argc,argv,"-ata"))atari_usage(argc,argv);
else if(argcmp(argc,argv,"-coleco"))coleco_usage(argc,argv);
else if(argcmp(argc,argv,"-vboy"))virtualboy_usage(argc,argv);
else if(argcmp(argc,argv,"-swan"))wonderswan_usage(argc,argv);
else if(argcmp(argc,argv,"-vec"))vectrex_usage(argc,argv);
else if(argcmp(argc,argv,"-int"))intelli_usage(argc,argv);
else 
{
	gbadvance_usage(argc,argv);
	nintendo64_usage(argc,argv);
	supernintendo_usage(argc,argv);
	neogeopocket_usage(argc,argv);
	neogeo_usage(argc,argv);
	genesis_usage(argc,argv);
	gameboy_usage(argc,argv);
	jaguar_usage(argc,argv);
	lynx_usage(argc,argv);
	pcengine_usage(argc,argv);
	sms_usage(argc,argv);
	nes_usage(argc,argv);
/*
	sys16_usage(argc,argv);
	atari_usage(argc,argv);
	coleco_usage(argc,argv);
	virtualboy_usage(argc,argv);
	wonderswan_usage(argc,argv);
	vectrex_usage(argc,argv);
	intelli_usage(argc,argv);
*/
printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n"
	"  -s16, -ata, -coleco, -vboy, -swan, -vec, -int\n"
	"		force recognition\n"
	"  *		show info (default)\n\n"
,system16_TITLE
,atari_TITLE
,coleco_TITLE
,virtualboy_TITLE
,wonderswan_TITLE
,vectrex_TITLE
,intelli_TITLE
);

}

printf(
	"Database: %ld known ROMs in ucon64_db.c (%+ld)\n\n"
	,ucon64_dbsize(ucon64_UNKNOWN)
	,ucon64_dbsize(ucon64_UNKNOWN)-ucon64_DBSIZE
);

printf("TIP: %s -help -snes (would show only Super Nintendo related help)\n"
	"     %s -help|less (to see everything in less)\n"
	"     give the force recognition option a try if something went wrong\n"
	"\n"
	"All CD-based consoles are supported by uCONCD; go to http://ucon64.sf.net\n"
	"Report problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net\n"
	"\n"
,ucon64_name(),ucon64_name()
);
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
