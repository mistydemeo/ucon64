/*
swc.c - Super Wild Card support for uCON64

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
#include "swc.h"

/*
swc_main(int argc, char *argv[])
{
   FILE *file_ptr;
   int i=0, totalblocks, currentblock=0;
   char filename[12];
   char block[8192];
   int filesize=0;
   
   system("clear");
   printf("SWCSend -- x86 Unix SNES Sender by madman\n\n");
   if (argc==1) {
     printf ("Usage: swcsend filename\n\n");
      exit(0); 
   }
      
   ioperm(0x378, 3, 1);
   ioperm(0x80, 1, 1);
   
   file_ptr=fopen(argv[1], "rb");
   if (file_ptr==NULL) {
      printf ("Could not open the file %s\n\n", argv[1]);
      exit(0); 
   }
   
   PrintInfo(file_ptr, &filesize);
   
   totalblocks=(filesize*131072)/8192;

   InitSWC();

   while (!feof(file_ptr)) {
      PrintBar(totalblocks, i);
      fread(block, 8192, 1, file_ptr);
      SendData(5, i&0xFF, (i>>8)&0xFF, 0, 0);
      SendBlock(block);
      i++;
   }  

   SendData(6, 1, 0x00, 0, 0);	  
   SendByte(0x00);
   outb_p(0x09, 0x37a);
   
   printf ("\n");
}
*/



void SendBlock (char *Data)
{
   int i;
   char crc=0x81;
   
   SendData(0, 0x00, 0x80, 0x00, 0x20);
   for(i=0; i<8192; i++)
     {
	SendByte(Data[i]);
	crc^=Data[i];
	// Data++;
     }
   SendByte(crc);
}

void SendByte(char Data)
{
/*
   outb(Data, 0x378);  
   outb(inb(0x37a)^1, 0x37a);
   while ((inb(0x379)^0x80)==0x9F) 
*/
   out1byte(0x378,Data);  
   out1byte( 0x37a,in1byte(0x37a)^1);
   while ((in1byte(0x379)^0x80)==0x9F) 
	
     printf ("%c", 0);
}

void SendData (char Byte4, char Byte5, char Byte6, char Byte7, char Byte8)
{
   SendByte(0xD5);
   SendByte(0xAA);
   SendByte(0x96);
   SendByte(Byte4);
   SendByte(Byte5);
   SendByte(Byte6);
   SendByte(Byte7);
   SendByte(Byte8);
   SendByte(0x81^Byte4^Byte5^Byte6^Byte7^Byte8);
}

void InitSWC()
{
   SendData (6, 0, 0, 0, 0);
}

int swc_read(		char *filename
			,unsigned int parport
)
{
//TODO swc_read()
	return(0);
}                        
                        
int swc_write(		char *filename
			,long start
			,long len
			,unsigned int parport
)
{
char buf[8192];
int x=0;
FILE *fh;
unsigned long size,inittime,pos;

   ioperm(0x378, 3, 1);
   ioperm(0x80, 1, 1);
//if(initCommunication(parport)==-1)return(-1);
InitSWC();

inittime=time(0);

//  	if(sendUploadHeader(parport,filename,(quickftell(filename)-start))!=0)return(-1);
	
	if(!(fh=fopen(filename,"rb")))return(-1);

	printf("Send: %ld Bytes (%.4f Mb)\n\n", (quickftell(filename)-start), (float)(quickftell(filename)-start)/MBIT);
	size=quickftell(filename);
	for(;;)
	{
                if(!(pos=fread(buf,1,8192,fh)))break;
//    		if(parport_write(buf,pos,parport)!=0)break;
      SendData(5, x&0xFF, (x>>8)&0xFF, 0,0);
      SendBlock(buf);
	    	size=size-pos;
		parport_gauge(inittime,(quickftell(filename)-start)-size,(quickftell(filename)-start));
		x++;
	}

   SendData(6, 1, 0x00, 0, 0);	  
   SendByte(0x00);
   out1byte(0x37a,0x09);
   sync();
fclose(fh);
return(0);
}


int swc_usage(int argc,char *argv[])
{
if(argcmp(argc,argv,"-help"))printf("\n%s\n",swc_TITLE);


printf("TODO:  -xswc    send/receive ROM to/from Super Wild Card*/(all)SWC; $FILE=PORT\n\
		receives automatically when $ROM does not exist\n\
");

if(argcmp(argc,argv,"-help"))
{
//TODO more info like technical info about cabeling and stuff for the copier


}
	return(0);
}
