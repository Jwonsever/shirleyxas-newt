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

void zone_free(gpointer);
gpointer zone_make(gdouble, struct model_pak *);
void zone_init(struct model_pak *);
gint zone_index(gdouble *, gpointer);
gpointer zone_get(gdouble *, gpointer);
void zone_core_add(struct core_pak *, gpointer);

GSList *zone_cores(gpointer);
GSList *zone_shells(gpointer);
GSList *zone_area_cores(gint, gpointer, gpointer);

void zone_display_init(gpointer, struct model_pak *);
void zone_visible_init(struct model_pak *);

