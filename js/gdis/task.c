/*
Copyright (C) 2000 by Sean David Fleming

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

/* irix */
#define _BSD_SIGNALS 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gdis.h"
#include "file.h"
#include "task.h"
#include "grid.h"
#include "interface.h"

/* top level data structure */
extern struct sysenv_pak sysenv;

#ifndef __WIN32
#include <sys/wait.h>
#endif

/**********************************************/
/* execute task in thread created by the pool */
/**********************************************/
void task_process(struct task_pak *task, gpointer data)
{
gint pid, status;

/* checks */
if (!task)
  return;
if (task->status != QUEUED)
  return;

/* setup for current task */
task->pid = getpid();

//printf("Running pid: %d\n", task->pid);
//printf("Running thread: %p\n", g_thread_self());

/* TODO - should be mutex locking this */
task->status = RUNNING;

task->timer = g_timer_new();

/* NB: the primary task needs to not do anything that could */
/* cause problems eg update GUI elements since this can cause conflicts */

if (task->function)
  {
// execute primary task (non-blocking)
//  task->pid = task->function(task->ptr1, task);
  task->pid = task->function(task->ptr1);

//printf("Interruptable child pid: %d\n", task->pid);

// argh, windows strikes again
#ifndef WIN32
/* use wait() waits for all */
pid = wait(&status);
#endif

//printf("All children have completed.\n");
// TODO - use g_child_watch_add_full? or g_poll ? try and make this portable

  }
else
  {
/* execute the primary task (blocking) */
  if (task->primary)
    task->primary(task->ptr1, task);
  }

/* NB: GUI updates should ALL be done in the cleanup task, since we */
/* do the threads enter to avoid problems - this locks the GUI */
/* until we call the threads leave function at the end */

gdk_threads_enter();

if (task->status != KILLED)
  {
/* execute the cleanup task */
  if (task->cleanup)
    task->cleanup(task->ptr2);
  task->status = COMPLETED;
  }

gdk_flush();
gdk_threads_leave();

/* job completion notification */
gdk_beep();
}

/***************************************************/
/* set up the thread pool to process task requests */
/***************************************************/
void task_queue_init(void)
{
#ifdef G_THREADS_ENABLED
g_thread_init(NULL);
gdk_threads_init();
if (!g_thread_supported())
  {
/* TODO - disallow queueing of background tasks if this happens */
  gui_text_show(ERROR, "Task queue initialization failed.\n");
  }
else
  {
  sysenv.thread_pool = g_thread_pool_new((GFunc) task_process, NULL,
                                         sysenv.max_threads,
                                         FALSE, NULL);
  }
#endif
}

/*****************************/
/* terminate the thread pool */
/*****************************/
void task_queue_free(void)
{
g_thread_pool_free(sysenv.thread_pool, TRUE, FALSE);
}

/*************************/
/* free a task structure */
/*************************/
void task_free(gpointer data)
{
struct task_pak *task = data;

g_assert(task != NULL);

g_free(task->label);
g_free(task->time);
g_free(task->message);
g_free(task->status_file);

/* NEW */
if (task->timer)
  g_timer_destroy(task->timer);

if (task->status_fp)
  fclose(task->status_fp);

g_string_free(task->status_text, TRUE);

g_free(task);
}

/****************************/
/* submit a background task */
/****************************/
/* TODO - only show certain tasks in the manager, since this */
/* could be used to do any tasks in the background - some of */
/* which may be slow GUI tasks we dont want to be cancellable */
void task_new(const gchar *label,
              gpointer func1, gpointer arg1,
              gpointer func2, gpointer arg2,
              gpointer model)
{
struct task_pak *task;

/* duplicate the task data */
task = g_malloc(sizeof(struct task_pak));
sysenv.task_list = g_slist_prepend(sysenv.task_list, task);
task->pid = -1;
task->status = QUEUED;
task->time = NULL;
task->message = NULL;
task->timer = NULL;
task->pcpu = 0.0;
task->pmem = 0.0;
task->progress = 0.0;
task->locked_model = model;

task->status_file = NULL;
task->status_fp = NULL;
task->status_index = -1;
task->status_text = g_string_new(NULL);

task->label = g_strdup(label);
task->primary = func1;
task->function = NULL;
task->cleanup = func2;
task->ptr1 = arg1;
task->ptr2 = arg2;
/*
if (model)
  ((struct model_pak *) model)->locked = TRUE;
*/

/* queue the task */
g_thread_pool_push(sysenv.thread_pool, task, NULL);
}

/****************************/
/* submit a background task */
/****************************/
// CURRENT - interruptable version of above 
void task_add(const gchar *label,
              gpointer func1, gpointer arg1,
              gpointer func2, gpointer arg2,
              gpointer model)
{
struct task_pak *task;

/* duplicate the task data */
task = g_malloc(sizeof(struct task_pak));
sysenv.task_list = g_slist_prepend(sysenv.task_list, task);
task->pid = -1;
task->status = QUEUED;
task->time = NULL;
task->message = NULL;
task->timer = NULL;
task->pcpu = 0.0;
task->pmem = 0.0;
task->progress = 0.0;
task->locked_model = model;

task->status_file = NULL;
task->status_fp = NULL;
task->status_index = -1;
task->status_text = g_string_new(NULL);

task->label = g_strdup(label);

task->primary = NULL;
task->function = func1;
task->cleanup = func2;
task->ptr1 = arg1;
task->ptr2 = arg2;
/*
if (model)
  ((struct model_pak *) model)->locked = TRUE;
*/

/* queue the task */
g_thread_pool_push(sysenv.thread_pool, task, NULL);
}

/**************************************/
/* platform independant task spawning */
/**************************************/
#define DEBUG_TASK_SYNC 0
gint task_sync(const gchar *command) 
{
gint status;
gchar **argv;
GError *error=NULL;

/* checks */
if (!command)
  return(1);

#if _WIN32
chdir(sysenv.cwd);
system(command);
#else
/* setup the command vector */
argv = g_malloc(4 * sizeof(gchar *));
*(argv) = g_strdup("/bin/sh");
*(argv+1) = g_strdup("-c");
*(argv+2) = g_strdup(command);
*(argv+3) = NULL;
status = g_spawn_sync(sysenv.cwd, argv, NULL, 0, NULL, NULL, NULL, NULL, NULL, &error);
g_strfreev(argv);
#endif

if (!status)
  printf("task_sync() error: %s\n", error->message);

return(status);
}

/**********************************************************/
/* see if there is a running or queued task of given name */
/**********************************************************/
gint task_is_running_or_queued(const gchar *label)
{
gint n;
GSList *item;
struct task_pak *task;

if (!label)
  return(FALSE);

n = strlen(label);
for (item=sysenv.task_list ; item ; item=g_slist_next(item))
  {
  task = item->data;

  if (g_ascii_strncasecmp(label, task->label, n) == 0)
    {
    switch (task->status)
      {
      case RUNNING:
      case QUEUED:
        return(TRUE);
      }
    }
  }
return(FALSE);
}

/***********************************************************/
/* generic fortran style execution ie exe < input > output */
/***********************************************************/
/* all strings should be full path references to files */
#define DEBUG_FORTRAN_EXEC 0
gint task_fortran_exe(const gchar *program, const gchar *input, const gchar *output)
{
gint status=0;
gchar *cmd, *cwd;

/* checks */
if (!program)
  return(-1);
if (!input)
  return(-1);
if (!output)
  return(-1);

/* get cwd from input and change there (so dump files get put there too) */
cwd = g_path_get_dirname(input);

/* put the GULP path in quotes to avoid problems with spaces in pathnames etc */
/* TODO - need to do the same with input/output names? */

#ifdef _WIN32
// FIXME - how do we change the dir to cwd under windows???
// try cmd1 & cmd2 or cmd1 && cmd2
cmd = g_strdup_printf("\"%s\" < %s > %s", program, input, output);
#else
cmd = g_strdup_printf("cd %s ; %s < %s > %s", cwd, program, input, output);
#endif

#if DEBUG_FORTRAN_EXEC
printf("executing: [%s]\n",cmd);
#endif

status = task_sync(cmd);

/* done */
g_free(cwd);
g_free(cmd);

return(status);
}

/********************************************/
/* filter out unwanted lines in status file */
/********************************************/
gint task_status_keep(gint type, const gchar *line)
{
switch (type)
  {
  case GULP:
    if (strstr(line, "CPU"))
      return(1);
    if (strstr(line, " **"))
      return(1);
/*
    if (strstr(line, "="))
      if (strstr(line, "energy"))
        return(1);
*/
    break;

  default:
    return(1);
  }
return(0);
}

/**************************************************/
/* create descriptive string from the status file */
/**************************************************/
void task_status_update(struct task_pak *task)
{
gchar *line;

g_assert(task != NULL);

/* setup any status file filtering */
/*
if (g_ascii_strncasecmp("gulp", task->label, 4) == 0)
  filter = GULP;
else
  filter = 0;
*/

/* read in the status file */
if (task->status_file)
  {
  if (!task->status_fp)
    {
    task->status_index = 0;
/* exit if we've read in the file and closed it (due to completion) */
    if (strlen((task->status_text)->str))
      return;
    task->status_fp = fopen(task->status_file, "rt");
    }

  line = file_read_line(task->status_fp);
  while (line)
    {
/*
    if (task_status_keep(filter, line))
*/
      g_string_append(task->status_text, line);

    g_free(line);
    line = file_read_line(task->status_fp);
    }

  if (task->status == COMPLETED || task->status == KILLED)
    {
    fclose(task->status_fp);
    task->status_fp = NULL;
    }
  }
}

/******************************************************************/
/* parse the list of input files and determine require executable */
/******************************************************************/
const gchar *task_executable_required(const gchar *filename)
{
struct file_pak *file;

if (filename)
  {
  file = file_type_from_filename(filename);
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
      case REAXMD:
        return(sysenv.reaxmd_exe);
      }
    }
  }
return("none");
}

/**************************************************/
/* return true if executable is locally available */
/**************************************************/
// TODO - replace all this with generic hash table
// ie exe name is key , exe path is value
gint task_executable_available(const gchar *filename)
{
struct file_pak *file;

if (filename)
  {
  file = file_type_from_filename(filename);
  if (file)
    {
    switch (file->id)
      {
      case FDF:
        if (sysenv.siesta_path)
          return(TRUE);
        break;
      case GAMESS:
        if (sysenv.gamess_path)
          return(TRUE);
        break;
      case GULP:
        if (sysenv.gulp_path)
          return(TRUE);
        break;
      case REAXMD:
        if (sysenv.reaxmd_path)
          return(TRUE);
        break;
      }
    }
  }

return(FALSE);
}
