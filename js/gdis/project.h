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

gpointer project_new(const gchar *, const gchar *);
gpointer project_grid_new(const gchar *);

void project_id_set(gint, gpointer);
gint project_id_get(gpointer);

void project_data_add(const gchar *, gpointer, gpointer);
gpointer project_data_get(const gchar *, gpointer);

void project_setup_start(gpointer);
void project_setup_stop(gpointer);

void project_job_record(const gchar *, gpointer, gpointer);
void project_job_remove(const gchar *, gpointer);
void project_job_remove_all(gpointer);
gpointer project_job_lookup(const gchar *, gpointer);
gpointer project_job_lookup_or_create(const gchar *, gpointer);
GList *project_job_name_list(gpointer);

void project_job_file_append(const gchar *, const gchar *, gpointer);
void project_job_file_remove(const gchar *, gpointer);
GList *project_job_file_list(gpointer);

void project_free(gpointer);

gint project_filter_get(gpointer);
gint project_remote_filter_get(gpointer);

void project_files_refresh(gpointer);
void project_filter_set(gint, gpointer);
void project_remote_filter_set(gint, gpointer);
void project_path_set(const gchar *, gpointer);

void project_label_set(const gchar *, gpointer);
gchar *project_label_get(gpointer);
gchar *project_path_get(gpointer);
const gchar *project_fullpath_get(gpointer);
GSList *project_files_get(gpointer);

GSList *project_model_list_get(gpointer);
GSList *project_model_name_list_get(gpointer);
GSList *project_input_list_get(gpointer);

gpointer project_import_file(gpointer, const gchar *);
void project_model_add(gpointer, const gchar *, gpointer);

void project_imports_add(gpointer, gpointer);
void project_imports_remove(gpointer, gpointer);
GSList *project_imports_get(gpointer);
gpointer project_import_get(gint, gpointer);
gpointer project_parent_import(gpointer);

gchar *project_grid_jobname(gpointer);
gchar *project_remote_path_get(gpointer);
GSList *project_remote_files_get(gpointer);
void project_remote_files_set(GSList *, gpointer);

void project_import_include(gpointer, gpointer);

const gchar *project_type_get(gpointer);
const gchar *project_description_get(gpointer);
void project_description_set(const gchar *, gpointer);
gchar *project_summary_get(gpointer);
struct grid_pak *project_parse_job_details(const gchar *);

const gchar *project_executable_required(gpointer);

void project_local_stop(gpointer);
void project_local_start(gpointer);

gint project_save(gpointer);
gint project_read(const gchar *, gpointer);

/* reference counting */
void project_inc(gpointer);
gint project_dec(gpointer);
gint project_count(gpointer);

void project_browse_set(gint, gpointer);
void project_browse_init(void);
void project_browse_free(void);

void project_import_workspace(gpointer);

