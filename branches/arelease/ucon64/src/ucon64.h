#include "ucon64_misc.h"

#define ucon64_KNOWN		-1
#define ucon64_UNKNOWN		0
#define ucon64_GB		1
#define ucon64_GENESIS		2
#define ucon64_SMS		3
#define ucon64_JAGUAR		4
#define ucon64_LYNX		5
#define ucon64_N64		6
#define ucon64_NEOGEO		7
#define ucon64_NES		8
#define ucon64_PCE		9
#define ucon64_PSX		10
#define ucon64_PSX2		11
#define ucon64_SNES		12
#define ucon64_SATURN		13
#define ucon64_DC		14
#define ucon64_CD32		15
#define ucon64_CDI		16
#define ucon64_REAL3DO		17
#define ucon64_ATARI		18
#define ucon64_SYSTEM16		19
#define ucon64_NEOGEOPOCKET	20
#define ucon64_GBA		21

//char *forceargs[];




#define ucon64_VERSION "1.9.6"
#define ucon64_TITLE "uCON64 1.9.6 GNU/Linux 1999/2000/2001 by NoisyB (noisyb@gmx.net)"


#define MBIT	131072

#define ucon64_NAME	0
#define ucon64_ROM	1
#define ucon64_FILE	2
#define ucon64_name() (getarg(argc,argv,ucon64_NAME))
#define ucon64_rom() (getarg(argc,argv,ucon64_ROM))
#define ucon64_file() (getarg(argc,argv,ucon64_FILE))

int ucon64_usage(int argc,char *argv[]);

unsigned int ucon64_parport;