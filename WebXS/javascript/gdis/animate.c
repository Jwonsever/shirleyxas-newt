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
#include "file.h"
#include "import.h"
#include "coords.h"
#include "model.h"
#include "matrix.h"
#include "measure.h"
#include "numeric.h"
#include "parse.h"
#include "render.h"
#include "interface.h"

/* TODO - use analysis_pak instead for this? */
/* global pak structures */
extern struct sysenv_pak sysenv;

/* NB: objects are allowed to be NULL => keep existing data */
struct animate_pak
{
gdouble latmat[9];
GSList *cores;
GSList *shells;
GSList *velocities;
gpointer camera;

/* CURRENT - just follow the standard property table */
GHashTable *property_table;
/* analysis supplements */
/*
gdouble time;
gdouble energy;
gdouble gradient;
gdouble temperature;
gdouble pressure;
gdouble pe;
gdouble ke;
*/
};

/**********************************************/
/* output info about a model's animation data */
/**********************************************/
void animate_debug(struct model_pak *model)
{
gint i, n, nc, cflag;
gdouble *x, *v;
GList *item;
struct animate_pak *frame;

n = g_list_length(model->animate_list);

printf("Number of frames: %d\n", n);

nc=0;
cflag=FALSE;
i=0;
for (item=model->animate_list ; item ; item=g_list_next(item))
  {
  frame = item->data;

  if (!nc)
    {
    nc = g_slist_length(frame->cores);
printf("First frame # cores: %d\n", nc);
    }
  else
    {
    if (nc != g_slist_length(frame->cores))
      cflag=TRUE;
    }

/* sanity check - display coords of 1st atom for 1st few frames */
  if (i < 10)
    {
    x = g_slist_nth_data(frame->cores, 0);
    v = g_slist_nth_data(frame->velocities, 0);

    if (x)
      printf("x [%d] %f,%f,%f\n", i, x[0], x[1], x[2]);
    if (v)
      printf("v [%d] %f,%f,%f\n", i, v[0], v[1], v[2]);
    }

  i++;
  }

if (cflag)
  {
  printf("animate_debug() Warning: inconsistent number of cores in frames.\n");
  }
}

/**************************************************/
/* view an image file using some external program */
/**************************************************/
/* TODO - put elsewhere, along with povray_exec() etc */
/* TODO - maybe even unify in some standard exec form */
void viewer_exec(const gchar *filename)
{
gchar *cmd, *tmp;

if (!sysenv.viewer_path)
  {
#ifdef __WIN32
  sysenv.viewer_path = g_strdup("mspaint.exe");
#else
  printf("Error: a suitable image viewer was not found in your path.\n");
  return;
#endif
  }

tmp = g_shell_quote(filename);
cmd = g_strdup_printf("%s %s", sysenv.viewer_path, tmp);
g_spawn_command_line_async(cmd, NULL);

g_free(cmd);
g_free(tmp);
}

/*************************************************/
/* create a movie from single frames using ffmeg */
/*************************************************/
/* NB: input should be something like "frame_%06d.png" */
void animate_ffmpeg_exec(const gchar *input, const gchar *output)
{
gint ret;
GString *cmd;

cmd = g_string_new(NULL);

/* CURRENT - trying to control FPS, but codecs (eg mpg) seem very limited */
//g_string_sprintf(cmd, "%s -sameq -y -i %s %s", sysenv.ffmpeg_path, input, output);
/* NB : if input -r is used, needs out -r 25 (or 24 or something supported) to be set */
/* otherwise the codec may complain about unsupported rate */
g_string_sprintf(cmd, "%s -sameq -y -r %.0f -i %s -r 25 %s", sysenv.ffmpeg_path, sysenv.render.delay, input, output);

//printf("movie: [%s]\n", cmd->str);

ret = system(cmd->str);

g_string_free(cmd, TRUE);
}

/*******************************/
/* setup animation for a model */
/*******************************/
void animate_model_init(struct model_pak *model)
{
model->animate_list = NULL;
model->animate_current = 0;
}

/***********************************/
/* free animation data for a model */
/***********************************/
void animate_model_free(struct model_pak *model)
{
GList *list;
struct animate_pak *animate;

g_assert(model != NULL);

//printf("Freeing new animation data...\n");

for (list=model->animate_list ; list ; list=g_list_next(list))
  {
  animate = list->data;

  free_slist(animate->cores);
  free_slist(animate->shells);
  free_slist(animate->velocities);
  g_free(animate->camera);
  g_hash_table_destroy(animate->property_table);

  g_free(animate);
  }
g_list_free(model->animate_list);

/* NEW */
model->camera = model->camera_default;

model->animate_list = NULL;
model->animate_current = 0;
}

/***************/
/* add a frame */
/***************/
gpointer animate_frame_new(struct model_pak *model)
{
struct animate_pak *animate;

animate = g_malloc(sizeof(struct animate_pak));

/* init */
memcpy(animate->latmat, model->latmat, 9*sizeof(gdouble));
// FIXME - should be called coords, not cores
animate->cores = NULL;
animate->shells = NULL;
animate->velocities = NULL;
animate->camera = NULL;

/* analysis data */
/*
animate->time = 0.0;
animate->energy = 0.0;
animate->gradient = 0.0;
animate->temperature = 0.0;
animate->pressure = 0.0;
animate->pe = 0.0;
animate->ke = 0.0;
*/

animate->property_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

model->animate_list = g_list_append(model->animate_list, animate);

return(animate);
}

/*****************************************/
/* append to the frame's coordinate list */
/*****************************************/
void animate_frame_core_add(gdouble *x, gpointer data)
{
gdouble *cx;
struct animate_pak *animate = data;

cx = g_malloc(3*sizeof(gdouble));

ARR3SET(cx, x);

animate->cores = g_slist_append(animate->cores, cx);
}

/*****************************************/
/* append to the frame's coordinate list */
/*****************************************/
void animate_frame_velocity_add(gdouble *x, gpointer data)
{
gdouble *vx;
struct animate_pak *animate = data;

vx = g_malloc(3*sizeof(gdouble));

ARR3SET(vx, x);

animate->velocities = g_slist_append(animate->velocities, vx);
}

/*********************************/
/* set the lattice for the frame */
/*********************************/
void animate_frame_lattice_set(gdouble *lattice, gpointer data)
{
struct animate_pak *animate = data;

g_assert(animate != NULL);
g_assert(lattice != NULL);

memcpy(animate->latmat, lattice, 9*sizeof(gdouble));
}

/***************************************/
/* add a snapshot of the current frame */
/***************************************/
void animate_frame_camera_set(gpointer camera, gpointer data)
{
struct animate_pak *animate = data;

animate->camera = camera;
}

/************************************************/
/* add a new frame - current state of the model */
/************************************************/
/* TODO - fine tune eg turn on/off camera */
/* NEW - called once at end of frame */
#define DEBUG_ANIMATE_FRAME_STORE 0
void animate_frame_store(struct model_pak *model)
{
const gchar *tmp;
GSList *list;
struct core_pak *core;
struct animate_pak *frame;

g_assert(model != NULL);

/* create new frame */
frame = animate_frame_new(model);

#if DEBUG_ANIMATE_FRAME_STORE
printf(" === animate_frame_store() : %d ===\n", g_list_length(model->animate_list));
#endif

if (!model->construct_pbc)
  matrix_lattice_init(model);

/* store latmat */
/* NB: as we only store latmat, construct_pbc must be on when reading back */
animate_frame_lattice_set(model->latmat, frame);

#if DEBUG_ANIMATE_FRAME_STORE
P3MAT("latmat: ", model->latmat);
list=model->cores;
core=list->data;
P3VEC("1st core: ", core->x);
#endif

/* store cores */
for (list=model->cores ; list ; list=g_slist_next(list))
  {
  core = list->data;
  animate_frame_core_add(core->x, frame);
  }

// TODO - keyword enum of these?
/* store properties */
tmp = property_lookup("Time", model);
if (tmp)
  g_hash_table_insert(frame->property_table, "Time", g_strdup(tmp));
//  frame->time = str_to_float(tmp);

tmp = property_lookup("Energy", model);
if (tmp)
  g_hash_table_insert(frame->property_table, "Energy", g_strdup(tmp));
//  frame->energy = str_to_float(tmp);

tmp = property_lookup("Pressure", model);
if (tmp)
  g_hash_table_insert(frame->property_table, "Pressure", g_strdup(tmp));
//  frame->pressure = str_to_float(tmp);
}

/******************************************************/
/* retrieve the nth occurance of a specified property */
/******************************************************/
/* NEW - this (or something like it) will replace analysis stuff */
const gchar *animate_frame_property_get(gint n, const gchar *property, struct model_pak *model)
{
const gchar *value=NULL;
struct animate_pak *frame;

frame = g_list_nth_data(model->animate_list, n);
if (frame)
  value = g_hash_table_lookup(frame->property_table, property);

return(value);
}

/************************************************************/
/* set the value of the specified property in the nth frame */
/************************************************************/
void animate_frame_property_put(gint n, const gchar *property, const gchar *value, struct model_pak *model)
{
struct animate_pak *frame;

frame = g_list_nth_data(model->animate_list, n);

//printf("[%d][%p] : [%s][%s]\n", n, frame, property, value);

if (frame)
  g_hash_table_insert(frame->property_table, g_strdup(property), g_strdup(value));
}

/**********************************************************/
/* replace a property with the value in the current frame */
/**********************************************************/
void animate_property_replace(gint step, const gchar *property, struct model_pak *model)
{
const gchar *value;

value = animate_frame_property_get(step, property, model);
if (value)
  property_replace(property, value, model);
}


/*********************************************************************/
/* update the current model property list with values from nth frame */
/*********************************************************************/
void animate_property_table_update(gint n, struct model_pak *model)
{
const gchar *value;
GList *item, *list;

list = g_hash_table_get_keys(model->property_table);
for (item=list ; item ; item=g_list_next(item))
  {
  value = animate_frame_property_get(n, item->data, model);

//printf("[%s][%s]\n", (gchar *) item->data, value);

  if (value)
    property_replace(item->data, value, model);
  }
}

/**************************/
/* retrieve the nth frame */
/**************************/
#define DEBUG_READ_FRAME 0
gint animate_frame_select(gint n, struct model_pak *model)
{
gdouble *x;
GSList *old, *new;
struct animate_pak *animate;
struct core_pak *core;

if (!model)
  return(FALSE);

/* NEW - models with only 1 frame are not animations */
if (g_list_length(model->animate_list) == 1)
  {
  animate_model_free(model);
  return(TRUE);
  }

/* FIXME - we must have construct pbc on since we only store latmat */
/* this means all frames need to be correctly read BEFORE this routine is called */
animate = g_list_nth_data(model->animate_list, n);

#if DEBUG_READ_FRAME
printf(" *** animate_frame_select(): %d ***\n", n);
#endif

if (animate)
  {
  model->construct_pbc = TRUE;
 
  if (animate->camera)
    model->camera = animate->camera;

  model->animate_current = n;

  if (animate->latmat)
    {
    memcpy(model->latmat, animate->latmat, 9*sizeof(gdouble));
    matrix_lattice_init(model);
    }

#if DEBUG_READ_FRAME
P3MAT("latmat:", animate->latmat);
printf("model # old cores: %d\n", g_slist_length(old));
printf("model # new cores: %d\n", g_slist_length(new));
P3VEC("1st core: ", (gdouble *) new->data);
#endif

/* replace coordinates */
  old = model->cores;
  new = animate->cores;
  while (old && new)
    {
    x = new->data;
    core = old->data;

/* set the new coordinates */
/* fractional/cartesian */
    ARR3SET(core->x, x);
    if (!model->fractional)
      vecmat(model->ilatmat, core->x);

    old = g_slist_next(old);
    new = g_slist_next(new);
    }

/* NEW - replace velocities */
  old = model->cores;
  new = animate->velocities;

  while (old && new)
    {
    x = new->data;
    core = old->data;

/* set the new velocity */
    ARR3SET(core->v, x);

    old = g_slist_next(old);
    new = g_slist_next(new);
    }

/* NEW - update any properties */
/* TODO - iterate through list of properties and replace */

animate_property_table_update(n, model);

/*
   animate_property_replace(n, "Time", model);
   animate_property_replace(n, "Energy", model);
   animate_property_replace(n, "Temperature", model);
   animate_property_replace(n, "Pressure", model);
*/

/* NB: initial frame store misses all the coords_confine_ stuff that */
/* happens after in model prep/init -> can we store this? for now - redo */
  coords_confine_cores(model->cores, model);
  connect_midpoints(model);

  if (sysenv.render.connect_redo)
    {
    connect_bonds(model);
    }

/* TODO - call coords_confine_cores(model->cores, model); */
/* if confine atoms to pbc is desired */

  connect_molecules(model);
  coords_compute(model);

  measure_refresh(model);

  return(TRUE);
  }

return(FALSE);
}

/**************************************/
/* structure for movie rendering jobs */
/**************************************/
struct animate_render_pak
{
/* common */
gchar *filename;
gchar *output_path;
struct model_pak *model;

/* frame info */
gint range[3];

/* phonon info */
gint mode;
gdouble resolution;
gdouble scale;
};

/********************************/
/* allocate rendering structure */
/********************************/
gpointer animate_render_new(const gchar *name, struct model_pak *model)
{
gpointer import;
struct animate_render_pak *data;

g_assert(name != NULL);

data = g_malloc(sizeof(struct animate_render_pak));

data->filename = g_strdup(name);
data->model = model;

import = import_find(IMPORT_MODEL, model);
if (import)
  data->output_path = g_strdup(import_path(import));
else
  data->output_path = NULL;

data->range[0]=-1;
data->range[1]=-1;
data->range[2]=-1;

data->mode=0;
data->resolution=1.0;
data->scale=1.0;

return(data);
}

/****************************/
/* free rendering structure */
/****************************/
void animate_render_free(gpointer ptr)
{
struct animate_render_pak *data=ptr;

g_free(data->filename);
g_free(data->output_path);
g_free(data);
}

/*****************************/
/* update a render structure */
/*****************************/
void animate_render_range_set(gint *range, gpointer ptr)
{
struct animate_render_pak *data=ptr;

g_assert(data != NULL);

data->range[0]=range[0];
data->range[1]=range[1];
data->range[2]=range[2];
}

/*****************************/
/* update a render structure */
/*****************************/
void animate_render_phonon_set(gint mode, gdouble resolution, gdouble scale, gpointer ptr)
{
struct animate_render_pak *data=ptr;

g_assert(data != NULL);

data->mode = mode;
data->resolution = resolution;
data->scale = scale;
}

/*************************************/
/* interruptable single frame render */
/*************************************/
gint animate_render_single2_cleanup(gpointer ptr)
{
gchar *img;
struct animate_render_pak *data=ptr;

// TODO - check if killed
if (!sysenv.render.no_povray_exec)
  {
  img = parse_extension_set(data->filename, "png");
  viewer_exec(img);

  g_free(data->filename);
  data->filename = img;
  }

// TODO - free
//animate_render_cleanup(ptr);

return(0);
}

/************************************/
/* non-blocking single frame render */
/************************************/
gint animate_render_single2(gpointer ptr)
{
gint pid=-1;
gchar *pov, *path;
struct animate_render_pak *data=ptr;

/* write scene */
path = file_unused_directory();
pov = "dummy.pov";
g_free(data->filename);
data->filename = g_build_filename(path, pov, NULL);
write_povray(data->filename, data->model);

/* render scene */
if (!sysenv.render.no_povray_exec)
  pid = povray_exec_async(path, pov);

return(pid);
}

/**********************************/
/* background single frame render */
/**********************************/
void animate_render_single(gpointer ptr)
{
gchar *pov, *img, *path;
//const gchar *path;
struct animate_render_pak *data=ptr;

g_assert(data->model != NULL);

/* write scene */
path = file_unused_directory();
pov = "dummy.pov";
g_free(data->filename);
data->filename = g_build_filename(path, pov, NULL);
write_povray(data->filename, data->model);

/* render scene */
if (!sysenv.render.no_povray_exec)
  {
  povray_exec(path, pov);

/* pov input file cleanup */
  if (sysenv.render.no_keep_tempfiles)
    g_remove(data->filename);

/* construct viewing task command */
/* FIXME - may not be png? ... */
  img = parse_extension_set(data->filename, "png");
  viewer_exec(img);

/* set filename to result */
  g_free(data->filename);
  data->filename = img;
  }

g_free(path);
}

/****************************/
/* background job rendering */
/****************************/
void animate_render_movie(gpointer ptr)
{
gint i, n;
gchar *tmpdir, *base, *text, *name;
GSList *item, *list;
struct model_pak *model;
struct animate_render_pak *data=ptr;

g_assert(data != NULL);

model = data->model;
base = parse_strip_extension(data->filename);

g_assert(base != NULL);

tmpdir = file_unused_directory();

//printf("movie tmpdir: %s\n", tmpdir);

n = g_list_length(model->animate_list);

/* this part is vulnerable to model being deleted */
//printf("Writing %d frames [%s] \n", n, base);
list=NULL;
//for (i=0 ; i<n ; i++)
for (i=data->range[0] ; i<=data->range[1] ; i+=data->range[2])
  {
  text = g_strdup_printf("%s_%06d.pov", base, i);
//  name = g_build_filename(sysenv.cwd, text, NULL);
  name = g_build_filename(tmpdir, text, NULL);
  g_free(text);

  animate_frame_select(i, model);
  write_povray(name, model);

  list = g_slist_prepend(list, name);
  }
//printf("Writing done.\n");

/* this part is indep of model pointer */
if (!sysenv.render.no_povray_exec)
  {
//printf("Rendering %d frames [%s] \n", n, base);
  for (item=list ; item ; item=g_slist_next(item))
    {
    name = item->data;
    povray_exec(tmpdir, name);
    }

//printf("Rendering done.\n");

/* stitch images together */
/* see gui_animate.c */

//printf("Creating movie ...\n");

  if (sysenv.ffmpeg_path)
    {
    gchar *name, *input, *output;

    name = g_strdup_printf("%s_%%06d.png", base);
    input = g_build_filename(tmpdir, name, NULL);

    if (data->output_path)
      output = g_build_filename(data->output_path, data->filename, NULL);
    else
      output = g_build_filename(sysenv.cwd, data->filename, NULL);

    g_free(data->filename);
    data->filename = output;

    animate_ffmpeg_exec(input, data->filename);

//    animate_ffmpeg_exec(input, output);

    g_free(input);
    g_free(name);
    }

/*
  if (sysenv.convert_path)
    {
    text = g_strdup_printf("%s -quality %d -delay %d %s_*.png %s.mpg",
                          sysenv.convert_path, (gint) sysenv.render.mpeg_quality,
                          (gint) sysenv.render.delay,
                          base, base);
    system(text);
    g_free(text);
    }
  else
    {
    printf("Missing program \"convert\" from the ImageMagick suite.\n");
    }
*/
  }

/* file cleanup */
if (sysenv.render.no_keep_tempfiles)
  {
  for (item=list ; item ; item=g_slist_next(item))
    {
    g_remove(item->data);
    }
  }

g_free(tmpdir);
g_free(base);

free_slist(list);
}

/***************************************/
/* background job for phonon rendering */
/***************************************/
void animate_render_phonon(gpointer ptr)
{
gint i, n;
gchar *tmpdir, *name, *text;
gdouble amp, pulse, step, *xarray;
GSList *item, *list, *frames, *xlist, *ylist, *zlist;
struct animate_render_pak *data=ptr;
struct core_pak *core, *prim;
struct model_pak *model;

printf("creating phonon movie: %s\n", data->filename);

/*
printf("mode: %d\n", data->mode);
printf("scale: %f\n", data->scale);
printf("resolution: %f\n", data->resolution);
*/

model = data->model;

step = 2.0*PI / data->resolution;

tmpdir = file_unused_directory();
//printf("movie tmpdir: %s\n", tmpdir);

/* save */
n = g_slist_length(model->cores);
xarray = g_malloc(3*n*sizeof(gdouble));
i=0;
for (list=model->cores ; list; list=g_slist_next(list))
  {
  core = list->data;
  xarray[i++] = core->x[0];
  xarray[i++] = core->x[1];
  xarray[i++] = core->x[2];
  }

/* generate pov files for the vibrational mode */
i=0;
frames=NULL;
for (pulse=0.0 ; pulse<2.0*PI ; pulse+=step)
  {
  amp = data->scale * tbl_sin(pulse);

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

/* add vibrational eigenvector offset */
    ptr = g_slist_nth_data(xlist, data->mode-1);
    if (ptr)
      core->offset[0] = *((gdouble *) ptr);

    ptr = g_slist_nth_data(ylist, data->mode-1);
    if (ptr)
      core->offset[1] = *((gdouble *) ptr);

    ptr = g_slist_nth_data(zlist, data->mode-1);
    if (ptr)
      core->offset[2] = *((gdouble *) ptr);

/* pulse offset scaling */
    VEC3MUL(core->offset, amp);
    vecmat(model->ilatmat, core->offset);
    }

/* recalc coords and adjust bond orientations */
  coords_compute(model);
  connect_midpoints(model);

  text = g_strdup_printf("dummy_%06d.pov", i);
  name = g_build_filename(tmpdir, text, NULL);
  g_free(text);
  write_povray(name, model);
  frames = g_slist_prepend(frames, name);

  i++;
  }

/* restore */
i=0;
for (list=model->cores ; list; list=g_slist_next(list))
  {
  core = list->data;
  core->x[0] = xarray[i++];
  core->x[1] = xarray[i++];
  core->x[2] = xarray[i++];
  }
g_free(xarray);
coords_compute(model);
connect_midpoints(model);

/* render */
/* this part is indep of model pointer */
if (!sysenv.render.no_povray_exec)
  {
  for (item=frames ; item ; item=g_slist_next(item))
    {
    name = item->data;
    povray_exec(tmpdir, name);
    }

//printf("Rendering done.\n");
//printf("Creating movie ...\n");

  if (sysenv.ffmpeg_path)
    {
    gchar *name, *input, *output;

    name = g_strdup("dummy_%06d.png");
    input = g_build_filename(tmpdir, name, NULL);

    if (data->output_path)
      output = g_build_filename(data->output_path, data->filename, NULL);
    else
      output = g_build_filename(sysenv.cwd, data->filename, NULL);

    g_free(data->filename);
    data->filename = output;

    animate_ffmpeg_exec(input, data->filename);

    g_free(input);
    g_free(name);
    }
  }

if (sysenv.render.no_keep_tempfiles)
  {
/* TODO - file cleanup - remove tmpdir */
/* g_rmdir wont work - and cant do an -rf */
/*
 printf("cleanup: %d\n", g_rmdir(tmpdir));
*/
  }

/* cleanup */
free_slist(frames);
g_free(tmpdir);
}

/************************************/
/* background job rendering cleanup */
/************************************/
void animate_render_cleanup(gpointer ptr)
{
gchar *text;
struct animate_render_pak *data=ptr;
struct model_pak *model;

g_assert(data != NULL);

model = data->model;
model->animating = FALSE;

text = g_strdup_printf("Created: %s\n", data->filename);
gui_text_show(STANDARD, text);
g_free(text);

animate_render_free(ptr);
}

