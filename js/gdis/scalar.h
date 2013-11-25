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

gpointer scalar_field_new(const gchar *);
void scalar_field_free(gpointer);
void scalar_field_lattice_set(gdouble *, gpointer);
void scalar_field_size_set(gint *, gpointer);
void scalar_field_start_set(gdouble *, gpointer);
void scalar_field_values_set(gdouble *, gpointer);
void scalar_field_scan(gpointer);

gdouble scalar_field_min(gpointer);
gdouble scalar_field_max(gpointer);
gdouble scalar_field_value(gdouble *, gpointer);

