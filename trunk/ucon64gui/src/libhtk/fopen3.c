/*
fopen3.c - fopen() like wrapper for network io
           TODO: supposed to be merged into misc.c later
           
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "fopen3.h"
#include "misc.h"
#ifdef  HAVE_SOCKET
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif //  HAVE_SOCKET

//#define USE_IPV6 1
#define ANONYMOUS_S "anonymous"

URL_t url_new__;
#if 0
static void url_unescape_string (char *outbuf, char *inbuf); 
static void url_escape_string (char *outbuf, char *inbuf);
#endif

#ifdef  HAVE_SOCKET
static int tcp_open (char *address, int port);
//static int udp_open (char *address, int port);

static int
send2 (int tcp_sock, const char *s)
{
  int i;
  char c;
  char answer[1024];

  if (s)  send (tcp_sock, s, strlen (s), 0);

  do
    {
      /* Read a line */
      for (i = 0, c = 0; i < 1024 && c != '\n'; i++)
        {
          read (tcp_sock, &c, sizeof (char));
          answer[i] = c;
        }
      answer[i] = 0;
      fprintf (stderr, answer);
    }
  while (answer[3] == '-');

  answer[3] = 0;

  return atoi (answer);
}


int
fopen3 (const char *url, const void *mode)
{
  int fd = 0;
  URL_t *Curl = 0;
#ifdef  DEBUG
  int x = 0;
  unsigned long addr;
  struct hostent *host = NULL;
#endif

  if (!(Curl = url_new (url)))
//    return fopen2 (path, mode);
//    return fopen (path, mode);
    return 0;

#ifdef  DEBUG
  if (!Curl->file)
    {
      if (gethostname (Curl->host, MAX_HOSTNAME) != -1)
        host = gethostbyname (Curl->host);
    }
  else
    {
      if ((addr = inet_addr (Curl->host)) == -1)
        host = gethostbyname (Curl->host);
      else
        host = gethostbyaddr ((char *) &addr, sizeof (addr), AF_INET);
    }

  fprintf (stderr, "hostname: %s\n", Curl->host);

  for (x = 0; host->h_addr_list[x] > 0; x++)
    fprintf (stderr, "ip: %s\n",
             inet_ntoa (*(struct in_addr *) host->h_addr_list[x]));
#endif

  if (!(fd = tcp_open (Curl->host, Curl->port)))
    {
      url_free (Curl);
      return 0;
    }

  return fd;
}


int
wget (const char *url)
// wget function
{
  char buf[FILENAME_MAX];
  char c;
  int fd;
  int data_sock;
  struct sockaddr_in stLclAddr;
  socklen_t namelen;
  int i;
  URL_t *Curl = 0;
  FILE *fh;

  if (!(Curl = url_new (url)))
    return -1;

  /* Open a TCP socket */
  if (!(fd = fopen3 (url, "rb")))
    {
      url_free (Curl);
      return -1;
    }

  switch (Curl->port)
    {
    case 21:                   // ftp
      /* Send FTP USER and PASS request */

      send2 (fd, NULL);

      snprintf (buf, sizeof buf, "USER %s\r\n", Curl->user);
      if (send2 (fd, buf) != 331)
        return 0;

      snprintf (buf, sizeof buf, "PASS %s@\r\n", Curl->pass);
      if (send2 (fd, buf) != 230);
        return 0;

      if (send2 (fd, "TYPE I\r\n") != 200);
        return 0;

      snprintf (buf, sizeof buf, "CWD %s\r\n", dirname2 (Curl->file));
      if (send2 (fd, buf) != 250);
        return 0;

      /* Get interface address */
      namelen = sizeof (stLclAddr);
      if (getsockname (fd, (struct sockaddr *) &stLclAddr, &namelen) < 0)
        return 0;

      /* Open data socket */
      if ((data_sock = socket (PF_INET, SOCK_STREAM, 0)) < 0)
        return 0;

      stLclAddr.sin_family = AF_INET;

      /* Get the first free port */
      for (i = 0; i < 0xC000; i++)
        {
          stLclAddr.sin_port = htons (0x4000 + i);
          if (bind (data_sock, (struct sockaddr *) &stLclAddr,
               sizeof (stLclAddr)) >= 0)
            break;
        }
      Curl->port = 0x4000 + i;

      if (listen (data_sock, 1) < 0)
        return 0;

      i = ntohl (stLclAddr.sin_addr.s_addr);
      sprintf (buf, "PORT %d,%d,%d,%d,%d,%d\r\n",
               (i >> 24) & 0xFF, (i >> 16) & 0xFF,
               (i >> 8) & 0xFF, i & 0xFF, (Curl->port >> 8) & 0xFF, Curl->port & 0xFF);
      if (send2 (fd, buf) != 200)
        return 0;

      snprintf (buf, sizeof (buf), "RETR %s\r\n", Curl->file);
      if (send2 (fd, buf) != 150)
        return 0;

      return accept (data_sock, NULL, NULL);
      break;

    case 80:                   // http
    case 8080:
    default:
/* Send HTTP GET request
   Please don't use a Agent know by shoutcast (Lynx, Mozilla) seems to
   be reconized and print a html page and not the stream */

      snprintf (buf, sizeof (buf),
                "GET /%s HTTP/1.0\r\n"
//                "User-Agent: Mozilla/2.0 (Win95; I)\r\n"
                "User-Agent: %d (%d)\r\n"
                "Pragma: no-cache\r\n"
                "Host: %s\r\n"
                "Accept: */*\r\n"
                "\r\n", Curl->file, rand (), rand (), Curl->host);

      send (fd, buf, strlen (buf), 0);
      break;
    }


  if ((fh = fopen (basename2 (Curl->file), "wb")))
    {
      while ((read (fd, &c, sizeof (char))))
        {
          fputc (c, fh);
#ifdef  DEBUG
          fprintf (stderr, "%c", c);
          fflush (stderr);
#endif
        }
 
      fclose (fh);
    }

  url_free (Curl);
  close (fd);

  return 0;
}


int
portscan (const char *url, int port_start, int port_end)
{
//  fopen3();
  return 0;
}


#ifndef USE_IPV6
int
tcp_open (char *address, int port)
{
  struct sockaddr_in stAddr;
  struct hostent *host;
  int sock;
  struct linger l;

  memset (&stAddr, 0, sizeof (stAddr));
  stAddr.sin_family = AF_INET;
  stAddr.sin_port = htons (port);

  if ((host = gethostbyname (address)) == NULL)
    return 0;

  stAddr.sin_addr = *((struct in_addr *) host->h_addr_list[0]);

  if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    return 0;

  l.l_onoff = 1;
  l.l_linger = 5;
  if (setsockopt (sock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof (l)) < 0)
    return 0;

  if (connect (sock, (struct sockaddr *) &stAddr, sizeof (stAddr)) < 0)
    return 0;

  return sock;
}
#else
int
tcp_open (char *address, int port)
{
  struct addrinfo hints, *res, *res_tmp;
  int sock;
  struct linger l;
  char ipstring[MAX_HOSTNAME];
  char portstring[MAX_HOSTNAME];

  sprintf (portstring, "%d", port);

  memset (&hints, 0, sizeof (hints));
  /*
   * hints.ai_protocol  = 0;
   * hints.ai_addrlen   = 0;
   * hints.ai_canonname = NULL;
   * hints.ai_addr      = NULL;
   * hints.ai_next      = NULL;
   */
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;



  if (getaddrinfo (address, portstring, &hints, &res) != 0)
    return 0;

  for (res_tmp = res; res_tmp != NULL; res_tmp = res_tmp->ai_next)
    {
      if ((res_tmp->ai_family != AF_INET) && (res_tmp->ai_family != AF_INET6))
        continue;
      if ((sock = socket (res_tmp->ai_family, res_tmp->ai_socktype,
                          res_tmp->ai_protocol)) == -1)
        {
          sock = 0;
          continue;
        }

      l.l_onoff = 1;
      l.l_linger = 5;
      if (setsockopt (sock, SOL_SOCKET, SO_LINGER, (char *) &l, sizeof (l)) <
          0)
        {
          /* return 0; */
          sock = 0;
          continue;
        }

      if (connect (sock, res_tmp->ai_addr, res_tmp->ai_addrlen) == -1)
        {
          close (sock);
          sock = 0;
          continue;
        }

      ipstring[0] = 0;
      getnameinfo (res_tmp->ai_addr, res_tmp->ai_addrlen, ipstring,
                   sizeof (ipstring), NULL, 0, NI_NUMERICHOST);

      if (ipstring == NULL)
        sock = 0;

      break;
    }

  freeaddrinfo (res);
  return sock;
}
#endif


#if 0
#ifndef USE_IPV6
int
udp_open (char *address, int port)
{
  int enable = 1L;
  struct sockaddr_in stAddr;
  struct sockaddr_in stLclAddr;
  struct ip_mreq stMreq;
  struct hostent *host;
  int sock;

  stAddr.sin_family = AF_INET;
  stAddr.sin_port = htons (port);

  if ((host = gethostbyname (address)) == NULL)
    return 0;

  stAddr.sin_addr = *((struct in_addr *) host->h_addr_list[0]);

  /* Create a UDP socket */
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    return 0;

  /* Allow multiple instance of the client to share the same address and port */
  if (setsockopt
      (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &enable,
       sizeof (unsigned long int)) < 0)
    return 0;

  /* If the address is multicast, register to the multicast group */
#define IS_MULTICAST(a) ((a & 255) >= 224 && (a & 255) <= 239)
  if (IS_MULTICAST (stAddr.sin_addr.s_addr))
    {
      /* Bind the socket to port */
      stLclAddr.sin_family = AF_INET;
      stLclAddr.sin_addr.s_addr = htonl (INADDR_ANY);
      stLclAddr.sin_port = stAddr.sin_port;
      if (bind (sock, (struct sockaddr *) &stLclAddr, sizeof (stLclAddr)) < 0)
        return 0;

      /* Register to a multicast address */
      stMreq.imr_multiaddr.s_addr = stAddr.sin_addr.s_addr;
      stMreq.imr_interface.s_addr = INADDR_ANY;
      if (setsockopt
          (sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &stMreq,
           sizeof (stMreq)) < 0)
        return 0;
    }
  else
    {
      /* Bind the socket to port */
      stLclAddr.sin_family = AF_INET;
      stLclAddr.sin_addr.s_addr = htonl (INADDR_ANY);
      stLclAddr.sin_port = htons (0);
      if (bind (sock, (struct sockaddr *) &stLclAddr, sizeof (stLclAddr)) < 0)
        return 0;
    }

  return sock;
}
#else
int
udp_open (char *address, int port)
{
  int enable = 1L;
  struct addrinfo hints, *res, *res_tmp;
  int sock;
  struct linger l;
  char ipstring[MAX_HOSTNAME];
  char portstring[MAX_HOSTNAME];
  struct ipv6_mreq imr6;
  struct ip_mreq imr;

  sprintf (portstring, "%d", port);

  memset (&hints, 0, sizeof (hints));
  /*
   * hints.ai_protocol  = 0;
   * hints.ai_addrlen   = 0;
   * hints.ai_canonname = NULL;
   * hints.ai_addr      = NULL;
   * hints.ai_next      = NULL;
   */
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if (getaddrinfo (address, portstring, &hints, &res) != 0)
    return 0;

  for (res_tmp = res; res_tmp != NULL; res_tmp = res_tmp->ai_next)
    {
      if ((res_tmp->ai_family != AF_INET) && (res_tmp->ai_family != AF_INET6))
        continue;
      if ((sock = socket (res_tmp->ai_family, res_tmp->ai_socktype,
                          res_tmp->ai_protocol)) < 0)
        {
          sock = 0;
          continue;
        }
      /* Allow multiple instance of the client to share the same address and port */
      if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
                      (char *) &enable, sizeof (unsigned long int)) < 0)
        {
          sock = 0;
          continue;
        }

      /* If the address is multicast, register to the multicast group */
      if ((res_tmp->ai_family == AF_INET6) &&
          IN6_IS_ADDR_MULTICAST (&
                                 (((struct sockaddr_in6 *) res_tmp->ai_addr)->
                                  sin6_addr)))
        {
          /* Bind the socket to port */
          if (bind (sock, res_tmp->ai_addr, res_tmp->ai_addrlen) < 0)
            {
              close (sock);
              sock = 0;
              continue;
            }

          imr6.ipv6mr_multiaddr =
            ((struct sockaddr_in6 *) res_tmp->ai_addr)->sin6_addr;
          imr6.ipv6mr_interface = INADDR_ANY;
          if (setsockopt (sock,
                          IPPROTO_IPV6,
                          IPV6_ADD_MEMBERSHIP,
                          (char *) &imr6, sizeof (struct ipv6_mreq)) < 0)
            return 0;
        }
      else if ((res_tmp->ai_family == AF_INET) &&
               IN_MULTICAST (ntohl ((((struct sockaddr_in *) res_tmp->
                                      ai_addr)->sin_addr.s_addr))))
        {
          /* Bind the socket to port */
          if (bind (sock, res_tmp->ai_addr, res_tmp->ai_addrlen) < 0)
            {
              close (sock);
              sock = 0;
              continue;
            }

          imr.imr_multiaddr =
            ((struct sockaddr_in *) res_tmp->ai_addr)->sin_addr;
          imr.imr_interface.s_addr = INADDR_ANY;

          if (setsockopt (sock,
                          IPPROTO_IP,
                          IP_ADD_MEMBERSHIP,
                          (char *) &imr, sizeof (struct ip_mreq)) < 0)
            return 0;
        }
      else
        {
          /* Bind the socket to port */
          if (bind (sock, res_tmp->ai_addr, res_tmp->ai_addrlen) < 0)
            {
              close (sock);
              sock = 0;
              continue;
            }
        }
    }

  freeaddrinfo (res);
  return sock;
}
#endif
#endif
#endif // HAVE_SOCKET


URL_t *
url_new (const char *url)
{
  int pos, pos2;
  URL_t *Curl;
  char *p = NULL, *p2, *p3;
  char buf[MAXBUFSIZE];
  int x = 0, c = 0;

  if (!url)
    return NULL;

  // Create the URL container
  if (!(Curl = (URL_t *) malloc (sizeof (URL_t))))
    {
      fprintf (stderr, "ERROR: Memory allocation failed\n");
      return NULL;
    }

  // Initialisation of the URL container members
  memset (Curl, 0, sizeof (URL_t));

  // Copy the url in the URL container
  if (!(Curl->url = strdup (url)))
    {
      fprintf (stderr, "ERROR: Memory allocation failed\n");
      return NULL;
    }

  // extract the protocol
  p = strstr (url, "://");
  if (p == NULL)
    {
      fprintf (stderr, "ERROR: Not an URL\n");
      return NULL;
    }
  pos = p - url;
  Curl->protocol = (char *) malloc (pos + 1);
  strncpy (Curl->protocol, url, pos);
  if (Curl->protocol == NULL)
    {
      fprintf (stderr, "ERROR: Memory allocation failed\n");
      return NULL;
    }
  Curl->protocol[pos] = '\0';

  // jump the "://"
  p += 3;
  pos += 3;

  // check if a user:pass is given
  p2 = strstr (p, "@");
  if (p2 != NULL)
    {
      // We got something, at least a user...
      int len = p2 - p;
      Curl->user = (char *) malloc (len + 1);
      if (Curl->user == NULL)
	{
	  fprintf (stderr, "ERROR: Memory allocation failed\n");
	  return NULL;
	}
      strncpy (Curl->user, p, len);
      Curl->user[len] = '\0';

      p3 = strstr (p, ":");
      if (p3 != NULL && p3 < p2)
	{
	  // We also have a pass
	  int len2 = p2 - p3 - 1;
	  Curl->user[p3 - p] = '\0';
	  Curl->pass = (char *) malloc (len2 + 1);
	  if (Curl->pass == NULL)
	    {
	      fprintf (stderr, "ERROR: Memory allocation failed\n");
	      return NULL;
	    }
	  strncpy (Curl->pass, p3 + 1, len2);
	  Curl->pass[len2] = '\0';
	}
      p = p2 + 1;
      pos = p - url;
    }

  if (!Curl->user)
    {
      Curl->user = (char *) malloc (strlen (ANONYMOUS_S) + 2);
      strcpy (Curl->user, ANONYMOUS_S);

      Curl->pass = (char *) malloc (strlen (ANONYMOUS_S) + 2);
      strcpy (Curl->pass, ANONYMOUS_S);
    }

  // look if the port is given
  p2 = strstr (p, ":");
  // If the : is after the first / it isn't the port
  p3 = strstr (p, "/");
  if (p3 && p3 - p2 < 0)
    p2 = NULL;
  if (p2 == NULL)
    {
      // No port is given
      Curl->port = !strnicmp (Curl->protocol, "ftp", 3) ? 21 : 80;
      // Look if a path is given
      p2 = strstr (p, "/");
      if (p2 == NULL)
	{
	  // No path/filename
	  // So we have an URL like http://www.hostname.com
	  pos2 = strlen (url);
	}
      else
	{
	  // We have an URL like http://www.hostname.com/file.txt
	  pos2 = p2 - url;
	}
    }
  else
    {
      // We have an URL beginning like http://www.hostname.com:1212
      // Get the port number
      Curl->port = atoi (p2 + 1);
      pos2 = p2 - url;
    }
  // copy the hostname in the URL container
  Curl->host = (char *) malloc (pos2 - pos + 1);
  if (Curl->host == NULL)
    {
      fprintf (stderr, "ERROR: Memory allocation failed\n");
      return NULL;
    }
  strncpy (Curl->host, p, pos2 - pos);
  Curl->host[pos2 - pos] = '\0';

  // Look if a path is given
  p2 = strstr (p, "/");
  if (p2 != NULL)
    {
      // A path/filename is given
      // check if it's not a trailing '/'
      if (strlen (p2) > 1)
	{
	  // copy the path/filename in the URL container
	  Curl->file = strdup (p2);
	  if (Curl->file == NULL)
	    {
	      fprintf (stderr, "ERROR: Memory allocation failed\n");
	      return NULL;
	    }
	}
    }

  // Check if a filenme was given or set, else set it with '/'
  if (Curl->file == NULL)
    {
      Curl->file = (char *) malloc (2);
      if (Curl->file == NULL)
	{
	  fprintf (stderr, "ERROR: Memory allocation failed\n");
	  return NULL;
	}
      strcpy (Curl->file, "/");
    }
  else
    {
// turn query into a cmdline
      Curl->cmd = (char *) malloc (strlen (Curl->file) * 2);
      if (Curl->cmd == NULL)
	{
	  fprintf (stderr, "ERROR: Memory allocation failed\n");
	  return NULL;
	}

      for (x = 0; x < strlen (Curl->file); x++)
        {
          p = NULL;
          switch (Curl->file[x])
            {
              case '?':
              case '&':
              case '+':
                p = " ";
                break;

              case '%':
                sscanf (&Curl->file[x+1], "%02x", &c);
                sprintf (buf, "%c", c);
                x += 2;
                break;

              default:
                sprintf (buf, "%c", Curl->file[x]);
                break;
            }

            strcat (Curl->cmd, p?p:buf);
         }
    }

  if (Curl->cmd)
    {
//  turn cmdline into args
      strcpy (buf, Curl->cmd);

      while ((Curl->argv[Curl->argc] = strtok (!Curl->argc?buf:NULL, " ")) && Curl->argc < (ARGS_MAX - 1))
        Curl->argc++;
    }

#ifdef  DEBUG
  fprintf (stderr, "url:      %s\n", Curl->url);
  fprintf (stderr, "protocol: %s\n", Curl->protocol);
  fprintf (stderr, "hostname: %s\n", Curl->host);
  fprintf (stderr, "file:     %s\n", Curl->file);
  fprintf (stderr, "port:     %d\n", Curl->port);
  fprintf (stderr, "user:     %s\n", Curl->user);
  fprintf (stderr, "pass:     %s\n", Curl->pass);

  if (Curl->cmd)
    {
      fprintf (stderr, "cmd:      %s\n", Curl->cmd);
      for (x = 0; x < Curl->argc; x++) fprintf (stderr, "argv (%d): %s\n", x, Curl->argv[x]);
    }
#endif

  return Curl;
}


void
url_free (URL_t * url)
{
  if (!url)
    return;

  if (url->url)
    free (url->url);
  if (url->protocol)
    free (url->protocol);
  if (url->host)
    free (url->host);
  if (url->file)
    free (url->file);
  if (url->user)
    free (url->user);
  if (url->pass)
    free (url->pass);
  if (url->cmd)
    free (url->cmd);
  for (; url->argc; url->argc--)
    free (url->argv[url->argc - 1]);

  free (url);
}




#if 0
/* Replace escape sequences in an URL (or a part of an URL) */
/* works like strcpy(), but without return argument */
/* unescape_url_string comes from ASFRecorder */
void
url_unescape_string (char *outbuf, char *inbuf)
{
  unsigned char c;
  do
    {
      c = *inbuf++;
      if (c == '%')
	{
	  unsigned char c1 = *inbuf++;
	  unsigned char c2 = *inbuf++;
	  if (((c1 >= '0' && c1 <= '9') || (c1 >= 'A' && c1 <= 'F')) &&
	      ((c2 >= '0' && c2 <= '9') || (c2 >= 'A' && c2 <= 'F')))
	    {
	      if (c1 >= '0' && c1 <= '9')
		c1 -= '0';
	      else
		c1 -= 'A';
	      if (c2 >= '0' && c2 <= '9')
		c2 -= '0';
	      else
		c2 -= 'A';
	      c = (c1 << 4) + c2;
	    }
	}
      *outbuf++ = c;
    }
  while (c != '\0');
}


/* Replace specific characters in the URL string by an escape sequence */
/* works like strcpy(), but without return argument */
/* escape_url_string comes from ASFRecorder */
void
url_escape_string (char *outbuf, char *inbuf)
{
  unsigned char c;
  do
    {
      c = *inbuf++;
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 0x7f) ||	/* fareast languages(Chinese, Korean, Japanese) */
	  c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||	/* mark characters */
	  c == '*' || c == '\'' || c == '(' || c == ')' || c == '%' ||	/* do not touch escape character */
	  c == ';' || c == '/' || c == '?' || c == ':' || c == '@' ||	/* reserved characters */
	  c == '&' || c == '=' || c == '+' || c == '$' || c == ',' ||	/* see RFC 2396 */
	  c == '\0')
	{
	  *outbuf++ = c;
	}
      else
	{
	  /* all others will be escaped */
	  unsigned char c1 = ((c & 0xf0) >> 4);
	  unsigned char c2 = (c & 0x0f);
	  if (c1 < 10)
	    c1 += '0';
	  else
	    c1 += 'A';
	  if (c2 < 10)
	    c2 += '0';
	  else
	    c2 += 'A';
	  *outbuf++ = '%';
	  *outbuf++ = c1;
	  *outbuf++ = c2;
	}
    }
  while (c != '\0');
}


// broadcast your media files over the network
static u_int64_t
timenow (void)
{
  u_int64_t seconds, usec;
  struct timeval mtv;

  gettimeofday (&mtv, NULL);
  seconds = (u_int64_t) mtv.tv_sec;
  usec = (u_int64_t) mtv.tv_usec;

  return ((u_int64_t) (seconds * (u_int64_t) 1000000) + usec);

}


int
main (int argc, char *argv[])//udp_open (bla, "wb");
{
  // Variables used. Some obvious variables have not been explained.

  int sockfd;
  const int on = 1;		// option value for setsock
  struct sockaddr_in address;	// The addreess to store socket
  int result;			// Checks for broadcast errors
  u_int64_t starttimer;	// Timer values
  u_int64_t now;		// Timer values
  long sleeptime;		// Time to sleep
  char buf[1000];
  int buflen;
  int bytes_since_reset = 0;	// Number of bytes sent since last reset
  int byterate = 187500;	// Byterate
  int smoothness = 10;		// No of times rest is taken in a second.
  FILE *fp;

  // How to use this
  // TODO Add options for almost everything.

  if (argc != 3)
    {
      printf ("Usage: %s file port\n", argv[0]);
      return (1);
    }

  // Some CopyLeft Information 

  /* Initialise the socket.
     SOCK_DGRAM indicates a datagram protocol, or UDP.
     AF_INET indiacates it is a internet socket and not a file socket.
   */

  sockfd = socket (AF_INET, SOCK_DGRAM, 0);

  // Setup the broadcast address.
  // 255.255.255.255 is the universal broadcast address.

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr ("255.255.255.255");

  // convert the port number to network byte order.
  // TODO Put checks on the port number

  address.sin_port = htons (atoi (argv[2]));

  /*  Clarify that we really want to do a broadcast and that this isn't
     a mistake. The socket system makes this necessary just in case
     we're a program that takes an IP address at the command line and
     the user fooled us into making a broadcast.
   */

  setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on));

  /* Open the File to be broadcasted */

  fp = fopen (argv[1], "r");

  if (fp == NULL)
    {
      perror (argv[1]);
      exit (1);
    }

  // Start The timer so that we can keep the time difference.

  starttimer = timenow ();

  // The Magic loop

  while (!feof (fp))
    {

      buflen = fread (buf, 1, 1000, fp);
      bytes_since_reset += buflen;

      if (buflen <= 0)
	{
	  if (feof (fp))
	    {
	      printf ("This should not be so. Do report it to me\n");
	      exit (0);
	    }
	  else if (ferror (fp))
	    {
	      printf ("Error in reading from file %s\n", argv[1]);
	      exit (1);
	    }

	}

      // This is the whole magic.
      // We transmit some bytes and then wait to make sure we follow the
      // bitrate
      // TODO Check if the bitrate is followed precisely

      if (bytes_since_reset > byterate / smoothness)
	{
	  now = timenow ();
	  sleeptime = (long) (1000000 / smoothness - (now - starttimer));
	  if (sleeptime > 0)
	    {
	      // printf("sleeping for %ld microsec\n",sleeptime);
	      usleep (sleeptime);
	    }
	  else
	    // This should not be at least in general
	    // TODO bailout if this problem persists

	    fprintf (stderr, "Couldn't do it fast enough. Shit!\n");

	  // Reset the starting timer and bytes_since_reset

	  starttimer = timenow ();
	  bytes_since_reset = 0;
	}

      /* Broadcast now. 
       */
      result = sendto (sockfd, (void *) buf, buflen, 0,
		       (struct sockaddr *) &address, sizeof (address));

      /* Abort with an error message if the broadcast didn't work */

      if (result == -1)
	{
	  perror ("Broadcast Error");
	  exit (1);
	}
      if (result != buflen)
	{
	  printf ("Mismatch in read and send. Continuing anyway\n");
	}
    }
  return 0;
}

//client.c fopen3(..., "rb");
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define PORT        0x1234
             /* REPLACE with your server machine name */
#define HOST        "noisette2"
#define DIRSIZE     8192

main (argc, argv)
     int argc;
     char **argv;
{
  char hostname[100];
  char dir[DIRSIZE];
  int sd;
  struct sockaddr_in sin;
  struct sockaddr_in pin;
  struct hostent *hp;

  strcpy (hostname, HOST);
  if (argc > 2)
    {
      strcpy (hostname, argv[2]);
    }

  /* go find out about the desired host machine */
  if ((hp = gethostbyname (hostname)) == 0)
    {
      fprintf (stderr, "ERROR: gethostbyname");
      exit (1);
    }

  /* fill in the socket structure with host information */
  memset (&pin, 0, sizeof (pin));
  pin.sin_family = AF_INET;
  pin.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
  pin.sin_port = htons (PORT);

  /* grab an Internet domain socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      fprintf (stderr, "ERROR: socket");
      exit (1);
    }

  /* connect to PORT on HOST */
  if (connect (sd, (struct sockaddr *) &pin, sizeof (pin)) == -1)
    {
      fprintf (stderr, "ERROR: connect");
      exit (1);
    }

  /* send a message to the server PORT on machine HOST */
  if (send (sd, argv[1], strlen (argv[1]), 0) == -1)
    {
      fprintf (stderr, "ERROR: send");
      exit (1);
    }

  /* wait for a message to come back from the server */
  if (recv (sd, dir, DIRSIZE, 0) == -1)
    {
      fprintf (stderr, "ERROR: recv");
      exit (1);
    }

  /* spew-out the results and bail out of here! */
  printf ("%s\n", dir);

  close (sd);
}
//server.c --> fopen3(..., "wb")
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define PORT 		0x1234
#define DIRSIZE 	8192

main ()
{
  char dir[DIRSIZE];            /* used for incomming dir name, and
                                   outgoing data */
  int sd, sd_current, cc, fromlen, tolen;
  int addrlen;
  struct sockaddr_in sin;
  struct sockaddr_in pin;

  /* get an internet domain socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("socket");
      exit (1);
    }

  /* complete the socket structure */
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons (PORT);

  /* bind the socket to the port number */
  if (bind (sd, (struct sockaddr *) &sin, sizeof (sin)) == -1)
    {
      perror ("bind");
      exit (1);
    }

  /* show that we are willing to listen */
  if (listen (sd, 5) == -1)
    {
      perror ("listen");
      exit (1);
    }
  /* wait for a client to talk to us */
  addrlen = sizeof (pin);
  if ((sd_current = accept (sd, (struct sockaddr *) &pin, &addrlen)) == -1)
    {
      perror ("accept");
      exit (1);
    }

  /* get a message from the client */
  if (recv (sd_current, dir, sizeof (dir), 0) == -1)
    {
      perror ("recv");
      exit (1);
    }

  /* get the directory contents */
  read_dir (dir);

  /* strcat (dir," DUDE");
   */
  /* acknowledge the message, reply w/ the file names */
  if (send (sd_current, dir, strlen (dir), 0) == -1)
    {
      perror ("send");
      exit (1);
    }

  /* close up both sockets */
  close (sd_current);
  close (sd);

  /* give client a chance to properly shutdown */
  sleep (1);
}
#endif
