/*
    flc 1999-2002 by NoisyB (noisyb@gmx.net)
    flc lists information about the FILEs (the current directory by default)
    But most important it shows the FILE_ID.DIZ of every file (if present)
    Very useful for FTP Admins or people who have a lot to do with FILE_ID.DIZ
    Copyright (C) 1999-2002 by NoisyB (noisyb@gmx.net)

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
#include "sort.h"
#include "output.h"


void flc_exit(void)
{
  printf("+++EOF");
  fflush(stdout);
}


int main(int argc,char *argv[])
{
char buf[NAME_MAX+1];
char buf2[4096];
struct flc_ flc;
struct file_ *file0=NULL,*file=NULL,file_ns;
struct dirent *ep;
struct stat puffer;
long x = 0;
//int single_file=0;
DIR *dp;


/*
    support for frontends
*/
if(argcmp(argc, argv, "-frontend"))atexit(flc_exit);

/*
   configfile handling
*/
#ifdef	__DOS__
  strcpy(buf, "flc.cfg");
#else
  sprintf(buf, "%s%c.flcrc", getenv("HOME"), FILE_SEPARATOR);
#endif

if(access(buf,F_OK)==-1)printf("ERROR: %s not found: creating...",buf);
else if(getProperty(buf, "version", buf2, NULL) == NULL)
{
  strcpy(buf2,buf);
  newext(buf2,".OLD");

  printf("NOTE: updating config: will be renamed to %s...",buf2);

  rename(buf,buf2);

  sync();
}

if(access(buf,F_OK)==-1)
{
  FILE *fh;

  if(!(fh=fopen(buf,"wb")))
  {
    printf("FAILED\n\n");

    return -1;
  }
  else
  {
    fputs(       
"# flc config\n"
"#\n"
"version=100\n"
"#\n"
"# LHA support\n"
"#\n"
"lha_test=lha t\n"
"lha_extract=lha efi \n"
"#lha_extract=lha e \n"
"#\n"
"# LZH support\n"
"#\n"
"lzh_test=lha t\n"
"lzh_extract=lha efi\n"
"#lzh_extract=lha e\n"
"#\n"
"# ZIP support\n"
"#\n"
"zip_test=unzip -t\n"
"zip_extract=unzip -xojC\n"
"#zip_extract=unzip -xoj\n"
"#\n"
"# TXT/NFO/FAQ support\n"
"#\n"
"txt_extract=txtextract\n"
"nfo_extract=txtextract\n"
"faq_extract=txtextract\n"
"#\n"
"# FILE_ID.DIZ names/synonyms\n"
"#\n"
"file_id_diz=*_Id.* *_iD.* *_ID.* *_id.* FILE_ID.DIZ file_id.diz File_id.diz File_Id.Diz [Ff][Ii][Ll][Ee]_[Ii][Dd].[Dd][Ii][Zz]\n"
    ,fh);

    fclose(fh);
    printf("OK\n\n");
  }

  return 0;
}

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

strcpy(flc.path,getarg(argc,argv,flc_FILE));
#ifdef __DOS__
  strcpy(flc.configfile, "flc.cfg");
#else
  sprintf(flc.configfile, "%s%c.flcrc", getenv("HOME"), FILE_SEPARATOR);
#endif
getProperty(flc.configfile,"file_id_diz",flc.config,"file_id.diz");

if(flc.html)  printf("<html><head><title></title></head><body><pre><tt>");

if(!flc.path[0])
  getcwd(flc.path,(size_t)sizeof(flc.path));
if(flc.path[strlen(flc.path)-1]==FILE_SEPARATOR && strlen(flc.path) != 1)
  flc.path[strlen(flc.path)-1]=0;

/*
    single file handling
*/
if(stat(flc.path,&puffer)!=-1 &&
   S_ISREG(puffer.st_mode)==TRUE)
{
  file=&file_ns;

  file->next=NULL;
  (file->sub).date=puffer.st_mtime;
  (file->sub).size=puffer.st_size;  
  (file->sub).checked='N';
  strcpy((file->sub).name,flc.path);
  flc.path[0]=0;

  extract(&flc,&file->sub);

  output(&flc,&file->sub);

  return(0);
}

if(!(dp=opendir(flc.path)))
{
  flc_usage(argc,argv);
  return(-1);
}

while((ep=readdir(dp))!=NULL)
{
  sprintf(buf,"%s/%s",flc.path,ep->d_name);

  if(stat(buf,&puffer)==-1)continue;
  if(S_ISREG(puffer.st_mode)!=TRUE)continue;

  if(file0==NULL)
  {
    if(!(file=(struct file_ *)malloc(sizeof(struct file_))))
    {
      printf("%s: Error allocating memory\n",getarg(argc,argv,0));
      (void)closedir(dp);
      return(-1);
    }

    file0=file;
  }
  else
  {
    if(!((file->next)=(struct file_ *)malloc(sizeof(struct file_))))
    {
      printf("%s: Error allocating memory\n",getarg(argc,argv,0));
      (void)closedir(dp);
      return(-1);
    }
    file=file->next;
  }

  (file->sub).date=puffer.st_mtime;
  (file->sub).size=puffer.st_size;  
  (file->sub).checked='N';
  strcpy((file->sub).name,ep->d_name);
  extract(&flc,&file->sub);
}

(void)closedir(dp);
file->next=NULL;
file=file0;

if(flc.sort)sort(&flc,file);

for(;;)
{
  output(&flc,&file->sub);
  if(file->next==NULL)break;
  file=file->next;
}
free(file0);

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
  "Amiga version: noC-FLC Version v1.O (File-Listing Creator) - (C)1994 nocTurne deSign/MST\n"
  "Report problems to noisyb@gmx.net or go to http://ucon64.sf.net\n\n"
  ,flc_TITLE
  ,getarg(argc,argv,flc_NAME)
);

return(0);
}
