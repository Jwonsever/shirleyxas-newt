/*
Copyright (C) 2003 by Andrew Lloyd Rohl and Sean David Fleming

andrew@ivec.org
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "gdis.h"
#include "gamess.h"
#include "coords.h"
#include "file.h"
#include "task.h"
#include "grid.h"
#include "matrix.h"
#include "numeric.h"
#include "parse.h"
#include "spatial.h"
#include "gui_shorts.h"
#include "dialog.h"
#include "interface.h"
#include "opengl.h"

extern struct basis_pak basis_sets[];

enum {
  TIMLIM, MWORDS, WIDE_OUTPUT
};

struct callback_pak run_control[] = {
{"Time limit (minutes) ", TIMLIM},
{"Memory limit (megawords) ", MWORDS},
{NULL, 0}
};

enum {
  TOTAL_Q, MULTIPLICITY
};

struct callback_pak econfig[] = {
{"Total charge ", TOTAL_Q},
{"Multiplicity ", MULTIPLICITY},
{NULL, 0}
};

enum {
  NUM_P, NUM_D, NUM_F
};

struct callback_pak pol_funcs[] = {
{"p ", NUM_P},
{"d ", NUM_D},
{"f ", NUM_F},
{NULL, 0}
};

enum {
  HEAVY_DIFFUSE, HYDROGEN_DIFFUSE, NSTEP, GAMESS_TITLE, GAMESS_ENERGY, INPUTFILE, MODELNAME
};

extern struct sysenv_pak sysenv;

/*********************/
/* run a gamess file */
/*********************/
#define DEBUG_EXEC_GAMESS 0
gint exec_gamess(gchar *input)
{
gchar *cmd, hostname[256];

/* checks */
if (!sysenv.gamess_path)
  return(-1);

/* NEW - acquire hostname for GAMESS job submission */
/*
if (gethostname(hostname, sizeof(hostname)))
*/
  sprintf(hostname, "localhost");

#if __WIN32
{
gchar *basename, *bat, *fubar, *fname, *temp, *tmp, *dir, *inp, *out;
FILE *fp;

/* setup variables */
basename = parse_strip(input);
fname = g_strdup_printf("%s.F05", basename);
fubar = g_build_filename(sysenv.gamess_path, "scratch", fname, NULL);
temp = g_build_filename(sysenv.gamess_path, "temp", basename, NULL);
dir = g_build_filename(sysenv.gamess_path, NULL);
inp = g_build_filename(sysenv.cwd, input, NULL);
out = g_build_filename(sysenv.cwd, basename, NULL);

/* create a batch file to run GAMESS */
bat = gun("bat");
fp = fopen(bat, "wt");
fprintf(fp, "@echo off\n");
fprintf(fp, "echo Running GAMESS using 1 CPU, job: %s\n", input);

/* remove old crap in the temp directory */
fprintf(fp, "del %s.*\n", temp);

/* put the input file in the scratch directory */
fprintf(fp, "copy \"%s\" \"%s\"\n", inp, fubar);

/* run the execution script */
fprintf(fp, "cd %s\n", sysenv.gamess_path);
fprintf(fp, "csh -f runscript.csh %s 04 1 %s %s > \"%s.gmot\"\n",
             basename, dir, hostname, out);
fprintf(fp, "echo GAMESS job completed\n");
fclose(fp);

tmp = g_build_filename(sysenv.cwd, bat, NULL);
cmd = g_strdup_printf("\"%s\"", tmp);

g_free(basename);
g_free(fname);
g_free(fubar);
g_free(temp);
g_free(tmp);
g_free(inp);
g_free(out);
g_free(bat);
}
#else
cmd = g_strdup_printf("%s %s", sysenv.gamess_path, input); 
#endif

#if DEBUG_EXEC_GAMESS
printf("executing: [%s]\n",cmd);
#else
task_sync(cmd);
#endif

g_free(cmd);

return(0);
}

/*****************************/
/* execute a gamess run (task) */
/*****************************/
#define DEBUG_EXEC_GAMESS_TASK 0
void exec_gamess_task(struct model_pak *model)
{
gchar *out;

/* construct output filename */
g_free(model->gamess.out_file);

out = parse_extension_set(model->gamess.temp_file, "gmot");
model->gamess.out_file = g_build_filename(sysenv.cwd, out, NULL);
g_free(out);

#if __WIN32
/*
out = g_strdup_printf("\"%s\"", model->gamess.out_file);
*/
out = g_shell_quote(model->gamess.out_file);
g_free(model->gamess.out_file);
model->gamess.out_file = out;
#endif

/* save input file and execute */
file_save_as(model->gamess.temp_file, model);
exec_gamess(model->gamess.temp_file);
}

/*****************************/
/* process a gamess run (task) */
/*****************************/
#define DEBUG_PROC_GAMESS 0
void proc_gamess_task(gpointer ptr)
{
gchar *filename;
GString *line;
struct model_pak *data;

/* TODO - model locking (moldel_ptr RO/RW etc) to prevent screw ups */
g_return_if_fail(ptr != NULL);
data = ptr;

/* win32 fix - which can't make up its mind if it wants to quote or not */
filename = g_shell_unquote(data->gamess.out_file, NULL);

if (!g_file_test(filename, G_FILE_TEST_EXISTS))
  {
  printf("Missing output file [%s]\n", filename);
  return;
  }

if (data->gamess.run_type < GMS_OPTIMIZE && !data->animation)
  {
/* same model (ie with current energetics dialog) so update */
    file_load(filename, data);
/* update energy (TODO - only if successful) */
    line = g_string_new(NULL);
    if (data->gamess.have_energy)
      g_string_sprintf(line,"%f",data->gamess.energy);
    else
      g_string_sprintf(line, "not yet calculated");
/* is there a dialog entry to be updated? */
    if (GTK_IS_ENTRY(data->gamess.energy_entry))
      gtk_entry_set_text(GTK_ENTRY(data->gamess.energy_entry), line->str);
    g_string_free(line, TRUE);
  }
else
  {
/* TODO - make it possile to get dialog data by request */
/* so that we can check if a dialog exsits to be updated */
/* get new coords */
/* create new model for the minimized result */
  file_load(filename, NULL);
  }

g_free(filename);

redraw_canvas(ALL);
}

/*************************************************/
/* controls the use of GAMESS to optimise coords */
/*************************************************/
#define DEBUG_RUN_GAMESS 0
void gui_gamess_task(GtkWidget *w, struct model_pak *data)
{
/* checks */
g_return_if_fail(data != NULL);
if (!sysenv.gamess_path)
  {
  gui_text_show(ERROR, "GAMESS executable was not found.\n");
  return;
  }

#if DEBUG_RUN_GAMESS
printf("output file: %s\n", data->gamess.out_file);
#endif

task_new("Gamess", &exec_gamess_task, data, &proc_gamess_task, data, data);
}

/*********************************/
/* GAMESS execution type toggles */
/*********************************/
void gamess_exe_run(struct model_pak *mdata)
{
mdata->gamess.exe_type = GMS_RUN;
}
void gamess_exe_check(struct model_pak *mdata)
{
mdata->gamess.exe_type = GMS_CHECK;
}
void gamess_exe_debug(struct model_pak *mdata)
{
mdata->gamess.exe_type = GMS_DEBUG;
}

/***************************/
/* GAMESS run type toggles */
/***************************/
void gamess_run_single(struct model_pak *data)
{
data->gamess.run_type = GMS_ENERGY;
}
void gamess_run_gradient(struct model_pak *data)
{
data->gamess.run_type = GMS_GRADIENT;
}
void gamess_run_hessian(struct model_pak *data)
{
data->gamess.run_type = GMS_HESSIAN;
}
void gamess_run_optimize(struct model_pak *data)
{
data->gamess.run_type = GMS_OPTIMIZE;
}

/***************************/
/* GAMESS SCF type toggles */
/***************************/
void gamess_scf_rhf(struct model_pak *data)
{
data->gamess.scf_type = GMS_RHF;
}
void gamess_scf_uhf(struct model_pak *data)
{
data->gamess.scf_type = GMS_UHF;
}
void gamess_scf_rohf(struct model_pak *data)
{
data->gamess.scf_type = GMS_ROHF;
}
void gamess_units_angs(struct model_pak *data)
{
data->gamess.units = GMS_ANGS;
}
void gamess_units_bohr(struct model_pak *data)
{
data->gamess.units = GMS_BOHR;
}
void gamess_optimise_qa(struct model_pak *data)
{
data->gamess.opt_type = GMS_QA;
}
void gamess_optimise_nr(struct model_pak *data)
{
data->gamess.opt_type = GMS_NR;
}
void gamess_optimise_rfo(struct model_pak *data)
{
data->gamess.opt_type = GMS_RFO;
}
void gamess_optimise_schlegel(struct model_pak *data)
{
data->gamess.opt_type = GMS_SCHLEGEL;
}
void event_destroy_user_data_from_object(GtkObject *object, gpointer user_data)
{
g_free(user_data);
}

/*****************************/
/* basis set selection event */
/*****************************/
void basis_selected(GtkWidget *w, gpointer data)
{
gint i=0;
const gchar *label;
struct gamess_pak *gamess = data;

if (!gamess)
  return;

label = gtk_entry_get_text(GTK_ENTRY(w));

while (basis_sets[i].label)
  {
  if (g_strcasecmp(label, basis_sets[i].label) == 0)
    {
    gamess->basis = basis_sets[i].basis;
    gamess->ngauss = basis_sets[i].ngauss;
    }
  i++;
  }
}

/******************************/
/* functional selection event */
/******************************/
void gms_functional_select(GtkWidget *w, struct model_pak *model)
{
const gchar *label;

label = gtk_entry_get_text(GTK_ENTRY(w));

if (g_strncasecmp(label, "svwn", 4) == 0)
  {
  model->gamess.dft_functional = SVWN;
  return;
  }
if (g_strncasecmp(label, "blyp", 4) == 0)
  {
  model->gamess.dft_functional = BLYP;
  return;
  }
if (g_strncasecmp(label, "b3ly", 4) == 0)
  {
  model->gamess.dft_functional = B3LYP;
  return;
  }
}

/***************************/
/* text handling callbacks */
/***************************/
gint event_pol_text_fields(GtkWidget *w, gpointer *obj)
{
gint id;
struct model_pak *data;

/* checks */
g_return_val_if_fail(w != NULL, FALSE);

/* ascertain type of modification required */
id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "id"));

data = tree_model_get();
if (!data)
  return(FALSE);

sysenv.moving = FALSE;

switch(id)
  {
  case GAMESS_TITLE:
    g_free(data->gamess.title);
    data->gamess.title = g_strdup(gtk_entry_get_text(GTK_ENTRY(w)));
    break;
  case INPUTFILE:
    g_free(data->gamess.temp_file);
    data->gamess.temp_file = g_strdup(gtk_entry_get_text(GTK_ENTRY(w)));
    break; 
  }

return(FALSE);
}

/*********************************/
/* GAMESS job setup/results page */
/*********************************/
void gui_gamess_widget(GtkWidget *box, gpointer data)
{
gint i, j;
GString *line;
GList *list;
gpointer notebook, entry, radio;
GtkWidget *hbox, *vbox, *vbox1, *vbox2, *vbox4, *page;
GtkWidget *label;
GtkWidget *combo;
struct gamess_pak *gamess=data;

/* checks */
g_assert(box != NULL);
g_assert(gamess != NULL);

/* string manipulation scratchpad */
line = g_string_new(NULL);

/* create notebook */
notebook = gui_notebook_new(box);

/* Control page */
page = gtk_vbox_new(FALSE, 0);
gui_notebook_page_append(notebook, " Control ", page);

//label = gtk_label_new(" Control ");
//gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

hbox = gtk_hbox_new(TRUE, 0);
gtk_container_add(GTK_CONTAINER(page), hbox);

/* split panel */
vbox1 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 0);
vbox2 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);
//vbox3 = gtk_vbox_new(FALSE, 0);
//gtk_box_pack_start(GTK_BOX(hbox), vbox3, TRUE, TRUE, 0);

/* column 1 */
/* frame for execution type */
vbox = gui_frame_vbox("Execution type", FALSE, FALSE, vbox1);
radio = gui_radio_new(&gamess->exe_type, vbox);
gui_radio_button_add("Run", GMS_RUN, radio);
gui_radio_button_add("Check", GMS_CHECK, radio);
gui_radio_button_add("Debug", GMS_DEBUG, radio);

/* frame for run type */
vbox = gui_frame_vbox("Run type", FALSE, FALSE, vbox1);
radio = gui_radio_new(&gamess->run_type, vbox);
gui_radio_button_add("Single point", GMS_ENERGY, radio);
gui_radio_button_add("Gradient", GMS_GRADIENT, radio);
gui_radio_button_add("Hessian", GMS_HESSIAN, radio);
gui_radio_button_add("Optimise", GMS_OPTIMIZE, radio);

/* optimiser to use */
vbox = gui_frame_vbox("Optimiser", FALSE, FALSE, vbox1);
radio = gui_radio_new(&gamess->opt_type, vbox);
gui_radio_button_add("Quadratic approximation", GMS_QA, radio);
gui_radio_button_add("Newton-Raphson", GMS_NR, radio);
gui_radio_button_add("RFO", GMS_RFO, radio);
gui_radio_button_add("Quasi-NR", GMS_SCHLEGEL, radio);

/* frame for optimization cycles */
//vbox = gui_frame_vbox(NULL, FALSE, FALSE, vbox1);
/* optimization cycles */
hbox = gui_text_spin_new("Maximum cycles", &gamess->nstep, 0.0, 500.0, 10.0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);


/* column 2 */
/* frame for charge and multiplicity */
vbox = gui_frame_vbox("Electronic", FALSE, FALSE, vbox2);

/* Add spinners */
i=0;
while (econfig[i].label)
  {
  switch (econfig[i].id)
    {
    case TOTAL_Q:
      hbox = gui_text_spin_new(econfig[i].label, &gamess->total_charge, -5.0, 5.0, 1.0);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
      break;

    case MULTIPLICITY:
      hbox = gui_text_spin_new(econfig[i].label, &gamess->multiplicity, 1.0, 5.0, 1.0);
      break;

    default:
      g_assert_not_reached();
    }
  i++;
  }

/* frame for SCF type */
vbox = gui_frame_vbox("SCF options", FALSE, FALSE, vbox2);
radio = gui_radio_new(&gamess->scf_type, vbox);
gui_radio_button_add("RHF", GMS_RHF, radio);
gui_radio_button_add("UHF", GMS_UHF, radio);
gui_radio_button_add("ROHF", GMS_ROHF, radio);

/* frame for input units */
/* CURRENT - not really useful? */
/*
vbox = gui_frame_vbox("Input units", FALSE, FALSE, vbox2);
radio = gui_radio_new(&gamess->units, vbox);
gui_radio_button_add("Angstrom", GMS_ANGS, radio);
gui_radio_button_add("Bohr", GMS_BOHR, radio);
*/

/* frame for SCF options */
//vbox = gui_frame_vbox("SCF options", FALSE, FALSE, vbox2);

hbox = gui_text_spin_new("Maximum iterations", &gamess->maxit, 1.0, 999.0, 1.0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

/* frame for DFT */
vbox = gui_frame_vbox("DFT Options", FALSE, FALSE, vbox2);
vbox4 = gtk_vbox_new(TRUE, 0);
/* TODO - set sensitive stuff -> candidate for a short cut */
gui_direct_check("DFT calculation", &gamess->dft, gui_checkbox_refresh, vbox4, vbox);
gtk_box_pack_start(GTK_BOX(vbox), vbox4, FALSE, FALSE, 0);

/* TODO - the pulldown list is another candidate for a short cut */
hbox = gtk_hbox_new(FALSE, PANEL_SPACING);
gtk_box_pack_start(GTK_BOX(vbox4), hbox, FALSE, FALSE, 0);

label = gtk_label_new("Functional");
gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

list = NULL;
list = g_list_append(list, "SVWN (LDA)");
list = g_list_append(list, "BLYP");
list = g_list_append(list, "B3LYP");

combo = gtk_combo_new();
gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), FALSE);
gtk_combo_set_popdown_strings(GTK_COMBO(combo), list);
gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);

g_signal_connect(GTK_OBJECT(GTK_COMBO(combo)->entry), "changed", 
                 GTK_SIGNAL_FUNC(gms_functional_select), data);


/* frame for time, memory and output */
vbox = gui_frame_vbox("Resources", FALSE, FALSE, vbox2);

/* Add spinners */
i=0;
while (run_control[i].label)
  {
  switch (run_control[i].id)
    {
    case TIMLIM:
      hbox = gui_text_spin_new(run_control[i].label, &gamess->time_limit, 0.0, 30000.0, 10.0);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      break;

    case MWORDS:
      hbox = gui_text_spin_new(run_control[i].label, &gamess->mwords, 1.0, 150.0, 1.0);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      break;

    default:
      g_assert_not_reached();
    }
  i++;
  }

/* wide output button */
vbox = gui_frame_vbox(NULL, FALSE, FALSE, vbox2);
gui_direct_check("Wide output", &gamess->wide_output, NULL, NULL, vbox);


/* PAGE */
page = gtk_hbox_new(TRUE, 0);
gui_notebook_page_append(notebook, " Basis ", page);

/* split panel */
vbox1 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), vbox1, TRUE, TRUE, 0);
vbox2 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), vbox2, TRUE, TRUE, 0);

/* frame for basis set */
vbox = gui_frame_vbox("Basis set", FALSE, FALSE, vbox1);

/* configure basis set list for pulldown */
j = -1;
i = 0;
list = NULL;
while (basis_sets[i].label)
  {
  list = g_list_append(list, basis_sets[i].label);
  if ((basis_sets[i].basis == gamess->basis) && (basis_sets[i].ngauss == gamess->ngauss))
    j = i;
  i++;
  }
entry = gui_pulldown_new(NULL, list, FALSE, vbox);

/* set the current basis from internal value */
if (j >= 0)
  gtk_entry_set_text(entry, basis_sets[j].label);

/* update callback */
g_signal_connect(GTK_OBJECT(entry), "changed", GTK_SIGNAL_FUNC(basis_selected), gamess);

/* frame for polarization functions set */
vbox = gui_frame_vbox("Polarization functions", FALSE, FALSE, vbox2);

/* Add spinners */
i=0;
while (pol_funcs[i].label)
  {
  switch (pol_funcs[i].id)
    {
    case NUM_P:
      hbox = gui_text_spin_new(pol_funcs[i].label, &gamess->num_p, 0.0, 3.0, 1.0);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
      break;
    case NUM_D:
      hbox = gui_text_spin_new(pol_funcs[i].label, &gamess->num_d, 0.0, 3.0, 1.0);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
      break;
    case NUM_F:
      hbox = gui_text_spin_new(pol_funcs[i].label, &gamess->num_f, 0.0, 1.0, 1.0);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
      break;
    default:
      g_assert_not_reached();
    }
  i++;
  }

/* frame for polarization functions set */
vbox = gui_frame_vbox("Diffuse functions", FALSE, FALSE, vbox2);
gui_direct_check("Heavy atoms (s & p)", &gamess->have_heavy_diffuse, NULL, NULL, vbox);
gui_direct_check("Hydrogen (s only)", &gamess->have_hydrogen_diffuse, NULL, NULL, vbox);


/* PAGE - files? */
/* FIXME - if decide to do this - move Wide Output checkbox here */
/*
page = gtk_hbox_new(TRUE, 0);
gui_notebook_page_append(notebook, " Files ", page);

vbox1 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), vbox1, TRUE, TRUE, 0);
vbox2 = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), vbox2, TRUE, TRUE, 0);
*/

/* setup some gulp file name defaults */
if (g_ascii_strncasecmp(gamess->temp_file, "none", 4) == 0)
  {
  g_free(gamess->temp_file);
  gamess->temp_file = g_strdup_printf("temp.inp");
  }
if (g_ascii_strncasecmp(gamess->out_file, "none", 4) == 0)
  {
  g_free(gamess->out_file);
  gamess->out_file = g_strdup_printf("temp.gmot"); 
  }


/* done */
gtk_widget_show_all(box);

if (gamess->dft)
  gtk_widget_set_sensitive(vbox4, TRUE);
else
  gtk_widget_set_sensitive(vbox4, FALSE);

g_string_free(line, TRUE);
}

/****************************************/
/* run the approriate energetics method */
/****************************************/
void gamess_execute(GtkWidget *w, gpointer data)
{
struct model_pak *model = data;

g_assert(model != NULL);

gui_gamess_task(NULL, model);
}

/****************************/
/* GAMESS energetics dialog */
/****************************/
void gamess_dialog(void)
{
gpointer dialog;
GtkWidget *window, *frame, *vbox;
struct model_pak *model;

model = tree_model_get();
if (!model)
  return;

/* request an energetics dialog */
dialog = dialog_request(GAMESS, "GAMESS configuration", NULL, NULL, NULL);
if (!dialog)
  return;
window = dialog_window(dialog);

frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);

vbox = gtk_vbox_new(FALSE,0);
gtk_container_add(GTK_CONTAINER(frame), vbox);
//gui_gamess_widget(vbox, model);

/* terminating button */
gui_stock_button(GTK_STOCK_EXECUTE, gamess_execute, model,
                   GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog,
                   GTK_DIALOG(window)->action_area);

gtk_widget_show_all(window);
}

