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
#include <sys/stat.h>

#include "gdis.h"
#include "coords.h"
#include "matrix.h"
#include "edit.h"
#include "error.h"
#include "file.h"
#include "graph.h"
#include "morph.h"
#include "model.h"
#include "measure.h"
#include "project.h"
#include "analysis.h"
#include "animate.h"
#include "render.h"
#include "opengl.h"
#include "select.h"
#include "space.h"
#include "surface.h"
#include "spatial.h"
#include "interface.h"
#include "dialog.h"
#include "undo.h"
#include "zmatrix.h"
#include "zone.h"

/* the main pak structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];
/* main model database */
struct model_pak *models[MAX_MODELS];

/***********************************/
/* unified initialization/cleaning */
/***********************************/
void model_init(struct model_pak *data)
{
static gint n=0;

/* NB: all the important defines should go here */
/* ie those values that might cause a core dump due to */
/* code that requires they at least have a value assigned  */
data->label = g_strdup("model");
data->id = -1;
data->locked = FALSE;
data->protein = FALSE;
data->mode = 0;
data->state = 0;
data->grafted = FALSE;
data->redraw = FALSE;
data->redraw_count = 0;
data->redraw_current = 0;
data->redraw_cumulative = 0;
data->redraw_time = 0;
data->fractional = FALSE;
data->periodic = 0;
data->num_frames = 1;
data->cur_frame = 0;
data->header_size = 0;
data->frame_size = 0;
data->file_size = 0;
data->expected_cores = 0;
data->expected_shells = 0;
data->trj_swap = FALSE;
data->build_molecules = TRUE;
data->build_hydrogen = FALSE;
data->build_polyhedra = FALSE;
data->build_zeolite = FALSE;
data->coord_units = CARTESIAN;
data->construct_pbc = FALSE;
data->done_pbonds = FALSE;
data->cbu = TRUE;
data->colour_scheme = ELEM;
data->sof_colourize = FALSE;
data->atom_info = -1;
data->shell_info = -1;
data->default_render_mode = BALL_STICK;
data->grayscale = FALSE;
data->rmax = RMAX_FUDGE;
data->zoom = 1.0;
data->show_names = FALSE;
data->show_title = TRUE;
data->show_frame_number = TRUE;
data->show_charge = FALSE;
data->show_atom_charges = FALSE;
data->show_atom_labels = FALSE;
data->show_atom_types = FALSE;
data->show_atom_index = FALSE;
data->show_geom_labels = TRUE;
data->show_cores = TRUE;
data->show_shells = FALSE;
data->show_bonds = TRUE;
data->show_hbonds = FALSE;
data->show_links = FALSE;
data->show_axes = TRUE;
data->show_cell = TRUE;
data->show_cell_images = TRUE;
data->show_cell_lengths = FALSE;
data->show_waypoints = TRUE;
data->show_selection_labels = FALSE;
data->show_nmr_shifts = FALSE;
data->show_nmr_csa = FALSE;
data->show_nmr_efg = FALSE;
data->num_atoms = 0;
data->num_asym = 0;
data->num_shells = 0;
data->num_ghosts = 0;
data->num_geom = 0;

data->cores = NULL;
data->shels = NULL;
data->bonds = NULL;
data->ubonds = NULL;
data->moles = NULL;
data->planes = NULL;
data->ghosts = NULL;
data->images = NULL;
data->ff_list = NULL;
data->layer_list = NULL;
data->measure_list = NULL;
data->transform_list = NULL;
data->waypoint_list = NULL;
data->frame_data_list = NULL;
data->zone_array = NULL;
data->zmatrix = zmat_new();
data->overlay = NULL;

/* NEW - rendering primitives */
for (n=0 ; n<RENDER_LAST ; n++)
  data->atom_list[n] = NULL;
for (n=0 ; n<RENDER_LAST ; n++)
  data->pipe_list[n] = NULL;

data->property_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, property_free);

data->ribbons = NULL;
data->spatial = NULL;
data->phonons = NULL;
data->ir_list = NULL;
data->raman_list = NULL;
data->num_phonons = 0;
data->current_phonon = -1;
data->pulse_count = 0;
data->pulse_direction = 1;
data->pulse_max = 10.0;
data->show_eigenvectors = FALSE;
data->phonon_movie = FALSE;
data->phonon_movie_name = g_strdup("phonon_movie");
data->phonon_movie_type = g_strdup("gif");

data->epot_autoscale = TRUE;
data->epot_min = 999999999.9;
data->epot_max = -999999999.9;
data->epot_div = 11;

data->total_charge = 0.0;

data->camera = NULL;
data->camera_default = NULL;

data->morph_label = FALSE;

/*VZ*/
data->num_images = 0;
data->selection = NULL;

/* NEW */
undo_init(data);

data->gulp.phonon = FALSE;
data->gulp.kpoints = NULL;

/* VdW surface calc */
data->ms_colour_scale = FALSE;

/* init flags */
data->axes_type = CARTESIAN;
data->box_on = FALSE;
data->asym_on = FALSE;

/* symmetry */
space_init(&data->sginfo);
data->symmetry.num_symops = 0;
data->symmetry.symops = g_malloc(sizeof(struct symop_pak));
data->symmetry.num_items = 1;
data->symmetry.items = g_malloc(2*sizeof(gchar *));
*(data->symmetry.items) = g_strdup("none");
*((data->symmetry.items)+1) = NULL;
data->symmetry.pg_entry = NULL;
data->symmetry.summary = NULL;
data->symmetry.pg_name = g_strdup("unknown");
VEC3SET(&data->pbc[0], 1.0, 1.0, 1.0);
VEC3SET(&data->pbc[3], 0.5*G_PI, 0.5*G_PI, 0.5*G_PI);

/* periodic images */
data->image_limit[0] = 0.0;
data->image_limit[1] = 1.0;
data->image_limit[2] = 0.0;
data->image_limit[3] = 1.0;
data->image_limit[4] = 0.0;
data->image_limit[5] = 1.0;

data->region_max = 0;
/* existence */
data->region_empty[REGION1A] = FALSE;
data->region_empty[REGION2A] = TRUE;
data->region_empty[REGION1B] = TRUE;
data->region_empty[REGION2B] = TRUE;
/* lighting */
data->region[REGION1A] = TRUE;
data->region[REGION2A] = FALSE;
data->region[REGION1B] = FALSE;
data->region[REGION2B] = FALSE;
/* morphology */
data->morph_type = DHKL;
data->num_vertices = 0;
data->num_planes = 0;
data->sculpt_shift_use = FALSE;

data->basename = g_strdup("model");
data->filename = g_strdup("model");

n++;

/* gulp template init */
/* NEW - COSMO init ... TODO - include all GULP inits */
gulp_init(&data->gulp);

/* GAMESS defaults */
gamess_init(&data->gamess);

/* GAUSSIAN defaults */
data->gauss_cube.voxels = NULL;

data->abinit.energy = data->abinit.max_grad = data->abinit.rms_grad = 0.0;

data->castep.energy = data->castep.max_grad = data->castep.rms_grad = 0.0;
data->castep.have_energy = data->castep.have_max_grad = data->castep.have_rms_grad = data->castep.min_ok = FALSE;

/* SIESTA defaults */
siesta_init(&data->siesta);

/* diffraction defaults */
VEC3SET(data->diffract.theta, 0.0, 90.0, 0.1);
data->diffract.wavelength = 1.54180;
data->diffract.asym = 0.18;
data->diffract.u = 0.0;
data->diffract.v = 0.0;
data->diffract.w = 0.0;

/* surface creation init */
data->surface.optimise = FALSE;
data->surface.converge_eatt = FALSE;
data->surface.converge_r1 = TRUE;
data->surface.converge_r2 = TRUE;
data->surface.include_polar = FALSE;
data->surface.ignore_bonding = FALSE;
data->surface.create_surface = FALSE;
data->surface.true_cell = FALSE;
data->surface.keep_atom_order = FALSE;
data->surface.ignore_symmetry = FALSE;
data->surface.bonds_full = 0;
data->surface.bonds_cut = 0;
data->surface.miller[0] = 1;
data->surface.miller[1] = 0;
data->surface.miller[2] = 0;
data->surface.shift = 0.0;
data->surface.dspacing = 0.0;
data->surface.region[0] = 1;
data->surface.region[1] = 1;
data->surface.dipole_tolerance=0.005;
VEC3SET(data->surface.depth_vec, 0.0, 0.0, 0.0);

matrix_identity(data->latmat);
matrix_identity(data->ilatmat);
matrix_identity(data->rlatmat);

VEC3SET(data->offset, 0.0, 0.0, 0.0);
VEC3SET(data->centroid, 0.0, 0.0, 0.0);

/* no non-default element data for the model (yet) */
data->elements = NULL;
data->unique_atom_list = NULL;
/* file pointer */
data->animation = FALSE;
data->animating = FALSE;
data->anim_fix = FALSE;
data->anim_noscale = FALSE;
data->anim_loop = FALSE;
data->afp = NULL;
data->anim_confine = PBC_CONFINE_ATOMS;
data->anim_speed = 5.0;
data->anim_step = 1.0;
data->frame_list = NULL;
data->title = NULL;
data->analysis = analysis_new();

animate_model_init(data);
}

/*********************/
/* duplicate a model */
/*********************/
/* NB: this is primarily meant for threads to make a copy that can be */
/* manipulated without fear of deletion - consequently, much of the display */
/* related data does not need to be transferred (mainly want the coordinates) */
gpointer model_dup(struct model_pak *source)
{
struct model_pak *model;

/* allocation */
model = g_malloc(sizeof(struct model_pak));
model_init(model);

/* if no source, just return a fresh model */
if (!source)
  return(model);

/* duplication */
model->cores = dup_core_list(source->cores);
model->shels = dup_shell_list(source->shels);

/* TODO - have a lattice pointer that can be memcpy'd in one hit */
model->periodic = source->periodic;
model->fractional = source->fractional;

memcpy(model->latmat, source->latmat, 9*sizeof(gdouble));
memcpy(model->ilatmat, source->ilatmat, 9*sizeof(gdouble));
memcpy(model->rlatmat, source->rlatmat, 9*sizeof(gdouble));
memcpy(model->pbc, source->pbc, 6*sizeof(gdouble));

g_free(model->sginfo.spacename);
model->sginfo.spacename = source->sginfo.spacename;
space_lookup(model);

return(model);
}

/************************************/
/* standard preparation for display */
/************************************/
#define DEBUG_PREP_MODEL 0
gint model_prep(struct model_pak *data)
{
g_return_val_if_fail(data != NULL, 1);

#if DEBUG_PREP_MODEL
printf("start prep: %p\n", data);
#endif

error_table_clear();

/* init the original values */
/* FIXME - what if there are asymmetric duplicates? */
data->num_asym = g_slist_length(data->cores);

/* create the all important lattice matrix before anything else */
matrix_lattice_init(data);

/* convert to angstroms if required */
coords_init_units(data);

/* convert input cartesian coords to fractional */
if (!data->fractional)
  coords_make_fractional(data);

/* large model exception test */
/*
if (data->num_asym > 1000)
  {
  core_render_mode_set(STICK, data->cores);
  }
*/

/* animation init */
/* NB: needs to be after all first time init (eg lattice matrix, cart/frac conversion etc)
   has been set up, but before connectivity etc calcs are done */
animate_frame_select(0, data);
//animate_debug(data);

/* NB: core-shell links are required before doing the space group stuff */
shell_make_links(data);

/* mark the largest region label */
data->region_max = region_max(data);

/* space group lookup/full cell generation */
if (data->periodic == 3)
  {
  if (!data->sginfo.spacenum)
    data->sginfo.spacenum=1;
  if (space_lookup(data))
    gui_text_show(ERROR, "Error in Space Group lookup.\n");

  data->axes_type = OTHER;
  }

/* init */
data->num_atoms = g_slist_length(data->cores);
data->num_shells = g_slist_length(data->shels);
data->mode = FREE;

#if DEBUG_PREP_MODEL
printf(" num atoms: %d\n", data->num_atoms);
printf("num shells: %d\n", data->num_shells);
#endif

/* initial center/coord calc */
coords_init(INIT_COORDS, data);

/* connectivity */
connect_bonds(data);
connect_molecules(data);

/* assign colours */
model_colour_scheme(data->colour_scheme, data);

/* final periodicity setup */
if (data->periodic == 2)
  {
/* order by z, unless input core order should be kept */
  if (!data->surface.keep_atom_order)
    sort_coords(data);
  }

/* final recenter/coord calc (after molecule unfragmenting/surface sorting) */
coords_init(CENT_COORDS, data);

/* pre-calc rendering primitives */
core_render_mode_set(data->default_render_mode, data->cores);
render_refresh(data);

/* set up the camera - should be done after all coord centering etc. */
camera_init(data);

#if DEBUG_PREP_MODEL
printf("end prep: %p\n", data);
#endif

error_table_print_all();

return(0);
}

/*************************************************/
/* store the current position of the file stream */
/*************************************************/
#define DEBUG_ADD_FRAME_OFFSET 0
gint add_frame_offset(FILE *fp, struct model_pak *data)
{
fpos_t *offset;

g_assert(fp != NULL);
g_assert(data != NULL);

offset = g_malloc(sizeof(fpos_t));
/* prepend & reverse? */
if (!fgetpos(fp, offset))
  data->frame_list = g_list_append(data->frame_list, offset); 
else
  {
  g_free(offset);
  return(1);
  }

#if DEBUG_ADD_FRAME_OFFSET
printf("stored: %p\n", offset);
#endif

return(0);
}

/*********************************************************/
/* convenience routine for duplicating a list of strings */
/*********************************************************/
GSList *slist_gchar_dup(GSList *src)
{
GSList *list, *dest=NULL;

for (list=src ; list ; list=g_slist_next(list))
  dest = g_slist_prepend(dest, g_strdup(list->data));

if (dest)
  dest = g_slist_reverse(dest);

return(dest);
}

/*************************************************************/
/* generic call for an slist containing one freeable pointer */
/*************************************************************/
void free_slist(GSList *list)
{
GSList *item;

//printf("free_slist(): %p\n", list);

if (!list)
  return;

for (item=list ; item ; item=g_slist_next(item))
  g_free(item->data);

g_slist_free(list);
list=NULL;
}

/***********************************************************/
/* generic call for a list containing one freeable pointer */
/***********************************************************/
void free_list(GList *list)
{
GList *item;

for (item=list ; item ; item=g_list_next(item))
  g_free(item->data);

g_list_free(list);
list=NULL;
}

/*************************************************/
/* TODO - put stuff like this in a gamess.c file */
/*************************************************/
void gamess_data_free(struct model_pak *model)
{
g_free(model->gamess.title);
g_free(model->gamess.temp_file);
g_free(model->gamess.out_file);
}

/***********************************/
/* free the entire model structure */
/***********************************/
#define DEBUG_FREE_MODEL 0
void model_free(struct model_pak *data)
{
if (!data)
  return;

#ifdef WITH_GUI
/* destroy any associated dialogs */
dialog_destroy_model(data);
#endif

#if DEBUG_FREE_MODEL
printf("freeing model: %p...\n", data);
printf("freeing string data...\n");
#endif

g_free(data->label);
g_free(data->basename);
g_free(data->filename);
g_free(data->symmetry.pg_name);
g_free(data->title);
g_free(data->phonon_movie_name);
g_free(data->phonon_movie_type);

space_free(&data->sginfo);

#if DEBUG_FREE_MODEL
printf("freeing coord data...\n");
#endif

/* free lists with associated data */
free_core_list(data);
free_slist(data->images);
free_list(data->frame_list);
free_slist(data->layer_list);
free_slist(data->phonons);
free_slist(data->ir_list);
free_slist(data->raman_list);
free_slist(data->waypoint_list);
free_slist(data->transform_list);
free_slist(data->ff_list);

/* camera */
g_free(data->camera_default);

/* destroy the property table and list */
g_hash_table_destroy(data->property_table);

/* external package configurations */
gamess_data_free(data);
gulp_files_free(data);
gulp_data_free(data);

#if DEBUG_FREE_MODEL
printf("freeing symmetry data...\n");
#endif
g_free(data->symmetry.symops);
g_strfreev(data->symmetry.items);

#if DEBUG_FREE_MODEL
printf("freeing measurements...\n");
#endif
meas_prune_model(data);
measure_free_all(data);

#if DEBUG_FREE_MODEL
printf("freeing vertices... (%d)\n", data->num_vertices);
#endif

plane_data_free(data->planes);
g_slist_free(data->planes);
data->planes = NULL;
data->num_planes = 0;

/* spatial objects */
spatial_destroy_all(data);

#if DEBUG_FREE_MODEL
printf("freeing element data...\n");
#endif
free_slist(data->elements);
data->elements = NULL;

/*
#if DEBUG_FREE_MODEL
printf("closing animation stream...\n");
#endif
if (data->afp)
  fclose(data->afp);
*/

analysis_free(data->analysis);
data->analysis = NULL;

animate_model_free(data);

zone_free(data->zone_array);

sysenv.mal = g_slist_remove(sysenv.mal, data);

g_free(data);
}

/************************/
/* delete a given model */
/************************/
void model_delete(struct model_pak *model)
{
GSList *list;
struct canvas_pak *canvas;

//printf("model_delete: %p\n", model);

/* checks */
if (!model)
  return;
if (model->locked)
  {
  gui_text_show(ERROR, "Model is locked.\n");
  return;
  }

if (!g_slist_find(sysenv.mal, model))
  {
  printf("WARNING: unregistered model.\n");
  }
else
  {
/* remove reference from the canvas list */
  for (list=sysenv.canvas_list ; list ; list=g_slist_next(list))
    {
    canvas = list->data;
    if (canvas->model == model)
      canvas->model = NULL;
    }
  sysenv.mal = g_slist_remove(sysenv.mal, model);
  }


/* free the model's pointers */
model_free(model);
/*
sysenv.mal = g_slist_remove(sysenv.mal, model);
g_free(model);
*/

/* update */
sysenv.refresh_dialog=TRUE;
canvas_shuffle();

redraw_canvas(ALL);
}

/**********************************/
/* update the model/canvas layout */
/**********************************/
/* TODO - rename - eg model_active_shuffle() */
void canvas_shuffle(void)
{
gint c, m;
GSList *clist, *mlist;
struct canvas_pak *canvas;
struct model_pak *model;

model = tree_model_get();

/*
printf("canvas_shuffle()\n");
printf("active model: %p\n", model);
*/

/* find active model in canvas list */
c = 0;
mlist = NULL;
for (clist=sysenv.canvas_list ; clist ; clist=g_slist_next(clist))
  {
  canvas = clist->data;
  if (model)
    {

    if (!canvas->model &&!mlist)
      {
      canvas->model = model;
      }

    if (canvas->model == model)
      {
      m = g_slist_index(sysenv.mal, canvas->model);
      if (m < c)
        {
/* not enough prior models to display active model at current canvas poisiton */
/* start at active model */
/* TODO - start at low enough model number so that active model is displayed */
        mlist = g_slist_find(sysenv.mal, model);
        }
      else
        {
/* we have enough prior models to display active model at current canvas poisiton */
        mlist = g_slist_nth(sysenv.mal, m-c);
        }
      }

    }
  c++;
  }

/* active model not in the canvas list */
if (model && !mlist)
  {
/* start at active model */
  mlist = g_slist_find(sysenv.mal, model);
  }

/* fill the canvas list */
if (mlist)
  {
  for (clist=sysenv.canvas_list ; clist ; clist=g_slist_next(clist))
    {
    canvas = clist->data;
    if (mlist)
      {
      canvas->model = mlist->data;
      mlist = g_slist_next(mlist);
      }
    else
      canvas->model = NULL;
    }
  }
}

/*******************************/
/* replacement for ASSIGN call */
/*******************************/
struct model_pak *model_new(void)
{
struct model_pak *model;

model = g_malloc(sizeof(struct model_pak));

sysenv.mal = g_slist_append(sysenv.mal, model);

model_init(model);

return(model);
}

/*******************************************/
/* get pointer to a requested model's data */
/*******************************************/
#define DEBUG_PTR 0
struct model_pak *model_ptr(gint model, gint mode)
{
struct model_pak *ptr=NULL;

/* how were we called? */
switch (mode)
  {
  case RECALL:
/* FIXME - there are an awful lot of calls to this; check for routines */
/* that pass the model number, when they could pass the model ptr instead */
#if DEBUG_PTR
printf("Request for model %d [%d,%d], ptr %p\n", model, 0, sysenv.num_models, ptr);
#endif
    ptr = (struct model_pak *) g_slist_nth_data(sysenv.mal, model);
    break;
  }
return(ptr);
}

/***********************************************/
/* model property list modification primitives */
/***********************************************/
struct property_pak
{
guint rank;
gchar *value;
};

/*************************/
/* destruction primitive */
/*************************/
void property_free(gpointer ptr)
{
struct property_pak *p = ptr;

g_assert(p != NULL);

g_free(p->value);
g_free(p);
}

/*********************/
/* ranking primitive */
/*********************/
gint property_ranking(gpointer ptr_p1, gpointer ptr_p2)
{
struct property_pak *p1 = ptr_p1, *p2 = ptr_p2;

if (p1->rank < p2->rank)
  return(-1);
if (p1->rank > p2->rank)
  return(1);
return(0);
}

/***************************************/
/* add a pre-ranked new model property */
/***************************************/
/* NB: property is not displayed if rank = 0 */
void property_add_ranked(guint rank,
                         const gchar *key,
                         const gchar *value,
                         struct model_pak *model)
{
struct property_pak *p;

g_assert(key != NULL);
g_assert(value != NULL);
g_assert(model != NULL);
g_assert(model->property_table != NULL);

p = g_malloc(sizeof(struct property_pak));
p->value = g_strdup(value);
p->rank = rank;

g_hash_table_insert(model->property_table, g_strdup(key), p);
}

/*******************************************************************/
/* primitive for adding two strings to generate the property value */
/*******************************************************************/
void property_add_with_units(gint rank,
                             gchar *key, gchar *value, gchar *units,
                             struct model_pak *model)
{
gchar *text;

text = g_strjoin(" ", value, units, NULL);
property_add_ranked(rank, key, text, model);
g_free(text);
}

/*****************************/
/* replace an existing value */
/*****************************/
void property_replace(const gchar *key, const gchar *value, struct model_pak *model)
{
struct property_pak *new, *old;

new = g_malloc(sizeof(struct property_pak));
new->value = g_strdup(value);

old = g_hash_table_lookup(model->property_table, key);
if (old)
  new->rank = old->rank;
else
  new->rank = 999;

g_hash_table_insert(model->property_table, g_strdup(key), new);
}

/****************************************/
/* property value extraction primitives */
/****************************************/
const gchar *property_lookup(const gchar *key, struct model_pak *model)
{
struct property_pak *p;

g_assert(model != NULL);

p = g_hash_table_lookup(model->property_table, key);
if (p)
  return(p->value);
return(NULL);
}

/************************************/
/* property value removal primitive */
/************************************/
gint property_remove(const gchar *key, struct model_pak *model)
{
return(g_hash_table_remove(model->property_table, key));
}

/****************************************/
/* refresh the model content properties */
/****************************************/
void model_content_refresh(struct model_pak *model)
{
gint n;
gchar *text;

g_assert(model != NULL);


text = g_strdup_printf("%d", g_slist_length(model->moles));
property_add_ranked(1, "Total molecules", text, model);
g_free(text);

n = g_slist_length(model->shels);
if (n)
  {
  text = g_strdup_printf("%d", n);
  property_add_ranked(1, "Total shells", text, model);
  g_free(text);
  }
else
  property_remove("Total shells", model);

text = g_strdup_printf("%d", g_slist_length(model->cores));
property_add_ranked(1, "Total atoms", text, model);
g_free(text);


n = g_slist_length(model->phonons);
if (n)
  {
  text = g_strdup_printf("%d", n);
  property_add_ranked(2, "Total phonons", text, model);
  g_free(text);
  }
else
  property_remove("Total phonons", model);


// NEW
text = g_strdup_printf("%.4f", model->total_charge);
property_add_ranked(2, "Total charge", text, model);
g_free(text);


/* NEW */
n = g_slist_length(model->undo_list);
if (n)
  {
  text = g_strdup_printf("%d", n);
  property_add_ranked(99, "Undo operations", text, model);
  g_free(text);
  }
else
  property_remove("Undo operations", model);
}

