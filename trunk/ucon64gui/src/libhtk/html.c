/*
html.c - HTML support for libhtk

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


typedef struct
{
  GtkWidget *widget;

  int active;

  int type;                     //button, checkbox, etc.

  const char *name;
  const char *value;

//  int size;                     //could be width, cols or size
//  int size2;                    //could be height, rows or maxsize

  const char *img_src;          //load and use image from src if possible
  const char *tooltip;

  const char *target;           //target specified in the form tag (the backend)
}
htk_widget_t;


#define MAXWIDGETS 4096
static htk_widget_t widgets[MAXWIDGETS];
static htk_widget_t window, filedialog, vbox, hbox;
static int count = 0;
static const char *current_target;


htk_widget_t *
new_widget (void)
{
  if (count + 1 == MAXWIDGETS)
    {
      fprintf (stderr, "ERROR: maximum widgets (%d) reached", MAXWIDGETS);
      return NULL;
    }
  count++;
  memset (&widgets[count], 0, sizeof (htk_widget_t));

  widgets[count].target = current_target;       /* set target (backend) */

  return &widgets[count];
}


void
create (htk_widget_t * p)
{
  if (p->tooltip)
    {
      GtkTooltips *tooltips = gtk_tooltips_new ();
      gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), p->widget, p->tooltip,
                            p->tooltip);
    }
  gtk_box_pack_start (GTK_BOX (hbox.widget), p->widget, FALSE, FALSE, FALSE);
  gtk_widget_show (p->widget);

  p->active = TRUE;
}


void
destroy (htk_widget_t * p)
{
  gtk_widget_destroy (GTK_WIDGET (p->widget));
  p->active = FALSE;
}


void
event (htk_widget_t * p)
{
  char buf[MAXBUFSIZE];
  char target[MAXBUFSIZE];
  char name[MAXBUFSIZE];
  char value[MAXBUFSIZE];

/*
  rescue the name and the value bfore the widget gets destroyed
*/
  strcpy (target, NULL_TO_EMPTY (p->target));
  strcpy (name, NULL_TO_EMPTY (p->name));
  strcpy (value, NULL_TO_EMPTY (p->value));

  switch (p->type)
    {
    case WIDGET_INPUT_SUBMIT:
      break;

    case WIDGET_INPUT_TEXT:
      p->value = gtk_entry_get_text (GTK_ENTRY (p->widget));
      strcpy (value, p->value);
      break;

    case WIDGET_INPUT_FILE:
      p->value = gtk_entry_get_text (GTK_ENTRY (p->widget));
      strcpy (value, p->value);
      break;

    case WIDGET_INPUT_FILEDIALOG:
      p->value =
        gtk_file_selection_get_filename (GTK_FILE_SELECTION (p->widget));
      strcpy (value, p->value);
      destroy (p);
      break;

    case WIDGET_BUTTON:
    case WIDGET_INPUT_IMAGE:
    case WIDGET_INPUT_RESET:
    case WIDGET_INPUT_CHECKBOX:
    case WIDGET_INPUT_RADIO:
    case WIDGET_TEXTAREA:
    case WIDGET_SELECT:
      break;

    case WIDGET_HTML:
    case WIDGET_HTML_END:
    case WIDGET_TITLE:
    case WIDGET_BODY:
    case WIDGET_BODY_END:
    case WIDGET_HEAD:
    case WIDGET_HEAD_END:
    case WIDGET_FORM:
    case WIDGET_FORM_END:
    case WIDGET_IMG:
    case WIDGET_PRINTF:
    case WIDGET_HR:
    case WIDGET_BR:
    default:
      break;
    }

  printf ("url: %s?%s=%s\n", target, name, value);
  sprintf (buf, "%s=%s", name, value);

  htk_request (target, buf);       /* target == uri */
}


// ***************************************************************************************


void
htk_html (int width, int height, int border)
{
  fprintf (html_fp, "<html>");
}


void
htk_title (const char *title, char *img_src)
{
  if (img_src)
    fprintf (html_fp, "<link rel=\"icon\" href=\"%s\" type=\"image/png\">", img_src);

  fprintf (html_fp, "<title>%s</title>", title);
}


void
htk_br (void)
{
  fprintf (html_fp, "<br>");
}


void
htk_hr (void)
{
  fprintf (html_fp, "<hr>");
}


void
htk_button (const char *name, const char *value,
            char **img_src, int width, int height, const char *alt)
{
  fprintf (html_fp, "<button type=\"%s\" name=\"%s\" value=\"%s\" img=\"%s\""
    " width=\"%s\" height=\"%s\" alt=\"%s\">", "", "", "", "", "", "", "");
}


void
htk_img (char **img_src, int width, int height, int border, const char *alt)
{
}


void
htk_ (const char *text)
{
  fprintf (html_fp, text);
}


void
htk_input_text (const char *name, const char *value, int size, int maxsize,
                int readonly, const char *alt)
{
}


void
htk_input_file (const char *name, const char *value, int size, int maxsize,
                char **img_src, int width, int height, const char *alt)
{
}


void
htk_input_checkbox (const char *name, const char *value, int checked,
                    const char *alt)
{
}


void
htk_input_radio (const char *name, const char *value, int checked,
                 const char *alt)
{
}


void
htk_textarea (const char *name, const char *value, int cols, int rows,
              int readonly, int wordwrap, const char *alt)
{
}


void
htk_select (const char *name, int selected, int size, const char *alt, ...)
{
}


void
htk_html_end (void)
{
}


void
htk_body (char **img_src, const char *bgcolor)
{
}


void
htk_body_end (void)
{
}


void
htk_head (void)
{
}


void
htk_head_end (void)
{
}


void
htk_form (const char *target)
{
  current_target = target;
}


void
htk_form_end (void)
{
}
