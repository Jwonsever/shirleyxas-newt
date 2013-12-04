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
#include <time.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "gdis.h"
#include "coords.h"
#include "edit.h"
#include "file.h"
#include "parse.h"
#include "import.h"
#include "graph.h"
#include "task.h"
#include "morph.h"
#include "model.h"
#include "module.h"
#include "matrix.h"
#include "numeric.h"
#include "measure.h"
#include "project.h"
#include "render.h"
#include "select.h"
#include "space.h"
#include "sginfo.h"
#include "spatial.h"
#include "opengl.h"
#include "quaternion.h"
#include "surface.h"
#include "gui_shorts.h"
#include "interface.h"
#include "dialog.h"
#include "zmatrix.h"
#include "gui_image.h"
#include "undo.h"
#include "grid.h"
#include "grisu_client.h"
#include "animate.h"
#include "gui_project.h"

#include "logo_left.xpm"
#include "logo_right.xpm"

/* top level data structure */
extern struct sysenv_pak sysenv;

extern GMutex *gdk_threads_mutex;

/* bit messy - put some of these in sysenv? */
/* main window */
GtkWidget *window;
/* backing pixmap */
GdkPixmap *pixmap = NULL;
/* model pane stuff */
GtkWidget *scrolled_window, *clist;
/* gdisrc? */
gint pane_width=65;
/* current viewing mode combo box */
GtkWidget *viewing_mode;

GtkWidget *text_list_buffer;
GtkWidget *entry_import_name, *entry_import_path;
GtkWidget *entry_duplicate_filter;
GtkWidget *vbox_project_active;
GtkWidget *vbox_model_active;
GtkWidget *vbox_model_select;
GtkWidget *gui_task_current=NULL;
GtkWidget *gui_task_summary=NULL;

/* alternate text widgets */
GtkWidget *text_view;
GtkWidget *text_import_view;
GtkWidget *text_project_view;
GtkWidget *text_config_view=NULL;
GtkWidget *gui_task_box, *gui_text_box;
gpointer gui_text_buffer=NULL;

GtkWidget *gui_text_swin, *gui_text_current, *gui_text_summary;
GtkWidget *gui_task_button, *gui_text_button;
GtkWidget *gui_graph_vbox;

gint gui_display_mode=TREE_MODEL;

/*******************************/
/* modal binary message dialog */
/*******************************/
gint gui_popup_message(const gchar *text)
{
gint button;
GtkWidget *dialog;

/* checks */
if (!text)
  return(FALSE);

/* create popup with message */
dialog = gtk_message_dialog_new(NULL,
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_WARNING,
                                GTK_BUTTONS_CLOSE,
                                text, NULL);

button = gtk_dialog_run(GTK_DIALOG(dialog));
gtk_widget_destroy(dialog);

/* yes response */
if (button == -8)
  return(TRUE);

return(FALSE);
}

/************************************/
/* modal binary confirmation dialog */
/************************************/
gint gui_popup_confirm(const gchar *text)
{
gint button;
GtkWidget *dialog;

/* checks */
if (!text)
  return(FALSE);

/* create popup with message */
dialog = gtk_message_dialog_new(NULL,
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_WARNING,
                                GTK_BUTTONS_YES_NO,
                                text, NULL);

button = gtk_dialog_run(GTK_DIALOG(dialog));
gtk_widget_destroy(dialog);

switch (button)
  {
  case GTK_RESPONSE_YES:
    return(1);

  case GTK_RESPONSE_NO:
    return(0);
  }

// other than yes/no - should only ever get the "deleted" response 
// which can be interpreted as cancel if desired
return(-1);
}

/********************************/
/* set main canvas display item */
/********************************/
/* default is openGL window */
/* TODO - need more options ie project_view, import_view etc -> sets visibilty up */
void gui_display_set(gint type)
{
gpointer model=NULL;

gui_display_mode = type;

switch (type)
  {
/* opengl */
  case TREE_MODEL:
    gtk_widget_hide(text_view);
    gtk_widget_hide(gui_graph_vbox);
    gtk_widget_show(sysenv.glarea);
    model = tree_model_get();
    if (model)
      {
      gtk_widget_show(vbox_model_active);
      gtk_widget_show(vbox_model_select);
      }
    else
      {
      gtk_widget_hide(vbox_model_active);
      gtk_widget_hide(vbox_model_select);
      }
    break;

  case TREE_GRAPH:
    gtk_widget_hide(text_view);
    gtk_widget_show(sysenv.glarea);
    gtk_widget_show(gui_graph_vbox);
    gtk_widget_hide(vbox_model_active);
    gtk_widget_hide(vbox_model_select);
    break;

// this is deprec
  case TREE_PROJECT:
/*
    gtk_widget_hide(sysenv.glarea);
    gtk_widget_hide(vbox_model_active);
    gtk_widget_hide(vbox_model_select);
    gtk_widget_hide(text_import_view);
    if (GTK_IS_WIDGET(text_config_view))
      gtk_widget_hide(text_config_view);
    gtk_widget_show(text_project_view);
    gtk_widget_show(text_view);
*/
    break;

  case TREE_IMPORT:
    gtk_widget_hide(sysenv.glarea);
    gtk_widget_hide(gui_graph_vbox);
    gtk_widget_hide(vbox_model_active);
    gtk_widget_hide(vbox_model_select);
//    gtk_widget_hide(text_project_view);
    if (GTK_IS_WIDGET(text_config_view))
      gtk_widget_hide(text_config_view);
    gtk_widget_show(text_import_view);
    gtk_widget_show(text_view);
    break;

  case TREE_CONFIG:
    gtk_widget_hide(sysenv.glarea);
    gtk_widget_hide(gui_graph_vbox);
    gtk_widget_hide(vbox_model_active);
    gtk_widget_hide(vbox_model_select);
//    gtk_widget_hide(text_project_view);
    gtk_widget_hide(text_import_view);
    gtk_widget_show(text_config_view);
    gtk_widget_show(text_view);
    break;
  }
}

gint gui_display_get(void)
{
return(gui_display_mode);
}

/**********************************************/
/* select particular GUI elements for display */
/**********************************************/
/* deprec. */
void gui_widget_active(gint name)
{
}

/******************************************************/
/* generate a new preview of the import (if possible) */
/******************************************************/
void gui_import_preview(gpointer import)
{
g_assert(import != NULL);

import_preview_set(TRUE, import);
import_file_save(import);
gui_scrolled_text_set(text_list_buffer, import_preview_buffer_get(import));
import_preview_set(FALSE, import);
}

/*******************************************************/
/* change an import path to match current project path */
/*******************************************************/
/* NEW - changes path, but doesnt save */
void gui_import_move(GtkWidget *w, gpointer dummy)
{
gint id;
const gchar *path, *name;
gchar *fullpath, *label;
gpointer import, project;

/* we require an import to be selected */
if (tree_selected_get(&id, &label, &import))
  {
  if (id != TREE_IMPORT)
    {
    printf("No import\n");
    return;
    }

/* get project the current selection belongs to */
  project = tree_project_get();
  if (!project)
    {
    printf("No project\n");
    return;
    }

  path = project_path_get(project);
  name = import_filename(import);

  fullpath = g_build_filename(path, name, NULL); 
  import_fullpath_set(fullpath, import);
  g_free(fullpath);
  gtk_entry_set_text(GTK_ENTRY(entry_import_path), path);

/* TODO - if save is desired as well -> call gui_selected_save() */

  }
}

/*****************************************************/
/* duplicate an import and convert to specified type */
/*****************************************************/
/* NB: supply w=NULL if the exported file should not be reloaded */
#define DEBUG_GUI_EXPORT 0
void gui_import_export(GtkWidget *w, gpointer dummy)
{
gint id;
gchar *fullpath, *name, *ext, *label, *msg;
gpointer import;
const gchar *text;
struct file_pak *type=NULL;

text = gtk_entry_get_text(GTK_ENTRY(entry_duplicate_filter));

#if DEBUG_GUI_EXPORT
printf("write copy as: %s\n", text);
#endif

type = file_type_from_label(text);
if (!type)
  {
  msg = g_strdup_printf("Can't export as type [%s]\n", text);
  gui_popup_message(msg);
  g_free(msg);
  return;
  }
else
  {
  if (type->ext)
    ext = (type->ext)->data;
  else
    {
    msg = g_strdup_printf("Can't export as type [%s]\n", text);
    gui_popup_message(msg);
    g_free(msg);
    return;
    }
  }

#if DEBUG_GUI_EXPORT
printf("new extension: %s\n", ext);
#endif

/* we require an import to be selected */
if (tree_selected_get(&id, &label, &import))
  {
  if (id != TREE_IMPORT)
    {
    printf("No import\n");
    return;
    }
  }

/* CURRENT - dont give the same name in case user wants to duplicate the same type */
/* create filename - currently = source with new extension */
/* create new import of desired type for saving */

name = parse_extension_set(label, ext);
fullpath = g_build_filename(tree_project_cwd(), name, NULL);

#if DEBUG_GUI_EXPORT
printf("Save as: %s\n", fullpath);
#endif

/* TODO - display fullpath + warn if exists */
msg = g_strdup_printf("Create file: %s ?", fullpath);
if (gui_popup_confirm(msg))
  {
  import_save_as(fullpath, import);

/* GUI triggered - load exported file */
  if (w)
    project_import_file(tree_project_get(), fullpath);
  }

g_free(fullpath);
g_free(name);
g_free(msg);
}

/************************************************/
/* callback to handle chunk data edited by user */
/************************************************/
void gui_text_changed(GtkWidget *w, gpointer data)
{
gint i;
GtkTextBuffer *buffer;

buffer = GTK_TEXT_BUFFER(w);

i = gtk_text_buffer_get_line_count(buffer);
//printf("total lines : %d\n", i);
}

/***************************************************/
/* name text entry project/import "enter" callback */
/***************************************************/
void gui_name_handler(GtkWidget *w, gpointer dummy)
{
gint id;
gchar *label;
gpointer data;
const gchar *text;

text = gtk_entry_get_text(GTK_ENTRY(w));

if (tree_selected_get(&id, &label, &data))
  {
  switch (id)
    {
    case TREE_IMPORT:
      import_filename_set(text, data);
      gui_tree_rename(text, data);
      break;
    }
  }
}

/****************************************/
/* relocate the import to project's cwd */
/****************************************/
void gui_import_path_handler(GtkWidget *w, gpointer dummy)
{
gint id, reset=FALSE;
const gchar *text;
gchar *label, *filename, *fullpath;
gpointer data;

text = gtk_entry_get_text(GTK_ENTRY(w));
if (!g_file_test(text, G_FILE_TEST_IS_DIR))
  reset=TRUE;
else
  reset=FALSE;

if (tree_selected_get(&id, &label, &data))
  {
  switch (id)
    {
    case TREE_IMPORT:
      if (reset)
        {
/* bad path - set back to orig import path */
        gtk_entry_set_text(GTK_ENTRY(entry_import_path), import_path(data));
        }
      else
        {
/* change import path */
        filename = import_filename(data);
        fullpath = g_build_filename(text, filename, NULL);

// if exists, prompt for overwrite else plain move
        if (g_file_test(fullpath, G_FILE_TEST_EXISTS))
          label = g_strdup_printf("Overwrite %s at new path?", filename);
        else
          label = g_strdup_printf("Move %s to new path?", filename);

        if (gui_popup_confirm(label) == 1)
          {
          import_fullpath_set(fullpath, data);
          sysenv.workspace_changed = TRUE;
// save
          import_file_save(data);
          }
        else
          gtk_entry_set_text(GTK_ENTRY(entry_import_path), import_path(data));

        g_free(fullpath);
        g_free(label);
        }
      break;
    }
  }
}

/**********************/
/* import view widget */
/**********************/
void gui_import_widget(GtkWidget *box)
{
GtkWidget *vbox, *hbox, *label, *button;

if (!box)
  return;

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);

/* left */
vbox = gtk_vbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

label = gtk_label_new("File path ");
gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

/*
label = gtk_label_new("File name ");
gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
*/


/* right */
vbox = gtk_vbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);


entry_import_path = gtk_entry_new();
gtk_entry_set_editable(GTK_ENTRY(entry_import_path), FALSE);
gtk_box_pack_start(GTK_BOX(vbox), entry_import_path, TRUE, TRUE, 0);
g_signal_connect(GTK_OBJECT(entry_import_path), "activate",
                 GTK_SIGNAL_FUNC(gui_import_path_handler), NULL);

/*
hbox = gtk_hbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
*/


/* CURRENT */
/*
button = gtk_button_new_with_label("Move to Working Directory");
gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
g_signal_connect(G_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(gui_import_move_cwd), NULL);
*/

// TODO - only show basename
/*
entry_import_name = gtk_entry_new();
gtk_entry_set_editable(GTK_ENTRY(entry_import_name), TRUE);
gtk_box_pack_start(GTK_BOX(hbox), entry_import_name, TRUE, TRUE, 0);

g_signal_connect(GTK_OBJECT(entry_import_name), "activate",
                 GTK_SIGNAL_FUNC(gui_name_handler), NULL);
*/


/* IMPORT VIEW */
/*
hbox2 = gtk_hbox_new(FALSE, 0);
gtk_box_pack_end(GTK_BOX(hbox), hbox2, FALSE, FALSE, 0);
*/

//gui_label_button("Show content", gui_import_preview, NULL, hbox);
//gui_label_button("Save to current directory", gui_import_save, NULL, hbox);
//gui_label_button("Move to project path", gui_import_move, NULL, hbox);

/* CURRENT */
entry_duplicate_filter = gui_file_filter(NULL, hbox);
button = gtk_button_new_with_label("Convert to");
gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
g_signal_connect(G_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(gui_import_export), NULL);


/* separator - chunk data display */
gtk_box_pack_start(GTK_BOX(box), gtk_hseparator_new(), FALSE, FALSE, 0);

/* text buffer for chunk data of unknown/default type */
/* TODO - a widget (eg basis set editor etc) if chunk data is of suitable type */
text_list_buffer = gui_scrolled_text_new(TRUE, box);

/* trigger text update on when user finishes doing something */
g_signal_connect(G_OBJECT(text_list_buffer), "end-user-action",
                 GTK_SIGNAL_FUNC(gui_text_changed), NULL);
}

/****************************************/
/* set up the main text display widgets */
/****************************************/
void gui_text_init(GtkWidget *box)
{
/* IMPORT */
text_import_view = gtk_vbox_new(FALSE, PANEL_SPACING);
gui_import_widget(text_import_view);
gtk_box_pack_start(GTK_BOX(box), text_import_view, TRUE, TRUE, 0);
}

/*****************************************************/
/* setup the text display to reflect selected import */
/*****************************************************/
void gui_import_select(gpointer import)
{
const gchar *text;

g_assert(import != NULL);

gui_display_set(TREE_IMPORT);

/* displaying name is deprecated */
/*
text = import_filename(import);
if (text)
  gtk_entry_set_text(GTK_ENTRY(entry_import_name), text);
else
  gtk_entry_set_text(GTK_ENTRY(entry_import_name), "");
*/

text = import_path(import);
if (text)
  gtk_entry_set_text(GTK_ENTRY(entry_import_path), text);
else
  gtk_entry_set_text(GTK_ENTRY(entry_import_path), "");


//gtk_widget_hide(text_project_view);
gtk_widget_show(text_import_view);


/*
gtk_widget_set_sensitive(GTK_WIDGET(label_remote_path), FALSE);
gtk_widget_set_sensitive(GTK_WIDGET(entry_remote_path), FALSE);
*/

/* TODO - only show if not too big ... */
//gui_scrolled_text_set(text_list_buffer, "");
gui_import_preview(import);
}

/********************************************/
/* a configuration was selected on the tree */
/********************************************/
void gui_config_select(gpointer config)
{
gint id;
gpointer data;
GtkWidget *label;

g_assert(config != NULL);

id = config_id(config);
data = config_data(config);

/* destroy old config dialog */
if (text_config_view)
  gtk_widget_destroy(text_config_view);

/* create a new one */
text_config_view = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(text_view), text_config_view, TRUE, TRUE, 0);

switch (id)
  {
  case FDF:
    gui_siesta_widget(text_config_view, config);
    break;

  case GULP:
    gui_gulp_widget(text_config_view, data);
    break;

  case GAMESS:
    gui_gamess_widget(text_config_view, data);
    break;

  case NWCHEM:
    gui_nwchem_widget(text_config_view, config);
    break;

  default:
    label = gtk_label_new("No GUI element for this configuration");
    gtk_box_pack_start(GTK_BOX(text_config_view), label, TRUE, TRUE, 0);
  }

/* hide other text widgets */
gui_display_set(TREE_CONFIG);

/* show config widget (and all children) */
gtk_widget_show_all(text_config_view);
}

/*****************************/
/* schedule widget update(s) */
/*****************************/
void gui_refresh(gint type)
{
if (type & GUI_ACTIVE)
  sysenv.refresh_properties = TRUE;
if (type & GUI_PREVIEW)
  sysenv.refresh_text = TRUE;
if (type & GUI_TREE)
  sysenv.refresh_tree = TRUE;
if (type & GUI_CANVAS)
  redraw_canvas(SINGLE);
}

/************************/
/* EVENT - button press */
/************************/
#define DEBUG_BUTTON_PRESS_EVENT 0
gint gui_press_event(GtkWidget *w, GdkEventButton *event)
{
gint refresh=0, x, y;
gint shift=FALSE, ctrl=FALSE;
GdkModifierType state;
struct model_pak *data;
struct bond_pak *bond;
struct core_pak *core;

/* HACK */
if (sysenv.stereo)
  return(FALSE);

/* get model */
data = tree_model_get();
if (!data)
  return(FALSE);

/* event info */
x = event->x;
y = event->y;
sysenv.moving = FALSE;

canvas_select(x, y);

/* analyse the current state */
state = (GdkModifierType) event->state;
if ((state & GDK_SHIFT_MASK))
  shift = TRUE;
if ((state & GDK_CONTROL_MASK))
  ctrl = TRUE;

/* only want button 1 (for now) */
if (event->button != 1)
  return(FALSE);

/* TODO - alternate tree_select to get graph */
/*
if (data->graph_active)
  {
  diffract_select_peak(x, y, data);
  return(FALSE);
  }
*/

/* can we assoc. with a single atom */
core = gl_seek_core(x, y, data);

/* allow shift+click to add/remove single atoms in the selection */
/* TODO - depending on mode add/remove objects in selection eg atom/mols */
if (shift)
  {
  if (core)
    {
    select_core(core, TRUE, data);
    refresh++;
    }
  else
    {
/* otherwise start a box selection */
    update_box(x,y,data,START);
    }
  }
else
  {
/* determine the type of action required */
  switch(data->mode)
    {
    case DIST_INFO:
      measure_dist_event(core, data);
      break;
    case BOND_INFO:
      measure_bond_event(x,y,data);
      break;
    case ANGLE_INFO:
      measure_angle_event(core, data);
      break;
    case DIHEDRAL_INFO:
      measure_torsion_event(core, data);
      break;

    case BOND_DELETE:
/* or by bond midpoints */
      bond = gl_seek_bond(x,y,data);
      if (bond)
        {
        connect_user_bond(bond->atom1, bond->atom2, BOND_DELETE, data);
        refresh++;
        }
      break;

    case BOND_SINGLE:
    case BOND_DOUBLE:
    case BOND_TRIPLE:
      if (core)
        {
        connect_make_bond(core, data->mode, data); 
        refresh++;
        }
      break;

    case ATOM_ADD:
      add_atom(x, y, data);
      refresh++; 
      break;

    case DEFINE_RIBBON:
      if (core)
        construct_backbone(core, data);
      break;

    case DEFINE_VECTOR:
    case DEFINE_PLANE:
      if (core)
        spatial_point_add(core, data);
      break;

/* selection stuff */
    default:
      select_clear(data);
      if (core)
        select_core(core, TRUE, data);
      else
        update_box(x,y,data,START);
      refresh++;
      break; 
    }
  }

if (refresh)
  {
  gui_active_refresh();
/* CURRENT */
  gui_refresh_selection();
  }

return(FALSE);
}

/***************************/
/* EVENT - button released */
/***************************/
gint gui_release_event(GtkWidget *w, GdkEventButton *event)
{
struct model_pak *model;

if (sysenv.stereo)
  return(FALSE);

/* get model */
model = tree_model_get();
if (!model)
  return(FALSE);

/* first mouse button */
if (model->box_on)
  {
  gl_select_box(w);
  model->box_on = FALSE;
  }

gui_refresh_selection();

sysenv.moving = FALSE;

switch (model->mode)
  {
  case ATOM_H_ADD:
    if (event->button == 1)
      select_add_hydrogens(model);
    break;
  }

gui_refresh(GUI_CANVAS);

return(FALSE);
}

/************************/
/* EVENT - mouse scroll */
/************************/
gint gui_scroll_event(GtkWidget *w, GdkEventScroll *event)
{
/* change zoom -- based on "zoom section" of gui_press_event() */
const gdouble scroll_factor = 10.0;
gdouble scroll, v[3];
struct model_pak *data;
struct camera_pak *camera;

/* get model */
data = tree_model_get();
if (!data)
  return(FALSE);

camera = data->camera;

switch (event->direction)
{
  case GDK_SCROLL_UP:
    scroll = -scroll_factor;
    break;
  case GDK_SCROLL_DOWN:
    scroll = scroll_factor;
    break;
  default: /* left and right are not used yet */
    return FALSE;
}
scroll *= PIX2SCALE;

if (camera->perspective)
  {
  ARR3SET(v, camera->v);
  VEC3MUL(v, -scroll*10.0);
  ARR3ADD(camera->x, v);
  }
else
  camera->zoom += scroll;

data->zoom = data->rmax;

gui_camera_refresh();

sysenv.moving = FALSE;

redraw_canvas(SINGLE);
return FALSE;
}

/************************/
/* EVENT - mouse motion */
/************************/
#define DEBUG_MOTION 0
gint gui_motion_event(GtkWidget *w, GdkEventMotion *event)
{
gint x, y, dx, dy, fx, fy, refresh;
gint shift=FALSE, ctrl=FALSE;
gdouble da, dv, zoom, v[3], mat[9];
GdkModifierType state;
static gint ox=0, oy=0;
struct model_pak *data;
struct camera_pak *camera;

/* get model */
data = tree_model_get();
if (!data)
  return(FALSE);

camera = data->camera;

/* get mouse data */
if (event->is_hint)
  gdk_window_get_pointer(event->window, &x, &y, &state);
else
  {
/* MS windows apparently reaches this */
  x = event->x;
  y = event->y;
  state = (GdkModifierType) event->state;
  }

/* discard discontinuous jumps (ie ox,oy are invalid on 1st call) */
/*
if (!sysenv.moving)
  {
  ox = x;
  oy = y;
  sysenv.moving = TRUE;
  return(TRUE);
  }
*/

/* convert relative mouse motion to an increment */
dx = x-ox;
dy = oy-y;        /* inverted y */

/* single update */
refresh=0;

/* analyse the current state */
if ((state & GDK_SHIFT_MASK))
  shift = TRUE;
if ((state & GDK_CONTROL_MASK))
  ctrl = TRUE;

/* first mouse button - mostly selection stuff */
if (state & GDK_BUTTON1_MASK)
  {
  switch(data->mode)
    {
    case FREE:
      update_box(x, y, data, UPDATE);
      refresh++;
      break;

    default:
      break;
    }
/* don't switch to low quality drawing */
  sysenv.moving = FALSE;
  if (refresh)
    redraw_canvas(SINGLE);
  ox=x;
  oy=y;
  return(TRUE);
  }

/* second mouse button */
if (state & GDK_BUTTON2_MASK)
  {
  if (shift)
    {
/* zoom */
    zoom = PIX2SCALE * (dx+dy);
    if (camera->perspective)
      {
      ARR3SET(v, camera->v);
      VEC3MUL(v, zoom*10.0);
      ARR3ADD(camera->x, v);
      }
    else
      {
      camera->zoom -= zoom;
      data->zoom = data->rmax;
      }
    sysenv.moving = TRUE;
    refresh++;
    }
  else
    {
    if (ctrl)
      {
/* selection only translation */
      select_translate(x-ox, y-oy, data);
      sysenv.moving = TRUE;
      refresh++;
      }
    else
      {
/* horizontal camera translation */
      dv = ox-x;
      ARR3SET(v, camera->e);
      VEC3MUL(v, dv*PIX2ANG);
      ARR3ADD(camera->x, v);
/* vertical  camera translation */
      dv = y-oy;
      ARR3SET(v, camera->o);
      VEC3MUL(v, dv*PIX2ANG);
      ARR3ADD(camera->x, v);

      sysenv.moving = TRUE;
      refresh++;
      }
    }
  }

/* third mouse button clicked? */
if (state & GDK_BUTTON3_MASK)
  {
/* shift clicked? */
  if (shift)
    {
/* yaw */
    if (dx || dy)
      {
/* rotation amount */
      da = abs(dx) + abs(dy);
/* vector from center to mouse pointer (different for OpenGL window) */
/* FIXME - if ctrl is pressed, the center should be the selection */
/* centroid & not the middle of the window */
      fx = x - data->offset[0] - (sysenv.x + sysenv.width/2);
      fy = data->offset[1] + (sysenv.y + sysenv.height/2 - y);

/* rotation direction via z component of cross product (+=clock, -=anti) */
      if ((fx*dy - dx*fy) < 0)
        da *= -1.0;

#if DEBUG_MOTION
printf("(%d,%d) x (%d,%d) : %f\n",dx,dy,fx,fy, da);
#endif

/* assign and calculate */
      da *= D2R * ROTATE_SCALE;
      if (ctrl)
        {
        matrix_relative_rotation(mat, da, ROLL, data);
        rotate_select(data, mat);
        }
      else
        {
        if (camera->mode == LOCKED)
          quat_concat_euler(camera->q, ROLL, da);
        else
          {
          matrix_v_rotation(mat, camera->v, -da);
          vecmat(mat, camera->o);
          vecmat(mat, camera->e);
          }
        }
      }
    sysenv.moving = TRUE;
    refresh++;
    }
  else
    {
#if DEBUG_MOTION
printf("(%d,%d)\n",dx,dy);
#endif
/* pitch and roll */
    if (dy)
      {
      da = D2R * ROTATE_SCALE * dy;
      if (ctrl)
        {
        matrix_relative_rotation(mat, -da, PITCH, data);
        rotate_select(data, mat);
        }
      else
        {
        if (camera->mode == LOCKED)
          quat_concat_euler(camera->q, PITCH, da);
        else
          {
          matrix_v_rotation(mat, camera->e, -da);
          vecmat(mat, camera->v);
          vecmat(mat, camera->o);
          }
        }
      sysenv.moving = TRUE;
      refresh++;
      }
    if (dx)
      {
      da = D2R * ROTATE_SCALE * dx;
      if (ctrl)
        {
        matrix_relative_rotation(mat, -da, YAW, data);
        rotate_select(data, mat);
        }
      else
        {
        if (camera->mode == LOCKED)
          quat_concat_euler(camera->q, YAW, -da);
        else
          {
          matrix_v_rotation(mat, camera->o, da);
          vecmat(mat, camera->v);
          vecmat(mat, camera->e);
          }
        }
      sysenv.moving = TRUE;
      refresh++;
      }
    }
  }

/* save old values */
ox=x;
oy=y;

/* redraw? */
if (refresh)
  redraw_canvas(SINGLE);

return(TRUE);
}

/***************************/
/* expose all hidden atoms */
/***************************/
void unhide_atoms(void)
{
GSList *list;
struct model_pak *data;
struct core_pak *core;

/* deletion for the active model only */
data = tree_model_get();

/* unhide */
for (list=data->cores ; list ; list=g_slist_next(list))
  {
  core = list->data;
  core->status &= ~HIDDEN;
  }

render_atoms_setup(data);
render_pipes_setup(data);

/* update */
redraw_canvas(SINGLE);
}

/****************/
/* change mode */
/***************/
void gui_mode_switch(gint new_mode)
{
GSList *list;
struct model_pak *data;
struct core_pak *core1;

/* get selected model */
data = tree_model_get();
if (!data)
  return;

/* special case for morphology */
if (data->id == MORPH)
  {
  switch(new_mode)
    {
    case FREE:
    case RECORD:
      break;

    default:
      gui_text_show(WARNING, "Disallowed mode.\n");
      return;
    }
  }

/* clean up (if necessary) from previous mode */
switch (data->mode)
  {
  case DIST_INFO:
  case BOND_INFO:
    for (list=data->cores ; list ; list=g_slist_next(list))
      {
      core1 = list->data;
      core1->status &= ~SELECT;
      }
    break;

  case RECORD:
/*
    if (data->num_frames > 2)
      {
      camera_init(data);
      data->num_frames = g_slist_length(data->transform_list);
      }
    else
      {
      data->num_frames = 1;
      data->animation = FALSE;
      }
*/
    break;
  }

/* special initialization */
switch (new_mode)
  {
  case RECORD:
/* NEW - disallow transformation record mode if file is */
/* a standard animation as this will confuse read_frame() */
/*
    if (data->frame_list || data->gulp.trj_file)
      {
      gui_text_show(ERROR, "Disallowed mode change.\n");
      return;
      }
    data->animation = TRUE;
*/
    break;
  }

/* general initialization */
select_clear(data);
data->mode = new_mode;
data->state = 0;
redraw_canvas(SINGLE);
}

/* Need this for the menu item factory widget */
void gtk_mode_switch(GtkWidget *w, gint new_mode)
{
gui_mode_switch(new_mode);
}

/***************/
/* change view */
/***************/
void switch_view(gint new_mode)
{
}

/************/
/* GTK hook */
/************/
void gtk_switch_view(GtkWidget *w, gint new_mode)
{
switch_view(new_mode);
}

/**************************************/
/* symmetry list store free primitive */
/**************************************/
gboolean symmetry_free(GtkTreeModel *treemodel, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
gchar *a, *b;

gtk_tree_model_get(treemodel, iter, 0, &a, 1, &b, -1);
if (a)
  g_free(a);
if (b)
  g_free(b);
return(TRUE);
}

/*****************************/
/* redraw the content widget */
/*****************************/
void gui_content_refresh(GtkWidget *box)
{
GList *item, *list;
GtkTreeIter iter;
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
static GtkListStore *ls=NULL;
static GtkWidget *tv=NULL;

if (box)
  {
g_assert(ls == NULL);
g_assert(tv == NULL);

/* new tree list */
  ls = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
  tv = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ls));
  gtk_box_pack_start(GTK_BOX(box), tv, TRUE, TRUE, 0);

/* setup cell renderers */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(" ", renderer, "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(" ", renderer, "text", 1, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv), FALSE);

/* currently, allow nothing to be selected */
  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(tv)),
                              GTK_SELECTION_NONE);
  }
else
  {
  struct model_pak *model = tree_model_get();

g_assert(ls != NULL);
g_assert(tv != NULL);

  gtk_list_store_clear(ls);

  if (!model)
    return;

  model_content_refresh(model);

  list = g_hash_table_get_keys(model->property_table);

  list = g_list_sort(list, (gpointer) property_ranking);

  for (item=list ; item ; item=g_list_next(item))
    {
/* NEW - omit any text key with ! prefix (see reaxmd) */
    if (!g_strrstr(item->data, "!"))
      {
      gtk_list_store_append(ls, &iter);
      gtk_list_store_set(ls, &iter, 0, item->data, 1, property_lookup(item->data, model), -1);
      }
    }

  g_list_free(list);
  }
}

/************************************/
/* pick a (loaded) model to display */
/************************************/
#define DEBUG_PICK_MODEL 0
void gui_model_select(gpointer data)
{
struct model_pak *model = data;

/* checks */
if (!model)
  return;

gui_display_set(TREE_MODEL);

gui_relation_update(model);

dialog_refresh_all();

canvas_shuffle();

gui_refresh(GUI_CANVAS | GUI_ACTIVE);
}

/************************************************/
/* reset the scale and viewing angle to default */
/************************************************/
void gui_view_default(void)
{
struct model_pak *model;

/* do we have a loaded model? */
model = tree_model_get();
if (!model)
  return;

/* NEW - recentre (eg if atoms added -> centroid change) */
coords_init(CENT_COORDS, model);

/* make the changes */
VEC3SET(model->offset, 0.0, 0.0, 0.0);

/* update */
camera_init(model);

/* trigger any dialog updates */
gui_model_select(model);
}

/************************/
/* view down the x axis */
/************************/
void gui_view_x(void)
{
struct camera_pak *camera;
struct model_pak *model = tree_model_get();

if (model)
  {
  camera_init(model);
  camera = model->camera;
  quat_concat_euler(camera->q, YAW, 0.5*G_PI);
  gui_model_select(model);
  }
}

/************************/
/* view down the y axis */
/************************/
void gui_view_y(void)
{
struct camera_pak *camera;
struct model_pak *model = tree_model_get();

if (model)
  {
/*
  camera_init(model);
  camera = model->camera;
  quat_concat_euler(camera->q, YAW, G_PI);
  gui_model_select(model);
*/

  camera = model->camera;
  camera_default(model->rmax, camera);
  quat_concat_euler(camera->q, YAW, G_PI);
  gui_model_select(model);

  }
}

/************************/
/* view down the z axis */
/************************/
void gui_view_z(void)
{
struct camera_pak *camera;
struct model_pak *model = tree_model_get();

if (model)
  {
  camera_init(model);
  camera = model->camera;
  quat_concat_euler(camera->q, PITCH, -0.5*G_PI);
  gui_model_select(model);
  }
}

/************************/
/* view down the a axis */
/************************/
void gui_view_a(void)
{
gui_view_x();
}

/************************/
/* view down the b axis */
/************************/
void gui_view_b(void)
{
struct camera_pak *camera;
struct model_pak *model = tree_model_get();

if (model)
  {
  if (model->periodic > 1 || model->id == MORPH)
    {
    gui_view_x();
    camera = model->camera;
    quat_concat_euler(camera->q, YAW, model->pbc[5]);
    gui_model_select(model);
    }
  else
    gui_view_y();
  }
}

/************************/
/* view down the c axis */
/************************/
void gui_view_c(void)
{
gdouble a, c[3], v[3];
struct camera_pak *camera;
struct model_pak *model = tree_model_get();

if (model)
  {
  if (model->periodic > 2 || model->id == MORPH)
    {
    camera_init(model);
    camera = model->camera;

/* c axis vector */
    VEC3SET(c, 0.0, 0.0, -1.0);
    vecmat(model->latmat, c);

/* angle */
    a = via(camera->v,c,3);

/* rotation axis */
    crossprod(v, camera->v, c);
    normalize(v, 3);

/* align */
    quat_concat(camera->q, v, a);

    gui_model_select(model);
    }
  else
    gui_view_z();
  }
}

/**********************/
/* viewing widget box */
/**********************/
/* TODO - add camera properties? */
void gui_view_widget(GtkWidget *w)
{
GtkWidget *hbox;

hbox = gtk_hbox_new(TRUE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(w), hbox, FALSE, FALSE, 0);
gui_button(" x ", gui_view_x, NULL, hbox, TT);
gui_button(" y ", gui_view_y, NULL, hbox, TT);
gui_button(" z ", gui_view_z, NULL, hbox, TT);
gui_button(" a ", gui_view_a, NULL, hbox, TT);
gui_button(" b ", gui_view_b, NULL, hbox, TT);
gui_button(" c ", gui_view_c, NULL, hbox, TT);
}

/***************************************/
/* toggle the full text buffer display */
/***************************************/
void gui_text_toggle(GtkWidget *w, gpointer dummy)
{
static gint state=1;

state ^= 1;
if (state)
  {
  gtk_widget_hide(gui_text_swin);
  }
else
  {
  gtk_widget_show(gui_text_swin);
  }
}

/********************/
/* create text tray */
/********************/
void gui_text_widget(GtkWidget *box)
{
GtkWidget *view, *button;

/* view 1 - scrolled window of all past messages */
/* text status pane */
gui_text_swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gui_text_swin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_box_pack_start(GTK_BOX(box), gui_text_swin, TRUE, TRUE, PANEL_SPACING);

/* first time init */
view = gtk_text_view_new();
gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
gui_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
gtk_container_add(GTK_CONTAINER(gui_text_swin), view);

/* tags */
gtk_text_buffer_create_tag(gui_text_buffer, "fg_blue", "foreground", "blue", NULL);  
gtk_text_buffer_create_tag(gui_text_buffer, "fg_red", "foreground", "red", NULL);
gtk_text_buffer_create_tag(gui_text_buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);

/* view 2 - only the current message */
gui_text_current = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), gui_text_current, FALSE, FALSE, 0);

gui_text_summary = gtk_entry_new();
gtk_box_pack_start(GTK_BOX(gui_text_current), gui_text_summary, TRUE, TRUE, 0);

button = gui_image_button(IMAGE_INFO, gui_text_toggle, NULL);
gtk_box_pack_end(GTK_BOX(gui_text_current), button, FALSE, FALSE, 0);

}

/**************************************************************/
/* update the text tray summary with last entry in the buffer */
/**************************************************************/
#define TEXT_TRAY_LIFETIME 30
void gui_text_tray_refresh(void)
{
gint n;
static gint last=-1, count=0;
gchar *text, *strip;
GtkTextIter start, stop;

if (gui_text_buffer)
  {
  n = gtk_text_buffer_get_line_count(gui_text_buffer);

//printf("# lines = %d\n", n);

/* TODO - display the last non-blank line ... */
  if (n > 1)
    {
    if (n == last)
      count++;
    else
      count=0;

    if (count < TEXT_TRAY_LIFETIME)
      {
      gtk_text_buffer_get_iter_at_line(gui_text_buffer, &start, n-2);
      gtk_text_buffer_get_end_iter(gui_text_buffer, &stop);
      text =  gtk_text_buffer_get_text(gui_text_buffer, &start, &stop, FALSE);

//    text =  gtk_text_buffer_get_slice(gui_text_buffer, &start, &stop, FALSE);

      strip = parse_strip_newline(text);

      gtk_entry_set_text(GTK_ENTRY(gui_text_summary), strip);

      g_free(strip);
      g_free(text);
      }
    else
      gtk_entry_set_text(GTK_ENTRY(gui_text_summary), "");

    last = n;
    }
  }
}

/****************************/
/* add message to text tray */
/****************************/
void gui_text_show(gint type, gchar *message)
{
GtkTextIter iter;

/* checks */
if (!message)
  return;
if (!sysenv.canvas)
  return;

if (gui_text_buffer)
  {
  gtk_text_buffer_get_end_iter(gui_text_buffer, &iter);

/* assign colour via message type */
  switch (type)
    {
    case ERROR:
      gtk_text_buffer_insert_with_tags_by_name(gui_text_buffer, &iter, message, -1, "fg_red", NULL); 
      break;

    case WARNING:
      gtk_text_buffer_insert_with_tags_by_name(gui_text_buffer, &iter, message, -1, "fg_blue", NULL); 
      break;

    case ITALIC:
      gtk_text_buffer_insert_with_tags_by_name(gui_text_buffer, &iter, message, -1, "italic", NULL); 
      break;

    default:
      gtk_text_buffer_insert(gui_text_buffer, &iter, message, -1);
    }

/* NEW - scroll to the end, so recent message is visible */
/*
if (GTK_IS_TEXT_VIEW(view))
  {
gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(view), &iter, 0.0, FALSE, 0.0, 0.0);
  }
*/

/* TODO - delete items from buffer when line gets too big */
  }
else
  printf("[%s]\n", message);
}

/***********************************/
/* general panel visibility toggle */
/***********************************/
void pane_toggle(GtkWidget *w, GtkWidget *frame)
{
sysenv.write_gdisrc = TRUE;

if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
  gtk_widget_show(frame);
else
  gtk_widget_hide(frame);
}

/*****************************************/
/* cleanup/save state before actual exit */
/*****************************************/
void gdis_exit(void)
{
/* if user has changed element colours, but doesn't want them saved */
/* this will do it anyway - but they can just click reset next time */
if (sysenv.write_gdisrc)
  {
  sysenv.width = window->allocation.width;
  sysenv.height = window->allocation.height;

  write_gdisrc();
  }

/* cleanup */
sys_free();

if (sysenv.canvas)
  {
  gtk_widget_pop_visual();
  gtk_widget_pop_colormap();
  gtk_exit(0);
  }
exit(0);
}

/*********************************************************/
/* exit confirmation if jobs/files may be active/unsaved */
/*********************************************************/
// NB: avoid more than 1 popup with a confirm enquiry
void gui_exit_confirm(void)
{
gint response;

/* running tasks */
if (g_thread_pool_get_num_threads(sysenv.thread_pool))
  {
  response = gui_popup_confirm("There are running tasks. Really exit?");

// exit straight away if yes 
  if (response == 1)
    gdis_exit();
  else
    return;
  }

/* workspace */
if (sysenv.workspace_changed)
  {
/* NEW - only bother prompting if we have a non-empty workspace */
  if (g_slist_length(project_imports_get(sysenv.workspace)))
    {
/* 1 = yes, 0 = no , -1 = cancel (assume user doesnt want to exit) */
    response = gui_popup_confirm("Save your current workspace before exiting?");
    switch (response)
      {
      case -1:
        return;
      case 1:
        gui_workspace_save();
      default:
        gdis_exit();
      }
    }
  }

/* nothing running or potentially needed to be saved - exit */
gdis_exit();
}

/*************************************/
/* create fresh import from filename */
/*************************************/
gpointer gui_import_new(const gchar *filename)
{
gpointer import=NULL;
struct file_pak *file;

if (!filename)
  return(NULL);

//dialog_destroy_type(FILE_SELECT);

/* get the file type to import as */
/* FIXME - override with extension first as other things depend on it */
file = file_type_from_filename(filename);

/* read the file in */
if (file)
  {
  import = import_file(filename, file);

/* if successful add it to the display tree */
  if (import)
    {
    project_imports_add(sysenv.workspace, import);
    gui_tree_append_import(import);
    }
  }

return(import);
}

/******************************/
/* refresh an existing import */
/******************************/
/* FIXME - bug somewhere here causing an access violation (prob a double free) */
/* CURRENT - I think I've fixed it - import->object_array need to be set to NULL after a free */
/* see import_object_free_all() */
void gui_import_refresh(gpointer import)
{
gchar *filename, *fullpath;
struct file_pak *type;
struct model_pak *model;

if (!import)
  return;

filename = import_filename(import);
if (!filename)
  {
/* this happens if a dud grid job is imported */
  printf("Empty import.\n");
  return;
  }

/* NEW */
gui_tree_selection_push();

//printf("Re-reading: %s\n", filename);

type = file_type_from_filename(filename);
if (type)
  {
/* new file reading */
  if (type->import_file)
    {
/* delete all import children */
    gui_tree_remove_children(import);
    import_object_free_all(import);

    if (type->import_file(import))
      {
      printf("Refresh failed.\n");
      }
    else
      {
/* update the tree structure */
      gui_tree_import_refresh(import);
//      tree_select_refresh();
      }
    }
  else
    {
/* legacy file reading */
    if (type->read_file)
      {
/* delete all import children */
      gui_tree_remove_children(import);
      import_object_free_all(import);
/* read updated model in */
      model = model_new();

/* create full path filename */
      fullpath = g_build_filename(import_path(import), filename, NULL);
      if (!type->read_file(fullpath, model))
        {
/* update the tree structure */
        import_object_add(IMPORT_MODEL, model, import);
        gui_tree_import_refresh(import);
//        tree_select_refresh();
        }
      else
        {
        gui_text_show(ERROR, "Legacy read failed.\n");
        model_delete(model);
        }
      g_free(fullpath);
      }
    else
      gui_text_show(ERROR, "I don't know how to read this type of file.\n");
    }
  }
else
  gui_popup_message("Unknown save type for file.");

/* NEW */
gui_tree_selection_pop();
}

/**************************************/
/* refresh/reload the selected object */
/**************************************/
void gui_refresh_selected(void)
{
gint id;
gchar *label;
gpointer data;

if (tree_selected_get(&id, &label, &data))
  {
  switch(id)
    {
    case TREE_PROJECT:
printf("TODO - refresh project %p\n", data);
      break;

    case TREE_IMPORT:
      gui_import_refresh(data);
      break;

    case TREE_CONFIG:
    case TREE_GRAPH:
printf("TODO - refresh config/graph %p\n", data);
      break;
    }
  }
}

/*********************/
/* import gui action */
/*********************/
void gui_import_add_dialog(void)
{
gpointer dialog;

dialog = dialog_request(FILE_LOAD, "Browse Files", NULL, NULL, NULL);

if (dialog)
  {
  GtkWidget *window = dialog_window(dialog);
  gui_project_widget(GTK_DIALOG(window)->vbox);
  gtk_widget_set_size_request(window, 700, 600);

  gui_stock_button(GTK_STOCK_HELP, gui_help_show, "File Browser",
                   GTK_DIALOG(window)->action_area);

  gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog,
                   GTK_DIALOG(window)->action_area);

  gtk_widget_show_all(window);
  }
}

/***************************/
/* save the current import */
/***************************/
void gui_export(const gchar *fullpath)
{
gint id;
gchar *filename, *label;
gpointer data;

dialog_destroy_type(FILE_SELECT);

if (tree_selected_get(&id, &label, &data))
  {
  switch (id)
    {
    case TREE_IMPORT:
/* save the file */
      import_fullpath_set(fullpath, data);
      import_file_save(data);
/* update GUI tree */
      filename = g_path_get_basename(fullpath);
      gui_tree_rename(filename, data);
      g_free(filename);
      break;

    default:
      printf("Unsuported export object.\n");
    }
  }
}

/******************************************/
/* export with file path/name/type change */
/******************************************/
void gui_selected_save_as(void)
{
gui_dialog_file_save_as();
}

/*********************/
/* export gui action */
/*********************/
void gui_selected_save(void)
{
gint id, parent_id;
gchar *filename, *tmp, *text, *label;
const gchar *fullpath;
gpointer project, data;

if (tree_selected_get(&id, &label, &data))
  {
  switch (id)
    {
    case TREE_MODEL:
    case TREE_CONFIG:
/* trigger a save on parent import */
      data = gui_tree_parent_get(&parent_id, data);
      if (parent_id != TREE_IMPORT || !data)
        break;

    case TREE_IMPORT:
      fullpath = import_fullpath(data);
      if (g_file_test(fullpath, G_FILE_TEST_EXISTS))
        {
        text = g_strdup_printf("%s exists, overwrite?", fullpath);
        if (!gui_popup_confirm(text))
          {
          g_free(text);
          break;
          }
        g_free(text);
        }

/* if non NULL preview buffer - save that */
      text = gui_scrolled_text_get(text_list_buffer);
/* NEW - dont save if no text OR if preview disabled due to file being flagged as a bigfile */
      if (text && !import_bigfile_get(data))
        {
        if (strlen(text))
          {
          g_file_set_contents(import_fullpath(data), text, -1, NULL);
          g_free(text);
          }
/* refresh (buffer may have changed) */
        gui_import_refresh(data);
        }
      else
        {
/* otherwise - normal save */
        import_file_save(data);
        }

/* TODO - re-import to push the text changes to the model/config/etc ... */



      break;

    case TREE_GRAPH:
/* get project */
      project = tree_project_get();
      if (!project)
        break;

/* build and test full path filename */
      filename = g_strdup_printf("%s.plot", label);
      tmp = g_build_filename(project_path_get(project), filename, NULL);
      g_free(filename);
      if (g_file_test(tmp, G_FILE_TEST_EXISTS))
        text = g_strdup_printf("%s exists: overwrite?", tmp);
      else
        text = g_strdup_printf("Dump graph to: %s?", tmp);

/* prompt */
        if (!gui_popup_confirm(text))
          {
          g_free(text);
          break;
          }
        g_free(text);

/* write */
      graph_write(tmp, data);
      g_free(tmp);
      break;
    }
  }
}

/*********************/
/* project importing */
/*********************/
void import_pcf(gchar *filename)
{
struct model_pak *model;

dialog_destroy_type(FILE_SELECT);

model = tree_model_get();
if (!model)
  return;

//project_read(filename, model);

redraw_canvas(SINGLE);
}

/*****************/
/* mode switches */
/*****************/
void gui_mode_record(void)
{
gui_mode_switch(RECORD);
}

/*****************/
/* mode switches */
/*****************/
void gui_mode_default(void)
{
gui_mode_switch(FREE);
}

/****************************/
/* delete atoms and refresh */
/****************************/
void gui_delete_selection(void)
{
select_delete();
gui_refresh(GUI_CANVAS | GUI_ACTIVE);
}

/******************************/
/* save the current workspace */
/******************************/
void gui_workspace_save(void)
{
gchar *text;

//printf("Saving workspace...\n");

/* description */
text = g_strdup_printf("Workspace at %s", timestamp());

project_label_set(text, sysenv.workspace);
project_description_set(text, sysenv.workspace);

g_free(text);

project_save(sysenv.workspace);

sysenv.workspace_changed = FALSE;
}

/*******************************/
/* clear the current workspace */
/*******************************/
void gui_workspace_clear(void)
{
gpointer project;

/* prompt for save */
if (sysenv.workspace_changed)
  {
  if (gui_popup_confirm("Save workspace before clear?"))
    gui_workspace_save(); 
  }

/* clear the GUI tree */
gui_tree_clear();

/* free old workspace, create new */
project = sysenv.workspace;
sysenv.workspace = project_new("default", sysenv.cwd);
project_free(project);

sysenv.workspace_changed = FALSE;

gui_refresh(GUI_CANVAS);
}

/********************************/
/* display the About help topic */
/********************************/
void gui_help_about(void)
{
gui_help_show(NULL, "About");
}

void gui_mode_normal(void)
{
gui_mode_switch(FREE);
}

void gui_mode_overlay(void)
{
gui_mode_switch(OVERLAY);
}

/******************/
/* MENU structure */
/******************/
/* FIXME - this has been deprecated for a while in GTK */
static GtkItemFactoryEntry menu_items[] = 
{
  { "/_File",                            NULL, NULL, 0, "<Branch>" },
  { "/File/Browse Files...",             NULL, gui_import_add_dialog, 1, NULL },
  { "/File/Save File",                   NULL, gui_selected_save, 1, NULL },
  { "/File/Save File As...",             NULL, gui_selected_save_as, 1, NULL },
  { "/File/_Close File",                 NULL, tree_select_delete, 1, NULL },
  { "/File/sep1",                        NULL, NULL, 0, "<Separator>" },
  { "/File/Browse Workspaces...",        NULL, gui_project_browse_dialog, 1, NULL },
  { "/File/Save Workspace",              NULL, gui_workspace_save, 1, NULL },
  { "/File/Clear Workspace",             NULL, gui_workspace_clear, 1, NULL },
  { "/File/sep1",                        NULL, NULL, 0, "<Separator>" },
  { "/File/Executable Locations...",     NULL, gui_setup_dialog, 0, NULL},
  { "/File/sep1",                        NULL, NULL, 0, "<Separator>" },
  { "/File/_Quit",                       NULL, gui_exit_confirm, 0, NULL },

  { "/_Edit",               NULL, NULL, 0, "<Branch>" },
  { "/Edit/_Copy",          NULL, select_copy, 0, NULL },
  { "/Edit/_Paste",         NULL, select_paste, 0, NULL },
  { "/Edit/sep1",           NULL, NULL, 0, "<Separator>" },
  { "/Edit/Delete",         NULL, gui_delete_selection, 0, NULL },
  { "/Edit/Undo",           NULL, undo_active, 0, NULL },
  { "/Edit/sep1",           NULL, NULL, 0, "<Separator>" },
  { "/Edit/Colour...",      NULL, select_colour, 0, NULL },
  { "/Edit/Hide",           NULL, select_hide, 0, NULL },
  { "/Edit/Unhide all",     NULL, unhide_atoms, 0, NULL },
  { "/Edit/sep1",           NULL, NULL, 0, "<Separator>" },
  { "/Edit/Select all",     NULL, select_all, 0, NULL },
  { "/Edit/Invert",         NULL, select_invert, 0, NULL },

  { "/_Tools",                           NULL, NULL, 0, "<Branch>" },
  { "/Tools/Animation and Rendering...", NULL, gui_animate_dialog, 0, NULL },
  { "/Tools/Display Properties...",      NULL, gui_render_dialog, 0, NULL},
  { "/Tools/Periodic Table...",          NULL, gui_gperiodic_dialog, 0, NULL },
  { "/Tools/sep1",                       NULL, NULL, 0, "<Separator>" },
  { "/Tools/Analysis and Plotting...",   NULL, gui_analysis_dialog, 0, NULL },
  { "/Tools/Diffraction...",             NULL, gui_diffract_dialog, 0, NULL },
  { "/Tools/Dislocation Builder...",     NULL, gui_defect_dialog, 0, NULL },
  { "/Tools/Dynamics Initializer",       NULL, gui_mdi_dialog, 0, NULL },
  { "/Tools/Iso-surfaces...",            NULL, gui_isosurf_dialog, 0, NULL },
  { "/Tools/Measurements...",            NULL, gui_measure_dialog, 0, NULL },
  { "/Tools/Model Editing...",           NULL, gui_edit_dialog, 0, NULL },
  { "/Tools/Surface Builder",            NULL, gui_surface_dialog, 0, NULL },
  { "/Tools/sep1",                       NULL, NULL, 0, "<Separator>" },
  { "/Tools/Task Manager...",            NULL, gui_task_dialog, 0, NULL},
// FIXME - currently kinda broken
//  { "/Tools/Zmatrix...",                 NULL, gui_zmat_dialog, 0, NULL },
#ifdef WITH_GRISU
  { "/Tools/Grid Authentication...",     NULL, gui_grid_dialog, 0, NULL},
#endif

  { "/_View",                            NULL, NULL, 0, "<Branch>" },
  { "/View/Reset Model camera",          NULL, gui_view_default, 0, NULL },
  { "/View/sep1",                        NULL, NULL, 0, "<Separator>" },
  { "/View/Normal mode",                 NULL, gui_mode_normal, 0, NULL },
  { "/View/Overlay mode",                NULL, gui_mode_overlay, 0, NULL },

/* about info -> manual acknowlegements */
  { "/_Help",                  NULL, NULL, 0, "<Branch>"},
  { "/Help/Manual...",         NULL, gui_help_dialog, 0, NULL},
  { "/Help/sep1",              NULL, NULL, 0, "<Separator>" },
  { "/Help/About...",          NULL, gui_help_about, 0, NULL},
};

/********************************/
/* process key presses directly */
/********************************/
gint cb_key_press(GtkWidget *w, GdkEventKey *event, gpointer dummy)
{
switch(event->keyval)
  {
/* selection delete */
  case GDK_Insert:
    undo_active();
    break;

/* selection delete */
  case GDK_Delete:
    select_delete();
    gui_refresh(GUI_CANVAS | GUI_ACTIVE);
    break;

/* fullscreen */
  case GDK_F1:
    if (!sysenv.stereo)
      {
      sysenv.stereo = TRUE;
      sysenv.stereo_fullscreen = TRUE;
      sysenv.render.perspective = TRUE;
      stereo_open_window();
      }
    break;

/* windowed */
/* CURRENT - hacked to work with new camera code */
  case GDK_F2:
/*
    if (!sysenv.stereo && sysenv.stereo_windowed)
*/

    if (!sysenv.stereo)
      {
      sysenv.stereo = TRUE;
      sysenv.stereo_fullscreen = FALSE;
      sysenv.render.perspective = TRUE;

      gui_refresh(GUI_CANVAS);
      }
    break;

/* NEW - switching between windowed/fullscreen stereo while in */
/* stereo mode can screw things up - so just use esc to exit */
/* for both. ie F1/F2 do not act as stereo toggles */
  case GDK_Escape:
    sysenv.stereo = FALSE;
    sysenv.render.perspective = FALSE;
    stereo_close_window();
    gui_refresh(GUI_CANVAS);
    break;

  case 'C':
  case 'c':
    if (event->state & GDK_CONTROL_MASK)
      select_copy();
    break;

  case 'V':
  case 'v':
    if (event->state & GDK_CONTROL_MASK)
      select_paste();
    break;

  }
return(FALSE);
}

/*******************************************************/
/* record current model/property pane divider position */
/*******************************************************/
gboolean cb_pane_refresh(GtkPaned *w, gpointer data)
{
sysenv.tree_divider = gtk_paned_get_position(w);
return(FALSE);
}

/**************************/
/* GUI graph edit widgets */
/**************************/
GtkWidget *gui_xstart_entry, *gui_xstop_entry;
GtkWidget *gui_ystart_entry, *gui_ystop_entry;
GtkWidget *gui_ymean_entry, *gui_ysd_entry;

/********************************************/
/* update the current graph with new values */
/********************************************/
void gui_graph_refresh(GtkWidget *w, gpointer dummy)
{
gint id;
gchar *label;
gpointer data;

if (tree_selected_get(&id, &label, &data))
  {
  if (id == TREE_GRAPH)
    {
/*
printf("updating graph: %p\n", data);
printf("x start = %f\n", graph_xstart_get(data));
printf("x stop = %f\n", graph_xstop_get(data));
*/
    label = g_strdup_printf("%f", graph_xstart_get(data));
    gtk_entry_set_text(GTK_ENTRY(gui_xstart_entry), label);
    g_free(label);

    label = g_strdup_printf("%f", graph_xstop_get(data));
    gtk_entry_set_text(GTK_ENTRY(gui_xstop_entry), label);
    g_free(label);

    label = g_strdup_printf("%f", graph_ymin(data));
    gtk_entry_set_text(GTK_ENTRY(gui_ystart_entry), label);
    g_free(label);

    label = g_strdup_printf("%f", graph_ymax(data));
    gtk_entry_set_text(GTK_ENTRY(gui_ystop_entry), label);
    g_free(label);

    label = g_strdup_printf("%f", graph_ymean_get(data));
    gtk_entry_set_text(GTK_ENTRY(gui_ymean_entry), label);
    g_free(label);

    label = g_strdup_printf("%f", graph_ysd_get(data));
    gtk_entry_set_text(GTK_ENTRY(gui_ysd_entry), label);
    g_free(label);
    }
  }
}

/*************************************/
/* callback for complete widget push */
/*************************************/
void gui_graph_changed(GtkWidget *w, gpointer dummy)
{
gint id;
gchar *label;
const gchar *text;
gdouble v;
gpointer data;

if (tree_selected_get(&id, &label, &data))
  {
  if (id == TREE_GRAPH)
    {
    text = gtk_entry_get_text(GTK_ENTRY(gui_xstart_entry));
    v = str_to_float(text);
    graph_xstart_set(v, data);

    text = gtk_entry_get_text(GTK_ENTRY(gui_xstop_entry));
    v = str_to_float(text);
    graph_xstop_set(v, data);

    text = gtk_entry_get_text(GTK_ENTRY(gui_ystart_entry));
    v = str_to_float(text);
    graph_ymin_set(v, data);

    text = gtk_entry_get_text(GTK_ENTRY(gui_ystop_entry));
    v = str_to_float(text);
    graph_ymax_set(v, data);

/* refresh graph and stats display */
    gui_refresh(GUI_CANVAS | GUI_ACTIVE);
    }
  }
}

/******************************************/
/* revert x and y axis to default extents */
/******************************************/
void gui_graph_reset(GtkWidget *w, gpointer dummy)
{
gint id;
gchar *label;
gpointer data;

if (tree_selected_get(&id, &label, &data))
  {
  if (id == TREE_GRAPH)
    {
    graph_default_x(data);
    graph_default_y(data);

    gui_refresh(GUI_CANVAS | GUI_ACTIVE);
    }
  }
}

/******************************************************************/
/* revert displayed data sets to original (ie remove pasted sets) */
/******************************************************************/
void gui_graph_data_reset(GtkWidget *w, gpointer dummy)
{
gint id;
gchar *label;
gpointer data;

if (tree_selected_get(&id, &label, &data))
  {
  if (id == TREE_GRAPH)
    {
    graph_default_data(data);
    graph_default_x(data);
    graph_default_y(data);

    gui_refresh(GUI_CANVAS | GUI_ACTIVE);
    }
  }
}

/**************************************/
/* tool for editing graph extents etc */
/**************************************/
void gui_graph_init(GtkWidget *box)
{
GtkWidget *table, *label, *vbox;

// TODO - reset button to restore to min/max defaults
/*
gui_button_x("Test graph function", NULL, NULL, box);
gui_text_entry("Start x", NULL, TRUE, TRUE, box);
*/

table = gtk_table_new(2, 6, FALSE);
gtk_box_pack_start(GTK_BOX(box), table, TRUE, TRUE, 0);

/* labels */

label = gtk_label_new("x start ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);

label = gtk_label_new("x stop ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);

label = gtk_label_new("y start ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);

label = gtk_label_new("y stop ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,3,4);

label = gtk_label_new("mean ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,4,5);
label = gtk_label_new("sd ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,5,6);

/* values */

gui_xstart_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),gui_xstart_entry,1,2,0,1);
g_signal_connect(GTK_OBJECT(gui_xstart_entry), "activate",
                 GTK_SIGNAL_FUNC(gui_graph_changed), NULL);

gui_xstop_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),gui_xstop_entry,1,2,1,2);
g_signal_connect(GTK_OBJECT(gui_xstop_entry), "activate",
                 GTK_SIGNAL_FUNC(gui_graph_changed), NULL);

gui_ystart_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),gui_ystart_entry,1,2,2,3);
g_signal_connect(GTK_OBJECT(gui_ystart_entry), "activate",
                 GTK_SIGNAL_FUNC(gui_graph_changed), NULL);

gui_ystop_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),gui_ystop_entry,1,2,3,4);
g_signal_connect(GTK_OBJECT(gui_ystop_entry), "activate",
                 GTK_SIGNAL_FUNC(gui_graph_changed), NULL);

gui_ymean_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),gui_ymean_entry,1,2,4,5);
gtk_entry_set_editable(GTK_ENTRY(gui_ymean_entry), FALSE);

gui_ysd_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),gui_ysd_entry,1,2,5,6);
gtk_entry_set_editable(GTK_ENTRY(gui_ysd_entry), FALSE);

/* options */
vbox = gtk_vbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);

gui_button_x("Restore axis range ", gui_graph_reset, NULL, vbox);
gui_button_x("Restore original data ", gui_graph_data_reset, NULL, vbox);
}

/***************************************/
/* update the GDIS information widgets */
/***************************************/
gint gui_widget_handler(void)
{
static gint running_count=0;
gchar *text;
GSList *list;
struct task_pak *task;

/* TODO - all components of the gui */
if (sysenv.refresh_properties)
  {
  gui_active_refresh();
  sysenv.refresh_properties = FALSE;

// NEW
  gui_graph_refresh(NULL, NULL);
  }

/* text summary */
gui_text_tray_refresh();

/* task summary */
gtk_entry_set_text(GTK_ENTRY(gui_task_summary), "");
if (gui_task_summary)
  {
  for (list=sysenv.task_list ; list ; list=g_slist_next(list))
    {
    task = list->data;

    if (task->status == RUNNING)
      {
// TODO - display % completed?
      switch (running_count)
        {
        case 0:
          text = g_strdup_printf("%s", task->label);
          running_count++;
          break;
        case 1:
          text = g_strdup_printf("%s.", task->label);
          running_count++;
          break;
        case 2:
          text = g_strdup_printf("%s..", task->label);
          running_count++;
          break;
        default:
        case 3:
          text = g_strdup_printf("%s...", task->label);
          running_count=0;
          break;
        }

      gtk_entry_set_text(GTK_ENTRY(gui_task_summary), text);
      g_free(text);
      }
    }
  }

return(TRUE);
}

/******************************/
/* kill event on current task */
/******************************/
void gui_task_remove(GtkWidget *w, gpointer dummy)
{
GSList *list;
struct task_pak *task;

for (list=sysenv.task_list ; list ; list=g_slist_next(list))
  {
  task = list->data;
  if (task->status == RUNNING)
    {
/* NB: only tasks that return a worker pid are interruptable */
#ifndef __WIN32
    if (task->function)
      {
      if (gui_popup_confirm("Really kill active task?") == 1)
        {
        task->status = KILLED;
        kill(task->pid, SIGKILL);
//      task_kill_running(task);
        }
      }
    else
#endif
      gui_text_show(ERROR, "Active task is not interruptable.\n");
    return;
    }
  }
}

/***************************************************/
/* refresh GUI after a background monitor download */
/***************************************************/
void gui_task_monitor_cleanup(gpointer data, gpointer dummy)
{
gpointer import;

import = file_transfer_import_get(data);
if (import)
  gui_import_refresh(import);
else
  gui_text_show(ERROR, "Import is no longer valid, deleted?\n");

file_transfer_free(data);
}

/*********************************************************/
/* force a background task to refresh a monitored import */
/*********************************************************/
void gui_task_monitor(GtkWidget *w, gpointer dummy)
{
gpointer import, data;

import = tree_import_get();
if (!import)
  return;

data = file_import_download(import);
if (data)
  {
  task_new("Downloading remote file", file_transfer_start, data, gui_task_monitor_cleanup, data, NULL);
  }
else
  {
// NEW - test gui_import_refresh() ... reload from local disk anyway
gui_import_refresh(import);

//  gui_text_show(ERROR, "You can only monitor grid objects.\n");
  }
}

/***************/
/* task widget */
/***************/
void gui_task_tray(GtkWidget *box)
{
GtkWidget *button;

gui_task_current = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), gui_task_current, FALSE, FALSE, 0);

gui_task_summary = gtk_entry_new();
gtk_box_pack_start(GTK_BOX(gui_task_current), gui_task_summary, TRUE, TRUE, 0);

/* NEW */
button = gui_image_button(IMAGE_CONNECT, gui_task_monitor, NULL);
gtk_box_pack_start(GTK_BOX(gui_task_current), button, FALSE, FALSE, 0);

button = gui_image_button(IMAGE_CROSS, gui_task_remove, NULL);
gtk_box_pack_start(GTK_BOX(gui_task_current), button, FALSE, FALSE, 0);
}

/****************************/
/* selection model callback */
/****************************/
void gui_selection_mode_set(GtkWidget *w, gpointer dummy)
{
const gchar *line;

g_assert(GTK_IS_ENTRY(w));

line = gtk_entry_get_text(GTK_ENTRY(w));

gui_mode_switch(FREE);

if (g_strrstr(line, "Atoms"))
  {
  sysenv.select_mode = CORE;
  return;
  }
if (g_strrstr(line, "Label"))
  {
  sysenv.select_mode = ATOM_LABEL;
  return;
  }
if (g_strrstr(line, "Type"))
  {
  sysenv.select_mode = ATOM_TYPE;
  return;
  }
if (g_strrstr(line, "Elements"))
  {
  if (g_strrstr(line, "molecule"))
    sysenv.select_mode = ELEM_MOL;
  else
    sysenv.select_mode = ELEM;
  return;
  }
if (g_strrstr(line, "Molecules"))
  {
  sysenv.select_mode = MOL;
  return;
  }
if (g_strrstr(line, "fragments"))
  {
  sysenv.select_mode = FRAGMENT;
  gui_mode_switch(SELECT_FRAGMENT);
  return;
  }
if (g_strrstr(line, "Regions"))
  {
  sysenv.select_mode = REGION;
  return;
  }
}


/* CURRENT */
/* functionality for extending active model pulldown capabilities */
/* TODO - put in shortcuts */

struct active_pak
{
gchar *label;
void (*setup)(gpointer);
void (*refresh)(gpointer);
GtkWidget *vbox;
};

GList *active_list=NULL;
struct active_pak *active_data=NULL;

/***********************************/
/* refresh the active model widget */
/***********************************/
void gui_active_refresh(void)
{
GList *list;
struct active_pak *current;

for (list=active_list ; list ; list=g_list_next(list))
  {
  current = list->data;

  if (active_data == current)
    {
/* restore "proper" size */
    gtk_widget_set_size_request(current->vbox, -1, -1);
    gtk_widget_show(current->vbox);
    }
  else
    gtk_widget_hide(current->vbox);
  }

if (active_data)
  {
  if (active_data->refresh)
    active_data->refresh(NULL);
  }
}

/****************************************/
/* active pulldown change event handler */
/****************************************/
void gui_active_handler(GtkWidget *w, gpointer dummy)
{
const gchar *tmp;
GList *list;
struct active_pak *active;

tmp = gtk_entry_get_text(GTK_ENTRY(w));

for (list=active_list ; list ; list=g_list_next(list))
  {
  active = list->data;

  if (g_strcasecmp(active->label, tmp) == 0)
    {
    active_data = active;
    gui_active_refresh();
    return;
    }
  }
}

/*********************************/
/* setup the active model widget */
/*********************************/
void gui_active_setup(GtkWidget *box)
{
gpointer entry;
GList *list, *menu;
struct active_pak *active;

/* build the list of labels */
menu = NULL;
for (list=active_list ; list ; list=g_list_next(list))
  {
  active = list->data;
  menu = g_list_append(menu, g_strdup(active->label));
  }

/* create the header pulldown */
entry = gui_pulldown_new(NULL, menu, FALSE, box);

/* create and pack the data boxes */
for (list=active_list ; list ; list=g_list_next(list))
  {
  active = list->data;

  gtk_box_pack_start(GTK_BOX(box), active->vbox, FALSE, FALSE, 0);

/* stop GTK maxing out the allocated height for all active model children */
  gtk_widget_set_size_request(active->vbox, -1, 1);

  if (active->setup)
    active->setup(active->vbox);
  }

/* top item is active by default */
if (active_list)
  active_data = active_list->data;

g_signal_connect(GTK_OBJECT(entry), "changed", GTK_SIGNAL_FUNC(gui_active_handler), NULL);
}

/*************************************************/
/* create a new entry in the active model widget */
/*************************************************/
void gui_active_new(const gchar *label, gpointer setup, gpointer refresh)
{
struct active_pak *active;

active = g_malloc(sizeof(struct active_pak));

active->label = g_strdup(label);
active->setup = setup;
active->refresh = refresh;
active->vbox = gtk_vbox_new(FALSE, 0);

active_list = g_list_append(active_list, active);
}

/*********/
/* SETUP */
/*********/
void gui_init(int argc, char *argv[])
{
gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
gchar *text;
GList *list;
gpointer ptr;
GtkWidget *gdis_wid;
GtkWidget *hpaned;
GtkWidget *vbox, *hbox, *vbox_lower, *menu_bar, *toolbar;
GtkWidget *frame, *event_box;
GdkBitmap *mask;
GdkPixmap *gdis_pix=NULL;
GdkPixbuf *pixbuf;
GtkStyle *style;
GtkItemFactory *item;
GdkColor colour;

gtk_init(&argc, &argv);
gtk_gl_init(&argc, &argv);

/* enforce true colour (fixes SG problems) */
sysenv.visual = gdk_visual_get_best_with_type(GDK_VISUAL_TRUE_COLOR);
if (!sysenv.visual)
  {
  printf("Error: could not get requested visual.\n");
  exit(1);
  }
sysenv.depth = sysenv.visual->depth;
sysenv.colourmap = gdk_colormap_new(sysenv.visual, TRUE);
if (!sysenv.colourmap)
  {
  printf("Error: could not allocate colourmap.\n");
  exit(1);
  }
gtk_widget_push_colormap(sysenv.colourmap);
gtk_widget_push_visual(sysenv.visual);

/* NB: GTK graphical init needs to be done before this is called, */
/* NB: but this must be done before we build the GDIS interface */
image_table_init();

/* main window */
window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, FALSE);
gtk_window_set_title(GTK_WINDOW(window),"GTK Display Interface for Structures");
gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

gtk_window_set_default_size(GTK_WINDOW(window), sysenv.width, sysenv.height);

g_signal_connect(GTK_OBJECT(window), "key_press_event",
                (GtkSignalFunc) cb_key_press, NULL);


/* vbox for the menubar */
vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(window), vbox);

/* item factory menu creation */
item = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", NULL);
gtk_item_factory_create_items(item, nmenu_items, menu_items, NULL);
menu_bar = gtk_item_factory_get_widget(item, "<main>");

/* FALSE,FALSE => don't expand to fill (eg on resize) */
gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

/* hbox for the toolbar */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
toolbar = gtk_toolbar_new();
gtk_box_pack_start(GTK_BOX(hbox), toolbar, FALSE, FALSE, 0);

/* extract some important style info */
gtk_widget_realize(window);
style = gtk_widget_get_style(window);

sysenv.gtk_fontsize = pango_font_description_get_size(style->font_desc) 
                    / PANGO_SCALE;

/*
printf("psize = %d\n", pango_font_description_get_size(style->font_desc));
printf("scale = %d\n", PANGO_SCALE);
printf(" size = %d\n", sysenv.gtk_fontsize);
*/

/* new project */
/* TODO - different button/icon for projects? */
/*
pixbuf = image_table_lookup(IMAGE_CABINET);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar), NULL,
                        "Browse Projects", "Private",
                        gdis_wid, GTK_SIGNAL_FUNC(gui_project_browse_dialog), NULL);
*/

/* CURRENT - project/import views cover this functionality */
/* load button */
pixbuf = image_table_lookup(IMAGE_FOLDER);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), NULL,
                        "Browse Files", "Private",
                        gdis_wid, GTK_SIGNAL_FUNC(gui_import_add_dialog), NULL);

/*
pixbuf = image_table_lookup(IMAGE_FOLDER);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), NULL,
                        "New Project", "Private",
                        gdis_wid, GTK_SIGNAL_FUNC(gui_project_new), NULL);
*/

/* save button */
pixbuf = image_table_lookup(IMAGE_DISK);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), NULL,
                        "Save File", "Private", gdis_wid,
                        GTK_SIGNAL_FUNC(gui_selected_save), NULL);

/* delete button */
pixbuf = image_table_lookup(IMAGE_CROSS);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Close File",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(tree_select_delete),
                        NULL);

/* gap */
gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

/* animation and rendering */
pixbuf = image_table_lookup(IMAGE_ANIMATE);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Animation and Rendering",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_animate_dialog),
                        NULL);

/* display properties */
pixbuf = image_table_lookup(IMAGE_PALETTE);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Display Properties",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_render_dialog),
                        NULL);

/* gperiodic button */
pixbuf = image_table_lookup(IMAGE_ELEMENT);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Periodic Table",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_gperiodic_dialog),
                        NULL);

/* gap */
gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

/* TODO - plotting button */
pixbuf = image_table_lookup(IMAGE_PLOT);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Analysis and Plotting",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_analysis_dialog),
                        NULL);

/* diffraction button */
pixbuf = image_table_lookup(IMAGE_DIFFRACTION);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Diffraction",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_diffract_dialog),
                        NULL);

/* dislocations */
/* FIXME - placeholder */
pixbuf = image_table_lookup(IMAGE_DEFECT);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Dislocation Builder",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_defect_dialog),
                        NULL);

/* dynamics initializer */
/* FIXME - placeholder */
pixbuf = image_table_lookup(IMAGE_DYNAMICS);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Dynamics Initializer",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_mdi_dialog),
                        NULL);

/* iso surfaces */
pixbuf = image_table_lookup(IMAGE_ISOSURFACE);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Iso-surfaces",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_isosurf_dialog),
                        NULL);

/* measurements button */
pixbuf = image_table_lookup(IMAGE_COMPASS);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Measurements",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_measure_dialog),
                        NULL);

/* model editing */
pixbuf = image_table_lookup(IMAGE_TOOLS);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Model Editing",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_edit_dialog),
                        NULL);

/* surface building */
pixbuf = image_table_lookup(IMAGE_SURFACE);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Surface Builder",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_surface_dialog),
                        NULL);

gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));



/* CURRENT */
pixbuf = image_table_lookup(IMAGE_COG);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar), NULL,
                        "Task Manager", "Private",
                        gdis_wid, GTK_SIGNAL_FUNC(gui_task_dialog), NULL);

#ifdef WITH_GRISU
/* grid manager */
pixbuf = image_table_lookup(IMAGE_USER);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                        NULL,
                        "Grid Authentication",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_grid_dialog),
                        NULL);
#endif


/* VIEW menu items */
gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

/* model scale/orientation */
pixbuf = image_table_lookup(IMAGE_AXES);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                        NULL,
                        "Reset Model Camera",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gui_view_default),
                        NULL);

/* model images */
/*
pixbuf = image_table_lookup(IMAGE_PERIODIC);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                        NULL,
                        "Reset Model Images",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(space_image_widget_reset),
                        NULL);
*/

gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

/*
pixbuf = image_table_lookup(IMAGE_CAMERA);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Record mode",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gtk_mode_switch),
                        GINT_TO_POINTER(RECORD));
*/

#if MULTI_CANVAS
/* single canvas */
pixbuf = image_table_lookup(IMAGE_CANVAS_SINGLE);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Single canvas",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(canvas_single),
                        GINT_TO_POINTER(CANVAS_SINGLE));

/* create canvas */
pixbuf = image_table_lookup(IMAGE_CANVAS_CREATE);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "increase canvases",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(canvas_create),
                        NULL);

/* delete canvas */
pixbuf = image_table_lookup(IMAGE_CANVAS_DELETE);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "decrease canvases",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(canvas_delete),
                        NULL);

gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
#endif

/* normal viewing button */
pixbuf = image_table_lookup(IMAGE_ARROW);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Normal mode",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gtk_mode_switch),
                        GINT_TO_POINTER(FREE));

/* normal viewing button */
pixbuf = image_table_lookup(IMAGE_OVERLAY);
gdis_wid = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR (toolbar),
                        NULL,
                        "Overlay mode",
                        "Private",
                        gdis_wid,
                        GTK_SIGNAL_FUNC(gtk_mode_switch),
                        GINT_TO_POINTER(OVERLAY));

/* MAIN LEFT/RIGHT HBOX PANE */
/* paned window */
hpaned = gtk_hpaned_new();
gtk_container_add(GTK_CONTAINER(GTK_BOX(vbox)), hpaned);

/* LEFT PANE */
sysenv.mpane = gtk_vbox_new(FALSE, 0);
gtk_paned_pack1(GTK_PANED(hpaned), sysenv.mpane, FALSE, TRUE);
gtk_widget_set_size_request(sysenv.mpane, sysenv.tree_width, -1);

/* model tree box */
vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(sysenv.mpane), vbox, TRUE, TRUE, 0);
tree_init(vbox);

/* model pulldown */
vbox_lower = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(sysenv.mpane), vbox_lower, FALSE, FALSE, 0);

/* general model data box */
vbox_model_active = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox_lower), vbox_model_active, FALSE, FALSE, 0);

/* model properties */
gui_active_new("Model : Content", gui_content_refresh, gui_content_refresh);
gui_active_new("Model : Images",  space_image_widget_setup, space_image_widget_redraw);
gui_active_new("Model : Symmetry", gui_symmetry_refresh, gui_symmetry_refresh);
gui_active_new("Model : Viewing", gui_view_widget, NULL);
gui_active_setup(vbox_model_active);

/* selection pulldown */
vbox_model_select = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(sysenv.mpane), vbox_model_select, FALSE, FALSE, 0);
list = NULL;
list = g_list_append(list, "Select : Atoms");
list = g_list_append(list, "Select : Atom Label");
list = g_list_append(list, "Select : Atom FF Type");
list = g_list_append(list, "Select : Elements");
list = g_list_append(list, "Select : Elements in Molecule");
list = g_list_append(list, "Select : Molecules");
list = g_list_append(list, "Select : Molecule Fragments");
list = g_list_append(list, "Select : Regions");
ptr = gui_pulldown_new(NULL, list, FALSE, vbox_model_select);
g_signal_connect(GTK_OBJECT(ptr), "changed", GTK_SIGNAL_FUNC(gui_selection_mode_set), NULL);


/* NEW - graph editing tool */
gui_graph_vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(sysenv.mpane), gui_graph_vbox, FALSE, FALSE, 0);
gui_graph_init(gui_graph_vbox);


/* GDIS logo */
frame = gtk_frame_new(NULL);
gtk_box_pack_end(GTK_BOX(sysenv.mpane), frame, FALSE, TRUE, 0);

/* event box (get an x window for setting black background) */
event_box = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(frame), event_box);

/* black background */
colour.red = 0.0;
colour.green = 0.0;
colour.blue = 0.0;
gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &colour);

hbox = gtk_hbox_new (FALSE, 10);
gtk_container_add (GTK_CONTAINER(event_box), hbox);
gtk_container_set_border_width(GTK_CONTAINER(hbox), PANEL_SPACING);

/* logo images */
gdis_pix = gdk_pixmap_create_from_xpm_d(window->window, &mask,
                          &style->bg[GTK_STATE_NORMAL], logo_left_xpm);
gdis_wid = gtk_image_new_from_pixmap(gdis_pix, mask);
gtk_box_pack_start(GTK_BOX(hbox), gdis_wid, FALSE, FALSE, 0);

gdis_pix = gdk_pixmap_create_from_xpm_d(window->window, &mask,
                          &style->bg[GTK_STATE_NORMAL], logo_right_81_xpm);
gdis_wid = gtk_image_new_from_pixmap(gdis_pix, mask);
gtk_box_pack_end(GTK_BOX(hbox), gdis_wid, FALSE, FALSE, 0);

/* RIGHT PANE */
vbox = gtk_vbox_new(FALSE, 0);
gtk_paned_pack2(GTK_PANED(hpaned), vbox, TRUE, TRUE);

/* OpenGL box */
sysenv.display_box = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), sysenv.display_box, TRUE, TRUE, 0);

/* create the main drawing area */
if (gl_init_visual())
  exit(1);
canvas_init(sysenv.display_box);
canvas_new(0, 0, sysenv.width, sysenv.height);

/* text info - configurations/import preview */
text_view = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(sysenv.display_box), text_view, TRUE, TRUE, 0);
gui_text_init(text_view);

/* bottom tray */
sysenv.tpane = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), sysenv.tpane, FALSE, FALSE, 0);

/* text summary information */
gui_text_box = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(sysenv.tpane), gui_text_box, FALSE, FALSE, 0);
gui_text_widget(gui_text_box);

/* task summary information */
gui_task_box = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(sysenv.tpane), gui_task_box, FALSE, FALSE, 0);
gui_task_tray(gui_task_box);

/* start message */
text = g_strdup_printf("Welcome to GDIS version %4.2f, Copyright (C) %d by Sean Fleming\n",VERSION,YEAR); 
gui_text_show(WARNING, text);
g_free(text);

/* confirmation callback (ie really exit?) */
g_signal_connect(GTK_OBJECT(window), "delete-event", GTK_SIGNAL_FUNC(gui_exit_confirm), NULL);

/* set default show state */
gtk_widget_show_all(window);
gtk_widget_hide(gui_text_swin);

gui_active_refresh();

gui_display_set(TREE_MODEL);
}

