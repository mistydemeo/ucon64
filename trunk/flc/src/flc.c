/*
    flc 1999/2000/2001 by NoisyB (noisyb@gmx.net)
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

/*
  I dedicate this to dbjh who told me to use 2 spaces instead of tabs!
*/
#include "flc.h"

#include "extract.h"
#include "sort.h"
#include "output.h"

int main(int argc,char *argv[])
{
char buf[4096];
struct flc_ flc;
struct file_ *file,*file0,file_ns;
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
  argcmp(argc,argv,"-d") ||
  argcmp(argc,argv,"-a") ||
  argcmp(argc,argv,"-b") ||
  argcmp(argc,argv,"-fr")
)
{
  flc.sort=1;
  flc.bydate = (argcmp(argc,argv,"-d")) ? 1 : 0;
  flc.byname = (argcmp(argc,argv,"-a")) ? 1 : 0;
  flc.bysize = (argcmp(argc,argv,"-b")) ? 1 : 0;
  flc.fr = (argcmp(argc,argv,"-fr")) ? 1 : 0 ;
}

flc.argc=argc;
for( x = 0 ; x < argc ; x++ )flc.argv[x]=argv[x];

flc.kb = (argcmp(argc,argv,"-k")) ? 1 : 0;
flc.html = (argcmp(argc,argv,"-html")) ? 1 : 0 ;
flc.files = 0;

strcpy(flc.path,getarg(argc,argv,flc_FILE));

if(stat(flc.path,&puffer)!=-1 &&
   S_ISREG(puffer.st_mode)==TRUE)
{
  single_file=1;
  flc.sort=0;
}
else
{
  if(!flc.path[0])
    getcwd(flc.path,(size_t)sizeof(flc.path));
  if(flc.path[strlen(flc.path)-1]==FILE_SEPARATOR)
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

  if(!(file=(struct file_ *)malloc((flc.files+2)*sizeof(struct file_))))
  {
    printf("%s: Error allocating memory\n",getarg(argc,argv,0));
    (void)closedir(dp);
    return(-1);
  }
  file0=file;
  rewinddir(dp);
}
else file=&file_ns;


flc.files=0;
while( (!single_file) ?
       ((ep=readdir(dp))!=NULL) : 1
)
{
  if(!single_file)sprintf(buf,"%s/%s",flc.path,ep->d_name);
  else strcpy(buf,flc.path);
  
  if(stat(buf,&puffer)==-1)continue;
  if(S_ISREG(puffer.st_mode)!=TRUE)continue;

  file->date=puffer.st_mtime;
  file->size=puffer.st_size;  
  file->checked='N';
  strcpy(file->name,(!single_file) ? ep->d_name : flc.path);  
  if(single_file)flc.path[0]=0;

  extract(&flc,file);

  if(!flc.sort)
  {
    output(&flc,file);
    if(single_file)break;
    continue;
  }

  file->pos=flc.files;
  file++;
  flc.files++;
}
if(!single_file)(void)closedir(dp);
file=file0;

if(flc.sort)
{
  sort(&flc,file);
for( x = 0 ; x < flc.files ; x++ )
  output(&flc,file+(file+x)->pos);

  free(file);
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
  "  -t		also check/test every archive/file in DIRECTORY\n"
  "		flags: N=not checked (default), P=passed, F=failed\n"
  "  -html		output as HTML document with links to the files\n"
  "  -d		sort chronological\n"
  "  -a		sort alphabetical\n"
  "  -b		sort by byte size\n"
  "  -fr		sort reverse\n"
  "  -k		show sizes in kilobytes\n"
  "\n"
  "Report problems to noisyb@gmx.net or go to http://ucon64.sf.net\n\n"
  ,flc_TITLE
  ,getarg(argc,argv,flc_NAME)
);

return(0);
}
