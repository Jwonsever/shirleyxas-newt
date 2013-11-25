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

#ifdef __WIN32
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

#define KEYWORD_LEN 10

/* FORTRAN ugliness */
#define READ_RECORD fread(&sysenv.fortran_buffer, sizeof(int), 1, fp)
#define WRITE_RECORD fwrite(&sysenv.fortran_buffer, sizeof(int), 1, fp)

/* enumerated types for POV-Ray colour-style selection */
enum {HSV,	  			/*Colour-wheel style red-green-blue */
	  REDWHITEBLUE		/* Red-fades to white-fades to blue */
};

struct keyword_pak
{
gchar *label;
gint code;
};

extern struct keyword_pak keywords[];

/* main */
void file_init(void);
void file_free(void);
gint fgetline(FILE *, gchar *);
gchar *file_read_line(FILE *);
void file_skip_comment(gint);
gint file_copy(const gchar *, const gchar *);

gint set_path(const gchar *);
gchar *file_path_delta(const gchar *, const gchar *);
GSList *file_dir_list(const gchar *, gint);
GSList *file_dir_list_pattern(const gchar *, const gchar *);
GSList *file_list_filter(gint, GSList *);
GSList *file_folder_list(const gchar *);

gint file_mkdir(const gchar *);

gint file_id_get(gint, const gchar *);
struct file_pak *get_file_info(gpointer, gint);
struct file_pak *file_type_from_id(gint);
struct file_pak *file_type_from_filename(const gchar *);
struct file_pak *file_type_from_extension(const gchar *);
struct file_pak *file_type_from_label(const gchar *);

void file_load(gchar *, struct model_pak *);
void file_save(void);
gint file_save_as(gchar *, struct model_pak *);
gchar *gun(const gchar *);
gchar *file_unused_name(const gchar *, const gchar *, const gchar *);
gchar *format_value_and_units(gchar *, gint);
gint file_extension_valid(const gchar *);
gchar *file_extension_get(const gchar *);
gint file_byte_size(const gchar *);
gchar *file_find_program(const gchar *);
const gchar *file_modified_string(const gchar *);

/* dialog control */
void file_dialog(gchar *, gchar *, gint, 
                 gpointer (gchar *,  struct model_pak *), gint);

/* file writing routines */
gint write_aims(gchar *, struct model_pak *);
gint write_arc(gchar *, struct model_pak *);
gint write_cif(gchar *, struct model_pak *);
gint write_fdf(gchar *, struct model_pak *);
gint write_gulp(gchar *, struct model_pak *);
gint write_gmf(gchar *, struct model_pak *);
gint write_planes(gchar *, struct model_pak *);
gint write_marvin(gchar *, struct model_pak *);
gint write_xml(gchar *, struct model_pak *);
gint write_xtl(gchar *, struct model_pak *);
gint write_xyz(gchar *, struct model_pak *);
gint write_gms(gchar *, struct model_pak *);
gint write_diffax(gchar *, struct model_pak *);
gint write_povray(gchar *, struct model_pak *);
gint write_pdb(gchar *, struct model_pak *);
gint write_gauss(gchar *, struct model_pak *);
gint write_cssr(gchar *, struct model_pak *); 
gint write_dmol(gchar *, struct model_pak *);
gint write_dlpoly(gchar *, struct model_pak *);
gint write_bgf(gchar *, struct model_pak *);
gint write_cgf(gchar *, struct model_pak *);
gint write_dlp(gchar *, struct model_pak *);
gint write_gromacs(gchar *, struct model_pak *);
gint write_castep_cell(gchar *, struct model_pak *);
gint write_meta(gchar *, struct model_pak *);
gint write_xsf(gchar *, struct model_pak *);
gint write_pwi(gchar *, struct model_pak *);
gint write_openmx_input(gchar *, struct model_pak *);

void write_povray_colour_textures(FILE *, struct model_pak *, int);
void write_sfc_data(FILE *);

gint write_arc_header(FILE *, struct model_pak *);
gint write_arc_frame(FILE *, struct model_pak *);
gint write_trj_header(FILE *, struct model_pak *);
gint write_trj_frame(FILE *, struct model_pak *);

/* file reading routines */
gint read_aims(gchar *, struct model_pak *);
gint read_arc(gchar *, struct model_pak *);
gint read_cif(gchar *, struct model_pak *);
gint read_fdf(gchar *, struct model_pak *);
gint read_gulp(gchar *, struct model_pak *);
gint read_gulp_output(gchar *, struct model_pak *);
gint read_gmf(gchar *, struct model_pak *);
gint read_planes(gchar *, struct model_pak *);
gint read_marvin(gchar *, struct model_pak *);
gint read_mvnout(gchar *, struct model_pak *);
gint read_sout(gchar *, struct model_pak *);
gint read_xml(gchar *, struct model_pak *);
gint read_xtl(gchar *, struct model_pak *);
gint read_xyz(gchar *, struct model_pak *);
gint read_using_babel(gchar *, struct model_pak *);
gint read_gms(gchar *, struct model_pak *);
gint read_gms_out(gchar *, struct model_pak *);
gint read_diffax(gchar *, struct model_pak *);
gint read_about(gchar *, struct model_pak *);
gint read_nwchem_input(gchar *, struct model_pak *);
gint read_nwout(gchar *, struct model_pak *);
gint read_pdb(gchar *, struct model_pak *);
gint read_castep_out(gchar *, struct model_pak *);
gint read_castep_cell(gchar *, struct model_pak *);
gint read_gauss(gchar *, struct model_pak *);
gint read_gauss_out(gchar *, struct model_pak *);
gint read_gauss_cube(gchar *, struct model_pak *);
gint read_rietica(gchar *, struct model_pak *);
gint read_off(gchar *, struct model_pak *);
gint read_moldy(gchar *, struct model_pak *);
gint read_moldy_restart(gchar *, struct model_pak *);
gint read_cssr(gchar *, struct model_pak *);
gint read_cel(gchar *, struct model_pak *);
gint read_dmol(gchar *, struct model_pak *);
gint read_dlpoly(gchar *, struct model_pak *);
gint read_bgf(gchar *, struct model_pak *);
gint read_cgf(gchar *, struct model_pak *);
gint read_dlp(gchar *, struct model_pak *);
gint read_gromacs_gro(gchar *, struct model_pak *);
gint read_xsf(gchar *, struct model_pak *);
gint read_pwi(gchar *, struct model_pak *);
gint read_pwo(gchar *, struct model_pak *);
gint read_openmx_input(gchar *, struct model_pak *);

gint read_trj_header(FILE *, struct model_pak *);
gint read_trj_frame(FILE *, struct model_pak *, gint);
gint read_arc_frame(FILE *, struct model_pak *);
gint read_sout_frame(FILE *, struct model_pak *);
gint read_gms_out_frame(FILE *, struct model_pak *);
gint read_about_frame(FILE *, struct model_pak *);
gint read_nwout_frame(FILE *, struct model_pak *);
gint read_pdb_frame(FILE *, struct model_pak *);
gint read_castep_out_frame(FILE *, struct model_pak *);
gint read_gauss_out_frame(FILE *, struct model_pak *);
gint read_xyz_frame(FILE *, struct model_pak *);
gint read_dlpoly_frame(FILE *, struct model_pak *);
gint read_dmol_frame(FILE *, struct model_pak *);

gint read_pdb_block(FILE *, struct model_pak *);

/* NEW - import routines */
gint gamess_input_import(gpointer);
gint gamess_input_export(gpointer);
gint gamess_output_import(gpointer);
gint gulp_input_import(gpointer);
gint gulp_input_export(gpointer);
gint gulp_output_import(gpointer);
gint siesta_input_import(gpointer);
gint siesta_input_export(gpointer);
gint siesta_output_import(gpointer);
gint file_text_import(gpointer);
gint nwchem_input_import(gpointer);
gint nwchem_input_export(gpointer);

gint castep_import_cell(gpointer);
gint castep_export_cell(gpointer);

gint graph_import(gpointer import);
gint graph_export(gpointer import);

gchar *file_unused_directory(void);

gint load_planes(gchar *, struct model_pak *);

gint mark_trj_frames(struct model_pak *);

void import_planes(gchar *);

void swap_bytes(void *, const gint);

void gdis_blurb(FILE *);

/* parsing */
void capitals(gchar *, gchar *);
gchar **get_tokenized_line(FILE *, gint *);
gint get_keyword_code(const gchar *);

gint read_raw_frame(FILE *, gint, struct model_pak *);
gint add_frame_offset(FILE *, struct model_pak *);

gint hash_strcmp(gconstpointer, gconstpointer);

GSList *fdf_species_build(struct model_pak *);
gint fdf_species_index(gchar *, GSList *);

GSList *gromacs_read_ff(const gchar *);

void file_path_find(void);

gint file_remove_directory(const gchar *);

gpointer file_import_download(gpointer);
void file_transfer_start(gpointer);
void file_transfer_free(gpointer);
gpointer file_transfer_import_get(gpointer);

