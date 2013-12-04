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
#include <math.h>
#include <string.h>
#include <time.h>

#include "gdis.h"
#include "coords.h"
#include "model.h"
#include "edit.h"
#include "file.h"
#include "import.h"
#include "matrix.h"
#include "measure.h"
#include "parse.h"
#include "graph.h"
#include "gui_shorts.h"
#include "interface.h"
#include "analysis.h"
#include "numeric.h"
#include "count.h"
#include "animate.h"

extern struct elem_pak elements[];
extern struct sysenv_pak sysenv;

#define MAX_PIE_SHELL 0

/* determine number of vector repeats to satisfy a given distance requirement in a given direction */
/* NB: direction should be given in normalize form */
gint calc_rep_3d(gdouble *vec, gdouble *normal, gdouble max)
{
gdouble len, x[3];

/* project vector and compute length */
ARR3SET(x, vec);
ARR3MUL(x, normal);
len = VEC3MAG(x);

/* orthogonal */
if (len <  G_MINDOUBLE)
  return(0);
/* one vector is all we need */
if (len > max)
  return(1);

/* return multiple required (NB: round up for safety) */
return(0.999 + max/len);
}

/* separation + lattice => compute repetitions */

void test(gint *images, gdouble *latmat, gdouble max)
{
gint i, j, m;
gdouble n[3], v[3];

/* compute minimum lattice images needed to ensure the 3 cartesian directions */
/* will be filled out to a specified minimum distance */
VEC3SET(images, G_MAXINT, G_MAXINT, G_MAXINT);
for (i=0 ; i<3 ; i++)
  {
/* normal */
  VEC3SET(n, 0.0, 0.0, 0.0);
  n[i] = 1.0;

  for (j=0 ; j<3 ; j++)
    {
/* iterate through lattice vectors */
    VEC3SET(v, latmat[j], latmat[j+3], latmat[j+6]);

/* record minimum non-zero value for each lattice vector */
    m = calc_rep_3d(v, n, max);
    if (m > 0) 
      {
      if (m < images[j])
        images[j] = m; 
      }
    }
  }

/*
for (i=0 ; i<3 ; i++)
  {
  printf("[%d] ", min[i]);
  }
printf("\n");
*/

}


/* fill out bins (NB: +) for 2 atoms, given periodic boundary conditions */
/* xsep should be the (already calculated) min separation */
/* pie = periodic image enumeration */
void count_pie_3d(gdouble *xsep, gdouble *latmat, gpointer count)
{
gint i, j, k, images[3];
gdouble r, x1[3], offset[3];

/* compute images needed to satisify the count max distance */
test(images, latmat, count_stop(count));

/* FIXME - need to correct the volume */
//printf("using extents: [%d][%d][%d]\n", images[0], images[1], images[2]);

for (k=-images[2] ; k<=images[2] ; k++)
  {
  for (j=-images[1] ; j<=images[1] ; j++)
    {
    for (i=-images[0] ; i<=images[0] ; i++)
      {

/*
printf("computing distance for: [%d][%d][%d]\n", i, j, k);
*/

/* cartesian offset */
      ARR3SET(x1, xsep);
      VEC3SET(offset, i, j, k);
      ARR3ADD(x1, offset);
      vecmat(latmat, x1);
      r = VEC3MAG(x1);

/* avoid adding 0.0 - ie distance between atom and itself */
/* CURRENT - should no longer occur now that we loop over unique atom pairs */
//if (r > G_MINDOUBLE)
  {
      count_insert(r, count);
  }

      }
    }
  }

}

/****************************/
/* display analysis results */
/****************************/
void analysis_show(struct model_pak *model)
{
gpointer import;

g_assert(model != NULL);

/* display the new plot */
import = gui_tree_parent_object(model);
if (import)
  gui_tree_import_refresh(import);

redraw_canvas(SINGLE);
}

/**************************/
/* read in all frame data */
/**************************/
// TODO - this could probably be gotten rid of by implementing
// convenience functions that access animation frame data
#define DEBUG_LOAD_ANALYSIS 0
gint analysis_load(struct analysis_pak *analysis, struct model_pak *model, struct task_pak *task)
{
gint start, stop, step;
gint orig, i, j, k, m, n;
GSList *list;
struct core_pak *core;

/* checks */
g_assert(analysis != NULL);
g_assert(model != NULL);
g_assert(task != NULL);

//m = analysis->num_frames;
//n = analysis->num_atoms;
//m = g_list_length(model->animate_list);
start = analysis->frame_start;
stop = analysis->frame_stop;
step = analysis->frame_step;
m = ((stop - start) / step) + 1;
n = g_slist_length(model->cores);

/* no frame data - just stick to the model itself */
if (m < 1)
  m=1;

#if DEBUG_LOAD_ANALYSIS
printf("Allocating for %d frames and %d atoms.\n", m, n);
#endif

/*
analysis->latmat = g_malloc((m+1)*sizeof(struct gd9_pak));
analysis->time = g_malloc((m+1)*sizeof(gdouble));
analysis->ke = g_malloc((m+1)*sizeof(gdouble));
analysis->pe = g_malloc((m+1)*sizeof(gdouble));
analysis->temp = g_malloc((m+1)*sizeof(gdouble));
analysis->position = g_malloc((m+1)*n*sizeof(struct gd3_pak));
analysis->velocity = g_malloc((m+1)*n*sizeof(struct gd3_pak));
*/

/*
analysis->time = g_malloc(m*sizeof(gdouble));
analysis->ke = g_malloc(m*sizeof(gdouble));
analysis->pe = g_malloc(m*sizeof(gdouble));
analysis->temp = g_malloc(m*sizeof(gdouble));
*/

analysis->latmat = g_malloc(m*sizeof(struct gd9_pak));
analysis->position = g_malloc(m*n*sizeof(struct gd3_pak));
analysis->velocity = g_malloc(m*n*sizeof(struct gd3_pak));


/* flag the current frame */
orig = model->animate_current;

/* TODO - analysis pak not necessary any more? ie just use animate_list data directly */
/* NB: could be done without changing the cores in model_pak at all */
/* cycle frames and store in analysis structure */
k=0;
//for (i=0 ; i<m ; i++)
for (i=start ; i<=stop ; i+=step)
  {
  animate_frame_select(i, model);

  memcpy((analysis->latmat+k)->m, model->latmat, 9*sizeof(gdouble));

// deprec - should be all in property table
/*
  *(analysis->time+i) = model->frame_time;
  *(analysis->ke+i) = model->frame_ke;
  *(analysis->pe+i) = model->frame_pe;
  *(analysis->temp+i) = model->frame_temp;
*/

  j = g_slist_length(model->cores);

/* fill out coordinates */
  if (j == n)
    {
    j=0;
    for (list=model->cores ; list ; list=g_slist_next(list))
      {
      core = list->data;
      ARR3SET((analysis->position+k*n+j)->x, core->x);
      ARR3SET((analysis->velocity+k*n+j)->x, core->v);
      j++;
      }
    k++;
    }
  else
    printf("WARNING: inconsistent frame %d has %d/%d cores.\n", i, j, n);

/* progress */
  task->progress += 50.0/analysis->num_frames;
  }

/* revert to original frame */
animate_frame_select(orig, model);

analysis->num_frames = k;

// TODO - if there is no Time property - fall back to frame count
// bridging code from old to new
analysis->time_start = str_to_float(animate_frame_property_get(analysis->frame_start-1, "Time", model));
analysis->time_stop = str_to_float(animate_frame_property_get(analysis->frame_stop-1, "Time", model));
analysis->time_step = analysis->time_stop - analysis->time_start;
analysis->time_step /= (gdouble) (analysis->num_frames-1);

#if DEBUG_LOAD_ANALYSIS
printf("Time range: %f-%f (%f)\n", analysis->time_start, analysis->time_stop, analysis->time_step);
printf("Read in %d complete frames.\n", k);
#endif

return(0);
}

/***********************/
/* free all frame data */
/***********************/
void analysis_free(struct analysis_pak *analysis)
{
if (!analysis)
  return;

if (analysis->latmat)
  g_free(analysis->latmat);
if (analysis->time)
  g_free(analysis->time);
if (analysis->ke)
  g_free(analysis->ke);
if (analysis->pe)
  g_free(analysis->pe);
if (analysis->temp)
  g_free(analysis->temp);
if (analysis->position)
  g_free(analysis->position);
if (analysis->velocity)
  g_free(analysis->velocity);

g_free(analysis);
}

/***********************************/
/* allocate the analysis structure */
/***********************************/
gpointer analysis_new(void)
{
struct analysis_pak *analysis;

analysis = g_malloc(sizeof(struct analysis_pak));

analysis->latmat = NULL;
analysis->time = NULL;
analysis->ke = NULL;
analysis->pe = NULL;
analysis->temp = NULL;
analysis->position = NULL;
analysis->velocity = NULL;

return(analysis);
}

/***********************************/
/* duplicate an analysis structure */
/***********************************/
/* intended to preserve the dialog values at the time a job is requested */
gpointer analysis_dup(gpointer data)
{
struct analysis_pak *src=data, *dest;

g_assert(src != NULL);

dest = analysis_new();

dest->frame_start = src->frame_start;
dest->frame_stop = src->frame_stop;
dest->frame_step = src->frame_step;

dest->start = src->start;
dest->stop = src->stop;
dest->step = src->step;

dest->num_atoms = src->num_atoms;
dest->num_frames = src->num_frames;

dest->atom1 = g_strdup(src->atom1);
dest->atom2 = g_strdup(src->atom2);

dest->rdf_normalize = src->rdf_normalize;

return(dest);
}

/**********************/
/* setup the analysis */
/**********************/
#define DEBUG_INIT_ANALYSIS 0
gint analysis_init(struct model_pak *model)
{
struct analysis_pak *analysis;

/* checks */
g_assert(model != NULL);
g_assert(model->analysis != NULL);

analysis = model->analysis;

analysis->num_atoms = g_slist_length(model->cores);
analysis->num_frames = g_list_length(model->animate_list);

analysis->frame_start = 1;
analysis->frame_stop = 1+analysis->num_frames;
analysis->frame_step = 1;

analysis->time_start = 0.0;
analysis->time_stop = 0.0;
analysis->time_step = 0.0;

analysis->atom1 = NULL;
analysis->atom2 = NULL;

/* TODO - should really be renamed eg rdf_start/stop/step */
analysis->start = 0.0;
analysis->stop = (gint) model->rmax;
analysis->step = 0.1;

analysis->rdf_normalize = TRUE;

#if DEBUG_INIT_ANALYSIS
printf("Frames: %d, Atoms: %d\n", analysis->num_frames, analysis->num_atoms);
#endif

return(0);
}

/* NEW - plot properties */
/* NEW - uses new animation code, which is close to replacing analysis altogether */
//void analysis_plot(gint type, struct model_pak *model)
#define DEBUG_ANALYSIS_PLOT 0
void analysis_plot(const gchar *property, struct analysis_pak *analysis, struct model_pak *model)
{
gint i, n, frame, start, stop, step;
gdouble *data;
gpointer graph, import;

/* init */
start = analysis->frame_start;
stop = analysis->frame_stop;
step = analysis->frame_step;
n = ((stop - start) / step) + 1;

#if DEBUG_ANALYSIS_PLOT
printf("[%d][%d][%d] (%d)\n", start, stop, step, n);
#endif

/* checks */
if (n < 1)
  {
  gui_text_show(ERROR, "Bad frame range.\n");
  return;
  }

/* loop and extract property */
data = g_malloc(n * sizeof(gdouble));
frame = start;
for (i=0 ; i<n ; i++)
  {
  const gchar *value = animate_frame_property_get(frame, property, model);

  if (value)
    *(data+i) = str_to_float(value);
  else
    *(data+i) = 0.0;

  frame += step;
  }

/* create and attach graph */
graph = graph_new(property);
graph_add_data(n, data, start, stop, graph);
import = gui_tree_parent_object(model);
if (import)
  {
  import_object_add(IMPORT_GRAPH, graph, import);
  gui_tree_append(import, TREE_GRAPH, graph);
  }
}

/*******************/
/* RDF calculation */
/*******************/
#define DEBUG_CALC_RDF 0
void analysis_plot_rdf(struct analysis_pak *analysis, struct task_pak *task)
{
gint i, j, k, n;
gint start, stop, step;
gint frame, size;
gint *bins;
gdouble *cz, slice;
gdouble volume, c, r1, r2, xi[3], xj[3];
gdouble *latmat;
gpointer import, graph, count1;
gchar *label;
const gchar *a, *b;
struct core_pak *corei, *corej;
struct model_pak *model;

/* checks */
g_assert(analysis != NULL);
g_assert(task != NULL);
model = task->locked_model;
g_assert(model != NULL);

/* load frames? */
if (!analysis->position)
  {
/* time slice - if load_analysis() needed assume it consumes half ie 50.0 */
  slice = 50.0;
  if (analysis_load(analysis, model, task))
    return;
  }
else
  slice = 100.0;

/* bin object */
count1 = count_new(analysis->start, analysis->stop, analysis->step);

/* frame interval */
start = analysis->frame_start;
stop = analysis->frame_stop;
step = analysis->frame_step;
n = ((analysis->frame_stop - analysis->frame_start) / analysis->frame_step) + 1;

#if DEBUG_CALC_RDF
printf("Starting RDF calc (%d frames)\n", analysis->num_frames);
printf("%f - %f : %f\n", analysis->start, analysis->stop, analysis->step);
printf("%s - %s\n", analysis->atom1, analysis->atom2);
printf("[%d][%d][%d] (%d)\n", start, stop, step, n);
#endif

/* loop over desired frames */
volume = 0.0;

/* user perspective - frame count starts from 1 - convert to base 0 offset */
frame = start - 1;
for (k=0 ; k<analysis->num_frames ; k++)
  {
  latmat = (analysis->latmat+k)->m;
/* lattice *may* change between frames => volume changes must be incorporated */
  volume += calc_volume(latmat);

/* loop over unique atom pairs */
  for (i=0 ; i<analysis->num_atoms-1 ; i++)
    {
    corei = g_slist_nth_data(model->cores, i);

    ARR3SET(xi, (analysis->position+k*analysis->num_atoms+i)->x);

    for (j=i+1 ; j<analysis->num_atoms ; j++)
      {
      corej = g_slist_nth_data(model->cores, j);

      if (!pair_match(analysis->atom1, analysis->atom2, corei, corej))
        continue;

/* calculate closest distance */
      ARR3SET(xj, (analysis->position+k*analysis->num_atoms+j)->x);
      ARR3SUB(xj, xi);
      fractional_min(xj, model->periodic);

#define OLD 0
#if OLD
      vecmat(latmat, xj);
      r1 = VEC3MAG(xj); 
      count_insert(r1, count1);
#else
      count_pie_3d(xj, latmat, count1);
#endif

      }
    }

/* update progess */
  task->progress += slice/n;

//printf("k=%d, frame=%d\n", k, frame);

  frame += step;
  }

/* convert count integer array into gdouble array */
size = count_size(count1);
bins = count_bins(count1);
cz = g_malloc(size * sizeof(gdouble));
for (i=size ; i-- ; )
  {
  *(cz+i) = (gdouble) *(bins+i);
  }
/* free old int count array */
count_free(count1);

/* labels for graph */
if (analysis->atom1)
  a = analysis->atom1;
else
  a = "any";

if (analysis->atom2)
  b = analysis->atom2;
else
  b = "any";


/* normalize the RDF */
if (analysis->rdf_normalize)
  {
  volume /= analysis->num_frames;

  c = 0.6666666667*PI*analysis->num_atoms/volume;
  c *= analysis->num_atoms*analysis->num_frames;

  r1 = r2 = analysis->start;
  r2 += analysis->step;
  for (i=0 ; i<size ; i++)
    {
    *(cz+i) /= (c * (r2*r2*r2 - r1*r1*r1));
    r1 += analysis->step;
    r2 += analysis->step;
    }

  label = g_strdup_printf("RDF_%s_%s", a, b);
  }
else
  label = g_strdup_printf("Pair_%s_%s", a, b);


/* NB: add graph to the model's data structure, but don't update */
/* the model tree as we're still in a thread */
graph = graph_new(label);
g_free(label);

#if DEBUG_CALC_RDF
printf("Creating graph: size = %d, x: %f-%f\n", size, analysis->start, analysis->stop);
#endif

graph_add_data(size, cz, analysis->start, analysis->stop, graph);
graph_set_yticks(TRUE, 2, graph);
g_free(cz);


/* CURRENT */
graph_y_label_set("g(r)", graph);


/* get model's parent import */
import = gui_tree_parent_object(model);
if (import)
  {
/* append the graph to the import */
  import_object_add(IMPORT_GRAPH, graph, import);
  }

analysis_free(analysis);
}

/**************************************/
/* plot the evolution of measurements */
/**************************************/
/* CURRENT - only do the 1st, until we figure out if we should */
/* distinguish bonds angles etc ans selectable issues etc */
/* TODO - correct for input frame range (analysis->frame_start etc) property and RDF done (see above) */
void analysis_plot_meas(struct analysis_pak *analysis, struct task_pak *task)
{
gint i, j, n, type, index[4];
gdouble value, *data, x[4][3];
gpointer import, m, graph;
GSList *list, *clist;
struct model_pak *model;

/* checks */
g_assert(analysis != NULL);
g_assert(task != NULL);
model = task->locked_model;
g_assert(model != NULL);

/* load frames? */
if (!analysis->position)
  if (analysis_load(analysis, model, task))
    return;

/* CURRENT - only dealing with 1st measurement */
m = g_slist_nth_data(model->measure_list, 0);
if (!m)
  {
  gui_text_show(ERROR, "No measurements to plot.\n");
  return;
  }

type = measure_type_get(m);
clist = measure_cores_get(m);

/*
printf("measurement cores: %d\n", g_slist_length(clist));
*/

/* record the position of each core */
n=0;
for (list=clist ; list ; list=g_slist_next(list))
  {
  if (n<4)
    {
    index[n] = g_slist_index(model->cores, list->data);
/*
printf(" - %p (%d)\n", list->data, index[n]);
*/
    n++;
    }
  else
    break;
  }

/* allocation for graph plot */
data = g_malloc(analysis->num_frames*sizeof(gdouble));

/* loop over each frame */
for (i=0 ; i<analysis->num_frames ; i++)
  {

/*
printf("frame: %d ", i);
*/

/* get cartesian coords for constituent atoms */
  for (j=0 ; j<n ; j++)
    {
    ARR3SET(&x[j][0], (analysis->position+i*analysis->num_atoms+index[j])->x);
    vecmat(model->latmat, x[j]);
    }

/* CURRENT - make measurement -> data */
  switch (type)
    {
    case MEASURE_TORSION:
/* FIXME - check this */
      value = measure_dihedral(x[0], x[1], x[2], x[3]);
      break;

    case MEASURE_ANGLE:
      value = measure_angle(x[0], x[1], x[2]);
      break;

    default:
      value = measure_distance(x[0], x[1]);
    }

/*
printf(" (value = %f)\n", value);
*/

  *(data+i) = value;
  }

/* graph */
graph = graph_new("Measurement");
graph_add_data(analysis->num_frames, data, 1, analysis->num_frames, graph);

/* get model's parent import */
import = gui_tree_parent_object(model);
if (import)
  {
/* append the graph to the import */
  import_object_add(IMPORT_GRAPH, graph, import);
  }

/* done */
g_free(data);
g_slist_free(clist);

analysis_free(analysis);
}

/*********************************************/
/* plot the FFT of the VACF (power spectrum) */
/*********************************************/
/* TODO - correct for input frame range (analysis->frame_start etc) property and RDF done (see above) */
void analysis_plot_power(gint m, gdouble *vacf, gdouble step, struct model_pak *model)
{
gint i, e, n;
gdouble r_power, s_power, *x, *y;
gpointer import;
struct graph_pak *graph;

/* checks */
g_assert(vacf != NULL);
g_assert(model != NULL);

/* get the best n = 2^e >= m */
e = log((gdouble) m) / log(2.0);
n = pow(2.0, ++e);

/*
printf("input size: %d, fft size: %d\n", m, n);
*/

g_assert(n >= m);
x = g_malloc(2*n*sizeof(gdouble));

/* assign data */
r_power = 0.0;
for (i=0 ; i<m ; i++)
  {
  x[2*i] = vacf[i];
  x[2*i+1] = 0.0; 
  r_power += vacf[i] * vacf[i];
  }

/* pad the rest */
for (i=2*m ; i<2*n ; i++)
  x[i] = 0.0; 

/* compute fourier transform */
fft(x, n, 1);

/* compute power spectrum */
y = g_malloc(n*sizeof(gdouble));
s_power = 0.0;
for (i=0 ; i<n ; i++)
  {
/* see numerical recipies in C p401 */
  y[i] = x[2*i]*x[2*i] + x[2*i+1]*x[2*i+1];
  s_power += y[i];
  }

/* see num recipics - p407 (12.1.10) */
/*
printf("total power (real)    : %f\n", r_power);
printf("total power (spectral): %f\n", s_power / (gdouble) m);
*/

/* plot */
graph = graph_new("POWER");
/* only plot +ve frequencies (symmetrical anyway, since input is real) */
graph_add_data(n/2, y, 0.0, 0.5/step, graph);
graph_set_yticks(FALSE, 2, graph);

/* get model's parent import */
import = gui_tree_parent_object(model);
if (import)
  {
/* append the graph to the import */
  import_object_add(IMPORT_GRAPH, graph, import);
  }

g_free(x);
g_free(y);
}

/********************/
/* VACF calculation */
/********************/
/* TODO - correct for input frame range (analysis->frame_start etc) property and RDF done (see above) */
#define DEBUG_CALC_VACF 0
void analysis_plot_vacf(struct analysis_pak *analysis, struct task_pak *task)
{
gint i, n;
gdouble slice, step=1.0;
gdouble vi_dot_v0, v0_dot_v0;
gdouble *vacf, v0[3], vi[3];
gpointer import, graph;
struct model_pak *model;

/* checks */
g_assert(analysis != NULL);
g_assert(task != NULL);
model = task->locked_model;
g_assert(model != NULL);

if (analysis->num_frames < 2)
  return;

/* vacf setup */
vacf = g_malloc(analysis->num_frames * sizeof(gdouble));

/* load frames? */
if (!analysis->position)
  {
/* time slice - if load_analysis() needed assume it consumes half ie 50.0 */
  slice = 50.0;
  if (analysis_load(analysis, model, task))
    return;
  }
else
  slice = 100.0;

/* reference frame is 0 ... sum from 1 to num_frames-1 */
for (n=1 ; n<analysis->num_frames ; n++)
  {
//printf(" --- frame %d\n", n);

  vacf[n-1] = 0.0;
  for (i=0 ; i<analysis->num_atoms ; i++)
    {
    ARR3SET(v0, (analysis->velocity+i)->x);
    ARR3SET(vi, (analysis->velocity+n*analysis->num_atoms+i)->x);

vi_dot_v0 = dotprod(vi, v0);
v0_dot_v0 = dotprod(v0, v0);
vacf[n-1] += vi_dot_v0 / v0_dot_v0;

//    ARR3MUL(vi, v0);
//    vacf[n] += vi[0]+vi[1]+vi[2];

/*
printf("atom %d, ", i);
P3VEC("vi : ", vi);
*/
    }
  vacf[n-1] /= (gdouble) analysis->num_atoms;
/* update progess */
  task->progress += slice/analysis->num_frames;
  }

step = analysis->time_stop - analysis->time_start;
step /= (gdouble) (analysis->num_frames-1);

#if DEBUG_CALC_VACF
printf("t : [%f - %f](%d x %f)\n", analysis->time_start, analysis->time_stop,
                                   analysis->num_frames, step);
#endif

/* NB: add graph to the model's data structure, but don't update */
/* the model tree as we're still in a thread */
graph = graph_new("VACF");
/*
graph_add_data(analysis->num_frames, vacf,
                  analysis->time_start, analysis->time_stop, graph);
*/
/* FIXME - get time at frame values */
// TODO - if there was no time property in model - default to frame count
//graph_add_data(analysis->num_frames, vacf, analysis->frame_start, analysis->frame_stop, graph);
// NB: vacf should omit 1st frame as this is the reference 
graph_add_data(analysis->num_frames, vacf, analysis->time_start+analysis->time_step, analysis->time_stop, graph);

graph_set_yticks(FALSE, 2, graph);

// CURRENT - I've forgotten some of what the power spectrum was about, and with all the changes
// to analysis/animation code it's probably broken, so removing (for now)
//analysis_plot_power(analysis->num_frames, vacf, analysis->time_step, model);

/* get model's parent import */
import = gui_tree_parent_object(model);
if (import)
  {
/* append the graph to the import */
  import_object_add(IMPORT_GRAPH, graph, import);
  }

g_free(vacf);

analysis_free(analysis);
}

