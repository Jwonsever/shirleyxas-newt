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
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gdis.h"
#include "coords.h"
#include "error.h"
#include "file.h"
#include "parse.h"
#include "matrix.h"
#include "model.h"
#include "space.h"
#include "render.h"
#include "select.h"
#include "import.h"
#include "project.h"
#include "gui_shorts.h"
#include "interface.h"
#include "dialog.h"
#include "opengl.h"
#include "numeric.h"
#include "reaxmd.h"

#ifdef WITH_GRISU
#include "grisu_client.h"
#endif

#define DEBUG_MORE 0
#define MAX_KEYS 15

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];


/************************************************/
/* allocate and initialize a new file structure */
/************************************************/
gpointer file_new(gint id, gint group, gint menu, const gchar *label)
{
struct file_pak *file;

file = g_malloc(sizeof(struct file_pak));
sysenv.file_list = g_slist_prepend(sysenv.file_list, file);

file->id = id;                         /* unique identifier */ 
file->group = group;                      /* used to group inp/out types */
file->menu = menu;                         /* include in menu listing */
file->label = g_strdup(label);
file->ext = NULL;                          /* extension matching */
/* old */
file->write_file = NULL;                   /* file creation */
file->read_file = NULL;                    /* file reading */
file->read_frame = NULL;                   /* frame reading */
/* NEW */
file->import_file = NULL;
file->export_file = NULL;

return(file);
}

/***************************************/
/* setup the recognized file type list */
/***************************************/
#define DEBUG_FILE_INIT 0
void file_init(void)
{
struct file_pak *file;

/* build the recognized file type list */
sysenv.file_list = NULL;

/* supported file type */
file = file_new(NODATA, NODATA, TRUE, "All files");

/* supported file type */
file = file_new(DATA, DATA, TRUE, "All known types");

/* supported file type */
file = file_new(ABINIT_OUT, ABINIT, FALSE, "ABINIT output");
file->ext = g_slist_prepend(file->ext, "about");
file->ext = g_slist_prepend(file->ext, "abot");
file->read_file = read_about;
file->read_frame = read_about_frame;

/* supported file type */
file = file_new(AIMS_INPUT, AIMS_INPUT, TRUE, "AIMS input");
file->ext = g_slist_prepend(file->ext, "aims");
file->read_file = read_aims;
file->write_file = write_aims;

/* supported file type */
file = file_new(MD_ANALYSIS, MD_ANALYSIS, FALSE, "Analysis");

/* supported file type  */
file = file_new(BIOGRAF, BIOGRAF, TRUE, "Biograf");
file->ext = g_slist_prepend(file->ext, "bgf");
file->write_file = write_bgf;
file->read_file = read_bgf;

/* supported file type */
file = file_new(BIOSYM, BIOSYM, TRUE, "Biosym");
file->ext = g_slist_prepend(file->ext, "car");
file->ext = g_slist_prepend(file->ext, "cor");
file->ext = g_slist_prepend(file->ext, "arc");
file->write_file = write_arc;
file->read_file = read_arc;
file->read_frame = read_arc_frame;

/* supported file type */
file = file_new(CASTEP, CASTEP, TRUE, "Castep");
file->ext = g_slist_append(file->ext, "cell");
file->import_file = castep_import_cell;
file->export_file = castep_export_cell;

/* supported file type */
file = file_new(CASTEP_OUT, CASTEP, FALSE, "Castep output");
file->ext = g_slist_append(file->ext, "castep");
file->read_file = read_castep_out;
file->read_frame = read_castep_out_frame;

/* supported file type */
file = file_new(CIF, CIF, TRUE, "CIF");
file->ext = g_slist_prepend(file->ext, "cif");
file->write_file = write_cif;
file->read_file = read_cif;

/* monty crystal graph file format */
file = file_new(CRYSTAL_GRAPH, CRYSTAL_GRAPH, TRUE, "Crystal Graph Files");
file->ext = g_slist_prepend(file->ext, "cgf");
file->write_file = write_cgf;
file->read_file = read_cgf;

/* CSSR */
file = file_new(CSSR, CSSR, TRUE, "CSSR");
file->ext = g_slist_append(file->ext, "cssr");
file->write_file = write_cssr;
file->read_file = read_cssr;

/* DLP file type - what is this? */
file = file_new(DLP, DLP, TRUE, "DLP");
file->ext = g_slist_prepend(file->ext, "dlp");
file->write_file = write_dlp;
file->read_file = read_dlp;

/* DL_POLY input */
file = file_new(DLPOLY, DLPOLY, TRUE, "DL_POLY");
file->ext = g_slist_append(file->ext, "dlpoly");
file->write_file = write_dlpoly;
file->read_file = read_dlpoly;

/* DMOL input */
file = file_new(DMOL_INPUT, DMOL_INPUT, TRUE, "DMOL");
file->ext = g_slist_prepend(file->ext, "dmol");
file->write_file = write_dmol;
file->read_file = read_dmol;
file->read_frame = read_dmol_frame;

/* GAMESS input */
file = file_new(GAMESS, GAMESS, TRUE, "GAMESS-US");
file->ext = g_slist_append(file->ext, "inp");
file->import_file = gamess_input_import;
file->export_file = gamess_input_export;

/* GAMESS output */
file = file_new(GAMESS_OUT, GAMESS, FALSE, "GAMESS output");
file->ext = g_slist_prepend(file->ext, "gmout");
file->ext = g_slist_prepend(file->ext, "gmot");
file->read_file = read_gms_out;
file->read_frame = read_gms_out_frame;
//file->import_file = gamess_output_import;

/* GAUSSIAN input */
file = file_new(GAUSS, GAUSS, TRUE, "GAUSSIAN");
file->ext = g_slist_append(file->ext, "com");
file->write_file = write_gauss;

/* GAUSSIAN input */
file = file_new(GAUSS_OUT, GAUSS, FALSE, "GAUSSIAN output");
file->ext = g_slist_append(file->ext, "log");
file->read_file = read_gauss_out;
file->read_frame = read_gauss_out_frame;

/* supported file type */
file = file_new(GAUSS_CUBE, GAUSS, FALSE, "GAUSSIAN cube file");
file->ext = g_slist_append(file->ext, "cube");
file->read_file = read_gauss_cube;

/* supported file type */
file = file_new(GROMACS, GROMACS, TRUE, "GROMACS input");
file->ext = NULL;
file->ext = g_slist_append(file->ext, "gro");
file->write_file = write_gromacs;
file->read_file = read_gromacs_gro;

/* GDIS morphology (replace with XML?) */
/* TODO - replace with XML */
/*
file = file_new(MORPH, MORPH, TRUE, "GDIS morphology");
file->ext = g_slist_append(file->ext, "gmf");
file->write_file = write_gmf;
file->read_file = read_gmf;
*/
file = file_new(PLOT, PLOT, TRUE, "GDIS plot");
file->ext = g_slist_append(file->ext, "plot");
file->import_file = graph_import;

/* Geomview */
file = file_new(GEOMVIEW_OFF, GEOMVIEW_OFF, TRUE, "Geomview OFF");
file->ext = g_slist_prepend(file->ext, "off");
file->read_file = read_off;

/* GULP input */
file = file_new(GULP, GULP, TRUE, "GULP input");
file->ext = g_slist_prepend(file->ext, "res");
file->ext = g_slist_prepend(file->ext, "gin");
file->import_file = gulp_input_import;
file->export_file = gulp_input_export;

/* GULP output */
file = file_new(GULPOUT, GULP, FALSE, "GULP output");
file->ext = g_slist_prepend(file->ext, "gout");
file->ext = g_slist_prepend(file->ext, "got");
//file->import_file = gulp_output_import;
file->read_file = read_gulp_output;

/* Marvin input */
file = file_new(MARVIN, MARVIN, TRUE, "Marvin input");
file->ext = g_slist_prepend(file->ext, "mvn-r");
file->ext = g_slist_prepend(file->ext, "mvn");
file->ext = g_slist_prepend(file->ext, "mar");
file->write_file = write_marvin;
file->read_file = read_marvin;

/* Marvin output */
file = file_new(MVNOUT, MARVIN, FALSE, "Marvin output");
file->ext = g_slist_prepend(file->ext, "mvnout");
file->ext = g_slist_prepend(file->ext, "mvout");
file->ext = g_slist_prepend(file->ext, "mot");
file->read_file = read_mvnout;

/* meta data output (chemistry archiving) */
file = file_new(META_DATA, META_DATA, FALSE, "Meta data");
file->ext = g_slist_prepend(file->ext, "meta");
file->write_file = write_meta;

/* supported file type */
file = file_new(NWCHEM, NWCHEM, TRUE, "NWChem");
file->ext = g_slist_prepend(file->ext, "nwi");
file->ext = g_slist_prepend(file->ext, "nw");
//file->read_file = read_nwchem_input;
file->import_file = nwchem_input_import;
file->export_file = nwchem_input_export;

/* supported file type */
file = file_new(NWCHEM_OUT, NWCHEM, FALSE, "NWChem output");
file->ext = g_slist_prepend(file->ext, "nwout");
file->ext = g_slist_prepend(file->ext, "nwot");
file->read_file = read_nwout;
file->read_frame = read_nwout_frame;

/* supported file type */
file = file_new(OPENMX_INPUT, OPENMX, TRUE, "OpenMX input");
file->ext = g_slist_append(file->ext, "mxi");
file->write_file = write_openmx_input;
file->read_file = read_openmx_input;

/* supported file type */
file = file_new(PDB, PDB, TRUE, "PDB");
file->ext = g_slist_append(file->ext, "pdb");
file->write_file = write_pdb;
file->read_file = read_pdb;
file->read_frame = read_pdb_frame;

/* supported file type */
file = file_new(POVRAY, POVRAY, FALSE, "POVRay");
file->ext = g_slist_append(file->ext, "pov");
file->write_file = write_povray;

/* supported file type */
file = file_new(CEL, CEL, TRUE, "PowderCell");
file->ext = g_slist_append(file->ext, "cel");
file->read_file = read_cel;

/* supported file type */
file = file_new(PWSCF, PWSCF, TRUE, "PWscf input");
file->ext = g_slist_append(file->ext, "pwi");
file->read_file = read_pwi;
file->write_file = write_pwi;

/* supported file type */
file = file_new(PWSCF_OUTPUT, PWSCF, TRUE, "PWscf output");
file->ext = g_slist_append(file->ext, "pwo");
file->read_file = read_pwo;

/* supported file type */
file = file_new(RIETICA, RIETICA, TRUE, "Rietica");
file->ext = g_slist_prepend(file->ext, "inp");
file->read_file = read_rietica;

/* supported file type */
file = file_new(REAXMD, REAXMD, TRUE, "ReaxMD");
file->ext = g_slist_prepend(file->ext, "irx");
file->import_file = reaxmd_input_import;

/* supported file type */
file = file_new(REAXMD_OUTPUT, REAXMD, FALSE, "ReaxMD output");
file->ext = g_slist_prepend(file->ext, "orx");
file->import_file = reaxmd_output_import;

/* supported file type */
file = file_new(FDF, FDF, TRUE, "SIESTA input");
file->ext = g_slist_prepend(file->ext, "fdf");
//file->write_file = write_fdf;
//file->read_file = read_fdf;
file->import_file = siesta_input_import;
file->export_file = siesta_input_export;

/* supported file type */
file = file_new(SIESTA_OUT, FDF, FALSE, "SIESTA output");
file->ext = g_slist_prepend(file->ext, "sout");
file->ext = g_slist_prepend(file->ext, "sot");
//file->read_file = read_sout;
//file->read_frame = read_sout_frame;
file->import_file = siesta_output_import;

/* supported file type */
file = file_new(SIESTA_POTENTIALS, FDF, FALSE, "SIESTA data");
file->ext = g_slist_prepend(file->ext, "psf");
file->ext = g_slist_prepend(file->ext, "vps");
file->ext = g_slist_prepend(file->ext, "bands");
file->ext = g_slist_prepend(file->ext, "CG");
file->ext = g_slist_prepend(file->ext, "DIM");
file->ext = g_slist_prepend(file->ext, "DM");
file->ext = g_slist_prepend(file->ext, "DOS");
file->ext = g_slist_prepend(file->ext, "EIG");
file->ext = g_slist_prepend(file->ext, "LDOS");
file->ext = g_slist_prepend(file->ext, "PDOS");
file->ext = g_slist_prepend(file->ext, "PLD");
file->ext = g_slist_prepend(file->ext, "RHO");
file->ext = g_slist_prepend(file->ext, "DRHO");
file->ext = g_slist_prepend(file->ext, "VH");
file->ext = g_slist_prepend(file->ext, "VT");
file->ext = g_slist_prepend(file->ext, "WFS");
file->ext = g_slist_prepend(file->ext, "XV");

/* supported file type */
file = file_new(TEXT, TEXT, FALSE, "Text files");
file->ext = g_slist_prepend(file->ext, "txt");
file->import_file = file_text_import;

/* supported file type */
/*
file = file_new(XML, XML, TRUE, "XML (Project)");
file->ext = g_slist_prepend(file->ext, "xml");
file->write_file = write_xml;
file->read_file = read_xml;
*/

/* supported file type */
file = file_new(XSF, XSF, TRUE, "XCrysDen");
file->ext = g_slist_prepend(file->ext, "xsf");
file->write_file = write_xsf;
file->read_file = read_xsf;

/* supported file type */
file = file_new(XTL, XTL, TRUE, "XTL");
file->ext = g_slist_append(file->ext, "xtl");
file->write_file = write_xtl;
file->read_file = read_xtl;

/* supported file type */
file = file_new(XYZ, XYZ, TRUE, "XYZ");
file->ext = g_slist_append(file->ext, "xyz");
file->ext = g_slist_append(file->ext, "ani");
file->write_file = write_xyz;
file->read_file = read_xyz;

sysenv.file_list = g_slist_reverse(sysenv.file_list);
}

/******************************/
/* free file recognition list */
/******************************/
void file_free(void)
{
GSList *list;

/* free data embedded in the file structure */
for (list=sysenv.file_list ; list ; list=g_slist_next(list))
  {
  struct file_pak *file = list->data;

  g_slist_free(file->ext);
  g_free(file->label);
  }

/* free the list and the file structure pointers */
free_slist(sysenv.file_list);
}

/************************/
/* current version info */
/************************/
void gdis_blurb(FILE *fp)
{
fprintf(fp, "Created by GDIS version %4.2f\n", VERSION);
}

/***********************************************/
/* filter a list of filenames against a filter */
/***********************************************/
/* returned list and it's data should be freed */
GSList *file_list_filter(gint id, GSList *list)
{
gchar *filename;
GSList *item, *result=NULL;
struct file_pak *file;

for (item=list ; item ; item=g_slist_next(item))
  {
  filename = item->data;

/* FIXME - using NODATA as a filter to allow everything */
  if (id == NODATA)
    {
    result = g_slist_prepend(result, g_strdup(filename));
    }
  else
    {
    file = file_type_from_filename(filename); 
    if (file)
      {
/* DATA - allow all recognized filetypes through */
/* otherwise only allow through if it matches the general group type */
      if (id == DATA)
        result = g_slist_prepend(result, g_strdup(filename));
      else if (id == file->group)
        result = g_slist_prepend(result, g_strdup(filename));
      }
    }
  }
return(g_slist_reverse(result));
}

/*************************************/
/* list all folders in the directory */
/*************************************/
/* returned list and its data should be freed */
GSList *file_folder_list(const gchar *path)
{
gchar *fullpath;
const gchar *name;
GDir *dir;
GSList *list=NULL;
GError *error=NULL;

/* build the directory list */
dir = g_dir_open(path, 0, &error);
if (error)
  printf(">>> %s\n", error->message);

if (dir)
  {
  name = g_dir_read_name(dir);
  while (name)
    {
    fullpath = g_build_filename(path, name, NULL);

    if (g_file_test(fullpath, G_FILE_TEST_IS_DIR))
      list = g_slist_prepend(list, g_strdup(name));

    g_free(fullpath);

    name = g_dir_read_name(dir);
    }
  g_dir_close(dir);

  list = g_slist_sort(list, (GCompareFunc) g_ascii_strcasecmp);
  }

/* ensure we can always navigate back up */
list = g_slist_prepend(list, g_strdup(".."));

return(list);
}

/*********************************************/
/* an all-platform directory listing routine */
/*********************************************/
/* list should be completely freed after use */
GSList *file_dir_list(const gchar *path, gint sort)
{
const gchar *name;
GDir *dir;
GSList *files=NULL;

/* ensure we can go up a directory */

/*
printf("path = [%s]\n", path);
printf(" cwd = [%s]\n", sysenv.cwd);
*/

/* build the directory list */
if (g_ascii_strncasecmp(".\0", path, 2) == 0)
  dir = g_dir_open(sysenv.cwd, 0, NULL);
else
  {
  GError *error=NULL;

  dir = g_dir_open(path, 0, &error);

  if (error)
    printf(">>> %s\n", error->message);
  }

if (!dir)
  {
  printf("Failed to open %s\n", path);
  return(NULL);
  }

name = g_dir_read_name(dir);
while (name)
  {
  if (g_ascii_strncasecmp(".", name, 1) != 0)
    files = g_slist_prepend(files, g_strdup(name));

  name = g_dir_read_name(dir);
  }
g_dir_close(dir);

if (sort)
  files = g_slist_sort(files, (GCompareFunc) g_ascii_strcasecmp);

files = g_slist_prepend(files, g_strdup(".."));

return(files);
}

/*********************************************************************/
/* get a directory listing matched against a supplied (glob) pattern */
/*********************************************************************/
/* list should be completely freed after use */
#define DEBUG_FILE_PATTERN 0
GSList *file_dir_list_pattern(const gchar *dir, const gchar *pattern)
{
gchar *name;
GPatternSpec *ps;
GSList *list, *files;

g_assert(dir != NULL);

files = file_dir_list(dir, FALSE);

if (pattern)
  ps = g_pattern_spec_new(pattern);
else
  return(files);

list = files;
while (list)
  {
  name = list->data;
  list = g_slist_next(list);

  if (!g_pattern_match_string(ps, name))
    {
    files = g_slist_remove(files, name);
    g_free(name);
    }
  }

g_pattern_spec_free(ps);

#if DEBUG_FILE_PATTERN
printf("Found %d matches\n", g_slist_length(files));
#endif

return(files);
}

/*******************************************************/
/* allocate a new string containing the file extension */
/*******************************************************/
gchar *file_extension_get(const gchar *name)
{
gint i,n;
gchar *ext;

if (!name)
  return(NULL);

/* search for '.' character in reverse order */
i = n = strlen(name);
/* minimum size, avoids troublesome cases (eg "..") */
if (n > 2)
  {
  for (i=n-1 ; i-- ; )
    {
    if (name[i] == '.')
      {
      ext = g_strndup(name+i+1, n-i-1);
/* ignore any .# extension */
      if (!str_is_float(ext))
        return(ext);
      g_free(ext);
/* mark the .# as the new end */
      n = i;
      }
    }
  }
return(NULL);
}

/***********************************************************/
/* see if the label/filename can be matched to a file type */
/***********************************************************/
gint file_id_get(gint type, const gchar *text)
{
gchar *txt, *tmp, *ext;
GSList *list1, *list2;
struct file_pak *file;

if (!text)
  return(-1);

if (type == BY_EXTENSION)
  tmp = file_extension_get(text);
else
  tmp = g_ascii_strdown(text, -1);

/* no extension - unknown */
if (!tmp)
  return(-1);

for (list1=sysenv.file_list ; list1 ; list1=g_slist_next(list1))
  {
  file = list1->data;

  switch (type)
    {
    case BY_LABEL:
/* NB: all lower case compare */
      txt = g_ascii_strdown(file->label, -1);
      if (g_strrstr(txt, tmp))
        {
        g_free(tmp);
        g_free(txt);
        return(file->id);
        }
      g_free(txt);
      break;

    case BY_EXTENSION:
/* go through all extensions listed under this file type */
      for (list2=file->ext ; list2 ; list2=g_slist_next(list2))
        {
        ext = list2->data;
        if (strlen(tmp) == strlen(ext))
          {
          if (g_ascii_strcasecmp(tmp, ext) == 0)
            {
            g_free(tmp);
            return(file->id);
            }
          }
        }
    }
  }

g_free(tmp);
return(-1);
}

/*************************************/
/* file type determination primitive */
/*************************************/
struct file_pak *file_type_from_id(gint id)
{
GSList *file_list;
struct file_pak *file;

/* search */
for (file_list=sysenv.file_list ; file_list ; file_list=g_slist_next(file_list))
  {
  file = file_list->data;
/* go through all extensions listed under this file type */
  if (file->id == id)
    return(file);
  }
return(NULL);
}

/*************************************/
/* file type determination primitive */
/*************************************/
struct file_pak *file_type_from_label(const gchar *text)
{
gint len;
GSList *file_list;
struct file_pak *file;

if (!text)
  return(NULL);

len = strlen(text);
if (!len)
  return(NULL);

/* search */
for (file_list=sysenv.file_list ; file_list ; file_list=g_slist_next(file_list))
  {
  file = file_list->data;
/* go through all extensions listed under this file type */
  if (strlen(file->label) == len)
    {
    if (g_ascii_strcasecmp(text, file->label) == 0)
      return(file);
    }
  }
return(NULL);
}

/*************************************/
/* file type determination primitive */
/*************************************/
struct file_pak *file_type_from_extension(const gchar *text)
{
gint len;
const gchar *tmp;
GSList *file_list, *ext_list;
struct file_pak *file;

if (!text)
  return(NULL);

len = strlen(text);
if (!len)
  return(NULL);

/* search */
for (file_list=sysenv.file_list ; file_list ; file_list=g_slist_next(file_list))
  {
  file = file_list->data;
/* go through all extensions listed under this file type */
  for (ext_list=file->ext ; ext_list ; ext_list=g_slist_next(ext_list))
    {
    tmp = ext_list->data;
    if (strlen(tmp) == len)
      {
      if (g_ascii_strcasecmp(text, tmp) == 0)
        return(file);
      }
    }
  }
return(NULL);
}

/*************************************/
/* file type determination primitive */
/*************************************/
struct file_pak *file_type_from_filename(const gchar *filename)
{
gchar *text;
struct file_pak *file;

if (!filename)
  return(NULL);

#if __WIN32
gchar *tmp = g_shell_unquote(filename, NULL);
text = file_extension_get(tmp);
g_free(tmp); 
#else
text = file_extension_get(filename);
#endif

file = file_type_from_extension(text);

g_free(text);

return(file);
}

/************************************************/
/* routine to determine if a file is recognized */
/************************************************/
/* returns pointer to file info if found, NULL otherwise */
#define DEBUG_GET_FILE_INFO 0
struct file_pak *get_file_info(gpointer ptr, gint type)
{
gint code=-1;
gchar *text=NULL, *ext;
GSList *file, *ext_list;
struct file_pak *file_data;

/* checks */
g_return_val_if_fail(ptr != NULL, NULL);

/* init for search */
switch(type)
  {
  case BY_LABEL:
    text = g_strdup(ptr);

#if DEBUG_GET_FILE_INFO
printf("Searching for type [%s]\n", text);
#endif
    break;

  case BY_EXTENSION:
/* fix to get around the quotes needed to properly pass win32 filesnames with spaces */
#if __WIN32
    {
    gchar *tmp = g_shell_unquote(ptr, NULL);
    text = file_extension_get(tmp);
    g_free(tmp); 
    }
#else
    text = file_extension_get(ptr);
#endif

    if (!text)
      return(NULL);

#if DEBUG_GET_FILE_INFO
printf("Searching for extension [%s]\n", text);
#endif
    break;

  case BY_FILE_ID:
    code = GPOINTER_TO_INT(ptr);
#if DEBUG_GET_FILE_INFO
printf("Searching for code [%d]\n", code);
#endif
    break;
  }

/* search */
for (file=sysenv.file_list ; file ; file=g_slist_next(file))
  {
  file_data = file->data;

  switch (type)
    {
/* compare to all extensions in list */
    case BY_EXTENSION: 
/* go through all extensions listed under this file type */
      for (ext_list=file_data->ext ; ext_list ; ext_list=g_slist_next(ext_list))
        {
        ext = ext_list->data;
        if (strlen(text) == strlen(ext))
          {
          if (g_ascii_strcasecmp(text, ext) == 0)
            {
#if DEBUG_GET_FILE_INFO
printf("Matched: %s\n", file_data->label);
#endif
            g_free(text);
            return(file_data);
            }
          }
        }
      break;

/* compare with label */
    case BY_LABEL:
      if (strlen(text) != strlen(file_data->label))
        break;
      if (g_ascii_strcasecmp(text, file_data->label) == 0)
        {
#if DEBUG_GET_FILE_INFO
printf("Matched: %s\n", file_data->label);
#endif
        g_free(text);
        return(file_data);
        }
      break;

    case BY_FILE_ID:
      if (code == file_data->id)
        {
#if DEBUG_GET_FILE_INFO
printf("Matched: %s\n", file_data->label);
#endif
        return(file_data);
        }
      break;


    default:
      printf("get_file_info() error: bad search type.\n");
    }
  }

if (text)
  g_free(text);

return(NULL);
}

/**************************/
/* detect valid filetypes */
/**************************/
gint file_extension_valid(const gchar *name)
{
gchar *ext;
GSList *item, *list;
struct file_pak *file;

/* locate the extension */
 /*FIXME: can find_char be replaced by standard strchr()/strrchr() functions?
   - MW */
ext = find_char(name, '.', LAST);
if (ext)
  ext++;

if (sysenv.file_type == DATA) 
  /* compare against all extension types - any match is allowed */
  {
  if (!ext) /* files without extensions are matched only in strict case */
    return(FALSE);  
  for (list=sysenv.file_list ; list ; list=g_slist_next(list))
    {
    file = list->data; 
    for (item=file->ext ; item ; item=g_slist_next(item))
      if (g_ascii_strcasecmp(item->data, ext) == 0)
        return(TRUE);
    }
  }
else
  /* strict case - file must match the dialog filter */
  {
  for (list=sysenv.file_list ; list ; list=g_slist_next(list))
    {
    file = list->data; 
    if (sysenv.file_type == file->group)
        for (item=file->ext ; item ; item=g_slist_next(item))
            if ((ext && g_ascii_strcasecmp(item->data, ext) == 0)
                || (!ext && strlen(item->data) == 0))
              return(TRUE);
    }
  }

return(FALSE);
}

/**************************/
/* get an unused filename */
/**************************/
#define DEBUG_FILE_UNUSED 0
gchar *file_unused_name(const gchar *dir, const gchar *prefix, const gchar *ext)
{
gint i;
gchar *tmp=NULL, *fullpath=NULL;
FILE *fp;

/* defaults on NULL arguments */
if (!dir)
  dir = sysenv.cwd;
if (!prefix)
  prefix = "dummy";
if (!ext)
  ext = "txt";

/* seek a file name that doesn't exist (avoid background overwriting) */
i=0;
do
  {
/* on exit we leave this allocated, so free already used filenames (if any) */
  g_free(fullpath);

  tmp = g_strdup_printf("%s_%d.%s", prefix, i, ext);
  fullpath = g_build_filename(dir, tmp, NULL);
  g_free(tmp);

#if DEBUG_FILE_UNUSED
printf("testing: %s\n", fullpath);
#endif

  i++;
  }
while (g_file_test(fullpath, G_FILE_TEST_EXISTS));

g_assert(fullpath != NULL);

/* create the file to prevent another process from taking it */
/* b4 the current caller gets around to writing anything to it */
fp = fopen(fullpath, "wt");
if (fp)
  {
  fprintf(fp, "locked.\n");
  fclose(fp);
  }
else
  return(NULL);

return(fullpath);
}

/*******************************************************/
/* get an unused BASENAME (of supplied extension type) */
/*******************************************************/
/* TODO - supply a basename+ext & this inserts _? until new? */
#define DEBUG_GUN 0
gchar *gun(const gchar *ext)
{
gint i;
gchar *name;
GString *filename;
FILE *fp;

/* seek a file name that doesn't exist (avoid background overwriting) */
filename = g_string_new(NULL);
i=0;
do
  {
  g_string_sprintf(filename,"dummy_%d.%s", i, ext);
#if DEBUG_GUN
printf("testing: %s\n", filename->str);
#endif
  i++;
  }
while (g_file_test(filename->str, G_FILE_TEST_EXISTS));

/* create the file to prevent another process from taking it */
/* b4 the current caller gets around to writing anything to it */
fp = fopen(filename->str, "wt");
if (fp)
  {
  fprintf(fp, "locked.\n");
  fclose(fp);
  }
else
  {
  printf("Fatal error in gun()\n");
  return(NULL);
  }

name = g_string_free(filename, FALSE);

#if DEBUG_GUN
printf("using base: %s\n", name);
#endif

return(name);
}

/************************************************/
/* generate a new (unused) directory to work in */
/************************************************/
gchar *file_unused_directory(void)
{
gint i;
gchar *filename=NULL, *directory=NULL;

/* a bit naughty ... */
for (i=0 ; i<999999999 ; i++)
  {
  filename = g_strdup_printf("tmp_%d", i);
  directory = g_build_filename(sysenv.cwd, filename, NULL);

//printf("testing: %s\n", directory);

  if (!g_file_test(directory, G_FILE_TEST_EXISTS))
    break;

  g_free(directory);
  g_free(filename);
  }

//printf("creating: %s\n", directory);

g_mkdir(directory, 0700);

return(directory);
}

/**************************************************************/
/* correct numbers in binary files with reverse byte ordering */
/**************************************************************/
void swap_bytes(void *ptr, const gint size)
{
gint i,j;
gchar tmp;

/*
printf("start: ");
for (i=0 ; i<size ; i++)
  printf("%d ", *((char *)(ptr+i)));
printf("\n");
*/
j=size-1;
for (i=0 ; i<size/2 ; i++)
  {
  tmp = *((gchar *)(ptr+j));
  *((gchar *)(ptr+j)) = *((gchar *)(ptr+i));
  *((gchar *)(ptr+i)) = tmp;
  j--;
  }
/*
printf(" stop: ");
for (i=0 ; i<size ; i++)
  printf("%d ", *((char *)(ptr+i)));
printf("\n");
*/
}

/**************************************/
/* get a non trivial line from a file */
/**************************************/
gint fgetline(FILE *fp, gchar *line)
{
gint i, linlen;

for(;;)
  {
/* get a line */
  if (fgets(line, LINELEN/2, fp) == NULL)
    return(1);
  linlen = strlen(line);
/* only treated as data if not a comment and non empty */
  if (line[0] != '#' && linlen)
    {
/* ampersand concatenation */
/* TODO - extra var in fgetline() (eg mode) that requests this */
/* TODO - handle multiple line concatenation */
    for (i=linlen ; i-- ; )
      {
      if (line[i] == '&')
        {
        if (fgets(&line[linlen], LINELEN/2, fp) == NULL)
          return(1);
        break;
        }
      }
    break;
    }
  }

/* all clear */
return(0);
}

/*****************************************/
/* read in and return a line of any size */
/*****************************************/
gint skip_comment = TRUE;
void file_skip_comment(gint skip)
{
skip_comment = skip;
}
/* returns NULL on EOF */
/* returned string should be freed when finished */
gchar *file_read_line(FILE *fp)
{
gboolean skip;
gchar c;
GString *buff;

/* checks */
g_assert(fp != NULL);
c = fgetc(fp);
if (c == EOF)
  return(NULL);

/* read single chars into an expandable buffer */
buff = g_string_new(NULL);
while (c != EOF)
  {
  skip = FALSE;
  if (c == '#' && skip_comment)
    skip = TRUE;

/* read a complete line */
  while (c != EOF)
    {
    g_string_append_c(buff, c);
    if (c == '\n')
      break;
    if (c == '\r')
      {
      gchar d;

      d = fgetc(fp);
      if (d == '\n')
        {
/*
        printf("skipping MSDOS crap.\n");
*/
        }
      else
        ungetc(d, fp);

      break;
      }
    c = fgetc(fp);
    }
/* ignore? (keep reading) */
  if (skip)
    {
    g_string_assign(buff, "");
    c = fgetc(fp);
    }
  else
    break;
  }

/* create a properly terminated line of text */
g_string_append_c(buff, '\0');

/* free the GString, but not the text */
return(g_string_free(buff, FALSE));
}

/**********************************************************/
/* format value as a float printed to dp places plus unit */
/**********************************************************/
gchar *format_value_and_units(gchar *string, gint dp)
{
gint num_tokens;
GString *text, *format_string;
gchar *text_str, *units, **buff;
gdouble float_val;

float_val = str_to_float(string);
buff = tokenize(string, &num_tokens);
if (num_tokens < 2)
  units = "";
else
  units = *(buff+1);
format_string = g_string_new("");
g_string_append_printf(format_string, "%%.%df %%s", dp);
text = g_string_new("");
g_string_append_printf(text, format_string->str, float_val, units);
text_str = text->str;
g_string_free(format_string, TRUE); 
g_string_free(text, FALSE); 
g_strfreev(buff);

return (text_str);
}

/***********************************/
/* handler for normal file loading */
/***********************************/
#define DEBUG_FILE_LOAD 0
void file_load(gchar *filename, struct model_pak *mdata)
{
gint j;
gint flag, status=-1;
gchar *fullname;
GSList *list;
struct model_pak *data;
struct file_pak *file_data;

#if DEBUG_FILE_LOAD
printf("loading: [%s] into: %p\n", filename, mdata);
#endif

/* FIXME - new error reporting - doesnt work for gulp */
error_table_clear();
error_table_enable();

/* get the file structure required to parse the file */
switch (sysenv.file_type)
  {
/* cases for which duplicate extensions occur */
  case RIETICA:
  case GAMESS:
  case DLPOLY:
    file_data = get_file_info(GINT_TO_POINTER(sysenv.file_type), BY_FILE_ID);
    break;

/* cases for which extensions must be used to determine type (eg .gin/.got) */
  default:
    file_data = file_type_from_filename(filename);
  }
if (!file_data)
  return;

#if DEBUG_FILE_LOAD
printf("Using file load routine for: [%s] files.\n", file_data->label);
#endif

if (mdata)
  data = mdata;
else
  {
/* get the new model number */
  data = model_new();
  if (!data)
    {
    gui_text_show(ERROR, "Model memory allocation failed.\n");
    return;
    }
  }

data->id = file_data->id;

/* read file if exists, else try prepending current working directory */
if (file_data->read_file)
  {
  if (g_path_is_absolute(filename))
    {
    status = file_data->read_file(filename, data);
    }
  else
    {
    fullname = g_build_filename(sysenv.cwd, filename, NULL);
    status = file_data->read_file(fullname, data);
    g_free(fullname);
    }
  }
else
  gui_text_show(ERROR, "No read routine for this type. ");

/* check for successful file load */
if (status)
  {
  gui_text_show(ERROR, "Load failed.\n");

printf("Load failed, error code: %d\n", status);

  model_delete(data);
  }
else
  {
/* we don't know how many new models were loaded so */
/* scan through them all & check for initialization */
  for (list=sysenv.mal ; list ; list=g_slist_next(list))
    {
    data = list->data;

/* skip if already on the tree */
    if (data->grafted)
      continue;


/* NEW - big model display mode exceptions */
  if (data->num_atoms > 1000)
    core_render_mode_set(STICK, data->cores);

/* surfaces are always conv by default */
    if (data->periodic == 2)
      data->gulp.method = CONV;
/* not on tree - must have just been loaded */
    tree_model_add(data);

/* create gulp supercells */
    flag=0;
    for (j=0 ; j<3 ; j++)
      {
      if (data->gulp.super[j] > 1)
        {
        data->image_limit[2*j+1] = data->gulp.super[j];
        data->expected_cores *= data->gulp.super[j];
        data->expected_shells *= data->gulp.super[j];
        flag++;
        }
      }
    if (flag)
      {
      space_make_images(CREATE, data);
      space_make_supercell(data);
      model_prep(data);
      }

/* NEW - store initial frame, as trajectory animation will overwrite */
if (data->gulp.trj_file && !data->gulp.orig_cores)
  {
/* FIXME - saving fractional type here instead of at the end of read_gulp() stuffs things up  - why? */
/* NB: we need to save the coords here (and not earlier) in case a supercell was created */

  memcpy(data->gulp.orig_pbc, data->pbc, 6*sizeof(gdouble));
  memcpy(data->gulp.orig_latmat, data->latmat, 9*sizeof(gdouble));
  data->gulp.orig_cores = dup_core_list(data->cores);
  data->gulp.orig_shells = dup_shell_list(data->shels);
  }

    }
  }

/* finished - only now we destroy the file dialog */
dialog_destroy_type(FILE_SELECT);
tree_select_active();

error_table_print_all();
}

/***********************/
/* save file with name */
/***********************/
/* deprec */
gint file_save_as(gchar *filename, struct model_pak *model)
{
gint id, ret;
gchar *name;
struct file_pak *file_data;

/* setup & checks */
if (!model)
  model = tree_model_get();

if (!filename || !model)
  return(1);

file_data = file_type_from_filename(filename);

if (file_data)
  {
/* use extension */
  id = file_data->id;

  g_free(model->filename);
  model->filename = g_strdup(filename);

  g_free(model->basename);
  model->basename = parse_strip(filename);
  }
else
  {
  gchar *ext;

/* no extension - attempt to use filter type */
  file_data = get_file_info(GINT_TO_POINTER(sysenv.file_type), BY_FILE_ID);
  if (!file_data)
    return(2);
  ext = g_slist_nth_data(file_data->ext, 0);
  if (!ext)
    return(2);

  id = file_data->id;
  g_free(model->basename);
  model->basename = parse_strip(filename);

  g_free(model->filename);
  model->filename = g_strdup_printf("%s.%s", model->basename, ext);
  }

/* file info indicates routine to call */
if (file_data->write_file == NULL)
  {
  printf("No write routine for this type.\n");
  return(3);
  }
else
  {
/* build the full name */
  name = g_build_filename(sysenv.cwd, model->filename, NULL);
  ret = file_data->write_file(name, model);
  g_free(name);
  }

/* update */
if (ret)
  gui_text_show(ERROR, "Save failed.\n");
else
  {
  gui_text_show(STANDARD, "Saved model.\n");
  dialog_destroy_type(FILE_SELECT);
  tree_model_refresh(model);
  redraw_canvas(SINGLE);
  }

return(ret);
}

/*************************************/
/* save active model using same name */
/*************************************/
void file_save(void)
{
gchar *name;
struct model_pak *model;

model = tree_model_get();
if (!model)
  return;

/* strip the path, as file_save_as() prepends the cwd */
name = g_path_get_basename(model->filename);
file_save_as(name, model);
g_free(name);
}

/***********************************/
/* get time last modifed as string */
/***********************************/
const gchar *file_modified_string(const gchar *fullpath)
{
struct stat buff;

if (!g_stat(fullpath, &buff))
  return(time_string(&(buff.st_mtime))); 

return("unknown");
}

/*************************************/
/* get the size of a file (in bytes) */
/*************************************/
gint file_byte_size(const gchar *filename)
{
struct stat buff;

stat(filename, &buff);

return(buff.st_size);
}

/************************************/
/* search for an executable program */
/************************************/
/* TODO - put elsewhere ... look for a program ... ask for a path (can cancel) if not found */
gchar *file_find_program(const gchar *name)
{
gchar *path;

path = g_find_program_in_path(name);

return(path);
}

/***************************************************/
/* check a directory and create if it doesnt exist */
/***************************************************/
gint file_mkdir(const gchar *fullpath)
{
if (g_file_test(fullpath, G_FILE_TEST_EXISTS))
  {
/* check if it's a directory, if not - fail */
  if (g_file_test(fullpath, G_FILE_TEST_IS_DIR))
    {
//printf("path already exists.\n");
    return(TRUE);
    }
  else
    {
//printf("file_mkdir() ERROR: path exists and is a file.\n");
    return(FALSE);
    }
  }
else
  {
#if WIN32
if (mkdir(fullpath))
#else
if (mkdir(fullpath, 0700))
#endif
    {
    printf("Failed to create: [%s], check permissions.\n", fullpath);
    return(FALSE);
    }
  }
return(TRUE);
}

/******************************************/
/* change cwd to reflect an offset change */
/******************************************/
gchar *file_path_delta(const gchar *dir, const gchar *offset)
{
gint i, n;
gchar *path=NULL, *last;

/* special case */
if (g_ascii_strncasecmp("..", offset, 2) == 0)
  {
  n = strlen(dir);
  last = g_strrstr_len(dir, n, DIR_SEP);
  if (last)
    {
    i = strlen(last);
    path = g_strndup(dir, n-i); 
    }
  }
else
  path = g_build_filename(dir, offset, NULL);

if (g_file_test(path, G_FILE_TEST_IS_DIR))
  return(path);

g_free(path);
return(NULL);
}

/***********************************/
/* change to a specified directory */
/***********************************/
#define DEBUG_SET_PATH 0
gint set_path(const gchar *txt)
{
gint i, n, status=0;
gchar **buff, *text;
GString *path;

/* split by directory separator */
text = g_strdup(txt);
g_strstrip(text);
buff = g_strsplit(text, DIR_SEP, 0);
path = g_string_new(NULL);

/* find the number of tokens */
n=0;
while(*(buff+n))
  {
#if DEBUG_SET_PATH
printf("[%s] ", *(buff+n));
#endif
  n++;
  }
#if DEBUG_SET_PATH
printf(": found %d tokens.\n", n);
#endif

/* truncate token list if parent directory selected */
if (n > 1)
  if (g_ascii_strncasecmp("..",*(buff+n-1),2) == 0)
    n -= 2;

/* build new path */
/* this is a bit fiddly, as we don't want a dir_sep on the end */
/* EXCEPT if it's the root directory (blame windows for this) */
g_string_sprintf(path, "%s", *buff);
i=1;
while (i < n)
  {
  g_string_sprintfa(path, "%s%s", DIR_SEP, *(buff+i));
  i++;
  }
if (n < 2)
  g_string_sprintfa(path, "%s", DIR_SEP);

#if DEBUG_SET_PATH
printf("testing path [%s] ... \n", path->str); 
#endif

if (g_file_test(path->str, G_FILE_TEST_IS_DIR))
  {
  g_free(sysenv.cwd);
  sysenv.cwd = g_strdup(path->str);
  }
else
  {
#if DEBUG_SET_PATH
printf(" Not a directory.\n");
#endif
  status++;
  }

g_free(text);
g_strfreev(buff);
g_string_free(path, TRUE);

return(status);
}

/**************************/
/* minimal import routine */
/**************************/
gint file_text_import(gpointer import)
{
g_assert(import != NULL);

/* allow through -> show content will be able to display the text */

return(0);
}

/********************/
/* simple file copy */
/********************/
gint file_copy(const gchar *source, const gchar *dest)
{
gchar c;
FILE *src, *dst;

/* checks */
g_assert(source != NULL);
g_assert(dest != NULL);

/* open read/write streams */
src = fopen(source, "rb");
if (!src)
  return(FALSE);

dst = fopen(dest, "wb");
if (!dst)
  {
  fclose(src);
  return(FALSE);
  }

/* char by char copy */
for (;;)
  {
  c = fgetc(src);
  if (feof(src))
    break;

  fputc(c, dst);
  }

/* cleanup */
fclose(src);
fclose(dst);

return(TRUE);
}

/**************************/
/* search for executables */
/**************************/
void file_path_find(void)
{
/* step 1 - clear path if it's a dud */
if (sysenv.babel_path)
  {
  if (!g_file_test(sysenv.babel_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.babel_path);
    sysenv.babel_path = NULL;
    }
  }
if (sysenv.convert_path)
  {
  if (!g_file_test(sysenv.convert_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.convert_path);
    sysenv.convert_path = NULL;
    }
  }
if (sysenv.povray_path)
  {
  if (!g_file_test(sysenv.povray_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.povray_path);
    sysenv.povray_path = NULL;
    }
  }
if (sysenv.viewer_path)
  {
  if (!g_file_test(sysenv.viewer_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.viewer_path);
    sysenv.viewer_path = NULL;
    }
  }
if (sysenv.ffmpeg_path)
  {
  if (!g_file_test(sysenv.ffmpeg_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.ffmpeg_path);
    sysenv.ffmpeg_path = NULL;
    }
  }
if (sysenv.gamess_path)
  {
  if (!g_file_test(sysenv.gamess_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.gamess_path);
    sysenv.gamess_path = NULL;
    }
  }
if (sysenv.gulp_path)
  {
  if (!g_file_test(sysenv.gulp_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.gulp_path);
    sysenv.gulp_path = NULL;
    }
  }
if (sysenv.siesta_path)
  {
  if (!g_file_test(sysenv.siesta_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.siesta_path);
    sysenv.siesta_path = NULL;
    }
  }
if (sysenv.reaxmd_path)
  {
  if (!g_file_test(sysenv.reaxmd_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.reaxmd_path);
    sysenv.reaxmd_path = NULL;
    }
  }
if (sysenv.monty_path)
  {
  if (!g_file_test(sysenv.monty_path, G_FILE_TEST_EXISTS))
    {
    g_free(sysenv.monty_path);
    sysenv.monty_path = NULL;
    }
  }

/* get executable paths */
if (!sysenv.babel_path)
  sysenv.babel_path = g_find_program_in_path(sysenv.babel_exe);
if (!sysenv.convert_path)
  sysenv.convert_path = g_find_program_in_path(sysenv.convert_exe);
if (!sysenv.gulp_path)
  sysenv.gulp_path = g_find_program_in_path(sysenv.gulp_exe);
if (!sysenv.gamess_path)
  sysenv.gamess_path = g_find_program_in_path(sysenv.gamess_exe);
if (!sysenv.siesta_path)
  sysenv.siesta_path = g_find_program_in_path(sysenv.siesta_exe);
if (!sysenv.reaxmd_path)
  sysenv.reaxmd_path = g_find_program_in_path(sysenv.reaxmd_exe);
if (!sysenv.monty_path)
  sysenv.monty_path = g_find_program_in_path(sysenv.monty_exe);
if (!sysenv.povray_path)
  sysenv.povray_path = g_find_program_in_path(sysenv.povray_exe);
if (!sysenv.ffmpeg_path)
  sysenv.ffmpeg_path = g_find_program_in_path(sysenv.ffmpeg_exe);
if (!sysenv.viewer_path)
  sysenv.viewer_path = g_find_program_in_path(sysenv.viewer_exe);
if (!sysenv.viewer_path)
  sysenv.viewer_path = g_find_program_in_path("eog");

/* HACK FIX - GAMESS path contains the path, not the path + file */
/*
temp = g_find_program_in_path(sysenv.gamess_exe);
if (temp)
  sysenv.gamess_path = g_path_get_dirname(temp);
else
#if __WIN32
  sysenv.gamess_path = g_strdup("c:\\wingamess");
#else
  sysenv.gamess_path = NULL;
#endif
g_free(temp);
*/
}


/********************************/
/* get directory tree structure */
/********************************/
// recursive, parent=NULL on first call
GNode *file_directory_tree(GNode *parent, const gchar *path)
{
gchar *fullpath;
const gchar *name;
GDir *dir;
GError *error=NULL;
GNode *child=NULL;

if (!parent)
  parent = g_node_new(g_strdup(path));

dir = g_dir_open(path, 0, &error);

if (error)
  {
  fprintf(stderr, "%s\n", error->message);
  return(NULL);
  }

if (dir)
  {
  name = g_dir_read_name(dir);
  while (name)
    {
    fullpath = g_build_filename(path, name, NULL);
    if (g_file_test(fullpath, G_FILE_TEST_IS_DIR))
      {
/* add subdir to parent */
      child = g_node_append_data(parent, g_strdup(fullpath));
/* recursively add subdirs of subdirs */
      file_directory_tree(child, fullpath);
      }
    g_free(fullpath);
    name = g_dir_read_name(dir);
    }
  g_dir_close(dir);
  }

return(parent);
}

/*****************************************/
/* plain traversal for freeing node data */
/*****************************************/
gboolean file_traverse(GNode *node, gpointer data)
{
g_free(node->data);
return(FALSE);
}

/******************************************/
/* free complete directory tree structure */
/******************************************/
void file_directory_tree_free(GNode *tree)
{
g_node_traverse(tree, G_IN_ORDER, G_TRAVERSE_ALL, -1, file_traverse, NULL);
g_node_destroy(tree);
}

#define DEBUG_REMOVE_DIRECTORY 0

/**********************************************************/
/* remove all files (and then directory) at a given level */
/**********************************************************/
gboolean file_remove_level(GNode *node, gpointer data)
{
gint depth;
gchar *fullpath;
const gchar *path, *name;
GDir *dir;
GError *error=NULL;

depth = GPOINTER_TO_INT(data);

if (depth == g_node_depth(node))
  {
/* open directory */
  path = node->data;
  dir = g_dir_open(path, 0, &error);
  if (error)
    {
    fprintf(stderr, "%s\n", error->message);
    return(FALSE);
    }

/* remove all files */
// NB: it would be an error in the node tree (or navigation thereof) to encounter a directory 
  if (dir)
    {
    name = g_dir_read_name(dir);
    while (name)
      {
      fullpath = g_build_filename(path, name, NULL);
#if DEBUG_REMOVE_DIRECTORY
printf("Removing: %s\n", fullpath);
#endif
      g_remove(fullpath);
      g_free(fullpath);
      name = g_dir_read_name(dir);
      }
    g_dir_close(dir);
    }

/* remove directory itself */
#if DEBUG_REMOVE_DIRECTORY
printf("Removing: %s\n", path);
#endif
  g_remove(path);
  }

return(FALSE);
}

/**********************************************************************/
/* remove a directory including sub-directories (recurse from bottom) */
/**********************************************************************/
/* platform independent */
gint file_remove_directory(const gchar *path)
{
gint i, max;
GNode *tree;

#if DEBUG_REMOVE_DIRECTORY
printf("file_remove_directory(): %s\n", path);
#endif

tree = file_directory_tree(NULL, path);
max = g_node_max_height(tree);

#if DEBUG_REMOVE_DIRECTORY
printf("Max tree depth = %d\n", max);
#endif

/* TODO - don't really understand why top level is depth=1 rather than depth=0 */
for (i=max ; i>0 ; i--)
  {
#if DEBUG_REMOVE_DIRECTORY
printf("Enumerating nodes at level: %d\n", i);
#endif
  g_node_traverse(tree, G_IN_ORDER, G_TRAVERSE_ALL, -1, file_remove_level, GINT_TO_POINTER(i));
  }

// free node data
file_directory_tree_free(tree);

return(TRUE);
}

/* structure for transfering files in the background */
// ie partition the process of file transfers
struct file_transfer_pak
{
gint method;
gpointer import;
gchar *destination;
gchar *source;
};

/**************************************************************/
/* create a new file transfer structure for grisu downloading */
/**************************************************************/
gpointer file_import_download(gpointer import)
{
const gchar *filename, *remote_path;
struct file_transfer_pak *ftp;

if (!import)
  return(NULL);

filename = import_filename(import);
if (!filename)
  return(NULL);

remote_path = import_remote_path_get(import);
if (!remote_path)
  return(NULL);

ftp = g_malloc(sizeof(struct file_transfer_pak));

// TODO - allow different methods (local copy, gridftp, grisu xfer etc)
ftp->method = 1;
// NB: this is a soft reference to the original import ... and does not guarantee the import exists
// use correct testing primitives (below) to see if the file transfer's assoc import still exists
ftp->import = import;
ftp->destination = g_build_filename(import_path(import), filename, NULL);
ftp->source = g_build_filename(remote_path, filename, NULL);

//printf("xfr: %s -> %s\n", ftp->source, ftp->destination);

return(ftp);
}

/*****************************************************************************/
/* return the import associated with a file transfer IFF it's still resident */
/*****************************************************************************/
gpointer file_transfer_import_get(gpointer data)
{
GSList *list;
struct file_transfer_pak *ftp=data;

if (!ftp)
  return(NULL);

if (!ftp->import)
  return(NULL);

/* only return the referenced import if still exists in the workspace */
list = project_imports_get(sysenv.workspace);
if (g_slist_find(list, ftp->import))
  return(ftp->import); 

return(NULL);
}

/************************************************/
/* thread safe mechanism for transferring files */
/************************************************/
void file_transfer_start(gpointer data)
{
struct file_transfer_pak *ftp=data;

if (!ftp)
  return;

switch (ftp->method)
  {
#ifdef WITH_GRISU
  case 1:
    grisu_file_download(ftp->source, ftp->destination);
#endif
    break;

  default:
    printf("xfr[%d]: %s -> %s\n", ftp->method, ftp->source, ftp->destination);
  }
}

/********************************/
/* free file transfer structure */
/********************************/
void file_transfer_free(gpointer data)
{
struct file_transfer_pak *ftp=data;

if (!ftp)
  return;

g_free(ftp->source);
g_free(ftp->destination);
g_free(ftp);
}

