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

void space_init(gpointer);
void space_free(gpointer);
gint space_lookup(struct model_pak *);

void space_make_p1(struct model_pak *);
void space_make_images(gint, struct model_pak *);
void space_make_supercell(struct model_pak *);

gint space_primitive_cell(struct model_pak *);

void space_image_widget_setup(GtkWidget *);
void space_image_widget_redraw(void);
void space_image_widget_reset(void);

