/*
Copyright (C) 2004 by Andrew Lloyd Rohl and Sean David Fleming

andrew@ivec.org

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

#include "gdis.h"
#include "coords.h"
#include "file.h"
#include "parse.h"
#include "scan.h"
#include "model.h"
#include "import.h"
#include "interface.h"
#include "animate.h"
#include "matrix.h"
#include "zmatrix.h"
#include "nwchem.h"

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];


gchar *nwchem_symbol_table[] = {
// control
"dummy", "start", "geometry",
"system", "crystal", "surface", "polymer",
"basis", "charge", "task", "end",
// theory
"scf", "dft", "sodft", "mp2", "direct_mp2",
"rimp2", "ccsd", "ccsd(t)", "ccsd+t(ccsd)", "mcscf",
"selci", "md", "pspw", "band", "tce",
// operation
"energy", "gradient", "optimize", "saddle", "hessian",
"frequencies", "property", "dynamics", "thermodynamics"};


/***************************************************/
/* populate the symbol table for the token scanner */
/***************************************************/
void nwchem_scan_symbols_init(gpointer scan)
{
gint i;

for (i=1 ; i<NWCHEM_LAST ; i++)
  scan_symbol_add(scan, nwchem_symbol_table[i], i);
}

/*************************/
/* free an nwchem object */
/*************************/
void nwchem_free(gpointer data)
{
struct nwchem_pak *nwchem=data;

if (!nwchem)
  return;

g_free(nwchem->start);
g_free(nwchem->charge);
g_free(nwchem->task_theory);
g_free(nwchem->task_operation);
g_hash_table_destroy(nwchem->library);

g_free(nwchem);
}

/***********************************/
/* create a new nwchem info object */
/***********************************/
gpointer nwchem_new(void)
{
struct nwchem_pak *nwchem;

nwchem = g_malloc(sizeof(struct nwchem_pak));

nwchem->start = NULL;
nwchem->charge = NULL;
nwchem->task_theory = NULL;
nwchem->task_operation = NULL;
nwchem->library = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

return(nwchem);
}

/*************************/
/* read a system section */
/*************************/
gint read_nwchem_system(gpointer scan, struct model_pak *model)
{
gint *symbols, num_symbols, num_tokens;
gchar **buff;

// rewind
scan_put_line(scan);

while (!scan_complete(scan))
  {
  symbols = scan_get_symbols(scan, &num_symbols);

  if (num_symbols)
    {
/* process first recognized symbol */
    switch (symbols[0])
      {
      case NWCHEM_SYSTEM:
        if (num_symbols > 1)
          {
          switch (symbols[1])
            {
            case NWCHEM_CRYSTAL:
              model->periodic = 3;
              model->fractional = TRUE;
/* lengths */
              buff = scan_get_tokens(scan, &num_tokens);
              if (num_tokens > 5)
                {
                model->pbc[0] = str_to_float(buff[1]);
                model->pbc[1] = str_to_float(buff[3]);
                model->pbc[2] = str_to_float(buff[5]);
                }
              g_strfreev(buff);
/* angles */
              buff = scan_get_tokens(scan, &num_tokens);
              if (num_tokens > 5)
                {
                model->pbc[3] = D2R*str_to_float(buff[1]);
                model->pbc[4] = D2R*str_to_float(buff[3]);
                model->pbc[5] = D2R*str_to_float(buff[5]);
                }
              g_strfreev(buff);
              break;

            case NWCHEM_SURFACE:
              model->periodic = 2;
              model->fractional = TRUE;
/* lengths */
              buff = scan_get_tokens(scan, &num_tokens);
              if (num_tokens > 3)
                {
                model->pbc[0] = str_to_float(buff[1]);
                model->pbc[1] = str_to_float(buff[3]);
                }
              g_strfreev(buff);
/* angles */
              buff = scan_get_tokens(scan, &num_tokens);
              if (num_tokens > 1)
                model->pbc[5] = D2R*str_to_float(buff[1]);
              g_strfreev(buff);
              break;

            case NWCHEM_POLYMER:
              model->periodic = 1;
              model->fractional = TRUE;
/* length */
              buff = scan_get_tokens(scan, &num_tokens);
              if (num_tokens > 1)
                model->pbc[0] = str_to_float(buff[1]);
              g_strfreev(buff);
              break;

            default:
              model->periodic = 0;
              model->fractional = FALSE;
            }
          }

        break; 

      case NWCHEM_END:
        g_free(symbols);
        return(0);
      }
    g_free(symbols);
    }
  }

return(0);
}

/***************************/
/* read a geometry section */
/***************************/
#define DEBUG_READ_NWCHEM_GEOMETRY 0
gint read_nwchem_geometry(gpointer scan, struct model_pak *model)
{
gint n, zmode=0, end_count=0;
gchar **buff;
struct core_pak *core;

#if DEBUG_READ_NWCHEM_GEOMETRY
printf("read_nwchem_geometry(): start\n");
#endif

// TODO - rewrite this using symbol scanning functionality
while (!scan_complete(scan))
  {
  buff = scan_get_tokens(scan, &n);

// zmatrix?
   if (g_ascii_strncasecmp(*buff, "zmatrix", 7) == 0)
     {
     zmode = 1;
     g_strfreev(buff);
     continue;
     }
   if (g_ascii_strncasecmp(*buff, "variables", 9) == 0)
     {
     zmode = 2;
     g_strfreev(buff);
     continue;
     }
   if (g_ascii_strncasecmp(*buff, "constants", 9) == 0)
     {
     zmode = 3;
     g_strfreev(buff);
     continue;
     }

// end?
   if (n)
     {
     if (g_ascii_strncasecmp(*buff, "end", 3) == 0)
       {
       g_strfreev(buff);
/* if we processed zmatrix entries - expect 2 x end */
       if (zmode && !end_count)
         {
/* keep going until we hit a second end ... */
         end_count++;
         continue;
         }
/* exit */
       break;
       }
     }

/* main parse */
  switch (zmode)
    {
// cartesian coords
    case 0:
// can cart coord line have >4 tokens?
      if (n == 4)
        {
        if (elem_test(*buff))
          {
          core = core_new(*buff, NULL, model);
          model->cores = g_slist_prepend(model->cores, core);
          core->x[0] = str_to_float(*(buff+1));
          core->x[1] = str_to_float(*(buff+2));
          core->x[2] = str_to_float(*(buff+3));

#if DEBUG_READ_NWCHEM_GEOMETRY
printf("Added [%s] ", *buff);
P3VEC(": ", core->x);
#endif
          }
        }
      break;

// zmatrix coords
    case 1:
#if DEBUG_READ_NWCHEM_GEOMETRY
printf("zcoord: %s", scan_cur_line(scan));
#endif
      zmat_nwchem_core_add(scan_cur_line(scan), model->zmatrix);
      break;

// zmatrix vars
    case 2:
#if DEBUG_READ_NWCHEM_GEOMETRY
printf("zvar: %s", scan_cur_line(scan));
#endif
      zmat_var_add(scan_cur_line(scan), model->zmatrix);
      break;

// zmatrix consts
    case 3:
#if DEBUG_READ_NWCHEM_GEOMETRY
printf("zconst: %s", scan_cur_line(scan));
#endif
      zmat_const_add(scan_cur_line(scan), model->zmatrix);
      break;
    }

  g_strfreev(buff);
  }

if (zmode)
  {
#if DEBUG_READ_NWCHEM_GEOMETRY
printf("Creating zmatrix cores...\n");
#endif
  zmat_angle_units_set(model->zmatrix, DEGREES);
  zmat_process(model->zmatrix, model);
  }

#if DEBUG_READ_NWCHEM_GEOMETRY
printf("read_nwchem_geometry(): stop\n");
#endif

return(0);
}

/*************************/
/* process a basis block */
/*************************/
#define DEBUG_READ_NWCHEM_BASIS_LIBRARY 0
void read_nwchem_basis_library(gpointer scan, gpointer config)
{
gint i, num_tokens;
gchar *line, **buff1, **buff2;
struct nwchem_pak *nwchem;

nwchem = config_data(config);
if (!nwchem)
  return;

while (!scan_complete(scan))
  {
  line = scan_get_line(scan);

  if (g_strrstr(line, "end"))
    break;

#if DEBUG_READ_NWCHEM_BASIS_LIBRARY
printf("+%s", line);
#endif

/* cope with ";" element separator */
  buff1 = g_strsplit(line, ";", -1);
  i=0;

  if (buff1)
    {
    while (buff1[i])
      {
      buff2 = tokenize(buff1[i], &num_tokens);

      if (num_tokens > 2)
        {
#if DEBUG_READ_NWCHEM_BASIS_LIBRARY
printf("[%s] -> [%s]\n", buff2[0], buff2[2]);
#endif
        g_hash_table_insert(nwchem->library, g_strdup(buff2[0]), g_strdup(buff2[2]));
        }
      g_strfreev(buff2);
      i++;
      }
    }
  g_strfreev(buff1);
  }
}

/***********************/
/* process a task line */
/***********************/
void read_nwchem_task(gpointer scan, gpointer config, gint *symbols, gint num_symbols)
{
gint i, value;
struct nwchem_pak *nwchem;

//printf("task line = %s\n", scan_cur_line(scan));

nwchem = config_data(config);
if (!nwchem)
  return;

for (i=0 ; i<num_symbols ; i++)
  {
  value = symbols[i];

  switch (value)
    {
/* theory */
    case NWCHEM_TASK_SCF:
    case NWCHEM_TASK_DFT:
    case NWCHEM_TASK_MP2:
    case NWCHEM_TASK_DIRECT_MP2:
    case NWCHEM_TASK_RIMP2:
    case NWCHEM_TASK_CCSD:
    case NWCHEM_TASK_CCSD3:
    case NWCHEM_TASK_CCSD4:
    case NWCHEM_TASK_MCSCF:
    case NWCHEM_TASK_SELCI:
    case NWCHEM_TASK_MD:
    case NWCHEM_TASK_PSPW:
    case NWCHEM_TASK_BAND:
    case NWCHEM_TASK_TCE:
      g_free(nwchem->task_theory);
      nwchem->task_theory = g_strdup(nwchem_symbol_table[value]);
      break;

/* operation */
    case NWCHEM_TASK_ENERGY:
    case NWCHEM_TASK_GRADIENT:
    case NWCHEM_TASK_OPTIMIZE:
    case NWCHEM_TASK_SADDLE:
    case NWCHEM_TASK_HESSIAN:
    case NWCHEM_TASK_FREQUENCIES:
    case NWCHEM_TASK_PROPERTY:
    case NWCHEM_TASK_DYNAMICS:
    case NWCHEM_TASK_THERMODYNAMICS:
      g_free(nwchem->task_operation);
      nwchem->task_operation = g_strdup(nwchem_symbol_table[value]);
      break;
    }
  }
}

/*****************************/
/* process NWChem input file */
/*****************************/
gint nwchem_input_import(gpointer import)
{
gint *symbols, num_symbols, num_tokens, value;
gchar *line, **buff;
gpointer scan, config;
GString *unparsed;
struct model_pak *model=NULL; // CURRENT - will break if more than one in a file
struct nwchem_pak *nwchem;

scan = scan_new(import_fullpath(import));
if (!scan)
  return(1);

/* populate symbol table */
nwchem_scan_symbols_init(scan);

config = config_new(NWCHEM, NULL);
nwchem = config_data(config);

unparsed = g_string_new(NULL);

while (!scan_complete(scan))
  {
  symbols = scan_get_symbols(scan, &num_symbols);

  if (num_symbols)
    {
/* process first recognized symbol */
    value = symbols[0];
    switch (value)
      {
      case NWCHEM_START:
/* TODO - could this serve as title? */
        nwchem->start = parse_strip(scan_cur_line(scan));
        break;

      case NWCHEM_BASIS:
// TODO - don't want to use this for predefined basis sets
        read_nwchem_basis_library(scan, config);
        break;

      case NWCHEM_CHARGE:
        line = scan_cur_line(scan);
        buff = tokenize(line, &num_tokens);
        if (num_tokens > 1)
          nwchem->charge = g_strdup(buff[1]);
        g_strfreev(buff);
        break;

      case NWCHEM_GEOMETRY:
        if (!model)
          {
          model = model_new();
          import_object_add(IMPORT_MODEL, model, import);
          }
        read_nwchem_geometry(scan, model);
        break;

      case NWCHEM_SYSTEM:
        if (!model)
          {
          model = model_new();
          import_object_add(IMPORT_MODEL, model, import);
          }
        read_nwchem_system(scan, model);
        break;

      case NWCHEM_TASK:
        read_nwchem_task(scan, config, symbols, num_symbols);
        break;

      default:
/* add lines that have recognized symbols (but no special trigger) to unparsed */
/* this'll happen for symbols that occur in multiple locations (eg dft) */
        line = scan_cur_line(scan);
        if (line)
          g_string_append(unparsed, line);
      }

    g_free(symbols);
    }
  else
    {
/* add non-NULL lines to unparsed list */
    line = scan_cur_line(scan);
    if (line)
      g_string_append(unparsed, line);
    }
  }

config_unparsed_set(g_string_free(unparsed, FALSE), config);
import_object_add(IMPORT_CONFIG, config, import);

scan_free(scan);

import_init(import);

return(0);
}

/****************************/
/* create NWChem input file */
/****************************/
gint nwchem_input_export(gpointer import)
{
gint i, n;
gdouble x[3];
gchar *text;
gpointer config, value;
GList *list, *keys;
GSList *item;
GString *buffer;
struct model_pak *model;
struct nwchem_pak *nwchem;
struct core_pak *core;

model = import_object_nth_get(IMPORT_MODEL, 0, import);
if (!model)
  return(1);

config = import_config_get(NWCHEM, import);
if (!config)
  config = config_new(NWCHEM, NULL);

//nwchem = import_config_data_get(NWCHEM, import);
nwchem = config_data(config);
if (!nwchem)
  {
  printf("Error: couldn't obtain an nwchem config.\n");
  return(2);
  }

buffer = g_string_new(NULL);
g_string_append_printf(buffer, "#--- created by GDIS\n\n");

/* output start */
if (nwchem->start)
  g_string_append_printf(buffer, "%s\n", nwchem->start);

/* output charge */
if (nwchem->charge)
  g_string_append_printf(buffer, "charge %s\n\n", nwchem->charge);

/* output periodicity info */
switch (model->periodic)
  {
  case 3:
    g_string_append_printf(buffer, "system crystal\n");
    g_string_append_printf(buffer, "lat_a %f lat_b %f lat_c %f\n",
                           model->pbc[0], model->pbc[1], model->pbc[2]);
    g_string_append_printf(buffer, "alpha %f beta %f gamma %f\n",
                           R2D*model->pbc[3], R2D*model->pbc[4], R2D*model->pbc[5]);
    g_string_append_printf(buffer, "end\n");
    break;

  case 2:
    g_string_append_printf(buffer, "system surface\n");
    g_string_append_printf(buffer, "lat_a %f lat_b %f\n", model->pbc[0], model->pbc[1]);
    g_string_append_printf(buffer, "gamma %f\n", R2D*model->pbc[5]);
    g_string_append_printf(buffer, "end\n");
    break;

  case 1:
    g_string_append_printf(buffer, "system polymer\n");
    g_string_append_printf(buffer, "lat_a %f\n", model->pbc[0]);
    g_string_append_printf(buffer, "end\n");
    break;
  }

/* output geometry */
g_string_append_printf(buffer, "geometry\n");
n = zmat_entries_get(model->zmatrix);
if (n)
  {
  g_string_append_printf(buffer, "  zmatrix\n");
  text = zmatrix_connect_text(model->zmatrix);
  g_string_append(buffer, text);
  g_free(text);

  g_string_append_printf(buffer, "  constants\n");
  text = zmatrix_constants_text(model->zmatrix);
  g_string_append(buffer, text);
  g_free(text);

  g_string_append_printf(buffer, "  variables\n");
  text = zmatrix_variables_text(model->zmatrix);
  g_string_append(buffer, text);
  g_free(text);

  g_string_append_printf(buffer, "  end\n");
  }
else
  {
  for (item=model->cores ; item ; item=g_slist_next(item))
    {
    core = item->data;
/* cartesian coords */
    ARR3SET(x, core->x);
    vecmat(model->latmat, x); 
/* set fractional part */
    for (i=0 ; i<model->periodic ; i++)
      if (model->fractional)
        x[i] = core->x[i];
/* from NWCHEM manual, coords are always fractional in periodic directions, else cartesian */
    g_string_append_printf(buffer, "%s  %f %f %f\n", core->atom_label, x[0], x[1], x[2]);
    }
  }
g_string_append_printf(buffer, "end\n\n");

/* output basis library */
keys = g_hash_table_get_keys(nwchem->library);
if (g_list_length(keys))
  {
  g_string_append_printf(buffer, "basis\n");
  for (list=keys ; list ; list=g_list_next(list))
    {
    value = g_hash_table_lookup(nwchem->library, list->data);

    g_string_append_printf(buffer, "%s library %s\n", (gchar *) list->data, (gchar *) value);
    }
  g_string_append_printf(buffer, "end\n");
  }

/* unparsed */
// TODO - if it's just blank lines - skip
text = config_unparsed_get(config);
if (text)
  g_string_append_printf(buffer, "\n# --- unparsed start ---\n\n%s\n# --- unparsed stop ---\n\n", text);

/* last item is the task to be performed */
/* CURRENT - some files may not have a task? */
if (nwchem->task_operation)
  {
  g_string_append_printf(buffer, "task");
  if (nwchem->task_theory)
    g_string_append_printf(buffer, " %s", nwchem->task_theory);
  if (nwchem->task_operation)
    g_string_append_printf(buffer, " %s", nwchem->task_operation);
  }

/* all done, write to disk or preview mode? */
if (import_preview_get(import))
  {
/* preview mode only */
  import_preview_buffer_set(buffer->str, import);
  g_string_free(buffer, FALSE);
  }
else
  {
/* flush contents to disk */
  g_file_set_contents(import_fullpath(import), buffer->str, -1, NULL);
  g_string_free(buffer, TRUE);
  }

return(1);
}

/*******************************************/
/* read single NWChem output configuration */
/*******************************************/
#define DEBUG_READ_NWOUT 0
gint read_nwout_block(FILE *fp, struct model_pak *model)
{
gint num_tokens;
gchar **buff, line[LINELEN], *text;
GString *gstring;
GSList *clist;
struct core_pak *core;

clist = model->cores;
  
/* skip until get a 1 in first column for first coordinate */
if (fgetline(fp, line))
  return(1);
while (g_ascii_strncasecmp(line, "    1", 5) != 0)
  {
  if (fgetline(fp, line))
    return(1);
  }

buff = tokenize(line, &num_tokens);
while (num_tokens > 0)
  {
  if (clist)
    {
    core = clist->data;
    clist = g_slist_next(clist);
    }
  else
    {
    core = new_core(*(buff+1), model);
    model->cores = g_slist_append(model->cores, core);
    }

  core->x[0] = str_to_float(*(buff+3));
  core->x[1] = str_to_float(*(buff+4));
  core->x[2] = str_to_float(*(buff+5));

#if DEBUG_READ_NWOUT
P3VEC("coords: ", core->x);
#endif

/* get next line */
  g_strfreev(buff);
  if (fgetline(fp, line))
    return(2);
  buff = tokenize(line, &num_tokens);
  }

g_strfreev(buff);

/* read until get the details of the current optimisation step */  
while (g_ascii_strncasecmp(line, "@", 1) != 0)
  {
  if (fgetline(fp, line))
    return(0);
  }

buff = tokenize(line, &num_tokens);
while (g_ascii_strncasecmp(line, "@", 1) == 0)
  {
  if (g_ascii_isdigit(buff[1][0]))
    {
    text = format_value_and_units(*(buff+2), 5);
    gstring = g_string_new(text);
    g_free(text);
    g_string_append(gstring, " a.u.");
    property_add_ranked(3, "Energy", gstring->str, model);
    g_string_free(gstring, TRUE);
 
    text = format_value_and_units(*(buff+5), 5);
    gstring = g_string_new(text);
    g_free(text);
    g_string_append(gstring, " a.u./A");
    property_add_ranked(4, "RMS Gradient", gstring->str, model);
    g_string_free(gstring, TRUE);
    }
  /* get next line */
  g_strfreev(buff);
  if (fgetline(fp, line))
    return(2);
  buff = tokenize(line, &num_tokens);
  }

return(0);
}

/*******************************/
/* NWChem output frame reading */
/*******************************/
gint read_nwout_frame(FILE *fp, struct model_pak *model)
{
return(read_nwout_block(fp, model));
}

/********************************/
/* Read in a NWChem output file */
/********************************/
gint read_nwout(gchar *filename, struct model_pak *model)
{
gchar line[LINELEN];
FILE *fp;

fp = fopen(filename, "rt");
if (!fp)
  return(1);

while (!fgetline(fp, line))
  {
  if (g_strrstr(line, "Geometry \"geometry\" -> \"geometry\"") != NULL)
    {
/* go through all frames to count them */
    read_nwout_block(fp, model);
    animate_frame_store(model);
    }
  }

/* done */
g_free(model->filename);
model->filename = g_strdup(filename);

g_free(model->basename);
model->basename = parse_strip(filename);

model_prep(model);

fclose(fp);

return(0);
}
