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

/* spacing */
#define PANEL_SPACING 4
/* sub windows */
#define W_BORDER 5
#define H_BORDER 5
/* graphics window defs */
#define START_WIDTH 640
#define START_HEIGHT 600
#define MIN_WIDTH 200
#define MIN_HEIGHT 200
/* use GTKGL_ as GL_ are often reserved by OpenGL */
#define GTKGL_DIM 600
#define GTKGL_LINE_WIDTH 1.5
/* angle drawing */
#define MIN_ARC_RAD 15
#define PIX2SCALE 0.005

/* display tabs */
#define PROJECT_FILES "File Manager"
#define PROJECT_TASKS "Task Manager"
#define PROJECT_BUILDER "Model Builder"
#define PROJECT_SURFACES "Surface Builder"
#define PROJECT_DYNAMICS "Dynamics Builder"
#define PROJECT_GRID "Grid Manager"

/* TODO - change above to TAB as well? ie general solution */
#define TAB_PREVIEW "File Preview"
#define TAB_SURFACES "Surface Builder"


/* Added by C. Fisher 2004 */
#define MAXIMUM(x,y)  ((x>y)?x:y)

/* main operation modes */
enum 
{
FREE, XLAT, ROLL, YAW, PITCH, LOCKED, QUIT,
SELECT_FRAGMENT,
CREATE_RIBBON,
ATOM_ADD, ATOM_CHANGE_TYPE, ATOM_H_ADD,
BOND_NORMAL, BOND_SPLIT,
BOND_DELETE, BOND_SINGLE, BOND_DOUBLE, BOND_TRIPLE, BOND_HBOND, BOND_ZEOLITE,
BOND_INFO, DIST_INFO, ANGLE_INFO, DIHEDRAL_INFO,
DEFINE_RIBBON, DEFINE_VECTOR, DEFINE_PLANE,
ANIMATING, RECORD, OVERLAY
};

/* widget & file/package types */
/* including dialog request types */
enum 
{
GDIS, SGINFO, SYMMETRY, GPERIODIC, ELEM_EDIT, POVRAY, GENSURF,
ABOUT, ANIM, SURF, DISPLAY, GEOMETRY, SPATIAL, TASKMAN, MANUAL, SETUP,
FILE_SELECT, FILE_LOAD, FILE_SAVE, FILE_SAVE_AS, FILE_EXPORT,
NODATA, DATA, BIOSYM, CIF, FDF, GULP, MONTY, MARVIN, MORPH,
META_DATA, PWSCF, PWSCF_OUTPUT, PLOT,
OPENMX, OPENMX_INPUT, OPENMX_OUTPUT,
XML, XSF, XTL, XYZ,
AUTO, CSSR, GAMOUT, PDB, GEOMVIEW_OFF, PROJECT, DOCKING,
MDI, CREATOR, MVNOUT, GULPOUT, GULP_TRJ, SIESTA_OUT, SIESTA_POTENTIALS,
REPLACE_ATOMS, DYNAMICS, ZMATRIX,
OPENGL, OPENGL_OPTIONS, GAMESS, GAMESS_OUT, DIFFAX, DIFFAX_INP, DMOL_INPUT,
ABINIT, ABINIT_OUT, NWCHEM, NWCHEM_OUT, CASTEP, CASTEP_OUT,
GAUSS, GAUSS_OUT, GAUSS_CUBE,
REAXMD, REAXMD_OUTPUT,
HIRSHFELD, CONNECT, RDF, LIQUIDS, VACF, MD_ANALYSIS,
PICTURE, TEXT, RIETICA, CEL, DLPOLY, CRYSTAL_GRAPH, BIOGRAF, DLP, GROMACS,
AIMS_INPUT, 
LAST
};

/* model display features */
enum 
{
PLANE_LABELS, ASYM_TOGGLE,
ATOM_LABELS, FRAME, MESH, SHELLS, AXES_TYPE, 
PBC_CONFINE_NONE, PBC_CONFINE_ATOMS, PBC_CONFINE_MOLS 
};

/* switch_view() call modes */
enum {CANVAS_INIT, CANVAS_SINGLE, CANVAS_VSPLIT, CANVAS_HSPLIT, CANVAS_HVSPLIT,
      PREV_MODEL, NEXT_MODEL};

/* view actions */
enum {ROTATION, UPDATE_X, UPDATE_Y, UPDATE_Z};

/* main GUI elements */
#define GUI_TREE 1        // model tree
#define GUI_ACTIVE 2      // model properties pulldown
#define GUI_CANVAS 4      // opengl window
#define GUI_PREVIEW 8     // text display buffer


enum {TREE_PROJECT, TREE_IMPORT, TREE_CONFIG, TREE_MODEL, TREE_GRAPH};

/* prototypes */

/* main */
void gui_init(int, char **);
gint gui_motion_event(GtkWidget *, GdkEventMotion *);
gint gui_press_event(GtkWidget *, GdkEventButton *);
gint gui_scroll_event(GtkWidget *, GdkEventScroll *);
gint gui_release_event(GtkWidget *, GdkEventButton *);

/* display control */
void gui_mode_switch(gint);
void gtk_mode_switch(GtkWidget *, gint);
void gui_text_show(gint, gchar *);
void gui_text_update(void);
void gui_text_project_update(gpointer);
void gui_text_import_update(gpointer);
void gui_angles_refresh(void);
void gui_angles_reset(void);
void gui_camera_refresh(void);
gint gui_popup_confirm(const gchar *);
gint gui_popup_message(const gchar *);

void gui_widget_active(gint);
gint gui_widget_state_get(gint);

/* model tree */
gint tree_selected_get(gint *, gchar **, gpointer *);
gpointer tree_project_get(void);
gpointer tree_import_get(void);
gpointer tree_model_get(void);
GSList *tree_project_all(void);
const gchar *tree_project_cwd(void);

void gui_tree_selected_set(gpointer);
void gui_tree_selection_push(void);
void gui_tree_selection_pop(void);

void tree_project_select(gpointer);
void tree_project_add(gpointer);
void tree_import_add(gpointer, gpointer);
void tree_graph_add(gpointer, gpointer);

void gui_tree_clear(void);
void gui_tree_delete(gpointer);

void tree_select_delete(void);
void tree_select_active(void);
void tree_select_model(struct model_pak *);
void tree_select_refresh(void);

void gui_tree_append_import(gpointer);
gpointer gui_tree_append_model(const gchar *, gpointer);

void gui_tree_import_refresh(gpointer);

void gui_tree_remove_children(gpointer);
void gui_tree_rename(const gchar *, gpointer);

gpointer gui_tree_parent_object(gpointer);
gpointer gui_tree_parent_get(gint *, gpointer);

gint gui_tree_append(gpointer, gint, gpointer);
gint gui_tree_grafted(gint *, gpointer);

void tree_model_add(struct model_pak *);
void tree_model_refresh(struct model_pak *);
void tree_init(GtkWidget *);

/* NEW: export edit_model_create for use in button toolbar */
void edit_model_create(void);

void diffract_select_peak(gint, gint, struct model_pak *);
void analysis_export_dialog(void);
void symmetry_widget_redraw(void);
void image_export_dialog(void);
void image_import_dialog(void);
void image_write(gpointer);

gpointer gui_colour_selection_dialog(const gchar *, gpointer);

void gui_grid_dialog(void);
void gui_job_dialog(void);
void gui_animate_dialog(void);
void gui_diffract_dialog(void);
void gui_gulp_task(GtkWidget *, struct model_pak *);
void gui_gamess_task(GtkWidget *, struct model_pak *);
void gui_measure_dialog(void);
void gui_edit_dialog(void);
void gui_analysis_dialog(void);
void gui_isosurf_dialog(void);
void gui_gperiodic_dialog(void);
void gui_render_dialog(void);
void gui_help_dialog(void);
void gui_dock_dialog(void);
void gui_defect_dialog(void);
void gui_setup_dialog(void);
void gui_surface_dialog(void);
void gui_project_browse_dialog(void);
void gui_mdi_dialog(void);

void gui_mdi_widget(GtkWidget *);

void gui_builder_widget(GtkWidget *);

void gui_surface_widget_update(void);
void gui_surface_widget(GtkWidget *);
void gui_surface_setup(GtkWidget *);
void gui_surface_update(gpointer);

void gui_phonon_plot(const gchar *, struct model_pak *);

void gui_edit_widget(GtkWidget *);
void gui_display_widget(GtkWidget *);
void gui_symmetry_refresh(GtkWidget *);

void gui_gamess_widget(GtkWidget *, gpointer);
void gui_gulp_widget(GtkWidget *, gpointer);
void gui_siesta_widget(GtkWidget *, gpointer);
void gui_nwchem_widget(GtkWidget *, gpointer);

void gui_edit_build_page(GtkWidget *);
void gui_project_data_widget(GtkWidget *);

void gui_help_show(GtkWidget *, gpointer);

void gui_active_refresh(void);

void gui_view_x(void);
void gui_view_y(void);
void gui_view_z(void);
void gui_view_a(void);
void gui_view_b(void);
void gui_view_c(void);

void canvas_new(gint, gint, gint, gint);
void canvas_init(GtkWidget *);
void canvas_shuffle(void);
void canvas_single(void);
void canvas_create(void);
void canvas_delete(void);
void canvas_resize(void);
void canvas_select(gint, gint);
gpointer canvas_find(struct model_pak *);

void help_init(void);

void redraw_canvas(gint);

void gui_view_default(void);

gint gui_canvas_handler(gpointer);
gint gui_widget_handler(void);
gint gui_project_handler(void);

void gui_display_set(gint);
gint gui_display_get(void);

void gui_project_select(gpointer);
void gui_import_select(gpointer);
void gui_config_select(gpointer);
void gui_model_select(gpointer);
void gui_selected_save(void);
void gui_import_add_dialog(void);

gpointer gui_import_new(const gchar *);

void gui_refresh(gint);

void gui_refresh_selection(void);

void gui_import_refresh(gpointer);

void gui_workspace_save(void);

void destroy_widget(GtkWidget *, gpointer);

gpointer camera_new(void);
gpointer camera_dup(gpointer);
void camera_copy(gpointer, gpointer);
void camera_init(struct model_pak *);
void camera_dump(gpointer);
void camera_view(gdouble *, gpointer);
void camera_waypoint_animate(gint, gint, struct model_pak *);
void camera_rotate_animate(gint, gdouble *, gint, struct model_pak *);
gdouble *camera_q(gpointer);
void camera_rescale(gdouble, gpointer);
void camera_default(gdouble, gpointer);

