/*
smd.c - Super Magic Drive support for uCON64

written by 1999 - 2001 NoisyB (noisyb@gmx.net)
                  2001 dbjh

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

/*
 The environment created by the Genesis and SMD is not completely
 compatible with the SMS. Therefore, some SMS games will not work.

 When using the following options:

 - Backup Mega Drive cartridge.
 - Backup SRAM data.
 - Dump SMD BIOS.
 - Display cartridge and copier RAM information.

 You may get a 'timeout' error. If that happens, please try again.
 This is because the PC software runs much faster than the SMD, and
 sometimes will get out of sync when communicating.

 I'll also point out that reading data takes twice as long as writing,
 so dumping large cartridges can take a while. If no cartridge is loaded,
 the smd utility will dump the entire first four megabytes. This is useful
 for special carts which the SMD cannot identify.

 It's been reported that there are some difficulties when using 8MB
 and 24MB copiers.
*/

#include "smd.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#ifdef  __UNIX__
#include <unistd.h>                             // usleep(), microseconds
#elif   __DOS__
#include <dos.h>                                // delay(), milliseconds
#elif   __BEOS__
#include <OS.h>                                 // snooze(), microseconds
#endif

#ifndef __BEOS__
typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
#endif

static int smd_argc;
static char *smd_argv[128];
static uint16 lpt;
static uint8 block[0x4000];
static char *opts[] = {"-bc", "-lc", "-bs", "-ls", "-ci", "-db", "-rc", NULL};

/* from loader.asm */
static unsigned char loader[0x100] =
{
         0xF3, 0xDB, 0xBF, 0x31, 0xF0, 0xDF, 0x21, 0x00, 0xC0, 0x11, 0x01, 0xC0, 0x01, 0xFF, 0x0F, 0xAF,
         0x77, 0xED, 0xB0, 0x21, 0x00, 0xD1, 0x11, 0x01, 0xD1, 0x01, 0xFF, 0x0E, 0xAF, 0x77, 0xED, 0xB0,
         0x21, 0xB0, 0xD0, 0x01, 0xBF, 0x16, 0xED, 0xB3, 0xAF, 0xD3, 0xBF, 0x3E, 0xC0, 0xD3, 0xBF, 0xAF,
         0x0E, 0x3E, 0xD3, 0xBE, 0x0D, 0x20, 0xFB, 0xAF, 0xD3, 0xBF, 0x3E, 0x40, 0xD3, 0xBF, 0xAF, 0x06,
         0x3F, 0x0E, 0x00, 0xD3, 0xBE, 0x0D, 0x20, 0xFB, 0x05, 0x20, 0xF6, 0xAF, 0xD3, 0xBF, 0xD3, 0xBF,
         0xDD, 0xE5, 0xDD, 0xE1, 0xDB, 0xBE, 0xDD, 0xE5, 0xDD, 0xE1, 0xAF, 0xD3, 0xBF, 0x3E, 0x40, 0xD3,
         0xBF, 0xDB, 0xBF, 0xAF, 0xED, 0x47, 0xED, 0x4F, 0xDD, 0x21, 0xFF, 0xFF, 0xFD, 0x21, 0xFF, 0xFF,
         0x01, 0x00, 0x00, 0x11, 0x00, 0x00, 0x21, 0x00, 0x00, 0xE5, 0xF1, 0x08, 0xD9, 0x01, 0x00, 0x00,
         0x11, 0x00, 0x00, 0x21, 0x00, 0x00, 0xE5, 0xF1, 0x08, 0xD9, 0x31, 0x00, 0x00, 0xFB, 0xED, 0x46,
         0xAF, 0x32, 0x00, 0x20, 0x3E, 0x01, 0x32, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, 0xAF, 0x32, 0xFC,
         0xFF, 0x32, 0xFD, 0xFF, 0x3C, 0x32, 0xFE, 0xFF, 0x3C, 0x32, 0xFF, 0xFF, 0xAF, 0xC3, 0x00, 0x00,
         0x00, 0x80, 0x00, 0x81, 0x00, 0x82, 0x00, 0x83, 0x00, 0x84, 0x00, 0x85, 0x00, 0x86, 0x00, 0x87,
         0x00, 0x88, 0x00, 0x89, 0x00, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Function prototypes */
static void smd_send_byte(uint8 data);
static uint8 smd_recieve_byte(void);
static void smd_send_command(uint8 parameter, uint16 address, uint16 length);
static uint8 smd_recieve_block(uint32 length, uint8 *buffer);
static void smd_send_block(uint32 length, uint8 *buffer);
static void smd_poke(uint16 address, uint8 data);
static uint8 smd_peek(uint16 address);
static int save_smd(char *filename);
static int load_smd(char *filename);
static void interleave_buffer(uint8 *buffer, int size);
static int load_sram(char *filename);
static int save_sram(char *filename);
static int dump_bios(char *filename);

int smd_main (int argc, char **argv)
{
    int i;

    /* Print help text */
/*
    if(argc < 2)
    {
        printf("smd v0.1b - by Charles MacDonald\n");
        printf("Usage: %s [-options] <file.ext> ...\n", argv[0]);
        printf("Type '%s -help' for a list of options.\n", argv[0]);
        exit(1);
    }
*/
        /* Print help text */
/*
    if(strcmp(argv[1], "-help") == 0)
    {
        printf("Options:\n");
        printf(" -bc <file.smd>\tSave Mega Drive cartridge to disk.\n");
        printf(" -lc <file.ext>\tLoad Mega Drive (SMD, BIN) or SMS cartridge.\n");
        printf(" -bs <file.smd>\tSave SRAM data to disk.\n");
        printf(" -ls <file.smd>\tLoad SRAM data.\n");
        printf(" -ci           \tShow loaded cartridge size and available SMD RAM.\n");
        printf(" -db <file.bin>\tDump SMD BIOS to disk.\n");
        printf(" -rc           \tRun loaded cartridge.\n");
        exit(1);
    }
*/

    /* Check options */
    for(i = 0; opts[i] != NULL; i += 1)
    {
        if(strcmp(opts[i], argv[1]) == 0)
        {
            switch(i)
            {
                case 0: /* Backup cartridge */
                    save_smd(argv[2]);
                    break;

                case 1: /* Send cartridge */
                    load_smd(argv[2]);
                    break;

                case 2: /* Backup SRAM */
                    save_sram(argv[2]);
                    break;

                case 3: /* Send SRAM */
                    load_sram(argv[2]);
                    break;

                case 4: /* Show cartridge information */
                    {
                        uint8 mem;
                        uint8 blocks;
                        mem = smd_peek(0xDFF0);
                        blocks = smd_peek(0xDFF1);

                        printf("The SMD has %d bytes of RAM. (%dMB)\n", mem * 0x4000, (mem * 0x4000) / 0x20000);

                        if(!blocks)
                            printf("The SMD does not detect a cartridge.\n");
                        else
                            printf("The cartidge size is %d bytes. (%dMB)\n", blocks * 0x4000, (blocks * 0x4000) / 0x20000);
                    }
                    break;

                case 5: /* Dump BIOS */
                    dump_bios(argv[2]);
                    break;

                case 6: /* Run loaded cartridge */
                    smd_poke(0x2001, 0x02);
                    break;
            }
        }
    }

    return (0);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void smd_wait_busy(int state)
{
    int timeout = 0x2000;
    int busy = (inportb(lpt + PARPORT_STATUS) >> 7) & 1;

    while((busy == state) && (timeout))
    {
        busy = (inportb(lpt + PARPORT_STATUS) >> 7) & 1;
        timeout -= 1;
    }

    if(!timeout)
    {
        printf("Timeout - check connections and try again.\n");
        exit(1);
    }
}

void smd_send_byte(uint8 data)
{
    /* Wait for SMD busy flag to be set */
    smd_wait_busy(0);

    /* Send data to SMD */
    outportb(lpt + PARPORT_DATA, data);

    /* Invert PC busy flag */
    outportb(lpt + PARPORT_CONTROL, inportb(lpt + PARPORT_CONTROL) ^ 1);
}

uint8 smd_recieve_byte(void)
{
    uint8 temp;

    /* Wait for SMD busy flag to be cleared */
    smd_wait_busy(1);

    /* Read low nibble from SMD */
    temp = (inportb(lpt + PARPORT_STATUS) & 0x78) >> 3;

    /* Invert PC busy flag */
    outportb(lpt + PARPORT_CONTROL, inportb(lpt + PARPORT_CONTROL) ^ 1);

    /* Wait for SMD busy flag to be cleared */
    smd_wait_busy(1);

    /* Read high nibble from SMD */
    temp |= (inportb(lpt + PARPORT_STATUS) & 0x78) << 1;

    /* Invert PC busy flag */
    outportb(lpt + PARPORT_CONTROL, inportb(lpt + PARPORT_CONTROL) ^ 1);

    return (temp);
}

void smd_send_command(uint8 parameter, uint16 address, uint16 length)
{
    uint8 temp;
    uint8 count;
    uint8 checksum = 0x81;
    uint8 packet[5];

    packet[0] = parameter;
    packet[1] = (address & 0xFF);
    packet[2] = (address >> 8) & 0xFF;
    packet[3] = (length & 0xFF);
    packet[4] = (length >> 8) & 0xFF;

    smd_send_byte(0xD5);
    smd_send_byte(0xAA);
    smd_send_byte(0x96);


    /* Send command packet to SMD */
    for(count = 0; count < 5; count += 1)
    {
        temp = packet[count];
        checksum ^= temp;
        smd_send_byte(temp);
    }

    /* Send packet checksum */
    smd_send_byte(checksum);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

uint8 smd_recieve_block(uint32 length, uint8 *buffer)
{
    uint32 count;
    uint8 temp, checksum = 0x81;

#ifdef  __UNIX__                                // wait 32 milliseconds
    usleep(32000);
#elif   __DOS__
    delay(32);
#elif   __BEOS__
    snooze(32000);
#endif

    /* Read data from the SMD */
    for(count = 0; count < length; count += 1)
    {
        temp = smd_recieve_byte();
        checksum ^= temp;
        buffer[count] = temp;
    }

    checksum = smd_recieve_byte();

    /* Were the checksums the same? */
    return (checksum);
}


void smd_send_block(uint32 length, uint8 *buffer)
{
    uint32 count;
    uint8 temp, checksum = 0x81;

    /* Write data to the SMD */
    for(count = 0; count < length; count += 1)
    {
        temp = buffer[count];
        checksum ^= temp;
        smd_send_byte(temp);
    }

    /* Send our checksum */
    smd_send_byte(checksum);
}

void smd_poke(uint16 address, uint8 data)
{
    uint8 temp[1];
    temp[0] = data;
    smd_send_command(0x00, address, 1);
    smd_send_block(1, temp);
}

uint8 smd_peek(uint16 address)
{
    uint8 temp[1];
    smd_send_command(0x01, address, 1);
    smd_recieve_block(1, temp);
    return (temp[0]);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

/* Convert binary data to the SMD interleaved format */
void interleave_buffer(uint8 *buffer, int size)
{
    int count, offset;

    for(count = 0; count < size; count += 1)
    {
        memcpy(block, &buffer[count * 0x4000], 0x4000);

        for(offset = 0; offset < 0x2000; offset += 1)
        {
            buffer[(count * 0x4000) + 0x0000 + offset] = block[(offset << 1) | (1)];
            buffer[(count * 0x4000) + 0x2000 + offset] = block[(offset << 1) | (0)];
        }
    }
}

int load_smd(char *filename)
{
    uint8 header[0x200];
    uint8 count;
    uint8 *buf;
    int file_size;
    int block_size;
    int is_smd = 0;
    FILE *fd = NULL;

    /* Attempt to open file */
    fd = fopen(filename, "rb");
    if(!fd) return (0);

    /* Get file size */
    fseek(fd, 0, SEEK_END);
    file_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* Load SMD header */
    if(extcmp(filename, ".smd") == 0)
    {
        is_smd = 1;
        fread(header, 0x200, 1, fd);
        file_size -= 0x200;
    }

    /* Set up file buffer */
    block_size = (file_size / 0x4000) + ((file_size & 0x3FFF) ? 1 : 0);
    buf = malloc(block_size * 0x4000);
    if(!buf) return (0);
    memset(buf, 0, block_size * 0x4000);
    fread(buf, file_size, 1, fd);
    fclose(fd);

    /* Set up file data and header if the file is not in SMD foramt */
    if(!is_smd)
    {
        /* Initialize header and size field */
        memset(header, 0, 0x200);
        header[0] = block_size;

        /* 68000 binary files need file data to be interleaved */
        if(extcmp(filename, ".bin") == 0)
        {
            header[1] = 0x03;
            interleave_buffer(buf, block_size);
        }
        else
        {
            /* Assume all other extensions are Z80 programs */
            header[1] = 0x01;
        }
    }
    else
    {
        /* Use actual file size in case header had it wrong */
        header[0] = block_size;
    }

    /* Send blocks to SMD */
    for(count = 0; count < header[0]; count += 1)
    {
        printf("Sending block %d of %d\r", 1+count, header[0]);
        fflush(stdout);
        smd_send_command(0x05, count, 0x00);
        smd_send_command(0x00, 0x8000, 0x4000);
        smd_send_block(0x4000, buf + (count * 0x4000));
    }

    /* Advance to next line */
    printf("\n");

    /* Load and run loader program for Z80 files */
    if(header[1] == 0x01)
    {
        /* Send load command to D000 */
        smd_send_command(0x00, 0xD000, 0x0100);

        /* Upload loader program to D000-D0FF */
        smd_send_block(0x0100, loader);

        /* Force Z80 jump to $D000 */
        smd_send_command(0x04, 0xD000, 0x0000);
    }
    else
    {
        /* Disable M3 and enable program execution out of copier DRAM.
           For >2MB games, write 0x07 on SMD+ copiers.
           This turns off DRAM refresh on my original 16MB SMD, since
           bit 3 enables battery backed SRAM instead of having a
           special functions */

        smd_poke(0x2001, (header[0] > 0x80) ? 0x07 : 0x03);
    }

    /* Free file buffer */
    if(buf) free(buf);

    return (1);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

int save_smd(char *filename)
{
    uint8 header[0x200];
    int count;
    FILE *fd;

    fd = fopen(filename, "wb");
    if(!fd) return (0);

    /* The BIOS stores the cartridge size (in blocks) at $DFF1 */
    memset(header, 0, 0x200);
    header[0x00] = smd_peek(0xDFF1);

    header[0x01] = 0x03; /* File type: 68000 program */
    header[0x08] = 0xAA; /* Identifier #1 */
    header[0x09] = 0xBB; /* Identifier #2 */
    header[0x0A] = 0x06; /* File type: 68000 program */

    /* Write header to disk */
    fwrite(header, 0x200, 1, fd);

    for(count = 0; count < header[0]; count += 1)
    {
        printf("Recieving block %d of %d\r", 1+count, header[0]);
        fflush(stdout);
        smd_send_command(0x05, count, 0x00);
        smd_send_command(0x01, 0x4000, 0x4000);
        if(!smd_recieve_block(0x4000, block))
        {
            printf("Checksum mismatch - check connections and try again.\n");
            exit(1);            
        }
        fwrite(block, 0x4000, 1, fd);
    }

    printf("\n");
    fclose(fd);
    return (1);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

int load_sram(char *filename)
{
    uint8 header[0x200];
    FILE *fd = NULL;

    /* Attempt to open file */
    fd = fopen(filename, "rb");
    if(!fd) return (0);

    /* Load header */
    fread(header, 0x200, 1, fd);

    /* Map SRAM */
    smd_poke(0x2000, 0x00);
    smd_poke(0x2001, 0x04);

    printf("Loading SRAM block 1\r");
    fflush(stdout);
    smd_send_command(0x00, 0x4000, 0x4000);
    fread(block, 0x4000, 1, fd);
    smd_send_block(0x4000, block);

    printf("Loading SRAM block 2\r");
    fflush(stdout);
    smd_send_command(0x00, 0x8000, 0x4000);
    fread(block, 0x4000, 1, fd);
    smd_send_block(0x4000, block);

    fclose(fd);

    /* Advance to next line */
    printf("\n");

    /* Un-map SRAM */
    smd_poke(0x2001, 0x00);

    return (1);
}


int save_sram(char *filename)
{
    uint8 header[0x200];
    FILE *fd = NULL;

    /* Attempt to open file */
    fd = fopen(filename, "wb");
    if(!fd) return (0);

    /* Set up header */
    memset(header, 0, 0x200);
    header[0x00] = 0x00;
    header[0x01] = 0x00; /* SRAM file */
    header[0x02] = 0x00;
    header[0x08] = 0xAA;
    header[0x09] = 0xBB;
    header[0x0A] = 0x07; /* SRAM file */
    fwrite(header, 0x200, 1, fd);

    /* Map SRAM */
    smd_poke(0x2000, 0x00);
    smd_poke(0x2001, 0x04);

    printf("Saving SRAM block 1\r");
    fflush(stdout);
    smd_send_command(0x01, 0x4000, 0x4000);
    smd_recieve_block(0x4000, block);
    fwrite(block, 0x4000, 1, fd);

    printf("Saving SRAM block 2\r");
    fflush(stdout);
    smd_send_command(0x01, 0x8000, 0x4000);
    smd_recieve_block(0x4000, block);
    fwrite(block, 0x4000, 1, fd);

    fclose(fd);

    /* Un-map SRAM */
    smd_poke(0x2001, 0x00);

    /* Advance to next line */
    printf("\n");

    return (1);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

int dump_bios(char *filename)
{
    FILE *fd = NULL;

    /* Attempt to open file */
    fd = fopen(filename, "wb");
    if(!fd) return (0);

    /* Map SRAM */
    smd_poke(0x2000, 0x00);
    smd_poke(0x2001, 0x00);

    printf("Saving BIOS\r");
    fflush(stdout);
    smd_send_command(0x01, 0x0000, 0x2000);
    smd_recieve_block(0x2000, block);
    fwrite(block, 0x2000, 1, fd);

    fclose(fd);

    /* Advance to next line */
    printf("\n");

    return (1);
}

int smd_usage(int argc, char *argv[])
{
  if(argcmp(argc, argv, "-help"))
    printf("\n%s\n",smd_TITLE);

  printf("  -xsmd         send/receive ROM to/from Super Magicom Drive/SMD; $FILE=PORT\n"
         "                receives automatically when $ROM does not exist\n"
         "  -xsmds        send/receive SRAM to/from Super Magicom Drive/SMD; $FILE=PORT\n"
         "                receives automatically when $ROM(=SRAM) does not exist\n");

  if(argcmp(argc, argv, "-help"))
  {
  //TODO more info like technical info about cabeling and stuff for the copier
  }

  return 0;
}

int smd_read_rom(char *filename, unsigned int parport)
{
  lpt = parport;

  smd_argv[0] = "ucon64";
  smd_argv[1] = "-bc";
  smd_argv[2] = filename;
  smd_argc = 3;

  smd_main(smd_argc, smd_argv);

  return 0;
}

int smd_write_rom(char *filename, unsigned int parport)
{
  lpt = parport;

  smd_argv[0] = "ucon64";
  smd_argv[1] = "-lc";
  smd_argv[2] = filename;
  smd_argc = 3;

  smd_main(smd_argc, smd_argv);

  smd_argv[0] = "ucon64";
  smd_argv[1] = "-rc";
  smd_argc = 2;

  smd_main(smd_argc, smd_argv);

  return 0;
}

int smd_read_sram(char *filename, unsigned int parport)
{
  lpt = parport;

  smd_argv[0] = "ucon64";
  smd_argv[1] = "-bs";
  smd_argv[2] = filename;
  smd_argc = 3;

  smd_main(smd_argc, smd_argv);

  return 0;
}

int smd_write_sram(char *filename, unsigned int parport)
{
  lpt = parport;

  smd_argv[0] = "ucon64";
  smd_argv[1] = "-ls";
  smd_argv[2] = filename;
  smd_argc = 3;

  smd_main(smd_argc, smd_argv);

  return 0;
}
