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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "gdis.h"
#include "coords.h"
#include "file.h"
#include "parse.h"
#include "library.h"
#include "matrix.h"
#include "model.h"
#include "scan.h"
#include "interface.h"

#define DEBUG_MORE 0
#define MAX_KEYS 15

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

enum {
LIBRARY_START,
LIBRARY_FOLDER_NAME, LIBRARY_ENTRY_NAME,
LIBRARY_UNIT_CELL, LIBRARY_LATTICE_VECTORS, LIBRARY_COORDINATES,
LIBRARY_STOP
};

/***************************************************/
/* populate the symbol table for the token scanner */
/***************************************************/
void library_symbols_init(gpointer scan)
{
gint i;

for (i=LIBRARY_START ; i<LIBRARY_STOP ; i++)
  {
  switch (i)
    {
    case LIBRARY_FOLDER_NAME:
      scan_symbol_add(scan, "library_folder", i);
      break;
    case LIBRARY_ENTRY_NAME:
      scan_symbol_add(scan, "library_entry", i);
      break;
    case LIBRARY_UNIT_CELL:
      scan_symbol_add(scan, "unit_cell", i);
      break;
    case LIBRARY_LATTICE_VECTORS:
      scan_symbol_add(scan, "lattice_vectors", i);
      break;
    case LIBRARY_COORDINATES:
      scan_symbol_add(scan, "coordinates", i);
      break;
    }
  }
}

/*************************/
/* free a library folder */
/************************/
void library_folder_free(gpointer data)
{
GSList *list;
struct folder_pak *folder = data;
struct entry_pak *entry;

list = folder->list;
while (list)
  {
  entry = list->data;
  list = g_slist_next(list);
  g_free(entry->name);
  g_string_free(entry->info, TRUE);
//  g_free(entry->offset);
  model_free(entry->model);

  g_free(entry);
  }
g_slist_free(folder->list);
g_free(folder);
}

/******************************/ 
/* create a new library entry */
/******************************/ 
gpointer library_entry_add(const gchar *folder_name, const gchar *entry_name, gpointer model)
{
struct folder_pak *folder;
struct entry_pak *entry;

/* TODO - check for entry repetitions within a given folder */
entry = g_malloc(sizeof(struct entry_pak));
entry->model = model;
entry->name = g_strdup(entry_name);
entry->info = g_string_new(NULL);

folder = g_hash_table_lookup(sysenv.library, folder_name);
if (!folder)
  {
  folder = g_malloc(sizeof(struct folder_pak));
  folder->list = NULL;
  g_hash_table_insert(sysenv.library, g_strdup(folder_name), folder);
  }
folder->list = g_slist_prepend(folder->list, entry);

return(entry);
}

/**************************/
/* retrieve library entry */
/**************************/
gpointer library_entry_get(const gchar *folder_name, const gchar *entry_name)
{
gint len;
GSList *list;
struct folder_pak *folder;
struct entry_pak *entry;
struct model_pak *model;

len = strlen(entry_name);

folder = g_hash_table_lookup(sysenv.library, folder_name);
if (folder)
  {
  for (list=folder->list ; list ; list=g_slist_next(list))
    {
    entry = list->data;

    if (g_ascii_strncasecmp(entry->name, entry_name, len) == 0)
      {
      model = model_dup(entry->model);
      return(model);
      }
    }
  }

return(NULL);
}

/*****************************/
/* unit cell parse primitive */
/*****************************/
void library_parse_cell(gpointer scan, struct model_pak *model)
{
gint num_tokens;
gchar **buff;

if (!model)
  return;

buff = scan_get_tokens(scan, &num_tokens);
if (num_tokens > 5)
  {
  model->periodic = 3;
  model->fractional = TRUE;
  model->pbc[0] = str_to_float(buff[0]);  
  model->pbc[1] = str_to_float(buff[1]);  
  model->pbc[2] = str_to_float(buff[2]);  
  model->pbc[3] = D2R*str_to_float(buff[3]);  
  model->pbc[4] = D2R*str_to_float(buff[4]);  
  model->pbc[5] = D2R*str_to_float(buff[5]);  
  }

g_strfreev(buff);
}

/**********************************/
/* lattice vector parse primitive */
/**********************************/
void library_parse_lattice(gpointer scan, struct model_pak *model)
{
}

/*******************************/
/* coordinates parse primitive */
/*******************************/
void library_parse_coords(gpointer scan, struct model_pak *model)
{
gint num_tokens;
gchar **buff;
struct core_pak *core;

if (!model)
  return;

while (!scan_complete(scan))
  {
  buff = scan_get_tokens(scan, &num_tokens);
  if (!buff)
    break;

  if (num_tokens == 4)
    {
    if (elem_test(*buff))
      {
      core = core_new(*buff, NULL, model);
      model->cores = g_slist_prepend(model->cores, core);
      core->x[0] = str_to_float(*(buff+1));
      core->x[1] = str_to_float(*(buff+2));
      core->x[2] = str_to_float(*(buff+3));
      } 
    }
  else
    {
// encountered something not a coordinates line
// put it back and exit so we can test it again for keywords
    scan_put_line(scan);
    g_strfreev(buff);
    break;
    }

  g_strfreev(buff);
  }
}

/********************************/
/* setup the library hash table */
/********************************/
#define DEBUG_LIBRARY_INIT 0
void library_init(void)
{
gint num_tokens, symbol;
gchar **buff, *folder=NULL, *filename;
gpointer scan;
struct entry_pak *entry = NULL;
struct model_pak *model=NULL;

/* setup hash table */
sysenv.library = g_hash_table_new_full(g_str_hash, g_str_equal,
                                       g_free, library_folder_free);

/* scan the library file (gdis.library = cif format) for folder/entry info */
filename = g_build_filename(sysenv.gdis_path, LIBRARY_FILE, NULL);

#if DEBUG_LIBRARY_INIT
printf("library_init(): %s\n", filename);
#endif

scan = scan_new(filename);
library_symbols_init(scan);

while (!scan_complete(scan))
  {
  buff = scan_get_tokens(scan, &num_tokens);
  if (!buff)
    break;

  symbol = scan_symbol_match(scan, *buff);

  switch (symbol)
    {
    case LIBRARY_FOLDER_NAME:
      if (num_tokens > 1)
        folder = g_strdup(*(buff+1));
      entry = NULL;
      break;

    case LIBRARY_ENTRY_NAME:
      if (num_tokens > 1)
        {
#if DEBUG_LIBRARY_INIT
printf("Attempting to add [%s] - [%s]\n", folder, buff[1]);
#endif
        model = model_dup(NULL);
        if (folder)
          {
          entry = library_entry_add(folder, *(buff+1), model);
          g_free(folder);
          }
        else
          entry = library_entry_add("default", *(buff+1), model);
        }
      break;

    case LIBRARY_UNIT_CELL:
      library_parse_cell(scan, model);
      break;

    case LIBRARY_LATTICE_VECTORS:
// TODO
      break;

    case LIBRARY_COORDINATES:

if (num_tokens > 1)
  {
  if (g_ascii_strncasecmp(*(buff+1), "cart", 4) == 0)
    model->fractional = FALSE;
  else
    model->fractional = TRUE;
  }

      library_parse_coords(scan, model);
      break;
    }

  g_strfreev(buff);
  }

scan_free(scan);

return;
}

