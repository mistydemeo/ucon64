/*
    flc 0.9.3 linux/unix 1999/2000/2001 by NoisyB (noisyb@gmx.net)
    A filelisting creator which uses FILE_ID.DIZ from archives/files
    to generate a bbs style filelisting
    Copyright (C) 1999/2000/2001 by NoisyB (noisyb@gmx.net)

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

#include "flc.h"

#include "extract.h"
#include "mkfl2.h"
#include "zippy.h"
#include "sort.h"
#include "mkfl.h"
#include "resortfl.h"
#include "output.h"
#include "txtextract.h"


int main(int argc,char *argv[])
{
register long x=0;
long fcount=-1;
char buf[4096];
int kb=NKB,fr=NREV,what=BYDATE,how=WRITE,check=0;
int edit=0,html=0;

//fgauge(START,0);

//strcpy(arg,argv[0]);
//arg=argv[0];

if(argc<2||(argv[1][0]=='-'&&argv[1][1]=='-'&&(argv[1][2]=='?'||argv[1][2]=='h')))usage();

//for(x=1;x<argc+1;x++)
//{
//	if(x==argc)usage();
//	if(argv[x][0]=='-')break;
//}

if(argv[0][0]=='t'||argv[0][0]=='T')
{
	if(txtextract(argv[argc-1])==-1)usage();
	exit(0);
}
if(argv[0][0]=='z'||argv[0][0]=='Z')
{
//	if(
zippysearch(argv[argc-2],argv[argc-1]);
//==-1)usage();
	exit(0);
}

for(x=1;x<argc;x++)if(argv[x][0]=='-')
{
        strcpy(buf,&argv[x][1]);

	if(!strdcmp(buf,"ap"))how=APPEND;
	if(!strdcmp(buf,"d"))what=BYDATE;
	if(!strdcmp(buf,"a"))what=BYNAME;
	if(!strdcmp(buf,"b"))what=BYSIZE;
	if(!strdcmp(buf,"k"))kb=KB;
	if(!strdcmp(buf,"h"))html=1;
	if(!strdcmp(buf,"ed"))edit=1;
	if(!strdcmp(buf,"t"))check=1;
	if(!strdcmp(buf,"fr"))fr=REV;
//	if(!strdcmp(buf,"ng0"))fgauge(NOANIM,0);
}

for(x=1;x<argc;x++)if(argv[x][0]=='-')
{
//todo: kind of $ARG overrides $OPTION !!!

        strcpy(buf,&argv[x][1]);
        
	if(!strdcmp(buf,"z"))
	{
//		if(
zippysearch(argv[argc-2],argv[argc-1]);
//==-1)usage();
	}
	if(!strdcmp(buf,"x"))
	{
		if(txtextract(argv[argc-1])==-1)usage();
//more...
	}
	if(!strdcmp(buf,"s")||!strdcmp(buf,"c")||!strdcmp(buf,"c2"))
	{
		if(!strdcmp(buf,"n"))
		{
//sprintf(buf,"echo >%s \"Nuked with %s\n",(argv[argc-2],"flc");
//system(buf);
//exit(0);
//nuke file
		}
		if(!strdcmp(buf,"s"))fcount=resortfl(argv[argc-2]);
		if(!strdcmp(buf,"c"))fcount=mkfl(argv[argc-2],check);
		if(!strdcmp(buf,"c2"))fcount=mkfl2(argv[argc-3],argv[argc-2],check);
		if(fcount==-1)usage();

if(remove("file_id.diz")!=0)
{
	remove("FILE_ID.DIZ");
	remove("File_id.diz");
}	


		sort(fcount,fr,what);
		printf("\n");

		if(how==WRITE)output(argv[argc-1],fcount,kb,edit,html,"wb");
		else output(argv[argc-1],fcount,kb,edit,html,"ab");
		exit(0);
	}
}
exit(0);
}




void usage(void)
{
printf("\nflc 0.9.3 linux/unix 1999/2000 by NoisyB (noisyb@gmx.net)\n\
This may be freely redistributed under the terms of the GNU Public License\n\n\
USAGE: %s [OPTION[MODIFIERS]] ARG1 ARG2 ARG3\n\n\
  -c		create a new file-listing;\n\
  		$ARG1=DIRECTORY $ARG2=NEW_FILELISTING\n\
  -c2		create a new file-listing but take the FILE_ID.DIZ out of an\n\
  		existing file-listing;\n\
		$ARG1=DIRECTORY $ARG2=EXISTING_FILELISTING $ARG3=NEW_FILELISTING\n\
  -s		resort an existing file-listing;\n\
		$ARG1=FILELISTING $ARG2=NEW_FILELISTING\n\
TODO:  -n	nuke (be careful); $ARG1=FILENAME\n\
TODO:  -rn	rename all files in DIRECTORY to lowercase; $ARG1=DIRECTORY
$0=z,\n\
  -z		zippy-search file-listing; $ARG1=PATTERN $ARG2=FILELISTING\n\
modifiers:\n\
  -t		also check/test every archive/file in DIRECTORY\n\
		flags: N=not checked (default), P=passed, F=failed\n\
  -h		output as HTML document with links to the files\n\
TODO:  -ed	use editor for archives/files w/o FILE_ID.DIZ\n\
  -ap		append at NEW_FILELISTING\n\
  -d		sort chronological\n\
  -a		sort alphabetical\n\
  -b		sort by byte size\n\
  -fr		sort reverse\n\
  -k		show sizes in kilobytes\n\
\n\
intern FILE_ID.DIZ extract routines:\n\
$0=txtextract,\n\
  -x		extract FILE_ID.DIZ from plain text; $ARG1=PLAIN_TXT_FILE\n\
TODO:		extract FILE_ID.DIZ from zip archive; $ARG1=ZIP_ARCHIVE\n\
\n\
Amiga version: noC-FLC Version v1.O (File-Listing Creator) - (C)1994 nocTurne deSign/MST\n\
Report problems to noisyb@gmx.net\n\n\
","");



	exit(0);
}
