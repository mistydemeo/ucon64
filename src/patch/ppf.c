/*
ppf.c - Playstation Patch Format support for uCON64

written by ???? - ???? Icarus/Paradox
                  2001 NoisyB (noisyb@gmx.net)


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
#include "../ucon64.h"

/*
 * MakePPF v2.0 Sourcecode by Icarus/Paradox
 * enter "gcc makeppf.c" on linux/unix to compile!
 *
 * Feel free to optimize speed or something!
 *
 */

int makeppf_main(int argc, char *argv[])
{
#define null 0

FILE *originalbin;
FILE *patchedbin;
FILE *ppffile;
FILE *fileid = NULL;
char desc[52];
char block[1025];
char fileidbuf[3073];
char enc=1;
char obuf[512];
char pbuf[512];
char cbuf[512];
unsigned char anz; /* Hell YES! UNSIGNED CHAR! */
int i, z, a, x, y, osize, psize, fsize, seekpos=0, pos;


//        printf("MakePPF v2.0 Linux/Unix by Icarus/Paradox\n");
        if(argc==1||argc<4){
//        printf("Usage: MakePPF <Original Bin> <Patched Bin> <ppffile> [file_id.diz]\n");
        exit(0);
        }

	/* Open all neccessary files */
        originalbin=fopen(argv[1], "rb");
        if(originalbin==null){
        printf("File %s does not exist. (Original BIN)\n",argv[1]);
        exit(0);
        }
        patchedbin=fopen(argv[2], "rb");
        if(patchedbin==null){
        printf("File %s does not exist. (Patched BIN)\n",argv[2]);
        fclose(originalbin);
        exit(0);
        }
        fseek(originalbin, 0, SEEK_END);
        osize=ftell(originalbin);
        fseek(patchedbin, 0, SEEK_END);
        psize=ftell(patchedbin);
        if(osize!=psize){
        printf("Error: Filesize does not match - byebye..\n");
        fclose(originalbin);
        fclose(patchedbin);
        exit(0);
        }
        if(argc>=5){
        fileid=fopen(argv[4], "rb");
        if(fileid==null){
        printf("File %s does not exist. (File_id.diz)\n",argv[4]);
        fclose(patchedbin);
        fclose(originalbin);
        exit(0);
        }}
        ppffile=fopen(argv[3], "wb+");
        if(ppffile==null){
        printf("Could not create file %s\n",argv[3]);
        if(argc>=5) fclose(fileid);
        fclose(patchedbin);
        fclose(originalbin);
        exit(0);
        }

        printf("Enter PPF-Patch description (max. 50 chars):\n[---------1o--------2o--------3o--------4o---------]\n ");
        fgets(desc, 50, stdin);
        for(i=0;i<50;i++){
        if(desc[i]==0x00) desc[i]=0x20;
        }

	/* creating PPF2.0 header */
        printf("Creating PPF2.0 header data.. ");
        fwrite("PPF20", 5, 1, ppffile); /* Magic (PPF20) */
        fwrite(&enc, 1, 1, ppffile);	/* Enc.Method (0x01) */
        fwrite(desc, 50, 1, ppffile);	/* Description line */
        fwrite(&osize, 4, 1,ppffile);	/* BINfile size */
        fseek(originalbin, 0x9320, SEEK_SET);
        fread(block, 1024, 1, originalbin);
        fwrite(block, 1024, 1,ppffile);	/* 1024 byte block */
        printf("done!\n");

        printf("Writing patchdata, please wait.. ");
	fflush(stdout);
	
	/* Finding changes.. i know it slow.. so feel free to optimize */
        z=(osize/255);
        a=osize-(z*255);
        do{
           fseek(originalbin, seekpos, SEEK_SET);
           fseek(patchedbin, seekpos, SEEK_SET);
           fread(obuf, 255, 1,originalbin);
           fread(pbuf, 255, 1,patchedbin);
           x=0; pos=0;
           do{
              if(obuf[x]!=pbuf[x]){
                  pos=seekpos+x;
                  y=0;
                  anz=0;
                  do{
                     cbuf[y]=pbuf[x];
                     anz++; x++; y++;
                  }while(x!=255&&obuf[x]!=pbuf[x]);
                  fwrite(&pos, 4, 1, ppffile);
                  fwrite(&anz, 1, 1, ppffile);
                  fwrite(cbuf, anz, 1, ppffile);
              }else x++;

           }while(x!=255);

        seekpos+=255;
        z--;
        }
        while(z!=0);

        if(a!=0){
        fseek(originalbin, seekpos, SEEK_SET);
        fseek(patchedbin, seekpos, SEEK_SET);
        fread(obuf, 255, 1,originalbin);
        fread(pbuf, 255, 1,patchedbin);
        x=0; pos=0;
        do{
           if(obuf[x]!=pbuf[x]){
               pos=seekpos+x;
               y=0;
               anz=0;
               do{
                  cbuf[y]=pbuf[x];
                  anz++; x++; y++;
               }while(x!=a&&obuf[x]!=pbuf[x]);
               fwrite(&pos, 4, 1, ppffile);
               fwrite(&anz, 1, 1, ppffile);
               fwrite(cbuf, anz, 1, ppffile);
           }else x++;
         }while(x!=a);
        }
        printf("done!\n");

	/* was a file_id.diz argument present? */
        if(argc>=5){
        printf("Adding file_id.diz .. ");
        fseek(fileid, 0, SEEK_END);
        fsize=ftell(fileid);
        fseek(fileid, 0, SEEK_SET);
        if(fsize>3072) fsize=3072;	/* File id only up to 3072 bytes! */
        fread(fileidbuf, fsize, 1 ,fileid);
        fwrite("@BEGIN_FILE_ID.DIZ", 18, 1, ppffile); /* Write the shit! */
        fwrite(fileidbuf, fsize, 1, ppffile);
        fwrite("@END_FILE_ID.DIZ", 16, 1, ppffile);
        fwrite(&fsize, 4, 1, ppffile);
        printf("done!\n");
        }

        fclose(ppffile);	/* Thats it! */
        fclose(originalbin);
        fclose(patchedbin);
        return 0;
}

/*
 * ApplyPPF v2.0 for Linux/Unix. Coded by Icarus/Paradox 2k
 * If you want to compile applyppf just enter "gcc applyppf.c"
 * that's it but i think the Linux users know :)
 * 
 * This one applies both, PPF1.0 and PPF2.0 patches.
 *
 * Sorry for the bad code i had no time for some cleanup.. but
 * it works 100% ! Byebye!
 *
 * Btw feel free to use this in your own projects etc of course!!
 */
 
 
#include <stdio.h>
#include <string.h>
int applyppf_main(int argc, char *argv[])
{
#define null 0

FILE *binfile;
FILE *ppffile;
char buffer[5];
char method, in;
char desc[50];
char diz[3072];
int  dizlen, binlen, dizyn, dizlensave = 0;
char ppfmem[512];
int  count, seekpos, pos, anz;
char ppfblock[1025];
char binblock[1025];

	
//        printf("ApplyPPF v2.0 for Linux/Unix (c) Icarus/Paradox\n");
        if(argc==1){
//        printf("Usage: ApplyPPF <Binfile> <PPF-File>\n");
        exit(0);
        }

        /* Open the bin and ppf file */
        binfile=fopen(filebackup(argv[1]), "rb+");
        if(binfile==null){
        printf("File %s does not exist.\n",argv[1]);
        exit(0);
        }
        ppffile=fopen(argv[2], "rb");
        if(ppffile==null){
        printf("File %s does not exist.\n",argv[2]);
        exit(0);
        }

        /* Is it a PPF File ? */
        fread(buffer, 3, 1, ppffile);
        if(strcmp("PPF", buffer)){
        printf("File %s if *NO* PPF file.\n",argv[2]);
        fclose(ppffile);
        fclose(binfile);
        exit(0);
        }

        /* What encoding Method? PPF1.0 or PPF2.0? */
        fseek(ppffile, 5, SEEK_SET);
        fread(&method, 1, 1,ppffile);

        switch(method)
        {
                case 0:
			 /* Show PPF-Patchinformation. */
			 /* This is a PPF 1.0 Patch! */
                         fseek(ppffile, 6,SEEK_SET);  /* Read Desc.line */
                         fread(desc, 50, 1,ppffile);
                         printf("\nFilename       : %s\n",argv[2]);
                         printf("Enc. Method    : %d (PPF1.0)\n",method);
                         printf("Description    : %s\n",desc);
                         printf("File_id.diz    : no\n\n");
                         
			 /* Calculate the count for patching the image later */
			 /* Easy calculation on a PPF1.0 Patch! */
                         fseek(ppffile, 0, SEEK_END);
                         count=ftell(ppffile);
                         count-=56;
                         seekpos=56;
                         printf("Patching ... ");
                         break;
                case 1:
			 /* Show PPF-Patchinformation. */
			 /* This is a PPF 2.0 Patch! */
                         fseek(ppffile, 6,SEEK_SET);
                         fread(desc, 50, 1,ppffile);
                         printf("\nFilename       : %s\n",argv[2]);
                         printf("Enc. Method    : %d (PPF2.0)\n",method);
                         printf("Description    : %s\n",desc);

                         fseek(ppffile, -8,SEEK_END);
                         fread(buffer, 4, 1,ppffile);

			 /* Is there a File id ?! */
                         if(strcmp(".DIZ", buffer)){
                         printf("File_id.diz    : no\n\n");
                         dizyn=0;
                         }
                         else{
                         printf("File_id.diz    : yes, showing...\n");
                         fread(&dizlen, 4, 1, ppffile);
                         fseek(ppffile, -dizlen-20, SEEK_END);
                         fread(diz, dizlen, 1, ppffile);
                         diz[dizlen-7]='\0';
                         printf("%s\n",diz);
                         dizyn=1;
                         dizlensave=dizlen;
                         }
                        
                         /* Do the BINfile size check! */
                         fseek(ppffile, 56, SEEK_SET);
                         fread(&dizlen, 4, 1,ppffile);
                         fseek(binfile, 0, SEEK_END);
                         binlen=ftell(binfile);
                         if(dizlen!=binlen){
                         printf("The size of the BIN file isnt correct\nCONTINUE though? (y/n): ");
                         in=getc(stdin);
                         if(in!='y'&&in!='Y'){
                         fclose(ppffile);
                         fclose(binfile);
                         printf("\nAborted...\n");
                         exit(0);
                         }}

			 /* do the Binaryblock check! this check is 100% secure! */
                         fseek(ppffile, 60, SEEK_SET);
                         fread(ppfblock, 1024, 1, ppffile);
                         fseek(binfile, 0x9320, SEEK_SET);
                         fread(binblock, 1024, 1, binfile);
                         in=memcmp(ppfblock, binblock, 1024);
                         if(in!=0){
                         printf("The BINfile does not seem to be the right one\nCONTINUE though? (Suggestion: NO) (y/n): ");
                         in=getc(stdin);
                         if(in!='y'&&in!='Y'){
                         fclose(ppffile);
                         fclose(binfile);
                         printf("\nAborted...\n");
                         exit(0);
                         }}

			 /* Calculate the count for patching the image later */
                         fseek(ppffile, 0, SEEK_END);
                         count=ftell(ppffile);
                         if(dizyn==0){
                         count-=1084;
                         seekpos=1084;
                         }else{
                         count-=1084;
                         count-=38;
                         count-=dizlensave;
                         seekpos=1084;
                         }
                         printf("Patching ... ");
                         break;
                default:
                
                	 /* Enc. Method wasnt 0 or 1 i bet you wont see this */
                         printf("Unknown Encodingmethod! - check for updates.\n");
                         fclose(ppffile);
                         fclose(binfile);
                         exit(0);

        }

        /* Patch the Image */
        
        do{
        fseek(ppffile, seekpos, SEEK_SET);  /* seek to patchdataentry */
        fread(&pos, 4, 1, ppffile);	    /* Get POS for binfile */
        fread(&anz, 1, 1, ppffile);         /* How many byte do we have to write? */
        fread(ppfmem, anz, 1, ppffile);     /* And this is WHAT we have to write */
        fseek(binfile, pos, SEEK_SET);      /* Go to the right position in the BINfile */
        fwrite(ppfmem, anz, 1, binfile);    /* write 'anz' bytes to that pos from our ppfmem */
        seekpos=seekpos+5+anz;		    /* calculate next patchentry! */
        count=count-5-anz;                  /* have we reached the end of the PPFfile?? */
        }
        while(count!=0);		    /* if not -> LOOOOOP! */

        printf("DONE..\n");		    /* byebye :) */
        fclose(ppffile);
        fclose(binfile);
        return 0;
}

#include "../ucon64.h"
int addppfid(int argc, char *argv[])
{
	long fsize;
	char fileidbuf[3072];
	char buf[4095];

        printf("Adding file_id.diz .. ");
	fsize=quickftell(getarg(argc,argv,ucon64_ROM));
        if(fsize>3072) fsize=3072;	/* File id only up to 3072 bytes! */
        quickfread(fileidbuf, 0, fsize ,getarg(argc,argv,ucon64_FILE));

	sprintf(buf,"@BEGIN_FILE_ID.DIZ%s@END_FILE_ID.DIZ",fileidbuf);

        quickfwrite(buf,quickftell(getarg(argc,argv,ucon64_ROM)), strlen(buf), getarg(argc,argv,ucon64_ROM),"r+b");


        quickfwrite(&fsize, quickftell(getarg(argc,argv,ucon64_ROM)), 4, getarg(argc,argv,ucon64_ROM),"r+b");
        printf("done!\n");

	return(0);

}


int ppf_usage(int argc,char *argv[])
{

printf("  -ppf		apply PPF patch (<=2.0); $ROM=ISO_IMAGE $FILE=PATCHFILE\n\
  -mkppf	create PPF patch; $ROM=ISO_IMAGE $FILE=CHANGED_IMAGE\n\
  -nppf		change PPF description; $ROM=PATCHFILE $FILE=DESCRIPTION\n\
  -idppf	change PPF FILE_ID.DIZ (2.0); $ROM=PATCHFILE $FILE=FILE_ID.DIZ\n\
");

	return(0);
}
