/*
met.c - eDonkey MET file support for flc

Copyright (c) 2004 by NoisyB

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
#include <string.h>
#include <time.h>
#include "misc/itypes.h"
#include "misc/filter.h"
#include "flc.h"
#include "flc_defines.h"
#include "met.h"


typedef struct
{
  char name[4096];
  uint32_t size;
  char type[4096];
  char format[4096];
  uint32_t copied_so_far;
  char temp_name[4096];
  char priority[4096];
  char status[4096];
} met_t;


static const char *
priority (int value)
{
  static char buffer[128];

  switch (value)
    {
    case 0:
      return "Low";
    case 1:
      return "Normal";
    case 2:
      return "High";
    default:
      sprintf (buffer, "Unknown (%d)", value);
      return buffer;
    }
}


static const char *
status (int value)
{
  static char buffer[128];

  switch (value)
    {
    case 0:
      return "Looking...";
    case 1:
      return "Paused";
    default:
      sprintf (buffer, "Unknown (%d)", value);
      return buffer;
    }
}


int
met_open (st_flc_t *flc)
{
  met_t met;
  unsigned char version;
  int date;                     // fixme: really a date? isn't it always zero? 
  unsigned char file_id[16];
  short number_partial_hashes;
  int num_meta_tags;
  int gap_start = 0, total_missing = 0;
  int first_gap = 0x7fffffff;
  int x;
  int hash_num;
  int tag_num;
  FILE *fh = NULL, *tmp = NULL;
  
  if (!(tmp = fopen (flc->dstfile, "wb")))
    return -1;
        
  if (!(fh = fopen (flc->srcfile, "rb")))
    {
      fclose (tmp);
      return -1;
    }

  if (fread (&version, 1, 1, fh) != 1)
    return -1; //cannot read version
  if (version != 224)
    return -1;
  if (fread (&date, 4, 1, fh) != 1)
    return -1; //cannot read date
  if (fread (&file_id, 1, 16, fh) != 16)
    return -1; //cannot read file id

  fprintf (stderr, "file id =");
  for (x = 0; x < 16; x++)
    fprintf (stderr, " %02x", (int) file_id[x]);
  fprintf (stderr, "\n");

  if (fread (&number_partial_hashes, 2, 1, fh) != 1)
    return -1; //cannot read number of partial hashes
  fprintf (stderr, "number of partial hashes = %d\n", number_partial_hashes);

  for (hash_num = 0; hash_num < number_partial_hashes; hash_num++)
    {
      unsigned char hash[16];

      if (fread (&hash, 1, 16, fh) != 16)
        return -1; //cannot read hash

      fprintf (stderr, "hash %3d =", hash_num);
      for (x = 0; x < 16; x++)
        fprintf (stderr, " %02x", (int) hash[x]);
      fprintf (stderr, "\n");
    }

  if (fread (&num_meta_tags, 4, 1, fh) != 1)
    return -1; //cannot read number of number of meta tags

  fprintf (stderr, "\nnumber of meta tags = %d\n", num_meta_tags);

  for (tag_num = 0; tag_num < num_meta_tags; tag_num++)
    {
      unsigned char tag_type;
      short name_length;
      char name[4096];

      if (fread (&tag_type, 1, 1, fh) != 1)
        return -1; //cannot read tag type

      if (fread (&name_length, 2, 1, fh) != 1)
        return -1; //cannot read name length

      if (fread (&name, 1, name_length, fh) != (size_t) name_length)
        return -1; //cannot read name
      name[name_length] = '\0';

      if (tag_type == 2)
        {
          short value_length;
          char value[4096];

          if (fread (&value_length, 2, 1, fh) != 1)
            return -1; //cannot read value length

          if (fread (&value, 1, value_length, fh) != (size_t) value_length)
            return -1; //cannot read value
          value[value_length] = '\0';

          if (name_length == 1)
            {
              switch (name[0])
                {
                case 1:
                  fprintf (stderr, "filename = \"%s\"\n", value);
                  strcpy (met.name, value);
                  break;
                case 3:
                  fprintf (stderr, "file type = \"%s\"\n", value);
                  strcpy (met.type, value);
                  break;
                case 4:
                  fprintf (stderr, "file format = \"%s\"\n", value);
                  strcpy (met.format, value);
                  break;
                case 18:
                  fprintf (stderr, "tempfile name = \"%s\"\n", value);
                  strcpy (met.temp_name, value);
                  break;
                default:
                  fprintf (stderr, "tag number %02d = \"%s\"\n", name[0], value);
                  break;
                }
            }
          else
            fprintf (stderr, "%-13s = \"%s\"\n", name, value);
        }
      else if (tag_type == 3)
        {
          int value;
          if (fread (&value, 4, 1, fh) != 1)
            return -1; //cannot read value

          if (name_length == 1)
            {
              switch (name[0])
                {
                case 2:
                  fprintf (stderr, "file size = %d (%.2f mb)\n", value,
                          (double) value / (1024 * 1024));
                  met.size = value;
                  break;
                case 8:
                  fprintf (stderr, "copied so far = %d (%.2f mb)\n", value,
                          (double) value / (1024 * 1024));
                  met.copied_so_far = value;
                  break;
                case 19:
                  fprintf (stderr, "priority = %s\n", priority (value));
                  strcpy (met.priority, priority (value));
                  break;
                case 20:
                  fprintf (stderr, "status = %s\n", status (value));
                  strcpy (met.status, status (value));
                  break;
                default:
                  fprintf (stderr, "tag number %02d = %d\n", name[0], value);
                  break;
                }
            }
          else
            {
              if (name[0] == 9)
                {
                  fprintf (stderr, "gap %3s from %10d", &name[1], value);
                  gap_start = value;
                  if (gap_start < first_gap)
                    first_gap = gap_start;
                }
              else if (name[0] == 10)
                {
                  int missing = value - gap_start;
                  total_missing += missing;
                  fprintf (stderr, " to %10d = %10d bytes missing\n", value, missing);
                }
              else
                fprintf (stderr, "%-13s = %d\n", name, value);
            }
        }
      else
        return -1; //unknown tag type
    }

  fprintf (stderr, "%d bytes = %.2f mb missing\n", total_missing,
          (double) total_missing / (1024 * 1024));
  if (first_gap < 0x7fffffff)
    fprintf (stderr, "first gap starts at %d (%d blocks are complete)\n", first_gap,
            first_gap / (9500 * 1024));

  fclose (fh);

  fprintf (tmp, "%s\n", met.name);
  fprintf (tmp, "Type: %s (%s)", met.type, met.format);
  fprintf (tmp, "Copied: %d (%s)", met.copied_so_far, met.temp_name);
  fprintf (tmp, "Status: %s (%s)", met.status, met.priority);

  fclose (tmp);

  return 0;
}


const st_filter_t met_filter = {
  FLC_MET,
  "met (eDonkey)",
  ".met",
  -1,
  .open = (int (*) (void *)) &met_open
};
