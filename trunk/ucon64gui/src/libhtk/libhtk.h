/*
libhtk.h - a HTML like Tool Kit for GUI's

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
#ifndef LIBHTK_H
#define LIBHTK_H
#include "libhtk_cfg.h"

#define stricmp strcasecmp
#define strnicmp strncasecmp

#define NULL_TO_EMPTY(str) ((str) ? (str) : (""))

struct htk_option
{
#if defined (__STDC__) && __STDC__
  const char *name;
#else
  char *name;
#endif
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
};

extern const char *optindex;


/* 
  htk_request()   waits for next query (event)
  htk_parser()    parses html code into a curses or GTK gui
  htk_getopt_long_only()    getopt_long_only() clone for libhtk
*/
extern void htk_parser (const char *html);
extern int htk_getopt_long_only (const char *uri, const char *query,
                            const struct htk_option *long_options, 
                            int *option_index);
extern void htk_request (const char *uri, const char *query);



/*
  these functions will become obsolete if htk_parser() works
*/
extern void htk_start (int argc, char **argv);  /* start htk */
extern void htk_end (void);     /* end of htk */

extern void htk_html (int width, int height, int border);       /* <html> */
extern void htk_html_end (void);        /* </html> */

extern void htk_title (const char *title, 
                       char *img_src);      /* <title>...</title> */

extern void htk_body (char *img_src, const char *bgcolor);     /* <body ...> */

extern void htk_body_end (void);        /* </body> */

extern void htk_head (void);    /* <head> */
extern void htk_head_end (void);        /* </head> */

extern void htk_form (const char *target);    /* <form ...> */
extern void htk_form_end (void);        /* </form> */

extern void htk_img (char *img_src, int width, int height, int border, 
                     const char *alt);       /* <img ...> */

extern void htk_ (const char *text);  /* plain text */

extern void htk_hr (void);      /* <hr> */
extern void htk_br (void);      /* <br> */

extern void htk_button (const char *name, const char *value,
                        char *img_src, int width, int height, const char *alt);

/*
  v,n,a == value, name, alt
*/
#define htk_input_submit(v,n,a) htk_button(v,n,NULL,0,0,a)
#define htk_input_reset htk_input_submit
#define htk_input_image htk_button
#define htk_a htk_input_submit

extern void htk_input_checkbox (const char *name, const char *value, int checked,
                                const char *alt);

extern void htk_input_radio (const char *name, const char *value, int checked, const char *alt);

extern void htk_textarea (const char *name, const char *value, int cols, int rows,
                          int readonly, int wordwrap, const char *alt);

extern void htk_input_text (const char *name, const char *value, int size, int maxsize,
                            int readonly, const char *alt);

extern void htk_input_file (const char *name, const char *value, int size, int maxsize,
                            char *img_src, int width, int height, const char *alt);

extern void htk_select (const char *name, int selected, int size, const char *alt,
                         ...);   /* <select> */

#endif // LIBHTK_H
