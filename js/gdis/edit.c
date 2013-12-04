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

#ifndef __WIN32
#include <sys/times.h>
#endif

#include "gdis.h"
#include "coords.h"
#include "edit.h"
#include "matrix.h"
#include "measure.h"
#include "opengl.h"
#include "render.h"
#include "select.h"
#include "gui_shorts.h"
#include "interface.h"
#include "undo.h"

#define DEBUG 0

extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

gint current_colour[3];

/**********************/
/* undelete primitive */
/**********************/
gint core_undelete(struct model_pak *model, GSList *list)
{
struct core_pak *core;
struct shel_pak *shell;

g_assert(model != NULL);
g_assert(list != NULL);

core = list->data;

core->status &= ~DELETED;

model->cores = g_slist_prepend(model->cores, core);
if (core->shell)
  {
  shell = core->shell;

  shell->status &= ~DELETED;
  model->shels = g_slist_prepend(model->shels, core->shell);
  }

/* FIXME - this doesnt work */
/*
connect_atom_compute(core, model);
*/

/* TODO - more fine grained molecule recalc (ie recalc of affected molecules only) */
connect_bonds(model);
connect_molecules(model);

return(TRUE);
}

/**********************************/
/* flag core and associated shell */
/**********************************/
void delete_core(struct core_pak *core)
{
core->status |= DELETED;
if (core->shell)
  (core->shell)->status |= DELETED;
}

/*********************************/
/* remove all cores from a model */
/*********************************/
/* NB: leave symmetry / pbc structure intact */
void core_delete_all(struct model_pak *model)
{
g_assert(model != NULL);

/* free core reference lists */
free_mol_list(model);
wipe_bonds(model);
g_slist_free(model->unique_atom_list);
model->unique_atom_list = NULL;

/* free cores */
free_slist(model->cores);
model->cores = NULL;

/* redo dependancies */
calc_emp(model);
gui_refresh(GUI_ACTIVE);
model->state = 0;
}

/*******************************************************/
/* permanently remove all deleted cores from all lists */
/*******************************************************/
#define DEBUG_DELETE_COMMIT 0
void delete_commit(struct model_pak *data)
{
gint flag1=FALSE, flag2=FALSE;
gpointer m;
GSList *list1, *list2;
struct core_pak *core;
struct shel_pak *shell;
struct bond_pak *bond;

g_assert(data != NULL);

/* delete flaged cores in list */
list1 = data->cores;
while (list1)
  {
  core = list1->data;
  list1 = g_slist_next(list1);

  if (core->status & DELETED)
    {
#if DEBUG_DELETE_COMMIT
printf("Deleting %s core [%p] ...\n", core->label, core);
#endif
    flag1 = TRUE;

/* flag assoc. shell */
    if (core->shell)
      {
      shell = core->shell;
      shell->status |= DELETED;
      }

/* update connectivity */
    connect_atom_clear(core, data);
    list2 = data->ubonds;
    while (list2)
      {
      bond = list2->data;
      list2 = g_slist_next(list2);

      if (bond->atom1 == core || bond->atom2 == core)
        {
        data->ubonds = g_slist_remove(data->ubonds, bond);
        g_free(bond);
        }
      }

/* update selection */
    data->selection = g_slist_remove(data->selection, core);

/* delete any labels that reference the deleted core */
    list2 = data->measure_list;
    while (list2)
      {
      m = list2->data;
      list2 = g_slist_next(list2);
      if (measure_has_core(core, m))
        measure_free(m, data);
      }
/* update main list */
    data->cores = g_slist_remove(data->cores, core);
    g_free(core);
    }
  }

/* delete flaged shells in list */
list1 = data->shels;
while (list1)
  {
  shell = list1->data;
  list1 = g_slist_next(list1);

  if (shell->status & DELETED)
    {
    flag2 = TRUE;
/* update main list */
    data->shels = g_slist_remove(data->shels, shell);
    g_free(shell);
    }
  }

/* refresh totals */
data->num_atoms = g_slist_length(data->cores);
data->num_shells = g_slist_length(data->shels);

/* cope with deleted bonds - expensive, so only do if required */
if (flag1)
  {
  connect_bonds(data);
  connect_molecules(data);
  }

/* refresh net charge calc */
calc_emp(data);

/* refresh unique atom list */
g_slist_free(data->unique_atom_list);
data->unique_atom_list = find_unique(ELEMENT, data);

/* refresh widgets */
meas_graft_model(data);
gui_refresh(GUI_ACTIVE);

/* reset functions with static pointers to cores */
data->state = 0;
}

/*********************/
/* copy an atom core */
/*********************/
/* return the number of items copied (ie 0-2) */
#define DEBUG_COPY_CORE 0
struct core_pak *copy_core(struct core_pak *core, struct model_pak *src,
                                                  struct model_pak *dest)
{
gint items=0;
gdouble vec[3];
struct core_pak *copyc;
struct shel_pak *copys;

/* checks */
g_assert(core != NULL);
g_assert(src != NULL);
g_assert(dest != NULL);

/* duplicate data structure */
copyc = dup_core(core);
items++;

/* setup status */
copyc->status = copyc->status & (~SELECT & ~SELECT);
copyc->orig = copyc->primary = TRUE;
copyc->primary_core = NULL;
VEC3SET(copyc->offset, 0.0, 0.0, 0.0);

/* coords, account for transformation matrices */
ARR3SET(vec, core->rx);
vecmat(dest->ilatmat, vec);
ARR3ADD(vec, dest->centroid);
ARR3SET(copyc->x, vec);

dest->cores = g_slist_prepend(dest->cores, copyc);

/* attached shell? */
if (copyc->shell)
  {
  copys = copyc->shell;
  items++;

/* main info */
  copys->status = copys->status & (~SELECT);
  copys->primary=copys->orig=TRUE; 
  copys->primary_shell = NULL;
  VEC3SET(copys->offset, 0.0, 0.0, 0.0);

/* coords, account for transformation matrices */
  ARR3SET(vec, copys->rx);
  vecmat(dest->ilatmat, vec);
  ARR3ADD(vec, dest->centroid);
  ARR3SET(copys->x, vec);

  dest->shels = g_slist_prepend(dest->shels, copys);
  }

return(copyc);
}

/********************/
/* duplicate a core */
/********************/
struct core_pak *dup_core(struct core_pak *orig)
{
struct core_pak *core;

/* checks */
g_assert(orig != NULL);

core = g_malloc(sizeof(struct core_pak));
memcpy(core, orig, sizeof(struct core_pak));

/* clear pointers */
core->atom_label = NULL;
core->atom_type = NULL;
core->res_name = NULL;
core->chain = NULL;
core->bonds = NULL;
core->shell = NULL;
core->mol = NULL;
core->primary_core = NULL;
core->vibx_list = NULL;
core->viby_list = NULL;
core->vibz_list = NULL;
core->flags = NULL;

/* duplicate strings */
if (orig->atom_label)
  core->atom_label = g_strdup(orig->atom_label);

if (orig->atom_type)
  core->atom_type = g_strdup(orig->atom_type);

if (orig->res_name)
  core->res_name = g_strdup(orig->res_name);

if (orig->flags)
  core->flags = g_strdup(orig->flags);

/* duplicate the shell */
if (orig->shell)
  {
  core->shell = dup_shel(orig->shell);
  (core->shell)->core = core;
  }

return(core);
}

/*********************/
/* shell duplication */
/*********************/
struct shel_pak *dup_shel(struct shel_pak *orig)
{
struct shel_pak *shel;

/* checks */
g_assert(orig != NULL);

shel = g_malloc(sizeof(struct shel_pak));

memcpy(shel, orig, sizeof(struct shel_pak));

/* clear pointers */
shel->core = NULL;
shel->primary_shell = NULL;
VEC3SET(shel->pic, 0, 0, 0);

/* duplicate strings */
shel->flags = g_strdup(orig->flags);

return(shel);
}

/****************************/
/* base atom initialization */
/****************************/
void core_init(const gchar *elem, struct core_pak *core, struct model_pak *model)
{
gint code;
struct elem_pak elem_data;

/* attempt to match atom type with database */
if (model->protein)
  code = pdb_elem_type(elem);
else
  code = elem_test(elem);

/* flags, labels, ... */
core->atom_code = code;
core->atom_type = NULL;
core->atom_label = NULL;
core->res_name = NULL;
core->res_no = 1;
core->chain = NULL;
core->atom_order = 0;
core->molecule = 0;
core->ghost = FALSE;
core->status = NORMAL;
core->primary = TRUE;
core->orig = TRUE;
core->breathe = FALSE;
core->growth = FALSE;
core->translate = FALSE;
core->render_mode = BALL_STICK;
core->render_wire = FALSE;
core->region = REGION1A;
core->radius = 0.0;
core->has_sof = FALSE;
core->sof = 1.0;
core->charge = 0.0;
core->lookup_charge = TRUE;
core->flags = NULL;
core->bonds = NULL;
core->mol = NULL;
core->shell = NULL;
core->primary_core = NULL;
core->vibx_list = NULL;
core->viby_list = NULL;
core->vibz_list = NULL;

/* coords, velocities,... */
VEC4SET(core->x, 0.0, 0.0, 0.0, 1.0);
VEC4SET(core->rx, 0.0, 0.0, 0.0, 1.0);
VEC3SET(core->v, 0.0, 0.0, 0.0);
VEC3SET(core->offset, 0.0, 0.0, 0.0);
/* default values for NMR values */
core->atom_nmr_shift=0.0;
core->atom_nmr_aniso=0.0;
core->atom_nmr_asym=0.0;
core->atom_nmr_cq=0.0;
core->atom_nmr_efgasym=0.0;

/* element related initialization */
get_elem_data(code, &elem_data, model);
core->bond_cutoff = elem_data.cova;
ARR3SET(core->colour, elem_data.colour);
VEC3MUL(core->colour, 65535.0);
core->colour[3] = 1.0;
}

/***************************/
/* atom creation primitive */
/***************************/
gpointer core_new(const gchar *elem, const gchar *label, struct model_pak *model)
{
struct core_pak *core;

if (!elem)
  return(NULL);

g_assert(model != NULL);

core = g_malloc(sizeof(struct core_pak));

/* element related initialization */
core_init(elem, core, model);

/* general initialization */
if (label)
  core->atom_label = g_strdup(label);
else
  core->atom_label = g_strdup(elem);

/* NEW: hydrogen bond capabilities */
if (g_strrstr(elem, "H") != NULL)
  core->hydrogen_bond = TRUE;
else 
  core->hydrogen_bond = FALSE;

return(core);
}

/*******************************/
/* unified atom initialization */
/*******************************/
/* deprec */
#define DEBUG_NEW_CORE 0
struct core_pak *new_core(gchar *elem, struct model_pak *model)
{
struct core_pak *core;

core = core_new(elem, NULL, model);

return(core);
}

/****************************/
/* shell creation primitive */
/****************************/
gpointer shell_new(const gchar *elem, const gchar *label, struct model_pak *model)
{
gint code;
struct elem_pak elem_data;
struct shel_pak *shell;

g_assert(elem != NULL);
g_assert(model != NULL);

shell = g_malloc(sizeof(struct shel_pak));

/* attempt to match atom type with database */
code = elem_test(elem);
if (!code)
  printf("Warning: element [%s] not found.\n", elem);

/* init modifiable element data */
get_elem_data(code, &elem_data, model);
ARR3SET(shell->colour, elem_data.colour);

shell->atom_code = code;
if (label)
  shell->shell_label = g_strdup(label);
else
  shell->shell_label = g_strdup(elem);
VEC4SET(shell->x, 0.0, 0.0, 0.0, 1.0);
VEC4SET(shell->rx, 0.0, 0.0, 0.0, 1.0);
VEC3SET(shell->v, 0.0, 0.0, 0.0);
VEC3SET(shell->offset, 0.0, 0.0, 0.0);
shell->status = NORMAL;
shell->primary = TRUE;
shell->orig = TRUE;
shell->breathe = FALSE;
shell->translate = FALSE;
shell->region = REGION1A;
shell->core = NULL;
shell->primary_shell = NULL;
shell->radius = 0.0;
shell->charge = 0.0;
shell->has_sof = FALSE;
shell->sof = 1.0;
shell->lookup_charge = TRUE;
shell->flags = NULL;

return(shell);
}

/**********************************/
/* replacement shell init routine */
/**********************************/
/* deprec */
struct shel_pak *new_shell(gchar *elem, struct model_pak *model)
{
struct shel_pak *shell;

shell = shell_new(elem, NULL, model);

return(shell);
}

/***************************************/
/* fully duplicate a model's core list */
/***************************************/
GSList *dup_core_list(GSList *orig)
{
GSList *copy, *list;
struct core_pak *core;

copy = NULL;
for (list=orig ; list ; list=g_slist_next(list))
  {
  core = dup_core(list->data);
  copy = g_slist_prepend(copy, core);
  }
copy = g_slist_reverse(copy);

return(copy);
}

/****************************************/
/* fully duplicate a model's shell list */
/****************************************/
GSList *dup_shell_list(GSList *orig)
{
GSList *copy, *list;
struct shel_pak *shell;

copy = NULL;
for (list=orig ; list ; list=g_slist_next(list))
  {
  shell = dup_shel(list->data);
  copy = g_slist_prepend(copy, shell);
  }
copy = g_slist_reverse(copy);

return(copy);
}

/********************************************************/
/* set vector to minimum fractional offset (-0.5 - 0.5) */
/********************************************************/
#define DEBUG_FRAC_MINSQ 0
void fractional_min(gdouble *x, gint dim)
{
gint i, j;
gdouble whole;

g_assert(dim < 4);

#if DEBUG_FRAC_MINSQ
P3VEC("b4: ", x);
#endif

for (i=0 ; i<dim ; i++)
  {
/* clamp value -1 < x < 1 */ 
  x[i] = modf(x[i], &whole);

/* clamp to range (-0.5, 0.5) */
  j = (gint) (-2.0 * x[i]);
  x[i] += (gdouble) j;
  }

#if DEBUG_FRAC_MINSQ
P3VEC("af: ", x);
#endif
}

/*************************************************/
/* clamp coords to fractional limits [0.0 - 1.0) */
/*************************************************/
#define DEBUG_CLAMP 0
void fractional_clamp(gdouble *vec, gint *mov, gint dim)
{
gint i;
gdouble ip;

#if DEBUG_CLAMP
P3VEC("inp: ", vec);
#endif

/* NB: init ALL 3 coords as isolated molecules call this */
/* routine & it is expected that mov to be set to 0,0,0 */
mov[0] = mov[1] = mov[2] = 0;

/*
for (i=0 ; i<dim ; i++)
*/
for (i=dim ; i-- ; )
  {
  mov[i] = -vec[i];
  if (vec[i] < 0.0)
    {
/* increment the move for -ve's with exception -1.0, -2.0, -3.0,... */
    if (modf(vec[i], &ip) != 0.0)
      mov[i]++;
    }
  else
    mov[i] = -vec[i];

  vec[i] += (gdouble) mov[i];
  }

#if DEBUG_CLAMP
P3VEC("out: ", vec);
#endif
}

/********************************************/
/* confine a list of atoms to the unit cell */
/********************************************/
void coords_confine_cores(GSList *cores, struct model_pak *model)
{
gint dummy[3];
gdouble x[3];
GSList *list;
struct core_pak *core;
struct shel_pak *shell;

g_assert(model != NULL);

if (!model->periodic)
  return;

/* translate cores to within the cell */
for (list=cores ; list ; list=g_slist_next(list))
  {
  core = list->data;
  fractional_clamp(core->x, dummy, model->periodic);

/* move shell */
  if (core->shell)
    {
    shell = core->shell;

/* want core-shell distance to be smallest possible */
    ARR3SET(x, shell->x);
    ARR3SUB(x, core->x);
    fractional_min(x, model->periodic);
    ARR3SET(shell->x, core->x);
    ARR3ADD(shell->x, x);
    }
  }
}

/**********************************************/
/* confine a molecule's centroid to unit cell */
/**********************************************/
void coords_confine_centroid(struct mol_pak *mol, struct model_pak *model)
{
gint xlat[3];
gdouble mov[3];
GSList *list;
struct core_pak *core;

/* calc moves required to bring centroid within pbc */
fractional_clamp(mol->centroid, xlat, model->periodic);
ARR3SET(mov, xlat);

/* apply to all atoms/shells in this molecule */
for (list=mol->cores ; list ; list=g_slist_next(list))
  {
  core = list->data;
  ARR3ADD(core->x, mov);
  if (core->shell)
    {
    ARR3ADD((core->shell)->x, mov);
    }
  }
}

/***************************************/
/* find and remove any duplicate cores */
/***************************************/
// NB: expensive, should only be called if unavoidable
// NB: should only be called at startup (before bonds/mols are calculated)
void coords_remove_duplicates(struct model_pak *model)
{
gdouble dr, x[3], x1[3];
GSList *item1, *item2;
struct core_pak *core1, *core2;

if (!model)
  return;

item1=model->cores;
while (item1)
  {
  core1 = item1->data;

  ARR3SET(x1, core1->x);

  item2 = g_slist_next(item1);
  while (item2)
    {
    core2 = item2->data;
    item2 = g_slist_next(item2);

    ARR3SET(x, x1);
    ARR3SUB(x, core2->x);

    dr = VEC3MAG(x);
    if (dr < FRACTION_TOLERANCE)
      {
      model->cores = g_slist_remove(model->cores, core2);
      core_free(core2);
      }
    }
  item1 = g_slist_next(item1);
  }
}

/*******************************************/
/* draw a box - contents are the selection */
/*******************************************/
void update_box(gint x, gint y, struct model_pak *data, gint call_type)
{
switch (call_type)
  {
  case START:
/* don't clean the selection (allows multiple box selections) */
/* setup the box selection object */
    data->box_on = TRUE;
    data->select_box[0] = x;
    data->select_box[1] = y;
  case UPDATE:
    data->select_box[2] = x;
    data->select_box[3] = y;
    break;
  }
}

/**********************************************************/
/* check if a given label matches with element/atom label */
/**********************************************************/
#define DEBUG_SHELL_MATCH 0
gint shel_match(const gchar *label, struct shel_pak *shell)
{
gint code;

#if DEBUG_SHELL_MATCH
printf("[%s] : [%s]\n", label, shell->shell_label);
#endif

code = elem_symbol_test(label);

/* if input label doesnt match the element symbol length - it means the */
/* user has put in something like H1 - compare this with the atom label */
if (code)
  {
  if (g_ascii_strcasecmp(label, elements[shell->atom_code].symbol) != 0)
    {
    if (g_ascii_strcasecmp(shell->shell_label, label) == 0)
      return(1);
    }
  else
    return(1);
  }
else
  return(1);

#if DEBUG_SHELL_MATCH
printf("rejected.\n");
#endif

return(0);
}

/**********************************************************/
/* check if a given label matches with element/atom label */
/**********************************************************/
#define DEBUG_ATOM_MATCH 0
gint core_match(const gchar *label, struct core_pak *core)
{
gint code;

#if DEBUG_ATOM_MATCH
printf("[%s] : [%s]\n", label, core->atom_label);
#endif

code = elem_symbol_test(label);

/* if input label doesnt match the element symbol length - it means the */
/* user has put in something like H1 - compare this with the atom label */
if (code)
  {
  if (g_ascii_strcasecmp(label, elements[core->atom_code].symbol) != 0)
    {
    if (g_ascii_strcasecmp(core->atom_label, label) == 0)
      return(1);
    }
  else
    return(1);
  }
else
  return(1);

#if DEBUG_ATOM_MATCH
printf("rejected.\n");
#endif

return(0);
}

/*************************************************************/
/* region changing routine - for dealing with polar surfaces */
/*************************************************************/
/* can now go both ways via the direction flag (UP or DOWN) */
#define DEBUG_REGION_SWITCH_ATOM 0
gint region_move_atom(struct core_pak *core, gint direction,
                                    struct model_pak *data)
{
gint flag, primary, secondary, mov[2];
gdouble vec[3], tmp[3], d[3];
GSList *list;
struct core_pak *comp;

#if DEBUG_REGION_SWITCH_ATOM
printf("       model: %s\n", data->basename);
printf(" periodicity: %d\n", data->periodic);
printf("         hkl: %f %f %f\n", data->surface.miller[0],
          data->surface.miller[1], data->surface.miller[2]);
printf("        dhkl: %f\n", data->surface.dspacing);
printf("region sizes: %f %f\n", data->surface.region[0], data->surface.region[1]);
printf("      moving: ");
if (direction == UP)
  printf("UP\n");
else
  printf("DOWN\n");
#endif

/* checks */
g_return_val_if_fail(data != NULL, 1);
g_return_val_if_fail(data->periodic == 2, 1);
if (data->surface.region[0] < 1)
  {
  gui_text_show(ERROR, "region 1 is empty.\n");
  return(1);
  }

/* setup region switching labels */
if (direction == UP)
  {
  primary = REGION1A;
  secondary = REGION2A;
  }
else
  {
  primary = REGION2A;
  secondary = REGION1A;
  }

/* get fractional depth translation vector */
ARR3SET(vec, data->surface.depth_vec);
vecmat(data->ilatmat, vec);

/* calculate offset to region boundary */
ARR3SET(tmp, vec);
if (direction == DOWN)
  {
  VEC3MUL(tmp, data->surface.region[0]);
  VEC3MUL(tmp, -1.0);
  }
else
  {
  if (data->surface.region[1] == 0)
    {
    VEC3MUL(tmp, data->surface.region[0]);
    }
  else
    {
    VEC3MUL(tmp, data->surface.region[1]);
    }
  }

/* if region 2 is empty, just move core to the bottom */
if (data->surface.region[1] == 0.0)
  {
  ARR3ADD(core->x, tmp);
  if (core->shell)
    {
    ARR3ADD((core->shell)->x, tmp);
    }
  atom_colour_scheme(data->colour_scheme, core, data);
  return(0);
  }

/* get coordinates of target atom */
ARR3ADD(tmp, core->x);

#if DEBUG_REGION_SWITCH_ATOM
P3VEC("    translation: ", vec);
P3VEC("  target coords: ", tmp);
#endif

/* find the target */
flag=0;
for (list=data->cores ; list ; list=g_slist_next(list))
  {
  comp = list->data;

/* only atoms of the same type need apply */
  if (core->atom_code != comp->atom_code)
    continue;

/* get difference vector */
  ARR3SET(d, comp->x);
  ARR3SUB(d, tmp);

/* pbc constraint */
  while(d[0] < -FRACTION_TOLERANCE)
    d[0] += 1.0;
  while(d[0] > 0.5)
    d[0] -= 1.0;
  while(d[1] < -FRACTION_TOLERANCE)
    d[1] += 1.0;
  while(d[1] > 0.5)
    d[1] -= 1.0;

/* test difference vector's magnitude */
  if (VEC3MAGSQ(d) < FRACTION_TOLERANCE)
    {
/* change its labelling */
#if DEBUG_REGION_SWITCH_ATOM
printf("Matched core: %p\n", comp);
#endif
    comp->region = secondary;
    if (comp->shell)
      {
      (comp->shell)->region = secondary;
      }
    atom_colour_scheme(data->colour_scheme, comp, data);
    flag++;
    break;
    }
  }

if (!flag)
  {
  gui_text_show(ERROR, "Failed to find a boundary image.\n");
  return(1);
  }

/* now move selected atom to bottom of region 2 */
ARR3SET(tmp, vec);
VEC3MUL(tmp, (data->surface.region[0] + data->surface.region[1]));
if (direction == UP)
  VEC3MUL(tmp, -1.0);
ARR3SUB(core->x, tmp);
core->region = primary;
/* pbc constrain */
fractional_clamp(core->x, mov, 2);

if (core->shell)
  {
  ARR3SUB((core->shell)->x, tmp);
  ARR2ADD((core->shell)->x, mov);
  (core->shell)->region = primary;
  }

atom_colour_scheme(data->colour_scheme, core, data);

return(0);
}

/*********************************************/
/* find and record the maximum region number */
/*********************************************/
gint region_max(struct model_pak *mdata)
{
gint max;
GSList *list;
struct core_pak *core;

max = 0;
for (list=mdata->cores ; list ; list=g_slist_next(list))
  {
  core = list->data;
  if (core->region > max)
    max = core->region;
  }
return(max);
}

/**************************************/
/* compute vector sum of bonded atoms */
/**************************************/
#define DEBUG_EDIT_VSUM_GET 0
gint edit_vsum_get(gdouble *sum, struct core_pak *target, struct model_pak *model)
{
gint cp_count;
gdouble a, n, dr, x[3], cp[3], prev[3], cp_sum[3];
GSList *item;
struct core_pak *core;
struct bond_pak *bond;

VEC3SET(cp_sum, 0.0, 0.0, 0.0);
VEC3SET(sum, 0.0, 0.0, 0.0);
n = 0.0;
cp_count = 0;

#if DEBUG_EDIT_VSUM_GET
printf("processing [%s] with bonds: %d\n", target->atom_label, g_slist_length(target->bonds));
#endif

for (item=target->bonds ; item ; item=g_slist_next(item))
  {
  bond = item->data;

// TODO - need special case to cope with bonds "broken" by periodicity
// this will affect the coordinates (some atoms may be bonded to an image 
// of themselves, for example)
  if (bond->atom1 == target)
    {
    core = bond->atom2;
ARR3SET(x, bond->offset);
VEC3MUL(x, -1.0);
    }
  else
    {
    core = bond->atom1;
ARR3SET(x, bond->offset);
    }

#if DEBUG_EDIT_VSUM_GET
P3VEC("offset: ", x);
#endif

/* skip dummy atoms */
/* skip H's */
  if (core->atom_code > 1)
    {
/*
    ARR3SET(x, target->rx);
    ARR3SUB(x, core->rx);
    normalize(x, 3);
*/
    vecmat(model->latmat, x);
    normalize(x, 3);


/* can't calculate cross produce until we get 2 vector's */
if (cp_count)
  {
  crossprod(cp, x, prev);
  ARR3ADD(cp_sum, cp);
  }
ARR3SET(prev, x);
cp_count++;
 

#if DEBUG_EDIT_VSUM_GET
printf("+ [%s]", core->atom_label);
P3VEC(" : ", x);
#endif


    ARR3ADD(sum, x);
    n++;
    }
  else
    {
#if DEBUG_EDIT_VSUM_GET
printf("- [%s]\n", core->atom_label);
#endif
    }

  }

/* here we cope with cases where the vector sum of bonds is < 1 */
/* eg no bonds or a linear/planar/etc balanced configuration */
dr = VEC3MAG(sum);

#if DEBUG_EDIT_VSUM_GET
printf("vmag = %f, ", dr);
P3VEC("v = ", sum);
#endif

if (dr < 0.5)
  {
  if (cp_count)
    {
    normalize(cp_sum, 3);

#if DEBUG_EDIT_VSUM_GET
P3VEC("cross product: ", cp_sum);
#endif

    ARR3SET(sum, cp_sum);

// ensure cp is orientated away from centroid
// TODO - other cases?
    ARR3SET(x, target->x);
    ARR3SUB(x, model->centroid);
    vecmat(model->latmat, x); 
    a = via(x, sum, 3);
    if (a > 0.5*PI)
      {
      VEC3MUL(sum, -1.0);
      }
    }

/* if centroid corresponds to target atom position */
/* just pick a direction and hope for the best */
  dr = VEC3MAG(sum);
  if (dr < POSITION_TOLERANCE)
    {
    VEC3SET(sum, 0.0, 0.0, 1.0);
    }
  else
    normalize(sum, 3);
  }

#if DEBUG_EDIT_VSUM_GET
P3VEC("final offset: ", sum);
printf("mag = %f\n", VEC3MAG(sum));
#endif

return(TRUE);
}

/***********************************************/
/* vector sum based approach to add a new atom */
/***********************************************/
#define MAX_H_CORES 4
void edit_vsum_add_core(const gchar *element, struct core_pak *target, struct model_pak *model)
{
gint nh=0;
gdouble r, sum[3], z[3], mat[9];
GSList *item, *list;
struct core_pak *core, *hcore[MAX_H_CORES];

/*
printf("Adding [%s] to [%s]", element, target->atom_label);
P3VEC(" : ", target->x);
*/

list = connect_neighbours(target);
for (item=list ; item ; item=g_slist_next(item))
  {
  core = item->data;
  if (core->atom_code == 1)
    {
    hcore[nh] = core;
    nh++;
    }
  }

/* FIXME - assumes latter is H (H add) */
r = elements[target->atom_code].cova + elements[1].cova;

//printf("bond length = %f\n", r);

if (edit_vsum_get(sum, target, model))
  {
  normalize(sum, 3);
  VEC3MUL(sum, r);

  switch (nh)
    {
    case 2:
// add 1 + modify 2 - tripod
/* compute transformation to align with vsum */
      VEC3SET(z, 0.0, 0.0, 1.0);
      matrix_v_alignment(mat, z, sum);

/* set new H */
      core = core_new(element, element, model);
      VEC3SET(core->x, 0.0, 0.94, 0.33);
VEC3MUL(core->x, r);
      vecmat(mat, core->x);
vecmat(model->ilatmat, core->x);
      ARR3ADD(core->x, target->x);
      model->cores = g_slist_prepend(model->cores, core);

/* modify existing */
      core = hcore[1];
      VEC3SET(core->x,  0.87, -0.5, 0.33);
VEC3MUL(core->x, r);
      vecmat(mat, core->x);
vecmat(model->ilatmat, core->x);
      ARR3ADD(core->x, target->x);

/* modify existing */
      core = hcore[0];
      VEC3SET(core->x, -0.87, -0.5, 0.33);
VEC3MUL(core->x, r);
      vecmat(mat, core->x);
vecmat(model->ilatmat, core->x);
      ARR3ADD(core->x, target->x);
      break;

    case 1:
// add 1 + modify 1 with 105 angle
/* compute transformation to align with vsum */
      VEC3SET(z, 0.0, 0.0, 1.0);
      matrix_v_alignment(mat, z, sum);

/* set new H */
      core = core_new(element, element, model);
      VEC3SET(core->x, 0.79, 0.0, 0.6);
VEC3MUL(core->x, r);
      vecmat(mat, core->x);
vecmat(model->ilatmat, core->x);
      ARR3ADD(core->x, target->x);
      model->cores = g_slist_prepend(model->cores, core);

/* modify existing */
      core = hcore[0];
      VEC3SET(core->x, -0.79, 0.0, 0.6);
VEC3MUL(core->x, r);
      vecmat(mat, core->x);
vecmat(model->ilatmat, core->x);
      ARR3ADD(core->x, target->x);
      break;

    case 0:
/* no other H's - plain add */
      core = core_new(element, element, model);
      ARR3SET(core->x, target->x);
vecmat(model->ilatmat, sum);
      ARR3ADD(core->x, sum);
      model->cores = g_slist_prepend(model->cores, core);
      break;

/* too many existing H's ... */
    default:
      gui_text_show(WARNING, "Unsupported number of Hydrogens.\n");
    }
  }
}
