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
#include "edit.h"
#include "matrix.h"
#include "model.h"
#include "numeric.h"
#include "mdi.h"
#include "mdi_pak.h"
#include "zone.h"
#include "library.h"
#include "interface.h"

extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/***************************************************************/
/* check if a (new) core is bonded to anything else in a model */
/***************************************************************/
// NB: currently only tests overlap with pre-existing model cores,
// since newly added solvent cores won't be listed in the zone array
// see connect_atom_compute()
gint mdi_overlap_test(struct core_pak *core, struct model_pak *model)
{
gint ret=0;
GSList *list;

//printf("%f\n", core->bond_cutoff);
/*
P3VEC("core x:", core->x);
print_core_list(model->cores);
printf("core mol: %d\n", core->molecule);
*/

/* HACK - using core->molecule to temporarily disable double count check */
core->molecule = -1;
connect_atom_compute(core, model);
core->molecule = 0;

/* flag as overlapping if bonded - IGNORE hbonds though */
list = connect_neighbours(core);
if (list)
  {
  g_slist_free(list);
  ret=1;
  }

connect_atom_clear(core, model);

return(ret);
}

/******************************************************/
/* solvate the supplied target with the named solvent */
/******************************************************/
#define DEBUG_MDI_LIBRARY_SOLVATE 0
void mdi_library_solvate(gchar *solvent, struct model_pak *target)
{
gint overlap, image[3], xi[3];
gdouble x[3], lim[3], ilim[3];
GSList *list, *item, *mlist;
struct model_pak *source=NULL;
struct core_pak *core;

if (!target)
  return;

/* search for solvent in library hash table */
source = library_entry_get("solvents", solvent);

#if DEBUG_MDI_LIBRARY_SOLVATE
printf("Using solvent [%s]: %p.\n", solvent, source);
#endif

/* check we found a solvent model */
if (source)
  {
/* do molecule calc on fresh library source model */
  model_prep(source);

// need to enforce this (in case target is a new box)
  zone_init(target);

  lim[0] = target->pbc[0] / source->pbc[0];
  lim[1] = target->pbc[1] / source->pbc[1];
  lim[2] = target->pbc[2] / source->pbc[2];
  ilim[0] = 1.0 / lim[0];
  ilim[1] = 1.0 / lim[1];
  ilim[2] = 1.0 / lim[2];

// TODO - loop from 1 to lim to generate periodic images

  image[0] = nearest_int(lim[0] + 0.5);
  image[1] = nearest_int(lim[1] + 0.5);
  image[2] = nearest_int(lim[2] + 0.5);

#if DEBUG_MDI_LIBRARY_SOLVATE
P3VEC("Fractional limits: ", lim);
P3VEC("inv Fractional limits: ", ilim);
printf("images: %d %d %d\n", image[0], image[1], image[2]);
printf("molecules: %d\n", g_slist_length(source->moles));
#endif

/* TODO - skip the bond tests for all but one of the edge cases to speed */
/* things up - since we know that for whole copies of the library solvent */
/* box there is no overlap - so dont waste time in mdi_overlap_test() */
  for (xi[0]=0 ; xi[0]<image[0] ; xi[0]++)
  for (xi[1]=0 ; xi[1]<image[1] ; xi[1]++)
  for (xi[2]=0 ; xi[2]<image[2] ; xi[2]++)
    {

//printf("xi: %d %d %d\n", xi[0], xi[1], xi[2]);

    for (list=source->moles ; list ; list=g_slist_next(list))
      {
      struct mol_pak *mol=list->data;
//P3VEC("centroid: ", mol->centroid);

      ARR3SET(x, mol->centroid);
      ARR3ADD(x, xi);

      if ((x[0] < lim[0]) && (x[1] < lim[1]) && (x[2] < lim[2]))
        {
        mlist = NULL;
        overlap = 0;
        for (item=mol->cores ; item ; item=g_slist_next(item))
          {
          core = dup_core(item->data);

          core->res_no = -1;
          ARR3ADD(core->x, xi);
          ARR3MUL(core->x, ilim);

          overlap += mdi_overlap_test(core, target);

// TODO - omit if overlapping source model atoms
// also if it doesn't fit nicely in the box (ie overlap with periodic images)
//          target->cores = g_slist_prepend(target->cores, core);
          mlist = g_slist_prepend(mlist, core);
          }

        if (overlap)
          {
// FIXME - not sufficient - need to core_free() on the data
          free_slist(mlist);
          }
        else
          {
          target->cores = g_slist_concat(target->cores, mlist);
          }

        }
      }
    }

  model_free(source);

  }
else
  gui_text_show(ERROR, "Solvent missing from internal library.\n");
}

/*************************************************/
/* fill the gaps in model with specified solvent */
/*************************************************/
#define DEBUG_MDI_SOLVATE 0
gint mdi_model_solvate(gpointer data, struct model_pak *model)
{
GSList *list;
struct core_pak *core;
struct mdi_pak *mdi = data;

if (!mdi)
  return(FALSE);
if (!model)
  return(FALSE);

// TODO - only need to do this if a brand new MDI model
/*
model->fractional = TRUE;
matrix_lattice_init(model);
*/

#if DEBUG_MDI_SOLVATE
printf("mdi_model_solvate(): ...\n");
P3MAT("mat:", model->latmat);
printf("solvent: %s\n", mdi->solvent);
#endif

/* remove any solvent molecules from previous run */
for (list=model->cores ; list ; list=g_slist_next(list))
  {
  core = list->data;
  if (core->res_no == -1)
    delete_core(core);
  }
delete_commit(model);

/* solvate from library */
mdi_library_solvate(mdi->solvent, model);

return(TRUE);
}

