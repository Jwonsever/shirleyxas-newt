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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gdis.h"
#include "gui_shorts.h"
#include "dialog.h"
#include "scan.h"
#include "file.h"
#include "parse.h"
#include "interface.h"

extern struct sysenv_pak sysenv;

/*****************************/
/* the gdis configure dialog */
/*****************************/
void gui_setup_dialog(void)
{
gint row=0;
gpointer dialog;
GtkWidget *window, *frame, *vbox, *table, *label;

dialog = dialog_request(SETUP, "Executable Locations", NULL, NULL, NULL);
if (!dialog)
  return;
window = dialog_window(dialog);

gtk_widget_set_size_request(window, 640, -1);

frame = gtk_frame_new(NULL);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), frame, FALSE, FALSE, PANEL_SPACING);
gtk_container_set_border_width(GTK_CONTAINER(frame), PANEL_SPACING);

table = gtk_table_new(2, 9, FALSE);
gtk_container_add(GTK_CONTAINER(frame), table);
gtk_container_set_border_width(GTK_CONTAINER(table), PANEL_SPACING);

label = gtk_label_new("Babel");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.babel_path, TRUE, TRUE, vbox);
row++;

label = gtk_label_new("GAMESS");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.gamess_path, TRUE, TRUE, vbox);
row++;

label = gtk_label_new("GULP");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.gulp_path, TRUE, TRUE, vbox);
row++;

label = gtk_label_new("Monty");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.monty_path, TRUE, TRUE, vbox);
row++;

label = gtk_label_new("SIESTA");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.siesta_path, TRUE, TRUE, vbox);
row++;

label = gtk_label_new("ReaxMD");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.reaxmd_path, TRUE, TRUE, vbox);
row++;

label = gtk_label_new("POVRay");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.povray_path, TRUE, TRUE, vbox);
row++;

label = gtk_label_new("Movie encoder          ");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.ffmpeg_path, TRUE, TRUE, vbox);
row++;

label = gtk_label_new("Image viewer          ");
gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
vbox = gtk_vbox_new(TRUE, 0);
gtk_table_attach_defaults(GTK_TABLE(table),vbox,1,2,row,row+1);
gui_text_entry(NULL, &sysenv.viewer_path, TRUE, TRUE, vbox);
row++;

/* terminating buttons */
gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Executable Locations", GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog, GTK_DIALOG(window)->action_area);

gtk_widget_show_all(window);
}
