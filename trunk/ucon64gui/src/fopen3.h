/*
fopen3.h - fopen() like wrapper for network io
           TODO: supposed to be merged into misc.h later

written by 2002 NoisyB (noisyb@gmx.net)


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
#ifndef FOPEN3_H
#define FOPEN3_H
#ifdef  __cplusplus
extern "C"
{
#endif

#ifndef MAX_HOSTNAME
#define MAX_HOSTNAME 256
#endif

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif

#ifndef ARGS_MAX
#define ARGS_MAX 128
#endif

#ifdef  HAVE_SOCKET
/*
  fopen3() works like fopen() but for URLs
  
  returns file_ptr
  
  mode
    "r","rb"
   TODO: "w","wb","a","ab"
*/
extern int fopen3 (const char *url, const void *mode);
#endif //  HAVE_SOCKET


typedef struct
{
  char *url;
  
  char *protocol;
  char *user;
  char *pass;
  char *host;
  unsigned int port;
  char *file;

  char *cmd;
  int argc;
  char *argv[ARGS_MAX];
} URL_t;


/*
  url_new() parse url into 
            [protocol://][username][:password]hostname[:port][/...]
            returns URL_t
            the special thing is that url_new() parses   [/...] into argc and
              argv for main() or getopt()
            by doing this http request's could be handled with getopt()

  url_free() free URL_t
  wget()   like wget for cmdline; will write the stream to a file in cwd
  portscan() portscan function 
*/
#ifdef  HAVE_SOCKET
extern int wget (const char *url);
//extern int portscan (const char *url, int port_start, int port_end);
#endif //  HAVE_SOCKET
extern URL_t *url_new (const char *url);
extern void url_free (URL_t * url);

#ifdef  __cplusplus
}
#endif
#endif // FOPEN3_H
