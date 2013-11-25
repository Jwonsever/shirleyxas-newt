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
#include "matrix.h"
#include "numeric.h"

/* main structures */
extern struct sysenv_pak sysenv;

/* intended for storing/processing scalar fields */
/* where information is entered as a regular lattice */
/* and can be extracted (interpolated) at any point */
struct scalar_field_pak
{
gchar *id;

/* grid lattice vectors */
gdouble lattice[9];
gdouble ilattice[9];

/* number of lattice points for each lattice vector */
gint size[3];

/* physical dimensions */
gdouble start[3];
gdouble stop[3];
/* min/max scalar values */
gdouble min;
gdouble max;

/* array of scalars */
gdouble *values;
};

/**********************/
/* creation primitive */
/**********************/
gpointer scalar_field_new(const gchar *name)
{
struct scalar_field_pak *scalar;

scalar = g_malloc(sizeof(struct scalar_field_pak));

/* CURRENT - why is setting these causing a core dump? */
matrix_identity(scalar->lattice);
matrix_identity(scalar->ilattice);

scalar->id = g_strdup(name);

VEC3SET(scalar->size, 1, 1, 1);
VEC3SET(scalar->start, 0.0, 0.0, 0.0);
VEC3SET(scalar->stop, 1.0, 1.0, 1.0);

scalar->min = G_MAXDOUBLE;
scalar->max = -G_MAXDOUBLE;
scalar->values = NULL;

return(scalar);
}

/*************************/
/* destruction primitive */
/*************************/
void scalar_field_free(gpointer data)
{
struct scalar_field_pak *scalar=data;

g_free(scalar->values);
g_free(scalar);
}

/*****************************/
/* initialization primitives */
/*****************************/
void scalar_field_lattice_set(gdouble *lattice, gpointer data)
{
struct scalar_field_pak *scalar=data;

g_assert(scalar != NULL);

memcpy(scalar->lattice, lattice, 9*sizeof(gdouble));
memcpy(scalar->ilattice, lattice, 9*sizeof(gdouble));
matrix_invert(scalar->ilattice);
}

void scalar_field_size_set(gint *size, gpointer data)
{
struct scalar_field_pak *scalar=data;

g_assert(scalar != NULL);

memcpy(scalar->size, size, 3*sizeof(gint));
}

void scalar_field_start_set(gdouble *x, gpointer data)
{
struct scalar_field_pak *scalar=data;

g_assert(scalar != NULL);

memcpy(scalar->start, x, 3*sizeof(gdouble));
}

/* TODO - other data input primitives (eg at individual lattice points) */
void scalar_field_values_set(gdouble *values, gpointer data)
{
struct scalar_field_pak *scalar=data;

g_assert(scalar != NULL);

/* NB: not a copy - straight storage */
scalar->values = values;
}

/************************/
/* processing primitive */
/************************/
void scalar_field_scan(gpointer data)
{
gint i, total;
gdouble v;
struct scalar_field_pak *scalar=data;

if (!scalar->values)
  {
  printf("scalar_field_scan(): error, empty scalar field values.\n");
  return;
  }

P3MAT("Scalar lattice: ", scalar->lattice);
P3MAT("Inverse: ", scalar->ilattice);
printf("Lattice size: %d %d %d\n", scalar->size[0], scalar->size[1], scalar->size[2]);

/* TODO - set min/max & extents */
total = scalar->size[0]*scalar->size[1]*scalar->size[2];

for (i=total ; i-- ; )
  {
  v = scalar->values[i];

  if (v > scalar->max)
    scalar->max = v;
  if (v < scalar->min)
    scalar->min = v;
  }

printf("Scalar min: %f\n", scalar->min);
printf("Scalar max: %f\n", scalar->max);

/* physical size */
VEC3SET(scalar->stop, scalar->size[0], scalar->size[1], scalar->size[2]);
vecmat(scalar->lattice, scalar->stop);
ARR3ADD(scalar->stop, scalar->start);

P3VEC("origin: ", scalar->start);
P3VEC("extent: ", scalar->stop);
}

/***********************/
/* retrieval primitive */
/***********************/
gdouble scalar_field_min(gpointer data)
{
struct scalar_field_pak *scalar=data;
return(scalar->min);
}

gdouble scalar_field_max(gpointer data)
{
struct scalar_field_pak *scalar=data;
return(scalar->max);
}

/*************************************************************/
/* primitive for converting latice coordinates into an index */
/*************************************************************/
gint scalar_field_index(gint i, gint j, gint k, gpointer data)
{
struct scalar_field_pak *scalar=data;

g_assert(scalar != NULL);

/* FIXME - loop indexing here was chosen to follow gaussian */
/* which is z innermost & x outermost */
/* testing - x inner -> z - outer */
//i = c*scalar->size[1]*scalar->size[0] + b*scalar->size[0] + a;

return (i*scalar->size[1]*scalar->size[2] + j*scalar->size[2] + k);
}

/***********************************************************/
/* primitive for extracting data at any point in the field */
/***********************************************************/
#define DEBUG_SCALAR_FIELD_VALUE 0
gdouble scalar_field_value(gdouble *x, gpointer data)
{
gint a, b, c, h, i, j, k;
gdouble offset[3], v[3], v0;
struct scalar_field_pak *scalar=data;

g_assert(scalar != NULL);

#if DEBUG_SCALAR_FIELD_VALUE 
P3VEC("scalar x: ", x);
#endif

/* remove origin & convert to lattice point */
ARR3SET(v, x);
ARR3SUB(v, scalar->start);
vecmat(scalar->ilattice, v);

#if DEBUG_SCALAR_FIELD_VALUE 
P3VEC("scalar v: ", v);
printf("lattice v: %d %d %d\n", nearest_int(v[0]), nearest_int(v[1]), nearest_int(v[2]));
#endif

/* clamp to input data range to avoid out of bounds */
/* clamp to lowest lattice point */
a = CLAMP(v[0], 0, scalar->size[0]-1);
b = CLAMP(v[1], 0, scalar->size[1]-1);
c = CLAMP(v[2], 0, scalar->size[2]-1);

/* offset vector from the lowest lattice point */
offset[0] = v[0] - a;
offset[1] = v[1] - b;
offset[2] = v[2] - c;

/* FIXME - convert/scale offset to fit in lattice/grid size ??? */

/* TODO - precalculate the grads for all grid points? */
h =  scalar_field_index(a, b, c, scalar);
v0 =  scalar->values[h];

#if DEBUG_SCALAR_FIELD_VALUE 
printf("lattice v = %f\n", v0);
#endif

i =  scalar_field_index(a+1, b, c, scalar);
v[0] = scalar->values[i] - v0;

#if DEBUG_SCALAR_FIELD_VALUE 
printf("lattice i = %f\n", scalar->values[i]);
#endif

j =  scalar_field_index(a, b+1, c, scalar);
v[1] = scalar->values[j] - v0;

#if DEBUG_SCALAR_FIELD_VALUE 
printf("lattice j = %f\n", scalar->values[j]);
#endif

k =  scalar_field_index(a, b, c+1, scalar);
v[2] = scalar->values[k] - v0;

#if DEBUG_SCALAR_FIELD_VALUE 
printf("lattice k = %f\n", scalar->values[k]);
#endif

/* dot product grad with offset vector (linear interpolate) */
ARR3MUL(v, offset);
v0 += v[0]+v[1]+v[2];

#if DEBUG_SCALAR_FIELD_VALUE 
printf("interpol v = %f\n", v0);
#endif

return(v0);
}

