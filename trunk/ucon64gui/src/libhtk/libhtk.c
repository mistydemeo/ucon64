/*
libhtk.c - a HTML like Tool Kit for GUI's

written by 2002 Dirk Reinelt (d_i_r_k_@gmx.net)


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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "libhtk_cfg.h"
#include "libhtk.h"
#include "libhtk_defines.h"

#if 0
int htk_argc;
char *htk_argv[MAX_ARGS];
#endif
const char *optindex;

#if 0
//TODO
typedef struct
  {
    int num;
    char *name;
  } func_t;
                

static const char *
get_value (int argc, const char **argv, const char *name)
{
  int x = 0;
  int len = strlen (name);

  for (; (strnicmp (name, argv[x], len) != 0) && x < argc; x++);

  if (strnicmp (name, argv[x], len) != 0)
    return "";

  printf ("%s\n", argv[x]);
  fflush (stdout);
//TODO
  return "";
}


static void
htk_render (int argc, const char **argv)
//function that renders the widgets according to the args from htk_parser()
{
  int x = 0;
#define LIBHTK_FUNCS 16
  const func_t htk_func[LIBHTK_FUNCS] = {
      {WIDGET_HTML,     "html"},
      {WIDGET_HTML_END, "/html"},
      {WIDGET_TITLE,    "title"},
      {WIDGET_BODY,     "body"},
      {WIDGET_BODY_END, "/body"},
      {WIDGET_HEAD,     "head"},
      {WIDGET_HEAD_END, "/head"},
      {WIDGET_FORM,     "form"},
      {WIDGET_FORM_END, "/form"},
      {WIDGET_IMG,      "img"},
      {WIDGET_HR,       "hr"},
      {WIDGET_BR,       "br"},
      {WIDGET_BUTTON,   "button"},
      {WIDGET_INPUT,    "input"}, 
      {WIDGET_TEXTAREA, "textarea"},
      {WIDGET_SELECT,   "select"}
    };


#define VALUE_S(x) ((get_value (argc, argv, (x)))? \
                        (get_value (argc, argv, (x))):(""))
#if 1
#define VALUE(x) ((get_value (argc, argv, (x)))? \
                        (strtol (get_value (argc, argv, (x)), NULL, 10)):0)
#else
#define VALUE(x) (strtol ((VALUE_S (x)), NULL, 10))
#endif

  for (; x < LIBHTK_FUNCS; x++)
    if (!stricmp (argv[0], htk_func[x].name))
      {
        switch (htk_func[x].num)
          {
            case WIDGET_HTML:
              htk_html (VALUE ("width"), VALUE ("height"), VALUE ("border"));
              break;
            case WIDGET_HTML_END:
              htk_html_end ();
              break;
            case WIDGET_TITLE:
              htk_title (VALUE_S ("title"), VALUE_S ("src"));
              break;
            case WIDGET_BODY:
              htk_body (VALUE_S ("src"), VALUE_S ("bgcolor"));
              break;
            case WIDGET_BODY_END:
              htk_body_end ();
              break;
            case WIDGET_HEAD:
              htk_head ();
              break;
            case WIDGET_HEAD_END:
              htk_head_end ();
              break;
            case WIDGET_FORM:
              htk_form (VALUE ("target"));
              break;
            case WIDGET_FORM_END:
              htk_form_end ();
              break;
            case WIDGET_IMG:
              htk_img (VALUE_S ("src"), VALUE ("width"), VALUE ("height"),
                         VALUE ("border"), VALUE_S ("alt"));
              break;
            case WIDGET_HR:
              htk_hr ();
              break;
            case WIDGET_BR:
              htk_br ();
              break;
            case WIDGET_BUTTON:
              htk_button (VALUE_S ("name"), VALUE_S ("value"), VALUE_S ("src"),
                            VALUE ("width"), VALUE ("height"), VALUE_S ("alt"));
              break;
            case WIDGET_INPUT:
              break;
            case WIDGET_TEXTAREA:
              break;
            case WIDGET_SELECT:
              break;
            
            default:
              break;
          }
        break;
      }

  return;
}
#endif

int 
htk_getopt_long_only (const char *url,
                 const struct htk_option *long_options, int *option_index)
{
#ifdef USE_GETOPT
  return getopt_long_only (argc, argv, "", long_options, &option_index);
#else 
  const char *p = NULL;
  const char *query = url;
  char *p2 = NULL;
  int ind = 0; //&option_index;
  char buf[MAXBUFSIZE];

  p = optindex = strchr (query, '=');

  if (optind) optindex++;

  if (*(++p) != '-')
    p = query;
        
  while (*p == '-') p++;

  strcpy (buf, p);
  
  p2 = strchr (buf, '=');
  
  if (p2) *p2 = 0;
      
  for (; long_options[ind].name; ind++)
    if (!strnicmp (long_options[ind].name, buf, strlen (buf)))
      {
        return long_options[ind].val;
//arg
      }

  return -1;
#endif
}                 


#if 0
void
htk_parser (const char *html)
{
  for (; *html; html++)
    if (*html == '<')
      {
        int n = 0;
        char buf[MAXBUFSIZE];
        const char *args[ARGS_MAX]; // attributes

        n = strcspn (++html, ">\0"); // 0 ?
        strncpy (buf, html, n);
        buf[n] = 0;
        html += n;

        for (n = 0;
               (args[n] = strtok (!n?buf:NULL, " ")) && n < (ARGS_MAX - 1);
               n++);
        
        htk_render (n, args); // add widget
      }
    else // plain text
      {
        char buf[2];
        sprintf (buf, "%c", *html);
        htk_ (buf);
      }
  return;
}
#endif


#if     defined HAVE_GTK
#include "gtk.c"
#else
#include "html.c"
#endif
