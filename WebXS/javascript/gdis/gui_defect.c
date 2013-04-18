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
#include <time.h>

#include "gdis.h"
#include "defect.h"
#include "dialog.h"
#include "matrix.h"
#include "gui_shorts.h"
#include "interface.h"

/* globals */
extern struct sysenv_pak sysenv;
struct defect_pak defect;
GtkWidget *gui_defect_iso;
GtkWidget *gui_defect_ani;
gpointer gui_source_model;

/**********************/
/* setup gui defaults */
/**********************/
void gui_defect_default(void)
{
defect.cleave = FALSE;
defect.neutral = FALSE;
defect.cluster = FALSE;

defect.isotropic = TRUE;
defect.modulus = 100.0;
defect.ratio = 0.4;

/* setup s from these values */
defect_isotropic_compliance_get(defect.s, &defect);

VEC2SET(defect.region, 10, 10);
VEC3SET(defect.orient, 1, 0, 0);
VEC3SET(defect.burgers, 1, 0, 0);
VEC2SET(defect.origin, 0, 0);
VEC2SET(defect.center, 0, 0);
}

/*****************************************/
/* callback to start the defect building */
/*****************************************/
void gui_defect_build(void)
{
struct model_pak *model;

/* checks and setup */
model = gui_model_pulldown_active(gui_source_model);
if (!model)
  {
  gui_popup_message("Missing source model.");
  return;
  }
if (model->periodic != 3)
  {
  gui_popup_message("Your source model is not 3D periodic.");
  return;
  }

/* build */
defect_new(&defect, model);
}

/**************************/
/* update on model switch */
/**************************/
void gui_defect_refresh(gpointer data)
{
}

/***************************/
/* elasticity model change */
/***************************/
void gui_defect_elasticity_changed(GtkWidget *pulldown, gpointer dummy)
{
gchar *text;

text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(pulldown));

//printf("Current = %s\n", text);

/* custom */
if (g_ascii_strncasecmp(text, "cust", 4) == 0)
  {
  defect.isotropic = FALSE;
  gtk_widget_hide(gui_defect_iso);
  gtk_widget_show(gui_defect_ani);
  }
else
  {
/* isotropic */
  defect.isotropic = TRUE;
  gtk_widget_hide(gui_defect_ani);
  gtk_widget_show(gui_defect_iso);
  }
}

/************************************/
/* setup the defect creation dialog */
/************************************/
void gui_defect_dialog(void)
{
gint i, x, y;
gpointer dialog, pulldown;
GSList *list;
GtkWidget *window, *vbox, *table, *label, *spin;

/* create new dialog */
/* TODO - get rid of the need for a unique code in this call */
/* replace with lookups based on the name & single/multiple instance allowed */
dialog = dialog_request(100, "Dislocation Builder", gui_defect_refresh, NULL, NULL);
if (!dialog)
  return;
window = dialog_window(dialog);

/* init */
gui_defect_default();

/* defect build setup frame */
vbox = gui_frame_vbox("Defect Geometry", FALSE, FALSE, GTK_DIALOG(window)->vbox);
table = gtk_table_new(4, 5, FALSE);
gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0); 

label = gtk_label_new("Orientation vector ");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
spin = gui_spin_new(&defect.orient[0], -20, 20, 1);
gtk_table_attach_defaults(GTK_TABLE(table),spin,1,2,0,1);
spin = gui_spin_new(&defect.orient[1], -20, 20, 1);
gtk_table_attach_defaults(GTK_TABLE(table),spin,2,3,0,1);
spin = gui_spin_new(&defect.orient[2], -20, 20, 1);
gtk_table_attach_defaults(GTK_TABLE(table),spin,3,4,0,1);

label = gtk_label_new("Burgers vector ");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
spin = gui_spin_new(&defect.burgers[0], -20, 20, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,1,2,1,2);
spin = gui_spin_new(&defect.burgers[1], -20, 20, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,2,3,1,2);
spin = gui_spin_new(&defect.burgers[2], -20, 20, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,3,4,1,2);

label = gtk_label_new("Defect origin ");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);
spin = gui_spin_new(&defect.origin[0], 0.0, 1.0, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,2,3,2,3);
spin = gui_spin_new(&defect.origin[1], 0.0, 1.0, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,3,4,2,3);

label = gtk_label_new("Defect center ");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,3,4);
spin = gui_spin_new(&defect.center[0], 0.0, 1.0, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,2,3,3,4);
spin = gui_spin_new(&defect.center[1], 0.0, 1.0, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,3,4,3,4);

label = gtk_label_new("Region sizes ");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,4,5);
spin = gui_direct_spin(NULL, &defect.region[0], 0, 999, 1, NULL, NULL, NULL);
gtk_table_attach_defaults(GTK_TABLE(table),spin,2,3,4,5);
spin = gui_direct_spin(NULL, &defect.region[1], 0, 999, 1, NULL, NULL, NULL);
gtk_table_attach_defaults(GTK_TABLE(table),spin,3,4,4,5);

/* source material properties */
/* TODO - pulldown that switches between iso/ani-iso case */
vbox = gui_frame_vbox("Material Properties", FALSE, FALSE, GTK_DIALOG(window)->vbox);
list = NULL;
list = g_slist_prepend(list, "Custom");
list = g_slist_prepend(list, "Isotropic");
pulldown = gui_pd_new("Elasticity", NULL, list, vbox);
g_signal_connect(GTK_OBJECT(pulldown), "changed", GTK_SIGNAL_FUNC(gui_defect_elasticity_changed), NULL);

/* isotropic case */
gui_defect_iso = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), gui_defect_iso, FALSE, FALSE, 0); 

spin = gui_text_spin_new("Young's modulus", &defect.modulus, 0, 9999, 1.0);
gtk_box_pack_start(GTK_BOX(gui_defect_iso), spin, FALSE, FALSE, 0); 

spin = gui_text_spin_new("Poisson's ratio", &defect.ratio, 0.0, 1.0, 0.01);
gtk_box_pack_start(GTK_BOX(gui_defect_iso), spin, FALSE, FALSE, 0); 

/* anisotropic case */
gui_defect_ani = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), gui_defect_ani, FALSE, FALSE, 0); 
table = gtk_table_new(6, 6, FALSE);
gtk_box_pack_start(GTK_BOX(gui_defect_ani), table, TRUE, TRUE, 0); 

for (i=0 ; i<36 ; i++)
  {
/* TODO - check this range is proper */
  spin = gui_spin_new(&defect.s[i], 0.0, 2.0, 0.0001);
  y = i / 6;
  x = i - 6*y;

//printf("[%d] = (%d,%d)\n", i, x, y);
  gtk_table_attach_defaults(GTK_TABLE(table),spin,x,x+1,y,y+1);
  }

/* construct setup frame */
vbox = gui_frame_vbox("Options", FALSE, FALSE, GTK_DIALOG(window)->vbox);
gui_direct_check("Allow bond cleaving ", &defect.cleave, NULL, NULL, vbox);
gui_direct_check("Discard periodicity ", &defect.cluster, NULL, NULL, vbox);


/* CURRENT - model to act on */
vbox = gui_frame_vbox("Source Structure", FALSE, FALSE, GTK_DIALOG(window)->vbox);
gui_source_model = gui_model_pulldown_new(NULL);
gtk_box_pack_start(GTK_BOX(vbox), gui_model_pulldown_widget_get(gui_source_model), TRUE, TRUE, 0); 


/* terminating buttons */
gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Dislocation Builder", GTK_DIALOG(window)->action_area);

gui_stock_button(GTK_STOCK_EXECUTE, gui_defect_build, NULL,
                   GTK_DIALOG(window)->action_area);

gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog,
                   GTK_DIALOG(window)->action_area);

gtk_widget_show_all(window);
gtk_widget_hide(gui_defect_ani);
}

