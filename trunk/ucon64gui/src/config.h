/*
config.h - config file for uCON64

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


#ifndef __MSDOS__ //AFAIK there is no Cdrdao for MSDOS
/*
  enables/disables support for cd backups

  comment this if there is no Cdrdao (http://cdrdao.sourceforge.net)
  port for your platform available
*/
#define BACKUP_CD

#endif // __MSDOS__

#endif // CONFIG_H
