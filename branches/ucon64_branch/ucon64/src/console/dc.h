/*
dc.h - Dreamcast support for uCON64

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
#ifndef DC_H
#define DC_H
extern int dc_init (st_rominfo_t *rominfo);
extern const st_usage_t dc_usage[];
//extern int ip0000 (char *dev, char *name);
extern int dc_scramble (void);
extern int dc_unscramble (void);

typedef struct
{
  unsigned char hardware_id[0x10];  // Hardware ID (always "SEGA SEGAKATANA ")
  unsigned char maker_id[0x10];     // Maker ID (always "SEGA ENTERPRISES")
  unsigned char device_inf[0x10];   // Device Information
  unsigned char area_symbols[0x08]; // Area Symbols
  unsigned char peripherals[0x08];  // Peripherals
  unsigned char prod_nr[0x0a];      // Product number ("HDR-nnnn" etc.)
  unsigned char prod_v[0x06];       // Product version
  unsigned char release_date[0x10]; // Release date (YYYYMMDD)
  unsigned char boot_filename[0x10]; // Boot filename (usually "1ST_READ.BIN")
  unsigned char company[0x10];      // Name of the company that produced the disc
  unsigned char name[0x80];         // Name of the software
} dc_ip0000_header_t;

#endif /* DC_H */
