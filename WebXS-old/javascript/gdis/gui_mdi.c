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
#include <stdlib.h>
#include <string.h>

#ifndef __WIN32
#include <sys/times.h>
#endif

#include "gdis.h"
#include "file.h"
#include "import.h"
#include "project.h"
#include "project.h"
#include "coords.h"
#include "edit.h"
#include "matrix.h"
#include "model.h"
#include "render.h"
#include "select.h"
#include "gui_shorts.h"
#include "interface.h"
#include "dialog.h"
#include "mdi_pak.h"
#include "mdi.h"
#include "parse.h"
#include "library.h"

#define DEBUG 0

extern struct sysenv_pak sysenv;

gpointer mdi_project=NULL;

GtkWidget *mdi_tree_view=NULL;
GtkWidget *gui_mdi_box_widget=NULL;
GtkListStore *mdi_list_store=NULL;
gdouble mdi_length=4.0, mdi_quantity=1.0;
gpointer gui_mdi_solvent=NULL, gui_mdi_solute=NULL;
gpointer gui_mdi_project=NULL;
gpointer gui_mdi_action=NULL;

/* TODO - replace all lookups of model ptrs with a lookup within current project */
/* TODO - specify project in arg */
/* TODO - move t0 project */
gpointer project_model_get(gint n)
{
gint i;
gpointer project;
GSList *item1, *item2, *list1, *list2;

//project = tree_project_get();
project = mdi_project;
if (!project)
  return(NULL);

i=0;
list1 = project_imports_get(project);
for (item1=list1 ; item1 ; item1=g_slist_next(item1))
  {
  list2 = import_object_list_get(IMPORT_MODEL, item1->data);
  for (item2=list2 ; item2 ; item2=g_slist_next(item2))
    {
    if (i == n)
      return(item2->data);
    i++;
    }
  }

return(NULL);
}

/**********************************************/
/* reconfigure dialog based on desired action */
/**********************************************/
void gui_mdi_widget_update(GtkWidget *w, gpointer dummy)
{
const gchar *text;

text = gui_pd_text(gui_mdi_action);

if (g_strncasecmp(text, "Current", 7) == 0)
  gtk_widget_hide(gui_mdi_box_widget);
else
  gtk_widget_show(gui_mdi_box_widget);
}

/*******************************/
/* update the mdi building tab */
/*******************************/
void gui_mdi_solvent_update(void)
{
GSList *item1;
GSList *solvent_list=NULL;
struct folder_pak *folder;
struct entry_pak *entry;

/* library of predefined solvents */
folder = g_hash_table_lookup(sysenv.library, "solvents");
if (folder)
  {
  for (item1=folder->list ; item1 ; item1=g_slist_next(item1))
    {
    entry = item1->data;

    solvent_list = g_slist_prepend(solvent_list, entry->name);
    }
  }

gui_pd_list_set(solvent_list, gui_mdi_solvent);

g_slist_free(solvent_list);
}

/******************************/
/* bridge from new GUI to old */
/******************************/
#define DEBUG_MDI_BUILD 0
void gui_mdi_build(void)
{
const gchar *text;
struct mdi_pak *mdi;
struct model_pak *model;

/* alloc */
mdi = g_malloc(sizeof(struct mdi_pak));
mdi->box_dim = mdi_length;

/* process GUI settings */
text = gui_pd_text(gui_mdi_action);
if (g_strncasecmp(text, "Current", 7) == 0)
  {
#if DEBUG_MDI_BUILD
printf("Solvating existing box...\n");
#endif

  mdi->model = tree_model_get();
  }
else
  {
#if DEBUG_MDI_BUILD
printf("Creating new box...\n");
#endif

  mdi->model = NULL;
  }

/* NEW - get solvent from pulldown (don't require manual add) */
mdi->solvent = g_strdup(gui_pd_text(gui_mdi_solvent));

/* copy box into the model coord data array */
if (mdi->model)
  {
  mdi_model_solvate(mdi, mdi->model);

  coords_compute(mdi->model);
  connect_bonds(mdi->model);
  connect_molecules(mdi->model);

  render_refresh(mdi->model);
  }
else
  {
/* setup model */
  model = model_new();
  model->id = MDI;
  model->fractional = TRUE;
  model->periodic = 3;
  model->pbc[0] = mdi->box_dim;
  model->pbc[1] = mdi->box_dim;
  model->pbc[2] = mdi->box_dim;
  model->pbc[3] = 0.5*PI;
  model->pbc[4] = 0.5*PI;
  model->pbc[5] = 0.5*PI;
  mdi->model = model;

  matrix_lattice_init(model);

/* build */
  mdi_model_solvate(mdi, model);

  model_prep(model);

  gui_tree_append_model("MDI", model);

  tree_select_model(model);
  }

/* free */
g_free(mdi->solvent);
g_free(mdi);

gui_refresh(GUI_CANVAS);
}

/********************/
/* MDI input dialog */
/********************/
void gui_mdi_widget(GtkWidget *box)
{
GSList *list;
GtkWidget *left, *vbox, *hbox, *label, *spin;

if (!box)
  return;

left = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), left, TRUE, TRUE, 0);

/* solvent selection */
vbox = gui_frame_vbox("Solvent", FALSE, FALSE, left);
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
list = NULL;
gui_mdi_solvent = gui_pd_new(NULL, NULL, list, hbox);

/* solvation target selection */
vbox = gui_frame_vbox("Solvation target", FALSE, FALSE, left);
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
list = NULL;
list = g_slist_prepend(list, "New box");
list = g_slist_prepend(list, "Current model");
gui_mdi_action = gui_pd_new(NULL, NULL, list, hbox);
g_slist_free(list);

gui_mdi_box_widget = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(vbox), gui_mdi_box_widget, FALSE, FALSE, 0);
label = gtk_label_new("Box size (Angs) ");
gtk_box_pack_start(GTK_BOX(gui_mdi_box_widget), label, FALSE, TRUE, 0);
spin = gui_spin_new(&mdi_length, 2, 99, 1);
gtk_box_pack_end(GTK_BOX(gui_mdi_box_widget), spin, FALSE, TRUE, 0);

// dialog modify callback
g_signal_connect(GTK_OBJECT(gui_mdi_action), "changed", GTK_SIGNAL_FUNC(gui_mdi_widget_update), NULL);
}

/******************************/
/* box generator close signal */
/******************************/
void gui_mdi_dialog_close(GtkWidget *w, gpointer dialog)
{
// TODO - here's where we can lock if threaded 
mdi_project = NULL;

dialog_destroy(NULL, dialog);
}

/********************************/
/* box generator dialog request */
/********************************/
void gui_mdi_dialog(void)
{
gpointer dialog, window;

mdi_project = tree_project_get();
if (!mdi_project)
  {
// TODO - alert - need data
  return;
  }

dialog = dialog_request(MDI, "Dynamics Initializer", NULL, NULL, NULL);
if (!dialog)
  return;

window = dialog_window(dialog);

//gtk_window_set_default_size(GTK_WINDOW(window), -1, 300);

gui_mdi_widget(GTK_DIALOG(window)->vbox);

gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Dynamics Initializer",
                 GTK_DIALOG(window)->action_area);

gui_stock_button("Build", gui_mdi_build, NULL,
                 GTK_DIALOG(window)->action_area);

gui_stock_button(GTK_STOCK_CLOSE, gui_mdi_dialog_close, dialog,
                   GTK_DIALOG(window)->action_area);

gtk_widget_show_all(window);

gui_mdi_solvent_update();
gui_mdi_widget_update(NULL, NULL);
}
