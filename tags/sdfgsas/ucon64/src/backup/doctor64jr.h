#include "../ucon64.h"

/*#include <dos.h>*/
#include <stdio.h>
#include <stdlib.h>
/*#include <io.h>*/
/*#include <dir.h>*/
#include <unistd.h>


//#define ai 0x37b
//#define data 0x37c
#define trans_size 32768
#define set_ai_write outportb(port_a,5);		// ninit=1, nwrite=0
#define set_data_write outportb(port_a,1);	// ninit=0, nwrite=0
#define set_data_read outportb(port_a,0);	// ninit=0, nwrite=1
#define set_normal outportb(port_a,4);		// ninit=1, nwrite=1
/*
unsigned char inportb(arg1);
unsigned char outportb(arg1,arg2);
unsigned short int inport(unsigned int arg1);
unsigned short int outport(unsigned int arg1,unsigned int arg2);
*/

/**************************************
*               Subroutine            *
**************************************/

void dump_buffer(void);
void set_ai(unsigned char _ai);
void set_ai_data(unsigned char _ai,unsigned char _data);
void init_port(void);
void end_port(void);
char write_32k(unsigned short int hi_word, unsigned short int lo_word);
char verify_32k(unsigned short int hi_word, unsigned short int lo_word);
void read_adr(void);
void read_some(void);
unsigned char check_card(void);
short int read_file(void);
short int download_n64();
void gen_pat_32k(unsigned short int offset);
unsigned short int test_dram(void);
void d64jr_usage(char *progname);

/*************************************************
*                  MAIN ENTRY                    *
*************************************************/
int d64jr_main(int argc, char *argv[]);


int doctor64jr_read(	char *filename
			,unsigned int parport
);

int doctor64jr_write(	char *filename
			,long start
			,long len
			,unsigned int parport
);


#define doctor64jr_TITLE "Doctor64 Jr"

int doctor64jr_usage(int argc,char *argv[]);
