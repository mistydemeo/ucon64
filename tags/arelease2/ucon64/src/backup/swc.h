#include "../ucon64.h"

void InitSWC(void);
void SendByte(char);
void SendData(char, char, char, char, char);
void SendBlock(char *);
void PrintBar(int, int);
void PrintInfo(FILE *, int *);

int swc_read(	char *filename
		,unsigned int parport
);

int swc_write(	char *filename
		,long start
		,long len
		,unsigned int parport
);
                                                                        
int swc_usage(int argc,char *argv[]);

#define swc_TITLE "Super WildCard 1.6XC/Super WildCard 2.8CC/Super Wild Card DX(2)/SWC\n1993/1994/1995/19XX Front Far East/FFE http://www.front.com.tw"
