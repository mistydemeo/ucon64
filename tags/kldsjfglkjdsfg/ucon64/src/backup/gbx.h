#include "../ucon64.h"

#define gbx_TITLE "GameBoy Xchanger"

int gbx_usage(int argc,char *argv[]);


int send_file(char *fname, int (*progress)(int), void (status)(char *status));
int xchanger_status(void);
int backup_cart(char *fname, int (*progress)(int), void (status)(char *status));
void init_xchanger(void);


int gbx_read(	char *filename
			,unsigned int parport
);
                        
int gbx_write(	char *filename
			,long start
			,long len
			,unsigned int parport
);
                                                                                                