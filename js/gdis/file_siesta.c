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
#include "import.h"
#include "parse.h"
#include "scan.h"
#include "matrix.h"
#include "zmatrix.h"
#include "zmatrix_pak.h"
#include "model.h"
#include "interface.h"
#include "animate.h"
#include "graph.h"
#include "siesta.h"
#include "surface.h"


enum {SIESTA_DEFAULT, SIESTA_SPECIES, SIESTA_LATTICE_PARAMETERS, SIESTA_LATTICE_VECTORS,
      SIESTA_COORDS, SIESTA_ZMATRIX, SIESTA_COORDS_FORMAT};

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/*****************************************/
/* set defaults for siesta configuration */
/*****************************************/
void siesta_init(gpointer data)
{
gint i;
struct siesta_pak *siesta = data;

g_assert(siesta != NULL);

siesta->num_atoms = 0;
siesta->energy = siesta->max_grad = 0.0;
siesta->have_energy = siesta->have_max_grad = 0.0;
siesta->epot_min = 0.0;
siesta->epot_max = 0.0;
siesta->eden_scale = 1.0;
siesta->eden = NULL;
siesta->edos = NULL;
siesta->epot = NULL;
siesta->grid_rho = NULL;
siesta->grid_drho = NULL;
siesta->grid_ldos = NULL;
siesta->grid_vh = NULL;
siesta->grid_vt = NULL;

siesta->freq_disp_str = NULL;
siesta->eigen_values = NULL;
siesta->eigen_xyz_atom_mat = NULL;

siesta->solution_method = NULL;
siesta->divide_and_conquer = NULL;

siesta->occupation_function=NULL;
siesta->occupation_mp_order=NULL;
siesta->ordern_functional=NULL;
siesta->ordern_iterations=NULL;
siesta->ordern_energy_tolerance=NULL;
siesta->ordern_eta=NULL;
siesta->ordern_eta_alpha=NULL;
siesta->ordern_eta_beta=NULL;

siesta->split_zeta_norm = NULL;
siesta->energy_shift = NULL;
siesta->mesh_cutoff = NULL;
siesta->electronic_temperature = NULL;
siesta->spin_polarised = NULL;

siesta->fixed_spin = NULL;
siesta->total_spin = NULL;
siesta->non_collinear_spin = NULL;
siesta->single_excitation = NULL;

siesta->basis_type = NULL;
siesta->basis_size = NULL;

siesta->custom_zeta = NULL;
siesta->custom_zeta_polarisation = NULL;

siesta->harris_functional = NULL;

siesta->xc_functional = NULL;
siesta->xc_authors = NULL;

siesta->kgrid_cutoff = NULL;

siesta->no_of_cycles = NULL;
siesta->mixing_weight = NULL;
siesta->no_of_pulay_matrices = NULL;

//siesta MD options
siesta->md_type_of_run = NULL;
siesta->md_variable_cell = NULL;
siesta->md_num_cg_steps = NULL;
siesta->md_max_cg_displacement = NULL;
siesta->md_max_force_tol = NULL;
siesta->md_max_stress_tol = NULL;
siesta->md_precondition_variable_cell = NULL;
siesta->md_initial_temperature = NULL;
siesta->md_target_temperature = NULL;
siesta->md_initial_time_step = NULL;
siesta->md_final_time_step = NULL;
siesta->md_length_time_step = NULL;
siesta->md_quench = NULL;
siesta->md_target_pressure = NULL;
for (i=6 ; i-- ; )
  siesta->md_target_stress[i] = NULL;
siesta->md_nose_mass = NULL;
siesta->md_parrinello_rahman_mass = NULL;
siesta->md_anneal_option = NULL;
siesta->md_tau_relax = NULL;
siesta->md_bulk_modulus = NULL;
siesta->md_fc_displ = NULL;
siesta->md_fc_first = NULL;
siesta->md_fc_last = NULL;

siesta->density_of_states = NULL;
siesta->density_on_mesh = NULL;
siesta->electrostatic_pot_on_mesh = NULL;

siesta->long_output = NULL;
siesta->use_saved_data = NULL;
siesta->write_density_matrix = NULL;

siesta->system_label = NULL; 
siesta->write_coor_init = NULL; 
siesta->write_coor_step = NULL; 
siesta->write_forces = NULL; 
siesta->write_kpoints = NULL; 
siesta->write_eigenvalues = NULL; 
siesta->write_kbands = NULL; 
siesta->write_bands = NULL; 
siesta->write_wavefunctions = NULL; 
siesta->write_dm = NULL; 
siesta->write_coor_xmol = NULL; 
siesta->write_coor_cerius = NULL; 
siesta->write_md_xmol = NULL; 
siesta->write_md_history = NULL; 
siesta->write_mullikenpop = NULL;

siesta->vibration_calc_complete = FALSE;
siesta->custom_scaler = 0.3;

siesta->modelfilename = g_strdup("unknown");
}

/*******************************/
/* free a siesta configuration */
/*******************************/
void siesta_free(gpointer data)
{
gint i;
struct siesta_pak *siesta=data;

if (!siesta)
  return;

g_free(siesta->system_label);
g_free(siesta->eden);
g_free(siesta->edos);
g_free(siesta->epot);
g_free(siesta->grid_rho);
g_free(siesta->grid_drho);
g_free(siesta->grid_ldos);
g_free(siesta->grid_vh);
g_free(siesta->grid_vt);
g_free(siesta->freq_disp_str);
g_free(siesta->eigen_values);
g_free(siesta->eigen_xyz_atom_mat);
g_free(siesta->solution_method);
g_free(siesta->divide_and_conquer);
g_free(siesta->occupation_function);
g_free(siesta->occupation_mp_order);
g_free(siesta->ordern_functional);
g_free(siesta->ordern_iterations);
g_free(siesta->ordern_energy_tolerance);
g_free(siesta->ordern_eta);
g_free(siesta->ordern_eta_alpha);
g_free(siesta->ordern_eta_beta);
g_free(siesta->split_zeta_norm);
g_free(siesta->energy_shift);
g_free(siesta->mesh_cutoff);
g_free(siesta->electronic_temperature);
g_free(siesta->spin_polarised);
g_free(siesta->fixed_spin);
g_free(siesta->total_spin);
g_free(siesta->non_collinear_spin);
g_free(siesta->single_excitation);
g_free(siesta->basis_type);
g_free(siesta->basis_size);
g_free(siesta->custom_zeta);
g_free(siesta->custom_zeta_polarisation);
g_free(siesta->harris_functional);
g_free(siesta->xc_functional);
g_free(siesta->xc_authors);
g_free(siesta->kgrid_cutoff);
g_free(siesta->no_of_cycles);
g_free(siesta->mixing_weight);
g_free(siesta->no_of_pulay_matrices);
g_free(siesta->md_type_of_run);
g_free(siesta->md_variable_cell);
g_free(siesta->md_num_cg_steps);
g_free(siesta->md_max_cg_displacement);
g_free(siesta->md_max_force_tol);
g_free(siesta->md_max_stress_tol);
g_free(siesta->md_precondition_variable_cell);
g_free(siesta->md_initial_temperature);
g_free(siesta->md_target_temperature);
g_free(siesta->md_initial_time_step);
g_free(siesta->md_final_time_step);
g_free(siesta->md_length_time_step);
g_free(siesta->md_quench);
g_free(siesta->md_target_pressure);
for (i=6 ; i-- ; )
  g_free(siesta->md_target_stress[i]);
g_free(siesta->md_nose_mass);
g_free(siesta->md_parrinello_rahman_mass);
g_free(siesta->md_anneal_option);
g_free(siesta->md_tau_relax);
g_free(siesta->md_bulk_modulus);
g_free(siesta->md_fc_displ);
g_free(siesta->md_fc_first);
g_free(siesta->md_fc_last);
g_free(siesta->density_of_states);
g_free(siesta->density_on_mesh);
g_free(siesta->electrostatic_pot_on_mesh);
g_free(siesta->long_output);
g_free(siesta->use_saved_data);
g_free(siesta->write_density_matrix);
g_free(siesta->write_coor_init); 
g_free(siesta->write_coor_step); 
g_free(siesta->write_forces); 
g_free(siesta->write_kpoints); 
g_free(siesta->write_eigenvalues); 
g_free(siesta->write_kbands); 
g_free(siesta->write_bands); 
g_free(siesta->write_wavefunctions); 
g_free(siesta->write_dm); 
g_free(siesta->write_coor_xmol); 
g_free(siesta->write_coor_cerius); 
g_free(siesta->write_md_xmol); 
g_free(siesta->write_md_history); 
g_free(siesta->write_mullikenpop);
g_free(siesta->modelfilename);
}

/***************************************/
/* allocate a new siesta configuration */
/***************************************/
gpointer siesta_new(void)
{
struct siesta_pak *siesta;

siesta = g_malloc(sizeof(struct siesta_pak));

siesta_init(siesta);

return(siesta);
}

/*****************************************/
/* obtain a parse setup for siesta input */
/*****************************************/
gpointer siesta_input_parse_get(gpointer data)
{
gpointer parse;
struct siesta_pak *siesta=data;

g_assert(siesta != NULL);

parse = parse_new();

parse_keyword_add("XC.Functional", PARSE_FIRST_CHAR, &siesta->xc_functional, parse);
parse_keyword_add("XC.Authors", PARSE_FIRST_CHAR, &siesta->xc_authors, parse);
parse_keyword_add("Harris_functional", PARSE_FIRST_CHAR, &siesta->harris_functional, parse);
parse_keyword_add("SpinPolarized", PARSE_FIRST_CHAR, &siesta->spin_polarised, parse);

parse_keyword_add("NonCollinearSpin", PARSE_FIRST_CHAR, &siesta->non_collinear_spin, parse);
parse_keyword_add("FixSpin", PARSE_FIRST_CHAR, &siesta->fixed_spin, parse);
parse_keyword_add("TotalSpin", PARSE_FIRST_CHAR, &siesta->total_spin, parse);
parse_keyword_add("SingleExcitation", PARSE_FIRST_CHAR, &siesta->single_excitation, parse);

parse_keyword_add("PAO.BasisType", PARSE_FIRST_CHAR, &siesta->basis_type, parse);
parse_keyword_add("PAO.BasisSize", PARSE_FIRST_CHAR, &siesta->basis_size, parse);
parse_keyword_add("PAO.SplitNorm", PARSE_FIRST_CHAR, &siesta->split_zeta_norm, parse);
parse_keyword_add("PAO.EnergyShift", PARSE_ALL_CHAR, &siesta->energy_shift, parse);

parse_keyword_add("DM.NumberPulay", PARSE_FIRST_CHAR, &siesta->no_of_pulay_matrices, parse);
parse_keyword_add("DM.MixingWeight", PARSE_FIRST_CHAR, &siesta->mixing_weight, parse);

parse_keyword_add("SolutionMethod", PARSE_FIRST_CHAR, &siesta->solution_method, parse);
parse_keyword_add("Diag.DivideAndConquer", PARSE_FIRST_CHAR, &siesta->divide_and_conquer, parse);

parse_keyword_add("OccupationFunction", PARSE_FIRST_CHAR, &siesta->occupation_function, parse);
parse_keyword_add("OccupationMPOrder", PARSE_FIRST_CHAR, &siesta->occupation_mp_order, parse);
parse_keyword_add("ON.functional", PARSE_FIRST_CHAR, &siesta->ordern_functional, parse);
parse_keyword_add("ON.MaxNumIter", PARSE_FIRST_CHAR, &siesta->ordern_iterations, parse);
parse_keyword_add("ON.etol", PARSE_FIRST_CHAR, &siesta->ordern_energy_tolerance, parse);
parse_keyword_add("ON.eta", PARSE_FIRST_CHAR, &siesta->ordern_eta, parse);
parse_keyword_add("ON.eta_alpha", PARSE_FIRST_CHAR, &siesta->ordern_eta_alpha, parse);
parse_keyword_add("ON.eta_beta", PARSE_FIRST_CHAR, &siesta->ordern_eta_beta, parse);

parse_keyword_add("MD.TypeOfRun", PARSE_FIRST_CHAR, &siesta->md_type_of_run, parse);
parse_keyword_add("MD.VariableCell", PARSE_FIRST_CHAR, &siesta->md_variable_cell, parse);
parse_keyword_add("MD.NumCGSteps", PARSE_FIRST_CHAR, &siesta->md_num_cg_steps, parse);
parse_keyword_add("MD.MaxCGDispl", PARSE_ALL_CHAR, &siesta->md_max_cg_displacement, parse);
parse_keyword_add("MD.MaxForceTol", PARSE_ALL_CHAR, &siesta->md_max_force_tol, parse);
parse_keyword_add("MD.MaxStressTol", PARSE_ALL_CHAR, &siesta->md_max_stress_tol, parse);
parse_keyword_add("MD.InitialTimeStep", PARSE_FIRST_CHAR, &siesta->md_initial_time_step, parse);
parse_keyword_add("MD.FinalTimeStep", PARSE_FIRST_CHAR, &siesta->md_final_time_step, parse);
parse_keyword_add("MD.LengthTimeStep", PARSE_ALL_CHAR, &siesta->md_length_time_step, parse);
parse_keyword_add("MD.InitialTemperature", PARSE_ALL_CHAR, &siesta->md_initial_temperature, parse);
parse_keyword_add("MD.TargetTemperature", PARSE_ALL_CHAR, &siesta->md_target_temperature, parse);
parse_keyword_add("MD.Quench", PARSE_FIRST_CHAR, &siesta->md_quench, parse);
parse_keyword_add("MD.TargetPressure", PARSE_ALL_CHAR, &siesta->md_target_pressure, parse);
parse_keyword_add("MD.NoseMass", PARSE_ALL_CHAR, &siesta->md_nose_mass, parse);
parse_keyword_add("MD.ParrinelloRahmanMass", PARSE_ALL_CHAR, &siesta->md_parrinello_rahman_mass, parse);
parse_keyword_add("MD.AnnealOption", PARSE_FIRST_CHAR, &siesta->md_anneal_option, parse);
parse_keyword_add("MD.TauRelax", PARSE_ALL_CHAR, &siesta->md_tau_relax, parse);
parse_keyword_add("MD.BulkModulus", PARSE_ALL_CHAR, &siesta->md_bulk_modulus, parse);

parse_keyword_add("MeshCutoff", PARSE_ALL_CHAR, &siesta->mesh_cutoff, parse);
parse_keyword_add("ElectronicTemperature", PARSE_ALL_CHAR, &siesta->electronic_temperature, parse);

parse_keyword_add("MaxSCFIterations", PARSE_FIRST_CHAR, &siesta->no_of_cycles, parse);
parse_keyword_add("kgrid_cutoff", PARSE_ALL_CHAR, &siesta->kgrid_cutoff, parse);
parse_keyword_add("UseSaveData", PARSE_FIRST_CHAR, &siesta->use_saved_data, parse);
parse_keyword_add("NumberOfSpecies", PARSE_FIRST_CHAR, NULL, parse);

parse_keyword_add("LongOutput", PARSE_FIRST_CHAR, &siesta->long_output, parse);
parse_keyword_add("WriteCoorInitial", PARSE_FIRST_CHAR, &siesta->write_coor_init, parse);
parse_keyword_add("WriteCoorStep", PARSE_FIRST_CHAR, &siesta->write_coor_step, parse);
parse_keyword_add("WriteCoorCerius", PARSE_FIRST_CHAR, &siesta->write_coor_cerius, parse);
parse_keyword_add("WriteCoorXmol", PARSE_FIRST_CHAR, &siesta->write_coor_xmol, parse);
parse_keyword_add("WriteForces", PARSE_FIRST_CHAR, &siesta->write_forces, parse);
parse_keyword_add("WriteKpoints", PARSE_FIRST_CHAR, &siesta->write_kpoints, parse);
parse_keyword_add("WriteEigenvalues", PARSE_FIRST_CHAR, &siesta->write_eigenvalues, parse);
parse_keyword_add("WriteKbands", PARSE_FIRST_CHAR, &siesta->write_kbands, parse);
parse_keyword_add("WriteBands", PARSE_FIRST_CHAR, &siesta->write_bands, parse);
parse_keyword_add("WriteWaveFunctions", PARSE_FIRST_CHAR, &siesta->write_wavefunctions, parse);
parse_keyword_add("WriteDM", PARSE_FIRST_CHAR, &siesta->write_dm, parse);
parse_keyword_add("WriteMDXmol", PARSE_FIRST_CHAR, &siesta->write_md_xmol, parse);
parse_keyword_add("WriteMDhistory", PARSE_FIRST_CHAR, &siesta->write_md_history, parse);

return(parse);
}

/**********************************/
/* obtain a core's species number */
/**********************************/
gint fdf_species_index(gchar *atom_label, GSList *species_list)
{
gint i;
struct species_pak *species;
GSList *list;

/* find corresponding number of this element in the species block */
i=1;
for (list=species_list ; list ; list=g_slist_next(list))
  {
  species = list->data;

  if (g_ascii_strcasecmp(atom_label, species->label) == 0)
    return(i);

  i++;
  }
return(0);
}

/*****************************************************/
/* generate the list of unique species, SIESTA style */
/*****************************************************/
GSList *fdf_species_build(struct model_pak *model)
{
gint code;
struct species_pak *species_data;
GSList *list, *normal_list, *ghost_list, *species_list;

/* init the normal, ghost, and overall species lists */
normal_list = find_unique(LABEL_NORMAL, model);
ghost_list = find_unique(LABEL_GHOST, model);
species_list = NULL;

for (list=normal_list ; list ; list=g_slist_next(list))
  {
  code = elem_symbol_test(list->data);

/* update the species identifier list */
  species_data = g_malloc(sizeof(struct species_pak));
  species_data->label = list->data;
  species_data->number = code;
  species_list = g_slist_prepend(species_list, species_data);
  }

for (list=ghost_list ; list ; list=g_slist_next(list))
  {
  code = elem_symbol_test(list->data);

/* update the species identifier list */
  species_data = g_malloc(sizeof(struct species_pak));
  species_data->label = list->data;
  species_data->number = -code;
  species_list = g_slist_prepend(species_list, species_data);
  }
species_list = g_slist_reverse(species_list);

/* parse the zmatrix entries and make the type match the (possible new) species order */
if (model->zmatrix)
  {
  struct zmat_pak *zmatrix = model->zmatrix;

/* scan zval entries - make sure type value reflects element order */
  for (list=zmatrix->zlines ; list ; list=g_slist_next(list))
    {
    struct zval_pak *zval = list->data;
    zval->type = fdf_species_index(zval->elem, species_list);
    }
  }

return(species_list);
}

/****************/
/* file writing */
/****************/
//gint write_fdf(gchar *filename, struct model_pak *model)
gint siesta_input_export(gpointer import)
{
gint i, print_stress;
gchar *filename, *text;
gdouble x[3], depth;
gpointer config, parse;
GSList *list, *clist, *species_list;
GString *buffer;
struct core_pak *core;
struct species_pak *species_data;
struct siesta_pak *siesta, siesta_default;
struct model_pak *model;

g_assert(import != NULL);

model = import_object_nth_get(IMPORT_MODEL, 0, import);
if (!model)
  return(1);

filename = import_fullpath(import);
siesta = import_config_data_get(FDF, import);
if (!siesta)
  {
  siesta_init(&siesta_default);
  siesta = &siesta_default;
  }

/* create buffer */
buffer = g_string_new(NULL);

/* print header */
g_string_append_printf(buffer, "# ");
g_string_append_printf(buffer, "#\n\n");
if (siesta->system_label)
  g_string_append_printf(buffer, "SystemLabel      %s\n\n", siesta->system_label);
else
  g_string_append_printf(buffer, "SystemLabel      %s\n\n", model->basename);
g_string_append_printf(buffer, "NumberOfAtoms    %d\n\n", g_slist_length(model->cores));

/* init the normal, ghost, and overall species lists */
species_list = fdf_species_build(model);

g_string_append_printf(buffer, "NumberOfSpecies  %d\n", g_slist_length(species_list));

/* write the species (unique atom types) block */
g_string_append_printf(buffer, "%%block ChemicalSpeciesLabel\n");
i=1;
for (list=species_list ; list ; list=g_slist_next(list))
  {
  species_data = list->data;

  g_string_append_printf(buffer, "  %3d  %3d  %s\n", i, species_data->number, species_data->label);
  i++;
  }
g_string_append_printf(buffer, "%%endblock ChemicalSpeciesLabel\n\n");


/* write the lattice data */
if (model->periodic)
  {
  g_string_append_printf(buffer, "LatticeConstant 1.0 Ang\n");
  if (model->construct_pbc)
    {
/* NB: siesta matrices are transposed wrt gdis */
    g_string_append_printf(buffer, "%%block LatticeVectors\n");
    g_string_append_printf(buffer,"%15.10f %15.10f %15.10f\n",
                model->latmat[0], model->latmat[3], model->latmat[6]);
    g_string_append_printf(buffer,"%15.10f %15.10f %15.10f\n",
                model->latmat[1], model->latmat[4], model->latmat[7]);
    g_string_append_printf(buffer,"%15.10f %15.10f %15.10f\n",
                model->latmat[2], model->latmat[5], model->latmat[8]);
    g_string_append_printf(buffer, "%%endblock LatticeVectors\n\n");
    }
  else
    {
    g_string_append_printf(buffer, "%%block LatticeParameters\n");
/* if it's a surface - print Dhkl (mult by region sizes) */
    if (model->periodic == 2)
      {
/* get depth info */
      depth = (model->surface.region[0]+model->surface.region[1])
                                          *model->surface.depth;
/* no depth info - make it large enough to fit everything */
      if (depth < POSITION_TOLERANCE)
        depth = 2.0*model->rmax;

      g_string_append_printf(buffer, "  %f  %f  %f", model->pbc[0], model->pbc[1], depth);
      }
    else
      g_string_append_printf(buffer, "  %f  %f  %f", model->pbc[0], model->pbc[1], model->pbc[2]);

    g_string_append_printf(buffer, "  %f  %f  %f\n", R2D*model->pbc[3], R2D*model->pbc[4], R2D*model->pbc[5]);
    g_string_append_printf(buffer, "%%endblock LatticeParameters\n\n");
    }
  }

/* write the zmatrix data */
/* FIXME - removed until zmat_ stuff brought in line with new preview buffer stuff */
/*
if (zmat_entries_get(model->zmatrix))
  {
  g_string_append_printf(buffer, "ZM.UnitsLength %s\n", zmat_distance_units_get(model->zmatrix));
  g_string_append_printf(buffer, "ZM.UnitsAngle %s\n\n", zmat_angle_units_get(model->zmatrix));

  g_string_append_printf(buffer, "%%block Zmatrix\n");
 
  zmat_coord_print(buffer, species_list, model);
 
  zmat_mol_print(buffer, model->zmatrix);
 
  zmat_var_print(buffer, model->zmatrix);

  zmat_const_print(buffer, model->zmatrix);
 
  g_string_append_printf(buffer, "%%endblock Zmatrix\n");
  }
else
*/

  {

/* write the atoms - for surfaces (and isolated molecules), coords must be cartesian */
if (model->periodic && model->fractional)
  g_string_append_printf(buffer, "AtomicCoordinatesFormat Fractional\n");
else
  g_string_append_printf(buffer, "AtomicCoordinatesFormat NotScaledCartesianAng\n");

g_string_append_printf(buffer, "%%block AtomicCoordinatesAndAtomicSpecies\n");

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
  i=1;
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

/* found? */
  if (list)
    g_string_append_printf(buffer,"  %14.9f  %14.9f  %14.9f    %d\n", x[0], x[1], x[2], i);
  else
    printf("write_fdf() error: bad species type.\n");
  }
g_string_append_printf(buffer, "%%endblock AtomicCoordinatesAndAtomicSpecies\n\n");

  }

free_slist(species_list);

/* output all the recognized keyword<->variable relations */
parse = siesta_input_parse_get(siesta);
text = parse_inverse(parse);
if (text)
  {
  g_string_append(buffer, text);
  g_free(text);
  }

/* special case outputs */

/* any non-NULL value -> output stress tensor */
print_stress=FALSE;
for (i=6 ; i-- ; )
  {
  if (siesta->md_target_stress[i])
    print_stress=TRUE;
  }
if (print_stress)
  {
  g_string_append_printf(buffer, "\n%%block MD.TargetStress\n");
  for (i=0 ; i<6 ; i++)
    {
    if (siesta->md_target_stress[i])
      g_string_append_printf(buffer, "%s ", siesta->md_target_stress[i]);
    else
      g_string_append_printf(buffer, "0.0 ");
    }
  g_string_append_printf(buffer, "\n%%endblock MD.TargetStress\n\n");
  }

/* unprocessed/unparsed lines */
config = import_config_get(FDF, import);
if (config)
  {
  text = config_unparsed_get(config);
  if (text)
    g_string_append_printf(buffer, "\n# --- unparsed ---\n%s\n", text);
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
  g_file_set_contents(filename, buffer->str, -1, NULL);
  g_string_free(buffer, TRUE);
  }

return(0);
}

/****************************************************/
/* attempt to read header at a particular precision */
/****************************************************/
gint siesta_header_read(FILE *fp, gint precision, gdouble *cell, gint *grid)
{
gint i, swap=FALSE;
int grid_int[4];
/*long grid_long[4];*/
/*float cell_float[9];*/
double cell_double[9];

/* standard return values */
for (i=9 ; i-- ; )
  cell[i] = 0.0;
for (i=4 ; i-- ; )
  grid[i] = 0;

/* precision dependent read */
switch (precision)
  {
  case 1:
    i = READ_RECORD;
    i = fread(cell_double, sizeof(double), 9, fp);
    i = READ_RECORD;

    i = READ_RECORD;
    i = fread(&grid_int, sizeof(int), 4, fp);
    i = READ_RECORD;

/* byte ordering check */
    for (i=9 ; i-- ; )
      {
      if (fabs(cell_double[i]) > 1000000.0)
        {
        swap = TRUE;
        break;
        }
      }
    if (swap)
      {
      for (i=9 ; i-- ; )
        swap_bytes(&cell_double[i], sizeof(double));
      for (i=4 ; i-- ; )
        swap_bytes(&grid_int[i], sizeof(int));
      }

/* pass back */
    for (i=9 ; i-- ; )
      cell[i] = cell_double[i];
    for (i=4 ; i-- ; )
      grid[i] = grid_int[i];
    break;

  default:
    printf("FIXME - unsuported precision.\n");
  }

return(swap);
}

/******************************************/
/* check for additional siesta data files */
/******************************************/
// new - source is the grid/mesh file to read
#define DEBUG_SIESTA_DENSITY 0
gint siesta_density_read(const gchar *source, struct model_pak *model)
{
gint grid[4];
gint i, j, k, n, br, size, file_size, type, swap;
gchar *text, *key1, *key2, *value1, *value2;
float *f;
gdouble min, max;
gdouble cell[9], icell[9], *ptr;
FILE *fp;

g_assert(source != NULL);
g_assert(model != NULL);

#if DEBUG_SIESTA_DENSITY
printf("source: %s\n", source);
#endif

// check source extension for type
// FIXME - order of RHO, DRHO check is important
type = MS_SIESTA_RHO;
if (g_strrstr(source, "RHO"))
  type = MS_SIESTA_RHO;
if (g_strrstr(source, "DRHO"))
  type = MS_SIESTA_DRHO;
if (g_strrstr(source, "LDOS"))
  type = MS_SIESTA_LDOS;
if (g_strrstr(source, "VH"))
  type = MS_SIESTA_VH;
if (g_strrstr(source, "VT"))
  type = MS_SIESTA_VT;

// attempt to open file
// NB: freads on a binary file (under windows) will terminate incorrectly
// when it hits what it thinks is an EOF 
  fp = fopen(source, "rb");
  if (!fp)
    {
    text = g_strdup_printf("Missing data file: %s\n", source);
    gui_text_show(ERROR, text);
    g_free(text);
    return(FALSE);
    }

  file_size = file_byte_size(source);

  swap = siesta_header_read(fp, 1, cell, grid);

  memcpy(icell, cell, 9*sizeof(gdouble));
  matrix_invert(icell);

#if DEBUG_SIESTA_DENSITY
printf("swap bytes: %d\n", swap);
P3MAT(" cell: ", cell);
P3MAT("icell: ", icell);
printf("grid: %d x %d x %d : %d\n", grid[0], grid[1], grid[2], grid[3]);
#endif

/* NB: FORTRAN has a 4 byte separator either side of each record (single write() call) */
/*
real*8 cell(3,3)
real*4 rho(nmesh(1)*nmesh(2)*nmesh(3),nspin)
integer mesh(3)
integer nspin

    write(iu) cell
    write(iu) mesh, nspin
    ind = 0
    do is  = 1,nspin
       do iz = 1,mesh(3)
         do iy = 1,mesh(2)
            write(iu) rho(ind+ix,is),ix = 1,mesh(1)
            ind = ind + mesh(1)
         enddo
       enddo
    enddo
*/

/* data array size */
  n = grid[1] * grid[2] * grid[3];
  size = 8 + 4*grid[0];
  size *= n;
/* cell + mesh size */
  size += 104;

#if DEBUG_SIESTA_DENSITY
printf("Data size: %d bytes\n", size);
printf("File size: %d bytes\n", file_size);
#endif

  if (size != file_size)
    {
/* TODO - retry at different precision */
    gui_text_show(ERROR, "Failed to parse binary file.\n");
    return(FALSE);
    }

  memcpy(model->siesta.cell, cell, 9*sizeof(gdouble));
  memcpy(model->siesta.icell, icell, 9*sizeof(gdouble));
  memcpy(model->siesta.grid, grid, 4*sizeof(gint));

/* test for a scaling factor (ie au output instead of angstrom) */
matmat(model->latmat, icell);

if (icell[0] > 0.2)
  {
#if DEBUG_SIESTA_DENSITY
printf("density grid in au, scale = %f\n", icell[0]);
#endif
  model->siesta.eden_scale = icell[0];
  }

/* read function data */
  switch (type)
    {
    case MS_SIESTA_RHO:
      model->siesta.grid_rho = g_malloc(n*grid[0]*sizeof(gdouble));
      ptr = model->siesta.grid_rho;
      break;
    case MS_SIESTA_DRHO:
      model->siesta.grid_drho = g_malloc(n*grid[0]*sizeof(gdouble));
      ptr = model->siesta.grid_drho;
      break;
    case MS_SIESTA_LDOS:
      model->siesta.grid_ldos = g_malloc(n*grid[0]*sizeof(gdouble));
      ptr = model->siesta.grid_ldos;
      break;
    case MS_SIESTA_VH:
      model->siesta.grid_vh = g_malloc(n*grid[0]*sizeof(gdouble));
      ptr = model->siesta.grid_vh;
      break;
    case MS_SIESTA_VT:
      model->siesta.grid_vt = g_malloc(n*grid[0]*sizeof(gdouble));
      ptr = model->siesta.grid_vt;
      break;

    default:
      gui_text_show(ERROR, "Bad siesta read type");
      return(FALSE);
    }

  min = G_MAXDOUBLE;
  max = -G_MAXDOUBLE;

#if DEBUG_SIESTA_DENSITY
printf("Reading %d rows with %d values.\n", n, grid[0]);
#endif

  f = g_malloc(grid[0]*sizeof(float));
  k=0;
  for (i=0 ; i<n ; i++)
    {
    br = READ_RECORD;
    br += fread(f, sizeof(float), grid[0], fp);
    br += READ_RECORD;

    if (swap)
      {
      for (j=0 ; j<grid[0] ; j++)
        {
        swap_bytes(f+j, sizeof(float));
        *(ptr+k++) = *(f+j);

        if (f[j] < min)
          min = f[j];
        if (f[j] > max)
          max = f[j];
        }
      }
    else
      {
      for (j=0 ; j<grid[0] ; j++)
        {
        *(ptr+k++) = *(f+j);

        if (f[j] < min)
          min = f[j];
        if (f[j] > max)
          max = f[j];
        }
      }

#if DEBUG_SIESTA_DENSITY
if (i > 5 && i < 15)
  {
  printf("row %d, bytes read: %d, 1st value: %f\n", i, br, f[0]);
  }
#endif

    }
  g_free(f);

#if DEBUG_SIESTA_DENSITY
printf("grid, min = %f, max= %f\n", min, max);
#endif

/* this shouldn't happen (file moved while reading?) */
  if (k != n*grid[0])
    {
    gui_text_show(ERROR, "Fatal error reading binary file.\n");
    }

/* calculate and store the min/max values */
switch (type)
  {
  case MS_SIESTA_RHO:
    key1 = g_strdup_printf("Siesta RHO min");
    key2 = g_strdup_printf("Siesta RHO max");
    break;
  case MS_SIESTA_DRHO:
    key1 = g_strdup_printf("Siesta DRHO min");
    key2 = g_strdup_printf("Siesta DRHO max");
    break;
  case MS_SIESTA_LDOS:
    key1 = g_strdup_printf("Siesta LDOS min");
    key2 = g_strdup_printf("Siesta LDOS max");
    break;
  case MS_SIESTA_VH:
    key1 = g_strdup_printf("Siesta VH min");
    key2 = g_strdup_printf("Siesta VH max");
    break;
  case MS_SIESTA_VT:
    key1 = g_strdup_printf("Siesta VT min");
    key2 = g_strdup_printf("Siesta VT max");
    break;
  }

value1 = g_strdup_printf("%f", min);
value2 = g_strdup_printf("%f", max);

property_add_ranked(6+type, key1, value1, model);
property_add_ranked(6+type, key2, value2, model);

g_free(key1);
g_free(key2);
g_free(value1);
g_free(value2);

return(TRUE);
}

/***********************************************************************/
/* process lattice constant line to produce a cartesian scaling factor */
/***********************************************************************/
#define DEBUG_SIESTA_SCALE 0
gdouble siesta_scale_parse(const gchar *text)
{
gint num_tokens;
gchar **buff;
gdouble scale = 1.0;

g_assert(text != NULL);

#if DEBUG_SIESTA_SCALE
printf("input line: %s", text);
#endif

/* process scale */
buff = tokenize(text, &num_tokens);
if (num_tokens > 1)
  scale *= str_to_float(*(buff+1));

/* process units */
if (num_tokens > 2)
  if ((g_ascii_strncasecmp(*(buff+2), "au", 2) == 0) ||
      (g_ascii_strncasecmp(*(buff+2), "Bohr", 4) == 0))
    scale *= AU2ANG;  

g_strfreev(buff);

#if DEBUG_SIESTA_SCALE
printf("output scale: %f\n", scale);
#endif

return(scale);
}

/********************************/
/* process a species block line */
/********************************/
void seista_parse_species(const gchar *line, GSList **list)
{
gint num_tokens;
gchar **buff;
struct species_pak *species;

g_assert(line != NULL);

buff = tokenize(line, &num_tokens);

if (num_tokens > 2)
  {
  species = g_malloc(sizeof(struct species_pak));

  species->number = str_to_float(*(buff+1));
  species->label = g_strdup(*(buff+2));
  *list = g_slist_append(*list, species);
  }

g_strfreev(buff);
}

/************************************/
/* process a lattice parameter line */
/************************************/
void seista_parse_cell(const gchar *line, gdouble scale, struct model_pak *model)
{
gint num_tokens;
gchar **buff;

g_assert(line != NULL);

buff = tokenize(line, &num_tokens);
if (num_tokens > 5)
  {
  model->pbc[0] = scale * str_to_float(*(buff+0));
  model->pbc[1] = scale * str_to_float(*(buff+1));
  model->pbc[2] = scale * str_to_float(*(buff+2));
  model->pbc[3] = D2R*str_to_float(*(buff+3));
  model->pbc[4] = D2R*str_to_float(*(buff+4));
  model->pbc[5] = D2R*str_to_float(*(buff+5));
  }

model->construct_pbc = FALSE;

/* hack for determining if it's a (GDIS created) surface */
if (fabs(model->pbc[2]) < FRACTION_TOLERANCE)
  model->periodic = 2;
else
  model->periodic = 3;

g_strfreev(buff);
}

/*********************************/
/* process a lattice vector line */
/*********************************/
void seista_parse_lattice(const gchar *line, gint n, gdouble scale, struct model_pak *model)
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

model->construct_pbc = TRUE;
model->periodic = 3;

g_strfreev(buff);
}

/***********************************/
/* process a coordinate block line */
/***********************************/
void seista_parse_coords(const gchar *line, GSList *species_list, gdouble scale, struct model_pak *model)
{
gint i, num_tokens;
gchar **buff;
struct species_pak *species;
struct core_pak *core;

g_assert(line != NULL);

buff = tokenize(line, &num_tokens);

if (num_tokens > 3)
  {
/* find corresponding number of this element in the species block */
/* NB: list counts from 0, fdf counts from 1 (hence the minus 1) */
  if (species_list)
    {
    i = abs((gint) str_to_float(*(buff+3)) - 1);
    species = g_slist_nth_data(species_list, i);

    core = new_core(species->label, model);

/* translucent ghost atom */
    if (species->number < 0)
      {
      core->ghost = TRUE;
      core->colour[3] = 0.5;
      }
    }
  else
    core = new_core("X", model);
 
  model->cores = g_slist_prepend(model->cores, core);
    
  core->x[0] = str_to_float(*(buff+0));
  core->x[1] = str_to_float(*(buff+1));
  core->x[2] = str_to_float(*(buff+2));

  VEC3MUL(core->x, scale);
  }

g_strfreev(buff);
}

/***************************/
/* zmatrix line processing */
/***************************/
void seista_parse_zmatrix(const gchar *text,
                          GSList *species_list,
                          gdouble scale,
                          struct model_pak *model)
{
gint i, num_tokens;
gchar **buff;
struct core_pak *core;
struct species_pak *species_data;
static gint state=0;

/* keyword processing within zmatrix block */
if (g_ascii_strncasecmp("molecule", text, 8) == 0)
  {
  state = 1;
  if (strstr(text, "frac"))
    zmat_fractional_set(model->zmatrix);
  else
    zmat_cartesian_set(model->zmatrix);
  return;
  }

if (g_ascii_strncasecmp("variable", text, 8) == 0)
  {
  state = 2;
  return;
  }

if (g_ascii_strncasecmp("constant", text, 8) == 0)
  {
  state = 3;
  return;
  }

if (g_ascii_strncasecmp("fractional", text, 10) == 0)
  {
  state = 4;
  model->fractional = TRUE;
  return;
  }

if (g_ascii_strncasecmp("cartesian", text, 9) == 0)
  {
  state = 4;
  model->fractional = FALSE;
  return;
  }

/* data processing within zmatrix block */
switch (state)
  {
  case 1:
/* process zmatrix coords */
    zmat_core_add(text, model->zmatrix);
    break;

  case 2:
/* process zmatrix variables */
    zmat_var_add(text, model->zmatrix);
    break;

  case 3:
/* process zmatrix constants */
    zmat_const_add(text, model->zmatrix);
    break;

  case 4:
/* fractional input coords */
    buff = tokenize(text, &num_tokens);
    if (num_tokens > 3)
      {
/* find corresponding number of this element in the species block */
/* NB: list counts from 0, fdf counts from 1 (hence the minus 1) */
      if (species_list)
        {
        i = abs((gint) str_to_float(*(buff)) - 1);
        species_data = g_slist_nth_data(species_list, i);

        core = new_core(species_data->label, model);

/* translucent ghost atom */
        if (species_data->number < 0)
          {
          core->ghost = TRUE;
          core->colour[3] = 0.5;
          }
        }
      else
        core = new_core("X", model);

      model->cores = g_slist_prepend(model->cores, core);

      core->x[0] = scale * str_to_float(*(buff+1));
      core->x[1] = scale * str_to_float(*(buff+2));
      core->x[2] = scale * str_to_float(*(buff+3));
      }
    g_strfreev(buff);
    break;
  }
}

/******************************/
/* main fdf data block reader */
/******************************/
gint import_fdf(gpointer import)
{
gint count, num_tokens, block_type, siesta_sucks = FALSE;
gchar **buff, *line, *filename;
gdouble scale_lattice = 1.0, scale_coords = 1.0;
gpointer config, parse;
GSList *species_list=NULL;
GString *unparsed;
struct siesta_pak *siesta;
struct model_pak *model;
FILE *fp;

g_assert(import != NULL);

/* NEW */
model = model_new();
siesta = siesta_new();
config = config_new(FDF, siesta);

filename = import_fullpath(import);

//printf("FDF reading: %s\n", filename);

fp = fopen(filename, "rt");
if (!fp)
  return(1);

/* CURRENT - new parse approach */
/* TODO - use gtk lexical scanner */
parse = siesta_input_parse_get(siesta);

unparsed = g_string_new(NULL);

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
  if (g_ascii_strncasecmp("\%block", line, 6) == 0)
    {
    block_type = SIESTA_DEFAULT;

    if (num_tokens > 1)
      {
      if (g_ascii_strncasecmp("ChemicalSpecies", *(buff+1), 15) == 0)
        block_type = SIESTA_SPECIES;
      if (g_ascii_strncasecmp("LatticeParameters", *(buff+1), 17) == 0)
        block_type = SIESTA_LATTICE_PARAMETERS;
      if (g_ascii_strncasecmp("LatticeVectors", *(buff+1), 14) == 0)
        block_type = SIESTA_LATTICE_VECTORS;
      if (g_ascii_strncasecmp("AtomicCoordinates", *(buff+1), 17) == 0)
        block_type = SIESTA_COORDS;
      if (g_ascii_strncasecmp("zmatrix", *(buff+1), 7) == 0)
        block_type = SIESTA_ZMATRIX;
      }

/* record unprocessed */
    if (block_type == SIESTA_DEFAULT)
      g_string_append(unparsed, line);

    count = 0;
    for (;;)
      {
      g_free(line);
      line = file_read_line(fp);
      if (!line)
        goto siesta_done_file;

      if (g_ascii_strncasecmp("\%endblock", line, 9) == 0)
        {
        if (block_type == SIESTA_DEFAULT)
          g_string_append(unparsed, line);
        goto siesta_done_line;
        }

      switch (block_type)
        {
        case SIESTA_SPECIES:
          seista_parse_species(line, &species_list);
          break;

        case SIESTA_LATTICE_PARAMETERS:
          seista_parse_cell(line, scale_lattice, model);
          break;

        case SIESTA_LATTICE_VECTORS:
          seista_parse_lattice(line, count, scale_lattice, model);
          break;

        case SIESTA_COORDS:
          seista_parse_coords(line, species_list, scale_coords, model);
          break;

        case SIESTA_ZMATRIX:
          seista_parse_zmatrix(line, species_list, scale_coords, model);
          break;

        default:
          g_string_append(unparsed, line);
          break;
        }
      count++;
      }
/* done block processing - get next line */
    continue;
    }

/* cartesian/fractional */
  if (g_ascii_strncasecmp("AtomicCoordinatesFormat", line, 23) == 0)
    {
    if (g_strrstr(line, "ractional"))
      model->fractional = TRUE;
    else
      {
      model->fractional = FALSE;

/* cope with cartesians that arent really cartesian */
      if (g_strrstr(line, "ScaledCartesian"))
        siesta_sucks = TRUE;

      if (g_strrstr(line, "Bohr"))
        scale_coords = AU2ANG;
      }
    }
  else if (g_ascii_strncasecmp("LatticeConstant", *buff, 15) == 0)
    {
    scale_lattice = siesta_scale_parse(line);
    }
  else if (g_ascii_strncasecmp("NumberOfAtoms", *buff, 13) == 0)
    {
    if (num_tokens > 1)
      siesta->num_atoms = (gint) str_to_float(*(buff+1));
    }
  else if (g_ascii_strncasecmp("SystemLabel", *buff, 11) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(model->basename);
      model->basename = g_strdup(*(buff+1));
      g_free(siesta->system_label);
//      siesta->system_label = g_strdup(*(buff+1));
//      siesta->system_label = g_strdup(get_token_pos(line, 1));
      siesta->system_label = parse_strip_newline(get_token_pos(line, 1));

      }
    }
  else if (g_ascii_strncasecmp("zm.unitslength", *buff, 14) == 0)
    {
    if (num_tokens)
      {
      if (g_ascii_strncasecmp("ang", *(buff+1), 3) == 0)
        zmat_distance_units_set(model->zmatrix, ANGSTROM);
      else
        zmat_distance_units_set(model->zmatrix, BOHR);
      }
    }
  else if (g_ascii_strncasecmp("zm.unitsangle", *buff, 13) == 0)
    {
    if (num_tokens)
      {
      if (g_ascii_strncasecmp("deg", *(buff+1), 3) == 0)
        zmat_angle_units_set(model->zmatrix, DEGREES); 
      else
        zmat_angle_units_set(model->zmatrix, RADIANS); 
      }
    }
  else
    {
    if (!parse_line(line, parse))
      {
/* unrecognized line */
/* TODO - store for output (unrecognized siesta config line) */
      g_string_append(unparsed, line);
      }
    }
    
    if (g_ascii_strncasecmp("**************************", *buff, 26) == 0)
    {
        if (g_ascii_strncasecmp("End", *(buff+1), 3) == 0)
        {
        break;
        }
    }

/* loop cleanup */
  siesta_done_line:;

  g_strfreev(buff);
  g_free(line);
  }

siesta_done_file:;

/* convert scaled cartesians to proper cartesians */
if (siesta_sucks)
  {
  GSList *list;
  struct core_pak *core;

  for (list=model->cores ; list ; list=g_slist_next(list))
    {
    core = list->data;
    VEC3MUL(core->x, scale_lattice);
    }
  }

/* NEW - process zmatrix cores */
zmat_type(model->zmatrix, species_list);
zmat_process(model->zmatrix, model);

/* free the species list */
free_slist(species_list);

/* model setup */
model->cores = g_slist_reverse(model->cores);
model_prep(model);

//siesta_density_files(filename, model);

/* NEW */
config_unparsed_set(g_string_free(unparsed, FALSE), config);

import_object_add(IMPORT_CONFIG, config, import);
import_object_add(IMPORT_MODEL, model, import);

parse_free(parse);
 
return 0;
}

/**************************/
/* fdf input file reading */
/**************************/
#define DEBUG_READ_FDF 0
gint read_fdf(gchar *filename, struct model_pak *model)
{
return(0);
}

/*******************************************/
/* read single SIESTA output configuration */
/*******************************************/
/* NEW - skip coords for variable cell calc */
#define DEBUG_READ_SOUT 0
gint read_sout_block(FILE *fp, gint skip_coords, struct model_pak *model)
{
gint i, num_tokens;
gchar *text;
gchar **temp, **buff, line[LINELEN];
gdouble x[3];
GSList *core_list;
struct core_pak *core;

if (!skip_coords)
  {
core_list = model->cores;

/* get 1st line of coords */
if (fgetline(fp, line))
  return(1);
buff = tokenize(line, &num_tokens);

while (num_tokens > 4)
  {
/* siesta can change the column the element symbol appears in */
  for (i=3 ; i<num_tokens ; i++)
    {
    if (elem_symbol_test(*(buff+i)))
      {
      x[0] = str_to_float(*(buff+0));
      x[1] = str_to_float(*(buff+1));
      x[2] = str_to_float(*(buff+2));

      if (core_list)
        {
        core = core_list->data;
        core_list = g_slist_next(core_list);
        }
      else
        {
        core = new_core(*(buff+i), model);
        model->cores = g_slist_append(model->cores, core);
        }
      ARR3SET(core->x, x);
      break;
      }
    }

/* get next line */
  g_strfreev(buff);
  if (fgetline(fp, line))
    return(2);
  buff = tokenize(line, &num_tokens);
  }
g_strfreev(buff);
  }

/* FIXME - I don't think it ever gets this far... ? */
/* attempt to get the lattice matrix */
/* if not found, use the initial values */
while (!fgetline(fp, line))
  {
/* terminate on encountering next CG step */
  if (g_strrstr(line, " Begin CG move"))
    break;
  if (g_strrstr(line, " Begin MD step"))
    break;

/* acquire cell lengths */
  if (g_ascii_strncasecmp(line, "outcell: Cell vector modules", 28) == 0)
    {
    model->construct_pbc = FALSE;
    model->periodic = 3;
    temp = g_strsplit(line, ":", 3);
    buff = tokenize(*(temp+2), &num_tokens);
    g_strfreev(temp);

    if (num_tokens > 2)
      {
      model->pbc[0] = str_to_float(*(buff+0));
      model->pbc[1] = str_to_float(*(buff+1));
      model->pbc[2] = str_to_float(*(buff+2));
      }
    else
      printf("Unexpected data format reading cell lengths.\n");
    g_strfreev(buff);
    }

  if (g_strrstr(line, "constrained") != NULL)
    {
    buff = tokenize(line, &num_tokens);
    if (num_tokens > 1)
      {
      model->siesta.max_grad = str_to_float(*(buff+1));
      model->siesta.have_max_grad = TRUE;
      }
    g_strfreev(buff);
    }

/* acquire cell angles */
  if (g_ascii_strncasecmp(line, "outcell: Cell angles", 20) == 0)
    {
    temp = g_strsplit(line, ":", 3);
    buff = tokenize(*(temp+2), &num_tokens);
    g_strfreev(temp);

    if (num_tokens > 2)
      {
/* get the angles (in radians) */
      model->pbc[3] = D2R*str_to_float(*(buff+0));
      model->pbc[4] = D2R*str_to_float(*(buff+1));
      model->pbc[5] = D2R*str_to_float(*(buff+2));
      }
    else
      printf("Unexpected data format reading cell angles.\n");
    g_strfreev(buff);
    }

/* energy parse during steps */
  if (g_ascii_strncasecmp(line, "siesta: Temp", 12) == 0)
    {
    buff = tokenize(line, &num_tokens);
    if (num_tokens > 3)
      {
      text = g_strdup_printf("%s K", *(buff+3));
      property_add_ranked(3, "Temperature", text, model);
      g_free(text);
      }
    g_strfreev(buff);
    }

/* energy parse during steps */
  if (g_ascii_strncasecmp(line, "siesta: E_KS(eV)", 16) == 0)
    {
    buff = tokenize(line, &num_tokens);
    if (num_tokens > 3)
      {
      text = g_strdup_printf("%s eV", *(buff+3));
      property_add_ranked(3, "Energy", text, model);
      g_free(text);
      }
    g_strfreev(buff);
    }

/* FIXME - use this section of a siesta output to flag the end of the 2nd last frame */
/* FIXME - This only works at the end of an optimization. Need to fix for all frames */
  if (g_ascii_strncasecmp(line, "siesta: Pressure (static)", 25) == 0)
    {
    buff = tokenize(line, &num_tokens);
    if (num_tokens > 4)
      {
      property_add_ranked(5, "Pressure", get_token_pos(line, 3), model);
//        property_add_ranked(5, "Pressure", *(buff+3), model);
      }
    else
      {
/* final step */
      for (i=0; i<4; i++)
        {
        if (fgetline(fp, line))
          break;
        }
      g_strfreev(buff);
      buff = tokenize(line, &num_tokens);
      if (num_tokens > 3)
        property_add_ranked(5, "Pressure", *(buff+1), model);
      }
    g_strfreev(buff);

/* FIXME - see above */
//    break;
    }
  }

g_free(model->title);

return(0);
}

/*******************************/
/* SIESTA output frame reading */
/*******************************/
gint read_sout_frame(FILE *fp, struct model_pak *model)
{
    return(read_sout_block(fp, FALSE, model));
}

/********************************/
/* Read in a SIESTA output file */
/********************************/
/* could almost use the read_fdf() code, as SIESTA spits out a copy */
/* however, we do need to know the number of frames for animation... */
gint read_sout(gchar *filename, struct model_pak *model)
{
gint skip_coords = FALSE;
gchar **buff, line[LINELEN], *text;
FILE *fp;
    
gint num_tokens, i;
gdouble scale = 1.0;

fp = fopen(filename, "rt");
if (!fp)
  return(1);

fgetline(fp, line);

while (!strstr(line, "Dump of input data file"))
  {
  if (fgetline(fp, line))
    break;
  }

//should be at the input file params -- dont want initial co-ords.
//read_fdf_block(fp,model, TRUE);
//should magicly return at the ======

while (!fgetline(fp, line))
  {

/* lattice constant scaling */
  if (g_ascii_strncasecmp("LatticeConstant", line, 15) == 0)
    {
    scale = siesta_scale_parse(line);
    continue;
    }

/* default cell dimensions */
  if (g_ascii_strncasecmp("\%block LatticeParameters", line, 24) == 0)
  {
     if (fgetline(fp, line))
       break;
     buff = tokenize(line, &num_tokens);
     if (num_tokens > 5)
      {
      model->pbc[0] = scale*str_to_float(*(buff+0));
      model->pbc[1] = scale*str_to_float(*(buff+1));
      model->pbc[2] = scale*str_to_float(*(buff+2));
      model->pbc[3] = D2R*str_to_float(*(buff+3));
      model->pbc[4] = D2R*str_to_float(*(buff+4));
      model->pbc[5] = D2R*str_to_float(*(buff+5));
      model->construct_pbc = FALSE;
      model->periodic = 3;
      }
    g_strfreev(buff);
  }
  else if (g_ascii_strncasecmp("\%block LatticeVectors", line, 20) == 0)
  {
      for (i=0; i<3; i++)
      {
        if (fgetline(fp, line))
          break;
//          return(2);
        buff = tokenize(line, &num_tokens);
        if (num_tokens == 3)
        {
          model->latmat[0+i] = scale*str_to_float(*(buff+0));
          model->latmat[3+i] = scale*str_to_float(*(buff+1));
          model->latmat[6+i] = scale*str_to_float(*(buff+2));
        }
        g_strfreev(buff);
        model->construct_pbc = TRUE;
        model->periodic = 3;
      }
  }

/* energy parse during steps */
/* FIXME - I think this doesn't happen (read_sout_block gets called instead) */
/* see the coordinates read (for steps) below */
  if (g_ascii_strncasecmp(line, "siesta: E_KS(eV)", 16) == 0)
    {
    buff = tokenize(line, &num_tokens);
    if (num_tokens > 3)
      {
      text = g_strdup_printf("%s eV", *(buff+3));
      property_add_ranked(3, "Energy", text, model);
      g_free(text);
      }
    g_strfreev(buff);
    }

/* energy parse for final step */
  if (g_ascii_strncasecmp(line, "siesta:         Total =", 23) == 0)
    {
    buff = tokenize(line, &num_tokens);
    if (num_tokens > 3)
      {
      text = g_strdup_printf("%s eV", *(buff+3));
      property_add_ranked(3, "Energy", text, model);
      g_free(text);
      }
    g_strfreev(buff);
    }

/* functional */
  if (g_ascii_strncasecmp(line, "xc_check: Exchange-correlation functional:", 42) == 0)
    {
    if (fgetline(fp, line))
      {
      break;
//      return(2);
      }
    if (g_ascii_strncasecmp(line, "xc_check: GGA Perdew, Burke & Ernzerhof 1996", 44) == 0)
      property_add_ranked(7, "Functional", "PBE", model);
    else if (g_ascii_strncasecmp(line, "xc_check: GGA Becke Lee Yang Parr", 33) == 0)
      property_add_ranked(7, "Functional", "BLYP", model);
    else
      property_add_ranked(7, "Functional", "Unknown", model);
    }

/* k-points */
  if (g_ascii_strncasecmp(line, "siesta: k-grid: Number of k-points", 34) == 0)
    {
    buff = g_strsplit(line, "=", 2);
    g_strchug(g_strchomp(*(buff+1)));
    property_add_ranked(10, "K-points", *(buff+1), model);
    g_strfreev(buff);
    }

/* Mesh cutoff */
  if (g_ascii_strncasecmp(line, "redata: Mesh Cutoff", 19) == 0)
  {
    buff = g_strsplit(line, "=", 2);
    text = format_value_and_units(*(buff+1), 2);
    property_add_ranked(6, "Mesh Cutoff", text, model);
    g_free(text);
    g_strfreev(buff);
  }

/* Energy Shift */
  if (g_ascii_strncasecmp(line, "SPLIT: energy shift",  19) == 0)
  {
    buff = g_strsplit(line, "=", 2);
    text = format_value_and_units(*(buff+1), 6);
    property_add_ranked(6, "Energy Shift", text, model);
    g_free(text);
    g_strfreev(buff);
  }

/* coordinates */
    if (g_ascii_strncasecmp(line, "outcoor: ", 9) == 0)
      {
        if (g_strrstr(line, "Ang") != NULL)
          model->fractional = FALSE;
        else if (g_strrstr(line, "fractional") != NULL)
          model->fractional = TRUE;
        else if (g_strrstr(line, "Bohr") != NULL)
          model->coord_units = BOHR;
        else
          {
          gui_text_show(ERROR, "unexpected coordinate type\n");
          break;
          }
/* NB: this bypass routine needs to read all properties that we wish stored */
      read_sout_block(fp, skip_coords, model);
      animate_frame_store(model);
      }

  }

model_prep(model);

/* done */
g_free(model->filename);
model->filename = g_strdup(filename);

g_free(model->basename);
model->basename = parse_strip(filename);

fclose(fp);

return(0);
}

/*************************/
/* create/update a graph */
/*************************/
/* TODO - more sophisticated scan where multiple graphs */
/* can be refreshed or created */
void siesta_graph_import(gpointer import)
{
gint i, size, num_tokens;
gdouble *y, stop;
gchar *line, **buff;
gpointer scan, graph;
GSList *energy, *item;

g_assert(import != NULL);

/* scan for list of energy values */
energy = NULL;
scan = scan_new(import_fullpath(import));
for (;;)
  {
  line = scan_get_line(scan);
  if (!line)
    break;

  if (g_strrstr(line, "siesta: E_KS(eV)"))
    {
    buff = tokenize(line, &num_tokens);
    if (num_tokens > 3)
      {
      y = g_malloc(sizeof(gdouble));
      *y = str_to_float(*(buff+3));
      energy = g_slist_prepend(energy, y);
      }
    g_strfreev(buff);
    }
  }
scan_free(scan);

/* if a list of energy values were found - create graph */
if (energy)
  {
  graph = graph_new("energy");

  size = g_slist_length(energy);
  stop = size;
  y = g_malloc(size * sizeof(gdouble));

  i=size;
  for (item=energy ; item ; item=g_slist_next(item))
    {
    i--;
    memcpy(y+i, item->data, sizeof(gdouble));
    }

  graph_set_data(size, y, 1.0, stop, graph);

  import_object_add(IMPORT_GRAPH, graph, import);

  free_slist(energy);
  }
}

/*************************/
/* siesta import routine */
/*************************/
gint siesta_input_import(gpointer import)
{
g_assert(import != NULL);

error_table_clear();

import_fdf(import);

error_table_print_all();

return(0);
}

/********************************/
/* siesta output import routine */
/********************************/
gint siesta_output_import(gpointer import)
{
gchar *filename;
struct model_pak *model, *new;

g_assert(import != NULL);

filename = import_fullpath(import);

model = model_new();

if (read_sout(filename, model))
  {
  model_delete(model);
  return(1);
  }
else
  {
/* did we find any atoms? */
  if (g_slist_length(model->cores))
    {
    import_object_add(IMPORT_MODEL, model, import);
    }
  else
    {
/* no atoms - re-read the fdf at start */
    import_fdf(import);
    new = import_object_nth_get(IMPORT_MODEL, 0, import);

/* transfer the property table from the parsed output (in model) */
/* TODO - merge rather than replace? */
    if (new)
      {
      gpointer tmp = model->property_table;
      model->property_table = new->property_table;
      new->property_table = tmp;
      }
    model_delete(model);
    }
  }

/* TODO - scan for graphs */
/* TODO - encapsulate in single pass read */
/* TODO - pass filter, get a list of graphs returned? */
/* deprec - use analysis to generate a property plot */
//siesta_graph_import(import);

return(0);
}

