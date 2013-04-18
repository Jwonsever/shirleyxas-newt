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

/************************/
/* siesta configuration */
/************************/
struct siesta_pak
{
gint num_atoms;
gint have_energy;
gint have_max_grad;
gdouble energy;
gdouble max_grad;
gdouble epot_min;
gdouble epot_max;
gdouble eden_scale;
/* cartesian + spin grid */
gdouble cell[9];
gdouble icell[9];
gint grid[4];
gdouble *eden;
gdouble *edos;
gdouble *epot;

/* replace above */
gdouble *grid_rho;
gdouble *grid_drho;
gdouble *grid_ldos;
gdouble *grid_vh;
gdouble *grid_vt;

/* speed */
gchar *solution_method;
gchar *divide_and_conquer;
gchar *occupation_function;
gchar *occupation_mp_order;
gchar *ordern_functional;
gchar *ordern_iterations;
gchar *ordern_energy_tolerance;
gchar *ordern_eta;
gchar *ordern_eta_alpha;
gchar *ordern_eta_beta;

/*
 * Electronic 
 */

gchar *xc_functional;
gchar *xc_authors;

gchar *split_zeta_norm;
gchar *energy_shift;
gchar *mesh_cutoff;
gchar *electronic_temperature;
gchar *spin_polarised;

gchar *fixed_spin;
gchar *total_spin;
gchar *non_collinear_spin;
gchar *single_excitation;

gchar *basis_type;
gchar *basis_size;
gchar *custom_zeta;
gchar *custom_zeta_polarisation;
gchar *harris_functional;
gchar *kgrid_cutoff;

/*
 * SCF
 */

gchar *no_of_cycles;
gchar *mixing_weight;
gchar *no_of_pulay_matrices;

/*
 * MD run stuff
 */

gchar *md_type_of_run;
gchar *md_variable_cell;
gchar *md_num_cg_steps;
gchar *md_max_cg_displacement;
gchar *md_max_force_tol;
gchar *md_max_stress_tol;
gchar *md_precondition_variable_cell;
gchar *md_initial_time_step;
gchar *md_final_time_step;
gchar *md_length_time_step;
gchar *md_quench;
gchar *md_initial_temperature;
gchar *md_target_temperature;
gchar *md_target_pressure;
gchar *md_target_stress[6];
gchar *md_nose_mass;
gchar *md_parrinello_rahman_mass;
gchar *md_anneal_option;
gchar *md_tau_relax;
gchar *md_bulk_modulus;
gchar *md_fc_displ;
gchar *md_fc_first;
gchar *md_fc_last;

gchar *density_of_states;
gchar *density_on_mesh;
gchar *electrostatic_pot_on_mesh;

/*
 * FILE I/O
 */
gchar *system_label;
gchar *long_output;
gchar *use_saved_data;
gchar *write_density_matrix;
gchar *write_coor_init;
gchar *write_coor_step;
gchar *write_forces;
gchar *write_kpoints;
gchar *write_eigenvalues;
gchar *write_kbands;
gchar *write_bands;
gchar *write_wavefunctions;
gchar *write_dm;
gchar *write_coor_xmol;
gchar *write_coor_cerius;
gchar *write_md_xmol;
gchar *write_md_history;
gchar *write_mullikenpop;

/* Animate frequencies */

gboolean show_vect;
gchar * freq_disp_str;
gchar * current_frequency_string;
gint current_animation;
gint num_animations;
gdouble current_frequency;
gboolean toggle_animate;
gboolean vibration_calc_complete;
gboolean animation_paused;
gdouble custom_scaler;

gpointer eigen_xyz_atom_mat;
gpointer eigen_values;

gint * sorted_eig_values;

gdouble atoms_per_job;

gchar * modelfilename;

};

