/*
ucon64_misc.c - miscellaneous functions for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)


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

#ifdef BACKUP

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


#define getParPort(x) parport_probe(x)
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

//TODO show average speed after transfer

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

#endif

int testsplit(char *filename)
{
//TODO ignore paths
long x=0;
char buf[4096];

strcpy(buf,filenameonly(filename));
if(!strchr(buf,'.'))return(0);

strcpy(buf,filename);
buf[strcspn(buf,".")-1]++;
while(!access(buf,F_OK))
{
	buf[strcspn(buf,".")-1]++;
	x++;
}

if(x!=0)return(x);

strcpy(buf,filename);
buf[strcspn(buf,".")+1]++;
while(!access(buf,F_OK))
{
	buf[strcspn(buf,".")+1]++;
	x++;
}

return(x);
}