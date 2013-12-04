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

#include <glib.h>
#include <time.h>
#include <sys/stat.h>

#include "gdis.h"
#include "grid.h"
#include "numeric.h"
#include "parse.h"
#include "logging.h"
#include "file.h"
#include "interface.h"
#include "grisu_client.h"

#ifdef WITH_GRISU
#include "soapH.h"
#include "grisuHttpBinding.nsmap"
#endif

#define DEBUG_GRISU_CLIENT 0

extern struct sysenv_pak sysenv;

//const char *ws = "";  // send SOAP xml to stdout (deprec use logging instead)
const char ivec_ws[] = "https://grisu.ivec.org/grisu-ws/services/grisu";
const char vpac_ws[] = "https://ngportal.vpac.org/grisu-ws/services/grisu";
const char prod_ws[] = "https://grisu.vpac.org/grisu-ws/services/grisu";
const char vdev_ws[] = "https://ngportaldev.vpac.org/grisu-ws/services/grisu";
const char markus[] = "http://localhost:8080/grisu-ws/services/grisu";
const char *ws = NULL;

#ifdef WITH_GRISU
struct SOAP_ENV__Header *grisu_soap_header=NULL;
#else
gpointer grisu_soap_header=NULL;
#endif

gboolean grisu_auth = FALSE;
gchar *grisu_username=NULL;
gchar *grisu_password=NULL;
gchar *grisu_vo=NULL;
gchar *grisu_server=NULL;
gchar *grisu_port=NULL;
gchar *grisu_cert_path=NULL;

/***********************************/
/* turn soap logging plugin on/off */
/***********************************/
#define GSOAP_LOGGING_SENT "SENT.log"
#define GSOAP_LOGGING_RECV "RECV.log"
void grisu_soap_logging(gboolean state, gpointer newsoap)
{
#ifdef WITH_GRISU
struct logging_data *logdata;

g_assert(newsoap != NULL);

/* retrieve data struct */
logdata = (struct logging_data *) soap_lookup_plugin(newsoap, logging_id);
if (!logdata)
  {
  if (soap_register_plugin(newsoap, logging))
     soap_print_fault(newsoap, stderr);
  else
    {
/* register plugin if not loaded */
    printf("Loaded gsoap logging plugin...\n");
    logdata = (struct logging_data *) soap_lookup_plugin(newsoap, logging_id);
    }
  }

/* set logging state */
if (logdata)
  {
  if (state)
    {
/* TODO - open and write something to indicate logging on/off switches? */
/* then append rather than overwrite (for clarity) */
    logdata->inbound = stdout; 
    logdata->outbound = stdout;
//    logdata->outbound = fopen(GSOAP_LOGGING_SENT, "wt");
//    logdata->inbound = fopen(GSOAP_LOGGING_RECV, "wt");
    }
  else
    {
    logdata->inbound = NULL; 
    logdata->outbound = NULL;
    }
// process messages 
//  size_t bytes_in = logdata->stat_recv;
//  size_t bytes_out = logdata->stat_sent;
  }
else
  printf("Gsoap logging plugin not found.\n");
#endif
}

/******************************************/
/* soap environment reusability variables */
/******************************************/
GStaticMutex grisu_soap_mutex=G_STATIC_MUTEX_INIT;
GSList *grisu_soap_active=NULL, *grisu_soap_available=NULL;

/******************************/
/* acquire a soap environment */
/******************************/
//struct soap *grisu_soap_new(void)
gpointer grisu_soap_new(void)
{
#ifdef WITH_GRISU
static struct soap *template=NULL;
struct soap *soap=NULL;

g_static_mutex_lock(&grisu_soap_mutex);

if (grisu_soap_available)
  {
/* reuse an available soap environment */
  soap = grisu_soap_available->data;
  grisu_soap_available = g_slist_remove(grisu_soap_available, soap);
  grisu_soap_active = g_slist_prepend(grisu_soap_active, soap);
  }
else
  {
  if (!template)
    {
/* first time standard soap template */
    template = soap_new1(SOAP_ENC_MTOM);
/* NEW - from grisuHttpBinding.nsmap */
    soap_set_namespaces(template, namespaces); 

    if (soap_ssl_client_context(template, SOAP_SSL_SKIP_HOST_CHECK,
        NULL, NULL, NULL, grisu_cert_path, NULL))
      printf("grisu_soap_new(): SSL init failed.\n");
    }
/* create a new soap environment from the template */
  soap = soap_copy(template);
  grisu_soap_active = g_slist_prepend(grisu_soap_active, soap);
  }

/* use the header if auth is turned on */
if (grisu_auth)
  {
/* allocate if necessary */
  if (!grisu_soap_header)
    grisu_soap_header = g_malloc(sizeof(struct SOAP_ENV__Header));

/* fill out the header with current values */
  grisu_soap_header->username = grisu_username;
  grisu_soap_header->password = grisu_password;
  grisu_soap_header->myproxyserver = grisu_server;
  grisu_soap_header->myproxyport = grisu_port;

  soap->header = grisu_soap_header;
  }
else
  soap->header = NULL;

g_static_mutex_unlock(&grisu_soap_mutex);

/* DEBUG */
//printf("*** soap ptr = %p\n", soap);

return(soap);
#else
return(NULL);
#endif
}

/**************************************/
/* release a soap environment pointer */
/**************************************/
void grisu_soap_free(gpointer data)
{
#ifdef WITH_GRISU
struct soap *soap = data;

g_assert(soap != NULL);

g_static_mutex_lock(&grisu_soap_mutex);

/* adjust active/available lists */
grisu_soap_active = g_slist_remove(grisu_soap_active, soap);
grisu_soap_available = g_slist_prepend(grisu_soap_available, soap);

g_static_mutex_unlock(&grisu_soap_mutex);
#endif
}

/*************************************/
/* completely free soap environments */
/*************************************/
void grisu_soap_cleanup(void)
{
#ifdef WITH_GRISU
GSList *list;

g_static_mutex_lock(&grisu_soap_mutex);

for (list=grisu_soap_available ; list ; list=g_slist_next(list))
  {
  soap_destroy(list->data); 
  soap_end(list->data); 
  soap_done(list->data); 
  }
g_slist_free(grisu_soap_available);
grisu_soap_available = NULL;

if (grisu_soap_active)
  {
printf("Warning: skipping cleanup on active soap environments...\n");
  }

g_static_mutex_unlock(&grisu_soap_mutex);
#endif
}

/*************************************/
/* retrieve current credential setup */
/*************************************/
const gchar *grisu_username_get(void)
{
return(grisu_username);
}
const gchar *grisu_password_get(void)
{
return(grisu_password);
}
const gchar *grisu_vo_get(void)
{
return(grisu_vo);
}
const gchar *grisu_myproxy_get(void)
{
return(grisu_server);
}
const gchar *grisu_myproxyport_get(void)
{
return(grisu_port);
}
const gchar *grisu_ws_get(void)
{
return(ws);
}

/****************************************/
/* set the temporary credential keypair */
/****************************************/
void grisu_keypair_set(const gchar *username, const gchar *password)
{
if (username)
  {
  g_free(grisu_username);
  grisu_username = g_strdup(username);
  }
if (password)
  {
  g_free(grisu_password);
  grisu_password = g_strdup(password);
  }
}

/*******************************/
/* user's virtual organization */
/*******************************/
void grisu_vo_set(const gchar *vo)
{
if (vo)
  {
  g_free(grisu_vo);
  grisu_vo = g_strdup(vo);
  }
}

/********************************/
/* the service interface to use */
/********************************/
void grisu_ws_set(const gchar *grisu_ws)
{
if (grisu_ws)
  ws = g_strdup(grisu_ws);
}

/********************************************************/
/* set the myproxy server:port to upload credentials to */
/********************************************************/
void grisu_myproxy_set(const gchar *server, const gchar *port)
{
if (server)
  {
  g_free(grisu_server);
  grisu_server = g_strdup(server);
  }
if (port)
  {
/* TODO - check it's a number? */
  g_free(grisu_port);
  grisu_port = g_strdup(port);
  }
}

/*************************************************************/
/* control if authentication tokens are placed in the header */
/*************************************************************/
// deprecate? should always do this?
void grisu_auth_set(gboolean value)
{
grisu_auth = value;
}

/******************/
/* xml primitives */
/******************/
void grisu_xml_start(GMarkupParseContext *context,
                     const gchar *element_name,
                     const gchar **attribute_names,
                     const gchar **attribute_values,
                     gpointer data,
                     GError **error)
{
gint i;
GSList **list = data;

//printf("grisu_xml() start: [%s]\n", element_name);

/* process the file element */
if (g_ascii_strncasecmp(element_name, "file\0", 5) == 0)
  {
  i=0;
  while (*(attribute_names+i))
    {
    if (g_ascii_strncasecmp(*(attribute_names+i), "name", 4) == 0)
      {
      *list = g_slist_prepend(*list, g_strdup(*(attribute_values+i)));
      }
    if (g_ascii_strncasecmp(*(attribute_names+i), "size", 4) == 0)
      {
/* TODO - do something with the file size (could be useful) */
      }
    i++;
    }
  }
}

void grisu_xml_end(GMarkupParseContext *context,
                   const gchar *element_name,
                   gpointer data,
                   GError **error)
{
//printf("grisu_xml() stop: [%s]\n", element_name);
}

void grisu_xml_text(GMarkupParseContext *context,
                    const gchar *text,
                    gsize text_len,  
                    gpointer data,
                    GError **error)
{
}

/******************/
/* xml primitives */
/******************/
void grisu_xml_fs_start(GMarkupParseContext *context,
                        const gchar *element_name,
                        const gchar **attribute_names,
                        const gchar **attribute_values,
                        gpointer data,
                        GError **error)
{
gint i;
GSList **list = data;

/* process the file element */
if (g_ascii_strncasecmp(element_name, "file\0", 5) == 0)
  {
  i=0;
  while (*(attribute_names+i))
    {
    if (g_ascii_strncasecmp(*(attribute_names+i), "name", 4) == 0)
      {
      *(list+0) = g_slist_prepend(*(list+0), g_strdup(*(attribute_values+i)));
      }
    if (g_ascii_strncasecmp(*(attribute_names+i), "size", 4) == 0)
      {
      *(list+1) = g_slist_prepend(*(list+1), g_strdup(*(attribute_values+i)));
      }
    i++;
    }
  }
}

/******************/
/* xml primitives */
/******************/
/* CURRENT - grisu xml parsing (1) file lists & 2) job description) */
gpointer grisu_xml_parse(const gchar *xml_text)
{
GMarkupParser xml_parser;
GMarkupParseContext *xml_context;
GError *error;
GSList *file_list=NULL;

xml_parser.start_element = &grisu_xml_start;
xml_parser.end_element = &grisu_xml_end;
xml_parser.text = &grisu_xml_text;
xml_parser.passthrough = NULL;
xml_parser.error = NULL;

xml_context = g_markup_parse_context_new(&xml_parser, 0, &file_list, NULL);

if (!g_markup_parse_context_parse(xml_context, xml_text, strlen(xml_text), &error))
    printf("grisu_xml_parse() : parsing error.\n");

/* cleanup */
if (!g_markup_parse_context_end_parse(xml_context, &error))
  printf("grisu_xml_parse() : errors occurred processing xml.\n");

g_markup_parse_context_free(xml_context);

return(file_list);
}

/*********************************************/
/* parse grisu file list for names and sizes */
/*********************************************/
void grisu_xml_parse_files_sizes(GSList **lists, const gchar *xml_text)
{
GMarkupParser xml_parser;
GMarkupParseContext *xml_context;
GError *error;

/* files */
*(lists+0) = NULL;
/* sizes */
*(lists+1) = NULL;

xml_parser.start_element = &grisu_xml_fs_start;
xml_parser.end_element = NULL;
xml_parser.text = NULL;
xml_parser.passthrough = NULL;
xml_parser.error = NULL;

xml_context = g_markup_parse_context_new(&xml_parser, 0, lists, NULL);

if (!g_markup_parse_context_parse(xml_context, xml_text, strlen(xml_text), &error))
  printf("grisu_xml_parse_files_sizes() : parsing error.\n");

/* cleanup */
if (!g_markup_parse_context_end_parse(xml_context, &error))
  printf("grisu_xml_parse_files_sizes() : errors occurred processing xml.\n");

g_markup_parse_context_free(xml_context);
}

/******************************/
/* get file listing and sizes */
/******************************/
void grisu_file_size_list(GSList **list, const gchar *dir)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__ls_USCOREstring put;
struct _ns1__ls_USCOREstringResponse get;

g_assert(dir != NULL);

//printf("grisu_file_size_list(): %s\n", dir);

put.in0 = g_strdup(dir);
put.in1 = 1;
put.in2 = 1;

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__ls_USCOREstring(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  grisu_xml_parse_files_sizes(list, get.out);
  }
else
  {
  printf("grisu_file_list(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

g_free(put.in0);

grisu_soap_free(newsoap);
#endif
}

/************************************************/
/* remote directory listing (url or virtual fs) */
/************************************************/
GSList *grisu_file_list(const gchar *dir)
{
GSList *list=NULL;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__ls_USCOREstring put;
struct _ns1__ls_USCOREstringResponse get;

g_assert(dir != NULL);

//printf("grisu_file_list(): %s\n", dir);

put.in0 = g_strdup(dir);
put.in1 = 1;
put.in2 = 1;

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__ls_USCOREstring(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  list = grisu_xml_parse(get.out);
  }
else
  {
  printf("grisu_file_list(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

g_free(put.in0);

grisu_soap_free(newsoap);
#endif

return(list);
}

/*****************************************/
/* mount a remote filesystem for staging */
/*****************************************/
/* currently unused */
gboolean grisu_mount(const gchar *mount_point, const gchar *url)
{
gint value=FALSE;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__mount put;
struct _ns1__mountResponse get;

//printf("*** GRISU WS - creating mountpoint ***\n");

g_assert(mount_point != NULL);
g_assert(url != NULL);

put.in0 = g_strdup(url);
put.in1 = g_strdup(mount_point);
put.in2 = 1;

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__mount(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  value = TRUE;
  }
else
  {
  printf("grisu_mount(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  value = FALSE;
  }

g_free(put.in0);
g_free(put.in1);

grisu_soap_free(newsoap);
#endif

return(value);
}

/*************************************************************/
/* mount a remote filesystem (under supplied VO) for staging */
/*************************************************************/
/* currently unused */
gboolean grisu_mount_vo(const gchar *mount_point, const gchar *url, const gchar *vo)
{
gint value=FALSE;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__mount1 put;
struct _ns1__mount1Response get;

//printf("*** GRISU WS - creating mountpoint with VO ***\n");

g_assert(mount_point != NULL);
g_assert(url != NULL);
g_assert(vo != NULL);

put.in0 = g_strdup(url);
put.in1 = g_strdup(mount_point);
put.in2 = g_strdup(vo);
put.in3 = 1;

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__mount1(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  value = TRUE;
  }
else
  {
  printf("grisu_mount_vo(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  value = FALSE;
  }

g_free(put.in0);
g_free(put.in1);
g_free(put.in2);

grisu_soap_free(newsoap);
#endif

return(value);
}

/***************************************************/
/* copy a file from one remote location to another */
/***************************************************/
void grisu_file_copy(const gchar *source, const gchar *target)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__cp put;
struct _ns1__cpResponse get;

g_assert(source != NULL);
g_assert(target != NULL);

put.in0 = g_strdup(source);
put.in1 = g_strdup(target);
/* overwrite */
put.in2 = 1;
put.in3 = 1;

grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__cp(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
printf("cp success.\n");
  }
else
  {
  printf("grisu_file_copy(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

grisu_soap_free(newsoap);
#endif
}

/******************************/
/* streaming upload callbacks */
/******************************/
#ifdef WITH_GRISU
void *mime_read_open(struct soap *soap, void *handle, const char *id, const char *type, const char *description)
{
//printf("mime: opening outbound (%s)\n", type);
return handle;
}
void mime_read_close(struct soap *soap, void *handle)
{
//printf("mime: closing outbound\n");
if (handle)
  fclose((FILE*)handle);
}
size_t mime_read(struct soap *soap, void *handle, char *buf, size_t len)
{ 
if (handle)
  return fread(buf, 1, len, (FILE*)handle);

return(0);
}
#endif

/***********************************************/
/* upload a file to the jobs working directory */
/***********************************************/
gint grisu_file_upload(const gchar *fullpath, const gchar *remotedir)
{
gint value=FALSE;
#ifdef WITH_GRISU
struct soap *newsoap;
struct _ns1__upload put;
struct _ns1__uploadResponse get;
struct xsd__base64Binary image;
struct stat sb;
FILE *fd;

g_assert(fullpath != NULL);
g_assert(remotedir != NULL);

fd = fopen(fullpath, "rb");
if (!fd)
  {
  printf("grisu_file_upload() ERROR, could not find: %s\n", fullpath);
  return(FALSE);
  }

/* pre-req */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

/* try to get file length */
if (!fstat(fileno(fd), &sb) && sb.st_size >= 0)
   {
   newsoap->fmimereadopen = mime_read_open;
   newsoap->fmimereadclose = mime_read_close;
   newsoap->fmimeread = mime_read;
   image.__ptr = (unsigned char *) fd;
/* NB: size must be set */
   image.__size = sb.st_size;
   }
 else
   {
   printf("grisu_file_upload() ERROR, couldn't get size of file: %s\n", fullpath);
   return(FALSE);

/* TODO - don't know/can't get the size - buffer it? - deal with this later */
/* see gsoap docs on Streaming Chunked MTOM/MIME maybe? */
/*
printf("unknown size ...\n");
   size_t i;
   int c;
   image.__ptr = (unsigned char*)soap_malloc(&soap, MAX_FILE_SIZE);
   for (i = 0; i < MAX_FILE_SIZE; i++)
      {
         if ((c = fgetc(fd)) == EOF)
            break;
         image.__ptr[i] = c;
      }
   fclose(fd);
   image.__size = i;
*/
   }

/* NB - must set id & options to NULL */
/* or else crap characters can be inserted that screw up transmission */
image.id = NULL;
image.options = NULL;
/* MIME type */
/* FIXME - do we need to use image/xxx for binary uploads? */
image.type = "text/html";

/* source is the MIME/MTOM attachments */
put.in0 = &image;
put.in1 = g_strdup(remotedir);
put.in2 = TRUE;

if (soap_call___ns1__upload(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  value = TRUE;
  }
else
  {
  printf("grisu_file_upload(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  value = FALSE;
  }

g_free(put.in1);

grisu_soap_free(newsoap);
#endif

return(value);
}

/********************************/
/* streaming download callbacks */
/********************************/
#define DEBUG_GRISU_DOWNLOAD 0
#ifdef WITH_GRISU
void *mime_write_open(struct soap *soap,
                      void *unused_handle,
                      const char *id, const char *type, const char *desc,
                      enum soap_mime_encoding encoding)
{
g_assert(soap != NULL);

/* TODO - cope with untested MIME encoding types */
#if DEBUG_GRISU_DOWNLOAD
printf("id [%s]\n", id);
printf("desc [%s]\n", desc);
printf("mime: handle = %p\n", unused_handle);
printf("mime: opening inbound (%s)\n", type);
printf("mime: encoding type -> ");
switch (encoding)
  {
/* known to work */
  case SOAP_MIME_BINARY:
    printf("ok\n");
    break;
/* untested */
  case SOAP_MIME_NONE:
    printf("untested 1\n");
    break;
  case SOAP_MIME_7BIT:
    printf("untested 2\n");
    break;
  case SOAP_MIME_8BIT:
    printf("untested 3\n");
    break;
  case SOAP_MIME_QUOTED_PRINTABLE:
    printf("untested 4\n");
    break;
  case SOAP_MIME_BASE64:
    printf("untested 5\n");
    break;
  case SOAP_MIME_IETF_TOKEN:
    printf("untested 6\n");
    break;
  case SOAP_MIME_X_TOKEN:
    printf("untested 7\n");
    break;
/* unknown */
  default:
    printf("unknown\n");
  }
#endif

/* CURRENT - using a pointer in the soap struct for setting the destination file */

/* the FD to use (gets passed to mime callbacks) */
return(soap->os);
}

/********************************/
/* streaming download callbacks */
/********************************/
void mime_write_close(struct soap *soap, void *handle)
{
#if DEBUG_GRISU_DOWNLOAD
printf("mime: closing inbound\n");
#endif

fclose((FILE*)handle);
}

/********************************/
/* streaming download callbacks */
/********************************/
int mime_write(struct soap *soap, void *handle, const char *buf, size_t len)
{
size_t nwritten;

//printf("mime: writing to %p\n", handle);

while (len)
  {
  nwritten = fwrite(buf, 1, len, (FILE*)handle);
  if (!nwritten)
    {
    soap->errnum = errno;
    return SOAP_EOF;
    }
  len -= nwritten;
  buf += nwritten;
  }
return SOAP_OK;
}
#endif

/***************************************************/
/* download a file from the jobs working directory */
/***************************************************/
#define DEBUG_GRISU_FILE_DOWNLOAD 0
gint grisu_file_download(const gchar *remote_src, const gchar *local_dest)
{
gint value=FALSE;
#ifdef WITH_GRISU
struct soap *newsoap;
struct _ns1__download put;
struct _ns1__downloadResponse get;
FILE *fd;

g_assert(remote_src != NULL);
g_assert(local_dest != NULL);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

/* CURRENT - using a pointer in the current soap struct to pass destination file for writing */
fd = fopen(local_dest, "wb");
if (fd)
  {
/* FIXME - a bit naughty as I don't know what soap.os (output stream?) is for */
#if DEBUG_GRISU_FILE_DOWNLOAD
  if (newsoap->os)
    printf("grisu_file_download() warning, soap.os pointer was not NULL\n");
#endif

//printf("Created file handle: %p\n", fd);

  newsoap->os = (void *) fd;
  }
else
  {
  printf("grisu_file_download() ERROR, couldn't open [%s] for writing\n", local_dest);
  return(FALSE);
  }

newsoap->fmimewriteopen = mime_write_open;
newsoap->fmimewriteclose = mime_write_close;
newsoap->fmimewrite = mime_write; 

/* are these reasonable? */
newsoap->connect_timeout = 10;
newsoap->send_timeout = 30;
newsoap->recv_timeout = 30;

/*
soap.accept_timeout = 3600;
soap.accept_flags = SO_NOSIGPIPE;
soap.socket_flags = MSG_NOSIGNAL;
*/

/* remote source URL */
put.in0 = g_strdup(remote_src);

if (soap_call___ns1__download(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  value = TRUE;
  }
else
  {
  printf("grisu_file_download(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  value = FALSE;
  }

g_free(put.in0);

grisu_soap_free(newsoap);
#endif

return(value);
}

/******************************/
/* sort the list of grid jobs */
/******************************/
gint grisu_jobs_sort(gpointer ptr1, gpointer ptr2)
{
gint diff=0;
const gchar *job1=ptr1, *job2=ptr2;
gchar **buff1, **buff2;

/*
compare_func : 	 the comparison function used to sort the GSList. This function is passed the data from 2 elements of the GSList and should return 0 if they are equal, a negative value if the first element comes before the second, or a positive value if the first element comes after the second.
*/

/* strip the fronst application name off */
buff1 = g_strsplit(job1, "_", 2);
buff2 = g_strsplit(job2, "_", 2);

if (buff1 && buff2)
  diff = time_stamp_compare(*(buff1+1), *(buff2+1));

g_strfreev(buff1);
g_strfreev(buff2);

return(diff);
}

/*********************/
/* get all job names */
/*********************/
/* returned list should be completely freed */
GSList *grisu_job_names(void)
{
GSList *list=NULL;
#ifdef WITH_GRISU
gint i;
gpointer newsoap;
struct _ns1__getAllJobnames put;
struct _ns1__getAllJobnamesResponse get;
struct ns1__ArrayOfString *out; 

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__getAllJobnames(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  out = get.out;
  for (i=out->__sizestring ; i-- ; )
    list = g_slist_prepend(list, g_strdup(out->string[i]));
  }
else
  {
  printf("grisu_job_names(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

grisu_soap_free(newsoap);
#endif

return(list);
}

/**************************/
/* kill and cleanup a job */
/**************************/
void grisu_job_remove(const gchar *name)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__kill put;
struct _ns1__killResponse get;

/* prereq */
g_assert(name != NULL);

grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

put.in0 = g_strdup(name);
/* TODO - true = delete files? if so implement a stop */
/* and a stop + cleanup */
put.in1 = 1;

if (soap_call___ns1__kill(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
//printf("Job deleted.\n");
  }
else
  {
  printf("grisu_job_remove(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

grisu_soap_free(newsoap);

g_free(put.in0);
#endif
}

/************************************/
/* submit a job to the grisu server */
/************************************/
gint grisu_job_submit(const gchar *name, const gchar *vo)
{
gint value=FALSE;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__submitJob put;
struct _ns1__submitJobResponse get;

g_assert(name != NULL);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

/* input */
put.in0 = g_strdup(name);
if (vo)
  put.in1 = g_strdup(vo);
else
  put.in1 = g_strdup("/ARCS/Startup");

/* submit the job */
if (soap_call___ns1__submitJob(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  value = TRUE;
  }
else
  {
  printf("grisu_job_submit(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  value = FALSE;
  }

g_free(put.in0);
g_free(put.in1);

grisu_soap_free(newsoap);
#endif

return(value);
}

/****************************************/
/* get job type from the jobname string */
/****************************************/
/* NB: may be deprecated if a better way of recording the job type is developed */
gint grisu_job_type(const gchar *name)
{
g_assert(name != NULL);

if (g_strncasecmp(name, "gulp", 4) == 0)
  return(JOB_GULP);

if (g_strncasecmp(name, "gamess", 6) == 0)
  return(JOB_GAMESS);

return(JOB_UNKNOWN);
}

/*************************************************/
/* generate executable specific job descriptions */
/*************************************************/
gchar *grisu_xml_exe(gpointer data)
{
gint type;
gchar *input;
GString *xml;
struct grid_pak *grid = data;

g_assert(grid != NULL);

xml = g_string_new(NULL);

//type = grisu_job_type(grid->jobname);
type = file_id_get(BY_EXTENSION, grid->local_input);

switch (type)
  {
  case GAMESS:
    g_string_append_printf(xml, "<Executable>%s</Executable>\n", grid->remote_exe);
/* GAMESS likes to be different - strip off input file's extension */
    input = parse_strip_extension(grid->local_input);
    g_string_append_printf(xml, "<Argument>%s</Argument>\n", input);
    g_free(input);
    g_string_append_printf(xml, "<Argument>%s</Argument>\n", grid->exe_version);
    g_string_append_printf(xml, "<Argument>%d</Argument>\n", grid->remote_exe_np); 
    g_string_append_printf(xml, "<Output>%s</Output>\n", grid->local_output);
    g_string_append_printf(xml, "<Error>stderr.txt</Error>\n");
    break;

  case REAXMD:
    g_string_append_printf(xml, "<Executable>%s</Executable>\n", grid->remote_exe);
/* REAXMD - base filename only -> implies .irx .pdb etc are all present */
    input = parse_strip_extension(grid->local_input);
    g_string_append_printf(xml, "<Argument>%s</Argument>\n", input);
    g_free(input);
    g_string_append_printf(xml, "<Output>stdout.txt</Output>\n");
    g_string_append_printf(xml, "<Error>stderr.txt</Error>\n");
    break;

/* TODO - argument or input/output the more sensible default? */
  default:
  case FDF:
  case GULP:
    g_string_append_printf(xml, "<Executable>%s</Executable>\n", grid->remote_exe);
    g_string_append_printf(xml, "<Input>%s</Input>\n", grid->local_input);
    g_string_append_printf(xml, "<Output>%s</Output>\n", grid->local_output);
    g_string_append_printf(xml, "<Error>stderr.txt</Error>\n");
    break;
  }

if (grid->remote_exe_module)
  g_string_append_printf(xml, "<Module>%s</Module>\n", grid->remote_exe_module);

return(g_string_free(xml, FALSE));
}

/***********************************************/
/* generate a job description string for grisu */
/***********************************************/
/* TODO - more sophisticated XML generator, eg for CPU time etc values */
gchar *grisu_xml_create(gpointer data)
{
GString *xml;
struct grid_pak *grid = data;

g_assert(grid != NULL);
g_assert(grid->jobname != NULL);
g_assert(grid->remote_q != NULL);
g_assert(grid->remote_exe != NULL);
g_assert(grid->remote_root != NULL);

xml = g_string_new(NULL);

/* REFERENCE - http://www.gridforum.org/documents/GFD.56.pdf */

g_string_append_printf(xml, "<JobDefinition xmlns=\"http://schemas.ggf.org/jsdl/2005/11/jsdl\">\n");
g_string_append_printf(xml, "<JobDescription>\n");

/* NEW - not 100% sure if needs to be here (under JobDesc), but playing safe as it works */
/* CURRENT - force mpi/non-mpi explicitly as it is a show stopper if not correct */
/* FIXME - need a better way of determining if remote exe is mpi compiled or not */
if (grid->remote_exe_type == GRID_MPI)
  {
  g_string_append_printf(xml, "<JobType xmlns=\"http://arcs.org.au/jsdl/jsdl-grisu\">mpi</JobType>\n");
  }
else
  {
  g_string_append_printf(xml, "<JobType xmlns=\"http://arcs.org.au/jsdl/jsdl-grisu\">single</JobType>\n");
  }

g_string_append_printf(xml, "<JobIdentification><JobName>%s</JobName></JobIdentification>\n", grid->jobname);
g_string_append_printf(xml, "<Application>\n");
/* FIXME - does this matter? */
//g_string_append_printf(xml, "<ApplicationName>gdis</ApplicationName>\n");
g_string_append_printf(xml, "<ApplicationName>%s</ApplicationName>\n", grid->description);

g_string_append_printf(xml, "<POSIXApplication xmlns=\"http://schemas.ggf.org/jsdl/2005/11/jsdl-posix\">\n");
g_string_append_printf(xml, "<WorkingDirectory filesystemName=\"userExecutionHostFs\">");
//g_string_append_printf(xml, "grisu-jobs-dir/%s</WorkingDirectory>\n", grid->jobname);
g_string_append_printf(xml, "%s</WorkingDirectory>\n", grid->remote_offset);

/* exectuable specific xml */
g_string_append_printf(xml, "%s", grisu_xml_exe(grid));

g_string_append_printf(xml, "</POSIXApplication>\n");

/* this element IS correctly being presented as the number of cpus */
//g_string_append_printf(xml, "<TotalCPUCount>%d</TotalCPUCount>\n", grid->remote_exe_np);
g_string_append_printf(xml, "<TotalCPUCount><exact>%d</exact></TotalCPUCount>\n", grid->remote_exe_np);

/* this is in seconds and gets divided by # cpus before ending in PBS script */
if (grid->remote_walltime)
  g_string_append_printf(xml, "<TotalCPUTime>%s</TotalCPUTime>\n", grid->remote_walltime);


/* NEW - fixed - add lowerboundedrange element */
/* NB: correctly report failure if this is too large ... eg a thousand GB */
/* NB: units are MB */
if (grid->remote_memory)
  {
  g_string_append_printf(xml, "<TotalPhysicalMemory><LowerBoundedRange>%s", grid->remote_memory);
  g_string_append_printf(xml, "</LowerBoundedRange></TotalPhysicalMemory>\n");
  }

g_string_append_printf(xml, "</Application>\n");

g_string_append_printf(xml, "<Resources>\n");
g_string_append_printf(xml, "<CandidateHosts><HostName>%s</HostName></CandidateHosts>\n", grid->remote_q);
g_string_append_printf(xml, "<FileSystem name=\"userExecutionHostFs\">\n");
g_string_append_printf(xml, "<MountSource>%s</MountSource>\n", grid->remote_root);

/* CURRENT - jobfs request? (in kb?) */
/*
g_string_append_printf(xml, "<DiskSpace><LowerBoundedRange>300000");
g_string_append_printf(xml, "</LowerBoundedRange></DiskSpace>\n");
*/

g_string_append_printf(xml, "<FileSystemType>normal</FileSystemType>\n");
g_string_append_printf(xml, "</FileSystem></Resources></JobDescription></JobDefinition>\n");

return(g_string_free(xml, FALSE));
}

/*************************************/
/* get a valid grisu job name to use */
/*************************************/
gint grisu_jobname_request(gchar *jobname)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__createJob put;
struct _ns1__createJobResponse get;

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

put.in0 = jobname;
put.in1 = 0;

if (!soap_call___ns1__createJob(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
/* TODO - delete the job and start again? */
  soap_print_fault(newsoap, stderr);
  return(FALSE);
  }

grisu_soap_free(newsoap);
#endif

return(TRUE);
}

/*******************************************/
/* attach a job description to a grisu job */
/*******************************************/
gint grisu_job_configure(gchar *job_name, gchar *job_xml)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__setJobDescription_USCOREstring put;
struct _ns1__setJobDescription_USCOREstringResponse get;

g_assert(job_name != NULL);
g_assert(job_xml != NULL);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

/* args */
put.in0 = job_name;
put.in1 = job_xml;

if (!soap_call___ns1__setJobDescription_USCOREstring(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  printf("grisu_job_configure(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

grisu_soap_free(newsoap);
#endif

return(FALSE);
}

/**********************/
/* get job properties */
/**********************/
gchar *grisu_job_property_get(const gchar *jobname, const gchar *key)
{
gchar *value=NULL;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__getJobProperty put;
struct _ns1__getJobPropertyResponse get;

grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

put.in0 = g_strdup(jobname);
put.in1 = g_strdup(key);

if (soap_call___ns1__getJobProperty(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  value = g_strdup(get.out);
  }
else
  {
  printf("grisu_job_cwd(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }
#endif

return(value);
}

/***********************************/
/* get the job's working directory */
/***********************************/
/* returned value should be freed */
gchar *grisu_job_cwd(const gchar *name)
{
gchar *cwd=NULL;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__getJobDirectory put;
struct _ns1__getJobDirectoryResponse get;

/* prereqs */
g_assert(name != NULL);

grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

put.in0 = g_strdup(name);

if (soap_call___ns1__getJobDirectory(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  cwd = g_strdup(get.out);
  }
else
  {
  printf("grisu_job_cwd(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

g_free(put.in0);

grisu_soap_free(newsoap);
#endif

return(cwd);
}

/***************************************/
/* convert grisu status code to string */
/***************************************/
const gchar *grisu_status_from_code(gint code)
{
/* TODO - exit code = (out - 1000) */
if (code > 1000)
  return("Done");

switch (code)
  {
  case -1002:
    return("Loading...");
  case -1001:
    return("N/A");
  case -1000:
    return("Undefined");
  case -999:
    return("No such job");
  case -100:
    return("Job created");
  case -4:
    return("Ready to submit");
  case -3:
    return("External handle ready");
  case -2:
    return("Staging in");
  case -1:
    return("Unsubmitted");
  case 0:
    return("Pending");
  case 1:
    return("Active");
  case 101:
    return("Cleaning up"); 
  case 900:
    return("Unknown");
  case 998:
    return("Killed");
  case 999:
    return("Failed");
  case 1000:
    return("Done");
  }

return("Unknown");
}

/***************************/
/* get the status of a job */
/***************************/
const gchar *grisu_job_status(const gchar *name)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__getJobStatus put;
struct _ns1__getJobStatusResponse get;

/* prereqs */
g_assert(name != NULL);

grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

put.in0 = g_strdup(name);

if (soap_call___ns1__getJobStatus(newsoap, ws, "", &put, &get) == SOAP_OK)
  {

g_free(put.in0);

  if (get.out > 1000)
    {
/* TODO - exit code = (out - 1000) */
    return("Done");
    }

  switch (get.out)
    {
    case -1002:
      return("Loading...");
    case -1001:
      return("N/A");
    case -1000:
      return("Undefined");
    case -999:
      return("No such job");
    case -100:
      return("Job created");
    case -4:
      return("Ready to submit");
    case -3:
      return("External handle ready");
    case -2:
      return("Staging in");
    case -1:
      return("Unsubmitted");
    case 0:
      return("Pending");
    case 1:
      return("Active");
    case 101:
      return("Cleaning up"); 
    case 900:
      return("Unknown");
    case 998:
      return("Killed");
    case 999:
      return("Failed");
    case 1000:
      return("Done");
    }
  }
else
  {
  printf("grisu_job_status(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);

  g_free(put.in0);
  }

grisu_soap_free(newsoap);
#endif

return(NULL);
}

/***********************/
/* get the job details */
/***********************/
// this is not used - replaced with a fill XML parse during the refresh job list
gchar *grisu_job_details(gpointer data)
{
gchar *xml=NULL;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__getJobDetails_USCOREstring put;
struct _ns1__getJobDetails_USCOREstringResponse get;
struct grid_pak *grid = data;

/* prereqs */
g_assert(grid != NULL);

//printf("grisu_job_details(): %s\n", grid->jobname);

grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

put.in0 = g_strdup(grid->jobname);

if (soap_call___ns1__getJobDetails_USCOREstring(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  xml = g_strdup(get.out);
  }
else
  {
  printf("grisu_job_details(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

grisu_soap_free(newsoap);
#endif

return(xml);
}

/***********************************************/
/* grisu job description xml parsing primitive */
/***********************************************/
void grisu_xml_job_text(GMarkupParseContext *context,
                        const gchar *text,
                        gsize text_len,  
                        gpointer data,
                        GError **error)
{
const GSList *list;
struct grid_pak *grid=data;

//printf("level 2 text [%s]\n", text);
/* checks */
if (!text_len)
  return;

list = g_markup_parse_context_get_element_stack(context);
if (!list)
  return;

//printf("stack 1 [%s]\n", (gchar *) list->data);

if (g_ascii_strncasecmp(list->data, "HostName", 8) == 0)
  {
  g_free(grid->remote_q);
  grid->remote_q = g_strdup(text); 

#if DEBUG_GRISU_CLIENT
printf("queue = [%s]\n", grid->remote_q);
#endif
  }

if (g_ascii_strncasecmp(list->data, "stdout", 8) == 0)
  {
  g_free(grid->local_output);
  grid->local_output = g_strdup(text); 

#if DEBUG_GRISU_CLIENT
printf("output = [%s]\n", grid->local_output);
#endif
  }

if (g_ascii_strncasecmp(list->data, "input", 5) == 0)
  {
  g_free(grid->local_input);
  grid->local_input = g_strdup(text); 

#if DEBUG_GRISU_CLIENT
printf("input = [%s]\n", grid->local_input);
#endif
  }

if (g_ascii_strncasecmp(list->data, "output", 6) == 0)
  {
  g_free(grid->local_output);
  grid->local_output = g_strdup(text); 

#if DEBUG_GRISU_CLIENT
printf("output = [%s]\n", grid->local_output);
#endif
  }
}

/*******************************************/
/* grisu job details xml parsing primitive */
/*******************************************/
void grisu_xml_details_start(GMarkupParseContext *context,
                             const gchar *element,
                             const gchar **names,
                             const gchar **values,
                             gpointer data,
                             GError **error)
{
gint i;
struct grid_pak *grid=data;

// NB: grisu's returned XML is quite ugly
// has a combination of <, >, &lt, &gt
// this may be because it has xml documents embeded as text
// fields inside the main xml document

/*
printf("start: [%s]\n", element);
i=0;
while (*(names+i) && *(values+i))
  {
  printf("  -  [%s][%s]\n", *(names+i), *(values+i));
  i++;
  }
*/

// there is also a [jobs] element - which we dont want
/* element */
if (g_ascii_strncasecmp(element, "job\0", 4) == 0)
  {
  i=0;
  while (*(names+i) && *(values+i))
    {
    if (g_ascii_strncasecmp(*(names+i), "fqan", 4) == 0)
      {
      g_free(grid->user_vo);
      grid->user_vo = g_strdup(*(values+i));
#if DEBUG_GRISU_CLIENT
printf("vo = %s\n", grid->user_vo);
#endif
      }

    if (g_ascii_strncasecmp(*(names+i), "status", 6) == 0)
      {
      gint code = str_to_float(*(values+i));
      g_free(grid->status);
      grid->status = g_strdup(grisu_status_from_code(code));
#if DEBUG_GRISU_CLIENT
printf("status = %s\n", grid->status);
#endif
      }
    i++;
    }
  }
}

/*******************************************/
/* grisu job details xml parsing primitive */
/*******************************************/
void grisu_xml_details_text(GMarkupParseContext *context,
                    const gchar *text,
                    gsize text_len,  
                    gpointer data,
                    GError **error)
{
GMarkupParser xml_parser;
GMarkupParseContext *xml_context;

// grisu passes complete xml documents as text data inside the main xml document
// so, we have to parse this for lower level job details 

//printf("text = [%s]\n", text);

if (g_strrstr(text, "xml"))
  {
// if strrstr xml ...
/* setup */
  xml_parser.start_element = NULL;
  xml_parser.end_element = NULL;
  xml_parser.text = grisu_xml_job_text;
  xml_parser.passthrough = NULL;
  xml_parser.error = NULL;

  xml_context = g_markup_parse_context_new(&xml_parser, 0, data, NULL);

/* parse */
  if (!g_markup_parse_context_parse(xml_context, text, text_len, NULL))
    printf("grisu_xml_parse() : level 2 parsing error.\n");

/* cleanup */
  if (!g_markup_parse_context_end_parse(xml_context, NULL))
    printf("grisu_xml_parse() : level 2 errors occurred processing xml.\n");

  g_markup_parse_context_free(xml_context);
  }
}

/***************************************/
/* get job details - grisu XML jobfile */
/***************************************/
void grisu_job_refresh(gpointer data)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__getJobDetails_USCOREstring put;
struct _ns1__getJobDetails_USCOREstringResponse get;
struct grid_pak *grid = data;
GMarkupParser xml_parser;
GMarkupParseContext *xml_context;

#if DEBUG_GRISU_CLIENT
printf("grisu_job_refresh(): %s\n", grid->jobname);
#endif

grisu_auth_set(TRUE);
newsoap = grisu_soap_new();
put.in0 = g_strdup(grid->jobname);

if (soap_call___ns1__getJobDetails_USCOREstring(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
/* setup */
  xml_parser.start_element = grisu_xml_details_start;
  xml_parser.end_element = NULL;
  xml_parser.text = grisu_xml_details_text;
  xml_parser.passthrough = NULL;
  xml_parser.error = NULL;

  xml_context = g_markup_parse_context_new(&xml_parser, 0, grid, NULL);

/* parse */
  if (!g_markup_parse_context_parse(xml_context, get.out, strlen(get.out), NULL))
    printf("grisu_xml_parse() : parsing error.\n");

/* cleanup */
  if (!g_markup_parse_context_end_parse(xml_context, NULL))
    printf("grisu_xml_parse() : errors occurred processing xml.\n");

  g_markup_parse_context_free(xml_context);
  }
else
  {
  printf("grisu_job_details(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

/* done */
g_free(put.in0);
grisu_soap_free(newsoap);
#endif
}

/**********************************/
/* get current list of grisu jobs */
/**********************************/
/* Currently unused */
gchar *grisu_ps(void)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__ps_USCOREstring put;
struct _ns1__ps_USCOREstringResponse get;

printf(" *** GRISU WS - get job information ***\n");

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__ps_USCOREstring(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
printf("Response: %s\n", get.out);
  }
else
  {
  printf(" *** soap error ***\n");
  soap_print_fault(newsoap, stderr);
  }

grisu_soap_free(newsoap);
#endif

return(NULL);
}

/*****************************************/
/* retreive a list of available user VOs */
/*****************************************/
GSList *grisu_fqans_get(void)
{
GSList *list=NULL;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__getFqans put;
struct _ns1__getFqansResponse get;

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

/*
printf(" FQAN >>> [%s][%s]\n", grisu_username, grisu_password);
grisu_soap_logging(TRUE, newsoap);
*/

if (soap_call___ns1__getFqans(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  int i;
  struct ns1__ArrayOfString *out = get.out;

  for (i=0 ; i<out->__sizestring ; i++)
    {
    list = g_slist_prepend(list, g_strdup(out->string[i]));
//    printf("[%d] %s\n", i, out->string[i]);
    }
  }
else
  {
  fprintf(stderr, "FQAN retrieval failed, bad MYPROXY credentials?\n");
  soap_print_fault(newsoap, stderr);
  }

grisu_soap_free(newsoap);
#endif

return(list);
}

/*******************************************************/
/* request the remote job's absolute working directory */
/*******************************************************/
gchar *grisu_absolute_job_dir(const gchar *jobname, const gchar *submit, const gchar *vo)
{
gchar *dir=NULL;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__calculateAbsoluteJobDirectory put;
struct _ns1__calculateAbsoluteJobDirectoryResponse get;

g_assert(jobname != NULL);
g_assert(submit != NULL);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

/* input */
put.in0 = g_strdup(jobname);
put.in1 = g_strdup(submit);

if (vo)
  put.in2 = g_strdup(vo);
else
  put.in2 = g_strdup("");

if (soap_call___ns1__calculateAbsoluteJobDirectory(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  dir = g_strdup(get.out);
  }
else
  {
  fprintf(stderr, "grisu_absolute_job_dir(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

g_free(put.in0);
g_free(put.in1);
g_free(put.in2);

grisu_soap_free(newsoap);
#endif

return(dir);
}

/**********************************************/
/* request the remote job's working directory */
/**********************************************/
gchar *grisu_relative_job_dir(const gchar *jobname)
{
gchar *dir=NULL;
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__calculateRelativeJobDirectory put;
struct _ns1__calculateRelativeJobDirectoryResponse get;

g_assert(jobname != NULL);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

put.in0 = g_strdup(jobname);

if (soap_call___ns1__calculateRelativeJobDirectory(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  dir = g_strdup(get.out);
  }
else
  {
  fprintf(stderr, "grisu_relative_job_dir(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr);
  }

g_free(put.in0);

grisu_soap_free(newsoap);
#endif

return(dir);
}

/*************************/
/* query available sites */
/*************************/
/* currently unused */
GSList *grisu_site_list(void)
{
GSList *list=NULL;
#ifdef WITH_GRISU
gint i;
gpointer newsoap;
struct _ns1__getAllSites put;
struct _ns1__getAllSitesResponse get;
struct ns1__ArrayOfString *out;

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

/* query WS for available sites */
if (soap_call___ns1__getAllSites(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  out = get.out;
  for (i=out->__sizestring ; i-- ; )
    list = g_slist_prepend(list, g_strdup(out->string[i]));
  }
else
  {
  printf(" *** soap error ***\n");
  soap_print_fault(newsoap, stderr); 
  }

grisu_soap_free(newsoap);
#endif

return(list);
}

/******************************************/
/* query available applications at a site */
/******************************************/
/* currently unused */
GSList *grisu_application_list(const gchar *site)
{
GSList *list=NULL;
#ifdef WITH_GRISU
gint i;
gpointer newsoap;
struct _ns1__getAllAvailableApplications put;
struct _ns1__getAllAvailableApplicationsResponse get;
struct ns1__ArrayOfString tmp, *out;

g_assert(site != NULL);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

/* args */
tmp.__sizestring = 1;
tmp.string = malloc(sizeof(char *));
tmp.string[0] = g_strdup(site); 
put.in0 = &tmp;

if (soap_call___ns1__getAllAvailableApplications(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  out = get.out;
  for (i=out->__sizestring ; i-- ; )
    list = g_slist_prepend(list, g_strdup(out->string[i]));
  }
else
  {
  fprintf(stderr, "grisu_application_list(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr); 
  }

g_free(tmp.string[0]);
g_free(tmp.string);

grisu_soap_free(newsoap);
#endif

return(list);
}

/*****************************************************/
/* retrieve the site name from a submission location */
/*****************************************************/
/* TODO - cache - getAllHosts() value */
/* TODO - during grid connect phase - grab & cache a must as we can */
gchar *grisu_site_name(const gchar *submit)
{
gchar *site=NULL;
#ifdef WITH_GRISU
gchar **host;
gpointer newsoap;
struct _ns1__getSite put;
struct _ns1__getSiteResponse get;

g_assert(submit != NULL);

/* CURRENT - bit of a hack to get the site name of a submission location */
/* can only query the site of a host ... so we process the submit location */
/* to get this for the query */

host = g_strsplit(submit, ":", 3);

/* TODO - some sites have #Loadlevel crap after the hostname - get rid of this? */
/* NEW - grisu expected to be updated to cope with submission location as input */
//printf(" *** GRISU WS - requesting site name for [%s] ***\n", host[1]);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

put.in0 = host[1];

if (soap_call___ns1__getSite(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  if (get.out)
    site = g_strdup(get.out);
  }
else
  {
  fprintf(stderr, "grisu_site_name(): SOAP ERROR\n");
  soap_print_fault(newsoap, stderr); 
  }

g_strfreev(host);

grisu_soap_free(newsoap);
#endif

return(site);
}

/*******************************************************************/
/* find all queue/host pairs where an application can be submitted */
/*******************************************************************/
#define DEBUG_GRISU_APPLICATION_FIND 0
GSList *grisu_submit_find(const gchar *name)
{
GSList *list=NULL;
#ifdef WITH_GRISU
gint i;
gpointer newsoap;
struct _ns1__getSubmissionLocationsForApplication put;
struct _ns1__getSubmissionLocationsForApplicationResponse get;
struct ns1__ArrayOfString *out;

g_assert(name != NULL);

#if DEBUG_GRISU_APPLICATION_FIND
printf("Looking for grid application [%s]:\n", name);
#endif

newsoap = grisu_soap_new();

put.in0 = g_strdup(name);

if (soap_call___ns1__getSubmissionLocationsForApplication(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  out = get.out;
  for (i=out->__sizestring ; i-- ; )
    {
    list = g_slist_prepend(list, g_strdup(out->string[i]));
#if DEBUG_GRISU_APPLICATION_FIND
printf(" - [%s]\n", out->string[i]);
#endif
    }
  }
else
  {
  fprintf(stderr, "grisu_submit_find(): soap error\n");
  soap_print_fault(newsoap, stderr); 
  }

g_free(put.in0);

grisu_soap_free(newsoap);
#endif

return(list);
}

/****************************************/
/* get version names of site executable */
/****************************************/
#define DEBUG_GRISU_APPLICATION_VERSION 0
GSList *grisu_application_versions(const gchar *name, const gchar *site)
{
GSList *list=NULL;
#ifdef WITH_GRISU
gint i;
gpointer newsoap;
struct _ns1__getVersionsOfApplicationOnSite put;
struct _ns1__getVersionsOfApplicationOnSiteResponse get;
struct ns1__ArrayOfString *out;

#if DEBUG_GRISU_APPLICATION_VERSION
printf("Searching versions for [%s] at site [%s]\n", name, site);
#endif

/* checks */
if (!name || !site)
  return(NULL);

put.in0 = g_strdup(name);
put.in1 = g_strdup(site);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__getVersionsOfApplicationOnSite(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  out = get.out;
  for (i=out->__sizestring ; i-- ; )
    {
    list = g_slist_prepend(list, g_strdup(out->string[i]));
#if DEBUG_GRISU_APPLICATION_VERSION
printf("+ [%s]\n", out->string[i]);
#endif
    }
  }
else
  {
#if DEBUG_GRISU_APPLICATION_VERSION
  printf(" *** soap error ***\n");
  soap_print_fault(newsoap, stderr); 
#endif
  }

g_free(put.in0);
g_free(put.in1);

grisu_soap_free(newsoap);
#endif

return(list);
}

/*******************************************************/
/* get information about how the exectuable is invoked */
/*******************************************************/
#define DEBUG_GRISU_APPLICATION_DETAILS 0
GHashTable *grisu_application_details(const gchar *name, const gchar *site)
{
GHashTable *details=NULL;
#ifdef WITH_GRISU
gint i;
gpointer newsoap;
struct _ns1__getApplicationDetails put;
struct _ns1__getApplicationDetailsResponse get;
struct ns1__anyType2anyTypeMap *map;	
struct _ns1__anyType2anyTypeMap_entry *entry;

g_assert(name != NULL);
g_assert(site != NULL);

//details = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
details = g_hash_table_new_full(g_str_hash, hash_strcmp, g_free, g_free);

put.in0 = g_strdup(name);
put.in1 = g_strdup(site);

/* prereqs */
grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

if (soap_call___ns1__getApplicationDetails(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  map = get.out;

  for (i=0 ; i<map->__sizeentry ; i++)
    {
    entry = map->entry+i;

    g_hash_table_insert(details, g_strdup(entry->key), g_strdup(entry->value));

#if DEBUG_GRISU_APPLICATION_DETAILS
printf("[%s] = [%s]\n", entry->key, entry->value); 
#endif
    }
  }
else
  {
  printf(" *** soap error ***\n");
  soap_print_fault(newsoap, stderr); 
  }

g_free(put.in0);
g_free(put.in1);

grisu_soap_free(newsoap);
#endif

return(details);
}

/* NEW*/

/***********************************/
/* get the current lifetime (mins) */
/***********************************/
gint grisu_credential_lifetime(void)
{
#ifdef WITH_GRISU
gpointer newsoap;
struct _ns1__getCredentialEndTime put;
struct _ns1__getCredentialEndTimeResponse get;

grisu_auth_set(TRUE);
newsoap = grisu_soap_new();

printf("grisu_credential_lifetime(): start\n");

if (soap_call___ns1__getCredentialEndTime(newsoap, ws, "", &put, &get) == SOAP_OK)
  {
  long x;

  x = get.out;

printf("server response: %ld\n", x);

  }
else
  {
  printf(" *** soap error ***\n");
  soap_print_fault(newsoap, stderr); 
  }

#endif

printf("grisu_credential_lifetime(): stop\n");

return(0);
}

/******************************************/
/* initialize subsequent webservice calls */
/******************************************/
int grisu_init(void)
{
gint value=0;
#ifdef WITH_GRISU
const gchar *home;

/* grisu init */
if (!ws)
  {
  if (sysenv.default_ws)
    ws = sysenv.default_ws;
  else
    {
    sysenv.default_ws = g_strdup(prod_ws);
    ws = prod_ws;
    }
  }

/*
if (!grisu_username)
  grisu_username = grid_random_alpha(10);
if (!grisu_password)
  grisu_password = grid_random_alphanum(12);
*/

if (!grisu_server)
  grisu_server=g_strdup("myproxy.arcs.org.au");

// FIXME - sometimes needed when myproxy plays up also might need to setenv
// MYPROXY_SERVER_DN = /C=AU/O=APACGrid/OU=VPAC/CN=myproxy2.arcs.org.au
//  grisu_server=g_strdup("myproxy2.arcs.org.au");

if (!grisu_port)
  grisu_port=g_strdup("7512");

/* certificate path init */
home = g_get_home_dir();
if (home)
  grisu_cert_path = g_build_filename(home, ".globus", "certificates", NULL);
if (!g_file_test(grisu_cert_path, G_FILE_TEST_IS_DIR))
  {
#if DEBUG_GRISU_CLIENT
printf("Warning: no certificate found in: %s\n", grisu_cert_path);
#endif
  g_free(grisu_cert_path);

/* don't return if no certs - we can soldier on */
/* as it's only myproxy-init that won't work */
  grisu_cert_path=NULL;
  }

#if DEBUG_GRISU_CLIENT
printf("Using certificate location: %s\n", grisu_cert_path);
#endif

/* use SOAP_SSL_DEFAULT in production code */
/* keyfile: required only when client must authenticate to server */
/* password to read the keyfile */
/* optional cacert file to store trusted certificates */
/* optional capath to directory with trusted certificates */
/* if randfile!=NULL: use a file with random data to seed randomness */ 
/* CURRENT - skipping host check ... presumably we have to retrieve/store */
/* server's public cert if we want to use the default method??? */
/*
if (soap_ssl_client_context(&soap, SOAP_SSL_DEFAULT, NULL, NULL, NULL, path, NULL))
if (soap_ssl_client_context(&soap, SOAP_SSL_SKIP_HOST_CHECK, NULL, NULL, NULL, path, NULL))
*/
#endif

return(value);
}

/**********************/
/* webservice cleanup */
/**********************/
void grisu_free(void)
{
#ifdef WITH_GRISU
/* grisu */
g_free(grisu_username);
g_free(grisu_password);
g_free(grisu_server);
g_free(grisu_port);
g_free(grisu_soap_header);
g_free(grisu_cert_path);

/* cleanup the soap environment allocations */
grisu_soap_cleanup();
#endif
}

