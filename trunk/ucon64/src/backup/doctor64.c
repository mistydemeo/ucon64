/*
doctor64.c - Bung Doctor 64 support for uCON64

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
#include "doctor64.h"

#define SYNC_MAX_CNT 8192
#define SYNC_MAX_TRY 32
#define SEND_MAX_WAIT 0x300000
#define REC_HIGH_NIBBLE 0x80
#define REC_LOW_NIBBLE 0x00
#define REC_MAX_WAIT SEND_MAX_WAIT


int parport_write(char src[], unsigned int len, unsigned int parport)
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

int parport_read(char dest[], unsigned int len, unsigned int parport)
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

int syncHeader(unsigned int baseport)
{
int i=0;

out1byte(baseport, 0);                        /* data = 00000000    */
out1byte(baseport+2, 4);                      /* ~strobe=0          */
while( i<SYNC_MAX_CNT )
{
	if( (in1byte(baseport+2) & 8) == 0 )      /* wait for select=0  */
	{
      		out1byte(baseport, 0xaa);                 /* data = 10101010    */
      		out1byte(baseport+2, 0);                  /* ~strobe=0, ~init=0 */
      		while( i<SYNC_MAX_CNT )
      		{
        		if( (in1byte(baseport+2) & 8) != 0 )  /* wait for select=1  */
        		{
				out1byte(baseport+2, 4);              /* ~strobe=0          */
          			while( i<SYNC_MAX_CNT )
          			{
            				if( (in1byte(baseport+2) & 8) == 0 ) /* w for select=0  */
            				{
				              out1byte(baseport, 0x55);         /* data = 01010101    */
				              out1byte(baseport+2, 0);          /* ~strobe=0, ~init=0 */
				              while( i<SYNC_MAX_CNT ) 
				              {
                					if( (in1byte(baseport+2) & 8) != 0 )   /* w select=1 */
                					{
                  						out1byte(baseport+2, 4);      /* ~strobe=0          */
                  						while( i<SYNC_MAX_CNT )
                  						{
                    							if( (in1byte(baseport+2) & 8) == 0)  /* select=0 */
                      							{
                      								return 0;
			                    				}
                    							i++;
                  						}
                					}
                					i++;
              					}
            				}
            				i++;
          			}
        		}
        		i++;
      		}
      		i++;
    	}
    	i++;
}
out1byte(baseport+2, 4);
return 1;
}

int initCommunication(unsigned int port)
{
	int i;
	for( i=0; i<SYNC_MAX_TRY; i++ ) 
	{
		if( syncHeader(port) == 0 ) break;
	}
	if( i>=SYNC_MAX_TRY )  return(-1);
	return(0);
}

int checkSync(unsigned int baseport)
{
  int i, j;

for( i=0; i<SYNC_MAX_CNT; i++ )
{
    	if( ((in1byte(baseport+2) & 3) == 3) || ((in1byte(baseport+2) & 3) == 0) )
    	{
      		out1byte(baseport, 0);                        /* ~strobe, auto feed */
      		for( j=0; j<SYNC_MAX_CNT; j++ ) 
      		{
        		if( (in1byte(baseport+1) & 0x80) == 0 )   /* wait for ~busy=0 */
        		{
          			return 0;
        		}
      		}
      		return 1;
    	}
}
return 1;
}

int sendFilename(unsigned int baseport, char name[])
{
  int i;
  char *c;
  char mname[12];

  memset(mname, ' ', 11);
  c = (strrchr(name, FILE_SEPARATOR));
  if( c == NULL ) {
    c = name;
  } else {
    c++;
  }
  for( i=0; i<8 && *c!='.' && *c!='\0' ; i++, c++ ) mname[i] = toupper(*c);
  c = strrchr(c, '.');
  if( c != NULL ) {
    c++;
    for( i=8; i<11 && *c!='\0'; i++, c++ ) mname[i] = toupper(*c);
  }

  return parport_write(mname, 11,baseport);
}

int sendUploadHeader(unsigned int baseport, char name[], long len)
{
  char mname[12];

  char lenbuffer[4];
  static char protocolId[] = "GD6R\1";

  if( parport_write(protocolId, strlen(protocolId), baseport) != 0 ) return 1;

  lenbuffer[0] = (len    );
  lenbuffer[1] = (len>> 8);
  lenbuffer[2] = (len>>16);
  lenbuffer[3] = (len>>24);
  if(parport_write(lenbuffer, 4, baseport) != 0 ) return 1;

  memset(mname, ' ', 11);
//  if( 
sendFilename(baseport,name);
//   != 0 ) return 1;
  return 0;
}

int sendDownloadHeader(unsigned int baseport, char name[], long *len)
{
char mname[12];

  static char protocolId[] = "GD6W";
  unsigned char recbuffer[15];

  if( parport_write(protocolId, strlen(protocolId),baseport) != 0 ) return 1;
memset(mname, ' ', 11);
  if( parport_write(mname, 11, baseport) != 0 ) return 1;
  if( checkSync(baseport) != 0 ) return 1;

  if( parport_read((char *)recbuffer, 1,baseport) != 0 ) return 1;
  if( recbuffer[0] != 1 ) {
    return(-1);
  }
  if(parport_read((char *)recbuffer, 15,baseport) != 0 ) return 1;
  *len = (long)recbuffer[0] | ((long)recbuffer[1] << 8) | ((long)recbuffer[2] << 16) | ((long)recbuffer[3] << 24);
  return(0);
}


int doctor64_read(	char *filename
			,unsigned int parport
)
{
char buf[32768];
FILE *fh;
unsigned long size,inittime;

if(initCommunication(parport)==-1)return(-1);

inittime=time(0);
  	if(sendDownloadHeader(parport,filename,&size)!=0)return(-1);
	if(!(fh=fopen(filename,"wb")))return(-1);
	printf("Receive: %ld Bytes (%.4f Mb)\n\n", size, (float)size/MBIT);
		
  	for(;;)
  	{
    		if(parport_read(buf,sizeof(buf),parport)!=0)
	  	{
			fclose(fh);
			return(0);
	  	}
    		fwrite(buf,1,sizeof(buf),fh);
		parport_gauge(inittime,quickftell(filename),size);
  	}
sync();
fclose(fh);
return(0);
}



int doctor64_write(	char *filename
			,long start
			,long len
			,unsigned int parport
)
{
char buf[32768];
FILE *fh;
unsigned long size,inittime,pos;

if(initCommunication(parport)==-1)return(-1);

inittime=time(0);

  	if(sendUploadHeader(parport,filename,(quickftell(filename)-start))!=0)return(-1);
	
	if(!(fh=fopen(filename,"rb")))return(-1);

	printf("Send: %ld Bytes (%.4f Mb)\n\n", (quickftell(filename)-start), (float)(quickftell(filename)-start)/MBIT);
	size=quickftell(filename);
	
	for(;;)
	{
                if(!(pos=fread(buf,1,sizeof(buf),fh)))break;
    		if(parport_write(buf,pos,parport)!=0)break;
	    	size=size-pos;
		parport_gauge(inittime,(quickftell(filename)-start)-size,(quickftell(filename)-start));
	}
fclose(fh);
return(0);
}


int doctor64_usage(int argc,char *argv[])
{
if(argcmp(argc,argv,"-help"))printf("%s\n",doctor64_TITLE);

printf(	"  -xv64		send/receive ROM to/from Doctor V64; $FILE=PORT\n"
	"		receives automatically when $ROM does not exist\n");

if(argcmp(argc,argv,"-help"))
{
//TODO more info like technical info about cabeling and stuff for the copier

}
	return(0);
}
