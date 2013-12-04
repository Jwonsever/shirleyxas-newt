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
#include <stdlib.h>
#include <string.h>

#include "gdis.h"
#include "coords.h"
#include "edit.h"
#include "file.h"
#include "matrix.h"
#include "opengl.h"
#include "dialog.h"
#include "interface.h"
#include "gui_image.h"

#include "folder.xpm"
#include "disk.xpm"
#include "arrow.xpm"
#include "axes.xpm"
#include "tools.xpm"
#include "palette.xpm"
#include "cross.xpm"
#include "geom.xpm"
#include "cell.xpm"
#include "camera.xpm"
#include "element.xpm"
#include "graph.xpm"
#include "tb_animate.xpm"
#include "tb_defect.xpm"
#include "tb_diffraction.xpm"
#include "tb_dynamics.xpm"
#include "tb_isosurface.xpm"
#include "tb_surface.xpm"
#include "tb_overlay.xpm"
#include "tb_cog.xpm"
#include "tb_info.xpm"
#include "canvas_single.xpm"
#include "canvas_create.xpm"
#include "canvas_delete.xpm"
#include "plus.xpm"
#include "cabinet.xpm"
#include "user.xpm"
#include "gdis_arcs.xpm"
#include "home.xpm"
#include "download.xpm"
#include "connect.xpm"

extern struct sysenv_pak sysenv;

extern GtkWidget *window;

//GdkPixbuf *image_table[IMAGE_LAST];
gpointer image_table[IMAGE_LAST];

/****************************************/
/* write an image of the current canvas */
/****************************************/
void image_write(gpointer w)
{
GdkPixbuf *pixbuf;
GError *error=NULL;

pixbuf = gdk_pixbuf_get_from_drawable(NULL, w, NULL,
                                      0, 0, 0, 0,
                                      sysenv.width, sysenv.height);

gdk_pixbuf_save(pixbuf, sysenv.snapshot_filename, "jpeg",
                &error, "quality", "90", NULL);

g_free(sysenv.snapshot_filename);
sysenv.snapshot = FALSE;
}

/****************************************/
/* callback to schedule a canvas export */
/****************************************/
void image_export(gchar *name)
{
g_assert(name != NULL);

dialog_destroy_type(FILE_SELECT);

/* FIXME - all GTK stuff (ie closing the file dialog/raising windows */
/* to the foreground etc.) will be done only AFTER this routine ends */
/* ie returns to the control of gtk_main_loop - so there is no way */
/* of preventing the dialog from getting in the way */
/* probably have to set a flag to do it after the next redraw_canvas() */
sysenv.snapshot = TRUE;
sysenv.snapshot_filename = g_build_filename(sysenv.cwd, name, NULL);
}

/*************************************************/
/* add a filename to active model's picture list */
/*************************************************/
void image_import(const gchar *name)
{
}

/***********************************/
/* get a filename for image export */
/***********************************/
void image_export_dialog(void)
{
}

/*******************************************/
/* get a file dialog for picture selection */
/*******************************************/
void image_import_dialog(void)
{
}

/**************************************/
/* retrieve a standard image's pixbuf */
/**************************************/
gpointer image_table_lookup(gint index)
{
if (index >= 0 && index<IMAGE_LAST)
  return(image_table[index]);
return(NULL);
}

/*****************************************/
/* free the internal pixbuf lookup table */
/*****************************************/
void image_table_free(void)
{
gint i;

for (i=IMAGE_LAST ; i-- ; )
  {
  if (image_table[i])
    g_object_unref(image_table[i]);
  }
}

/******************************************/
/* setup the internal pixbuf lookup table */
/******************************************/
void image_table_init(void)
{
gint i;

for (i=IMAGE_LAST ; i-- ; )
  {
/* init the name and pixbuf data */
  switch (i)
    {
    case IMAGE_ANIMATE:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_animate_xpm);
      break;
    case IMAGE_ARROW:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) arrow_xpm);
      break;
    case IMAGE_AXES:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) axes_xpm);
      break;
    case IMAGE_CANVAS_SINGLE:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) canvas_single_xpm);
      break;
    case IMAGE_CANVAS_CREATE:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) canvas_create_xpm);
      break;
    case IMAGE_CANVAS_DELETE:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) canvas_delete_xpm);
      break;
    case IMAGE_CAMERA:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) camera_xpm);
      break;
    case IMAGE_COMPASS:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) geom_xpm);
      break;
    case IMAGE_CROSS:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) cross_xpm);
      break;
    case IMAGE_DEFECT:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_defect_xpm);
      break;
    case IMAGE_DYNAMICS:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_dynamics_xpm);
      break;
    case IMAGE_DIFFRACTION:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_diffraction_xpm);
      break;
    case IMAGE_CABINET:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) cabinet_xpm);
      break;
    case IMAGE_COG:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_cog_xpm);
      break;
    case IMAGE_DISK:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) disk_xpm);
      break;
    case IMAGE_ELEMENT:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) element_xpm);
      break;
    case IMAGE_FOLDER:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) folder_xpm);
      break;
    case IMAGE_INFO:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_info_xpm);
      break;
    case IMAGE_ISOSURFACE:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_isosurface_xpm);
      break;
    case IMAGE_MEASURE:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) geom_xpm);
      break;
    case IMAGE_PERIODIC:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) cell_xpm);
      break;
    case IMAGE_PALETTE:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) palette_xpm);
      break;
    case IMAGE_PLUS:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) plus_xpm);
      break;    
    case IMAGE_PLOT:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) graph_xpm);
      break;    
    case IMAGE_SURFACE:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_surface_xpm);
      break;
    case IMAGE_OVERLAY:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tb_overlay_xpm);
      break;
    case IMAGE_TOOLS:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) tools_xpm);
      break;
    case IMAGE_USER:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) user_xpm);
      break;
    case IMAGE_ARCS:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) gdis_arcs_xpm);
      break;
    case IMAGE_HOME:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) home_xpm);
      break;
    case IMAGE_DOWNLOAD:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) download_xpm);
      break;
    case IMAGE_CONNECT:
      image_table[i] = gdk_pixbuf_new_from_xpm_data((const gchar **) connect_xpm);
      break;

    default:
      image_table[i] = NULL;
    }
  }
}

