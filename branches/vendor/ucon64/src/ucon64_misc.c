#include "ucon64_misc.h"

#define MAXBUFSIZE 32768

char hexDigit(	int value
)
{
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

int hexValue(	char digit
) 
{
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

int hexByteValue(	char x
		,char y
)
{
  return (hexValue(x) << 4) + hexValue(y);
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
		if(argv[x][0]=='-'&&!strdcmp(argv[x],str))return(x);
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

int findlwr(	char *str
)
{
//searches the string for ANY lowercase char
	char *str2;

	if(!(str2=strrchr(str,'/')))str2=str;//strip path if it is a filename
	else str2++;

	while(*str2)
	{                
		if( isalpha(*str2) && islower(*str2) )return(TRUE);
		str2++;
	}
	return(FALSE);
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
	filename[strcspn(filename,".")]=0;
	strcat(filename,ext);
	
	return(	
		findlwr(filename) ?
		strlwr(filename) :
		strupr(filename)
	);
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
		if( !x || !(x%16) ) printf("%08lx  ",x+start);

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



if(access( ((dir[0]!=0)?dir:".") ,R_OK)==-1 ||
(dp=opendir( ((dir[0]!=0)?dir:".") ))==NULL)return(-1);

chdir( ((dir[0]!=0)?dir:".") );

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


unsigned long CRCTable[ 256 ];

#define CRC32_POLYNOMIAL     0xEDB88320L

void BuildCRCTable()
{
    int i;
    int j;
    unsigned long crc;

    for ( i = 0; i <= 255 ; i++ ) {
        crc = i;
        for ( j = 8 ; j > 0; j-- ) {
            if ( crc & 1 )
                crc = ( crc >> 1 ) ^ CRC32_POLYNOMIAL;
            else
                crc >>= 1;
        }
        CRCTable[ i ] = crc;
    }
}


unsigned long CalculateBufferCRC(	unsigned int count
					,unsigned long crc
					,void *buffer
)
{
    unsigned char *p;
    unsigned long temp1;
    unsigned long temp2;

    p = (unsigned char*) buffer;
    while ( count-- != 0 ) {
        temp1 = ( crc >> 8 ) & 0x00FFFFFFL;
        temp2 = CRCTable[ ( (int) crc ^ *p++ ) & 0xff ];
        crc = temp1 ^ temp2;
    }
    return( crc );
}

unsigned long CalculateFileCRC(	FILE *file
)
{
    unsigned long crc;
    int count;
    unsigned char buffer[ 512 ];
    int i;

    crc = 0xFFFFFFFFL;
    i = 0;
    for ( ; ; ) {
        count = fread( buffer ,1 ,512 ,file );
        if ( ( i++ % 32 ) == 0 )
//            putc( '.' ,stdout );
        if ( count == 0 )
            break;
        crc = CalculateBufferCRC( count ,crc ,buffer );
    }
//    putc( ' ' ,stdout );
    return( crc ^= 0xFFFFFFFFL );
}

unsigned long fileCRC32(	char *filename
				,long start
)
{
	unsigned long val;
	FILE *fh;

	BuildCRCTable();

        if(!(fh=fopen(filename,"rb")))return(-1);
	fseek(fh,start,SEEK_SET);
        val=CalculateFileCRC( fh );
	fclose(fh);

	return(val);
}




long quickftell(	char *filename
)
{
	long size;
	FILE *fh;

	if(!(fh=fopen(filename,"rb")))return(-1);
	fseek(fh,0,SEEK_END);
	size=ftell(fh);
	fclose(fh);
	return(size);
}


long filencmp(	char *filename
		,char *search
		,long start
		,long len
)
{
	register unsigned long x,y;
	char *buf;
	FILE *fh;

	len=( ( (quickftell(filename)-start) < len ) ? (quickftell(filename)-start) : len );

	x=0;
	if(!(fh=fopen(filename,"rb")))return(-1);

	if(!(buf=(char *)malloc(((quickftell(filename)-start)+2)*sizeof(char))))
	{
		fclose(fh);
	        return(-1);
	}

	fseek(fh,start,SEEK_SET);
	if(!fread(buf,(quickftell(filename)-start),1,fh))
	{
		free(buf);
		fclose(fh);
		return(-1);
	}	

	fclose(fh);

	y=(quickftell(filename)-start);
	while((x+len)<y)
	{
		if(!strncmp(&buf[x],search,len))return(x+start);
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
	result=fread((void *)dest,len,1,fh);
//	dest[len]=0;
	fclose(fh);
	return(result);
}



size_t quickfwrite(	const void *src
			,size_t start
			,size_t len
			,char *dest
)
{
	size_t result;
	FILE *fh;

	if( !( fh=fopen(dest ,( access(dest,F_OK)!=0 ) ? "wb" : "r+b" ) ) )return(-1);
	fseek(fh,start,SEEK_SET);
	result=fwrite(src,len,1,fh);
	fclose(fh);
	return(result);
}


int quickfgetc(	char *filename
		,long pos
)
{
	int c;
	FILE *fh;

        if(!(fh=fopen(filename,"rb")) || fseek(fh,pos,SEEK_SET)!=0)return(EOF);
	c=fgetc(fh);
	fclose(fh);
	
	return(c);
}

int filehexdump(	char *filename
			,long start
			,long len
)
{
	register long x;
	char buf[256];
	int dump;
	FILE *fh;

	len=( ( (quickftell(filename)-start) < len ) ? (quickftell(filename)-start) : len );

	if(!(fh=fopen(filename,"rb")))return(-1);
	
	buf[16]=0;

	for(x=0;x<len;x++)
	{
		if(x!=0&&!(x%16))printf("%s\n",buf);
		if(!x||!(x%16))printf("%08lx  ",x+start);

		dump=quickfgetc(filename,x+start);

		printf("%02x %s"
				,dump&0xff
				,( ( !( (x+1)%4 ) ) ? " " : "" )
			);

		buf[(x%16)]=isprint(dump)?dump:'.';
		buf[(x%16)+1]=0;
	}
	printf("%s\n",buf);
	fclose(fh);
	return(0);
}



size_t filepad(	char *filename
		,long start
		,long unit
)
{
	size_t size;

	size=(quickftell(filename)-start);

	if(!(size%unit))return(size);

	size=(size_t)size/unit;
	size=size+1;
	size=size*unit;

	truncate(filename,size+start);
	return(size);
}

long filetestpad(	char *filename
)
{
	long size;
	register long x;
	int y;
	char *buf;
	FILE *fh;
	
	size=quickftell(filename);

	if(!(fh=fopen(filename,"rb")))return(-1);

	if( !( buf=(char *)malloc( (size+2)*sizeof(char) ) ) )
	{
		fclose(fh);
       		return(-1);
	}
	if( !fread(buf,size,1,fh) )
	{
		fclose(fh);
		free(buf);
		return(-1);
	}
	fclose(fh);

	y=buf[size-1]&0xff;
	x=size-2;
	while(y==(buf[x]&0xff))x--;
//	if(y!=(buf[x+1]&0xff))x++;

	free(buf);
	if( ( ( ( size )-x )-1 )==1)return(0);
	return( ( ( size )-x )-1 );
}




int filecopy(	char *src
		,long start
		,long len
		,char *dest
		,char *mode
)
{
	long size;
	char buf[MAXBUFSIZE];
	FILE *fh,*fh2;

	len=( ( (quickftell(src)-start) < len ) ? (quickftell(src)-start) : len );

	if(!strdcmp(dest,src))return(-1);
	if(!(fh=fopen(src,"rb")))return(-1);
	if(!(fh2=fopen(dest,mode)))
	{
	        fclose(fh);
		return(-1);
	}

	fseek(fh,0,SEEK_END);
	size=(ftell(fh)-start)%MAXBUFSIZE;
	fseek(fh,start,SEEK_SET);

	fseek(fh2,0,SEEK_END);

	while(fread(buf,MAXBUFSIZE,1,fh))fwrite(buf,MAXBUFSIZE,1,fh2);

	fread(buf,size,1,fh);
	fwrite(buf,size,1,fh2);
		
	fclose(fh);
	fclose(fh2);

	return(0);
}

char *filebackup(	char *filename
)
{
	char buf[MAXBUFSIZE];

	if(!access(filename,F_OK))
	{
		strcpy(buf,filename);
//		newext(buf,".BAK");
		strcat(buf,(findlwr(buf)? ".bak" : ".BAK"));//~
		filecopy(filename,0,quickftell(filename),buf,"wb");
	}
	return(filename);
}

unsigned long filefile(	char *filename
			,long start
			,char *filename2
			,long start2
			,int similar
)
{
	register long size,x,len;
	char *buf,*buf2;
	FILE *fh,*fh2;
	
	if(!strdcmp(filename,filename2))return(0);
	
        if(!(fh=fopen(filename,"rb")))return(-1);
	if(!(fh2=fopen(filename2,"rb")))
	{
		fclose(fh);
		return(-1);
	}

	if( !(buf=(char *)malloc( ( quickftell(filename)+2 ) *sizeof(char) ) ) )
	{
		fclose(fh);
		fclose(fh2);
        	return(-1);
	}

	if( !(buf2=(char *)malloc( ( quickftell(filename2)+2 ) *sizeof(char) ) ) )
	{
		free(buf);
		fclose(fh);
		fclose(fh2);
		return(-1);
	}

	if(	!fread(buf,quickftell(filename),1,fh) ||
		!fread(buf2,quickftell(filename2),1,fh2)
	)
	{
		free(buf);
		free(buf2);
		fclose(fh);
		fclose(fh2);
		return(-1);
	}	
	fclose(fh);
	fclose(fh2);

	size=(	(quickftell(filename)-start)<(quickftell(filename2)-start2) ?
		(quickftell(filename)-start) :
		(quickftell(filename2)-start2)
	);

	for(x=0;x<=size;x++)
	{
		if
		(
			( similar==FALSE && buf[x+start] != buf2[x+start2] ) ||
			( similar==TRUE && buf[x+start] == buf2[x+start2] )
		)
		{
			len=0;
			while(
				( similar==TRUE ) ?
				(buf[x+start+len]==buf2[x+start2+len]) :
				(buf[x+start+len]!=buf2[x+start2+len])
			)len++;

			strhexdump(buf,x+start,len);
			printf("\n");
			x+=len;
		}
	}
	free(buf);
	free(buf2);
	return(x);
}





int filereplace(	char *filename
			,long start
			,char *search
			,long slen
			,char *replace
			,long rlen
)
{
	register long x;
	x=start;
	
	for(;;)
	{
		if((x=filencmp(filename,search,x,slen))==-1)return(0);
		filehexdump(filename,x,slen);
		quickfwrite(replace,x,rlen,filename);
		filehexdump(filename,x,rlen);
		printf("\n");
		x++;
	}
}





int fileswap(	char *filename
		,long start
		,long len
)
{
	register unsigned long x;
	size_t size;
	FILE *in,*out;
	char buf[MAXBUFSIZE],buf2[3],buf3;

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


/*
unsigned char inportb(unsigned int arg1)
{
  __asm__("
  movl    8(%ebp),%edx
  inb     %dx,%al
  movzbl  %al,%eax                  
  ");
}

unsigned char outportb(unsigned int arg1,unsigned int arg2)
{
  __asm__("
  movl    0x8(%ebp),%edx
  movl    0xc(%ebp),%eax
  outb    %al,%dx
  ");
}

unsigned short int inport(unsigned int arg1)
{
  __asm__("
  movl    8(%ebp),%edx
  inw     %dx,%ax
  ");
}

unsigned short int outport(unsigned int arg1,unsigned int arg2)
{
  __asm__("
  movl    0x8(%ebp),%edx
  movl    0xc(%ebp),%eax
  outw    %ax,%dx
  ");
}
*/
void out1byte(unsigned int port, unsigned char c)
{
  __asm__ volatile ("outb %0,%1"
		    ::"a" ((char) c), "d"((unsigned short) port));
}

unsigned char in1byte(unsigned int port)
{
  char _v;
  __asm__ volatile ("inb %1,%0"
		    :"=a" (_v):"d"((unsigned short) port));

  return _v;
}


#define DETECT_MAX_CNT 1000

int detectParPort(unsigned int port)
{
  int i;

  if(ioperm(port, 1, 1)==-1)return(-1);
  out1byte(port, 0xaa);
  for( i=0; i<DETECT_MAX_CNT; i++ )
   {
    if( in1byte(port) == 0xaa ) break;
  }
  if( i < DETECT_MAX_CNT ) {
    out1byte(port, 0x55);
    for( i=0; i<DETECT_MAX_CNT; i++ ) {
      if( in1byte(port) == 0x55 ) break;
    }
  }
  if(ioperm(port, 1, 0)==-1)return(-1);
  
  if( i >= DETECT_MAX_CNT ) return 0;
  return 1;
}



unsigned int parport_probe(unsigned int port)
{
  uid_t uid;
  unsigned int parPortAddresses[] = { 0x3bc, 0x378, 0x278 };
  int i;
  
  if( port == 0 ) port = 1;
  if( port <= 3 ) {
    for( i=0; i<3; i++ ) {
      port -= detectParPort(parPortAddresses[i]);
      if( port == 0 ) {
        port = parPortAddresses[i];
        break;
      }
    }
    if( i >= 3 ) return (0);
  } else {
    if( (port != parPortAddresses[0]) && (port != parPortAddresses[1]) && (port != parPortAddresses[2]) ) {
	return(0);
    }
  }
  if( port != 0 ) {
    if(ioperm(port, 3, 1)==-1)return(0);
    uid = getuid();
    seteuid(uid);
  }
  return port;
}



int parport_gauge(	time_t init_time
		,long pos
		,long size
)
{
	long cps=0;
	time_t curr,left;
	char buf[256];
	
	if( !( curr=( time(0)-init_time ) ) )return(0);

	cps=pos/curr;
	left=(size-pos)/cps;

	buf[0]=0;
	strncat(buf,"===============================",(size_t)((long)24*pos/size));
	strcat(buf,"-------------------------------");
	buf[24]=0;

	printf("\r%10lu Bytes [%s] %lu%% ,CPS=%lu ,"
	,pos
	,buf
	,(unsigned int)100*pos/size
	,(unsigned long)cps
	);

	if(pos==size)
	{
		printf("TOTAL=%03ld:%02ld "
		,(long)(curr/60)
		,((long)curr)%60
		);
	}
	else
	{
		printf("ETA=%03ld:%02ld   "
		,(long)(left/60)
		,((long)left)%60
		);
	}
	fflush(stdout);

	return(0);
}




#define SEND_MAX_WAIT 0x300000
                                                
int parport_write(	char src[]
			,unsigned int len
			,unsigned int parport
)
{
long maxwait;
unsigned int i;

for( i=0; i<len; i++ ) 
{
	maxwait = SEND_MAX_WAIT;
	if( (in1byte(parport+2) & 1) == 0 )       /* check ~strobe */
	{
		while( ((in1byte(parport+2) & 2) != 0) && maxwait-- ) ;  /* wait for */
		if( maxwait <= 0 ) return 1;                        /* auto feed == 0 */
		out1byte(parport, src[i]);         /* write data    */
		out1byte(parport+2, 5);                  /* ~strobe = 1   */
	}
	else
	{
		while( ((in1byte(parport+2) & 2) == 0) && maxwait-- ) ;  /* wait for */
		if( maxwait <= 0 ) return 1;                        /* auto feed == 1 */
		out1byte(parport, src[i]);         /* write data    */
		out1byte(parport+2, 4);                  /* ~strobe = 0   */
	}
}
return 0;
}




#define REC_HIGH_NIBBLE 0x80
#define REC_LOW_NIBBLE 0x00
#define REC_MAX_WAIT SEND_MAX_WAIT

int parport_read(	char dest[]
			,unsigned int len
			,unsigned int parport
)
{
int i;
long maxwait;
unsigned char c;

for( i=0; i<len; i++ ) 
{
	out1byte(parport, REC_HIGH_NIBBLE);
	maxwait = REC_MAX_WAIT;
	while( ((in1byte(parport+1) & 0x80) == 0) && maxwait-- ) ; /* wait for ~busy=1 */
	if( maxwait <= 0 ) return len-i;
	c = (in1byte(parport+1) >> 3) & 0x0f; /* ~ack, pe, slct, ~error */

	out1byte(parport, REC_LOW_NIBBLE);
	maxwait = REC_MAX_WAIT;
	while( ((in1byte(parport+1) & 0x80) != 0) && maxwait-- ) ; /* wait for ~busy=0 */
	if( maxwait <= 0 ) return len-i;
	c |= (in1byte(parport+1) << 1) & 0xf0; /* ~ack, pe, slct, ~error */

	dest[i] = c;
}
out1byte(parport, REC_HIGH_NIBBLE);
return 0;
}
