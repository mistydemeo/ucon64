#include "flc_misc.h"

#define WRITE 0
#define APPEND 1
#define MAXLINES 20

struct dirent *ep;
struct stat puffer;

#define flc_NAME	0
#define flc_FILE	1
#define flc_name() (getarg(argc,argv,flc_NAME))
#define flc_file() (getarg(argc,argv,flc_FILE))

void usage(void);

#define flc_TITLE "flc 0.9.3 1999/2000/2001 by NoisyB (noisyb@gmx.net)\nThis may be freely redistributed under the terms of the GNU Public License"
