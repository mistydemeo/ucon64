#include "mgd.h"

int mgd_usage(int argc,char *argv[])
{
if(argcmp(argc,argv,"-help"))printf("\n%s\n",mgd_TITLE);


printf("TODO:  -xmgd    send/receive to/from Multi Game*/MGD2/MGH/RAW; $FILE=PORT\n\
		receives automatically when $ROM does not exist\n\
");

if(argcmp(argc,argv,"-help"))
{
//TODO more info like technical info about cabeling and stuff for the copier

}
	return(0);
}