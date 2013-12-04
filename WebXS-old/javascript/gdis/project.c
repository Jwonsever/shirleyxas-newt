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

#include "gdis.h"
#include "coords.h"
#include "model.h"
#include "graph.h"
#include "surface.h"
#include "molsurf.h"
#include "numeric.h"
#include "parse.h"
#include "project.h"
#include "file.h"
#include "scan.h"
#include "import.h"
#include "interface.h"
#include "grisu_client.h"
#include "grid.h"
#include "task.h"

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/* all this will be removed - users found it too confusing */
/* sysenv.workspace will be a single always present project in gdis */
/* this can be saved/restored ... but there can only be one per gdis session */

/* NEW - wrapper around import objects */
/* this is intended to be the top level object in the tree */
/* files hang off this object ... and models/other data hang off files */
struct project_pak
{
gint id;
gint count;
gint browse;
gint imports_save;
/* GUI tree label */
gchar *label;
gchar *description;
gchar *modified;
gchar *fullpath;

/* current directory for project */
gchar *path;
GSList *files;
gint filter;

/* grid stuff */
/* TODO replace */
gchar *remote_jobname;
gchar *remote_path;
GSList *remote_files;
gint remote_filter;

/* new grid reference mechanism */
/* list of (grisu) job names */
/* TODO - may want to cache more data (eg remote dir etc) */
/* could just make this a list of grid_paks */
GHashTable *job_table;

/* list of file transfers to do befor job submission */
GHashTable *job_file_table;

/* NEW - data */
GHashTable *data;

/* NEW - names of imports for project browser */
GSList *browse_imports;
/* TODO - more control eg which import is the control file etc */
GSList *imports;
};

GSList *project_list=NULL;

/********************************************/
/* check a project reference is still valid */
/********************************************/
gint project_valid(gpointer project)
{
#ifdef WITH_GUI
GSList *item, *list;

g_assert(project != NULL);

list = tree_project_all();
for (item=list ; item ; item=g_slist_next(item))
  {
  if (item->data == project)
    return(TRUE);
  }
#endif

return(FALSE);
}

/*****************************************/
/* project id: follows file_pak id types */
/*****************************************/
/* TODO - primitive for setting gchar label version */
void project_id_set(gint id, gpointer data)
{
struct project_pak *project = data;

project->id = id;
}

/*****************************************/
/* project id: follows file_pak id types */
/*****************************************/
/* TODO - primitive for returning gchar label version */
gint project_id_get(gpointer data)
{
struct project_pak *project = data;

return(project->id);
}

/*****************************************************/
/* flag for saving all imports when project is saved */
/*****************************************************/
void project_imports_save(gpointer data)
{
struct project_pak *project = data;

project->imports_save = TRUE;
}

/*******************************/
/* refresh the local file list */
/*******************************/
void project_files_refresh(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

if (project->path)
  {
  if (project->files)
    free_slist(project->files);

  project->files = file_dir_list(project->path, TRUE);
  }
}

/****************************************/
/* set the current path for the project */ 
/****************************************/
/* change name? eg cwd ... distinguish it from fullpath */
void project_path_set(const gchar *path, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

/* set path */
g_free(project->path);
if (path)
  project->path = g_strdup(path);
else
  project->path = g_strdup(sysenv.cwd);

/* get filtered file list */
if (project->files)
  free_slist(project->files);

/* check if dir path exists, create if doesnt */
if (!file_mkdir(project->path))
  {
  printf("project_path_set() error: bad path %s\n", project->path);
  project->files = NULL;
  return;
  }

project->files = file_dir_list(project->path, TRUE);
}

/************************/
/* retrieve file filter */
/************************/
gint project_filter_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(project->filter);
}

/****************************************************************/
/* set the filter to use in determining files in a project path */
/****************************************************************/
void project_filter_set(gint filter_id, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

project->filter = filter_id;
}

/************************/
/* retrieve file filter */
/************************/
gint project_remote_filter_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(project->remote_filter);
}

/****************************************************************/
/* set the filter to use in determining files in a project path */
/****************************************************************/
void project_remote_filter_set(gint filter_id, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

project->remote_filter = filter_id;
}

/*************************************/
/* set the flag for browse/read mode */
/*************************************/
void project_browse_set(gint browse, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

project->browse = browse;
}

/*****************************************/
/* return the current description string */
/*****************************************/
const gchar *project_description_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

if (project->description)
  return(project->description);
else
  return("Unknown");
}

/**************************************/
/* set the current description string */
/**************************************/
void project_description_set(const gchar *text, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

g_free(project->description);

project->description = g_strdup(text);
}

/*****************************************/
/* return the current description string */
/*****************************************/
const gchar *project_modified_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(project->modified);
}

/*****************************************************/
/* create a summary text block for the browse widget */
/*****************************************************/
gchar *project_summary_get(gpointer data)
{
GString *summary;
GSList *list;
struct project_pak *project = data;

g_assert(project != NULL);

summary = g_string_new(NULL);

/*
if (project->description)
  g_string_append_printf(summary, "%s\n\n", project->description);
else
  g_string_append_printf(summary, "No description\n\n");

g_string_append_printf(summary, "Last modified: %s\n\n", project->modified);
*/

for (list=project->browse_imports ; list ; list=g_slist_next(list))
  {
  gchar *text = g_path_get_basename(list->data);

  g_string_append_printf(summary, "%s\n", text);

  g_free(text);
  }

return(g_string_free(summary, FALSE));
}

/*****************************************************************/
/* return the current path and filename of the project save file */
/*****************************************************************/
const gchar *project_fullpath_get(gpointer data)
{
struct project_pak *project = data;

if (!project)
  return(NULL);
if (!project->fullpath)
  project->fullpath = file_unused_name(sysenv.project_path, "project", "xml");

return(project->fullpath);
}

/**************************/
/* project data primitive */
/**************************/
void project_data_add(const gchar *key, gpointer value, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

g_hash_table_insert(project->data, g_strdup(key), value);
}

/**************************/
/* project data primitive */
/**************************/
gpointer project_data_get(const gchar *key, gpointer data)
{
struct project_pak *project = data;

if (!project)
  return(NULL);

return(g_hash_table_lookup(project->data, key));
}

/************************/
/* create a new project */
/************************/
gpointer project_new(const gchar *label, const gchar *path)
{
static gint counter=1;
struct project_pak *project;

project = g_malloc(sizeof(struct project_pak));

g_atomic_int_set(&(project->count), 0);

if (label)
  project->label = g_strdup(label);
else
  project->label = g_strdup_printf("project %d", counter++);

/* unknown */
project->id = -1;
project->browse = FALSE;
project->imports_save = FALSE;
project->description = NULL;
project->modified = NULL;
project->fullpath = NULL;

/* local files */
project->path = NULL;
project->files = NULL;
/* default of all known files */
project->filter = DATA;
project_path_set(path, project);

/* remote files */
project->remote_jobname=NULL;
project->remote_path=NULL;
project->remote_files=NULL;
project->remote_filter = DATA;

project->job_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, grid_job_free);

/* imported files */
project->imports = NULL;
project->browse_imports = NULL;

/* grid/job file list */
project->job_file_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, import_free);

/* NEW */
project->data = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
project_data_add(PROJECT_SURFACES, surface_new(), project);
project_data_add(PROJECT_DYNAMICS, NULL, project);

project_list = g_slist_prepend(project_list, project);

return(project);
}

/*******************/
/* grid primitives */
/*******************/
/* jobname=NULL => local file, otherwise => previous grid job */
/* using a hash table as all files are copied to the same job */
/* directory so it makes sense that identical names replace */
void project_job_file_append(const gchar *jobname, const gchar *fullpath, gpointer data)
{
gchar *filename;
gpointer import;
struct project_pak *project=data;

g_assert(project != NULL);
g_assert(fullpath != NULL);

import = import_new();
import_fullpath_set(fullpath, import);

filename = g_strdup(import_filename(import));

/* enforce NULL label if a local file */
if (jobname)
  import_label_set(jobname, import);
else
  import_label_set(NULL, import);

g_hash_table_insert(project->job_file_table, filename, import);
}

/******************************************************************/
/* parse the list of input files and determine require executable */
/******************************************************************/
/* deprec */
const gchar *project_executable_required(gpointer data)
{
GList *item, *list;
struct file_pak *file;
struct project_pak *project=data;

g_assert(project != NULL);

list = g_hash_table_get_keys(project->job_file_table);
for (item=list ; item ; item=g_list_next(item))
  {
  file = file_type_from_filename(item->data);
  if (file)
    {
    switch (file->id)
      {
      case FDF:
        return(sysenv.siesta_exe);
      case GAMESS:
        return(sysenv.gamess_exe);
      case GULP:
        return(sysenv.gulp_exe);
      }
    }
  }
return("none");
}

/*******************/
/* grid primitives */
/*******************/
void project_job_file_remove(const gchar *filename, gpointer data)
{
struct project_pak *project=data;

g_assert(project != NULL);

g_hash_table_remove(project->job_file_table, filename);
}

/*******************/
/* grid primitives */
/*******************/
GList *project_job_file_list(gpointer data)
{
struct project_pak *project=data;

g_assert(project != NULL);

return(g_hash_table_get_values(project->job_file_table));
}

/*******************/
/* grid primitives */
/*******************/
void project_job_record(const gchar *name, gpointer grid, gpointer data)
{
struct project_pak *project=data;

g_assert(project != NULL);

g_hash_table_insert(project->job_table, g_strdup(name), grid);
}

/*******************/
/* grid primitives */
/*******************/
gpointer project_job_lookup(const gchar *name, gpointer data)
{
struct project_pak *project=data;

g_assert(project != NULL);

return(g_hash_table_lookup(project->job_table, name));
}

/*******************/
/* grid primitives */
/*******************/
gpointer project_job_lookup_or_create(const gchar *name, gpointer data)
{
struct grid_pak *grid=NULL;
struct project_pak *project=data;

g_assert(project != NULL);

grid = g_hash_table_lookup(project->job_table, name);

if (!grid)
  {
  grid = grid_job_new();
  grid->jobname = g_strdup(name);
  project_job_record(name, grid, project);
  }

return(grid);
}

/*******************/
/* grid primitives */
/*******************/
void project_job_remove_all(gpointer data)
{
struct project_pak *project=data;

g_hash_table_destroy(project->job_table);
project->job_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, grid_job_free);
}

/*******************/
/* grid primitives */
/*******************/
void project_job_remove(const gchar *name, gpointer data)
{
gpointer grid;
struct project_pak *project=data;

g_assert(project != NULL);

grid = g_hash_table_lookup(project->job_table, name);

g_hash_table_remove(project->job_table, name);
}

/*******************/
/* grid primitives */
/*******************/
/* returned list should not be freed */
GList *project_job_name_list(gpointer data)
{
GList *list;
struct project_pak *project=data;

g_assert(project != NULL);

list = g_hash_table_get_keys(project->job_table);

list = g_list_sort(list, (gpointer) grisu_jobs_sort);

return(list);
}

/*******************/
/* grid primitives */
/*******************/
/* deprec */
gchar *project_grid_jobname(gpointer data)
{
struct project_pak *project=data;

g_assert(project != NULL);

return(project->remote_jobname);
}

/*******************/
/* grid primitives */
/*******************/
gpointer project_grid_new(const gchar *jobname)
{
gchar *path;
struct project_pak *project;

/* TODO - truncate jobname? */
path = g_build_filename(sysenv.cwd, jobname, NULL);
project = project_new(jobname, path);
g_free(path);

project->remote_jobname = g_strdup(jobname);

return(project);
}

/* NB: g_atomic stuff is since 2.4 - might have to use mutexes? */
/**********************************/
/* thread safe reference counting */
/**********************************/
void project_inc(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

g_atomic_int_inc(&(project->count));
}

/**********************************/
/* thread safe reference counting */
/**********************************/
gint project_dec(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(g_atomic_int_dec_and_test(&(project->count)));
}

/**********************************/
/* get the current referene count */
/**********************************/
gint project_count(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(g_atomic_int_get(&(project->count)));
}

/***********************/
/* data free primitive */
/***********************/
void project_data_free(gpointer data)
{
gpointer item;
struct project_pak *project = data;

g_assert(project != NULL);

item = g_hash_table_lookup(project->data, PROJECT_SURFACES);
if (item)
  surface_free(item);

item = g_hash_table_lookup(project->data, PROJECT_DYNAMICS);
if (item)
  free_slist(item);

g_hash_table_destroy(project->data);
}

/*********************/
/* destroy a project */
/*********************/
void project_free(gpointer data)
{
GSList *list;
struct project_pak *project = data;

g_assert(project != NULL);

if (g_atomic_int_get(&(project->count)))
  {
printf("project_free() error: project %p still in use.\n", project);
  return;
  }

project_list = g_slist_remove(project_list, project);

g_free(project->label);
g_free(project->path);
g_free(project->description);
g_free(project->modified);
g_free(project->fullpath);

if (project->files)
  free_slist(project->files);

for (list=project->imports ; list ; list=g_slist_next(list))
  import_free(list->data);
g_slist_free(project->imports);

free_slist(project->browse_imports);

g_free(project->remote_path);
if (project->remote_files)
  free_slist(project->remote_files);

/* NEW */
g_hash_table_destroy(project->job_table);
g_hash_table_destroy(project->job_file_table);

project_data_free(project);

g_free(project);
}

/************************/
/* extract project path */
/************************/
void project_label_set(const gchar *label, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

g_free(project->label);

project->label = g_strdup(label);
}

/************************/
/* extract project path */
/************************/
gchar *project_label_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(project->label);
}

/************************/
/* extract project path */
/************************/
gchar *project_path_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(project->path);
}

/*************************************/
/* extract all files in project path */
/*************************************/
GSList *project_files_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

//return(project->files);

return(file_list_filter(project->filter, project->files));
}

/*********************/
/* import primitives */
/*********************/
void project_imports_remove(gpointer data, gpointer import)
{
struct project_pak *project = data;

g_assert(project != NULL);

project->imports = g_slist_remove(project->imports, import);

/* remove from project job files */
//printf("remove [%s]\n", import_filename(import));

project_job_file_remove(import_filename(import), project);
}

/*********************/
/* import primitives */
/*********************/
void project_imports_add(gpointer data, gpointer import)
{
struct project_pak *project = data;

g_assert(project != NULL);

project->imports = g_slist_append(project->imports, import);

/* add assoc project job files (if any) */
project_import_include(project, import);
}

/*********************/
/* import primitives */
/*********************/
GSList *project_imports_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(project->imports);
}

/*********************/
/* import primitives */
/*********************/
gpointer project_import_get(gint n, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(g_slist_nth_data(project->imports, n));
}

/*******************/
/* grid primitives */
/*******************/
GSList *project_remote_files_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(project->remote_files);
}

/*******************/
/* grid primitives */
/*******************/
void project_remote_files_set(GSList *list, gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

project->remote_files = list;
}

/*******************/
/* grid primitives */
/*******************/
gchar *project_remote_path_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

return(project->remote_path);
}

/*******************************/
/* higher level file primitive */
/*******************************/
gpointer project_import_file(gpointer project, const gchar *filename)
{
gpointer import;

/* create if no project */
if (!project)
  {
printf("project_import_file(): Error, empty project\n");
  return(NULL);
  }

/* checks */
g_assert(filename != NULL);

import = import_file(filename, NULL);
if (import)
  {
  project_imports_add(project, import);
#ifdef WITH_GUI
  gui_tree_append_import(import);
#endif
  }

return(import);
}

/************************************************************************************/
/* also a little bit of a hack, labels a project type according to 1st config found */
/************************************************************************************/
const gchar *project_type_get(gpointer data)
{
struct project_pak *project = data;

g_assert(project != NULL);

switch (project->id)
  {
  case FDF:
    return("siesta");
  case GULP:
    return("gulp");
  case GAMESS:
//    return("gamess");
    return("gamess-us");
  }

return("Unknown");
}

/*******************************************/
/* build list of candidate input filenames */
/*******************************************/
/* list and its elements should be freed */
GSList *project_input_list_get(gpointer data)
{
const gchar *filename, *exe;
GSList *item, *list=NULL;
struct project_pak *project=data;

if (!project)
  return(NULL);

for (item=project->imports ; item ; item=g_slist_next(item))
  {
  filename = import_filename(item->data);

/* only add file to list if it corresponds to a matching exe gdis knows about */
  exe = task_executable_required(filename);
  if (exe)
    {
    if (g_strncasecmp(exe, "none", 4) != 0)
      {
      list = g_slist_prepend(list, g_strdup(filename));    
      }
    }
  }
return(list);
}

/***************************************/
/* build a list of models in a project */
/***************************************/
/* the list but not the elements should be freed when done */
GSList *project_model_list_get(gpointer data)
{
GSList *item1, *item2, *list2, *model_list;
struct project_pak *project=data;

if (!project)
  return(NULL);

model_list=NULL;
for (item1=project->imports ; item1 ; item1=g_slist_next(item1))
  {
  list2 = import_object_list_get(IMPORT_MODEL, item1->data);
  for (item2=list2 ; item2 ; item2=g_slist_next(item2))
    {
    model_list = g_slist_prepend(model_list, item2->data);
    }
  }
return(g_slist_reverse(model_list));
}

/********************************************/
/* build a list of model names in a project */
/********************************************/
/* both the list and the elements should be freed when done */
GSList *project_model_name_list_get(gpointer data)
{
GSList *item1, *item2, *list2, *model_list;
struct project_pak *project=data;
struct model_pak *model;

if (!project)
  return(NULL);

model_list=NULL;
for (item1=project->imports ; item1 ; item1=g_slist_next(item1))
  {
  list2 = import_object_list_get(IMPORT_MODEL, item1->data);
  for (item2=list2 ; item2 ; item2=g_slist_next(item2))
    {
    model = item2->data;
    model_list = g_slist_prepend(model_list, g_strdup(model->basename));
    }
  }
return(g_slist_reverse(model_list));
}

/********************************************************/
/* build a list of project input files (and assoc data) */
/********************************************************/
void project_import_include(gpointer project_ptr, gpointer import)
{
gint id;
gchar *pseudo, *fullname;
GSList *item2, *item3, *model_list, *species_list;
struct model_pak *model;
struct project_pak *project = project_ptr;

g_assert(project != NULL);

//printf("Scanning for includes...\n");

id = file_id_get(BY_EXTENSION, import_filename(import));

/* add input file */

/* if project id not set - set to file type */
if (project->id == -1)
  project->id = id;

switch (id)
  {
  case FDF:
//printf("Hacking project for siesta...\n");
/* append input file */
    project_job_file_append(NULL, import_fullpath(import), project);
    model_list = import_object_list_get(IMPORT_MODEL, import);
    for (item2=model_list ; item2 ; item2=g_slist_next(item2))
      {
      model = item2->data;

      species_list = find_unique(LABEL_NORMAL, model);
      for (item3=species_list ; item3 ; item3=g_slist_next(item3))
        {
        pseudo = g_strdup_printf("%s.psf", (gchar *) item3->data);
        fullname = g_build_filename(project->path, pseudo, NULL);
        if (g_file_test(fullname, G_FILE_TEST_EXISTS))
          {
//printf("Requiring pseudopotential: %s\n", fullname);
          project_job_file_append(NULL, fullname, project);
          }
        g_free(fullname);

        pseudo = g_strdup_printf("%s.vps", (gchar *) item3->data);
        fullname = g_build_filename(project->path, pseudo, NULL);
        if (g_file_test(fullname, G_FILE_TEST_EXISTS))
          {
//printf("Requiring pseudopotential: %s\n", fullname);
          project_job_file_append(NULL, fullname, project);
          }
        g_free(fullname);
        }
      g_slist_free(species_list);
      }
    break;

/* add input files for computational packages */
  case GULP:
    project_job_file_append(NULL, import_fullpath(import), project);
    break;

  case GAMESS:
    project_job_file_append(NULL, import_fullpath(import), project);
    break;

  case REAXMD:
// add control file itself
    project_job_file_append(NULL, import_fullpath(import), project);

// add coords
    fullname = parse_extension_set(import_fullpath(import), "pdb");
    if (g_file_test(fullname, G_FILE_TEST_EXISTS))
      project_job_file_append(NULL, fullname, project);
    g_free(fullname);

// add forcefield
    fullname = parse_extension_set(import_fullpath(import), "frx");
    if (g_file_test(fullname, G_FILE_TEST_EXISTS))
      project_job_file_append(NULL, fullname, project);
    g_free(fullname);
    break;

  default:
// not a supported job type
    break;
  }
}

/*****************************************/
/* find import parent (ie assoc project) */
/*****************************************/
gpointer project_parent_import(gpointer import)
{
GSList *list;

for (list=project_list ; list ; list=g_slist_next(list))
  {
  struct project_pak *project = list->data;

  if (g_slist_find(project->imports, import))
    return(project);
  }

return(NULL);
}

/*************************/
/* save a project as XML */
/*************************/
gint project_save(gpointer data)
{
gchar *filename, *tmp;
GSList *sitem, *slist;
GList *item, *list;
GString *buffer;
struct project_pak *project=data;
struct surface_pak *surface;
struct plane_pak *plane;
struct shift_pak *shift;

g_assert(project != NULL);

/* always save to the settings file directory */
tmp = g_path_get_dirname(sysenv.init_file);
filename = file_unused_name(tmp, "project", "xml");
g_free(tmp);

//printf("Saving project (%s) as: %s\n", project->label, filename);

buffer = g_string_new(NULL);

g_string_append_printf(buffer, "<project label=\"%s\" path=\"%s\">\n", project->label, project->path);

/* description */
if (project->description)
  g_string_append_printf(buffer, "<description>%s</description>\n", project->description);

/* imports */
for (sitem=project->imports ; sitem ; sitem=g_slist_next(sitem))
  {
  g_string_append_printf(buffer, "  <import>%s</import>\n", import_fullpath(sitem->data));
  }

/* includes */
list = project_job_file_list(project);
for (item=list ; item ; item=g_list_next(item))
  {
  g_string_append_printf(buffer, "  <include>%s</include>\n", import_fullpath(item->data));
  }
g_list_free(list);

/* jobs */
list = project_job_name_list(project);
for (item=list ; item ; item=g_list_next(item))
  {
  struct grid_pak *grid;

//  g_string_append_printf(buffer, "  <jobname>%s</jobname>\n", (gchar *) item->data);

/* NEW - store summary info (details) for the job */
grid = g_hash_table_lookup(project->job_table, item->data);
if (grid)
  {
  g_string_append_printf(buffer, "  <job name=\"%s\"", (gchar *) item->data);

  if (grid->user_vo)
    g_string_append_printf(buffer, " vo=\"%s\"", grid->user_vo);
  if (grid->remote_q)
    g_string_append_printf(buffer, " queue=\"%s\"", grid->remote_q);
  if (grid->local_cwd)
    g_string_append_printf(buffer, " localcwd=\"%s\"", grid->local_cwd);
  if (grid->local_input)
    g_string_append_printf(buffer, " input=\"%s\"", grid->local_input);
  if (grid->local_output)
    g_string_append_printf(buffer, " output=\"%s\"", grid->local_output);

/*
  printf("exename=%s\n", grid->exename);
  printf("remote_exe=%s\n", grid->remote_exe);
  printf("remote_exe_module=%s\n", grid->remote_exe);
  printf("remote_site=%s\n", grid->remote_exe_module);
*/

  g_string_append_printf(buffer, "></job>\n");
  }

  }
g_list_free(list);

/* TODO - other stuff eg grid jobs it knows about */

/* save surface builder data */
surface = g_hash_table_lookup(project->data, PROJECT_SURFACES);
if (surface)
  {
  for (slist=surface->planes ; slist ; slist=g_slist_next(slist))
    {
    plane = slist->data;
/*
    if (!plane->primary)
      continue;
*/

g_string_append_printf(buffer, "<plane>%d %d %d %f %d</plane>\n",
  plane->index[0], plane->index[1], plane->index[2], plane->dhkl, plane->primary);

    for (sitem=plane->shifts ; sitem ; sitem=g_slist_next(sitem))
      {
      shift = sitem->data;

g_string_append_printf(buffer, "<shift>%f %d %d %f %f %f %f %f %f</shift>\n",
            shift->shift, shift->region[0], shift->region[1], 
            shift->esurf[0], shift->eatt[0],
            shift->esurf[1], shift->eatt[1], shift->dipole, shift->gnorm);
      }
    }
  }

/* done writing project file */
g_string_append_printf(buffer, "</project>\n");
g_file_set_contents(filename, buffer->str, -1, NULL);
g_string_free(buffer, TRUE);
g_free(filename);

/* NEW - save the imports themselves */
if (project->imports_save)
  {
  for (sitem=project->imports ; sitem ; sitem=g_slist_next(sitem))
    {
printf("TODO - save: %s\n", import_fullpath(sitem->data));
    }
  project->imports_save=FALSE;
  }

return(FALSE);
}

/****************************/
/* import a saved workspace */
/****************************/
void project_import_workspace(gpointer data)
{
GSList *list;
struct project_pak *project = data;

if (!project)
  return;

/* read the (browsed) project into the current workspace */
project_read(project->fullpath, sysenv.workspace);

/* update selection */
list = g_slist_last(sysenv.mal);
if (list)
  tree_select_model(list->data);

}

/**************************/
/* element start callback */
/**************************/
void project_start_element(GMarkupParseContext *context,
                       const gchar *element,
                       const gchar **names,
                       const gchar **values,
                       gpointer data,
                       GError **error)
{
gint i;
struct project_pak *project = data;
struct grid_pak *grid;

//printf("start [%s]\n", element);

g_assert(project != NULL);

if (g_ascii_strncasecmp(element, "project", 7) == 0)
  {
  if (names)
    {
    i=0;
    while (*(names+i))
      {
      if (g_ascii_strncasecmp(*(names+i), "label", 5) == 0)
        {
        g_free(project->label);

//printf("setting label [%s]\n", *(values+i));

        project->label = g_strdup(*(values+i));
        }
      else if (g_ascii_strncasecmp(*(names+i), "path", 4) == 0)
        {
//printf("setting path [%s]\n", *(values+i));
        project_path_set(*(values+i), project);
        }

      i++;
      }
    }
  }

/* parse jobs */
if (!project->browse)
if (g_ascii_strncasecmp(element, "job", 3) == 0)
  {
  grid = grid_job_new();
  if (names)
    {
    i=0;
// don't process null value
    while (*(names+i) && *(values+i))
      {
      if (g_ascii_strncasecmp(*(names+i), "name", 4) == 0)
        grid->jobname = g_strdup(*(values+i));
      if (g_ascii_strncasecmp(*(names+i), "queue", 5) == 0)
        grid->remote_q = g_strdup(*(values+i));
      if (g_ascii_strncasecmp(*(names+i), "vo", 2) == 0)
        grid->user_vo = g_strdup(*(values+i));
      if (g_ascii_strncasecmp(*(names+i), "localcwd", 8) == 0)
        grid->local_cwd = g_strdup(*(values+i));
      if (g_ascii_strncasecmp(*(names+i), "input", 5) == 0)
        grid->local_input = g_strdup(*(values+i));
      if (g_ascii_strncasecmp(*(names+i), "output", 6) == 0)
        grid->local_output = g_strdup(*(values+i));
      i++;
      }

    if (grid->jobname)
      {
//printf("Adding job: %s\n", grid->jobname);
      project_job_record(grid->jobname, grid, project);
      }
    }
  }
}

/************************/
/* element end callback */
/************************/
void project_end_element(GMarkupParseContext *context,
                     const gchar *element_name,
                     gpointer project,
                     GError **error)
{
}

/****************/
/* process text */
/****************/
void project_parse_text(GMarkupParseContext *context,
                    const gchar *text,
                    gsize text_len,  
                    gpointer data,
                    GError **error)
{
const gchar *element;
gpointer import;
struct project_pak *project=data;

element = g_markup_parse_context_get_element(context);

if (g_strncasecmp(element, "description", 11) == 0)
  {
  g_free(project->description);
  project->description = g_strdup(text);
  }

if (g_strncasecmp(element, "import", 5) == 0)
  {
  import = import_file(text, NULL);
  if (import)
    {
// load import and add to tree
    project_imports_add(project, import);
#ifdef WITH_GUI
    gui_tree_append_import(import);
#endif
    }
  else
    printf("Warning, missing import: %s\n", text);
  }

if (g_strncasecmp(element, "include", 7) == 0)
  {
/* TODO - how to store jobname if remote? */
  project_job_file_append(NULL, text, project);
  }

if (g_ascii_strncasecmp(element, "plane", 5) == 0)
  {
  gint n;
  gchar **buff;
  gdouble m[3];
  struct surface_pak *surface;
  struct plane_pak *plane;

surface = g_hash_table_lookup(project->data, PROJECT_SURFACES);

  buff = tokenize(text, &n);
  if (n == 5)
    {
//printf("adding plane ...\n");
//printf(">>>%s\n", text);

    m[0] = str_to_float(*(buff+0));
    m[1] = str_to_float(*(buff+1));
    m[2] = str_to_float(*(buff+2));

    plane = plane_new(m, NULL);

plane->dhkl = str_to_float(*(buff+3));
plane->primary = str_to_float(*(buff+4));

    if (surface && plane)
      surface->planes = g_slist_prepend(surface->planes, plane);

    }
  else
    {
//printf("bad plane ...\n");
//printf(">>>%s\n", text);
    }

  g_strfreev(buff);

  }

/* assume shift goes on the first (prepend!) plane */
if (g_ascii_strncasecmp(element, "shift", 5) == 0)
  {
  gint n;
  gchar **buff;
  GSList *list;
  struct surface_pak *surface;
  struct plane_pak *plane=NULL;
  struct shift_pak *shift;

  surface = g_hash_table_lookup(project->data, PROJECT_SURFACES);
  if (surface)
    {
    list = surface->planes;
    if (list)
      plane = list->data;
    }

  if (plane)
    {
    buff = tokenize(text, &n);
    if (n == 9)
      {
//printf("adding shift ...\n");
//printf(">>>%s\n", text);

      shift = shift_new(str_to_float(*(buff+0)));
      shift->region[0] = str_to_float(*(buff+1));
      shift->region[1] = str_to_float(*(buff+2));
      shift->esurf[0] = str_to_float(*(buff+3));
      shift->eatt[0] = str_to_float(*(buff+4));
      shift->esurf[1] = str_to_float(*(buff+5));
      shift->eatt[1] = str_to_float(*(buff+6));
      shift->dipole = str_to_float(*(buff+7));
      shift->dipole_computed = TRUE;
      shift->gnorm = str_to_float(*(buff+8));

      plane->shifts = g_slist_append(plane->shifts, shift);
      }
    }
  }
}

/***************************************************/
/* browse a project for a summary of it's contents */
/***************************************************/
void project_browse_text(GMarkupParseContext *context,
                         const gchar *text,
                         gsize text_len,  
                         gpointer data,
                         GError **error)
{
const gchar *element;
struct project_pak *project=data;

g_assert(project != NULL);

element = g_markup_parse_context_get_element(context);

if (g_strncasecmp(element, "description", 11) == 0)
  {
  g_free(project->description);
  project->description = g_strdup(text);
  }

/* record import names for browse mode */
if (g_strncasecmp(element, "import", 5) == 0)
  {
  project->browse_imports = g_slist_append(project->browse_imports, g_strdup(text));
  }

if (g_strncasecmp(element, "include", 7) == 0)
  {
  }

if (g_strncasecmp(element, "jobname", 7) == 0)
  {
  }

}

/****************************/
/* read an XML gdis project */
/****************************/
gint project_read(const gchar *fullpath, gpointer data)
{
gpointer scan;
gchar *line;
GMarkupParser xml_parser;
GMarkupParseContext *xml_context;
struct project_pak *project = data;
struct surface_pak *surface;

g_assert(project != NULL);

//printf("project_read(%p): %s\n", project, fullpath);

if (project->fullpath)
  g_free(project->fullpath);

project->fullpath = g_strdup(fullpath);

if (project->modified)
  g_free(project->modified);

project->modified = g_strdup(file_modified_string(project->fullpath));

if (project->browse)
  {
/* careful here - dont want to load imports or anything - just a light scan */
  xml_parser.start_element = &project_start_element;
  xml_parser.end_element = &project_end_element;
  xml_parser.text = &project_browse_text;
  xml_parser.passthrough = NULL;
  xml_parser.error = NULL;
  xml_context = g_markup_parse_context_new(&xml_parser, 0, project, NULL);
  }
else
  {
/* setup context parse (ie callbacks) */
  xml_parser.start_element = &project_start_element;
  xml_parser.end_element = &project_end_element;
  xml_parser.text = &project_parse_text;
  xml_parser.passthrough = NULL;
  xml_parser.error = NULL;
  xml_context = g_markup_parse_context_new(&xml_parser, 0, project, NULL);
  }

scan = scan_new(fullpath);
if (!scan)
  return(FALSE);

/* read in blocks (lines) of text */
line = scan_get_line(scan);
while (!scan_complete(scan))
  {
/* parse the line */
  if (!g_markup_parse_context_parse(xml_context, line, strlen(line), NULL))
    {
    printf("read_xml() : parsing error.\n");
    g_markup_parse_context_free(xml_context);
    break;
    }
  line = scan_get_line(scan);
  }

g_markup_parse_context_free(xml_context);

scan_free(scan);

/* TODO - reverse surface plane list (if any) since we prepend when reading */
surface = g_hash_table_lookup(project->data, PROJECT_SURFACES);
if (surface)
  {
  if (surface->planes)
    surface->planes = g_slist_reverse(surface->planes);
  }

return(FALSE);
}

/*****************************/
/* setup project browse mode */
/*****************************/
void project_browse_init(void)
{
gchar *fullpath;
GSList *item, *list;
struct project_pak *project;

g_assert(sysenv.init_file != NULL);

sysenv.project_path = g_path_get_dirname(sysenv.init_file);

//printf("project dir: %s\n", sysenv.project_path);

if (file_mkdir(sysenv.project_path))
  {
  list = file_dir_list_pattern(sysenv.project_path, "*.xml");

  for (item=list ; item ; item=g_slist_next(item))
    {
    fullpath = g_build_filename(sysenv.project_path, item->data, NULL);

/* TODO - parse and add to list */
project = project_new(NULL, NULL);

project->browse = TRUE;

project_read(fullpath, project);

sysenv.projects = g_slist_prepend(sysenv.projects, project);

    g_free(fullpath);

    }
  }
else
  printf("Warning: failed to initialize project directory.\n");

sysenv.projects = g_slist_reverse(sysenv.projects);
}

/*****************************/
/* setup project browse mode */
/*****************************/
void project_browse_free(void)
{
GSList *list, *next;
struct project_pak *project;

list = sysenv.projects;
while (list)
  {
  next = g_slist_next(list);

  project = list->data;

  sysenv.projects = g_slist_remove(sysenv.projects, project);

  project_free(project);

  list = next;
  }

sysenv.projects = NULL;
}

/***************************************/
/* parse start elements in job details */
/***************************************/
void project_details_start(GMarkupParseContext *context,
                           const gchar *element_name,
                           const gchar **attribute_names,
                           const gchar **attribute_values,
                           gpointer data,
                           GError **error)
{
gint i;

if (g_strncasecmp(element_name, "job\0", 4) == 0)
  {
  i=0;
  while (*(attribute_names+i))
    {
//printf(" >>> [%d] = [%s] : [%s]\n", i, *(attribute_names+i), *(attribute_values+i));

    if (g_strncasecmp(*(attribute_names+i), "fqan", 4) == 0)
      {
      grid_vo_set(*(attribute_values+i), data);
      }
    i++;
    }
  }
}

/*****************************************/
/* store job details in a grid structure */
/*****************************************/
void project_details_text(GMarkupParseContext *context,
                          const gchar *text,
                          gsize text_len,  
                          gpointer data,
                          GError **error)
{
const gchar *element;
struct grid_pak *grid=data;

element = g_markup_parse_context_get_element(context);

//printf("CONTEXT [%s]\n TEXT: [%s]\n", element, text);

if (g_strncasecmp(element, "HostName", 8) == 0)
  {
  grid->remote_q = g_strdup(text);
  }
if (g_strncasecmp(element, "Input", 5) == 0)
  {
  grid->local_input = g_strdup(text);
  }
if (g_strncasecmp(element, "Output", 6) == 0)
  {
  grid->local_output = g_strdup(text);
  }
/* TODO - use a different element for this (passthrough job comment) */
if (g_strncasecmp(element, "ApplicationName", 15) == 0)
  {
  grid->description = g_strdup(text);
  }
}

/******************************************/
/* XML parsing for grisu job descriptions */
/******************************************/
struct grid_pak *project_parse_job_details(const gchar *text)
{
GMarkupParser xml_parser;
GMarkupParseContext *xml_context;
struct grid_pak *grid;
gchar *tmp1, *tmp2, *tmp3;

if (!text)
  return(NULL);

grid = grid_job_new();

/*
printf("---------------------------------------\n");
printf("%s\n", text);
printf("---------------------------------------\n");
*/

/* CURRENT - job details are a mixture of < > and &lt &gt symbols */
// g_markup_escape_text does the opposite of what we want - it converts everything to &lt &gt
//processed = g_markup_escape_text(text, -1);
// this does nothing
//processed = g_utf8_normalize(text, -1, G_NORMALIZE_DEFAULT);
// this does nothing
//processed = g_locale_from_utf8(text, -1, NULL, NULL, NULL);
//processed = g_uri_unescape_string(text, NULL);

/* fine, let's just pattern match replace the didgy stuff */
{
GRegex *regex;

/* &lt; -> < */
regex = g_regex_new("&lt;", 0, 0, NULL);
tmp1 = g_regex_replace_literal(regex, text, -1, 0, "<", G_REGEX_MATCH_NEWLINE_ANY, NULL);
g_regex_unref(regex);

/* &gt; -> < */
regex = g_regex_new("&gt;", 0, 0, NULL);
tmp2 = g_regex_replace_literal(regex, tmp1, -1, 0, ">", G_REGEX_MATCH_NEWLINE_ANY, NULL);
g_regex_unref(regex);

/* &amp; -> & */
regex = g_regex_new("&amp;", 0, 0, NULL);
tmp3 = g_regex_replace_literal(regex, tmp2, -1, 0, "&", G_REGEX_MATCH_NEWLINE_ANY, NULL);
g_regex_unref(regex);
}

/*
printf("=======================================\n");
printf("%s\n", tmp3);
printf("=======================================\n");
*/

/* setup context parse (ie callbacks) */
xml_parser.start_element = &project_details_start;
xml_parser.end_element = NULL;
xml_parser.text = &project_details_text;
xml_parser.passthrough = NULL;
xml_parser.error = NULL;
xml_context = g_markup_parse_context_new(&xml_parser, 0, grid, NULL);

//if (!g_markup_parse_context_parse(xml_context, text, strlen(text), NULL))
if (!g_markup_parse_context_parse(xml_context, tmp3, -1, NULL))
  {
  printf("read_xml() : parsing error.\n");
  g_markup_parse_context_free(xml_context);
  g_free(grid);
  return(NULL);
  }

g_markup_parse_context_free(xml_context);

g_free(tmp1);
g_free(tmp2);
g_free(tmp3);

return(grid);
}

/****************************/
/* local submission cleanup */
/****************************/
void project_local_stop(gpointer data)
{
GSList *item;
struct grid_pak *grid = data;

/* TODO - load output file (if any) */
//printf("parent project: %p\n", grid->parent);

if (grid->report)
  printf("Error: %s\n", grid->report);

/*
if (!project_valid(grid->parent))
  {
printf("Warning: Parent project has been destroyed.\n");
  grid->parent = NULL;
  }
*/

for (item=grid->remote_file_list ; item ; item=g_slist_next(item))
  {
  project_import_file(sysenv.workspace, item->data);
  }

// we save this now...
//grid_job_free(grid);
}

/************************************/
/* local submission background task */
/************************************/
#define DEBUG_PROJECT_LOCAL_START 1
void project_local_start(gpointer data)
{
gchar *input, *output;
struct file_pak *file;
struct grid_pak *grid=data;

g_assert(grid != NULL);

#if DEBUG_PROJECT_LOCAL_START
printf("local tmp working dir: %s\n", grid->local_cwd);
printf("executable: %s\n", grid->exename);
printf("input file: %s\n", grid->local_input);
printf("output file: %s\n", grid->local_output);
#endif

/* TODO - add output file to grid->remote_file_list ... for post processing */
input = g_build_filename(grid->local_cwd, grid->local_input, NULL);
output = g_build_filename(grid->local_cwd, grid->local_output, NULL);

file = file_type_from_filename(grid->local_input);
if (file)
  {
  switch (file->id)
    {
    case FDF:
      if (sysenv.siesta_path)
        task_fortran_exe(sysenv.siesta_path, input, output);
      break;

    case GAMESS:
      if (sysenv.gamess_path)
        task_fortran_exe(sysenv.gamess_path, input, output);
      break;

    case GULP:
      if (sysenv.gulp_path)
        task_fortran_exe(sysenv.gulp_path, input, output);
      break;

    case REAXMD:
//"reaxmd.x input.irx input.frx input.pdb"
      if (sysenv.reaxmd_path)
        {
        gint status;
        gchar *cmd;
        gchar *base;

// TODO - win32 ifdef needed (see task.c) line 321
// TODO - rewrite to use gstring and append includes
// TODO - might have to rewrite if the order pdb/irx/etc is important */
//cmd = g_strdup_printf("cd %s ; %s %s", grid->local_cwd, grid->exename, grid->local_input);
// NEW - reaxmd takes the basename for the project
// NB: the only gotcha is plumed on => additional project file needed
base = parse_strip_extension(grid->local_input);
cmd = g_strdup_printf("cd %s ; %s %s", grid->local_cwd, grid->exename, base);
status = task_sync(cmd);

        g_free(base);
        g_free(cmd);
        }
      break;

    default:
      printf("Unknown job type.\n");
    }
  }

grid->remote_file_list = g_slist_prepend(grid->remote_file_list, output);

g_free(input);
}

/*******************************/
/* local submission task setup */
/*******************************/
/* deprec */
void project_submit_local(gpointer project)
{
gchar *tmp, *filename;
const gchar *text;
GList *item, *list;
GSList *sitem, *slist;
struct grid_pak *job;

/* TODO - reusing grid structure ... rename? */
job = grid_job_new();
job->description = g_strdup("Local task");
job->parent = project;
job->remote_cwd = file_unused_directory();

/* copy all project files to job temp directory */
list = project_job_file_list(project);
for (item=list ; item ; item=g_list_next(item))
  {
/* build fullpath filename of copy */
/* NB: this import reference is NOT the same as the project tree imports list */
  text = import_fullpath(item->data);
  tmp = g_path_get_basename(text);
  filename = g_build_filename(job->remote_cwd, tmp, NULL);
  g_free(tmp);
  file_copy(text, filename);
  job->local_upload_list = g_slist_prepend(job->local_upload_list, filename);
  }

/* overwrite copied files with text buffer contents */
slist = project_imports_get(project);
for (sitem=slist ; sitem ; sitem=g_slist_next(sitem))
  {
/* build fullpath filename of copy */
  text = import_fullpath(sitem->data);
  tmp = g_path_get_basename(text);
  filename = g_build_filename(job->remote_cwd, tmp, NULL);
  g_free(tmp);

/* flush current copy (may have been altered in the text buffer) to job directory */
  import_save_as(filename, sitem->data);
  g_free(filename);
  }

/* fire off the background job and cleanup task */
tmp = g_strdup_printf("%s job", project_executable_required(project));
task_new(tmp, &project_local_start, job, &project_local_stop, job, NULL);
g_free(tmp);
}

