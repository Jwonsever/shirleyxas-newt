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
#include "host.h"
#include "task.h"
#include "file.h"
#include "import.h"
#include "project.h"
#include "parse.h"
#include "dialog.h"
#include "interface.h"
#include "gui_shorts.h"
#include "grid.h"
#include "grisu_client.h"
#include "gui_project.h"
#include "gui_image.h"

#ifdef WITH_PYTHON
#include "py_login.h"
#endif

#define DEFAULT_MYPROXY_LIFETIME 12

extern struct sysenv_pak sysenv;

#ifdef WITH_GRISU
GtkListStore *job_ls=NULL;
GtkWidget *job_tv=NULL;
GtkWidget *grid_status_entry;
GtkWidget *shib_password_entry;
GtkWidget *shib_username_entry;
GtkWidget *myproxy_username_entry, *myproxy_password_entry;
GtkWidget *grid_vo_combo;
GtkWidget *grid_configure_box;
GtkWidget *grid_service_entry;
GtkWidget *grid_myproxy_server_entry;
GtkWidget *grid_myproxy_port_entry;
gint grid_method = GRID_MYPROXY;
gpointer gui_grid_idp_pd=NULL;
GtkWidget *grid_import, *grid_cpus, *grid_memory, *grid_walltime;
GtkWidget *grid_status_bar;
/* grid applications */
GtkWidget *grid_entry_search=NULL;
// NEW - circumvent race conditions
//GStaticMutex gui_grid_mutex=G_STATIC_MUTEX_INIT;

/***********/
/* cleanup */
/***********/
void gui_grid_cleanup(GtkWidget *w, gpointer dummy)
{
g_static_mutex_unlock(&sysenv.grid_auth_mutex);
}

/*************************************/
/* replacement cleanup after connect */
/*************************************/
void grid_connect_done(gpointer dummy)
{
gchar *text;

if (grid_auth_get())
  {
  gui_text_show(STANDARD, "Authentication success.\n");
  grid_lifetime_set(DEFAULT_MYPROXY_LIFETIME*3600);
  }
else
  {
  gui_text_show(STANDARD, "Authentication failed.\n");
  }

/* widget udpdates */
if (grid_status_entry)
  {
  if (GTK_IS_ENTRY(grid_status_entry))
    {
/* status display */
    text = grid_status_text();
    gtk_entry_set_text(GTK_ENTRY(grid_status_entry), text);
    g_free(text);
/* security */
    gtk_entry_set_text(GTK_ENTRY(shib_password_entry), "");
    gtk_entry_set_text(GTK_ENTRY(myproxy_password_entry), "");
    gtk_entry_set_text(GTK_ENTRY(myproxy_username_entry), "");
    }
  }

if (!g_slist_length(grid_table_list_get()))
  gui_grid_refresh(NULL);

}

/********************************/
/* grid connect GUI update task */
/********************************/
void grid_connect_stop(gpointer dummy)
{
gchar *text;

/* if we're authenticated - kick off timer */
if (grid_auth_get())
  {
  grid_lifetime_set(DEFAULT_MYPROXY_LIFETIME*3600);
  gui_text_show(STANDARD, "Authentication success.\n");
  }

/* widget udpdates */
if (grid_status_entry)
  {
  if (GTK_IS_ENTRY(grid_status_entry))
    {
/* status display */
    text = grid_status_text();
    gtk_entry_set_text(GTK_ENTRY(grid_status_entry), text);
    g_free(text);
/* security */
    gtk_entry_set_text(GTK_ENTRY(shib_password_entry), "");
    gtk_entry_set_text(GTK_ENTRY(myproxy_password_entry), "");
    gtk_entry_set_text(GTK_ENTRY(myproxy_username_entry), "");
    }
  }

/* auth dependent updates */
/* wipe current keypair to prevent the current (bad) set messing */
/* up any future (eg shib) login */
//grisu_keypair_set("", "");


/* update list of discovered apps (if none) */
//gui_project_widget_update(tree_project_get());

/* refresh if nothing */
if (!g_slist_length(grid_table_list_get()))
  gui_grid_refresh(NULL);
}

/*******************************************/
/* apply appropriate authentication method */
/*******************************************/
// TODO - set this in init.jar (or make an argument)
#define DEBUG_GUI_CONNECT 0
void gui_grid_connect(GtkWidget *w, gpointer dummy)
{
gint status;
gchar *idp;
gdouble left;
const gchar *su, *sp, *mu, *mp;

/* NEW - auth too hard to run in the background */
/* argument passed to java_proxy_upload is getting correupted ... no idea why */
/* tried strdup on text (so it won't be destroyed when dialog closed) */
su = gtk_entry_get_text(GTK_ENTRY(shib_username_entry));
sp = gtk_entry_get_text(GTK_ENTRY(shib_password_entry));
mu = gtk_entry_get_text(GTK_ENTRY(myproxy_username_entry));
mp = gtk_entry_get_text(GTK_ENTRY(myproxy_password_entry));

idp = gui_pd_text(gui_grid_idp_pd);

/* tried closing the dialog and locking, didnt work */
/*
if (g_static_mutex_trylock(&sysenv.grid_auth_mutex))
  {
  }
else
  {
  gui_text_show(ERROR, "Fatal error trying to setup background authentication.\n");
  g_free(idp);
  return;
  }
*/

/* NEW - allow the GUI to be updated before we run our heavy task */
gtk_entry_set_text(GTK_ENTRY(grid_status_entry), "Authenticating, please wait...");
while (gtk_events_pending())
  gtk_main_iteration();

/* case 1 - myproxy */
if (strlen(mu) && strlen(mp))
  {
/* case 2 - authenticate using myproxy */
  grisu_auth_set(TRUE);
  grisu_keypair_set(mu, mp);
//  task_new("Authenticating", grid_auth_check, NULL, grid_connect_done, NULL, NULL);
  grid_auth_check();
  }
else
  {
/* default - shibboleth */
//  idp = gui_pd_text(gui_grid_idp_pd);
  grid_idp_set(idp);
  grid_user_set(su);
/* if the usercert/userkey has a long enough lifetime still left */
/* then dont bug slcs for a new pair - just upload a new myproxy cred */
  left = grid_pem_hours_left(grid_cert_file());
//printf("left = %f\n", left);

// TODO - cutoff reasonable?
  if (left < 6)
    {
#if DEBUG_GUI_CONNECT
printf("Retrieving new SLCS credential.\n");
#endif
    status = grid_java_slcs_login(idp, su, sp);
    left = grid_pem_hours_left(grid_cert_file());
    if (left < 0.0)
      {
#if DEBUG_GUI_CONNECT
printf("Failed to obtain SLCS certificate.\n");
#endif
      grid_auth_set(FALSE);
      goto gui_grid_connect_done;
      }
    }
#if DEBUG_GUI_CONNECT
  else
    printf("Re-using existing SLCS cert...\n");
#endif

//  task_new("Authenticating", grid_java_proxy_upload, sp, grid_connect_stop, NULL, NULL);
//  task_new("Authenticating", grid_java_proxy_upload, sp, gui_grid_cleanup, NULL, NULL);
//  task_new("Authenticating", grid_java_proxy_upload, sp, grid_connect_done, NULL, NULL);
/* NEW - background jobs seem to succeed/fail on certain platforms */
/* with no clear pattern ... so lets just run it in the foreground */
  if (!grid_java_proxy_upload(sp))
    {
#if DEBUG_GUI_CONNECT
printf("Failed to upload proxy credential.\n");
#endif
    grid_auth_set(FALSE);
    }
  }

gui_grid_connect_done:;

grid_connect_done(NULL);

g_free(idp);
}

/**********************************************/
/* remove SLCS certificate and deauthenticate */
/**********************************************/
void gui_grid_disconnect(GtkWidget *w, gpointer dummy)
{
// disconnect
grid_disconnect();

// remove state information (downloaded job names)
project_job_remove_all(sysenv.workspace);

gtk_entry_set_text(GTK_ENTRY(grid_status_entry), "Not authenticated.");
}

/***************************/
/* configuration callbacks */
/***************************/
void gui_grid_ws_change(GtkWidget *w, gpointer dummy)
{
const gchar *ws;
ws = gtk_entry_get_text(GTK_ENTRY(grid_service_entry));
grisu_ws_set(ws);
}

void gui_grid_server_change(GtkWidget *w, gpointer dummy)
{
const gchar *server;
server = gtk_entry_get_text(GTK_ENTRY(grid_myproxy_server_entry));
grisu_myproxy_set(server, NULL);
}

void gui_grid_port_change(GtkWidget *w, gpointer dummy)
{
const gchar *port;
port = gtk_entry_get_text(GTK_ENTRY(grid_myproxy_port_entry));
grisu_myproxy_set(NULL, port);
}

/***********************/
/* configuration panel */
/***********************/
void gui_grid_configure_page(GtkWidget *box)
{
GtkWidget *table, *label, *button;

g_assert(box != NULL);

table = gtk_table_new(2, 4, FALSE);
gtk_container_add(GTK_CONTAINER(box), table);

label = gtk_label_new("Service interface");
gtk_table_attach(GTK_TABLE(table),label,0,1,0,1, GTK_SHRINK, GTK_SHRINK, 0, 0);

label = gtk_label_new("Myproxy server ");
gtk_table_attach(GTK_TABLE(table),label,0,1,1,2, GTK_SHRINK, GTK_SHRINK, 0, 0);

label = gtk_label_new("Mpyroxy port ");
gtk_table_attach(GTK_TABLE(table),label,0,1,2,3, GTK_SHRINK, GTK_SHRINK, 0, 0);

grid_service_entry = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(grid_service_entry), grisu_ws_get());
gtk_table_attach_defaults(GTK_TABLE(table),grid_service_entry,1,2,0,1);
g_signal_connect(GTK_OBJECT(grid_service_entry), "changed",
                 GTK_SIGNAL_FUNC(gui_grid_ws_change), NULL);

grid_myproxy_server_entry = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(grid_myproxy_server_entry), grisu_myproxy_get());
gtk_table_attach_defaults(GTK_TABLE(table),grid_myproxy_server_entry,1,2,1,2);
g_signal_connect(GTK_OBJECT(grid_myproxy_server_entry), "changed",
                 GTK_SIGNAL_FUNC(gui_grid_server_change), NULL);

grid_myproxy_port_entry = gtk_entry_new();
gtk_entry_set_text(GTK_ENTRY(grid_myproxy_port_entry), grisu_myproxyport_get());
gtk_table_attach_defaults(GTK_TABLE(table),grid_myproxy_port_entry,1,2,2,3);
g_signal_connect(GTK_OBJECT(grid_myproxy_port_entry), "changed",
                 GTK_SIGNAL_FUNC(gui_grid_port_change), NULL);

/* NEW */
button = gui_checkbox("Use GridFTP ", &sysenv.grid_use_gridftp, NULL);
gtk_table_attach_defaults(GTK_TABLE(table),button,1,2,3,4);
}

/*********************************************/
/* update the authentication status/lifetime */
/*********************************************/
gint gui_grid_status_handler(void)
{
gchar *text;

if (!grid_status_entry)
  return(FALSE);

if (GTK_IS_ENTRY(grid_status_entry))
  {
  text = grid_status_text();
  gtk_entry_set_text(GTK_ENTRY(grid_status_entry), text);
  g_free(text);
  return(TRUE);
  }
return(FALSE);
}

/**************************************************************/
/* update auth dialog with username/idp/etc defaults (if any) */
/**************************************************************/
void gui_auth_dialog_defaults(void)
{
#ifdef WITH_PYTHON
gchar *text;
GSList *list;

list = py_idp_lookup();
gui_pd_list_set(list, gui_grid_idp_pd);

/* TODO - set with default IDP */
text = grid_user_get();
if (text)
  gtk_entry_set_text(GTK_ENTRY(shib_username_entry), text);

#endif
}

/***********************************/
/* post background idp list update */
/***********************************/
void gui_grid_idp_setup(gpointer dummy)
{
GSList *list=NULL;

#ifdef WITH_PYTHON
list = py_idp_lookup();
gui_pd_list_set(list, gui_grid_idp_pd);
#else
list = grid_idp_list_get();
gui_pd_list_set(list, gui_grid_idp_pd);
#endif
}

/****************************************/
/* fill out defaults in the auth dialog */
/****************************************/
void gui_auth_defaults(void)
{
gchar *text;
GSList *list=NULL;

text = grid_user_get();
if (text)
  gtk_entry_set_text(GTK_ENTRY(shib_username_entry), text);

//from https://slcs1.arcs.org.au/idp-acl.txtC
  list = g_slist_prepend(list, "University of Canterbury");
  list = g_slist_prepend(list, "Lincoln University");
  list = g_slist_prepend(list, "The University of Auckland");
  list = g_slist_prepend(list, "VPAC");
  list = g_slist_prepend(list, "TPAC");
  list = g_slist_prepend(list, "ANSTO");
  list = g_slist_prepend(list, "iVEC");
  list = g_slist_prepend(list, "ARCS IdP");
  list = g_slist_prepend(list, "eResearch SA");
  list = g_slist_prepend(list, "ac3");
  list = g_slist_prepend(list, "The University of Queensland");
  list = g_slist_prepend(list, "James Cook University");
  list = g_slist_prepend(list, "Monash University");
  list = g_slist_prepend(list, "Griffith University");
  list = g_slist_prepend(list, "University of Tasmania");
  list = g_slist_prepend(list, "Curtin University of Technology");
  list = g_slist_prepend(list, "Australian Catholic University");
  list = g_slist_prepend(list, "University of South Australia");

// add the default (if any) idp to the top - so it's displayed first
// TODO - remove from standard list (above) so it's not displayed twice
text = grid_idp_get();
if (text)
  list = g_slist_prepend(list, text);

gui_pd_list_set(list, gui_grid_idp_pd);
}

/**********************/
/* grid configuration */
/**********************/
void gui_grid_setup_page(GtkWidget *box)
{
GtkWidget *notebook, *table, *page, *hbox, *label, *image;
GdkPixbuf *pixbuf;

notebook = gtk_notebook_new();
gtk_box_pack_start(GTK_BOX(box), notebook, FALSE, FALSE, 0);

/* method - shibboleth */
page = gtk_vbox_new(FALSE, 0);
label = gtk_label_new("Authenticate");
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, FALSE, 0);
table = gtk_table_new(3, 2, FALSE);
gtk_box_pack_start(GTK_BOX(hbox), table, TRUE, TRUE, 0);

// ARCS logo
pixbuf = image_table_lookup(IMAGE_ARCS);
if (pixbuf)
  {
  image = gtk_image_new_from_pixbuf(pixbuf);
  gtk_box_pack_end(GTK_BOX(hbox), image, FALSE, FALSE, PANEL_SPACING);
  }

label = gtk_label_new(" Institution ");
gtk_table_attach(GTK_TABLE(table),label,0,1,0,1, GTK_SHRINK, GTK_SHRINK, 0, 0);

label = gtk_label_new(" Username ");
gtk_table_attach(GTK_TABLE(table),label,0,1,1,2, GTK_SHRINK, GTK_SHRINK, 0, 0);

label = gtk_label_new(" Password ");
gtk_table_attach(GTK_TABLE(table),label,0,1,2,3, GTK_SHRINK, GTK_SHRINK, 0, 0);

hbox = gtk_hbox_new(FALSE, 0);

//gui_grid_idp_pd = gui_pd_new(NULL, NULL, grid_idp_list_get(), hbox);
gui_grid_idp_pd = gui_pd_new(NULL, NULL, NULL, hbox);
gtk_table_attach_defaults(GTK_TABLE(table),hbox,1,2,0,1);

shib_username_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),shib_username_entry,1,2,1,2);

shib_password_entry = gtk_entry_new();
gtk_entry_set_visibility(GTK_ENTRY(shib_password_entry), FALSE);
gtk_table_attach_defaults(GTK_TABLE(table),shib_password_entry,1,2,2,3);

g_signal_connect(GTK_OBJECT(shib_password_entry), "activate",
                 GTK_SIGNAL_FUNC(gui_grid_connect), NULL);

/* method - grid cert */
/*
page = gtk_hbox_new(FALSE, 0);
label = gtk_label_new("Certificate");

gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

label = gtk_label_new("Grid Passphrase ");
gtk_box_pack_start(GTK_BOX(page), label, FALSE, FALSE, 0);

grid_password_entry = gtk_entry_new();
gtk_entry_set_visibility(GTK_ENTRY(grid_password_entry), FALSE);
gtk_box_pack_start(GTK_BOX(page), grid_password_entry, TRUE, TRUE, 0);

g_signal_connect(GTK_OBJECT(grid_password_entry), "activate",
                 GTK_SIGNAL_FUNC(gui_grid_connect), NULL);
*/

/* method - grid + myproxy */
page = gtk_vbox_new(FALSE, 0);

label = gtk_label_new("Advanced");

gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, label);

table = gtk_table_new(3, 2, FALSE);
gtk_box_pack_start(GTK_BOX(page), table, FALSE, FALSE, 0);

/*
label = gtk_label_new(" Private key passphrase ");
gtk_table_attach(GTK_TABLE(table),label,0,1,0,1, GTK_SHRINK, GTK_SHRINK, 0, 0);
*/

label = gtk_label_new(" MyProxy Username ");
gtk_table_attach(GTK_TABLE(table),label,0,1,1,2, GTK_SHRINK, GTK_SHRINK, 0, 0);

label = gtk_label_new(" MyProxy Password ");
gtk_table_attach(GTK_TABLE(table),label,0,1,2,3, GTK_SHRINK, GTK_SHRINK, 0, 0);

/*
private_passphrase_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),private_passphrase_entry,1,2,0,1);
gtk_entry_set_visibility(GTK_ENTRY(private_passphrase_entry), FALSE);
*/

/*
gtk_widget_set_sensitive(private_passphrase_entry, FALSE);
*/

myproxy_username_entry = gtk_entry_new();
gtk_table_attach_defaults(GTK_TABLE(table),myproxy_username_entry,1,2,1,2);

myproxy_password_entry = gtk_entry_new();
gtk_entry_set_visibility(GTK_ENTRY(myproxy_password_entry), FALSE);
gtk_table_attach_defaults(GTK_TABLE(table),myproxy_password_entry,1,2,2,3);

g_signal_connect(GTK_OBJECT(myproxy_password_entry), "activate",
                 GTK_SIGNAL_FUNC(gui_grid_connect), NULL);

grid_configure_box = gui_toggled_vbox("Configuration", FALSE, page);
gui_grid_configure_page(grid_configure_box);

/* status */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);

grid_status_entry = gtk_entry_new();
gtk_box_pack_start(GTK_BOX(hbox), grid_status_entry, TRUE, TRUE, 0);

// method 1
//gui_label_button(" Connect ", gui_grid_connect, NULL, hbox);


// method 2
{
GtkWidget *toolbar;
GtkWidget *image;
GdkPixbuf *pixbuf;


toolbar = gtk_toolbar_new();
gtk_box_pack_start(GTK_BOX(hbox), toolbar, FALSE, FALSE, 0);

pixbuf = image_table_lookup(IMAGE_CONNECT);
image = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), NULL, "Connect", "Private",
                        image, GTK_SIGNAL_FUNC(gui_grid_connect), NULL);

pixbuf = image_table_lookup(IMAGE_CROSS);
image = gtk_image_new_from_pixbuf(pixbuf);
gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), NULL, "Disconnect", "Private",
                        image, GTK_SIGNAL_FUNC(gui_grid_disconnect), NULL);
}
}

/********************************************/
/* close dialog (if nothing is in progress) */
/********************************************/
void gui_grid_close(GtkWidget *w, gpointer data)
{
/* destroy */
dialog_destroy(NULL, data);
}

/*****************************/
/* the gdis job setup dialog */
/*****************************/
#define DEBUG_AUTH_DIALOG 0
void gui_auth_dialog(void)
{
gpointer dialog;
GtkWidget *window, *vbox, *page;

if (g_static_mutex_trylock(&sysenv.grid_auth_mutex))
  {
#if DEBUG_AUTH_DIALOG
printf("gui_grid_mutex lock success.\n");
#endif
  }
else
  {
#if DEBUG_AUTH_DIALOG
printf("gui_grid_mutex lock failed.\n");
#endif
  gui_text_show(WARNING, "Please wait for grid operation to complete.\n");
  return;
  }

dialog = dialog_request(666, "Grid Authentication", NULL, NULL, NULL);
if (!dialog)
  return;
window = dialog_window(dialog);

vbox = gtk_vbox_new(FALSE, PANEL_SPACING);
gtk_container_add(GTK_CONTAINER(GTK_DIALOG(window)->vbox), vbox);

/* HACK - default seems to be too small */
gtk_widget_set_size_request(window, 450, 350);

/* authentication display */
page = gui_frame_vbox(NULL, FALSE, FALSE, vbox);
gui_grid_setup_page(page);

/* configuration display */
/* hidden by default ... put in it's own tab? */
/*
grid_configure_box = gui_toggled_vbox("Configuration", FALSE, vbox);
gui_grid_configure_page(grid_configure_box);
*/

/* known (localy cached) application/queue pairs */
/*
page = gui_frame_vbox(NULL, TRUE, TRUE, vbox);
gui_grid_discover_page(page);
*/

/* TODO - disallow even X close button if in the middle of a grid operation */
gui_stock_button(GTK_STOCK_HELP, gui_help_show, "Grid Authentication", GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_CLOSE, gui_grid_close, dialog, GTK_DIALOG(window)->action_area);

gtk_widget_show_all(window);

/* default to hiden */
gtk_widget_hide(grid_configure_box);

// ugh - update status on entry + if auth action ... not in a timeout
gui_grid_status_handler();

g_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gui_grid_cleanup), NULL);

#ifdef WITH_PYTHON
gui_auth_dialog_defaults();
#else
gui_auth_defaults();
#endif
}

#endif

/*************************************/
/* request for authentication dialog */
/*************************************/
void gui_grid_dialog(void)
{
#ifdef WITH_GRISU
gui_auth_dialog();
#endif

/* CURRENT - this can be made to work - datafabric hookup */
//TODO - ensure we have a usable proxy
/*
{
GSList *list=NULL;
grid_gsiftp_list(&list, "gsiftp://arcs-df.vpac.org/ARCS/home");
//grid_gsiftp_list(&list, "gsiftp://arcs-df.vpac.org/~");
//grid_gsiftp_list(&list, "gsiftp://arcs-df.ivec.org/ARCS/home/~");
}
*/

}

