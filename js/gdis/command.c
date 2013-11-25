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
#include <stdlib.h>
#include <sys/stat.h>

#include "gdis.h"
#include "file.h"
#include "parse.h"
#include "model.h"
#include "test.h"
#include "grid.h"
#include "grisu_client.h"

/* main data structures */
extern struct sysenv_pak sysenv;

GHashTable *command_table;

enum 
{
COMMAND_DUMMY, // avoid pattern matching 0th <-> NULL element
COMMAND_EXIT, COMMAND_HISTORY, COMMAND_HELP, COMMAND_PWD, COMMAND_CD, COMMAND_COPY, COMMAND_LS,
COMMAND_SET, COMMAND_SCRIPT, COMMAND_VERSION,
COMMAND_FIND, COMMAND_JOBS, COMMAND_DOWNLOAD, COMMAND_REMOVE, COMMAND_STATUS,
COMMAND_GULP, COMMAND_GAMESS, COMMAND_SUBMIT,
COMMAND_GRID_CONFIG, COMMAND_GRID_MYPROXY, COMMAND_GRID_SHIBBOLETH, COMMAND_GRID_CONNECT, COMMAND_GRID_VOS,
COMMAND_TEST
};

void command_init(void)
{
command_table = g_hash_table_new(g_str_hash, g_str_equal);

/* general */
g_hash_table_replace(command_table, "exit", GINT_TO_POINTER(COMMAND_EXIT));
g_hash_table_replace(command_table, "ex", GINT_TO_POINTER(COMMAND_EXIT));
g_hash_table_replace(command_table, "quit", GINT_TO_POINTER(COMMAND_EXIT));

g_hash_table_replace(command_table, "help", GINT_TO_POINTER(COMMAND_HELP));
g_hash_table_replace(command_table, "he", GINT_TO_POINTER(COMMAND_HELP));
g_hash_table_replace(command_table, "?", GINT_TO_POINTER(COMMAND_HELP));

g_hash_table_replace(command_table, "history", GINT_TO_POINTER(COMMAND_HISTORY));
g_hash_table_replace(command_table, "hi", GINT_TO_POINTER(COMMAND_HISTORY));
g_hash_table_replace(command_table, "h", GINT_TO_POINTER(COMMAND_HISTORY));

g_hash_table_replace(command_table, "pwd", GINT_TO_POINTER(COMMAND_PWD));
g_hash_table_replace(command_table, "cd", GINT_TO_POINTER(COMMAND_CD));

g_hash_table_replace(command_table, "ls", GINT_TO_POINTER(COMMAND_LS));
g_hash_table_replace(command_table, "dir", GINT_TO_POINTER(COMMAND_LS));

g_hash_table_replace(command_table, "copy", GINT_TO_POINTER(COMMAND_COPY));
g_hash_table_replace(command_table, "co", GINT_TO_POINTER(COMMAND_COPY));
g_hash_table_replace(command_table, "cp", GINT_TO_POINTER(COMMAND_COPY));

g_hash_table_replace(command_table, "test", GINT_TO_POINTER(COMMAND_TEST));

g_hash_table_replace(command_table, "set", GINT_TO_POINTER(COMMAND_SET));

g_hash_table_replace(command_table, "script", GINT_TO_POINTER(COMMAND_SCRIPT));

g_hash_table_replace(command_table, "version", GINT_TO_POINTER(COMMAND_VERSION));
g_hash_table_replace(command_table, "ver", GINT_TO_POINTER(COMMAND_VERSION));

/* grid */
g_hash_table_replace(command_table, "mlog", GINT_TO_POINTER(COMMAND_GRID_MYPROXY));
g_hash_table_replace(command_table, "slog", GINT_TO_POINTER(COMMAND_GRID_SHIBBOLETH));
g_hash_table_replace(command_table, "con", GINT_TO_POINTER(COMMAND_GRID_CONNECT));
g_hash_table_replace(command_table, "connect", GINT_TO_POINTER(COMMAND_GRID_CONNECT));

g_hash_table_replace(command_table, "config", GINT_TO_POINTER(COMMAND_GRID_CONFIG));
g_hash_table_replace(command_table, "conf", GINT_TO_POINTER(COMMAND_GRID_CONFIG));

g_hash_table_replace(command_table, "vos", GINT_TO_POINTER(COMMAND_GRID_VOS));

/* grid/local job related */
g_hash_table_replace(command_table, "find", GINT_TO_POINTER(COMMAND_FIND));
g_hash_table_replace(command_table, "jobs", GINT_TO_POINTER(COMMAND_JOBS));
g_hash_table_replace(command_table, "rm", GINT_TO_POINTER(COMMAND_REMOVE));
g_hash_table_replace(command_table, "remove", GINT_TO_POINTER(COMMAND_REMOVE));

g_hash_table_replace(command_table, "stat", GINT_TO_POINTER(COMMAND_STATUS));
g_hash_table_replace(command_table, "status", GINT_TO_POINTER(COMMAND_STATUS));

g_hash_table_replace(command_table, "dl", GINT_TO_POINTER(COMMAND_DOWNLOAD));
g_hash_table_replace(command_table, "download", GINT_TO_POINTER(COMMAND_DOWNLOAD));

/* job creation */
g_hash_table_replace(command_table, "gamess", GINT_TO_POINTER(COMMAND_GAMESS));
g_hash_table_replace(command_table, "gamess-us", GINT_TO_POINTER(COMMAND_GAMESS));
g_hash_table_replace(command_table, "gulp", GINT_TO_POINTER(COMMAND_GULP));

g_hash_table_replace(command_table, "sub", GINT_TO_POINTER(COMMAND_SUBMIT));
g_hash_table_replace(command_table, "submit", GINT_TO_POINTER(COMMAND_SUBMIT));
}

/*********************/
/* free command data */
/*********************/
void command_cleanup(void)
{
g_hash_table_destroy(command_table);
}

/************************************/
/* set variable name/value keypairs */
/************************************/
void command_set(gchar **buff, gint num_tokens)
{

if (num_tokens > 2)
  {
  printf("Setting [%s] = [%s]\n", *(buff+1), *(buff+2));

  if (g_strncasecmp("service_address", *(buff+1), 15) == 0)
    grisu_ws_set(*(buff+2));

  if (g_strncasecmp("myproxy_username", *(buff+1), 16) == 0)
    grisu_keypair_set(*(buff+2), NULL);

  if (g_strncasecmp("myproxy_server", *(buff+1), 14) == 0)
    grisu_myproxy_set(*(buff+2), NULL);

  if (g_strncasecmp("myproxy_port", *(buff+1), 12) == 0)
    grisu_myproxy_set(NULL, *(buff+2));

  if (g_strncasecmp("current_vo", *(buff+1), 10) == 0)
    grisu_vo_set(*(buff+2));

  }
else
  printf("Insufficient tokens.\n");

}

/*****************************/
/* display command line help */
/*****************************/
void command_print_help(gchar **buff, gint num_tokens)
{

if (num_tokens < 2)
  {

printf("GDIS v%4.2f general commands\n\n", VERSION);

printf("cd <destination>\n");
printf("    - changes to target directory\n");

printf("pwd\n");
printf("    - prints the current working directory\n");

printf("ls\n");
printf("    - lists the files in the current working directory\n");

printf("copy <source> <destination>\n");
printf("    - convert between filetypes, based on extension (wildcards allowed)\n");

printf("script <filename>\n");
printf("    - executes the series of gdis commands contained in <filename>\n");

printf("help <topic>\n");
printf("    - available topics are: grid\n");

printf("version\n");
printf("    - display GDIS version information.\n");

printf("quit\n");
printf("    - exit program\n");

  }
else
  {

  if (g_strncasecmp("grid", *(buff+1), 4) == 0)
    {

printf("GDIS v%4.2f grid commands\n\n", VERSION);

printf("config\n");
printf("    - print the current configuration. Values can be altered using\n");
printf("    - set <name> <new_value>.\n");

printf("connect\n");
printf("    - authenticate to the grid using myproxy password.\n");

printf("mlog\n");
printf("    - authenticate to the grid using myproxy username and password.\n");

//printf("slog\n");
//printf("    - authenticate to the grid using shibboleth (not implemented yet).\n");

printf("find <application>\n");
printf("    - searches the grid for queues where <application> can be submitted.\n");

printf("jobs\n");
printf("    - prints all current jobs and their status.\n");

printf("status <jobname>\n");
printf("    - prints status of the job specified by <jobname>.\n");

printf("submit <application> <queue> <filename>\n");
printf("    - submits an <application> job using <filename> as an input file to the supplied <queue>.\n");
printf("    - currently supported applications are gulp and gamess.\n");

printf("download <jobname>\n");
printf("    - retrieves the results of the job specified by <jobname>.\n");

printf("remove <jobname>\n");
printf("    - removes and cleans up the job specified by <jobname>.\n");

    }

  }

}

/*******************************************************************/
/* convert one file type to another by loading/parsing then saving */
/*******************************************************************/
#define DEBUG_COMMAND_FILE_CONVERT 1
gint command_file_convert(gpointer type1, gpointer type2, gchar *path1, gchar *path2)
{
gint n, ns;
gchar *base, *ext, *path3;
GSList *item, *structures;
struct file_pak *inp=type1, *out=type2;
struct model_pak *model;

g_assert(inp != NULL);
g_assert(out != NULL);
g_assert(path1 != NULL);
g_assert(path2 != NULL);

/* allocate a new model and load the source file */
/* NB: some input files contain more than one structure */
/* CURRENT - core dump here ... suspect it's related to illegal memory free/write in fdf code */
model = model_new();
inp->read_file(path1, model);
structures = g_slist_find(sysenv.mal, model);

if (structures)
  ns = g_slist_length(structures);
else
  ns = 0;

#if DEBUG_COMMAND_FILE_CONVERT
printf("Converting [%d] item(s) of type [%s] to [%s]\n", ns, inp->label, out->label);
#endif

if (ns > 1)
  {
/* multiple structures in input file */
  base = parse_strip_extension(path2);
  ext = file_extension_get(path2);
  n = 1;
  for (item=structures ; item ; item=g_slist_next(item))
    {
    model = item->data;

    model->id = inp->id;

    path3 = g_strdup_printf("%s_%d.%s", base, n, ext);

//printf("* [%s][#%d] -> [%s]\n", path1, n, path3);

    out->write_file(path3, model);

    g_free(path3);

    model_delete(model);
    n++;
    }

  g_free(ext);
  g_free(base);

  }
else
  {
/* single structure in input file */
  if (ns)
    {
    model->id = inp->id;

//printf("* [%s] -> [%s]\n", path1, path2);

    out->write_file(path2, model);
    }

  model_delete(model);
  }

return(TRUE);
}

/*****************************************/
/* general copy command (with wildcards) */
/*****************************************/
void command_copy(gchar **buff, gint num_tokens)
{
gint n;
gchar *path1, *path2, *ext, *tmp, *inp, *out, *pattern;
GSList *list, *files;
struct file_pak *inp_pak, *out_pak;

if (num_tokens > 2)
  {
/* destination */
  out_pak = file_type_from_filename(*(buff+2));
  if (!out_pak)
    {
    printf("Unrecognized output filetype: [%s]\n", *(buff+2));
    g_strfreev(buff);
    return;
    }
  if (!out_pak->write_file)
    {
    printf("No write method for output file [%s]\n", *(buff+2));
    g_strfreev(buff);
    return;
    }

/* build a list of source files to be converted */
  path1 = g_path_get_dirname(*(buff+1));
  if (g_ascii_strncasecmp(".\0", path1, 2) == 0)
    {
    if (sysenv.cwd)
      {
      g_free(path1);
      path1 = g_strdup(sysenv.cwd);
      }
    }
  pattern = g_path_get_basename(*(buff+1));
  files = file_dir_list_pattern(path1, pattern);

  n = g_slist_length(files);

  if (n > 1)
    {
/* multi file copy - destination base name now uses input basename and output extension */
    ext = file_extension_get(*(buff+2));

/* iterate each source file and convert */
    for (list=files ; list ; list=g_slist_next(list))
      {
      inp_pak = file_type_from_filename(list->data);

      if (inp_pak)
        {
        if (inp_pak->read_file)
          {
/* build full input file path */
          inp = g_build_filename(path1, list->data, NULL);
/* replace input file extension with output file extension */
          tmp = parse_extension_set(list->data, ext);
/* get required output path */
          path2 = g_path_get_dirname(*(buff+2));
/* build full output file path */
          if (g_ascii_strncasecmp(".\0", path2, 2) == 0)
            out = g_build_filename(sysenv.cwd, tmp, NULL);
          else
            out = g_build_filename(path2, tmp, NULL);

/* copy the file */
          command_file_convert(inp_pak, out_pak, inp, out);

          g_free(path2);
          g_free(tmp);
          g_free(out);
          g_free(inp);
          }
        else
          {
printf("No read method for input file: [%s]\n", (gchar *) list->data);
          }
        }
      else
        {
printf("Unrecognized input filetype: [%s]\n", (gchar *) list->data);
        }
      }
    g_free(ext);
    }
  else
    {
/* single file copy */
    if (n == 1)
      {
      inp_pak = file_type_from_filename(*(buff+1));

      if (inp_pak)
        {
        if (inp_pak->read_file)
          {
          command_file_convert(inp_pak, out_pak, *(buff+1), *(buff+2));
          }
         else
          {
printf("No read method for input file: [%s]\n", *(buff+1));
          }
        }
      else
        {
printf("Unrecognized input filetype: [%s]\n", *(buff+1));
        }
      }
    }

  g_free(path1);
  g_free(pattern);

  free_slist(files);
  }
else
  printf("Error: insufficient tokens for <copy> command\n");
}

/****************************************/
/* print the current grid configuration */
/****************************************/
void command_print_grid(void)
{
printf("service_address: %s\n", grisu_ws_get());
printf("myproxy_username: %s\n", grisu_username_get());
printf("myproxy_server: %s\n", grisu_myproxy_get());
printf("myproxy_port: %s\n", grisu_myproxyport_get());
printf("current_VO: %s\n", grisu_vo_get());
}

/*********************************/
/* print a list of available VOs */
/*********************************/
void command_print_vos(void)
{
GSList *item, *list;

list = grisu_fqans_get();
for (item=list ; item ; item=g_slist_next(item))
  {
  printf("[%s]\n", (gchar *) item->data);
  }
}

/*********************/
/* submit a grid job */
/*********************/
void command_grid_run(gchar **buff)
{
}

/**************************************/
/* retrieve a job's working directory */
/**************************************/
void command_download(gchar **buff, gint num_tokens)
{
}

/***************************/
/* command line processing */
/***************************/
#define DEBUG_COMMAND_MAIN_LOOP 1
void command_main_loop(int argc, char **argv)
{
gint n, num_tokens;
gchar *line=NULL, *inp, **buff;
const gchar *name;
gboolean exit=FALSE;
gpointer token;
GSList *list, *files, *history=NULL;
FILE *script=NULL;

if (argc < 2)
  printf("gdis> Commandline interface. Enter ? for help.\n");

/* if additional keywords - process as single command line */
if (argc > 1)
  {
/* convert options into single string for processing */
  buff = g_malloc(argc*sizeof(gchar *));
  for (n=1 ; n<argc ; n++)
    *(buff+n-1) = g_strdup(argv[n]);
  *(buff+argc-1) = NULL;
  line = g_strjoinv(" ", buff);
  g_strfreev(buff);
  exit = TRUE;
  }

/* command line */
command_init();
do
  {
/* standard repeating prompt */

  printf("gdis> ");

  if (history)
    {
    if (history->data)
      {
/* TODO - some kind of fancy footwork needed to put info */
/* in stdin from the history list (eg if up arrow pressed) */
/* see parse_getchar_hidden() for a starting point */
      }
    }

/* attempt to get input from open script file */
  if (script)
    {
    line = file_read_line(script);
    if (!line)
      {
/* EOF - revert back to stdin input */
      fclose(script);
      script = NULL;
      }
    else
      {
/* found a line - print it to command line */
      printf("%s", line);
      }
    }

/* no input - get from stdin */
  if (!line)
    line = file_read_line(stdin);

/* if user does something like gdis < script.txt ... file_read_line(stdin) */
/* will return NULL when the it reaches EOF - so terminate at this point */
  if (!line)
    {
/* CURRENT - some way to reset so normal stdin is used instead of input script? */
    printf("exit\n");
    break;
    }

/* add to command history */
//history = g_slist_append(history, line);
history = g_slist_append(history, g_strstrip(line));

/* split the current input line and lookup 1st token */
  buff = tokenize(line, &num_tokens);
  if (num_tokens)
    token = g_hash_table_lookup(command_table, *buff);
  else
    token = NULL;

/* token matches a keyword? */
  if (token)
    {
    switch (GPOINTER_TO_INT(token))
      {
      case COMMAND_EXIT:
        exit=TRUE;
        break;

      case COMMAND_VERSION:
/* TODO - automate the buildstamp eg checkout/compile tests? */
        printf("GDIS v%4.2f [%s]\n", VERSION, BUILDSTAMP);
        break;

      case COMMAND_HISTORY:
        printf(" Command history:\n");
        for (list=history ; list ; list=g_slist_next(list))
          printf(" %s\n", (gchar *) list->data);
        break;

      case COMMAND_HELP:
        command_print_help(buff, num_tokens);
        break;

      case COMMAND_PWD:
        printf("%s\n", sysenv.cwd);
        break;

      case COMMAND_CD:
        if (num_tokens > 1)
          {
/* TODO - check if absolute path ... rather than just appending */
          inp = g_build_path(DIR_SEP, sysenv.cwd, *(buff+1), NULL);
          set_path(inp);
          g_free(inp);
          }
        printf("%s\n", sysenv.cwd);
        break;

      case COMMAND_LS:
        if (num_tokens > 1)
          files = file_dir_list_pattern(sysenv.cwd, *(buff+1));
        else
          files = file_dir_list(sysenv.cwd, FALSE);
        for (list=files ; list ; list=g_slist_next(list))
          printf("%s\n", (gchar *) list->data);
        break;

      case COMMAND_COPY:
        command_copy(buff, num_tokens);
        break;

      case COMMAND_TEST:
        for (n=1 ; n<num_tokens ; n++)
          test_run(*(buff+n));
        break;

      case COMMAND_SET:
        command_set(buff, num_tokens);
        break;

      case COMMAND_SCRIPT:
        if (num_tokens > 1)
          {
          script = fopen(*(buff+1), "rt");
          if (!script)
            printf("Failed to open: %s\n", *(buff+1));
          else
            {
            printf("Running: %s\n", *(buff+1));
/* stop auto termination - we have a file to read commands from */
            exit=FALSE;
            }
          }
        break;

      case COMMAND_GRID_CONFIG:
        command_print_grid();
        break;

      case COMMAND_GRID_CONNECT:
/* TODO - assert username already set */
name = grisu_username_get();
if (name)
  printf("Connecting using MYPROXY username: %s\n", name);
else
  {
  printf("Cannot connect: MYPROXY username not set.\n");
  break;
  }

        grid_authenticate(GRID_MYPROXY_PASSWORD);
        grid_connect(GRID_MYPROXY);
        break;

      case COMMAND_GRID_MYPROXY:
        printf("You have requested myproxy login.\n");
        grid_authenticate(GRID_MYPROXY);
        grid_connect(GRID_MYPROXY);
        break;

      case COMMAND_GRID_SHIBBOLETH:
        printf("You have requested shibboleth login.\n");
        grid_authenticate(GRID_SHIBBOLETH);
        break;

      case COMMAND_GRID_VOS:
        break;

      case COMMAND_STATUS:
        break;

      case COMMAND_JOBS:
        break;

      case COMMAND_DOWNLOAD:
        break;

      case COMMAND_REMOVE:
        break;

      case COMMAND_SUBMIT:
        break;
      }
    }
  g_strfreev(buff);

  line = NULL;
  }
while (!exit);

free_slist(history);

command_cleanup();
}

