#include "../ucon64.h"

int syncHeader(unsigned int baseport);
int initCommunication(unsigned int port);
int checkSync(unsigned int baseport);
int sendFilename(unsigned int baseport, char name[]);
int sendUploadHeader(unsigned int baseport, char name[], long len);
int sendDownloadHeader(unsigned int baseport, char name[], long *len);

int doctor64_read(	char *filename
			,unsigned int parport
);

int doctor64_write(	char *filename
			,long start
			,long len
			,unsigned int parport
);

int doctor64_usage(int argc, char *argv[]);

#define doctor64_TITLE "Doctor V64\nBung Enterprises Ltd http://www.bung.com.hk"
