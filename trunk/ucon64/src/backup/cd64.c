/*
cd64.c - CD64 support for uCON64

written by 2001 NoisyB (noisyb@gmx.net)


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
#include "cd64.h"
#include "../misc.h"                            // kbhit(), getch()
/*


Using CD64COMM.EXE and PRO. COMMS LINK CARD to link up the CD64 with a PC:

Install the PRO. COMMS LINK CARD into a PC, and connect it to the CD64 with
the special comms link cable. Copy the CD64COMM.EXE and DOS4GW.EXE into the
C:\N64 directory. Type CD64COMM.EXE at the DOS prompt and follow the on screen prompts. If the communication fails, have a look at the Trouble Shooting section
of this manual.



Using CD64COMM.EXE and Parallel Port Adaptor at PC ECP PRINTER PORT 1 (LPT1)
to link up with the CD64:

Install the Parallel Port Adaptor at PC ECP PRINTER PORT 1 (LPT1), and connect
it to the CD64 with a parallel cable. Copy the CD64COMM.EXE and DOS4GE.EXE into
the C:\N64 directory. Type CD64COMM.EXE at the DOS prompt and follow the on screen prompts. If the communication fails, have a look at the Trouble Shooting section
of this manual.



Optimizing your PC:

To get the best performance with the Comms software you may wish to try the
following.

1. The comms software uses disk access a lot, a disk cache will speed this
   process up considerably. Ensure you have some form of disk cache running.
   If in DOS mode (i.e. Not in a Windows DOS shell) then install SMARTDRIVE.
   If you are in Windows 95 then you will find a disk cache is already on.

2. If in Windows, run the CD64COMM.EXE program at full screen and not in a
   window. Although the program does work perfectly OK in a window it is
   considerably faster at full screen.

3. Ensure your ISA bus speed is set to its fastest in your machines BIOS.
   (Some machines do not have this setting).



Type the following command line at dos prompts at C:\N64 directory:

1. Download file GAME.BIN to CD64 DRAM ADDRESS and IO port 310.
   C:\N64>CD64COMM -p310 -t -fgame.bin -sb4000000

2. Download file GAME.BIN to CD64 DRAM ADDRESS and IO port 310 and execute it.
   C:\N64>CD64COMM -p310 -x -fgame.bin -sb4000000

3. Grab file GAME.BIN from CD64 CARD ADDRESS length 0x800000 and IO port 310.
   C:\N64>CD64COMM -p310 -g -fgame.bin -sb2000000 -l800000

4. Download file GAME.BIN to CD64 DRAM ADDRESS and using Parallel Port Adaptor
   at ECP PRINTER PORT 1 (LPT1). 
   C:\N64>CD64COMM -p378 -t -fgame.bin -sb4000000

5. Download file GAME.BIN to CD64 DRAM ADDRESS and using Parallel Port Adaptor
   at ECP PRINTER PORT 1 (LPT1).
   C:\N64>CD64COMM -p378 -x -fgame.bin -sb4000000

6. Grab file GAME.BIN from CD64 CARD ADDRESS length 0x800000 and using Parallel
   Port Adaptor at ECP PRINTER PORT 1 (LPT1).
   C:\N64>CD64COMM -p378 -g -fgame.bin -sb2000000 -l800000

7. Dump system memory from N64 to file.
   C:\N64>CD64COMM -p378 -d -fTEST.BIN -s80000400 -l100000

8. download BOOT EMULATOR to N64 and play uncracked game in CD64.
   C:\N64>CD64COMM -p378 -b -fTEST.BIN -s80300000


******************************************************************************
*
* C:\n64>cd64comm
*
* CD64 Up/Download utility Ver2.10
*
*
*Usage:   CD64COMM -P<port> -T -X -G -D -B -F<fname> -S<start> -L<length>
*
*Example: CD64COMM -x -fTEST.BIN -sb4000000
*         CD64COMM -p300 -t -fTEST.BIN -sb4000000
*         CD64COMM -g -fTEST.BIN -sb2000000 -l800000
*         CD64COMM -p378 -x -fTEST.BIN -sb4000000    
*         CD64COMM -d -fTEST.BIN -s80000400 -l100000
*         CD64COMM -b -fTEST.BIN -s80300000
*
*Options:
*  -P     pc Port address 300, 310-default, 320, 330, 378-ECP PRINTER PORT
*  -T     Transfer a block of data to CD64 any dram address, but don't do
*         anything with it
*  -X     eXecute code after download to b4000000-default by CD64
*  -G     Grab game from CD64 to file
*  -D     Dump system memory from N64 to file
*  -B     download BOOT EMULATOR to N64 and play uncracked game in CD64
*  -F     Filename of binary image to download/upload
*  -S     Start address b2000000=card start, b4000000=dram start,
*         80000000=N64 system memory start (total 32Mb)
*         80300000=boot emulator start (max 4Mb)
*  -L     Length of grab or dump 400000=32Mb, 800000=64Mb, 1000000=128Mb,
*         1800000=192Mb, 2000000=256Mb
*
*All value are hex
******************************************************************************


Trouble Shooting:

a. Failed to communicate. 

1. Check the comms board's IO port setting is the same as set in the software.
   By default the IO port is set to 310 on the board and in the software.
   Check the boards setting by examining its jumper settings (see the IO port
   table in the Trouble Shooting section). Check the software setting as you
   typed at the CD64COMM.EXE program command line.

2. The IO port used by the Comms board may be conflicting with another device
   or card in you system. By default the setting is IO port 310. You can check
   this simply by changing the IO port setting on the card and software.

   - NOTE: You must change both the cards and the software's settings and they
           must be the same.  

3. If using Parallel Port Adaptor, the PC PRINTER PORT should be ECP MODE PRINTER
   PORT and PORT 1 (LPT1). And the port address setting should be -p378.


b. Check the IO port setting as following.

   The plastic jumpers can be found in the middle of the comms board. Jumper
   1 is the jumper closest to the parallel cable connector (the left jumper
   if you hold the board so the edge connector strip is at the top, parallel
   connector to the left). 


   Jumper 1                 Jumper 2                 IO Port

   Closed (Pins Covered)    Closed (Pins Covered)    300

   Closed (Pins Covered)    Open (Pins Unconnected)  310 (default)

   Open (Pins Unconnected)  Closed (Pins Covered)    320

   Open (Pins Unconnected)  Open (Pins Unconnected)  330

*/
/*
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

/*
#include <iostream.h>
#include <stdio.h>
#include <malloc.h>
#include <dos.h>
#include <process.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <graph.h>

int data_reg=0x310;
int status_reg=0x312;
int control_reg;
int ecp_reg;
int ecp_mode;
int ecp=0;
char buf[32768];

void c_break (int signum)
{
        int temp;

//        kbhit ();
//        getch ();

        if (ecp==0)
        {
                inp (data_reg);
                outp (data_reg, 0xff);
                inp (data_reg);
        }
                else
                {
// set ecp port to ps/2 mode
                        temp=ecp_mode;
                        temp&=0x1f;
                        temp|=0x20;
                        outp (ecp_reg, temp);

                        outp (control_reg, 0x04);
                        delay (1);

//           _   _
// set /PCRD  |_| to reset PCX
//                      outp (control_reg, 0x24);
                        outp (control_reg, 0x26);
//                      outp (control_reg, 0x24);

//           _   _
// set /PCWR  |_| to set PCX
// set all output hi
                        outp (control_reg, 0x04);
                        outp (data_reg, 0xff);
                        outp (control_reg, 0x05);
                        outp (control_reg, 0x04);

// reset ecp mode
                        outp (ecp_reg, ecp_mode);
                }
                                                                        
                printf ("\nControl-Break pressed.  Program aborting ...\n");
                exit (13);
}
                                                                                                  

void test_key (void)
{
        int temp;

        if (kbhit ())
        {
                temp=getch ();
                if (temp==27||temp==3)
                {
                        if (ecp==0)
                        {
                                inp (data_reg);
                                outp (data_reg, 0xff);
                                inp (data_reg);
                        }
                                else
                                {
// set ecp port to ps/2 mode
                                        temp=ecp_mode;
                                        temp&=0x1f;
                                        temp|=0x20;
                                        outp (ecp_reg, temp);

                                        outp (control_reg, 0x04);
                                        delay (1);

//           _   _
// set /PCRD  |_| to reset PCX
//                                      outp (control_reg, 0x24);
                                        outp (control_reg, 0x26);
//                                      outp (control_reg, 0x24);

//           _   _
// set /PCWR  |_| to set PCX
// set all output hi
                                        outp (control_reg, 0x04);
                                        outp (data_reg, 0xff);
                                        outp (control_reg, 0x05);
                                        outp (control_reg, 0x04);

// reset ecp mode
                                        outp (ecp_reg, ecp_mode);
                                }
                                                                        
                        printf ("\nQuitting program!!\n");
                        exit (13);
                }
        }
}


enum {X,P,F,S,G,L,T,D,B};
enum {x,p,f,s,g,l,t,d,b};
char _switches[] = {'X', 'P', 'F', 'S', 'G', 'L', 'T', 'D', 'B'};
char switches[sizeof (_switches)];
char *ptr_switches[sizeof (_switches)];


void parse_switches (int argc, char *argv[])
{
        char *cptr;
        char c;
        long i, j, k, l;

        for (i=1; argc>1; argc--)
        {
                cptr=argv[i++];                         // string

                j=0;                                            // first char
                c=cptr[j++];

                if (c=='-')                                     // switch following
                {
                        c=cptr[j];                              // switch
                        c=toupper (c);

                        for (k=0; k<sizeof (switches); k++)
                        {
                                if (c==_switches[k])
                                {
                                        switches[k]=1;          // switch on
                                        j++;                    // identified switch next char

                                        if (isalnum (cptr[j])) ptr_switches[k]=&cptr[j];

                                                else
                                                {
                                                        cptr=argv[i];                   
                                                        if (isalnum (cptr[0])) ptr_switches[k]=cptr;
                                                }

                                        break;
                                }
                        }
                }
        }
}


// export to CD64
// import same data from CD64
void send_byte (unsigned char c)
{
        unsigned char status;
        unsigned long i;

        outp (data_reg, c);
        i=0;
        while ((inp (status_reg)&1)&&(i<800000)) i++;
        if (i<800000)
        {
                if (c!=inp (data_reg))
                {
                        status=inp (data_reg);
                        printf ("\nBad!!");
                        printf ("\nvalue out was %2.2X\nvalue back was %2.2X\n", c, status);
                        exit (11);
                }
        }
                else
                {
                        printf ("\nBad!!");
                        printf ("\ncommunication time out error!!\n");
                        exit (11);
                }
}


// export to CD64  by ECP port
void send_pbyte (unsigned char c)
{
        unsigned char status;
        unsigned long i;

        i=0;
        while ((inp (status_reg)&0x80)&&(i<800000)) i++;
        if (i<800000)
        {
//           _   _
// set /PCRD  |_| to reset PCX
//              outp (control_reg, 0x24);
                outp (control_reg, 0x26);
//              outp (control_reg, 0x24);

//           _   _
// set /PCWR  |_| to set PCX
// export to cd64
                outp (control_reg, 0x04);
                outp (data_reg, c);
                outp (control_reg, 0x05);
                outp (control_reg, 0x04);
        }

                else
                {
                        printf ("\nBad!!");
                        printf ("\ncommunication time out error!!\n");
                        exit (11);
                }
}


// export to CD64
// import new data from CD64
unsigned char exchange_byte (char c)
{
        char status;
        unsigned long i;

        outp (data_reg, c);
        i=0;
        while ((inp (status_reg)&1)&&(i<800000)) i++;
        if (i<800000)
        {
                status=inp (data_reg);
        }
                else
                {
                        printf ("\nBad!!");
                        printf ("\ncommunication time out error!!\n");
                        exit (11);
                }

        return status;
}


// export to CD64
// import new data from CD64
unsigned char exchange_pbyte (char c)
{
        char status;
        unsigned long i;

        i=0;
        while ((inp (status_reg)&0x80)&&(i<800000)) i++;
        if (i<800000)
        {
//                       _   _
// set /PCRD  |_| to reset PCX
// import from cd64
//              outp (control_reg, 0x24);
                outp (control_reg, 0x26);
                status=inp (data_reg);
//              outp (control_reg, 0x24);

//                       _   _
// set /PCWR  |_| to set PCX
// export to cd64
                outp (control_reg, 0x04);
                outp (data_reg, c);
                outp (control_reg, 0x05);
                outp (control_reg, 0x04);

        }
                else
                {
                        printf ("\nBad!!");
                        printf ("\ncommunication time out error!!\n");
                        exit (11);
                }

        return status;
}


// export to CD64
// import new data from CD64
unsigned char read_pbyte (void)
{
        char status;
        unsigned long i;

        i=0;
        while ((inp (status_reg)&0x80)&&(i<800000)) i++;
        if (i<800000)
        {
//                       _   _
// set /PCRD  |_| to reset PCX
// import from cd64
//              outp (control_reg, 0x24);
                outp (control_reg, 0x26);
                status=inp (data_reg);
//              outp (control_reg, 0x24);

//                       _   _
// set /PCWR  |_| to set PCX
// export to cd64
                outp (control_reg, 0x04);
                outp (control_reg, 0x05);
                outp (control_reg, 0x04);

        }
                else
                {
                        printf ("\nBad!!");
                        printf ("\ncommunication time out error!!\n");
                        exit (11);
                }

        return status;
}


// export header to CD64
// import new data from CD64
unsigned char header_byte (char c)
{
        char status, spinner;

        spinner ='|';
        outp (data_reg, c);
        do
        {
                status=inp (status_reg);

                switch (spinner)
                {
                        case '|':
                                spinner='/';
                                break;

                        case '/':
                                spinner='-';
                                break;

                        case '-':
                                spinner='\\';
                                break;

                        case '\\':
                                spinner='|';
                                break;
                }

                delay (1);
                test_key();
                printf ("%c\b", spinner);
        }
        while (status&1);

        status=inp (data_reg);

        return status;
}


// export header to CD64
// import new data from CD64
unsigned char header_pbyte (char c)
{
        char status, spinner;

        spinner ='|';
        do
        {
                status=inp (status_reg);

                switch (spinner)
                {
                        case '|':
                                spinner='/';
                                break;

                        case '/':
                                spinner='-';
                                break;

                        case '-':
                                spinner='\\';
                                break;

                        case '\\':
                                spinner='|';
                                break;
                }

                delay (1);
                test_key();
                printf ("%c\b", spinner);
        }
        while (status&0x80);

//           _   _
// set /PCRD  |_| to reset PCX
// import from cd64
//      outp (control_reg, 0x24);
        outp (control_reg, 0x26);
        status=inp (data_reg);
//      outp (control_reg, 0x24);

//           _   _
// set /PCWR  |_| to set PCX
// export to cd64
        outp (control_reg, 0x04);
        outp (data_reg, c);
        outp (control_reg, 0x05);
        outp (control_reg, 0x04);

        return status;
}


//----------------------------------------------------------------------------
void send_long (unsigned long value)
{
        send_byte (value>>24);
        send_byte (value>>16);
        send_byte (value>>8);
        send_byte (value);
}


//----------------------------------------------------------------------------
void send_plong (unsigned long value)
{
        send_pbyte (value>>24);
        send_pbyte (value>>16);
        send_pbyte (value>>8);
        send_pbyte (value);
}


//----------------------------------------------------------------------------
unsigned int verify_checksum (unsigned int checksum_out)
{
        unsigned int checksum_in;

        checksum_in=exchange_byte (checksum_out>>8);
        checksum_in=(checksum_in<<8)&0xff00;
        checksum_in=checksum_in+exchange_byte (checksum_out);
        checksum_in=checksum_in&0xfff;

        return checksum_in;
}


void usage_message (void)
{
        cout
        << "\n"
        << "Usage:   CD64COMM -P<port> -T -X -G -D -B -F<fname> -S<start> -L<length>\n"
        << "\n"
        << "Example: CD64COMM -x -fTEST.BIN -sb4000000\n"
        << "         CD64COMM -p300 -t -fTEST.BIN -sb4000000\n"
        << "         CD64COMM -g -fTEST.BIN -sb2000000 -l800000\n"
        << "         CD64COMM -p378 -x -fTEST.BIN -sb4000000\n"    
        << "         CD64COMM -d -fTEST.BIN -s80000400 -l100000\n"
        << "         CD64COMM -b -fTEST.BIN -s80300000\n"
        << "\n"
        << "Options:\n"
        << "  -P     pc Port address 300, 310-default, 320, 330, 378-ECP PRINTER PORT\n"
        << "  -T     Transfer a block of data to CD64 any dram address, but don't do\n"
        << "         anything with it\n"
        << "  -X     eXecute code after download to b4000000-default by CD64\n"
        << "  -G     Grab game from CD64 to file\n"
        << "  -D     Dump system memory from N64 to file\n"
        << "  -B     download BOOT EMULATOR to N64 and play uncracked game in CD64\n"
        << "  -F     Filename of binary image to download\\upload\n"
        << "  -S     Start address b2000000=card start, b4000000=dram start,\n"
        << "         80000000=N64 system memory start (total 32Mb)\n"
        << "         80300000=boot emulator start (max 4Mb)\n"
        << "  -L     Length of grab or dump 400000=32Mb, 800000=64Mb, 1000000=128Mb,\n"
        << "         1800000=192Mb, 2000000=256Mb\n"
        << "\n"
        << "All value are hex\n"
        << "\n";
}


unsigned long long_hex_atoi (char *str)
{
        unsigned long i=0, i2=0;
        int pos=0;
        char c;
        char *astr;

        while (str[pos]=='0') pos++;

        astr=(char *)str+pos;
        pos=0;

        while ((astr[pos]>='0'&&astr[pos]<='9')||(astr[pos]>='A'&&astr[pos]<='F')||(astr[pos]>='a'&&astr[pos]<='f'))
        {
                if (astr[pos]>='0'&&astr[pos]<='9')
                        i=astr[pos]-'0';

                        else if (astr[pos]>='A'&&astr[pos]<='F')
                                i=astr[pos]-'A'+10;

                        else if (astr[pos]>='a'&&astr[pos]<='f')
                                i=astr[pos]-'a'+10;

                pos++;
                i2<<=4;
                i2+=i;
        }

        return i2;
}


int send (char comm, unsigned long saddr, char *str)
{
        unsigned char c;
        unsigned int checksum, checksum_in, temp, temp2;
        unsigned long flength, count;
        FILE *in;

        in=fopen (str, "rb");
        if (in==NULL)
        {
                printf ("\nCan't open file %s\n", str);
                exit (2);
        }

        if (setvbuf (in, buf, _IOFBF, 32767)!=0)
        {
                printf ("\nfailed to set up buffer for %s\n", str);
                exit (2);
        }
        
        fseek (in, 0, SEEK_END);
        flength=ftell (in);
        fseek (in, 0, SEEK_SET);

        if ((flength&0x00000003)!=0)
        {
                printf ("\nFile length not in longword! (ending with 0,4,8,c)\n");
                exit (20);
        }

        if (flength==0)
        {
                printf ("\nFile length equal zero!\n");
                exit (20);
        }

// Initiate the download
        printf ("\nWaiting for target to respond...");

        if (ecp==0)
        {
                inp (data_reg);
                do
                {
                        while (header_byte ('W')!='R');
                }
                while (header_byte ('B')!='W');

                printf ("\nSending file %s to CD64...\n", str);

                send_byte (comm);
                send_long (saddr);
                send_long (flength);

                checksum = 0;
                for (count=0; count<flength; count++)
                {
                        c=fgetc (in);
                        checksum+=c;
                        send_byte (c);
                }

                checksum&=0xfff;
                printf ("\nmy checksum %X", checksum);
                checksum_in=verify_checksum (checksum);
                printf ("\nconsoles %X", checksum_in);

                temp=exchange_byte (0);
                temp2=exchange_byte (0);

                if ((checksum_in==checksum)&&(temp=='O')&&(temp2=='K')) return 0;

                        else return 11;
        }

                        else
                        {

                                do
                                {
//           _   _
// set /PCRD  |_| to reset PCX
//                                      outp (control_reg, 0x24);
                                        outp (control_reg, 0x26);
//                                      outp (control_reg, 0x24);

//           _   _
// set /PCWR  |_| to set PCX
// export to cd64
                                        outp (control_reg, 0x04);
                                        outp (data_reg, 'W');
                                        outp (control_reg, 0x05);
                                        outp (control_reg, 0x04);
                        
                                        header_pbyte ('B');
                                        temp=header_pbyte ('B');
                                        temp2=header_pbyte ('B');
                                }                               
                                while ((temp!='R')||(temp2!='W'));

                                printf ("\nSending file %s to CD64...\n", str);

                                send_pbyte (comm);
                                send_plong (saddr);
                                send_plong (flength);


                                checksum = 0;
                                for (count=0; count<flength; count++)
                                {
                                        c=fgetc (in);
                                        checksum+=c;
                                        send_pbyte (c);
                                }

                                checksum&=0xfff;
                                send_pbyte (checksum>>8);
                                send_pbyte (checksum);

// send dummy for import handshake
                                read_pbyte ();

                                temp=exchange_pbyte (0);
                                temp2=exchange_pbyte (0);

                                if ((temp=='O')&&(temp2=='K')) return 0;

                                        else return 11;

                        }

        return 0;
}


int grab (char comm, unsigned long addr, unsigned long length, char *str)
{
        unsigned char c;
        unsigned int pc_checksum, n64_checksum, temp, temp2;
        unsigned long count, scount, chunksize, chunkcount, delay;
        FILE *out;

        out=fopen (str, "wb");
        if (out==NULL)
        {
                printf ("Can't open file %s\n", str);
                exit (21);
        }

        if (setvbuf (out, buf, _IOFBF, 32767)!=0)
        {
                printf ("failed to set up buffer for %s\n", str);
                exit (2);
        }
        
        if (length==0)
        {
                printf ("\nLength of zero!\n");
                exit (20);
        }

        if ((length&0x00000003)!=0)
        {
                printf ("\nLength not in longword! (last digit not 0,4,8,c)\n");
                exit (20);
        }

        if ((addr&0x00000003)!=0)
        {
                printf ("\naddress not in longword! (last digit not 0,4,8,c)\n");
                exit (20);
        }

// Initiate the grab 
        printf ("\nWaiting for target to respond...");

        if (ecp==0)
        {
                inp (data_reg);
                do
                {
                        while (header_byte ('W')!='R');
                }
                while (header_byte ('B')!='W');

                printf ("\nGrabing file %s from CD64...\n", str);

                send_byte (comm);
                send_long (addr);
                send_long (length);

                pc_checksum=0;
                for (count=0; count<length; count++)
                {
                        c=exchange_byte (0);
                        fputc (c, out);
                        pc_checksum+=c;
                }

                pc_checksum&=0xfff;
                n64_checksum=exchange_byte (0)<<8;
                n64_checksum+=exchange_byte (0);
                n64_checksum&=0xfff;

                if (n64_checksum!=pc_checksum)
                {
                        cout<<"\nERROR : Checksum failed - Upload corrupt!\n";

                        return 1;
                }

                return 0;
        }
                        else
                        {

                                do
                                {
//           _   _
// set /PCRD  |_| to reset PCX
//                      outp (control_reg, 0x24);
                        outp (control_reg, 0x26);
//                      outp (control_reg, 0x24);

//           _   _
// set /PCWR  |_| to set PCX
// export to cd64
                                        outp (control_reg, 0x04);
                                        outp (data_reg, 'W');
                                        outp (control_reg, 0x05);
                                        outp (control_reg, 0x04);
                        
                                        header_pbyte ('B');
                                        temp=header_pbyte ('B');
                                        temp2=header_pbyte ('B');
                                }                               
                                while ((temp!='R')||(temp2!='W'));

                printf ("\nGrabing file %s from CD64...\n", str);

                send_pbyte (comm);
                send_plong (addr);
                send_plong (length);

// send dummy for import handshake
                read_pbyte ();

                pc_checksum=0;
                for (count=0; count<length; count++)
                {
                        c=read_pbyte ();
                        fputc (c, out);
                        pc_checksum+=c;
                }

                pc_checksum&=0xfff;
                n64_checksum=exchange_pbyte (0)<<8;
                n64_checksum+=exchange_pbyte (0);
                n64_checksum&=0xfff;

                if (n64_checksum!=pc_checksum)
                {
                        cout<<"\nERROR : Checksum failed - Upload corrupt!\n";

                        return 1;
                }

                return 0;
                        }

        return 0;
}


main (int argc, char *argv[])
{
        FILE *in, *up;
        unsigned char c, *str;
        unsigned int checksum, checksum_in, temp, temp2;
        unsigned long flength, count, saddr;

        signal (SIGBREAK, c_break);
        parse_switches (argc, argv);

        cout<<"\nCD64 Up/Download utility Ver2.10\n";

        if (switches[f]==0)
        {
                usage_message();
                exit (18);
        }

        if (switches[p])
        {
                data_reg=atoi (ptr_switches[p]);

                switch (data_reg)
                {
                        case 378:
                                data_reg=0x378;
                                break;

                        case 300:
                                data_reg=0x300;
                                break;

                        case 310:
                                data_reg=0x310;
                                break;

                        case 320:
                                data_reg=0x320;
                                break;

                        case 330:
                                data_reg=0x330;
                                break;

                        default:
                                data_reg=0x310;
                }

                if (data_reg!=0x378)
                {
                        status_reg=data_reg+2;
                        ecp=0;
                }
                        else
                        {

// set ecp port to ps/2 mode
                                ecp=1;
                                ecp_reg=0x77a;
                                status_reg=0x379;
                                control_reg=0x37a;
                                ecp_mode=inp (ecp_reg);
                                temp=ecp_mode;
                                temp&=0x1f;
                                temp|=0x20;
                                outp (ecp_reg, temp);

// set power supply
                                outp (control_reg, 0x04);
                                                                delay (1);

//                      _   _
// set /PCRD |_| to reset PCX
//                              outp (control_reg, 0x24);
                                outp (control_reg, 0x26);
//                              outp (control_reg, 0x24);

//                       _       _
// set /PCWR  |_| to set PCX
// set all output hi
                                outp (control_reg, 0x04);
                                outp (data_reg, 0xff);
                                outp (control_reg, 0x05);
                                outp (control_reg, 0x04);
                        }
        }

        if (!(switches[x]||switches[g]||switches[t]||switches[d]||switches[b]))
        {
                printf ("\nRequires either -X or -G or -T or -D or -B\n");
                exit (14);
        }

        if (!switches[s])
        {
                printf ("\nRequires -S\n");
                exit (14);
        }

        if (strlen (ptr_switches[S])!=8)
        {
                printf ("\n-S requires 8 digits\n");
                exit (14);
        }

        saddr=long_hex_atoi (ptr_switches[S]);

        if (switches[x])
        {
                in=fopen (ptr_switches[f], "rb");
                if (in==NULL )
                {
                        printf ("\nCan't open file %s\n", ptr_switches[f]);
                        exit (2);
                }

                if (send ('X', saddr, ptr_switches[f])==0)
                        printf ("\n%s file sent and executed OK!!!\n", ptr_switches[f]);

                        else
                        {
                                printf ("\nWARNING!!!!! errors transferring file\n");
                                exit (17);
                        }

                exit (0);
        }

        if (switches[t])
        {
                in=fopen (ptr_switches[f], "rb");
                if (in==NULL )
                {
                        printf ("\nCan't open file %s\n", ptr_switches[f]);
                        exit (2);
                }

                if (send ('T', saddr, ptr_switches[f])==0)
                        printf ("\n%s file sent and OK!!!\n", ptr_switches[f]);

                        else
                        {
                                printf ("\nWARNING!!!!! errors transferring file\n");
                                exit (17);
                        }

                exit (0);                

        }
    
        if (switches[b])
        {
                in=fopen (ptr_switches[f], "rb");
                if (in==NULL )
                {
                        printf ("\nCan't open file %s\n", ptr_switches[f]);
                        exit (2);
                }

                if (send ('B', saddr, ptr_switches[f])==0)
                        printf ("\n%s boot emulator file sent and executed OK!!!\n", ptr_switches[f]);

                        else
                        {
                                printf ("\nWARNING!!!!! errors transferring file\n");
                                exit (17);
                        }

                exit (0);
        }

        if (switches[g]&&switches[l])
        {
                if (grab ('G', saddr,long_hex_atoi (ptr_switches[l]), ptr_switches[F])==0)
                        printf ("\n%s file grabed and OK!!!\n", ptr_switches[f]);

                        else
                        {
                                printf ("\nWARNING!!!!! errors grabing file\n");
                                exit (17);
                        }

                exit (0);
        }

        if (switches[d]&&switches[l])
        {
                if (grab ('D', saddr,long_hex_atoi (ptr_switches[l]), ptr_switches[F])==0)
                        printf ("\n%s system memory dumped and OK!!!\n", ptr_switches[f]);

                        else
                        {
                                printf ("\nWARNING!!!!! errors dumping system memory\n");
                                exit (17);
                        }

                exit (0);
        }

                else usage_message();
}
*/

int cd64_usage(int argc, char *argv[])
{
  if (argcmp(argc, argv, "-help"))
    printf("\n%s\n", cd64_TITLE);

  printf("TODO:  -xcd64		send/receive ROM to/from CD64; $FILE=PORT\n"
         "		receives automatically when $ROM does not exist\n"
        );
  //TODO more info like technical info about cabeling and stuff for the copier

  return 0;
}
