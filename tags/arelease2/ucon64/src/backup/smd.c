#include "smd.h"

int smd_usage(int argc,char *argv[])
{
if(argcmp(argc,argv,"-help"))printf("\n%s\n",smd_TITLE);


printf("TODO:  -xsmd    send/receive to/from Super Magicom Drive/SMD; $FILE=PORT\n\
		receives automatically when $ROM does not exist\n\
");

if(argcmp(argc,argv,"-help"))
{
//TODO more info like technical info about cabeling and stuff for the copier

}
	return(0);
}