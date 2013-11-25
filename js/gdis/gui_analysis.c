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
#include "coords.h"
#include "file.h"
#include "graph.h"
#include "model.h"
#include "analysis.h"
#include "dialog.h"
#include "interface.h"
#include "gui_shorts.h"

extern struct elem_pak elements[];
extern struct sysenv_pak sysenv;

/******************************/
/* export graphs as text data */
/******************************/
void analysis_export(gchar *name)
{
}

/******************************/
/* export graph file selector */
/******************************/
void analysis_export_dialog(void)
{
}

/**********************/
/* submit an rdf task */
/**********************/
void exec_analysis_task(GtkWidget *w, gpointer dialog)
{
const gchar *type;
gpointer gui_menu_calc;
struct analysis_pak *analysis;
struct model_pak *model;

/* duplicate the analysis structure at time of request */
g_assert(dialog != NULL);
model = dialog_model(dialog);
g_assert(model != NULL);
analysis = analysis_dup(model->analysis);

/* get the task type requested from the dialog */
gui_menu_calc = dialog_child_get(dialog, "gui_menu_calc");
type = gui_pulldown_text(gui_menu_calc);

/* if it matches a model property - plot that */
if (property_lookup(type, model))
  {
  analysis_plot(type, analysis, model);
  return;
  }

/* otherwise, special case runs */
if (g_ascii_strncasecmp(type, "RDF", 3) == 0)
  {
  analysis->rdf_normalize = TRUE;
  task_new("RDF", &analysis_plot_rdf, analysis, &analysis_show, model, model);
  return;
  }
if (g_ascii_strncasecmp(type, "Pair", 4) == 0)
  {
  analysis->rdf_normalize = FALSE;
  task_new("Pair", &analysis_plot_rdf, analysis, &analysis_show, model, model);
  return;
  }
if (g_ascii_strncasecmp(type, "VACF", 4) == 0)
  {
  task_new("VACF", &analysis_plot_vacf, analysis, &analysis_show, model, model);
  return;
  }
if (g_ascii_strncasecmp(type, "Meas", 4) == 0)
  {
  task_new("Measure", &analysis_plot_meas, analysis, &analysis_show, model, model);
  return;
  }
if (g_ascii_strncasecmp(type, "IR ", 3) == 0)
  {
  gui_phonon_plot(type, model);
  return;
  }
if (g_ascii_strncasecmp(type, "Raman ", 3) == 0)
  {
  gui_phonon_plot(type, model);
  return;
  }

/* no match => user probably mispelled the property */
gui_text_show(WARNING, "No such property found in current model.\n");
}

/**********************/
/* atom entry changed */
/**********************/
void md_atom_changed(GtkWidget *w, gchar **atom)
{
g_assert(w != NULL);

/* NB: we pass a pointer to the character pointer */
if (*atom)
  g_free(*atom);

*atom = g_strdup(gtk_entry_get_text(GTK_ENTRY(w)));
}

/****************************************************************/
/* set the sensitivity of the RDF box based on calculation type */
/****************************************************************/
void gui_analysis_state_set(GtkWidget *w, gpointer dialog)
{
const gchar *type;
gpointer gui_menu_calc, gui_vbox_rdf;

gui_menu_calc = dialog_child_get(dialog, "gui_menu_calc");
gui_vbox_rdf = dialog_child_get(dialog, "gui_vbox_rdf");

type = gui_pulldown_text(gui_menu_calc);

if (g_ascii_strncasecmp(type, "RDF", 3) == 0)
  {
  gtk_widget_set_sensitive(GTK_WIDGET(gui_vbox_rdf), TRUE);
  return;
  }
if (g_ascii_strncasecmp(type, "Pair", 4) == 0)
  {
  gtk_widget_set_sensitive(GTK_WIDGET(gui_vbox_rdf), TRUE);
  return;
  }

gtk_widget_set_sensitive(GTK_WIDGET(gui_vbox_rdf), FALSE);
}

/*******************************/
/* multi-frame analysis dialog */
/*******************************/
void gui_analysis_dialog(void)
{
gpointer dialog;
GtkWidget *window, *combo, *label;
GtkWidget *frame, *main_hbox, *main_vbox, *hbox, *vbox;
GtkWidget *gui_menu_calc, *gui_vbox_rdf;
GSList *list, *item;
GList *match_list=NULL, *calc_list=NULL;
struct model_pak *model;
struct analysis_pak *analysis;

/* checks and setup */
model = tree_model_get();
if (!model)
  return;
if (analysis_init(model))
  return;

analysis = model->analysis;

/* create new dialog */
dialog = dialog_request(MD_ANALYSIS, "Analysis Plotting", NULL, NULL, model);
if (!dialog)
  return;
window = dialog_window(dialog);

/* --- main box */
main_hbox = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), main_hbox, TRUE, TRUE, 0);

/* --- left pane */
main_vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(main_hbox), main_vbox, TRUE, TRUE, 0);

/* --- analysis dialogs are specific to the active model when initiated */
frame = gtk_frame_new("Model");
gtk_box_pack_start(GTK_BOX(main_vbox), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);
hbox = gtk_hbox_new(TRUE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), hbox);
label = gtk_label_new(model->basename);
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

/* --- calculation menu */
frame = gtk_frame_new(" Plot ");
gtk_box_pack_start(GTK_BOX(main_vbox), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);
vbox = gtk_vbox_new(TRUE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), vbox);

/* --- menu items */
calc_list = g_list_prepend(calc_list, "Raman spectrum");
calc_list = g_list_prepend(calc_list, "IR spectrum");
calc_list = g_list_prepend(calc_list, "Measurement");
//calc_list = g_list_prepend(calc_list, "Temperature");
//calc_list = g_list_prepend(calc_list, "Pressure");
//calc_list = g_list_prepend(calc_list, "Energy");
calc_list = g_list_prepend(calc_list, "VACF");
calc_list = g_list_prepend(calc_list, "RDF");

gui_menu_calc = gui_pulldown_new(NULL, calc_list, TRUE, vbox);
dialog_child_set(dialog, "gui_menu_calc", gui_menu_calc);
g_signal_connect(GTK_OBJECT(gui_menu_calc), "changed",
                 GTK_SIGNAL_FUNC(gui_analysis_state_set), dialog);

/* frame range to use in analysis */
vbox = gui_frame_vbox("Frame Interval", FALSE, FALSE, main_vbox);
gui_direct_spin("Start", &analysis->frame_start, 1, analysis->frame_stop-1, 1, NULL, NULL, vbox);
gui_direct_spin("Stop", &analysis->frame_stop, 1, analysis->frame_stop, 1, NULL, NULL, vbox);
gui_direct_spin("Step", &analysis->frame_step, 1, analysis->frame_stop-analysis->frame_start, 1, NULL, NULL, vbox);

/* --- right pane */
gui_vbox_rdf = gtk_vbox_new(FALSE, 0);
dialog_child_set(dialog, "gui_vbox_rdf", gui_vbox_rdf);
gtk_box_pack_start(GTK_BOX(main_hbox), gui_vbox_rdf, TRUE, TRUE, 0);

// TODO - this is really an x axis modifier which varies according
// to what's being plotted ... need a graph editing tool
/* --- interval setup */
frame = gtk_frame_new("Distance Interval");
gtk_box_pack_start(GTK_BOX(gui_vbox_rdf), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);
vbox = gtk_vbox_new(TRUE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), vbox);

/* TODO - some warning about the RDF being valid only for < L/2? */
/* see Allan & Tildesley pg 199 */
gui_direct_spin("Start", &analysis->start, 0.0, 10.0*model->rmax, 0.1, NULL, NULL, vbox);
gui_direct_spin("Stop", &analysis->stop, 0.1, 10.0*model->rmax, 0.1, NULL, NULL, vbox);
gui_direct_spin("Step", &analysis->step, 0.01, model->rmax, 0.01, NULL, NULL, vbox);

/* --- RDF atom setup */
frame = gtk_frame_new("Atoms");
gtk_box_pack_start(GTK_BOX(gui_vbox_rdf), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);
vbox = gtk_vbox_new(TRUE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(frame), vbox);

/* setup unique element label list */
list = find_unique(LABEL, model);
match_list = g_list_append(match_list, "Any");
for (item=list ; item  ; item=g_slist_next(item))
  {
  match_list = g_list_append(match_list, item->data);
  }
g_slist_free(list);

/* match 1 */
combo = gtk_combo_new();
gtk_combo_set_popdown_strings(GTK_COMBO(combo), match_list);
gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), TRUE);
gtk_box_pack_start(GTK_BOX(vbox), combo, TRUE, TRUE, 0);
g_signal_connect(GTK_OBJECT(GTK_COMBO(combo)->entry), "changed",
                 GTK_SIGNAL_FUNC(md_atom_changed), (gpointer) &analysis->atom1);

/* match 2 */
combo = gtk_combo_new();
gtk_combo_set_popdown_strings(GTK_COMBO(combo), match_list);
gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), TRUE);
gtk_box_pack_start(GTK_BOX(vbox), combo, TRUE, TRUE, 0);
g_signal_connect(GTK_OBJECT(GTK_COMBO(combo)->entry), "changed",
                 GTK_SIGNAL_FUNC(md_atom_changed), (gpointer) &analysis->atom2);

/* terminating buttons */
gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Analysis and Plotting", GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_EXECUTE, exec_analysis_task, dialog,
                   GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog,
                   GTK_DIALOG(window)->action_area);

gtk_widget_show_all(window);

gui_analysis_state_set(NULL, dialog);
}

