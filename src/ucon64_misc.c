#include "ucon64_misc.h"

#define n_BUFSIZE 32768

enum
{
        FALSE
        ,TRUE
};

char 
hexDigit(int value) {
  switch(toupper(value)) {
    case 0: return '0';
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';
    case 10: return 'A';
    case 11: return 'B';
    case 12: return 'C';
    case 13: return 'D';
    case 14: return 'E';
    case 15: return 'F';
    default: break;
  }
  return '?';

}

int 
hexValue(char digit) {
  switch(toupper(digit)) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'A': return 10;
    case 'B': return 11;
    case 'C': return 12;
    case 'D': return 13;
    case 'E': return 14;
    case 'F': return 15;
    default: break;
  }
  return 0;
}

int 
hexByteValue(char x, char y) {
  return (hexValue(x) << 4) + hexValue(y);
}


int strdcmp(char *str,char *str1)
{
//compares length and case
	return(
		(
			(strlen(str)==strlen(str1)) &&
			(!strncmp(str,str1,strlen(str)))
		) ?
		0 :
		(-1)
	);
}

int argcmp(int argc,char *argv[],char *str)
{
//check for an option
	register int x=0;

	for(x=1;x<argc;x++)//leave out first arg
	{
		if(argv[x][0]=='-'&&!strdcmp(argv[x],str))return(x);
	}

	return(0);//not found
}

char *getarg(int argc,char *argv[],int pos)
{
//return a filename
	register int x=0,y=0;

	for(x=0;x<argc;x++)
	{
		if(argv[x][0]!='-')
		{
			if(y!=pos)y++;
			else return(argv[x]);
		}
	}

	return("");//not found
}

int findlwr(char *str)
{
//searches the string for ANY lowercase char
	char *str2;

	if(!(str2=strrchr(str,'/')))str2=str;//strip path if it is a filename
	else str2++;

	while(*str2)
	{                
		if(	isalpha(*str2) &&
			islower(*str2)
		)return(TRUE);
		str2++;
	}
	return(FALSE);
}

char *strupr(char *str)
{
	char *str1=str;

	while(*str)
	{
		*str=toupper(*str);
		str++;
	}

	return(str1);
}
                                                        
char *strlwr(char *str)
{
	char *str1=str;

	while(*str)
	{
		*str=tolower(*str);
		str++;
	}

	return(str1);
}

char *newext(char *str,char *ext)
{
	//replace file extension

	str[strcspn(str,".")]=0;
	strcat(str,ext);
	
	return(	
		findlwr(str) ?
		strlwr(str) :
		strupr(str)
	);
}

/*
char *stprblk(char *str1,char *str2)
{
	register long x;
	str1[0]=0;
	
	x=strlen(str2)-1;
	while(x>-1&&str2[x]==32)x--;
	x++;
	return(strncat(str1,str2,x));
}
*/

char *stpblk(char *str)
{
	while(*str=='\t'||*str==32)str++;
	return(str);
}

char *stplcr(char *str)
{
	while(*str!=0x00&&*str!=0x0a&&*str!=0x0d)str++;
	*str=0;
	return(str);
}

int rencase(char *dir,char *mode)
{
struct dirent *ep;
struct stat puffer;
DIR *dp;
char buf[n_BUFSIZE];

if(access(dir,R_OK)==-1 ||
(dp=opendir(dir))==NULL)return(-1);

chdir(dir);

while((ep=readdir(dp))!=0)
{
	if(!stat(ep->d_name,&puffer))
	{
//		if(S_ISREG(puffer.st_mode)==1)
		{
			strcpy(buf,ep->d_name);
			rename(ep->d_name,(mode[0]=='l') ? strlwr(buf) : strupr(buf));
		}
	}                
}
(void)closedir(dp);
return(0);
}


long filechksum(char *str,long fstart,int chksum_len)
{
FILE *fh;
register long x,y;
char buf[n_BUFSIZE];
unsigned long chksum=0;

if(!(fh=fopen(str,"rb")))return(-1);
fseek(fh,fstart,SEEK_SET);

while(fread(buf,1,chksum_len,fh)>0)
{
	for(x=0;x<chksum_len;x++)
	{
		chksum+=((buf[x])<<(x*8));
	}
}

fclose(fh);

y=0;
for(x=0;x<chksum_len;x++)
{
	y=y<<8;
	y=y+0x0ff;
}
return(chksum&y);
}



int gauge(time_t fgtstart,long fpos,long fsize)
{
//#ifdef LINUX
//#define START 0
	long cps=0;
	time_t curr,left;
	char buf[256];
	
/*	if(fpos==START)
	{
		fgtstart=time(0);
		return(0);
	}
*/
	curr=time(0)-fgtstart;
	if(curr!=0)
	{
		cps=fpos/curr;
		left=(fsize-fpos)/cps;

		buf[0]=0;
		strncat(buf,"===============================",(size_t)((long)24*fpos/fsize));
		strcat(buf,"-------------------------------");
		buf[24]=0;

		printf("\r%10lu Bytes [%s] %lu%%, CPS=%lu, ",fpos,buf,(unsigned int)100*fpos/fsize,(unsigned long)cps);
if(fpos==fsize)
		printf("TOTAL=%03ld:%02ld ",(long)(curr/60),((long)curr)%60);
else 		printf("ETA=%03ld:%02ld   ",(long)(left/60),((long)left)%60);

		fflush(stdout);
	}
/*
#else
	printf(".");
	fflush(stdout);
#endif
*/
	return(0);
}


long quickftell(char *str)
{
	long size=0;
	FILE *fh;

	if(!(fh=fopen(str,"rb")))return(strlen(str));
	fseek(fh,0,SEEK_END);
	size=ftell(fh);
	fclose(fh);
	return(size);
}


long filegrep(char *fname,char *search,long fstart,long slen)//fstrnstr
{
	register unsigned long x,y;
	char *buf;
	FILE *fh;

	x=0;
	if(!(fh=fopen(fname,"rb")))return(-1);

	if(!(buf=(char *)malloc(((quickftell(fname)-fstart)+2)*sizeof(char))))
	{
		fclose(fh);
	        return(-1);
	}

	fseek(fh,fstart,SEEK_SET);
	if(!fread(buf,(quickftell(fname)-fstart),1,fh))
	{
		free(buf);
		fclose(fh);
		return(-1);
	}	

	fclose(fh);

	y=(quickftell(fname)-fstart);
	while((x+slen)<y)
	{
		if(!strncmp(&buf[x],search,slen))return(x+fstart);
		x++;
	}
	free(buf);
	return(-1);
}

long quickfread(char *dest,long start,long len,char *source)
{
	long x=0;
	FILE *fh;

	if(!(fh=fopen(source,"rb")))return(-1);
	fseek(fh,start,SEEK_SET);
	x=fread(dest,len,1,fh);
	dest[len]=0;
	fclose(fh);
	return(x);
}



long quickfwrite(char *source,long start,long len,char *dest)
{
	long x=0;
	FILE *fh;

	if(!(fh=fopen(dest,"r+b")))return(-1);
	fseek(fh,start,SEEK_SET);
	x=fwrite(source,len,1,fh);
	fclose(fh);
	return(x);
}

int fileswap(char *name,long start,long len)
{
	register unsigned long x;
	size_t size;
	int failed=0;
	FILE  *fh,*fh2=0;
	char buf[n_BUFSIZE],buf2[3],buf3;

	if(len==fileswap_EOF)len=quickftell(name);
	
	if(!(fh=fopen(name,"rb")))failed=1;
	if(!failed)
	{
		strcpy(buf,name);
		newext(buf,".TMP");
	                                        
		if(!(fh2=fopen(buf,"wb")))
		{
			fclose(fh);
			return(-1);
		}
	
		fseek(fh,start,SEEK_SET);
	}
	for(x=0;x<(len-start);x=x+2)
	{
		if(!failed)
		{	
			if(!fread(buf2,2,1,fh))break;
			buf3=buf2[0];
			buf2[0]=buf2[1];
			buf2[1]=buf3;
			fwrite(buf2,2,1,fh2);
		}
		else
		{
			buf3=name[x+start];
			name[x+start]=name[x+1+start];
			name[x+1+start]=buf3;
		}
	}
	if(!failed)
	{
		size=(len-start)%2;
		fread(buf2,size,1,fh);
		fwrite(buf2,size,1,fh2);
		fclose(fh);
		fclose(fh2);
		
		remove(name);
		rename(buf,name);
	}
	return(0);
}

int quickfgetc(char *name,long pos)
{
	int c;
	FILE *fh;

        if(!(fh=fopen(name,"rb")) ||
	fseek(fh,pos,SEEK_SET)!=0)return(EOF);
	c=fgetc(fh);
	fclose(fh);
	
	return(c);
}

int hexdump(char *str,long start,long len)
{
//wenn name[0]=0; dann memdump!
	register long x;
	int failed=0;
	char buf[256];
	int dump;
	FILE *fh;

	if(!(fh=fopen(str,"rb")))failed=1;
	
	buf[16]=0;
	for(x=0;x<((len==hexdump_EOF)?quickftell(str):len);x++)
	{
		if(x!=0&&!(x%16))printf("%s\n",buf);
		if(!x||!(x%16))printf("%08lx  ",x+start);

		dump=(!failed)?quickfgetc(str,start+x):str[start+x];

		printf("%02x %s"
				,dump&0xff
				,(!((x+1)%4)) ?
				" " :
				""
			);

		buf[(x%16)]=isprint(dump)?dump:'.';
		buf[(x%16)+1]=0;
	}
	printf("%s\n",buf);
	if(!failed)fclose(fh);
	return(0);
}

long filepad(char *name,long start,char *mode)
{
	size_t size;
	register long x;
	int y;
	char *buf;
	FILE *fh;
	
	if(!islower(mode[0]))return(-1);

	if(mode[0]=='r')//counts the size of the pad (if padded)
	{
		if(!(fh=fopen(name,mode)))return(-1);
		if(!(buf=(char *)malloc(((quickftell(name))+2)*sizeof(char))))
		{
			fclose(fh);
        		return(-1);
		}
		if(!fread(buf,(quickftell(name)),1,fh))
		{
			fclose(fh);
			free(buf);
			return(-1);
		}
		fclose(fh);

		y=buf[(quickftell(name))-1]&0xff;
		x=(quickftell(name))-2;
		while(y==(buf[x]&0xff))x--;//x-=2;
//		if(y!=(buf[x+1]&0xff))x++;

		free(buf);
		if((((quickftell(name))-x)-1)==1)return(0);
		return(((quickftell(name))-x)-1);
	}
	size=(quickftell(name)-start);

	if(!(size%MBIT))return(size);

	size=(size_t)size/MBIT;
	size=size+1;
	size=size*MBIT;

	truncate(name,size+start);
	return(size);
}

int filecopy(char *s_name,long s_start,long s_len,char *d_name,char *mode)
{
	long size,ab_size=0;
	char buf[n_BUFSIZE];
	FILE *fh,*fh2;

	if(s_len==filecopy_EOF)s_len=quickftell(s_name);
        if(!islower(mode[0]))return(-1);

	if(mode[0]=='a')ab_size=(quickftell(d_name));

//if(!strdcmp(d_name,sname))return(-1);

	if(!(fh=fopen(s_name,"rb")))return(-1);
	if(!(fh2=fopen(d_name,mode)))
	{
	        fclose(fh);
		return(-1);
	}

	fseek(fh,0,SEEK_END);
	size=(ftell(fh)-s_start)%n_BUFSIZE;
	fseek(fh,s_start,SEEK_SET);
	fseek(fh2,0,SEEK_END);
	while(fread(buf,n_BUFSIZE,1,fh))fwrite(buf,n_BUFSIZE,1,fh2);
	fread(buf,size,1,fh);
	fwrite(buf,size,1,fh2);
		
	fclose(fh);
	fclose(fh2);

/*
	if(s_len!=0)
	{
	if(s_len<(quickftell(d_name)))
	truncate(d_name,s_len+ab_size);
}
*/
	return(0);
}

char *filebackup(char *name)
{
	char buf[n_BUFSIZE];
	if(!access(name,F_OK))
	{
		strcpy(buf,name);
//		newext(buf,".bak");
		strcat(buf,".bak");//~
		filecopy(name,0,filecopy_EOF,filebackup(buf),"wb");
	}
	return(name);
}


int filecmp(char *fname,long fstart,char *fname2,long fstart2,int mode)
{
	register long x,y,len;
	char *buf,*buf2;
	FILE *fh,*fh2;
	
	if(!strdcmp(fname,fname2))return(0);


        if(!(fh=fopen(fname,"rb")))return(-1);
	if(!(fh2=fopen(fname2,"rb")))
	{
		fclose(fh);
		return(-1);
	}
	if(!(buf=(char *)malloc(((quickftell(fname))+2)*sizeof(char))))
	{
		fclose(fh);
		fclose(fh2);
        	return(-1);
	}
	if(!(buf2=(char *)malloc(((quickftell(fname2))+2)*sizeof(char))))
	{
		free(buf);
		fclose(fh);
		fclose(fh2);
		return(-1);
	}
	if(!fread(buf,(quickftell(fname)),1,fh)||!fread(buf2,(quickftell(fname2)),1,fh2))
	{
		free(buf);
		free(buf2);
		fclose(fh);
		fclose(fh2);
		return(-1);
	}	

	fclose(fh);
	fclose(fh2);
	x=(	(quickftell(fname)-fstart)<(quickftell(fname2)-fstart2) ?
		(quickftell(fname)-fstart) :
		(quickftell(fname2)-fstart2)
	);

	for(y=0;y<x;y++)
	{
		if
		(
		(mode==filecmp_DIFF&&buf[y+fstart]!=buf2[y+fstart2])
		||
		(mode==filecmp_SIMI&&buf[y+fstart]==buf2[y+fstart2])	
		)
		{
			len=0;
			if(mode==filecmp_DIFF)while(buf[y+fstart+len]!=buf2[y+fstart2+len])len++;
			else while(buf[y+fstart+len]==buf2[y+fstart2+len])len++;
			printf("%s\n",fname);
			hexdump(buf,y+fstart,len);
			printf("%s\n",fname2);
			hexdump(buf2,y+fstart2,len);
			printf("\n");
			y+=len;
		}
	}
	free(buf);
	free(buf2);
	return(0);
}

int filereplace(char *fname,char wildcard,char *search,long slen,char *replace,long rlen,long fstart)
{
	register long x;
	x=fstart;
	
	for(;;)
	{
		x=filegrep(fname,search,x,slen);

		if(x==-1)return(0);

		hexdump(fname,x,slen);

		quickfwrite(replace,x,rlen,fname);

		hexdump(fname,x,rlen);

		printf("\n");
		x++;
	}
}
