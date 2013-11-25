/*
Copyright (C) 2010 by Sean David Fleming

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

#ifdef WITH_PYTHON

#include <stdio.h>

#include <Python.h>

#include "gdis.h"
#include "grid.h"
#include "grisu_client.h"

// name of the python script (minus the .py)
#define MODULE_NAME "init"

extern struct sysenv_pak sysenv;

PyObject *py_module = NULL;
PyObject *py_dict = NULL;
GSList *py_idp_list=NULL;

/********************************************/
/* shut down python and free all references */
/********************************************/
void py_stop(void)
{
//TODO
//g_slist_free(py_idp_list);

if (py_module && py_dict)
  {
/* destroy python env */
  Py_Finalize();
  }
py_module = NULL;
py_dict = NULL;
}

/***********************************************/
/* initialize python and load a script for use */
/***********************************************/
void py_start(const gchar *filename)
{
gchar *module_path;
PyObject *path = NULL, *pwd = NULL;
PyObject *p_name = NULL;

if (py_module || py_dict)
  {
  printf("py_start(): already running\n");
  return;
  }

/* Initialize the Python framework */
printf("py_start(): initializing...\n");
Py_Initialize();

/* TODO - can we check here for GLOBUS_LOCATION and nuke it if exists? */
/* add the python script location to the path */
path = PySys_GetObject("path");

module_path = g_build_filename(sysenv.gdis_path, "lib", NULL);
pwd = PyString_FromString(module_path);
g_free(module_path);

PyList_Insert(path, 0, pwd);
Py_DECREF(pwd);

/* load the python module */
if (filename)
  {
  p_name = PyString_FromString(filename);
  py_module = PyImport_Import(p_name);
  Py_DECREF(p_name);
  if (!py_module)
    {
    PyErr_Print();
    PyErr_Clear();
    printf("py_start(): failed to import: %s\n", filename);
    return;
    }
  }

/* Get a dictionary object from the module */
py_dict = PyModule_GetDict(py_module);

if (!py_dict)
  {
  printf("py_start(): failed to get dictionary\n");
  py_module=NULL;
  py_stop();
  }
}

/**************************************************/
/* location a function in the current environment */
/**************************************************/
PyObject *py_function(const gchar *function)
{
PyObject *f = NULL;

if (!py_dict)
  {
  py_start(MODULE_NAME);
  if (!py_dict)
    return(NULL);
  }
if (!function)
  return(NULL);

f = PyDict_GetItemString(py_dict, function);

/* check if found and callable */
if (f && PyCallable_Check(f) == TRUE) 
  {
  printf("Located function: %s\n", function);
  }
else 
  {
  printf("Failed to locate callable function: %s in the dictionary\n", function);
   if (PyErr_Occurred())
     {
     PyErr_Print();
     PyErr_Clear();
     }
  }
return(f);
}

/**********************************************/
/* attempt to get the list of federation idps */
/**********************************************/
#define IDP_FALLBACK 1
GSList *py_idp_lookup(void)
{
gchar *text;
PyObject *func = NULL, *args = NULL, *ret = NULL;

if (IDP_FALLBACK)
  {
// FIXME - inserting list of important idps until python idp lookup is working
  if (!py_idp_list)
    {
    text = grid_idp_get();
    if (text) 
      py_idp_list = g_slist_prepend(py_idp_list, text);
    else
      {
      py_idp_list = g_slist_prepend(py_idp_list, "VPAC");
      py_idp_list = g_slist_prepend(py_idp_list, "TPAC");
      py_idp_list = g_slist_prepend(py_idp_list, "ARCS IdP");
      py_idp_list = g_slist_prepend(py_idp_list, "iVEC");
      }
    }
  return(py_idp_list);
  }
else
  {

func = py_function("slcs_idp_list");

args = PyTuple_New(1);

PyTuple_SetItem(args, 0, PyString_FromString("url://"));

/* call the function */
ret = PyObject_CallObject(func, args);
if (!ret)
  {
/* Print error and abort */
  PyErr_Print();
  PyErr_Clear();
  printf("py_idp_lookup(): failed.\n");
  return(NULL);
  }

/* unref objects */
Py_XDECREF(ret);
Py_DECREF(args);
  }

return(NULL);
}

/********************************************************/
/* authenticate and get a slcs key/cert from the server */
/********************************************************/
void py_authenticate(const gchar *idp, const gchar *u, const gchar *p)
{
PyObject *func = NULL, *args = NULL, *ret = NULL;

func = py_function("myproxy_upload");

/* NB: argument lists must be tuples */
/* create argument to pass to function */
args = PyTuple_New(7);
// NB: 0th argument will be ignored (ie sys.argv[0])
// NB: order (after sys.argv values) is important - see init.py
PyTuple_SetItem(args, 0, PyString_FromString("dummy"));
PyTuple_SetItem(args, 1, PyString_FromString("-i"));
PyTuple_SetItem(args, 2, PyString_FromString(idp));

PyTuple_SetItem(args, 3, PyString_FromString(u));
PyTuple_SetItem(args, 4, PyString_FromString(p));

/* CURRENT - enforce random myproxy credentials */
grisu_keypair_set(grid_random_alpha(10), grid_random_alphanum(12));

//printf("uploading [%s : %s]\n", grisu_username_get(), grisu_password_get());

PyTuple_SetItem(args, 5, PyString_FromString(grisu_username_get()));
PyTuple_SetItem(args, 6, PyString_FromString(grisu_password_get()));

/*
Usage: init.py [options] [idp]

Options:
  -h, --help            show this help message and exit
  -c, --proxy           create a local 12 hour proxy.
  -d DIR, --storedir=DIR
                        the directory to store the certificate/key and
                        config file
  -k, --key             use Shibboleth password as key passphrase
  -w, --write           write the idp specified on the command line to
                        a config file
  -v, --verbose         print status messages to stdout
  -V, --version         print version number and exit

  SLCS Options:
    -i SLCS_IDP, --idp=SLCS_IDP
                        unique ID of the IdP used to log in
    -s SLCS_URL, --slcs=SLCS_URL
                        location of SLCS server (if not specified, use
                        SLCS_SERVER system variable or settings from
                        [storedir]/slcs-client.properties

  MyProxy Options:
    -a, --myproxy-autostore
                        upload the proxy to myproxy each time.
    -u MYPROXY_USER, --myproxy-user=MYPROXY_USER
                        the username to connect to the myproxy server as.
    -m MYPROXY_HOST, --myproxy-host=MYPROXY_HOST
                        the hostname of the myproxy server
    -p MYPROXY_PORT, --myproxy-port=MYPROXY_PORT
                        the port of the myproxy server
    -l MYPROXY_LIFETIME, --myproxy-lifetime=MYPROXY_LIFETIME
                        the lifetime of the certificate to put in myproxy
*/

/* call the function */
ret = PyObject_CallObject(func, args);
if (!ret)
  {
/* Print error and abort */
  PyErr_Print();
  PyErr_Clear();
  printf("py_authenticate(): failed.\n");
  return;
  }

/* unref objects */
Py_XDECREF(ret);
Py_DECREF(args);
}

#endif
