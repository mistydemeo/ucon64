/*
    flc 1999/2000/2001 by NoisyB (noisyb@gmx.net)
    flc lists information about the FILEs (the current directory by default)
    But most important it shows the FILE_ID.DIZ of every file (if present)
    Very useful for FTP Admins or people who have a lot to do with FILE_ID.DIZ
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

/*
  I dedicate this to dbjh who told me to use 2 spaces instead of tabs!
*/
#include "flc.h"

#include "extract.h"
#include "sort.h"
#include "output.h"

int main(int argc,char *argv[])
{
char buf[NAME_MAX+1];
struct flc_ flc;
struct files_ *files,*file0,file_ns;
struct dirent *ep;
struct stat puffer;
long x = 0;
int single_file=0;
DIR *dp;

if (
    argcmp(argc, argv, "-h") ||
    argcmp(argc, argv, "-help") ||
    argcmp(argc, argv, "-?"))
  {
  flc_usage(argc,argv);
  return 0;
}

if(
  argcmp(argc,argv,"-t") ||
  argcmp(argc,argv,"-X") ||
  argcmp(argc,argv,"-S") ||
  argcmp(argc,argv,"-fr")
)
{
  flc.sort=1;
  flc.bydate = (argcmp(argc,argv,"-t")) ? 1 : 0;
  flc.byname = (argcmp(argc,argv,"-X")) ? 1 : 0;
  flc.bysize = (argcmp(argc,argv,"-S")) ? 1 : 0;
  flc.fr = (argcmp(argc,argv,"-fr")) ? 1 : 0 ;
}

flc.argc=argc;
for( x = 0 ; x < argc ; x++ )flc.argv[x]=argv[x];

flc.kb = (argcmp(argc,argv,"-k")) ? 1 : 0;
flc.html = (argcmp(argc,argv,"-html")) ? 1 : 0 ;
flc.files = 0;

strcpy(flc.path,getarg(argc,argv,flc_FILE));
#ifdef __DOS__
  strcpy(flc.configfile, "flc.cfg");
#else
  sprintf(flc.configfile, "%s%c.flcrc", getenv("HOME"), FILE_SEPARATOR);
#endif
getProperty(flc.configfile,"file_id_diz",flc.config,"file_id.diz");

if(stat(flc.path,&puffer)!=-1 &&
   S_ISREG(puffer.st_mode)==TRUE)
{
  single_file=1;
  flc.sort=0;
}

if(!single_file)
{
  if(!flc.path[0])
    getcwd(flc.path,(size_t)sizeof(flc.path));
  if(flc.path[strlen(flc.path)-1]==FILE_SEPARATOR && strlen(flc.path) != 1)
    flc.path[strlen(flc.path)-1]=0;

  if(!(dp=opendir(flc.path)))
  {
    flc_usage(argc,argv);
    return(-1);
  }
}

if(flc.html)  printf("<html><head><title></title></head><body><pre><tt>");

if(flc.sort && !single_file)
{
/*
    find out how many regular files are in the current dir
    and malloc files*struct for them 
*/

  while((ep=readdir(dp))!=NULL)
  {
    stat(ep->d_name,&puffer);
    if(S_ISREG(puffer.st_mode))flc.files++;
  }

  if(!(files=(struct files_ *)malloc((flc.files+2)*sizeof(struct files_))))
  {
    printf("%s: Error allocating memory\n",getarg(argc,argv,0));
    (void)closedir(dp);
    return(-1);
  }
  file0=files;
  rewinddir(dp);
}
else files=&file_ns;

flc.files=0;
while( (!single_file) ?
       ((ep=readdir(dp))!=NULL) : 1
)
{
  if(!single_file)sprintf(buf,"%s/%s",flc.path,ep->d_name);
  else strcpy(buf,flc.path);

  if(stat(buf,&puffer)==-1)continue;
  if(S_ISREG(puffer.st_mode)!=TRUE)continue;

  files->pos=flc.files;
  files->date=puffer.st_mtime;
  files->size=puffer.st_size;  
  files->checked='N';
  strcpy(files->name,(!single_file) ? ep->d_name : flc.path);  
  if(single_file)flc.path[0]=0;

//  extract(&flc,files);

  if(!flc.sort)
  {
    output(&flc,files);
    if(single_file)break;
    continue;
  }

  flc.files++;
  files++;
}
if(!single_file)(void)closedir(dp);
files=file0;

if(flc.sort)
{
  sort(&flc,files);
for( x = 0 ; x < flc.files ; x++ )
  output(&flc,files+(files+x)->pos);

  free(files);
}


if(flc.html)printf(
  "</pre></tt></body></html>\n"
);

return(0);
}

int flc_usage(int argc, char *argv[])
{
printf(
  "\n%s\n"
  "This may be freely redistributed under the terms of the GNU Public License\n\n"
  "USAGE: %s [OPTION]... [FILE]...\n\n"
  "  -c		also test every possible archive in DIRECTORY for errors\n"
  "		return flags: N=not checked (default), P=passed, F=failed\n"
  "  -html		output as HTML document with links to the files\n"
  "  -t		sort by modification time\n"
  "  -X		sort alphabetical\n"
  "  -S		sort by byte size\n"
  "  -fr		sort reverse\n"
  "  -k		show sizes in kilobytes\n"
  "\n"
  "Report problems to noisyb@gmx.net or go to http://ucon64.sf.net\n\n"
  ,flc_TITLE
  ,getarg(argc,argv,flc_NAME)
);

return(0);
}
