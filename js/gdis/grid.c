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
#include <unistd.h>
#include <sys/types.h>

#ifndef __WIN32
#include <sys/wait.h>
#endif

#include "gdis.h"
#include "file.h"
#include "grid.h"
#include "task.h"
#include "parse.h"
#include "numeric.h"
#include "model.h"
#include "import.h"
#include "grisu_client.h"
#include "interface.h"

extern struct sysenv_pak sysenv;

/* CURRENT - register apps for a GRID destination */
/* NULL -> not on grid */
/* TODO - consider storing structs containing more info */
/*        ie site, app details etc - for faster job building */

//GHashTable *grid_table=NULL;
/* CURRENT - assume credential already uploaded for the time being */
gint grid_authenticated=FALSE;
gchar *myproxy_init=NULL;

/*
gchar *grid_shib_idp=NULL;
gchar *grid_shib_user=NULL;
*/

/* struct for connecting an application to a destination */
struct grid_table_pak
{
gint id;
gchar *application;
gchar *queue;
GSList *versions;
};

GSList *grid_table_list=NULL;
GSList *grid_fqan_list=NULL;
GSList *grid_idp_list=NULL;

/********************************/
/* shibboleth config primitives */
/********************************/
void grid_idp_set(const gchar *idp)
{
g_free(sysenv.default_idp);
sysenv.default_idp = g_strdup(idp);
}

void grid_user_set(const gchar *user)
{
g_free(sysenv.default_user);
sysenv.default_user = g_strdup(user);
}

gchar *grid_idp_get(void)
{
return(sysenv.default_idp);
}

gchar *grid_user_get(void)
{
return(sysenv.default_user);
}

/**************************************************************/
/* return the list of VOs we got from the last authentication */
/**************************************************************/
GSList *grid_vo_list(void)
{
return(grid_fqan_list);
}

/******************/
/* free primitive */
/******************/
void grid_table_entry_free(struct grid_table_pak *entry)
{
g_free(entry->application);
g_free(entry->queue);
free_slist(entry->versions);
g_free(entry);
}

/***********************************/
/* record an application and queue */
/***********************************/
/* TODO - disallow repetitions */
#define DEBUG GRID_TABLE_APPEND 0
void grid_table_append(const gchar *application, const gchar *queue, GSList *versions)
{
gint alen, qlen;
GSList *item;
struct grid_table_pak *entry;

g_assert(application != NULL);
g_assert(queue != NULL);

alen = strlen(application);
qlen = strlen(queue);

/* check to see if entry already exists */
for (item=grid_table_list ; item ; item=g_slist_next(item))
  {
  entry = item->data;

  if (alen == strlen(entry->application) && qlen == strlen(entry->queue))
    {
    if (g_ascii_strncasecmp(application, entry->application, alen) == 0)
      if (g_ascii_strncasecmp(queue, entry->queue, qlen) == 0)
        return;
    }
  }

entry = g_malloc(sizeof(struct grid_table_pak));
grid_table_list = g_slist_prepend(grid_table_list, entry);

entry->id = file_id_get(BY_LABEL, application);
/* annoying and not used? */
/*
if (entry->id == -1)
  printf("Warning: application [%s] has unknown id\n", application);
*/

entry->application = g_strdup(application);
entry->queue = g_strdup(queue);

if (versions)
  {
  entry->versions = slist_gchar_dup(versions);
  }
else
  {
  if (grid_authenticated)
    {
    gchar *site = grisu_site_name(queue);

#if DEBUG_GRID_TABLE_APPEND
printf("Seeking version info for [%s] at [%s]\n", application, site);
#endif

    entry->versions = grisu_application_versions(application, site);

    g_free(site);
    }
  else
  entry->versions = NULL;
  }
}

/*******************************/
/* remove an application queue */
/*******************************/
void grid_table_remove(const gchar *application, const gchar *queue)
{
gint alen, qlen;
GSList *item;
struct grid_table_pak *entry;

g_assert(application != NULL);
g_assert(queue != NULL);

alen = strlen(application);
qlen = strlen(queue);

/* check to see if entry already exists */
for (item=grid_table_list ; item ; item=g_slist_next(item))
  {
  entry = item->data;

  if (alen == strlen(entry->application) && qlen == strlen(entry->queue))
    {
    if (g_ascii_strncasecmp(application, entry->application, alen) == 0)
      {
      if (g_ascii_strncasecmp(queue, entry->queue, qlen) == 0)
        {
        grid_table_list = g_slist_remove(grid_table_list, entry); 
        grid_table_entry_free(entry);
        return; 
        }
      }
    }
  }
}

/*****************************************************/
/* completely clear the grid application/queue table */
/*****************************************************/
void grid_table_free(void)
{
GSList *item;

for (item=grid_table_list ; item ; item=g_slist_next(item))
  grid_table_entry_free(item->data);

g_slist_free(grid_table_list);
grid_table_list = NULL;
}

/***********************************************/
/* attempt to get the latest version at a site */
/***********************************************/
#define MAX_VERSION_DIVISIONS 9
#define DEBUG_GRID_VERSION_LATEST 0
gchar *grid_version_latest(const gchar *application, const gchar *site)
{
gint i, buff_digit, best_digit;
gchar **buff, **best, *version=NULL;
GSList *item, *versions;

g_assert(application != NULL);
g_assert(site != NULL);

#if DEBUG_GRID_VERSION_LATEST
printf("scanning for latest version of [%s] at site [%s]\n", application, site);
#endif

versions = grisu_application_versions(application, site);

best = NULL;
for (item=versions ; item ; item=g_slist_next(item))
  {
#if DEBUG_GRID_VERSION_LATEST
printf("  %s\n", (gchar *) item->data);
#endif

/* NB: assumes program versions don't have any more than about 8 .'s */
  buff = g_strsplit(item->data, ".", MAX_VERSION_DIVISIONS);

  if (!best)
    best = buff;
  else
    {
    for (i=0 ; i<MAX_VERSION_DIVISIONS ; i++)
      {
      best_digit = str_to_float(*(best+i));
      buff_digit = str_to_float(*(buff+i));

      if (buff_digit > best_digit)
        {
        g_strfreev(best);
        best = g_strsplit(item->data, ".", MAX_VERSION_DIVISIONS);
        break;
        }
      }
    g_strfreev(buff);
    }
  }

free_slist(versions);

if (best)
  {
  version = g_strjoinv(".", best);
  g_strfreev(best);
  }

#if DEBUG_GRID_VERSION_LATEST
printf("picked: %s\n", version);
#endif

return(version);
}

/********************************/
/* shortcut retrieval primitive */
/********************************/
gpointer grid_table_entry_get(const gchar *application, const gchar *queue)
{
GSList *item;

/* checks */
if (!application || !queue)
  return(NULL);

/* search for exact match */
for (item=grid_table_list ; item ; item=g_slist_next(item))
  {
  struct grid_table_pak *entry = item->data;

  if (g_ascii_strcasecmp(entry->application, application) == 0)
    if (g_ascii_strcasecmp(entry->queue, queue) == 0)
      return(entry);
  }
return(NULL);
}

/********************************/
/* shortcut retrieval primitive */
/********************************/
/* returned list should be freed, but not its elements */
GSList *grid_queue_list(const gchar *application)
{
GSList *item, *list=NULL;

for (item=grid_table_list ; item ; item=g_slist_next(item))
  {
  struct grid_table_pak *entry = item->data;

  if (application)
    {
    if (g_ascii_strcasecmp(entry->application, application) == 0)
      list = g_slist_prepend(list, entry->queue);
    }
  else
    list = g_slist_prepend(list, entry->queue);
  }

return(list);
}

/***********************/
/* retrieval primitive */
/***********************/
const gchar *grid_table_queue_get(gpointer data)
{
struct grid_table_pak *entry = data;

if (entry)
  return(entry->queue);
else
  return(NULL);
}

/***********************/
/* retrieval primitive */
/***********************/
const gchar *grid_table_application_get(gpointer data)
{
struct grid_table_pak *entry = data;

if (entry)
  return(entry->application);
else
  return(NULL);
}

/***********************/
/* retrieval primitive */
/***********************/
GSList *grid_table_versions_get(gpointer data)
{
struct grid_table_pak *entry = data;

if (entry)
  return(entry->versions);
else
  return(NULL);
}

/***********************/
/* retrieval primitive */
/***********************/
GSList *grid_table_list_get(void)
{
return(grid_table_list);
}

/***********************/
/* retrieval primitive */
/***********************/
GSList *grid_search_by_name(const gchar *name)
{
return(grisu_submit_find(name));
}

/*********************************************/
/* allocate and initialize a grid job object */
/*********************************************/
gpointer grid_job_new(void)
{
struct grid_pak *grid;

grid = g_malloc(sizeof(struct grid_pak));

grid->id = -1;
grid->parent = NULL;
grid->user_vo = NULL;
grid->jobname = NULL;
grid->status = NULL;
grid->exename = NULL;
grid->exe_version = NULL;
//grid->jobcode = JOB_UNKNOWN;

grid->remote_q = NULL;
grid->remote_exe = NULL;
grid->remote_exe_module = NULL;
grid->remote_site = NULL;
grid->remote_memory = NULL;
grid->remote_walltime = NULL;

grid->remote_root = NULL;
grid->remote_cwd = NULL;
grid->remote_offset = NULL;

grid->remote_exe_type=-1;
grid->remote_exe_np=1;

grid->report=NULL;
grid->description=NULL;

grid->local_cwd = NULL;
grid->local_input = NULL;
grid->local_output = NULL;

grid->local_upload_list = NULL;
grid->remote_upload_list = NULL;

grid->remote_file_list = NULL;
grid->remote_size_list = NULL;

return(grid);
}

/**************************************/
/* obtain remote file size (in bytes) */
/**************************************/
const gchar *grid_file_size(const gchar *name, gpointer data)
{
GSList *item1, *item2;
struct grid_pak *grid=data;


g_assert(grid != NULL);

item2 = grid->remote_size_list;
for (item1=grid->remote_file_list ; item1 ; item1=g_slist_next(item1))
  {
  if (!item2)
    break;

  if (item1->data && item2->data)
    {
    if (g_ascii_strcasecmp(name, item1->data) == 0)
      {
      return(item2->data); 
      }
    }
  item2 = g_slist_next(item2);
  }
return("?");
}

/* new grisu method for doing stuff ... */
void grid_list_hack(gpointer data)
{
gchar *tmp;
struct grid_pak *grid=data;

g_assert(grid != NULL);

printf("Hacking your grisu file listing ...\n");

tmp =  grisu_job_property_get(grid->jobname, "JobDirectory");

printf("job dir: %s\n", tmp);

g_free(tmp);
}

/****************************************************/
/* primitive designed for background download tasks */
/****************************************************/
void grid_file_list(gpointer data)
{
GSList *list[2];
struct grid_pak *grid=data;

g_assert(grid != NULL);

/*
grid_list_hack(data);
return;
*/

if (!grid->remote_cwd)
  grid->remote_cwd = grisu_job_cwd(grid->jobname);

if (grid->remote_cwd)
  {
/* init file and size lists */
  list[0] = NULL;
  list[1] = NULL;

/* pick gridftp or grisu method */
  switch (sysenv.grid_use_gridftp)
    {
    case TRUE:
      if (grid->user_vo)
        {
        grid_local_proxy_init(grid->user_vo);
        if (grid_gsiftp_list(list, grid->remote_cwd))
          {
          printf("Fast method worked!\n");
          break;
          }
        }
      printf("Falling back to grisu service.\n");

    default:
      grisu_file_size_list(list, grid->remote_cwd);
    }

/* replace lists */
  free_slist(grid->remote_file_list);
  free_slist(grid->remote_size_list);

  grid->remote_file_list = list[0];
  grid->remote_size_list = list[1];

/*
for (item=list[0] ; item ; item=g_slist_next(item))
  {
printf("f [%s]\n", (gchar *) item->data);
  }
for (item=list[1] ; item ; item=g_slist_next(item))
  {
printf("s [%s]\n", (gchar *) item->data);
  }
*/

  }
else
  printf("grid_file_list() error: failed to get working directory for job [%s]\n", grid->jobname);
}

/*******************/
/* copy a grid job */
/*******************/
/* CURRENT - only some variables copied as we use this for job monitoring */
/* where not everything is important (TODO - add them all for consistency) */
gpointer grid_job_dup(gpointer data)
{
struct grid_pak *copy, *source=data;

g_assert(source != NULL);

copy = grid_job_new();

copy->id = source->id;

if (source->jobname)
  copy->jobname = g_strdup(source->jobname);

return(copy);
}

/***********************************/
/* add a local file to be uploaded */
/***********************************/
void grid_upload_add(const gchar *filename, gpointer data)
{
struct grid_pak *grid=data;

g_assert(grid != NULL);

/* TODO - stat for existence? */
if (filename)
  grid->local_upload_list = g_slist_prepend(grid->local_upload_list, g_strdup(filename));
}

/***************************************/
/* add a remote file to be transferred */
/***************************************/
void grid_remote_add(const gchar *filename, gpointer data)
{
struct grid_pak *grid=data;

g_assert(grid != NULL);

/* TODO - stat for existence? */
if (filename)
  grid->remote_upload_list = g_slist_prepend(grid->remote_upload_list, g_strdup(filename));
}

/************************************/
/* set the VO for a grid job object */
/************************************/
void grid_vo_set(const gchar *vo, gpointer data)
{
struct grid_pak *grid=data;

g_assert(grid != NULL);

g_free(grid->user_vo);

grid->user_vo = g_strdup(vo);
}

/************************************/
/* get the VO for a grid job object */
/************************************/
const gchar *grid_vo_get(gpointer data)
{
struct grid_pak *grid=data;

g_assert(grid != NULL);

return(grid->user_vo);
}

/**************************/
/* local/remote type test */
/**************************/
gint grid_job_is_local(gpointer data)
{
struct grid_pak *grid=data;

if (grid)
  if (g_strrstr(grid->remote_q, "local"))
    return(TRUE);

return(FALSE);
}

/**************************/
/* free a grid job object */
/**************************/
void grid_job_free(gpointer data)
{
struct grid_pak *grid=data;

g_assert(grid != NULL);

g_free(grid->user_vo);
g_free(grid->jobname);
g_free(grid->status);
g_free(grid->exename);
g_free(grid->exe_version);

g_free(grid->remote_q);
g_free(grid->remote_root);
g_free(grid->remote_cwd);
g_free(grid->remote_offset);
g_free(grid->remote_exe);
g_free(grid->remote_exe_module);
g_free(grid->remote_site);
g_free(grid->remote_memory);
g_free(grid->remote_walltime);

g_free(grid->report);
g_free(grid->description);

g_free(grid->local_cwd);
g_free(grid->local_input);
g_free(grid->local_output);

free_slist(grid->local_upload_list);
free_slist(grid->remote_upload_list);

free_slist(grid->remote_file_list);
free_slist(grid->remote_size_list);

g_free(grid);
}

/************************************/
/* general grid authenticate method */
/************************************/
void grid_authenticate(gint method)
{
gchar *username=NULL, *password=NULL;
gchar buff[999];

switch (method)
  {
  case GRID_MYPROXY:
    printf("Enter your MYPROXY credential username: ");
    if (fgets(buff, sizeof(buff), stdin) != NULL)
      {
      username = parse_strip_newline(buff);
      }
    else
      {
      printf("Input error.\n");
      return;
      }

/* for cases where user has specified a username already */
/* NB: username should be NULL to avoid overwriting existing value */
  case GRID_MYPROXY_PASSWORD:
    printf("Enter your MYPROXY credential password: ");
    password = parse_getline_hidden();
    grisu_keypair_set(username, password);
    break;


  default:
    printf("Unsupported authentication mechanism.\n");
  }
}

/***************************************/
/* get the DN from an x509 certificate */
/***************************************/
// openssl x509 -noout -in usercert.pem -subject
gchar *grid_get_DN(gchar *certificate_fullpath)
{
gint i, j, status;
gchar *cmd, *out, *err, *subject=NULL;
GError *error=NULL;

cmd = g_strdup_printf("openssl x509 -noout -in %s -subject", certificate_fullpath);

/* TODO - get the output */
if (g_spawn_command_line_sync(cmd, &out, &err, &status, &error))
  {
  for (i=0 ; i<8000 ; i++)
    {
    if (out[i] == '/')
      {
/* not sure why but plain g_strdup adds a crappy char (null?) */
/* to the string, so scan for the end and use strndup instead */
      for (j=i ; j<9000 ; j++)
        {
        if (out[j] == '\0')
          break;
        }
      subject = g_strndup(&out[i], j-i-1);
      break;
      }
    }
  }
g_free(cmd);

return(subject);
}

gboolean grid_credential_get()
{
return(FALSE);
}

/*******************************************************/
/* set valid auth in case of external means (eg) grisu */
/*******************************************************/
/* TODO - could chuck into sysenv eventually */
void grid_auth_set(gint value)
{
grid_authenticated=value;
}
gint grid_auth_get(void)
{
return(grid_authenticated);
}

/*********************************************/
/* verify current authentication information */
/*********************************************/
gint grid_auth_check(void)
{
/*
printf("auth check\n");
printf("credentials [%s : %s]\n", grisu_username_get(), grisu_password_get());
*/

/* retrieve VOs for user */
grisu_auth_set(TRUE);
free_slist(grid_fqan_list);
grid_fqan_list = grisu_fqans_get();

/* FIXME - this will report not authenticated even if successfull, but */
/* user is not a member of any VO which is sort of correct, but misleading */
if (!grid_fqan_list)
  grid_authenticated = FALSE;
else
  grid_authenticated = TRUE;

return(grid_authenticated);
}

/*********************************/
/* poll myproxy for a credential */
/*********************************/
/* quick and dirty polling to check if the credential is up and available */
/* used after a fresh proxy has been created (either cert or shib based) to see */
/* if it's "stuck" to the server yet */
#define DEBUG_POLL_MYPROXY 0
gint grid_poll_myproxy(gint count)
{
gint i, status=FALSE;

/* give the uploaded proxy some time to "stick" to the server */
#if DEBUG_POLL_MYPROXY
printf("Polling Credential, please wait ");
#endif

/* properly register a new uploaded credential */
for (i=0 ; i<count ; i++)
  {
  status = grid_auth_check();

  if (status)
    break;

#if DEBUG_POLL_MYPROXY
  else
    {
    printf(".");
    fflush(stdout);
    }
#endif

#ifndef __WIN32
// unix - units - seconds
sleep(6);
#else
// win32 - units - milliseconds
_sleep(6000);
#endif
  }

#if DEBUG_POLL_MYPROXY
printf("done.\n");
#endif

return(status);
}

/******************************************************/
/* use the swiss proxy knife tool to upload a myproxy */
/******************************************************/
/* NB: this should not create a new proxy, but reuse a local one */
// CURRENT - temp workaround until python myproxy upload works
#define DEBUG_MYPROXY_PUT 1
const gchar *grid_myproxy_put(const gchar *grid_password)
{
gint inp, out, err;
gchar **argv, **envp;
gchar *line, *java, *myproxy, *bouncy;
const gchar *username, *password, *lifetime, *server, *port;
GError *error=NULL;
GPid pid;
FILE *fpi, *fpo, *fpe;

if (grid_authenticated)
  return(NULL);
if (!grid_password)
  return("No password");

//printf("gdis exe path: %s\n", sysenv.gdis_path);

/* append to same directory gdis was in */
myproxy = g_build_filename(sysenv.gdis_path, "swiss-proxy-knife.jar", NULL);
bouncy = g_build_filename(sysenv.gdis_path, "bcprov-jdk15-140.jar", NULL);

/* might need to check java version */
java = g_find_program_in_path("java");
if (!java)
  {
  return("Missing prequisite: JAVA runtime environment\n");
  }
if (!g_file_test(myproxy, G_FILE_TEST_EXISTS))
  {
  return("Missing myproxy tool.\n");
  }
if (!g_file_test(bouncy, G_FILE_TEST_EXISTS))
  {
  return("Missing bouncy castle library");
  }

/* get myproxy username/password to upload */
username = grisu_username_get();
password = grisu_password_get();

/* lifetime (hours) */
//lifetime = "168";
lifetime = "48";

/* user created (advanced) */

/* 1 possibility - NULL */
if (!username || !password)
  {
//printf("[0] Setting random MYPROXY username/password...\n");
  grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
  username = grisu_username_get();
  password = grisu_password_get();
/* short lifetime for temp/random credential */
  lifetime = "24";
  }
/* other possibility - just empty strings */
if (!strlen(username) || !strlen(password))
  {
//printf("[1] Setting random MYPROXY username/password...\n");
  grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
  username = grisu_username_get();
  password = grisu_password_get();
/* short lifetime for temp/random credential */
  lifetime = "24";
  }

server = grisu_myproxy_get();
port = grisu_myproxyport_get();

/*
printf("Uploading credential: "); 
printf("%s : %s -> %s : %s\n", username, password, server, port);
*/

/* FIXME - authentication issues with myproxy/myproxy2 */
/* solution is to point at myproxy2 and set */
/* MYPROXY_SERVER_DN /C=AU/O=APACGrid/OU=VPAC/CN=myproxy2.arcs.org.au */

/* FIXME - need to ensure enviroment has GT_PROXY_MODE set to "old" */
/* unless markus fixes grisu's credential retrieval */
/* build argument vector */

/* FIXME - implement this */

envp = g_malloc(3 * sizeof(gchar *));
*(envp) = g_strdup("GT_PROXYMODE=old");
*(envp+1) = NULL;

/* no longer necessary? */
/*
*(envp+1) = g_strdup("MYPROXY_SERVER_DN=/C=AU/O=APACGrid/OU=VPAC/CN=myproxy2.arcs.org.au");
*(envp+2) = NULL;
*/

argv = g_malloc(14 * sizeof(gchar *));
/* NB: need full path to executable */
*(argv) = g_strdup(java);
*(argv+1) = g_strdup("-jar");
*(argv+2) = g_strdup(myproxy);

*(argv+3) = g_strdup("-l");
*(argv+4) = g_strdup(lifetime);

*(argv+5) = g_strdup("-m");
*(argv+6) = g_strdup("load-local-proxy");

*(argv+7) = g_strdup("-a");
*(argv+8) = g_strdup("myproxy-upload");

*(argv+9) = g_strdup("-d");
*(argv+10) = g_strdup(username);

*(argv+11) = g_strdup("-s");
*(argv+12) = g_strdup(server);
*(argv+13) = NULL;

/*
for (i=0 ; i<11 ; i++)
  {
  printf("%s ", argv[i]);
  }
printf("\n");
*/

/* spawn process */
if (g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
                             NULL, NULL, &pid, &inp, &out, &err, &error))
  {
  if (pid)
    {
/* parent - wait until child is done */
//printf("parent: waiting for child...\n");

fpi = fdopen(inp, "a");
fpo = fdopen(out, "r");
fpe = fdopen(err, "r");

/* NB: -S will make myproxy prompt for grid passphrase */
/* and then the credential password once only */
/* CURRENT - not sure why this isnt working - it seems */
/* to work if run manually from the command line */

fprintf(fpi, "%s\n", grid_password);
fflush(fpi);

/* CURRENT - this seems to fix the problem! */
/* obviously a delay is needed while myproxy-init processes the */
/* grid_password to generate a local proxy */
/* the problem is - how to we poll stdout to know when to send the */
/* passphrase ... which is probably better than an arbitrary sleep */

/*
#ifndef __WIN32
sleep(5);
#endif
*/

/* test if process has terminated */
/* note that if it has - child is reaped and we cant call waitpid() again */

/* CURRENT - can get rid of this with the swiss proxy tool? */
/*
#ifndef __WIN32
if (waitpid(pid, NULL, WNOHANG) > 0)
  {
printf("Bad password.\n");

    g_spawn_close_pid(pid);
    grid_authenticated = FALSE;

  return;
  }
#endif
*/

//printf("waitpid 4: %d\n", waitpid(pid, NULL, WEXITED | WNOHANG));

fprintf(fpi, "%s\n", password);
fflush(fpi);
fclose(fpi);

do
  {
  line = file_read_line(fpo);
#if DEBUG_MYPROXY_PUT
  printf("FORK, stdout: %s\n", line);
#endif

/* check for bad password - hopefully Markus doesn't change this */
  if (line)
    {
    if (g_strrstr(line, "error"))
      {
      g_spawn_close_pid(pid);
      grid_authenticated = FALSE;
      g_free(line);
      return("Incorrect password, please try again.");
      }
    }

  g_free(line);
  }
while(line != NULL);

do
  {
  line = file_read_line(fpe);
#if DEBUG_MYPROXY_PUT
  printf("FORK, stderr : %s\n", line);
#endif
  g_free(line);
  }
while(line != NULL);

/* cleanup */
fclose(fpo);
fclose(fpe);

close(inp);
close(out);
close(err);

#ifndef __WIN32
    waitpid(pid, NULL, WEXITED);
#endif

    g_spawn_close_pid(pid);

//    grid_authenticated = TRUE;

/* CURRENT - this seems to be the  only way to deal with a slow */
/* myproxy server which can sometimes take almost a minute to */
grid_poll_myproxy(10);


    }
  }
else
  {
/* problem with myproxy server? */
  return("Failed, bad network connection?");
  }

g_strfreev(argv);
g_strfreev(envp);

return(NULL);
}

/**************************/
/* java - get a slcs cert */
/**************************/
/* NB: to make this work, the SLCSclient.java code had to be modified
to use the auth password to store the priv key - instead of popping up
a GUI box for a priv key passphrase. In cert request, comment out:
		if (config.getCredentialStore() != null) {
			(new PasswdBox(this)).createAndShowGUI();
		}
*/
#define DEBUG_JAVA_SLCS 0
gint grid_java_slcs_login(const gchar *idp, const gchar *username, const gchar *password)
{
gint inp, out, err;
gchar **argv, **envp;
gchar *line, *java, *jproxy, *path;
GError *error=NULL;
GPid pid;
FILE *fpo, *fpe;

#if DEBUG_JAVA_SLCS
printf("c - Retrieving SLCS certificate...\n");
#endif

/* append to same directory gdis was in */
jproxy = g_build_filename(sysenv.gdis_path, "lib", "init.jar", NULL);

/* might need to check java version */
java = g_find_program_in_path("java");
if (!java)
  {
  return(FALSE);
  }
if (!g_file_test(jproxy, G_FILE_TEST_EXISTS))
  {
  return(FALSE);
  }

/* get location where the slcs cert/key should be placed */
path = g_path_get_dirname(grid_cert_file());

/* FIXME - need to ensure enviroment has GT_PROXY_MODE set to "old" */
/* unless markus fixes grisu's credential retrieval */
/* build argument vector */
envp = g_malloc(6 * sizeof(gchar *));
*(envp) = g_strdup("GT_PROXYMODE=old");
// slcs key/cert location
// cert location (myproxy server auth chain)
*(envp+1) = g_strdup_printf("X509_USER_CERT=%s", grid_cert_file());
*(envp+2) = g_strdup_printf("X509_USER_KEY=%s", grid_key_file());
*(envp+3) = g_strdup_printf("X509_CERT_DIR=%s", path);

// known windows bug (xp home edition)
//http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6256268
#ifdef __WIN32
*(envp+4) = g_strdup_printf("SYSTEMROOT=C:\\windows");
#else
*(envp+4) = NULL;
#endif

*(envp+5) = NULL;

argv = g_malloc(7 * sizeof(gchar *));
/* NB: need full path to executable */
*(argv) = g_strdup(java);
*(argv+1) = g_strdup("-jar");
*(argv+2) = g_strdup(jproxy);
*(argv+3) = g_strdup(idp);
*(argv+4) = g_strdup(username);
*(argv+5) = g_strdup(password);
*(argv+6) = NULL;

/* spawn process */
if (g_spawn_async_with_pipes(NULL, argv, envp, G_SPAWN_DO_NOT_REAP_CHILD,
                             NULL, NULL, &pid, &inp, &out, &err, &error))
  {
  if (pid)
    {
/* parent - wait until child is done */
//printf("parent: waiting for child...\n");

fpo = fdopen(out, "r");
fpe = fdopen(err, "r");

do
  {
  line = file_read_line(fpo);

#if DEBUG_JAVA_SLCS
  printf("FORK, stdout: %s\n", line);
#endif

  g_free(line);
  }
while(line != NULL);

do
  {
  line = file_read_line(fpe);
#if DEBUG_JAVA_SLCS
  printf("FORK, stderr : %s\n", line);
#endif
  g_free(line);
  }
while(line != NULL);

/* cleanup */
fclose(fpo);
fclose(fpe);

close(inp);
close(out);
close(err);

#ifndef __WIN32
    waitpid(pid, NULL, WEXITED);
#endif
    g_spawn_close_pid(pid);

    }
  }
else
  {
/* problem with myproxy server? */
  return(FALSE);
  }

g_strfreev(argv);
g_strfreev(envp);

g_free(jproxy);
g_free(path);
g_free(java);

return(TRUE);
}

/****************************************************/
/* use the swiss proxy knife tool to create a proxy */
/****************************************************/
// CURRENT - using this to work around python code only able to create an RFC3820 proxy,
// which doesnt work with the grisu production backend
/* myproxy server ca: */
/* ~/.globus/certificates/1e12d831.* */
#define DEBUG_PROXY_INIT 0
gint grid_java_proxy_upload(const gchar *grid_password)
{
gint inp, out, err;
gchar **argv, **envp;
gchar *line, *java, *jproxy;
const gchar *username, *password, *server;
GError *error=NULL;
GPid pid;
FILE *fpo, *fpe;

#if DEBUG_PROXY_INIT
printf("Creating local legacy proxy...\n");
#endif

if (!grid_password)
  return(FALSE);

/* append to same directory gdis was in */
jproxy = g_build_filename(sysenv.gdis_path, "lib", "init.jar", NULL);

/* might need to check java version */
java = g_find_program_in_path("java");
if (!java)
  {
  return(FALSE);
  }
if (!g_file_test(jproxy, G_FILE_TEST_EXISTS))
  {
  return(FALSE);
  }

/* get myproxy username/password to upload */
username = grisu_username_get();
password = grisu_password_get();

/* 1 possibility - NULL */
if (!username || !password)
  {
//printf("[0] Setting random MYPROXY username/password...\n");
  grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
  username = grisu_username_get();
  password = grisu_password_get();
  }
/* other possibility - just empty strings */
if (!strlen(username) || !strlen(password))
  {
//printf("[1] Setting random MYPROXY username/password...\n");
  grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
  username = grisu_username_get();
  password = grisu_password_get();
  }

server = grisu_myproxy_get();

#if DEBUG_PROXY_INIT
printf("Uploading credential: "); 
printf("%s : %s -> %s\n", username, password, server);
#endif

/* FIXME - need to ensure enviroment has GT_PROXY_MODE set to "old" */
/* unless markus fixes grisu's credential retrieval */
/* build argument vector */
envp = g_malloc(6 * sizeof(gchar *));
*(envp) = g_strdup("GT_PROXYMODE=old");
// slcs key/cert location
// cert location (myproxy server auth chain)
*(envp+1) = g_strdup_printf("X509_USER_CERT=%s", grid_cert_file());
*(envp+2) = g_strdup_printf("X509_USER_KEY=%s", grid_key_file());
*(envp+3) = g_strdup_printf("X509_CERT_DIR=%s", grid_ca_directory());
// known windows bug (xp home edition)
//http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6256268
#ifdef __WIN32
*(envp+4) = g_strdup_printf("SYSTEMROOT=C:\\windows");
#else
*(envp+4) = NULL;
#endif
*(envp+5) = NULL;

argv = g_malloc(9 * sizeof(gchar *));
/* NB: need full path to executable */
*(argv) = g_strdup(java);

*(argv+1) = g_strdup("-jar");
*(argv+2) = g_strdup(jproxy);
*(argv+3) = g_strdup(grid_password);
*(argv+4) = g_strdup(username);
*(argv+5) = g_strdup(password);
*(argv+6) = g_strdup(server);
*(argv+7) = NULL;

/*
*(argv+1) = g_strdup("-cp");
*(argv+2) = g_strdup_printf("%s:%s:%s", bouncy, swiss, jproxy);
*(argv+3) = g_strdup("jproxy.init");
*(argv+4) = g_strdup(grid_password);
*(argv+5) = g_strdup(username);
*(argv+6) = g_strdup(password);
*(argv+7) = g_strdup(server);
*(argv+8) = NULL;
*/

/* spawn process */
if (g_spawn_async_with_pipes(NULL, argv, envp, G_SPAWN_DO_NOT_REAP_CHILD,
                             NULL, NULL, &pid, &inp, &out, &err, &error))
  {
  if (pid)
    {
/* parent - wait until child is done */
//printf("parent: waiting for child...\n");

fpo = fdopen(out, "r");
fpe = fdopen(err, "r");

do
  {
  line = file_read_line(fpo);

#if DEBUG_PROXY_INIT
  printf("FORK, stdout: %s\n", line);
#endif

  g_free(line);
  }
while(line != NULL);

do
  {
  line = file_read_line(fpe);
#if DEBUG_PROXY_INIT
  printf("FORK, stderr : %s\n", line);
#endif
  g_free(line);
  }
while(line != NULL);

/* cleanup */
fclose(fpo);
fclose(fpe);

close(inp);
close(out);
close(err);

#ifndef __WIN32
    waitpid(pid, NULL, WEXITED);
#endif
    g_spawn_close_pid(pid);

    }
  }
else
  {
/* problem with myproxy server? */
  return(FALSE);
  }

g_strfreev(argv);
g_strfreev(envp);

g_free(jproxy);
g_free(java);

grid_auth_check();

return(TRUE);
}

/******************************************************/
/* use the swiss proxy knife tool to upload a myproxy */
/******************************************************/
#define DEBUG_MYPROXY_INIT 0
const gchar *grid_myproxy_init(const gchar *grid_password)
{
gint inp, out, err;
gchar **argv, **envp;
gchar *line, *java, *myproxy, *bouncy;
const gchar *username, *password, *lifetime, *server, *port;
GError *error=NULL;
GPid pid;
FILE *fpi, *fpo, *fpe;

if (grid_authenticated)
  return(NULL);
if (!grid_password)
  return("No password");

//printf("gdis exe path: %s\n", sysenv.gdis_path);

/* append to same directory gdis was in */
myproxy = g_build_filename(sysenv.gdis_path, "swiss-proxy-knife.jar", NULL);
bouncy = g_build_filename(sysenv.gdis_path, "bcprov-jdk15-140.jar", NULL);

/* might need to check java version */
java = g_find_program_in_path("java");
if (!java)
  {
  return("Missing prequisite: JAVA runtime environment\n");
  }
if (!g_file_test(myproxy, G_FILE_TEST_EXISTS))
  {
  return("Missing myproxy tool.\n");
  }
if (!g_file_test(bouncy, G_FILE_TEST_EXISTS))
  {
  return("Missing bouncy castle library");
  }

/* get myproxy username/password to upload */
username = grisu_username_get();
password = grisu_password_get();
/* lifetime (hours) */
lifetime = "168";
//lifetime = "48";

/* user created (advanced) */

/* 1 possibility - NULL */
if (!username || !password)
  {
//printf("[0] Setting random MYPROXY username/password...\n");
  grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
  username = grisu_username_get();
  password = grisu_password_get();
/* short lifetime for temp/random credential */
  lifetime = "24";
  }
/* other possibility - just empty strings */
if (!strlen(username) || !strlen(password))
  {
//printf("[1] Setting random MYPROXY username/password...\n");
  grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
  username = grisu_username_get();
  password = grisu_password_get();
/* short lifetime for temp/random credential */
  lifetime = "24";
  }

server = grisu_myproxy_get();
port = grisu_myproxyport_get();

printf("Uploading credential: "); 
printf("%s : %s -> %s : %s\n", username, password, server, port);

/* FIXME - authentication issues with myproxy/myproxy2 */
/* solution is to point at myproxy2 and set */
/* MYPROXY_SERVER_DN /C=AU/O=APACGrid/OU=VPAC/CN=myproxy2.arcs.org.au */

/* FIXME - need to ensure enviroment has GT_PROXY_MODE set to "old" */
/* unless markus fixes grisu's credential retrieval */
/* build argument vector */

/* FIXME - need to ensure enviroment has GT_PROXY_MODE set to "old" */
/* unless markus fixes grisu's credential retrieval */
/* build argument vector */
envp = g_malloc(5 * sizeof(gchar *));
*(envp) = g_strdup("GT_PROXYMODE=old");
*(envp+1) = g_strdup_printf("X509_USER_CERT=%s", grid_cert_file());
*(envp+2) = g_strdup_printf("X509_USER_KEY=%s", grid_key_file());
*(envp+3) = g_strdup_printf("X509_CERT_DIR=%s", grid_ca_directory());
*(envp+4) = NULL;

/* no longer necessary? */
/*
*(envp+1) = g_strdup("MYPROXY_SERVER_DN=/C=AU/O=APACGrid/OU=VPAC/CN=myproxy2.arcs.org.au");
*(envp+2) = NULL;
*/

argv = g_malloc(14 * sizeof(gchar *));
/* NB: need full path to executable */
*(argv) = g_strdup(java);
*(argv+1) = g_strdup("-jar");
*(argv+2) = g_strdup(myproxy);

*(argv+3) = g_strdup("-l");
*(argv+4) = g_strdup(lifetime);

*(argv+5) = g_strdup("-m");
*(argv+6) = g_strdup("grid-proxy-init");

*(argv+7) = g_strdup("-a");
*(argv+8) = g_strdup("myproxy-upload");

*(argv+9) = g_strdup("-d");
*(argv+10) = g_strdup(username);

*(argv+11) = g_strdup("-s");
*(argv+12) = g_strdup(server);
*(argv+13) = NULL;

/*
for (i=0 ; i<11 ; i++)
  {
  printf("%s ", argv[i]);
  }
printf("\n");
*/

/* spawn process */
if (g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
                             NULL, NULL, &pid, &inp, &out, &err, &error))
  {
  if (pid)
    {
/* parent - wait until child is done */
//printf("parent: waiting for child...\n");

fpi = fdopen(inp, "a");
fpo = fdopen(out, "r");
fpe = fdopen(err, "r");

/* NB: -S will make myproxy prompt for grid passphrase */
/* and then the credential password once only */
/* CURRENT - not sure why this isnt working - it seems */
/* to work if run manually from the command line */

fprintf(fpi, "%s\n", grid_password);
fflush(fpi);

/* CURRENT - this seems to fix the problem! */
/* obviously a delay is needed while myproxy-init processes the */
/* grid_password to generate a local proxy */
/* the problem is - how to we poll stdout to know when to send the */
/* passphrase ... which is probably better than an arbitrary sleep */

/*
#ifndef __WIN32
sleep(5);
#endif
*/

/* test if process has terminated */
/* note that if it has - child is reaped and we cant call waitpid() again */

/* CURRENT - can get rid of this with the swiss proxy tool? */
/*
#ifndef __WIN32
if (waitpid(pid, NULL, WNOHANG) > 0)
  {
printf("Bad password.\n");

    g_spawn_close_pid(pid);
    grid_authenticated = FALSE;

  return;
  }
#endif
*/

//printf("waitpid 4: %d\n", waitpid(pid, NULL, WEXITED | WNOHANG));

fprintf(fpi, "%s\n", password);
fflush(fpi);
fclose(fpi);

do
  {
  line = file_read_line(fpo);
#if DEBUG_MYPROXY_INIT
  printf("FORK, stdout: %s\n", line);
#endif

/* check for bad password - hopefully Markus doesn't change this */
  if (line)
    {
    if (g_strrstr(line, "error"))
      {
      g_spawn_close_pid(pid);
      grid_authenticated = FALSE;
      g_free(line);
      return("Incorrect password, please try again.");
      }
    }

  g_free(line);
  }
while(line != NULL);

do
  {
  line = file_read_line(fpe);
#if DEBUG_MYPROXY_INIT
  printf("FORK, stderr : %s\n", line);
#endif
  g_free(line);
  }
while(line != NULL);

/* cleanup */
fclose(fpo);
fclose(fpe);

close(inp);
close(out);
close(err);

#ifndef __WIN32
    waitpid(pid, NULL, WEXITED);
#endif

    g_spawn_close_pid(pid);

//    grid_authenticated = TRUE;

/* CURRENT - this seems to be the  only way to deal with a slow */
/* myproxy server which can sometimes take almost a minute to */
grid_poll_myproxy(10);


    }
  }
else
  {
/* problem with myproxy server? */
  return("Failed, bad network connection?");
  }

g_strfreev(argv);
g_strfreev(envp);

return(NULL);
}

/*********************************/
/* retrieve the current idp list */
/*********************************/
GSList *grid_idp_lookup(void)
{
gint i, status;
gchar *command, *java, *knife;
gchar **buff, *out=NULL, *err=NULL;
GSList *list=NULL;

/* TODO - standard startup init - convert to subroutine */
java = g_find_program_in_path("java");
if (!java)
  {
  printf("Failed to find java, required for Grid interaction.\n");
  return(NULL);
  }
knife = g_build_filename(sysenv.gdis_path, "swiss-proxy-knife.jar", NULL);
if (!g_file_test(knife, G_FILE_TEST_EXISTS))
  {
  printf("Failed to find Grid component: swiss-proxy-knife\n");
  g_free(knife);
  g_free(java);
  return(NULL);
  }

#ifdef __WIN32
{
gchar *tmp = g_shell_quote(knife);
g_free(knife);
knife = tmp;
}
#endif

/* repopulate */
command = g_strdup_printf("java -jar %s -m shib-list", knife);
if (g_spawn_command_line_sync(command, &out, &err, &status, NULL))
  {
/* if err - free? */
//printf("==========================\n");
//printf("%s\n", out);
  buff = g_strsplit(out, "\n", -1);

  if (buff)
    {
    i=0;
    while (*(buff+i))
      {
//printf(">>>%s\n", *(buff+i));

/* FIXME - this needs to be made thread safe */
      if (strlen(*(buff+i)))
        list = g_slist_prepend(list, g_strdup(*(buff+i)));

      i++;
      }
    g_strfreev(buff);
    }
  g_free(err);
  g_free(out);
  }

//printf("==========================\n");

g_free(java);
g_free(knife);
g_free(command);

return(list);
}

/************************/
/* idp return primitive */
/************************/
GSList *grid_idp_list_get(void)
{
static GSList *list=NULL;

if (!list)
  {
  list = grid_idp_lookup();
  }

return(list);
}

/***********************/
/* initialize idp list */
/***********************/
gint grid_idp_setup(gpointer dummy, gpointer task)
{

#ifdef WITH_PYTHON

#else
grid_idp_list_get();
#endif
return(TRUE);
}

/*****************************/
/* get info on current proxy */
/*****************************/
//java -jar swiss-proxy-knife.jar -m load-local-proxy -a info

/******************************************/
/* static values for local proxy lifetime */
/******************************************/
static gint grid_life=0;
GTimer *grid_timer=NULL;

/***********************************************************/
/* set the lifetime the local proxy is currently valid for */
/***********************************************************/
void grid_lifetime_set(gint seconds)
{
grid_life = seconds;

if (grid_timer)
  g_timer_start(grid_timer);
else
  grid_timer = g_timer_new();
}

/****************************************************************/
/* retrieve the lifetime the local proxy is currently valid for */
/****************************************************************/
gint grid_lifetime_get(void)
{
int secs;
gulong ms;

if (!grid_timer)
  return(0);

secs = (gint) g_timer_elapsed(grid_timer, &ms);

return(grid_life - secs);
}

/*********************************************/
/* get lifetime from current proxy (seconds) */
/*********************************************/
void grid_lifetime_refresh(const gchar *text)
{
gint i, time=0;
gchar **buff;

//printf("lifetime refresh:\n%s\n", text);
if (text)
  {
  buff = g_strsplit(text, "\n", -1);
  if (buff)
    {
    i=0;
    while (*(buff+i))
      {
      if (g_strrstr(*(buff+i), "time"))
        {
        time = parse_time_hms(*(buff+i));
        }
      i++;
      }
    g_strfreev(buff);
    }
  }

//printf(">>>time = %d\n", time);

grid_lifetime_set(time);
}

/*************************/
/* store a proxy locally */
/*************************/
//java -jar swiss-proxy-knife.jar -m myproxy-login -a store-local -u markus-startup
/* FIXME - does the same job as grid_local_proxy_init(NULL); */
/* CURRENT - don't bother - only gets a local proxy (based on remote) and the */
/* lifetime seems screwed anyway ie cant be used to tell remote credential lifetime */
void grid_myproxy_logon(void)
{
gint inp, out, err;
gchar **argv;
gchar *line, *java, *myproxy;
const gchar *username, *password;
GError *error=NULL;
GPid pid;
GString *output;
FILE *fpi, *fpo, *fpe;

/* get myproxy username/password to upload */
username = grisu_username_get();
password = grisu_password_get();

if (!username || !password)
  {
  printf("MyProxy username/password not set.\n");
  return;
  }

java = g_find_program_in_path("java");
myproxy = g_build_filename(sysenv.gdis_path, "swiss-proxy-knife.jar", NULL);

argv = g_malloc(10 * sizeof(gchar *));
*(argv) = g_strdup(java);
*(argv+1) = g_strdup("-jar");
*(argv+2) = g_strdup(myproxy);
*(argv+3) = g_strdup("-m");
*(argv+4) = g_strdup("myproxy-login");
*(argv+5) = g_strdup("-a");
//*(argv+6) = g_strdup("store-local");
*(argv+6) = g_strdup("info");
*(argv+7) = g_strdup("-u");
*(argv+8) = g_strdup(username);
*(argv+9) = NULL;

output = g_string_new(NULL);

/* spawn process */
if (g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
                             NULL, NULL, &pid, &inp, &out, &err, &error))
  {
  if (pid)
    {
/* parent - wait until child is done */
//printf("parent: waiting for child...\n");

fpi = fdopen(inp, "a");
fpo = fdopen(out, "r");
fpe = fdopen(err, "r");

/* enter myproxy password */
fprintf(fpi, "%s\n", password);
fflush(fpi);
fclose(fpi);

do
  {
  line = file_read_line(fpo);

if (line)
  g_string_append(output, line);

//  printf("FORK, stdout : %s\n", line);
  g_free(line);
  }
while(line != NULL);
do
  {
  line = file_read_line(fpe);
//  printf("FORK, stderr : %s\n", line);
  g_free(line);
  }
while(line != NULL);

fclose(fpo);
fclose(fpe);

    }
  }

/* init the lifetime counter */
grid_lifetime_refresh(output->str);

g_string_free(output, TRUE);
}

/**************************************************/
/* connect using only a myproxy username/password */
/**************************************************/
/*
void grid_myproxy_logon(void)
{
grid_auth_check();
}
*/

/*****************************************************************/
/* use the swiss proxy knife tool to upload a SLCS based myproxy */
/*****************************************************************/
/* TODO - retrieve the proxy ... and re-use if current one in /tmp is still valid */
#define DEBUG_SHIBBOLETH_INIT 0
void grid_shibboleth_init(const gchar *pass)
{
gint inp, out, err;
gchar **argv;
gchar *line, *java, *knife;
const gchar *username, *password, *lifetime, *server, *port;
GError *error=NULL;
GPid pid;
FILE *fpi, *fpo, *fpe;

#if DEBUG_SHIBBOLETH_INIT
printf("IdP = [%s]\n", grid_shib_idp);
printf("User = [%s]\n", grid_shib_user);
#endif

/* checks */
if (!sysenv.default_idp)
  return;
if (!sysenv.default_user)
  return;
if (!pass)
  return;

/* append to same directory gdis was in */
knife = g_build_filename(sysenv.gdis_path, "swiss-proxy-knife.jar", NULL);

//bouncy = g_build_filename(sysenv.gdis_path, "bcprov-jdk15-140.jar", NULL);
java = g_find_program_in_path("java");
if (!java)
  return;
if (!g_file_test(knife, G_FILE_TEST_EXISTS))
  return;
/*
if (!g_file_test(bouncy, G_FILE_TEST_EXISTS))
  return;
*/

/* get myproxy username/password to upload */
username = grisu_username_get();
password = grisu_password_get();

/* lifetime (hours) - not 100% sure this is being observed */
lifetime = "168";

/* 1 possibility - NULL */
if (!username || !password)
  {
//printf("[0] Setting random MYPROXY username/password...\n");
  grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
  username = grisu_username_get();
  password = grisu_password_get();
  }
/* other possibility - just empty strings */
if (!strlen(username) || !strlen(password))
  {
//printf("[1] Setting random MYPROXY username/password...\n");
  grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
  username = grisu_username_get();
  password = grisu_password_get();
  }

server = grisu_myproxy_get();
port = grisu_myproxyport_get();

#if DEBUG_SHIBBOLETH_INIT
printf("Uploading credential: "); 
printf("%s : %s -> %s : %s\n", username, password, server, port);
#endif

/* FIXME - authentication issues with myproxy/myproxy2 */
/* solution is to point at myproxy2 and set */
/* MYPROXY_SERVER_DN /C=AU/O=APACGrid/OU=VPAC/CN=myproxy2.arcs.org.au */
/* FIXME - need to ensure enviroment has GT_PROXY_MODE set to "old" */
/* unless markus fixes grisu's credential retrieval */
/* build argument vector */
/* FIXME - implement this */

argv = g_malloc(18 * sizeof(gchar *));
/* NB: need full path to executable */
*(argv) = g_strdup(java);
*(argv+1) = g_strdup("-jar");
*(argv+2) = g_strdup(knife);

*(argv+3) = g_strdup("-l");
*(argv+4) = g_strdup(lifetime);

*(argv+5) = g_strdup("-m");
*(argv+6) = g_strdup("shib-proxy-init");

*(argv+7) = g_strdup("-i");
*(argv+8) = g_strdup(sysenv.default_idp);

*(argv+9) = g_strdup("-u");
*(argv+10) = g_strdup(sysenv.default_user);

*(argv+11) = g_strdup("-a");
*(argv+12) = g_strdup("myproxy-upload");

*(argv+13) = g_strdup("-d");
*(argv+14) = g_strdup(username);

*(argv+15) = g_strdup("-s");
*(argv+16) = g_strdup(server);
*(argv+17) = NULL;

/* spawn process */
if (g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
                             NULL, NULL, &pid, &inp, &out, &err, &error))
  {
/* parent - wait until child is done */
//printf("parent: waiting for child...\n");
/* CURRENT - WIN32 version */
/*
        WaitForSingleObject();
        _read();
        _close();
*/

fpi = fdopen(inp, "w");
fpo = fdopen(out, "r");
fpe = fdopen(err, "r");

/* enter institution passphrase */
fprintf(fpi, "%s\n", pass);
fflush(fpi);

/* enter myproxy password */
fprintf(fpi, "%s\n", password);
fflush(fpi);
fclose(fpi);

/* no errors/exceptions in output -> success */
//grid_authenticated = TRUE;

/* NB: it is vital to parse these streams to completion before we do an auth check */
/* so that password prompt/entry gets properly pushed through */
do
  {
  line = file_read_line(fpo);

#if DEBUG_SHIBBOLETH_INIT
  printf("FORK, stdout: %s\n", line);
#endif

  if (line)
    if (g_strrstr(line, "error"))
      grid_authenticated = FALSE;

  g_free(line);
  }
while(line != NULL);
do
  {
  line = file_read_line(fpe);

#if DEBUG_SHIBBOLETH_INIT
  printf("FORK, stderr : %s\n", line);
#endif

/* CURRENT - determine failure if stderr lists exceptions */
  if (line)
    if (g_strrstr(line, "Exception"))
      grid_authenticated = FALSE;

  g_free(line);
  }
while(line != NULL);

fclose(fpo);
fclose(fpe);

close(inp);
close(out);
close(err);

#ifndef __WIN32
    waitpid(pid, NULL, WEXITED);
#else
// TODO - win32 equiv waitfor* ?
#endif

    g_spawn_close_pid(pid);
  }
else
  printf("spawn failed.\n");

/* check for successful auth (also refreshes FQANS) */
//grid_auth_check();
grid_poll_myproxy(3);
}

/****************************************************/
/* upload a temporary credential to a remote server */
/****************************************************/
/* TODO - if grid_passwd is NULL -> use stdin */
#define DEBUG_GRID_CREDENTIAL_INIT 0
void grid_credential_init(const gchar *grid_password)
{
const gchar *username, *password, *server, *port;

/* TODO - informative user error messages */
if (!grid_password)
  return;
if (grid_authenticated)
  return;

username = grisu_username_get();
password = grisu_password_get();
server = grisu_myproxy_get();
port = grisu_myproxyport_get();

#if DEBUG_GRID_CREDENTIAL_INIT
printf("TODO - Credential for: "); 
printf("%s : %s -> %s : %s\n", username, password, server, port);
#endif
}

/**************************/
/* get site name from url */
/**************************/
/* assumes we get protocol://hostname/path */
gchar *grid_host_get(const gchar *url)
{
gchar *hostname=NULL, **buff;

g_assert(url != NULL);

buff = g_strsplit(url, "/", 5);

hostname = g_strdup(*(buff+2));

g_strfreev(buff);

return(hostname);
}

/*******************************************/
/* primitive for extracting directory path */
/*******************************************/
gchar *grid_path_get(const gchar *url)
{
gint i, n;
gchar *path=NULL;

if (!url)
  return(NULL);

n = strlen(url);
i = 0;

/* look for first : (assume of the form protocol://) */
while (i<n)
  {
  if (url[i] == ':')
    {
    i+= 3;
    break;
    }
  i++;
  }

/* skip hostname part (ie goto first /) */
while (i<n)
  {
  if (url[i] == '/')
    {
    path = g_strdup(url+i);
    break;
    }
  i++;
  }

return(path);
}

/**********************************************/
/* bypass grisu and use gridFTP to get a file */
/**********************************************/
gint grid_gsiftp_get(const gchar *remote, const gchar *local)
{
gchar **argv, *java, *myproxy, *bouncy, *gridftp, *host, *path;
GError *error=NULL;

printf("Attempting fast transfer: %s -> %s\n", remote, local);

/* NB: assumes a local VOMS proxy already exists */
/* currently - this is created each time by the "Show Files" call */

/* append to same directory gdis was in */
myproxy = g_build_filename(sysenv.gdis_path, "lib/swiss-proxy-knife.jar", NULL);
bouncy = g_build_filename(sysenv.gdis_path, "lib/bcprov-jdk15-140.jar", NULL);
gridftp = g_build_filename(sysenv.gdis_path, "lib/sggc.jar", NULL);

/* might need to check java version */
java = g_find_program_in_path("java");
if (!java)
  {
  printf("Missing prequisite: JAVA runtime environment.\n");
  return(FALSE);
  }
if (!g_file_test(myproxy, G_FILE_TEST_EXISTS))
  {
  printf("Missing myproxy tool.\n");
  return(FALSE);
  }
if (!g_file_test(bouncy, G_FILE_TEST_EXISTS))
  {
  printf("Missing bouncy castle library.\n");
  return(FALSE);
  }
if (!g_file_test(gridftp, G_FILE_TEST_EXISTS))
  {
  printf("Missing gridFTP client.\n");
  return(FALSE);
  }

host = grid_host_get(remote);
path = grid_path_get(remote);

printf("host [%s]\n", host);
printf("path [%s]\n", path);

/* do the transfer */

/* build command */
argv = g_malloc(11 * sizeof(gchar *));
*(argv) = g_strdup(java);
*(argv+1) = g_strdup("-cp");

/* FIXME - "use semi-colons on windows?" */
*(argv+2) = g_strdup_printf("%s:%s:%s", bouncy, myproxy, gridftp);
*(argv+3) = g_strdup("sggc.AppMain");

*(argv+4) = g_strdup("-s");
*(argv+5) = g_strdup(host);

*(argv+6) = g_strdup("-r");
*(argv+7) = g_strdup(path);

*(argv+8) = g_strdup("-l");
*(argv+9) = g_strdup(local);

*(argv+10) = NULL;

/* spawn process */
if (g_spawn_sync(NULL, argv, NULL, 0, NULL, NULL, NULL, NULL, NULL, &error))
  {
printf("success\n");
  }

g_strfreev(argv);

g_free(myproxy);
g_free(bouncy);
g_free(gridftp);
g_free(host);
g_free(path);

return(TRUE);
}

/****************************************************/
/* bypass grisu and use gridFTP to list a directory */
/****************************************************/
/* NB: requires gridFTP ports be permitted */
/* needs a local VOMS proxy */
/* intended to be a (faster) drop-in replacement for grisu_file_list() */
#define DEBUG_GSIFTP_LIST 1
gint grid_gsiftp_list(GSList **list, const gchar *url)
{
gint inp, out, err, num_tokens;
gchar **argv, **envp, **buff, *java, *myproxy, *bouncy, *gridftp, *line, *host, *path;
GError *error=NULL;
GPid pid;
FILE *fpo, *fpe;

printf("TODO - standard java checks.\n");

list[0] = NULL;
list[1] = NULL;

/* FIXME - might need to extract job name and request vo of that job - since we need voms proxy */

/* append to same directory gdis was in */
myproxy = g_build_filename(sysenv.gdis_path, "lib/swiss-proxy-knife.jar", NULL);
bouncy = g_build_filename(sysenv.gdis_path, "lib/bcprov-jdk15-140.jar", NULL);
gridftp = g_build_filename(sysenv.gdis_path, "lib/sggc.jar", NULL);

/* might need to check java version */
java = g_find_program_in_path("java");
if (!java)
  {
  printf("Missing prequisite: JAVA runtime environment.\n");
  return(FALSE);
  }
if (!g_file_test(myproxy, G_FILE_TEST_EXISTS))
  {
  printf("Missing myproxy tool.\n");
  return(FALSE);
  }
if (!g_file_test(bouncy, G_FILE_TEST_EXISTS))
  {
  printf("Missing bouncy castle library.\n");
  return(FALSE);
  }
if (!g_file_test(gridftp, G_FILE_TEST_EXISTS))
  {
  printf("Missing gridFTP client.\n");
  return(FALSE);
  }

host = grid_host_get(url);
path = grid_path_get(url);

#if DEBUG_GSIFTP_LIST
printf("host [%s]\n", host);
printf("path [%s]\n", path);
#endif

/* do the listing */
/*
 java -cp bcprov-jdk15-140.jar:swiss-proxy-knife.jar:sggc.jar sggc.AppMain -s ngdata.ivec.org -r /store/ng03/grid-nri/C_AU_O_APACGrid_OU_iVEC_CN_Sean_Fleming/
*/

/* build command */
argv = g_malloc(11 * sizeof(gchar *));
*(argv) = g_strdup(java);
*(argv+1) = g_strdup("-cp");

/* FIXME - "use semi-colons on windows?" */
*(argv+2) = g_strdup_printf("%s:%s:%s", bouncy, myproxy, gridftp);
*(argv+3) = g_strdup("sggc.AppMain");

*(argv+4) = g_strdup("-s");
*(argv+5) = g_strdup(host);

// CURENT - df gridftp is on non-standard port
*(argv+6) = g_strdup("-p");

// 2810 or 2811 ? stay tuned ...
*(argv+7) = g_strdup("2811");

*(argv+8) = g_strdup("-r");
*(argv+9) = g_strdup(path);

*(argv+10) = NULL;



envp = g_malloc(5 * sizeof(gchar *));
*(envp) = g_strdup("GT_PROXYMODE=old");
*(envp+1) = g_strdup_printf("X509_USER_CERT=%s", grid_cert_file());
*(envp+2) = g_strdup_printf("X509_USER_KEY=%s", grid_key_file());
*(envp+3) = g_strdup_printf("X509_CERT_DIR=%s", grid_ca_directory());
*(envp+4) = NULL;



/* spawn process */
if (g_spawn_async_with_pipes(NULL, argv, envp, G_SPAWN_DO_NOT_REAP_CHILD,
                             NULL, NULL, &pid, &inp, &out, &err, &error))
  {
  if (pid)
    {
//fpi = fdopen(inp, "a");
fpo = fdopen(out, "r");
fpe = fdopen(err, "r");

/*
fflush(fpi);
fclose(fpi);
*/

do
  {
  line = file_read_line(fpo);

#if DEBUG_GSIFTP_LIST
printf("FORK, stdout: %s\n", line);
#endif

/* check for error */
  if (line)
    {
    buff = tokenize(line, &num_tokens);

    if (num_tokens > 8)
      {
/* omit standard directory items */
      if (g_ascii_strncasecmp(".", *(buff+8), 1) != 0)
        {
/* name */
      list[0] = g_slist_prepend(list[0], g_strdup(*(buff+8)));
/* size */
      list[1] = g_slist_prepend(list[1], g_strdup(*(buff+4)));
        }
      }

    g_strfreev(buff);
    }

  g_free(line);
  }
while(line != NULL);

do
  {
  line = file_read_line(fpe);

#if DEBUG_GSIFTP_LIST
printf("FORK, stderr : %s\n", line);
#endif

/* TODO - if "Failed" or other error message -> return NULL in lists? */
  if (line)
  if (g_strrstr(line, "error") || g_strrstr(line, "ailed"))
    {
    g_spawn_close_pid(pid);
    grid_authenticated = FALSE;
    g_free(line);
    break;
    }

  g_free(line);
  }
while(line != NULL);

/* cleanup */
fclose(fpo);
fclose(fpe);

close(inp);
close(out);
close(err);

#ifndef __WIN32
    waitpid(pid, NULL, WEXITED);
#endif

    g_spawn_close_pid(pid);

    }
  }
else
  {
/* problem with myproxy server? */
  printf("Failed, bad network connection?");
  }

g_strfreev(argv);
g_strfreev(envp);

g_free(myproxy);
g_free(bouncy);
g_free(gridftp);
g_free(host);
g_free(path);


/* FIXME - what to do with the sizes - order won't match */
if (list[0])
  list[0] = g_slist_sort(list[0], (GCompareFunc) g_ascii_strcasecmp);

return(TRUE);
}

/************************************************/
/* retrieve a local proxy (with vo if non-null) */
/************************************************/
void grid_local_proxy_init(const gchar *vo)
{
gint inp, out, err;
gchar **argv, **envp, *java, *myproxy, *bouncy, *line;
const gchar *username, *password, *server, *port;
GError *error=NULL;
GPid pid;
FILE *fpi, *fpo, *fpe;

username = grisu_username_get();
password = grisu_password_get();
server = grisu_myproxy_get();
port = grisu_myproxyport_get();

if (!username || !password || !server || !port)
  {
  printf("Error: missing MyProxy details.\n");
  return;
  }

/* append to same directory gdis was in */
myproxy = g_build_filename(sysenv.gdis_path, "swiss-proxy-knife.jar", NULL);
bouncy = g_build_filename(sysenv.gdis_path, "bcprov-jdk15-140.jar", NULL);

/* might need to check java version */
java = g_find_program_in_path("java");
if (!java)
  {
  printf("Missing prequisite: JAVA runtime environment.\n");
  return;
  }
if (!g_file_test(myproxy, G_FILE_TEST_EXISTS))
  {
  printf("Missing myproxy tool.\n");
  return;
  }
if (!g_file_test(bouncy, G_FILE_TEST_EXISTS))
  {
  printf("Missing bouncy castle library.\n");
  return;
  }

/* use swiss-proxy-knife to create local voms proxy */
/*
java -jar swiss-proxy-knife.jar -a store-local -m myproxy-login -u someguy -s myproxy.arcs.org.au -v /ARCS/NRI
*/

/* build environment */
envp = g_malloc(3 * sizeof(gchar *));
*(envp) = g_strdup("GT_PROXYMODE=old");
/*
*(envp+1) = g_strdup("MYPROXY_SERVER_DN=/C=AU/O=APACGrid/OU=VPAC/CN=myproxy2.arcs.org.au");
*(envp+2) = NULL;
*/
*(envp+1) = NULL;
*(envp+2) = NULL;

/* build command */
argv = g_malloc(14 * sizeof(gchar *));
*(argv) = g_strdup(java);
*(argv+1) = g_strdup("-jar");
*(argv+2) = g_strdup(myproxy);

*(argv+3) = g_strdup("-u");
*(argv+4) = g_strdup(username);

*(argv+5) = g_strdup("-a");
*(argv+6) = g_strdup("store-local");
*(argv+7) = g_strdup("-m");
*(argv+8) = g_strdup("myproxy-login");

*(argv+9) = g_strdup("-s");
*(argv+10) = g_strdup(server);
if (vo)
  {
printf("Attempting to create a voms proxy (%s)\n", vo);
  *(argv+11) = g_strdup("-v");
  *(argv+12) = g_strdup(vo);
  }
else
 {
printf("Attempting to create a local plain proxy\n");
 *(argv+11) = NULL;
 *(argv+12) = NULL;
 }
*(argv+13) = NULL;

/* spawn process */
if (g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
                             NULL, NULL, &pid, &inp, &out, &err, &error))
  {
  if (pid)
    {
fpi = fdopen(inp, "a");
fpo = fdopen(out, "r");
fpe = fdopen(err, "r");

fprintf(fpi, "%s\n", password);
fflush(fpi);
fclose(fpi);

do
  {
  line = file_read_line(fpo);

//printf("FORK, stdout: %s\n", line);

/* check for bad password - hopefully Markus doesn't change this */
  if (line)
    {
    if (g_strrstr(line, "error"))
      {
      g_spawn_close_pid(pid);
      grid_authenticated = FALSE;
      g_free(line);
//printf("Incorrect password, please try again.");
      break;
      }
    }

  g_free(line);
  }
while(line != NULL);

do
  {
  line = file_read_line(fpe);

//printf("FORK, stderr : %s\n", line);

  g_free(line);
  }
while(line != NULL);

/* cleanup */
fclose(fpo);
fclose(fpe);

close(inp);
close(out);
close(err);

#ifndef __WIN32
    waitpid(pid, NULL, WEXITED);
#endif

    g_spawn_close_pid(pid);

    }
  }
else
  {
/* problem with myproxy server? */
  printf("Failed, bad network connection?");
  }

g_strfreev(argv);
g_strfreev(envp);
}

/****************************************************/
/* get short text message describing current status */
/****************************************************/
/* free when finished */
gchar *grid_status_text(void)
{
gint s, m=0, h=0, d=0;
GString *text;

// TODO - incorporate
// grid_pem_hours_left()

text = g_string_new(NULL);

if (grid_authenticated)
  {
  s = grid_lifetime_get();

  d = s / 86400;
  s -= d * 86400;

  h = s / 3600;
  s -= h * 3600;

  m = s / 60;
  s -= m * 60;

g_string_append_printf(text, "Authenticated,");

if (d)
  g_string_append_printf(text, " %d days", d);

if (h)
  g_string_append_printf(text, " %d hours", h);

if (m)
  g_string_append_printf(text, " %d minutes", m);

g_string_append_printf(text, " remaining.");

  }
else
  g_string_append(text, "Not authenticated.");

return(g_string_free(text, FALSE));
}

/***********************************************************/
/* generate a random alphabetical string of specified size */
/***********************************************************/
gchar *grid_random_alpha(gint length)
{
gint i;
GRand *r;
GString *text;

r = g_rand_new();
text = g_string_new(NULL);

for (i=length ; i-- ; )
  {
  g_string_sprintfa(text, "%c", g_rand_int_range(r, 'a', 'z'));
  }

g_rand_free(r);

return(g_string_free(text, FALSE));
}

/************************************************************/
/* generate a random alpha-numeric string of specified size */
/************************************************************/
gchar *grid_random_alphanum(gint length)
{
gint i, decide;
GRand *r;
GString *text;

r = g_rand_new();
text = g_string_new(NULL);

/* NB: some systems require an alphabetical character first */
g_string_sprintfa(text, "%c", g_rand_int_range(r, 'a', 'z'));
i=1;
while (i<length)
  {
  decide = g_rand_int_range(r, 0, 99);
  if (decide < 50)
    g_string_sprintfa(text, "%c", g_rand_int_range(r, 'a', 'z'));
  else
    g_string_sprintfa(text, "%c", g_rand_int_range(r, '0', '9'));
  i++;
  }

g_rand_free(r);

return(g_string_free(text, FALSE));
}

/**********************************/
/* attempt to upload a credential */
/**********************************/
gint grid_connect(gint method)
{
switch (method)
  {
  case GRID_MYPROXY:
    grid_auth_check();
    break;

  case GRID_LOCALPROXY:
/* if successful, send info to grisu client for soap header */
/* CURRENT - myproxy-init fed with grid_passwd */
    grid_credential_init(grisu_password_get());
    break;
  }

grisu_auth_set(TRUE);

return(TRUE);
}

/************************************************************/
/* standard locations for slcs/myproxy authentication chain */
/************************************************************/
gchar *grid_cert=NULL, *grid_key=NULL, *grid_cadir=NULL;

const gchar *grid_cert_file(void)
{
return(grid_cert);
}

const gchar *grid_key_file(void)
{
return(grid_key);
}

const gchar *grid_ca_directory(void)
{
return(grid_cadir);
}

/*****************************************/
/* destroy existing credentials (if any) */
/*****************************************/
#define DEBUG_GRID_DISCONNECT 0
gint grid_disconnect(void)
{
gint status=0;
gchar *path, *fullname;
const gchar *file;
GDir *dir;
GError *error=NULL;

#if DEBUG_GRID_DISCONNECT
printf("Destroying credentials...\n");
printf("cert: %s\n", grid_cert);
printf(" key: %s\n", grid_key);
#endif

/* open slcs cert directory */
path = g_path_get_dirname(grid_cert);
dir = g_dir_open(path, 0, &error);
if (error)
  fprintf(stderr, "grid_disconnect(): %s\n", error->message);

/* remove everything in the directory */
if (dir)
  {
  do
    {
    file = g_dir_read_name(dir);
    if (file)
      {
      fullname = g_build_filename(path, file, NULL);
      g_remove(fullname);
      g_free(fullname);
      }
    }
  while (file);
  }
g_dir_close(dir);
g_free(path);

/*
if (g_remove(grid_key))
  status++;
if (g_remove(grid_cert))
  status++;
*/

// force a new (non-authenticated) myproxy credential
grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));
grid_authenticated = FALSE;

// trash all existing soap/SSL connections
// TODO - more tests on this
grisu_soap_cleanup();

return(status);
}

/*********/
/* setup */
/*********/
#define DEBUG_GRID_INIT 0
void grid_init(void)
{
const gchar *home;

/* certificate path init */
home = g_get_home_dir();
if (home)
  {
  grid_cert = g_build_filename(home, ".globus-slcs", "usercert.pem", NULL);
  grid_key = g_build_filename(home, ".globus-slcs", "userkey.pem", NULL);
  grid_cadir = g_build_filename(sysenv.gdis_path, "lib", "certificates", NULL);

#if DEBUG_GRID_INIT
printf("cert: %s\n", grid_cert);
printf(" key: %s\n", grid_key);
printf("  ca: %s\n", grid_cadir);
#endif
  }

sysenv.grid_use_gridftp = FALSE;
grisu_init();
}

/***********/
/* cleanup */
/***********/
void grid_free(void)
{
grisu_free();

g_free(grid_cert);
g_free(grid_key);
g_free(grid_cadir);
}

/*********************/
/* refresh an import */
/*********************/
void grid_download_import(gpointer import)
{
gchar *file, *local, *remote;

g_assert(import != NULL);

file = import_filename(import);

local = g_build_filename(import_path(import), file, NULL);
remote =  g_build_filename(import_remote_path_get(import), file, NULL);

//printf("grid download: [%s] -> [%s]\n", remote, local);

/*
if (sysenv.grid_use_gridftp)
  {
// FIXME - need to establish a local proxy if there isn't one
  if (grid_gsiftp_get(remote, local))
    {
    printf("Fast transfer worked!\n");
    }
  else
    {
    grisu_file_download(remote, local);
    }
  }
else
  {
  grisu_file_download(remote, local);
  }
*/

grisu_file_download(remote, local);

/* if download is successful add to results list for post-processing */
g_free(local);
g_free(remote);
}

/************************************/
/* print contents of grid structure */
/************************************/
void grid_print(gpointer data)
{
struct grid_pak *grid=data;

printf("grid->local_cwd: %s\n", grid->local_cwd);
printf("grid->local_input: %s\n", grid->local_input);

printf("grid->remote_memory: %s\n", grid->remote_memory);
printf("grid->remote_walltime: %s\n", grid->remote_walltime);
}

