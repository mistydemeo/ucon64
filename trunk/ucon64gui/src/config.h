/*
config.h - config file for uCON64gui

written by 2002 NoisyB (noisyb@gmx.net)

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
#ifndef CONFIG_H
#define CONFIG_H
//#define TODO
#define DEBUG
#define MAXBUFSIZE 32768

/*
sample/sample.[ch] are for consoles which have no specific support, yet

it is a good start to add support for a new console
*/
//#define SAMPLE

/*
  enables/disables support for zip files

  comment this if there is no zlib for your platform
  NOTE: not implemented yet and has therefore no effect
*/
//#define UNZIP


/*
  enables/disables probing in <console>_init()
  
  it's a good idea to leave this defined/enabled
*/
#define CONSOLE_PROBE


/*
  enables/disables the internal ROM database

  comment this and uCON64 will be only ~160kB in size but won't recognize
  ROMs for NES and other systems where ROMs have no internal header
*/
#define DB

/*
  enables/disables support for parallel port backup units

  comment this if your hardware has no parallel port
*/
#define BACKUP


#ifndef __MSDOS__               //AFAIK there is no Cdrdao for MSDOS
/*
  enables/disables support for cd backups

  comment this if there is no Cdrdao (http://cdrdao.sourceforge.net)
  port for your platform available
*/
#define BACKUP_CD

#endif // __MSDOS__

#endif // CONFIG_H
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
#undef const

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef gid_t

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* Define if you have the ANSI C header files.  */
#undef STDC_HEADERS

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef uid_t

/* Define if you have the getcwd function.  */
#undef HAVE_GETCWD

/* Define if you have the mkdir function.  */
#undef HAVE_MKDIR

/* Define if you have the rmdir function.  */
#undef HAVE_RMDIR

/* Define if you have the socket function.  */
#undef HAVE_SOCKET

/* Define if you have the strcspn function.  */
#undef HAVE_STRCSPN

/* Define if you have the strspn function.  */
#undef HAVE_STRSPN

/* Define if you have the strstr function.  */
#undef HAVE_STRSTR

/* Define if you have the strtol function.  */
#undef HAVE_STRTOL

/* Define if you have the <dirent.h> header file.  */
#undef HAVE_DIRENT_H

/* Define if you have the <limits.h> header file.  */
#undef HAVE_LIMITS_H

/* Define if you have the <ndir.h> header file.  */
#undef HAVE_NDIR_H

/* Define if you have the <sys/dir.h> header file.  */
#undef HAVE_SYS_DIR_H

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H

/* Define if you have the ibs library (-libs).  */
#undef HAVE_LIBIBS
