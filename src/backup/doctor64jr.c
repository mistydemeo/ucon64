/*
doctor64jr.c - Bung Doctor 64jr support for uCON64

written by 1999 - 2002 NoisyB (noisyb@gmx.net)


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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "misc.h"
#include "ucon64.h"
#include "ucon64_db.h"
#include "ucon64_misc.h"
#include "doctor64jr.h"

const char *doctor64jr_usage[] =
  {
    "Doctor64 Jr",
    "19XX Bung Enterprises Ltd http://www.bung.com.hk",
#ifdef BACKUP
    "  " OPTION_LONG_S "xdjr        send/receive ROM to/from Doctor64 Jr; " OPTION_LONG_S "file=PORT\n"
//          "                receives automatically when ROM does not exist\n"
    "                  currently only sending is supported\n",
#ifdef TODO
#warning TODO --xdjrs send/receive SRAM to/from Doctor64 Jr
#endif
#if 0
    "  " OPTION_LONG_S "xdjrs       send/receive SRAM to/from Doctor64 Jr; " OPTION_LONG_S "file=PORT\n"
    "                  receives automatically when SRAM does not exist\n"
#endif
#endif // BACKUP
    NULL
  };


/*
drjr transfer protocol


DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX  ai  XXXXX  data  XXXX
p1    nwrite   ~~~~~~~~~|_____________________|~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~~~~~~~~|_|~~~~~~
p17   nastb    ~~~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~


ai[]=0	r/w a[7..0]
ai[]=1	r/w a[15..8]
ai[]=2	r/w a[23..16]
ai[]=3	w a[28..24]
ai[]=3  r (rst,wdf,wcf,a[28..24])
ai[]=4	r/w data
ai[]=5	w mode
ai[]=6	w en_1
ai[]=7	w en_0
*remark
        a[8..1] support page count up

        ai[3]d7:0=N64 power off, 1=N64 power on
             d6:0=no dram data written, 1=dram data written
             d5:0=no data write in b4000000~b7ffffff, 1=some data written in b4000000~b7ffffff

        mode d0:0=dram read only and clear wdf, 1=dram write enable
             d1:0=disable cartridge read and clear wcf flag,
                1=enable cartridge read(write b4000000~b7ffffff will switch off dram and cartridge will present at b0000000~b3ffffff)

        en_0=05 and en_1=0a is enable port control







mode:q0              0                   1                  0                    1
mode:q1              0                   0                  1                    1
b7ff ffff
b400 0000      dram read only         dram r/w        cartridge read     cartridge read(* write this area will switch off dram)

b3ff ffff
b000 0000      dram read only         dram r/w        dram read only        dram r/w



eg:enable port control

DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX 07 XX 05 XXXX 06 XX 0a XXXXXXXXXXXX
p1    nwrite   ~~~~~~~~~|_____________________________|~~~~~~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~
p17   nastb    ~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~
                            en_0=05       en_1=0a



eg:write adr $b0123456, data $a55a,$1234..

DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX 00 XX 56 XXXX 01 XX 34 XXXX 02 XX 12 XXXX 03 XX b0 XXXXXX 04 XX 5a XX a5 XX 34 XX 12 XXXXXXXXXXX
p1    nwrite   ~~~~~~~~~|_______________________________________________________________________________________|~~~~~~~~~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~|_|~~~|_|~~~|_|~~~|_|~~~~~~~~~~~
p17   nastb    ~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                              set adr word low            set adr word high                wdata a55a  wdata 1234 (after write adr=b012345a)



eg:read adr $b0123400~$b01235ff, 512 data

DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX 00 XX 00 XXXX 01 XX 34 XXXX 02 XX 12 XXXX 03 XX b0 XXXXXX 04 XX data0 XX data1 X ... X data510 XX data511 XXXXX
p1    nwrite   ~~~~~~~~~|________________________________________________________________~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~~|_|~~~~~~|_|~~~ ~~~ ~~~~|_|~~~~~~~~|_|~~~~~~~~
p17   nastb    ~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                              set adr word low            set adr word high                (after 512 read adr=b0123400)


eg:dram write protect, disable N64 access to cartridge and disable port control

DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX 05 XX 00 XXXX 07 XX 00 XXXX 06 XX 00 XXXXXXXXXXXX
p1    nwrite   ~~~~~~~~~|________________________________________|~~~~~~~~~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~
p17   nastb    ~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~
                            mode=00       en_0=00       en_1=00




simple backup rountine for N64

void writePI(unsigned long addr, unsigned long value)
{
  do {} while (*(volatile unsigned long *) (0xa4600010) & 3);     // check parallel interface not busy
  addr &=0xbffffffc;
  *(unsigned long *)(addr)=value;
}

unsigned long readPI(unsigned long addr)
{
  do {} while (*(volatile unsigned long *) (0xa4600010) & 3);     // check parallel interface not busy
  addr &=0xbffffffc;
  return *(unsigned long *)(addr);
}

// MAIN -- START OF USER CODE
void mainproc(void *arg) {
    u32 base_adr;
    for (base_adr=0;base_adr<0x1000000;base_adr++){                     // backup 128Mbits
       writePI(0xb0000000+base_adr,readPI(0xb4000000 + base_adr));      // write data
    }
}
*/

#ifdef BACKUP

#ifdef TODO
#warning TODO doctor64jr.c sram routines
#endif

#if 0

// The following include-block should be moved to the other includes when the SRAM
// code is finished. misc.h should be included after OS.h to avoid a warning.
#ifdef  __unix__
#include <unistd.h>                             // usleep(), microseconds
#elif   defined __MSDOS__
#include <dos.h>                                // delay(), milliseconds
#elif   defined __BEOS__
#include <OS.h>                                 // snooze(), microseconds
 #endif

typedef unsigned short u16;                     // unsigned 16-bit
//void docmd(jr_command cmd);
int saveram_pc_main (int argc, char *argv[], char *envp[]);

#define CART_BASE       0xB0000000U
#define CART_BASE2      0xB4000000U
#define SRAM_BASE       0xA8000000U
#define SRAM_LEN        0x8000
#define RAMROM_SRAM_ADDY 0x00200000U
#define EPP_PORT 0x378

typedef enum _jr_command
{
  ReadSram      = 0x4001,                       // Copy SRAM from DS1/Cart to designated address
  WriteSram     = 0x4002,                       // Copy SRAM from designated address to DS1/Cart
  ReadEeprom    = 0x4004,                       // Ditto for DX256/Cart EEPROM
  WriteEeprom   = 0x4008,                       //
  ReadPfs       = 0x4010,                       // Ditto for Mempak
  WritePfs      = 0x4020,

  NoCmd         = 0x8037,                       // Do nothing for now
  ColdBoot      = 0xABCD                        // Game is loaded, now cold boot.
} jr_command;

static unsigned char buffer[SRAM_LEN * 8];
static FILE *fp;

int
saveram_pc_main (int argc, char *argv[], char *envp[])
{
  unsigned long i, j, temp;

  i = v64jr_status ();          // get v64jr status (or 0 if no v64jr detected)
  do
    i = v64jr_quickstatus ();
  while (1);

  if (!i)
    {
      fprintf (stderr, "v64jr not found at EPP 0x%x\n", EPP_PORT);
      exit (5);
    }

  printf
    ("v64jr status: N64 power %s, v64jr RAM %saltered, ROM @ 0xB%s000000\n",
     i & V64JR_N64POWER ? "ON" : "OFF", i & V64JR_RAMALTERED ? "was " : "un",
     i & V64JR_ROMSELECT ? "0" : "4");

/*
  if ((i & V64JR_N64POWER))
    {
      printf ("\nTurn off n64...");
      fflush (stdout);
      while (i & V64JR_N64POWER)
        i = v64jr_status ();
    }
  printf ("Turn ON n64...\n");
  fflush (stdout);
  while (!(i & V64JR_N64POWER))
    i = v64jr_status ();

#ifdef  __unix__                                // wait 2000 milliseconds
  usleep (2000);
#elif   defined __MSDOS__
  delay (2);
#elif   defined __BEOS__
  snooze (2000);
 #endif

  printf("\nPower N64 and press any key to continue...");
  fflush(stdout);
  fgetc(stdin);
  printf("ok.\n");
  fflush(stdout);
*/

/*
  if ((fp = fopen (FILENAME, "wb")) == NULL)
    {
      fprintf (stderr, "Can't open %s for writing\n", FILENAME);
      exit (5);
    }
*/
  printf ("Reading SRAM...\n");
  docmd (ReadSram);                     // buffer is is motorola format
  i = fwrite (buffer, SRAM_LEN, 1, fp); // buffer is is motorola format
  fclose (fp);

  if ((fp = fopen (FILENAME2, "rb")) == NULL)
    {
      fprintf (stderr, "Can't open %s for reading\n", FILENAME2);
      exit (6);
    }
  i = fread (buffer, 0x8000, 1, fp);
  printf ("Writing SRAM...\n");
  docmd (WriteSram);                    // buffer is is motorola format

  printf ("Sending ROM...\n");
  docmd (ColdBoot);

  fclose (fp);
  return 0;
}

void
docmd (jr_command cmd)
{
  long fatcmd, i, j;
  fatcmd = cmd | 0x12400000;

  switch (cmd)
    {
    case ReadSram:
      v64jr_open ();            // PC control of v64jr
      v64jr_io (V64JR_WRITE, (unsigned char *) (&fatcmd), 0x0, 4);
      v64jr_close (V64JR_WRITEENABLE | V64JR_ROMENABLE);

      do
        i = v64jr_quickstatus ();
      while (!(i & V64JR_ROMSELECT));

      v64jr_open ();
      v64jr_io (V64JR_READ, buffer, RAMROM_SRAM_ADDY, SRAM_LEN);
      v64jr_close (V64JR_WRITEENABLE | V64JR_ROMENABLE);

      for (j = 0; j < SRAM_LEN >> 1; j++)
        ((unsigned short *) buffer)[j] =
          ((unsigned short *) buffer)[j] >> 8 | ((unsigned short *) buffer)[j] << 8;

      break;

    case WriteSram:
      for (j = 0; j < SRAM_LEN >> 1; j++)
        ((unsigned short *) buffer)[j] =
          ((unsigned short *) buffer)[j] >> 8 | ((unsigned short *) buffer)[j] << 8;

      v64jr_open ();
      v64jr_io (V64JR_WRITE, buffer, RAMROM_SRAM_ADDY, SRAM_LEN);
      v64jr_io (V64JR_WRITE, (unsigned char *) (&fatcmd), 0x0, 4);
      v64jr_close (V64JR_WRITEENABLE | V64JR_ROMENABLE);

      for (j = 0; j < SRAM_LEN >> 1; j++)
        ((unsigned short *) buffer)[j] =
          ((unsigned short *) buffer)[j] >> 8 | ((unsigned short *) buffer)[j] << 8;

      do
        i = v64jr_quickstatus ();
      while (!(i & V64JR_ROMSELECT));

      break;

    case ColdBoot:
      v64jr_open ();
      v64jr_io (V64JR_WRITE, (unsigned char *) (&fatcmd), 0x0, 4);
      v64jr_close (V64JR_WRITEENABLE | V64JR_ROMENABLE);

      do
        i = v64jr_quickstatus ();
      while (!(i & V64JR_ROMSELECT));

      fatcmd = NoCmd | 0x12400000;
      v64jr_open ();
      v64jr_io (V64JR_WRITE, (unsigned char *) (&fatcmd), 0x0, 4);
      v64jr_close (0);

    default:
      ;
    }
}
#endif // #if 0

/**************************************
*        program name: v64jr.c          *
*  N64 cart emulator transfer program *
**************************************/

//#define ai 0x37b
//#define data 0x37c
#define trans_size 32768
#define set_ai_write outportb(port_a,5);	// ninit=1, nwrite=0
#define set_data_write outportb(port_a,1);	// ninit=0, nwrite=0
#define set_data_read outportb(port_a,0);	// ninit=0, nwrite=1
#define set_normal outportb(port_a,4);		// ninit=1, nwrite=1

static char *file_name=NULL;
char write_en=0;
char test_en=0;
char verify_en=0;
char disp_on=1;
FILE *fptr;
union mix_buffer {
  unsigned char buffer[32768];
  unsigned short int bufferx[16384];
} mix;
unsigned short int port[2];
unsigned char port_no;
unsigned short int port_8,port_9,port_a,port_b,port_c;
unsigned char disp_buf[16];
short int i,j,page,sel,err,wv_mode;
char ch=' ';
static unsigned long init_time = 0, size = 0, pos = 0;

void drjr_set_ai (unsigned char _ai);
void drjr_set_ai_data (unsigned char _ai, unsigned char _data);
void drjr_init_port (void);
void drjr_end_port (void);
static void dump_buffer (void);
static char write_32k (unsigned short int hi_word, unsigned short int lo_word);
static char verify_32k (unsigned short int hi_word, unsigned short int lo_word);
static void read_adr (void);
void read_some (void);
unsigned char drjr_check_card (void);
static short int read_file (void);
static short int download_n64 ();
static void gen_pat_32k (unsigned short int offset);
static unsigned short int test_dram (void);
void d64jr_usage (char *progname);

/**************************************
*               Subroutine            *
**************************************/
void dump_buffer(void)
{
for (i=0; i<128 ; i++)
   {
   j=i & 0x0f;
   disp_buf[j]=mix.buffer[i];
   if (j==15)
      {
      printf("%04x : ",i&0xfff0);
      for (j=0; j<16 ; j++)
	 {
	 if (j==8) printf("- ");
	 printf("%02x ",disp_buf[j]);
	 }
      printf("-> ");
      for (j=0; j<16 ; j++)
	 {
	 if (disp_buf[j] < 0x20 || disp_buf[j] > 0x80)
	    printf (".");
	 else
	    printf ("%c",disp_buf[j]);
	 }
      printf("\n");
      }
   }
}

void set_ai(unsigned char _ai)
{
   set_ai_write			// ninit=1, nwrite=0
   outportb(port_b,_ai);
}

void set_ai_data(unsigned char _ai,unsigned char _data)
{
   set_ai(_ai);
   set_data_write		// ninit=0, nwrite=0
   outportb(port_c,_data);
}

void init_port(void)
{
   outportb(port_9,1);		// clear EPP time flag
   set_ai_data(6,0x0a);
   set_ai_data(7,0x05);		// 6==0x0a, 7==0x05 is pc_control mode
//   set_ai(5);
//   set_data_read
//   write_en=inportb(port_c);
   set_ai_data(5,write_en);		// d0=0 is write protect mode
}

void end_port(void)
{
   set_ai_data(5,write_en);		// d0=0 is write protect mode
   set_ai_data(7,0);		// release pc mode
   set_ai_data(6,0);		// 6==0x0a, 7==0x05 is pc_control mode
   set_normal			// ninit=1, nWrite=1
}

char write_32k(unsigned short int hi_word, unsigned short int lo_word)
{
   unsigned char unpass,pass1;
   unsigned short int i,j;
   unsigned short int fix,temp;
   init_port();
   set_ai_data(3,0x10|(hi_word>>8));
   set_ai_data(2,(hi_word & 0xff));
   for (i=0;i<0x40;i++){
      unpass=3;
      while(unpass){
	 set_ai_data(1,((i<<1)|lo_word));
	 set_ai_data(0,0);
	 set_ai(4);		// set address index=4
	 set_data_write		// ninit=0, nWrite=0
	 fix=i<<8;
	 for (j=0;j<256;j++){
	    outportw(port_c,mix.bufferx[j+fix]);
	 }
	 set_data_read		// ninit=0, nWrite=1
	 if (wv_mode){
	    for (j=0;j<256;j++){
	       temp=inportw(port_c);
	       if(mix.bufferx[j+fix]!=temp){
//		  printf("%2x%2x dram=%x, buffer=%x\n",i,j*2,temp,mix.bufferx[j+fix]);
		  break;
	       }
	    }
	 }
	 else{
	    pass1=1;
	    for (j=0;j<4;j++){
	       temp=inportw(port_c);
	       if(mix.bufferx[j+fix]!=temp){
//		  printf("%2x%2x dram=%x, buffer=%x\n",i,j*2,temp,mix.bufferx[j+fix]);
		  pass1=0;
		  break;
	       }
	    }
	    if (pass1) {
//	       printf("@");
	       set_ai_data(1,((i<<1)|lo_word|1));
	       set_ai_data(0,0xf8);
	       set_ai(4);
	       set_data_read		// ninit=0, nWrite=1
	       for (j=252;j<256;j++){
		  temp=inportw(port_c);
		  if(mix.bufferx[j+fix]!=temp){
//		     printf("%2x%2x dram=%x, buffer=%x\n",i,j*2,temp,mix.bufferx[j+fix]);
		     break;
		  }
	       }
	    }
	 }
	 set_ai(0);
	 set_data_read			// ninit=0, nwrite=1
	 if (inportb(port_c)!=0x00){
	    unpass--;
//	    printf("counter=%x ",inportb(data));
	    outportb(port_a,0x0b);	// set all pin=0 for debug
//	    if (disp_on) printf("*");
	    init_port();
	    set_ai_data(3,0x10|(hi_word>>8));
	    set_ai_data(2,(hi_word & 0xff));
	    if (unpass==0)
	       return(1);
	 }
	 else
	    unpass=0;
      }
   }
//   outportb(ai,0);
//   printf("\na[7..0]=%02x\n",inportb(data));
//   outportb(ai,1);
//   printf("a[15..8]=%02x\n",inportb(data));
//   end_port();
   return(0);
}

char verify_32k(unsigned short int hi_word, unsigned short int lo_word)
{
   char unpass;
   unsigned short int i,j,temp;
   unsigned short int fix;
   init_port();
   set_ai_data(3,0x10|(hi_word>>8));
   set_ai_data(2,(hi_word & 0xff));
//   set_ai_data(3,0x10);
//   set_ai_data(2,hi_word);
   for (i=0;i<0x40;i++){
      unpass=3;
      while(unpass){
	 set_ai_data(1,((i<<1)|lo_word));
	 set_ai_data(0,0);
	 set_ai(4);
	 set_data_read			// ninit=0, nwrite=1
	 fix=i<<8;
	 for (j=0;j<256;j++){
	    temp=inportw(port_c);
	    if (temp!=mix.bufferx[j+fix]){
//	       printf("verify error!!!\07\n");
//	       printf("%2x%2x dram=%x, buffer=%x\n",i,j*2,temp,mix.bufferx[j+fix]);
	       outportb(port_a,0x0b);	// all pin=0 for debug
//	       if (disp_on) printf("#");
	       init_port();
	       set_ai_data(3,0x10|(hi_word>>8));
	       set_ai_data(2,(hi_word & 0xff));
	       unpass--;
	       if (unpass==0)
		   return(1);
	       else
		  break;
	    }
	 }
	 if (j==256) break;
      }
   }
//   outportb(ai,0);
//   printf("\na[7..0]=%02x\n",inportb(data));
//   outportb(ai,1);
//   printf("a[15..8]=%02x\n",inportb(data));
//   end_port();
   return(0);
}

void read_adr(void)
{
   set_ai_data(6,0x0a);		// enable pc mode
   set_ai_data(7,0x05);		// enable pc mode
   printf("\na[31..0]=");
   set_ai(3);
   set_data_read		// ninit=0, nwrite=1
   printf("%02x",inportb(port_c));
   set_ai(2);
   set_data_read		// ninit=0, nwrite=1
   printf("%02x",inportb(port_c));
   set_ai(1);
   set_data_read		// ninit=0, nwrite=1
   printf("%02x",inportb(port_c));
   set_ai(0);
   set_data_read		// ninit=0, nwrite=1
   printf("%02x\n",inportb(port_c));
   end_port();
}

void read_some(void)
{
   init_port();
   set_ai_data(0,0);
   set_ai_data(1,0);
   set_ai_data(2,0);
   set_ai_data(3,0x10);
   set_ai(4);
   set_data_read		// ninit=0, nWrite=1
   for (i=0;i<64;i++) {
      mix.bufferx[i]=inportw(port_c);
   }
   dump_buffer();
   read_adr();
}

unsigned char check_card(void)
{
   init_port();
   set_ai_data(3,0x12);
   set_ai_data(2,0x34);
   set_ai_data(1,0x56);
   set_ai_data(0,0x78);
   set_ai(3);
   set_data_read		// ninit=0, nwrite=1
   if ((inportb(port_c)&0x1f)!=0x12) return(1);
   set_ai(2);
   set_data_read		// ninit=0, nwrite=1
   if (inportb(port_c)!=0x34) return(1);
   set_ai(1);
   set_data_read		// ninit=0, nwrite=1
   if (inportb(port_c)!=0x56) return(1);
   set_ai(0);
   set_data_read		// ninit=0, nwrite=1
   if (inportb(port_c)!=0x78) return(1);
   end_port();
   return(0);
}

short int read_file(void)
{
   if (fread((char *)mix.buffer,sizeof(char),trans_size,fptr)!=trans_size)
      {
      fclose(fptr);	/* read data error */
      return(-1);
      }
#if 0
  printf(".");
  fflush(stdout);
#endif
  pos += trans_size;
  if (!(pos % (trans_size * 2))) ucon64_gauge (init_time, pos, size);
   return(0);
}

short int download_n64()
{
   if((fptr=fopen(file_name,"rb"))==NULL)
      {/* open error */
      printf("open error !!!\07\n");
      return(-1);
      }
   size = quick_fsize (file_name);

   if (sel==0)
      printf("Downloading");
   else
      printf("Verifying");
   for (page=0;page<0x400;page++) {
      if (read_file()!=0){
	 /*fclose(fptr);*/
	 printf("\n");
	 return (0);
      }
      if (sel==0){
	 if (write_32k(page,0)) {
	    read_adr();
	    /*fclose(fptr);*/
	    return(-1);
	 }
      }
      else{
	 if(verify_32k(page,0)) {
	    read_adr();
	    /*fclose(fptr);*/
	    return(-1);
	 }
      }
      if (read_file()!=0){
	 /*fclose(fptr);*/
	 return (-1);
      }
      if (sel==0){
	 if (write_32k(page,0x80)) {
	    read_adr();
	    /*fclose(fptr);*/
	    return(-1);
	 }
      }
      else{
	 if(verify_32k(page,0x80)) {
	    read_adr();
	    /*fclose(fptr);*/
	    return(-1);
	 }
      }

      }
   printf("\n");
   /*fclose(fptr);*/
   end_port();
   return(0);
}

void gen_pat_32k(unsigned short int offset)
{
   for (i=0; i<0x4000; i++){
      mix.bufferx[i]=i+offset;
   }
}


unsigned short int test_dram(void)
{
   unsigned short int pages;
   disp_on=0;		// display off
   sel=0;		//write 32k dram data
   gen_pat_32k(0x0000);
   write_32k(0,0);
   gen_pat_32k(0x8000);
   write_32k(0x100,0);
   gen_pat_32k(0x0000);
   pages=0;
   if (verify_32k(0,0)==0){	// find lower 128Mbits
      pages=0x100;
   }
   gen_pat_32k(0x8000);
   if (verify_32k(0x100,0)==0){	// find upper 128Mbits
      pages=0x200;
   }
   printf("DRAM testing...");
   for (page=0; page<pages; page++){
       gen_pat_32k(page*2);
       if (write_32k(page,0))
	  return(0);
       else
	  printf("w");
       fflush(stdout);
       gen_pat_32k(page*2 +1);
       if (write_32k(page,0x80))
	  return(0);
       else
	  printf("w");
       fflush(stdout);
   }
   for (page=0; page<pages; page++){
       gen_pat_32k(page*2);
       if (verify_32k(page,0))
	  return(0);
       else
	  printf("v");
       fflush(stdout);
       gen_pat_32k(page*2 +1);
       if (verify_32k(page,0x80))
	  return(0);
       else
	  printf("v");
       fflush(stdout);
   }
   return(pages);
}

static void usage(char *progname)
{
#if 0
   fprintf(stderr, "Usage: %s [-w] [-v] [-t] [-a] <File>\n", progname);
   fprintf(stderr, "-w : DRAM write protect disabled.\n");
   fprintf(stderr, "-v : verify File data vs DRAM data.\n");
   fprintf(stderr, "-t : test DRAM.\n");
   fprintf(stderr, "-a : enable cartridge and unprotect.\n");
   /*end_port();*/
   exit(2);
#endif
  return;   
}

/*************************************************
*                  MAIN ENTRY                    *
*************************************************/
int doctor64jr_main(int argc, char *argv[])
{
   char card_present;
   char *progname=argv[0];
   unsigned short int dram_size;
#if 0
   printf("---------- DrJr pc-download EPP version 1.0 ----------\n");
/*
   port[0]=peek(0x40,8); 		// lpt1 base address
   port[1]=peek(0x40,10);		// lpt2 base address
   if (port[0]==0){
      printf("No Printer Port Avialable!\07\n");
      return(-1);
   }
*/

   port[0]=0x378;
   port[1]=0;
   ioperm(0x378, 6, 1);
#endif
   if (argc==1) usage(progname);
    for( i=1; i<argc; i++ ) {
       if( argv[i][0] == '-' ) {
	  char *c = argv[i]+1;
	  if( *(c+1) != '\0' ) usage(progname);
	  switch( *c ) {
	     case 'w':
		write_en = 1;
		break;
	     case 'v':
		verify_en = 1;
		break;
	     case 't':
		test_en = 1;
		break;
	     case 'W':
		write_en = 1;
		break;
	     case 'V':
		verify_en = 1;
		break;
	     case 'T':
		test_en = 1;
		break;
	     case 'a':
		write_en = 3;
		break;
	     default:
		usage(progname);
	  }
       }
       else {
	  if( file_name == NULL ) {
	     file_name = argv[i];
	  }
	  else {
	     usage(progname);
	  }
       }
    }
//   printf("program name=%s\n",progname);
//   printf("write_en=%x, verify=%x, test=%x\n",write_en,verify_en,test_en);
//   printf("filename=%s\n",file_name);

   wv_mode=0;
   if (verify_en){      // if sel=2 for write/verify
      sel=1;		// sel=1 for verify
   }
   else{
      sel=0;		// sel=0 for write
   }
   if (port[1]==0)
      port_no=1;		// only one printer port
   else
      port_no=2;		// two printer port
   card_present=0;
   for (i=0;i<port_no;i++){
      port_8=port[i];
      port_9=port_8+1;
      port_a=port_9+1;
      port_b=port_a+1;
      port_c=port_b+1;
      if (check_card()==0){
	 card_present=1;
	 break;
      }
   }

   if (card_present==0){
      printf("\nNo V64jr card present!!!\07\n\n");
      return(-1);
   }
   else
      printf("V64jr card found at port%d\n",port_no);
   init_port();
   set_ai(3);
   set_data_read
   printf("control(rst,wdf,rcf) = %x\n",inportb(port_c));


   if (test_en){
      dram_size=test_dram();
      if (dram_size){
	 printf("\nDRAM size=%dMbits\n",(dram_size/2));
      }
      else {
	 printf("Error!!!\07\n");
      }
   }
   disp_on=1;		// display #/*
   if (file_name!=NULL){
      if (download_n64()!=0)
	 printf("download error !!!\n");
   }
   if (write_en)
      printf("dram write protect disable\n");
   if (write_en & 2)
      printf("run cart enable\n");

//   set_ai_data(5,write_en);		// d0=0 is write protect mode
   end_port();
   return(0);
}

/*
  It will save you some work if you don't fully integrate the code above with uCON64's code,
  because it is a project separate from the uCON64 project.
*/
int doctor64jr_argc;
char *doctor64jr_argv[128];

int
doctor64jr_read (const char *filename, unsigned int parport)
{
  char buf[MAXBUFSIZE];
  init_time = time (0);

  port[0] = parport;
  port[1] = 0;

  strcpy (buf, filename);

  doctor64jr_argv[0] = "jrsend";
  doctor64jr_argv[1] = buf;
  doctor64jr_argc = 2;

  doctor64jr_main (doctor64jr_argc, doctor64jr_argv);

  return 0;
}


int
doctor64jr_write (const char *filename, long start, long len, unsigned int parport)
{
  char buf[MAXBUFSIZE];
  init_time = time (0);
  port[0] = parport;
  port[1] = 0;
  strcpy (buf, filename);

  doctor64jr_argv[0] = "jrsend";
  doctor64jr_argv[1] = buf;
  doctor64jr_argc = 2;

  doctor64jr_main (doctor64jr_argc, doctor64jr_argv);

  return 0;
}

#endif // BACKUP
