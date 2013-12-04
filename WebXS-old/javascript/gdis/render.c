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
#include <string.h>
#include <math.h>

#include "gdis.h"
#include "coords.h"
#include "matrix.h"
#include "opengl.h"
#include "render.h"
#include "interface.h"

/* externals */
extern struct sysenv_pak sysenv;
extern GtkWidget *window;
extern GdkPixmap *pixmap;

/********************/
/* construct a pipe */
/********************/
#define COLOUR_NORMALIZE 0.000015259
gpointer render_pipe_new(struct core_pak *core, gdouble *offset)
{
struct pipe_pak *pipe;

/* init pipe */
pipe = g_malloc(sizeof(struct pipe_pak));

pipe->core = core;
ARR3SET(pipe->offset, offset);

ARR4SET(pipe->colour, core->colour);
VEC3MUL(pipe->colour, COLOUR_NORMALIZE);

return(pipe);
}

/***************************************************/
/* update the pipe colours associated with an atom */
/***************************************************/
// NB: assumes atom core colour is correct
void render_pipe_colour_refresh(gpointer data, struct model_pak *model)
{
gint i;
GSList *list;
struct pipe_pak *pipe;
struct core_pak *core=data;

if (!model)
  return;

for (i=0 ; i<RENDER_LAST ; i++)
  {
  for (list=model->pipe_list[i] ; list ; list=g_slist_next(list))
    {
    pipe = list->data;

    if (pipe->core == core)
      {
      ARR4SET(pipe->colour, core->colour);
      VEC3MUL(pipe->colour, COLOUR_NORMALIZE);
      }
    }
  }
}

/*********************************************/
/* construct all pipe lists for bond drawing */
/*********************************************/
/* TODO - store pipes in model and only redo when needed (ie render mode change / bond calc) */
void render_make_pipes(GSList **pipes, struct model_pak *model)
{
gint i, show1, show2;
gdouble radius;
gdouble v12[3], v21[3];
GSList *list;
struct pipe_pak *pipe;
struct bond_pak *bond;
struct core_pak *core1, *core2;

/* checks */
g_assert(model != NULL);

/* init the return list(s) */
for (i=0 ; i<RENDER_LAST ; i++)
  pipes[i] = NULL;

/* bond display turned off? */
if (!model->show_bonds)
  return;

/* common pipe width */
radius = sysenv.render.stick_rad;

/* enumerate bonds to construct the pipe list */
for (list=model->bonds; list ; list=g_slist_next(list))
  {
  bond = list->data;
  switch (bond->status)
    {
    case HIDDEN:
    case DELETED:
      continue;
    }

/* the two atoms */
  core1 = bond->atom1;
  core2 = bond->atom2;

/* visibility tests */
  show1 = show2 = TRUE;
  switch (core1->render_mode)
    {
    case CPK:
    case ZONE:
      show1 = FALSE;
    }
  switch(core2->render_mode)
    {
    case CPK:
    case ZONE:
      show2 = FALSE;
    }

  if (core1->status & (DELETED | HIDDEN | ZEOL_HIDDEN))
    show1 = FALSE;
  if (core2->status & (DELETED | HIDDEN | ZEOL_HIDDEN))
    show2 = FALSE;

/* offset (atom -> midpoint) calculation */
   ARR3SET(v12, bond->offset);
   vecmat(model->latmat, v12);
   ARR3SET(v21, v12);
   VEC3MUL(v21, -1.0);

/* colour setup */
/* FIXME - colour change if bond->type == BOND_HBOND */
/*
      VEC4SET(colour1, 1.0, 1.0, 0.6, 0.0);
      VEC4SET(colour2, 1.0, 1.0, 0.6, 0.0);
*/

/* setup half-bond (pipe) for core1 */
  if (show1)
    {
    pipe = render_pipe_new(core1, v12);

/* assign to appropriate pipe list */
    if (core1->render_mode == STICK)
      pipes[3] = g_slist_prepend(pipes[3], pipe);
    else
      {
      if (core1->ghost)
        pipes[1] = g_slist_prepend(pipes[1], pipe);
      else
        {
        if (core1->render_wire)
          pipes[2] = g_slist_prepend(pipes[2], pipe);
        else
          pipes[0] = g_slist_prepend(pipes[0], pipe);
        }
      }
    }

/* setup half-bond (pipe) for core1 */
  if (show2)
    {
    pipe = render_pipe_new(core2, v21);

/* assign to appropriate pipe list */
    if (core2->render_mode == STICK)
      pipes[3] = g_slist_prepend(pipes[3], pipe);
    else
      {
      if (core2->ghost)
        pipes[1] = g_slist_prepend(pipes[1], pipe);
      else
        {
        if (core2->render_wire)
          pipes[2] = g_slist_prepend(pipes[2], pipe);
        else
          pipes[0] = g_slist_prepend(pipes[0], pipe);
        }
      }
    }
  }
}

/*******************************************/
/* recalculate the bond drawing primitives */
/*******************************************/
void render_pipes_setup(struct model_pak *model)
{
gint i;

g_assert(model != NULL);

for (i=0 ; i<RENDER_LAST ; i++)
  {
  free_slist(model->pipe_list[i]);
  model->pipe_list[i] = NULL;
  }

render_make_pipes(model->pipe_list, model);

/* DEBUG */
/*
for (i=0 ; i<RENDER_LAST ; i++)
  {
  printf("# pipes [%d] : %d\n", i, g_slist_length(model->pipe_list[i]));
  }
*/
}

/********************************************/
/* build lists for respective drawing types */
/********************************************/
/* TODO - this could be done at the same time render_pipe_init is done ie pre-calc render primitives */
#define DEBUG_BUILD_CORE_LISTS 0
void render_atoms_build(struct model_pak *model)
{
GSList *list;
struct core_pak *core;

g_assert(model != NULL);

for (list=model->cores ; list ; list=g_slist_next(list))
  {
  core = list->data;

/* bailout checks */
  if (core->status & (HIDDEN | DELETED))
    continue;

/*
  if (core->render_mode == ZONE)
    continue;
*/
/* build appropriate lists */
/*
  if (core->render_wire)
    model->atom_list[RENDER_WIRE] = g_slist_prepend(model->atom_list[RENDER_WIRE], core);
  else if (core->ghost)
    model->atom_list[RENDER_GHOST] = g_slist_prepend(model->atom_list[RENDER_GHOST], core);
  else
    model->atom_list[RENDER_SOLID] = g_slist_prepend(model->atom_list[RENDER_SOLID], core);
*/

  if (core->ghost)
    {
    model->atom_list[RENDER_GHOST] = g_slist_prepend(model->atom_list[RENDER_GHOST], core);
    }
  else
    {
    switch (core->render_mode)
      {
      case ZONE:
        break;

      case STICK:
        if (!core->bonds)
          model->atom_list[RENDER_LINE] = g_slist_prepend(model->atom_list[RENDER_LINE], core);
        break;

      default:
        model->atom_list[RENDER_SOLID] = g_slist_prepend(model->atom_list[RENDER_SOLID], core);
      }
    }
  }
}

/*******************************************/
/* recalculate the atom drawing primitives */
/*******************************************/
void render_atoms_setup(struct model_pak *model)
{
gint i;

g_assert(model != NULL);

for (i=0 ; i<RENDER_LAST ; i++)
  {
  g_slist_free(model->atom_list[i]);
  model->atom_list[i] = NULL;
  }

render_atoms_build(model);

/* DEBUG */
/*
for (i=0 ; i<RENDER_LAST ; i++)
  {
  printf("# atoms [%d] : %d\n", i, g_slist_length(model->atom_list[i]));
  }
*/
}

/**********************************/
/* refresh all display primitives */
/**********************************/
void render_refresh(struct model_pak *model)
{
render_atoms_setup(model);
render_pipes_setup(model);
}

/*****************************/
/* pipe z-ordering primitive */ 
/*****************************/
gint render_pipe_depth_sort(struct pipe_pak *p1, struct pipe_pak *p2)
{
gdouble z1, z2;

/* TODO */
/*
z1 = 0.5 * (p1->v1[2] + p1->v2[2]);
z2 = 0.5 * (p2->v1[2] + p2->v2[2]);

if (z1 > z2)
  return(-1);
*/

return(1);
}

GSList *render_sort_pipes(GSList *pipes)
{
return(g_slist_sort(pipes, (gpointer) render_pipe_depth_sort));
}

/*************************/
/* foreground rendering  */
/*************************/
/* NEW - due to povrays annoying i/o restrictions we need to change directory */
/* to where the .pov file is to makes sure that's where the .tga ends up */
void povray_exec(const gchar *cwd, const gchar *filename)
{
gint i, status;
gchar *err, *out, **argv;
GError *error=NULL;

g_assert(filename != NULL);

argv = g_malloc(15 * sizeof(gchar *));
i=0;
*(argv+i++) = g_strdup(sysenv.povray_path);
*(argv+i++) = g_strdup("-D");
*(argv+i++) = g_strdup_printf("+I%s", filename);
*(argv+i++) = g_strdup("-GA");
*(argv+i++) = g_strdup("-P");
*(argv+i++) = g_strdup_printf("+W%d", (gint) sysenv.render.width);
*(argv+i++) = g_strdup_printf("+H%d", (gint) sysenv.render.height);
*(argv+i++) = g_strdup("+FN");
if (sysenv.render.antialias)
  {
  *(argv+i++) = g_strdup("+A");
  *(argv+i++) = g_strdup("+AM2");
  }

#ifdef __WIN32
{
gchar *path = g_path_get_dirname(sysenv.povray_path);
*(argv+i++) = g_strdup_printf("-L%s", path);
g_free(path);
}
#endif

*(argv+i++) = NULL;

if (g_spawn_sync(cwd, argv, NULL, 0, NULL, NULL, &out, &err, &status, &error))
  {
/*
  printf("povray_exec() success\n");
  printf("out: %s\n", out);
  printf("err: %s\n", err);
*/
  }
else
  {
  printf("povray_exec() error: %s\n", error->message);
  }

g_strfreev(argv);
}

/*************************/
/* background rendering  */
/*************************/
/* NEW - due to povrays annoying i/o restrictions we need to change directory */
/* to where the .pov file is to makes sure that's where the .tga ends up */
gint povray_exec_async(const gchar *cwd, const gchar *filename)
{
gint i, pid, status;
gchar *err, *out, **argv;
GError *error=NULL;

argv = g_malloc(15 * sizeof(gchar *));
i=0;
*(argv+i++) = g_strdup(sysenv.povray_path);
*(argv+i++) = g_strdup("-D");
*(argv+i++) = g_strdup_printf("+I%s", filename);
*(argv+i++) = g_strdup("-GA");
*(argv+i++) = g_strdup("-P");
*(argv+i++) = g_strdup_printf("+W%d", (gint) sysenv.render.width);
*(argv+i++) = g_strdup_printf("+H%d", (gint) sysenv.render.height);
*(argv+i++) = g_strdup("+FN");
if (sysenv.render.antialias)
  {
  *(argv+i++) = g_strdup("+A");
  *(argv+i++) = g_strdup("+AM2");
  }

#ifdef __WIN32
{
gchar *path = g_path_get_dirname(sysenv.povray_path);
*(argv+i++) = g_strdup_printf("-L%s", path);
g_free(path);
}
#endif

*(argv+i++) = NULL;

if (g_spawn_async(cwd, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, &error))
  {
  printf("povray_exec_async() success\n");
  printf("pid: %d\n", pid);
  }
else
  {
  printf("povray_exec() error: %s\n", error->message);
  }

g_strfreev(argv);

return(pid);
}

