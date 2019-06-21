/*
usb.c - USB support

Copyright (c) 2003       Ulrich Hecht <uli@emulinks.de>
Copyright (c) 2004       NoisyB
Copyright (c) 2015, 2018 dbjh


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
#ifdef  USE_USB
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif
#include <stdio.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include <string.h>
#include "misc/usb.h"


static int
usb_get_string_ascii (usb_dev_handle *dev, int index, char *buf, int buflen)
{
  char buffer[256];
  int rval, i;

  if ((rval = usb_get_string_simple (dev, index, buf, buflen)) >= 0)
    return rval;
  if ((rval = usb_control_msg (dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
                               (USB_DT_STRING << 8) + index, 0x0409, buffer,
                               sizeof buffer, 5000)) < 0)
    return rval;
  if (buffer[1] != USB_DT_STRING)
    {
      *buf = '\0';
      return 0;
    }
  if ((unsigned char) buffer[0] < rval)
    rval = (unsigned char) buffer[0];
  rval /= 2;
  // lossy conversion to ISO Latin1
  for (i = 1; i < rval; i++)
    {
      if (i > buflen)                           // destination buffer overflow
        break;
      buf[i - 1] = buffer[2 * i];
      if (buffer[2 * i + 1] != '\0')            // outside of ISO Latin1 range
        buf[i - 1] = '?';
    }
  buf[i - 1] = '\0';
  return i - 1;
}


int
usbport_open (usb_dev_handle **result_handle, int vendor_id, char *vendor_name,
              int product_id, char *product_name)
{
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *handle = NULL;
  int error_code = USBOPEN_ERR_NOTFOUND;

  usb_find_busses ();
  usb_find_devices ();
  for (bus = usb_get_busses (); bus; bus = bus->next)
    {
      for (dev = bus->devices; dev; dev = dev->next)
        if ((vendor_id == 0 || dev->descriptor.idVendor == vendor_id) &&
            (product_id == 0 || dev->descriptor.idProduct == product_id))
          {
            char vendor[256];
            int len = 0;

            handle = usb_open (dev);
            if (!handle)
              {
                error_code = USBOPEN_ERR_ACCESS;
                continue;
              }
            vendor[0] = '\0';
            if (dev->descriptor.iManufacturer > 0)
              len = usb_get_string_ascii (handle, dev->descriptor.iManufacturer,
                                          vendor, sizeof vendor);
            if (len < 0)
              error_code = USBOPEN_ERR_ACCESS;
            else
              {
                error_code = USBOPEN_ERR_NOTFOUND;
                if (strcmp (vendor, vendor_name) == 0)
                  {
                    char product[256];

                    product[0] = '\0';
                    if (dev->descriptor.iProduct > 0)
                      len = usb_get_string_ascii (handle, dev->descriptor.iProduct,
                                                  product, sizeof product);
                    if (len < 0)
                      error_code = USBOPEN_ERR_ACCESS;
                    else
                      {
                        error_code = USBOPEN_ERR_NOTFOUND;
                        if (strcmp (product, product_name) == 0)
                          {
                            char serial[256];

                            serial[0] = '\0';
                            if (dev->descriptor.iSerialNumber > 0)
                              len = usb_get_string_ascii (handle,
                                                          dev->descriptor.iSerialNumber,
                                                          serial, sizeof serial);
                            if (len < 0)
                              error_code = USBOPEN_ERR_ACCESS;
                            else
                              break;
                          }
                      }
                  }
              }
            usb_close (handle);
            handle = NULL;
          }
      if (handle)                               // we have found a device
        break;
    }
  if (handle)
    {
      error_code = USBOPEN_SUCCESS;
      *result_handle = handle;
    }
  return error_code;
}


struct usb_device *
usbport_probe (int vendor_id, int product_id)
{
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_init ();
  usb_find_busses ();
  usb_find_devices ();

  for (bus = usb_busses; bus; bus = bus->next)  // usb_busses is present in libusb
    for (dev = bus->devices; dev; dev = dev->next)
      if ((dev->descriptor.idVendor == vendor_id) &&
          (dev->descriptor.idProduct == product_id))
        return dev;

  return NULL;
}


int
usbport_read (usb_dev_handle *handle, int endpoint, char *buffer,
              int buffer_size, int timeout)
{
  int result = usb_bulk_read (handle, endpoint, buffer, buffer_size, timeout);

  if (result < 0)
    fprintf (stderr, "\n"
                     "ERROR: Could not read requested number of bytes from USB\n"
                     "       %s\n", usb_strerror ());
  return result;
}


int
usbport_write (usb_dev_handle *handle, int endpoint, char *buffer,
               int buffer_size, int timeout)
{
  int result = usb_bulk_write (handle, endpoint, buffer, buffer_size, timeout);

  if (result < 0)
    fprintf (stderr, "\n"
                     "ERROR: Could not write requested number of bytes to USB\n"
                     "       %s\n", usb_strerror ());
  return result;
}

#endif // USE_USB
