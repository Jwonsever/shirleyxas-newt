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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gdis.h"
#include "file.h"
#include "parse.h"
#include "project.h"
#include "gui_shorts.h"
#include "gui_image.h"
#include "interface.h"
#include "dialog.h"
#include "import.h"
#include "numeric.h"
#include "grid.h"
#include "grisu_client.h"
#include "task.h"

/* top level data structure */
extern struct sysenv_pak sysenv;

/* shows local directories & files */
#define PROJECT_LOCAL_FILES "Local files"
/* shows current uploads and job parameters */
#define PROJECT_REMOTE_JOBS "Job Scheduling"
/* shows state of project type jobs & allows downloads */
#define PROJECT_REMOTE_FILES "Job Monitoring"

gpointer project_notebook;

GtkWidget *project_entry_name=NULL;

gpointer project_local_filter;

//gpointer project_entry_application;
GtkWidget *project_entry_cpus;
GtkWidget *project_entry_memory;
GtkWidget *project_entry_walltime;
GtkWidget *project_job_description;
gchar *project_task_name=NULL;

/* remote task box */
GtkWidget *project_task_remote;

/* jobs */
GtkWidget *project_tv=NULL;
GtkListStore *project_ls=NULL;

/* data */
GtkWidget *project_data_tv=NULL;
GtkListStore *project_data_ls=NULL;

GtkWidget *project_entry_path;

GtkWidget *project_job_path;
GtkWidget *project_job_name;
GtkWidget *project_remote_file_box;

gpointer project_local_folders;
gpointer project_local_files;
gpointer project_remote_files;
gpointer project_remote_details;

gpointer project_vo_widget;
gpointer project_queue_widget; 
gpointer project_input_widget; 
gpointer project_version_widget; 
gpointer project_view;

GtkWidget *project_remote_files_page;
GtkWidget *project_submit_page;
GtkWidget *project_monitor_page;
GtkWidget *project_builder_page;
GtkWidget *gui_monitor_button;

/*********************************/
/* local file list widget update */
/*********************************/
void gui_project_local_files_update(gpointer dummy)
{
gpointer project;
GSList *list;

project = tree_project_get();
if (!project)
  return;

project_files_refresh(project);
list = project_files_get(project);
gui_scrolled_list_refresh(list, project_local_files);
free_slist(list);
}

/****************************************/
/* GUI update after file list operation */
/****************************************/
void gui_project_remote_files_update(gpointer dummy)
{
gpointer project;
const gchar *jobname;
struct grid_pak *grid;

project = tree_project_get();
if (!project)
  return;

/* alternate */
jobname = gtk_entry_get_text(GTK_ENTRY(project_job_name));
grid = project_job_lookup(jobname, project);
if (grid)
  {
// no filter - just display all files
  gui_scrolled_list_refresh(grid->remote_file_list, project_remote_files);
  }
else
  {
/* job doesnt exist in project - clear everything */
  gtk_entry_set_text(GTK_ENTRY(project_job_name), "");
//  gtk_entry_set_text(GTK_ENTRY(project_job_name), grid_status_text());
  gui_scrolled_list_refresh(NULL, project_remote_files);
  }
}

/**************************************/
/* directory navigation widget update */
/**************************************/
void gui_project_navigate_update(void)
{
gchar *path;
GSList *list;

path = project_path_get(sysenv.workspace);
list = file_folder_list(path);

// widget checks now that file open is a dialog
if (dialog_exists(FILE_LOAD, NULL))
  {
  gtk_entry_set_text(GTK_ENTRY(project_entry_path), path);
  gui_scrolled_list_refresh(list, project_local_folders);
  }

gui_project_local_files_update(sysenv.workspace);
}

/*********************************/
/* directory navigation callback */
/*********************************/
void gui_project_navigate(GtkWidget *w, gpointer dummy)
{
gchar *new;
gpointer project;
GSList *list;

project = tree_project_get();
if (!project)
  return;

list = gui_scrolled_list_selected_get(project_local_folders);
if (!list)
  return;

//printf("clicked: [%s]\n", (gchar *) list->data);

new = file_path_delta(project_path_get(project), list->data);

if (new)
  {
//printf("changing to: %s\n", new);
  project_path_set(new, project);
  g_free(new);
  gui_project_navigate_update();
  }
else
  printf("Can't change directory, check permissions.\n");

}

/******************************/
/* go to users home directory */
/******************************/
void gui_project_navigate_home(GtkWidget *w, gpointer dummy)
{
project_path_set(g_get_home_dir(), sysenv.workspace);
gui_project_navigate_update();
}

/******************************/
/* go to users home directory */
/******************************/
void gui_project_navigate_downloads(GtkWidget *w, gpointer dummy)
{
project_path_set(sysenv.download_path, sysenv.workspace);
gui_project_navigate_update();
}

/***********************/
/* create a new folder */
/***********************/
void gui_project_navigate_create(GtkWidget *w, gpointer data)
{
const gchar *text;
gchar *fullpath;
gpointer project;

project = tree_project_get();
if (!project)
  return;

text = gtk_entry_get_text(GTK_ENTRY(data));
if (!strlen(text))
  return;

fullpath = g_build_filename(project_path_get(project), text, NULL);

//printf("mkdir [%s]\n", fullpath);

g_mkdir(fullpath, 0700);

g_free(fullpath);

gui_project_navigate_update();
}

/*********************/
/* directory removal */
/*********************/
void gui_project_navigate_delete(GtkWidget *w, gpointer data)
{
gint n;
gchar *text, *fullpath;
const gchar *path;
gpointer project;
GSList *item, *list;

g_assert(data != NULL);

project = tree_project_get();
if (!project)
  return;

list = gui_scrolled_list_selected_get(data);
if (!list)
  return;

n = g_slist_length(list);
if (n > 1)
  text = g_strdup_printf("Please confirm removal of: %d directories\n", n);
else
  text = g_strdup_printf("Please confirm removal of: %s\n", (gchar *) list->data);

/* remove the selected item(s) */
if (gui_popup_confirm(text))
  {
  path = project_path_get(project);
  for (item=list ; item ; item=g_slist_next(item))
    {
    fullpath = g_build_filename(path, item->data, NULL);

/* glib's remove functions act like rm, but without any way of adding -rf */
/*
    g_remove(fullpath);
*/
    file_remove_directory(fullpath);

    g_free(fullpath);
    }
  }

g_free(text);

gui_project_navigate_update();
}

/************************************/
/* navigation tab directories/files */
/************************************/
void gui_project_navigate_widget(GtkWidget *box)
{
GtkWidget *vbox, *hbox, *hbox2, *entry;

vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);

project_local_folders = gui_scrolled_list_new(vbox);

g_signal_connect(GTK_OBJECT(project_local_folders), "row-activated",
                 GTK_SIGNAL_FUNC(gui_project_navigate), NULL);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

entry = gtk_entry_new();
gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
g_signal_connect(GTK_OBJECT(entry), "activate",
                 GTK_SIGNAL_FUNC(gui_project_navigate_create), entry);

hbox2 = gtk_hbox_new(TRUE, 0);
gtk_box_pack_end(GTK_BOX(hbox), hbox2, TRUE, TRUE, 0);

gui_button("Create", gui_project_navigate_create, entry, hbox2, TT);
gui_button("Delete", gui_project_navigate_delete, project_local_folders, hbox2, TT);
//gui_button("Home", gui_project_navigate_home, NULL, hbox2, TT);
}

/*************************************/
/* local file filter change callback */
/*************************************/
void gui_project_local_filter(const gchar *text)
{
struct file_pak *file;

//printf("set local filter: %s\n", text);

file = file_type_from_label(text);
if (!file)
  {
  printf("Unknown file type.\n");
  return;
  }

project_filter_set(file->id, sysenv.workspace);

/* redraw local file list */
gui_project_local_files_update(sysenv.workspace);
}

/*******************************/
/* update the job monitor list */
/*******************************/
void gui_project_monitor_widget_update(void)
{
const gchar *name, *status;
gpointer project;
GList *item, *list;
GtkTreeIter iter;
struct grid_pak *grid;

project = tree_project_get();
if (!project)
  return;

/* update job monitoring widget */
list = project_job_name_list(project);

gtk_list_store_clear(project_ls);
for (item=list ; item ; item=g_list_next(item))
  {
  name = item->data;

  grid = project_job_lookup(name, project);
  if (grid)
    status = grid->status;
  else
    status = "unknown";

  gtk_list_store_append(project_ls, &iter);
  gtk_list_store_set(project_ls, &iter, 0, name, 1, status, -1);
  }
g_list_free(list);
}

/********************************/
/* update the project data list */
/********************************/
void gui_project_data_widget_update(void)
{
const gchar *label, *name;
gpointer project;
GList *item, *list;
GtkTreeIter iter;

//printf("project_data_widget_update()\n");

project = tree_project_get();
if (!project)
  return;

/* convert hash table key (double linked) list to single linked list */
/* TODO - want the list of values ie imports ... (not keys) */
list = project_job_file_list(project);
gtk_list_store_clear(project_data_ls);
for (item=list ; item ; item=g_list_next(item))
  {
/* TODO - would be nice (eg tooltip?) to see where the include is coming from if remote */
/* but not really enough room in side tree for the full label name */
  label = import_label_get(item->data);
  if (!label)
    label = "Local: ";
  else
    label = "Remote: ";

  name = import_filename(item->data);

  gtk_list_store_append(project_data_ls, &iter);
  gtk_list_store_set(project_data_ls, &iter, 0, label, 1, name, -1);
  }
g_list_free(list);
}

/*******************************************************/
/* update the local/remote specifics of the task panel */
/*******************************************************/
void gui_project_task_view_update(GtkWidget *w, gpointer dummy)
{
gchar *queue;
gchar *filename;
gpointer entry;
const gchar *exe; 
const GSList *list;

queue = gui_pd_text(project_queue_widget);

//printf("queue = [%s]\n", queue);

if (strlen(queue))
  {
  if (g_strncasecmp("local", queue, 5) == 0 || g_strncasecmp("none", queue, 4) == 0)
    {
    gtk_widget_hide(project_task_remote);
    }
  else
    {
    filename = gui_pd_text(project_input_widget);
    exe = task_executable_required(filename);

    entry = grid_table_entry_get(exe, queue);

    list = grid_table_versions_get(entry);

    gui_pd_list_set(list, project_version_widget);

    g_free(filename);

/* use current filename to update queue and version locations */
    gtk_widget_show(project_task_remote);
    }
  }
else
  gtk_widget_hide(project_task_remote);

g_free(queue);
}

/*********************************************************/
/* update the destination list based on given input file */
/*********************************************************/
void gui_project_queue_update(const gchar *input)
{
const gchar *exe;
GSList *list=NULL;

/* get required executable */
exe = task_executable_required(input);

//printf("input = %s ; exe = %s\n", input, exe);

if (exe)
  {
/* set executable name in task dialog */
  gtk_entry_set_text(GTK_ENTRY(project_job_description), exe);

/* get remote submission locations */
  if (grid_auth_get())
    list = grid_queue_list(exe);
  }
else
  gtk_entry_set_text(GTK_ENTRY(project_job_description), "");

/* only add local host IF the exe is locally available */
if (task_executable_available(input))
  list = g_slist_prepend(list, "local host");

// fallback label if no remote or local submission locations found
if (!list)
  list = g_slist_prepend(list, "none available");

gui_pd_list_set(list, project_queue_widget);

g_slist_free(list);
}

/********************************/
/* update the task control pane */
/********************************/
void gui_project_task_update(gpointer project)
{
gchar *filename;
GSList *list=NULL;

if (!project)
  return;

/* grid tab */
/* set VOs */
list = grid_vo_list();
gui_pd_list_set(list, project_vo_widget);
gui_project_data_widget_update();
gui_project_monitor_widget_update();

/* update list of filenames */
list = project_input_list_get(project);
gui_pd_list_set(list, project_input_widget);
// NB: yes, this is the correct free, since gui_pd_list_set() only shallow copies the list
g_slist_free(list);

/* use current filename to update queue and version locations */
filename = gui_pd_text(project_input_widget);

gui_project_queue_update(filename);

g_free(filename);

/* update widget visibility (based on destination queue) */
gui_project_task_view_update(NULL, NULL);
}

/*****************************************************/
/* update the project widget due to a project change */
/*****************************************************/
void gui_project_widget_update(gpointer project)
{
if (!project)
  return;

/* TODO - incorporate in notebook as refresh functions ... */
/* title */
//gtk_entry_set_text(GTK_ENTRY(project_entry_name), project_label_get(project));
//gtk_entry_set_text(GTK_ENTRY(project_entry_description), project_description_get(project));

/* file browser tab */
gui_project_navigate_update();

/* task control tab */
//gui_project_task_update(project);

/* grid control tab */
//gui_project_remote_files_update(project);


}

/*****************************************************/
/* update the current project due to a widget action */
/*****************************************************/
void gui_project_widget_event(GtkWidget *w, gpointer dummy)
{
/* should only be triggerable if project selected */

printf("TODO - update project\n");


}

/**************************************************/
/* free clipboard when file open dialog destroyed */
/**************************************************/
GSList *project_clipboard=NULL;
void gui_project_clipboard_free(GtkWidget *w, gpointer dummy)
{
free_slist(project_clipboard);
project_clipboard=NULL;
}

/****************************/
/* record current selection */
/****************************/
void gui_project_local_copy(GtkWidget *w, gpointer dummy)
{
gchar *fullpath;
GSList *item, *list;

free_slist(project_clipboard);
project_clipboard=NULL;

list = gui_scrolled_list_selected_get(project_local_files);
for (item=list ; item ; item=g_slist_next(item))
  {
  fullpath = g_build_filename(project_path_get(sysenv.workspace), (gchar *) item->data, NULL);

  project_clipboard = g_slist_prepend(project_clipboard, fullpath);
  }

free_slist(list);
}

/****************************/
/* paste recorded selection */
/****************************/
// TODO - not 100% sure this approach is best
void gui_project_local_paste(GtkWidget *w, gpointer dummy)
{
GSList *item;

printf("copy dest: %s\n", project_path_get(sysenv.workspace));

for (item=project_clipboard ; item ; item=g_slist_next(item))
  {
printf("[%s]\n", (gchar *) item->data);
  }
}

/**********************/
/* remove local files */
/**********************/
void gui_project_local_remove(GtkWidget *w, gpointer dummy)
{
gchar *fullpath;
gpointer project;
GSList *item, *list;

project = tree_project_get();
if (!project)
  return;

list = gui_scrolled_list_selected_get(project_local_files);
for (item=list ; item ; item=g_slist_next(item))
  {
  fullpath = g_build_filename(project_path_get(project), (gchar *) item->data, NULL);
// TODO - shell quote under windows?
  g_remove(fullpath);

  g_free(fullpath);
  }

free_slist(list);

gui_project_local_files_update(NULL);
}

/**************************/
/* local include callback */
/**************************/
void gui_project_local_include(GtkWidget *w, gpointer dummy)
{
gchar *fullpath;
gpointer project;
GSList *item, *list;

project = tree_project_get();
if (!project)
  return;

list = gui_scrolled_list_selected_get(project_local_files);
for (item=list ; item ; item=g_slist_next(item))
  {
  fullpath = g_build_filename(project_path_get(project), (gchar *) item->data, NULL);

//printf("Adding local file: %s\n", fullpath);

  project_job_file_append(NULL, fullpath, project);

  g_free(fullpath);
  }

free_slist(list);

gui_project_data_widget_update();
}

/*************************/
/* local import callback */
/*************************/
void gui_project_local_import(GtkWidget *w, gpointer dummy)
{
gchar *fullpath;
gpointer project, import=NULL;
GSList *item, *list;

project = tree_project_get();
if (!project)
  return;

list = gui_scrolled_list_selected_get(project_local_files);
for (item=list ; item ; item=g_slist_next(item))
  {
  fullpath = g_build_filename(project_path_get(project), (gchar *) item->data, NULL);

  import = project_import_file(project, fullpath);

  g_free(fullpath);
  }

free_slist(list);

/* successful import - determine what should be drawn by default */
if (import)
  {
/* look for a model */
  list = import_object_list_get(IMPORT_MODEL, import);
/* nothing - look for a graph */
  if (!list)
    list = import_object_list_get(IMPORT_GRAPH, import);
/* select item */
  if (list)
    {
    gui_tree_selected_set(list->data);
    sysenv.workspace_changed = TRUE;
    }
  else
    {
/* no models or graphs - just select the file's imported text preview */
    gui_tree_selected_set(import);
    }

/* NEW - close the dialog on success */
/* not sure always closing after user clicks open on a file is correct */
/* behaviour, but it seems to be expected by most users... */
  dialog_destroy_type(FILE_LOAD);
  }

/* update widget info */
//gui_project_widget_update(project);

}

/***************************/
/* remote include callback */
/***************************/
void gui_project_remote_include(GtkWidget *w, gpointer dummy)
{
gchar *fullpath;
const gchar *name;
gpointer project;
GSList *item, *list;
struct grid_pak *grid=NULL;

project = tree_project_get();
if (!project)
  return;

name = gtk_entry_get_text(GTK_ENTRY(project_job_name));

grid = project_job_lookup(name, project);
if (!grid)
  return;

list = gui_scrolled_list_selected_get(project_remote_files);
for (item=list ; item ; item=g_slist_next(item))
  {
  fullpath = g_build_filename(grid->remote_cwd, (gchar *) item->data, NULL);

/* TODO - report this somewhere */
/*
printf("[%s] size = %s\n", (gchar *) item->data, grid_file_size(item->data, grid));
*/

  project_job_file_append(grid->jobname, fullpath, project);

  g_free(fullpath);
  }
free_slist(list);

gui_project_data_widget_update();
}


/*******************************/
/* post background job cleanup */
/*******************************/
void gui_download_browse(gpointer data, gpointer dummy)
{
gchar *dir = data;

if (gui_popup_confirm("Browse completed download folder?"))
  {
  project_path_set(dir, sysenv.workspace);
  gui_import_add_dialog();
  }

g_free(dir);
}

/*******************************/
/* post background job cleanup */
/*******************************/
void gui_project_import_download_cleanup(gpointer import)
{
if (import)
  import_free(import);
}

/*******************************/
/* post background job cleanup */
/*******************************/
// TODO - use file transfer pak primitives to redo this
void gui_project_import_download_open(gpointer source)
{
gpointer import;

if (!source)
  return;

/* read filename as fresh import */
import = gui_import_new(import_fullpath(source));

/* NEW - copy remote path (source url) to enable monitoring */
import_remote_path_set(import_remote_path_get(source), import);

/* free download import marker */
import_free(source);

/* select new import */
gui_tree_selected_set(import);
}

/*********************/
/* download callback */
/*********************/
void gui_project_download(GtkWidget *w, gpointer cleanup)
{
gint local_flag = FALSE;
gchar *localpath, *label, *dir=NULL;
const gchar *name;
gpointer project, import;
GSList *item, *list;
struct grid_pak *grid=NULL;

g_assert(cleanup != NULL);

project = tree_project_get();
if (!project)
  return;

name = gtk_entry_get_text(GTK_ENTRY(project_job_name));

grid = project_job_lookup(name, project);
if (!grid)
  return;

/* NEW - if not a local job - assign a unique directory */
if (!grid_job_is_local(grid))
  {
#ifdef __WIN32
// win32 workaround (doesn't like colons in directory names)
  gchar *tmp = g_strdup(name);
  tmp = g_strdelimit(tmp, ":", '-');
  dir = g_build_filename(sysenv.download_path, tmp, NULL);
  g_free(tmp);
#else
  dir = g_build_filename(sysenv.download_path, name, NULL);
#endif

  if (!file_mkdir(dir))
    {
printf("Fatal error creating: %s\n", dir);
    return;
    }
  g_free(grid->local_cwd);
  grid->local_cwd = g_strdup(dir);
  }

/* for all items in selection - download */
list = gui_scrolled_list_selected_get(project_remote_files);
for (item=list ; item ; item=g_slist_next(item))
  {
  if (grid_job_is_local(grid))
    {
/* local file */
    localpath = g_build_filename(grid->local_cwd, item->data, NULL);
    import = import_file(localpath, NULL);
    if (import)
      {
      project_imports_add(sysenv.workspace, import);
      gui_tree_append_import(import);
      }
    else
      local_flag = TRUE;
    }
  else
    { 
/* TODO - download additional assoc data files automatically? */
/* TODO - copy to a new unused sub-dir off the project path */
/* TODO - cleanup function opens a file browser on this */
/* remote file */
//    localpath = g_build_filename(project_path_get(project), item->data, NULL);
    localpath = g_build_filename(grid->local_cwd, item->data, NULL);

    import = import_new();
    import_fullpath_set(localpath, import);
    import_remote_path_set(grid->remote_cwd, import);
    label = g_strdup_printf("Download %s", (gchar *) item->data); 
    task_new(label, grid_download_import, import, cleanup, import, NULL);
    g_free(localpath);
    g_free(label);
    }
  }

/* NEW - if not a local job - assign a final background task to browse completed downloads */
// causes win32 to hang 
// removing - since file browser now has a quick navigate option anyway
/*
if (dir)
  {
  task_new("Browse", NULL, NULL, gui_download_browse, dir, NULL);
  }
*/

if (local_flag)
  gui_popup_message("Warning: missing or unreadable directories/files.");

/* NEW - if something selected for open - assume we can close dialog now that we've done it */
if (g_slist_length(list))
  {
  dialog_destroy_type(TASKMAN);
  }

free_slist(list);
}

/*********************************/
/* project upload and submission */
/*********************************/
/* TODO - check for interrupts in between uploads */
void gui_project_submit_start(gpointer data)
{
gint n;
gint alen, rlen;
gchar *jobxml, *tmp;
gchar *fs_absolute, *fs_relative;
gchar *local, *remote;
gdouble current=0.0, inc=0.1;
GSList *list;
GHashTable *details;
struct grid_pak *grid=data;

g_assert(grid != NULL);

/* I think this could only happen if a site goes offline or removes the queue */
grid->remote_site = grisu_site_name(grid->remote_q);
if (!grid->remote_site)
  {
  g_free(grid->report);
  grid->report = g_strdup_printf("Failed to locate queue [%s].\n\nContact administrator for site [%s].", grid->remote_q, grid->remote_site);
  return;
  }

//printf("INPUT: %s\n", grid->local_input);

/* CURRENT - due to gamess versions numbered by date - which breaks */
/* the traditional version compare routine and makes it a lot harder */
/* to auto determine the latest ... just make the user pick a version */
/*
g_free(grid->exe_version);
grid->exe_version = grid_version_latest(grid->exename, grid->remote_site);
*/

/* TODO - better test of failure to get version (eg blank) */
if (!grid->exe_version)
  printf("Warning: could not determine executable version of [%s] at [%s]\n", grid->exename, grid->remote_site);
else
  printf("Latest version: %s\n", grid->exe_version);

/*
list = grisu_application_versions(grid->exename, grid->remote_site);
if (list)
  {
  if (grid->exe_version)
    g_free(grid->exe_version);
  grid->exe_version = g_strdup(list->data); 
  }
else
  {
  printf("Warning: could not determine executable version of [%s] at [%s]\n", grid->exename, grid->remote_site);
  }
*/

details = grisu_application_details(grid->exename, grid->remote_site);
if (!details)
  {
  g_free(grid->report);
  grid->report = g_strdup_printf("Failed to get MDS description for [%s].\n\nContact administrator for site [%s].", grid->exename, grid->remote_site);
  return;
  }

tmp = g_hash_table_lookup(details, "Executables");
if (!tmp)
  {
  g_free(grid->report);
  grid->report = g_strdup_printf("Bad MDS description for application [%s].\n\nContact administrator for site [%s].", grid->exename, grid->remote_site);
  return;
  }
if (!strlen(tmp))
  {
  g_free(grid->report);
  grid->report = g_strdup_printf("Bad MDS description for application [%s].\n\nContact administrator for site [%s].", grid->exename, grid->remote_site);
  return;
  }

grid->remote_exe = g_strdup(tmp);

tmp = g_hash_table_lookup(details, "Module");
if (tmp)
  {
  grid->remote_exe_module = g_strdup(tmp);
  fprintf(stderr, "Requiring module [%s]\n", grid->remote_exe_module);
  }
else
  printf("Empty module field\n");

/* CURRENT - if parallel version is available then always use that */
/* since mpirun will work with 1 processor and programs compiled with */
/* mpi require mpirun to work ... even if you're using 1 processor */
/* because you need certain functionality (eg GULP) */

/*
switch (grid->remote_exe_np)
  {
  case 1:
printf("Attempting serial submission...\n");
    grid->remote_exe_type = GRID_SERIAL;
    break;

  default:
printf("Attempting MPI submission...\n");
    grid->remote_exe_type = GRID_MPI;
    break;
  }
*/

/* set to serial, unless parallel is available - if so - force that usage */
grid->remote_exe_type = GRID_SERIAL;
tmp = g_hash_table_lookup(details, "parallelAvail");
if (tmp)
  {
  if (g_strcasecmp(tmp, "true") == 0)
    {
    grid->remote_exe_type = GRID_MPI;
    fprintf(stderr, "Forcing MPI invoke...\n");
    }
  }

/* create the job, so we know destination filesystem */
if (!grisu_jobname_request(grid->jobname))
  {
/* at this point - grisu back-end error/timeout probably */
  grid->report = g_strdup_printf("Failed to create job: duplicate job name or remote server error.\n");
  return;
  }

/* calculate mount point */
fs_absolute = grisu_absolute_job_dir(grid->jobname, grid->remote_q, grid->user_vo);
if (!fs_absolute)
  {
/* report incorrect VO or ask sys admins to map you */
  g_free(grid->report);
  grid->report = g_strdup_printf("Site [%s] does not support VO [%s].\n\nContact site administrator or use a different VO.", grid->remote_site, grid->user_vo);

  grisu_job_remove(grid->jobname);
  return;
  }

fs_relative = grisu_relative_job_dir(grid->jobname);
g_assert(fs_relative != NULL);

/* NEW - store this for the job XML */
grid->remote_offset = g_strdup(fs_relative);

alen = strlen(fs_absolute);
rlen = strlen(fs_relative);
if (alen > rlen)
  grid->remote_root = g_strndup(fs_absolute, alen-rlen);
else
  {
  printf("Bad filesystem returned by grisu.\n");
// cope as best we can
  grid->remote_root = g_strdup(fs_absolute);
  }


/* CURRENT - upload dummy file first (commons-vfs bug workaround) */
/*
remote = g_build_filename(fs_absolute, "dummy.txt", NULL);
printf("Fudging for commons-vfs crappiness: %s\n", remote);
if (!grisu_file_upload("/home/sean/dummy.txt", remote))
  printf("Dummy upload failed.\n");
else
  printf("Dummy upload success.\n");
g_free(remote);
*/


/* upload items */
n = g_slist_length(grid->local_upload_list);
if (n)
  inc = 1.0 / (gdouble) n;

for (list=grid->local_upload_list ; list ; list=g_slist_next(list))
  {
  local = g_path_get_basename(list->data);
  remote = g_build_filename(fs_absolute, local, NULL);

//printf("(+) upload [%s] -> [%s]\n", (gchar *) list->data, remote);

  if (!grisu_file_upload(list->data, remote))
    {
/* CURRENT - cope with a bad filesystem */
/* here, we have the job directory but we cant upload - either */
/* it's down or mds is screwy and user doesnt have permissions */

    grid->report = g_strdup_printf("Failed to upload file(s) to site [%s] using VO [%s].\n\nContact site administrator or use a different VO.", grid->remote_site, grid->user_vo);

    g_free(local);
    g_free(remote);
    grisu_job_remove(grid->jobname);
    return;
    }

  g_free(local);
  g_free(remote);

  current += inc;
  }

/* remote data files */
for (list=grid->remote_upload_list ; list ; list=g_slist_next(list))
  {
  local = g_path_get_basename(list->data);
  remote = g_build_filename(fs_absolute, local, NULL);

//printf("(+) copy [%s] -> [%s]\n", (gchar *) list->data, remote);

  grisu_file_copy(list->data, remote);

  g_free(local);
  g_free(remote);
  }

/* create job description, attach and submit */

jobxml = grisu_xml_create(grid);

//printf(" *** XML:\n%s\n", jobxml);

grisu_job_configure(grid->jobname, jobxml);

/* don't submit when debugging... */
#define SUBMIT 1
#if SUBMIT
grisu_job_submit(grid->jobname, grid->user_vo);
#endif

g_free(jobxml);

}

/**************************/
/* cleanup and GUI update */
/**************************/
void gui_project_submit_stop(gpointer data)
{
struct grid_pak *grid=data;

/* TODO - free the grid pak? */
/* only free if failed? ... otherwise it was registered in project table? */

gui_project_monitor_widget_update();

/* NEW - reporting */
if (grid->report)
  {
  gui_popup_message(grid->report);
  g_free(grid->report);
  grid->report=NULL;
  }
else
  gui_text_show(STANDARD, "Job submission complete.\n");
}

/*******************************/
/* the new submission kick off */
/*******************************/
void gui_project_submit(GtkWidget *w, gpointer dummy)
{
gint confirm, local, cpus, memory, walltime;
gchar *text, *msg, *tmp, *filename;
gchar *queue, *executable, *input, *version;
gpointer project;
GList *item, *list;
GSList *sitem, *slist;
struct grid_pak *grid;
struct file_pak *file;

/* get project */
project = tree_project_get();
if (!project)
  {
printf("gui_project_submit() error: no project\n");
  return;
  }

/* get widget values */
executable = g_strdup(gtk_entry_get_text(GTK_ENTRY(project_job_description)));
queue = gui_pd_text(project_queue_widget);
input = gui_pd_text(project_input_widget);
version = gui_pd_text(project_version_widget);

if (!strlen(queue))
  {
  gui_popup_message("No valid destination found");
  return;
  }

/* prompt for confirmation */
msg = g_strdup_printf("Execute [%s] job on [%s]?", executable, queue);
confirm = gui_popup_confirm(msg);
g_free(msg);
if (!confirm)
  goto gui_project_submit_done;


if (g_strncasecmp(queue, "local", 5) == 0)
  {
  local = TRUE;
  }
else if (g_strncasecmp(queue, "none", 4) == 0)
  {
  gui_text_show(ERROR, "No valid destination for this job was found.\n");
  goto gui_project_submit_done;
  }
else
  {
  if (!grid_auth_get())
    {
    gui_text_show(ERROR, "Not authenticated.\n");
    goto gui_project_submit_done;
    }
  local = FALSE;
  }

/* task setup */
grid = grid_job_new();
grid->parent = project;
grid->local_cwd = file_unused_directory();

//printf("Temporary local directory: %s\n", grid->local_cwd);

/* queue */
grid->remote_q = g_strdup(queue);

/* description */
if (local)  
  grid->description = g_strdup("Local Task");
else
  grid->description = g_strdup("Remote Task");

/* file setup */
/* copy all project files to job temp directory */
list = project_job_file_list(project);
for (item=list ; item ; item=g_list_next(item))
  {
  text = import_fullpath(item->data);

/* build fullpath filename of copy */
/* NB: this import reference is NOT the same as the project tree imports list */
//  job->local_upload_list = g_slist_prepend(job->local_upload_list, filename);
//  grid_local_add(filename, grid);
  if (import_label_get(item->data))
    grid_remote_add(text, grid);
  else
    {
/* list of local files to upload/use */
    tmp = g_path_get_basename(text);
    filename = g_build_filename(grid->local_cwd, tmp, NULL);
    g_free(tmp);
    file_copy(text, filename);
    grid_upload_add(filename, grid);
    }
  }

/* overwrite copied files with text buffer contents */
slist = project_imports_get(project);
for (sitem=slist ; sitem ; sitem=g_slist_next(sitem))
  {
/* build fullpath filename of copy */
  text = import_fullpath(sitem->data);
  tmp = g_path_get_basename(text);
  filename = g_build_filename(grid->local_cwd, tmp, NULL);
  g_free(tmp);

/* flush current copy (may have been altered in the text buffer) to job directory */
  import_save_as(filename, sitem->data);
  g_free(filename);
  }

//      grid->id = file->id;
grid->exename = g_strdup(executable);
grid->exe_version = g_strdup(version);

/* if a job name supplied - attempt to use */
if (project_task_name)
  {
  if (strlen(project_task_name))
    {
    grid->jobname = g_strdup(project_task_name);
    }
  }
/* otherwise, default to timestamp */
if (!grid->jobname)
  grid->jobname =  g_strdup_printf("%s_%s", grid->exename, timestamp());

grid->local_input = g_strdup(input);

/* determine output filename */
file = file_type_from_filename(grid->local_input);
if (file)
  {
  switch (file->id)
    {
    case FDF:
      grid->local_output = parse_extension_set(grid->local_input, "sot");
      break;
    case GAMESS:
      grid->local_output = parse_extension_set(grid->local_input, "gmot");
      break;
    case GULP:
      grid->local_output = parse_extension_set(grid->local_input, "got");
      break;
    }
  }

if (local)
  {
task_new("tmp", &project_local_start, grid, &project_local_stop, grid, NULL);
  }
else
  {

/* cpus */
/* NB: used in walltime calculation */
cpus = str_to_float(gtk_entry_get_text(GTK_ENTRY(project_entry_cpus)));
grid->remote_exe_np = cpus;

/* memory */
memory = str_to_float(gtk_entry_get_text(GTK_ENTRY(project_entry_memory)));
/* gdis is MB -> jsdl (bytes) */
//memory *= 1048576;
memory *= 1024;
grid->remote_memory = g_strdup_printf("%d", memory);

/* walltime */
walltime = str_to_float(gtk_entry_get_text(GTK_ENTRY(project_entry_walltime)));
walltime *= 60*cpus;
grid->remote_walltime = g_strdup_printf("%d", walltime);

/* vo */
grid->user_vo = gui_pd_text(project_vo_widget);

/* TODO - jobcode deprec - get rid of it */
//grid->jobcode = grisu_job_type(grid->exename);

/* attach jobname to project */
//project_job_record(grid->jobname, grid, project);

/* execute the grid job (uploads + submission) in the background */
task_new("Submitting", &gui_project_submit_start, grid,
                       &gui_project_submit_stop, grid, NULL);

  }

// NEW - record local jobs too
project_job_record(grid->jobname, grid, project);
// NEW - force update (so local jobs register)
gui_project_monitor_widget_update();

sysenv.workspace_changed = TRUE;

gui_project_submit_done:;

/* cleanup */
g_free(executable);
g_free(queue);
g_free(input);
g_free(version);
}

/*******************/
/* action callback */
/*******************/
void project_job_kill(GtkWidget *w, gpointer dummy)
{
gint local;
gchar *jobname, *label;
const gchar *name;
gpointer project, grid;
GList *item, *list;
GtkTreeModel *treemodel;
GtkTreeSelection *selection;
GtkTreeIter iter;

project = tree_project_get();
if (!project)
  return;

treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(project_tv));
selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(project_tv));
list = gtk_tree_selection_get_selected_rows(selection, &treemodel);
if (!list)
  return;

/* NB: remove in reverse order so we dont get the iterators confused */
for (item=g_list_reverse(list) ; item ; item=g_list_next(item))
  {
  if (gtk_tree_model_get_iter(treemodel, &iter, item->data))
    {
    gtk_tree_model_get(treemodel, &iter, 0, &name, -1);

/* determine if it's a local job */
    jobname = g_strdup(name);
    grid = project_job_lookup(jobname, sysenv.workspace);
    if (grid_job_is_local(grid))
      local=TRUE;
    else
      local=FALSE;

/* remove from GUI display */
    gtk_list_store_remove(project_ls, &iter);

/* remove internal reference */
    project_job_remove(jobname, sysenv.workspace);

/* background grisu call */
/* TODO - check auth */
    if (!local)
      {
      label = g_strdup_printf("Removing %s", jobname);
      task_new(label, grisu_job_remove, jobname, g_free, jobname, NULL);
      g_free(label);
      }
    }
  }
}

/****************************************/
/* stop running tasks / cancel grid job */
/****************************************/
void gui_project_cancel(GtkWidget *w, gpointer dummy)
{
task_kill_selected();
}

/********************************************************/
/* background job for getting the project type job list */
/********************************************************/
void project_jobs_refresh(gpointer data)
{
GSList *item, *list;
struct grid_pak *grid;

if (data)
  {
/* if we were passed a list - refresh the items in that */
  list = data;
  for (item=list ; item ; item=g_slist_next(item))
    {
    grid = project_job_lookup_or_create(item->data, sysenv.workspace);
    grisu_job_refresh(grid);
    }
  }
else
  {
/* empty list - rebuild the full job list from scratch */
  list = grisu_job_names();
  for (item=list ; item ; item=g_slist_next(item))
    {
    grid = project_job_lookup_or_create(item->data, sysenv.workspace);
    grisu_job_refresh(grid);
    }
  free_slist(list);
  }
}

/*******************************/
/* completely refresh job list */
/*******************************/
void gui_project_jobs_refresh(GtkWidget *w, gpointer dummy)
{
gchar *label;
const gchar *name;
GList *item, *list;
GSList *job_list=NULL;
GtkTreeModel *treemodel;
GtkTreeSelection *selection;
GtkTreeIter iter;

/* NEW - auth popup */
if (!grid_auth_get())
  {
  gui_text_show(ERROR, "Not authenticated.\n");
//  gui_grid_dialog();
  return;
  }

treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(project_tv));
selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(project_tv));

list = gtk_tree_selection_get_selected_rows(selection, &treemodel);

/* NB: remove in reverse order so we dont get the iterators confused */
for (item=list ; item ; item=g_list_next(item))
  {
  if (gtk_tree_model_get_iter(treemodel, &iter, item->data))
    {
    gtk_tree_model_get(treemodel, &iter, 0, &name, -1);

    job_list = g_slist_prepend(job_list, g_strdup(name));
    }
  }

/* NEW - if no list => refresh all => display please wait msg */
if (!job_list)
  {
  gtk_list_store_clear(project_ls);
  gtk_list_store_append(project_ls, &iter);
  gtk_list_store_set(project_ls, &iter, 0, "Initializing, please wait...", 1, "", -1);
  label = g_strdup_printf("Refreshing Job List");
  }
else
  label = g_strdup_printf("Refreshing Single Job");

/* HACK - since the job list will be rebuilt we will lose the currently */
/* selected job ... which means the right pane will also lose data */
/* consequently we blank the entry that stores the currently selected */
/* jobname as it will be misleading */
gtk_entry_set_text(GTK_ENTRY(project_job_name), "");

/* NEW - allow the GUI to be updated before we run our refresh */
while (gtk_events_pending())
  gtk_main_iteration();

/* CURRENT - seems to be a deep race condition doing this as a background */
/* task (only apears under some conditions/platforms) so just run in fore */
//task_new(label, project_jobs_refresh, job_list, gui_project_monitor_widget_update, NULL, NULL);

project_jobs_refresh(job_list);
gui_project_monitor_widget_update();

g_free(label);
}

/*******************/
/* action callback */
/*******************/
void project_status_refresh(GtkWidget *w, gpointer dummy)
{
}

/*******************/
/* action callback */
/*******************/
void project_remote_files_update(gpointer data)
{
gpointer project;

project = tree_project_get();
if (project)
  gui_project_remote_files_update(project);
}

/*******************/
/* action callback */
/*******************/
void project_remote_files_refresh(GtkWidget *w, gpointer dummy)
{
const gchar *name;
gpointer project;
GList *list;
GSList *slist;
GtkTreeModel *treemodel;
GtkTreeSelection *selection;
GtkTreeIter iter;
struct grid_pak *grid;

/* NB: this now has a local component ie refresh on localhost job */
/* so don't force authentication unless we know it's a remote job */
project = tree_project_get();
if (!project)
  return;

treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(project_tv));
selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(project_tv));

list = gtk_tree_selection_get_selected_rows(selection, &treemodel);
if (!list)
  return;

/* NEW */
gtk_widget_show(project_remote_file_box);

slist = NULL;
if (gtk_tree_model_get_iter(treemodel, &iter, list->data))
  {
  gtk_tree_model_get(treemodel, &iter, 0, &name, -1);

  grid = project_job_lookup_or_create(name, project);

/* if local (NB: this does not require auth) - show dir */
  if (grid_job_is_local(grid))
    {
/* TODO - filter with exe type ... mainly so we don't get the .. item */
    if (grid->local_cwd)
      slist = file_dir_list(grid->local_cwd, FALSE);
    if (!slist)
      slist = g_slist_prepend(slist, "Missing local working directory...");
    gui_scrolled_list_refresh(slist, project_remote_files);
    }
  else
    {
/* TODO - popup auth dialog if not authenticated */
    gtk_entry_set_text(GTK_ENTRY(project_job_name), name);
    slist = g_slist_prepend(slist, "Initializing...");
    gui_scrolled_list_refresh(slist, project_remote_files);
    g_slist_free(slist);
/* grisu call to lookup */
    task_new("Get Remote Directory", grid_file_list, grid, project_remote_files_update, grid, NULL);
    }
  }
}

/*********************************/
/* show details for selected job */
/*********************************/
void gui_job_details_update(const gchar *name)
{
gchar *text;
GSList *slist=NULL;
struct grid_pak *grid;

/* clear widget if nothing */
if (!name)
  {
  gui_scrolled_list_refresh(NULL, project_remote_files);
  return;
  }

/* lookup job data */
grid = project_job_lookup_or_create(name, sysenv.workspace);
if (!grid)
  {
  printf("Couldn't find job: %s\n", name);
  return;
  }

// CURRENT - always display locally cached job details
if (grid->local_output)
  {
  text = g_strdup_printf("Output: %s", grid->local_output);
  slist = g_slist_prepend(slist, text);
  }
if (grid->local_input)
  {
  text = g_strdup_printf("Input: %s", grid->local_input);
  slist = g_slist_prepend(slist, text);
  }
if (grid->remote_q)
  {
  text = g_strdup_printf("Queue: %s", grid->remote_q);
  slist = g_slist_prepend(slist, text);
  }
if (grid->user_vo)
  {
  text = g_strdup_printf("VO: %s", grid->user_vo);
  slist = g_slist_prepend(slist, text);
  }
if (grid->description)
  {
  text = g_strdup_printf("Description: %s", grid->description);
  slist = g_slist_prepend(slist, text);
  }

gui_scrolled_list_refresh(slist, project_remote_files);
}

/***************************************************/
/* gui event hook to show a selected job's details */
/***************************************************/
void gui_job_select(GtkWidget *w, gpointer data)
{
gchar *name=NULL;
GList *list; 
GtkTreeIter iter;
GtkTreeModel *treemodel;
GtkTreeSelection *selection;

selection=data;

/* hide the file listing */
gtk_widget_hide(project_remote_file_box);

/* only update details if a single job is selected */
list = gtk_tree_selection_get_selected_rows(selection, &treemodel);
if (g_list_length(list) == 1)
  {
  if (gtk_tree_model_get_iter(treemodel, &iter, list->data))
    {
    gtk_tree_model_get(treemodel, &iter, 0, &name, -1);
    gtk_entry_set_text(GTK_ENTRY(project_job_name), name);
    }
  }

/* if name=NULL -> clear the details widget */
gui_job_details_update(name);
}

/**************************************************/
/* create the project job monitor/download widget */
/**************************************************/
void gui_project_monitor_widget(GtkWidget *box)
{
gchar *titles[] = {" Task Name ", " Status " };
GtkWidget *swin, *vbox, *hbox;
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *selection;

vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);

swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_box_pack_start(GTK_BOX(vbox), swin, TRUE, TRUE, 0);

project_ls = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
project_tv = gtk_tree_view_new_with_model(GTK_TREE_MODEL(project_ls));
gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin), project_tv);

renderer = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes(titles[0], renderer, "text", 0, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(project_tv), column);
gtk_tree_view_column_set_expand(column, TRUE);

renderer = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes(titles[1], renderer, "text", 1, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(project_tv), column);
gtk_tree_view_column_set_expand(column, TRUE);

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(project_tv));
gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

/* CURRENT */
g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(gui_job_select), selection);

/* actions bar */
/*
hbox2 = gui_frame_hbox(NULL, FALSE, FALSE, box);
hbox = gtk_hbox_new(TRUE, PANEL_SPACING);
gtk_box_pack_end(GTK_BOX(hbox2), hbox, FALSE, FALSE, 0);
*/

hbox = gtk_hbox_new(TRUE, PANEL_SPACING);
gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

/* actions ... replace with auto update handlers later? */
gui_button("Refresh", gui_project_jobs_refresh, NULL, hbox, TT);
gui_button("Remove", project_job_kill, NULL, hbox, TT);
gui_button("Files", project_remote_files_refresh, NULL, hbox, TT);
//gui_button("Details", gui_project_show_details, selection, hbox, TT);

}

/**************************************************/
/* remove project data from the job transfer list */
/**************************************************/
void gui_project_data_remove(GtkWidget *w, gpointer dummy)
{
const gchar *filename;
gpointer project;
GList *item, *list;
GtkTreeModel *treemodel;
GtkTreeSelection *selection;
GtkTreeIter iter;

project = tree_project_get();
if (!project)
  return;

treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(project_data_tv));
selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(project_data_tv));

list = gtk_tree_selection_get_selected_rows(selection, &treemodel);
if (!list)
  return;

/* NB: remove in reverse order so we dont get the iterators confused */
for (item=g_list_reverse(list) ; item ; item=g_list_next(item))
  {
  if (gtk_tree_model_get_iter(treemodel, &iter, item->data))
    {
    gtk_tree_model_get(treemodel, &iter, 1, &filename, -1);

    project_job_file_remove(filename, project);
    }
  }

gui_project_data_widget_update();
}

/****************************************************************/
/* input file changed, update required executable and locations */
/****************************************************************/
void gui_project_input_changed(GtkWidget *w, gpointer data)
{
gchar *text;

text = gui_pd_text(w);

gui_project_queue_update(text);

g_free(text);
}

/*****************************************/
/* project data - uploads and grid files */
/*****************************************/
void gui_project_data_widget(GtkWidget *box)
{
gchar *titles[] = {" Source ", " Name " };
GtkWidget *vbox, *hbox, *button, *label;
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *selection;

vbox = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);

label = gtk_label_new("Task Data Files");
gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

project_data_ls = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
project_data_tv = gtk_tree_view_new_with_model(GTK_TREE_MODEL(project_data_ls));
gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(project_data_tv), FALSE);
gtk_box_pack_start(GTK_BOX(vbox), project_data_tv, TRUE, TRUE, 0);

renderer = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes(titles[0], renderer, "text", 0, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(project_data_tv), column);
gtk_tree_view_column_set_expand(column, TRUE);

renderer = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes(titles[1], renderer, "text", 1, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(project_data_tv), column);
gtk_tree_view_column_set_expand(column, TRUE);

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(project_data_tv));
gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

/* actions bar */
hbox = gtk_hbox_new(TRUE, PANEL_SPACING);
gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

/* actions ... take out later? (replace with g_timeout's) */
button = gtk_button_new_with_label("Remove");
gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 0);
g_signal_connect(GTK_OBJECT(button), "clicked",
                 GTK_SIGNAL_FUNC(gui_project_data_remove), NULL);
}

/********************************************/
/* create the project job scheduling widget */
/********************************************/
/* TODO - in principle - can create a new siesta project */
/* with NO imports ... then attach a list of fdfs/psfs to upload */
/* and do batch submission to a queue */
/* TODO - now that this covers localhost jobs ... make it self modify to reflect */
/* grid or local destinations */
void gui_project_schedule_widget(GtkWidget *box)
{
GtkWidget *vbox, *hbox, *label;
GtkWidget *left, *right;

/* local section */
hbox = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
left = gtk_vbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(hbox), left, TRUE, TRUE, 0);
right = gtk_vbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(hbox), right, TRUE, TRUE, 0);

label = gtk_label_new("Executable");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);
label = gtk_label_new("Destination");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);
label = gtk_label_new("Input File");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);
label = gtk_label_new("Task Name");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);

/* executable */
project_job_description = gtk_entry_new();
gtk_entry_set_editable(GTK_ENTRY(project_job_description), FALSE);
gtk_box_pack_start(GTK_BOX(right), project_job_description, TRUE, TRUE, 0);

/* queue */
vbox = gtk_vbox_new(FALSE, 0);
project_queue_widget = gui_pd_new(NULL, NULL, NULL, vbox);
gtk_box_pack_start(GTK_BOX(right), vbox, TRUE, TRUE, 0);
gui_pd_edit_set(FALSE, project_queue_widget);

/* input file */
vbox = gtk_vbox_new(FALSE, 0);
project_input_widget = gui_pd_new(NULL, NULL, NULL, vbox);
gtk_box_pack_start(GTK_BOX(right), vbox, TRUE, TRUE, 0);
gui_pd_edit_set(FALSE, project_input_widget);
g_signal_connect_after(GTK_OBJECT(project_input_widget), "changed",
                       GTK_SIGNAL_FUNC(gui_project_input_changed), NULL);

/* task name */
/*
project_task_name = gtk_entry_new();
gtk_entry_set_editable(GTK_ENTRY(project_task_name), TRUE);
gtk_box_pack_start(GTK_BOX(right), project_task_name, TRUE, TRUE, 0);
*/
gui_text_entry(NULL, &project_task_name, TRUE, TRUE, right);


/* remote (grid) only section */
project_task_remote = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(box), project_task_remote, FALSE, FALSE, 0);
left = gtk_vbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(project_task_remote), left, TRUE, TRUE, 0);
right = gtk_vbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(project_task_remote), right, TRUE, TRUE, 0);


label = gtk_label_new("CPUs");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);
label = gtk_label_new("Memory per CPU (MB)");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);
label = gtk_label_new("Walltime (mins)");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);
label = gtk_label_new("Submission VO");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);
/* CURRENT */
label = gtk_label_new("Version");
gtk_box_pack_start(GTK_BOX(left), label, TRUE, TRUE, 0);


project_entry_cpus = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(project_entry_cpus), "1");
gtk_box_pack_start(GTK_BOX(right), project_entry_cpus, TRUE, TRUE, 0);

project_entry_memory = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(project_entry_memory), "1000");
gtk_box_pack_start(GTK_BOX(right), project_entry_memory, TRUE, TRUE, 0);

project_entry_walltime = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(project_entry_walltime), "10");
gtk_box_pack_start(GTK_BOX(right), project_entry_walltime, TRUE, TRUE, 0);

vbox = gtk_vbox_new(FALSE, 0);
project_vo_widget = gui_pd_new(NULL, &sysenv.default_vo, NULL, vbox);
g_signal_connect_after(GTK_OBJECT(project_queue_widget), "changed",
                       GTK_SIGNAL_FUNC(gui_project_task_view_update), NULL);
gtk_box_pack_start(GTK_BOX(right), vbox, TRUE, TRUE, 0);
gui_pd_edit_set(FALSE, project_vo_widget);

/* CURRENT */
vbox = gtk_vbox_new(FALSE, 0);
project_version_widget = gui_pd_new(NULL, NULL, NULL, vbox);
gtk_box_pack_start(GTK_BOX(right), vbox, TRUE, TRUE, 0);
gui_pd_edit_set(FALSE, project_version_widget);

/* actions */
hbox = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
// FIXME - cancel is not compat with new task ordering
//gui_button("Cancel Task", gui_project_cancel, NULL, hbox, TT);
gui_button("Create Task", gui_project_submit, NULL, hbox, FF);
}

/*********************/
/* local files panel */
/*********************/
void gui_project_file_widget(GtkWidget *box)
{
GtkWidget *vbox, *hbox, *hbox2;

/* TODO - split pane file view (dir + files) ? */
vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);

/* local files */
project_local_files = gui_scrolled_list_new(vbox);

/* NEW - double click -> does the same thing as open */
g_signal_connect(project_local_files, "row_activated",
                 G_CALLBACK(gui_project_local_import), NULL);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
hbox2 = gtk_hbox_new(TRUE, 0);
gtk_box_pack_end(GTK_BOX(hbox), hbox2, FALSE, FALSE, 0);

// TODO - not 100% sure this is best
//gui_button("Copy", gui_project_local_copy, NULL, hbox, TT);
//gui_button("Paste", gui_project_local_paste, NULL, hbox, TT);
gui_button("Remove", gui_project_local_remove, NULL, hbox2, TT);
gui_button("Include", gui_project_local_include, NULL, hbox2, TT);
gui_button("Open", gui_project_local_import, NULL, hbox2, TT);
}

/****************************************************/
/* copy the current project's data URL to clipboard */
/****************************************************/
void gui_grid_job_copy(GtkWidget *w, gpointer dummy)
{
gchar *cwd;
gpointer project;
const gchar *jobname;
GtkClipboard *clipboard; 
struct grid_pak *grid;

jobname = gtk_entry_get_text(GTK_ENTRY(project_job_name));

//printf("Getting URL for job: %s\n", jobname);

project = tree_project_get();
if (!project)
  return;

grid = project_job_lookup(jobname, project);
if (!grid)
  return;

cwd = grisu_job_cwd(jobname);
if (!cwd)
  return;

//printf("remote cwd: %s\n", cwd);

clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);

gtk_clipboard_set_text(clipboard, cwd, -1);

g_free(cwd);
}

/*******************************/
/* list contents of my df home */
/*******************************/
//gsiftp://arcs-df.vpac.org:2810/ARCS/home
//gsiftp://arcs-df.vpac.org:2810/~
void gui_grid_datafabric(GtkWidget *w, gpointer dummy)
{
//GSList *item, *list=NULL;

//TODO - ensure we have a usable proxy
//grid_gsiftp_list(&list, "gsiftp://arcs-df.vpac.org/ARCS/home");
//grid_gsiftp_list(&list, "gsiftp://arcs-df.vpac.org/~");
//grid_gsiftp_list(&list, "gsiftp://arcs-df.ivec.org/ARCS/home/~");

printf("Creating mount point...\n");

//grisu_mount("user_df", "gsiftp://arcs-df.vpac.org:2810/~");
//grisu_mount_vo("user_df", "gsiftp://arcs-df.vpac.org:2810/~", "/ACC");

printf("Listing home directory on DF...\n");

/* FIXME - doesn't work at the moment ... grisu complains about no mountpoint */

//list = grisu_file_list("gsiftp://arcs-df.vpac.org:2810/~");
/*
list = grisu_file_list("user_df");
for (item=list ; item ; item=g_slist_next(item))
  {
printf("+ %s\n", (gchar *) item->data);
  }
*/
}

/**********************/
/* remote files panel */
/**********************/
void gui_remote_files_widget(GtkWidget *box)
{
GtkWidget *vbox, *hbox;

vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);

hbox = gtk_hbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);


project_job_name = gtk_entry_new();
gtk_box_pack_start(GTK_BOX(hbox), project_job_name, TRUE, TRUE, 0);
gtk_entry_set_editable(GTK_ENTRY(project_job_name), FALSE);


/* CURRENT */
//gui_button("Copy", gui_grid_job_copy, NULL, hbox, FF);
//gui_button(" DF ", gui_grid_datafabric, NULL, hbox, FF);


//project_remote_details = gui_scrolled_list_new(vbox);

/* files */
project_remote_files = gui_scrolled_list_new(vbox);

project_remote_file_box = gtk_hbox_new(TRUE, PANEL_SPACING);
gtk_box_pack_end(GTK_BOX(vbox), project_remote_file_box, FALSE, FALSE, 0);

// CURRENT - cope with "grid" jobs that are actually local */
gui_button("Include", gui_project_remote_include, NULL, project_remote_file_box, TT);
gui_button("Download", gui_project_download, gui_project_import_download_cleanup, project_remote_file_box, TT);

// CURRENT - since download prompts to browse local dir, dont really need this
// and probably should be replaced by job monitor functionality of some kind
// TODO - cant replace this just yet - as it's used to open local files too
gui_button("Open", gui_project_download, gui_project_import_download_open, project_remote_file_box, TT);

// NEW - replaced by a button in task tray that download/refreshes a grid file
//gui_monitor_button = gui_button("Monitor", gui_project_monitor, NULL, project_remote_file_box, TT);

}

/**************************************/
/* user requested project name change */
/**************************************/
void gui_project_name_change(GtkWidget *w, gpointer dummy)
{
const gchar *text;
gpointer project;

project = tree_project_get();
if (!project)
  return;

text = gtk_entry_get_text(GTK_ENTRY(w));

//printf("change [%s]\n", text);

project_label_set(text, project);
gui_tree_rename(text, project);
}

/*********************************************/
/* user requested project description change */
/*********************************************/
void gui_project_description_change(GtkWidget *w, gpointer dummy)
{
const gchar *text;
gpointer project;

project = tree_project_get();
if (!project)
  return;

text = gtk_entry_get_text(GTK_ENTRY(w));

//printf("change [%s]\n", text);

project_description_set(text, project);
}

// deprec
/********************************************************/
/* interface for setting current project tab visibility */
/********************************************************/
void gui_project_tab_visible_set(const gchar *label, gint value)
{
return;
gui_notebook_page_visible_set(project_notebook, label, value);
}

/**************************/
/* get current visibility */
/**************************/
gint gui_project_tab_visible_get(const gchar *label)
{
return(FALSE);

return(gui_notebook_page_visible_get(project_notebook, label));
}

/*************************/
/* toggle the visibility */
/*************************/
void gui_project_tab_toggle(const gchar *label)
{
gint state;

return;

state = gui_notebook_page_visible_get(project_notebook, label);
state ^= 1;
gui_notebook_page_visible_set(project_notebook, label, state);
}

/*****************************/
/* create the project widget */
/*****************************/
void gui_project_widget(GtkWidget *box)
{
GtkWidget *page, *vbox, *hbox, *label, *toolbar;
GtkWidget *image;
GdkPixbuf *pixbuf;

hbox = gui_frame_hbox(NULL, FALSE, FALSE, box);

// method 1
/*
gui_button("Home", gui_project_navigate_home, NULL, hbox, FF);
gui_button("Downloads", gui_project_navigate_downloads, NULL, hbox, FF);
*/

// method 2
/*
//GtkWidget *button;
button = gui_image_button(IMAGE_HOME, gui_project_navigate_home, NULL);
gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

button = gui_image_button(IMAGE_DOWNLOAD, gui_project_navigate_downloads, NULL);
gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
*/

// method 3
toolbar = gtk_toolbar_new();
gtk_box_pack_start(GTK_BOX(hbox), toolbar, FALSE, FALSE, 0);

pixbuf = image_table_lookup(IMAGE_HOME);
image = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), NULL, "Home Folder", "Private",
                        image, GTK_SIGNAL_FUNC(gui_project_navigate_home), NULL);

// FIXME - gtk_toolbar_append_item is deprecated - redo to use:
//gtk_toolbar_insert(GtkToolbar *toolbar, GtkToolItem *item, gint pos);

pixbuf = image_table_lookup(IMAGE_DOWNLOAD);
image = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), NULL, "Downloads Folder", "Private",
                        image, GTK_SIGNAL_FUNC(gui_project_navigate_downloads), NULL);

/* current path */
project_entry_path = gtk_entry_new();
gtk_box_pack_start(GTK_BOX(hbox), project_entry_path, TRUE, TRUE, 0);
gtk_entry_set_editable(GTK_ENTRY(project_entry_path), FALSE);

project_local_filter = gui_file_filter(gui_project_local_filter, hbox);

/* TAB - directory navigation */
page = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(box), page, TRUE, TRUE, 0);

/* folders */
vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), vbox, TRUE, TRUE, 0);

label = gtk_label_new("Folders");
gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

gui_project_navigate_widget(vbox);

/* files */
vbox = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), vbox, TRUE, TRUE, 0);

label = gtk_label_new("Files");
gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

gui_project_file_widget(vbox);

gui_project_navigate_update();

// initialize the filter to workspace value
gui_file_filter_set(project_filter_get(sysenv.workspace), project_local_filter);

// FIXME - temp hack to workaround file open/project dialog split
g_signal_connect(GTK_OBJECT(box), "destroy", (gpointer) gui_project_clipboard_free, NULL);
}

/*********************************/
/* delete the selected workspace */
/*********************************/
void gui_project_browse_delete(GtkWidget *w, gpointer tree_view)
{
gpointer project_preview;
GtkTreeIter iter;
GtkTreeModel *tree_model;
GtkTreeSelection *selection;
GtkTreeStore *tree_store;

tree_store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));

if (gtk_tree_selection_get_selected(selection, &tree_model, &iter))
  {
  gtk_tree_model_get(tree_model, &iter, 2, &project_preview, -1);

  gtk_tree_store_remove(tree_store, &iter);

  sysenv.projects = g_slist_remove(sysenv.projects, project_preview);

  g_remove(project_fullpath_get(project_preview));

// FIXME - the free doesn't work (hits the thread protecting atomic)
//project_free(project_preview);
  
  }
}

/*******************************/
/* open the selected workspace */
/*******************************/
void gui_project_browse_open(GtkWidget *w, gpointer tree_view)
{
gpointer project_preview;
GtkTreeIter iter;
GtkTreeModel *treemodel;
GtkTreeSelection *selection;

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));

if (gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  {
  gtk_tree_model_get(treemodel, &iter, 2, &project_preview, -1);

  project_import_workspace(project_preview);

/* close the browser ... not sure if this is an obvious thing to do ... */
  dialog_destroy_type(123);
  }
}

/******************************************************/
/* update the project details of the selected project */
/******************************************************/
void gui_project_browse_select_refresh(gpointer selection, gpointer buffer)
{
gchar *text;
gpointer project;
GtkTreeIter iter;
GtkTreeModel *treemodel;

//printf("%p : %p\n", w, selection);

/* TODO */
if (gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  {
  gtk_tree_model_get(treemodel, &iter, 2, &project, -1);

/* TODO - some checks if still a valid project? */
/* TODO - can make faster -> get description and modified strings and */
/* insert without string re-alloc everytime */
  text = project_summary_get(project);

  gui_scrolled_text_set(buffer, text);

  g_free(text);
  }
}

/************************************/
/* project timestamp sort primitive */
/************************************/
gint project_time_sort(gpointer a, gpointer b, gpointer dummy)
{
gint n1, n2, result=0;
gchar **buff1, **buff2;

buff1 = tokenize(project_label_get(a), &n1);
buff2 = tokenize(project_label_get(b), &n2);

if ((n1>1) && (n2>1))
  {
//printf("c: %s <> %s\n", *(buff1+2), *(buff2+2));
  result = time_stamp_compare(*(buff1+2), *(buff2+2));
  }

g_strfreev(buff1);
g_strfreev(buff2);

return(result);
}

/****************************************/
/* update the GUI with the project list */
/****************************************/
void gui_project_browse_populate(GtkTreeStore *tree_store)
{
const gchar *text;
GSList *item;
GtkTreeIter iter;

if (!tree_store)
  return;

/* timestamp sort */
sysenv.projects = g_slist_sort(sysenv.projects, (gpointer) project_time_sort);

/* populate */
for (item=sysenv.projects ; item ; item=g_slist_next(item))
  {
  gtk_tree_store_append(tree_store, &iter, NULL);

  text = project_label_get(item->data);

  gtk_tree_store_set(tree_store, &iter, 1, text, 2, item->data, -1);

/*
  text = project_label_get(item->data);
  printf("label: %s\n", text);

  text = project_description_get(item->data);
  printf("pd: %s\n", text);
*/

  }
}

/***************************/
/* project browsing layout */
/***************************/
gpointer gui_project_browse_widget(GtkWidget *box)
{
gpointer buffer;
GtkWidget *hbox, *swin, *tree_view;
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *select;
GtkTreeStore *tree_store;

g_assert(box != NULL);

/* split pane */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, 0);

/* left - list of past projects */
/* scrolled window for the model pane */
swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_AUTOMATIC,
                               GTK_POLICY_AUTOMATIC);
gtk_box_pack_start(GTK_BOX(hbox), swin, TRUE, TRUE, 0);

/* underlying data storage */
tree_store = gtk_tree_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER);

/* actual tree widget */
tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_store));
gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin), tree_view);

/* setup the model pixmap rendering colum */
renderer = gtk_cell_renderer_pixbuf_new();
column = gtk_tree_view_column_new_with_attributes("image", renderer,
                                                  "pixbuf", 0, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

/* setup the model name rendering colum */
renderer = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes("name", renderer,
                                                  "text", 1, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);

//project_browse_buffer = gui_scrolled_text_new(FALSE, hbox);
buffer = gui_scrolled_text_new(FALSE, hbox);

/* setup the selection handler */
select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
//select = gtk_tree_view_get_selection(GTK_TREE_VIEW(GTK_TREE_MODEL(tree_store)));
gtk_tree_selection_set_mode(select, GTK_SELECTION_BROWSE);
g_signal_connect(G_OBJECT(select), "changed",
                 G_CALLBACK(gui_project_browse_select_refresh),
                 buffer);

/* CURRENT */
/* TODO - may migrate this whole thing to the main window at some point */
/* if so - the ugliness of this will be naturally removed */
//gui_stock_button(GTK_STOCK_OPEN, gui_project_browse_open, select, box);

gui_project_browse_populate(tree_store);

return(tree_view);
}

/******************************/
/* save the current workspace */
/******************************/
void gui_project_save(void)
{
}

/***************************/
/* project browsing dialog */
/***************************/
void gui_project_browse_dialog(void)
{
gpointer tree_view, dialog;
GtkWidget *window;

//project_browse_free();
project_browse_init();

dialog = dialog_request(123, "Saved Workspaces", NULL, NULL, NULL);
if (!dialog)
  return;

window = dialog_window(dialog);
gtk_widget_set_size_request(window, 700, 400);

tree_view = gui_project_browse_widget(GTK_DIALOG(window)->vbox);

/* action buttons */
gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Workspace Browser",
                 GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_DELETE, gui_project_browse_delete, tree_view, 
                 GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_OPEN, gui_project_browse_open, tree_view, 
                 GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog,
                 GTK_DIALOG(window)->action_area);

// NEW
g_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(project_browse_free), NULL);

gtk_widget_show_all(window);
}

