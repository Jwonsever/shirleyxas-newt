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

gpointer graph_new(const gchar *);
void graph_free(gpointer);
void graph_add_data(gint, gdouble *, gdouble, gdouble, gpointer);
void graph_set_data(gint, gdouble *, gdouble, gdouble, gpointer);
void graph_set_grafted(gint, gpointer);
void graph_set_xticks(gint, gint, gpointer);
void graph_set_yticks(gint, gint, gpointer);
void graph_set_wavelength(gdouble, gpointer);
void graph_set_select(gdouble, gchar *, gpointer);
gchar *graph_treename(gpointer);

void graph_set_parent(gpointer, gpointer);
void graph_set_id(gint, gpointer);

gpointer graph_get_parent(gpointer);
gint graph_get_id(gpointer);

gint graph_grafted(gpointer);
gint graph_xlabel(gpointer);
gint graph_ylabel(gpointer);
gdouble graph_xmin(gpointer);
gdouble graph_xmax(gpointer);
gdouble graph_ymin(gpointer);
gdouble graph_ymax(gpointer);
gdouble graph_wavelength(gpointer);

gdouble graph_ymean_get(gpointer);
gdouble graph_ysd_get(gpointer);

void graph_ymin_set(gdouble, gpointer);
void graph_ymax_set(gdouble, gpointer);

gdouble graph_x_value(gint, gpointer);
gint graph_x_index(gdouble, gpointer);

gdouble graph_xstart_get(gpointer);
gdouble graph_xstop_get(gpointer);
void graph_xstart_set(gdouble, gpointer);
void graph_xstop_set(gdouble, gpointer);
void graph_x_label_set(const gchar *, gpointer);
void graph_y_label_set(const gchar *, gpointer);

void graph_default_x(gpointer);
void graph_default_y(gpointer);
void graph_default_data(gpointer);

/* TODO - relocate */
gint anim_next_frame(struct model_pak *);

void graph_write(const gchar *, gpointer);

gint graph_overlay(gpointer, gpointer);

/*******************/
/* data structures */
/*******************/
struct graph_pak
{
/* GUI tree */
gint grafted;
gchar *treename;
gpointer parent;
gint id;

/* graph generation parameters */
gdouble wavelength;

/* axis labels on/off */
gint xlabel;
gint ylabel;

/* axis tick marks */
gint xticks;
gint yticks;

/* axis labels */
gchar *xname;
gchar *yname;

/* raw data extents (default plotting extents) */
gdouble xmin;
gdouble xmax;
gdouble ymin;
gdouble ymax;

/* CURRENT - stats */
// TODO - turn on/off? ie may not always be relevant
gdouble ymean;
gdouble ysd;

/* actual plotting extents */
gint start;
gint stop;
/* NB: all sets are required to be <= size */
gint size;
GSList *set_list;

/* peak selection */
gint select;
gchar *select_label;
};

