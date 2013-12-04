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

#include "gdis.h"
#include "coords.h"
#include "model.h"
#include "edit.h"
#include "file.h"
#include "render.h"
#include "matrix.h"
#include "parse.h"
#include "quaternion.h"
#include "measure.h"
#include "numeric.h"
#include "spatial.h"
#include "opengl.h"
#include "gui_shorts.h"
#include "interface.h"
#include "dialog.h"
#include "animate.h"
#include "task.h"

#define DEBUG 0
#define DEBUG2 0

/* global pak structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

GtkWidget *cf_spin;
gchar *entry_frame_start=NULL, *entry_frame_stop=NULL, *entry_frame_step=NULL;
gchar *entry_movie_type;
gint animate_phonons=FALSE;
GtkWidget *box_phonon_options;
gchar *entry_current_mode=NULL;
//gdouble displayed_phonon=1.0;
gdouble gui_animate_fps=25.0;

/***********************************/
/* pbc confinement option handlers */
/***********************************/
void atom_pbc(struct model_pak *model)
{
model->anim_confine = PBC_CONFINE_ATOMS;
}
void mol_pbc(struct model_pak *model)
{
model->anim_confine = PBC_CONFINE_MOLS;
}
void no_pbc(struct model_pak *model)
{
model->anim_confine = PBC_CONFINE_NONE;
}

/************************/
/* display eigenvectors */
/************************/
// not really an animate function as such...
void gui_animate_phonon_vectors(struct model_pak *model)
{
gdouble *v, x1[3], x2[3], colour[3];
gpointer spatial, ptr;
GSList *list, *xlist, *ylist, *zlist;
struct core_pak *core, *prim;

if (!model)
  return;

/* create & init the spatial object */
spatial_destroy_by_label("phonons", model);
spatial = spatial_new("phonons", SPATIAL_VECTOR, 2, TRUE, model);

/* get eigenvectors from all atoms */
for (list=model->cores ; list; list=g_slist_next(list))
  {
  core = list->data;
  ARR3SET(x1, core->x);

/* get eigenvector list */
  if (core->primary)
    {
    xlist = core->vibx_list; 
    ylist = core->viby_list; 
    zlist = core->vibz_list; 
    }
  else
    {
    prim = core->primary_core;
    xlist = prim->vibx_list; 
    ylist = prim->viby_list; 
    zlist = prim->vibz_list; 
    }

  if (!xlist || !ylist || !zlist)
    {
    printf("Missing phonon eigenvectors.\n");
    return;
    }

/* vibrational eigenvector */
/* NB: current_phonon starts from 1 */
  ptr = g_slist_nth_data(xlist, model->current_phonon-1);
  if (ptr)
    x2[0] = *((gdouble *) ptr);

  ptr = g_slist_nth_data(ylist, model->current_phonon-1);
  if (ptr)
    x2[1] = *((gdouble *) ptr);

  v = g_slist_nth_data(zlist, model->current_phonon-1);
  if (ptr)
    x2[2] = *v;

/* pulse offset scaling */
  VEC3MUL(x2, sysenv.render.phonon_scaling);
  vecmat(model->ilatmat, x2);

/* TODO - unify with siesta */
/*
  i = model->siesta.sorted_eig_values[model->siesta.current_animation];
  x2[0] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom, i);
  x2[1] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom+1, i);
  x2[2] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom+2, i);
  atom++;
*/

/* compute coords */
  ARR3ADD(x2, x1);

/* add to spatial */
  spatial_vertex_add(x2, colour, spatial);
  spatial_vertex_add(x1, colour, spatial);
  }

coords_compute(model);

gui_refresh(GUI_CANVAS);
}

/***********************************************/
/* refresh the animation range from the dialog */
/***********************************************/
void gui_animate_range_get(gint *range, struct model_pak *model)
{
gint i, max;

range[0] = str_to_float(entry_frame_start);
range[1] = str_to_float(entry_frame_stop);
range[2] = str_to_float(entry_frame_step);

//printf("gui: %d-%d , %d\n", range[0], range[1], range[2]);

/* get current model and check */
if (model)
  {
  max = g_list_length(model->animate_list);

/* following the convention of 1 as first frame ... */
  if (range[0] > 0)
    range[0]--;
  if (range[1] > 0)
    range[1]--;
  else
    range[1] = max-1;

/* check nothing is greater than actual number of frames */
  for (i=3 ; i-- ; )
    {
    if (range[i] > max-1)
      range[i] = max-1;
    }

/* max >= min check */
  if (range[1] < range[0])
    range[1] = max-1;
  }

/* undefined defaults */
if (range[1] < 1)
  range[1] = 1;
if (range[2] < 1)
  range[2] = 1;

//printf("gui_animate_range(): %d-%d , %d\n", range[0], range[1], range[2]);

}

/***********************/
/* animation primitive */
/***********************/
void gui_animate_first(void)
{
gint range[3];
struct model_pak *model;

model = tree_model_get();
if (model)
  {
  if (animate_phonons)
    model->current_phonon = 1;
  else
    {
    gui_animate_range_get(range, model);
    animate_frame_select(range[0], model);
    }
  gui_refresh(GUI_CANVAS | GUI_ACTIVE);
  }
}

/***********************/
/* animation primitive */
/***********************/
void gui_animate_last(void)
{
gint range[3];
struct model_pak *model;

model = tree_model_get();
if (model)
  {
  if (animate_phonons)
    model->current_phonon = model->num_phonons;
  else
    {
    gui_animate_range_get(range, model);
    animate_frame_select(range[1], model);
    }
  gui_refresh(GUI_CANVAS | GUI_ACTIVE);
  }
}

/***********************/
/* animation primitive */
/***********************/
void gui_animate_back(void)
{
gint range[3];
struct model_pak *model;

model = tree_model_get();
if (model)
  {
  if (animate_phonons)
    {
    if (model->current_phonon > 1)
      {
      model->current_phonon--;
/* if we're not animating - re-draw eigenvectors */
      if (!model->animating)
        gui_animate_phonon_vectors(model);
      }
    }
  else
    {
    gui_animate_range_get(range, model);

/* subtract step */
    model->animate_current -= range[2];

/* enforce maximum */
    if (model->animate_current > range[1])
      model->animate_current = range[1];

/* enforce minimum */
    if (model->animate_current > range[0]-1)
      animate_frame_select(model->animate_current, model);
    else
      animate_frame_select(range[0], model);
    }

  gui_refresh(GUI_CANVAS | GUI_ACTIVE);
  }
}

/***********************/
/* animation primitive */
/***********************/
void gui_animate_next(void)
{
gint range[3];
struct model_pak *model;

model = tree_model_get();
if (model)
  {
  if (animate_phonons)
    {
    if (model->current_phonon < model->num_phonons)
      {
      model->current_phonon++;
/* if we're not animating - re-draw eigenvectors */
      if (!model->animating)
        gui_animate_phonon_vectors(model);
      }
    }
  else
    {
    gui_animate_range_get(range, model);

/* add step */
    model->animate_current += range[2];

/* enforce minimum */
    if (model->animate_current < range[0])
      model->animate_current = range[0];

/* enforce maximum */
    if (model->animate_current < range[1]+1)
      animate_frame_select(model->animate_current, model);
    else
      animate_frame_select(range[1], model);
    }

  gui_refresh(GUI_CANVAS | GUI_ACTIVE);
  }
}

/****************************/
/* phonon animation cleanup */
/****************************/
void gui_phonon_cleanup(struct model_pak *model)
{
GSList *list;
struct core_pak *core;

g_assert(model != NULL);

/* reset offsets */
for (list=model->cores ; list; list=g_slist_next(list))
  {
  core = list->data;
  VEC3SET(core->offset, 0.0, 0.0, 0.0);
  }
model->animating = FALSE;

/* recalc coords and adjust bond orientations */
coords_compute(model);
connect_midpoints(model);
redraw_canvas(SINGLE);
}

/******************************/
/* phonon animation primitive */
/******************************/
#define DEBUG_PHONON_LOOP 1
gint gui_phonon_loop(void)
{
gint current_phonon;
gdouble f;
gpointer ptr;
GSList *list, *xlist, *ylist, *zlist;
struct core_pak *core, *prim;
struct model_pak *model;

model = tree_model_get();
if (!model)
  return(FALSE);
if (!model->animating)
  {
  gui_phonon_cleanup(model);
  return(FALSE);
  }

//current_phonon = displayed_phonon;
current_phonon = model->current_phonon;

/* NB: list numbering starts from 0 */
ptr = g_slist_nth_data(model->phonons, current_phonon-1);
if (!ptr)
  {
#if DEBUG_PHONON_LOOP
printf("Bad phonon %d/%d\n",  current_phonon, g_slist_length(model->phonons));
#endif
  gui_phonon_cleanup(model);
  return(FALSE);
  }
if (!model->pulse_direction)
  {
  gui_phonon_cleanup(model);
  return(FALSE);
  }

/* setup scaling for this step */
model->pulse_count += model->pulse_direction;
if (model->pulse_count <= -sysenv.render.phonon_resolution)
  {
  model->pulse_count = -sysenv.render.phonon_resolution;
  model->pulse_direction = 1;
  }
if (model->pulse_count >= sysenv.render.phonon_resolution)
  {
  model->pulse_count = sysenv.render.phonon_resolution;
  model->pulse_direction = -1;
  }

f = sysenv.render.phonon_scaling;
f *= (gdouble) model->pulse_count;
f /= sysenv.render.phonon_resolution;

/* get eigenvectors from all atoms */
for (list=model->cores ; list; list=g_slist_next(list))
  {
  core = list->data;

/* get eigenvector list */
  if (core->primary)
    {
    xlist = core->vibx_list; 
    ylist = core->viby_list; 
    zlist = core->vibz_list; 
    }
  else
    {
    prim = core->primary_core;
    xlist = prim->vibx_list; 
    ylist = prim->viby_list; 
    zlist = prim->vibz_list; 
    }
  g_assert(xlist != NULL);
  g_assert(ylist != NULL);
  g_assert(zlist != NULL);

/* vibrational eigenvector */
  ptr = g_slist_nth_data(xlist, current_phonon-1);
  if (!ptr)
    {
    gui_phonon_cleanup(model);
    return(FALSE);
    }
  core->offset[0] = *((gdouble *) ptr);

  ptr = g_slist_nth_data(ylist, current_phonon-1);
  if (!ptr)
    {
    gui_phonon_cleanup(model);
    return(FALSE);
    }
  core->offset[1] = *((gdouble *) ptr);

  ptr = g_slist_nth_data(zlist, current_phonon-1);
  if (!ptr)
    {
    gui_phonon_cleanup(model);
    return(FALSE);
    }
  core->offset[2] = *((gdouble *) ptr);

/* pulse offset scaling */
  VEC3MUL(core->offset, f);
  vecmat(model->ilatmat, core->offset);

/* TODO - shell also? */
  }

/* recalc coords and adjust bond orientations */
coords_compute(model);
connect_midpoints(model);
redraw_canvas(SINGLE);

return(TRUE);
}

/***********************/
/* animation primitive */
/***********************/
gint gui_frame_loop(void)
{
gint range[3];
struct model_pak *model;

model = tree_model_get();
if (model)
  {
  if (model->animating)
    {
    gui_animate_range_get(range, model);

/* add step */
    model->animate_current += range[2];

/* enforce minimum */
  if (model->animate_current < range[0])
    model->animate_current = range[0];

/* loop on maximum */
    if (model->animate_current < range[1]+1)
      animate_frame_select(model->animate_current, model);
    else
      animate_frame_select(range[0], model);

    gui_refresh(GUI_CANVAS | GUI_ACTIVE);
    return(TRUE);
    }
  }
return(FALSE);
}

/***********************/
/* animation primitive */
/***********************/
void gui_animate_play(void)
{
gint t;
struct model_pak *model;

model = tree_model_get();
if (model)
  {
  model->animating = TRUE;

/* increase alpha to make the drop sharper moving to higher resolutions */
#define SCALE_ALPHA 0.08
#define DELAY_MIN 5
#define DELAY_MAX 100

/* NB: phonon resolution range: [1,100] */
/*
x = SCALE_ALPHA * (sysenv.render.phonon_resolution-1.0);
x = DELAY_MAX * pow(10.0, -x);
t = CLAMP(x, DELAY_MIN, DELAY_MAX);
*/

/* CURRENT - use desired FPS to get timeout value (ms) */
t = 1000 / gui_animate_fps;

//printf("anim : %f -> %d\n", sysenv.render.phonon_resolution, t);

  if (animate_phonons)
    {
    model->pulse_count = 0;
    model->pulse_direction = 1;
/* hide eigenvectors while animating */
    spatial_destroy_by_label("phonons", model);

/* HACK - quick fix to better align phonon animation with resulting movie */
/* FIXME - work it out properly */
t *= 4.0 / sysenv.render.phonon_resolution;

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
    gdk_threads_add_timeout(t, (GSourceFunc) &gui_phonon_loop, NULL);
#else
    g_timeout_add(t, (GSourceFunc) &gui_phonon_loop, NULL);
#endif
    }
  else
    {
#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
    gdk_threads_add_timeout(t, (GSourceFunc) &gui_frame_loop, NULL);
#else
    g_timeout_add(t, (GSourceFunc) &gui_frame_loop, NULL);
#endif
    }
  }
}

/***********************/
/* animation primitive */
/***********************/
void gui_animate_stop(void)
{
struct model_pak *model;

model = tree_model_get();
if (model)
  {
  model->animating = FALSE;
/* redraw eigenvectors if we're in phonon display mode */
  if (animate_phonons)
    gui_animate_phonon_vectors(model);
  }
}

/************************************/
/* render a frame in the background */
/************************************/
void gui_animate_render(GtkWidget *w, gpointer dummy)
{
gpointer data;
struct model_pak *model;

model = tree_model_get();
if (model)
  {
  data = animate_render_new(sysenv.render.animate_file, model);

//  task_new("Rendering", &animate_render_single, data, &animate_render_cleanup, data, NULL);
  task_add("Rendering2", &animate_render_single2, data, &animate_render_single2_cleanup, data, NULL);
  }
else
  printf("No active model to render.\n");
}

/******************************/
/* movie type change callback */
/******************************/
void gui_animate_movie_type_change(GtkWidget *type, gpointer name)
{
gchar *movie;
const gchar *text, *ext;

text = gtk_entry_get_text(GTK_ENTRY(type));

//printf("type = %s\n", text);

if (g_strncasecmp(text, "avi", 3) == 0)
  ext = "avi";
if (g_strncasecmp(text, "flv", 3) == 0)
  ext = "flv";
if (g_strncasecmp(text, "mp4", 3) == 0)
  ext = "mp4";
if (g_strncasecmp(text, "mpg", 3) == 0)
  ext = "mpg";

text = gtk_entry_get_text(GTK_ENTRY(name));

movie = parse_extension_set(text, ext);

//printf("movie = %s\n", movie);

gtk_entry_set_text(GTK_ENTRY(name), movie);

g_free(movie);
}

/************************************/
/* render a movie in the background */
/************************************/
void gui_animate_movie(GtkWidget *w, gpointer dummy)
{
gint current_phonon;
gint range[3];
gpointer data;
struct model_pak *model;

model = tree_model_get();
if (model)
  {

if (model->animating)
  {
  printf("Can't create movie while animation is in progress.\n");
  return;
  }
else
  {

/* lock */
  model->animating = TRUE;

/* create structure with task details */
  data = animate_render_new(sysenv.render.animate_file, model);
  gui_animate_range_get(range, model);
  animate_render_range_set(range, data);
//  current_phonon = displayed_phonon;
  current_phonon = model->current_phonon;
  animate_render_phonon_set(current_phonon,
                            sysenv.render.phonon_resolution,
                            sysenv.render.phonon_scaling, data);

/* CURRENT - hack to adjust ffmpeg movie speed */
  sysenv.render.delay = gui_animate_fps;

/* execute task */
  if (animate_phonons)
    task_new("Rendering", &animate_render_phonon, data, &animate_render_cleanup, data, NULL);
  else
    task_new("Rendering", &animate_render_movie, data, &animate_render_cleanup, data, NULL);
  }
  }
else
  printf("No active model to render.\n");
}

/**********************************/
/* animation type change callback */
/**********************************/
void gui_animate_type_change(GtkWidget *w,  gpointer data)
{
struct model_pak *model;

animate_phonons ^= 1;

model = tree_model_get();

if (animate_phonons)
  {
  if (model)
    model->current_phonon=1;

  gui_animate_phonon_vectors(model);

  gtk_widget_show(box_phonon_options);
  }
else
  {
  if (model)
    model->current_phonon=-1;

  gtk_widget_hide(box_phonon_options);
  }

gui_refresh(GUI_CANVAS);
}

/***************************************/
/* frame selection slider change event */
/***************************************/
void gui_animate_frame_select(GtkWidget *w, gpointer data)
{
gint i;
gdouble f, max;
struct model_pak *model;

/* TODO - if range is not enough to cover the number of values (test below) */
/* then change the range accordingly */

f = gtk_range_get_value(GTK_RANGE(w));

model = tree_model_get();
if (model)
  {
  if (animate_phonons)
    {
    max = model->num_phonons;
    f *= max/100.0;
    model->current_phonon = f;

/* if we're not animating - re-draw eigenvectors */
    if (!model->animating)
      gui_animate_phonon_vectors(model);
    }
  else
    {
    max = g_list_length(model->animate_list);
    f *= max/100.0;
    i = f;
    animate_frame_select(i, model);
    }

  gui_refresh(GUI_CANVAS | GUI_ACTIVE);
  }
}

/***********************/
/* configure animation */
/***********************/
void gui_animate_dialog(void)
{
gpointer dialog;
GtkWidget *window, *main_vbox, *table, *vbox, *hbox;
GtkWidget *notebook, *page, *label, *entry, *scale;
GList *list;

/* prevent recording whilst in animation */
gui_mode_switch(FREE);

/* dialog setup */
dialog = dialog_request(ANIM, "Animation and Rendering", NULL, NULL, NULL);
if (!dialog)
  return;
window = dialog_window(dialog);
gtk_widget_set_size_request(window, 350, -1);

/* notebook frame */
main_vbox = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(GTK_DIALOG(window)->vbox), main_vbox);

/* create notebook */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, main_vbox);
notebook = gtk_notebook_new();
gtk_box_pack_start(GTK_BOX(vbox),notebook,FALSE,TRUE,0);
gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), TRUE);

/* page */
page = gtk_vbox_new(FALSE, PANEL_SPACING);
label = gtk_label_new("Control");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

vbox = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(page),vbox,FALSE,FALSE,0);

/* format pulldown */
animate_phonons=FALSE;
list = NULL;
list = g_list_append(list, "Frames");
list = g_list_append(list, "Phonons");
hbox = gtk_hbox_new(FALSE, 0);
label = gtk_label_new("Display ");
gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
entry = gui_pulldown_new(NULL, list, 0, hbox);
g_list_free(list);
g_signal_connect(GTK_OBJECT(entry), "changed",
                 GTK_SIGNAL_FUNC(gui_animate_type_change), (gpointer) entry);
gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

/* phonon options */
box_phonon_options = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(vbox),box_phonon_options,FALSE,FALSE,0);

gui_direct_spin("Phonon scaling ", &sysenv.render.phonon_scaling,
                0.1, 9.9, 0.1, NULL, NULL, box_phonon_options);
gui_direct_spin("Phonon resolution ", &sysenv.render.phonon_resolution,
                10.0, 100.0, 1.0, NULL, NULL, box_phonon_options);

gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);


/* CURRENT - FPS */
gui_direct_spin("Apparent FPS for animation ", &gui_animate_fps, 1, 50, 1, NULL, NULL, vbox);

/* connectivity options */
gui_direct_check("Recalculate connectivity", &sysenv.render.connect_redo, NULL, NULL, vbox);

/*
gui_checkbox("Don't recalculate scale", NULL, vbox);
gui_checkbox("Loop", NULL, vbox);
*/

/* actions at start of each cycle */
/*
new_radio_group(0, vbox, FF);

button = add_radio_button("Confine atoms to PBC", (gpointer) atom_pbc, data);
if (data->anim_confine == PBC_CONFINE_ATOMS)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);

button = add_radio_button("Confine mols to PBC", (gpointer) mol_pbc, data);
if (data->anim_confine == PBC_CONFINE_MOLS)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);

button = add_radio_button("Cell confinement off", (gpointer) no_pbc, data);
if (data->anim_confine == PBC_CONFINE_NONE)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
*/

/* page */
page = gtk_vbox_new(FALSE, PANEL_SPACING);
label = gtk_label_new("Render");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

vbox = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(page),vbox,FALSE,FALSE,0);

gui_direct_spin("Width", &sysenv.render.width, 100, 2000, 100, NULL, NULL, vbox);
gui_direct_spin("Height", &sysenv.render.height, 100, 2000, 100, NULL, NULL, vbox);

gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);

gui_direct_check("Shadowless", &sysenv.render.shadowless, NULL, NULL, vbox);
gui_direct_check("Create povray input then stop", &sysenv.render.no_povray_exec, NULL, NULL, vbox);
gui_direct_check("Cleanup temporary files", &sysenv.render.no_keep_tempfiles, NULL, NULL, vbox);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
gui_button("  Render  ", gui_animate_render, NULL, hbox, TF);

/* page */
page = gtk_vbox_new(FALSE, PANEL_SPACING);
label = gtk_label_new("Movie");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

vbox = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(page),vbox,FALSE,FALSE,0);

table = gtk_table_new(4, 4, FALSE);
gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,0);

/* name label */
label = gtk_label_new("Filename");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);

/* name entry */
/*
entry = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(entry), sysenv.render.animate_file);
*/
entry = gui_text_entry(NULL, &sysenv.render.animate_file, TRUE, FALSE, NULL);
gtk_table_attach_defaults(GTK_TABLE(table),entry,1,2,0,1);

/* format label */
label = gtk_label_new("Format");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);

/* format pulldown */
list=NULL;
list = g_list_append(list, "mpg");
list = g_list_append(list, "mp4");
list = g_list_append(list, "flv");
list = g_list_append(list, "avi");
hbox = gtk_hbox_new(FALSE, 0);
entry_movie_type = gui_pulldown_new(NULL, list, 0, hbox);
g_list_free(list);
g_signal_connect(GTK_OBJECT(entry_movie_type), "changed",
                 GTK_SIGNAL_FUNC(gui_animate_movie_type_change), (gpointer) entry);
gtk_table_attach_defaults(GTK_TABLE(table),hbox,1,2,1,2);


/* update hook */
g_signal_connect(GTK_OBJECT(entry), "changed",
                 GTK_SIGNAL_FUNC(event_render_modify), (gpointer) entry);
g_object_set_data(G_OBJECT(entry), "id", (gpointer) ANIM_NAME);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
gui_button("  Create  ", gui_animate_movie, NULL, hbox, TF);

/* slider for current frame */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, main_vbox);
scale = gtk_hscale_new_with_range(1.0, 100.0, 1.0);
gtk_range_set_update_policy(GTK_RANGE(scale), GTK_UPDATE_CONTINUOUS);
gtk_box_pack_start(GTK_BOX(vbox), scale, TRUE, TRUE, 0);
g_signal_connect(GTK_OBJECT(scale), "value_changed",
                 GTK_SIGNAL_FUNC(gui_animate_frame_select), NULL);

gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);

/* animation limits */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, main_vbox);
gui_text_entry("First ", &entry_frame_start, TRUE, FALSE, vbox);
gui_text_entry("Last ", &entry_frame_stop, TRUE, FALSE, vbox);
gui_text_entry("Step ", &entry_frame_step, TRUE, FALSE, vbox);

/* control buttons */
hbox = gtk_hbox_new(TRUE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(main_vbox), hbox, FALSE, FALSE, 0);

gui_icon_button("GDIS_REWIND", NULL,
                  gui_animate_first, NULL,
                  hbox);

gui_icon_button("GDIS_STEP_BACKWARD", NULL,
                  gui_animate_back, NULL,
                  hbox);

gui_icon_button("GDIS_PLAY", NULL,
                  gui_animate_play, NULL,
                  hbox);

gui_icon_button("GDIS_PAUSE", NULL,
                   gui_animate_stop, NULL,
                  hbox);

gui_icon_button("GDIS_STEP_FORWARD", NULL,
                  gui_animate_next, NULL,
                  hbox);

gui_icon_button("GDIS_FASTFORWARD", NULL,
                  gui_animate_last, NULL,
                  hbox);

gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Animation and Rendering", GTK_DIALOG(window)->action_area);

/* terminating button */
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog,
                   GTK_DIALOG(window)->action_area);

/* display the dialog */
gtk_widget_show_all(window);

gtk_widget_hide(box_phonon_options);
}

