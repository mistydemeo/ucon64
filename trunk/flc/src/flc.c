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
long x = 0;
char buf[4096];
struct flc_ flc;


  if (argc<2 ||
      argcmp(argc, argv, "-h") ||
      argcmp(argc, argv, "-help") ||
      argcmp(argc, argv, "-?"))
    {
    usage(argc,argv);
    return 0;
  }

strcpy(flc.arg0,getarg(argc,argv,0));
strcpy(flc.arg1,getarg(argc,argv,1));
strcpy(flc.arg2,getarg(argc,argv,2));
strcpy(flc.arg3,getarg(argc,argv,3));

flc.fcount=-1;

flc.kb=(argcmp(argc,argv,"-k")) ? KB : NKB;
flc.fr=(argcmp(argc,argv,"-fr")) ? REV : NREV;

flc.what=
  (
    (argcmp(argc,argv,"-a")) ? BYNAME :
    (
      (argcmp(argc,argv,"-b")) ? BYSIZE :
      BYDATE
    )
  );

flc.how=(argcmp(argc,argv,"-ap")) ? APPEND : WRITE;
flc.check=(argcmp(argc,argv,"-t")) ? 1 : 0;
flc.edit=(argcmp(argc,argv,"-ed")) ? 1 : 0;
flc.html=(argcmp(argc,argv,"-h")) ? 1 : 0;

if(strlwr(flc.arg0)[0]=='t' || argcmp(argc,argv,"-x"))
{
  if(txtextract(argv[argc-1])==-1)usage(argc,argv);
  return(0);
}

if(strlwr(flc.arg0)[0]=='z' || argcmp(argc,argv,"-z"))
{
//		if(
  zippysearch(argv[argc-2],argv[argc-1]);
//==-1)usage(argc,argv);
}

if(argcmp(argc,argv,"-n"))
{
//sprintf(buf,"echo >%s \"Nuked with %s\n",(argv[argc-2],"flc");
//system(buf);
//exit(0);
//nuke file
  return(0);
}

if(argcmp(argc,argv,"-s")||argcmp(argc,argv,"-c")||argcmp(argc,argv,"-c2"))
{
  if(argcmp(argc,argv,"-s"))      resortfl(flc);
  else if(argcmp(argc,argv,"-c")) mkfl(flc);
  else mkfl2(flc);

  if(flc.fcount==-1)usage(argc,argv);

  if(remove("file_id.diz")!=0)
  {
    remove("FILE_ID.DIZ");
    remove("File_id.diz");
  }	

  sort(flc);

//  if(how==WRITE)output(argv[argc-1],fcount,kb,edit,html,"wb");
//  else output(argv[argc-1],fcount,kb,edit,html,"ab");

  output(flc);
}
  exit(0);
}




void usage(int argc, char *argv[])
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
",getarg(argc,argv,0));

return(0);
}
