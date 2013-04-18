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
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "gdis.h"
#include "gui_shorts.h"
#include "dialog.h"
#include "scan.h"
#include "parse.h"
#include "interface.h"
#include "gui_image.h"

extern struct sysenv_pak sysenv;

#define MANUAL_FILE "gdis.manual"

GtkTextTag *help_tag_bold=NULL, *help_tag_italic=NULL, *help_tag_fixed=NULL;
GSList *help_topics=NULL;
GSList *help_text=NULL;

/**********************/
/* add a single topic */
/**********************/
void help_topic_new(const gchar *topic, const gchar *text)
{
if (topic)
  {
  help_topics = g_slist_append(help_topics, g_strdup(topic));

  if (text)
    help_text = g_slist_append(help_text, g_strdup(text));
  else
    help_text = g_slist_append(help_text, NULL);
  }
}

/************************/
/* free all help topics */
/************************/
void help_free(void)
{
free_slist(help_topics);
free_slist(help_text);
help_topics = NULL;
help_text = NULL;
}

/**************************************/
/* retrieve data for a topic (if any) */
/**************************************/
const gchar *help_topic_get(const gchar *topic)
{
gint i, n;
GSList *item1, *item2;

if (!topic)
  return(NULL);

i=0;
n = strlen(topic);
item1 = help_topics;
item2 = help_text;
while (item1 && item2)
  {
  if (g_strncasecmp(topic, item1->data, n) == 0)
    return(item2->data);
 
  item1 = g_slist_next(item1);
  item2 = g_slist_next(item2);
  }

return(NULL);
}

/********************/
/* XML text handler */
/********************/
void help_topic_text(GMarkupParseContext *context,
                     const gchar *text,
                     gsize text_len,  
                     gpointer data,
                     GError **error)
{
gint n;
const GSList *list;
gpointer pixbuf;
GtkTextMark *mark;
GtkTextBuffer *buffer=data;
GtkTextIter start, stop;

gtk_text_buffer_get_end_iter(buffer, &start);

/* check tags */
list=g_markup_parse_context_get_element_stack(context);

if (g_strncasecmp(list->data, "p\0", 2) == 0)
  {
  n = str_to_float(text);

//printf("request for image icon: %d\n", n);

  pixbuf = image_table_lookup(n);

  gtk_text_buffer_insert_pixbuf(buffer, &start, pixbuf);

  }
else
  {
/* mark the insertion point */
  mark = gtk_text_buffer_create_mark(buffer, NULL, &start, TRUE); 

/* insert the text */
  gtk_text_buffer_insert(buffer, &start, text, strlen(text));

/* get the adjusted start iter */
  gtk_text_buffer_get_iter_at_mark(buffer, &start, mark);
/* get the new stop iter */
  gtk_text_buffer_get_end_iter(buffer, &stop);

  if (g_strncasecmp(list->data, "h\0", 2) == 0)
    gtk_text_buffer_apply_tag(buffer, help_tag_bold, &start, &stop);

  if (g_strncasecmp(list->data, "i\0", 2) == 0)
    gtk_text_buffer_apply_tag(buffer, help_tag_italic, &start, &stop);

  if (g_strncasecmp(list->data, "f\0", 2) == 0)
    gtk_text_buffer_apply_tag(buffer, help_tag_fixed, &start, &stop);

/* cleanup */
  gtk_text_buffer_delete_mark(buffer, mark);
  }
}

/*******************************************/
/* parse a topic segment and do formatting */
/*******************************************/
void help_topic_parse(GtkTextBuffer *buffer, const gchar *source)
{
GMarkupParser xml_parser;
GMarkupParseContext *xml_context;
GError *error=NULL;

/*
printf("==========================================\n");
printf("%s\n", source);
printf("==========================================\n");
*/

//xml_parser.start_element = &help_topic_start;
//xml_parser.end_element = &help_topic_end;
xml_parser.start_element = NULL;
xml_parser.end_element = NULL;
xml_parser.text = &help_topic_text;
xml_parser.passthrough = NULL;
xml_parser.error = NULL;
xml_context = g_markup_parse_context_new(&xml_parser, 0, buffer, NULL);

/* clear */
gtk_text_buffer_set_text(buffer, "", 0);

/* parse the source */
if (!g_markup_parse_context_parse(xml_context, source, strlen(source), &error))
  {
  printf("help_document_parse() XML error:\n");
  printf("%s\n", error->message);
  }

/* cleanup */
g_markup_parse_context_free(xml_context);
}

/*********************/
/* XML start handler */
/*********************/
void help_document_start(GMarkupParseContext *context,
                         const gchar *element,
                         const gchar **names,
                         const gchar **values,
                         gpointer data,
                         GError **error)
{
gint i;
static gint n=1;
const gchar *title;
GString *buffer=data;

if (g_strncasecmp(element, "topic", 5) == 0)
  {
/* scan names/values for title */
  i=0;
  title=NULL;
  while (*(names+i))
    {
    if (g_ascii_strncasecmp(*(names+i), "title", 5) == 0)
      {
      title = *(values+i);
      }
    i++;
    }
/* first line of buffer -> topic title */
  if (title)
    g_string_printf(buffer, "%s\n", title);
  else
    g_string_printf(buffer, "title %d\n", n++);
  }

//else
/* TODO - save names/values too */
  g_string_append_printf(buffer, "<%s>", element);

}

/*******************/
/* XML end handler */
/*******************/
void help_document_end(GMarkupParseContext *context,
                       const gchar *element,
                       gpointer data,
                       GError **error)
{
static gint n=1;
gchar *title;
GString *buffer=data;

g_string_append_printf(buffer, "</%s>", element);

if (g_strncasecmp(element, "topic", 5) == 0)
  {
/* get title */
  title = parse_strip_newline(buffer->str);

/* safely skip title in buffer */
  n = strlen(title);
  if (n < strlen(buffer->str)-2)
    help_topic_new(title, &buffer->str[n+1]);
  else
    help_topic_new(title, buffer->str);

  g_free(title);
  }
}

/********************/
/* XML text handler */
/********************/
void help_document_text(GMarkupParseContext *context,
                        const gchar *text,
                        gsize text_len,  
                        gpointer data,
                        GError **error)
{
GString *buffer=data;

if (text)
  g_string_append(buffer, text);
}

/***********************************/
/* split source document by topics */
/***********************************/
void help_document_parse(const gchar *filename)
{
gchar *line;
gpointer scan;
GString *buffer;
GMarkupParser xml_parser;
GMarkupParseContext *xml_context;
GError *error=NULL;

scan = scan_new(filename);
if (!scan)
  return;

xml_parser.start_element = &help_document_start;
xml_parser.end_element = &help_document_end;
xml_parser.text = &help_document_text;
xml_parser.passthrough = NULL;
xml_parser.error = NULL;

buffer = g_string_new(NULL);
xml_context = g_markup_parse_context_new(&xml_parser, 0, buffer, NULL);

/* read in blocks (lines) of text */
line = scan_get_line(scan);
while (!scan_complete(scan))
  {
/* parse the line */
  if (!g_markup_parse_context_parse(xml_context, line, strlen(line), &error))
    {
    printf("help_document_parse() XML error:\n");
    printf("%s\n", error->message);
    break;
    }
  line = scan_get_line(scan);
  }

/* cleanup */
g_markup_parse_context_free(xml_context);
g_string_free(buffer, TRUE);
scan_free(scan);
}

/**************************************************/
/* load the help file and process the manual text */
/**************************************************/
void help_init(void)
{
gchar *filename;

/* open the manual file for scanning */
filename = g_build_filename(sysenv.gdis_path, MANUAL_FILE, NULL);

help_document_parse(filename);

g_free(filename);
}

/***************************/
/* topic selection handler */
/***************************/
void gui_help_topic_selected(GtkTreeSelection *selection, gpointer data)
{
const gchar *topic, *text;
GtkTreeIter iter;
GtkTreeModel *treemodel;
GtkTextBuffer *buffer;

/* record selection as the active entry */
if (gtk_tree_selection_get_selected(selection, &treemodel, &iter))
  {
  gtk_tree_model_get(treemodel, &iter, 0, &topic, -1);

  text = help_topic_get(topic);
  if (text)
    {
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
    help_topic_parse(buffer, text);
    }
  }
}

/****************************************/
/* fill out the available manual topics */
/****************************************/
void gui_help_init(GtkListStore *list, gpointer data)
{
const gchar *text;
GSList *topic;
GtkTreeIter iter;
GtkTextBuffer *buffer;

/* topics */
gtk_list_store_clear(list);
for (topic=help_topics ; topic ; topic=g_slist_next(topic))
  {
  gtk_list_store_append(list, &iter);
  gtk_list_store_set(list, &iter, 0, topic->data, -1);
  }

/* default page */
if (help_topics)
  {
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
  text = help_topic_get(help_topics->data);
  help_topic_parse(buffer, text);
  }
}

/*******************************************/
/* reload and display the gdis help manual */
/*******************************************/
void gui_help_refresh(GtkWidget *w, gpointer dialog)
{
dialog_destroy(NULL, dialog);

help_free();
help_init();

gui_help_dialog();
}

/****************************/
/* display a manual browser */
/****************************/
void gui_help_dialog(void)
{
gpointer dialog;
GtkCellRenderer *r;
GtkTreeViewColumn *c;
GtkListStore *list;
GtkTreeSelection *select;
GtkWidget *window, *swin, *hbox, *tree, *view;
GtkTextBuffer *buffer;

/* create dialog */
dialog = dialog_request(MANUAL, "Manual", NULL, NULL, NULL);
if (!dialog)
  return;
window = dialog_window(dialog);
gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);

/* split pane display */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), hbox, TRUE, TRUE, 1);

/* left pane - topics browser */
swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_NEVER, GTK_POLICY_NEVER);
gtk_box_pack_start(GTK_BOX(hbox), swin, FALSE, TRUE, 0);

/* list */
list = gtk_list_store_new(1, G_TYPE_STRING);
tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));
gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin), tree);
r = gtk_cell_renderer_text_new();
c = gtk_tree_view_column_new_with_attributes(" ", r, "text", 0, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c);
gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);

/* set the width of the topics pane */
gtk_widget_set_size_request(swin, 15*sysenv.gtk_fontsize, -1);

/* right pane - text */
swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_box_pack_start(GTK_BOX(hbox), swin, TRUE, TRUE, 0);
view = gtk_text_view_new();
gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
gtk_container_add(GTK_CONTAINER(swin), view);

/* configure the text viewing area */
gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
/* NB: GTK_JUSTIFY_FILL was not supported at the time this was written */
/*
gtk_text_view_set_justification(GTK_TEXT_VIEW(view), GTK_JUSTIFY_FILL);
*/
gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), PANEL_SPACING);
gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), PANEL_SPACING);


/* NB: these are associated with the text buffer and must be reallocated each */
/* time a new text buffer is created */
buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
help_tag_bold = gtk_text_buffer_create_tag(buffer, NULL, "weight", PANGO_WEIGHT_BOLD, NULL);
help_tag_italic = gtk_text_buffer_create_tag(buffer, NULL, "style", PANGO_STYLE_ITALIC, NULL);
//help_tag_fixed = gtk_text_buffer_create_tag(buffer, NULL, "font", "fixed 10", NULL); 
help_tag_fixed = gtk_text_buffer_create_tag(buffer, NULL, "font", "monospace 10", NULL); 


/* TODO - default selection of 1st item */
/* topic selection handler */
select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
g_signal_connect(G_OBJECT(select), "changed",
                 G_CALLBACK(gui_help_topic_selected),
                 view);

/* terminating button */
gui_stock_button(GTK_STOCK_REFRESH, gui_help_refresh, dialog, GTK_DIALOG(window)->action_area);
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog, GTK_DIALOG(window)->action_area);

/* populate the manual page */
gui_help_init(list, view);

/* expose the dialog */
gtk_widget_show_all(window);
}

/***************/
/* help dialog */
/***************/
/*
      g_string_assign(title,"OpenGL");
      g_string_sprintf(buff,"GL_RENDERER    %s\n", glGetString(GL_RENDERER));
      g_string_sprintfa(buff,"GL_VERSION       %s\n", glGetString(GL_VERSION));
      g_string_sprintfa(buff,"GL_VENDOR        %s\n", glGetString(GL_VENDOR));
*/


void gui_help_popup(const gchar *topic)
{
const gchar *text;
gpointer dialog;
GtkWidget *window, *swin, *hbox, *view;
GtkTextBuffer *buffer;

text = help_topic_get(topic);
if (!text)
  {
  gui_popup_message("No such help topic.");
  return;
  }

/* create dialog */
dialog = dialog_request(MANUAL, "Help Topic", NULL, NULL, NULL);
if (!dialog)
  return;
window = dialog_window(dialog);
gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);

/* split pane display */
hbox = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), hbox, TRUE, TRUE, 1);

/* left pane - topics browser */
/*
swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_NEVER, GTK_POLICY_NEVER);
gtk_box_pack_start(GTK_BOX(hbox), swin, FALSE, TRUE, 0);
*/

/* list */
/*
list = gtk_list_store_new(1, G_TYPE_STRING);
tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));
gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin), tree);
r = gtk_cell_renderer_text_new();
c = gtk_tree_view_column_new_with_attributes(" ", r, "text", 0, NULL);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c);
gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
*/

/* set the width of the topics pane */
//gtk_widget_set_size_request(swin, 15*sysenv.gtk_fontsize, -1);

/* right pane - text */
swin = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_box_pack_start(GTK_BOX(hbox), swin, TRUE, TRUE, 0);
view = gtk_text_view_new();
gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
gtk_container_add(GTK_CONTAINER(swin), view);

/* configure the text viewing area */
gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
/* NB: GTK_JUSTIFY_FILL was not supported at the time this was written */
/*
gtk_text_view_set_justification(GTK_TEXT_VIEW(view), GTK_JUSTIFY_FILL);
*/
gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), PANEL_SPACING);
gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), PANEL_SPACING);


/* NB: these are associated with the text buffer and must be reallocated each */
/* time a new text buffer is created */
buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
help_tag_bold = gtk_text_buffer_create_tag(buffer, NULL, "weight", PANGO_WEIGHT_BOLD, NULL);
help_tag_italic = gtk_text_buffer_create_tag(buffer, NULL, "style", PANGO_STYLE_ITALIC, NULL);
//help_tag_fixed = gtk_text_buffer_create_tag(buffer, NULL, "font", "fixed 10", NULL); 
help_tag_fixed = gtk_text_buffer_create_tag(buffer, NULL, "font", "monospace 10", NULL); 


/* TODO - default selection of 1st item */
/* topic selection handler */
/*
select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
g_signal_connect(G_OBJECT(select), "changed",
                 G_CALLBACK(gui_help_topic_selected),
                 view);
*/

/* terminating button */
gui_stock_button(GTK_STOCK_CLOSE, dialog_destroy, dialog, GTK_DIALOG(window)->action_area);

/* populate the manual page */
//gui_help_init(list, view);
help_topic_parse(buffer, text);

/* expose the dialog */
gtk_widget_show_all(window);
}

/************/
/* callback */
/************/
void gui_help_show(GtkWidget *w, gpointer data)
{
gui_help_popup(data);
}

