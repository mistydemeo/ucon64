/*
ufosd.c - Super UFO Pro 8 SD support for uCON64

Copyright (c) 2017 - 2019 dbjh


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
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <stdlib.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "misc/archive.h"
#include "misc/misc.h"
#include "misc/term.h"
#include "misc/usb.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "backup/ufosd.h"


#ifdef  USE_USB
static st_ucon64_obj_t ufosd_obj[] =
  {
    {UCON64_SNES, WF_DEFAULT | WF_STOP | WF_NO_SPLIT}
  };
#endif

const st_getopt2_t ufosd_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Super UFO Pro 8 SD"/*"2012 Superufo.com http://www.superufo.com"*/,
      NULL
    },
#ifdef  USE_USB
    {
      "xufosd", 0, 0, UCON64_XUFOSD,            // send only
      NULL, "send ROM to Super UFO Pro 8 SD",
      &ufosd_obj[0]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

#ifdef  USE_USB

#define READ_BUFFER_SIZE 32768 // must be a multiple of 64

static int
find_output_endpoint (struct usb_config_descriptor *config,
                      unsigned char *endpoint_address)
{
  unsigned char n;

  for (n = 0; n < config->bNumInterfaces; n++)
    {
      struct usb_interface *interface = &config->interface[n];
      int m;

      for (m = 0; m < interface->num_altsetting; m++)
        {
          struct usb_interface_descriptor *interface_dsc = &interface->altsetting[m];
          unsigned char k;

          for (k = 0; k < interface_dsc->bNumEndpoints; k++)
            {
              struct usb_endpoint_descriptor *endpoint = &interface_dsc->endpoint[k];
              // endpoint direction is output?
              if ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == 0)
                {
                  *endpoint_address = endpoint->bEndpointAddress;
                  return interface_dsc->bInterfaceNumber;
                }
            }
        }
    }

  return -1;
}


static void
send_data (usb_dev_handle *handle, int interface_number,
           unsigned char endpoint_address, char *buffer, int buffer_size,
           time_t starttime, int totalsize, FILE *file)
{
  static int bytessent = 0;
  int numbytes = usbport_write (handle, endpoint_address, buffer, buffer_size,
                                1000);

  if (numbytes < 0)
    {
      free (buffer);
      fclose (file);
      usb_release_interface (handle, interface_number);
      usb_close (handle);
      exit (1);
    }
  bytessent += numbytes;
  ucon64_gauge (starttime, bytessent, totalsize);
}


static int
check_quit (void)
{
  return (!ucon64.frontend ? kbhit () : 0) && getch () == 'q';
}


int
ufosd_write_rom (const char *filename)
{
  int vendor_id = 0x1292, product_id = 0x4653, interface_number, totalsize,
      bytesread, quit = 0;
  struct usb_device *device;
  usb_dev_handle *handle = NULL;
  unsigned char endpoint_address;
  FILE *file;
  char *buffer;
  time_t starttime;

#if     defined __unix__ || defined __BEOS__ || defined __APPLE__
  init_conio ();
  if (register_func (deinit_conio) == -1)
    {
      fputs ("ERROR: Could not register function with register_func()\n", stderr);
      exit (1);
    }
#if     !defined __BEOS__ && !defined __CYGWIN__
  regain_privileges ();
#endif
#endif

  if ((device = usbport_probe (vendor_id, product_id)) == NULL ||
      (handle = usb_open (device)) == NULL)
    {
      fprintf (stderr,
               "ERROR: Could not open Super UFO USB device with vendor ID 0x%04x and product ID\n"
               "       0x%04x\n", vendor_id, product_id);
      exit (1);
    }
  printf ("Opened Super UFO USB device with vendor ID 0x%04x and product ID 0x%04x\n",
          vendor_id, product_id);

  // use the first configuration
  if (usb_set_configuration (handle, device->config->bConfigurationValue) < 0)
    {
      fputs ("ERROR: Could not set active configuration of Super UFO USB device\n", stderr);
      usb_close (handle);
      exit (1);
    }

  // use the first configuration here too
  if ((interface_number = find_output_endpoint (&device->config[0], &endpoint_address)) < 0)
    {
      fputs ("ERROR: Could not find output endpoint of Super UFO USB device\n", stderr);
      usb_close (handle);
      exit (1);
    }

#ifdef  __linux__
  /*
    According to the libusb-win32 documentation, on Linux, detaching the kernel
    driver is required before claiming the interface. However, the (Linux) F2A
    USB code does not do this. The same documentation also states that
    usb_set_configuration() must be called before claiming the interface which
    is also not something the (Linux) F2A USB code does. Because it is
    questionable whether the code can be tested on Linux at all I treat failure
    of usb_detach_kernel_driver_np() as nonfatal. - dbjh
  */
  if (usb_detach_kernel_driver_np (handle, interface_number) < 0)
    puts ("WARNING: Could not detach kernel driver of Super UFO USB device");
#endif

  if (usb_claim_interface (handle, interface_number) < 0)
    {
      fprintf (stderr, "ERROR: Could not claim interface (%u) of Super UFO USB device\n",
               interface_number);
      usb_close (handle);
      exit (1);
    }

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      usb_release_interface (handle, interface_number);
      usb_close (handle);
      exit (1);
    }
  if ((buffer = (char *) malloc (READ_BUFFER_SIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], READ_BUFFER_SIZE);
      fclose (file);
      usb_release_interface (handle, interface_number);
      usb_close (handle);
      exit (1);
    }

  totalsize = (int) ucon64.file_size - ucon64.nfo->backup_header_len + 64;
  printf ("Send: %d Bytes\n", totalsize);
  puts ("Press q to abort\n");
  starttime = time (NULL);

  send_data (handle, interface_number, endpoint_address, buffer, 0, starttime,
             totalsize, file);
  // send first 64 bytes of backup unit header
  if ((bytesread = fread (buffer, 1, 64, file)) > 0)
    send_data (handle, interface_number, endpoint_address, buffer, bytesread,
               starttime, totalsize, file);
  fseek (file, ucon64.nfo->backup_header_len, SEEK_SET);
  // send ROM image data
  while ((bytesread = fread (buffer, 1, READ_BUFFER_SIZE, file)) > 0)
    {
      quit = check_quit ();
      if (quit)
        {
          fputs ("\nTransfer aborted", stdout);
          break;
        }
      send_data (handle, interface_number, endpoint_address, buffer, bytesread,
                 starttime, totalsize, file);
    }
  if (!quit)
    send_data (handle, interface_number, endpoint_address, buffer, 0, starttime,
               totalsize, file);

  free (buffer);
  fclose (file);
  usb_release_interface (handle, interface_number);
  usb_close (handle);
#if     defined __unix__ || defined __APPLE__
  drop_privileges ();
#endif

  return 0;
}

#endif // USE_USB
