/*
Copyright (C) 2008 by Joerg Meyer

meyer@fhi-berlin.mpg.de

[based on file_dmol.c]

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

#include "gdis.h"
#include "coords.h"
#include "matrix.h"
#include "model.h"
#include "parse.h"
#include "file.h"
#include "scan.h"
#include "interface.h"

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/****************/
/* file writing */
/****************/
gint write_aims(gchar *filename, struct model_pak *model)
{
gint i;
gdouble x[3];
GSList *list;
struct core_pak *core;
FILE *fp;

/* checks */
g_return_val_if_fail(model != NULL, 1);
g_return_val_if_fail(filename != NULL, 2);

/* open the file */
fp = fopen(filename,"wt");
if (!fp) return(3);

if (model->periodic == 3) {

  /* TODO: check whether lattice vectors in AIMS are transposed with respect GDIS */
  for (i=0 ; i<3 ; i++) {
    fprintf(fp,"lattice_vector %14.8f %14.8f %14.8f \n", 
            model->latmat[0+i], model->latmat[3+i], model->latmat[6+i]);
  }
}

for (list=model->cores ; list ; list=g_slist_next(list)) {

  core = list->data;
  if (core->status & DELETED) continue;

  /* everything is cartesian after latmat mult */
  ARR3SET(x, core->x);
  vecmat(model->latmat, x);
  fprintf(fp,"          atom %14.8f %14.8f %14.8f %3s \n",
          x[0], x[1], x[2], elements[core->atom_code].symbol);
}

fclose(fp);
return(0);
}

/****************/
/* file reading */
/****************/
gint read_aims(gchar *filename, struct model_pak *model)
{
gint i, num_tokens;
gchar **buff;
gpointer scan;
struct core_pak *core;

g_assert(model != NULL);

scan = scan_new(filename);
if (!scan) return(1);

while (!scan_complete(scan)) {

  buff = scan_get_tokens(scan, &num_tokens);

/* 
   for debugging purposes 
   produces a compiler warning about an
	implicit declaration of function 'g_printf'
   though this is a valid glib function since 2.2
	http://library.gnome.org/devel/glib/2.18/glib-String-Utility-Functions.html#g-printf
*/
/*
  for (i=0; i<num_tokens; i++) {
	g_printf(" %s ", buff[i]);
  }
  printf("\n");
*/

  if (!buff) break;

  /* read cell vectors */
  if ( g_strrstr(*buff, "lattice_vector") != NULL ) {
     for (i=0 ; i<3 ; i++) {
         if (num_tokens >= 3) {
            model->latmat[i] = str_to_float(*(buff+1));
            model->latmat[i+3] = str_to_float(*(buff+2));
            model->latmat[i+6] = str_to_float(*(buff+3));
         }
         else { 
            gui_text_show(ERROR, "error reading AIMS lattice vectors \n"); 
            return(2);
         }
         g_strfreev(buff);
         buff = scan_get_tokens(scan, &num_tokens);
     }
     model->periodic = 3;
     model->construct_pbc = TRUE;
  }

  /* read coordinates */
  if ( g_strrstr(*buff, "atom") != NULL ) {
     if ( ( num_tokens >= 4 ) && ( elem_symbol_test(*(buff+4)) ) ) {
        core = new_core(*(buff+4), model);
        core->x[0] = str_to_float(*(buff+1));
        core->x[1] = str_to_float(*(buff+2));
        core->x[2] = str_to_float(*(buff+3));
        model->cores = g_slist_prepend(model->cores, core);
     }
     else {
        gui_text_show(ERROR, "error reading AIMS lattice vectors \n"); 
        return(2);
     }
  }

  g_strfreev(buff);
}

/* done reading */
scan_free(scan);

/* model setup */
g_free(model->filename);
model->filename = g_strdup(filename);

g_free(model->basename);
model->basename = g_path_get_basename(filename);

model->fractional = FALSE;
model_prep(model);

return(0);
}
