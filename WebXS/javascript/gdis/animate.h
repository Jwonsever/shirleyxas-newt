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

void animate_model_init(struct model_pak *);
void animate_model_free(struct model_pak *);

gpointer animate_frame_new(struct model_pak *);
void animate_frame_store(struct model_pak *);

void animate_frame_core_add(gdouble *, gpointer);
void animate_frame_velocity_add(gdouble *, gpointer);
void animate_frame_lattice_set(gdouble *, gpointer);
void animate_frame_camera_set(gpointer, gpointer);

gint animate_frame_select(gint, struct model_pak *);
const gchar *animate_frame_property_get(gint, const gchar *, struct model_pak *);
void animate_frame_property_put(gint, const gchar *, const gchar *, struct model_pak *);

gpointer animate_render_new(const gchar *, struct model_pak *);
void animate_render_range_set(gint *, gpointer);
void animate_render_phonon_set(gint, gdouble, gdouble, gpointer);

// NEW
gint animate_render_single2(gpointer);
gint animate_render_single2_cleanup(gpointer);

void animate_render_single(gpointer);
void animate_render_movie(gpointer);
void animate_render_phonon(gpointer);
void animate_render_cleanup(gpointer);

void animate_ffmpeg_exec(const gchar *, const gchar *);

void animate_debug(struct model_pak *);

