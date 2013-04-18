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

#include "gdis.h"
#include "coords.h"
#include "model.h"
#include "space.h"
#include "graph.h"
#include "select.h"
#include "matrix.h"
#include "project.h"
#include "gui_shorts.h"
#include "interface.h"
#include "dialog.h"
#include "import.h"
#include "gui_image.h"

#include "page.xpm"
#include "methane.xpm"
#include "box.xpm"
#include "surface.xpm"
#include "polymer.xpm"
#include "diamond2.xpm"
#include "graph.xpm"
#include "config.xpm"

/* top level data structure */
extern struct sysenv_pak sysenv;

/**********************/
/* model tree globals */
/**********************/
enum
{
TREE_PIXMAP,
TREE_LABEL,
TREE_ID,
TREE_POINTER,
// TODO - get rid of DATA (TREE_POINTER should be enough)
TREE_DATA,
TREE_NCOLS
};

/*************************/
/* tree search structure */
/*************************/
struct tree_search_pak
{
gpointer search;
GtkTreeRowReference *ref;
};

/*************************/
/* tree search primitive */
/*************************/
gint tree_search(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
gpointer ptr;
struct tree_search_pak *ts = data;

/* get the data pointer at this iter */
gtk_tree_model_get(model, iter, TREE_POINTER, &ptr, -1);
if (ts->search == ptr)
  {
/* this was the only way I found to pass a found row back */
/* NB: iters are ephemeral beasties */
  ts->ref = gtk_tree_row_reference_new(model, path);
  return(TRUE);
  }
return(FALSE);
}

/********************************************/
/* search for a pointer in tree data column */
/********************************************/
/* sets the found iter at the node where data was found (if at all) */
/* NB: the row_reference calls seem to fix problems with emphemeral iters */
gint gui_tree_find(GtkTreeIter *iter, gpointer data)
{
GtkTreePath *path;
struct tree_search_pak ts;

ts.search = data;
ts.ref = NULL;

gtk_tree_model_foreach(GTK_TREE_MODEL(sysenv.tree_store), &tree_search, &ts);

if (ts.ref)
  {
  path = gtk_tree_row_reference_get_path(ts.ref);
  gtk_tree_row_reference_free(ts.ref);
  gtk_tree_model_get_iter(GTK_TREE_MODEL(sysenv.tree_store), iter, path);
  return(TRUE);
  }

return(FALSE);
}

/**********************************************************************/
/* simple wrapper for determining if an object is grafted to the tree */
/**********************************************************************/
gint gui_tree_grafted(gint *id, gpointer data)
{
GtkTreeIter iter;

if (gui_tree_find(&iter, data))
  {
  gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter, TREE_ID, id, -1);
  return(TRUE);
  }

return(FALSE);
}

/*************************************************/
/* attempt to find the parent object in the tree */
/*************************************************/
/* FIXME - a little dangerous as we're not examining the id field */
/* so there is no checking on return type */
/* maybe return parent_import -> check id = TREE_IMPORT ? */
gpointer gui_tree_parent_object(gpointer child)
{
gpointer data=NULL;
GtkTreeIter iter, parent;

g_assert(child != NULL);

if (gui_tree_find(&iter, child))
  {
  if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(sysenv.tree_store), &parent, &iter))
    gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &parent, TREE_POINTER, &data, -1);
  }
return(data);
}

/*************************************************/
/* return parent object (if any) and its ID type */
/*************************************************/
gpointer gui_tree_parent_get(gint *id, gpointer child)
{
gpointer data=NULL;
GtkTreeIter iter, parent;

*id=-1;
if (!child)
  return(NULL);

if (gui_tree_find(&iter, child))
  {
  if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(sysenv.tree_store), &parent, &iter))
    gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &parent,
                       TREE_ID, id, TREE_POINTER, &data, -1);
  }
return(data);
}

/****************************************/
/* rename (change label) of tree object */
/****************************************/
/* TODO - icon change as well? eg 3D -> 0D model (remove periodicity) */
void gui_tree_rename(const gchar *label, gpointer data)
{
GtkTreeIter iter;

if (gui_tree_find(&iter, data))
  gtk_tree_store_set(sysenv.tree_store, &iter, TREE_LABEL, label, -1);
}

/*************************************************************************/
/* return the current selected object (id) and associated pointer (data) */
/*************************************************************************/
gint tree_selected_get(gint *id, gchar **label, gpointer *data)
{
GtkTreeIter iter;
GtkTreeSelection *selection;
GtkTreeModel *treemodel;

*id = -1;
*data = NULL;

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
if (!selection)
  return(FALSE);
if (!gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  return(FALSE);

gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter,
                   TREE_ID, id,
                   TREE_LABEL, label,
                   TREE_POINTER, data,
                   -1);

return(TRUE);
}

/******************************/
/* generic tree object select */
/******************************/
void gui_tree_selected_set(gpointer data)
{
GtkTreeIter iter;
GtkTreeSelection *selection;

if (gui_tree_find(&iter, data))
  {
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
  gtk_tree_selection_select_iter(selection, &iter);
  }
}

/*********************************/
/* return a list of all projects */
/*********************************/
GSList *tree_project_all(void)
{
gpointer data;
GSList *list;
GtkTreeIter iter;

list=NULL;
if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(sysenv.tree_store), &iter))
  {
  do
    {
/* if current iterator is the project - append the import as a new child */
    gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter, TREE_POINTER, &data, -1);
    list = g_slist_prepend(list, data);
    }
  while (gtk_tree_model_iter_next(GTK_TREE_MODEL(sysenv.tree_store), &iter));
  }
return(g_slist_reverse(list));
}

/*************************************************/
/* attempt to get the currently selected project */
/*************************************************/
/* NB: returns the parent project if (eg) a child import is selected */
/* returns NULL on failure */
gpointer tree_project_get(void)
{
/*
gpointer id, ptr;
GtkTreeIter iter, next;
GtkTreeSelection *selection;
GtkTreeModel *treemodel;

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
if (!selection)
  return(NULL);
if (!gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  return(NULL);

while (gtk_tree_model_iter_parent(treemodel, &next, &iter))
  iter = next;

gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter,
                   TREE_ID, &id,
                   TREE_POINTER, &ptr,
                   -1);

if (GPOINTER_TO_INT(id) == TREE_PROJECT)
  return(ptr);

return(NULL);
*/

return(sysenv.workspace);
}

/**************************************************************/
/* return the working directory of the current active project */
/**************************************************************/
/* TODO - completely replace sysenv.cwd with this? */
const gchar *tree_project_cwd(void)
{
gpointer project;

project = tree_project_get();
if (project)
  return(project_path_get(project));

return(sysenv.cwd);
}

/************************************************/
/* attempt to get the currently selected import */
/************************************************/
/* returns NULL on failure (ie an import is not selected) */
gpointer tree_import_get(void)
{
gint type;
gpointer id, ptr;
GtkTreeIter iter;
GtkTreeSelection *selection;
GtkTreeModel *treemodel;

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
if (!selection)
  return(NULL);
if (!gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  return(NULL);

gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter,
                   TREE_ID, &id,
                   TREE_POINTER, &ptr,
                   -1);

/* we're done if selected item is the import object */
if (GPOINTER_TO_INT(id) == TREE_IMPORT)
  return(ptr);
else
  {
/* if not - check parent of selected object */
  ptr = gui_tree_parent_get(&type, ptr);
  if (type == TREE_IMPORT)
    return(ptr);
  }

return(NULL);
}

/***********************************************/
/* attempt to get the currently selected model */
/***********************************************/
/* TODO - replace sysenv.active */
/* TODO - mutex locking based on tree_store for thread safety */
gpointer tree_model_get(void)
{
gpointer id, ptr;
GtkTreeIter iter;
GtkTreeSelection *selection;
GtkTreeModel *treemodel;

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
if (!selection)
  return(NULL);
if (!gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  return(NULL);

gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter,
                   TREE_ID, &id,
                   TREE_POINTER, &ptr,
                   -1);

if (GPOINTER_TO_INT(id) == TREE_MODEL)
  return(ptr);

return(NULL);
}

/**********************************************************************/
/* get a reference to the next iter, assuming current will be deleted */
/**********************************************************************/
/* returns NULL if there is nothing else */
gpointer gui_tree_mark_next(GtkTreeIter *iter)
{
GtkTreeIter next;
GtkTreePath *treepath;
GtkTreeModel *treemodel;
GtkTreeRowReference *mark=NULL;

treemodel = GTK_TREE_MODEL(sysenv.tree_store);
treepath = gtk_tree_model_get_path(treemodel, iter);

/* attempt to locate a new tree item */
if (gtk_tree_path_prev(treepath))
  {
/* success at this treepath */
  mark = gtk_tree_row_reference_new(treemodel, treepath);
  }
else
  {
/* attempt to select next iterator, else - select parent */
  next = *iter;
  if (gtk_tree_model_iter_next(treemodel, &next))
    {
/* success at next iter */
    gtk_tree_path_free(treepath);
    treepath = gtk_tree_model_get_path(treemodel, &next);
    mark = gtk_tree_row_reference_new(treemodel, treepath);
    }
  else
    {
    if (gtk_tree_model_iter_parent(treemodel, &next, iter))
      {
/* success at next iter */
      gtk_tree_path_free(treepath);
      treepath = gtk_tree_model_get_path(treemodel, &next);
      mark = gtk_tree_row_reference_new(treemodel, treepath);
      }
    }
  }

gtk_tree_path_free(treepath);

return(mark);
}

/*****************************************/
/* select the reference to the next iter */
/*****************************************/
void gui_tree_mark_select(gpointer mark)
{
GtkTreeIter iter;
GtkTreeModel *treemodel;
GtkTreePath *treepath;
GtkTreeSelection *selection;

if (mark)
  {
  treepath = gtk_tree_row_reference_get_path(mark);
  treemodel = GTK_TREE_MODEL(sysenv.tree_store);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
  if (gtk_tree_model_get_iter(treemodel, &iter, treepath))
    gtk_tree_selection_select_iter(selection, &iter);
  gtk_tree_row_reference_free(mark);
  }
}

/******************************************/
/* remove all children of the data object */
/******************************************/
/* NB: does not free data - this is up to the caller which knows the parent type */
void gui_tree_remove_children(gpointer data)
{
gint i, n;
GtkTreeIter parent, child;

g_assert(data != NULL);

if (gui_tree_find(&parent, data))
  {
/* iterate from bottom to top */
  n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(sysenv.tree_store), &parent);
  for (i=n ; i-- ; )
    {
/* get child iter and remove */
    if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(sysenv.tree_store), &child, &parent, i))
      gtk_tree_store_remove(sysenv.tree_store, &child);
    }
  }
}

/********************************/
/* replacement deletion routine */
/********************************/
void gui_tree_delete(gpointer object)
{
gint type;
gpointer id, ptr, data, parent_ptr=NULL;
GtkTreeIter iter, parent;

/* get child tree details */
if (gui_tree_find(&iter, object))
  {
  gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter,
                     TREE_ID, &id,
                     TREE_POINTER, &ptr,
                     TREE_DATA, &data,
                     -1);
  }
else
  return;

/* get parent details */
if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(sysenv.tree_store), &parent, &iter))
  {
  gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &parent, TREE_POINTER, &parent_ptr, -1);
  }
else
  return;

g_assert(ptr != NULL);

type = GPOINTER_TO_INT(id);
switch (type)
  {
  case TREE_PROJECT:
    if (project_count(ptr))
      printf("Error: project is in use.\n");
    else
      {
      gtk_tree_store_remove(sysenv.tree_store, &iter);
      project_free(ptr);
      }
    break;

  case TREE_IMPORT:
/* free import references */
      if (import_count(ptr))
        {
printf("Error: import is in use.\n");
        }
      else
        {
        gtk_tree_store_remove(sysenv.tree_store, &iter);
        project_imports_remove(parent_ptr, ptr);
        import_free(ptr);
        }
    break;

  case TREE_MODEL:
/* NEW */
    if (parent_ptr)
      import_object_remove(IMPORT_MODEL, ptr, parent_ptr);
    gtk_tree_store_remove(sysenv.tree_store, &iter);
    model_delete(ptr);
    break;

  case TREE_GRAPH:
/* NEW */
    if (parent_ptr)
      import_object_remove(IMPORT_GRAPH , ptr, parent_ptr);
    gtk_tree_store_remove(sysenv.tree_store, &iter);
    graph_free(ptr);
    break;
  }
}

/********************************/
/* replacement deletion routine */
/********************************/
void tree_select_delete(void)
{
gint type;
gpointer id, ptr, data, parent_ptr=NULL, mark;
GtkTreeIter iter, parent;
GtkTreeSelection *selection;
GtkTreeModel *treemodel;

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
if (!selection)
  return;
if (!gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  return;

/* get child details */
gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter,
                   TREE_ID, &id,
                   TREE_POINTER, &ptr,
                   TREE_DATA, &data,
                   -1);

/* get parent details */
if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(sysenv.tree_store), &parent, &iter))
  {
  gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &parent, TREE_POINTER, &parent_ptr, -1);
  }

g_assert(ptr != NULL);

type = GPOINTER_TO_INT(id);
switch (type)
  {
/*
  case TREE_PROJECT:
    if (project_count(ptr))
      printf("Error: project is in use.\n");
    else
      {
      mark = gui_tree_mark_next(&iter);
      gtk_tree_store_remove(sysenv.tree_store, &iter);
      gui_tree_mark_select(mark);
      project_free(ptr);
      }
    break;
*/

  case TREE_IMPORT:
/* free import references */
      if (import_count(ptr))
        {
printf("Error: import is in use.\n");
        }
      else
        {
        mark = gui_tree_mark_next(&iter);
        gtk_tree_store_remove(sysenv.tree_store, &iter);

//        project_imports_remove(parent_ptr, ptr);
        project_imports_remove(sysenv.workspace, ptr);

        gui_tree_mark_select(mark);
        import_free(ptr);
        }
    break;

  case TREE_MODEL:
/* NEW */
    if (parent_ptr)
      import_object_remove(IMPORT_MODEL, ptr, parent_ptr);

    mark = gui_tree_mark_next(&iter);
    gtk_tree_store_remove(sysenv.tree_store, &iter);
    gui_tree_mark_select(mark);
    model_delete(ptr);
    break;

  case TREE_GRAPH:
/* NEW */
    if (parent_ptr)
      import_object_remove(IMPORT_GRAPH , ptr, parent_ptr);

    mark = gui_tree_mark_next(&iter);
    gtk_tree_store_remove(sysenv.tree_store, &iter);
    gui_tree_mark_select(mark);
    graph_free(ptr);
    break;
  }
}
/**************************/
/* tree traverse function */
/**************************/
gint func2(GtkTreeModel *treemodel, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
GtkTreeSelection *selection;
gpointer ptr;

gtk_tree_model_get(treemodel, iter, TREE_POINTER, &ptr, -1);

if (ptr == data)
  {
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
  gtk_tree_selection_select_iter(selection, iter);
  return(TRUE);
  }
return(FALSE);
}

/**************************/
/* tree traverse function */
/**************************/
gint func(GtkTreeModel *treemodel, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
GtkTreeSelection *selection;
gpointer model;

gtk_tree_model_get(treemodel, iter, TREE_POINTER, &model, -1);

if (model == data)
  {
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
  gtk_tree_selection_select_iter(selection, iter);
  return(TRUE);
  }

return(FALSE);
}

/******************************/
/* select the specified model */
/******************************/
void tree_select_model(struct model_pak *model)
{
if (!model)
  return;

gtk_tree_model_foreach(GTK_TREE_MODEL(sysenv.tree_store), &func, model);
}

/********************/
/* select an import */
/********************/
void tree_project_select(gpointer project)
{
gpointer id, data;
GtkTreeIter iter;
GtkTreeSelection *selection;

if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(sysenv.tree_store), &iter))
  return;

do
  {
  gtk_tree_model_get(GTK_TREE_MODEL(sysenv.tree_store), &iter, TREE_ID, &id, TREE_POINTER, &data, -1);

  if (GPOINTER_TO_INT(id) == TREE_PROJECT)
    {
    if (data == project)
      {
      selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
      gtk_tree_selection_select_iter(selection, &iter);
      }
    }
  }
while (gtk_tree_model_iter_next(GTK_TREE_MODEL(sysenv.tree_store), &iter));
}

/******************************/
/* append a graph to the tree */
/******************************/
void gui_tree_config_append(GtkTreeIter *root, gpointer data)
{
GtkTreeIter branch;
GdkPixbuf *pixbuf;

g_assert(data != NULL);

pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) config_xpm);

gtk_tree_store_append(sysenv.tree_store, &branch, root);

gtk_tree_store_set(sysenv.tree_store, &branch,
                   TREE_PIXMAP, pixbuf, 
                   TREE_LABEL, config_label(data),
                   TREE_ID, GINT_TO_POINTER(TREE_CONFIG),
                   TREE_POINTER, data,
                   TREE_DATA, NULL,
                   -1);

config_grafted_set(TRUE, data);
}

/***************************/
/* select the active model */
/***************************/
void tree_select_active(void)
{
/* TODO */
}

/******************************/
/* append a graph to the tree */
/******************************/
void gui_tree_graph_append(GtkTreeIter *root, gpointer data)
{
GtkTreeIter branch;
GdkPixbuf *pixbuf;

g_assert(data != NULL);

pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) graph_xpm);

gtk_tree_store_append(sysenv.tree_store, &branch, root);

gtk_tree_store_set(sysenv.tree_store, &branch,
                   TREE_PIXMAP, pixbuf, 
                   TREE_LABEL, graph_treename(data),
                   TREE_ID, GINT_TO_POINTER(TREE_GRAPH),
                   TREE_POINTER, data,
                   TREE_DATA, NULL,
                   -1);

graph_set_grafted(TRUE, data);
}

/*******************/
/* refresh a model */
/*******************/
void tree_model_refresh(struct model_pak *model)
{
gboolean flag=FALSE;
GtkTreeIter root, active;
GtkTreeSelection *selection;

/* checks */
if (!model)
  return;
if (!gui_tree_find(&root, model))
  return;

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 

/* update name */
gtk_tree_store_set(sysenv.tree_store, &root, TREE_LABEL, model->basename, -1);

/* NB: have to expand first and THEN select - as by default */
/* appending to the tree store is done in unexpanded fashion */
gtk_tree_view_expand_all(GTK_TREE_VIEW(sysenv.tree));
if (flag)
  gtk_tree_selection_select_iter(selection, &active);
}

/************************************/
/* append a model to the given iter */
/************************************/
void gui_tree_model_append(GtkTreeIter *parent, gpointer data)
{
GtkTreeIter iter;
GdkPixbuf *pixbuf;
struct model_pak *model=data;

g_assert(model != NULL);

switch(model->periodic)
  {
  case 3:
    pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) box_xpm);
    break;
  case 2:
    pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) surface_xpm);
    break;
  case 1:
    pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) polymer_xpm);
    break;
  default:
    pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) methane_xpm);
    break;
  }

gtk_tree_store_append(sysenv.tree_store, &iter, parent);
gtk_tree_store_set(sysenv.tree_store, &iter,
                   TREE_PIXMAP, pixbuf, 
//                   TREE_LABEL, model->label,
                   TREE_LABEL, model->basename,
                   TREE_ID, GINT_TO_POINTER(TREE_MODEL),
                   TREE_POINTER, model,
                   TREE_DATA, NULL,
                   -1);

model->grafted = TRUE;
}

/****************************************************************/
/* add any non-grafted import structures (models) to the import */
/****************************************************************/
void gui_tree_import_refresh(gpointer import)
{
GSList *item, *list=NULL;
GtkTreeIter iter;
gpointer graph, config;
struct model_pak *model;

g_assert(import != NULL);

if (gui_tree_find(&iter, import))
  {

/* add all configs */
  list = import_object_list_get(IMPORT_CONFIG, import);
  for (item=list ; item ; item=g_slist_next(item))
    {
    config = item->data;
    g_assert(config != NULL);
    if (!config_grafted_get(config))
      gui_tree_config_append(&iter, config);
    }

/* add all models */
  list = import_object_list_get(IMPORT_MODEL, import);
  for (item=list ; item ; item=g_slist_next(item))
    {
    model = item->data;
    g_assert(model != NULL);
    if (!model->grafted)
      gui_tree_model_append(&iter, model);
    }

/* add all graphs */
  list = import_object_list_get(IMPORT_GRAPH, import);
  for (item=list ; item ; item=g_slist_next(item))
    {
    graph = item->data;
    g_assert(graph != NULL);
    if (!graph_grafted(graph))
      gui_tree_graph_append(&iter, graph);
    }

  gtk_tree_view_expand_all(GTK_TREE_VIEW(sysenv.tree));
  }
}

/***********************************************/
/* append an import and all models to the tree */
/***********************************************/
void gui_tree_append_import(gpointer import)
{
GSList *item, *list;
GtkTreeIter child1;
GdkPixbuf *pixbuf;
struct model_pak *model;
gpointer graph, config;

//g_assert(parent != NULL);
g_assert(import != NULL);

gtk_tree_store_append(sysenv.tree_store, &child1, NULL);
pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) page_xpm);
gtk_tree_store_set(sysenv.tree_store, &child1,
                   TREE_PIXMAP, pixbuf, 
                   TREE_LABEL, import_label(import),
                   TREE_ID, GINT_TO_POINTER(TREE_IMPORT),
                   TREE_POINTER, import,
                   TREE_DATA, NULL,
                   -1);

/* configurations inside import */
list = import_object_list_get(IMPORT_CONFIG, import);
for (item=list ; item ; item=g_slist_next(item))
  {
  config = item->data;
  g_assert(config != NULL);
  gui_tree_config_append(&child1, config);
  }

/* models inside import */
list = import_object_list_get(IMPORT_MODEL, import);
for (item=list ; item ; item=g_slist_next(item))
  {
  model = item->data;

  g_assert(model != NULL);
  gui_tree_model_append(&child1, model);
  }

/* graphs inside import */
list = import_object_list_get(IMPORT_GRAPH, import);
for (item=list ; item ; item=g_slist_next(item))
  {
  graph = item->data;
  g_assert(graph != NULL);
  gui_tree_graph_append(&child1, graph);
  }

gtk_tree_view_expand_all(GTK_TREE_VIEW(sysenv.tree));
}

/*****************************************/
/* add an new model (auto create import) */
/*****************************************/
gpointer gui_tree_append_model(const gchar *filename, gpointer model)
{
gpointer import;

/* create "wrapper" import */
import = import_new();
import_filename_set(filename, import);

/* add a model if non NULL */
if (!model)
  model = model_new();

import_object_add(IMPORT_MODEL, model, import);

project_imports_add(sysenv.workspace, import);

gui_tree_append_import(import);

return(model);
}

/********************************************/
/* general method of adding to the GUI tree */
/********************************************/
gint gui_tree_append(gpointer parent_data, gint child_id, gpointer child_data)
{
GtkTreeIter iter;

if (gui_tree_find(&iter, parent_data))
  {
  switch (child_id)
    {
    case TREE_IMPORT:
      gui_tree_append_import(child_data);
      return(TRUE);
    case TREE_MODEL:
      gui_tree_model_append(&iter, child_data);
      return(TRUE);
    case TREE_GRAPH:
      gui_tree_graph_append(&iter, child_data);
      return(TRUE);

    default:
printf("gui_tree_append() error, unknown child ID: %d\n", child_id);
    }
  }

return(FALSE);
}

/**************************************/
/* add (or refresh if exists) a model */
/**************************************/
void tree_model_add(struct model_pak *model)
{
GtkTreeIter root;
GdkPixbuf *pixbuf;

/* checks */
g_assert(model != NULL);
if (model->grafted)
  {
  tree_model_refresh(model);
  return;
  }

/* graft the model */
gtk_tree_store_append(sysenv.tree_store, &root, NULL);
model->grafted = TRUE;

/* setup pixmap */
if (model->id == MORPH)
  pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) diamond2_xpm);
else
  {
  switch(model->periodic)
    {
    case 3:
      pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) box_xpm);
      break;
    case 2:
      pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) surface_xpm);
      break;
    case 1:
      pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) polymer_xpm);
      break;
    default:
      pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **) methane_xpm);
      break;
    }
  }

/* set the parent iterator data */
gtk_tree_store_set(sysenv.tree_store, &root,
                   TREE_PIXMAP, pixbuf, 
                   TREE_LABEL, model->basename,
                   TREE_ID, GINT_TO_POINTER(TREE_MODEL),
                   TREE_POINTER, model,
                   TREE_DATA, NULL,
                   -1);

canvas_shuffle();
}

/********************************/
/* handle tree selection events */
/********************************/
#define DEBUG_TREE_SELECTION_CHANGED 0
void tree_selection_changed(GtkTreeSelection *selection, gpointer data)
{
gpointer id, ptr;
GtkTreeIter iter;
GtkTreeModel *treemodel;

if (gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  {
  gtk_tree_model_get(treemodel, &iter, TREE_ID, &id, TREE_POINTER, &ptr, -1);

  switch (GPOINTER_TO_INT(id))
    {
    case TREE_IMPORT:
      if (ptr)
        gui_import_select(ptr);
      break;

    case TREE_CONFIG:
      if (ptr)
        gui_config_select(ptr);
      break;

    case TREE_MODEL:
      if (ptr)
        gui_model_select(ptr);
      break;

    case TREE_GRAPH:
      gui_display_set(TREE_GRAPH);
      gui_refresh(GUI_ACTIVE | GUI_CANVAS);
      break;
    }
  }
else
  {
/* empty selection - nothing loaded - display a blank canvas */
  gui_display_set(TREE_MODEL);
  gui_refresh(GUI_CANVAS);
  }
}

/**************************************************************/
/* re-select the currently selected item to trigger a refresh */
/**************************************************************/
void tree_select_refresh(void)
{
GtkTreeSelection *select;

/*
selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree)); 
if (!selection)
  return(NULL);
if (!gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  return(NULL);
if (gtk_tree_model_iter_parent(treemodel, &next, &iter))
  iter = next;
*/

select = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree));

/* emit a "change" signal */

g_signal_emit_by_name(select, "changed", select, NULL);

}

/********************************************/
/* data structures for tree selection stack */
/********************************************/
// refs to actual object selected ... (and parent?)
// if not selectable - get # offset to select
struct tree_select_pak
{
gint id;
gint index;
gpointer data;
gpointer parent;
};

GSList *tree_stack = NULL;

#define TREE_STACK_DEBUG 0

/********************************************************************/
/* primitives for converting from tree object to import object id's */
/********************************************************************/
gint tree_index_get(struct tree_select_pak *ts)
{
gint import_id=-1;
GSList *list;

switch (ts->id)
  {
  case TREE_CONFIG:
    import_id = IMPORT_CONFIG;
    break;
  case TREE_GRAPH:
    import_id = IMPORT_GRAPH;
    break;
  case TREE_MODEL:
    import_id = IMPORT_MODEL;
    break;

  default:
    printf("tree_index_set(): unexpected tree structure error.\n");
  }

list = import_object_list_get(import_id, ts->parent);

return(g_slist_index(list, ts->data));
}

/********************************************************************/
/* primitives for converting from tree object to import object id's */
/********************************************************************/
gpointer tree_object_get(struct tree_select_pak *ts)
{
gint import_id=-1;
GSList *list;

switch (ts->id)
  {
  case TREE_CONFIG:
    import_id = IMPORT_CONFIG;
    break;
  case TREE_GRAPH:
    import_id = IMPORT_GRAPH;
    break;
  case TREE_MODEL:
    import_id = IMPORT_MODEL;
    break;

  default:
    printf("tree_object_get(): unexpected tree structure error.\n");
  }

list = import_object_list_get(import_id, ts->parent);

return(g_slist_nth_data(list, ts->index));
}

/*************************************/
/* store the current selection state */
/*************************************/
// NB: should be robust enough to withstand import/model/etc deletion
//gint gui_tree_find(GtkTreeIter *iter, gpointer data)
//gint tree_selected_get(gint *id, gchar **label, gpointer *data)
void gui_tree_selection_push(void)
{
gint id;
gchar *label;
gpointer data;
struct tree_select_pak *ts;

if (tree_selected_get(&id, &label, &data))
  {
  ts = g_malloc(sizeof(struct tree_select_pak));

  ts->id = id;
  ts->index = -1;
  ts->data = data;

  switch (ts->id)
    {
    case TREE_IMPORT:
      ts->parent = NULL;
      break;

    default:
      ts->parent = gui_tree_parent_get(&id, ts->data);
      if (id != TREE_IMPORT)
        fprintf(stderr, "gui_tree_selection_push(): unexpected tree structure error.\n");
/* record the child's position index within the parent imports object list */
      ts->index = tree_index_get(ts);
    }

#if TREE_STACK_DEBUG
printf("push: [id=%d] [pointer=%p] [index=%d] [parent=%p]\n",
       ts->id, ts->data, ts->index, ts->parent);
#endif

  tree_stack = g_slist_prepend(tree_stack, ts);
  }
}

/*************************************/
/* retrieve the current selection state */
/*************************************/
// NB: should be robust enough to withstand import/model/etc deletion
// ie make a best guess if deleted or import refresh occured
void gui_tree_selection_pop(void)
{
gpointer child;
GtkTreeIter iter;
struct tree_select_pak *ts;

if (tree_stack)
  {
  ts = tree_stack->data;

  tree_stack = g_slist_remove(tree_stack, ts);

#if TREE_STACK_DEBUG
printf("pop: [id=%d] [pointer=%p] [index=%d] [parent=%p]\n",
       ts->id, ts->data, ts->index, ts->parent);
#endif

  if (gui_tree_find(&iter, ts->data))
    {
// select data
    gui_tree_selected_set(ts->data);
    }
  else
    {
    if (gui_tree_find(&iter, ts->parent))
      {
// otherwise - find another item of same id (and position) within parent
      child = tree_object_get(ts);
      if (child)
        {
// child of same id and index within parent
        gui_tree_selected_set(child);
        }
      }

// TODO - fallback if all else fails eg select previous/next

    }

/* force a selection changed event */
  tree_select_refresh();
  g_free(ts);
  }
}

/**********************/
/* clear the GUI tree */
/**********************/
void gui_tree_clear(void)
{
gtk_tree_store_clear(sysenv.tree_store);
}

/*********************************/
/* create the model viewing pane */
/*********************************/
void tree_init(GtkWidget *box)
{
GtkWidget *swin;
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *select;

/* scrolled window for the model pane */
swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_AUTOMATIC,
                               GTK_POLICY_AUTOMATIC);
gtk_container_add(GTK_CONTAINER(box), swin);

/* underlying data storage */
sysenv.tree_store = gtk_tree_store_new(TREE_NCOLS,
                                       GDK_TYPE_PIXBUF,
                                       G_TYPE_STRING,
                                       G_TYPE_POINTER,
                                       G_TYPE_POINTER,
                                       G_TYPE_POINTER);
/* actual tree widget */
sysenv.tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(sysenv.tree_store));
gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin), sysenv.tree);

/* setup the model pixmap rendering colum */
renderer = gtk_cell_renderer_pixbuf_new();
column = gtk_tree_view_column_new_with_attributes("image", renderer,
                                                  "pixbuf", TREE_PIXMAP, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(sysenv.tree), column);

/* setup the model name rendering colum */
renderer = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes("name", renderer,
                                                  "text", TREE_LABEL, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(sysenv.tree), column);

/* setup the selection handler */
select = gtk_tree_view_get_selection(GTK_TREE_VIEW(sysenv.tree));
gtk_tree_selection_set_mode(select, GTK_SELECTION_BROWSE);
g_signal_connect(G_OBJECT(select), "changed",
                 G_CALLBACK(tree_selection_changed),
                 NULL);

gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(sysenv.tree), FALSE);
gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(sysenv.tree), TRUE);
/* FIXME - ideally, want indendation = width of the icons used in the model tree */
gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(sysenv.tree), 0);

gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(sysenv.tree), TRUE);

gtk_widget_show(swin);
}

