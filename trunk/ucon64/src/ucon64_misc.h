/*
ucon64_misc.h - miscellaneous functions for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
           2001 - 2002 dbjh
                  2001 Caz


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
#ifndef UCON64_MISC_H
#define UCON64_MISC_H

#ifdef  BACKUP
#ifdef  __BEOS__
typedef struct st_ioport
{
  unsigned long port;
  unsigned char data8;
  unsigned short data16;
} st_ioport_t;
#endif // __BEOS__

#define out1byte(p,x)   outportb(p,x)
#define in1byte(p)      inportb(p)

// DJGPP (DOS) has outportX() & inportX()
#if     defined __unix__ || defined __BEOS__
extern unsigned char inportb (unsigned short port);
extern unsigned short inportw (unsigned short port);
extern void outportb (unsigned short port, unsigned char byte);
extern void outportw (unsigned short port, unsigned short word);
#endif // defined __unix__ || defined __BEOS__
#endif // BACKUP

typedef struct st_track_modes
{
  char *common;
  char *cdrdao;
} st_track_modes_t;
#define MODE1_2048 0
#define MODE1_2352 1
#define MODE2_2336 2
#define MODE2_2352 3
extern const st_track_modes_t track_modes[];

#define MBIT 131072

#define PARPORT_DATA    0       // output
#define PARPORT_STATUS  1       // input
#define PARPORT_CONTROL 2
/*
  defines for unknown backup units/emulators
*/
typedef struct st_unknown_header
// uses SWC/FIG header layout ("hirom", "emulation1" and "emulation2" are FIG fields)
{
/*
  Don't create fields that are larger than one byte! For example size_low and size_high
  could be combined in one unsigned short int. However, this gives problems with little
  endian vs. big endian machines (e.g. writing the header to disk).
*/
  unsigned char size_low;
  unsigned char size_high;
  unsigned char emulation;
  unsigned char hirom;
  unsigned char emulation1;
  unsigned char emulation2;
  unsigned char pad[2];
  unsigned char id_code1;
  unsigned char id_code2;
  unsigned char type;
  unsigned char pad2[501];
} st_unknown_header_t;

#define UNKNOWN_HEADER_START 0
#define UNKNOWN_HEADER_LEN (sizeof (st_unknown_header_t))
extern const char *unknown_usage[];


/*
  wrapper for misc.c/filebackup()

  if move_name != NULL then filename will just be moved (renamed) and NOT
  duplicated (faster); move_name will contain the new name then
*/
extern const char *ucon64_fbackup (char *move_name, const char *filename);
extern void handle_existing_file (const char *dest, char *src);
extern void remove_temp_file (void); // possible temp file created by handle_existing_file()

/*
  wrapper for misc.c/gauge()
*/
extern int ucon64_gauge (time_t init_time, long pos, long size);

extern size_t filepad (const char *filename, long start, long unit);//pad a ROM in Mb
extern long filetestpad (const char *filename, st_rominfo_t *rominfo); //test if a ROM is padded

extern int ucon64_testsplit (const char *filename);//test if a ROM is split

extern unsigned int ucon64_parport_probe (unsigned int parport);
extern const char *ucon64_parport_error; //std. error message for parport
extern const char *ucon64_console_error; //std. error message if the correct console couldn't be found

extern void ucon64_wrote (const char *filename);

/*
  open an archive and look for a rom; if rom found return romname else
  return (char *)archive

  to delete the tempdir *dp must be closed with closedir2()
*/
extern const char *ucon64_rom_in_archive (DIR **temp, const char *archive,
  char *romname, const char *configfile);
extern int ucon64_bin2iso (const char *image, int track_mode);
extern int ucon64_trackmode_probe (const char *image);
extern int ucon64_mktoc (st_rominfo_t *rominfo);
extern int ucon64_mkcue (st_rominfo_t *rominfo);

extern int ucon64_e (const char *rominfo);
extern int ucon64_ls (const char *path, int mode);
extern int ucon64_configfile (void);

#endif // #ifndef UCON64_MISC_H
