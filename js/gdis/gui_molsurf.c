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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "gdis.h"
#include "file.h"
#include "parse.h"
#include "coords.h"
#include "matrix.h"
#include "molsurf.h"
#include "model.h"
#include "spatial.h"
#include "surface.h"
#include "sginfo.h"
#include "task.h"
#include "gui_shorts.h"
#include "interface.h"
#include "dialog.h"
#include "opengl.h"

/* main pak structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/* molsurf globals */
gdouble ms_prad=1.0;
gdouble ms_blur=0.3;
//gdouble ms_eden=0.1;

gchar *ms_iso_value=NULL;
gpointer gui_ms_colour=NULL, gui_ms_type=NULL, gui_ms_siesta=NULL;

GtkWidget *epot_vbox, *surf_epot_min, *surf_epot_max, *surf_epot_div;
GtkWidget *epot_pts, *ms_eden_box, *ms_siesta_box;

/********************************************/
/* get the type of surface desired from GUI */
/********************************************/
gint gui_ms_siesta_type_get(void)
{
gint type=MS_SIESTA_RHO;
const gchar *tmp;

if (!GTK_IS_ENTRY(gui_ms_siesta))
  return(type);

/* get selected iso-surface type */
tmp = gtk_entry_get_text(GTK_ENTRY(gui_ms_siesta));

// FIXME - order of RHO, DRHO check is important
if (g_strrstr(tmp, "RHO"))
  type = MS_SIESTA_RHO;
if (g_strrstr(tmp, "DRHO"))
  type = MS_SIESTA_DRHO;
if (g_strrstr(tmp, "LDOS"))
  type = MS_SIESTA_LDOS;
if (g_strrstr(tmp, "VH"))
  type = MS_SIESTA_VH;
if (g_strrstr(tmp, "VT"))
  type = MS_SIESTA_VT;

return(type);
}

/********************************************/
/* get the type of surface desired from GUI */
/********************************************/
gint gui_ms_type_get(void)
{
gint type=MS_MOLECULAR;
const gchar *tmp;

if (!GTK_IS_ENTRY(gui_ms_type))
  return(type);

/* get selected iso-surface type */
tmp = gtk_entry_get_text(GTK_ENTRY(gui_ms_type));

if (g_ascii_strncasecmp(tmp, "Siesta", 6) == 0)
  type = gui_ms_siesta_type_get();
if (g_ascii_strncasecmp(tmp, "Hirshfeld", 9) == 0)
  type = MS_HIRSHFELD;
if (g_ascii_strncasecmp(tmp, "Promolecule", 11) == 0)
  type = MS_SSATOMS;
if (g_ascii_strncasecmp(tmp, "Gaussian Cube", 13) == 0)
  type = MS_GAUSS_CUBE;

return(type);
}

/*********************************************/
/* get the colouring method desired from GUI */
/*********************************************/
gint gui_ms_colour_get(void)
{
gint colour=MS_TOUCH;
const gchar *tmp;

if (!GTK_IS_ENTRY(gui_ms_colour))
  return(colour);

/* get desired colouring type */
tmp = gtk_entry_get_text(GTK_ENTRY(gui_ms_colour));

/* FIXME - can we rename De Josh? It's too close to Default and */
/* will cause the "default" to be "de" if the strcmp order is reversed */
if (g_ascii_strncasecmp(tmp,"De", 2) == 0)
  colour = MS_DE;
if (g_ascii_strncasecmp(tmp,"Default", 7) == 0)
  colour = MS_TOUCH;
if (g_ascii_strncasecmp(tmp,"AFM", 3) == 0)
  colour = MS_AFM;
if (g_ascii_strncasecmp(tmp,"Electrostatic", 13) == 0)
  colour = MS_EPOT;
if (g_ascii_strncasecmp(tmp,"Curvedness", 10) == 0)
  colour = MS_CURVEDNESS;
if (g_ascii_strncasecmp(tmp,"Shape Index", 11) == 0)
  colour = MS_SHAPE_INDEX;

return(colour);
}

/***************************************/
/* setup and run a surface calculation */
/***************************************/
#define DEBUG_CALC_MOLSURF 0
void ms_calculate(void)
{
gint ms_method, ms_colour, ms_positive, ms_negative, ms_bad;
gchar *text;
gdouble ms_value, c[3];
const gchar *property_min, *property_max;
struct model_pak *model;

model = tree_model_get();
if (!model)
  return;

/* remove old surface (if any) */
spatial_destroy_by_type(SPATIAL_MOLSURF, model);

/* get desired surface type */
ms_method = gui_ms_type_get();

/* get desired colouring type */
ms_colour = gui_ms_colour_get();

ms_value = str_to_float(ms_iso_value);

//printf("%s -> %f\n", ms_iso_value, ms_value);

/* get the scale */
if (ms_colour == MS_EPOT && !model->epot_autoscale)
  {
  model->epot_min = str_to_float(gtk_entry_get_text(GTK_ENTRY(surf_epot_min)));
  model->epot_max = str_to_float(gtk_entry_get_text(GTK_ENTRY(surf_epot_max)));
  model->epot_div = str_to_float(gtk_entry_get_text(GTK_ENTRY(surf_epot_div)));
  }
else
  {
  model->epot_min = G_MAXDOUBLE;
  model->epot_max = -G_MAXDOUBLE;
  }

/* method based setup */
property_min = NULL;
property_max = NULL;
ms_positive = TRUE;
ms_negative = FALSE;
ms_bad = FALSE;
switch (ms_method)
  {
  case MS_GAUSS_CUBE:
    property_min = property_lookup("Gaussian cube min", model); 
    property_max = property_lookup("Gaussian cube max", model); 
    ms_colour = MS_DENSITY_VALUE;
// allow -ve for gaussian cube 
    ms_negative = TRUE;    
    break;

  case MS_SIESTA_RHO:
    property_min = property_lookup("Siesta RHO min", model); 
    property_max = property_lookup("Siesta RHO max", model); 
    ms_colour = MS_DENSITY_VALUE;
    break;

  case MS_SIESTA_DRHO:
    property_min = property_lookup("Siesta DRHO min", model); 
    property_max = property_lookup("Siesta DRHO max", model); 
    ms_colour = MS_DENSITY_VALUE;
// allow -ve for drho 
    ms_negative = TRUE;    
    break;

  case MS_SIESTA_LDOS:
    property_min = property_lookup("Siesta LDOS min", model); 
    property_max = property_lookup("Siesta LDOS max", model); 
    ms_colour = MS_DENSITY_VALUE;
    break;

  case MS_SIESTA_VH:
    property_min = property_lookup("Siesta VH min", model); 
    property_max = property_lookup("Siesta VH max", model); 
    ms_colour = MS_DENSITY_VALUE;
    break;

  case MS_SIESTA_VT:
    property_min = property_lookup("Siesta VT min", model); 
    property_max = property_lookup("Siesta VT max", model); 
    ms_colour = MS_DENSITY_VALUE;
    break;

  case MS_SSATOMS:
    ms_colour = MS_DENSITY_VALUE;
    break;

  default:
    ms_value = ms_blur;
  }

/* property bounds check */
if (property_max)
  {
  if (ms_value > str_to_float(property_max))
    ms_bad = TRUE;
  }
if (property_min)
  {
  if (ms_value < str_to_float(property_min))
    ms_bad = TRUE;
  }

/* warn if value is outside range */
if (ms_bad)
  {
  gui_text_show(WARNING, "Iso-surface value is outside range (see Model : Content)\n");
  return;
  }

/* don't bother with -ve if it's 0 */
if (fabs(ms_value) < G_MINDOUBLE)
  ms_negative = FALSE;

/* +ve call */
if (ms_positive)
  ms_cube(ms_value, ms_method, ms_colour, model);

if (ms_negative)
  {
/* ms colour - swap red and blue components */
/* FIXME - if +ve colour is green - this won't show a difference */
  ARR3SET(c, sysenv.render.ms_colour);
  sysenv.render.ms_colour[0] = c[2];
  sysenv.render.ms_colour[2] = c[0];

/* -ve call */
  ms_cube(-ms_value, ms_method, ms_colour, model);

/* restore colour */
  ARR3SET(sysenv.render.ms_colour, c);
  }

/* update widget */
if (ms_colour == MS_EPOT && model->epot_autoscale)
  {
  text = g_strdup_printf("%f", model->epot_min);
  gtk_entry_set_text(GTK_ENTRY(surf_epot_min), text);
  g_free(text);

  text = g_strdup_printf("%f", model->epot_max);
  gtk_entry_set_text(GTK_ENTRY(surf_epot_max), text);
  g_free(text);

  text = g_strdup_printf("%d", model->epot_div);
  gtk_entry_set_text(GTK_ENTRY(surf_epot_div), text);
  g_free(text);
  }

coords_compute(model);

sysenv.refresh_dialog = TRUE;

gui_refresh(GUI_CANVAS);
}

/************************/
/* simple deletion hook */
/************************/
void ms_delete(void)
{
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);

/* remove any previous surfaces */
spatial_destroy_by_label("molsurf", model);
model->ms_colour_scale = FALSE;
coords_init(CENT_COORDS, model);
redraw_canvas(SINGLE);
}

/*******************************************/
/* Molecular surface colour mode selection */
/*******************************************/
void ms_iso_method(gpointer dummy)
{
gint ms_method;

ms_method = gui_ms_type_get();

/* update GUI based on selected type */
gtk_widget_hide(ms_siesta_box);
switch (ms_method)
  {
   case MS_SIESTA_RHO:
   case MS_SIESTA_DRHO:
   case MS_SIESTA_LDOS:
   case MS_SIESTA_VH:
   case MS_SIESTA_VT:
     gtk_widget_set_sensitive(ms_eden_box, TRUE);
     gtk_widget_show(ms_siesta_box);
     break;

   case MS_GAUSS_CUBE:
     gtk_widget_set_sensitive(ms_eden_box, TRUE);
     break;

  default:
     gtk_widget_set_sensitive(ms_eden_box, FALSE);
  }
}

/************************************************************/
/* callback to update electrostatic autoscaling sensitivity */
/************************************************************/
void gui_epot_scale_sensitive(GtkWidget *w, gpointer data)
{
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
  gtk_widget_set_sensitive(epot_vbox, FALSE);
else
  gtk_widget_set_sensitive(epot_vbox, TRUE);
}

/***************************/
/* molecular surface setup */
/***************************/
void gui_isosurf_dialog()
{
gchar *text;
gpointer dialog;
GList *list;
GtkWidget *window, *frame, *main_vbox, *vbox, *vbox2, *hbox, *label;
struct model_pak *data;

/* checks */
data = tree_model_get();
if (!data)
  return;
if (data->id == MORPH)
  return;

/* create dialog */
dialog = dialog_request(SURF, "Iso-surfaces", NULL, NULL, data);
if (!dialog)
  return;
window = dialog_window(dialog);

main_vbox = GTK_DIALOG(window)->vbox;

/* isosurface type */
frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(main_vbox), frame, FALSE, FALSE, 0); 
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);

vbox = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), vbox);
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

/* combo box */
list = NULL;
list = g_list_prepend(list, "Molecular surface");
list = g_list_prepend(list, "Hirshfeld surface");
list = g_list_prepend(list, "Promolecule iso-surface");
list = g_list_prepend(list, "Gaussian cube");
list = g_list_prepend(list, "Siesta grid");
list = g_list_reverse(list);
gui_ms_type = gui_pulldown_new("Iso-surface Type", list, FALSE, hbox);
g_signal_connect(GTK_OBJECT(gui_ms_type), "changed", GTK_SIGNAL_FUNC(ms_iso_method), NULL);

/* additional box (siesta) */
ms_siesta_box = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), ms_siesta_box, FALSE, TRUE, 0);
list = NULL;
list = g_list_prepend(list, "Siesta VT");
list = g_list_prepend(list, "Siesta VH");
list = g_list_prepend(list, "Siesta LDOS");
list = g_list_prepend(list, "Siesta DRHO");
list = g_list_prepend(list, "Siesta RHO");
gui_ms_siesta = gui_pulldown_new("Siesta data", list, FALSE, ms_siesta_box);
g_signal_connect(GTK_OBJECT(gui_ms_siesta), "changed", GTK_SIGNAL_FUNC(ms_iso_method), NULL);

/* colour mode */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

/* NB: Default != Nearest atom - eg electron density */
list = NULL;
list = g_list_prepend(list, "Default");
list = g_list_prepend(list, "AFM");
//list = g_list_prepend(list, "Electrostatic");
list = g_list_prepend(list, "Curvedness");
list = g_list_prepend(list, "Shape index");
list = g_list_prepend(list, "De");
list = g_list_reverse(list);
gui_ms_colour = gui_pulldown_new("Colour method", list, FALSE, hbox);

/* frame for spinner setup */
frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(main_vbox),frame,FALSE,FALSE,0); 
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);

vbox2 = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), vbox2);

gui_direct_spin("Triangulation grid size",
                  &sysenv.render.ms_grid_size, 0.05, 10.0, 0.05, NULL, NULL, vbox2);

gui_direct_spin("Molecular surface blurring", &ms_blur, 0.05, 1.0, 0.05, NULL, NULL, vbox2);


/* FRAME - electron density setup */
frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(main_vbox),frame,FALSE,FALSE,0); 
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);

ms_eden_box = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), ms_eden_box);

//gui_direct_spin("Iso-surface value", &ms_eden, 0.00001, 1.0, 0.00001, NULL, NULL, ms_eden_box);
gui_text_entry("Iso-surface value ", &ms_iso_value, TRUE, TRUE, ms_eden_box);

gui_colour_box("Positive iso-surface colour", sysenv.render.ms_colour, ms_eden_box);


/* FRAME - electrostatic potential scale setup */
frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(main_vbox), frame, FALSE, TRUE,0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);

vbox2 = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(frame), vbox2);

/* CURRENT - disable this as electrostatic calc (gulp based) is broken */
gtk_widget_set_sensitive(vbox2, FALSE);

gui_auto_check("Electrostatic autoscaling", gui_epot_scale_sensitive, NULL, &data->epot_autoscale, vbox2);

epot_vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox2), epot_vbox, FALSE, TRUE,0);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(epot_vbox), hbox, FALSE, TRUE,0);
label = gtk_label_new("maximum ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
surf_epot_max = gtk_entry_new();
gtk_box_pack_end(GTK_BOX(hbox), surf_epot_max, FALSE, FALSE, 0);
text = g_strdup_printf("%f", data->epot_max);
gtk_entry_set_text(GTK_ENTRY(surf_epot_max), text);
g_free(text);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(epot_vbox), hbox, FALSE, TRUE,0);
label = gtk_label_new("minimum ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
surf_epot_min = gtk_entry_new();
gtk_box_pack_end(GTK_BOX(hbox), surf_epot_min, FALSE, FALSE, 0);
text = g_strdup_printf("%f", data->epot_min);
gtk_entry_set_text(GTK_ENTRY(surf_epot_min), text);
g_free(text);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(epot_vbox), hbox, FALSE, TRUE,0);
label = gtk_label_new("divisions ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
surf_epot_div = gtk_entry_new();
gtk_box_pack_end(GTK_BOX(hbox), surf_epot_div, FALSE, FALSE, 0);
text = g_strdup_printf("%d", data->epot_div);
gtk_entry_set_text(GTK_ENTRY(surf_epot_div), text);
g_free(text);

/* terminating buttons */
gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Iso-surfaces", GTK_DIALOG(window)->action_area);

gui_stock_button(GTK_STOCK_EXECUTE, ms_calculate, NULL,
                   GTK_DIALOG(window)->action_area);

gui_stock_button(GTK_STOCK_REMOVE, ms_delete, NULL,
                   GTK_DIALOG(window)->action_area);

gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog,
                   GTK_DIALOG(window)->action_area);

/* done */
gtk_widget_show_all(window);
gtk_widget_set_sensitive(epot_vbox, FALSE);
gtk_widget_set_sensitive(ms_eden_box, FALSE);

gtk_widget_hide(ms_siesta_box);
}

