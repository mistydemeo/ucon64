/*
defines.h - miscellaneous definitions

Copyright (c) 1999 - 2005 NoisyB
Copyright (c) 2001 - 2005 dbjh


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef DEFINES_H
#define DEFINES_H

#ifdef __sun
#ifdef __SVR4
#define __solaris__
#endif
#endif

#ifdef  WORDS_BIGENDIAN
#undef WORDS_BIGENDIAN
#endif

#if     defined _LIBC || defined __GLIBC__
  #include <endian.h>
  #if __BYTE_ORDER == __BIG_ENDIAN
    #define WORDS_BIGENDIAN 1
  #endif
#elif   defined AMIGA || defined __sparc__ || defined __BIG_ENDIAN__ || \
        defined __APPLE__
  #define WORDS_BIGENDIAN 1
#endif

#ifdef  __MSDOS__                               // __MSDOS__ must come before __unix__,
  #define CURRENT_OS_S "MSDOS"                  //  because DJGPP defines both
#elif   defined __unix__
  #ifdef  __CYGWIN__
    #define CURRENT_OS_S "Win32 (Cygwin)"
  #elif   defined __FreeBSD__
    #define CURRENT_OS_S "Unix (FreeBSD)"
  #elif   defined __OpenBSD__
    #define CURRENT_OS_S "Unix (OpenBSD)"
  #elif   defined __linux__
    #define CURRENT_OS_S "Unix (Linux)"
  #elif   defined __solaris__
    #ifdef __sparc__
      #define CURRENT_OS_S "Unix (Solaris/Sparc)"
    #else
      #define CURRENT_OS_S "Unix (Solaris/i386)"
    #endif
  #else
    #define CURRENT_OS_S "Unix"
  #endif
#elif   defined _WIN32
  #ifdef  __MINGW32__
    #define CURRENT_OS_S "Win32 (MinGW)"
  #else
    #define CURRENT_OS_S "Win32 (Visual C++)"
  #endif
#elif   defined __APPLE__
  #if   defined __POWERPC__ || defined __ppc__
    #define CURRENT_OS_S "Apple (PPC)"
  #else
    #define CURRENT_OS_S "Apple"
  #endif
#elif   defined __BEOS__
  #define CURRENT_OS_S "BeOS"
#elif   defined AMIGA
  #if defined __PPC__
    #define CURRENT_OS_S "Amiga (PPC)"
  #else
    #define CURRENT_OS_S "Amiga (68K)"
  #endif
#else
  #define CURRENT_OS_S "?"
#endif


#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined AMIGA || defined __APPLE__      // Mac OS X actually
// GNU/Linux, Solaris, FreeBSD, OpenBSD, Cygwin, BeOS, Amiga, Mac (OS X)
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_S "/"
#else // DJGPP, Win32
#define FILE_SEPARATOR '\\'
#define FILE_SEPARATOR_S "\\"
#endif


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


//#ifndef LIB_VERSION
//#define LIB_VERSION(major, minor, step) (((major) << 16) | ((minor) << 8) | (step))
//#endif


//#define OFFSET(a, offset) ((((unsigned char *) &(a)) + (offset))[0])


#if     (!defined TRUE || !defined FALSE)
#define FALSE 0
#define TRUE (!FALSE)
#endif


#endif  // DEFINES_H
