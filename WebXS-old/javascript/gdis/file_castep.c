/*
Copyright (C) 2011 by Andrew Lloyd Rohl and Sean Fleming

andrew@ivec.org
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

#include "gdis.h"
#include "coords.h"
#include "file.h"
#include "import.h"
#include "scan.h"
#include "parse.h"
#include "model.h"
#include "matrix.h"
#include "interface.h"
#include "animate.h"

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/*******************************************/
/* read single CASTEP output configuration */
/*******************************************/
#define DEBUG_READ_CASTEP_OUT 0
gint read_castep_out_block(FILE *fp, struct model_pak *model)
{
gint i;
gint num_tokens;
gint no_grad = 0;
gint last_frame = FALSE;
gdouble scale, grad, max_grad = 0.0, rms_grad = 0.0;
gchar **buff, line[LINELEN], *text, *label;
GString *title, *grad_string;
GSList *clist;
struct core_pak *core;

clist = model->cores;

/* NEW - cope with non-angstrom units */
scale = coords_scale_get(model);

/* read to end of iteration */
while (TRUE)
  {
  if (fgetline(fp, line))
    {
    gui_text_show(ERROR, "unexpected end of file reading to end of iteration\n");
    return(2);
    }

  if (g_ascii_strncasecmp(line, "==========", 10) == 0)
      break;
      
  if (g_ascii_strncasecmp(line, "Writing model to", 16) == 0)
      break;
  
/* read cell vectors? */
  if (g_strrstr(line, "Real Lattice") != NULL)
    {
    /* read in cell vectors */
    /* NB: gdis wants transposed matrix */
    for (i=0; i<3; i++)
      {
      if (fgetline(fp, line))
        {
        gui_text_show(ERROR, "unexpected end of file reading cell vectors\n");
        return(2);
        }
      buff = tokenize(line, &num_tokens);
      model->latmat[0+i] = scale * str_to_float(*(buff+0));
      model->latmat[3+i] = scale * str_to_float(*(buff+1));
      model->latmat[6+i] = scale * str_to_float(*(buff+2));
      g_strfreev(buff);
      }
    }


gdouble* shifts[2000];
int shift_counter=0;
gdouble* aniso[2000];
gdouble* asym[2000];
gdouble* cq[2000];
gdouble* efgasym[2000];

/* read coordinates */
  if (g_strrstr(line, "x  Element    Atom        Fractional coordinates of atoms  x") != NULL)
    {

int skip=0;
  for (skip=0; skip<3; skip++)
    if (fgetline(fp, line))
      {
      gui_text_show(ERROR, "unexpected end of file skipping to coordinates\n");
      return(2);
      }

    buff = tokenize(line, &num_tokens);
    while (num_tokens == 7)
      {
      if (clist)
        {
        core = clist->data;
        clist = g_slist_next(clist);
        }
      else
        {
/*
	gchar* atom_name=g_strdup(*(buff+1));
	g_strlcat(atom_name,*(buff+2),5);
*/
/*	g_strlcat(atom_name,*(buff+2),4);*/

/* SDF - simplify */
label = g_strconcat(*(buff+1), *(buff+2), NULL);
        core = core_new(*(buff+1),label, model);
        model->cores = g_slist_append(model->cores, core);
g_free(label);

        }
      core->x[0] = str_to_float(*(buff+3));
      core->x[1] = str_to_float(*(buff+4));
      core->x[2] = str_to_float(*(buff+5));

      shifts[shift_counter]=&core->atom_nmr_shift;
      aniso[shift_counter]=&core->atom_nmr_aniso;
      asym[shift_counter]=&core->atom_nmr_asym;
      cq[shift_counter]=&core->atom_nmr_cq;
      efgasym[shift_counter]=&core->atom_nmr_efgasym;
      shift_counter++;

      #if DEBUG_READ_CASTEP_OUT
      printf("new coords %f %f %f\n", core->x[0],  core->x[1], core->x[2]);
      #endif

    /* get next line */
      g_strfreev(buff);
      if (fgetline(fp, line))
        {
        gui_text_show(ERROR, "unexpected end of file reading coordinates\n");
        return(2);
        }
      buff = tokenize(line, &num_tokens);
      }
    g_strfreev(buff);
    clist = model->cores;
    }

  if (g_strrstr(line, "|  Species   Ion    Iso(ppm)") != NULL)
  {
    if (fgetline(fp, line))
      {
      gui_text_show(ERROR, "unexpected end of file skipping to coordinates\n");
      return(2);
      }
    buff = tokenize(line, &num_tokens);
    int i=0;
    while ((num_tokens == 9) || (num_tokens == 7))
    {
/*	if (i<=shift_counter)*/
	*shifts[i]=str_to_float(*(buff+3));
	*aniso[i]=str_to_float(*(buff+4));
	*asym[i]=str_to_float(*(buff+5));
	if (num_tokens == 9) { //read EFG parameters if present
		*cq[i]=str_to_float(*(buff+6));
		*efgasym[i]=str_to_float(*(buff+7));
	}
	i++;
    /* get next line */
      g_strfreev(buff);
      if (fgetline(fp, line))
        {
        gui_text_show(ERROR, "unexpected end of file reading coordinates\n");
        return(2);
        }
      buff = tokenize(line, &num_tokens);
      }
    g_strfreev(buff);
    }
/* EFG only computation*/
  if (g_strrstr(line, " |  Species   Ion             Cq(MHz)") != NULL)
  {
    if (fgetline(fp, line))
      {
      gui_text_show(ERROR, "unexpected end of file skipping to coordinates\n");
      return(2);
      }
    buff = tokenize(line, &num_tokens);
    int i=0;
    while (num_tokens == 6)
    {
	*cq[i]=str_to_float(*(buff+3));
	*efgasym[i]=str_to_float(*(buff+4));
	i++;
    /* get next line */
      g_strfreev(buff);
      if (fgetline(fp, line))
        {
        gui_text_show(ERROR, "unexpected end of file reading coordinates\n");
        return(2);
        }
      buff = tokenize(line, &num_tokens);
      }
    g_strfreev(buff);
    }

  if (g_ascii_strncasecmp(line, "Final energy", 12) == 0)
    {
    buff = g_strsplit(line, "=", 2);
    text = format_value_and_units(*(buff+1), 5);
    property_add_ranked(3, "Energy", text, model);
    g_free(text);
    model->castep.energy = str_to_float(*(buff+1));
    model->castep.have_energy = TRUE;
    g_strfreev(buff);
    }
  if (g_strrstr(line, "Final Enthalpy") != NULL)
    {
    last_frame = TRUE;
    model->castep.min_ok = TRUE;
    buff = g_strsplit(line, "=", 2);
    text = format_value_and_units(*(buff+1), 5);
    property_add_ranked(3, "Energy", text, model);
    g_free(text);
    model->castep.energy = str_to_float(*(buff+1));
    model->castep.have_energy = TRUE;
    g_strfreev(buff);
    }

  /* gradients */
  if (g_strrstr(line, "* -") != NULL)
    {
    for (i=0; i<2; i++)
      {
      if (fgetline(fp, line))
        {
        gui_text_show(ERROR, "unexpected end of file skipping to gradients\n");
        return(2);
        }
      }
    /* read gradients */
    if (fgetline(fp, line))
      {
      gui_text_show(ERROR, "unexpected end of file reading gradients\n");
      return(2);
      }
    buff = tokenize(line, &num_tokens);
    while (num_tokens == 7)
      {
      for (i=0; i<3; i++)
        {
        grad = str_to_float(*(buff+3+i));
        no_grad++;
        rms_grad = grad*grad;
        max_grad = MAX(fabs(grad), max_grad);
      }
      /* get next line */
      g_strfreev(buff);
      if (fgetline(fp, line))
        {
        gui_text_show(ERROR, "unexpected end of file reading gradients\n");
        return(2);
        }
      buff = tokenize(line, &num_tokens);
      }
    g_strfreev(buff);
    model->castep.rms_grad = sqrt(rms_grad/no_grad);
    model->castep.max_grad = max_grad;
    grad_string = g_string_new("");
    g_string_append_printf(grad_string, "%.5f %s", model->castep.rms_grad, property_lookup("Gradient Units", model));
    property_add_ranked(4, "RMS Gradient", grad_string->str, model);
    g_string_free(grad_string, TRUE); 

    #if DEBUG_READ_CASTEP_OUT
    printf("RMS gradient: %f Max gradient: %f\n", model->castep.rms_grad, model->castep.max_grad);
    #endif
    }
      
  }
 
g_free(model->title);
title = g_string_new("");
g_string_append_printf(title, "E");
g_string_append_printf(title, "(DFT)");
/* TODO read energy units from CASTEP file */
g_string_append_printf(title, " = %.5f eV", model->castep.energy);
g_string_append_printf(title, ", grad = %.5f", model->castep.rms_grad);
model->title = g_strdup(title->str);
g_string_free(title, TRUE); 

return(0);
}

/*******************************/
/* CASTEP output frame reading */
/*******************************/
gint read_castep_out_frame(FILE *fp, struct model_pak *model)
{
    return(read_castep_out_block(fp, model));
}

/********************************/
/* Read in a CASTEP output file */
/********************************/
gint read_castep_out(gchar *filename, struct model_pak *model)
{
gint frame;
gchar **buff, line[LINELEN];
FILE *fp;

if (g_strrstr(filename, ".cell") != NULL)
  return read_castep_cell(filename, model);

fp = fopen(filename, "rt");
if (!fp)
  return(1);

model->construct_pbc = TRUE;
model->periodic = 3;
model->fractional = TRUE;

frame=0;
while (!fgetline(fp, line))
  {
  /* find relevent units */
/* NB: there is an inv_length unit */
  if (g_strrstr(line, " length unit"))
    {
    buff = g_strsplit(line, ":", 2);
    if (*(buff+1))
      g_strstrip(*(buff+1));

    if (g_ascii_strncasecmp("nm", buff[1], 2) == 0)
      model->coord_units = NANOMETERS;
    if (g_ascii_strncasecmp("a0", buff[1], 2) == 0)
      model->coord_units = BOHR;
    if (g_ascii_strncasecmp("bohr", buff[1], 4) == 0)
      model->coord_units = BOHR;
    }

  if (g_strrstr(line, "force unit") != NULL)
    {
    buff = g_strsplit(line, ":", 2);
    if (*(buff+1))
      g_strstrip(*(buff+1));
    property_add_ranked(0, "Gradient Units", *(buff+1), model);
    g_strfreev(buff);
    }

  if (g_ascii_strncasecmp(line, " type of calculation", 20) == 0)
    {
    buff = g_strsplit(line, ":", 2);
    if (*(buff+1))
      g_strstrip(*(buff+1));
    if (g_ascii_strncasecmp(*(buff+1), "geometry optimization", 21) == 0)
      property_add_ranked(2, "Calculation", "Optimisation", model);
    else if (g_ascii_strncasecmp(*(buff+1), "single point energy", 19) == 0)
      property_add_ranked(2, "Calculation", "Single Point", model);
    else
      property_add_ranked(2, "Calculation", *(buff+1), model);
    g_strfreev(buff);
    }

  if (g_ascii_strncasecmp(line, " using functional", 17) == 0)
    {
    buff = g_strsplit(line, ":", 2);
    if (*(buff+1))
      g_strstrip(*(buff+1));
    if (g_ascii_strncasecmp(*(buff+1), "Perdew Burke Ernzerhof", 22) == 0)
      property_add_ranked(7, "Functional", "PBE", model);
    else if (g_ascii_strncasecmp(*(buff+1), "Perdew Wang (1991)", 18) == 0)
      property_add_ranked(7, "Functional", "PW91", model);
    else
      property_add_ranked(7, "Functional", *(buff+1), model);
    g_strfreev(buff);
    }
 
  /* read coordinates */
  if ((g_strrstr(line, "Unit Cell") != NULL) || (g_strrstr(line, "Cell Contents") != NULL))
    {
/* go through all frames to count them */
    read_castep_out_block(fp, model);
/* store */
    animate_frame_store(model);
    }
  }


/* TODO - add graphs of pressure/energy etc */


/* done */
g_free(model->filename);
model->filename = g_strdup(filename);

g_free(model->basename);
model->basename = parse_strip(filename);

model_prep(model);

fclose(fp);

return(0);
}


/* VEZ write in CASTEP cell file */

gint write_castep_cell(gchar *filename, struct model_pak *model)
{
gdouble c;
gdouble x[3];
GSList *list;
struct core_pak *core;
gint i = 0;
FILE *fp;

/* checks */
g_return_val_if_fail(model != NULL, 1);
g_return_val_if_fail(filename != NULL, 2);

/* open the file */
fp = fopen(filename, "wt");
if (!fp)
  return(3);

if (model->periodic)
  {
  if (model->periodic == 2)
    /* saving a surface as a 3D model - make depth large enough to fit everything */
    c =  2.0*model->rmax;
  else
    c = model->pbc[2];

  if ((180.0*model->pbc[3]/PI<0.0) || (180.0*model->pbc[3]/PI>180.0))
	gui_text_show(ERROR, "Cell angle alpha is not in 0-180 degree range, please correct manually\n");
  if ((180.0*model->pbc[4]/PI<0.0) || (180.0*model->pbc[4]/PI>180.0))
	gui_text_show(ERROR, "Cell angle beta is not in 0-180 degree range, please correct manually\n");
  if ((180.0*model->pbc[5]/PI<0.0) || (180.0*model->pbc[5]/PI>180.0))
	gui_text_show(ERROR, "Cell angle gamma is not in 0-180 degree range, please correct manually\n");


  fprintf(fp,"%%BLOCK LATTICE_ABC\n");
  fprintf(fp,"%f  %f   %f\n",model->pbc[0], model->pbc[1], c);
  fprintf(fp,"%f  %f   %f\n",fabs(180.0*model->pbc[3]/PI),fabs(180.0*model->pbc[4]/PI),fabs(180.0*model->pbc[5]/PI));
  fprintf(fp,"%%ENDBLOCK LATTICE_ABC\n\n");
  

  /*
    The ATOM records - fractional for a periodic model
  */

  fprintf(fp,"%%BLOCK POSITIONS_FRAC\n");

  for (list=model->cores ; list ; list=g_slist_next(list))
    {
      core = list->data;

      ARR3SET(x, core->x);

      i++;
      fprintf(fp,"%s   %f   %f   %f\n",elements[core->atom_code].symbol,x[0], x[1], x[2]);
    }

  fprintf(fp,"%%ENDBLOCK POSITIONS_FRAC\n\n");
  } 
 else 
   {
     fprintf(fp,"%%BLOCK POSITIONS_ABS\n");

     for (list=model->cores ; list ; list=g_slist_next(list))
       {
	 core = list->data;

	 ARR3SET(x, core->x);
	 vecmat(model->latmat, x);
	 i++;
	 fprintf(fp,"%s   %f   %f   %f\n",elements[core->atom_code].symbol,x[0], x[1], x[2]);

       }

     fprintf(fp,"%%ENDBLOCK POSITIONS_ABS\n\n");
   }

/* fixed atoms */
gint constrains_counter=1;
gint atomic_counter[200];
int qq;
for (qq=0; qq<200; qq++)
	atomic_counter[qq]=1;

fprintf(fp,"%%BLOCK IONIC_CONSTRAINTS\n");
for (list=model->cores ; list ; list=g_slist_next(list))
  {
	core = list->data;
	if (core->ghost) {
		fprintf(fp,"%d %s %d 1 0 0\n",constrains_counter, elements[core->atom_code].symbol, atomic_counter[core->atom_code]);
		constrains_counter++;
		fprintf(fp,"%d %s %d 0 1 0\n",constrains_counter, elements[core->atom_code].symbol, atomic_counter[core->atom_code]);
		constrains_counter++;
		fprintf(fp,"%d %s %d 0 0 1\n",constrains_counter, elements[core->atom_code].symbol, atomic_counter[core->atom_code]);
		constrains_counter++;
	}
	atomic_counter[core->atom_code]++;
  }
 fprintf(fp,"%%ENDBLOCK IONIC_CONSTRAINTS\n\n");

 if (model->periodic)
  {
    fprintf(fp,"KPOINTS_MP_SPACING 0.05\n\nFIX_ALL_CELL : TRUE\n\nFIX_COM : FALSE\n\nSYMMETRY_GENERATE\n");
  }
 else
   {
    fprintf(fp,"KPOINTS_MP_GRID 1 1 1\n\nFIX_ALL_CELL : TRUE\n\nFIX_COM : FALSE\n\nSYMMETRY_GENERATE\n");
   }

fclose(fp);
return(0);
}

/* From here and below is VZ routines to read .cell files */
/*************************************************/
/* read a .cell block into the model structure     */
/*************************************************/
#define DEBUG_READ_CELL_BLOCK 0
gint read_cell_block(gpointer scan, struct model_pak *data)
{
gchar *line, **buf;
struct core_pak *core;
GSList *clist;
gint num_tokens;
gfloat unit_mul;

clist = data->cores;

/* loop while there's data */
while (!scan_complete(scan))
  {
  line = scan_get_line(scan);
  if (!line)
    break;

/* read lattice */
  if (g_strrstr(g_strup(line), "%BLOCK LATTICE_ABC"))
    {
    data->periodic = 3;

    unit_mul = 1.0;
    buf = scan_get_tokens(scan, &num_tokens);

    if( num_tokens == 1)
      {
	if( g_ascii_strcasecmp(*(buf+0),"ang") == 0 )
	  {
	    unit_mul = 1.0;
	  } else if (g_ascii_strcasecmp(*(buf+0),"bohr") == 0 )
	  {
	    unit_mul = 0.529177;
	  } else 
	  {
	    gui_text_show(ERROR, "Unknown units for cell length\n");
	    return(2);
	  }
	buf = scan_get_tokens(scan, &num_tokens);
      }
    
    if (num_tokens > 2)
      {
      data->pbc[0] = unit_mul*str_to_float(*(buf+0));
      data->pbc[1] = unit_mul*str_to_float(*(buf+1));
      data->pbc[2] = unit_mul*str_to_float(*(buf+2));
      }
    g_strfreev(buf);

    buf = scan_get_tokens(scan, &num_tokens);
    if (num_tokens > 2)
      {
      data->pbc[3] = str_to_float(*(buf+0)) * D2R;
      data->pbc[4] = str_to_float(*(buf+1)) * D2R;
      data->pbc[5] = str_to_float(*(buf+2)) * D2R;
      }
    g_strfreev(buf);
    continue;
    }

  if (g_strrstr(g_strup(line), "%BLOCK LATTICE_CART"))
    {
    int i;

    /* read in cell vectors */

    data->periodic = 3;

    unit_mul = 1.0;
    buf = scan_get_tokens(scan, &num_tokens);

    if( num_tokens == 1)
      {
	if( g_ascii_strcasecmp(*(buf+0),"ang") == 0 )
	  {
	    unit_mul = 1.0;
	  } else if (g_ascii_strcasecmp(*(buf+0),"bohr") == 0 )
	  {
	    unit_mul = 0.529177;
	  } else 
	  {
	    gui_text_show(ERROR, "Unknown units for cell vectors\n");
	    return(2);
	  }
	buf = scan_get_tokens(scan, &num_tokens);
      }
    /* NB: gdis wants transposed matrix */
    i = 0;
    for (i=0; i<3; i++)
      {
      if( num_tokens < 3)
	{
	   gui_text_show(ERROR, "unexpected end of line reading cell vectors\n");
	   return(2);
	}

	data->latmat[0+i] = unit_mul*str_to_float(*(buf+0));
	data->latmat[3+i] = unit_mul*str_to_float(*(buf+1));
	data->latmat[6+i] = unit_mul*str_to_float(*(buf+2));
	g_strfreev(buf);
	buf = scan_get_tokens(scan, &num_tokens);
      }
    data->construct_pbc = 1;
    matrix_lattice_init(data);
    continue;
    }

/* read coords */
  if (g_strrstr(g_strup(line), "%BLOCK POSITIONS_"))
    {
    unit_mul = 1.0;
    if (g_strrstr(g_strup(line), "_ABS") != 0)
      {
	data->fractional = FALSE;

	buf = scan_get_tokens(scan, &num_tokens);

	if( num_tokens == 1)
	  {
	    if( g_ascii_strcasecmp(*(buf+0),"ang") == 0 )
	      {
		unit_mul = 1.0;
	      } else if (g_ascii_strcasecmp(*(buf+0),"bohr") == 0 )
	      {
		unit_mul = 0.529177;
	      } else 
	      {
		gui_text_show(ERROR, "Unknown units for atomic co-ordinates\n");
		return(2);
	      }
	  } else
	    scan_put_line(scan);
      }
    else 
      data->fractional = TRUE;

    while (!scan_complete(scan))
      {
      buf = scan_get_tokens(scan, &num_tokens);
      if (buf)
        {
	if (g_strrstr(g_strup(*buf), "END"))
          {
          g_strfreev(buf);
          break;
          }
        }
      if (num_tokens > 3)
        {
        if (clist)
          {
          core = clist->data;
          clist = g_slist_next(clist);
          }
        else
          {
          core = core_new(*buf, *buf, data);
          data->cores = g_slist_append(data->cores, core);
          }
        core->x[0] = str_to_float(*(buf+1));
        core->x[1] = str_to_float(*(buf+2));
        core->x[2] = str_to_float(*(buf+3));
        }
      g_strfreev(buf);
      } 
    }
  }
return(-1);
}

/******************/
/* cell reading   */
/******************/
gint read_castep_cell(gchar *filename, struct model_pak *data)
{
gpointer scan;

/* checks */
g_return_val_if_fail(data != NULL, 1);
g_return_val_if_fail(filename != NULL, 2);

/* initialisers */
data->periodic = 0;

scan = scan_new(filename);
while (read_cell_block(scan, data) == 0)
  {
  animate_frame_store(data);
  }
scan_free(scan);


/* model setup */
model_prep(data);

g_free(data->filename);
data->filename = g_strdup(filename);

g_free(data->basename);
data->basename = parse_strip(filename);

return(0);
}

/*****************************/
/* castep cell parsing setup */
/*****************************/
enum {CASTEP_BLOCK_START, CASTEP_BLOCK_STOP, CASTEP_LAST};
gchar *castep_symbol_table[] = {
"\%block", "\%endblock",
"dummy"
};
void castep_symbols_init(gpointer scan)
{
gint i;

for (i=0 ; i<CASTEP_LAST ; i++)
  scan_symbol_add(scan, castep_symbol_table[i], i);
}

/********************************/
/* process lattice matrix block */
/********************************/
void castep_parse_lattice(gpointer scan, struct model_pak *model)
{
gint n=0, num_symbols, num_tokens;
gint *symbols;
gchar *line, **buff;

//printf("castep_parse_lattice():\n");

/* check header */
line = scan_cur_line(scan);
if (g_strrstr(line, "cart") || g_strrstr(line, "CART"))
  model->construct_pbc = TRUE;
else
  model->construct_pbc = FALSE;

model->periodic = 3;

while (!scan_complete(scan))
  {
/* FIXME - lot of (repeated) code for a simple 1 symbol check */
/* FIXME - implement shortcut routine */
  symbols = scan_get_symbols(scan, &num_symbols);
  if (num_symbols)
    {
    if (symbols[0] == CASTEP_BLOCK_STOP)
      {
      g_free(symbols);
      break;
      }
    }
  g_free(symbols);

//printf("%s", scan_cur_line(scan));

  buff = tokenize(scan_cur_line(scan), &num_tokens);
  if (model->construct_pbc)
    {
    if (num_tokens == 3 && n<3)
      {
      model->latmat[0+n] = str_to_float(*(buff+0));
      model->latmat[3+n] = str_to_float(*(buff+1));
      model->latmat[6+n] = str_to_float(*(buff+2));
      n++;
      }
    }
  else
    {
    if (num_tokens == 3)
      {
      switch (n)
        {
        case 0:
//printf("PBC lengths: [%s][%s][%s]\n", buff[0], buff[1], buff[2]);
          model->pbc[0] = str_to_float(buff[0]);
          model->pbc[1] = str_to_float(buff[1]);
          model->pbc[2] = str_to_float(buff[2]);
          n++;
          break;

        case 1:
//printf("PBC angles: [%s][%s][%s]\n", buff[0], buff[1], buff[2]);
          model->pbc[3] = D2R * str_to_float(buff[0]);
          model->pbc[4] = D2R * str_to_float(buff[1]);
          model->pbc[5] = D2R * str_to_float(buff[2]);
          n++;

        default:
          n++;
        }
      }
    }
  g_strfreev(buff);
  }
}

/********************************/
/* process atom positions block */
/********************************/
void castep_parse_positions(gpointer scan, struct model_pak *model)
{
gint *symbols;
gint num_symbols, num_tokens;
gchar **buff, *line;
struct core_pak *core;

/* check header for cart/frac */
line = scan_cur_line(scan);
if (g_strrstr(line, "frac") || g_strrstr(line, "FRAC"))
  model->fractional = TRUE;

/* loop until block end */
while (!scan_complete(scan))
  {
  symbols = scan_get_symbols(scan, &num_symbols);
  if (num_symbols)
    {
    if (symbols[0] == CASTEP_BLOCK_STOP)
      {
      g_free(symbols);
      break;
      }
    }
  g_free(symbols);

/* add new atom */
  buff = tokenize(scan_cur_line(scan), &num_tokens);
  if (num_tokens > 3)
    {
    if (elem_test(buff[0]))
      {
      core = core_new(buff[0], NULL, model);
      model->cores = g_slist_prepend(model->cores, core);
      core->x[0] = str_to_float(buff[1]);
      core->x[1] = str_to_float(buff[2]);
      core->x[2] = str_to_float(buff[3]);
      }
    }
  g_strfreev(buff);
  }

model->cores = g_slist_reverse(model->cores);
}

/**********************/
/* castep cell import */
/**********************/
gint castep_import_cell(gpointer import)
{
gint *symbols, num_symbols, unknown;
gchar *line;
gpointer scan, config;
GString *unparsed;
struct model_pak *model;

scan = scan_new(import_fullpath(import));
if (!scan)
  return(1);

/* populate symbol table */
castep_symbols_init(scan);

/* TODO - line parsing below is fairly standard (cf reaxmd) can we unify? */
config = config_new(CASTEP, NULL);

model = model_new();

unparsed = g_string_new(NULL);

while (!scan_complete(scan))
  {
  symbols = scan_get_symbols(scan, &num_symbols);
  unknown = TRUE;
  if (num_symbols)
    {
/* process first recognized symbol */
    switch (symbols[0])
      {
      case CASTEP_BLOCK_START:
        line = scan_cur_line(scan);
        if (g_strrstr(line, "latt") || g_strrstr(line, "LATT"))
          {
          castep_parse_lattice(scan, model);
          unknown=FALSE;
          }
        if (g_strrstr(line, "pos") || g_strrstr(line, "POS"))
          {
          castep_parse_positions(scan, model);
          unknown=FALSE;
          }
        break;
      }
    g_free(symbols);
    }

 if (unknown)
    {
    line = scan_cur_line(scan);
    if (line)
      g_string_append(unparsed, line);
    }
  }

config_unparsed_set(g_string_free(unparsed, FALSE), config);
import_object_add(IMPORT_CONFIG, config, import);

/* TODO - only add if we found some atoms? */
import_object_add(IMPORT_MODEL, model, import);

scan_free(scan);

import_init(import);

return(0);
}

/**********************/
/* castep cell export */
/**********************/
gint castep_export_cell(gpointer import)
{
gdouble x[3];
gpointer config;
GSList *list;
GString *buffer;
struct model_pak *model;
struct core_pak *core;

buffer = g_string_new(NULL);

model = import_object_nth_get(IMPORT_MODEL, 0, import);
if (model)
  {
  if (model->periodic == 3)
    {
/* output lattice */
    g_string_append_printf(buffer, "%%block LATTICE_CART\nang\n");

    g_string_append_printf(buffer, "%f %f %f\n",
                           model->latmat[0], model->latmat[3], model->latmat[6]);
    g_string_append_printf(buffer, "%f %f %f\n",
                           model->latmat[1], model->latmat[4], model->latmat[7]);
    g_string_append_printf(buffer, "%f %f %f\n",
                           model->latmat[2], model->latmat[5], model->latmat[8]);

    g_string_append_printf(buffer, "%%endblock LATTICE_CART\n");

/* separator */
    g_string_append_printf(buffer, "\n");
    }
  else
    {
/* FIXME - if not 3D periodic - force cartesian output */
    model->fractional = FALSE;
    }

/* output coords */
  if (model->fractional)
    {
    g_string_append_printf(buffer, "%%block POSITIONS_FRAC\n");
    for (list=model->cores ; list ; list=g_slist_next(list))
      {
      core = list->data;
      g_string_append_printf(buffer, "%s  %f %f %f\n",
                             core->atom_label, core->x[0], core->x[1], core->x[2]);
      }
    g_string_append_printf(buffer, "%%endblock POSITIONS_FRAC\n");
    }
  else
    {
    g_string_append_printf(buffer, "%%block POSITIONS_ABS\n");
    for (list=model->cores ; list ; list=g_slist_next(list))
      {
      core = list->data;
      ARR3SET(x, core->x);
      vecmat(model->latmat, x);
      g_string_append_printf(buffer, "%s  %f %f %f\n", core->atom_label, x[0], x[1], x[2]);
      }
    g_string_append_printf(buffer, "%%endblock POSITIONS_ABS\n");
    }
  }

config = import_config_get(CASTEP, import);
if (config)
  {
//printf("unparsed:\n%s\n", config_unparsed_get(config));
  g_string_append(buffer, config_unparsed_get(config));
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

return(0);
}

