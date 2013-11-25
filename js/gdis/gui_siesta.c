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
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "gdis.h"
#include "coords.h"
#include "model.h"
#include "file.h"
#include "matrix.h"
#include "module.h"
#include "numeric.h"
#include "parse.h"
#include "project.h"
#include "render.h"
#include "spatial.h"
#include "quaternion.h"
#include "task.h"
#include "interface.h"
#include "dialog.h"
#include "gui_shorts.h"
/*
#include "meschach/matrix-meschach.h"
*/
#include "mesch.h"
#include "siesta.h"
#include "import.h"

extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

static gboolean siestafileWRITE;

/******************************/
/* toggle eigenvector display */
/******************************/
void gui_siesta_mode_show(GtkWidget *w, gpointer dialog)
{
gint i, atom, state;
gdouble scale, x1[3], x2[3], colour[3];
gpointer button, spin;
GSList *list;
struct core_pak *core;
struct spatial_pak *spatial;
struct model_pak *model;

g_assert(dialog != NULL);

model = dialog_model(dialog);
g_assert(model != NULL);

button = dialog_child_get(dialog, "phonon_toggle");
g_assert(button != NULL);

spatial_destroy_by_label("siesta_phonons", model);

state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
if (!state)
  {
  redraw_canvas(SINGLE);
  return;
  }

spin = dialog_child_get(dialog, "phonon_scaling");
scale = SPIN_FVAL(spin);

/* create & init the spatial object */
spatial = spatial_new("siesta_phonons", SPATIAL_VECTOR, 2, TRUE, model);

atom = 0;
/* get eigenvectors from all atoms */
for (list=model->cores ; list; list=g_slist_next(list))
  {
  core = list->data;
  ARR3SET(x1, core->x);

/* get current eigenvector */
  i = model->siesta.sorted_eig_values[model->siesta.current_animation];
  x2[0] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom, i);
  x2[1] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom+1, i);
  x2[2] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom+2, i);
  atom++;

/* compute coords */
  VEC3MUL(x2, scale);
  vecmat(model->ilatmat, x2);
  ARR3ADD(x2, x1);
/* add to spatial */
  spatial_vertex_add(x2, colour, spatial);
  spatial_vertex_add(x1, colour, spatial);
  }
    
/* drawing update */
coords_compute(model);
redraw_canvas(SINGLE);
}

/*******************************/
/* update frequency entry text */
/*******************************/
void gui_siesta_frequency_set(gpointer dialog)
{
gint m;
gchar *text;
gdouble f;
gpointer entry;
struct model_pak *model;

model = dialog_model(dialog);
g_assert(model != NULL);
entry = dialog_child_get(dialog, "phonon_entry");
g_assert(entry != NULL);

m = model->siesta.current_animation;

g_assert(m >= 0);
g_assert(m < model->siesta.num_animations);

f = make_eigvec_freq(mesch_ve_get(model->siesta.eigen_values, model->siesta.sorted_eig_values[m]));

text = g_strdup_printf("%f", f);
gtk_entry_set_text(GTK_ENTRY(entry), text);
g_free(text);
}

/***************************/
/* slider bar event change */
/***************************/
void gui_siesta_slider_changed(GtkWidget *w, gpointer dialog)
{
gint m;
struct model_pak *model;

g_assert(w != NULL);
g_assert(dialog != NULL);

model = dialog_model(dialog);

m = nearest_int(gtk_range_get_value(GTK_RANGE(w)));
model->siesta.current_animation = m-1;   /* 1..n range -> 0..n-1 */

gui_siesta_frequency_set(dialog);
gui_siesta_mode_show(NULL, dialog);
}

/*********************/
/* move to next mode */
/*********************/
void gui_siesta_mode_next(GtkWidget *w, gpointer dialog)
{
gdouble m;
gpointer slider;

slider = dialog_child_get(dialog, "phonon_slider");

m = gtk_range_get_value(GTK_RANGE(slider));
m++;
gtk_range_set_value(GTK_RANGE(slider), m);
}

/*************************/
/* move to previous mode */
/*************************/
void gui_siesta_mode_prev(GtkWidget *w, gpointer dialog)
{
gdouble m;
gpointer slider;

slider = dialog_child_get(dialog, "phonon_slider");

m = gtk_range_get_value(GTK_RANGE(slider));
m--;
gtk_range_set_value(GTK_RANGE(slider), m);
}

/***********************************/
/* cleanup a vibrational animation */
/***********************************/
void siesta_phonon_cleanup(struct model_pak *model)
{
GSList *list;
struct core_pak *core;

g_assert(model != NULL);

for (list=model->cores ; list; list=g_slist_next(list))
  {
  core = list->data;

  VEC3SET(core->offset, 0.0, 0.0, 0.0);
  }
}

/***********************************/
/* handler to update the animation */
/***********************************/
/* NB: lots of sanity checks that return with FALSE (ie stop animating) */
/* so if the model is deleted during an animation we don't segfault */
#define MAX_PULSE_COUNT 10.0
gint siesta_phonon_timer(gpointer dialog)
{
static gint count=0;
gint atom, mode;
gchar *text, *name;
gdouble f, x1[3];
gpointer spin;
GSList *list;
struct core_pak *core;
struct model_pak *model;

/* checks */
if (!dialog_valid(dialog))
  return(FALSE);

model = dialog_model(dialog);
g_assert(model != NULL);

/* stop animation? */
if (!model->pulse_direction)
  {
  siesta_phonon_cleanup(model);
  coords_compute(model);
  redraw_canvas(SINGLE);
  return(FALSE);
  }

/* setup animation resolution */
spin = dialog_child_get(dialog, "phonon_resolution");
model->pulse_max = SPIN_FVAL(spin);

/* setup scaling for this step */
model->pulse_count += model->pulse_direction;
if (model->pulse_count <= -model->pulse_max)
  {
  model->pulse_count = -model->pulse_max;
  model->pulse_direction = 1;
  }
if (model->pulse_count >= model->pulse_max)
  {
  model->pulse_count = model->pulse_max;
  model->pulse_direction = -1;
  }

spin = dialog_child_get(dialog, "phonon_scaling");
f = SPIN_FVAL(spin);
f *= (gdouble) model->pulse_count;
f /= model->pulse_max;

atom = 0;

mode = model->siesta.sorted_eig_values[model->siesta.current_animation];

/* get eigenvectors from all atoms */
for (list=model->cores ; list; list=g_slist_next(list))
  {
  core = list->data;
  ARR3SET(x1, core->x);

//get x,y,z for given freq.
  core->offset[0] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom, mode);
  core->offset[1] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom+1, mode);
  core->offset[2] = mesch_me_get(model->siesta.eigen_xyz_atom_mat, 3*atom+2, mode);
  atom++;

/* pulse offset scaling */
  VEC3MUL(core->offset, f);
  vecmat(model->ilatmat, core->offset);
  }

/* recalc coords */
coords_compute(model);

/* CURRENT - output to povray for movie rendering */
if (model->phonon_movie)
  {
  if (!model->pulse_count && model->pulse_direction==1)
    {
    model->phonon_movie = FALSE;
    count=0;

    text = g_strdup_printf("%s -delay 20 %s_*.tga %s.%s",
                           sysenv.convert_path, model->phonon_movie_name,
                           model->phonon_movie_name, model->phonon_movie_type);

    system(text);
    g_free(text);

    return(FALSE);
    }
  else
    {
    text = g_strdup_printf("%s_%06d.pov", model->phonon_movie_name, count++);
    name = g_build_filename(sysenv.cwd, text, NULL);
    write_povray(name, model);

    povray_exec(sysenv.cwd, name);

    g_free(text);
    g_free(name);
    }

  }
else
  redraw_canvas(SINGLE);

return(TRUE);
}

/****************************/
/* animate the current mode */
/****************************/
void siesta_phonon_start(GtkWidget *w, gpointer dialog)
{
struct model_pak *model;

g_assert(dialog != NULL);
model = dialog_model(dialog);
g_assert(model != NULL);

model->pulse_count = 0;
model->pulse_direction = 1;

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
gdk_threads_add_timeout(100, (GSourceFunc) siesta_phonon_timer, dialog);
#else
g_timeout_add(100, (GSourceFunc) siesta_phonon_timer, dialog);
#endif
}

/******************************/
/* stop phonon mode animation */
/*****************************/
void siesta_phonon_stop(GtkWidget *w, gpointer dialog)
{
struct model_pak *model;

g_assert(dialog != NULL);
model = dialog_model(dialog);
g_assert(model != NULL);

/* reset */
model->pulse_direction = 0;
model->pulse_count = 0;
model->phonon_movie = FALSE;
}

/**************************************/
/* create a movie of the current mode */
/**************************************/
void siesta_phonon_movie(GtkWidget *w, gpointer dialog)
{
gpointer entry;
struct model_pak *model;

model = dialog_model(dialog);
g_assert(model != NULL);

entry = dialog_child_get(dialog, "phonon_movie_name");
g_assert(entry != NULL);
/* FIXME - GULP phonon movie uses the same variable */
g_free(model->phonon_movie_name);
model->phonon_movie_name = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));

entry = dialog_child_get(dialog, "phonon_movie_type");
g_assert(entry != NULL);
g_free(model->phonon_movie_type);
model->phonon_movie_type = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));

model->phonon_movie = TRUE;
siesta_phonon_start(NULL, dialog);
}

/*****************************/
/* SIESTA phonon calculation */
/*****************************/
/* TODO - relocate */
gint siesta_phonon_calc(struct model_pak *model)
{
gint num_tokens, num_atoms, num_lines, i, j, k, l;
gchar ** buff;
gchar * modelFCname, *modelFCnameCSV;
gdouble wi, wj, value;
gpointer bigmat, correction_mat, mini_correction_zerooooo_mat;
GSList *list_i, *list_j;
struct core_pak *core_i;
struct core_pak *core_j;
FILE *fp, *matout=NULL;

g_assert(model != NULL);

/* check atom labels (since we must be able to do a valid weight lookup) */
for (list_i=model->cores ; list_i ; list_i=g_slist_next(list_i))
  {
  core_i = list_i->data;
  if (core_i->atom_code == 0)
    {
    gchar *text;

    text = g_strdup_printf("Unknown atom label: [%s]\n", core_i->atom_label); 
    gui_text_show(ERROR, text);
    g_free(text);

    return(1);
    }
  }

num_atoms = model->num_atoms;

//lines = 3 *    N * N     * 2;
//       xyz, each atom, back/forward
//
num_lines = 3*2*num_atoms*num_atoms;

modelFCname = g_strdup_printf("%s/%s.FC", sysenv.cwd, model->basename);
modelFCnameCSV = g_strdup_printf("%s.csv", modelFCname);

fp = fopen(modelFCname, "rt");

if (siestafileWRITE)
  {
  matout = fopen(modelFCnameCSV, "w");
  if (!matout)
    {
    gui_text_show(ERROR, "bugger - no save files\n");
    return(2);
    }
  }

if (!fp)
  {
  gchar * text;
  text = g_strdup_printf("*ERROR* - modelFCname file not opened\n");
  gui_text_show(ERROR, text);
  gui_text_show(ERROR, modelFCname);
  gui_text_show(ERROR, "\n");
  g_free(text);
  return(3);
  }

//no need for names anymore
g_free(modelFCname);

//initalise bigmat
bigmat = mesch_mat_new(3*num_atoms, 3*num_atoms);

//first line is crap.
buff = get_tokenized_line(fp, &num_tokens);

//FILE reading into bigmat
for (i = 0; i<num_lines; i++)
  {
  g_strfreev(buff);
  buff = get_tokenized_line(fp, &num_tokens);
  if (!buff)
    {
//error not enough lines in file...
//matrix_2d_free(bigmat);
    }

  if (num_tokens > 2)
    {
    if ( (i/num_atoms)%2 == 0)
      {
//first pass at row?

mesch_me_set(bigmat, i/(2*num_atoms), (3*i)%(3*num_atoms), str_to_float(*buff));
mesch_me_set(bigmat, i/(2*num_atoms), ((3*i)+1)%(3*num_atoms), str_to_float(*(buff+1)));
mesch_me_set(bigmat, i/(2*num_atoms), ((3*i)+2)%(3*num_atoms), str_to_float(*(buff+2)));

       }
     else
       {
//second pass - do the average

value = 0.5*(mesch_me_get(bigmat, i/(2*num_atoms), (3*i)%(3*num_atoms)) + str_to_float(*buff));
mesch_me_set(bigmat, i/(2*num_atoms), (3*i)%(3*num_atoms), value);

value = 0.5*(mesch_me_get(bigmat, i/(2*num_atoms), ((3*i)+1)%(3*num_atoms)) + str_to_float(*(buff+1)));
mesch_me_set(bigmat, i/(2*num_atoms), ((3*i)+1)%(3*num_atoms), value);

value = 0.5*(mesch_me_get(bigmat, i/(2*num_atoms), ((3*i)+2)%(3*num_atoms)) + str_to_float(*(buff+2)));
mesch_me_set(bigmat, i/(2*num_atoms), ((3*i)+2)%(3*num_atoms), value);

        }
      }
    else
      {
//what happened - why is there not 3 things on the line?
//matrix_2d_free(bigmat);
       }
//next line?
     }

//Symmetricalise? -> to make symmetric
for (i=0; i<(3*num_atoms); i++)
  {
  for (j=0; j<(3*num_atoms); j++)
    {
    value = mesch_me_get(bigmat, i, j) + mesch_me_get(bigmat, j, i);
    value *= 0.5;
    mesch_me_set(bigmat, i, j, value);
    mesch_me_set(bigmat, j, i, value);
    }
  }

correction_mat = mesch_mat_new(3*num_atoms,3);
mini_correction_zerooooo_mat = mesch_mat_new(3,3);

mesch_m_zero(correction_mat);
mesch_m_zero(mini_correction_zerooooo_mat);


//build the correction_matrix

for (i=0; i<mesch_rows_get(bigmat); i++)
  {
  for (j=0; j<mesch_cols_get(bigmat)/3; j++)
    {
//[3n][3] -> [i][0], [i][1], [i][2]

    value = mesch_me_get(bigmat, i, 3*j);
    mesch_me_add(correction_mat, i, 0, value);

    value = mesch_me_get(bigmat, i, (3*j)+1);
    mesch_me_add(correction_mat, i, 1, value);

    value = mesch_me_get(bigmat, i, (3*j)+2);
    mesch_me_add(correction_mat, i, 2, value);
    }

 //average each cell per row in the correction matrix
  value = 1.0 / (gdouble) num_atoms;

  mesch_me_mul(correction_mat, i, 0, value);
  mesch_me_mul(correction_mat, i, 1, value);
  mesch_me_mul(correction_mat, i, 2, value);
  }


//built mini matrix - [3][3]
for (i=0; i<mesch_rows_get(correction_mat); i++)
  {
  for (j=0; j<mesch_cols_get(correction_mat); j++)
    {
    value = mesch_me_get(correction_mat, i, j);
    mesch_me_add(mini_correction_zerooooo_mat, i%3, j, value);
    }
  }

//average the cells in mini_correction_zerooooo_mat

value = 1.0 / (gdouble) num_atoms;

for (i=0; i<mesch_rows_get(mini_correction_zerooooo_mat); i++)
  {
  for (j=0; j<mesch_cols_get(mini_correction_zerooooo_mat); j++)
    {
    mesch_me_mul(mini_correction_zerooooo_mat, i, j, value);
    }
  }

//zero point correction
//    crappy crappy fortran loop that i dont understand
//       do i=1,natoms
//         do j=1,natoms
//           do ii=1,3
//             do ij=1,3
//               correct = (zeroo(ii,ij)+zeroo(ij,ii))/2.0d0 -
//                         (zero(ii,ij,j)+zero(ij,ii,i))
//               do lx=-lxmax,lxmax
//               do ly=-lymax,lymax
//               do lz=-lzmax,lzmax
//                 phi(ii,i,ij,j,lx,ly,lz) = phi(ii,i,ij,j,lx,ly,lz) +
//                                           correct
//               enddo
//               enddo
//               enddo
//             enddo
//           enddo
//         enddo
//       enddo

        gdouble correction;
for (i=0; i<num_atoms; i++)
  {
  for (j=0; j<num_atoms; j++)
    {
    for (k=0; k<3; k++) //(ii)
      {
      for (l=0; l<3; l++) //(ij)
        {
// THIS WORKS - I HAVE TESTED IT - MANY TIMES......
        correction = mesch_me_get(mini_correction_zerooooo_mat, k, l);
        correction += mesch_me_get(mini_correction_zerooooo_mat, l, k);
        correction *= 0.5;
        correction -= mesch_me_get(correction_mat, 3*i+k, l);
        correction -= mesch_me_get(correction_mat, 3*j+l, k);

        mesch_me_add(bigmat, 3*i+k, 3*j+l, correction);
        }
      }
    }
  }

i=j=0;

for (list_i=model->cores ; list_i ; list_i=g_slist_next(list_i))
  {
  core_i = list_i->data;
  if (core_i->status & DELETED)
    {
    i++;
    continue;
    }

  wi = elements[core_i->atom_code].weight;
  g_assert(wi > G_MINDOUBLE);

  for (list_j=model->cores ; list_j ; list_j=g_slist_next(list_j))
    {
    core_j = list_j->data;
    if (core_j->status & DELETED)
      {
      j++;
      continue;
      }
//multi i rows.... 3 of them....

    wj = elements[core_j->atom_code].weight;
    g_assert(wj > G_MINDOUBLE);
    value  = 1.0 / sqrt(wi * wj);

    for (k=0; k<3; k++)
      {
      mesch_me_mul(bigmat, (3*i)+k, 3*j, value);
      mesch_me_mul(bigmat, (3*i)+k, (3*j)+1, value);
      mesch_me_mul(bigmat, (3*i)+k, (3*j)+2, value);
      }

    j++;
    }
  i++;
  j=0;
  }

model->siesta.eigen_xyz_atom_mat = mesch_mat_new(3*num_atoms, 3*num_atoms);
model->siesta.eigen_values = mesch_vec_new(3*num_atoms);

//external library call.

mesch_sev_compute(bigmat, model->siesta.eigen_xyz_atom_mat, model->siesta.eigen_values);

// stupid sort routine -> this is going to need a rewrite - its a bubble sort - O(n^2).
model->siesta.sorted_eig_values = g_malloc(sizeof(int[mesch_dim_get(model->siesta.eigen_values)]));

for (i=0; i<mesch_dim_get(model->siesta.eigen_values); i++)
  {
  model->siesta.sorted_eig_values[i] = i;
  }

gint temp_int;
gdouble freq_i, freq_ii;

gint sizeofeig = mesch_dim_get(model->siesta.eigen_values);

for (j=sizeofeig-1; j>1; j--)
  {
  for (i=0; i < j; i++)
    {
    freq_i = make_eigvec_freq(mesch_ve_get(model->siesta.eigen_values, model->siesta.sorted_eig_values[i]));

    freq_ii = make_eigvec_freq(mesch_ve_get(model->siesta.eigen_values, model->siesta.sorted_eig_values[i+1]));


    if (freq_i > freq_ii )
      {
      temp_int = model->siesta.sorted_eig_values[i];
      model->siesta.sorted_eig_values[i] = model->siesta.sorted_eig_values[i+1];
      model->siesta.sorted_eig_values[i+1] = temp_int;
      }
    }
  }

//PRINT METHOD FOR UWA VISIT
if (siestafileWRITE && matout)
  {
  fprintf(matout, "eig vectors 3N*3N\n-------------\n");
  for (j=0;j<(3*num_atoms); j++)
    {
    for (i=0; i<(3*num_atoms); i++)
      {
      fprintf(matout, "%f, ", mesch_me_get(bigmat, j, i));
      }
    fprintf(matout, "\n");
    }

  fprintf(matout, "\n\neig_vals\n-------------\n");
  for (i=0; i<mesch_dim_get(model->siesta.eigen_values); i++)
    {
    fprintf(matout, "%f, ", mesch_ve_get(model->siesta.eigen_values, i));
    }

  fprintf(matout, "\n\nfreqs\n-------------\n");
  for (i=0; i<mesch_dim_get(model->siesta.eigen_values); i++)
    {
    fprintf(matout, "%f, ", make_eigvec_freq(mesch_ve_get(model->siesta.eigen_values, i)));
    }

  fclose(matout);
  }

fclose(fp);

mesch_m_free(bigmat);
mesch_m_free(correction_mat);
mesch_m_free(mini_correction_zerooooo_mat);

model->siesta.current_animation = 0;

//Lookup using index array.
model->siesta.current_frequency = make_eigvec_freq(mesch_ve_get(model->siesta.eigen_values, model->siesta.sorted_eig_values[0]));
model->siesta.freq_disp_str = g_strdup_printf("%.2f", model->siesta.current_frequency);
model->siesta.num_animations = mesch_dim_get(model->siesta.eigen_values);
model->siesta.vibration_calc_complete = TRUE;

return(0);
}

/********************************/
/* SIESTA phonon display dialog */
/********************************/
void siesta_animation_dialog(GtkWidget *w, struct model_pak *model)
{
GList *list;
GtkWidget *window, *box, *hbox, *hbox2, *vbox, *label, *hscale, *entry, *button, *spin;
gpointer dialog;

g_assert(model != NULL);

/* don't recalculate modes if already done */
if (!model->siesta.vibration_calc_complete)
  if (siesta_phonon_calc(model))
    return;

/* request a dialog */
dialog = dialog_request(100, "Vibrational viewer", NULL, NULL, model);
if (!dialog)
  return;

window = dialog_window(dialog);
box = gtk_vbox_new(TRUE, 0);
gtk_container_add(GTK_CONTAINER(GTK_DIALOG(window)->vbox), box);

/* phonon selection */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, box);
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

/* phonon frequency */
label = gtk_label_new("Frequency  ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
entry = gtk_entry_new_with_max_length(LINELEN);
gtk_entry_set_text(GTK_ENTRY(entry), " ");
gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
dialog_child_set(dialog, "phonon_entry", entry);
gui_button(" < ", gui_siesta_mode_prev, dialog, hbox, FF);
gui_button(" > ", gui_siesta_mode_next, dialog, hbox, FF);

/* phonon slider */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
hscale = gtk_hscale_new_with_range(1.0, model->siesta.num_animations, 1.0);
gtk_box_pack_start(GTK_BOX(hbox), hscale, TRUE, TRUE, 0);
dialog_child_set(dialog, "phonon_slider", hscale);
g_signal_connect(GTK_OBJECT(hscale), "value_changed",
                 GTK_SIGNAL_FUNC(gui_siesta_slider_changed), dialog);

/* phonon display options */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, box);
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
button = new_check_button("Show eigenvectors", gui_siesta_mode_show, dialog, FALSE, hbox);
dialog_child_set(dialog, "phonon_toggle", button);

spin = new_spinner("Eigenvector scaling", 0.1, 9.9, 0.1, gui_siesta_mode_show, dialog, vbox);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 4.0);
dialog_child_set(dialog, "phonon_scaling", spin);

spin = new_spinner("Animation resolution", 5.0, 50.0, 1.0, NULL, NULL, vbox);
dialog_child_set(dialog, "phonon_resolution", spin);

/* phonon mode animation */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, box);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
label = gtk_label_new("Animate mode ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
hbox2 = gtk_hbox_new(FALSE, 0);
gtk_box_pack_end(GTK_BOX(hbox), hbox2, FALSE, FALSE, 0);
gui_icon_button("GDIS_PLAY", NULL, siesta_phonon_start, dialog, hbox2);
gui_icon_button("GDIS_STOP", NULL, siesta_phonon_stop, dialog, hbox2);

/* phonon mode movie generation */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
label = gtk_label_new("Create movie ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
entry = gtk_entry_new_with_max_length(LINELEN);
gtk_entry_set_text(GTK_ENTRY(entry), "movie_name");
gtk_entry_set_editable(GTK_ENTRY(entry), TRUE);
gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
dialog_child_set(dialog, "phonon_movie_name", entry);

/* movie type */
list = NULL;
list = g_list_append(list, "gif");
list = g_list_append(list, "mpg");
entry = gui_pulldown_new(NULL, list, FALSE, hbox);
dialog_child_set(dialog, "phonon_movie_type", entry);
gui_button_x(NULL, siesta_phonon_movie, dialog, hbox);

/* init and display */
gui_siesta_frequency_set(dialog);

gtk_widget_show_all(window);
}

/*************************/
/* main SIESTA interface */
/*************************/
void gui_siesta_dialog(void)
{
/* replaced by gui_siesta_widget() */
}

/******************************************************/
/* construct a new widget linked to the siesta config */
/******************************************************/
/* TODO - separate widget from config so it can be re-used */
void gui_siesta_widget(GtkWidget *box, gpointer config)
{
gchar *text;
GSList *list, *list1;
GtkWidget *notebook, *vbox, *hbox, *label, *textbox; 
GtkWidget *page, *left, *right;
struct siesta_pak *siesta;

g_assert(box != NULL);
g_assert(config != NULL);

siesta = config_data(config);

/* true false (default) list */
list1 = NULL;
list1 = g_slist_prepend(list1, "False");
list1 = g_slist_prepend(list1, "True");
list1 = g_slist_prepend(list1, "");

/* build widget */
notebook = gtk_notebook_new();
gtk_container_add(GTK_CONTAINER(box), notebook);

/* file write page */
page = gtk_vbox_new(FALSE, 0);
label = gtk_label_new(" File I/O ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);


vbox = gui_frame_vbox(NULL, FALSE, FALSE, page);
gui_text_entry("System Label  ", &siesta->system_label, TRUE, TRUE, vbox);

/*
vbox = gui_frame_vbox("Mesh potential", FALSE, FALSE, page);
gui_text_entry("Density of states ", &siesta->density_of_states, TRUE, FALSE, vbox);
gui_text_entry("Density on mesh ", &siesta->density_on_mesh, TRUE, FALSE, vbox);
gui_text_entry("Electrostatic pot on mesh ", &siesta->electrostatic_pot_on_mesh, TRUE, FALSE, vbox);
*/

hbox = gui_frame_hbox(NULL, FALSE, FALSE, page);
left = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), left, TRUE, TRUE, 0);
right = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), right, TRUE, TRUE, 0);

/* left */
gui_pd_new("Use saved data", &siesta->use_saved_data, list1, left);
gui_pd_new("Long output ", &siesta->long_output, list1, left);
gui_pd_new("Write coordinates", &siesta->write_coor_step, list1, left);
gui_pd_new("Write cerius coordinates", &siesta->write_coor_cerius, list1, left);
gui_pd_new("Write xmol coordinates", &siesta->write_coor_xmol, list1, left);
gui_pd_new("Write xmol animation", &siesta->write_md_xmol, list1, left);
gui_pd_new("Write trajectory history", &siesta->write_md_history, list1, left);
gui_pd_new("Write forces", &siesta->write_forces, list1, left);

/* right */
gui_pd_new("Write wavefunctions", &siesta->write_wavefunctions, list1, right);
gui_pd_new("Write density matrix", &siesta->write_dm, list1, right);
gui_pd_new("Write k points", &siesta->write_kpoints, list1, right);
gui_pd_new("Write eigenvalues", &siesta->write_eigenvalues, list1, right);
gui_pd_new("Write k bands", &siesta->write_kbands, list1, right);
gui_pd_new("Write bands", &siesta->write_bands, list1, right);
gui_pd_new("Write Mulliken population", &siesta->write_mullikenpop, list1, right);


/* electronic page */

page = gtk_vbox_new(FALSE, 0);
label = gtk_label_new("Electronic");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), hbox, TRUE, TRUE, 0);
left = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), left, TRUE, TRUE, 0);
right = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), right, TRUE, TRUE, 0);


/* basis set */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, left);

list = NULL;
list = g_slist_prepend(list, "nonodes");
list = g_slist_prepend(list, "nodes");
list = g_slist_prepend(list, "splitgauss");
list = g_slist_prepend(list, "split");
list = g_slist_prepend(list, "");
gui_pd_new("Basis type", &siesta->basis_type, list, vbox);
g_slist_free(list);

list = NULL;
list = g_slist_prepend(list, "DZP");
list = g_slist_prepend(list, "SZP");
list = g_slist_prepend(list, "DZ");
list = g_slist_prepend(list, "SZ");
list = g_slist_prepend(list, "");
gui_pd_new("Basis size", &siesta->basis_size, list, vbox);
g_slist_free(list);

/* custom zeta ie %block PAO.basis */
/*
gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
gui_text_entry("Zeta level ", &siesta->custom_zeta, TRUE, FALSE, vbox);
gui_text_entry("Polarisation level ", &siesta->custom_zeta_polarisation, TRUE, FALSE, vbox);
*/

/* functional */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, left);
list = NULL;
list = g_slist_prepend(list, "LDA");
list = g_slist_prepend(list, "GGA");
list = g_slist_prepend(list, "");
gui_pd_new("X-C functional", &siesta->xc_functional, list, vbox);
g_slist_free(list);

list = NULL;
list = g_slist_prepend(list, "LYP");
list = g_slist_prepend(list, "RPBE");
list = g_slist_prepend(list, "revPBE");
list = g_slist_prepend(list, "PBE");
list = g_slist_prepend(list, "PW92");
list = g_slist_prepend(list, "CA");
list = g_slist_prepend(list, "");
gui_pd_new("X-C authors", &siesta->xc_authors, list, vbox);
g_slist_free(list);

gui_pd_new("Harris functional", &siesta->harris_functional, list1, vbox);

/* spin */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, left);
gui_pd_new("Spin polarised", &siesta->spin_polarised, list1, vbox);
gui_pd_new("Non co-linear spin", &siesta->non_collinear_spin, list1, vbox);
gui_pd_new("Fixed spin", &siesta->fixed_spin, list1, vbox);

list = NULL;
list = g_slist_prepend(list, "0.5");
list = g_slist_prepend(list, "1.0");
list = g_slist_prepend(list, "1.5");
list = g_slist_prepend(list, "2.0");
list = g_slist_prepend(list, "");
gui_pd_new("Total spin", &siesta->total_spin, list, vbox);
g_slist_free(list);

gui_pd_new("Single excitation", &siesta->single_excitation, list1, vbox);


/* SCF options */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, right);

gui_text_entry("Number of SCF cycles ", &siesta->no_of_cycles, TRUE, FALSE, vbox);
gui_text_entry("Mixing weight ", &siesta->mixing_weight, TRUE, FALSE, vbox);
gui_text_entry("Number of Pulay matrices ", &siesta->no_of_pulay_matrices, TRUE, FALSE, vbox);


/* other options */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, right);
gui_text_entry("Energy shift (Ryd)", &siesta->energy_shift, TRUE, FALSE, vbox);
gui_text_entry("Split zeta norm", &siesta->split_zeta_norm, TRUE, FALSE, vbox);
gui_text_entry("Mesh cutoff (Ryd)", &siesta->mesh_cutoff, TRUE, FALSE, vbox);
gui_text_entry("kgrid cutoff ", &siesta->kgrid_cutoff, TRUE, FALSE, vbox);


/* speed hacks left/right */
/*
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), hbox, TRUE, TRUE, 0);
left = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), left, TRUE, TRUE, 0);
right = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), right, TRUE, TRUE, 0);
*/

/* speed hacks */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, left);
list = NULL;
list = g_slist_prepend(list, "orderN");
list = g_slist_prepend(list, "diagon");
list = g_slist_prepend(list, "");
gui_pd_new("Solution method", &siesta->solution_method, list, vbox);
g_slist_free(list);

/* NEW */
list = NULL;
list = g_slist_prepend(list, "MP");
list = g_slist_prepend(list, "FD");
list = g_slist_prepend(list, "");
gui_pd_new("Occupation function", &siesta->occupation_function, list, vbox);
g_slist_free(list);

list = NULL;
list = g_slist_prepend(list, "Files");
list = g_slist_prepend(list, "Ordejon-Mauri");
list = g_slist_prepend(list, "Kim");
list = g_slist_prepend(list, "");
gui_pd_new("Order N functional", &siesta->ordern_functional, list, vbox);
g_slist_free(list);

vbox = gui_frame_vbox(NULL, FALSE, FALSE, right);
gui_text_entry("Electronic temperature", &siesta->electronic_temperature, TRUE, FALSE, vbox);
gui_text_entry("Occupation MP order ", &siesta->occupation_mp_order, TRUE, FALSE, vbox);
gui_text_entry("Order N iterations ", &siesta->ordern_iterations, TRUE, FALSE, vbox);
gui_text_entry("Order N energy tolerance ", &siesta->ordern_energy_tolerance, TRUE, FALSE, vbox);
gui_text_entry("Order N eta ", &siesta->ordern_eta, TRUE, FALSE, vbox);
gui_text_entry("Order N eta_alpha ", &siesta->ordern_eta_alpha, TRUE, FALSE, vbox);
gui_text_entry("Order N eta_beta ", &siesta->ordern_eta_beta, TRUE, FALSE, vbox);


/* Run page */
page = gtk_vbox_new(FALSE, 0);
label = gtk_label_new("Geometric");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

/* split pane */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), hbox, TRUE, TRUE, 0);
left = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), left, TRUE, TRUE, 0);
right = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), right, TRUE, TRUE, 0);

/* main setup */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, left);
list = NULL;
list = g_slist_prepend(list, "Phonon");
list = g_slist_prepend(list, "FC");
list = g_slist_prepend(list, "Anneal");
list = g_slist_prepend(list, "NoseParrinelloRahman");
list = g_slist_prepend(list, "ParrinelloRahman");
list = g_slist_prepend(list, "Nose");
list = g_slist_prepend(list, "Verlet");
list = g_slist_prepend(list, "Broyden");
list = g_slist_prepend(list, "CG");
list = g_slist_prepend(list, "");
gui_pd_new("Run type", &siesta->md_type_of_run, list, vbox);
g_slist_free(list);

gui_pd_new("Variable cell", &siesta->md_variable_cell, list1, vbox);

/* MD steps */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, left);
gui_text_entry("Initial time step ", &siesta->md_initial_time_step, TRUE, FALSE, vbox);
gui_text_entry("Final time step ", &siesta->md_final_time_step, TRUE, FALSE, vbox);
gui_text_entry("Length of time step ", &siesta->md_length_time_step, TRUE, FALSE, vbox);

/* CG steps */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, left);
gui_text_entry("Max CG steps", &siesta->md_num_cg_steps, TRUE, FALSE, vbox);
gui_text_entry("Max CG displacement", &siesta->md_max_cg_displacement, TRUE, FALSE, vbox);
gui_text_entry("Max force tolerance", &siesta->md_max_force_tol, TRUE, FALSE, vbox);
gui_text_entry("Max stress tolerance", &siesta->md_max_stress_tol, TRUE, FALSE, vbox);

/* temperature */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, right);
gui_text_entry("Initial temperature ", &siesta->md_initial_temperature, TRUE, FALSE, vbox);
gui_text_entry("Target temperature ", &siesta->md_target_temperature,TRUE, FALSE, vbox);
gui_text_entry("Target pressure", &siesta->md_target_pressure, TRUE, FALSE, vbox);

//gui_text_entry("Number of Steps ", &siesta->number_of_steps, TRUE, FALSE, vbox);

/* additional options */
vbox = gui_frame_vbox("Target stress tensor", FALSE, FALSE, right);
gui_text_entry("xx", &siesta->md_target_stress[0], TRUE, FALSE, vbox);
gui_text_entry("yy", &siesta->md_target_stress[1], TRUE, FALSE, vbox);
gui_text_entry("zz", &siesta->md_target_stress[2], TRUE, FALSE, vbox);
gui_text_entry("xy", &siesta->md_target_stress[3], TRUE, FALSE, vbox);
gui_text_entry("xz", &siesta->md_target_stress[4], TRUE, FALSE, vbox);
gui_text_entry("yz", &siesta->md_target_stress[5], TRUE, FALSE, vbox);

//vbox = gui_frame_vbox("Differencing", FALSE, FALSE, geompage);
//gui_direct_spin("Finite Difference step size ", &siesta->finite_diff_step_size, 0.1, 10.0, 0.1, NULL, NULL, vbox);

/* unprocessed lines */
page = gtk_vbox_new(FALSE, 0);
label = gtk_label_new (" Unprocessed ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

vbox = gui_frame_vbox(NULL, TRUE, TRUE, page);

text = config_unparsed_get(config);

/* FIXME - allowing this to be edited -> core dump */
textbox = gui_text_window(&text, FALSE);

gtk_box_pack_start(GTK_BOX(vbox), textbox, TRUE, TRUE, 0);


/* cleanup */
g_slist_free(list1);

gui_relation_update(NULL);
}

/********************************/
/* SIESTA phonon display dialog */
/********************************/

gdouble make_eigvec_freq(gdouble value)
{
    gdouble xmagic;
    xmagic = 519.6;
    if (value > 0.0)
    {
        return sqrt(xmagic*xmagic*value);
    }
    else    // fake number!
    {
        return -sqrt(-xmagic*xmagic*value);
    }
}

void siesta_file_dialog(GtkWidget *w, struct model_pak * model)
{
}


void siesta_make_runscript(gchar * model_name, gchar * directory, struct grid_config_pak * grid)
{
    FILE *script;

    gchar *script_name;

    script_name = g_strdup_printf("run.script.%s", model_name);

    g_build_filename(directory, script_name, NULL);

    script = fopen (script_name, "w");
    if (!script_name)
        return;

    fprintf(script, "#!/bin/bash\n");
    fprintf(script, "#PBS -P d64\n");
    fprintf(script, "#PBS -l ncpus=8\n");
    fprintf(script, "#PBS -l vmem=2800mb\n");
    fprintf(script, "#PBS -l walltime=2:00:00\n");
    fprintf(script, "#PBS -l software=siesta\n");
    fprintf(script, "#PBS -l other=mpi\n");
    fprintf(script, "#PBS -wd\n");

    fprintf(script, "module load siesta\n");
    fprintf(script, "mpirun siesta < %s.fdf > %s.sot\n", model_name, model_name);

    fclose (script);
    g_free (script_name);

}

void siesta_load_animation(GtkWidget *w, struct model_pak *model)
{
}

void siesta_animation_dialog_destroy(GtkWidget *w, gpointer *dialog)
{

    //kill the dialog
    dialog_destroy(w, dialog);
}

