/*
Copyright (C) 2003 by Sean David Fleming

sean@ivec.org

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

The GNU GPL can also be found at http://www.gnu.org
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gdis.h"
#include "file.h"
#include "dialog.h"
#include "interface.h"
#include "gui_image.h"
#include "import.h"
#include "project.h"

#include "go.xpm"
#include "pause.xpm"
#include "play.xpm"
#include "rewind.xpm"
#include "fastforward.xpm"
#include "stop.xpm"
#include "step_forward.xpm"
#include "step_backward.xpm"

extern struct sysenv_pak sysenv;

extern GtkWidget *window;

enum {AUTO_CHECK, AUTO_SPIN, AUTO_RANGE,
      AUTO_TEXT_ENTRY, AUTO_TEXT_LABEL,
      AUTO_INT_LABEL, AUTO_FLOAT_LABEL};

struct relation_pak 
{
/* true  - direct variable <-> widget relation */
/* false - relative offset (ie relation depends on active model) */
gboolean direct;

/* related object data */
struct model_pak *model;
gpointer variable;
gint offset;
GtkWidget *widget;
/* widget type (for updating) ie spin, check */
/* TODO - eliminate this using if GTK_IS_SPIN_BUTTON/CHECK_BUTTON macros */
gint type;
};

GSList *gui_relation_list=NULL;

#define DEBUG_RELATION 0

/**************************************/
/* auto update widget handling events */
/**************************************/
void gui_relation_destroy(GtkWidget *w, gpointer dummy)
{
GSList *list;
struct relation_pak *relation;

#if DEBUG_RELATION
printf("destroying widget (%p)", w);
#endif

/* find appropriate relation */
list = gui_relation_list;
while (list)
  {
  relation = list->data;
  list = g_slist_next(list);

/* free associated data */
  if (relation->widget == w)
    {
#if DEBUG_RELATION
printf(" : relation (%p)", relation);
#endif
    g_free(relation);
    gui_relation_list = g_slist_remove(gui_relation_list, relation);
    }
  }
#if DEBUG_RELATION
printf("\n");
#endif
}

/***************************************************/
/* set a value according to supplied widget status */
/***************************************************/
void gui_relation_set_value(GtkWidget *w, struct model_pak *model)
{
const gchar *text;
gpointer value;
GSList *list;
struct relation_pak *relation;

g_assert(w != NULL);

/* if no model supplied, use active model */
if (!model)
  model = tree_model_get();

/* FIXME - get rid of this type of object - too dangerous */
if (!model)
  return;

/* find appropriate relation (direct -> widget, else model) */
list = gui_relation_list;
while (list)
  {
  relation = list->data;
  list = g_slist_next(list);

  if (relation->widget != w)
    continue;

  if (relation->direct)
    value = relation->variable;
  else
    {
    g_assert(model != NULL);
    value = (gpointer) model + relation->offset;
    }

/* update variable associated with the widget */
  switch (relation->type)
    {
    case AUTO_CHECK:
#if DEBUG_RELATION
printf("model %p : relation %p : setting variable to %d\n", 
       model, relation, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
#endif
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
        *((gint *) value) = TRUE;
      else
        *((gint *) value) = FALSE;
      break;

    case AUTO_SPIN:
#if DEBUG_RELATION
printf("model %p : relation %p : setting variable to %f\n", 
       model, relation, SPIN_FVAL(GTK_SPIN_BUTTON(w)));
#endif
      *((gdouble *) value) = SPIN_FVAL(GTK_SPIN_BUTTON(w));
      break;

    case AUTO_RANGE:
      *((gint *) value) = gtk_range_get_value(GTK_RANGE(w));
      break;

    case AUTO_TEXT_ENTRY:
      g_free(*((gchar **) value));
/* NEW - NULLify text pointer if string is empty */
      text = gtk_entry_get_text(GTK_ENTRY(w));
      if (strlen(text))
        *((gchar **) value) = g_strdup(text);
      else
        *((gchar **) value) = NULL;
      break;
    }
  }
}

/***************************************************/
/* updates dependant widget with new variable data */
/***************************************************/
void gui_relation_update_widget(gpointer value)
{
gchar *text;
GSList *list;
struct relation_pak *relation;

/* loop over all existing relations */
for (list=gui_relation_list ; list ; list=g_slist_next(list))
  {
  relation = list->data;

  g_assert(relation != NULL);

  if (!relation->direct)
    continue;
  if (relation->variable != value)
    continue;

#if DEBUG_RELATION
printf("relation %p : type %d : updating widget\n", relation, relation->type);
#endif

/* synchronize widget and variable */
  switch (relation->type)
    {
    case AUTO_CHECK:
      if (*((gint *) value))
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(relation->widget),
                                     TRUE);
      else
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(relation->widget),
                                     FALSE);
      break;

    case AUTO_SPIN:
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(relation->widget),
                                *((gdouble *) value));
      break;

    case AUTO_RANGE:
      gtk_range_set_value(GTK_RANGE(relation->widget), *((gint *) value));
      break;

    case AUTO_TEXT_ENTRY:
/* CURRENT */
      text = *((gchar **) value);
      if (text)
        gtk_entry_set_text(GTK_ENTRY(relation->widget), text);
      break;

    case AUTO_TEXT_LABEL:
      gtk_label_set_text(GTK_LABEL(relation->widget), *((gchar **) value));
      break;

    case AUTO_INT_LABEL:
      text = g_strdup_printf("%d", *((gint *) value));
      gtk_label_set_text(GTK_LABEL(relation->widget), text);
      g_free(text);
      break;

    case AUTO_FLOAT_LABEL:
      text = g_strdup_printf("%.4f", *((gdouble *) value));
      gtk_label_set_text(GTK_LABEL(relation->widget), text);
      g_free(text);
      break;
    }
  }
}

/*****************************/
/* called for a model switch */
/*****************************/
/* ie sets all widget states according to related variable */
/* NB: pass NULL to get everything updated */
void gui_relation_update(gpointer data)
{
gchar *text;
GSList *list;
struct relation_pak *relation;
gpointer value;

/* loop over all existing relations */
for (list=gui_relation_list ; list ; list=g_slist_next(list))
  {
  relation = list->data;

  g_assert(relation != NULL);

/* update test */
  if (relation->direct)
    {
    if (data)
      if (data != relation->model)
        continue;
    value = relation->variable;
    }
  else
    {
    if (data)
      value = data + relation->offset;
    else
      continue;
    }

#if DEBUG_RELATION
printf("relation %p : type %d : model change\n", relation, relation->type);
#endif

/* synchronize widget and variable */
  switch (relation->type)
    {
    case AUTO_CHECK:
      if (*((gint *) value))
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(relation->widget),
                                     TRUE);
      else
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(relation->widget),
                                     FALSE);
      break;

    case AUTO_SPIN:
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(relation->widget),
                                *((gdouble *) value));
      break;

    case AUTO_RANGE:
      gtk_range_set_value(GTK_RANGE(relation->widget), *((gint *) value));
      break;

    case AUTO_TEXT_ENTRY:
/* CURRENT */
      text = *((gchar **) value);
      if (text)
        gtk_entry_set_text(GTK_ENTRY(relation->widget), text);
      break;

    case AUTO_TEXT_LABEL:
      gtk_label_set_text(GTK_LABEL(relation->widget), *((gchar **) value));
      break;

    case AUTO_INT_LABEL:
      text = g_strdup_printf("%d", *((gint *) value));
      gtk_label_set_text(GTK_LABEL(relation->widget), text);
      g_free(text);
      break;

    case AUTO_FLOAT_LABEL:
      text = g_strdup_printf("%.4f", *((gdouble *) value));
      gtk_label_set_text(GTK_LABEL(relation->widget), text);
      g_free(text);
      break;
    }
  }
}

/**************************************************/
/* create a new variable to widget correspondance */
/**************************************************/
void
gui_relation_submit(GtkWidget        *widget,
                      gint              type,
                      gpointer          value,
                      gboolean          direct,
                      struct model_pak *model)
{
struct relation_pak *relation;

/* alloc and init relation */
relation = g_malloc(sizeof(struct relation_pak));
relation->direct = direct;
relation->type = type;
if (model)
  relation->offset = value - (gpointer) model;
else
  relation->offset = 0;
relation->variable = value;
relation->model = model;
relation->widget = widget;

/* this can happen if gui_auto_check() is used on a variable that is not in a model */
/* eg in sysenv ... in this case use gui_direct_check() */
/* FIXME - a better way to fail gracefully? better still - rewrite old code */
if (!direct)
  g_assert(relation->offset > 0);

#if DEBUG_RELATION
printf("submit: ");
printf("[%p type %d]", relation, type);
if (model)
  {
  if (direct)
    printf("[model %d][value %p]", model->number, value);
  else
    printf("[model %d][value %p][offset %d]", model->number, value, relation->offset);
  }
printf("\n");
#endif

switch (relation->type)
  {
  case AUTO_CHECK:
    if (*((gint *) value))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    else
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
    break;

  case AUTO_SPIN:
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), *((gdouble *) value));
    break;

  case AUTO_RANGE:
    gtk_range_set_value(GTK_RANGE(widget), *((gint *) value));
    break;
  }

gui_relation_list = g_slist_prepend(gui_relation_list, relation);
}

/*****************************************/
/* convenience routine for check buttons */
/*****************************************/
GtkWidget *new_check_button(gchar *label, gpointer callback, gpointer arg,
                                            gint state, GtkWidget *box)
{
GtkWidget *button;

/* create, pack & show the button */
button = gtk_check_button_new_with_label(label);
gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);
gtk_widget_show(button);

/* set the state (NB: before the callback is attached) */
if (state)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);

/* attach the callback */
if (callback)
  g_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(callback), arg);
return(button);
}

/*********************************/
/* relative updated check button */
/*********************************/
/* NB: assumes gui_mode_switchl() will call gui_relation_update() */
GtkWidget *gui_auto_check(gchar *label, gpointer cb, gpointer arg,
                            gint *state, GtkWidget *box)
{
GtkWidget *button;
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);

/* create the button */
button = gtk_check_button_new_with_label(label);
gui_relation_submit(button, AUTO_CHECK, state, FALSE, model);
if (box)
  gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

/* callback to set the variable to match the widget */
g_signal_connect(GTK_OBJECT(button), "clicked",
                 GTK_SIGNAL_FUNC(gui_relation_set_value), NULL);

/* callback to do (user defined) update tasks */
if (cb)
  g_signal_connect_after(GTK_OBJECT(button), "clicked", cb, arg);

/* callback to remove the variable <-> widget relation */
g_signal_connect(GTK_OBJECT(button), "destroy",
                 GTK_SIGNAL_FUNC(gui_relation_destroy), NULL);

return(button);
}

/*************************************************/
/* activate/deactivate based on a checkbox state */
/*************************************************/
/* TODO - shortcut for setup up one of these widgets */
void gui_checkbox_refresh(GtkWidget *w, GtkWidget *box)
{
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
  gtk_widget_set_sensitive(box, TRUE);
else
  gtk_widget_set_sensitive(box, FALSE);
}


/***********************/
/* check button update */
/***********************/
gint gui_direct_check_update(GtkWidget *w, gpointer data)
{
gint *value;

if (data)
  {
  value = data;
  
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
    *value = 1;
  else
    *value = 0;
  }

return(TRUE);
}

/*********************************/
/* directly updated check button */
/*********************************/
/* NB: assumes gui_mode_switch() will call gui_relation_update() */
GtkWidget *gui_direct_check(gchar *label, gint *state,
                              gpointer cb, gpointer arg,
                              GtkWidget *box)
{
GtkWidget *button;

/* create the button */
button = gtk_check_button_new_with_label(label);

if (*state)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);

if (box)
  gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

g_signal_connect(GTK_OBJECT(button), "clicked",
                 GTK_SIGNAL_FUNC(gui_direct_check_update), state);

/* callback to do (user defined) update tasks */
if (cb)
  g_signal_connect_after(GTK_OBJECT(button), "clicked", cb, arg);

return(button);
}

/****************************/
/* a simple labelled button */
/****************************/
GtkWidget *gui_button(gchar *txt, gpointer cb, gpointer arg, GtkWidget *w, gint mask)
{
gint fill1, fill2;
GtkWidget *button;

/* create the button */
button = gtk_button_new_with_label(txt);

/* setup the packing requirements */
if (w)
  {
  fill1 = fill2 = FALSE;
  if (mask & 1)
    fill1 = TRUE;
  if (mask & 2)
    fill2 = TRUE;
  if (mask & 4)
    gtk_box_pack_end(GTK_BOX(w), button, fill1, fill2, 0);
  else
    gtk_box_pack_start(GTK_BOX(w), button, fill1, fill2, 0);
  }

/* attach the callback */
if (cb)
  g_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(cb), arg);

return(button);
}

/************************************/
/* text label plus an X push button */
/************************************/
GtkWidget *gui_button_x(gchar *text,
                          gpointer cb, gpointer arg,
                          GtkWidget *box)
{
GtkWidget *hbox, *button, *label, *image;
GdkPixmap *pixmap;
GdkBitmap *mask;
GtkStyle *style;

/* create button */
button = gtk_button_new();
hbox = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(button), hbox);

/* create image */
style = gtk_widget_get_style(window);
pixmap = gdk_pixmap_create_from_xpm_d
          (window->window, &mask, &style->white, go_xpm);
image = gtk_image_new_from_pixmap(pixmap, mask);

gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

/* create label */
if (box)
  {
/* packing sub-widget */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, 0);
  if (text)
    {
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }
  gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  }

if (cb)
  g_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(cb), arg);

return(button);
}

/******************************************/
/* create and pack a labelled push button */
/******************************************/
void gui_label_button(gchar *label, gpointer cb, gpointer arg, GtkWidget *box)
{
GtkWidget *hbox, *button;

g_assert(box != NULL);
g_assert(label != NULL);

/* packing sub-widget */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);

/* create button */
button = gtk_button_new_with_label(label);
gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
if (cb)
  g_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(cb), arg);
}

/*****************************************/
/* convenience routine for radio buttons */
/*****************************************/
gint active=0, count=0, fill1=FALSE, fill2=FALSE;
GSList *group=NULL;
GtkWidget *box=NULL;

void new_radio_group(gint i, GtkWidget *pack_box, gint mask)
{
group = NULL;
active = i;
count = 0;
box = pack_box;

/* setup the packing requirements */
fill1 = fill2 = FALSE;
if (mask & 1)
  fill1 = TRUE;
if (mask & 2)
  fill2 = TRUE;
}

GtkWidget *add_radio_button(gchar *label, gpointer call, gpointer arg)
{
GtkWidget *button;

/*
printf("Adding button: %d (%d) to (%p)\n", count, active, group);
*/

/* better error checking - even call new_radio_group() ??? */
g_return_val_if_fail(box != NULL, NULL);

/* make a new button */
button = gtk_radio_button_new_with_label(group, label);

/* get the group (for the next button) */
/* NB: it is vital that this is ALWAYS done */
group = gtk_radio_button_group(GTK_RADIO_BUTTON(button));

/* attach the callback */
if (call)
g_signal_connect_swapped(GTK_OBJECT(button), "pressed",
                         GTK_SIGNAL_FUNC(call), (gpointer) arg);

/* pack the button */
gtk_box_pack_start(GTK_BOX(box), button, fill1, fill2, 0);

/* set active? */
if (active == count++)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);

return(button);
}

/*******************/
/* radio button v2 */
/*******************/
struct radio_pak
{
GtkWidget *box;
GSList *list;
gint *value;
};

/*********************/
/* free radio object */
/*********************/
gint gui_radio_free(GtkWidget *w, gpointer data)
{
struct radio_pak *radio=data;
g_free(radio);
return(TRUE);
}

/***********************/
/* create radio object */
/***********************/
gpointer gui_radio_new(gint *value, GtkWidget *w)
{
struct radio_pak *radio;

radio = g_malloc(sizeof(struct radio_pak));
radio->box = w;
radio->list = NULL;
radio->value = value;

//printf("radio new: %p\n", radio);

g_signal_connect(GTK_OBJECT(w), "destroy", GTK_SIGNAL_FUNC(gui_radio_free), radio);

return(radio);
}

/********************************/
/* radio button signal callback */
/********************************/
gint gui_radio_callback(GtkWidget *w, gpointer data)
{
struct radio_pak *radio=data;

g_assert(radio != NULL);

*radio->value = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "value"));

return(TRUE);
}

/**************************/
/* add a new radio button */
/**************************/
void gui_radio_button_add(const gchar *text, gint value, gpointer data)
{
GtkWidget *button;
struct radio_pak *radio=data;

//printf("(*) %s %d %p\n", text, value, data);

g_assert(radio != NULL);

/* create new button */
button = gtk_radio_button_new_with_label(radio->list, text);

/* update list in radio object */
radio->list = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));

gtk_box_pack_start(GTK_BOX(radio->box), button, TRUE, TRUE, 0);

if (value == *radio->value)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);

/* attach the callback */
g_signal_connect(GTK_OBJECT(button), "pressed", GTK_SIGNAL_FUNC(gui_radio_callback), radio);
g_object_set_data(G_OBJECT(button), "value", GINT_TO_POINTER(value));
}

/************************************/
/* create a colour selection dialog */
/************************************/
gpointer gui_colour_selection_dialog(const gchar *title, gpointer callback)
{
GtkWidget *csd;
GtkButton *ok, *cancel;

/* create colour selection dialog */
csd = gtk_color_selection_dialog_new(title);
ok = GTK_BUTTON(GTK_COLOR_SELECTION_DIALOG(csd)->ok_button);
cancel = GTK_BUTTON(GTK_COLOR_SELECTION_DIALOG(csd)->cancel_button);

/* rename goofy GTK default labels */
gtk_button_set_label(ok, GTK_STOCK_APPLY);
gtk_button_set_use_stock(ok, TRUE);
gtk_button_set_label(cancel, GTK_STOCK_CLOSE);
gtk_button_set_use_stock(cancel, TRUE);

/* setup callbacks */
g_signal_connect(ok, "clicked", callback, GTK_COLOR_SELECTION_DIALOG(csd)->colorsel);
g_signal_connect(cancel, "clicked", GTK_SIGNAL_FUNC(destroy_widget), (gpointer) csd);

/* done */
gtk_widget_show(csd);

return(csd);
}

/******************************/
/* create a label + a spinner */
/******************************/
GtkWidget *new_spinner(gchar *text, gdouble min, gdouble max, gdouble step,
                       gpointer callback, gpointer arg, GtkWidget *box)
{
GtkWidget *hbox, *label, *spin;

spin = gtk_spin_button_new_with_range(min, max, step);

/* optional */
if (box)
  {
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, 0);
  if (text)
    {
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }
  gtk_box_pack_end(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
  }

if (callback)
  g_signal_connect_after(GTK_OBJECT(spin), "value-changed",
                         GTK_SIGNAL_FUNC(callback), arg);

return(spin);
}

/************************************************************/
/* as above, but automatically attaches spinner to callback */
/************************************************************/
GtkWidget *gui_new_spin(gchar *text, gdouble x0, gdouble x1, gdouble dx,
                          gpointer callback, GtkWidget *box)
{
GtkWidget *hbox, *label, *spin;

spin = gtk_spin_button_new_with_range(x0, x1, dx);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), x0);

/* optional */
if (box)
  {
/* create the text/spinner layout */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, 0);
  if (text)
    {
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }
  gtk_box_pack_end(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
  }
if (callback)
  g_signal_connect_after(GTK_OBJECT(spin), "value-changed",
                         GTK_SIGNAL_FUNC(callback), spin);
return(spin);
}

/*********************************/
/* automatically updated spinner */
/*********************************/
/* current model relative correlation between value and spinner */
/* NB: assumes gui_mode_switchl() will call gui_relation_update() */
GtkWidget *gui_auto_spin(gchar *text, gdouble *value,
                           gdouble min, gdouble max, gdouble step,
                           gpointer callback, gpointer arg, GtkWidget *box)
{
GtkWidget *spin;
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);

/* create the text/spinner combo */
spin = new_spinner(text, min, max, step, callback, arg, box);

/* set up a relationship */
gui_relation_submit(spin, AUTO_SPIN, value, FALSE, model);

/* callback to set the variable to match the widget */
g_signal_connect(GTK_OBJECT(spin), "value-changed",
                (gpointer) gui_relation_set_value, NULL);

/* callback to remove the variable <-> widget relation */
g_signal_connect(GTK_OBJECT(spin), "destroy",
                 GTK_SIGNAL_FUNC(gui_relation_destroy), NULL);

return(spin);
}

/*********************************/
/* automatically updated spinner */
/*********************************/
/* direct correlation between value and spinner */
GtkWidget *gui_direct_spin(gchar *text, gdouble *value,
                             gdouble min, gdouble max, gdouble step,
                             gpointer callback, gpointer arg, GtkWidget *box)
{
gint size=0;
gdouble digits;
GtkWidget *spin;
struct model_pak *model;

model = tree_model_get();

/* create the text/spinner combo */
spin = new_spinner(text, min, max, step, NULL, NULL, box);

/* HACK - cope with GTK underestimating the size needed to display spin values */
/* TODO - examine sig fig of *value to get dp */
digits = log10(step);
if (digits < 0.0)
  {
  digits -= 0.9;
  size = 3 + fabs(digits);
  }
if (size < 5)
  size = 5;

gtk_widget_set_size_request(spin, sysenv.gtk_fontsize*size, -1);

/*
printf("%f : %f -> %f (%d)\n", max, step, digits, size);
gtk_widget_set_size_request(spin, sysenv.gtk_fontsize*5, -1);
*/

/* set up a relationship */
gui_relation_submit(spin, AUTO_SPIN, value, TRUE, model);

/* callback to set the variable to match the widget */
g_signal_connect(GTK_OBJECT(spin), "value-changed",
                 GTK_SIGNAL_FUNC(gui_relation_set_value), model);

/* callback to remove the variable <-> widget relation */
g_signal_connect(GTK_OBJECT(spin), "destroy",
                 GTK_SIGNAL_FUNC(gui_relation_destroy), NULL);

/* connect after, so all updates done before user callback is invoked */
if (callback)
  g_signal_connect_after(GTK_OBJECT(spin), "value-changed",
                         GTK_SIGNAL_FUNC(callback), arg);

return(spin);
}

/**************************/
/* auto update slider bar */
/**************************/
GtkWidget *gui_direct_hscale(gdouble min, gdouble max, gdouble step,
                               gpointer value, gpointer func, gpointer arg,
                                                            GtkWidget *box)
{
GtkWidget *hscale;
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);

hscale = gtk_hscale_new_with_range(min, max, step);
gtk_range_set_update_policy(GTK_RANGE(hscale), GTK_UPDATE_CONTINUOUS);

if (box)
  gtk_box_pack_start(GTK_BOX(box), hscale, TRUE, TRUE, 0);

gui_relation_submit(hscale, AUTO_RANGE, value, TRUE, model);

/* callback to set the variable to match the widget */
g_signal_connect(GTK_OBJECT(hscale), "value_changed",
                 GTK_SIGNAL_FUNC(gui_relation_set_value), model);

/* user callback */
if (func)
  g_signal_connect_after(GTK_OBJECT(hscale), "value_changed",
                         GTK_SIGNAL_FUNC(func), arg);

/* callback to remove the variable <-> widget relation */
g_signal_connect(GTK_OBJECT(hscale), "destroy",
                GTK_SIGNAL_FUNC(gui_relation_destroy), NULL);

return(hscale);
}

/*****************************/
/* pulldown callback handler */
/*****************************/
gint gui_text_entry_changed(GtkWidget *w, gpointer data)
{
gchar *text, **value;

/* update internal variable (if non NULL) */
if (data)
  {
/* get current (newly allocated) text */
  text = g_strdup(gtk_entry_get_text(GTK_ENTRY(w)));

  value = data;
/* free old string */
  g_free(*value);
  if (!strlen(text))
    {
/* empty string - assign NULL to internal pointer */
    g_nullify_pointer(data);
    g_free(text);
    }
  else
    {
/* assign address of newly allocated string */
    memcpy(data, &text, sizeof(gchar **));
    }
  }

return(TRUE);
}

/**************************************************/
/* convenience function for labelled text entries */
/**************************************************/
GtkWidget *gui_text_entry(gchar *text, gchar **value,
                          gint edit, gint pack, GtkWidget *box)
{
GtkWidget *hbox, *label, *entry;

entry = gtk_entry_new();
gtk_entry_set_editable(GTK_ENTRY(entry), edit);

if (box)
  {
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  if (text)
    {
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }
  gtk_box_pack_end(GTK_BOX(hbox), entry, pack, TRUE, 0);
  }

if (value)
  {
  if (*value)
    gtk_entry_set_text(GTK_ENTRY(entry), *value);

  if (edit)
    g_signal_connect(GTK_OBJECT(entry), "changed",
                     GTK_SIGNAL_FUNC(gui_text_entry_changed), value);
  }

return(entry);
}

/**************************/
/* auto update text label */
/**************************/
GtkWidget *gui_auto_text_label(gchar **text)
{
GtkWidget *label;
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);
g_assert(text != NULL);

if (*text)
  label = gtk_label_new(*text);
else
  label = gtk_label_new("null");

gui_relation_submit(label, AUTO_TEXT_LABEL, text, FALSE, model);

/* callback to remove the variable <-> widget relation */
g_signal_connect(GTK_OBJECT(label), "destroy",
                 GTK_SIGNAL_FUNC(gui_relation_destroy), NULL);

return(label);
}

/**************************/
/* auto update text label */
/**************************/
GtkWidget *gui_auto_int_label(gint *value)
{
gchar *text;
GtkWidget *label;
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);

text = g_strdup_printf("%d", *value);
label = gtk_label_new(text);
g_free(text);

gui_relation_submit(label, AUTO_INT_LABEL, value, FALSE, model);

/* callback to remove the variable <-> widget relation */
g_signal_connect(GTK_OBJECT(label), "destroy",
                 GTK_SIGNAL_FUNC(gui_relation_destroy), NULL);

return(label);
}

/**************************/
/* auto update text label */
/**************************/
GtkWidget *gui_auto_float_label(gdouble *value)
{
gchar *text;
GtkWidget *label;
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);

text = g_strdup_printf("%.4f", *value);
label = gtk_label_new(text);
g_free(text);

gui_relation_submit(label, AUTO_FLOAT_LABEL, value, FALSE, model);

/* callback to remove the variable <-> widget relation */
g_signal_connect(GTK_OBJECT(label), "destroy",
                 GTK_SIGNAL_FUNC(gui_relation_destroy), NULL);

return(label);
}

/*****************************/
/* create a stock GTK button */
/*****************************/
GtkWidget * 
gui_stock_button(const gchar *id, gpointer cb, gpointer arg, GtkWidget *box)
{
GtkWidget *button;

button = gtk_button_new_from_stock(id);

if (box)
  gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

g_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(cb), arg);

return(button);
}

/************************************/
/* GDIS internal pixmap icon button */
/************************************/
GtkWidget *gui_image_button(gint id, gpointer cb, gpointer arg)
{
GtkWidget *hbox, *button, *image;
GdkPixbuf *pixbuf;

button = gtk_button_new();

hbox = gtk_hbox_new(FALSE, 4);
gtk_container_add(GTK_CONTAINER(button), hbox);

pixbuf = image_table_lookup(id);
if (pixbuf)
  {
  image = gtk_image_new_from_pixbuf(pixbuf);

  gtk_box_pack_start(GTK_BOX(hbox), image, TRUE, FALSE, 0);
  }
else
  printf("image not found\n");

if (cb)
  g_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(cb), arg);

return(button);
}

/***************************************************/
/* create a button with a stock icon & custom text */
/***************************************************/
GtkWidget * 
gui_icon_button(const gchar *id, const gchar *text,
                  gpointer cb, gpointer arg,
                  GtkWidget *box)
{
gint stock = TRUE;
GtkWidget *hbox, *button, *label, *image=NULL;
GdkBitmap *mask;
GdkPixmap *pixmap;
GtkStyle *style;

/* button */
button = gtk_button_new();

/* TODO - incorporate in image_table_lookup() */

/* GDIS icons */
if (g_ascii_strncasecmp(id, "GDIS_PAUSE", 10) == 0)
  {
  style = gtk_widget_get_style(window);

  pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask, &style->white,
                                        pause_xpm);
  image = gtk_image_new_from_pixmap(pixmap, mask);

  stock = FALSE;
  }
if (g_ascii_strncasecmp(id, "GDIS_PLAY", 9) == 0)
  {
  style = gtk_widget_get_style(window);

  pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask, &style->white,
                                        play_xpm);
  image = gtk_image_new_from_pixmap(pixmap, mask);

  stock = FALSE;
  }
if (g_ascii_strncasecmp(id, "GDIS_REWIND", 11) == 0)
  {
  style = gtk_widget_get_style(window);

  pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask, &style->white,
                                        rewind_xpm);
  image = gtk_image_new_from_pixmap(pixmap, mask);

  stock = FALSE;
  }
if (g_ascii_strncasecmp(id, "GDIS_FASTFORWARD", 16) == 0)
  {
  style = gtk_widget_get_style(window);

  pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask, &style->white,
                                        fastforward_xpm);
  image = gtk_image_new_from_pixmap(pixmap, mask);

  stock = FALSE;
  }
if (g_ascii_strncasecmp(id, "GDIS_STOP", 9) == 0)
  {
  style = gtk_widget_get_style(window);

  pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask, &style->white,
                                        stop_xpm);
  image = gtk_image_new_from_pixmap(pixmap, mask);

  stock = FALSE;
  }
if (g_ascii_strncasecmp(id, "GDIS_STEP_FORWARD", 17) == 0)
  {
  style = gtk_widget_get_style(window);

  pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask, &style->white,
                                        step_forward_xpm);
  image = gtk_image_new_from_pixmap(pixmap, mask);

  stock = FALSE;
  }
if (g_ascii_strncasecmp(id, "GDIS_STEP_BACKWARD", 18) == 0)
  {
  style = gtk_widget_get_style(window);

  pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask, &style->white,
                                        step_backward_xpm);
  image = gtk_image_new_from_pixmap(pixmap, mask);

  stock = FALSE;
  }

/* standard GTK */
if (stock)
  image = gtk_image_new_from_stock(id, GTK_ICON_SIZE_MENU);


hbox = gtk_hbox_new(FALSE, 4);
gtk_container_add(GTK_CONTAINER(button), hbox);

/* label dependent packing  */
if (text)
  {
  gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
  label = gtk_label_new(text);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  }
else
  {
  gtk_box_pack_start(GTK_BOX(hbox), image, TRUE, FALSE, 0);
  }

/* packing & callback */
if (box)
  gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

if (cb)
  g_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(cb), arg);

return(button);
}

/****************************************************************************/
/* force a redraw of text in a scrolled window - workaround for display bug */
/****************************************************************************/
gint gui_scrolled_refresh(GtkWidget *w, gpointer data)
{
gtk_widget_queue_draw(data);
return(TRUE);
}

/********************************************/
/* add a scrolled window with a text buffer */
/********************************************/
gpointer gui_scrolled_text_new(gint edit, GtkWidget *box)
{
GtkWidget *swin, *view;
GtkTextBuffer *buffer;
GtkAdjustment *adj;

g_assert(box != NULL);

swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_box_pack_start(GTK_BOX(box), swin, TRUE, TRUE, 0);
view = gtk_text_view_new();
gtk_container_add(GTK_CONTAINER(swin), view);
gtk_text_view_set_editable(GTK_TEXT_VIEW(view), edit);
buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

/* CURRENT - might want to be able to toggle this? */
/* make the font fixed width - looks prettier when displaying diffraction data */
{
PangoFontDescription *fd = pango_font_description_from_string("monospace");
gtk_widget_modify_font(view, fd);
pango_font_description_free(fd);
}

/* CURRENT - trying to fix scrolling artifact problem */
/* NEW - force a redraw - minimize work by only invalidate/redrawing the text view */
adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(swin));
g_signal_connect(G_OBJECT(adj), "value-changed", GTK_SIGNAL_FUNC(gui_scrolled_refresh), view);

return(buffer);
}

/*********************************/
/* update a buffer with new text */
/*********************************/
void gui_scrolled_text_set(gpointer buffer, const gchar *text)
{
gtk_text_buffer_set_text(buffer, text, -1);
}

/************************/
/* get text from buffer */
/************************/
gchar *gui_scrolled_text_get(gpointer buffer)
{
GtkTextIter start, end;

gtk_text_buffer_get_bounds(buffer, &start, &end);

return(gtk_text_buffer_get_text(buffer, &start, &end, TRUE));
}

/*******************************************/
/* update function for scolled text window */
/*******************************************/
void gui_text_window_update(GtkTextBuffer *buffer, gchar **text)
{
GtkTextIter start, end;

gtk_text_buffer_get_bounds(buffer, &start, &end);
g_free(*text);
*text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

/*********************************/
/* short cut for displaying text */
/*********************************/
GtkWidget *gui_text_window(gchar **text, gint editable)
{
gint n;
GtkWidget *swin, *view;
GtkTextBuffer *buffer;

g_assert(text != NULL);

swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
view = gtk_text_view_new();
gtk_text_view_set_editable(GTK_TEXT_VIEW(view), editable);
gtk_container_add(GTK_CONTAINER(swin), view);
buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
/* tags??? */
/*
gtk_text_buffer_create_tag(buffer, "fg_blue", "foreground", "blue", NULL);  
gtk_text_buffer_create_tag(buffer, "fg_red", "foreground", "red", NULL);
*/
if (*text)
  {
  n = strlen(*text);
  gtk_text_buffer_insert_at_cursor(buffer, *text, n);
  }

if (editable)
  g_signal_connect(G_OBJECT(buffer), "changed",
                   GTK_SIGNAL_FUNC(gui_text_window_update), text);

return(swin);
}

/**********************************/
/* vbox in a frame layout shorcut */
/**********************************/
GtkWidget *gui_frame_vbox(const gchar *label, gint p1, gint p2, GtkWidget *box)
{
GtkWidget *frame, *vbox;

g_assert(box != NULL);

frame = gtk_frame_new(label);
gtk_box_pack_start(GTK_BOX(box), frame, p1, p2, 0); 
gtk_container_set_border_width(GTK_CONTAINER(frame), 0.5*PANEL_SPACING);
vbox = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), vbox);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 0.5*PANEL_SPACING);

return(vbox);
}

/**********************************/
/* hbox in a frame layout shorcut */
/**********************************/
GtkWidget *gui_frame_hbox(const gchar *label, gint p1, gint p2, GtkWidget *box)
{
GtkWidget *frame, *hbox;

g_assert(box != NULL);

frame = gtk_frame_new(label);
gtk_box_pack_start(GTK_BOX(box), frame, p1, p2, 0); 
gtk_container_set_border_width(GTK_CONTAINER(frame), 0.5*PANEL_SPACING);
hbox = gtk_hbox_new(FALSE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), hbox);
gtk_container_set_border_width(GTK_CONTAINER(hbox), 0.5*PANEL_SPACING);

return(hbox);
}

/***************************************************************/
/* horizontal packing function for label plus some data widget */
/***************************************************************/
void gui_hbox_pack(GtkWidget *box, gchar *text, GtkWidget *w, gint pack)
{
gint pack1, pack2;
GtkWidget *label, *hbox;

g_assert(box != NULL);

/* TODO - the pack argument is intended to allow the possibility of different packing styles */
/* TODO - switch (pack) {} */
pack1 = TRUE;
pack2 = TRUE;

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, pack1, pack2, 0);

if (text)
  {
  label = gtk_label_new(text);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(hbox), w, FALSE, FALSE, 0);
  }
else
  {
  if (w)
    gtk_box_pack_end(GTK_BOX(hbox), w, TRUE, TRUE, 0);
  }
}

/*****************************/
/* pulldown callback handler */
/*****************************/
gint gui_pd_cb(GtkWidget *w, gpointer data)
{
gchar *text, **value;

/* get current (allocated) text in pulldown */
text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(w));

/* update internal variable (if non NULL) */
if (data)
  {
  value = data;
/* free old string */
  g_free(*value);
  if (!strlen(text))
    {
/* empty string - assign NULL to internal pointer */
    g_nullify_pointer(data);
    g_free(text);
    }
  else
    {
/* assign address of newly allocated string */
    memcpy(data, &text, sizeof(gchar **));
    }
  }
else
  g_free(text);

return(TRUE);
}

/******************************************/
/* set the list of string in the pulldown */
/******************************************/
/* FIXME - this needs to not change the actual text if it's in the list */
void gui_pd_list_set(const GSList *list, gpointer pd)
{
gint i, index=-1, flag=FALSE;
gchar *active=NULL, *current=NULL, **text=NULL;
const GSList *item;

/* checks */
if (!pd)
  return;
if (!GTK_IS_COMBO_BOX(pd))
  return;

/* NEW - cope with an attached value */
text = g_object_get_data(G_OBJECT(pd), "value");

/* NEW - get the current value in the pulldown */
if (!text)
  current = gtk_combo_box_get_active_text(GTK_COMBO_BOX(pd));

/* FIXME - ideally want to just remove all existing items */
/* CURRENT - use a large maximum to hopefully remove all */
for (i=50 ; i-- ; )
  gtk_combo_box_remove_text(GTK_COMBO_BOX(pd), i);

/* add the supplied list items */
i=0;
for (item=list ; item ; item=g_slist_next(item))
  {
  gtk_combo_box_append_text(GTK_COMBO_BOX(pd), item->data);
/* if no text value, see if the current value matches a list item */
  if (current)
    {
    if (g_strcasecmp(current, item->data) == 0)
      {
      flag=TRUE;
      index=i;
      }
    }
  i++;
  }

/* if passed a text pointer - set active to this value */
if (text)
  {
  if (*text)
    active = *text;
  }

/* if still nothing active, attempt to use first list value */
if (!active)
  if (list)
    active = list->data;

/* attempt to set active text with index reference */
/* this is preferable as it allows gui_pd_index_get() to work properly */
if (flag)
  {
  gtk_combo_box_set_active(GTK_COMBO_BOX(pd), index);
  }
else
  {
/* set active text fallbacks */
  if (active)
    gtk_entry_set_text(GTK_ENTRY(GTK_BIN(pd)->child), active);
  else
    gtk_entry_set_text(GTK_ENTRY(GTK_BIN(pd)->child), "");
  }
g_free(current);
}

/*************************************/
/* set editable state for a pulldown */
/*************************************/
void gui_pd_edit_set(gint editable, gpointer pd)
{
gtk_entry_set_editable(GTK_ENTRY(GTK_BIN(pd)->child), editable);
}

/**************************************/
/* pulldown menu convenience function */
/**************************************/
/* TODO - replace the old gui_pulldown_new() function */
gpointer gui_pd_new(gchar *label, gchar **text, GSList *list, GtkWidget *box)
{
gint i, active=-1;
GSList *item;
GtkWidget *hbox, *w1, *w2 = NULL;

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 4
w2 = gtk_combo_box_entry_new_text();
if (box)
  {
  if (label)
    {
/* label - pack entry at end */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
    w1 = gtk_label_new(label);
    gtk_box_pack_start(GTK_BOX(hbox), w1, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), w2, FALSE, FALSE, 0);
    }
  else
    {
/* no label - fill max */
    gtk_box_pack_end(GTK_BOX(box), w2, TRUE, TRUE, 0);
    }
  }

i=0;
for (item=list ; item ; item=g_slist_next(item))
  {
  if (text)
    {
    if (*text)
      {
      if (g_strcasecmp(*text, (gchar *) item->data) == 0)
        active = i;
      }
    }
  gtk_combo_box_append_text(GTK_COMBO_BOX(w2), item->data);
  i++;
  }

/* TODO - force set the combo box to variable value - matching wont always */
/* work as we allow editing, case sensitivity etc */

/* set entry to variable value, if no variable - default to 1st value */
if (active != -1)
  {
/* if the current value was found in the list, display it */
  gtk_combo_box_set_active(GTK_COMBO_BOX(w2), active);
  }
else
  {
/* if the text string is non NULL, but not in list - add it as default */
/* ie this will be a user edited value */
  if (text)
    {
    if (*text)
      gtk_combo_box_prepend_text(GTK_COMBO_BOX(w2), *text);
    }

/* default to first string as active displayed value */
  gtk_combo_box_set_active(GTK_COMBO_BOX(w2), 0);
  }

/* change handling */
if (text)
  {
  g_object_set_data(G_OBJECT(w2), "value", text);
  g_signal_connect(GTK_OBJECT(w2), "changed", GTK_SIGNAL_FUNC(gui_pd_cb), text);
  }
else
  {
/* FIXME - if no internal variable -> assume not editable - see gui_mdi solvent/solute */
/* useage where we want the user to only pick supplied value (ie from the project file list) */
// gtk_widget_set_sensitive(w2, FALSE);
  }

#endif

return(w2);
}

/*******************************************************/
/* retrieve the current active pulldown menu item text */
/*******************************************************/
/* returned txt should be freed */
gchar *gui_pd_text(gpointer pd)
{
#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 6
return(gtk_combo_box_get_active_text(GTK_COMBO_BOX(pd)));
#else
/* you're screwed */
return(NULL);
#endif
}

/******************************************************/
/* retrieve the index of the current active menu item */
/******************************************************/
/* can it return -1 if the text value doesn't match? */
gint gui_pd_index_get(gpointer pd)
{
return(gtk_combo_box_get_active(GTK_COMBO_BOX(pd)));
}

/**************************************/
/* pulldown menu convenience function */
/**************************************/
gpointer gui_pulldown_new(const gchar *text, GList *list, gint edit, GtkWidget *box)
{
GtkWidget *hbox, *label, *combo;

combo = gtk_combo_new();
gtk_combo_set_popdown_strings(GTK_COMBO(combo), list);
gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), edit);

if (box)
  {
/* create the text/spinner layout */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, 0);
  if (text)
    {
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
    }
  else
    gtk_box_pack_end(GTK_BOX(hbox), combo, TRUE, TRUE, 0);
  }
return(GTK_COMBO(combo)->entry);
}

/********************************/
/* pulldown menu data retrieval */
/********************************/
const gchar *gui_pulldown_text(gpointer w)
{
if (GTK_IS_ENTRY(w))
  return(gtk_entry_get_text(GTK_ENTRY(w)));
return(NULL);
}

/*******************************/
/* labelled colour editing box */
/*******************************/
void gui_colour_box(const gchar *text, gdouble *rgb, GtkWidget *box)
{
GtkWidget *hbox, *label, *button;
GdkColor colour;

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
if (text)
  {
  label = gtk_label_new(text);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  }

button = gtk_button_new_with_label("    ");
gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
g_signal_connect(GTK_OBJECT(button), "clicked",
                 GTK_SIGNAL_FUNC(dialog_colour_new), rgb);

colour.red   = rgb[0]*65535.0;
colour.green = rgb[1]*65535.0;
colour.blue  = rgb[2]*65535.0;
gtk_widget_modify_bg(button, GTK_STATE_NORMAL, &colour);
}

/***********************************************/
/* returns the selected item in the list store */
/***********************************************/
/* TODO - multiple selections */
/*
GSList *gui_scrolled_list_selected_get(gpointer data)
{
gpointer ptr;
GSList *selected=NULL;
GtkTreeIter iter;
GtkTreeModel *treemodel;
GtkTreeSelection *selection;
GtkTreeView *tree_view=data;

g_assert(tree_view != NULL);

selection = gtk_tree_view_get_selection(tree_view); 
if (!selection)
  return(NULL);
if (!gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  return(NULL);

gtk_tree_model_get(treemodel, &iter, 0, &ptr, -1);

selected = g_slist_prepend(selected, g_strdup(ptr));

return(selected);
}
*/

/************************************************/
/* returns the selected items in the list store */
/************************************************/
GSList *gui_scrolled_list_selected_get(gpointer data)
{
const gchar *name;
GSList *slist=NULL;
GList *item, *list;
GtkTreeModel *treemodel;
GtkTreeSelection *selection;
GtkTreeView *tree_view=data;
GtkTreeIter iter;

if (!tree_view)
  return(NULL);

treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));

list = gtk_tree_selection_get_selected_rows(selection, &treemodel);

for (item=list ; item ; item=g_list_next(item))
  {
  if (gtk_tree_model_get_iter(treemodel, &iter, item->data))
    {
    gtk_tree_model_get(treemodel, &iter, 0, &name, -1);

    slist = g_slist_prepend(slist, g_strdup(name));
    }
  }

return(slist);
}

/****************************/
/* update the scrolled list */
/****************************/
void gui_scrolled_list_refresh(GSList *list, gpointer data)
{
GSList *item;
GtkTreeIter iter;
GtkListStore *list_store;
GtkTreeView *tree_view=data;

if (!tree_view)
  return;

list_store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_view));

g_assert(list_store != NULL);

/* clear & populate */
gtk_list_store_clear(list_store);
for (item=list ; item ; item=g_slist_next(item))
  {
  gtk_list_store_append(list_store, &iter);
  gtk_list_store_set(list_store, &iter, 0, item->data, -1);
  }
}

/************************************************************/
/* create a list store from a single linked list of strings */
/************************************************************/
/* TODO - add callbacks/access primitives for selected items etc */
gpointer gui_scrolled_list_new(GtkWidget *box) 
{
GtkWidget *swin;
GtkWidget *tree_view=NULL;
GtkListStore *list_store=NULL;
GtkCellRenderer *cell;
GtkTreeViewColumn *column;
GtkTreeSelection *selection;
GtkAdjustment *adj;

g_assert(box != NULL);

/* scrolled window for the list */
swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_box_pack_start(GTK_BOX(box), swin, TRUE, TRUE, 0);

list_store = gtk_list_store_new(1, G_TYPE_STRING);
tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin), tree_view);

cell = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes("title", cell, "text", 0, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

gtk_tree_view_column_set_expand(column, FALSE);
gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);

/*
printf("list store 1 = %p\n", list_store);
printf("list store 2 = %p\n", GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view))));
*/
selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

/* CURRENT - force refresh on scroll */
adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(swin));
g_signal_connect(G_OBJECT(adj), "value-changed", GTK_SIGNAL_FUNC(gui_scrolled_refresh), tree_view);

return(tree_view);
}

/******************************************************************/
/* pass text data from the pulldown (if changed) to user callback */
/******************************************************************/
void gui_file_filter_changed(GtkEntry *entry, gpointer callback())
{
const gchar *tmp;

tmp = gtk_entry_get_text(entry);

if (callback)
  {
  callback(tmp);
  }
}

/*****************************/
/* set the file filter value */
/*****************************/
void gui_file_filter_set(gint id, gpointer data)
{
GtkEntry *entry=data;
struct file_pak *file;

file = file_type_from_id(id);
if (file)
  gtk_entry_set_text(entry, file->label);
}

/**********************************************************/
/* convenience function for building a file type pulldown */
/**********************************************************/
gpointer gui_file_filter(gpointer callback, GtkWidget *box)
{
GSList *item;
GList *list;
GtkWidget *combo;
struct file_pak *file;

/* build the recognized extension list */
list = NULL;
for (item=sysenv.file_list ; item ; item=g_slist_next(item))
  {
  file = item->data;
/* include in menu? */
  if (file->menu)
    list = g_list_append(list, file->label);
  }

/* combo box for file filter */
combo = gtk_combo_new();
gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), FALSE);
gtk_combo_set_popdown_strings(GTK_COMBO(combo), list);
gtk_box_pack_end(GTK_BOX(box), combo, FALSE, FALSE, 0);

if (callback)
  g_signal_connect(GTK_OBJECT(GTK_COMBO(combo)->entry), "changed", 
                   GTK_SIGNAL_FUNC(gui_file_filter_changed), callback);

g_list_free(list);

return(GTK_COMBO(combo)->entry);
}

/**********************************************/
/* callback for check box visibility toggling */
/**********************************************/
void gui_toggle_box_callback(GtkWidget *w, gpointer box)
{
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
  gtk_widget_show(box);
else
  gtk_widget_hide(box);
}

/******************************************************************/
/* labelled check box that toggles visibility of the returned box */
/******************************************************************/
GtkWidget *gui_toggled_vbox(const gchar *label, gint state, GtkWidget *box)
{
GtkWidget *button, *vbox1, *vbox2;

/* overall container */
vbox1 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), vbox1, FALSE, FALSE, 0);

/* the button */
button = gtk_check_button_new_with_label(label);
gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);

/* the visibility box */
vbox2 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox1), vbox2, TRUE, TRUE, 0);

/* set the state (NB: before the callback is attached) */
if (state)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
else
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);

/* attach the callback */
g_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(gui_toggle_box_callback), vbox2);

return(vbox2);
}

/*******************************/
/* callback for auto check box */
/*******************************/
void gui_checkbox_update(GtkWidget *w, gpointer data)
{
gint *value;

g_assert(data != NULL);
g_assert(w != NULL);

value = data;

if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
  *value = TRUE;
else
  *value = FALSE;
}

/***********************************/
/* automatically updated check box */
/***********************************/
GtkWidget *gui_checkbox(gchar *label, gint *state, GtkWidget *box)
{
GtkWidget *button;

/* create the button */
if (label)
  button = gtk_check_button_new_with_label(label);
else
  button = gtk_check_button_new();

if (state)
  {
  if (*state)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);

  g_signal_connect(GTK_OBJECT(button), "clicked",
                   GTK_SIGNAL_FUNC(gui_checkbox_update), state);
  }

if (box)
  gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

return(button);
}

/* NEW - approach to the problem of 2 way assoc with widgets and internal vars */
/* store widget + value - default: widget change -> value change (automatic) */
/* BUT when value needs to be set/assoc with something else -> gui_spin_modify() */

/******************************************/
/* change a spinners variable association */
/******************************************/
void gui_spin_modify(GtkWidget *spin, gdouble *value)
{
g_object_set_data(G_OBJECT(spin), "float", value);

if (value)
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), *value);
}

/********************************/
/* update the internal variable */
/********************************/
void gui_spin_update(GtkWidget *w, gpointer dummy)
{
gdouble current, *saved;

current = SPIN_FVAL(GTK_SPIN_BUTTON(w));
saved = g_object_get_data(G_OBJECT(w), "float");

if (saved)
  memcpy(saved, &current, sizeof(gdouble));
}

/**************************************************/
/* create a new spinner with assoc internal value */
/**************************************************/
/* TODO - deprec gui_direct_spin() in favour of this */
GtkWidget *gui_spin_new(gdouble *value, gdouble min, gdouble max, gdouble step)
{
GtkWidget *spin;

spin = gtk_spin_button_new_with_range(min, max, step);

/*
if (!value)
  value = g_malloc(sizeof(gdouble));
*/

gui_spin_modify(spin, value);

g_signal_connect_after(GTK_OBJECT(spin), "value-changed", GTK_SIGNAL_FUNC(gui_spin_update), NULL);

return(spin);
}

/****************************************/
/* create a labelled spinner (as above) */
/****************************************/
/* TODO - replace gui_direct_spin() with this */
GtkWidget *gui_text_spin_new(const gchar *text, gdouble *value, gdouble min, gdouble max, gdouble step)
{
GtkWidget *hbox, *label, *spin;

hbox = gtk_hbox_new(FALSE, 0);
if (text)
  {
  label = gtk_label_new(text);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  }
spin = gui_spin_new(value, min, max, step);
gtk_box_pack_end(GTK_BOX(hbox), spin, FALSE, FALSE, 0);

return(hbox);
}

/************************/
/* notebook convenience */
/************************/
struct notebook_pak
{
GtkWidget *w;
GHashTable *pages;
};

#define DEBUG_NOTEBOOK 0
/********************************************/
/* get visibility of notebook page by label */
/********************************************/
gint gui_notebook_page_visible_get(gpointer data, const gchar *label)
{
gint state=FALSE;
GtkWidget *page;
struct notebook_pak *notebook=data;

if (!notebook)
  return(state);

/* search for the notebook page given by label */
page = g_hash_table_lookup(notebook->pages, label);

#if DEBUG_NOTEBOOK
printf("notebook: %p, tab: %s, page: %p\n", notebook->w, label, page);
#endif

/* get visiblity */
if (page)
  g_object_get(G_OBJECT(page), "visible", &state, NULL);

return(state);
}

/********************************************/
/* set visibility of notebook page by label */
/********************************************/
void gui_notebook_page_visible_set(gpointer data, const gchar *label, gint value)
{
gint i, n;
GtkWidget *page, *child;
struct notebook_pak *notebook=data;

if (!notebook)
  return;

/* search for the notebook page given by label */
page = g_hash_table_lookup(notebook->pages, label);

#if DEBUG_NOTEBOOK
printf("notebook: %p, tab: %s, page: %p\n", notebook->w, label, page);
#endif

/* set visiblity */
if (page)
  {
  if (value)
    gtk_widget_show(page);
  else
    gtk_widget_hide(page);
  }

/* if visibility is true, make this tab the active one */
if (value)
  {
  n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook->w));
#if DEBUG_NOTEBOOK
printf("# tabs = %d\n", n);
#endif
  for (i=0 ; i<n ; i++)
    {
    child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook->w), i);
    if (child == page)
      {
#if DEBUG_NOTEBOOK
printf("activate %d\n", i);
#endif
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook->w), i);
      }
    }
  }
}

/****************************/
/* create new notebook page */
/****************************/
void gui_notebook_page_append(gpointer data, const gchar *title, GtkWidget *page)
{
GtkWidget *label;
struct notebook_pak *notebook=data;

if (!notebook)
  return;
if (!title)
  return;
if (!page)
  return;

label = gtk_label_new(title);
gtk_notebook_append_page(GTK_NOTEBOOK(notebook->w), page, label);
g_hash_table_insert(notebook->pages, g_strdup(title), page);
}

/********************/
/* destroy notebook */
/********************/
void gui_notebook_free(gpointer data)
{
struct notebook_pak *notebook=data;

g_hash_table_destroy(notebook->pages);
g_free(notebook);
}

/***********************/
/* create new notebook */
/***********************/
gpointer gui_notebook_new(GtkWidget *box)
{
struct notebook_pak *notebook;

notebook = g_malloc(sizeof(struct notebook_pak));
notebook->w = gtk_notebook_new();

notebook->pages = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

if (box)
  gtk_box_pack_start(GTK_BOX(box), notebook->w, TRUE, TRUE, 0);

/* TODO - link widget destroy signal to gui_notebook_free */

return(notebook);
}

/* DS */
struct model_pulldown_pak
{
gpointer inclusion_function;
gpointer widget;
};

/**********************************************************/
/* update loaded model list widget with current workspace */
/**********************************************************/
void gui_model_pulldown_refresh(gpointer data)
{
gpointer import;
GSList *item, *list, *name_list;
struct model_pulldown_pak *mp=data;
struct model_pak *model;

if (!mp)
  return;

/* build list of model names */
name_list = NULL;

/* iterate over all models */
list = project_model_list_get(sysenv.workspace);
for (item=list ; item ; item=g_slist_next(item))
  {
  model = item->data;

/* TODO - call inclusion test */

/* apply list */
//  name_list = g_slist_prepend(name_list, g_strdup(model->basename));

/* NEW - a better way of naming? */
/* NB: have to append - so index matches order */
import = gui_tree_parent_object(model);
if (import)
  {
  gchar *text = g_strdup_printf("%s:%s", import_filename(import), model->basename);
  name_list = g_slist_append(name_list, text);
  }
else
  {
  name_list = g_slist_append(name_list, g_strdup(model->basename));
  }


  }
g_slist_free(list);

/* update the list */
gui_pd_list_set(name_list, mp->widget);

free_slist(name_list);
}

/***********************************************************/
/* a widget that lists the currently loaded models by name */
/***********************************************************/
// argument is a true/false function that returns if a model should be included or not
// if NULL, all models in the current workspace will be included
gpointer gui_model_pulldown_new(gpointer inclusion_function)
{
struct model_pulldown_pak *mp;

mp = g_malloc(sizeof(struct model_pulldown_pak));

mp->inclusion_function = inclusion_function;
mp->widget = gui_pd_new(NULL, NULL, NULL, NULL);
gui_model_pulldown_refresh(mp);


/* FIXME - not sure why, but active is (incorrectly) set to -1 initially */
/* it onlys get set correctly after the first changed event is processed */
/* Below is a workaround */
gtk_combo_box_set_active(GTK_COMBO_BOX(mp->widget), 0);


return(mp);
}

/**********************************/
/* get the widget packing element */
/**********************************/
gpointer gui_model_pulldown_widget_get(gpointer data)
{
struct model_pulldown_pak *mp=data;

if (!mp)
  return(NULL);

return(mp->widget);
}

/************************************************/
/* get the currently selected model in the list */
/************************************************/
gpointer gui_model_pulldown_active(gpointer data)
{
gint n;
GSList *list;
struct model_pak *model;
struct model_pulldown_pak *mp=data;

if (!mp)
  return(NULL);

n = gui_pd_index_get(mp->widget);

//printf("n = %d\n", n);

list = project_model_list_get(sysenv.workspace);
/* TODO - filter out entries that fail inclusion test */
model = g_slist_nth_data(list, n);
g_slist_free(list);

return(model);
}

