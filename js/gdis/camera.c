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
#include <string.h>
#include <math.h>

#include "gdis.h"
#include "matrix.h"
#include "render.h"
#include "opengl.h"
#include "dialog.h"
#include "quaternion.h"
#include "numeric.h"
#include "interface.h"
#include "animate.h"

/* externals */
extern struct sysenv_pak sysenv;

/*****************************/
/* initialize default camera */
/*****************************/
void camera_default(gdouble r, gpointer data)
{
struct camera_pak *camera = data;

camera->mode = LOCKED;
camera->perspective = FALSE;
camera->fov = 70.0;

/* FIXME - not sure why the < 1.0 case is needed ... */
if (r < 1.0)
  camera->zoom = r;
else
  camera->zoom = 1.0;

VEC4SET(camera->q, 1.0, 0.0, 0.0, 0.0);
VEC3SET(camera->x, 0.0, -2.0*r, 0.0);
VEC3SET(camera->o, 0.0, 0.0, 1.0);
VEC3SET(camera->v, 0.0, 1.0, 0.0);
VEC3SET(camera->e, 1.0, 0.0, 0.0);
}

/**************************/
/* restore default camera */
/**************************/
void camera_init(struct model_pak *model)
{
g_assert(model != NULL);

if (!model->camera_default)
  model->camera_default = g_malloc(sizeof(struct camera_pak));

model->camera = model->camera_default;

camera_default(model->rmax, model->camera);
}

/*************/
/* debugging */
/*************/
void camera_dump(gpointer data)
{
struct camera_pak *camera = data;

printf("camera [%p] zoom = %f\n", camera, camera->zoom);
P4VEC("q: ", camera->q);
P3VEC("x: ", camera->x);
P3VEC("o: ", camera->o);
P3VEC("v: ", camera->v);
}

/***************************/
/* default camera location */
/***************************/
gpointer camera_new(void)
{
struct camera_pak *camera;

camera = g_malloc(sizeof(struct camera_pak));
camera_default(10.0, camera);
return(camera);
}

/**********************/
/* duplicate a camera */
/**********************/
gpointer camera_dup(gpointer data)
{
struct camera_pak *cam1, *cam2 = data;

cam1 = g_malloc(sizeof(struct camera_pak));

memcpy(cam1, cam2, sizeof(struct camera_pak));

return(cam1);
}

/*****************/
/* copy a camera */
/*****************/
void camera_copy(gpointer a, gpointer b)
{
struct camera_pak *cam1=a, *cam2 = b;

memcpy(cam1, cam2, sizeof(struct camera_pak));
}

/****************************************************************/
/* adjust camera position for a new model scale (ie rmax value) */
/****************************************************************/
void camera_rescale(gdouble r, gpointer data)
{
gdouble x[3];
struct camera_pak *camera = data;

ARR3SET(x, camera->x);
normalize(x, 3);
VEC3MUL(x, r);
ARR3SET(camera->x, x);
}

/**********************/
/* extract quaternion */
/**********************/
gdouble *camera_q(gpointer data)
{
struct camera_pak *camera = data;
return(camera->q);
}

/***************************************************************/
/* extract the cartesian viewing vector (for visibility tests) */
/***************************************************************/
void camera_view(gdouble *v, gpointer data)
{
struct camera_pak *camera = data;

g_assert(data != NULL);

ARR3SET(v, camera->v);
if (camera->mode == LOCKED)
  quat_rotate(v, camera->q);
}

/*****************************************************/
/* create an animation from a rotation specification */
/*****************************************************/
void camera_rotate_animate(gint axis, gdouble *angle, gint overwrite, struct model_pak *model)
{
gdouble a, radian[3];
gpointer frame;
struct camera_pak *camera;

g_assert(model != NULL);

dialog_destroy_type(ANIM);

/* TODO - angle checks */
ARR3SET(radian, angle);
VEC3MUL(radian, D2R);

if (overwrite)
  animate_model_free(model);

for (a=radian[0] ; a<radian[1]+0.5*radian[2] ; a+=radian[2])
  {
  camera = camera_dup(model->camera);
  switch (axis)
    {
    case 1:
      quat_concat_euler(camera->q, ROLL, a);
      break;

    case 2:
      quat_concat_euler(camera->q, YAW, a);
      break;

    default:
      quat_concat_euler(camera->q, PITCH, a);
    }

/* NEW */
  frame = animate_frame_new(model);
  animate_frame_camera_set(camera, frame);
  }

redraw_canvas(SINGLE);
}

/*****************************************************/
/* create an animation from the camera waypoint list */
/*****************************************************/
/* replacement for the old animation */
void camera_waypoint_animate(gint frames, gint overwrite, struct model_pak *model)
{
gdouble x, dx, dz;
gpointer frame;
GSList *list;
struct camera_pak *cam, *cam1, *cam2;

/* checks */
g_assert(model != NULL);
g_assert(frames > 0);
if (g_slist_length(model->waypoint_list) < 2)
  {
  printf("You need to add at least 2 waypoints.\n");
  return;
  }

/* setup */
list = model->waypoint_list;
cam1 = list->data;
list = g_slist_next(list);
dx = 1.0 / (gdouble) (frames);

if (overwrite)
  animate_model_free(model);

/* create the camera journey */
while (list)
  {
  cam2 = list->data;
  dz = cam2->zoom - cam1->zoom;

  for (x=0.0 ; x<=1.0 ; x+=dx)
    {
    cam = camera_dup(cam1);

/* linear interpolatation of camera quat */
    quat_nlerp(cam->q, cam1->q, cam2->q, x);

/* interpolate zoom */
   cam->zoom += x*dz;

/* add the new camera as a frame */
    frame = animate_frame_new(model);
    animate_frame_camera_set(cam, frame);
    }

/* get next journey leg */
  cam1 = cam2;
  list = g_slist_next(list);
  }

redraw_canvas(SINGLE);
}

