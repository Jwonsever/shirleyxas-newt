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

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "gdis.h"
#include "coords.h"
#include "edit.h"
#include "file.h"
#include "graph.h"
#include "model.h"
#include "import.h"
#include "parse.h"
#include "scan.h"
#include "task.h"
#include "grid.h"
#include "matrix.h"
#include "surface.h"
#include "spatial.h"
#include "gui_shorts.h"
#include "interface.h"
#include "dialog.h"
#include "opengl.h"
#include "render.h"
#include "gui_image.h"

extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/* TODO - avoid using these globals */
GtkWidget *phonon_freq, *phonon_ir, *phonon_raman;

/*****************************/
/* phonon selection handlers */
/*****************************/
void update_phonon_range(struct model_pak *model)
{
/* checks */
g_assert(model != NULL);

/*
if (!model->phonon_slider)
  return;

if (model->num_phonons > 0)
  gtk_range_set_range(GTK_RANGE(model->phonon_slider), 1, model->num_phonons);
*/
}

/*****************************/
/* execute a gulp run (task) */
/*****************************/
#define DEBUG_EXEC_GULP_TASK 0
void exec_gulp_task(gpointer ptr, gpointer data)
{
#if FIX_EXEC_GULP_TASK
gchar *inpfile;
struct model_pak *model = ptr;
struct task_pak *task = data;

/* checks */
g_assert(model != NULL);
g_assert(task != NULL);

/* construct fullpath input filename - required for writing */
inpfile = g_build_filename(sysenv.cwd, model->gulp.temp_file, NULL);
task->status_file = g_build_filename(sysenv.cwd, model->gulp.out_file, NULL);

write_gulp(inpfile, model);
g_free(inpfile);

/* are we supposed to execute GULP? */
if (model->gulp.no_exec)
  {
  return;
  }

exec_gulp(model->gulp.temp_file, model->gulp.out_file);
#endif
}

/*****************************/
/* process a gulp run (task) */
/*****************************/
#define DEBUG_PROC_GULP 0
void proc_gulp_task(gpointer ptr)
{
gchar *inp, *res, *out;
GString *line;
struct model_pak *dest, *data;

/* TODO - model locking (moldel_ptr RO/RW etc) to prevent screw ups */
g_return_if_fail(ptr != NULL);
data = ptr;

/* don't attempt to process if gulp execution was turned off */
if (data->gulp.no_exec)
  return;

/* FIXME - if the user has moved directories since submitting */
/* the gulp job then cwd will have changed and this will fail */
inp = g_build_filename(sysenv.cwd, data->gulp.temp_file, NULL);
res = g_build_filename(sysenv.cwd, data->gulp.dump_file, NULL);

out = g_build_filename(sysenv.cwd, data->gulp.out_file, NULL);

switch (data->gulp.run)
  {
  case E_SINGLE:
/* same model (ie with current energetics dialog) so update */
    read_gulp_output(out, data);
/* update energy (TODO - only if successful) */
    line = g_string_new(NULL);
    if (data->gulp.free)
      g_string_sprintf(line, "%f (free energy)", data->gulp.energy);
    else
      g_string_sprintf(line, "%f", data->gulp.energy);

property_add_ranked(2, "Energy", line->str, data);

/* is there a dialog entry to be updated? */
    if (GTK_IS_ENTRY(data->gulp.energy_entry))
      gtk_entry_set_text(GTK_ENTRY(data->gulp.energy_entry), line->str);
    if (data->periodic == 2)
      {
/* update surface dipole dialog entry */
      g_string_sprintf(line,"%f e.Angs",data->gulp.sdipole);
      if (GTK_IS_ENTRY(data->gulp.sdipole_entry))
        gtk_entry_set_text(GTK_ENTRY(data->gulp.sdipole_entry), line->str);

/* update surface energy dialog entry */
      g_string_sprintf(line,"%f    %s",data->gulp.esurf[0], data->gulp.esurf_units);
      if (GTK_IS_ENTRY(data->gulp.esurf_entry))
        gtk_entry_set_text(GTK_ENTRY(data->gulp.esurf_entry), line->str);

/* update attachment energy dialog entry */
      g_string_sprintf(line,"%f    %s",data->gulp.eatt[0], data->gulp.eatt_units);
      if (GTK_IS_ENTRY(data->gulp.eatt_entry))
        gtk_entry_set_text(GTK_ENTRY(data->gulp.eatt_entry), line->str);
      }

    update_phonon_range(data);

    gui_active_refresh();

    g_string_free(line, TRUE);
    break;

  case E_OPTIMIZE:
/* TODO - make it possile to get dialog data by request */
/* so that we can check if a dialog exsits to be updated */
/* get new coords */
/* create new model for the minimized result */
    dest = model_new();
    g_return_if_fail(dest != NULL);

/* read main data from the res file (correct charges etc.) */
    read_gulp(res, dest);
/* graft to the model tree, so subsequent GULP read doesn't replace coords */
    tree_model_add(dest);
/* get the output energy/phonons etc. */
    read_gulp_output(out, dest);


/* FIXME - if the GULP job fails - model_prep() isnt called, and the */
/* model camera isnt initialized => error trap when gdis tries to visualize */
if (!dest->camera)
  {
printf("WARNING: GULP calculation has possibly failed.\n");
  model_prep(dest);
  }

    break;

/* MD */
  default:
    break;
  }

g_free(inp);
g_free(res);
g_free(out);

redraw_canvas(ALL);
return;
}

/***********************************************/
/* controls the use of GULP to optimise coords */
/***********************************************/
#define DEBUG_RUN_GULP 0
void gui_gulp_task(GtkWidget *w, struct model_pak *data)
{
/* checks */
g_return_if_fail(data != NULL);
if (!sysenv.gulp_path)
  {
  gui_text_show(ERROR, "GULP executable was not found.\n");
  return;
  }

#if DEBUG_RUN_GULP
printf("output file: %s\n", data->gulp.out_file);
#endif

task_new("gulp", &exec_gulp_task, data, &proc_gulp_task, data, data);
}

/***********************************/
/* register structure name changes */
/***********************************/
void gulp_jobname_changed(GtkWidget *w, struct model_pak *model)
{
const gchar *text;

g_assert(w != NULL);
g_assert(model != NULL);

g_free(model->gulp.dump_file);
g_free(model->gulp.temp_file);
g_free(model->gulp.trj_file);
g_free(model->gulp.out_file);

text = gtk_entry_get_text(GTK_ENTRY(w));
model->gulp.temp_file = g_strdup_printf("%s.gin", text);
model->gulp.dump_file = g_strdup_printf("%s.res", text);
model->gulp.trj_file = g_strdup_printf("%s.trg", text);
model->gulp.out_file = g_strdup_printf("%s.got", text);

/* disallow spaces (bad filename) */
parse_space_replace(model->gulp.temp_file, '_');
parse_space_replace(model->gulp.dump_file, '_');
parse_space_replace(model->gulp.trj_file, '_');
parse_space_replace(model->gulp.out_file, '_');

gui_relation_update_widget(&model->gulp.dump_file);
gui_relation_update_widget(&model->gulp.temp_file);
gui_relation_update_widget(&model->gulp.trj_file);
}

/*****************************/
/* phonon intensity plotting */
/*****************************/
void gui_phonon_plot(const gchar *type, struct model_pak *model)
{
gint i, n;
gdouble *spectrum;
gpointer import, graph;
GSList *item, *list;

g_assert(model != NULL);

if (g_ascii_strncasecmp(type, "IR ", 3) == 0)
  list = model->ir_list;
else
  list = model->raman_list;

n = g_slist_length(list);
if (!n)
  {
  gui_text_show(ERROR, "No phonon data.\n");
  return;
  }

/* copy data into temp array for plotting */
spectrum = g_malloc(n * sizeof(gdouble));
i=0;
for (item=list ; item ; item=g_slist_next(item))
  {
  spectrum[i++] = str_to_float(item->data);
  }

/* create a new graph */
graph = graph_new(type);
graph_add_data(n, spectrum, 1, n, graph);
graph_set_yticks(FALSE, 2, graph);

import = gui_tree_parent_object(model);
if (import)
  {
  import_object_add(IMPORT_GRAPH, graph, import);
  gui_tree_append(import, TREE_GRAPH, graph);
  }

g_free(spectrum);
}

/*********************************************/
/* read in solvent accessible surface points */
/*********************************************/
/* TODO - put elsewhere? */
void gulp_cosmic_read(gchar *filename, struct model_pak *model)
{
gint i, n=0, dim, expect=-1, num_tokens;
gchar **buff;
gdouble x[3], colour[3];
gpointer scan, spatial;

g_assert(filename != NULL);
g_assert(model != NULL);

VEC3SET(colour, 0.0, 1.0, 0.0);

n=0;
scan = scan_new(filename);
if (!scan)
  return;

/* header */
buff = scan_get_tokens(scan, &num_tokens);
if (num_tokens == 1)
  expect = str_to_float(*buff);

/* new GULP version  - dimension + cell vectors */
buff = scan_get_tokens(scan, &num_tokens);
if (num_tokens == 1)
  {
  dim = str_to_float(*buff);
  g_strfreev(buff);

/* TODO - process cell vectors */
  for (i=dim ; i-- ; )
    {
    buff = scan_get_tokens(scan, &num_tokens);
    g_strfreev(buff);
    }
  }
else
  scan_put_line(scan);

/* body */
while (!scan_complete(scan))
  {
  buff = scan_get_tokens(scan, &num_tokens);

  if (num_tokens == 6)
    {
/* coordinates */
    x[0] = str_to_float(*(buff+2));
    x[1] = str_to_float(*(buff+3));
    x[2] = str_to_float(*(buff+4));
    vecmat(model->ilatmat, x);
/* add a new point */
    spatial = spatial_new("SAS", SPATIAL_GENERIC, 1, TRUE, model);
    spatial_vertex_add(x, colour, spatial);
    n++;
    }
  else
    break;

  g_strfreev(buff);
  }


scan_free(scan);
}

/**********************************/
/* read sas file and draw surface */
/**********************************/
void gulp_cosmic_display(GtkWidget *w, struct model_pak *model)
{
gchar *name;

name = parse_extension_set(model->filename, "sas");

gulp_cosmic_read(name, model);

coords_init(CENT_COORDS, model);

redraw_canvas(SINGLE);

g_free(name);
}

/*************************************/
/* set COSMO widget sensitive status */
/*************************************/
void gulp_cosmo_panel_refresh(GtkWidget *w, gpointer dialog)
{
/*
gchar *text;
GtkWidget *box;
GtkEntry *solvation;
struct model_pak *model;

box = dialog_child_get(dialog, "cosmo_box");
g_assert(box != NULL);

model = dialog_model(dialog);
g_assert(model != NULL);

solvation = dialog_child_get(dialog, "solvation_model");
text = gui_pd_text(solvation);
if (!text)
  return;

if (g_ascii_strncasecmp(text, "none", 4) == 0)
  {
  gtk_widget_set_sensitive(box, FALSE); 
  model->gulp.solvation_model = GULP_SOLVATION_NONE;
  }

if (g_ascii_strncasecmp(text, "cosmo", 5) == 0)
  {
  gtk_widget_set_sensitive(box, TRUE); 
  model->gulp.solvation_model = GULP_SOLVATION_COSMO;
  }

if (g_ascii_strncasecmp(text, "cosmic", 6) == 0)
  {
  gtk_widget_set_sensitive(box, TRUE); 
  model->gulp.solvation_model = GULP_SOLVATION_COSMIC;
  }

g_free(text);
*/
}

/********************************************/
/* calculate points for a COSMO calculation */
/********************************************/
gint gulp_cosmo_points(gint shape, gint k, gint l)
{
gint points;

if (shape)
  points = 2 + 10 * pow(3, k) * pow(4, l);
else
  points = 2 + 4 * pow(3, k) * pow(4, l);

return(points);
}

/********************************************/
/* update points for a COSMO calculation */
/********************************************/
void gulp_cosmo_points_refresh(GtkWidget *w, gpointer dialog)
{
/*
gint k, l;
gchar *text;
GtkLabel *label;
GtkSpinButton *spin;
GtkEntry *shape;
struct model_pak *model;

model = dialog_model(dialog);
g_assert(model != NULL);

shape = dialog_child_get(dialog, "cosmo_shape");
if (g_ascii_strncasecmp(gtk_entry_get_text(shape), "dodec", 5) == 0)
  model->gulp.cosmo_shape = 1;
else
  model->gulp.cosmo_shape = 0;

spin = dialog_child_get(dialog, "cosmo_index_k");
k = SPIN_IVAL(spin);

spin = dialog_child_get(dialog, "cosmo_index_l");
l = SPIN_IVAL(spin);

model->gulp.cosmo_points = gulp_cosmo_points(model->gulp.cosmo_shape, k, l);

label = dialog_child_get(dialog, "cosmo_points");
g_assert(label != NULL);
text = g_strdup_printf("%6d", model->gulp.cosmo_points);
gtk_label_set_text(label, text);
g_free(text);

spin = dialog_child_get(dialog, "cosmo_segments");
g_assert(spin != NULL);

gtk_spin_button_set_range(spin, 1.0, (gdouble) model->gulp.cosmo_points);
*/
}

/*******************************************/
/* update segments for a COSMO calculation */
/*******************************************/
void gulp_cosmo_segments_refresh(GtkSpinButton *w, gpointer dialog)
{
struct model_pak *model;

model = dialog_model(dialog);
g_assert(model != NULL);

model->gulp.cosmo_segments = SPIN_IVAL(w);
}

/*********************************************************/
/* options for displaying the solvent accessible surface */
/*********************************************************/
void gulp_solvation_box(GtkWidget *box, struct gulp_pak *gulp)
{
GSList *slist;
GtkWidget *hbox, *left, *right, *vbox1, *vbox2, *label, *shape_k, *shape_l, *segments;

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(hbox), PANEL_SPACING);

left = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(hbox), left, TRUE, TRUE, 0);

right = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(hbox), right, TRUE, TRUE, 0);

/*
cosmo = new_check_button("COSMO calculation ", gulp_cosmo_panel_refresh, dialog,
                         model->gulp.cosmo, vbox);
*/

vbox1 = gui_frame_vbox(NULL, FALSE, FALSE, left);

/* solvation model */
slist = NULL;
slist = g_slist_prepend(slist, "cosmic");
slist = g_slist_prepend(slist, "cosmo");
slist = g_slist_prepend(slist, "");
gui_pd_new("Solvation model", &gulp->solvation_model, slist, vbox1);
g_slist_free(slist);

/* solvent surface construction geometry */
slist = NULL;
slist = g_slist_prepend(slist, "dodecahedron");
slist = g_slist_prepend(slist, "octahedron");
gui_pd_new("Shape approximation ", &gulp->cosmo_shape, slist, vbox1);
g_slist_free(slist);

/* solvent surface */
//gui_button_x("Visualize SAS points ", gulp_cosmic_display, model, vbox1);


vbox2 = gui_frame_vbox(NULL, FALSE, FALSE, right);
gui_direct_check("Output SAS points", &gulp->cosmo_sas, NULL, NULL, vbox2);

/* smoothing */
slist = NULL;
slist = g_slist_prepend(slist, "0.0");
slist = g_slist_prepend(slist, "");
gui_pd_new("Surface smoothing range", &gulp->cosmo_smoothing, slist, vbox1);
g_slist_free(slist);

/* epsilon dielectric or name */
slist = NULL;
slist = g_slist_prepend(slist, "conductor");
slist = g_slist_prepend(slist, "tetrachloromethane");
slist = g_slist_prepend(slist, "methanol");
slist = g_slist_prepend(slist, "ethylether");
slist = g_slist_prepend(slist, "cyclohexane");
slist = g_slist_prepend(slist, "chloroform");
slist = g_slist_prepend(slist, "chlorobenzene");
slist = g_slist_prepend(slist, "benzene");
slist = g_slist_prepend(slist, "acetone");
slist = g_slist_prepend(slist, "water");
slist = g_slist_prepend(slist, "");
gui_pd_new("Solvent name or dielectric constant ", &gulp->cosmo_solvent_epsilon, slist, vbox1);
g_slist_free(slist);

/* radius / deltaR */
slist = NULL;
slist = g_slist_prepend(slist, "1.0  0.1");
slist = g_slist_prepend(slist, "");
gui_pd_new("Solvent radius and delta", &gulp->cosmo_solvent_radius, slist, vbox1);
g_slist_free(slist);

slist = NULL;
slist = g_slist_prepend(slist, "10.0  1.0");
slist = g_slist_prepend(slist, "");
gui_pd_new("Solvent rmax and smoothing", &gulp->cosmo_solvent_rmax, slist, vbox1);
g_slist_free(slist);


/* TODO */
/*
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);

shape_k = new_spinner("Shape indices ", 0, 99, 1, gulp_cosmo_points_refresh, dialog, hbox);
shape_l = new_spinner(NULL, 0, 99, 1, gulp_cosmo_points_refresh, dialog, hbox);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
label = gtk_label_new("Points per atom ");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
label = gtk_label_new("?");
gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
*/

/*
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);


segments = new_spinner("Segments per atom ", 1, gulp->cosmo_points, 1,
                       gulp_cosmo_segments_refresh, dialog, hbox);
*/

/* initialize values */
/*
gtk_spin_button_set_value(GTK_SPIN_BUTTON(shape_k), gulp->cosmo_shape_index[0]);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(shape_l), gulp->cosmo_shape_index[1]);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(segments), gulp->cosmo_segments);
*/

/*
gulp_cosmo_points_refresh(NULL, dialog);
gulp_cosmo_panel_refresh(cosmo, dialog);
*/
}

/*******************************/
/* GULP job setup/results page */
/*******************************/
void gui_gulp_widget(GtkWidget *box, gpointer data)
{
gpointer radio;
GtkWidget *hbox, *vbox, *vbox1, *vbox2, *page, *swin;
GtkWidget *frame, *label, *notebook;
struct gulp_pak *gulp=data;

g_assert(gulp != NULL);

/* TODO - ref/count the gulp_pak */
/* TODO - closing this dialog unrefs */
//printf("gui_gulp_widget() : %p\n", gulp);

/* create notebook */
notebook = gtk_notebook_new();
gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
gtk_container_add(GTK_CONTAINER(box), notebook);
gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);

/* run type page */
page = gtk_vbox_new(FALSE, 0);
label = gtk_label_new (" Control ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);
hbox = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(page), hbox);

/* split panel */
vbox1 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 0);
vbox2 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);

/* run type */
vbox = gui_frame_vbox("Run type", FALSE, FALSE, vbox1);
radio = gui_radio_new(&gulp->run, vbox);
gui_radio_button_add("Single point", E_SINGLE, radio);
gui_radio_button_add("Optimize", E_OPTIMIZE, radio);
gui_radio_button_add("Dynamics", MD, radio);

/* method constraint */
vbox = gui_frame_vbox("Constraint", FALSE, FALSE, vbox1);
radio = gui_radio_new(&gulp->method, vbox);
gui_radio_button_add("Constant pressure", CONP, radio);
gui_radio_button_add("Constant volume", CONV, radio);

/* molecule building/coulomb interactions */
vbox = gui_frame_vbox("Molecule options", FALSE, FALSE, vbox1);
radio = gui_radio_new(&gulp->coulomb, vbox);
gui_radio_button_add("Coulomb subtract all intramolecular", MOLE, radio);
gui_radio_button_add("Coulomb subtract 1-2 and 1-3 intramolecular", MOLMEC, radio);
gui_radio_button_add("Retain all intramolecular coulomb terms", MOLQ, radio);
gui_radio_button_add("Molecule building off", NOBUILD, radio);

/* more molecular options */
gui_direct_check("Fix the initial connectivity", &gulp->fix, NULL, NULL, vbox);
gui_direct_check("Automatic connectivity off", &gulp->noautobond, NULL, NULL, vbox);


/* frame for keyword toggles */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, vbox1);
//gui_direct_check("Create input file then stop", &gulp->no_exec, NULL, NULL, vbox);
gui_direct_check("Build cell, then discard symmetry", &gulp->nosym, NULL, NULL, vbox);

// FIXME - this doesnt seem to work in some cases (no idea why) so hide
//gui_direct_check("Print charges with coordinates", &gulp->print_charge, NULL, NULL, vbox);

gui_direct_check("No attachment energy calculation", &gulp->no_eatt, NULL, NULL, vbox);
gui_direct_check("QEq Electronegativity equalisation", &gulp->qeq, NULL, NULL, vbox);
gui_direct_check("Compute vibrational modes", &gulp->phonon, NULL, NULL, vbox);
gui_direct_check("Compute eigenvectors", &gulp->eigen, NULL, NULL, vbox);


/* dynamics ensemble */
vbox = gui_frame_vbox("Dynamics", FALSE, FALSE, vbox2);
radio = gui_radio_new(&gulp->ensemble, vbox);
gui_radio_button_add("NVE", NVE, radio);
gui_radio_button_add("NVT", NVT, radio);
gui_radio_button_add("NPT", NPT, radio);

/* dynamics sampling */
gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);

gui_text_entry("Timestep", &gulp->timestep, TRUE, FALSE, vbox);
gui_text_entry("Equilibration  ", &gulp->equilibration, TRUE, FALSE, vbox);
gui_text_entry("Production", &gulp->production, TRUE, FALSE, vbox);
gui_text_entry("Sample", &gulp->sample, TRUE, FALSE, vbox);
gui_text_entry("Write", &gulp->write, TRUE, FALSE, vbox);

/* temperature & pressure */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, vbox2);
gui_text_entry("Temperature", &gulp->temperature, TRUE, FALSE, vbox);
gui_text_entry("Pressure", &gulp->pressure, TRUE, FALSE, vbox);

/* additional output */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, vbox2);
gui_text_entry("Dump file", &gulp->dump_file, TRUE, FALSE, vbox);
gui_text_entry("Trajectory file  ", &gulp->trj_file, TRUE, FALSE, vbox);



/* kpoints */
/* FIXME - strictly only if periodic ... */
vbox = gui_frame_vbox("K-points", TRUE, TRUE, vbox2);
swin = gui_text_window(&gulp->kpoints, TRUE);
gtk_box_pack_start(GTK_BOX(vbox), swin, TRUE, TRUE, 0);

/* NEW - text box for GULP control files */
/*
hbox = gtk_hbox_new(FALSE,0);
label = gtk_label_new(" Files ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, label);
vbox = gtk_vbox_new(FALSE,0);
gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);
vbox = gtk_vbox_new(FALSE, PANEL_SPACING/2);
gtk_container_add(GTK_CONTAINER(frame), vbox);

gui_text_entry("Dump file", &gulp->dump_file, TRUE, FALSE, vbox);
gui_text_entry("Trajectory file  ", &gulp->trj_file, TRUE, FALSE, vbox);
*/


/* minimize page */
page = gtk_vbox_new(FALSE,0);
label = gtk_label_new(" Optimisation ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, FALSE, 0);

/* optimiser to use */
vbox = gui_frame_vbox("Primary optimiser", TRUE, TRUE, hbox);

/* do the radio buttons */
radio = gui_radio_new(&gulp->optimiser, vbox);
gui_radio_button_add("bfgs", BFGS_OPT, radio);
gui_radio_button_add("conj", CONJ_OPT, radio);
gui_radio_button_add("rfo", RFO_OPT, radio);

/* optimiser to use */
vbox = gui_frame_vbox("Secondary optimiser", TRUE, TRUE, hbox);

/* do the first (DEFAULT MODE) radio button */
radio = gui_radio_new(&gulp->optimiser2, vbox);
gui_radio_button_add("bfgs", BFGS_OPT, radio);
gui_radio_button_add("conj", CONJ_OPT, radio);
gui_radio_button_add("rfo", RFO_OPT, radio);
gui_radio_button_add("none", SWITCH_OFF, radio);

/* optimiser to use */
vbox = gui_frame_vbox("Switching criteria", TRUE, TRUE, hbox);

radio = gui_radio_new(&gulp->switch_type, vbox);
gui_radio_button_add("cycle", CYCLE, radio);
gui_radio_button_add("gnorm", GNORM, radio);

/* FIXME - re-implement this */
gui_text_entry("Value", &gulp->switch_value, TRUE, FALSE, vbox);

/* frame */
hbox = gui_frame_hbox(NULL, FALSE, FALSE, page);

/* optimization cycles */
gui_text_entry("Maximum cycles", &gulp->maxcyc, TRUE, FALSE, hbox);

/* NEW - text box for potentials */
vbox = gtk_vbox_new(FALSE,0);
label = gtk_label_new(" Potentials ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);

frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
swin = gui_text_window(&gulp->potentials, TRUE);
gtk_container_add(GTK_CONTAINER(frame), swin);

gui_text_entry(" Potential Library", &gulp->libfile, TRUE, FALSE, vbox);

/* NEW - text box for element and species data */
hbox = gtk_hbox_new(TRUE, 0);
label = gtk_label_new(" Elements ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, label);

frame = gtk_frame_new("Element");
gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
swin = gui_text_window(&gulp->elements, TRUE);
gtk_container_add(GTK_CONTAINER(frame), swin);

frame = gtk_frame_new("Species");
gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
swin = gui_text_window(&gulp->species, TRUE);
gtk_container_add(GTK_CONTAINER(frame), swin);

/* solvation */
hbox = gtk_vbox_new(FALSE,0);
label = gtk_label_new(" Solvation ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, label);
gulp_solvation_box(hbox, gulp);

/* text box for "pass through" GULP data */
vbox = gtk_vbox_new(FALSE,0);
label = gtk_label_new(" Unprocessed ");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);

/* unprocessed keywords */
vbox1 = gui_frame_vbox(NULL, TRUE, TRUE, vbox);

hbox = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
gui_direct_check("Pass keywords through to GULP", &gulp->output_extra_keywords, NULL, NULL, hbox);
gui_text_entry("Keywords  ", &gulp->extra_keywords, TRUE, TRUE, vbox1);

/* unprocessed options */
hbox = gtk_hbox_new(TRUE, 0);
gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
gui_direct_check("Pass options through to GULP", &gulp->output_extra, NULL, NULL, hbox);
swin = gui_text_window(&gulp->extra, TRUE);
gtk_box_pack_start(GTK_BOX(vbox1), swin, TRUE, TRUE, 0);


/* job input filename */
/*
hbox = gui_frame_hbox("Files", FALSE, FALSE, box);
label = gtk_label_new("Job file name  ");
gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
entry = gtk_entry_new();
gtk_box_pack_end(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

if (gulp->temp_file)
  gtk_entry_set_text(GTK_ENTRY(entry), gulp->temp_file);
else
  gtk_entry_set_text(GTK_ENTRY(entry), "");
*/

/*
g_signal_connect(GTK_OBJECT(entry), "changed",
                 GTK_SIGNAL_FUNC(gulp_jobname_changed), (gpointer) model);
*/

/* summary details for the model */
/*
hbox = gui_frame_hbox("Details", FALSE, FALSE, box);

vbox = gtk_vbox_new(TRUE, 2);
gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

label = gtk_label_new("Structure name");
gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);

label = gtk_label_new("Total energy (eV)");
gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
if (model->periodic == 2)
  {
  label = gtk_label_new("Surface bulk energy (eV)");
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  label = gtk_label_new("Surface dipole");
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  label = gtk_label_new("Surface energy");
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  label = gtk_label_new("Attachment energy");
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  }

vbox = gtk_vbox_new(TRUE, 2);
gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);


entry = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(entry), model->basename);
gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
*/

/*
model->gulp.energy_entry = gtk_entry_new_with_max_length(LINELEN);
if (model->gulp.free)
  g_string_sprintf(line,"%f (free energy)",model->gulp.energy);
else
  g_string_sprintf(line,"%f",model->gulp.energy);

gtk_entry_set_text(GTK_ENTRY(model->gulp.energy_entry),line->str);
gtk_box_pack_start (GTK_BOX (vbox), model->gulp.energy_entry, TRUE, TRUE, 0);
gtk_entry_set_editable(GTK_ENTRY(model->gulp.energy_entry), FALSE);
*/

/*
if (model->periodic == 2)
  {
  model->gulp.sbe_entry = gtk_entry_new_with_max_length(LINELEN);
  g_string_sprintf(line,"%f",model->gulp.sbulkenergy);
  gtk_entry_set_text(GTK_ENTRY(model->gulp.sbe_entry),line->str);
  gtk_box_pack_start(GTK_BOX(vbox), model->gulp.sbe_entry, TRUE, TRUE, 0);

gtk_entry_set_editable(GTK_ENTRY(model->gulp.sbe_entry), TRUE);
g_signal_connect(GTK_OBJECT(model->gulp.sbe_entry), "changed",
                 GTK_SIGNAL_FUNC(cb_gulp_keyword), (gpointer) entry);
g_object_set_data(G_OBJECT(model->gulp.sbe_entry), "key", (gpointer) SBULK_ENERGY);
g_object_set_data(G_OBJECT(model->gulp.sbe_entry), "ptr", (gpointer) model);

  model->gulp.sdipole_entry = gtk_entry_new_with_max_length(LINELEN);
  g_string_sprintf(line,"%f e.Angs", model->gulp.sdipole);
  gtk_entry_set_text(GTK_ENTRY(model->gulp.sdipole_entry), line->str);
  gtk_box_pack_start(GTK_BOX(vbox), model->gulp.sdipole_entry, TRUE, TRUE, 0);
  gtk_entry_set_editable(GTK_ENTRY(model->gulp.sdipole_entry), FALSE);

  model->gulp.esurf_entry = gtk_entry_new_with_max_length(LINELEN);
  if (model->gulp.esurf[1] == 0.0)
     g_string_sprintf(line,"%f    %s",
                            model->gulp.esurf[0], model->gulp.esurf_units);
  else
     g_string_sprintf(line,"%f    %s",
                            model->gulp.esurf[1], model->gulp.esurf_units);

  gtk_entry_set_text(GTK_ENTRY(model->gulp.esurf_entry), line->str);
  gtk_box_pack_start(GTK_BOX(vbox), model->gulp.esurf_entry, TRUE, TRUE, 0);
  gtk_entry_set_editable(GTK_ENTRY(model->gulp.esurf_entry), FALSE);
  model->gulp.eatt_entry = gtk_entry_new_with_max_length(LINELEN);
  if (model->gulp.eatt[1] == 0.0)
     g_string_sprintf(line,"%f    %s",
                            model->gulp.eatt[0], model->gulp.eatt_units);
  else
     g_string_sprintf(line,"%f    %s",
                            model->gulp.eatt[1], model->gulp.eatt_units);
  gtk_entry_set_text(GTK_ENTRY(model->gulp.eatt_entry), line->str);
  gtk_box_pack_start(GTK_BOX(vbox), model->gulp.eatt_entry, TRUE, TRUE, 0);
  gtk_entry_set_editable(GTK_ENTRY(model->gulp.eatt_entry), FALSE);
  }
*/

/* done */
gtk_widget_show_all(box);
}

/****************************************/
/* run the approriate energetics method */
/****************************************/
void gulp_execute(GtkWidget *w, gpointer data)
{
struct model_pak *model = data;

g_assert(model != NULL);

gui_gulp_task(NULL, model);
}

/*********************************/
/* cleanup for energetics dialog */
/*********************************/
void gulp_cleanup(struct model_pak *model)
{
g_assert(model != NULL);

model->gulp.energy_entry = NULL;
model->gulp.sdipole_entry = NULL;
model->gulp.esurf_entry = NULL;
model->gulp.eatt_entry = NULL;
}

/**************************/
/* GULP energetics dialog */
/**************************/
void gulp_dialog(void)
{
gpointer dialog;
GtkWidget *window, *vbox;
struct model_pak *model;

model = tree_model_get();
if (!model)
  return;

gulp_files_init(model);

/* request an energetics dialog */
dialog = dialog_request(GULP, "GULP configuration", NULL, gulp_cleanup, model);
if (!dialog)
  return;
window = dialog_window(dialog);

vbox = gui_frame_vbox(NULL, FALSE, FALSE, GTK_DIALOG(window)->vbox);

//gui_gulp_widget(vbox, dialog);

/* terminating button */
gui_stock_button(GTK_STOCK_EXECUTE, gulp_execute, model,
                   GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog,
                   GTK_DIALOG(window)->action_area);

gtk_widget_show_all(window);
}

