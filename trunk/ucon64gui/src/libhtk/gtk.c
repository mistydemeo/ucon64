/*
gtk.c - GTK support for libhtk

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
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "trans.xpm"


#ifdef img_src
#undef img_src
#endif


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


// **********************************************************************************

void
file_dialog (htk_widget_t * p)
{
  filedialog.type = WIDGET_INPUT_FILEDIALOG;
  filedialog.value = p->value;
  filedialog.name = p->name;
  filedialog.target = p->target;

  if (filedialog.active == FALSE)
    {
      filedialog.widget = gtk_file_selection_new (NULL_TO_EMPTY (p->tooltip));
      filedialog.active = TRUE;
    }

  if (p->value)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (filedialog.widget),
                                     p->value);

  gtk_signal_connect_object (GTK_OBJECT
                             (GTK_FILE_SELECTION (filedialog.widget)->
                              ok_button), "clicked", GTK_SIGNAL_FUNC (event),
                             (gpointer) & filedialog);


  gtk_signal_connect_object (GTK_OBJECT
                             (GTK_FILE_SELECTION (filedialog.widget)->
                              cancel_button), "clicked",
                             GTK_SIGNAL_FUNC (destroy),
                             (gpointer) & filedialog);

  gtk_widget_show (filedialog.widget);
}


void
gtk_add_img (GtkWidget * p, char **img_src)
{
  GdkBitmap *mask;
  GtkStyle *style = gtk_widget_get_style (window.widget);
  GdkPixmap *pixmap =
    gdk_pixmap_create_from_xpm_d (window.widget->window, &mask,
                                  &style->bg[GTK_STATE_NORMAL],
                                  (gchar **) img_src);
  GtkWidget *pixmapwid = gtk_pixmap_new (pixmap, mask);

  gtk_container_add (GTK_CONTAINER (p), pixmapwid);
  gtk_widget_show (pixmapwid);
}


int
delete_event (void)
{
  return (FALSE);
}


void
menuitem_response (const char *string)
{
  printf ("%s\n", string);
}


char **
load_xpm (const char *img_src, char **dest)
{
#if 0
/* XPM */
  static char *trans_xpm[] = {
    "1 1 1 1",
    " 	c None",
    " "
  };
#endif
  return NULL;
}


// ***************************************************************************************


void
htk_start (int argc, char **argv)
{
  gtk_init (&argc, &argv);
  window.widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_signal_connect_object (GTK_OBJECT (window.widget), "delete_event",
                             GTK_SIGNAL_FUNC (delete_event), NULL);

  gtk_signal_connect_object (GTK_OBJECT (window.widget), "destroy",
                             GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
}


void
htk_end (void)
{
  gtk_main ();
}


void
htk_html (int width, int height, int border)
{
  if (window.active == TRUE)
    {
      gtk_widget_destroy (vbox.widget);
      count = 0;
    }

  window.active = TRUE;

  gtk_container_set_border_width (GTK_CONTAINER (window.widget),
                                  border < 0 ? 0 : border);

//TODO
//  gtk_widget_set_usize ( GTK_CONTAINER (window.widget), 
//                             width > 0 ? width : 0, height > 0 ? height : 0);


  gtk_window_set_policy (GTK_WINDOW (window.widget), FALSE, FALSE, TRUE);

  gtk_window_set_position (GTK_WINDOW (window.widget),  /*GTK_WIN_POS_CENTER */
                           GTK_WIN_POS_NONE /* GTK_WIN_POS_MOUSE */ );

  vbox.widget = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window.widget), vbox.widget);
  hbox.widget = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (vbox.widget), hbox.widget);

  gtk_widget_show (hbox.widget);
  gtk_widget_show (window.widget);
}


void
htk_title (const char *title, char **img_src)
{
  if (title)
    gtk_window_set_title (GTK_WINDOW (window.widget), title);

  if (img_src)
    {
      GtkStyle *style = gtk_widget_get_style (window.widget);
      GdkBitmap *mask;
      GdkPixmap *pixmap =
        gdk_pixmap_create_from_xpm_d (window.widget->window, &mask,
                                      &style->bg[GTK_STATE_NORMAL], img_src);

      gdk_window_set_icon (window.widget->window, (GdkWindow *) NULL,
                           pixmap, mask);
    }
}


void
htk_br (void)
{
  htk_widget_t *p = new_widget ();

  p->widget = gtk_hbox_new (FALSE, 0);
  hbox.widget = p->widget;

  gtk_container_add (GTK_CONTAINER (vbox.widget), hbox.widget);
  gtk_widget_show (hbox.widget);
}


void
htk_hr (void)
{
  htk_widget_t *p = new_widget ();
  p->widget = gtk_hseparator_new ();

  gtk_container_add (GTK_CONTAINER (hbox.widget), p->widget);

  gtk_widget_show (p->widget);
  htk_br ();
}


void
htk_button (const char *name, const char *value,
            char **img_src, int width, int height, const char *alt)
{
  htk_widget_t *p = new_widget ();

  p->widget = img_src ? gtk_button_new () : gtk_button_new_with_label (name);
  p->name = name;
  p->value = value;
  p->type = WIDGET_INPUT_SUBMIT;
  p->tooltip = alt;

  gtk_signal_connect_object (GTK_OBJECT (p->widget), "clicked",
                             GTK_SIGNAL_FUNC (event), (gpointer) p);

  gtk_widget_set_usize (GTK_WIDGET (p->widget),
                        (width > 0) ? width : 0, (height > 0) ? height : 0);


  if (img_src)
    gtk_add_img (p->widget, img_src);

  create (p);
}


void
htk_img (char **img_src, int width, int height, int border, const char *alt)
{
  GdkBitmap *mask;
  GtkStyle *style = gtk_widget_get_style (window.widget);
  GdkPixmap *pixmap =
    gdk_pixmap_create_from_xpm_d (window.widget->window, &mask,
                                  &style->bg[GTK_STATE_NORMAL],
                                  (gchar **) img_src ? img_src : trans_xpm);

  htk_widget_t *p = new_widget ();

  p->widget = gtk_pixmap_new (pixmap, mask);

//TODO fix this
  gtk_widget_set_usize (GTK_WIDGET (p->widget),
                        width > 0 ? width : 0, height > 0 ? height : 0);

  create (p);

}


void
htk_ (const char *text)
{
  htk_widget_t *p = new_widget ();

  p->widget = gtk_label_new (NULL_TO_EMPTY (text));

  create (p);
}


void
htk_input_text (const char *name, const char *value, int size, int maxsize,
                int readonly, const char *alt)
{
  htk_widget_t *p = new_widget ();

  p->widget = gtk_entry_new ();
  p->name = name;
  p->value = value;
  p->type = WIDGET_INPUT_TEXT;
  p->tooltip = alt;

  if (value)
    gtk_entry_set_text (GTK_ENTRY (p->widget), value);

  gtk_signal_connect_object (GTK_OBJECT (p->widget), "activate",
                             GTK_SIGNAL_FUNC (event), (gpointer) p);

  gtk_widget_set_usize (GTK_WIDGET (p->widget),
                        (size >
                         0 ? (gdk_char_width (p->widget->style->font, '0') *
                              size + 8) : 0), 0);

  gtk_entry_set_editable (GTK_ENTRY (p->widget), !readonly);

  if (maxsize > 0)
    gtk_entry_set_max_length (GTK_ENTRY (p->widget), maxsize);

  create (p);

}


void
htk_input_file (const char *name, const char *value, int size, int maxsize,
                char **img_src, int width, int height, const char *alt)
{

  htk_widget_t *p = new_widget ();

  p->widget = gtk_button_new ();//value ? gtk_button_new () : gtk_button_new_with_label (value);
  p->name = name;
  p->value = value;
  p->type = WIDGET_INPUT_FILE;
  p->tooltip = alt;

  htk_input_text (name, value, size, maxsize, 0, alt);

  gtk_signal_connect_object (GTK_OBJECT (p->widget), "clicked",
                             GTK_SIGNAL_FUNC (file_dialog), (gpointer) p);

  if (img_src)
    gtk_add_img (p->widget, img_src);

  create (p);
}


void
htk_input_checkbox (const char *name, const char *value, int checked,
                    const char *alt)
{
  htk_widget_t *p = new_widget ();

  p->widget = gtk_check_button_new ();
  p->name = name;
  p->value = value;
  p->type = WIDGET_INPUT_CHECKBOX;
  p->tooltip = alt;

  gtk_signal_connect_object (GTK_OBJECT (p->widget), "clicked",
                             GTK_SIGNAL_FUNC (event), (gpointer) p);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p->widget), checked);

  create (p);

}


void
htk_input_radio (const char *name, const char *value, int checked,
                 const char *alt)
{
  htk_widget_t *p = new_widget ();

  p->widget = gtk_radio_button_new (NULL);
  p->name = name;
  p->value = value;
  p->type = WIDGET_INPUT_RADIO;
  p->tooltip = alt;

  gtk_signal_connect_object (GTK_OBJECT (p->widget), "clicked",
                             GTK_SIGNAL_FUNC (event), (gpointer) p);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p->widget), checked);

  create (p);
}


void
htk_textarea (const char *name, const char *value, int cols, int rows,
              int readonly, int wordwrap, const char *alt)
{
  htk_widget_t *p = new_widget ();

  GdkFont *fixed_font = gdk_font_load ("-misc-fixed-plain-r-*-*-*-140-*-*-*-*-*-*");

  p->widget = gtk_text_new (NULL, NULL);
  p->name = name;
  p->value = value;
  p->type = WIDGET_TEXTAREA;
  p->tooltip = alt;

  gtk_text_set_editable (GTK_TEXT (p->widget), !readonly);
  gtk_text_set_word_wrap (GTK_TEXT (p->widget), wordwrap);
  gtk_text_set_line_wrap (GTK_TEXT (p->widget), TRUE);

#define FONT_HEIGHT(f)              ((f)->ascent + (f)->descent)

  gtk_widget_set_usize (GTK_WIDGET (p->widget),
    gdk_char_width (p->widget->style->font, '0') * cols + 8,
    FONT_HEIGHT (p->widget->style->font) * rows + 4);

  gtk_text_insert (GTK_TEXT (p->widget), fixed_font, NULL, NULL, value, -1);

  create (p);
}


void
htk_select (const char *name, int selected, int size, const char *alt, ...)
{
  va_list ap;
  const char *sp = NULL;

  htk_widget_t *p = new_widget ();
  htk_widget_t *p2 = new_widget ();
  htk_widget_t *p3 = new_widget ();

  p2->widget = gtk_menu_new ();
  p->widget = gtk_option_menu_new ();
  p->tooltip = alt;

  va_start (ap, alt);
  while ((sp = va_arg (ap, char *)))
    {
      if (!p3->name)
        {
          p3->widget = gtk_menu_item_new_with_label (sp);
          p3->name = sp;
          p3->type = WIDGET_SELECT;

          gtk_signal_connect_object (GTK_OBJECT (p3->widget),
                                     "activate",
                                     GTK_SIGNAL_FUNC (event), (gpointer) p3);

          gtk_menu_append (GTK_MENU (p2->widget), p3->widget);
          gtk_widget_show (p3->widget);
        }
      else
        {
          p3->value = sp;
          p3 = new_widget ();
        }


    }
  va_end (ap);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (p->widget), p2->widget);

  create (p);
}


void
htk_html_end (void)
{
  gtk_widget_show (vbox.widget);
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

#define img_src *img_src
