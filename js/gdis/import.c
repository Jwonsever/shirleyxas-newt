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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "gdis.h"
#include "file.h"
#include "scan.h"
#include "parse.h"
#include "interface.h"
#include "import.h"
#include "model.h"
#include "project.h"
#include "graph.h"
#include "reaxmd.h"

/* main structures */
extern struct sysenv_pak sysenv;

/* general wrapper for storing siesta_pak, games_pak etc */
/* FIXME - placing here, as it's small enough not to warrant its own file/header */
struct config_pak
{
gint id;
gint grafted;
gpointer data;
gchar *unparsed;
};

/* the aim of this is to provide a structure for storing a general */
/* collection of data (models, graphs, output data, etc) PLUS a */
/* new minimal change proceedure for dealing with input files */

struct import_pak
{
gint count;
gchar *label;
gchar *path;
gchar *filename;
gchar *fullpath;

/* NEW - record file_pak types and make it possible to force the type used */
gpointer import_type;
gpointer export_type;

/* NEW - extra goodness */
gint grid;
gchar *remote_path;
gulong timestamp;
gchar *state;

gint bigfile;
gint preview;
gchar *preview_buffer;

/* NEW - contains TRUE/FALSE as flags for determining if the object */
/* in question should be added */
gint object_filter[IMPORT_LAST];
/* NEW - this contains the recognized objects from the import */
/* intended to return GSList for key values IMPORT_MODEL, IMPORT_GRAPH etc */
GSList *object_array[IMPORT_LAST];

/* NEW - GUI elements */
/* the keys will be const gchar pointers to tab names eg FILE_PREVIEW */
GHashTable *gui_tabs;
};

/*******************/
/* alloc primitive */
/*******************/
gpointer import_new(void)
{
gint i;
struct import_pak *import;

import = g_malloc(sizeof(struct import_pak));

g_atomic_int_set(&(import->count), 0);

import->label = NULL;
import->path = NULL;
import->filename = NULL;
import->fullpath = NULL;
import->state = NULL;

import->import_type = NULL;
import->export_type = NULL;

import->grid = FALSE;
import->remote_path = NULL;
import->timestamp = 0;

import->bigfile = FALSE;
import->preview = FALSE;
import->preview_buffer = NULL;

/* lists of objects found (if any) */
for (i=IMPORT_LAST ; i-- ; )
  import->object_array[i] = NULL;

/* keys are const gchars, values are not used (yet) */
import->gui_tabs = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);

return(import);
}

/******************/
/* free primitive */
/******************/
void import_free(gpointer data)
{
gint i;
GSList *item, *list;
struct import_pak *import=data;

g_assert(import != NULL);

if (g_atomic_int_get(&(import->count)))
  {
printf("import_free() error: import %p still in use.\n", import);
  return;
  }

/* free import strings */
g_free(import->label);
g_free(import->path);
g_free(import->filename);
g_free(import->fullpath);
g_free(import->state);
g_free(import->remote_path);
g_free(import->preview_buffer);

/* free attached objects */
for (i=IMPORT_LAST ; i-- ; )
  {
  switch (i)
    {
    case IMPORT_MODEL:
      list = import->object_array[i];
      for (item=list ; item ; item=g_slist_next(item))
        model_free(item->data);
      break;

    case IMPORT_GRAPH:
      break;

    case IMPORT_CONFIG:
      list = import->object_array[i];
      for (item=list ; item ; item=g_slist_next(item))
        config_free(item->data);
      break;

    }

  g_slist_free(import->object_array[i]); 
  }

g_hash_table_destroy(import->gui_tabs);

g_free(import);
}

/*****************************/
/* set file_pak to export as */
/*****************************/
void import_export_type_set(gpointer type, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

import->export_type = type;
}

/*****************************/
/* get file_pak to export as */
/*****************************/
gpointer import_export_type_get(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->export_type);
}

/*****************************************/
/* get the object list of requested type */
/*****************************************/
GSList *import_object_list_get(gint type, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

if (type >=0 && type < IMPORT_LAST)
  return(import->object_array[type]);

return(NULL);
}

/****************************************/
/* get the nth object of requested type */
/****************************************/
gpointer import_object_nth_get(gint type, gint n, gpointer data)
{
GSList *list;

list = import_object_list_get(type, data);

return(g_slist_nth_data(list, n));
}

/*****************************************/
/* search for a config of requested type */
/*****************************************/
gpointer import_config_get(gint type, gpointer data)
{
GSList *item, *list;
struct config_pak *config;

list = import_object_list_get(IMPORT_CONFIG, data);

for (item=list ; item ; item=g_slist_next(item))
  {
  config = item->data;
  if (config->id == type)
    return(config);
  }
return(NULL);
}

/********************************************/
/* search for config data of requested type */
/********************************************/
/* NB: only returns first item found */
gpointer import_config_data_get(gint type, gpointer data)
{
GSList *item, *list;
struct config_pak *config;

list = import_object_list_get(IMPORT_CONFIG, data);

for (item=list ; item ; item=g_slist_next(item))
  {
  config = item->data;
  if (config->id == type)
    return(config->data);
  }
return(NULL);
}

/************************************/
/* add an object list of given type */
/************************************/
/* TODO - object_add ... which simply adds to existing list or creates */
void import_object_add(gint type, gpointer object, gpointer data)
{
struct import_pak *import = data;
struct model_pak *model;

g_assert(import != NULL);

/* NEW - setup GUI elements */
switch (type)
  {
  case IMPORT_MODEL:
    model = object;
    g_hash_table_insert(import->gui_tabs, TAB_PREVIEW, NULL);
    if (model->periodic == 3)
      g_hash_table_insert(import->gui_tabs, TAB_SURFACES, NULL);
    break;
  }

/* add to list */
if (type >=0 && type < IMPORT_LAST)
  {
  import->object_array[type] = g_slist_append(import->object_array[type], object);
  }
}

/**********************************/
/* remove an object from the list */
/**********************************/
void import_object_remove(gint type, gpointer object, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

if (type >=0 && type < IMPORT_LAST)
  {
  import->object_array[type] = g_slist_remove(import->object_array[type], object);
  }
}

/********************/
/* free all objects */
/********************/
void import_object_free_all(gpointer data)
{
gint i;
GSList *item, *next;
struct import_pak *import = data;

g_assert(import != NULL);

for (i=0 ; i<IMPORT_LAST ; i++)
  {
  item = import->object_array[i];
  while (item)
    {
    next = g_slist_next(item);

    import->object_array[i] = g_slist_remove(import->object_array[i], item);

    switch(i)
      {
      case IMPORT_CONFIG:
        config_free(item->data);
        break;
      case IMPORT_GRAPH:
        graph_free(item->data);
        break;
      case IMPORT_MODEL:
        model_delete(item->data);
        break;

      default:
        printf("TODO - free data: %p (id=%d)\n", item->data, i);
      }
    item=next;
    }

/* enforce NULL lists */
  import->object_array[i]=NULL;
  }
}

/*********************************************/
/* retrieve the list of tabs to be displayed */
/*********************************************/
GList *import_gui_tab_list_get(gpointer data)
{
struct import_pak *import = data;

if (import)
  return(g_hash_table_get_keys(import->gui_tabs));

return(NULL);
}

/**********************************/
/* thread safe reference counting */
/**********************************/
void import_inc(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

g_atomic_int_inc(&(import->count));
}

/**********************************/
/* thread safe reference counting */
/**********************************/
gint import_dec(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(g_atomic_int_dec_and_test(&(import->count)));
}

/**********************************/
/* get the current referene count */
/**********************************/
gint import_count(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(g_atomic_int_get(&(import->count)));
}

/***************************/
/* preview mode primitives */
/***************************/
void import_preview_set(gint value, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

import->preview = value;
}

gint import_preview_get(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->preview);
}

void import_preview_buffer_set(gchar *buffer, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

g_free(import->preview_buffer);

import->preview_buffer = buffer;
}

const gchar *import_preview_buffer_get(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

if (import->bigfile)
  return("File too large, preview mode disabled.\n");

if (import->preview_buffer)
  return(import->preview_buffer);

return("File preview unavailable (missing data or unspecified save type).");
}

/*********************/
/* access primitives */
/*********************/
/* FIXME - need to get some mutex locking happening around this */
gint import_grid_get(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->grid);
}

/*********************/
/* access primitives */
/*********************/
void import_remote_path_set(const gchar *path, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

g_free(import->remote_path);
if (path)
  import->remote_path = g_strdup(path);
else
  import->remote_path = NULL;
}

/*********************/
/* access primitives */
/*********************/
gchar *import_remote_path_get(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->remote_path);
}

/***********************/
/* initialize primtive */
/***********************/
/* FIXME - need to get some mutex locking happening around this */
void import_grid_set(gint value, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

import->grid = value;
}

/***************************************/
/* stamp the import with a time marker */
/***************************************/
void import_stamp_set(gpointer data)
{
struct import_pak *import = data;
GTimeVal result;

g_assert(import != NULL);

g_get_current_time(&result);

import->timestamp = result.tv_sec;
}

/**************************************************/
/* return time (secs) since last stamp took place */
/**************************************************/
glong import_stale_get(gpointer data)
{
struct import_pak *import = data;
GTimeVal result;

g_assert(import != NULL);

g_get_current_time(&result);

return(result.tv_sec - import->timestamp);
}

/***************************************************/
/* return string representation of an import state */
/***************************************************/
const gchar *import_state_get(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->state);
}

/***********************/
/* set an import state */
/***********************/
void import_state_set(const gchar *state, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

g_free(import->state);
import->state = g_strdup(state);
}

/*********************/
/* access primitives */
/*********************/
gchar *import_label(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

/* TODO - full path */

return(import->label);
}

/*********************/
/* access primitives */
/*********************/
gchar *import_fullpath(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->fullpath);
}

/*********************/
/* access primitives */
/*********************/
gchar *import_filename(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->filename);
}

/*********************/
/* access primitives */
/*********************/
gchar *import_path(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->path);
}

/***********************/
/* set an import label */
/***********************/
void import_label_set(const gchar *label, gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

g_free(import->label);
if (label)
  import->label = g_strdup(label);
else
  import->label = NULL;
}

/***********************/
/* get an import label */
/***********************/
const gchar *import_label_get(gpointer data)
{
struct import_pak *import = data;

g_assert(import != NULL);

return(import->label);
}

/**********************************/
/* set new fullpath for an import */
/**********************************/
void import_fullpath_set(const gchar *fullpath, gpointer data)
{
struct import_pak *import=data;

g_assert(fullpath != NULL);
g_assert(import != NULL);

g_free(import->fullpath);
g_free(import->filename);
g_free(import->path);

import->fullpath = g_strdup(fullpath);
import->filename = g_path_get_basename(fullpath);
import->path = g_path_get_dirname(fullpath);

if (!import->label)
  import->label = g_strdup(import->filename);
}

/***************************************/
/* set new path/filename for an import */
/***************************************/
void import_filename_set(const gchar *filename, gpointer data)
{
struct import_pak *import=data;

g_assert(filename != NULL);
g_assert(import != NULL);

g_free(import->filename);
g_free(import->fullpath);

import->filename = g_strdup(filename);

if (!import->label)
  import->label = g_strdup(filename);

if (import->path)
  import->fullpath = g_build_filename(import->path, filename, NULL);
else
  {
  import->path = g_strdup(sysenv.cwd);
  import->fullpath = g_build_filename(sysenv.cwd, filename, NULL);
  }
}

/******************************/
/* assign the bigfile setting */
/******************************/
// currently used to indicate no preview buffer should be generated
void import_bigfile_set(gint value, gpointer data)
{
struct import_pak *import=data;

import->bigfile = value;
}

/*****************************/
/* retrieve the bigfile flag */
/*****************************/
gint import_bigfile_get(gpointer data)
{
struct import_pak *import=data;

return(import->bigfile);
}

/****************************************************/
/* first time initialize on all models in an import */
/****************************************************/
// basically a model_prep() for imports
gint import_init(gpointer data)
{
GSList *item;
struct import_pak *import=data;

if (!import)
  return(FALSE);

for (item=import->object_array[IMPORT_MODEL] ; item ; item=g_slist_next(item))
  {
  model_prep(item->data);
  }

return(TRUE);
}

/*****************/
/* export a file */
/*****************/
#define DEBUG_IMPORT_SAVE 0
gint import_file_save(gpointer data)
{
struct file_pak *type;
struct import_pak *import=data;
struct model_pak *model;

#if DEBUG_IMPORT_SAVE
printf("import_file_save(): big=%d, preview=%d\n", import->bigfile, import->preview);
#endif

if (import->export_type)
  type = import->export_type;
else
  type = file_type_from_filename(import->fullpath);

if (!type)
  {
  fprintf(stderr, "Unknown save type for: %s\n", import->filename);
  return(1);
  }

/* NEW - if file is (or would be) too big - disable preview */
/* TODO - rather than disable ... display chunks of the file */
if (import->preview)
  {
  if (import->bigfile)
    {
    g_free(import->preview_buffer);
    import->preview_buffer = NULL;
    }
  }

#if DEBUG_IMPORT_SAVE
printf("Attempting to save as: %s\n", type->label);
#endif

if (!type->export_file)
  {
/* no export routine + preview mode -> display actual contents */
  if (import->preview)
    {
    g_free(import->preview_buffer);

/* NEW - write to tmp file and set that as preview buffer */
    if (type->write_file)
      {
      gchar *tmp;

      tmp = file_unused_name(sysenv.cwd, NULL, NULL);

      model = import_object_nth_get(IMPORT_MODEL, 0, import);

      if (model)
        {
        type->write_file(tmp, model);
        g_file_get_contents(tmp, &import->preview_buffer, NULL, NULL);
        g_remove(tmp);
        }
      else
        import->preview_buffer=NULL;

      g_free(tmp);
      }
    else
      g_file_get_contents(import->fullpath, &import->preview_buffer, NULL, NULL);

    return(0); 
    }
  else
    {

/* legacy fallback (as in import_file() */
#if DEBUG_IMPORT_SAVE
printf("No export routine for: %s\n", type->label);
printf("Attempting legacy fallback on: %s\n", import->fullpath);
#endif

model = import_object_nth_get(IMPORT_MODEL, 0, import);
if (!model)
  {
#if DEBUG_IMPORT_SAVE
printf("Nothing to save!\n");
#endif
  return(1);
  }

if (type->write_file)
  {
  return(type->write_file(import->fullpath, model));
  }
else
  {
#if DEBUG_IMPORT_SAVE
printf("No legacy write method available.\n");
#endif
  fprintf(stderr, "Save routine not implemented for [%s].\n", type->label);
  }

    return(1);
    }
  }

return(type->export_file(import));
}

/**********************************************/
/* export a file as a different filename/type */
/**********************************************/
gint import_save_as(gchar *fullpath, gpointer data)
{
gint status;
gchar *save_full;
gpointer save_type;
struct import_pak *import=data;

g_assert(import != NULL);

save_type = import->export_type;
save_full = import->fullpath;

/* force a save by extension */
import->fullpath = fullpath;
import->export_type = NULL;

/* force a file save */
import->preview = FALSE;

status = import_file_save(import);

/* restore */
import->export_type = save_type;
import->fullpath = save_full;

return(status);
}

/*******************************/
/* a new import with full path */
/*******************************/
#define DEBUG_IMPORT_FILE 0
#define IMPORT_BIGFILE 200000
/* TODO - can change file_pak type to use ... current - use extension */
gpointer import_file(const gchar *fullpath, struct file_pak *type)
{
gint bigfile=FALSE;
struct import_pak *import=NULL;

g_assert(fullpath != NULL);

/* no filter - use file extension */
if (!type)
  {
  type = file_type_from_filename(fullpath);
  if (!type)
    return(NULL);
  }

/* if filter set to generic - use file extension */
if (type->id == DATA)
  {
  type = file_type_from_filename(fullpath);
  if (!type)
    return(NULL);
  }

// decide if we should set the bigfile flag
if (file_byte_size(fullpath) > IMPORT_BIGFILE)
  {
#if DEBUG_IMPORT_FILE
printf("Setting bigfile flag.\n");
#endif
  bigfile=TRUE;
  }

if (!type->import_file)
  {
#if DEBUG_IMPORT_FILE
printf("No import routine for: %s\n", type->label);
printf("File size = %d\n", file_byte_size(fullpath));
#endif

/* CURRENT - attempt legacy fallback ? */
  if (type->read_file)
    {
    gchar *filename;
    struct model_pak *model;

    filename = g_strdup(fullpath);

#if DEBUG_IMPORT_FILE
printf("Attempting legacy fallback on: %s\n", filename);
#endif

    model = model_new();
    if (!type->read_file(filename, model))
      {
      import = import_new();
      import->bigfile = bigfile;
      import->fullpath = filename;
      import->filename = g_path_get_basename(fullpath);
      import->label = g_strdup(import->filename);

/* check and set to sysenv.cwd if . */
      import->path = g_path_get_dirname(fullpath);
      if (g_strncasecmp(import->path, ".\0", 2) == 0)
        {
        g_free(import->path);
        import->path = g_strdup(sysenv.cwd);

        g_free(import->fullpath);
        import->fullpath = g_build_filename(sysenv.cwd, import->filename, NULL);
        }

/* FIXME - some read_file() functions generate multiple models */
/* add model to import */
      import_object_add(IMPORT_MODEL, model, import);

      return(import);
      }

    gui_text_show(ERROR, "Legacy read failed.\n");
    model_delete(model);

    }

  return(NULL);
  }
else
  {
#if DEBUG_IMPORT_FILE
  printf("Importing as: %s\n", type->label);
#endif

  import = import_new();
  import->bigfile = bigfile;
  import->fullpath = g_strdup(fullpath);
  import->filename = g_path_get_basename(fullpath);
  import->label = g_strdup(import->filename);

/* check and set to sysenv.cwd if . */
  import->path = g_path_get_dirname(fullpath);
  if (g_strncasecmp(import->path, ".\0", 2) == 0)
    {
    g_free(import->path);
    import->path = g_strdup(sysenv.cwd);

    g_free(import->fullpath);
    import->fullpath = g_build_filename(sysenv.cwd, import->filename, NULL);
    }

/* call the necessary file parsing routines */
  if (type->import_file(import))
    {
    import_free(import);
    return(NULL);
    }
  }

return(import);
}

/************************/
/* debugging enumerator */
/************************/
void import_print(gpointer data)
{
}

/***********************************/
/* write all import chunks to file */
/***********************************/
void import_save(gchar *fullpath, gpointer data)
{
printf("TODO\n");
}

/***********************************************/
/* find parent import of the given data object */
/***********************************************/
gpointer import_find(gint id, gpointer data)
{
GSList *item1, *item2, *list;
struct import_pak *import;

//printf("import_find() : data=%p, id=%d\n", data, id);
list = project_imports_get(sysenv.workspace);

if (id >= 0 && id < IMPORT_LAST)
  {
  for (item1=list ; item1 ; item1=g_slist_next(item1))
    {
    import = item1->data;
    for (item2=import->object_array[id] ; item2 ; item2=g_slist_next(item2))
      {
      if (item2->data == data)
        {
        return(import);
        }
      }
    }
  }
return(NULL);
}

/**************************/
/* config alloc primitive */
/**************************/
gpointer config_new(gint id, gpointer data)
{
struct config_pak *config;

config = g_malloc(sizeof(struct config_pak));

//printf("new config, id=%d\n", id);

config->id = id;
config->grafted = FALSE;

if (data)
  config->data = data;
else
  {
  switch(id)
    {
    case GAMESS:
      config->data = gamess_new();
      break;
    case GULP:
      config->data = gulp_new();
      break;
    case NWCHEM:
      config->data = nwchem_new();
      break;
    case FDF:
      config->data = siesta_new();
      break;
    case REAXMD:
      config->data = reaxmd_new();
      break;

    default:
      fprintf(stderr, "Warning: unknown computational package.\n");
      config->data=NULL;
    }
  }

config->unparsed = NULL;

return(config);
}

/*********************/
/* set unparsed text */
/*********************/
void config_unparsed_set(gchar *unparsed, gpointer data)
{
struct config_pak *config=data;

g_assert(config != NULL);

config->unparsed = unparsed;
}

/***********************/
/* GUI tree primitives */
/***********************/
/* TODO - can we find a better way that applies to all tree objects? */
void config_grafted_set(gint value, gpointer data)
{
struct config_pak *config=data;

g_assert(config != NULL);

config->grafted = value;
}

/***********************/
/* GUI tree primitives */
/***********************/
gint config_grafted_get(gpointer data)
{
struct config_pak *config=data;

g_assert(config != NULL);

return(config->grafted);
}

/*************************/
/* config free primitive */
/*************************/
void config_free(gpointer data)
{
struct config_pak *config=data;

g_assert(config != NULL);

switch (config->id)
  {
  case FDF:
    siesta_free(config->data);
    break;

  case GULP:
    gulp_free(config->data);
    break;

  case GAMESS:
    gamess_free(config->data);
    break;

  case NWCHEM:
    nwchem_free(config->data);
    break;

  case REAXMD:
    reaxmd_free(config->data);
    break;

  default:
/* unknown config is only a problem if the data pointer is not NULL */
    if (config->data)
      fprintf(stderr, "Error: missing free for config [id=%d] pointer: %p\n", config->id, config->data);
  }

g_free(config);
}

/***************************/
/* config access primitive */
/***************************/
gint config_id(gpointer data)
{
struct config_pak *config=data;

g_assert(config != NULL);

return(config->id);
}

/**************************/
/* config label primitive */
/**************************/
const gchar *config_label(gpointer data)
{
struct config_pak *config=data;
struct file_pak *file;

g_assert(config != NULL);

file = get_file_info(GINT_TO_POINTER(config->id), BY_FILE_ID);
if (file)
  {
  return(file->label);
  }

return("unknown configuration");
}

/***************************/
/* config access primitive */
/***************************/
gpointer config_data(gpointer data)
{
struct config_pak *config=data;

g_assert(config != NULL);

return(config->data);
}

/***************************/
/* config access primitive */
/***************************/
gchar *config_unparsed_get(gpointer data)
{
struct config_pak *config=data;

g_assert(config != NULL);

return(config->unparsed);
}

