/*
misc.c - miscellaneous functions

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
#include "misc.h"
#include <stdarg.h>                             // va_arg()
#include <string.h>                             // strncpy()

#define MAXBUFSIZE 32768

#if     (__UNIX__ || __BEOS__)
#include <stdlib.h>                             // atexit()
#include <unistd.h>                             // tcsetattr()
#include <termios.h>

typedef struct termios tty_t;
#endif


#if     (__UNIX__ || __BEOS__)
static void deinit_conio(void);
static void set_tty(tty_t param);
#endif

int allascii(char *b, int size)
{
  int i;

  for (i = 0; i < size; i++)
    if (b[i] < 32)      // " || b[i] > 127" is unnecessary, b is signed
      return 0;         //  (unsigned char) b[i] > 127 == b[i] < 0 == b[i] < 32

  return 1;
}

int strdcmp(	char *str
		,char *str1
)
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

int argcmp(	int argc
		,char *argv[]
		,char *str
)
{
	register int x=0;

	for(x=1;x<argc;x++)//leave out first arg
	{
		if(argv[x][0]=='-' &&
		(	!strdcmp(argv[x],str) ||
			!strdcmp(&argv[x][1],str) //'--' also!
		)
			)return(x);
	}

	return(0);//not found
}

int argncmp(	int argc
		,char *argv[]
		,char *str
		,size_t len
)
{
	register int x=0;

	for(x=1;x<argc;x++)//leave out first arg
	{
		if(argv[x][0]=='-' &&
		(
			!strncmp(argv[x],str,len) ||
			!strncmp(&argv[x][1],str,len)
		)
			)return(x);
	}

	return(0);//not found
}

char *getarg(	int argc
		,char *argv[]
		,int pos
)
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

long getarg_intval(int argc, char **argv, char *argname)
{
  int n, len = strlen(argname);

  for (n = 1; n < argc; n++)
    if (!strncmp(argv[n], argname, len))
      return atol(&argv[n][len]);

  return 0;
}

int findlwr(char *str)
//searches the string for ANY lowercase char
{
  char *str2;

  if (!(str2 = strrchr(str, FILE_SEPARATOR)))
    str2 = str;				//strip path if it is a filename
  else
    str2++;

  while (*str2)
  {
    if (isalpha((int) *str2) && islower((int) *str2) )
      return TRUE;
    str2++;
  }
  return FALSE;
}

int findlast(const char *str, const char *str2)
{
/* 
	same as strcspn() but looks for the last appearance of str2

	findlast(".1234.6789",".") == 5
*/
	register int x=0;
	int pos=0;
	
	while( x < strlen(str) )
	{
		pos = ( strncmp( &str[x] ,str2 ,strlen(str2) )==0 ) ? x : pos;
		x++;
	}
	return( (pos!=0) ? pos : x );
}

char *strupr(	char *str
)
{
	char *str1=str;

	while(*str)
	{
		*str=toupper(*str);
		str++;
	}

	return(str1);
}
                                                        
char *strlwr(	char *str
)
{
	char *str1=str;

	while(*str)
	{
		*str=tolower(*str);
		str++;
	}

	return(str1);
}

char *newext(	char *filename
		,char *ext
)
{
  int pos=0;
  char ext2[4096];

  pos=findlast(filename,".");
  if( filename[pos-1] != FILE_SEPARATOR ) filename[pos] = 0; //some files might start with a dot (.)

  strcpy(ext2,ext);
  strcat(  filename
           ,( findlwr(filename) == TRUE ) ?
           strlwr(ext2) :
           strupr(ext2)
  );

  return(filename);
}

int extcmp(     char *filename
                ,char *ext
)
{
        char ext2[4096],ext3[4096];
        
	strcpy(ext2,ext);
	strcpy(ext3,&filename[findlast(filename,".")+1]);
	return(strcmp(strupr(ext2),strupr(ext3)));
}
                
char *stpblk(	char *str
)
{
	while( *str=='\t' || *str==32 )str++;
	return(str);
}

char *strtrim(	char *dest
		,char *str
)
{
	register long x;
	str[0]=0;

	x=strlen( str )-1;
	while( x>=0 && str[x]==32 )x--;

	return(strncat( dest ,stpblk(str) ,x+1 ));
}

char *stplcr(	char *str
)
{
	while( *str!=0x00 && *str!=0x0a && *str!=0x0d )str++;
	*str=0;
	return(str);
}

char *strswap(	char *str
		,long start
		,long len
)
{
	register unsigned long x;
	char c;


	for(x=start;x<(len-start);x+=2)
	{
		c=str[x];
		str[x]=str[x+1];
		str[x+1]=c;
	}
	return(str);
}

int strhexdump(	char *str
		,long start
		,long virtual_start
		,long len
)
{
	register long x;
	char buf[256];
	int dump;

	buf[16]=0;

	for(x=0;x<len;x++)
	{
		if( x!=0 && !(x%16) ) printf("%s\n",buf);
		if( !x || !(x%16) ) printf("%08lx  ",x+virtual_start);

		dump=str[x+start];

		printf("%02x %s"
				,dump&0xff
				,( ( !( (x+1) %4 ) ) ? " " : "" )
			);

		buf[(x%16)]=( isprint(dump) ? dump : '.' );
		buf[(x%16)+1]=0;
	}
	printf("%s\n",buf);
	return(0);
}

int rencase(	char *dir
		,char *mode
)
{
struct dirent *ep;
struct stat puffer;
DIR *dp;
char buf[MAXBUFSIZE];

if(access( dir ,R_OK)==-1 ||
(dp=opendir( dir ))==NULL)return(-1);

chdir( dir );

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


int renlwr(	char *dir
)
{
	return(rencase(dir,"lwr"));
}

int renupr(	char *dir
)
{
	return(rencase(dir,"upr"));
}


long quickftell(	char *filename
)
{
	struct stat puffer;

	return(
		(!stat(filename,&puffer)) ?
		((long)puffer.st_size) :
		(-1L)
	);
}


long filencmp(	char *filename
		,long start
		,long len
		,char *search
		,long searchlen	//length of *search in Bytes
)
{
	register long x,y;
	size_t size;
	char *buf;

  if(access(filename,R_OK)==-1)return(-1);

	x=0;
	size=quickftell(filename);

	if(!(buf=(char *) malloc( ( size-start+2 ) * sizeof(char)) ))
	{
	        return(-1);
	}

	if(!quickfread(buf,start,size,filename))
	{
		free(buf);
		return(-1);
	}	

	while((x+searchlen+start)<size)
	{
		for(y=0;y<searchlen;y++)
		{
			if(buf[x+y]!=search[y])break;
			if(y==searchlen-1)
			{
				free(buf);
				return(x+start);
			}
		}
		x++;
	}
	free(buf);
	return(-1);
}


long filencmp2(	char *filename
		,long start
		,long len
		,char *search
		,long searchlen	//length of *search in Bytes
		,char wildcard
)
{
	register long x,y;
	size_t size;
	char *buf;

  if(access(filename,R_OK)==-1)return(-1);
  
  	x=0;
	size=quickftell(filename);

	if(!(buf=(char *) malloc( ( size-start+2 ) * sizeof(char)) ))
	{
	        return(-1);
	}

	if(!quickfread(buf,start,size,filename))
	{
		free(buf);
		return(-1);
	}	

	while((x+searchlen+start)<size)
	{
		for(y=0;y<searchlen;y++)if(search[y]!=wildcard)
		{
			if(buf[x+y]!=search[y])break;
			if(y==searchlen-1)
			{
				free(buf);
				return(x+start);
			}
		}
		x++;
	}
	free(buf);
	return(-1);
}




size_t quickfread(	void *dest
			,size_t start
			,size_t len
			,char *src
)
{
	size_t result=0;
	FILE *fh;

	len=( ( (quickftell(src)-start) < len ) ? (quickftell(src)-start) : len );

	if(!(fh=fopen(src,"rb")))return(-1);
	fseek(fh,start,SEEK_SET);
	result=fread((void *)dest,1,len,fh);
//	dest[len]=0;
	fclose(fh);
	return(result);
}



size_t quickfwrite(	const void *src
			,size_t start
			,size_t len
			,char *dest
			,char *mode
)
{
	size_t result;
	FILE *fh;

//	if( !( fh=fopen(dest ,( access(dest,F_OK)!=0 ) ? "wb" : "r+b" ) ) )return(-1);
	if( !( fh=fopen(dest ,mode ) ) )return(-1);
	fseek(fh,start,SEEK_SET);
	result=fwrite(src,1,len,fh);
	fclose(fh);
	return(result);
}


int quickfgetc(	char *filename
		,long pos
)
{
	int c;
	FILE *fh;

        if((!(fh=fopen(filename,"rb"))) || fseek(fh,pos,SEEK_SET)!=0)return(-1);
	c=fgetc(fh);
	fclose(fh);
	
	return(c);
}

int quickfputc(	char *filename
		,long pos
		,int c
		,char *mode
)
{
	int result;
	FILE *fh;

        if((!(fh=fopen(filename,mode))) || fseek(fh,pos,SEEK_SET)!=0)return(-1);
	result=fputc(c,fh);
	fclose(fh);
	
	return(result);
}

int filehexdump(	char *filename
			,long start
			,long len
)
{
	register long x,size;
	char buf[4096];
	int dump;
	FILE *fh;

  if(access(filename,R_OK)==-1)return(-1);
  
  	size=quickftell(filename);
	
	len=( ( (size-start) < len ) ? (size-start) : len );

	if(!(fh=fopen(filename,"rb")))return(-1);
	fseek(fh,start,SEEK_SET);
	
	buf[16]=0;

	for(x=0;x<len;x++)
	{
		if(x!=0&&!(x%16))printf("%s\n",buf);
		if(!x||!(x%16))printf("%08lx  ",x+start);

		dump=fgetc(fh);

		printf("%02x %s"
				,dump&0xff
				,( ( !( (x+1)%4 ) ) ? " " : "" )
			);

		buf[(x%16)]=isprint(dump)?dump:'.';
		buf[(x%16)+1]=0;
	}
	printf("%s\n",buf);
	fclose(fh);

/* TODO range (also for strhexdump?!)
	buf[0]=0;
	strncat(buf,"-------------------------------------------------------------------------------"
			,(size_t)((long)69*start/size)
	);
	strncat(buf,"==============================================================================="
			,(!((size_t)((long)69*len/size))) ?
			(1) :
			((size_t)((long)69*len/size))
	);
	strcat(buf,"-------------------------------------------------------------------------------");
	buf[69]=0;

	printf("Range: [%s]\n",buf);
*/
	return(0);
}

int filecopy(char *src, long start, long len, char *dest, char *mode)
{
  long x;
  char buf[MAXBUFSIZE];
  FILE *fh, *fh2;

  if(access(src,R_OK)==-1)return(-1);

  len = (((quickftell(src)-start) < len ) ? (quickftell(src)-start) : len);

  if (!strdcmp(dest, src))
    return -1;

  if (!(fh = fopen(src, "rb")))
    return -1;

  if (!(fh2 = fopen(dest, mode)))
  {
    fclose(fh);
    return -1;
  }

  fseek(fh, start, SEEK_SET);
  fseek(fh2, 0, SEEK_END);

  if (len >= MAXBUFSIZE)
  {
    for(x = 0; x < len/MAXBUFSIZE; x++)
    {
      fread(buf, MAXBUFSIZE, 1, fh);
      fwrite(buf, MAXBUFSIZE, 1, fh2);
    }
  }

  fread(buf, len%MAXBUFSIZE, 1, fh);
  fwrite(buf, len%MAXBUFSIZE, 1, fh2);
		
  fclose(fh);
  fclose(fh2);

  sync();
  return 0;
}


char *filebackup(	char *filename
)
{
  char buf[MAXBUFSIZE];

  if(access(filename,R_OK)==-1)return(filename);

  strcpy(buf,filename);
#ifdef __DOS__
  newext(buf,".BAK");
#else
  strcat(buf ,( findlwr(buf) ? ".bak" : ".BAK" ) );
#endif
  rename(filename,buf);//keep file attributes like date
  filecopy(buf,0,quickftell(buf),filename,"wb");
		
  sync();
  return(filename);
}

char *filenameonly(	char *str
)
{
        char *str2;
        
       	if(!(str2=strrchr(str,FILE_SEPARATOR)))str2=str;//strip path if it is a filename
	else str2++;

	return(str2);
}

unsigned long filefile(char *filename, long start, char *filename2, long start2, int similar)
{
  unsigned long x;
  long size, size2, minsize, len;
  unsigned char *buf, *buf2;
	
  if (!strdcmp(filename, filename2))
    return 0;
	
  if (access(filename, R_OK) != 0 || access(filename2, R_OK) != 0)
    return -1;

  size = quickftell(filename);                  // quickftell() returns size in bytes
  size2 = quickftell(filename2);

  if (size < start || size2 < start2)
    return -1;

  // TODO not reading the entire files into memory, but in smaller chunks
  if (!(buf = (char *) malloc(size + 2)))
    return -1;

  if (!(buf2 = (char *) malloc(size2 + 2)))
  {
    free(buf);
    return -1;
  }

  if (!quickfread(buf, 0, size, filename) || !quickfread(buf2, 0, size2, filename2))
  {
    free(buf);
    free(buf2);
    return -1;
  }	

  minsize = size - start < size2 - start2 ? size - start : size2 - start2;
  for (x = 0; x <= minsize; x++)
  {
    if ((similar == FALSE && buf[x + start] != buf2[x + start2]) ||
	(similar == TRUE && buf[x + start] == buf2[x + start2]))
    {
      len = 0;
      while ((similar == TRUE) ?
               (buf[x + start + len] == buf2[x + start2 + len]) :
               (buf[x + start + len] != buf2[x + start2 + len]))
      {
        len++;
        if (x + start + len >= size || x + start2 + len >= size2)
          break;
      }

      printf("%s:\n",filename);
      strhexdump(buf, x+start, x+start, len);
      printf("%s:\n",filename2);
      strhexdump(buf2, x+start , x+start, len);
      printf("\n");
      x += len;
    }
  }

  free(buf);
  free(buf2);
  return x;
}


int filereplace(	char *filename
			,long start
			,char *search
			,long slen
			,char *replace
			,long rlen
)
{
	unsigned long pos,size;

  if(access(filename,R_OK)==-1)return(-1);

	pos=start;
	size=quickftell(filename);
	
	for(;;)
	{
		if((pos=filencmp(filename,pos,size,search,slen))==-1)return(0);
		filehexdump(filename,pos,slen);
		quickfwrite(replace,pos,rlen,filename,"r+b");
		filehexdump(filename,pos,rlen);
		printf("\n");
		pos++;
	}
}






void change_string(char *searchstr, int strsize, char wc, char esc,
                   char *end, int endsize, char *buf, int bufsize, int offset, ...)
/*
  Search for all occurrences of string searchstr in buf and replace endsize bytes in buf
  by copying string end to the end of the found searchstring in buf plus offset.
  If searchstr contains wildcard characters wc, then n wildcard characters in searchstr
  match any n characters in buf.
  If searchstr contains escape characters esc, change_string() must be called with two
  extra arguments for each escape character, set, which must be a string (char *) and
  setsize, which must be an int. searchstr matches for an escape character if one of the
  characters in set matches.
  Note that searchstr is not necessarily a C string; it may contain one or more zero bytes
  as strsize indicates the length.
  offset is the relative offset from the last character in searchstring and may have a
  negative value.
  This function was written to patch SNES ROM dumps. It does basically the same as the old
  uCON does, with one exception, the line with:
    bufpos -= nwc;

  As stated in the comment, this causes the search to restart at the first wildcard
  character of the sequence of wildcards that was most recently skipped if the current
  character in buf didn't match the current character in searchstr. This makes
  change_string() behave a bit more intuitive. For example
    char str[] = "f foobar means...";
    change_string("f**bar", 6, '*', '!', "XXXXXXXX", 8, str, strlen(str), 2);
  finds and changes "foobar means..." into "foobar XXXXXXXX", while with uCON's algorithm
  it would not (but does the job good enough for patching SNES ROMs).

  One example of using sets:
    char str[] = "fu-bar     is the same as foobar    ";
    change_string("f!!", 3, '*', '!', "fighter", 7, str, strlen(str), 1,
                  "uo", 2, "o-", 2);
  This changes str into "fu-fighter is the same as foofighter".
*/
{
  va_list argptr;
  char *set;
  int bufpos, strpos = 0, pos_1st_esc = -1, setsize, i, nwc;

  va_start(argptr, offset);

  for (bufpos = 0; bufpos < bufsize; bufpos++)
  {
    if (strpos == 0 && searchstr[0] != esc && searchstr[0] != wc)
      while (bufpos < bufsize && searchstr[0] != buf[bufpos])
        bufpos++;

    // handle escape character in searchstr
    while (searchstr[strpos] == esc && bufpos < bufsize)
    {
      if (strpos == pos_1st_esc)
        va_start(argptr, offset);               // reset argument pointer
      if (pos_1st_esc == -1)
        pos_1st_esc = strpos;

      set = va_arg(argptr, char *);             // get next set of characters
      setsize = va_arg(argptr, int);            // get set size
      i = 0;
      // see if buf[bufpos] matches with any character in current set
      while (i < setsize && buf[bufpos] != set[i])
        i++;
      if (i == setsize)
        break;                                  // buf[bufpos] didn't match with any char

      if (strpos == strsize - 1)                // check if we are at the end of searchstr
      {
        memcpy(buf + bufpos + offset, end, endsize);
        break;
      }

      strpos++;
      bufpos++;
    }
    if (searchstr[strpos] == esc)
    {
      strpos = 0;
      continue;
    }

    // skip wildcards in searchstr
    nwc = 0;
    while (searchstr[strpos] == wc && bufpos < bufsize)
    {
      if (strpos == strsize - 1)                // check if at end of searchstr
      {
        memcpy(buf + bufpos + offset, end, endsize);
        break;
      }

      strpos++;
      bufpos++;
      nwc++;
    }
    if (bufpos == bufsize)
      break;
    if (searchstr[strpos] == wc)
    {
      strpos = 0;
      continue;
    }

    if (searchstr[strpos] == esc)
    {
      bufpos--;                                 // current char has to be checked, but `for'
      continue;                                 //  increments bufpos
    }

    // no escape char, no wildcard -> normal character
    if (searchstr[strpos] == buf[bufpos])
    {
      if (strpos == strsize - 1)                // check if at end of searchstr
      {
        memcpy(buf + bufpos + offset, end, endsize);
        strpos = 0;
      }
      else
        strpos++;
    }
    else
    {
      strpos = 0;
      bufpos -= nwc;                            // scan the most recent wildcards too if
    }                                           //  the character didn't match
  }

  va_end(argptr);
}

int fileswap(	char *filename
		,long start
		,long len
)
{
  unsigned long x;
  size_t size;
  FILE *in,*out;
  char buf[MAXBUFSIZE],buf2[3],buf3;


  if(access(filename,R_OK)==-1)return(-1);
  
  len=( ( (quickftell(filename)-start) < len ) ? (quickftell(filename)-start) : len );


  if(!(in=fopen(filename,"rb")))return(-1);
  fseek(in,start,SEEK_SET);

  strcpy(buf,filename);
  newext(buf,".TMP");
  if(!(out=fopen(buf,"wb")))
  {
    fclose(in);
    return(-1);
  }

  for(x=0;x<(len-start);x+=2)
  {
    if(!fread(buf2,2,1,in))break;
    buf3=buf2[0];
    buf2[0]=buf2[1];
    buf2[1]=buf3;
    fwrite(buf2,2,1,out);
  }
  size=(len-start)%2;
  fread(buf2,size,1,in);
  fwrite(buf2,size,1,out);

  fclose(in);fclose(out);
		
  remove(filename);
  rename(buf,filename);

  return(0);
}

char *getProperty(char *filename, char *propname, char *buffer, char *def)
{
  char buf[MAXBUFSIZE], *result;
  FILE *fh;

  if ((fh = fopen(filename, "rb")) != 0)
  {
    while (fgets(buf, sizeof buf, fh) != NULL)
    {
      stplcr(buf);
      if (stpblk(buf)[0] == '#')
        continue;

      buf[strcspn(buf,"#")]=0; //comment at end of a line

      if (!strncmp(buf, propname, strlen(propname)))
      {
        result = strchr(buf, '=');
	result++;
	strcpy(buffer, stpblk(result));
				
        fclose(fh);
	return buffer;
      }	
    }
    fclose(fh);
  }

  return getenv(propname) != NULL ? getenv(propname) : def;
}

#if     (__UNIX__  || __BEOS__)
static int stdin_tty = 1;                       // 1 => stdin is a tty, 0 => it's not
static tty_t oldtty, newtty;

void set_tty(tty_t param)
{
  if (stdin_tty && tcsetattr(STDIN_FILENO, TCSANOW, &param) == -1)
  {
    fprintf(stderr, "Could not set tty parameters\n");
    exit(100);
  }
}

void init_conio(void)
{
  if (!isatty(STDIN_FILENO))
  {
    stdin_tty = 0;
    return;                                     // rest is nonsense if not a tty
  }

  if (tcgetattr(STDIN_FILENO, &oldtty) == -1)
  {
    fprintf(stderr, "Could not get tty parameters\n");
    exit(101);
  }

  if (atexit(deinit_conio) == -1)
  {
    fprintf(stderr, "Could not register function with atexit()\n");
    exit(102);
  }

  newtty = oldtty;
  newtty.c_lflag &= ~(ICANON | ECHO);
  newtty.c_lflag |= ISIG;
  newtty.c_cc[VMIN] = 1;                        // if VMIN != 0, read calls
  newtty.c_cc[VTIME] = 0;                       //  block (wait for input)

  set_tty(newtty);
}

void deinit_conio(void)
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldtty);
}

int kbhit(void)
{
  tty_t tmptty = newtty;
  int ch, key_pressed;

  tmptty.c_cc[VMIN] = 0;
  set_tty(tmptty);

  if ((ch = fgetc(stdin)) != EOF)
  {
    key_pressed = 1;
    ungetc(ch, stdin);
  }
  else
    key_pressed = 0;

  set_tty(newtty);

  return key_pressed;
}
#endif
