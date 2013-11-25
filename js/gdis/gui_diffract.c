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
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "gdis.h"
#include "coords.h"
#include "file.h"
#include "graph.h"
#include "parse.h"
#include "import.h"
#include "project.h"
#include "sginfo.h"
#include "matrix.h"
#include "surface.h"
#include "spatial.h"
#include "task.h"
#include "numeric.h"
#include "interface.h"
#include "dialog.h"
#include "gui_shorts.h"
#include "opengl.h"

/* main pak structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

#define FWHM_GAUSSIAN 0.4246609
/* MEH = m*e*e/2*h*h */
#define MEH 0.026629795

GtkWidget *diff_rad_type, *diff_rad_length;
GtkWidget *diff_broadening;
GtkWidget *diff_filename, *diff_plot;

enum {DIFF_XRAY, DIFF_NEUTRON, DIFF_ELECTRON,
      DIFF_GAUSSIAN, DIFF_LORENTZIAN, DIFF_PSEUDO_VOIGT};

gint diff_all_frames = TRUE;

extern gint gl_fontsize;

struct diffract_pak basic;

/*****************************/
/* eliminate repeated planes */
/*****************************/
/* NB: assumes the input plane list is Dhkl sorted */
GSList *diff_get_unique_faces(GSList *list, struct model_pak *model)
{
int equiv;
gdouble delta, dhkl;
GSList *list1, *list2;
struct plane_pak *plane=NULL, *ref_plane=NULL;

list1=list;

while (list1)
  {
  ref_plane = list1->data;

  list2 = g_slist_next(list1);
  while (list2)
    {
    plane = list2->data;
    list2 = g_slist_next(list2);

    if (facet_equiv(model, ref_plane->index, plane->index))
      {
/* related plane */
      list = g_slist_remove(list, plane);
      g_free(plane);
      ref_plane->multiplicity++;
      } 
    else
      {
// ignore
/*
    ref_plane = plane;
    dhkl = ref_plane->dhkl;
    ref_plane->multiplicity = 1;
*/
      }
    }

  list1 = g_slist_next(list1);
  }

return(list);
}

/**************************/
/* hkl enumeration (exp.) */
/**************************/
/* TODO - merge common functionality with get_ranked_faces() in surface.c */
#define DEBUG_DIFF_RANK 0
GSList *diff_get_ranked_faces(gdouble min, struct model_pak *model)
{
gint h, k, l, hs, ks, ls;
gdouble f1[3];
GSList *list1;
struct plane_pak *plane;

/* FIXME - should we have a Dhkl cutoff instead of a HKL limit?*/
#define HKL_LIMIT 19

/* enumerate all unique hkl within defined limit */
#if DEBUG_DIFF_RANK
printf("trying:\n");
#endif
list1 = NULL;
for (hs=-1 ; hs<2 ; hs+= 2)
  {
/* start at 0 for -ve and at 1 for +ve */
  for (h=(hs+1)/2 ; h<HKL_LIMIT ; h++)
    {
    for (ks=-1 ; ks<2 ; ks+= 2)
      {
      for (k=(ks+1)/2 ; k<HKL_LIMIT ; k++)
        {
        for (ls=-1 ; ls<2 ; ls+= 2)
          {
          for (l=(ls+1)/2 ; l<HKL_LIMIT ; l++)
            {
#if DEBUG_DIFF_RANK
printf("(%d %d %d)", hs*h, ks*k, ls*l);
#endif
            if (!h && !k && !l)
              continue;

	    if (surf_sysabs(model, hs*h, ks*k, ls*l))
	      continue;

#if DEBUG_DIFF_RANK
printf("\n");
#endif

/* add the plane */
            VEC3SET(f1, hs*h, ks*k, ls*l);
            plane = plane_new(f1, model);
	    if (!plane)
	      continue;

            if (plane->dhkl > min)
	      list1 = g_slist_prepend(list1, plane);
	    else
	      break;
            }
          }
        }
      }
    }
  }

/* sort via Dhkl */
list1 = g_slist_sort(list1, (gpointer) dhkl_compare);

/*
{
GSList *list;

for (list=list1 ; list ; list=g_slist_next(list))
  {
  plane = list->data;

printf("s: %d %d %d : %f\n", plane->index[0], plane->index[1], plane->index[2], plane->dhkl);

  }
}
*/

/* remove faces with same Dhkl (NB: assumes they are sorted!) */
list1 = diff_get_unique_faces(list1, model);

return(list1);
}

/************************************/
/* compute atomic scattering factor */
/************************************/
/* FIXME - gain speed by prefetching for unique_atom_list */
gdouble diff_get_sfc(gint type, gdouble sol, struct core_pak *core)
{
gint i;
gboolean flag;
gdouble a, b, sfc, sol2;
GSList *list;

/* FIXME - CU will not match Cu in hash table lookup */
list = g_hash_table_lookup(sysenv.sfc_table, elements[core->atom_code].symbol);
/*
if (!list)
  {
  printf("No sfc found for [%s], fudging...\n", core->atom_label);
  return((gdouble) core->atom_code);
  }

g_assert(g_slist_length(list) > 8);
*/

if (list)
  {

flag = FALSE;
switch (type)
  {
  case DIFF_ELECTRON:
    flag = TRUE;
  case DIFF_XRAY:
/* wave form factor */
    sfc = *((gdouble *) g_slist_nth_data(list, 8));
    sol2 = sol*sol;
    for (i=0 ; i<4 ; i++)
      {
      a = *((gdouble *) g_slist_nth_data(list, 2*i));
      b = *((gdouble *) g_slist_nth_data(list, 2*i+1));
      sfc += a*exp(-b*sol2);
      }
/* electron correction */
    if (flag)
      sfc = MEH * ((gdouble) core->atom_code - sfc) / sol2;
    break;

  case DIFF_NEUTRON:
    sfc = *((gdouble *) g_slist_nth_data(list, 9));
    break;

  default:
    printf("Unsupported radiation type, fudging...\n");
    sfc = core->atom_code;
    break;
  }

  }
else
  {
  sfc = core->atom_code;
  }


/* NEW */
sfc *= core->sof;

//printf("[%s] %d (%f): %f\n", core->atom_label, core->atom_code, core->sof, sfc);

return(sfc);
}

/******************************/
/* diffraction output routine */
/******************************/
#define DEBUG_CALC_SPECTRUM 0
void diff_calc_spectrum(GSList *list, struct model_pak *model)
{
gint i, n;
gint cur_peak, old_peak;
gint import_spectrum;
gchar *filename=NULL;
gdouble angle, sol, f, g, lpf, dr;
gdouble c1, sqrt_c1, sqrt_pi, sina, cosa, sin2a, cos2a, tana, fwhm, bf, intensity;
gdouble *spectrum;
gpointer import, graph, project;
GSList *item;
struct plane_pak *plane;
struct diffract_pak *diffract=&basic;
FILE *fp=NULL;

if (!model)
  return;

/* currently always writes - redo the spectrum check box? */
project = tree_project_get();
if (project)
  {
  filename = file_unused_name(project_path_get(project), "spectrum", "txt");
  fp = fopen(filename, "wt");
  }

/* constant initialization */
c1 = 4.0*log(2.0);
sqrt_c1 = sqrt(c1);
sqrt_pi = sqrt(PI);

/* initialize the output spectrum */
n = 1 + (diffract->theta[1] - diffract->theta[0])/diffract->theta[2];
spectrum = g_malloc(n * sizeof(gdouble));
for (i=0 ; i<n ; i++)
  spectrum[i] = 0.0;

/* TODO - include header info eg crystal, wavelength, etc data */
/* spectrum output/import */
if (fp)
  {
  fprintf(fp, "-----------------------------------------------------------------------\n");
  fprintf(fp, "   hkl     :  m :  Dhkl   : 2theta :  lpf   :     |F|     :      I\n");
  fprintf(fp, "-----------------------------------------------------------------------\n");
  }

/* loop over all spplied peaks */
old_peak = -1;
for (item=list ; item ; item=g_slist_next(item))
  {
  plane = item->data;

/* structure factor squared */
  f = plane->f[0]*plane->f[0] + plane->f[1]*plane->f[1];

/* compute peak position and broadening parameters */
  sol = 0.5/plane->dhkl;
  sina = sol * diffract->wavelength;
  angle = asin(sina);
  cosa = cos(angle);
  angle *= 2.0;
  cos2a = 1 - 2.0*sina*sina;
  sin2a = 2.0*sina*cosa;
  tana = sina/cosa;
  fwhm = sqrt(diffract->w + diffract->v*tana + diffract->u*tana*tana);

/* lorentz polarization factor for powder (unpolarized incident beam) */
/* TODO - include polarization factor (default 0.5) */
  if (diffract->radiation == DIFF_XRAY)
    lpf = 0.5*(1.0 + cos2a*cos2a);
  else
    lpf = 1.0;

  lpf /= (sina * sin2a);

/* determine if the current peak lies on the same bin as the previous peak - this */
/* is the peak overlap problem (ie adding intensities over-emphasizes such peaks) */
  dr = (R2D*angle - diffract->theta[0]) / diffract->theta[2];
/*
  cur_peak = (gint) dr;
*/
  cur_peak = nearest_int(dr);

  old_peak = cur_peak;

/* fill out the spectrum with the current peak */
  for (i=0 ; i<n ; i++)
    {
/* compute distance to peak maximum */

/* absolute distance to peak */
/*
    dr = R2D*angle - (model->diffract.theta[2] * (gdouble) i) - model->diffract.theta[0];
*/

/* quantized distance to peak */
    dr = (gdouble) (i - cur_peak);
    dr *= diffract->theta[2];

/* compute broadening factor - peak contribution at current spectrum interval */
    bf = 1.0 / diffract->theta[2];

    if (fabs(fwhm) > 0.00001)
      {
      switch (diffract->broadening)
        {
        case DIFF_GAUSSIAN:
          bf = sqrt_c1*exp(-c1*dr*dr/(fwhm*fwhm))/(fwhm*sqrt_pi);
          break;

        case DIFF_LORENTZIAN:
          bf = fwhm/(2.0*PI*(dr*dr + 0.25*fwhm*fwhm));
          break;

        case DIFF_PSEUDO_VOIGT:
          g = diffract->asym;
          bf = g * 2.0 / (fwhm * PI * (1.0 + 4.0*(dr*dr/(fwhm*fwhm))));
          bf += (1-g) * sqrt_c1 * exp(-c1*dr*dr/(fwhm*fwhm)) / (fwhm*PI);
          break;

        default:
          g_assert_not_reached();
        }
      }
    else
      {
/* no contribution from peaks with zero FWHM outside their spectrum interval */
      if (cur_peak != i)
        bf = 0.0;
      }

/* compute intensity for the current bin */
    intensity = plane->multiplicity*lpf*f*bf;
    spectrum[i] += intensity;
    }

/* spectrum output/import */
  if (fp)
    {
    fprintf(fp, "[%2d %2d %2d] : %2d : %7.4f : %6.3f : %6.3f : %11.4f : %11.4f\n",
            plane->index[0], plane->index[1], plane->index[2], plane->multiplicity,
            plane->dhkl, R2D*angle, lpf, sqrt(f), plane->multiplicity*lpf*f);
    }
  }

/* create the plots */
if (model->animation && diff_all_frames)
  {
/*
printf("TODO - multiframe animation plotting.\n");
*/
  }
else
  {
/* try to make the 2theta scale nice */
  i = diffract->theta[1] - diffract->theta[0];
  if (i > 49)
    i /= 10;
  else if (i > 9)
    i /= 5;
  i++;

/* create a new graph */
  switch (diffract->radiation)
    {
    case DIFF_ELECTRON:
      graph = graph_new("Electron");
      break;

    case DIFF_NEUTRON:
      graph = graph_new("Neutron");
      break;

    default:
      graph = graph_new("X-Ray");
    }

  graph_add_data(n, spectrum, diffract->theta[0], diffract->theta[1], graph);
  graph_set_yticks(FALSE, 2, graph);
  graph_set_xticks(TRUE, i, graph);
  graph_set_wavelength(diffract->wavelength, graph);

/* get model's parent import */
import = gui_tree_parent_object(model);
if (import)
  {
/* append the graph to the import */
  import_object_add(IMPORT_GRAPH, graph, import);
/* update GUI */
  gui_tree_import_refresh(import);
  }

  }

/* spectrum output/import */
if (fp)
  {
  fclose(fp);

  import = import_file(filename, NULL);
  if (import)
    {
    project_imports_add(project, import);
    gui_tree_append_import(import);
    }

  g_free(filename);
  }


/* TODO - also output/import raw graph data */
/*
fp = fopen(gtk_entry_get_text(GTK_ENTRY(diff_filename)), "at");
if (!fp)
  {
  gui_text_show(ERROR, "Failed to open file for raw spectrum data.\n");
  return;
  }
for (i=0 ; i<n ; i++)
  fprintf(fp, "%5.4f %f\n", diffract->theta[0]+i*diffract->theta[2], spectrum[i]);
fclose(fp);
*/

/* cleanup */
g_free(spectrum);

gui_refresh(GUI_CANVAS);
}

/****************************/
/* main diffraction routine */
/****************************/
#define DEBUG_DIFF_CALC 0
gint diff_calc(void)
{
gdouble dhkl_min, tmp, sfc, sol;
gdouble vec[3], rdv[3];
const gchar *text;
GSList *item, *list, *clist;
struct model_pak *model;
struct plane_pak *plane;
struct core_pak *core;
struct diffract_pak *diffract=&basic;

model = tree_model_get();
if (!model)
  return(1);
if (model->periodic != 3)
  {
  gui_text_show(WARNING, "Your model is not 3D periodic.\n");
  return(1);
  }

/* checks */
if (diffract->theta[0] >= diffract->theta[1])
  {
  gui_text_show(ERROR, "Invalid 2theta range.\n");
  return(2);
  }

text = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(diff_rad_type)->entry));
if (g_ascii_strncasecmp(text, "Electron", 8) == 0)
  diffract->radiation = DIFF_ELECTRON;
if (g_ascii_strncasecmp(text, "Neutron", 7) == 0)
  diffract->radiation = DIFF_NEUTRON;
if (g_ascii_strncasecmp(text, "X-Ray", 5) == 0)
  diffract->radiation = DIFF_XRAY;

/* TODO - use str_is_float() to test first? */
text = gtk_entry_get_text(GTK_ENTRY(diff_rad_length));
diffract->wavelength = fabs(str_to_float(text));

text = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(diff_broadening)->entry));

if (g_ascii_strncasecmp(text, "Gaussian", 8) == 0)
  diffract->broadening = DIFF_GAUSSIAN;
if (g_ascii_strncasecmp(text, "Lorentzian", 10) == 0)
  diffract->broadening = DIFF_LORENTZIAN;
if (g_ascii_strncasecmp(text, "Pseudo-Voigt", 13) == 0)
  diffract->broadening = DIFF_PSEUDO_VOIGT;

/* get min Dhkl for (specified) 2theta max */
dhkl_min = 0.5*diffract->wavelength/tbl_sin(0.5*D2R*diffract->theta[1]);
diffract->dhkl_min = dhkl_min;

/* deal with the sin/lambda < 2.0 scattering restriction */
/* FIXME - check up on the necessity for this */
if (diffract->wavelength < 0.5)
  {
  tmp = R2D * asin(2.0 * diffract->wavelength);
  if (diffract->theta[1] > tmp)
    {
    diffract->theta[1] = tmp;
    gui_text_show(WARNING, "Your maximum theta value is too large.\n");
    }
  if (diffract->theta[0] > tmp)
    {
    diffract->theta[1] = tmp;
    gui_text_show(WARNING, "Your minimum theta value is too large.\n");
    }
  }

#if DEBUG_DIFF_CALC
printf(" radiation = %d\n", diffract->radiation);
printf("wavelength = %f\n", diffract->wavelength);
printf("     theta = %f %f %f\n", diffract->theta[0], diffract->theta[1], diffract->theta[2]);
printf("      fwhm = %f %f %f\n", diffract->u, diffract->v, diffract->w);
printf("  Dhkl min = %f\n", tmp);
#endif

/* NEW - setup for multiframe diffraction */
/*
if (diff_all_frames)
  {
  if (model->animating)
    {
    gui_text_show(ERROR, "Can't do multiple animations.\n");
    return(1);
    }
  if (!model->transform_list)
    {
    model->afp = fopen(model->filename, "r");
    if (!model->afp)
      {
      gui_text_show(ERROR, "Failed to open animation stream.\n");
      return(1);
      }
    }
  model->animating = TRUE;
  }
*/

/* NEW - handle multiple frames */
for (;;)
  {
/* TODO - enforce theta min < max */
/* calculate and loop over all planes satisfying the above */
/* TODO - free the planes??? */
  list = diff_get_ranked_faces(dhkl_min, model);

  for (item=list ; item ; item=g_slist_next(item))
    {
    plane = item->data;

//printf("r: %d %d %d\n", plane->index[0], plane->index[1], plabe->index[2]);

/* sin(angle) / lambda */
    sol = 0.5/plane->dhkl;

/* valid range for scattering coefficients */
/*
    g_assert(sol < 2.0);
*/

    plane->f[0] = 0.0;
    plane->f[1] = 0.0;

    for (clist=model->cores ; clist ; clist=g_slist_next(clist))
      {
      core = clist->data;

/* hkl * reciprocal lattice */
      ARR3SET(rdv, plane->index);
      vecmat(model->rlatmat, rdv);
/* core position */
      ARR3SET(vec, core->x);
      vecmat(model->latmat, vec);
/* 2pi * dot product */
      ARR3MUL(rdv, vec);
      tmp = 2.0 * PI * (rdv[0] + rdv[1] + rdv[2]);

/* get the atomic form factor */
      sfc = diff_get_sfc(diffract->radiation, sol, core);

/* structure factor sum */
      plane->f[0] += sfc*tbl_cos(tmp);
      plane->f[1] += sfc*tbl_sin(tmp);
      }
    }

/* compute/plot intensities */
  diff_calc_spectrum(list, model);
  free_slist(list);

    break;
  }
return(0);
}

/******************************/
/* diffraction peak selection */
/******************************/
#define DEBUG_PEAK_SELECT 0
void diffract_select_peak(gint x, gint y, struct model_pak *model)
{
gdouble ox, dx, xmin, xmax, xval;
gdouble dhkl, d, dmin, lambda;
gpointer graph;
GSList *item, *list;
struct canvas_pak *canvas;
struct plane_pak *plane, *plane_min;

g_assert(model != NULL);

/* TODO - redo using gui_tree ... */
return;

//graph = model->graph_active;
xmin = graph_xmin(graph);
xmax = graph_xmax(graph);
lambda = graph_wavelength(graph);

/* return if not a diffraction pattern */
if (lambda < 0.1)
  return; 

/* FIXME - this will break when we implement multi-canvas drawing */
canvas = g_slist_nth_data(sysenv.canvas_list, 0);
g_assert(canvas != NULL);

/* graph x offset */
ox = canvas->x + 4*gl_fontsize;
if (graph_ylabel(graph))
  ox += 4*gl_fontsize;

/* graph pixel width */
dx = (canvas->width-2.0*ox);

/* get graph x value */
xval = (x - ox)/dx;
xval *= (xmax - xmin);
xval += xmin;

if (xval >= xmin && xval <= xmax)
  {
  dhkl = 0.5 * lambda / sin(0.5*xval*D2R);

#if DEBUG_PEAK_SELECT
printf("Peak seach (%d, %d) : %f (%f) : ", x, y, xval, dhkl);
#endif

  list = diff_get_ranked_faces(model->diffract.dhkl_min, model);

dmin = 1.0;
plane_min = NULL;

/* Dhkl difference based search */
  for (item=list ; item ; item=g_slist_next(item))
    {
    plane = item->data;

    d = fabs(plane->dhkl - dhkl);
    if (d < dmin)
      {
      plane_min = plane;
      dmin = d;
      }
    }

if (plane_min && dmin < 0.1)
  {
  gint gcd, h, k, l;
  gchar *text;

  h = plane_min->index[0];
  k = plane_min->index[1];
  l = plane_min->index[2];
/*
  gcd = GCD(GCD(h, k), GCD(k, l));
*/
  gcd = 1;

  text = g_strdup_printf("(%d %d %d)", h/gcd, k/gcd, l/gcd);
  graph_set_select(xval, text, graph);
  g_free(text);

#if DEBUG_PEAK_SELECT
  printf("Dhkl = %f (%f)\n", plane_min->dhkl, dmin);
#endif
  }
#if DEBUG_PEAK_SELECT
else
  printf("(none)\n");
#endif

  free_slist(list);
  }
}

/****************************/
/* diffraction setup dialog */
/****************************/
void gui_diffract_dialog(void)
{
gpointer dialog;
GtkWidget *window, *frame, *hbox, *hbox2, *vbox, *label;
GtkWidget *table, *spin;
GList *list;

basic.wavelength = 1.54180;
basic.asym = 0.18;
basic.u = 0.0;
basic.v = 0.0;
basic.w = 0.0;
basic.theta[0] = 0.0;
basic.theta[1] = 90.0;
basic.theta[2] = 0.1;

/* request a dialog */
dialog = dialog_request(DIFFAX, "Powder Diffraction", NULL, NULL, NULL);
if (!dialog)
  return;
window = dialog_window(dialog);

/* radiation details */
frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);
vbox = gtk_vbox_new(TRUE, 0);
gtk_container_add(GTK_CONTAINER(frame), vbox);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, PANEL_SPACING);

label = gtk_label_new(" Radiation ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
list = NULL;
list = g_list_prepend(list, "Electrons");
list = g_list_prepend(list, "Neutrons");
list = g_list_prepend(list, "X-Rays");
diff_rad_type = gtk_combo_new();
gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(diff_rad_type)->entry), FALSE);
gtk_combo_set_popdown_strings(GTK_COMBO(diff_rad_type), list);
gtk_box_pack_end(GTK_BOX(hbox), diff_rad_type, FALSE, FALSE, PANEL_SPACING);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, PANEL_SPACING);

label = gtk_label_new(" Wavelength ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

diff_rad_length = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(diff_rad_length),
                   g_strdup_printf("%-9.6f", basic.wavelength));
gtk_box_pack_end(GTK_BOX(hbox), diff_rad_length, FALSE, FALSE, 0);

/* peak broadening */
frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);
vbox = gtk_vbox_new(TRUE, 0);
gtk_container_add(GTK_CONTAINER(frame), vbox);
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, PANEL_SPACING);

label = gtk_label_new(" Broadening function ");
gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

list = NULL;
list = g_list_prepend(list, "Pseudo-Voigt");
list = g_list_prepend(list, "Lorentzian");
list = g_list_prepend(list, "Gaussian");
diff_broadening = gtk_combo_new();
gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(diff_broadening)->entry), FALSE);
gtk_combo_set_popdown_strings(GTK_COMBO(diff_broadening), list);
gtk_box_pack_end(GTK_BOX(hbox), diff_broadening, FALSE, FALSE, PANEL_SPACING);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, PANEL_SPACING);
label = gtk_label_new("Mixing parameter ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, PANEL_SPACING);
spin = gui_spin_new(&basic.asym, 0.0, 1.0, 0.01);
gtk_box_pack_end(GTK_BOX(hbox), spin, FALSE, FALSE, PANEL_SPACING);

/* split pane */
hbox2 = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), hbox2, TRUE, TRUE, 0);

/* angle range */
vbox = gui_frame_vbox(NULL, TRUE, TRUE, hbox2);
table = gtk_table_new(3, 2, FALSE);
gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);

label = gtk_label_new(" 2Theta min ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
label = gtk_label_new(" 2Theta max ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
label = gtk_label_new(" 2Theta step ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);

spin = gui_spin_new(&basic.theta[0], 0.0, 160.0, 1);
gtk_table_attach_defaults(GTK_TABLE(table),spin,1,2,0,1);
spin = gui_spin_new(&basic.theta[1], 0.0, 170.0, 1);
gtk_table_attach_defaults(GTK_TABLE(table),spin,1,2,1,2);
spin = gui_spin_new(&basic.theta[2], 0.01, 1.0, 0.01);
gtk_table_attach_defaults(GTK_TABLE(table),spin,1,2,2,3);


/* U V W broadening parameters */
vbox = gui_frame_vbox(NULL, TRUE, TRUE, hbox2);
table = gtk_table_new(3, 2, FALSE);
gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);

label = gtk_label_new(" U ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
label = gtk_label_new(" V ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
label = gtk_label_new(" W ");
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);

spin = gui_spin_new(&basic.u, -9.9, 9.9, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,1,2,0,1);
spin = gui_spin_new(&basic.v, -9.9, 9.9, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,1,2,1,2);
spin = gui_spin_new(&basic.w, -9.9, 9.9, 0.05);
gtk_table_attach_defaults(GTK_TABLE(table),spin,1,2,2,3);

/* output stuff */

/*
frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);
vbox = gtk_vbox_new(TRUE, 0);
gtk_container_add(GTK_CONTAINER(frame), vbox);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, PANEL_SPACING);
*/
/*
label = gtk_label_new(g_strdup_printf(" Output filename "));
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

diff_filename = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(diff_filename), data->basename);
gtk_box_pack_end(GTK_BOX(hbox), diff_filename, FALSE, FALSE, 0);
*/
/*
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, PANEL_SPACING);
if (sysenv.grace_path)
  diff_plot = new_check_button("Plot spectrum ", NULL, NULL, 1, hbox);
else
  {
  diff_plot = new_check_button("Plot spectrum ", NULL, NULL, 0, hbox);
  gtk_widget_set_sensitive(GTK_WIDGET(diff_plot), FALSE);
  }
*/
/*
if (data->animation)
  gui_direct_check("Calculate for all frames (output file only)", &diff_all_frames, NULL, NULL, vbox);
  else
    diff_all_frames = FALSE;
*/

/* terminating buttons */
gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Diffraction", GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_EXECUTE, diff_calc, NULL, GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog, GTK_DIALOG(window)->action_area);

gtk_widget_show_all(window);
}

