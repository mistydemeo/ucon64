/*
smd.h - Super Magic Drive support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)


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
#ifndef _SMD_H
#define _SMD_H

#include "../ucon64.h"

#define smd_TITLE "Super Com Pro (HK)/Super Magic Drive/SMD\n" \
                   "19XX Front Far East/FFE http://www.front.com.tw"

int smd_usage(int argc,char *argv[]);
int smd_read_rom(char *filename, unsigned int parport);
int smd_write_rom(char *filename, unsigned int parport);
int smd_read_sram(char *filename, unsigned int parport);
int smd_write_sram(char *filename, unsigned int parport);


#define smd_HEADER_START 0
#define smd_HEADER_LEN 512


#include <stdio.h>
#include <malloc.h>
#include <string.h>
#ifndef __BEOS__
//#include <pc.h>         // inp
//#include <dos.h>        // delay
#endif
#define LPT_OUTPUT  (lpt + 0)
#define LPT_STATUS  (lpt + 1)
#define LPT_CONTROL (lpt + 2)

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;

/* Global data */
extern uint16 lpt;

/* Function prototypes */
void smd_send_byte(uint8 data);
uint8 smd_recieve_byte(void);
void smd_send_command(uint8 parameter, uint16 address, uint16 length);
uint8 smd_recieve_block(uint32 length, uint8 *buffer);
void smd_send_block(uint32 length, uint8 *buffer);
void smd_poke(uint16 address, uint8 data);
uint8 smd_peek(uint16 address);
int save_smd(char *filename);
int load_smd(char *filename);
void interleave_buffer(uint8 *buffer, int size);
int load_sram(char *filename);
int save_sram(char *filename);
int save_ram(char *filename, int blocks);
int dump_bios(char *filename);

#endif /* _SMD_H */
