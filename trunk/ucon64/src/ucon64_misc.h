#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/ioctl.h>
//#include <sys/perm.h>
#include <sys/stat.h>
//#include <sys/types.h>
#include <time.h>
#include <unistd.h>


#define MBIT 131072
                
int strdcmp(char *str,char *str1);
int argcmp(int argc,char *argv[],char *str);
char *getarg(int argc,char *argv[],int pos);
int findlwr(char *str);
char *strupr(char *str);
char *strlwr(char *str);
char *newext(char *str,char *ext);
//char *stprblk(char *str1,char *str2);
char *stpblk(char *str);
char *stplcr(char *str);

int rencase(char *dir,char *mode);


int filecmp(char *f_name,long f_start,char *f2_name,long f2_start,int mode);
#define filecmp_DIFF 0
#define filecmp_SIMI 1



long quickfread(char *buf,long len,long start,char *file);
long quickfwrite(char *buf,long len,long start,char *file);

//int filereplace(char *fname,char wildcard,char *search,long slen,char *replace,long rlen,long fstart)

/*
	generate a standard checksum -- until it will be replaced by the real checksum methods for each console
*/
long filechksum(char *file_name,long file_start,int byte_offset);

/*
	same as ftell() but takes filename instead of pointer
*/
long quickftell(char *file_name);

/*
	search in file_name for search from file_start for file_len relative from file_start
*/
long filegrep(char *file_name,char *search,long file_start,long file_len);//fstrnstr

/*
	swap file from start for length
*/
int fileswap(char *file_name,long file_start,long file_length);
#define fileswap_EOF -1//can be used for file_length (means: until EOF)

/*
	same as fgetc() but takes filename instead of a pointer
*/
int quickfgetc(char *file_name,long pos);

/*	hexdump a file (or the string itself if file does not exist)
	from file_start for file_length
*/
int hexdump(char *file,long file_start,long file_length);
#define hexdump_EOF -1//can be used for file_length (means: until EOF)

/*	pads a file (mode="w") or checks if a file is padded (mode="r")
	returns the number of bytes added

	TODO:take MBIT argument (units)
*/
long filepad(char *file,long file_start,char *mode);

//filecopy which takes start position and length of source
int filecopy(char *source_file,long source_start,long source_length,char *dest_name,char *mode);
#define filecopy_EOF -1//can be used for source_len (means: until EOF)

/*
	makes backup of file an returns the original name
*/
char *filebackup(char *file);

/*
	needed by WyrmCorp's GameGenie "codec"
*/
char hexDigit(int value);
int hexValue(char digit);
int hexByteValue(char x, char y);

/*
	needed by the transfer routines
*/
int gauge(time_t fgtstart,long fpos,long fsize);
