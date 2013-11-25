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

#include "gdis.h"
#include "coords.h"
#include "error.h"
#include "file.h"
#include "parse.h"
#include "matrix.h"
#include "spatial.h"
#include "model.h"
#include "interface.h"
#include "animate.h"

#define DEBUG_READ_XSF_BLOCK 0

gint set_true_false();
gint set_energy_units();
gint set_temperature_units();
gint set_length_units();
gint set_force_units();
gint set_pressure_units();
gint set_time_units();
gint set_mominert_units();

gint print_file_energy_units();
gint print_file_mass_units();
gint print_file_length_units();
gint print_file_time_units();
gint print_file_temperature_units();
gint print_file_pressure_units();
gint print_file_force_units();
gint print_file_mominert_units();

enum {XSF_TYPE, XSF_LATTICE_VECTORS, XSF_COORDS};

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/****************/
/* file writing */
/****************/
gint write_xsf(gchar *filename, struct model_pak *model)
{
gdouble x[3];
GSList *list;
struct core_pak *core;
FILE *fp;

/* checks */
g_return_val_if_fail(model != NULL, 1);
g_return_val_if_fail(filename != NULL, 2);

/* open the file */
fp = fopen(filename,"wt");
if (!fp)
  return(3);

/* print header */
fprintf(fp,"DIM-GROUP\n %d  1\n PRIMVEC\n", (model->periodic));

    fprintf(fp,"%15.10f %15.10f %15.10f\n",
                model->latmat[0], model->latmat[3], model->latmat[6]);
    fprintf(fp,"%15.10f %15.10f %15.10f\n",
                model->latmat[1], model->latmat[4], model->latmat[7]);
    fprintf(fp,"%15.10f %15.10f %15.10f\n",
                model->latmat[2], model->latmat[5], model->latmat[8]);

fprintf(fp,"PRIMCOORD\n %d  1 \n", g_slist_length(model->cores));
for (list=model->cores ; list ; list=g_slist_next(list))
  {
  core = list->data;
  if (core->status & DELETED)
    continue;

  ARR3SET(x, core->x);
  vecmat(model->latmat, x);
  fprintf(fp,"%d    %14.9f  %14.9f  %14.9f\n",
              elements[core->atom_code].number, x[0], x[1], x[2]);
  }

fclose(fp);
return(0);
}

/*********************************/
/* process a lattice vector line */
/*********************************/
void xsf_parse_lattice(const gchar *line, gint n,  struct model_pak *model)
{
gint num_tokens;
gchar **buff;

g_assert(line != NULL);

buff = tokenize(line, &num_tokens);
if (num_tokens > 2)
  {
  model->latmat[0+n] = str_to_float(*(buff+0));
  model->latmat[3+n] = str_to_float(*(buff+1));
  model->latmat[6+n] = str_to_float(*(buff+2));
  }

model->construct_pbc = TRUE;
model->periodic = 3;

g_strfreev(buff);
}

/******************************/
/* main xsf data block reader */
/******************************/
gint read_xsf_block(FILE *fp, struct model_pak *model)
{
struct core_pak *core;
gchar **buff, *line;
gint count,  num_tokens,  block_type=333;
GSList *clist=NULL;
g_assert(fp != NULL);
g_assert(model != NULL);
model->fractional = FALSE;

  clist = model->cores;

for (;;)
  {
  line = file_read_line(fp);
     if (!line)
        return (1);
  buff = tokenize(line, &num_tokens);

/* get next line if blank */
     if (!buff)
        continue;
     
     if (g_strrstr(*buff, "ANIMSTEPS") != NULL)
        model->num_frames=(gint) str_to_float(*(buff+1));

     if (g_strrstr(line, "PRIMVEC") != NULL && g_strrstr(line, "RECIP") == NULL)
        block_type = XSF_LATTICE_VECTORS;
       
     if (g_strrstr(line, "PRIMCOORD") != NULL)
        block_type = XSF_COORDS;

#if DEBUG_READ_XSF_BLOCK
printf("processing block [type %d]\n", block_type);
printf("line:  %s\n", line);
#endif
 if (block_type==1 ||block_type==2)
  {
    count = 0; 
    for (;;)
      {
      line = file_read_line(fp);
      if (!line )
      return (2);
  buff = tokenize(line, &num_tokens);

/* get next line if blank */
     if (!buff)
        continue;

      switch (block_type)
        {
        case XSF_TYPE:
          break;

        case XSF_LATTICE_VECTORS:
	  { 
          xsf_parse_lattice(line, count,  model);
		if (count==2)
	    { block_type = 333;goto xsf_done_line;}
	  }
          break;

        case XSF_COORDS:
// process the 1st line after PRIMCOORD header 
	{
	if (count == 0 )
	  {	
		if (num_tokens)
		model->num_atoms = (gint) str_to_float(*(buff+0));
		g_free(line);
		g_strfreev(buff);
		line = file_read_line(fp);
                buff = tokenize(line, &num_tokens);
	  }  
             core=NULL;
             if (clist)
              {
              core = clist->data;
              clist = g_slist_next(clist);
              g_free(core->atom_label);
              if (str_is_float(*(buff+0)))
              core->atom_label =elements[(int)str_to_float(*(buff+0))].symbol;
              else
              core->atom_label = g_strdup(*buff);
              elem_init(core, model);
              }
             else
               {
              if (str_is_float(*(buff+0)))
              core = new_core(elements[(int) str_to_float(*(buff+0))].symbol, model);
              else
              core = new_core(*(buff+0), model);
              model->cores = g_slist_append(model->cores, core);
               }
          core->x[0] = str_to_float(*(buff+1));
          core->x[1] = str_to_float(*(buff+2));
          core->x[2] = str_to_float(*(buff+3));

#if DEBUG_READ_XSF_BLOCK
printf("processing coord:%s, %s\n",*(buff+0), *(buff+1));
#endif
          g_free(line); 
          g_strfreev(buff);
        if (count == model->num_atoms-1)
	return (0);
	}
          break;

          default:
         goto xsf_done_line;
          break;
        } 
      count++;
      } 
  }
  xsf_done_line:;
/* loop cleanup */
  g_strfreev(buff);
  g_free(line);
  } 
  return (0);
}

/**************************/
/*reading (a)xsf input files*/
/**************************/
gint read_xsf(gchar *filename, struct model_pak *model)
{
FILE *fp;

/* checks */
g_return_val_if_fail(model != NULL, 1);
g_return_val_if_fail(filename != NULL, 2);

fp = fopen(filename, "rt");
if (!fp)
  return(3);

// loop while there's data
while (read_xsf_block(fp, model) == 0)
  animate_frame_store(model);

//printf("3D crystals with PRIMVEC and PRIMCOORD are readable from axsf as well");

model_prep(model);

/* model setup */
g_free(model->filename);
model->filename = g_strdup(filename);
g_free(model->basename);
model->basename = g_strdup("model");

return(0);
}

/*********************/
/* xsf frame read    */
/*********************/
gint read_xsf_frame(FILE *fp, struct model_pak *model)
{
read_xsf_block(fp, model);
return(0);
}
