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
#include <string.h>
#include <math.h>

#include "gdis.h"
#include "coords.h"
#include "model.h"
#include "file.h"
#include "import.h"
#include "project.h"
#include "parse.h"
#include "scan.h"
#include "graph.h"
#include "matrix.h"
#include "animate.h"
#include "reaxmd.h"
#include "reaxmd_symbols.h"
#include "interface.h"

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/* routines for processing Paolo's ReaxMD input/output files */
/*
 inpfile ="input.irx"
 topfile ="input.frx"
 coorfile="input.pdb"

 outfile     = 'output.orx'     ! communications from the code
 datfile     = 'output.erx'     ! simulation data
 trjfile     = 'output.dcd'     ! atoms coordinates
 velfile     = 'output.vcd'     ! atoms velocities
 resfile     = 'restart.rrx'    ! restart file
*/

/**********************/
/* free reaxmd object */
/**********************/
void reaxmd_free(gpointer data)
{
struct reaxmd_pak *reaxmd=data;

if (!reaxmd)
  return;

g_free(reaxmd->total_steps);
g_free(reaxmd->time_step);
g_free(reaxmd->write_energy);
g_free(reaxmd->write_trajectory);

g_free(reaxmd);
}

/************************/
/* create reaxmd object */
/************************/
gpointer reaxmd_new(void)
{
struct reaxmd_pak *reaxmd;

reaxmd = g_malloc(sizeof(struct reaxmd_pak));

reaxmd->total_steps = NULL;
reaxmd->time_step = NULL;
reaxmd->write_energy = NULL;
reaxmd->write_trajectory = NULL;

return(reaxmd);
}

/***************************************************/
/* populate the symbol table for the token scanner */
/***************************************************/
void reaxmd_symbols_init(gpointer scan)
{
gint i;

for (i=1 ; i<REAXMD_LAST ; i++)
  scan_symbol_add(scan, reaxmd_symbol_table[i], i);
}

/**********************************/
/* reading for ReaxMD input files */
/**********************************/
/* .irx */
// no coords - just control data for the run
// coords - .pdb file
// topology - .frx file
gint reaxmd_input_import(gpointer import)
{
gint *symbols, num_symbols, unknown;
gchar *line, *fullname_model;
gchar *filename, *filepath;
gpointer scan, config;
GString *unparsed;
struct reaxmd_pak *reaxmd;
struct model_pak *model;
FILE *fp;

scan = scan_new(import_fullpath(import));
if (!scan)
  return(1);

/* populate symbol table */
reaxmd_symbols_init(scan);

config = config_new(REAXMD, NULL);
reaxmd = config_data(config);

unparsed = g_string_new(NULL);

while (!scan_complete(scan))
  {
  symbols = scan_get_symbols(scan, &num_symbols);
  if (num_symbols)
    {
    unknown = FALSE;
/* process first recognized symbol */
    switch (symbols[0])
      {
      case REAXMD_NSTEPS:
        reaxmd->total_steps = parse_nth_token_dup(1, scan_cur_line(scan));
        break;

      case REAXMD_TIMESTEP:
      case REAXMD_TEMPERATURE:
      case REAXMD_THERMOSTAT:
      case REAXMD_BAROSTAT:
      case REAXMD_CUTOFF:
      case REAXMD_UPDATE:
      case REAXMD_NWRITE:
      case REAXMD_GRID:
        break;

      case REAXMD_PLUMED_FILE:
        filename = parse_nth_token_dup(1, scan_cur_line(scan));
        filepath = g_build_filename(import_path(import), filename, NULL);
//printf("Looking for: %s\n", filepath);
        if (g_file_test(filepath, G_FILE_TEST_EXISTS))
          project_job_file_append(NULL, filepath, sysenv.workspace);
        else
          gui_text_show(ERROR, "REAXMD plumed file was not found!\n"); 
        g_free(filename);
        g_free(filepath);
        break;

      default:
        unknown = TRUE;
      }
    g_free(symbols);
    }
  else
    unknown=TRUE;

  if (unknown)
    {
    line = scan_cur_line(scan);
    if (line)
      g_string_append(unparsed, line);
    }
  }

config_unparsed_set(g_string_free(unparsed, FALSE), config);
/* CURRENT - free this, until we add a GUI */
//import_object_add(IMPORT_CONFIG, config, import);
config_free(config);

/* done irx parse */
scan_free(scan);

/* NEW - attempt to load reference coordinates */
fullname_model = parse_extension_set(import_fullpath(import), "pdb");
fp = fopen(fullname_model, "rt");
if (fp)
  {
  model = model_new();
  read_pdb_block(fp, model);
  fclose(fp);
  import_object_add(IMPORT_MODEL, model, import);
  }
else
  {
  gui_text_show(WARNING, "Failed to open reference PDB model for ReaxMD.\n");
  }
g_free(fullname_model);

/* init */
import_init(import);

return(0);
}

/************************/
/* associate data reads */
/************************/
// strat is read file and add coord data to frames
// NB: this may happen multiple times (eg for velocities) so
// we need to modify exisiting and/or create new if needed
#define DEBUG_READ_DCD 0
gint reaxmd_output_coords_read(gpointer import)
{
gchar *fullname, *line;
FILE *fp;

#if DEBUG_READ_DCD
printf("Adding [dcd] data to file: %s\n", import_fullpath(import));
#endif

fullname = parse_extension_set(import_fullpath(import), "dcd");
if (!g_file_test(fullname, G_FILE_TEST_EXISTS))
  {
  gui_text_show(ERROR, "Failed to find associated .dcd file.\n");
  g_free(fullname);
  return(1);
  }

// TODO - if existing frames - assume replace mode
// otherwise create new frame at each step

/* NB: binary file data */
fp = fopen(fullname, "rb");
while (fp)
  {
  line = file_read_line(fp);
  if (!line)
    break;

  g_free(line);
  }

return(1);
}

/*****************************/
/* parse a (binary) dcd file */
/*****************************/
// buffer the standard 80 chars for fortran lines
#define REAXMD_BUFFER_LENGTH 81
#define DEBUG_REAXMD_PARSE_COORDS 0
gint reaxmd_parse_coordinates(FILE *fp, struct model_pak *model)
{
gint i, j, k;
gint nframes, nstart, nsanc, nstep, i4[4], namin, icell, i9[9];
gint ntitle, natom;
gchar dummy[REAXMD_BUFFER_LENGTH];
gchar car4[5], *time;
gfloat *xbuff, *ybuff, *zbuff, delta;
gdouble a,b,c,cosa,cosb,cosg, t;
GSList *list;
struct core_pak *core;

if (!fp)
  return(1);

if (!model)
  return(1);

// store initial PDB as 1st frame
  property_replace("Time", "0.0 ps", model);
  animate_frame_store(model);

// TODO - byte ordering check (see file_siesta.c)

// see also http://local.wasp.uwa.edu.au/~pbourke/dataformats/fortran/

// read header
/*
    real*4             :: delta
    integer*4          :: nframes, nstart, nsanc, nstep, i4(4), namin, icell, i9(9)
    integer*4          :: ntitle
    character(len=4)   :: car4
    character(len=80)  :: car(10)

    if(first)then
      read(uinp) car4, nframes, nstart, nsanc, nstep, i4, namin, delta, icell, i9
      read(uinp) ntitle, (car(j),j=1,ntitle)
      read(uinp) natom
      allocate(x(natom),y(natom),z(natom))
      first=.false.
    endif

    if(icell==1)then
      read(uinp,end=111)cell(1),cell(6),cell(2),cell(5),cell(4),cell(3)
      cell(4) = dacos(cell(4))
      cell(5) = dacos(cell(5))
      cell(6) = dacos(cell(6))
      call get_hmat(cell,h)
    else
      h=0.d0
      h(1,1)=1.d0
      h(2,2)=1.d0
      h(3,3)=1.d0
    endif

    read(uinp,end=111)x
    read(uinp,end=111)y
    read(uinp,end=111)z

printf("gint = [%d] bytes\n", sizeof(gint));
printf("gchar = [%d] bytes\n", sizeof(gchar));
printf("gfloat = [%d] bytes\n", sizeof(gfloat));
printf("gdouble = [%d] bytes\n", sizeof(gdouble));
*/

/* header line 1 */
i = READ_RECORD;
i = fread(car4, sizeof(gchar), 4, fp);
car4[4] = '\0';

#if DEBUG_REAXMD_PARSE_COORDS
printf("TODO - check [%s] == CORD\n", car4);
#endif

i = fread(&nframes, sizeof(gint), 1, fp);

i = fread(&nstart, sizeof(gint), 1, fp);
i = fread(&nsanc, sizeof(gint), 1, fp);
i = fread(&nstep, sizeof(gint), 1, fp);
i = fread(i4, sizeof(gint), 4, fp);
i = fread(&namin, sizeof(gint), 1, fp);
i = fread(&delta, sizeof(gfloat), 1, fp);
i = fread(&icell, sizeof(gint), 1, fp);
i = fread(i9, sizeof(gint), 9, fp);
i = READ_RECORD;

/* header line 2 */
i = READ_RECORD;
i = fread(&ntitle, sizeof(gint), 1, fp);
for (j=0 ; j<ntitle ; j++)
  {
  i = fread(dummy, sizeof(gchar), 80, fp);
  }
i = READ_RECORD;

/* header line 3 */
i = READ_RECORD;
i = fread(&natom, sizeof(gint), 1, fp);
i = READ_RECORD;

#if DEBUG_REAXMD_PARSE_COORDS
printf("nframes=%d, nstart=%d, nstep=%d, nsanc=%d, time step (delta) = %f\n",
        nframes, nstart, nstep, nsanc, delta);

printf("icell=%d\n", icell);
printf("namin=%d\n", namin);
printf("natom=%d\n", natom);
printf("Expected atoms = %d\n", g_slist_length(model->cores));
#endif

/*
if (icell)
  {
  i = READ_RECORD;
  i = fread(&a, sizeof(gdouble), 1, fp);
  i = fread(&cosg, sizeof(gdouble), 1, fp);
  i = fread(&b, sizeof(gdouble), 1, fp);
  i = fread(&cosb, sizeof(gdouble), 1, fp);
  i = fread(&cosa, sizeof(gdouble), 1, fp);
  i = fread(&c, sizeof(gdouble), 1, fp);
  i = READ_RECORD;

printf("[%f][%f][%f] : [%f][%f][%f]\n", a,b,c,cosa,cosb,cosg);

  }
else
  {
// lattice vectors
printf("TODO - process lattice vectors\n");
  }
*/

xbuff = g_malloc(natom * sizeof(gfloat));
ybuff = g_malloc(natom * sizeof(gfloat));
zbuff = g_malloc(natom * sizeof(gfloat));

// CURRENT - assume no existing frames, so we're creating
// this means subsequent property reads (eg for energy) must replace NOT create from scratch
//printf("frames = %d\n", g_list_length(model->animate_list));

/* read the frames */
for (j=0 ; j<nframes ; j++)
  {
/* parse the periodic boundary conditions */
// cell PBC
// clearly, a,b,c,alpha,beta,gamma is too obvious an order
  if (icell)
    {
    i = READ_RECORD;
    i = fread(&a, sizeof(gdouble), 1, fp);
    i = fread(&cosg, sizeof(gdouble), 1, fp);
    i = fread(&b, sizeof(gdouble), 1, fp);
    i = fread(&cosb, sizeof(gdouble), 1, fp);
    i = fread(&cosa, sizeof(gdouble), 1, fp);
    i = fread(&c, sizeof(gdouble), 1, fp);
    i = READ_RECORD;

    model->pbc[0] = a;
    model->pbc[1] = b;
    model->pbc[2] = c;
    model->pbc[3] = acos(cosa);
    model->pbc[4] = acos(cosb);
    model->pbc[5] = acos(cosg);

#if DEBUG_REAXMD_PARSE_COORDS
if (j < 3)
  {
  printf("pbc %d : [%f][%f][%f] : [%f][%f][%f]\n", j,a,b,c,cosa,cosb,cosg);
  }
#endif

// NB: must force frame store to generate latmat
    model->construct_pbc = FALSE;
    }
  else
    {
// lattice vectors
printf("TODO - process lattice vectors\n");
    model->construct_pbc = TRUE;
    }

/* NB: coords in cartesians */

/* x array */
  i = READ_RECORD;
  i = fread(xbuff, sizeof(gfloat), natom, fp);
  i = READ_RECORD;

/* y array */
  i = READ_RECORD;
  i = fread(ybuff, sizeof(gfloat), natom, fp);
  i = READ_RECORD;

/* z array */
  i = READ_RECORD;
  i = fread(zbuff, sizeof(gfloat), natom, fp);
  i = READ_RECORD;

/* overwrite current cores and then store */
  list = model->cores;
  for (k=0 ; k<natom ; k++)
    {
    if (list)
      {
      core = list->data;

      core->x[0] = xbuff[k];
      core->x[1] = ybuff[k];
      core->x[2] = zbuff[k];

      list = g_slist_next(list);
      }
    else
      {
// shouldn't happen if we disallow a read with natoms != g_slist_length(model->cores)
      break;
      }
    }

// calculate the time value for this frame
  t = (nstart + j*nsanc) * delta;

  time = g_strdup_printf("%.1f ps", t);

// store the current frame
  property_replace("Time", time, model);
  animate_frame_store(model);

  g_free(time);
/* DEBUG */
/*
  if (j == 0)
    {
    gint k;
 
    for (k=0 ; k<100 ; k++)
      {
printf("[%f , %f , %f]\n", xbuff[k], ybuff[k], zbuff[k]);
      }
    }
*/
  }

g_free(xbuff);
g_free(ybuff);
g_free(zbuff);

return(0);
}

/***************************************/
/* parse velocities (also a dcd file?) */
/***************************************/
#define DEBUG_REAXMD_PARSE_VELOCITIES 0
gint reaxmd_parse_velocities(FILE *fp, struct model_pak *model)
{
gint i, j, k;
gint nframes, nstart, nsanc, nstep, i4[4], namin, icell, i9[9];
gint ntitle, natom;
gchar dummy[REAXMD_BUFFER_LENGTH];
gchar car4[5];
gfloat *xbuff, *ybuff, *zbuff, delta;
gdouble a,b,c,cosa,cosb,cosg;
gdouble v[3];
gpointer frame;

if (!fp)
  return(1);

if (!model)
  return(1);

// store initial PDB as 1st frame
/*
  property_replace("Time", "0.0 ps", model);
  animate_frame_store(model);
*/

// TODO - byte ordering check (see file_siesta.c)

/* header line 1 */
i = READ_RECORD;
i = fread(car4, sizeof(gchar), 4, fp);
car4[4] = '\0';

#if DEBUG_REAXMD_PARSE_VELOCITIES
printf("TODO - check [%s] == VEL\n", car4);
#endif

i = fread(&nframes, sizeof(gint), 1, fp);
i = fread(&nstart, sizeof(gint), 1, fp);
i = fread(&nsanc, sizeof(gint), 1, fp);
i = fread(&nstep, sizeof(gint), 1, fp);
i = fread(i4, sizeof(gint), 4, fp);
i = fread(&namin, sizeof(gint), 1, fp);
i = fread(&delta, sizeof(gfloat), 1, fp);
i = fread(&icell, sizeof(gint), 1, fp);
i = fread(i9, sizeof(gint), 9, fp);
i = READ_RECORD;

/* header line 2 */
i = READ_RECORD;
i = fread(&ntitle, sizeof(gint), 1, fp);
for (j=0 ; j<ntitle ; j++)
  {
  i = fread(dummy, sizeof(gchar), 80, fp);
  }
i = READ_RECORD;

/* header line 3 */
i = READ_RECORD;
i = fread(&natom, sizeof(gint), 1, fp);
i = READ_RECORD;

#if DEBUG_REAXMD_PARSE_VELOCITIES
printf("nframes=%d, nstart=%d, nstep=%d, nsanc=%d, time step (delta) = %f\n",
        nframes, nstart, nstep, nsanc, delta);

printf("icell=%d\n", icell);
printf("namin=%d\n", namin);
printf("natom=%d\n", natom);
printf("Expected atoms = %d\n", g_slist_length(model->cores));
#endif

xbuff = g_malloc(natom * sizeof(gfloat));
ybuff = g_malloc(natom * sizeof(gfloat));
zbuff = g_malloc(natom * sizeof(gfloat));

// CURRENT - assume no existing frames, so we're creating
// this means subsequent property reads (eg for energy) must replace NOT create from scratch
//printf("frames = %d\n", g_list_length(model->animate_list));

/* read the frames */
for (j=0 ; j<nframes ; j++)
  {
/* parse the periodic boundary conditions */
// cell PBC
// clearly, a,b,c,alpha,beta,gamma is too obvious an order
  if (icell)
    {
    i = READ_RECORD;
    i = fread(&a, sizeof(gdouble), 1, fp);
    i = fread(&cosg, sizeof(gdouble), 1, fp);
    i = fread(&b, sizeof(gdouble), 1, fp);
    i = fread(&cosb, sizeof(gdouble), 1, fp);
    i = fread(&cosa, sizeof(gdouble), 1, fp);
    i = fread(&c, sizeof(gdouble), 1, fp);
    i = READ_RECORD;

#if DEBUG_REAXMD_PARSE_VELOCITIES
if (j < 3)
  {
  printf("pbc %d : [%f][%f][%f] : [%f][%f][%f]\n", j,a,b,c,cosa,cosb,cosg);
  }
#endif

// NB: must force frame store to generate latmat
//    model->construct_pbc = FALSE;
    }
  else
    {
// lattice vectors
//printf("TODO - process lattice vectors\n");
//    model->construct_pbc = TRUE;
    }

/* NB: coords in cartesians */

/* x array */
  i = READ_RECORD;
  i = fread(xbuff, sizeof(gfloat), natom, fp);
  i = READ_RECORD;

/* y array */
  i = READ_RECORD;
  i = fread(ybuff, sizeof(gfloat), natom, fp);
  i = READ_RECORD;

/* z array */
  i = READ_RECORD;
  i = fread(zbuff, sizeof(gfloat), natom, fp);
  i = READ_RECORD;

/* CURRENT */
  frame = g_list_nth_data(model->animate_list, j);

/* add velocities to list */
  for (k=0 ; k<natom ; k++)
    {
    v[0] = xbuff[k];
    v[1] = ybuff[k];
    v[2] = zbuff[k];

    if (frame)
      animate_frame_velocity_add(v, frame);
    else
      printf("Warning - problem parsing reaxmd velocity file.\n");
    }

// calculate the time value for this frame
/*
  t = (nstart + j*nsanc) * delta;
  time = g_strdup_printf("%.1f ps", t);
  property_replace("Time", time, model);
  animate_frame_store(model);
  g_free(time);
*/

/* DEBUG */
/*
  if (j == 0)
    {
    gint k;
 
    for (k=0 ; k<100 ; k++)
      {
printf("[%f , %f , %f]\n", xbuff[k], ybuff[k], zbuff[k]);
      }
    }
*/
  }

g_free(xbuff);
g_free(ybuff);
g_free(zbuff);

return(0);
}

/***********************************/
/* process properties results file */
/***********************************/
/* TODO - this is a bit more general than reaxmd ... ie column data for plotting */
#define DEBUG_REAXMD_PARSE_PROPERTIES 0
gint reaxmd_parse_properties(FILE *fp, struct model_pak *model, gpointer import)
{
gint i, j, k, n=1, time_column=0, num_tokens, num_properties;
gdouble frame_time=0.0, time, start, stop, value;
gchar *line, *label, **buff;
const gchar *xlabel, *ylabel;
GSList *item, *list;
GArray **property_array;

#if DEBUG_REAXMD_PARSE_PROPERTIES
printf("file_reaxmd(): parsing energy file...\n");
#endif

/* checks */
if (!fp)
  return(FALSE);
if (!import)
  return(FALSE);

/* setup internal frame counter */
if (model)
  {
  n = g_list_length(model->animate_list);
  i=0;
  frame_time = str_to_float(animate_frame_property_get(i, "Time", model));
  }

// NB: file_read_line() skips comments by default, so it'll auto skip the header line
file_skip_comment(FALSE);

// process first line for column locations of time, energy, etc ...
line = file_read_line(fp);

// error handling
if (!line)
  {
  return(FALSE);
  }
if (strlen(line) < 2)
  {
  return(FALSE);
  }

buff = g_strsplit(&line[1], "|", -1);
list = NULL;
if (buff)
  {
  j=0;
  while (buff[j])
    {
    label = g_strstrip(buff[j]);
    if (strlen(label))
      {
      if (g_strrstr(label, "Time"))
        {
        time_column = j;
        }
      list = g_slist_prepend(list, g_strdup(label));
      }
    j++;
    }
  }
g_strfreev(buff);
g_free(line);
line = file_read_line(fp);
list = g_slist_reverse(list);

#if DEBUG_REAXMD_PARSE_PROPERTIES
for (item=list ; item ; item=g_slist_next(item))
  {
  printf("[%s]\n", (gchar *) item->data);
  }
printf("Allocating %d property arrays, with start size %d\n", g_slist_length(list), n);
#endif

// NB: exclude the first - time
/* init arrays for properties */
num_properties = g_slist_length(list);
property_array = g_malloc(num_properties * sizeof(GArray));
for (k=0 ; k<num_properties ; k++)
  {
  property_array[k] = g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
  }

/* iterate over all output data lines */
while (line)
  {
  buff = tokenize(line, &num_tokens);

  if (num_tokens > time_column)
    {
    time = str_to_float(buff[time_column]);

/* NEW - grab all values for property graphs */
    for (j=0 ; j<num_tokens ; j++)
      {
      if (j < num_properties)
        {
        value = str_to_float(buff[j]);
        g_array_append_val(property_array[j], value);
        }
      }

/* if output data time matches internal frame frame - store */
    if (model)
      {
      if (time >= frame_time)
        {
/* add current output data to the internally stored frame */
        item = list;
        for (j=0 ; j<num_tokens ; j++)
          {
          if (item)
            {
//printf("[%d][%s][%s]\n", i, (gchar *) item->data, buff[j]);
// add to actual model property list (reference frame only)
// leave this out now, since we're auto constructing graphs anyway
/*
            if (!i)
              {
              property_add_ranked(j, item->data, buff[j], model);
              }
*/

            animate_frame_property_put(i, item->data, buff[j], model);
            }
          else
            break;

          item = g_slist_next(item);
          }

/* update the internal frame count */
        i++;
        frame_time = str_to_float(animate_frame_property_get(i, "Time", model));
        }
      }
    }

  g_strfreev(buff);

  g_free(line);
  line = file_read_line(fp);
  }

/* iterate property arrays and construct graphs */
xlabel = g_slist_nth_data(list, time_column);
for (j=0 ; j<num_properties ; j++)
  {
// don't graph time column
  if (j != time_column)
    {
    gpointer data, graph;

    k = property_array[j]->len;
    data = g_array_free(property_array[j], FALSE);

    start = g_array_index(property_array[time_column],gdouble,0);
    stop = g_array_index(property_array[time_column],gdouble,k-1);

#if DEBUG_REAXMD_PARSE_PROPERTIES
printf("Building graph for: %s with %d data points and ", (gchar *) g_slist_nth_data(list, j), k);
printf("range: %f - %f\n", start, stop);
#endif

/* add graph if no ! prefix */
    ylabel = g_slist_nth_data(list, j);
    if (!g_strrstr(ylabel, "!"))
      {
/* create and attach graph object */
      label = g_strndup(ylabel, 8);
      graph = graph_new(label);
      g_free(label);
      graph_set_data(k, data, start, stop, graph);
      graph_x_label_set(xlabel, graph);
      graph_y_label_set(ylabel, graph);
      import_object_add(IMPORT_GRAPH, graph, import);
      }
    }
  }

g_array_free(property_array[time_column], TRUE);
g_free(property_array);

/* cleanup */
free_slist(list);

return(0);
}

/*
 outfile     = 'output.orx'     ! communications from the code
 datfile     = 'output.erx'     ! simulation data
 trjfile     = 'output.dcd'     ! atoms coordinates
 velfile     = 'output.vcd'     ! atoms velocities
 resfile     = 'restart.rrx'    ! restart file
*/
/***************************************************/
/* process reaxmd output and associated data files */
/***************************************************/
#define DEBUG_REAXMD_OUTPUT 0
gint reaxmd_output_import(gpointer import)
{
gint status=0;
gchar *fullname_model, *fullname_energy, *fullname_coords, *fullname_velos;
struct model_pak *model=NULL;
FILE *fp_m, *fp_c, *fp_e, *fp_v;

if (!import)
  return(1);

fullname_model = parse_extension_set(import_fullpath(import), "pdb");
fullname_coords = parse_extension_set(import_fullpath(import), "dcd");
fullname_energy = parse_extension_set(import_fullpath(import), "erx");
fullname_velos = parse_extension_set(import_fullpath(import), "vrx");

fp_m = fopen(fullname_model, "rt");
fp_e = fopen(fullname_energy, "rt");
fp_c = fopen(fullname_coords, "rb");
fp_v = fopen(fullname_velos, "rb");

// bare minimum parse
if (fp_m)
  {
  model = model_new();
  read_pdb_block(fp_m, model);
  fclose(fp_m);
  }
else
  {
  status = 1;
  gui_text_show(ERROR, "Failed to open reference PDB model for ReaxMD.\n");
  goto reaxmd_output_import_cleanup;
  }

// main extra data (coords)
if (fp_c)
  {
  reaxmd_parse_coordinates(fp_c, model);
  fclose(fp_c);
  }

// optional extra data (velocities)
if (fp_v)
  {
  reaxmd_parse_velocities(fp_v, model);
  fclose(fp_v);
  }

// optional extra data (energy, temp, etc)
// CURRENT - only read corresponding values for which we have coord frames
if (fp_e)
  {
  reaxmd_parse_properties(fp_e, model, import);
  fclose(fp_e);
  }

// init
if (model)
  import_object_add(IMPORT_MODEL, model, import);

import_init(import);

#if DEBUG_REAXMD_OUTPUT
animate_debug(model);
#endif

// done
reaxmd_output_import_cleanup:

g_free(fullname_model);
g_free(fullname_coords);
g_free(fullname_energy);
g_free(fullname_velos);

return(status);
}

