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
#include <stdlib.h>

#include "gdis.h"
#include "grid.h"
#include "file.h"
#include "parse.h"
#include "model.h"
#include "grisu_client.h"

extern struct sysenv_pak sysenv;

/************************************************/
/* print list consisting of pointers to strings */
/************************************************/
void test_print_text_list(GSList *list)
{
GSList *item;

for (item=list ; item ; item=g_slist_next(item))
  {
  printf("[%s]\n", (gchar *) item->data);
  }
}

/*******************************/
/* run tests on grisu WS calls */
/*******************************/
void test_grisu(void)
{
gint passed=0, total=0;
GSList *list;

printf("testing grisu ...\n");

/* NEW - authenticate step */
/* TODO - make automatic whenever we get an auth fail */
while (!grid_auth_check())
  grid_authenticate(GRID_MYPROXY);


total++;
printf("\nTEST 1 - Submit location query\n");
grisu_auth_set(TRUE);
list = grisu_submit_find("mrbayes");
if (list)
  {
  test_print_text_list(list);
  passed++;
  }

total++;
printf("\nTEST 2 - Retrieve available VOs\n");
grisu_auth_set(TRUE);
list = grisu_fqans_get();
if (list)
  {
  test_print_text_list(list);
  passed++;
  }

/* TODO - upload/download file */
/* TODO - submit job */

printf("grisu tests passed: %d/%d\n", passed, total);
}


/***********************************************/
/* test a complete GULP job submit using GRISU */
/***********************************************/
void test_grisu_gulp(void)
{

printf("TODO - testing GULP submission to the GRID.\n");

/* auth */
/* find gulp */
/* VO ? */
/* create a test model (nanotube) */
/* save file */
/* upload */
/* submit job */
/* monitor */

}

/*******************************/
/* file I/O robustness testing */
/*******************************/
/* TODO - iterate through all file_pak's and use read_file (if exists) to */
/* read any and all files in the directory - should test robustness */
/* same for writing out */
/* TODO - may have to run as thread - so if one fails/hangs - it can be reported */
/* rather than terminating the overall application execution */
void test_file(void)
{
gint n;
GSList *list1, *list2, *files;

files = file_dir_list(sysenv.cwd, FALSE);

n = 0;

printf("Reading routine robustness test, please wait...\n");

for (list2=files ; list2 ; list2=g_slist_next(list2))
  {
  gchar *filename = list2->data;

printf("File: [%s]\n", filename);

  n++;
  for (list1=sysenv.file_list ; list1 ; list1=g_slist_next(list1))
    {
    struct file_pak *type = list1->data;
    if (type->read_file)
      {
      struct model_pak *model = model_new();

//if (type->id == 77)
  {
printf("Testing [%s][%d] reading routine ...\n", type->label, type->id);

      type->read_file(filename, model);
  }

      model_delete(model);

      }
    }
  }

printf("===========================================================================\n");
printf("Completed reading robustness tests on %d files in: %s\n", n, sysenv.cwd);
printf("===========================================================================\n");

}

/***************************/
/* run specified test type */
/***************************/
void test_run(gchar *name)
{
if (g_ascii_strncasecmp(name, "gri", 3) == 0)
  {
  test_grisu();
  }

if (g_ascii_strncasecmp(name, "gg\0", 3) == 0)
  {
  test_grisu_gulp();
  }

if (g_ascii_strncasecmp(name, "file", 4) == 0)
  {
  test_file();
  }
}

