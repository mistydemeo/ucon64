#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/ioctl.h>
#include <sys/perm.h>
#include <sys/stat.h>
//#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MBIT 131072

#define FALSE 0
#define TRUE 1

char hexDigit(	int value	//GameGenie "codec" routine
);

int hexValue(	char digit	//GameGenie "codec" routine
);

//#define hexByteValue(x ,y) ((hexValue(x) << 4) + hexValue(y))
int hexByteValue(	char x	//GameGenie "codec" routine
			,char y
);

int strdcmp(	char *str
		,char *str1
);

int argcmp(	int argc	// check the cmdline options for str
		,char *argv[]
		,char *str
);

char *getarg(	int argc	//get arg pos from cmdline options
		,char *argv[]
		,int pos
);

int findlwr(	char *str	//find any lower char in str
);

char *strupr(	char *str
);
                                                        
char *strlwr(	char *str
);

char *newext(	char *filename	//replace extension of str with ext
		,char *ext
);

char *stpblk(	char *str	//strip blanks from beginning of str
);

char *strtrim(	char *dest	//trim blanks from start and end of str
		,char *str
);

char *stplcr(	char *str	//kill all returns at end of str
);

char *strswap(	char *str	//swap bytes in str from start for len
		,long start
		,long len
);

int strhexdump(	char *str	//hexdump str from start for len
		,long start
		,long len
);

int renlwr(	char *dir	//rename all files in dir to lower case
);

int renupr(	char *dir	//rename all files in dir to upper case
);

void BuildCRCTable();		//needed for CRC32

unsigned long CalculateBufferCRC(	unsigned int count	//needed for CRC32
					,unsigned long crc
					,void *buffer
);

unsigned long CalculateFileCRC(	FILE *file	//needed for CRC32
);

unsigned long fileCRC32(	char *filename	//calculate CRC32 of filename beginning from start
				,long start
);

long quickftell(	char *filename	//return size of filename
);

long filencmp(	char *filename		//search filename for search from start for len
		,char *search
		,long start
		,long len
);

size_t quickfread(	void *dest	//same as fread but takes start and src is a filename
			,size_t start
			,size_t len
			,char *src
);


size_t quickfwrite(	const void *src	//same as fwrite but takes start and dest is a filename
			,size_t start
			,size_t len
			,char *dest
);


int quickfgetc(	char *filename		//same as fgetc but takes filename instead of FILE and a pos
		,long pos
);



int filehexdump(	char *filename	//same as strhexdump
			,long start
			,long len
);


size_t filepad(	char *filename	//pad file (if necessary) from start
		,long start	//ignore start bytes (if file has header or something)
		,long unit	//size of block (example: MBIT)
);

long filetestpad(	char *filename	//test if EOF is padded (repeating bytes) beginning from start
);

int filecopy(	char *src	//copy src from start for len to dest
		,long start
		,long len
		,char *dest
		,char *mode
);

char *filebackup(	char *filename
);



unsigned long filefile(	char *filename	//compare filename from start with filename2 from start2
			,long start
			,char *filename2
			,long start2
			,int similar	//TRUE==find similarities; FALSE==find differences
);




int filereplace(	char *filename	//search filename from start for search which has slen and replace with replace which has rlen
			,long start
			,char *search
			,long slen
			,char *replace
			,long rlen
);


int fileswap(	char *filename	//bytesswap filename from start for len
		,long start
		,long len
);


/*
unsigned char inportb(	unsigned int arg1	//read a byte from the parallel port
);

unsigned char outportb(	unsigned int arg1	//write a byte to the p.p.
			,unsigned int arg2
);

unsigned short int inport(	unsigned int arg1	//read a word from the p.p.
);

unsigned short int outport(	unsigned int arg1	//write a word to the p.p.
				,unsigned int arg2
);
*/

void out1byte(	unsigned int port		//read a byte from the p.p.
		,unsigned char c
);

unsigned char in1byte(	unsigned int port	//write a byte to the p.p.
);



unsigned int parport_probe(	unsigned int parport	//detect parallel port
);

int parport_write(	char src[]
			,unsigned int len
			,unsigned int parport
);

int parport_read(	char dest[]
			,unsigned int len
			,unsigned int parport
);

int parport_gauge(	time_t init_time
			,long pos
			,long size
);

