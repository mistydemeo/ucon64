/*
f2a.c - Flash 2 Advance support for uCON64

written by 2003 Ulrich Hecht (uli@emulinks.de)
           2004 NoisyB (noisyb@gmx.net)
           2004 dbjh


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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//#include <assert.h>                           // assert() is evil! E V I L !
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc.h"
#include "quick_io.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "f2a.h"
#include "console/gba.h"
#ifdef  HAVE_USB_H
#include <sys/types.h>
#include "misc_usb.h"
#endif
#ifdef  PARALLEL
#include "misc_par.h"
#endif


const st_usage_t f2a_usage[] =
  {
    {NULL, NULL, "Flash 2 Advance (Ultra)"},
    {NULL, NULL, "2003 Flash2Advance http://www.flash2advance.com"},
#ifdef PARALLEL
    {"xf2a", NULL, "send/receive ROM to/from Flash 2 Advance (Ultra); " OPTION_LONG_S "port=PORT\n"
                   "receives automatically (32 Mbits) when ROM does not exist"},
/*
    {"xf2amulti", "SIZE", "send multiple ROMs to Flash 2 Advance (Ultra) (makes\n"
                          "temporary multirom truncated to SIZE Mbit); file with loader\n"
                          "must be specified first, then all the ROMs; " OPTION_LONG_S "port=PORT"},
*/
    {"xf2ac", "N", "receive N Mbits of ROM from Flash 2 Advance (Ultra);\n"
                   OPTION_LONG_S "port=PORT"},
    {"xf2as", NULL, "send/receive SRAM to/from Flash 2 Advance (Ultra); " OPTION_LONG_S "port=PORT\n"
                    "receives automatically when SRAM does not exist"},
    {"xf2ab", "BANK", "send/receive SRAM to/from Flash 2 Advance (Ultra) BANK\n"
                      "BANK should be a number >= 1; " OPTION_LONG_S "port=PORT\n"
                      "receives automatically when SRAM does not exist"},
#endif // PARALLEL
    {NULL, NULL, NULL}
};


#if     (defined PARALLEL || defined HAVE_USB_H)

typedef struct
{
  unsigned int command;         // command to execute, see below
  unsigned int size;            // size of data block to read/write
  unsigned int pad1[2];
  unsigned int magic;           // magic number, see below
  unsigned int pad2[3];
  unsigned int unknown;         // no idea what this is for, seems to have to be 0xa for write, 0x7 for read
  unsigned int address;         // base address for read/write
  unsigned int sizekb;          // size of data block to read/write in kB
  unsigned char pad3[5 * 4 /*-1*/];
  // for some reason the original software uses a 63 bytes structure for outgoing messages,
  // not 64 as it does for incoming messages, hence the "-1". It all seems to work fine with
  // 64 bytes, too, and I therefore suspect this to be a bug in the original SW.

  // we use SENDMSG_SIZE to solve the problem - dbjh
} /*__attribute__ ((packed))*/ f2a_sendmsg_t;

#define SENDMSG_SIZE (sizeof (f2a_sendmsg_t) - 1)


typedef struct
{
  unsigned char data[64];
} recvmsg;

#define CMD_GETINF 5                    // get info on the system status
#define CMD_WRITEDATA 6                 // write data to RAM/ROM/SRAM
#define CMD_READDATA 7                  // read data from RAM/ROM/SRAM
#define CMD_MULTIBOOT1 0xff             // boot up the GBA stage 1, no parameters
#define CMD_MULTIBOOT2 0                // boot up the GBA stage 2, f2a_sendmsg_t.size has to be set

#define MAGIC_NUMBER 0xa46e5b91         // needs to be properly set for almost all commands

static int f2a_pport;
static time_t starttime = 0;
#ifdef  DEBUG
static int parport_debug = 1;
#else
static int parport_debug = 0;
#endif

typedef struct
{
  unsigned int pad1[3];
  unsigned int magic;
  unsigned int command;
  unsigned int address;
  unsigned int sizekb;
  unsigned int pad2;
  unsigned int exec;
  unsigned int exec_stub;
  unsigned char pad3[984];
} /*__attribute__ ((packed))*/ f2a_msg_cmd_t; // packed attribute is not necessary


#define PP_CMD_WRITEDATA  0x06
#define PP_CMD_READDATA   0x07
#define PP_CMD_WRITEROM   0x0a
#define PP_CMD_ERASE      0x0b
#define PP_HEAD_BOOT      0x01
#define PP_HEAD_READ      0x07
#define PP_HEAD_WRITE     0x06
#define PP_MAGIC_NUMBER   0xa46e5b91

#define LOGO_ADDR         0x06000000
#define EXEC_STUB         0x03002000
#define ERASE_STUB        0x03000c00
#define LOGO_SIZE         76800
#define BOOT_SIZE         18432
#define LOADER_SIZE       32768
#define F2A_FIRM_SIZE     23053

#define FLIP  1
#define HEAD  1
#define EXEC  1


static int f2a_boot_par (const char *ilclient2_fname, const char *illogo_fname);
static int f2a_write_par (int files_cnt, char **files, unsigned int addr);
static int f2a_read_par (unsigned int start, unsigned int size,
                         const char *filename);
static int f2a_erase_par (unsigned int start, unsigned int size);
static int f2a_send_buffer_par (int cmd, int addr,
                                int size, const unsigned char *resource, int head,
                                int flip, unsigned int exec, int mode);
static int f2a_send_cmd_par (int cmd, int addr, int size);
static int f2a_exec_cmd_par (int cmd, int addr, int size);
static int f2a_receive_data_par (int cmd, int addr, int size,
                                 const char *filename, int flip);
static int f2a_send_head_par (int cmd, int size);
static int f2a_send_raw_par (unsigned char *buffer, int len);
static int f2a_receive_raw_par (unsigned char *buffer, int len);
static int f2a_wait_par ();
static int parport_init (int port, int target_delay);
static int parport_init_delay (int target);
static void parport_out31 (unsigned char val);
static void parport_out91 (unsigned char val);
static void parport_nop ();

static int parport_nop_cntr;

static const unsigned char header[] = {
  0x49, 0x2d, 0x4c, 0x69, 0x6e, 0x6b, 0x65, 0x72,
  0x2e, 0x31, 0x30, 0x30, 0x00, 0x00, 0x01, 0xe8
};

enum
{
  UPLOAD_FAILED = 0,
  CANNOT_GET_FILE_SIZE
};

static const char *f2a_msg[] = {
  "ERROR: Upload failed\n",
  "ERROR: Can't determine size of file \"%s\"\n",
  NULL
};


#ifdef  HAVE_USB_H

static int f2a_init_usb (void);
static int usb_connect (void);
static int f2a_info (recvmsg *rm);
static int f2a_boot_usb (const char *ilclient_fname);
static int f2a_read_usb (int address, int size, const char *filename);
static int f2a_write_usb (int numfiles, char **files, int address);

usb_dev_handle *f2ahandle;


static int
f2a_init_usb (void)
{
  recvmsg rm;
  char ilclient_fname[FILENAME_MAX];

//  assert (sizeof (recvmsg) == 64);
  memset (&rm, 0, sizeof (rm));

  if (usb_connect ())
    {
      fprintf (stderr, "ERROR: Unable to connect to F2A USB linker\n");
      exit (1);                                 // fatal
    }
  f2a_info (&rm);
  if (rm.data[0] == 0)
    {
      if (ucon64.quiet < 0)
        printf ("Please turn on GBA with SELECT and START held down\n");

      get_property_fname (ucon64.configfile, "ilclient", ilclient_fname, "ilclient.bin");
      if (f2a_boot_usb (ilclient_fname))
        {
          fprintf (stderr, "ERROR: Booting GBA client binary was not successful\n");
          exit (1);                             // fatal
        }
      f2a_info (&rm);
    }
  return 0;
}


static int
usb_connect (void)
{
  int fp, result, firmware_loaded = 0;
  unsigned char f2afirmware[F2A_FIRM_SIZE];
  char f2afirmware_fname[FILENAME_MAX];
  struct usb_bus *busses;
  struct usb_bus *bus;
  struct usb_device *dev, *f2adev = NULL;

  get_property_fname (ucon64.configfile, "f2afirmware", f2afirmware_fname, "f2afirm.hex");
  if (q_fread (f2afirmware, 0, F2A_FIRM_SIZE, f2afirmware_fname) == -1)
    {
      fprintf (stderr, "ERROR: Unable to load F2A firmware (%s)\n", f2afirmware_fname);
      exit (1);                                 // fatal
    }

  usb_init ();
  usb_find_busses ();

find_f2a:
  usb_find_devices ();
  busses = usb_busses;                          // usb_busses is present in libusb

  for (bus = busses; bus; bus = bus->next)
    {
      for (dev = bus->devices; dev; dev = dev->next)
        {
          if ((dev->descriptor.idVendor == 0x547) &&
              (dev->descriptor.idProduct == 0x1002))
            f2adev = dev;
          if ((dev->descriptor.idVendor == 0x547) &&
              (dev->descriptor.idProduct == 0x2131) && !firmware_loaded)
            {
              if ((fp = open ("/proc/ezusb/dev0", O_WRONLY)))
                {
                  write (fp, f2afirmware, F2A_FIRM_SIZE);
                  close (fp);
                }
              else
                {
                  fprintf (stderr, "ERROR: Unable to upload F2A firmware\n"); // was: "[...] EZUSB firmware"
                  return -1;
                }
              firmware_loaded = 1;
              wait2 (2000);                     // give the EZUSB some time to renumerate
              goto find_f2a;
            }
        }
    }

  if (f2adev == NULL)
    {
      fprintf (stderr, "ERROR: Could not find F2A attached to USB\n");
      return -1;
    }

  f2ahandle = usb_open (f2adev);

  result = usb_claim_interface (f2ahandle, 0x4);
  if (result == -1)
    {
      fprintf (stderr, "ERROR: Could not claim USB interface\n");
      fprintf (stderr, "       %s\n", usb_strerror ());
      return -1;
    }
  result = usb_claim_interface (f2ahandle, 0x83);
  if (result == -1)
    {
      fprintf (stderr, "ERROR: Could not claim USB interface\n");
      fprintf (stderr, "       %s\n", usb_strerror ());
      return -1;
    }

  return 0;
}


static int
f2a_info (recvmsg *rm)
{
  f2a_sendmsg_t sm;
  unsigned int i;

  memset (&sm, 0, SENDMSG_SIZE);
  memset (rm, 0, sizeof (recvmsg));

  sm.command = CMD_GETINF;

/*
  if (usb_write (f2ahandle, (char *) &sm, SENDMSG_SIZE) == -1)
    return -1;
  if (usb_read (f2ahandle, (char *) rm, sizeof (recvmsg)) == -1)
    return -1;
*/

  for (i = 0; i < (SENDMSG_SIZE / 4); i++)
    printf ("%-2x %08X\n", i, *(((unsigned int *) (&sm)) + i));

  return 0;

#if 0 // unreachable code
  if (ucon64.quiet < 0)
    {
      printf ("info:");
      for (i = 0; i < (SENDMSG_SIZE / 4); i++)
        printf (" %08X", *(((unsigned int *) (rm)) + i));
      fputc ('\n', stdout);
    }

  return 0;
#endif
}


static int
f2a_boot_usb (const char *ilclient_fname)
{
  f2a_sendmsg_t sm;
  unsigned int ack[16], i;
  char ilclient[16 * 1024];

  printf ("Booting GBA\n"
          "Uploading iLinker client\n");
  if (q_fread (ilclient, 0, 16 * 1024, ilclient_fname) == -1)
    {
      fprintf (stderr, "ERROR: Unable to load GBA client binary (%s)\n", ilclient_fname);
      return -1;
    }

  // boot the GBA
  memset (&sm, 0, SENDMSG_SIZE);
  sm.command = CMD_MULTIBOOT1;
  usb_write (f2ahandle, (char *) &sm, SENDMSG_SIZE);
  sm.command = CMD_MULTIBOOT2;
  sm.size = 16 * 1024;
  usb_write (f2ahandle, (char *) &sm, SENDMSG_SIZE);

  // send the multiboot image
  if (usb_write (f2ahandle, ilclient, 16 * 1024) == -1)
    {
      fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
      return -1;
    }

  if (usb_read (f2ahandle, (char *) ack, 16 * 4) == -1)
    return -1;

  if (ucon64.quiet < 0)
    {
      printf ("post-boot:");
      for (i = 0; i < 16; i++)
        printf (" %08X", ack[i]);
      fputc ('\n', stdout);
    }

  return 0;
}


static int
f2a_read_usb (int address, int size, const char *filename)
{
  FILE *file;
  unsigned int i;
  f2a_sendmsg_t sm;
  char buffer[1024];

//  assert ((size & 1023) == 0);

  memset (&sm, 0, SENDMSG_SIZE);

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      //exit (1); for now, return, although registering usb_disconnect() is better
      return -1;
    }

  sm.command = CMD_READDATA;
  sm.magic = MAGIC_NUMBER;
  sm.unknown = 7;
  sm.address = address;
  sm.size = size;
  sm.sizekb = size / 1024;
  if (usb_write (f2ahandle, (char *) &sm, SENDMSG_SIZE) == -1)
    return -1;

  for (i = 0; i < sm.sizekb; i++)
    {
      if (usb_read (f2ahandle, buffer, 1024) == -1)
        {
          fclose (file);
          return -1;
        }
      if (!fwrite (buffer, 1024, 1, file))      // note order of arguments
        {
          fprintf (stderr, ucon64_msg[WRITE_ERROR], filename);
          fclose (file);
          return -1;                            // see comment for fopen() call
        }
    }
  fclose (file);
  return 0;
}


static int
f2a_write_usb (int numfiles, char **files, int address)
{
  f2a_sendmsg_t sm;
  int i, j, fsize;
  char buffer[1024];
  FILE *file;

  memset (&sm, 0, SENDMSG_SIZE);

  // initialize command buffer
  sm.command = CMD_WRITEDATA;
  sm.magic = MAGIC_NUMBER;
  sm.unknown = 0xa;                             // no idea what this is...

  for (j = 0; j < numfiles; j++)
    {
      if (ucon64.quiet < 1)
        printf ("Sending file %d: %s\n", j, files[j]);

      if ((fsize = q_fsize (files[j])) == -1)
        {
          fprintf (stderr, f2a_msg[CANNOT_GET_FILE_SIZE], files[j]);
          return -1;
        }

      // round up to 32k, FIXME: This has to be 128k for Turbo carts
      if (fsize & (32768 - 1))
        fsize += 32768;
      fsize &= ~(32768 - 1);

      if ((file = fopen (files[j], "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], files[j]);
          //exit (1); for now, return, although registering usb_disconnect() is better
          return -1;
        }

      // flash the ROM
      sm.size = fsize;
      sm.address = address;
      sm.sizekb = fsize / 1024;

      if (usb_write (f2ahandle, (char *) &sm, SENDMSG_SIZE) == -1)
        return -1;

      for (i = 0; i < fsize; i += 1024)
        {
          //printf ("writing chunk %d\n", i);
          if (!fread (buffer, 1024, 1, file))   // note order of arguments
            {
              fprintf (stderr, ucon64_msg[READ_ERROR], files[j]);
              fclose (file);
              return -1;                        // see comment for fopen() call
            }
          if (usb_write (f2ahandle, buffer, 1024) == -1)
            return -1;
        }

      fclose (file);
      address += fsize;
    }

  return 0;
}

#endif // HAVE_USB_H


static int
f2a_init_par (int parport, int parport_delay)
{
  char ilclient2_fname[FILENAME_MAX], illogo_fname[FILENAME_MAX];

  if (parport_init (parport, parport_delay))
    {
      fprintf (stderr, "ERROR: Unable to connect to F2A parport linker\n");
      exit (1);                                 // fatal
    }

  if (ucon64.quiet < 0)
    printf ("Please turn on GBA with SELECT and START held down\n");

  get_property_fname (ucon64.configfile, "ilclient2", ilclient2_fname, "ilclient2.bin");
  get_property_fname (ucon64.configfile, "illogo", illogo_fname, "illogo.bin");
  if (f2a_boot_par (ilclient2_fname, illogo_fname))
    {
      fprintf (stderr, "ERROR: Booting GBA client binary was not successful\n");
      exit (1);                                 // fatal
    }
  return 0;
}


int
parport_init (int port, int target_delay)
{
  int stat; //, ecpreg = port + 0x402, eppmode = 1;

  f2a_pport = port;
  parport_nop_cntr = parport_init_delay (target_delay);

  printf ("Using I/O port 0x%x\n", f2a_pport);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x04);
#if 0 // Don't write to the _STATUS_ register
  outportb ((unsigned short) (f2a_pport + PARPORT_STATUS), 0x01);
#endif
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_DATA), 0x04);

  parport_out91 (0x47);
  parport_out31 (0x02);
  parport_out91 (0x12);
  parport_out31 (0x01);
  parport_out91 (0x34);
  parport_out31 (0x00);
  parport_out91 (0x56);
  parport_out31 (0x02);
  parport_out91 (0x00);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x02);
  stat = inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
#if 0
  if (stat != 0x7e)
    {
      fprintf (stderr, "ERROR: Parallel port initialisation failed (%d)\n", stat);
      return -1;
    }
#endif
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x06);
  stat = inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
#if 0
  if (stat != 0x7e)
    {
      fprintf (stderr, "ERROR: Parallel port initialisation failed (%d)\n", stat);
      return -1;
    }
#endif

#if 0 // Don't write to the _STATUS_ register
  outportb ((unsigned short) (f2a_pport + PARPORT_STATUS), 0x01);
#endif
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x04);
  outportb ((unsigned short) (f2a_pport + PARPORT_EDATA), 0x07);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x02);
  outportb ((unsigned short) (f2a_pport + PARPORT_EDATA), 0x12);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EDATA), 0x34);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x00);
  outportb ((unsigned short) (f2a_pport + PARPORT_EDATA), 0x56);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x02);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x00);
  if (inportb ((unsigned short) (f2a_pport + PARPORT_EDATA)) != 0xff)
    {
      fprintf (stderr, "ERROR: EPP initialisation failed\n");
      return -1;
    }
  return 0;
}


int
f2a_boot_par (const char *ilclient2_fname, const char *illogo_fname)
{
  unsigned char recv[4], ilclient2[BOOT_SIZE];

  printf ("Booting GBA\n");
  if (f2a_send_head_par (PP_HEAD_BOOT, 1))
    return -1;
  if (f2a_receive_raw_par (recv, 4))
    return -1;

  if (illogo_fname != NULL)
    {
      unsigned char illogo[LOGO_SIZE];

      printf ("Uploading iLinker logo\n");
      if (q_fread (illogo, 0, LOGO_SIZE, illogo_fname) == -1)
        {
          fprintf (stderr, "ERROR: Unable to load logo file (%s)\n", illogo_fname);
          return -1;
        }
      if (f2a_send_buffer_par (PP_CMD_WRITEDATA, LOGO_ADDR, LOGO_SIZE, illogo,
                               0, 0, 0, 0))
        {
          fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
          return -1;
        }
    }

  printf ("Uploading iLinker client\n");
  if (q_fread (ilclient2, 0, BOOT_SIZE, ilclient2_fname) == -1)
    {
      fprintf (stderr, "ERROR: Unable to load GBA client binary (%s)\n", ilclient2_fname);
      return -1;
    }
  if (f2a_send_buffer_par (PP_CMD_WRITEDATA, EXEC_STUB, BOOT_SIZE, ilclient2,
                           HEAD, FLIP, EXEC, 0))
    {
      fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
      return -1;
    }
  return 0;
}


int
f2a_write_par (int files_cnt, char **files, unsigned int addr)
{
  int j, fsize, size;
  char loader_fname[FILENAME_MAX];
  unsigned char loader[LOADER_SIZE];

  if (files_cnt > 1)
    {
      printf ("Uploading multiloader\n");
      get_property_fname (ucon64.configfile, "f2aloader", loader_fname, "loader.bin");
      if (q_fread (loader, 0, LOADER_SIZE, loader_fname) == -1)
        {
          fprintf (stderr, "ERROR: Unable to load loader binary (%s)\n", loader_fname);
          return -1;
        }
      if (f2a_send_buffer_par (PP_CMD_WRITEROM, addr, LOADER_SIZE, loader, HEAD,
                               FLIP, 0, 0))
        {
          fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
          return -1;
        }
      addr += LOADER_SIZE;
    }
  for (j = 0; j < files_cnt; j++)
    {
      if ((fsize = q_fsize (files[j])) == -1)
        {
          fprintf (stderr, f2a_msg[CANNOT_GET_FILE_SIZE], files[j]);
          return -1;
        }
      size = fsize;
      if (size & (32768 - 1))
        size += 32768;
      size &= ~(32768 - 1);
      printf ("Uploading %s, %d kB, padded to %d kB\n",
               files[j], (int) (fsize / 1024), (int) (size / 1024));
      if (f2a_send_buffer_par (PP_CMD_WRITEROM, addr, size,
                               (unsigned char *) files[j], HEAD, FLIP, 0, 1))
        {
          fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
          return -1;
        }

      addr += size;
    }
  return 0;
}


int
f2a_erase_par (unsigned int start, unsigned int size)
{
  int end, addr;

  f2a_exec_cmd_par (PP_CMD_READDATA, ERASE_STUB, 1024);
  end = start + (size);

  printf ("Erase cart start=0x%08x end=0x%08x\n", start, end);
  for (addr = start; addr < end; addr += 0x40000)
    f2a_send_cmd_par (PP_CMD_ERASE, addr, 1024);
  return 0;
}


int
f2a_read_par (unsigned int start, unsigned int size, const char *filename)
{
  f2a_exec_cmd_par (PP_CMD_READDATA, ERASE_STUB, 1024);
  printf ("Reading from cart start=0x%08x size=0x%08x\n", start, size);
  f2a_receive_data_par (PP_CMD_READDATA, start, size, filename, FLIP);
  return 0;
}


#if 0
typedef struct
{
  unsigned char head[16];
  unsigned char command;
  unsigned char unknown;
  unsigned int size;
  unsigned char pad[58];
} __attribute__ ((packed)) f2a_msg_head_t;
#endif

static int
f2a_send_head_par (int cmd, int size)
{
  unsigned char trans[] = { 0xa, 0x8, 0xe, 0xc, 0x2, 0x0, 0x6, 0x4 }, msg_head[80];
//  f2a_msg_head_t msg_head;                    // Don't use structs with misaligned
  unsigned short int s;                         //  members for data streams (we don't
                                                //  want compiler-specific stuff)
  memcpy (&msg_head, header, 16);               // .head
  msg_head[16] = cmd;                           // .command
  s = size / 1024;
  msg_head[17] =                                // .unknown
    (trans[((s & 255) / 32)] << 4) | (((1023 - (s & 1023)) / 256) & 0x0f);
// msg_head.unkown = 0x82;
  msg_head[18] = (unsigned char) s;             // .size
  msg_head[19] = (unsigned char) (s >> 8);
  memset (&msg_head[20], 0, 80 - 20);

  if (f2a_send_raw_par (msg_head, 80))
    return -1;
  return 0;
}


static int
f2a_exec_cmd_par (int cmd, int addr, int size)
{
  unsigned char *buffer;
  f2a_msg_cmd_t msg_cmd;

  memset (&msg_cmd, 0, sizeof (f2a_msg_cmd_t));
  msg_cmd.magic = me2be_32 (PP_MAGIC_NUMBER);
  msg_cmd.command = me2be_32 (cmd);
  msg_cmd.address = me2be_32 (addr);
  msg_cmd.sizekb = me2be_32 (size / 1024);

  msg_cmd.exec_stub = me2be_32 (EXEC_STUB);
  msg_cmd.exec = me2be_32 (0x08);
  f2a_send_head_par (PP_HEAD_READ, size);
  f2a_wait_par ();

  if (parport_debug)
    fprintf (stderr,
             "sending msg_cmd cmd='0x%08x' addr='0x%08x' size='0x%08x' %d bytes\n",
             msg_cmd.command, msg_cmd.address, msg_cmd.sizekb,
             (int) sizeof (f2a_msg_cmd_t));


  f2a_send_raw_par ((unsigned char *) &msg_cmd, sizeof (f2a_msg_cmd_t));
//  f2a_wait_par ();
  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      exit (1);                                 // not return, caller doesn't handle it
    }
  f2a_receive_raw_par (buffer, size);
  free (buffer);

  return 0;
}


static int
f2a_receive_data_par (int cmd, int addr, int size, const char *filename, int flip)
{
  //unsigned char buffer[1024];
  //int i; //, n, progress;
  unsigned char recv[4], *mbuffer;
  int j;
  f2a_msg_cmd_t msg_cmd;
  FILE *file;

  memset (&msg_cmd, 0, sizeof (f2a_msg_cmd_t));
  msg_cmd.magic = me2be_32 (PP_MAGIC_NUMBER);
  msg_cmd.command = me2be_32 (cmd);
  msg_cmd.address = me2be_32 (addr);
  msg_cmd.sizekb = me2be_32 (size / 1024);

  if (f2a_send_head_par (PP_HEAD_READ, size))
    return -1;

  if (f2a_receive_raw_par (recv, 4))
    return -1;

  if (parport_debug)
    fprintf (stderr,
             "sending msg_cmd cmd='0x%08x' addr='0x%08x' size='0x%08x' %d bytes\n",
             msg_cmd.command, msg_cmd.address, msg_cmd.sizekb,
             (int) sizeof (f2a_msg_cmd_t));

  f2a_send_raw_par ((unsigned char *) &msg_cmd, sizeof (f2a_msg_cmd_t));

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      //exit (1); return, because the other code does it too...
      return -1;
    }

#if 1
  if ((mbuffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      //exit (1); see comment for fopen() call
      return -1;
    }
  f2a_receive_raw_par (mbuffer, size);
  if (flip)
    for (j = 0; j < size / 4; j++)
      ((int *) mbuffer)[j] = bswap_32 (((int *) mbuffer)[j]);

  if (!fwrite (mbuffer, size, 1, file))         // note order of arguments
    {
      fprintf (stderr, ucon64_msg[WRITE_ERROR], filename);
      fclose (file);
      free (mbuffer);
      return -1;                                // see comment for fopen() call
    }
  free (mbuffer);
#else
  for (i = 0; i < size; i += 1024)
    {
      f2a_receive_raw_par (buffer, 1024);
      if (flip)
        for (j = 0; j < 256; j++)
          ((int *) buffer)[j] = bswap_32 (((int *) buffer)[j]);

      if (!fwrite (buffer, 1024, 1, file))      // note order of arguments
        {
          fprintf (stderr, ucon64_msg[WRITE_ERROR], filename);
          fclose (file);
          return -1;                            // see comment for fopen() call
        }

      if (parport_debug)
        fprintf (stderr, "reading chunk %d of %d\n", (int) (i / 1024) + 1,
                 (int) (size / 1024));
      else
        {
#if 1
          ucon64_gauge (starttime, i, size);
#else
          progress = (int) ((100.0 / size) * (i + (1024)));
          printf ("\r[");
          for (n = 0; n <= 100; n++)
            if (n <= progress)
              printf ("=");
            else
              printf (" ");
          printf ("]\r");
          fflush (stdout);
#endif
        }
    }
  if (!parport_debug)
    fputc ('\n', stdout);
  fclose (file);
#endif

  return 0;
}


static int
f2a_send_cmd_par (int cmd, int addr, int size)
{
  unsigned char recv[4];
  f2a_msg_cmd_t msg_cmd;

  memset (&msg_cmd, 0, sizeof (f2a_msg_cmd_t));
  msg_cmd.magic = me2be_32 (PP_MAGIC_NUMBER);
  msg_cmd.command = me2be_32 (cmd);
  msg_cmd.address = me2be_32 (addr);
  msg_cmd.sizekb = me2be_32 (size / 1024);

  if (f2a_send_head_par (PP_HEAD_WRITE, size))
    return -1;

  if (f2a_receive_raw_par (recv, 4))
    return -1;

  if (parport_debug)
    fprintf (stderr,
             "parport_send_cmd cmd='0x%08x' addr='0x%08x' size='0x%08x' %d bytes\n",
             msg_cmd.command, msg_cmd.address, msg_cmd.sizekb,
             (int) sizeof (f2a_msg_cmd_t));

  if (f2a_send_raw_par ((unsigned char *) &msg_cmd, sizeof (f2a_msg_cmd_t)))
    return -1;
  return 0;
}


static int
f2a_send_buffer_par (int cmd, int addr, int size, const unsigned char *resource,
                     int head, int flip, unsigned int exec, int mode)
{
  unsigned char recv[4], buffer[1024];
  int i, j, n, progress;
  f2a_msg_cmd_t msg_cmd;
  FILE *file = NULL;

  (void) n;
  (void) progress;

  memset (&msg_cmd, 0, sizeof (f2a_msg_cmd_t));
  msg_cmd.magic = me2be_32 (PP_MAGIC_NUMBER);
  msg_cmd.command = me2be_32 (cmd);
  msg_cmd.address = me2be_32 (addr);
  msg_cmd.sizekb = me2be_32 (size / 1024);
  if (exec)
    {
      msg_cmd.exec_stub = me2be_32 (EXEC_STUB);
      msg_cmd.exec = me2be_32 (0x08);
    }
  if (f2a_send_head_par (PP_HEAD_WRITE, size))
    return -1;
  if (f2a_receive_raw_par (recv, 4))
    return -1;

  if (parport_debug)
    fprintf (stderr,
             "parport_send_buffer cmd='0x%08x' addr='0x%08x' size='0x%08x' %d bytes\n",
             msg_cmd.command, msg_cmd.address, msg_cmd.sizekb,
             (int) sizeof (f2a_msg_cmd_t));

  if (f2a_send_raw_par ((unsigned char *) &msg_cmd, sizeof (f2a_msg_cmd_t)))
    return -1;

  if (mode == 1)
    if ((file = fopen ((char *) resource, "rb")) == NULL)
      {
        fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], (char *) resource);
        //exit (1); return, because the other code does it too...
        return -1;
      }

  for (i = 0; i < size; i += 1024)
    {
      if (mode == 1)
        {
          if (!fread (buffer, 1024, 1, file))   // note order of arguments
            {
              fprintf (stderr, ucon64_msg[READ_ERROR], (char *) resource);
              fclose (file);
              return -1;                        // see comment for fopen() call
            }
        }
      else
        memcpy (buffer, resource, 1024);

      if (flip)
        for (j = 0; j < 256; j++)
          ((int *) buffer)[j] = bswap_32 (((int *) buffer)[j]);

      if (!i && head)
        {
          buffer[0] = 0x2e;                     // start address
          buffer[1] = 0x00;
          buffer[2] = 0x00;
          buffer[3] = 0xea;
          memcpy (buffer + 4, gba_logodata, GBA_LOGODATA_LEN);
          for (j = 0; j < 160 / 4; j++)         // 160 = 32-bit start address + logo size
            ((int *) buffer)[j] = bswap_32 (((int *) buffer)[j]);
        }
      if (parport_debug)
        fprintf (stderr, "sending chunk %d of %d\n", (int) (i / 1024) + 1,
                 (int) (size / 1024));
      else
        {
#if 1
          ucon64_gauge (starttime, i, size);
#else
          progress = (int) ((100.0 / size) * (i + 1024));
          printf ("\r[");
          for (n = 0; n <= 100; n++)
            if (n <= progress)
              printf ("=");
            else
              printf (" ");
          printf ("]\r");
          fflush (stdout);
#endif
        }
      f2a_send_raw_par (buffer, 1024);
      if (mode == 0)
        resource += 1024;
    }
  if (!parport_debug)
    fputc ('\n', stdout);
  if (mode == 1)
    fclose (file);
  return 0;
}


#ifdef  DEBUG
static void
parport_dump_byte (unsigned char byte)
{
  char i;

  for (i = 7; i >= 0; i--)
    {
      if ((byte >> i) & 1)
        fprintf (stderr, "1");
      else
        fprintf (stderr, "0");
    }
  fputc ('\n', stderr);
}
#endif


static int
f2a_receive_raw_par (unsigned char *buffer, int len)
{
  int err, i;
  unsigned char *ptr, nibble;
/*
  struct timespec t;

  t.tv_sec = 0;
  t.tv_nsec = 1;
*/

  ptr = buffer;
  if (parport_debug)
    fprintf (stderr, "\nreceive:\n%04x: ", 0);

  for (err = 0, i = 0; i < len * 2; i++)
    {
      nibble = 0;
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x04);
      parport_nop ();
      while (inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY)
        ; //nanosleep (&t, NULL); // nanosleep() is not portable & we like busy waiting
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x05);
      nibble = inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
      parport_nop ();
      while (!(inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY))
        ; //nanosleep (&t, NULL);
      if (i % 2)
        {
          *ptr |= (nibble >> 3) & 0x0f;
          if (parport_debug)
            {
              fprintf (stderr, "%02x ", (unsigned char) *ptr);
              if (!(((i / 2) + 1) % 32) && i && (i / 2) < len - 1)
                fprintf (stderr, "\n%04x: ", (i / 2) + 1);
            }
          ptr++;
        }
      else
        *ptr |= ((nibble >> 3) & 0xf) << 4;
    }
  if (parport_debug)
    fputc ('\n', stderr);

  return err;
}


static int
f2a_send_raw_par (unsigned char *buffer, int len)
{
  int timeout, i;
  unsigned char *pc;
/*
  struct timespec t;

  t.tv_sec = 0;
  t.tv_nsec = 1;
*/

  pc = buffer;
  if (parport_debug)
    fprintf (stderr, "\nsend:\n%04x: ", 0);
  for (i = 0; i < len; i++)
    {
      timeout = 2000;
      if (parport_debug)
        {
          fprintf (stderr, "%02x ", (unsigned char) *pc);
          if (!((i + 1) % 32) && i && i < len - 1)
            fprintf (stderr, "\n%04x: ", i + 1);
        }
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x04);
      parport_nop ();
      while ((inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY) &&
             (timeout--) > 0)
        ; //nanosleep (&t, NULL); // nanosleep() is not portable & we like busy waiting
      outportb ((unsigned short) (f2a_pport + PARPORT_DATA), *pc);
      parport_nop ();
      while ((inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY) &&
             (timeout--) > 0)
        ; //nanosleep (&t, NULL);
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x05);
      parport_nop ();
      while ((!(inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY)) &&
             (timeout--) > 0)
        ; //nanosleep (&t, NULL);
      pc++;
      if (timeout < 0)
        {
          fprintf (stderr, "\nERROR: Time-out\n");
          return -1;
        }
    }
  if (parport_debug)
    fputc ('\n', stderr);

  return 0;
}


static int
f2a_wait_par (void)
{
  int stat;

  while (1)
    {
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x04);
      parport_nop ();
      stat = inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
      if (stat & PARPORT_IBUSY)
        break;
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x05);
      parport_nop ();
      inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
    }
  return 0;
}


static int
parport_init_delay (int n_micros)
/*
  We only have millisecond accuracy, while we have to determine the correct
  initial loop counter value for a number of microseconds. Luckily, the function
  of time against the initial loop counter value is linear (provided that the
  initial counter value is large enough), so we can just divide the found loop
  value by 1000.
*/
{
#define N_CHECKS  10
#define N_HITSMAX 10
  struct timeb t0, t1;
  int n_millis = 0, n, n_hits = 0, loop = 10000, loop_sum = 0;
  volatile int m;                               // volatile is necessary for Visual C++...

  printf ("Determining delay loop value for %d microseconds...", n_micros);
  fflush (stdout);
  while (n_hits < N_HITSMAX)
    {
      n_millis = 0;
      for (n = 0; n < N_CHECKS; n++)
        {
          m = loop;
          ftime (&t0);
          while (m--)
            ;
          ftime (&t1);
          n_millis += (t1.time * 1000 + t1.millitm) - (t0.time * 1000 + t0.millitm);
        }
      n_millis /= N_CHECKS;

#ifndef DJGPP
      if (n_millis - n_micros == 0)             // we are aiming at microsecond accuracy...
#else // DJGPP's runtime system is quite inaccurate under Windows XP
      n = n_millis - n_micros;
      if (n < 0)
        n = -n;
      if (n <= 1)                               // allow a deviation of 1 ms?
#endif
        {
          n_hits++;
          loop_sum += loop;
          loop -= loop >> 3;                    // force "variation" in hope of better accuracy
          continue;
        }

      if (n_millis == 0)
        loop <<= 1;
      else
        loop = (int) (n_micros / ((float) n_millis / loop));
    }

  n = loop_sum / (1000 * N_HITSMAX);            // we summed N_HITSMAX loop values
  printf ("done (%d)\n", n);
  return n;
}


static void
parport_nop ()
{
  volatile int i = parport_nop_cntr;            // volatile is necessary for Visual C++...
  while (i--)
    ;
}


static void
parport_out31 (unsigned char val)
{
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x03);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_DATA), val);
}


static void
parport_out91 (unsigned char val)
{
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x09);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_DATA), val);
}


int
f2a_read_rom (const char *filename, unsigned int parport, int size)
{
  int offset = 0;

  starttime = time (NULL);
#ifdef  HAVE_USB_H
  if (ucon64.usbport)
    {
      f2a_init_usb ();
      f2a_read_usb (0x8000000 + offset * MBIT, size * MBIT, filename);
      usb_disconnect (f2ahandle);
    }
  else
#endif
    {
      f2a_init_par (parport, 10);
      f2a_read_par (0x08000000 + offset * MBIT, size * MBIT, filename);
    }
  return 0;
}


int
f2a_write_rom (const char *filename, unsigned int parport)
{
  int offset = 0;
  char *files[1] = { (char *) filename };

  starttime = time (NULL);
#ifdef  HAVE_USB_H
  if (ucon64.usbport)
    {
      f2a_init_usb ();
      f2a_write_usb (1, files, 0x8000000 + offset * MBIT);
      usb_disconnect (f2ahandle);
    }
  else
#endif
    {
      f2a_init_par (parport, 10);
//      f2a_erase_par (0x08000000, size * MBIT);
      f2a_write_par (1, files, 0x8000000 + offset * MBIT);
    }
  return 0;
}


int
f2a_read_sram (const char *filename, unsigned int parport, int bank)
{
  int size;

  if (bank == UCON64_UNKNOWN)
    {
      bank = 1;
      size = 256 * 1024;
    }
  else
    {
      if (bank < 1) // || bank > 4)
        {
//          fprintf (stderr, "ERROR: Bank must be 1, 2, 3 or 4\n");
          fprintf (stderr, "ERROR: Bank must be a number larger than or equal to 1\n");
          exit (1);
        }
      size = 64 * 1024;
    }
  bank--;

  starttime = time (NULL);
#ifdef  HAVE_USB_H
  if (ucon64.usbport)
    {
      f2a_init_usb ();
      f2a_read_usb (0xe000000 + bank * 64 * 1024, size, filename);
      usb_disconnect (f2ahandle);
    }
  else
#endif
    {
      f2a_init_par (parport, 10);
      f2a_read_par (0xe000000 + bank * 64 * 1024, size, filename);
    }
  return 0;
}


int
f2a_write_sram (const char *filename, unsigned int parport, int bank)
{
  char *files[1] = { (char *) filename };

  // define one bank as a 64 kilobyte unit
  if (bank == UCON64_UNKNOWN)
    bank = 1;
  else
    if (bank < 1) // || bank > 4)
      {
//        fprintf (stderr, "ERROR: Bank must be 1, 2, 3 or 4\n");
        fprintf (stderr, "ERROR: Bank must be a number larger than or equal to 1\n");
        exit (1);
      }
  bank--;

  starttime = time (NULL);
#ifdef  HAVE_USB_H
  if (ucon64.usbport)
    {
      f2a_init_usb ();
      f2a_write_usb (1, files, 0xe000000 + bank * 64 * 1024);
      usb_disconnect (f2ahandle);
    }
  else
#endif
    {
      f2a_init_par (parport, 10);
//      f2a_erase_par (0xe000000, size * MBIT);
      f2a_write_par (1, files, 0xe000000 + bank * 64 * 1024);
    }
  return 0;
}

#endif  // (defined PARALLEL || defined HAVE_USB_H)
