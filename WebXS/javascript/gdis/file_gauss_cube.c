/*
Copyright (C) 2007 by Jög Meyer

meyer@fhi-berlin.mpg.de

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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "gdis.h"
#include "coords.h"
#include "file.h"
#include "parse.h"
#include "matrix.h"
#include "model.h"
#include "interface.h"


#define DEBUG_READ_GAUSS_CUBE 0

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];


/************************/
/* i/o helper functions */
/************************/
#define DEBUG_GAUSS_CUBE_CHECK_LINE 0
gboolean gauss_cube_check_valid_line(gchar *line, gchar *caller, gchar *quantity)
{
gboolean valid_line = TRUE;

#if DEBUG_GAUSS_CUBE_CHECK_LINE
printf("%s has read line for %s \n", caller, quantity);
printf("-> %s", line);
#endif

if (line == NULL)
  {
  valid_line = FALSE;
  gchar *mesg = g_strdup_printf("%s: reached end of file when reading %s \n", caller, quantity);
  gui_text_show(ERROR, mesg);
  g_free(mesg);
  }

return valid_line;
}

#define DEBUG_GAUSS_CUBE_CHECK_VALID_TOKENS 0
gboolean gauss_cube_check_valid_tokens(gchar **buff, gint num_tokens, gint wanted_tokens, 
                                       gchar *caller, gchar *quantity, gchar *line)
{
gboolean valid_tokens = FALSE;

#if DEBUG_GAUSS_CUBE_CHECK_VALID_TOKENS
printf("%s has read and tokenized line for %s \n", caller, quantity);
printf("-> %s", line);
#endif

if (line == NULL)
  {
  valid_tokens = FALSE;
  gchar *mesg = g_strdup_printf("%s: reached end of file when reading %s \n", caller, quantity);
  gui_text_show(ERROR, mesg);
  g_free(mesg);
  }

if (num_tokens == wanted_tokens)
  {
  valid_tokens = TRUE;
#if DEBUG_GAUSS_CUBE_CHECK_VALID_TOKENS
printf("gauss_cube_check_valid_tokens: ACCEPTED (%d tokens wanted, %d tokens found) \n", wanted_tokens, num_tokens); 
#endif
  }
else
  { 
  valid_tokens = FALSE;
  gchar *mesg = g_strdup_printf("%s: wrong number of tokens for %s \n", caller, quantity);
  gui_text_show(ERROR, mesg);
  g_free(mesg);
#if DEBUG_GAUSS_CUBE_CHECK_VALID_TOKENS
printf("gauss_cube_check_valid_tokens: REJECTED (%d tokens wanted, %d tokens found) \n", wanted_tokens, num_tokens); 
#endif
  }

return valid_tokens;
}

/*****************************/
/* read Gaussian cube header */
/*****************************/
gint read_gauss_cube_header(FILE *fp, struct model_pak *model)
{
gint i, num_tokens;
gchar *caller = "read_gauss_cube_header";
gchar **buff, *line;

/* first comment line */
line = file_read_line(fp);
if (!gauss_cube_check_valid_line(line, caller, "(comment1)"))
  return(2);
model->gauss_cube.comment1 = g_string_new(line);
model->title = g_strdup(model->gauss_cube.comment1->str);

/* second comment line (currently ignored) */
g_free(line);
line = file_read_line(fp);
if (!gauss_cube_check_valid_line(line, caller, "(comment2)"))
  return(2);
model->gauss_cube.comment2 = g_string_new(line);

/* number of atoms and origin */
g_free(line);
line = file_read_line(fp);
buff = tokenize(line, &num_tokens);
if (gauss_cube_check_valid_tokens(buff, num_tokens, 4, caller, "(N, r0)", line))
  {
  model->gauss_cube.N = (gint) str_to_float(*(buff+0));
  for (i=0 ; i<3 ; i++)
    model->gauss_cube.r0[i] = str_to_float(*(buff+i+1));
#if DEBUG_READ_GAUSS_CUBE
printf("N = %d, r0_x = %f, r0_y = %f, r0_z = %f \n",
       model->gauss_cube.N, model->gauss_cube.r0[0], model->gauss_cube.r0[1], model->gauss_cube.r0[2]);
#endif
  }
else
  return(2);

/* increments in direction a */
g_free(line);
g_strfreev(buff);
line = file_read_line(fp);
buff = tokenize(line, &num_tokens);
if ( gauss_cube_check_valid_tokens(buff, num_tokens, 4, caller, "(na, da)", line) )
  {
  model->gauss_cube.na = (gint) str_to_float(*(buff+0));
  for (i=0;i<3;i++) model->gauss_cube.da[i] = str_to_float(*(buff+i+1));
#if DEBUG_READ_GAUSS_CUBE
printf("na = %d, da_x = %f, da_y = %f, da_z = %f \n",
       model->gauss_cube.na, model->gauss_cube.da[0], model->gauss_cube.da[1], model->gauss_cube.da[2]);
#endif
  }
else
  return(2);

/* increments in direction b */
g_free(line);
g_strfreev(buff);
line = file_read_line(fp);
buff = tokenize(line, &num_tokens);
if (gauss_cube_check_valid_tokens(buff, num_tokens, 4, caller, "(nb, db)", line))
  {
  model->gauss_cube.nb = (gint) str_to_float(*(buff+0));
  for (i=0;i<3;i++) model->gauss_cube.db[i] = str_to_float(*(buff+i+1));
#if DEBUG_READ_GAUSS_CUBE
printf("nb = %d, db_x = %f, db_y = %f, db_z = %f \n", 
       model->gauss_cube.nb, model->gauss_cube.db[0], model->gauss_cube.db[1], model->gauss_cube.db[2]);
#endif
  }
else
  return(2);

/* increments in direction c */
g_free(line);
g_strfreev(buff);
line = file_read_line(fp);
buff = tokenize(line, &num_tokens);
if (gauss_cube_check_valid_tokens(buff, num_tokens, 4, caller, "(nc, dc)", line))
  {
  model->gauss_cube.nc = (gint) str_to_float(*(buff+0));
  for (i=0;i<3;i++) model->gauss_cube.dc[i] = str_to_float(*(buff+i+1));
#if DEBUG_READ_GAUSS_CUBE
printf("nc = %d, dc_x = %f, dc_y = %f, dc_z = %f \n", 
       model->gauss_cube.nc, model->gauss_cube.dc[0], model->gauss_cube.dc[1], model->gauss_cube.dc[2]);
#endif
  }
else
  return(2);

/* NB: gdis wants transposed matrix */
for (i=0; i<3; i++)
  {
  model->latmat[0+3*i] = (abs(model->gauss_cube.na) - 1) * model->gauss_cube.da[i] * AU2ANG;
  model->latmat[1+3*i] = (abs(model->gauss_cube.nb) - 1) * model->gauss_cube.db[i] * AU2ANG;
  model->latmat[2+3*i] = (abs(model->gauss_cube.nc) - 1) * model->gauss_cube.dc[i] * AU2ANG;
  }
model->construct_pbc = TRUE;
model->periodic = 3;

g_free(line);
g_strfreev(buff);

return(0);
}

/****************************/
/* read Gaussian cube atoms */
/****************************/
gint read_gauss_cube_atoms(FILE *fp, struct model_pak *model)
{
gint n, i, num_tokens;
gchar *caller = "read_gauss_cube_atoms";
gchar *quantity;
gchar **buff, *line;
GSList *clist;
struct core_pak *core;
struct gauss_cube_pak gauss_cube; 

model->gauss_cube.atoms = g_malloc(abs(model->gauss_cube.N) * sizeof(struct gauss_cube_atoms_pak));

gauss_cube = model->gauss_cube;
clist = model->cores;

for (n=0 ; n<abs(gauss_cube.N) ; n++)
  {
  line = file_read_line(fp);
  quantity = g_strdup_printf("(atom %d)", n+1);
  buff = tokenize(line, &num_tokens);
  if (gauss_cube_check_valid_tokens(buff, num_tokens, 5, caller, quantity, line))
    {
    gauss_cube.atoms[n].number = (gint) str_to_float(*(buff+0));
    gauss_cube.atoms[n].unknown = str_to_float(*(buff+1));
    for (i=0 ; i<3 ; i++)
      gauss_cube.atoms[n].r[i] = str_to_float(*(buff+i+2));

    if (clist)
      {
      core = clist->data;
      clist = g_slist_next(clist);
      }
    else
      {
      core = new_core(*(buff+0), model);
      model->cores = g_slist_append(model->cores, core);
      }
    for (i=0 ; i<3 ; i++)
      core->x[i] = gauss_cube.atoms[n].r[i] * AU2ANG;

#if DEBUG_READ_GAUSS_CUBE
printf("new core: %s %f %f %f \n", 
       elements[core->atom_code].symbol, core->x[0],  core->x[1], core->x[2]);
#endif
    }
  else
    return(2);

  g_free(buff);
  g_free(line);
  }

#if DEBUG_READ_GAUSS_CUBE
printf("cores saved in model->gauss_cube.atoms: \n");
for (n=0; n<abs(model->gauss_cube.N); n++) {
  gint number = model->gauss_cube.atoms[n].number;
  gdouble unknown = model->gauss_cube.atoms[n].unknown;
  gdouble r[3];
  for (i=0; i<3; i++) r[i] = model->gauss_cube.atoms[n].r[i];
  printf("\t %3d \t %8.3f \t %13.8f \t %13.8f \t %13.8f \n", number, unknown, r[0], r[1], r[2]);
}
#endif 

return(0);
}

/******************************/
/* read Gaussian cube orbital */
/******************************/
gint read_gauss_cube_orbitals(FILE *fp, struct model_pak *model)
{
gint o, NMOs, num_tokens;
gchar *caller = "read_gauss_cube_orbitals";
gchar *quantity = "(orbitals)";
gchar **buff, *line;

if (model->gauss_cube.N < 0)
  {
  line = file_read_line(fp);
  if (!gauss_cube_check_valid_line(line, caller, quantity))
    {
    g_free(line);
    return(2);
    }
  buff = tokenize(line, &num_tokens);
  if (num_tokens >= 1)
    {
/* NMOs is not a pointer in gauss_cube_pak, therefore update original, not copy!*/
    model->gauss_cube.NMOs = (gint) str_to_float(*(buff+0));
    NMOs = abs(model->gauss_cube.NMOs);

#if DEBUG_READ_GAUSS_CUBE
printf("read_gauss_cube_orbitals: trying to read %d MO indices now...", NMOs);
#endif

    if (gauss_cube_check_valid_tokens(buff, num_tokens, NMOs + 1, caller, quantity, line))
      {
      model->gauss_cube.MOs = g_malloc(NMOs * sizeof(gint));
      for (o=0; o<NMOs; o++)
        model->gauss_cube.MOs[o] = (gint) str_to_float(*(buff+o+1));
      }
    else
      {
      g_free(line);
      g_free(buff);
      return(2);
      }
    }
  else
    {
    g_free(line);
    g_free(buff);
    gui_text_show(ERROR, "read_gauss_cube_orbitals: MO indices expected, but none found");
    return(2);
    }
  }
else
  {
  model->gauss_cube.NMOs = 0;
  #if DEBUG_READ_GAUSS_CUBE
  printf("read_gauss_cube_orbitals: no MO indices are read...");
  #endif
  }

#if DEBUG_READ_GAUSS_CUBE
printf("%d MO indices saved in model->gauss_cube.MOs: \n", abs(model->gauss_cube.NMOs));
printf("\t");
for (o=0; o<abs(model->gauss_cube.NMOs); o++) printf(" %d ", model->gauss_cube.MOs[o]);
printf("\n");
#endif 

return(0);
}

/********************************/
/* read Gaussian cube grid data */
/********************************/
#define VOXELS_PER_LINE 6
gint read_gauss_cube_grid_data(FILE *fp, struct model_pak *model)
{
gint num_tokens;
gint V, v, i, j, k, dk, tokens, t;
gdouble x, xmin, xmax;
gchar *caller = "read_gauss_cube_grid_data";
gchar *quantity = "(voxels)";
gchar **buff, *line, *tmp;

V = abs(model->gauss_cube.na) * abs(model->gauss_cube.nb) * abs(model->gauss_cube.nc);
#if DEBUG_READ_GAUSS_CUBE
printf("read_gauss_cube_grid_data: allocating memory for %d voxels \n", V);
#endif
model->gauss_cube.voxels = g_malloc(V * sizeof(gdouble));
#if DEBUG_READ_GAUSS_CUBE
printf("read_gauss_cube_grid_data: memory for %d voxels allocated \n", V);
#endif

xmin = G_MAXDOUBLE;
xmax = -G_MAXDOUBLE;

v=0;
while ( v<V )
  {
/* ranges of i,j,k are from 0 to model->gauss_cube.na-1, ~.nb-1, ~.nc-1 respectively */
  i = v / (model->gauss_cube.nb * model->gauss_cube.nc);
  j = ( v - i * model->gauss_cube.nb * model->gauss_cube.nc ) / model->gauss_cube.nc;
  k = v - i * model->gauss_cube.nb * model->gauss_cube.nc - j * model->gauss_cube.nc;

  #if DEBUG_READ_GAUSS_CUBE
  printf("v=%d, i=%d, j=%d, k=%d \n", v, i, j, k);
  #endif

  dk = model->gauss_cube.nc - k;
  if ( (dk >= VOXELS_PER_LINE ) || (dk == 0) ) {
    tokens = VOXELS_PER_LINE;
    }
  else {
    tokens = dk;
    } 

  line = file_read_line(fp);
  buff = tokenize(line, &num_tokens);

  if (gauss_cube_check_valid_tokens(buff, num_tokens, tokens, caller, quantity, line))
    {
    for (t=0; t<tokens; t++)
      {
      x = str_to_float(*(buff+t));
      if (x < xmin)
        xmin = x;
      if (x > xmax)
        xmax = x;

      model->gauss_cube.voxels[v+t] = x;
      }
    v += tokens;
    }
  else
    return(2);

  g_free(line);
  g_strfreev(buff);
  }

//printf("x range: [%f, %f]\n", xmin, xmax);

tmp = g_strdup_printf("%f", xmin);
property_add_ranked(5, "Gaussian cube min", tmp, model);
g_free(tmp);

tmp = g_strdup_printf("%f", xmax);
property_add_ranked(5, "Gaussian cube max", tmp, model);
g_free(tmp);

return(0);
}

/********************************/
/* Read in a Gaussian cube file */
/********************************/
gint read_gauss_cube(gchar *filename, struct model_pak *model)
{
FILE *fp;

fp = fopen(filename, "rt");
if (!fp)
  return(1);

model->construct_pbc = FALSE;
model->fractional = FALSE;

#if DEBUG_READ_GAUSS_CUBE
printf("start reading cube file \n");
#endif

read_gauss_cube_header(fp, model);
read_gauss_cube_atoms(fp, model);
read_gauss_cube_orbitals(fp, model);
read_gauss_cube_grid_data(fp, model);

#if DEBUG_READ_GAUSS_CUBE
printf("end reading cube file \n");
#endif

g_free(model->filename);
model->filename = g_strdup(filename);

g_free(model->basename);
model->basename = g_path_get_basename(filename);

model_prep(model);

return(0);
}
