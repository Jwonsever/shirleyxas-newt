/*
Copyright (C) 2010 by Sean David Fleming

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
#include <string.h>
#include <math.h>

#include "gdis.h"
#include "matrix.h"
#include "numeric.h"
#include "render.h"
#include "opengl.h"
#include "quaternion.h"

// mesh pak
struct va_pak
{
gint size;
gdouble *v;   // base vertices (also serve as normals, since radius=1.0)
gdouble *r;   // buffer for translation/scaling (ie real display coords)
};

/**************************/
/* free vertex array data */
/**************************/
void va_free(void *data)
{
struct va_pak *va=data;

free(va->v);
free(va->r);
free(va);
}

/**********************************/
/* allocate and fill out our mesh */
/**********************************/
// dt - quality of the sphere
void *va_init_sphere(gdouble dt, struct camera_pak *camera)
{
gint i, n, m;
gdouble a, b, r;
struct va_pak *va;

//dt=0.3;
/* calculate upper limit on array size required */
r = PI/dt;
m = r;
m++;
m *= 24.0*m;

//printf("pred size = %d\n", m);

va = malloc(sizeof(struct va_pak));
va->v = malloc(m * sizeof(gdouble));
va->r = malloc(m * sizeof(gdouble));

/* FIXME - quat_rotate + vertex enumeration not *quite* correct yet */
/* TODO - cut down on angle sweep */
i = n = 0;
for (b=0.0 ; b<PI ; b+=dt)
  {
  for (a=0.0 ; a<2.0*PI ; a+=dt)
    {
    va->v[i++] = sin(a) * sin(b);
    va->v[i++] = cos(a) * sin(b);
    va->v[i++] = cos(b);
quat_rotate(&va->v[i-3], camera->q);
    n++;
    va->v[i++] = sin(a) * sin(b + dt);
    va->v[i++] = cos(a) * sin(b + dt);
    va->v[i++] = cos(b + dt);
quat_rotate(&va->v[i-3], camera->q);
    n++;
    *(va->v+i++) = sin(a + dt) * sin(b);
    *(va->v+i++) = cos(a + dt) * sin(b);
    *(va->v+i++) = cos(b);
quat_rotate(&va->v[i-3], camera->q);
    n++;
    *(va->v+i++) = sin(a + dt) * sin(b + dt);
    *(va->v+i++) = cos(a + dt) * sin(b + dt);
    *(va->v+i++) = cos(b + dt);
quat_rotate(&va->v[i-3], camera->q);
    n++;
    }
  }

// record actual size
va->size = n;

//printf("n = %d (i = %d)\n", n, i);

return(va);
}

/*********************************/
/* sphere vertex array primitive */
/*********************************/
void va_draw_hemisphere(void *data, gdouble *x, gdouble rad)
{
gint i;
struct va_pak *va = data;

memcpy(va->r, va->v, 3*va->size*sizeof(gdouble));
for (i=va->size ; i-- ; )
  {
// apply rotation
  va->r[3*i+0] *= rad;
  va->r[3*i+1] *= rad;
  va->r[3*i+2] *= rad;
// apply translation
  va->r[3*i+0] += x[0];
  va->r[3*i+1] += x[1];
  va->r[3*i+2] += x[2];
  }

/* draw */
glEnableClientState(GL_VERTEX_ARRAY);
glEnableClientState(GL_NORMAL_ARRAY);

glVertexPointer(3, GL_DOUBLE, 0, va->r);

glNormalPointer(GL_DOUBLE, 0, va->v);

glDrawArrays(GL_TRIANGLE_STRIP, 0, va->size);

glDisableClientState(GL_NORMAL_ARRAY);
glDisableClientState(GL_VERTEX_ARRAY);
}

/**************************************/
/* allocate for cylinder vertex array */
/**************************************/
void *va_init_cylinder(gint segments)
{
struct va_pak *mesh;

if (segments < 1)
  return(NULL);

mesh = g_malloc(sizeof(struct va_pak));

mesh->size = 2 * (segments+1);
mesh->v = malloc(3 * mesh->size * sizeof(gdouble));
mesh->r = malloc(3 * mesh->size * sizeof(gdouble));

return(mesh);
}

/***********************************/
/* cylinder vertex array primitive */
/***********************************/
void va_draw_cylinder(void *data, gdouble *va, gdouble *vb, gdouble rad)
{
gint i, n, segments;
gdouble theta, st, ct;
gdouble v1[3], v2[3], v[3], v12[3], p[3], q[3];
struct va_pak *mesh=data;

if (!mesh)
  return;

segments = mesh->size/2 - 1;

/* force ordering of v1 & v2 to get correct quad normals */
if (va[2] > vb[2])
  {
  ARR3SET(v1, vb);
  ARR3SET(v2, va);
  }
else
  {
  ARR3SET(v1, va);
  ARR3SET(v2, vb);
  }

/* vector from v1 to v2 */
ARR3SET(v12, v2);
ARR3SUB(v12, v1);

/* make a guess at a vector with some orthogonal component to v12 */
VEC3SET(p, 1.0, 1.0, 1.0);
crossprod(q, p, v12);

/* our guess was bad - so fix it */
if (VEC3MAGSQ(q) < FRACTION_TOLERANCE)
  {
  VEC3SET(p, 0.0, 1.0, 0.0);
  crossprod(q, p, v12);
  }
crossprod(p, v12, q);

/* p and q are orthonormal and in the plane of the cylinder's cross section */
normalize(p, 3);
normalize(q, 3);

/* sweep over desired cylinder segments */
n=0;
for (i=0 ; i<segments+1 ; i++)
  {
/* compute unit vector from centre to surface */
  theta = (gdouble) i * 2.0 * PI / (gdouble) segments;
  st = tbl_sin(theta);
  ct = tbl_cos(theta);
  v12[0] = ct * p[0] + st * q[0];
  v12[1] = ct * p[1] + st * q[1];
  v12[2] = ct * p[2] + st * q[2];

/* set next two normals */
  ARR3SET(&mesh->v[n], v12);
  ARR3SET(&mesh->v[n+3], v12);

/* set next two vertices */
  VEC3MUL(v12, rad);
  ARR3SET(v, v2);
  ARR3ADD(v, v12);
  ARR3SET(&mesh->r[n], v);
  ARR3SET(v, v1);
  ARR3ADD(v, v12);
  ARR3SET(&mesh->r[n+3], v);

  n += 6;
  }

/* draw */
glEnableClientState(GL_VERTEX_ARRAY);
glEnableClientState(GL_NORMAL_ARRAY);

glVertexPointer(3, GL_DOUBLE, 0, mesh->r);

glNormalPointer(GL_DOUBLE, 0, mesh->v);

glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh->size);

glDisableClientState(GL_NORMAL_ARRAY);
glDisableClientState(GL_VERTEX_ARRAY);
}

/************************************/
/* unit cell vertex array primitive */
/************************************/
void va_draw_cell(struct model_pak *model)
{
gint i, j, n;
struct va_pak mesh;

mesh.size = 24;
mesh.v = malloc(3 * mesh.size * sizeof(gdouble));

/* enumerate the opposite ends of the frame */
n=0;
for (i=0 ; i<5 ; i+=4)
  {
  ARR3SET(&mesh.v[3*n], model->cell[i].rx);
  n++;
  ARR3SET(&mesh.v[3*n], model->cell[i+1].rx);
  n++;
  ARR3SET(&mesh.v[3*n], model->cell[i+1].rx);
  n++;
  ARR3SET(&mesh.v[3*n], model->cell[i+2].rx);
  n++;
  ARR3SET(&mesh.v[3*n], model->cell[i+2].rx);
  n++;
  ARR3SET(&mesh.v[3*n], model->cell[i+3].rx);
  n++;
  ARR3SET(&mesh.v[3*n], model->cell[i+3].rx);
  n++;
  ARR3SET(&mesh.v[3*n], model->cell[i].rx);
  n++;
  }

/* enumerate the sides of the frame */
for (i=4 ; i-- ; )
  {
  j = i+4;

  ARR3SET(&mesh.v[3*n], model->cell[i].rx);
  n++;
  ARR3SET(&mesh.v[3*n], model->cell[j].rx);
  n++;
  }

/* draw */
glEnableClientState(GL_VERTEX_ARRAY);

glVertexPointer(3, GL_DOUBLE, 0, mesh.v);

glDrawArrays(GL_LINES, 0, mesh.size);

glDisableClientState(GL_VERTEX_ARRAY);

free(mesh.v);
}

