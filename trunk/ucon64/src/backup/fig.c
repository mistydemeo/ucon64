#include "fig.h"

int fig_usage(int argc,char *argv[])
{
if(argcmp(argc,argv,"-help"))printf("\n%s\n",fig_TITLE);


printf("TODO:  -xfig	send/receive to/from *Pro Fighter*/(all)FIG; $FILE=PORT\n\
		receives automatically when $ROM does not exist\n\
");

if(argcmp(argc,argv,"-help"))
{
//TODO more info like technical info about cabeling and stuff for the copier

}
	return(0);
}