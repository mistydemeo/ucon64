/*
cd64.h - CD64 support for uCON64

written by 2001 NoisyB (noisyb@gmx.net)

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
#ifndef CD64_H
#define CD64_H
#include "../ucon64.h"

#define cd64_TITLE "CD64\n19XX UFO http://www.cd64.com"
#define cd64_HEADER_START 0
#define cd64_HEADER_LEN 0

/*
void c_break (int signum);
void test_key (void);
void parse_switches (int argc, char *argv[]);
void send_byte (unsigned char c);
void send_pbyte (unsigned char c);
unsigned char exchange_byte (char c);
unsigned char exchange_pbyte (char c);
unsigned char read_pbyte (void);
unsigned char header_byte (char c);
unsigned char header_pbyte (char c);
void send_long (unsigned long value);
void send_plong (unsigned long value);
unsigned int verify_checksum (unsigned int checksum_out);
void usage_message (void);
unsigned long long_hex_atoi (char *str);
int send (char comm, unsigned long saddr, char *str);
int grab (char comm, unsigned long addr, unsigned long length, char *str);
main (int argc, char *argv[]);
*/
int cd64_usage(int argc, char *argv[]);
#endif /* CD64_H */
