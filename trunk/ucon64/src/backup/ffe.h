/*
ffe.h - General Front Far East copier routines for uCON64

written by 2002 dbjh


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
#ifndef FFE_H
#define FFE_H


#ifdef BACKUP

#define STROBE_BIT      1

extern void ffe_init_io (unsigned int port);
extern void ffe_deinit_io (void);
extern void ffe_send_block (unsigned short address, unsigned char *buffer, int len);
extern void ffe_send_command0 (unsigned short address, unsigned char byte);
extern void ffe_send_command (unsigned char command_code, unsigned short a, unsigned short l);
extern void ffe_receive_block (unsigned short address, unsigned char *buffer, int len);
extern unsigned char ffe_receiveb (void);
extern void ffe_wait_for_ready (void);
extern void ffe_checkabort (int status);

extern int ffe_port;
#endif // BACKUP

#endif // FFE_H
