/*
Copyright (C) 2008 by Sean David Fleming

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

gpointer import_new(void);
void import_free(gpointer);
gpointer import_file(const gchar *, struct file_pak *);
gpointer import_job_new(const gchar *, const gchar *, const gchar *);

void import_inc(gpointer);
gint import_dec(gpointer);
gint import_count(gpointer);

gint import_file_save(gpointer);
gint import_save_as(gchar *, gpointer);
void import_save(gchar *, gpointer);

gchar *import_label(gpointer);
gchar *import_fullpath(gpointer);
gchar *import_filename(gpointer);
gchar *import_path(gpointer);

void import_export_type_set(gpointer, gpointer);
gpointer import_export_type_get(gpointer);

gint import_keep_unknown_get(gpointer);
void import_keep_unknown_set(gint, gpointer);

void import_bigfile_set(gint, gpointer);
gint import_bigfile_get(gpointer);

gint import_grid_get(gpointer);
void import_grid_set(gint, gpointer);
void import_remote_path_set(const gchar *, gpointer);
gchar *import_remote_path_get(gpointer);
GSList *import_grid_list(void);

const gchar *import_state_get(gpointer);
void import_state_set(const gchar *, gpointer);
void import_stamp_set(gpointer);
glong import_stale_get(gpointer);

void import_label_set(const gchar *, gpointer);
const gchar *import_label_get(gpointer);
void import_filename_set(const gchar *, gpointer);
void import_fullpath_set(const gchar *, gpointer);

void import_filename_set(const gchar *, gpointer);

GSList *import_object_list_get(gint, gpointer);
void import_object_add(gint, gpointer, gpointer);
void import_object_remove(gint, gpointer, gpointer);
void import_object_free_all(gpointer);

gint import_init(gpointer);

gpointer import_find(gint, gpointer);

gpointer gamess_new(void);
gpointer gulp_new(void);
gpointer nwchem_new(void);
gpointer siesta_new(void);

gpointer config_new(gint, gpointer);
void config_free(gpointer);
gpointer config_data(gpointer);
gint config_id(gpointer);
const gchar *config_label(gpointer);
gchar *config_unparsed_get(gpointer);
void config_unparsed_set(gchar *, gpointer);
void config_grafted_set(gint, gpointer);
gint config_grafted_get(gpointer);

void gamess_free(gpointer);
void gulp_free(gpointer);
void siesta_free(gpointer);
void nwchem_free(gpointer);

gpointer import_object_nth_get(gint, gint, gpointer );
gpointer import_config_data_get(gint, gpointer);
gpointer import_config_get(gint, gpointer);

void import_preview_set(gint, gpointer);
gint import_preview_get(gpointer);
void import_preview_buffer_set(gchar *, gpointer);
const gchar *import_preview_buffer_get(gpointer);

GList *import_gui_tab_list_get(gpointer);

void import_print(gpointer);

enum {
     IMPORT_MODEL, IMPORT_GRAPH, IMPORT_CONFIG,
// extra lines -> pass through
     IMPORT_TEXT,
     IMPORT_LAST
     };

