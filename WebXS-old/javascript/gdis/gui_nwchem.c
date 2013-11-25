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

#include "gdis.h"
#include "coords.h"
#include "model.h"
#include "interface.h"
#include "dialog.h"
#include "gui_shorts.h"
#include "import.h"
#include "nwchem.h"

/****************************************/
/* repopulate the basis library display */
/****************************************/
void gui_nwchem_basis_widget_populate(GtkWidget *w, struct nwchem_pak *nwchem)
{
gpointer value;
GList *item, *list;
GtkTreeIter iter;
GtkListStore *list_store;

if (!w)
  return;

/* get list of library elements */
list = g_hash_table_get_keys(nwchem->library);

/* repopulate */
list_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(w)));
gtk_list_store_clear(list_store);
for (item=list ; item ; item=g_list_next(item))
  {
  value = g_hash_table_lookup(nwchem->library, item->data);

  gtk_list_store_append(list_store, &iter);
  gtk_list_store_set(list_store, &iter, 0, item->data, 1, value, -1);
  }
}

/********************************************/
/* panel for displaying basis library items */
/********************************************/
void gui_nwchem_basis_widget(GtkWidget *box, struct nwchem_pak *nwchem)
{
GtkWidget *swin;
GtkWidget *tree_view=NULL;
GtkListStore *list_store=NULL;
GtkCellRenderer *cell;
GtkTreeViewColumn *column;

/* scrolled window for the list */
swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_box_pack_start(GTK_BOX(box), swin, TRUE, TRUE, 0);

list_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin), tree_view);

cell = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes("Element", cell, "text", 0, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
//gtk_tree_view_column_set_expand(column, FALSE);

cell = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes("Basis", cell, "text", 1, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
//gtk_tree_view_column_set_expand(column, FALSE);

gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), TRUE);

/* populate */
gui_nwchem_basis_widget_populate(tree_view, nwchem);
}

/*****************************************/
/* panel for displaying the desired task */
/*****************************************/
void gui_nwchem_task_widget(GtkWidget *box, struct nwchem_pak *nwchem)
{
gint i;
GSList *list;
GtkWidget *hbox;

hbox = gtk_hbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, 0);

/* theory option pulldown */
list = NULL;
for (i=NWCHEM_TASK_SCF ; i<=NWCHEM_TASK_TCE ; i++)
  list = g_slist_prepend(list, nwchem_symbol_table[i]);
list = g_slist_reverse(list);
list = g_slist_prepend(list, "");
gui_pd_new("Theory ", &nwchem->task_theory, list, hbox);
g_slist_free(list);

/* operation options pulldown */
list = NULL;
for (i=NWCHEM_TASK_ENERGY ; i<=NWCHEM_TASK_THERMODYNAMICS ; i++)
  list = g_slist_prepend(list, nwchem_symbol_table[i]);
list = g_slist_reverse(list);
list = g_slist_prepend(list, "");

gui_pd_new("Operation ", &nwchem->task_operation, list, hbox);
g_slist_free(list);

}

/***************************************/
/* panel for displaying the net charge */
/***************************************/
// TODO - combine with other properties?
void gui_nwchem_misc_widget(GtkWidget *box, struct nwchem_pak *nwchem)
{
gui_text_entry("Charge ", &nwchem->charge, TRUE, FALSE, box);
}

/***************************************/
/* main nwchem configuration GUI panel */
/***************************************/
void gui_nwchem_widget(GtkWidget *box, gpointer config)
{
GtkWidget *vbox;
struct nwchem_pak *nwchem;

nwchem = config_data(config);

vbox = gui_frame_vbox("Task", FALSE, FALSE, box);
gui_nwchem_task_widget(vbox, nwchem);
vbox = gui_frame_vbox("Properties", FALSE, FALSE, box);
gui_nwchem_misc_widget(vbox, nwchem);
vbox = gui_frame_vbox("Library", TRUE, TRUE, box);
gui_nwchem_basis_widget(vbox, nwchem);

// TODO - other directives, unprocessed, etc ... 

gtk_widget_show_all(box);
}
