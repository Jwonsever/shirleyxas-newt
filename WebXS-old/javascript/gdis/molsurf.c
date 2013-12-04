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
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "gdis.h"
#include "file.h"
#include "coords.h"
#include "matrix.h"
#include "molsurf.h"
#include "molsurf_data.h"
#include "hirshfeld.h"
#include "numeric.h"
#include "parse.h"
#include "project.h"
#include "spatial.h"
#include "surface.h"
#include "task.h"
#include "interface.h"
#include "opengl.h"
#include "colourlib.h"
#include "vector.h"
#include "scalar.h"
#include "import.h"
#include "siesta.h"

/* main pak structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/* molsurf globals */
gdouble ms_dsize;
gint ms_calc_grad=TRUE;
struct model_pak *ms_data;
extern gint ms_epot_autoscale;
extern GtkWidget *surf_epot_min, *surf_epot_max, *surf_epot_div;
/* TODO - lock this if in thread */
GSList *ms_points=NULL;
GSList *hsEnvironment=NULL;

/****************************/
/* interpolate a grid value */
/****************************/
gdouble ms_seista_interpol(gdouble *r, gdouble *v, struct model_pak *model)
{
gdouble x[3];

ARR3SET(x, r);

/* NB: this won't work for a non-orthogonal cell */
if (x[0] < 0.0)
  x[0] += model->siesta.cell[0];
if (x[1] < 0.0)
  x[1] += model->siesta.cell[4];
if (x[2] < 0.0)
  x[2] += model->siesta.cell[8];

/* compute grid coordinate from cartesian input position */
x[0] /= model->siesta.cell[0];
x[1] /= model->siesta.cell[4];
x[2] /= model->siesta.cell[8];
x[0] *= model->siesta.grid[0];
x[1] *= model->siesta.grid[1];
x[2] *= model->siesta.grid[2];

/* TODO - interpolate values */
/* round up to nearest integer */
/*
nx = x[0]+0.5;
ny = x[1]+0.5;
nz = x[2]+0.5;
nx = CLAMP(nx, 0, model->siesta.grid[0]-1);
ny = CLAMP(ny, 0, model->siesta.grid[1]-1);
nz = CLAMP(nz, 0, model->siesta.grid[2]-1);
*/

return(0.0);
}

/*******************************/
/* siesta grid value retrieval */
/*******************************/
gdouble ms_siesta_grid(gdouble *r, gint method, struct model_pak *model)
{
gint n, nx, ny, nz, dummy[3];
gdouble x[3], value=0.0;

ARR3SET(x, r);

/* compensate if coords in au */
VEC3MUL(x, 1.0/model->siesta.eden_scale);

/* convert from cartesian to fractional (grid space) */
vecmat(model->siesta.icell, x);
fractional_clamp(x, dummy, 3);

/* compute grid location */
x[0] *= model->siesta.grid[0];
x[1] *= model->siesta.grid[1];
x[2] *= model->siesta.grid[2];
nx = nearest_int(x[0]);
ny = nearest_int(x[1]);
nz = nearest_int(x[2]);
nx = CLAMP(nx, 0, model->siesta.grid[0]-1);
ny = CLAMP(ny, 0, model->siesta.grid[1]-1);
nz = CLAMP(nz, 0, model->siesta.grid[2]-1);

n = nx + ny*model->siesta.grid[0] + nz*model->siesta.grid[0]*model->siesta.grid[1];

g_assert(n >= 0);
g_assert(n < model->siesta.grid[0]*model->siesta.grid[1]*model->siesta.grid[2]);

switch (method)
  {
  case MS_SIESTA_RHO:
   value = *(model->siesta.grid_rho+n);
   break;
  case MS_SIESTA_DRHO:
   value = *(model->siesta.grid_drho+n);
   break;
  case MS_SIESTA_LDOS:
   value = *(model->siesta.grid_ldos+n);
   break;
  case MS_SIESTA_VH:
   value = *(model->siesta.grid_vh+n);
   break;
  case MS_SIESTA_VT:
   value = *(model->siesta.grid_vt+n);
   break;
  }

return(value);
}

/***************************************/
/* electron density function retrieval */
/***************************************/
//deprec
gdouble ms_siesta_eden(gdouble *r, struct model_pak *model)
{
return(0.0);
}

/************************************************/
/* retrieval of gaussian cube grid data         */
/************************************************/
#define DEBUG_MS_GAUSS_CUBE_VALUE 0
gdouble ms_gauss_cube_value(gint i, gint j, gint k, struct model_pak *model)
{
gdouble value; 
struct gauss_cube_pak g = model->gauss_cube;
gint v = i * g.nb * g.nc + j * g.nc + k;

value = model->gauss_cube.voxels[v];

#if DEBUG_MS_GAUSS_CUBE_VALUE
if ( (i<0) || (i>g.na) ) { value = 0.0; printf("i out of bounds"); }
if ( (j<0) || (j>g.nb) ) { value = 0.0; printf("j out of bounds"); }
if ( (k<0) || (k>g.nc) ) { value = 0.0; printf("k out of bounds"); }
#endif

return value;
}

/************************************************/
/* gaussian based distance to molecular surface */
/************************************************/
/* r is position, a is probe radius, list is core list */
gdouble ms_dist(gdouble *r, GSList *core_list)
{
gdouble a, sum, x[3], f[3];
GSList *list;
struct core_pak *core;

/* convert cartesian point to fractional */
ARR3SET(f, r);
vecmat(ms_data->ilatmat, f);
sum = 0.0;
for (list=core_list ; list ; list=g_slist_next(list))
  {
  core = list->data;

/* get minimum distance from core's VDW surface to point */
  ARR3SET(x, core->x);
  ARR3SUB(x, f);
  fractional_min(x, ms_data->periodic);
  vecmat(ms_data->latmat, x);
  a = elements[core->atom_code].vdw;

/* sum gaussian contribution */
  sum += exp(-(VEC3MAG(x) - a)/ms_dsize);
  }
return(-ms_dsize*log(sum));
}

/*********************************************/
/* gaussian based molecular surface gradient */
/*********************************************/
struct core_pak *ms_grad(struct smv_pak *point, GSList *core_list)
{
gint i, touch;
gdouble a, e, f, dx, sum1[3], sum2[3], x[3], r[3];
GSList *list, *cores;
struct core_pak *core, *touch_core;

g_assert(point != NULL);

/* fractional coords for point */
ARR3SET(r, point->rx);
vecmat(ms_data->ilatmat, r);

/* init for loop */
touch = 0;
touch_core = NULL;
VEC3SET(sum1, 0.0, 0.0, 0.0);
VEC3SET(sum2, 0.0, 0.0, 0.0);
f = exp(0.6666667 * ms_dsize);

cores = core_list;

for (list=cores ; list ; list=g_slist_next(list))
  {
  core = list->data;

/* get minimium separation vector */
  ARR3SET(x, r);
  ARR3SUB(x, core->x);
  fractional_min(x, ms_data->periodic);
  vecmat(ms_data->latmat, x);
  dx = VEC3MAG(x);

/* sum the vector components of the gradient */
  a = elements[core->atom_code].vdw;
  e = exp(-(dx - a)/ms_dsize);
  VEC3ADD(sum1, e, e, e);
  e /= (dx * ms_dsize);
  VEC3MUL(x, -e);
  ARR3ADD(sum2, x);

/* touch computation */
  if (dx < f*a)
    {
    touch_core = core;
    touch++;
    }
  }

/* compute overall normal */
for (i=3 ; i-- ; )
  point->nx[i] = sum2[i]/sum1[i];

normalize(point->nx, 3);

/* only return a core if it was the only one touching the supplied point */
if (touch == 1)
  return(touch_core);
return(NULL);
}

/***************************************/
/* create a triangular surface segment */
/***************************************/
struct smt_pak *new_triangle(struct smv_pak *p1,
                             struct smv_pak *p2,
                             struct smv_pak *p3)
{
struct smt_pak *triangle;

triangle = g_malloc(sizeof(struct smt_pak));

/* point references */
triangle->point[0] = p1;
triangle->point[1] = p2;
triangle->point[2] = p3;

/* adjacencies */
p1->adj = g_slist_prepend(p1->adj, p2);
p1->adj = g_slist_prepend(p1->adj, p3);
p2->adj = g_slist_prepend(p2->adj, p1);
p2->adj = g_slist_prepend(p2->adj, p3);
p3->adj = g_slist_prepend(p3->adj, p1);
p3->adj = g_slist_prepend(p3->adj, p2);

return(triangle);
}

/*******************************/
/* allocate for a new midpoint */
/*******************************/
struct smv_pak *new_point(gdouble *x)
{
struct smv_pak *p;

p = g_malloc(sizeof(struct smv_pak));
ms_points = g_slist_prepend(ms_points, p);

p->index = -1;

ARR3SET(p->rx, x);
p->adj = NULL;

return(p);
}

/*************************************************/
/* compute the real area of a triangular spatial */
/*************************************************/
gdouble triangle_area(struct smt_pak *triangle)
{
gdouble n[3];

calc_norm(n, triangle->point[0]->rx,
             triangle->point[1]->rx,
             triangle->point[2]->rx); 
return(0.5*VEC3MAG(n));
}

/****************************************/
/* calculate new molecular surface area */
/****************************************/
gdouble molsurf_area(GSList *tlist)
{
gdouble area=0.0;
GSList *item;
struct smt_pak *triangle;

for (item=tlist ; item ; item=g_slist_next(item))
  {
  triangle = item->data;

  area += triangle_area(triangle);
  }
return(area);
}

/********************************************/
/* estimate volume using divergence theorem */
/********************************************/
// NB: won't work if applied to periodic cases with discontinuities @ boundaries
gdouble molsurf_volume(GSList *tlist)
{
GSList *item;
gdouble volume=0.0, fn, ds, f[3], n[3];
struct smt_pak *triangle;

for (item=tlist ; item ; item=g_slist_next(item))
  {
  triangle = item->data;

/* triangle midpoint */
  ARR3SET(f, triangle->point[0]->rx);
  ARR3ADD(f, triangle->point[1]->rx);
  ARR3ADD(f, triangle->point[2]->rx);
  VEC3MUL(f, 0.333333333);

/* triangle normal */
/* FIXME - need to ensure these are pointing out */
  calc_norm(n, triangle->point[0]->rx,
               triangle->point[1]->rx,
               triangle->point[2]->rx); 
  normalize(n, 3);

/* f dot n */
  ARR3MUL(f, n);
  fn = fabs(f[0] + f[1] + f[2]);
// FIXME - fabs is wrong, but is a hack workaround since triangle normal is not guaranteed to point out
//  fn = f[0] + f[1] + f[2];

/* triangle area */
  ds = triangle_area(triangle);

/* volume contribution over surface triangulation */
  volume += fn * ds;
  }

return(0.333333333*volume);
}

/**************************/
/* common colour routines */
/**************************/
void ms_afm_colour(gdouble *colour, gdouble value, struct model_pak *data)
{
/* TODO - need AFM min/max stored in data */
}

void ms_hfs_colour(gdouble *colour, gdouble value)
{
VEC3SET(colour, 1.0, 0.0, 0.0);
VEC3MUL(colour, value);
}

void ms_epot_colour(gdouble *colour, gdouble value, gdouble min, gdouble max)
{
gdouble r, g, b;

r = g = b = 1.0;

if (value < 0.0)
  {
  b *= 1.0 - value/min;
  g *= 1.0 - value/min;
  }
else
  {
  r *= 1.0 - value/max;
  g *= 1.0 - value/max;
  }

VEC3SET(colour, r, g, b);
}

/****************************/
/* min/max point calculator */
/****************************/
void ms_get_zlimits(gdouble *min, gdouble *max, GSList *list)
{
GSList *item;
struct smv_pak *point;

g_assert(list != NULL);

/* init */
point = list->data;
*min = *max = point->rx[2];

/* loop */
for (item=list ; item ; item=g_slist_next(item))
  {
  point = item->data;

  if (point->rx[2] < *min)
    *min = point->rx[2];
  if (point->rx[2] > *max)
    *max = point->rx[2];
  }
}

/*****************************************/
/* uses GULP's new epot list calc method */
/*****************************************/
gint ms_get_epot(GSList *list, struct model_pak *model)
{
#if MS_GET_EPOT
gint run;
gchar *inp, *out, *full_inp, *full_out;
GSList *item;
struct vec_pak *vec;
struct smv_pak *point;

/* checks */
g_assert(model != NULL);

/* NEW - if we have an existing set of the same size - don't recalculate */
/* TODO - make this an option */
if (g_slist_length(list) == g_slist_length(model->gulp.epot_vecs))
  {
  return(0);
  }

/* setup the site coordinate list */
if (model->gulp.epot_vecs)
  free_slist(model->gulp.epot_vecs);
model->gulp.epot_vecs = NULL;

/* fill out the list */
for (item=list ; item ; item=g_slist_next(item))
  {
  point = item->data;
  if (!point->adj)
    continue;

  vec = g_malloc(sizeof(struct vec_pak));

  ARR3SET(vec->rx, point->rx);

  model->gulp.epot_vecs = g_slist_prepend(model->gulp.epot_vecs, vec);
  }
model->gulp.epot_vecs = g_slist_reverse(model->gulp.epot_vecs);

/* setup filenames (NB: GULP will end up being run in sysenv.cwd) */
inp = gun("gin");
out = gun("got");
full_inp =  g_build_filename(sysenv.cwd, inp, NULL);
full_out =  g_build_filename(sysenv.cwd, out, NULL);

/* create a single point GULP input file */
run = model->gulp.run;
model->gulp.run = E_SINGLE;
write_gulp(full_inp, model);
model->gulp.run = run;

/* run the GULP calc & get the results */
/* NB: don't use inp/out here as gulp is run in sysenv.cwd */
/* and it casues a channel error */
/*
if (exec_gulp(inp, out))
  return(1);
*/
if (task_fortran_exe(sysenv.gulp_path, inp, out))
  return(1);

if (read_gulp_output(full_out, model))
  return(1);

g_free(full_inp);
g_free(full_out);
g_free(inp);
g_free(out);
#endif
return(0);
}

/******************************************************/
/* compute the electrostatic potential min/max values */
/******************************************************/
void ms_siesta_epot_range(GSList *points, struct model_pak *model)
{
gdouble q;
GSList *list;
struct smv_pak *p;

g_assert(model != NULL);
g_assert(model->siesta.epot != NULL);

/* init min/max */
p = points->data;
q = 0.0;
// FIXME
//q = ms_siesta_epot(p->rx, model); 
model->siesta.epot_min = q;
model->siesta.epot_max = q;

/* point colouring */
for (list=points ; list ; list=g_slist_next(list))
  {
  p = list->data;
  if (!p->adj)
    continue;

q=0.0;
//  q = ms_seista_epot(p->rx, model); 
  
  if (q < model->siesta.epot_min)
    model->siesta.epot_min = q;
  if (q > model->siesta.epot_max)
    model->siesta.epot_max = q;
  }

/*
printf("min: %f\n", model->siesta.epot_min);
printf("max: %f\n", model->siesta.epot_max);
*/
}

/******************************/
/* colour a list of triangles */
/******************************/
void ms_colour_by_touch(GSList *points)
{
GSList *list;
struct smv_pak *p;
struct core_pak *core;

for (list=points ; list ; list=g_slist_next(list))
  {
  p = list->data;
  if (!p->adj)
    continue;

  core = ms_grad(p, ms_data->cores);

  if (core)
    {
    ARR3SET(p->colour, core->colour);
    VEC3MUL(p->colour, 1.0/65535.0);
    }
  else
    {
    ARR3SET(p->colour, sysenv.render.rsurf_colour);
    }
  }
}

/*****************************************************/
/* colour triangles by cartesian z value (AFM style) */
/*****************************************************/
void ms_colour_by_height(GSList *points)
{
gdouble z, zmin, zmax;
GSList *list;
struct smv_pak *p;

/* determine the range */
zmin = G_MAXDOUBLE;
zmax = -G_MAXDOUBLE;
for (list=points ; list ; list=g_slist_next(list))
  {
  p = list->data;
  if (!p->adj)
    continue;

  if (p->rx[2] < zmin)
    zmin = p->rx[2];

  if (p->rx[2] > zmax)
    zmax = p->rx[2];
  }

/* colour the points */
for (list=points ; list ; list=g_slist_next(list))
  {
  p = list->data;
  if (!p->adj)
    continue;
  ms_grad(p, ms_data->cores);
  z = p->rx[2] - zmin;
  z *= 1.0/(zmax - zmin);
  VEC3SET(p->colour, 0.7*z+0.2, 0.8*z, 0.5*z*z*z*z);
  }
}

/*************************************/
/* colour by electrostatic potential */
/*************************************/
void ms_colour_by_potential(GSList *points)
{
gdouble *q;
GSList *list1, *list2;
struct smv_pak *p;
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);

/* get potential for point list */
ms_get_epot(points, model);
model->ms_colour_scale = TRUE;
if (model->epot_autoscale)
  {
  model->epot_min = model->gulp.epot_min;
  model->epot_max = model->gulp.epot_max;
  model->epot_div = 11;
  }

/* point colouring */
list2 = model->gulp.epot_vals;
for (list1=points ; list1 ; list1=g_slist_next(list1))
  {
  p = list1->data;
  if (!p->adj)
    continue;

  ms_grad(p, ms_data->cores);
  if (list2)
    {
    q = (gdouble *) list2->data;
    ms_epot_colour(p->colour, *q, model->epot_min, model->epot_max);
    list2 = g_slist_next(list2);
    }
   else
    {
/* default colour (ie electrostatic calc has failed) */
    VEC3SET(p->colour, 0.5, 0.5, 0.5);
    }
  }
}

/*****************************************************/
/* colour by the weight function (Hirshfeld surface) */
/*****************************************************/
void ms_colour_by_weight(GSList *points)
{
GSList *list;
struct smv_pak *p;

for (list=points ; list ; list=g_slist_next(list))
  {
  p = list->data;

  VEC3SET(p->colour, 0.9, .9, .9);
/*
  ms_hfs_colour(p->colour, p->value);
*/
  }
}

/********************************************************************************/
/* colour by shape index 			     										*/
/*																				*/
/* This routine calculates the numerical second derivative of the surface		*/
/* function in order to calculate principal curvatures and thus 				*/
/* Shape Index and Curvedness													*/
/* Coding credits: JJM, from code originally written by Anthony S. Mitchell		*/
/*																				*/
/* References:																	*/
/* do Carmo, M.P. (1976) "Differential Geometry of Curves and Surfaces" ,		*/
/*		  Prentice-Hall, inc.													*/
/* Koenderink, J.J. (1990). Solid Shape. Cambridge MA, MIT Press. 				*/
/* Koenderink, J.J. and Van Doorn, A.J. (1992). 								*/
/*		  "Surface shape and curvature scales." 								*/
/*		  Image and Vision Computing 10: 557-565.								*/
/*																				*/
/********************************************************************************/

void ms_colour_by_shape(GSList *points, struct model_pak *model, gint propertyType, gint method)
{
GSList *pointsList;
struct smv_pak *p;
gdouble delta = 5.0e-1; 	/* distance offset for calculating numerical seconds */
gdouble oneOver2Delta, d2, d4;
gdouble h1, h2; 		/* principal curvatures */
/* Value of the surface function around the point */
gdouble F=0.0, Fx=0.0, Fy=0.0, Fz=0.0, F_x=0.0, F_y=0.0, F_z=0.0,
      	Fxy=0.0, Fx_y=0.0, F_xy=0.0, F_x_y=0.0,
       	Fxz=0.0, Fx_z=0.0, F_xz=0.0, F_x_z=0.0,
       	Fyz=0.0, Fy_z=0.0, F_yz=0.0, F_y_z=0.0;
gdouble x,y,z, xdx, x_dx, ydy, y_dy, zdz, z_dz; /* coordinates around the point */
gdouble r[3];                                   /* point to pass to functions that need a [3] */
gdouble h[9]; 				/* hessian */
gdouble eig[3];				/* eigenvalues of the hessian */
gdouble r0[3], r1[3], r2[3]; 
gdouble mat[9];
gdouble Dx, Dy, Dz; 			/* gradient */
gdouble temp[3]; 			/* the dreaded temporary variable */
gdouble theta, sinTheta, cosTheta;
gfloat hue, red, green, blue;

#if DEBUG_SHAPE 
	switch (propertyType){
		case MS_CURVEDNESS:
			fprintf(stderr," [Curvedness]");
			break;
		case MS_SHAPE_INDEX:
			fprintf(stderr," [Shape Index]");
			break;
	}
	jobStartTime = time(NULL);
#endif
	
	/* setup for colours */
    gfloat colourRange= 256;
    gfloat hueStep;
    
    hueStep = 240/colourRange;
	
	
    oneOver2Delta = 0.5/delta;
    d2 = 1.0/(delta*delta);
    d4 = d2/4;
    
    
    for (pointsList=points ; pointsList ; pointsList=g_slist_next(pointsList))
    {
        p = pointsList->data;
        if (!p->adj)
            continue;
        /*	Don't be confused: x is a single double. Here the coordinate vector is (x,y,z). 
            	In the p-> structure x (and rx) are vectors  (double[3]) */
        x = p->rx[0];
        y = p->rx[1];
        z = p->rx[2];
        xdx = x + delta;
        ydy = y + delta;
        zdz = z + delta;
        x_dx = x - delta;
        y_dy = y - delta;
        z_dz = z - delta;
        
        /* Lots of function evaluations here will slow things down. Unavoidable, I'm afraid */
		if ( method == MS_HIRSHFELD)
		{
			F     = hfs_calc_wf(   x,   y,   z, model, hsEnvironment);
			Fx    = hfs_calc_wf( xdx,   y,   z, model, hsEnvironment);
			Fy    = hfs_calc_wf(   x, ydy,   z, model, hsEnvironment);
			Fz    = hfs_calc_wf(   x,   y, zdz, model, hsEnvironment);
			F_x   = hfs_calc_wf(x_dx,   y,   z, model, hsEnvironment);
			F_y   = hfs_calc_wf(   x,y_dy,   z, model, hsEnvironment);
			F_z   = hfs_calc_wf(   x,   y,z_dz, model, hsEnvironment);
			Fxy   = hfs_calc_wf( xdx, ydy,   z, model, hsEnvironment);
			Fx_y  = hfs_calc_wf( xdx,y_dy,   z, model, hsEnvironment);
			F_xy  = hfs_calc_wf(x_dx, ydy,   z, model, hsEnvironment);
			F_x_y = hfs_calc_wf(x_dx,y_dy,   z, model, hsEnvironment);
			Fxz   = hfs_calc_wf( xdx,   y, zdz, model, hsEnvironment);
			Fx_z  = hfs_calc_wf( xdx,   y,z_dz, model, hsEnvironment);
			F_xz  = hfs_calc_wf(x_dx,   y, zdz, model, hsEnvironment);
			F_x_z = hfs_calc_wf(x_dx,   y,z_dz, model, hsEnvironment);
			Fyz   = hfs_calc_wf(   x, ydy, zdz, model, hsEnvironment);
			Fy_z  = hfs_calc_wf(   x, ydy,z_dz, model, hsEnvironment);
			F_yz  = hfs_calc_wf(   x,y_dy, zdz, model, hsEnvironment);
			F_y_z = hfs_calc_wf(   x,y_dy,z_dz, model, hsEnvironment);
		}
		if ( method == MS_SSATOMS)
		{
			F     = ssatoms_calc_density(   x,   y,   z, model);
			Fx    = ssatoms_calc_density( xdx,   y,   z, model);
			Fy    = ssatoms_calc_density(   x, ydy,   z, model);
			Fz    = ssatoms_calc_density(   x,   y, zdz, model);
			F_x   = ssatoms_calc_density(x_dx,   y,   z, model);
			F_y   = ssatoms_calc_density(   x,y_dy,   z, model);
			F_z   = ssatoms_calc_density(   x,   y,z_dz, model);
			Fxy   = ssatoms_calc_density( xdx, ydy,   z, model);
			Fx_y  = ssatoms_calc_density( xdx,y_dy,   z, model);
			F_xy  = ssatoms_calc_density(x_dx, ydy,   z, model);
			F_x_y = ssatoms_calc_density(x_dx,y_dy,   z, model);
			Fxz   = ssatoms_calc_density( xdx,   y, zdz, model);
			Fx_z  = ssatoms_calc_density( xdx,   y,z_dz, model);
			F_xz  = ssatoms_calc_density(x_dx,   y, zdz, model);
			F_x_z = ssatoms_calc_density(x_dx,   y,z_dz, model);
			Fyz   = ssatoms_calc_density(   x, ydy, zdz, model);
			Fy_z  = ssatoms_calc_density(   x, ydy,z_dz, model);
			F_yz  = ssatoms_calc_density(   x,y_dy, zdz, model);
			F_y_z = ssatoms_calc_density(   x,y_dy,z_dz, model);

		}
/*
		if (method == MS_EDEN)
		{
			VEC3SET(r,   x,   y,   z);
			F	  = ms_siesta_eden(r, model);
			VEC3SET(r, xdx,   y,   z);
			Fx	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x, ydy,   z);
			Fy    	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,   y, zdz);
			Fz	  = ms_siesta_eden(r, model);
			VEC3SET(r,x_dx,   y,   z);
			F_x	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,y_dy,   z);
			F_y	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,   y,z_dz);
			F_z	  = ms_siesta_eden(r, model);
			VEC3SET(r, xdx, ydy,   z);
			Fxy	  = ms_siesta_eden(r, model);
			VEC3SET(r, xdx,y_dy,   z);
			Fx_y	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,   y,z_dz);
			F_xy 	  = ms_siesta_eden(r, model);	
			VEC3SET(r,x_dx,y_dy,   z);
			F_x_y	  = ms_siesta_eden(r, model);	
			VEC3SET(r, xdx,   y, zdz);
			Fxz	  = ms_siesta_eden(r, model);	
			VEC3SET(r, xdx,   y,z_dz); 
			Fx_z	  = ms_siesta_eden(r, model);
			VEC3SET(r,x_dx,   y, zdz);
			F_xz 	  = ms_siesta_eden(r, model);	
			VEC3SET(r,x_dx,   y,z_dz);
			F_x_z 	  = ms_siesta_eden(r, model);	
			VEC3SET(r,   x, ydy, zdz);
			Fyz	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x, ydy,z_dz);
			Fy_z	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,y_dy, zdz);
			F_yz	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,y_dy,z_dz);
			F_y_z	  = ms_siesta_eden(r, model);
			
			ms_grad(p, ms_data->cores);

		}
*/

		if (method == MS_MOLECULAR)
		{
			VEC3SET(r,   x,   y,   z);
			F	  = ms_dist(r, model->cores);
			VEC3SET(r, xdx,   y,   z);
			Fx	  = ms_dist(r, model->cores);
			VEC3SET(r,   x, ydy,   z);
			Fy    	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,   y, zdz);
			Fz	  = ms_dist(r, model->cores);
			VEC3SET(r,x_dx,   y,   z);
			F_x	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,y_dy,   z);
			F_y	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,   y,z_dz);
			F_z	  = ms_dist(r, model->cores);
			VEC3SET(r, xdx, ydy,   z);
			Fxy	  = ms_dist(r, model->cores);
			VEC3SET(r, xdx,y_dy,   z);
			Fx_y	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,   y,z_dz);
			F_xy 	  = ms_dist(r, model->cores);	
			VEC3SET(r,x_dx,y_dy,   z);
			F_x_y	  = ms_dist(r, model->cores);	
			VEC3SET(r, xdx,   y, zdz);
			Fxz	  = ms_dist(r, model->cores);	
			VEC3SET(r, xdx,   y,z_dz); 
			Fx_z	  = ms_dist(r, model->cores);
			VEC3SET(r,x_dx,   y, zdz);
			F_xz 	  = ms_dist(r, model->cores);	
			VEC3SET(r,x_dx,   y,z_dz);
			F_x_z 	  = ms_dist(r, model->cores);	
			VEC3SET(r,   x, ydy, zdz);
			Fyz	  = ms_dist(r, model->cores);
			VEC3SET(r,   x, ydy,z_dz);
			Fy_z	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,y_dy, zdz);
			F_yz	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,y_dy,z_dz);
			F_y_z	  = ms_dist(r, model->cores);
			
			/* calculate the surface normal here */
			ms_grad(p, ms_data->cores);

			
		}
		
        
        h[0] = (Fx + F_x - 2*F)*d2;
        h[4] = (Fy + F_y - 2*F)*d2;
        h[8] = (Fz + F_z - 2*F)*d2;
        
        h[1] = (Fxy + F_x_y - F_xy - Fx_y) * d4;
        h[2] = (Fxz + F_x_z - F_xz - Fx_z) * d4;
        h[5] = (Fyz + F_y_z - F_yz - Fy_z) * d4;
        
        h[3] = h[1];
        h[6] = h[2];
        h[7] = h[5];
        
        Dx = (Fx - F_x)*oneOver2Delta;
        Dy = (Fy - F_y)*oneOver2Delta;
        Dz = (Fz - F_z)*oneOver2Delta;
        
        r2[0] = Dx;
        r2[1] = Dy;
        r2[2] = Dz;
        r1[0] = 0.0;
        r1[1] = Dz;
        r1[2] = -Dy;
        
        normalize(r1,3);
        normalize(r2,3);
        crossprod(r0,r1,r2); 		/* this forms an orthonormal set of local axes */
		
        
        ARR3SET(temp,r0);
		vecmat(h, temp);
		mat[0] = DOT3PROD(temp, r0);
		mat[1] = DOT3PROD(temp, r1);
		ARR3SET(temp, r1);
		vecmat(h,temp);
		mat[4] = DOT3PROD(temp, r1);
		
		theta =0.5 * atan2(2*mat[1], mat[4] - mat[0]);
		sinTheta = sin(theta);
		cosTheta = cos(theta);
		eig[0] = cosTheta*cosTheta*mat[0] + sinTheta*sinTheta*mat[4] - 2*sinTheta*cosTheta*mat[1];
		eig[1] = sinTheta*sinTheta*mat[0] + cosTheta*cosTheta*mat[4] + 2*sinTheta*cosTheta*mat[1];
		
		
		h1 = -eig[0] / sqrt(Dx*Dx+Dy*Dy+Dz*Dz);
		h2 = -eig[1] / sqrt(Dx*Dx+Dy*Dy+Dz*Dz);
		
		/* for the time being, set the colour here at the same time */
		
		switch(propertyType){
			case MS_CURVEDNESS:
				//p->property = 2.0*log(sqrt((h1*h1 + h2*h2)/2.0))/M_PI;
				/* functionally equivalent, but presumably much faster */
				p->property = 1.0*log((h1*h1 + h2*h2)/2.0)/M_PI;
				
				/* Curvedness always mapped between -4.0 and 0.4 */
				hue = (p->property + 4.0)/4.4 * colourRange * hueStep;
				hsv2rgb(hue,(float)1.0,(float)1.0,&red,&green,&blue);
				p->colour[0] = red ;
				p->colour[1] = green;
				p->colour[2] = blue;        
				break;
			
			case MS_SHAPE_INDEX:
				if (h1 != h2){
					p->property = 2.0*atan2(h1+h2,MAX(h1,h2)-MIN(h1,h2))/M_PI;
					
				} else {
					p->property = h1/h2;
				}
				/* Shape Index is always mapped on the surface between -1.0 and 1.0 */
				hue = (p->property + 1.0)/2.0 * colourRange * hueStep;
				hsv2rgb(hue,(float)1.0,(float)1.0,&red,&green,&blue);
				p->colour[0] = red ;
				p->colour[1] = green;
				p->colour[2] = blue;        
				
				break;
		}
	}
#if DEBUG_SHAPE
	
	fprintf(stderr," [%.1fs]\n",(float)(time(NULL) - jobStartTime));
#endif
    return;
}

#if INCLUDE_NEW_PROPERTY

void ms_property_shape(GSList *points, struct model_pak *model, gint method)
{
    GSList *pointsList;
    struct smv_pak *p;
    gdouble delta = 5.0e-1; 						/* distance offset for calculating numerical seconds */
    gdouble oneOver2Delta, d2, d4;
    gdouble h1, h2; 								/* principal curvatures */
    gdouble F, Fx, Fy, Fz, F_x, F_y, F_z, 			/* Value of the surface function around the point */
		Fxy, Fx_y, F_xy, F_x_y,
		Fxz, Fx_z, F_xz, F_x_z,
		Fyz, Fy_z, F_yz, F_y_z;
    gdouble x,y,z, xdx, x_dx, ydy, y_dy, zdz, z_dz; /* coordinates around the point */
	gdouble r[3];									/* point to pass to functions that need a [3] */
    gdouble h[9]; 								/* hessian */
	gdouble eig[3];									/* eigenvalues of the hessian */
    gdouble r0[3], r1[3], r2[3]; 
	gdouble mat[9];
    gdouble Dx, Dy, Dz; 							/* gradient */
	gdouble temp[3]; 								/* the dreaded temporary variable */
	gdouble theta, sinTheta, cosTheta;
	gfloat hue, red, green, blue;
	time_t jobStartTime;
	gchar *messageText;
	
#if DEBUG_SHAPE 
	messageText = g_strdup_printf("Calculating curvature-based surface properties... "); 
	gui_text_show(STANDARD, text);
	g_free(text);
	jobStartTime = time(NULL);
#endif
	
	/* setup for colours */
    gfloat colourRange= 256;
    gfloat hueStep;
    
    hueStep = 240/colourRange;
	
	
    oneOver2Delta = 0.5/delta;
    d2 = 1.0/(delta*delta);
    d4 = d2/4;
    
    
    for (pointsList=points ; pointsList ; pointsList=g_slist_next(pointsList))
    {
        p = pointsList->data;
        if (!p->adj)
            continue;
        /*	Don't be confused: x is a single double. Here the coordinate vector is (x,y,z). 
			In the p-> structure x (and rx) are vectors  (double[3]) */
        x = p->rx[0];
        y = p->rx[1];
        z = p->rx[2];
        xdx = x + delta;
        ydy = y + delta;
        zdz = z + delta;
        x_dx = x - delta;
        y_dy = y - delta;
        z_dz = z - delta;
        
        /* Lots of function evaluations here will slow things down. Unavoidable, I'm afraid */
		if ( method == MS_HIRSHFELD)
		{
			F     = hfs_calc_wf(   x,   y,   z, model, hsEnvironment);
			Fx    = hfs_calc_wf( xdx,   y,   z, model, hsEnvironment);
			Fy    = hfs_calc_wf(   x, ydy,   z, model, hsEnvironment);
			Fz    = hfs_calc_wf(   x,   y, zdz, model, hsEnvironment);
			F_x   = hfs_calc_wf(x_dx,   y,   z, model, hsEnvironment);
			F_y   = hfs_calc_wf(   x,y_dy,   z, model, hsEnvironment);
			F_z   = hfs_calc_wf(   x,   y,z_dz, model, hsEnvironment);
			Fxy   = hfs_calc_wf( xdx, ydy,   z, model, hsEnvironment);
			Fx_y  = hfs_calc_wf( xdx,y_dy,   z, model, hsEnvironment);
			F_xy  = hfs_calc_wf(x_dx, ydy,   z, model, hsEnvironment);
			F_x_y = hfs_calc_wf(x_dx,y_dy,   z, model, hsEnvironment);
			Fxz   = hfs_calc_wf( xdx,   y, zdz, model, hsEnvironment);
			Fx_z  = hfs_calc_wf( xdx,   y,z_dz, model, hsEnvironment);
			F_xz  = hfs_calc_wf(x_dx,   y, zdz, model, hsEnvironment);
			F_x_z = hfs_calc_wf(x_dx,   y,z_dz, model, hsEnvironment);
			Fyz   = hfs_calc_wf(   x, ydy, zdz, model, hsEnvironment);
			Fy_z  = hfs_calc_wf(   x, ydy,z_dz, model, hsEnvironment);
			F_yz  = hfs_calc_wf(   x,y_dy, zdz, model, hsEnvironment);
			F_y_z = hfs_calc_wf(   x,y_dy,z_dz, model, hsEnvironment);
		}
		if (method == MS_EDEN)
		{
			VEC3SET(r,   x,   y,   z);
			F	  = ms_siesta_eden(r, model);
			VEC3SET(r, xdx,   y,   z);
			Fx	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x, ydy,   z);
			Fy    	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,   y, zdz);
			Fz	  = ms_siesta_eden(r, model);
			VEC3SET(r,x_dx,   y,   z);
			F_x	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,y_dy,   z);
			F_y	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,   y,z_dz);
			F_z	  = ms_siesta_eden(r, model);
			VEC3SET(r, xdx, ydy,   z);
			Fxy	  = ms_siesta_eden(r, model);
			VEC3SET(r, xdx,y_dy,   z);
			Fx_y	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,   y,z_dz);
			F_xy 	  = ms_siesta_eden(r, model);	
			VEC3SET(r,x_dx,y_dy,   z);
			F_x_y	  = ms_siesta_eden(r, model);	
			VEC3SET(r, xdx,   y, zdz);
			Fxz	  = ms_siesta_eden(r, model);	
			VEC3SET(r, xdx,   y,z_dz); 
			Fx_z	  = ms_siesta_eden(r, model);
			VEC3SET(r,x_dx,   y, zdz);
			F_xz 	  = ms_siesta_eden(r, model);	
			VEC3SET(r,x_dx,   y,z_dz);
			F_x_z 	  = ms_siesta_eden(r, model);	
			VEC3SET(r,   x, ydy, zdz);
			Fyz	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x, ydy,z_dz);
			Fy_z	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,y_dy, zdz);
			F_yz	  = ms_siesta_eden(r, model);
			VEC3SET(r,   x,y_dy,z_dz);
			F_y_z	  = ms_siesta_eden(r, model);
			
			/* calculate the surface normal here */
			ms_grad(p, ms_data->cores);
			
		}
		if (method == MS_MOLECULAR)
		{
			VEC3SET(r,   x,   y,   z);
			F	  = ms_dist(r, model->cores);
			VEC3SET(r, xdx,   y,   z);
			Fx	  = ms_dist(r, model->cores);
			VEC3SET(r,   x, ydy,   z);
			Fy    	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,   y, zdz);
			Fz	  = ms_dist(r, model->cores);
			VEC3SET(r,x_dx,   y,   z);
			F_x	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,y_dy,   z);
			F_y	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,   y,z_dz);
			F_z	  = ms_dist(r, model->cores);
			VEC3SET(r, xdx, ydy,   z);
			Fxy	  = ms_dist(r, model->cores);
			VEC3SET(r, xdx,y_dy,   z);
			Fx_y	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,   y,z_dz);
			F_xy 	  = ms_dist(r, model->cores);	
			VEC3SET(r,x_dx,y_dy,   z);
			F_x_y	  = ms_dist(r, model->cores);	
			VEC3SET(r, xdx,   y, zdz);
			Fxz	  = ms_dist(r, model->cores);	
			VEC3SET(r, xdx,   y,z_dz); 
			Fx_z	  = ms_dist(r, model->cores);
			VEC3SET(r,x_dx,   y, zdz);
			F_xz 	  = ms_dist(r, model->cores);	
			VEC3SET(r,x_dx,   y,z_dz);
			F_x_z 	  = ms_dist(r, model->cores);	
			VEC3SET(r,   x, ydy, zdz);
			Fyz	  = ms_dist(r, model->cores);
			VEC3SET(r,   x, ydy,z_dz);
			Fy_z	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,y_dy, zdz);
			F_yz	  = ms_dist(r, model->cores);
			VEC3SET(r,   x,y_dy,z_dz);
			F_y_z	  = ms_dist(r, model->cores);
			
			/* calculate the surface normal here */
			ms_grad(p, ms_data->cores);
			
			
		}
		
        
        h[0] = (Fx + F_x - 2*F)*d2;
        h[4] = (Fy + F_y - 2*F)*d2;
        h[8] = (Fz + F_z - 2*F)*d2;
        
        h[1] = (Fxy + F_x_y - F_xy - Fx_y) * d4;
        h[2] = (Fxz + F_x_z - F_xz - Fx_z) * d4;
        h[5] = (Fyz + F_y_z - F_yz - Fy_z) * d4;
        
        h[3] = h[1];
        h[6] = h[2];
        h[7] = h[5];
        
        Dx = (Fx - F_x)*oneOver2Delta;
        Dy = (Fy - F_y)*oneOver2Delta;
        Dz = (Fz - F_z)*oneOver2Delta;
        
        r2[0] = Dx;
        r2[1] = Dy;
        r2[2] = Dz;
        r1[0] = 0.0;
        r1[1] = Dz;
        r1[2] = -Dy;
        
        normalize(r1,3);
        normalize(r2,3);
        crossprod(r0,r1,r2); 		/* this forms an orthonormal set of local axes */
		
        
        ARR3SET(temp,r0);
		vecmat(h, temp);
		mat[0] = DOT3PROD(temp, r0);
		mat[1] = DOT3PROD(temp, r1);
		ARR3SET(temp, r1);
		vecmat(h,temp);
		mat[4] = DOT3PROD(temp, r1);
		
		theta =0.5 * atan2(2*mat[1], mat[4] - mat[0]);
		sinTheta = sin(theta);
		cosTheta = cos(theta);
		eig[0] = cosTheta*cosTheta*mat[0] + sinTheta*sinTheta*mat[4] - 2*sinTheta*cosTheta*mat[1];
		eig[1] = sinTheta*sinTheta*mat[0] + cosTheta*cosTheta*mat[4] + 2*sinTheta*cosTheta*mat[1];
		
		
		h1 = -eig[0] / sqrt(Dx*Dx+Dy*Dy+Dz*Dz);
		h2 = -eig[1] / sqrt(Dx*Dx+Dy*Dy+Dz*Dz);
		
		

				/* by definition, curvedness is
				p->property = 2.0*log(sqrt((h1*h1 + h2*h2)/2.0))/M_PI;
				but we remove the sqrt to significantly improve speed
				(except on my G5, where it runs at the same speed!) */
				p->properties->curvedness = 1.0*log((h1*h1 + h2*h2)/2.0)/M_PI;
				
				
				if (h1 != h2){
					p->properties->shape_index = 2.0*atan2(h1+h2,MAX(h1,h2)-MIN(h1,h2))/M_PI;
					
				} else {
					p->properties->shape_index = h1/h2;
				}

	}
#if DEBUG_SHAPE
	text = g_strdup_printf("[%.1fs]\n",(float)(time(NULL) - jobStartTime)); 
	gui_text_show(STANDARD, text);
	g_free(text);

#endif
    return;
}

#endif
/*****************************************************/
/* colour by de (Hirshfeld surface) */
/*****************************************************/
void ms_colour_by_de(GSList *points, struct model_pak *model)
{
GSList *pointsList, *hsEnvList, *selection, *selectionList;
struct smv_pak *p;
gdouble dist, tmp, tmpVec[3], currentEnvCore[3], minProp, maxProp, propRange, thisProp;
gfloat hue, red, green, blue;
int firstPoint, firstCore, atomIsInMolecule;
struct core_pak *hsEnvCore, *selectionCore;
gchar *text;
time_t jobStartTime;
/* setup for colours */
gfloat colourRange= 256;
gfloat hueStep;

minProp = G_MAXDOUBLE;
maxProp = -G_MAXDOUBLE;

#if DEBUG_DE
	fprintf(stderr," [d_e] ");
#endif
    hueStep = 240/colourRange;
    jobStartTime = time(NULL);
    firstPoint = TRUE;
    for (pointsList=points ; pointsList ; pointsList=g_slist_next(pointsList))
    {
        p = pointsList->data;
        if (!p->adj)
            continue;

        firstCore = TRUE;
        selection = model->selection;
        /** test if current atom is actually *in* the molecule
        (because if it is, it must be excluded) **/
        for (hsEnvList = hsEnvironment; hsEnvList; hsEnvList = g_slist_next(hsEnvList)){
            hsEnvCore = hsEnvList->data;
            atomIsInMolecule = FALSE;
            ARR3SET(currentEnvCore, hsEnvCore->x);
            vecmat(model->latmat,currentEnvCore);
            for (selectionList = selection; selectionList; selectionList = g_slist_next(selectionList)){
                if (atomIsInMolecule == FALSE){
                    selectionCore = selectionList->data;
                    ARR3SET(tmpVec,selectionCore->x);
                    vecmat(model->latmat,tmpVec);  
                    ARR3SUB(tmpVec,currentEnvCore);
                    tmp = VEC3MAG(tmpVec);
                    if (tmp < 1.0e-1) { /* a crude test, I know */
                        atomIsInMolecule = TRUE;
                        break;
                    }
                    
                }
            }
            if (atomIsInMolecule == FALSE){
                ARR3SUB(currentEnvCore,p->rx);
                dist = VEC3MAG(currentEnvCore); 
                if (firstCore) p->property = dist;
                if (dist < p->property) p->property = dist;
            }
            firstCore = FALSE;
        }
        if (firstPoint){
            minProp = p->property;
            maxProp = p->property;
        } else {
            minProp = ( p->property < minProp ? p->property : minProp);
            maxProp = ( p->property > maxProp ? p->property : maxProp);
        }
        firstPoint = FALSE;
    }
//    minProp = 0.7;
    text = g_strdup_printf("range of de = %.2f - %.2f\n", minProp, maxProp);
    gui_text_show(NORMAL, text);
    g_free(text);
    
    /* For now, just set the colour here */
    propRange = maxProp - minProp;
    firstPoint = TRUE;
    for (pointsList=points ; pointsList ; pointsList=g_slist_next(pointsList)){
        p = pointsList->data;
        if (!p->adj)
            continue;
        thisProp = (p->property - minProp)/propRange;
        hue = thisProp * colourRange * hueStep;
        hsv2rgb(hue,(float)1.0,(float)1.0,&red,&green,&blue);
        p->colour[0] = red ;
        p->colour[1] = green;
        p->colour[2] = blue;        
    }
#if DEBUG_DE

    fprintf(stderr," [%.1fs]\n",(float)(time(NULL) - jobStartTime));
#endif
}

/*****************************************************/
/* calculate de (Hirshfeld surface) */
/*****************************************************/
#if INCLUDE_NEW_PROPERTY
void ms_property_de(GSList *points, struct model_pak *model)
{
    GSList *pointsList, *hsEnvList, *selection, *selectionList;
    struct smv_pak *p;
    gdouble dist, tmp, tmpVec[3], currentEnvCore[3], minDe, maxDe, minDi, maxDi, propRange, thisProp;
    gfloat hue, red, green, blue;
    int firstPoint, firstCore, atomIsInMolecule;
    struct core_pak *hsEnvCore, *selectionCore;
    gchar *messageText;
    time_t jobStartTime;
    
    /* setup for colours */
    gfloat colourRange= 256;
    gfloat hueStep;
#if DEBUG_DE
	messageText = g_strdup_printf("Calculating de on the Hirshfeld surface... "); 
	gui_text_show(STANDARD, text);
	g_free(text);
	
#endif
    hueStep = 240/colourRange;
    jobStartTime = time(NULL);
    firstPoint = TRUE;
    for (pointsList=points ; pointsList ; pointsList=g_slist_next(pointsList))
    {
        p = pointsList->data;
        if (!p->adj)
            continue;
		
        firstCore = TRUE;
        selection = model->selection;
        /** test if current atom is actually *in* the molecule
			(because if it is, it must be excluded) **/
        for (hsEnvList = hsEnvironment; hsEnvList; hsEnvList = g_slist_next(hsEnvList)){
            hsEnvCore = hsEnvList->data;
            atomIsInMolecule = FALSE;
            ARR3SET(currentEnvCore, hsEnvCore->x);
            vecmat(model->latmat,currentEnvCore);
            for (selectionList = selection; selectionList; selectionList = g_slist_next(selectionList)){
                if (atomIsInMolecule == FALSE){
                    selectionCore = selectionList->data;
                    ARR3SET(tmpVec,selectionCore->x);
                    vecmat(model->latmat,tmpVec);  
                    ARR3SUB(tmpVec,currentEnvCore);
                    tmp = VEC3MAG(tmpVec);
                    if (tmp < 1.0e-1) { /* a crude test, I know */
                        atomIsInMolecule = TRUE;
                        break;
                    }
                    
                }
            }
                ARR3SUB(currentEnvCore,p->rx);
                dist = VEC3MAG(currentEnvCore);
            if (atomIsInMolecule == FALSE){ /* this is the de part */
				if (firstCore) {
					p->property->de = dist;
					/* JJM - How do I get the atomic number???
					p->property->eType = hsEnvCore->  */
				}
                if (dist < p->property->de) {
					p->property->de = dist;
					/*p->property->eType = hsEnvCore-> */
				}
            } else { 						/* this is the di part */
				if (firstCore) {
					p->property->di = dist;
					/*p->property->iType = hsEnvCore->  */
				}
                if (dist < p->property->de) {
					p->property->di = dist;
					/*p->property->iType = hsEnvCore-> */
				}
			}
            firstCore = FALSE;
        }
        if (firstPoint){
            minDe = p->property->de;
            maxDe = p->property->de;
			minDi = p->property->di;
            maxDi = p->property->di;
        } else {
            minDe = ( p->property->de < minDe ? p->property->de : minDe);
            maxDe = ( p->property->de > maxDe ? p->property->de : maxDe);
			minDi = ( p->property->di < minDi ? p->property->di : minDi);
            maxDi = ( p->property->di > maxDi ? p->property->di : maxDi);
        }
        firstPoint = FALSE;
    }

messageText = g_strdup_printf("range of de = %.2f - %.2f\nrange of di = %.2f - %.2f\n", minDe, maxDe, minDi,maxDi);
gui_text_show(NORMAL, messageText);
g_free(messageText);

/* For now, just set the colour here */
propRange = maxProp - minProp;
firstPoint = TRUE;
for (pointsList=points ; pointsList ; pointsList=g_slist_next(pointsList)){
	p = pointsList->data;
	if (!p->adj)
		continue;
	thisProp = (p->property - minProp)/propRange;
	hue = thisProp * colourRange * hueStep;
	hsv2rgb(hue,(float)1.0,(float)1.0,&red,&green,&blue);
	p->colour[0] = red ;
	p->colour[1] = green;
	p->colour[2] = blue;        
}
#if DEBUG_DE
messageText = g_strdup_printf(" [%.1fs]\n",(float)(time(NULL) - jobStartTime));
gui_text_show(NORMAL, messageText);
g_free(messageText);
#endif
}
#endif

/*********************************************************************/
/* colour by the dialog default value (electron density iso-surface) */
/*********************************************************************/
void ms_colour_by_density(GSList *points)
{
GSList *list;
struct smv_pak *p;

for (list=points ; list ; list=g_slist_next(list))
  {
  p = list->data;

/* TODO - try for a speedup using zones */
  ms_grad(p, ms_data->cores);

/* NB: assumes value is in the range [0, 1] */
  ARR3SET(p->colour, sysenv.render.ms_colour);
  }
}

/************************************************************/
/* colour by the given value (electron density iso-surface) */
/************************************************************/
void ms_colour_by_siesta_density(gdouble value, GSList *points)
{
GSList *list;
struct smv_pak *p;

for (list=points ; list ; list=g_slist_next(list))
  {
  p = list->data;

/* TODO - try for a speedup using zones */
  ms_grad(p, ms_data->cores);

/* NB: assumes value is in the range [0, 1] */
  VEC3SET(p->colour, 0.0, 0.0, 0.5);
  VEC3MUL(p->colour, value);
  VEC3ADD(p->colour, 0.2, 0.2, 0.5);
  }
}

/********************************************/
/* colour by siesta electrostatic potential */
/********************************************/
void ms_colour_by_siesta_potential(GSList *points)
{
gdouble q;
GSList *list1;
struct smv_pak *p;
struct model_pak *model;

model = tree_model_get();
g_assert(model != NULL);
if (!model->siesta.epot)
  {
  gui_text_show(ERROR, "Missing electrostatic data file.\n");
  return;
  }

/* setup for colour scaling */
model->ms_colour_scale = TRUE;
if (model->epot_autoscale)
  {
  ms_siesta_epot_range(points, model);
  model->epot_min = model->siesta.epot_min;
  model->epot_max = model->siesta.epot_max;
  model->epot_div = 11;
  }

/* point colouring */
for (list1=points ; list1 ; list1=g_slist_next(list1))
  {
  p = list1->data;
  if (!p->adj)
    continue;

  ms_grad(p, ms_data->cores);
//  q = ms_seista_epot(p->rx, model); 
// FIXME
q = 0.0;
  ms_epot_colour(p->colour, q, model->epot_min, model->epot_max);
  }
}

/*******************************************/
/* colour according to solvent interaction */
/*******************************************/
void ms_dock_colour(gdouble *c, gdouble value, gdouble min, gdouble max)
{
gdouble f;

f = value - min;
f /= (max - min);
f = 1.0 - f;

VEC3SET(c, 1.0, f, f);
}

/*******************************************/
/* colour according to solvent interaction */
/*******************************************/
void ms_colour_by_solvent(GSList *points)
{
gint i, j, a, b;
gdouble e, min, max;
GSList *list;
gdouble *surface, x[3];
struct smv_pak *p;
struct model_pak *model;

/* FIXME - redo all this */
return;

model = tree_model_get();
g_assert(model != NULL);

/*
project = model->project;
if (!project)
  {
  gui_text_show(ERROR, "Missing project data.\n");
  return;
  }
*/

/* retrieve surface interaction energy map */
/*
a = GPOINTER_TO_INT(project_data_get("dock:a", project));
b = GPOINTER_TO_INT(project_data_get("dock:b", project));

min = str_to_float(project_data_get("dock:min", project));
max = str_to_float(project_data_get("dock:max", project));

surface = project_data_get("dock:energy", project);
if (!surface)
  {
  gui_text_show(ERROR, "Missing dock data.\n");
  return;
  }
*/

printf("=======================================\n");
printf("ms_colour_by_solvent:\n");
printf("min = %f\n", min);
printf("max = %f\n", max);
printf("=======================================\n");

/*
if (surface)
  {
  for (j=0 ; j<b ; j++)
    {
    for (i=0 ; i<a ; i++)
      {
printf("[%d][%d] : %f\n", i, j, *(surface+j*a+i));
      }
    }
  }
*/

model->ms_colour_scale = TRUE;
model->ms_colour_method = MS_SOLVENT;

/* point colouring */
for (list=points ; list ; list=g_slist_next(list))
  {
  p = list->data;
  if (!p->adj)
    continue;

  ms_grad(p, ms_data->cores);

  ARR3SET(x, p->rx);
  vecmat(model->ilatmat, x);

/* FIXME - we really need nearest_int() compensated for image repeats ie 4 -> 0 */
/* fractional coord -> grid point */
  i = nearest_int(x[0]*a);
  j = nearest_int(x[1]*b);
/* cope with periodic wrap around */
  i %= a;
  j %= b;

g_assert(i >= 0);
g_assert(j >= 0);

  e = *(surface+j*a+i);
  ms_dock_colour(p->colour, e, min, max);
  }
}

/**************************/
/* general colouring call */
/**************************/
void ms_colour_surface(gint method, gint colour, GSList *ms_points, struct model_pak *model)
{
if (!ms_points)
  return;

switch (method)
  {
/*
  case MS_EDEN:
    switch (colour)
      {
      case MS_EPOT:
        ms_colour_by_siesta_potential(ms_points);
        break;

      case MS_CURVEDNESS:
        ms_colour_by_shape(ms_points, model, colour, method);
        break;

      case MS_SHAPE_INDEX:
        ms_colour_by_shape(ms_points, model, colour, method);
        break;

      case MS_DENSITY_VALUE:
        ms_colour_by_density(ms_points);
        break;

      default:
        ms_colour_by_siesta_density(0.3, ms_points);
      }
    break;
*/

  case MS_HIRSHFELD:
    switch (colour)
      {
      case MS_CURVEDNESS:
      case MS_SHAPE_INDEX:
	ms_colour_by_shape(ms_points, model, colour, method);
	break;

      case MS_EPOT:
	ms_colour_by_potential(ms_points);
	break;

      case MS_DE:
      default:
   	ms_colour_by_de(ms_points, model);
	break;
      }
    break;

  default:
    switch (colour)
      {
      case MS_AFM:
        ms_colour_by_height(ms_points);
        break;

      case MS_EPOT:
        ms_colour_by_potential(ms_points);
        break;

      case MS_SOLVENT:
        ms_colour_by_solvent(ms_points);
        break;

      case MS_CURVEDNESS:
        ms_colour_by_shape(ms_points, model, colour, method);
        break;
		
      case MS_SHAPE_INDEX:
        ms_colour_by_shape(ms_points, model, colour, method);
        break;

      case MS_DE:
      case MS_DENSITY_VALUE:
        ms_colour_by_density(ms_points);
        break;
 
      default:
        ms_colour_by_touch(ms_points);
        break;
      }
  }
}

/**********************************************************/
/* correct the trianglulation by removing small triangles */
/**********************************************************/
void ms_adjust_triangles(GSList *tlist, struct model_pak *model)
{
gint count;
gdouble r2, x[3];
struct smt_pak *triangle;
struct smv_pak *p1, *p2, *p3;
GSList *list;

list = tlist;

/* cutoff for determining a poor triangle = 20% of grid size */
r2 = 0.2*sysenv.render.ms_grid_size;
r2 *= r2;

while (list)
  {
  triangle = list->data;

  list = g_slist_next(list);

  count=0;
  p1=p2=NULL;

  ARR3SET(x, triangle->point[0]->rx);
  ARR3SUB(x, triangle->point[1]->rx);
  if (VEC3MAGSQ(x) < r2)
    {
    p1 = triangle->point[0];
    p2 = triangle->point[1];
    count++;
    }

  ARR3SET(x, triangle->point[0]->rx);
  ARR3SUB(x, triangle->point[2]->rx);
  if (VEC3MAGSQ(x) < r2)
    {
    p1 = triangle->point[0];
    p2 = triangle->point[2];
    count++;
    }

  ARR3SET(x, triangle->point[1]->rx);
  ARR3SUB(x, triangle->point[2]->rx);
  if (VEC3MAGSQ(x) < r2)
    {
    p1 = triangle->point[1];
    p2 = triangle->point[2];
    count++;
    }

  switch (count)
    {
    case 1:
/* delete triangle */
/*
tlist = g_slist_remove(tlist, triangle);

      ARR3SET(x, p1->rx);
      ARR3ADD(x, p2->rx);
      VEC3MUL(x, 0.5);
      ARR3SET(p1->rx, x);
      ARR3SET(p2->rx, x);
*/
      break;
       

    case 3:
/* delete triangle */
tlist = g_slist_remove(tlist, triangle);

      p1 = triangle->point[0];
      p2 = triangle->point[1];
      p3 = triangle->point[2];

      ARR3SET(x, p1->rx);
      ARR3ADD(x, p2->rx);
      ARR3ADD(x, p3->rx);
      VEC3MUL(x, 0.333333333);
      ARR3SET(p1->rx, x);
      ARR3SET(p2->rx, x);
      ARR3SET(p3->rx, x);

      break;
    }

  }
}

/***********************************************/
/* create triangles for surface representation */
/***********************************************/

#define DEBUG_CREATE_TRIANGLES 0

void ms_create_triangles(GSList *tlist, struct model_pak *model, gint method)
{
gdouble x1[3], x2[3], x3[3], n1[3], n2[3], n3[3];
GSList *item1;
struct smt_pak *triangle;
struct spatial_pak *spatial;

#if DEBUG_CREATE_TRIANGLES
fprintf(stderr,"creating triangles...\n");
#endif

g_assert(model != NULL);

/* init a molsurf (triangles) spatial */
spatial = spatial_new("molsurf", SPATIAL_MOLSURF, 3, TRUE, model);

/* NB: best to calc the normals here - so it's indep. of the colouring method */
for (item1=tlist ; item1 ; item1=g_slist_next(item1))
  {
  triangle = item1->data;

/* fill out the vertex list */
/* copy vertex colour, coords & normal */
  ARR3SET(x1, triangle->point[0]->rx);
  ARR3SET(x2, triangle->point[1]->rx);
  ARR3SET(x3, triangle->point[2]->rx);
  
/* CURRENT - ms_grad gives better normals (and better looking surfaces) but is slow */
  if (ms_calc_grad)
    {
    ARR3SET(n1, triangle->point[0]->nx);
    ARR3SET(n2, triangle->point[1]->nx);
    ARR3SET(n3, triangle->point[2]->nx);
    if (method != MS_HIRSHFELD && method != MS_SSATOMS )
      {
    /*JJM This results in screwy normals on the Hirshfeld surface */
      VEC3MUL(n1, -1.0);
      VEC3MUL(n2, -1.0);
      VEC3MUL(n3, -1.0);
      }
    vecmat(model->ilatmat, n1);
    vecmat(model->ilatmat, n2);
    vecmat(model->ilatmat, n3);
    vecmat(model->ilatmat, x1);
    vecmat(model->ilatmat, x2);
    vecmat(model->ilatmat, x3);
    spatial_vnorm_add(x1, n1, triangle->point[0]->colour, spatial);
    spatial_vnorm_add(x2, n2, triangle->point[1]->colour, spatial);
    spatial_vnorm_add(x3, n3, triangle->point[2]->colour, spatial);
    }
  else
    {
    calc_norm(n1, x1, x2, x3);
    VEC3MUL(n1, -1.0);
    vecmat(model->ilatmat, n1);
    vecmat(model->ilatmat, x1);
    vecmat(model->ilatmat, x2);
    vecmat(model->ilatmat, x3);
    spatial_vnorm_add(x1, n1, triangle->point[0]->colour, spatial);
    spatial_vnorm_add(x2, n1, triangle->point[1]->colour, spatial);
    spatial_vnorm_add(x3, n1, triangle->point[2]->colour, spatial);
    }
  }
}

/**********************************************/
/* triangulation post process and improvement */
/**********************************************/
GSList *ms_process(gdouble r, GSList *tlist)
{
gint count, delete=0;
gdouble x[3];
GSList *item;
struct smv_pak *p[3];
struct smt_pak *triangle;

printf("ms_process(): start triangles = %d\n", g_slist_length(tlist));

printf("char length = %f\n", r);

/* enumerate triangles */
item = tlist;
while (item)
for (item=tlist ; item ; item=g_slist_next(item))
  {
  triangle = item->data;
  item = g_slist_next(item);

  p[0] = triangle->point[0];
  p[1] = triangle->point[1];
  p[2] = triangle->point[2];

/* determine if the triangle should be scrapped */
  x[0] = calc_sep(p[0]->rx, p[1]->rx);
  x[1] = calc_sep(p[0]->rx, p[2]->rx);
  x[2] = calc_sep(p[1]->rx, p[2]->rx);

//printf("[%f , %f , %f]\n", a/r, b/r, c/r);
// cutoff is basically a percentage of the charactistic grid
// body dialogal length used in the marching cubes algorithm
#define MS_CUTOFF 0.2

  count=0;
  if (x[0]/r < MS_CUTOFF)
    {
    ARR3SET(p[0]->rx, p[1]->rx);
    count++;
    }
  if (x[1]/r < MS_CUTOFF)
    {
    ARR3SET(p[2]->rx, p[0]->rx);
    count++;
    }
  if (x[2]/r < MS_CUTOFF)
    {
    ARR3SET(p[1]->rx, p[2]->rx);
    count++;
    }

  if (count)
    {
printf("deleting [%d][%d][%d]\n", p[0]->index, p[1]->index, p[2]->index);
P3VEC("0 : ", p[0]->rx);
P3VEC("1 : ", p[1]->rx);
P3VEC("2 : ", p[2]->rx);


    delete++;
    tlist = g_slist_remove(tlist, triangle);
    }


/*
if ((p[0]->index == p[1]->index) ||
    (p[0]->index == p[2]->index) ||
    (p[1]->index == p[2]->index))
  {
    tlist = g_slist_remove(tlist, triangle);
  }
*/


/* update adjacent triangles */

  }
  

printf("ms_process(): stop triangles = %d\n", g_slist_length(tlist));

return(tlist);
}

/******************************************************************/
/* Linearly interpolate the position where an isosurface cuts     */
/* an edge between two vertices, each with their own scalar value */
/******************************************************************/
struct smv_pak *ms_interpolate(gdouble isolevel, struct smv_pak *p1, struct smv_pak *p2)
{
gdouble mu, valp1, valp2, x[3];
struct smv_pak *p;

valp1 = p1->value;
valp2 = p2->value;

if (fabs(isolevel-valp1) < 0.00001)
  return(p1);
if (fabs(isolevel-valp2) < 0.00001)
  return(p2);
if (fabs(valp1-valp2) < 0.00001)
  return(p1);

mu = (isolevel - valp1) / (valp2 - valp1);
x[0] = p1->rx[0] + mu * (p2->rx[0] - p1->rx[0]);
x[1] = p1->rx[1] + mu * (p2->rx[1] - p1->rx[1]);
x[2] = p1->rx[2] + mu * (p2->rx[2] - p1->rx[2]);

p = new_point(x);

//p->index = (p1->index << 16) | (p2->index >> 16);
p->index = 0.5 * (p1->index + p2->index) * (p1->index + p2->index + 1) + p2->index;

return(p);
}

/*******************************************/
/* compute the triangulation for the given */
/* cube at the supplied isosurface value   */
/*******************************************/
GSList *ms_triangulate(gdouble isolevel, GSList *cube)
{
gint i, cubeindex;
struct smv_pak *p[8], *v[12], *v1, *v2, *v3;
struct smt_pak *t;
GSList *list;

// warn?
if (g_slist_length(cube) != 8)
  return(NULL);

i=0;
for (list=cube ; list ; list=g_slist_next(list))
  p[i++] = list->data;

cubeindex = 0;
if (p[0]->value < isolevel)
  cubeindex |= 1;
if (p[1]->value < isolevel)
  cubeindex |= 2;
if (p[2]->value < isolevel)
  cubeindex |= 4;
if (p[3]->value < isolevel)
  cubeindex |= 8;
if (p[4]->value < isolevel)
  cubeindex |= 16;
if (p[5]->value < isolevel)
  cubeindex |= 32;
if (p[6]->value < isolevel)
  cubeindex |= 64;
if (p[7]->value < isolevel)
  cubeindex |= 128;

/* cube is entirely in/out of the surface */
if (!ms_edge_table[cubeindex])
  return(NULL);

/* find the vertices where the surface intersects the cube */
if (ms_edge_table[cubeindex] & 1)
  v[0] = ms_interpolate(isolevel,p[0],p[1]);
if (ms_edge_table[cubeindex] & 2)
  v[1] = ms_interpolate(isolevel,p[1],p[2]);
if (ms_edge_table[cubeindex] & 4)
  v[2] = ms_interpolate(isolevel,p[2],p[3]);
if (ms_edge_table[cubeindex] & 8)
  v[3] = ms_interpolate(isolevel,p[3],p[0]);
if (ms_edge_table[cubeindex] & 16)
  v[4] = ms_interpolate(isolevel,p[4],p[5]);
if (ms_edge_table[cubeindex] & 32)
  v[5] = ms_interpolate(isolevel,p[5],p[6]);
if (ms_edge_table[cubeindex] & 64)
  v[6] = ms_interpolate(isolevel,p[6],p[7]);
if (ms_edge_table[cubeindex] & 128)
  v[7] = ms_interpolate(isolevel,p[7],p[4]);
if (ms_edge_table[cubeindex] & 256)
  v[8] = ms_interpolate(isolevel,p[0],p[4]);
if (ms_edge_table[cubeindex] & 512)
  v[9] = ms_interpolate(isolevel,p[1],p[5]);
if (ms_edge_table[cubeindex] & 1024)
  v[10] = ms_interpolate(isolevel,p[2],p[6]);
if (ms_edge_table[cubeindex] & 2048)
  v[11] = ms_interpolate(isolevel,p[3],p[7]);

/* create the triangulation */
list = NULL;
for (i=0 ; ms_tri_table[cubeindex][i]!=-1 ; i+=3)
  {
  v1 = v[ms_tri_table[cubeindex][i]];
  v2 = v[ms_tri_table[cubeindex][i+1]];
  v3 = v[ms_tri_table[cubeindex][i+2]];
  t = new_triangle(v1, v2, v3);
  list = g_slist_prepend(list, t);
  }
return(list);
}

/***************************/
/* cube indexing primitive */
/***************************/
gint ms_index(gint i, gint j, gint k, gint *g)
{
return(i*g[1]*g[2] + j*g[2] + k);
}

/***********************************************************/
/* check the appropriate siesta grid data has been read in */
/***********************************************************/
gint ms_siesta_check(gint method, struct model_pak *model)
{
gpointer import;
gchar *filename=NULL;

import = gui_tree_parent_object(model);
if (!import)
  {
  gui_text_show(ERROR, "Failed to locate import for model.\n");
  return(FALSE);
  }

switch (method)
  {
  case MS_SIESTA_RHO:
    if (!model->siesta.grid_rho)
      filename = parse_extension_set(import_fullpath(import), "RHO");
    break;
  case MS_SIESTA_DRHO:
    if (!model->siesta.grid_drho)
      filename = parse_extension_set(import_fullpath(import), "DRHO");
    break;
  case MS_SIESTA_LDOS:
    if (!model->siesta.grid_ldos)
      filename = parse_extension_set(import_fullpath(import), "LDOS");
    break;
  case MS_SIESTA_VH:
    if (!model->siesta.grid_vh)
      filename = parse_extension_set(import_fullpath(import), "VH");
    break;
  case MS_SIESTA_VT:
    if (!model->siesta.grid_vt)
      filename = parse_extension_set(import_fullpath(import), "VT");
    break;
  }

/* if filename set - attempt to read and return status */
if (filename)
  return(siesta_density_read(filename, model));

/* no read necessary - must be ok */
return(TRUE);
}

/******************************/
/* determine atom/grid limits */
/******************************/
void ms_grid_setup(gdouble *min, gdouble *max, gint *grid, struct model_pak *model)
{
gint i;
gdouble z, x[3];

cor_calc_xlimits(min, max, model->cores);

/* NEW - save for the internally generated surface exception */
x[0] = min[2];
x[1] = max[2];

/* enforce cartesian values for grid partitioning */
vecmat(model->latmat, min);
vecmat(model->latmat, max);

/* cartesian setup */
for (i=model->periodic ; i<3 ; i++)
  {
/* buffer - TODO - factor in prad/max vdw radius */
  min[i] -= 5.0;
  max[i] += 5.0;
  }

/* fractional setup */
for (i=model->periodic ; i-- ; )
  {
  min[i] = 0.0;
  max[i] = 1.0;
  }

/* main marching cube grid size paritioning */
for (i=model->periodic ; i<3 ; i++)
  grid[i] = 1 + (max[i] - min[i]) / sysenv.render.ms_grid_size;

for (i=model->periodic ; i-- ; )
  grid[i] = 1 + model->pbc[i] / sysenv.render.ms_grid_size;

for (i=3 ; i-- ; )
  if (grid[i] < 2)
    grid[i] = 2;

/* NEW - special case for surfaces generated in gdis (fully fractional) */
/* FIXME - why not use the dspacing??? (investigate if problems - seems ok at the moment) */
z = VEC3MAG(model->surface.depth_vec);
if (z > FRACTION_TOLERANCE)
  {
  min[2] = x[0] - 5.0/z;
  max[2] =  x[1] + 5.0/z;
  }
}

/**********************************/
/* cube partitioning surface calc */
/**********************************/
#define DEBUG_MS_CUBE 0
void ms_cube(gdouble value, gint method, gint colour, struct model_pak *model)
{
gchar *text;
gint i, j, k, index, eval, grid[3] = {1.0, 1.0, 1.0};
gint jjmTmp;
gdouble isolevel=0.0, dx, dy, dz;
gdouble x[3], min[3], max[3], latmat[9];
GSList *plist, *tlist, *list;
struct smv_pak **p;
int errorStat;
time_t jobStartTime, cubeStartTime;

/* checks */
g_assert(model != NULL);

/* setup */
ms_data = model;
ms_points = NULL;
ms_dsize = value;

cubeStartTime = time(NULL);

switch (method)
  {
  case MS_SIESTA_RHO:
  case MS_SIESTA_DRHO:
  case MS_SIESTA_LDOS:
  case MS_SIESTA_VH:
  case MS_SIESTA_VT:
    if (!ms_siesta_check(method, model))
      return;

    isolevel = value;
/* need to transpose? - NB: this is a fortran matrix */
    memcpy(latmat, model->siesta.cell, 9*sizeof(gdouble));

/* compensate if coords in au */
    for (i=9 ; i-- ; )
      latmat[i] *= model->siesta.eden_scale;

/* CURRENT - skip the (slow) ms_grad() call */
    ms_calc_grad = TRUE;

/* setup marching cube grid (== siesta grid??) */
/* FIXME - what about non orthogonal cells??? */
/* FIXME - need to distinguish fractional/cartesian */
/*
    VEC3SET(min, 0.0, 0.0, 0.0);
    VEC3SET(max, model->siesta.cell[0], model->siesta.cell[4], model->siesta.cell[8]);
    VEC3SET(max, 1.0, 1.0, 1.0);
*/
/* CHECK - wierd SIESTA grid */
if (model->periodic)
  {
    VEC3SET(min, 0.0, 0.0, 0.0);
    VEC3SET(max, 1.0, 1.0, 1.0);
  }
else
  {
    VEC3SET(min, -0.5, -0.5, -0.5);
    VEC3SET(max, 0.5, 0.5, 0.5);
  }

    ARR3SET(grid, model->siesta.grid);
    break;


  case MS_GAUSS_CUBE:
    if (!model->gauss_cube.voxels )
      {
      gui_text_show(ERROR, "Missing Gaussian cube file data.\n");
      return;
      }

    isolevel = value;

    struct gauss_cube_pak g = model->gauss_cube;
    for (i=0; i<3; i++) 
      {
      /* compensate for +1 in grid[i] */
      min[i] = g.r0[i] * AU2ANG;
      max[i] = (g.r0[i] + (abs(g.na)-1) * g.da[i] + (abs(g.nb)-1) * g.db[i] + (abs(g.nc)-1) * g.dc[i]) * AU2ANG;
      }
    grid[0] = abs(g.na); grid[1] = abs(g.nb); grid[2] = abs(g.nc);
    break;


  case MS_HIRSHFELD:
    if (model->selection == NULL)
      {
      gui_text_show(ERROR, "You must select some atoms in order to produce a Hirshfeld surface.");
      return;
      }	
    hfs_init();
    isolevel = 0.5;
    jjmTmp = 0;
    hsEnvironment = hfs_calc_env(model->selection,model);
#if DEBUG_MS_CUBE    
    fprintf(stderr,"Total atoms: %d\n",model->num_atoms);
    fprintf(stderr,"Selection atoms:\n");
    print_core_list_cart(model->selection);
#endif
    ms_grid_setup(min, max, grid, model);
    break;


  case MS_SSATOMS:
    if (model->selection == NULL)
      {
      gui_text_show(ERROR, "You must select some atoms in order to produce a promolecule surface.");
      return;
      }
    hfs_init();
    isolevel = value;

/* TODO - put this in the Help popup */
/*
    text = g_strdup_printf("For information on the promolecule electron density isosurface, see: Mitchell, A.S., Spackman, M.A. J. Comput. Chem., 2000, v.21, 933-942\n");
    gui_text_show(NORMAL, text);
    g_free(text);
*/

    ms_grid_setup(min, max, grid, model);
    break;

  default:
    ms_grid_setup(min, max, grid, model);
  }

if (method == MS_HIRSHFELD || method == MS_SSATOMS)
  errorStat = hfs_calulation_limits(model->selection,model,min,max);

#if DEBUG_MS_CUBE
P3VEC(" min: ", min);
P3VEC(" max: ", max);
fprintf(stderr,"grid: %d %d %d\n", grid[0], grid[1], grid[2]);
#endif

dx = (max[0] - min[0]) / (grid[0]-1);
dy = (max[1] - min[1]) / (grid[1]-1);
dz = (max[2] - min[2]) / (grid[2]-1);

jobStartTime = time(NULL);
/* create grid points */
p = g_malloc(grid[0]*grid[1]*grid[2]*sizeof(struct smv_pak *));
for (i=grid[0] ; i-- ; )
  {
  for (j=grid[1] ; j-- ; )
    {
    eval = TRUE;
    for (k=grid[2] ; k-- ; )
      {
      x[0] = min[0] + i*dx;
      x[1] = min[1] + j*dy;
      x[2] = min[2] + k*dz;

/* CURRENT - cartesian/periodic/seista grid crap */
/*
      vecmat(model->latmat, x);
*/

      if (method != MS_GAUSS_CUBE) 
        vecmat(model->latmat, x);

      index = ms_index(i, j, k, grid);
      p[index] = new_point(x);
      p[index]->index = index;

      list = model->cores;

/* evaluation */
      if (list)
        {
        if (eval)
          {
          switch (method)
            {
            case MS_SIESTA_RHO:
            case MS_SIESTA_DRHO:
            case MS_SIESTA_LDOS:
            case MS_SIESTA_VH:
            case MS_SIESTA_VT:
              p[index]->value = ms_siesta_grid(x, method, model);
              break;

            case MS_HIRSHFELD:
              p[index]->value = hfs_calc_wf(x[0], x[1], x[2], model, hsEnvironment);
              break;

  	    case MS_SSATOMS:
              p[index]->value = ssatoms_calc_density(x[0], x[1], x[2], model);
              break;

            case MS_GAUSS_CUBE:
              p[index]->value = ms_gauss_cube_value(i, j, k, model);
              break;

            default:
              p[index]->value = ms_dist(x, list);

/*
printf("[%f] ", p[index]->value);
P3VEC("x : ", x);
*/

            }
          }
        else
          p[index]->value = -1000.0;

/* cleanup */
        }
      else
        p[index]->value = 0.0;

/* surface exception - stop evaluating after first contact */
      if (model->periodic == 2)
        if (p[index]->value < 0.0)
          eval = FALSE;
      }
    }
  }
//jobEndTime = time(NULL);
#if DEBUG_MS_CUBE
fprintf(stderr," [%.1fs]\n",(float)(time(NULL) - jobStartTime));
fprintf(stderr,"polygonizing [isolevel = %f] ...", isolevel);
jobStartTime = time(NULL);
#endif

/* polygonize each cube */
tlist = NULL;
for (i=grid[0]-1 ; i-- ; )
  {
  for (j=grid[1]-1 ; j-- ; )
    {
    for (k=grid[2]-1 ; k-- ; )
      {
      plist = NULL;

      index = ms_index(i, j, k, grid);
      plist = g_slist_prepend(plist, p[index]);
      index = ms_index(i+1, j, k, grid);
      plist = g_slist_prepend(plist, p[index]);
      index = ms_index(i+1, j+1, k, grid);
      plist = g_slist_prepend(plist, p[index]);
      index = ms_index(i, j+1, k, grid);
      plist = g_slist_prepend(plist, p[index]);
      index = ms_index(i, j, k+1, grid);
      plist = g_slist_prepend(plist, p[index]);
      index = ms_index(i+1, j, k+1, grid);
      plist = g_slist_prepend(plist, p[index]);
      index = ms_index(i+1, j+1, k+1, grid);
      plist = g_slist_prepend(plist, p[index]);
      index = ms_index(i, j+1, k+1, grid);
      plist = g_slist_prepend(plist, p[index]);

      plist = g_slist_reverse(plist);

      list = ms_triangulate(isolevel, plist);

      if (list)
        tlist = g_slist_concat(tlist, list);

      g_slist_free(plist);
      }
    }
  }


#if DEBUG_MS_CUBE
fprintf(stderr," [%.1fs]\n",(float)(time(NULL) - jobStartTime));
fprintf(stderr,"processing...\n");
#endif

/* process the point list (remove points not part of the molsurf) */
/* TODO - enhancement phase (point merging/divide triangles etc.) */
VEC3SET(x, dx, dy, dz);
vecmat(model->latmat, x);

// CURRENT
//tlist = ms_process(VEC3MAG(x), tlist);

if (method == MS_HIRSHFELD)
  {
#if DEBUG_MS_CUBE
  fprintf(stderr,"computing Hirshfeld surface normals...");
#endif
  errorStat = hfs_calc_normals(ms_points, model, hsEnvironment);
  }
if (method == MS_SSATOMS)
{
#if DEBUG_MS_CUBE
	fprintf(stderr,"computing surface normals...");
#endif
	errorStat = ssatoms_calc_normals(ms_points, model);
} 


#if DEBUG_MS_CUBE
fprintf(stderr,"colouring...");
#endif

/* NEW - process colouring scheme for each method */
model->ms_colour_scale = FALSE;

ms_colour_surface(method, colour, ms_points, model);

#if DEBUG_MS_CUBE
fprintf(stderr,"exporting...\n");
#endif

/* TODO - fix re-triangulation */
/*
ms_adjust_triangles(tlist, model);
*/

/* create the display triangles */
ms_create_triangles(tlist, model, method);


P3VEC("centroid: ", model->centroid);


text = g_strdup_printf("Iso-surface = %f, Area = %f, Volume = %f\n",
                       value, molsurf_area(tlist), molsurf_volume(tlist));
gui_text_show(STANDARD, text);
g_free(text);

/* TODO - free point and triangle data */
g_slist_free(ms_points);
g_slist_free(tlist);

g_free(p);

#if DEBUG_MS_CUBE
printf("done. [%.1fs]\n",(float)(time(NULL) - cubeStartTime));
#endif
}

