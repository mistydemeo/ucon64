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

#include "misc.h"

void strrncpy(char *str,char *str2,long n);
char *strhtml(char *str);
