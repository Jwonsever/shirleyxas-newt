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

void gui_project_widget(GtkWidget *);
void gui_project_widget_update(gpointer);
void gui_project_tab_visible_set(const gchar *, gint);
gint gui_project_tab_visible_get(const gchar *);
void gui_project_tab_toggle(const gchar *);
void gui_project_save(void);

void gui_project_schedule_widget(GtkWidget *);
void gui_project_monitor_widget(GtkWidget *);
void gui_remote_files_widget(GtkWidget *);
void gui_project_task_update(gpointer);
void gui_project_task_view_update(GtkWidget *, gpointer);

