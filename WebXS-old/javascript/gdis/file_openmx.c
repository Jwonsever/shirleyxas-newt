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
#include "zmatrix.h"
#include "zmatrix_pak.h"
#include "model.h"
#include "interface.h"

#define DEBUG_READ_BLOCK 0

enum {OPENMX_DEFAULT, OPENMX_LATTICE_VECTORS, OPENMX_COORDS};

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/****************/
/* file writing */
/****************/
gint write_openmx_input(gchar *filename, struct model_pak *model)
{
gint i,j;
gdouble x[3];
GSList *list, *clist, *species_list;
struct core_pak *core;
struct species_pak *species_data;
FILE *fp;

/* checks */
g_return_val_if_fail(model != NULL, 1);
g_return_val_if_fail(filename != NULL, 2);
species_list = fdf_species_build(model);

/* open the file */
fp = fopen(filename,"wt");
if (!fp)
  return(3);

/* print header */
fprintf(fp, "\n#\n# File Name\n#\n\n");
fprintf(fp, "System.CurrrentDirectory         ./    # default=./\nSystem.Name                      %s\nlevel.of.stdout                   1    # default=1 (1-3)\nlevel.of.fileout                  1    # default=1 (0-2)\nDATA.PATH            ../DFT_DATA/      # default=../DFT_DATA/\n", model->basename);
/* init the normal, ghost, and overall species lists */
species_list = fdf_species_build(model);
fprintf(fp, "\n#\n# Definition of Atomic Species\n#\n\n");
fprintf(fp, "Species.Number       %d\n", g_slist_length(species_list));
fprintf(fp, "<Definition.of.Atomic.Species\n");
i=1;
for (list=species_list ; list ; list=g_slist_next(list))
  {
  species_data = list->data;

  fprintf(fp, "  %s  %s-s2p1d1f1  %s_LDA\n", species_data->label, species_data->label, species_data->label);
  i++;
  }
fprintf(fp, "Definition.of.Atomic.Species>\n");
/* write the species (unique atom types) block */
i=1;
fprintf(fp, "\n#\n# Atoms\n#\n\n");
fprintf(fp, "Atoms.Number        %d\n", g_slist_length(model->cores));
/* write the lattice data */
if (model->periodic)
  {
  fprintf(fp, "Atoms.UnitVectors.Unit             Ang # Ang|AU\n");
 /* NB: siesta matrices are transposed wrt gdis */
    fprintf(fp, "<Atoms.UnitVectors\n");
    fprintf(fp,"%15.10f %15.10f %15.10f\n",
                model->latmat[0], model->latmat[3], model->latmat[6]);
    fprintf(fp,"%15.10f %15.10f %15.10f\n",
                model->latmat[1], model->latmat[4], model->latmat[7]);
    fprintf(fp,"%15.10f %15.10f %15.10f\n",
                model->latmat[2], model->latmat[5], model->latmat[8]);
    fprintf(fp, "Atoms.UnitVectors>\n");
   
 
    }
/* write the atoms - for surfaces (and isolated molecules), coords must be cartesian */
if (model->fractional)
  fprintf(fp, "Atoms.SpeciesAndCoordinates.Unit   FRAC # Ang|AU|FRAC\n");
else
  fprintf(fp, "Atoms.SpeciesAndCoordinates.Unit   Ang # Ang|AU|FRAC\n");

fprintf(fp, "<Atoms.SpeciesAndCoordinates\n");  
j=1;
for (clist=model->cores ; clist ; clist=g_slist_next(clist))
  {
  core = clist->data;
  if (core->status & DELETED)
    continue;

  ARR3SET(x, core->x);

/* NB: want fractional if 3D periodic, otherwise cartesian */
  if (!model->fractional)
    vecmat(model->latmat, x);

/* find corresponding number of this element in the species block */
  species_data = NULL;
  for (list=species_list ; list ; list=g_slist_next(list))
    {
    species_data = list->data;

    if (g_ascii_strcasecmp(core->atom_label, species_data->label) == 0)
      {
      if (core->ghost)
        {
        if (species_data->number == -core->atom_code)
          break;
        }
      else
        {
        if (species_data->number == core->atom_code)
          break;
        }
      }
    i++;
    }

/* SDF - not sure what you're doing here clima, but species_data might be undefined */
/* so I put this if in to ensure it's non NULL (since the printf needs it defined) */
/* found? */
  if (species_data)
      {fprintf(fp,"%4d  %3s  %14.9f  %14.9f  %14.9f 0.0 0.0 0.0 0.0 0.0 0.0 1\n", j, species_data->label, x[0], x[1], x[2]);
        j++;
      }
//  else
//    printf("write_fdfmx() error: bad species type.\n");
 }
fprintf(fp, "Atoms.SpeciesAndCoordinates>\n");

free_slist(species_list);

/*
 * OpenMX options
 */
fprintf(fp, "\n#\n# SCF or Electronic System\n#\n\nscf.XcType                  LDA        # LDA|LSDA-CA|LSDA-PW|GGA-PBE\nscf.SpinPolarization        off        # On|Off|NC\nscf.ElectronicTemperature  100.0       # default=300 (K)\nscf.energycutoff           120.0       # default=150 (Ry)\nscf.maxIter                 60         # default=40\nscf.EigenvalueSolver        cluster    # DC|GDC|Cluster|Band\nscf.Kgrid                  1 1 1       # means n1 x n2 x n3\nscf.Mixing.Type           rmm-diis     # Simple|Rmm-Diis|Gr-Pulay|Kerker|Rmm-Diisk\nscf.Init.Mixing.Weight     0.30        # default=0.30 \nscf.Min.Mixing.Weight      0.001       # default=0.001 \nscf.Max.Mixing.Weight      0.400       # default=0.40 \nscf.Mixing.History          7          # default=5\nscf.Mixing.StartPulay       4          # default=6\nscf.criterion             1.0e-8       # default=1.0e-6 (Hartree) \nscf.lapack.dste            dstevx      # dstevx|dstedc|dstegr,default=dstevx\n\n");
fprintf(fp, "\n#\n# 1D FFT  \n#\n\n1DFFT.NumGridK             900         # default=900\n1DFFT.NumGridR             900         # default=900\n1DFFT.EnergyCutoff        2500.0       # default=3600 (Ry)\n");
fprintf(fp, "\n#\n# Orbital Optimization\n#\n\norbitalOpt.Method           off        # Off|Unrestricted|Restricted\norbitalOpt.InitCoes     Symmetrical    # Symmetrical|Free\norbitalOpt.initPrefactor   0.1         # default=0.1\norbitalOpt.scf.maxIter      15         # default=12\norbitalOpt.MD.maxIter        7         # default=5\norbitalOpt.per.MDIter       20         # default=1000000\norbitalOpt.criterion      1.0e-4       # default=1.0e-4 (Hartree/borh)\n\n");
fprintf(fp, "\n#\n# output of contracted orbitals\n#\n\nCntOrb.fileout               off       # on|off, default=off\nNum.CntOrb.Atoms             1         # default=1\n<Atoms.Cont.Orbitals\n 1\nAtoms.Cont.Orbitals>\n\n");
fprintf(fp, "\n#\n# SCF Order-N\n#\n\norderN.HoppingRanges        6.0        # default=5.0 (Ang) \norderN.NumHoppings           2         # default=2\n\n");
fprintf(fp, "\n#\n# MD or Geometry Optimization\n#\n\nMD.Type                     opt       # Nomd|Opt|NVE|NVT_VS|NVT_NH\nMD.Opt.DIIS.History          4\nMD.Opt.StartDIIS             5         # default=5\nMD.maxIter                   1         # default=1\nMD.TimeStep                 1.0        # default=0.5 (fs)\nMD.Opt.criterion         1.0e-5        # default=1.0e-4 (Hartree/bohr)\n\n");
fprintf(fp, "\n#\n# restart using a restart file, *.rst  \n#\n\nscf.restart                 off        # on|off,default=off\n\n");
fprintf(fp, "\n#\n# MO output\n#\n\nMO.fileout                   off       # on|off\nnum.HOMOs                     1        # default=1\nnum.LUMOs                     1        # default=1\nMO.Nkpoint                    1        # default=1 \n<MO.kpoint\n  0.0  0.0  0.0\nMO.kpoint>\n\n");
fprintf(fp, "\n#\n# DOS and PDOS\n#\n\nDos.fileout                  off       # on|off, default=off\nDos.Erange               -10.0  10.0   # default = -20 20 \nDos.Kgrid                  1  1  1     # default = Kgrid1 Kgrid2 Kgrid3\n\n");
fprintf(fp, "\n#\n# output Hamiltonian and overlap\n#  \n\nHS.fileout                   off       # on|off, default=off\n");
fprintf(fp, "\n");
fclose(fp);
return(0);
}

/*********************************/
/* process a lattice vector line */
/*********************************/
void openmx_parse_lattice(const gchar *line, gint n, gdouble scale, struct model_pak *model)
{
gint num_tokens;
gchar **buff;

g_assert(line != NULL);

buff = tokenize(line, &num_tokens);
if (num_tokens > 2)
  {
  model->latmat[0+n] = scale * str_to_float(*(buff+0));
  model->latmat[3+n] = scale * str_to_float(*(buff+1));
  model->latmat[6+n] = scale * str_to_float(*(buff+2));
  }
#if DEBUG_READ_BLOCK
    printf("lattice: %f %f %f\n", model->latmat[0+n],model->latmat[3+n],model->latmat[6+n]);
#endif

model->construct_pbc = TRUE;
model->periodic = 3;

g_strfreev(buff);
}

/***********************************/
/* process a coordinate block line */
/***********************************/
void openmx_parse_coords(const gchar *line, GSList *species_list, gdouble scale, struct model_pak *model)
{
gint  num_tokens;
gchar **buff;
struct core_pak *core;

g_assert(line != NULL);

buff = tokenize(line, &num_tokens);

if (num_tokens > 4)
  {
  core = new_core(*(buff+1), model);
  model->cores = g_slist_append(model->cores, core);

  core->x[0] = str_to_float(*(buff+2));
  core->x[1] = str_to_float(*(buff+3));
  core->x[2] = str_to_float(*(buff+4));
  VEC3MUL(core->x, scale);
  }
#if DEBUG_READ_BLOCK
                        printf("processing coord:%s",line);
                        printf("%f %f %f\n",core->x[0],core->x[1],core->x[2]);

#endif

g_strfreev(buff);
}


/******************************/
/* main fdf data block reader */
/******************************/
gint read_fdfmx_block(FILE *fp, struct model_pak *model)
{
gint count, num_tokens, block_type=333;
gchar **buff, *line;
gdouble scale_lattice = 1.0, scale_coords = 1.0;
GSList *species_list=NULL;

g_assert(fp != NULL);
g_assert(model != NULL);

for (;;)
  {
  line = file_read_line(fp);

/* terminate if NULL returned */
  if (!line)
    break;

  buff = tokenize(line, &num_tokens);

/* get next line if blank */
  if (!num_tokens)
    continue;

/* NEW - block/enblock processing */
  if (g_strrstr(line,"<") != NULL)
    { block_type = 333;
      if (g_strrstr(g_ascii_strdown(line,-1), "<atoms.unitvectors") != NULL)
        block_type = OPENMX_LATTICE_VECTORS;
        //model->construct_pbc = TRUE;
        //model->periodic = 3;
      if (g_strrstr(g_ascii_strdown(line,-1), "<atoms.speciesandcoordinates") != NULL)
        block_type = OPENMX_COORDS;

#if DEBUG_READ_BLOCK
printf("processing block [type %d]\n", block_type);
#endif
    count = 0;
    for (;;)
        {
        line = file_read_line(fp);
        if (!line)  return (2);
        buff = tokenize(line, &num_tokens);
        /* get next line if blank */
        if (!buff) continue;

        if (g_strrstr(line,">"))
            {
#if DEBUG_READ_BLOCK
            printf("end block [type %d]\n", block_type);
#endif
            goto mx_done_line;
            }

        switch (block_type)
            {
            case OPENMX_LATTICE_VECTORS:
                openmx_parse_lattice(line, count, scale_lattice, model);
                if (count==2) { 
                    block_type = 333;
                    goto  mx_done_line;
                    }
                break;

            case OPENMX_COORDS:
                openmx_parse_coords(line, species_list, scale_coords, model);
                break;

            default:
#if DEBUG_READ_BLOCK
printf("Unrecognized block [%i] encountered.\n", block_type);
#endif
                break;
            }
        count++;
        }
/* done block processing - get next line */
    continue;
    }

/* cartesian/fractional */
  if (g_strrstr(g_ascii_strdown(line,-1), "atoms.speciesandcoordinates.unit") != NULL)
          {
          model->fractional = FALSE;
          if (g_strrstr(*(buff+1), "AU"))   scale_coords = AU2ANG;
          if (g_strrstr(*(buff+1), "FRAC"))  model->fractional = TRUE;
#if DEBUG_READ_BLOCK
printf("scale_coords = %f\n", scale_coords);
#endif
          }

  if (g_strrstr(g_ascii_strdown(line,-1), "atoms.unitvectors.unit") != NULL)
          {
          if (g_strrstr(*(buff+1), "AU"))   scale_lattice = AU2ANG;
#if DEBUG_READ_BLOCK
printf("scale_lattice = %f\n", scale_lattice);
#endif
          }

  if (g_strrstr(g_ascii_strdown(line,-1), "atoms.number")!= NULL)
    {
    if (num_tokens > 1)
    model->num_atoms =(gint) str_to_float(*(buff+1)); 
#if DEBUG_READ_BLOCK
printf("atoms.number = %i\n", model->num_atoms);
#endif
    }

  if (g_strrstr(g_ascii_strdown(line,-1), "system.name")!= NULL)
    {
    if (num_tokens > 1)
      {
      g_free(model->basename);
      model->basename = g_strdup(*(buff+1));
      }
    }


/* loop cleanup */
  mx_done_line:;
  g_strfreev(buff);
  g_free(line);
  }
/* free the species list */
free_slist(species_list);
return 0;
}

//////////////////////////////////////////////////////////////////////
/**************************/
/* fdf input file reading */
/**************************/
gint read_openmx_input(gchar *filename, struct model_pak *model)
{
gint i;
gchar *text;
FILE *fp;

/* checks */
g_return_val_if_fail(model != NULL, 1);
g_return_val_if_fail(filename != NULL, 2);

fp = fopen(filename, "rt");
if (!fp)
  return(3);

error_table_clear();

/* terry's mod */
read_fdfmx_block(fp, model);

/* check cores */
model->cores = g_slist_reverse(model->cores);
i = g_slist_length(model->cores);
if (model->num_atoms != i)
  {
  text = g_strdup_printf("Inconsistancy reading %s: expected %d cores, but found %d.\n",
                          filename, model->num_atoms, i);
  gui_text_show(ERROR, text);
  g_free(text);
  }

/* model setup */
//strcpy(model->filename, filename);
g_free(model->filename);
model->filename = g_strdup(filename);
g_free(model->basename);
model->basename = parse_strip(filename);

model_prep(model);
error_table_print_all();
fclose(fp);
return(0);
}

