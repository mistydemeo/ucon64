/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
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
#include "backup/unknown_bu.h"
#include "backup/unknown_bu512.h"

#include "patch/aps.h"
#include "patch/ips.h"
#include "patch/ppf.h"
#include "patch/bsl.h"
#include "patch/xps.h"

int main(int argc,char *argv[])
{
  long x, y = 0;
  int ucon64_argc;
  struct dirent *ep;
  struct stat puffer;
  DIR *dp;
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

struct ucon64_ rom;
ucon64_flush(argc,argv,&rom);

  printf("%s\n",ucon64_TITLE);
  printf("Uses code from various people. See 'DEVELOPERS' for more!\n");
  printf("This may be freely redistributed under the terms of the GNU Public License\n\n");

  if (argc<2 ||
      argcmp(argc, argv, "-h") ||
      argcmp(argc, argv, "-help") ||
      argcmp(argc, argv, "-?"))
    {
    ucon64_usage(argc,argv);
    return 0;
  }

strcpy(rom.rom,getarg(argc,argv,ucon64_ROM));
if (!strlen(rom.rom)) getcwd(rom.rom,sizeof(rom.rom));

strcpy(rom.file,getarg(argc,argv,ucon64_FILE));






if(argcmp(argc, argv, "-sh"))
{
//TODO shell modus
//  for(;;)
  {
    printf("ucon64>");
  }
  return(0);
}

if (argcmp(argc, argv, "-crc"))
{
  printf("Checksum: %08lx\n\n", fileCRC32(rom.rom, 0));
  return 0;
}

if(argcmp(argc,argv,"-crchd"))
{
	printf("Checksum: %08lx\n\n",fileCRC32(rom.rom,512));
	return(0);
}


if(argcmp(argc,argv,"-rl"))
{
	renlwr(rom.rom);
	return(0);
}

if(argcmp(argc,argv,"-ru"))
{
	renupr(rom.rom);
	return(0);
}

if(argcmp(argc,argv,"-hex"))
{
	filehexdump(rom.rom,0,quickftell(rom.rom));
	return(0);
}

if(argcmp(argc,argv,"-c"))
{
	if(filefile(rom.rom,0,rom.file,0,FALSE)==-1)
		printf("ERROR: file not found/out of memory\n");
	return(0);
}

if (argcmp(argc, argv, "-cs"))
{
  if (filefile(rom.rom, 0, rom.file, 0, TRUE) == -1)
    printf("ERROR: file not found/out of memory\n");
  return 0;
}

if(argcmp(argc,argv,"-find"))
{
	x=0;
	y=quickftell(rom.rom);
	while((x=filencmp2(rom.rom,x,y,rom.file,strlen(rom.file),'?'))!=-1)
	{
		filehexdump(rom.rom,x,strlen(rom.file));
		x++;
		printf("\n");
	}
	return(0);
}

if(argcmp(argc,argv,"-swap"))
{
	fileswap(filebackup(rom.rom),0,quickftell(rom.rom));
	return(0);
}

if(argcmp(argc,argv,"-pad"))
{
	filepad(rom.rom,0,MBIT);
	return(0);
}

if(argcmp(argc,argv,"-padhd"))
{
	filepad(rom.rom,512,MBIT);
	return(0);
}

if(argcmp(argc,argv,"-ispad"))
{
	unsigned long padded;

	if((padded=filetestpad(rom.rom))!=-1)
	{
		if(!padded)printf("Padded: No\n");
		else printf("Padded: Maybe, %ld Bytes (%.4f Mb)\n",padded,(float)padded/MBIT);
	}
	printf("\n");
	return(0);
}

if(argcmp(argc,argv,"-strip"))
{
	truncate(rom.rom,quickftell(rom.rom)-atol(rom.file));
	return(0);
}

if(argcmp(argc,argv,"-stp"))
{
	strcpy(buf,filebackup(rom.rom));
	newext(buf,".TMP");
	rename(rom.rom,buf);

	filecopy(buf,512,quickftell(buf),rom.rom,"wb");
	remove(buf);
	return(0);
}

if(argcmp(argc,argv,"-ins"))
{
	strcpy(buf,filebackup(rom.rom));
	newext(buf,".TMP");
	rename(rom.rom,buf);

        memset(buf2,0,512);
	quickfwrite(buf2,0,512,rom.rom,"wb");

	filecopy(buf,0,quickftell(buf),rom.rom,"ab");

	remove(buf);
	return(0);
}

if(argcmp(argc,argv,"-b"))
{
	if(bsl(rom.rom,rom.file)!=0)printf("ERROR: failed\n");
	return(0);
}

if(argcmp(argc,argv,"-i"))
{
	ucon64_argv[0]="ucon64";
	ucon64_argv[1]=rom.file;
	ucon64_argv[2]=rom.rom;
	ucon64_argc=3;

	return(ips_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-a"))
{
	ucon64_argv[0]="ucon64";
	ucon64_argv[1]="-f";
	ucon64_argv[2]=rom.rom;
	ucon64_argv[3]=rom.file;
	ucon64_argc=4;

	return(n64aps_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-ppf"))
{
	ucon64_argv[0]="ucon64";
	ucon64_argv[1]=rom.rom;
	ucon64_argv[2]=rom.file;
	ucon64_argc=3;

	return(applyppf_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-mki"))
{
	cips(rom.rom,rom.file);
	return(0);
}

if(argcmp(argc,argv,"-mka"))
{
	ucon64_argv[0]="ucon64";
	ucon64_argv[1]="-d \"\"";
	ucon64_argv[2]=rom.rom;
	ucon64_argv[3]=rom.file;
	ucon64_argv[4]="test";
	ucon64_argc=5;

	return(n64caps_main(ucon64_argc,ucon64_argv));
}

if (argcmp(argc, argv, "-na"))
{
  strcpy(buf2, rom.file);
  strcat(buf2, "                                                            ");

  quickfwrite(buf2, 7, 50, filebackup(rom.rom), "r+b");

  return 0;
}

if(argcmp(argc,argv,"-mkppf"))
{
	ucon64_argv[0]="ucon64";
	ucon64_argv[1]=rom.rom;
	ucon64_argv[2]=rom.file;

	strcpy(buf,rom.file);
	newext(buf,".PPF");

	ucon64_argv[3]=buf;
	ucon64_argc=4;

	return(makeppf_main(ucon64_argc,ucon64_argv));
}

if(argcmp(argc,argv,"-nppf"))
{
	strcpy(buf2,rom.file);
	strcat(buf2
,"                                                            ");
                                                            
	quickfwrite(buf2,6,50,filebackup(rom.rom),"r+b");
	return(0);
}


if (argcmp(argc, argv, "-nppf"))
{
  strcpy(buf2, rom.file);
  strcat(buf2, "                                                            ");

  quickfwrite(buf2, 6, 50, filebackup(rom.rom), "r+b");
  return 0;
}

if(argcmp(argc,argv,"-idppf"))
{
	addppfid(argc,argv);
	return(0);
}

if (argcmp(argc, argv, "-ls") ||
    argcmp(argc, argv, "-lsv") ||
    argcmp(argc,argv,  "-rrom") ||
    argcmp(argc,argv,  "-rr83")
)
{
  char current_dir[FILENAME_MAX];
  if (access(rom.rom, R_OK) == -1 || (dp = opendir(rom.rom)) == NULL)
    return -1;

  getcwd(current_dir, FILENAME_MAX);
  chdir(rom.rom);

  while ((ep = readdir(dp)) != 0)
  {
    if (!stat(ep->d_name, &puffer))
    {
      if (S_ISREG(puffer.st_mode))
      {
	ucon64_flush(argc,argv,&rom);
	strcpy(rom.rom,ep->d_name);
        ucon64_init(&rom);
	if (argcmp(argc, argv, "-ls"))
	{
          strftime(buf, 13, "%b %d %H:%M", localtime(&puffer.st_mtime));
          printf("%-31.31s %10d %s %s\n", rom.name, (int) puffer.st_size, buf, rom.rom);
        }
        else if (argcmp(argc, argv, "-lsv"))
        {
          ucon64_nfo(&rom);
        }
        else if (argcmp(argc, argv, "-rrom") && rom.console != ucon64_UNKNOWN && rom.console != ucon64_KNOWN)
        {
          strcpy(buf,&rom.rom[findlast(rom.rom,".")+1]);
          printf("%s.%s\n",rom.name,buf);
        }
        else 
        {
          
        }
	fflush(stdout);
      }
    }
  }
  closedir(dp);
  chdir(current_dir);

  return 0;
}





#ifdef BACKUP
if (strlen( rom.file ))
{
  strcpy(buf, rom.file);
  sscanf(buf, "%x", &ucon64_parport);
}

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
  course with the uCON64 executable setuid root). Anyone a better idea?
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
gid = getgid();                                 // This shouldn't be necessary if `make install'
if (setgid(gid) == -1)                          //  was used, but just in case (root did `chmod +s')
{
  fprintf(stderr, "Could not set gid\n");
  exit(1);
}
#endif

#endif



if(!access(rom.rom,F_OK))
{
	ucon64_init(&rom);
	if(rom.console!=ucon64_UNKNOWN)ucon64_nfo(&rom);
}

strcpy(buf,rom.rom);
if(!strcmp(strupr(&buf[strlen(buf)-4]),".FDS")&&(quickftell(rom.rom)%65500)==0)
	rom.console=ucon64_NES;
if(argcmp(argc,argv,"-ata"))rom.console=ucon64_ATARI;
if(argcmp(argc,argv,"-s16"))rom.console=ucon64_SYSTEM16;
if(argcmp(argc,argv,"-n64"))rom.console=ucon64_N64;
if(argcmp(argc,argv,"-psx"))rom.console=ucon64_PSX;
if(argcmp(argc,argv,"-psx2"))rom.console=ucon64_PSX2;
if(argcmp(argc,argv,"-lynx"))rom.console=ucon64_LYNX;
if(argcmp(argc,argv,"-jag"))rom.console=ucon64_JAGUAR;
if(argcmp(argc,argv,"-sms"))rom.console=ucon64_SMS;
if(argcmp(argc,argv,"-snes"))rom.console=ucon64_SNES;
if(argcmp(argc,argv,"-gen"))rom.console=ucon64_GENESIS;
if(argcmp(argc,argv,"-gb"))rom.console=ucon64_GB;
if(argcmp(argc,argv,"-gba"))rom.console=ucon64_GBA;
if(argcmp(argc,argv,"-ng"))rom.console=ucon64_NEOGEO;
if(argcmp(argc,argv,"-ngp"))rom.console=ucon64_NEOGEOPOCKET;
if(argcmp(argc,argv,"-nes"))rom.console=ucon64_NES;
if(argcmp(argc,argv,"-pce"))rom.console=ucon64_PCE;
if(argcmp(argc,argv,"-sat"))rom.console=ucon64_SATURN;
if(argcmp(argc,argv,"-dc"))rom.console=ucon64_DC;
if(argcmp(argc,argv,"-cdi"))rom.console=ucon64_CDI;
if(argcmp(argc,argv,"-cd32"))rom.console=ucon64_CD32;
if(argcmp(argc,argv,"-3do"))rom.console=ucon64_REAL3DO;
if(argcmp(argc,argv,"-coleco"))rom.console=ucon64_COLECO;
if(argcmp(argc,argv,"-vboy"))rom.console=ucon64_VIRTUALBOY;
if(argcmp(argc,argv,"-swan"))rom.console=ucon64_WONDERSWAN;
if(argcmp(argc,argv,"-vec"))rom.console=ucon64_VECTREX;
if(argcmp(argc,argv,"-int"))rom.console=ucon64_INTELLI;

if(argcmp(argc,argv,"-col"))rom.console=ucon64_SNES;
if(argcmp(argc,argv,"-ip"))rom.console=ucon64_DC;
if(argcmp(argc,argv,"-sam"))rom.console=ucon64_NEOGEO;
if(argcmp(argc,argv,"-stp"))rom.console=ucon64_KNOWN;
if(argcmp(argc,argv,"-ins"))rom.console=ucon64_KNOWN;
if(argcmp(argc,argv,"-cdrom"))rom.console=ucon64_KNOWN;
if(argcmp(argc,argv,"-b2i"))rom.console=ucon64_KNOWN;
if(argcmp(argc,argv,"-hex"))rom.console=ucon64_KNOWN;
if(argcmp(argc,argv,"-xv64"))rom.console=ucon64_N64;
if(argcmp(argc,argv,"-xdjr"))rom.console=ucon64_N64;
if(argcmp(argc,argv,"-bot"))rom.console=ucon64_N64;
if(argcmp(argc,argv,"-n2gb"))rom.console=ucon64_GB;
if(argcmp(argc,argv,"-xfal"))rom.console=ucon64_GBA;
if(argcmp(argc,argv,"-xswc"))rom.console=ucon64_SNES;
if(argcmp(argc,argv,"-xswcs"))rom.console=ucon64_SNES;
if(argcmp(argc,argv,"-swcs"))rom.console=ucon64_SNES;
if(argcmp(argc,argv,"-figs"))rom.console=ucon64_SNES;
if(argcmp(argc,argv,"-ufos"))rom.console=ucon64_SNES;
if(argcmp(argc,argv,"-xfalm"))rom.console=ucon64_GBA;
if(argncmp(argc,argv,"-xfalc",6))rom.console=ucon64_GBA;
if(argcmp(argc,argv,"-xgbx"))rom.console=ucon64_GB;
if(argcmp(argc,argv,"-xgbxs"))rom.console=ucon64_GB;
if(argncmp(argc,argv,"-xgbxb",6))rom.console=ucon64_GB;


if(argcmp(argc,argv,"-db"))
{
	printf(
		"Database: %ld known ROMs in ucon64_db.c (%+ld)\n\n"
		,ucon64_dbsize(rom.console)
		,ucon64_dbsize(rom.console)-ucon64_DBSIZE
	);

	printf("TIP: %s -db -nes would show only the number of known NES ROMs\n\n",getarg(argc,argv,ucon64_NAME));
	return(0);
}

if(argcmp(argc,argv,"-dbs"))
{
	ucon64_flush(argc,argv,&rom);

	sscanf(rom.rom, "%lx", &rom.current_crc32);

	ucon64_dbsearch(&rom);
	
	ucon64_nfo(&rom);

	printf("TIP: %s -dbs -nes would search only for a NES ROM\n\n",getarg(argc,argv,ucon64_NAME));

	return(0);
}

if(argcmp(argc,argv,"-dbv"))
{
	ucon64_dbview(rom.console);

	printf("\nTIP: %s -db -nes would view only NES ROMs\n\n",getarg(argc,argv,ucon64_NAME));
	return(0);
}

/*
    options which depend on a special console system
*/
switch(rom.console)
{

case ucon64_GB:
	gameboy_main(&rom);
break;
case ucon64_GBA:
	gbadvance_main(&rom);
break;
case ucon64_GENESIS:
	genesis_main(&rom);
break;
case ucon64_SMS:
	sms_main(&rom);
break;
case ucon64_JAGUAR:
	jaguar_main(&rom);
break;
case ucon64_LYNX:
	lynx_main(&rom);
break;
case ucon64_N64:
	nintendo64_main(&rom);
break;
case ucon64_NEOGEO:
	neogeo_main(&rom);
break;
case ucon64_NES:
	nes_main(&rom);
break;
case ucon64_PCE:
	pcengine_main(&rom);
break;
/*  ucon64 is cartridge only
case ucon64_PSX:
case ucon64_PSX2:
	playstation_main(&rom);
break;
case ucon64_DC:
	dreamcast_main(&rom);
break;
*/
case ucon64_SYSTEM16:
	system16_main(&rom);
break;
case ucon64_ATARI:
	atari_main(&rom);
break;
case ucon64_SNES:
	supernintendo_main(&rom);
break;
case ucon64_NEOGEOPOCKET:
	neogeopocket_main(&rom);
break;
case ucon64_VECTREX:
	vectrex_main(&rom);
break;
case ucon64_VIRTUALBOY:
	virtualboy_main(&rom);
break;
case ucon64_WONDERSWAN:
	wonderswan_main(&rom);
break;
case ucon64_COLECO:
	coleco_main(&rom);
break;
case ucon64_INTELLI:
	intelli_main(&rom);
break;
case ucon64_UNKNOWN:
default:
	if(!access(rom.rom,F_OK))
	{
		filehexdump(rom.rom,0,512);//show possible header or maybe the internal rom header
		printf(
"\nERROR: unknown ROM: %s (not found in internal DB)\n"
"TIP:   if this is a ROM you can force recognition with the -<CONSOLE> option\n"
"       if you compiled from the sources you can add it to ucon64_db.c and\n"
"       recompile\n"
,rom.rom);
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

  if (rom.console != ucon64_UNKNOWN && rom.console != ucon64_KNOWN)
    sprintf(buf3, "emulate_%s", &forceargs[rom.console][1]);
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

  sprintf(buf, "%s %s", buf2, rom.file);
  for (x = 0; x < argc; x++)
  {
    if (strdcmp(argv[x], "-e") && strdcmp(argv[x], getarg(argc,argv,ucon64_NAME)) &&
        strdcmp(argv[x], rom.file))
    {
      sprintf(buf2, ((!strdcmp(argv[x], rom.rom)) ? " \"%s\"" : /*" %s"*/""), argv[x]);
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
  // Snes9x (Linux) for example returns a non-zero value on a normal exit (3)...

  // under WinDOS, system() immediately returns with exit code 0 when starting
  //  a Windows executable (as if a fork() happened) it also returns 0 when the
  //  exe could not be started
  if (x != 127 && x != -1 && x != 0)    // 127 && -1 are system() errors, rest are exit codes
  {
    printf("ERROR: the Emulator returned an error code (%d)\n"
           "       maybe %s is corrupt...\n"
           "       or use the \"-<CONSOLE> force recognition\" option next time\n",
           (int) x, rom.rom);
    return x;
  }
#endif

  return 0;
}

return(0);
}




/*
	flush the ucon64 struct with default values
*/
int ucon64_flush(int argc,char *argv[],struct ucon64_ *rom)
{
  long x = 0;
	
  rom->argc=argc;
//  for( x = 0 ; x < argc ; x++ )strcpy(rom->argv[x],argv[x]);
  for( x = 0 ; x < argc ; x++ )rom->argv[x]=argv[x];

  strcpy(rom->name,"");
  strcpy(rom->name2,"");

  rom->bytes=quickftell(rom->rom);

  strcpy(rom->rom,getarg(argc,argv,ucon64_ROM));
  strcpy(rom->file,getarg(argc,argv,ucon64_FILE));

  strcpy(rom->title,"");
  strcpy(rom->copier,"?");

  rom->mbit=0;
  rom->bytes=0;

  rom->console=ucon64_UNKNOWN;	//integer for the console system

  rom->swapped=0;
  for( x = 0 ; x < sizeof( rom->splitted ) / sizeof( rom->splitted[0] ) ; x++ )rom->splitted[x]=0;
  rom->padded=0;
  rom->intro=0;

  rom->db_crc32=0;
  rom->current_crc32=0;	//standart current_crc32 checksum of the rom

  rom->has_internal_crc=0;
  rom->current_internal_crc=0;	//current custom crc
  rom->internal_crc=0;	//custom crc (if not current_crc32)
  rom->internal_crc_start=0;
  rom->internal_crc_len=0;	//size in bytes
  rom->internal_inverse_crc=0;	//crc complement
  rom->internal_inverse_crc_start=0;
  rom->internal_inverse_crc_len=0;		//size in bytes

  memset(rom->buheader,0,sizeof(rom->buheader));
  rom->buheader_start=0;
  rom->buheader_len=0;	//header of backup unit

  memset(rom->header,0,sizeof(rom->header));
  rom->header_start=0;
  rom->header_len=0;	//header of rom itself (if there is one)

  strcpy(rom->name,"?");
  rom->name_start=0;
  rom->name_len=0;
	
  strcpy(rom->manufacturer,"?");
  rom->manufacturer_start=0;
  rom->manufacturer_len=0;
	
  strcpy(rom->country,"?");
  rom->country_start=0;
  rom->country_len=0;
	
  strcpy(rom->misc,"");

  return(0);
}


int ucon64_init_(struct ucon64_ *rom)
{
/*
rom->buheader_len=0;
rom->current_crc32=fileCRC32(rom->rom,rom->buheader_len);

ucon64_dbsearch(rom);

if(rom->db_crc32==0)
{
	rom->buheader_len=unknown_bu512_HEADER_LEN;
	rom->current_crc32=fileCRC32(rom->rom,rom->buheader_len);

	ucon64_dbsearch(rom);

	if(rom->db_crc32==0)
	{
			return(-1);
	}
}

	return(0);
*/
	return(-1);
}

int ucon64_init(struct ucon64_ *rom)
{
  if(
//    (ucon64_init_(rom)==-1) &&
    (supernintendo_init(rom)==-1) &&
    (genesis_init(rom)==-1) &&
    (nintendo64_init(rom)==-1) &&
    (gameboy_init(rom)==-1) &&
    (gbadvance_init(rom)==-1) &&
    (atari_init(rom)==-1) &&
    (nes_init(rom)==-1) &&
    (lynx_init(rom)==-1) &&
    (jaguar_init(rom)==-1) &&
    (pcengine_init(rom)==-1) &&
    (neogeo_init(rom)==-1) &&
    (neogeopocket_init(rom)==-1) &&
    (sms_init(rom)==-1) &&
    (system16_init(rom)==-1) &&
    (virtualboy_init(rom)==-1) &&
    (vectrex_init(rom)==-1) &&
    (coleco_init(rom)==-1) &&
    (intelli_init(rom)==-1) &&
    (wonderswan_init(rom)==-1)
  )
  {
    rom->console=ucon64_UNKNOWN;
    return(-1);
  }

  quickfread(rom->buheader,rom->buheader_start,rom->buheader_len,rom->rom);
//  quickfread(rom->header,rom->header_start,rom->header_len,rom->rom);

  if(!rom->current_crc32)rom->current_crc32=fileCRC32(rom->rom,rom->buheader_len);

  rom->bytes=quickftell(rom->rom);
  rom->mbit=(float)((rom->bytes-rom->buheader_len)/MBIT);

  rom->padded=filetestpad(rom->rom);
  rom->intro=(rom->bytes-rom->buheader_len)&MBIT;

  rom->splitted[0] = testsplit(rom->rom);
  if (argcmp(rom->argc, rom->argv, "-ns"))
    rom->splitted[0] = 0;


  return(0);
}


int ucon64_usage(int argc,char *argv[])
{
  printf("USAGE: %s [OPTION(S)] ROM [FILE]\n\n",getarg(argc,argv,ucon64_NAME));

  printf(/*"TODO: $ROM could also be the name of a *.ZIP archive\n"
	"      it will automatically find and extract the ROM\n"
	"\n"*/
//	"TODO:  -sh      use uCON64 in shell modus\n"
	"  -e            emulate/run ROM (check INSTALL and $HOME/.ucon64rc for more)\n"
	"  -crc          show CRC32 value of ROM\n"
	"  -crchd        show CRC32 value of ROM (regarding to +512 Bytes header)\n"
	"  -dbs          search ROM database (all entries) by CRC32; $ROM=0xCRC32\n"
	"  -db           ROM database statistics (# of entries)\n"
	"  -dbv          view ROM database (all entries)\n"
	"  -ls           generate ROM list for all ROMs; $ROM=DIRECTORY\n"
	"  -lsv          like -ls but more verbose; $ROM=DIRECTORY\n"
//	"TODO:  -rrom    rename all ROMs in DIRECTORY to their internal names; $ROM=DIR\n"
//	"TODO:  -rr83    like -rrom but with 8.3 filenames; $ROM=DIR\n"
//	"                this is often used by people who loose control of their ROMs\n"
	"  -rl           rename all files in DIRECTORY to lowercase; $ROM=DIRECTORY\n"
	"  -ru           rename all files in DIRECTORY to uppercase; $ROM=DIRECTORY\n"
#ifdef  __DOS__
        "  -hex          show ROM as hexdump; use \"ucon64 -hex $ROM|more\"\n"
#else
        "  -hex          show ROM as hexdump; use \"ucon64 -hex $ROM|less\"\n"   // less is better ;-)
#endif
        "  -find         find string in ROM; $FILE=STRING ('?'==wildcard for ONE char!)\n"
	"  -c            compare ROMs for differencies; $FILE=OTHER_ROM\n"
	"  -cs           compare ROMs for similarities; $FILE=OTHER_ROM\n"
	"  -swap         swap/(de)interleave ALL Bytes in ROM (1234<->2143)\n"
	"  -ispad        check if ROM is padded\n"
	"  -pad          pad ROM to full Mb\n"
	"  -padhd        pad ROM to full Mb (regarding to +512 Bytes header)\n"
	"  -stp          strip first 512 Bytes (possible header) from ROM\n"
	"  -ins          insert 512 Bytes (0x00) before ROM\n"
	"  -strip        strip Bytes from end of ROM; $FILE=VALUE\n"
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
	"		force recognition"
#ifndef DB
"; NEEDED"
#endif
	"\n  *		show info (default)\n\n"
,system16_TITLE
,atari_TITLE
,coleco_TITLE
,virtualboy_TITLE
,wonderswan_TITLE
,vectrex_TITLE
,intelli_TITLE
);

}

printf("Database: %ld known ROMs in ucon64_db.c (%+ld)\n\n",
       ucon64_dbsize(ucon64_UNKNOWN),
       ucon64_dbsize(ucon64_UNKNOWN)-ucon64_DBSIZE);

printf("TIP: %s -help -snes (would show only Super Nintendo related help)\n"
#ifdef  __DOS__
        "     %s -help|more (to see everything in more)\n"
#else
        "     %s -help|less (to see everything in less)\n"      // less is better ;-)
#endif
        "     give the force recognition option a try if something went wrong\n"
	"\n"
	"All CD-based consoles are supported by uCONCD; go to http://ucon64.sf.net\n"
	"Report problems/ideas/fixes to noisyb@gmx.net or go to http://ucon64.sf.net\n"
	"\n"
,getarg(argc,argv,ucon64_NAME),getarg(argc,argv,ucon64_NAME)
);
	return(0);
}

/*
  this is the now centralized nfo output for all kinds of ROMs
*/
int ucon64_nfo(struct ucon64_ *rom)
{

  printf("%s\n%s\n%s\n\n"
    ,rom->rom
    ,rom->title
    ,rom->copier
  );
  
  if(rom->header_len)
  {
     strhexdump(rom->header,0,rom->header_start+rom->buheader_len,rom->header_len);
     printf("\n");
  }	

  printf("%s\n%s%s%s\n%s\n%ld bytes (%.4f Mb)\n\n"
    ,rom->name
    ,rom->name2,(rom->name2[0])?"\n":""
    ,rom->manufacturer
    ,rom->country
    ,rom->bytes-rom->buheader_len
    ,rom->mbit
  );
        
  if(!rom->padded)printf("Padded: No\n");
  else if(rom->padded)printf("Padded: Maybe, %ld Bytes (%.4f Mb)\n",rom->padded,(float)rom->padded/MBIT);

  if(!rom->intro)printf("Intro/Trainer: No\n");
  else if(rom->intro)printf("Intro/Trainer: Maybe, %ld Bytes\n",rom->intro);

  if(!rom->splitted[0])printf("Splitted: No\n");
  else if(rom->splitted[0])printf("Splitted: Yes, %d parts (Note: for most options the ROM must be joined)\n",rom->splitted[0]);

  if(rom->misc[0])printf("%s\n",rom->misc);

  if(rom->has_internal_crc)
  {
    printf("Checksum: %s, %04lx %s= %04lx\n",
           (rom->current_internal_crc == rom->internal_crc) ? "ok" : "bad",
           rom->current_internal_crc,
           (rom->current_internal_crc == rom->internal_crc) ? "" : "!",
           rom->internal_crc);
    printf("Inverse checksum: %s, %04lx + %04lx = %04lx %s\n",
           (rom->current_internal_crc + rom->internal_inverse_crc == 0xffff) ? "ok" : "bad",
           rom->current_internal_crc, rom->internal_inverse_crc, rom->current_internal_crc + rom->internal_inverse_crc,
           (rom->current_internal_crc + rom->internal_inverse_crc == 0xffff) ? "" : "~0xffff");
  }
  printf("Checksum (CRC32): %08lx\n",rom->current_crc32);

  printf("\n");

  return 0;
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
