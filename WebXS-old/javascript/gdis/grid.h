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

void grid_table_append(const gchar *, const gchar *, GSList *);
void grid_table_remove(const gchar *, const gchar *);
void grid_table_free(void);
const gchar *grid_table_queue_get(gpointer);
const gchar *grid_table_application_get(gpointer);
gpointer grid_table_entry_get(const gchar *, const gchar *);
GSList *grid_table_versions_get(gpointer);
GSList *grid_table_list_get(void);

GSList *grid_queue_list(const gchar *);

gchar *grid_version_latest(const gchar *, const gchar *);

GSList *grid_search_by_name(const gchar *);

GSList *grid_vo_list(void);

gint grid_idp_setup(gpointer, gpointer);

GSList *grid_idp_list_get(void);

gint grid_lifetime_get(void);
void grid_lifetime_set(gint);

void grid_file_list(gpointer);
gint grid_gsiftp_list(GSList **, const gchar *);
gint grid_gsiftp_get(const gchar *, const gchar *);

gchar *grid_idp_get(void);
gchar *grid_user_get(void);

gchar *grid_get_DN(gchar *);
gchar *grid_status_text(void);
gint grid_status(void);
gchar *grid_random_alpha(gint);
gchar *grid_random_alphanum(gint);
gchar *grid_hostname_get(const gchar *);
gint grid_task_start(void);
void grid_task_stop(void);
void grid_job_start(gpointer);
void grid_job_stop(gpointer);
gint grid_connect(gint);
void grid_authenticate(gint);
gint grid_auth_check(void);
void grid_auth_set(gint);
gint grid_auth_get(void);
void grid_credential_init(const gchar *);
void grid_local_proxy_init(const gchar *);
const gchar *grid_myproxy_init(const gchar *);
const gchar *grid_myproxy_put(const gchar *);
void grid_shibboleth_init(const gchar *);
void grid_myproxy_logon(void);
gint grid_disconnect(void);

// NEW - these should be the only auth calls required
// ... and maybe an idp list retrieval
gint grid_java_slcs_login(const gchar *, const gchar *, const gchar *);
gint grid_java_proxy_upload(const gchar *);

gint grid_job_submit(const gchar *, gpointer);
GSList *grid_download_job(const gchar *);

void grid_upload_add(const gchar *, gpointer);
void grid_remote_add(const gchar *, gpointer);

const gchar *grid_file_size(const gchar *, gpointer);

void grid_download_import(gpointer);
void grid_process_import(gpointer);

void grid_idp_set(const gchar *);
void grid_user_set(const gchar *);

void grid_vo_set(const gchar *, gpointer);
const gchar *grid_vo_get(gpointer);

void grid_test(const gchar *, const gchar *);
gdouble grid_pem_hours_left(const gchar *);

const gchar *grid_cert_file(void);
const gchar *grid_key_file(void);
const gchar *grid_ca_directory(void);

void grid_init(void);
void grid_free(void);

gpointer grid_job_new(void);
void grid_job_free(gpointer);
gpointer grid_job_dup(gpointer);
gint grid_job_is_local(gpointer);

enum {GRID_MYPROXY, GRID_MYPROXY_PASSWORD, GRID_LOCALPROXY, GRID_SHIBBOLETH};
enum {GRID_SERIAL, GRID_MPI}; // other parallel types?

/* stuff in a job xml file - can be filled out by querying the server */
struct grid_pak
{
/* follows file_pak/project id */
gint id;
gchar *description;
gpointer parent;

/* CURRENT - bit of a hack for MDS query/setup */
gchar *exename;
gchar *exe_version;

/* auth */
gchar *user_vo;

/* submission */
gchar *jobname;
gchar *status;
gchar *remote_q;
gchar *remote_root;
gchar *remote_cwd;
gchar *remote_offset;
gchar *remote_exe;
gchar *remote_exe_module;
gchar *remote_site;

/* TODO - pop up error/report message (ie feedback) */
gchar *report;

/* CURRENT - units are MB */
gchar *remote_memory;
/* CURRENT - units are seconds and divided by # processors */
gchar *remote_walltime;

/* processor specific */
gint remote_exe_type;
gint remote_exe_np;

/* source/output destination for the job */
gchar *local_cwd; // if NULL on download - ask user
gchar *local_input;
gchar *local_output;

/* extra data files needed for job */ 
GSList *local_upload_list;
GSList *remote_upload_list;

/* job files */
GSList *remote_file_list;
GSList *remote_size_list;
};

enum {JOB_UNKNOWN, JOB_GAMESS, JOB_GULP};

