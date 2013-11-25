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
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "gdis.h"
#include "graph.h"
#include "matrix.h"
#include "numeric.h"
#include "file.h"
#include "import.h"
#include "parse.h"
#include "dialog.h"
#include "interface.h"

/* externals */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/******************************************/
/* extract info from graph data structure */
/******************************************/
gint graph_get_id(gpointer data)
{
struct graph_pak *graph = data;

g_assert(graph != NULL);

return(graph->id);
}

/******************************************/
/* extract info from graph data structure */
/******************************************/
gpointer graph_get_parent(gpointer data)
{
struct graph_pak *graph = data;

g_assert(graph != NULL);

return(graph->parent);
}

/******************************************/
/* extract info from graph data structure */
/******************************************/
gchar *graph_treename(gpointer data)
{
struct graph_pak *graph = data;

g_assert(graph != NULL);

return(graph->treename);
}

/***************************/
/* free a particular graph */
/***************************/
void graph_free(gpointer data)
{
struct graph_pak *graph = data;

free_slist(graph->set_list);

g_free(graph->treename);
g_free(graph->xname);
g_free(graph->yname);

g_free(graph); 
}

/************************/
/* allocate a new graph */
/************************/
/* return an effective graph id */
gpointer graph_new(const gchar *name)
{
struct graph_pak *graph;

graph = g_malloc(sizeof(struct graph_pak));

if (name)
  graph->treename = g_strdup_printf("%s", name);
else
  graph->treename = g_strdup_printf("graph");

graph->id = -1;
graph->parent = NULL;

graph->wavelength = 0.0;
graph->grafted = FALSE;
graph->xlabel = TRUE;
graph->ylabel = TRUE;
graph->xname = NULL;
graph->yname = NULL;
graph->xmin = 0.0;
graph->xmax = 0.0;
graph->ymin = 0.0;
graph->ymax = 0.0;
graph->ymean = 0.0;
graph->ysd = 0.0;
graph->xticks = 5;
graph->yticks = 5;
graph->start = 0;
graph->stop = 0;
graph->size = 0;
graph->select = -1;
graph->select_label = NULL;
graph->set_list = NULL;

return(graph);
}

/**************/
/* axes setup */
/**************/
//void graph_init_y(gdouble *x, struct graph_pak *graph)
void graph_init_y(struct graph_pak *graph)
{
gdouble ymin, ymax;
GSList *item;

g_assert(graph != NULL);

/* get min/max values */
item=graph->set_list;
if (item)
  {
/* calculate non-arbitrary start values */
  graph->ymin = min(graph->size, item->data);
  graph->ymax = max(graph->size, item->data);

/* factor any additional data sets */
  item = g_slist_next(item);
  while (item)
    {
    ymin = min(graph->size, item->data);
    ymax = max(graph->size, item->data);

    if (ymin < graph->ymin)
      graph->ymin = ymin;
  
    if (ymax > graph->ymax)
      graph->ymax = ymax;

    item = g_slist_next(item);
    }
  }
}

/********************************/
/* restore default x axis range */
/********************************/
void graph_default_x(gpointer data)
{
struct graph_pak *graph = data;

if (!graph)
  return;

graph->start = 0;
graph->stop = graph->size;
}

/********************************/
/* restore default y axis range */
/********************************/
void graph_default_y(gpointer data)
{
if (data)
  graph_init_y(data);
}

/*******************/
/* tree graft flag */
/*******************/
void graph_set_grafted(gint value, gpointer data)
{
struct graph_pak *graph = data;

if (!graph)
  return;

graph->grafted = value;
}

/************************************************/
/* revert to original single displayed data set */
/************************************************/
void graph_default_data(gpointer data)
{
gchar **buff;
GSList *item, *next;
struct graph_pak *graph = data;

if (!graph)
  return;

/* free all data except for the first set */
item = g_slist_next(graph->set_list);
while (item)
  {
  next = g_slist_next(item);

/* NB: we MUST free item->data before removing that element from the list */
/* since removing it from the list will actually change the value of the */
/* pointer itself ie item->data will change since it's part of the */
/* data structure of a GSList and is affected by g_slist_remove */
  g_free(item->data);
  graph->set_list = g_slist_remove(graph->set_list, item->data);

  item = next;
  }

/* NEW - reset yname */
buff = g_strsplit(graph->yname,"|",-1);
if (buff)
  {
  g_free(graph->yname);
  graph->yname = g_strdup(*buff);
  g_strfreev(buff);
  }
}

/**************************/
/* graph parent primitive */
/**************************/
void graph_set_parent(gpointer parent, gpointer data)
{
struct graph_pak *graph = data;

g_assert(graph != NULL);

graph->parent = parent;
}

/**************************/
/* graph parent primitive */
/**************************/
void graph_set_id(gint id, gpointer data)
{
struct graph_pak *graph = data;

g_assert(graph != NULL);

graph->id = id;
}

/********************/
/* set x axis label */
/********************/
void graph_x_label_set(const gchar *label, gpointer data)
{
struct graph_pak *graph = data;

g_free(graph->xname);
if (label)
  graph->xname = g_strdup(label);
else
  graph->xname = NULL;
}

/********************/
/* set y axis label */
/********************/
void graph_y_label_set(const gchar *label, gpointer data)
{
struct graph_pak *graph = data;

g_free(graph->yname);
if (label)
  graph->yname = g_strdup(label);
else
  graph->yname = NULL;
}

/**************************/
/* control label printing */
/**************************/
void graph_set_xticks(gint label, gint ticks, gpointer ptr_graph)
{
struct graph_pak *graph = ptr_graph;

g_assert(graph != NULL);
g_assert(ticks > 1);

graph->xlabel = label;
graph->xticks = ticks;
}

/**************************/
/* control label printing */
/**************************/
void graph_set_yticks(gint label, gint ticks, gpointer ptr_graph)
{
struct graph_pak *graph = ptr_graph;

g_assert(graph != NULL);
g_assert(ticks > 1);

graph->ylabel = label;
graph->yticks = ticks;
}

/**********************/
/* special graph data */
/**********************/
void graph_set_wavelength(gdouble wavelength, gpointer ptr_graph)
{
struct graph_pak *graph = ptr_graph;

g_assert(graph != NULL);

graph->wavelength = wavelength;
}

void graph_set_select(gdouble x, gchar *label, gpointer data)
{
gdouble n;
struct graph_pak *graph = data;

g_assert(graph != NULL);

/*
printf("x = %f, min, max = %f, %f\n", x, graph->xmin, graph->xmax);
*/

/* locate the value's position in the data point list */
n = (x - graph->xmin) / (graph->xmax - graph->xmin);
n *= graph->size;

graph->select = (gint) n;
g_free(graph->select_label);
if (label)
  graph->select_label = g_strdup(label);
else
  graph->select_label = NULL;

/*
printf("select -> %d : [0, %d]\n", graph->select, graph->size);
*/
}

/*****************************/
/* add dependent data set(s) */
/*****************************/
void graph_add_data(gint size, gdouble *x, gdouble x1, gdouble x2, gpointer data)
{
gdouble *ptr;
struct graph_pak *graph = data;

g_assert(graph != NULL);

/* try to prevent the user supplying different sized data */
/* TODO - sample in some fashion if different? */
if (graph->size)
  g_assert(graph->size == size);
else
  graph->size = size;

/* default extents are the entire array of bins */
graph->start = 0;
graph->stop = size;
graph->size = size;

ptr = g_malloc(size*sizeof(gdouble));

memcpy(ptr, x, size*sizeof(gdouble));

graph->xmin = x1;
graph->xmax = x2;
//graph_init_y(x, graph);

graph->set_list = g_slist_append(graph->set_list, ptr);

graph_init_y(graph);
}

/**********************************************/
/* overwrite existing graph with new data set */
/**********************************************/
void graph_set_data(gint size, gdouble *x, gdouble x1, gdouble x2, gpointer data)
{
gdouble *ptr;
struct graph_pak *graph = data;

g_assert(graph != NULL);

free_slist(graph->set_list);

/* default extents are the entire array of bins */
graph->start = 0;
graph->stop = size;
graph->size = size;

ptr = g_malloc(size*sizeof(gdouble));
memcpy(ptr, x, size*sizeof(gdouble));

graph->xmin = x1;
graph->xmax = x2;
//graph_init_y(x, graph);

graph->set_list = g_slist_append(graph->set_list, ptr);
graph_init_y(graph);
}

/************************************/
/* graph data extraction primitives */
/************************************/
gdouble graph_xmin(gpointer data)
{
struct graph_pak *graph = data;
return(graph->xmin);
}

gdouble graph_xmax(gpointer data)
{
struct graph_pak *graph = data;
return(graph->xmax);
}

gdouble graph_ymin(gpointer data)
{
struct graph_pak *graph = data;
return(graph->ymin);
}

gdouble graph_ymax(gpointer data)
{
struct graph_pak *graph = data;
return(graph->ymax);
}

gint graph_ylabel(gpointer data)
{
struct graph_pak *graph = data;
return(graph->ylabel);
}

gdouble graph_wavelength(gpointer data)
{
struct graph_pak *graph = data;
return(graph->wavelength);
}

gint graph_grafted(gpointer data)
{
struct graph_pak *graph = data;
return(graph->grafted);
}

/**********************************************/
/* translate array index to x axis real value */
/**********************************************/
gdouble graph_x_value(gint index, gpointer data)
{
gdouble  x;
struct graph_pak *graph = data;

/* check */
if (index < 0)
  index = 0;
if (index >= graph->size)
  index = graph->size-1;

/* translate */
x = graph->xmax - graph->xmin;
x *= (gdouble) index;
x /= (gdouble) (graph->size-1);
x += graph->xmin;

return(x);
}

/**********************************************/
/* translate x axis real value to array index */
/**********************************************/
gint graph_x_index(gdouble x, gpointer data)
{
gint i;
struct graph_pak *graph = data;

/* check */
if (x < graph->xmin)
  x = graph->xmin;
if (x > graph->xmax)
  x = graph->xmax;

/* translate */
x -= graph->xmin;
x *= (graph->size-1);
x /= (graph->xmax - graph->xmin);
i = nearest_int(x);

return(i);
}

/****************************************************************/
/* translate the current x start value into actual x axis value */
/****************************************************************/
gdouble graph_xstart_get(gpointer data)
{
struct graph_pak *graph = data;

return(graph_x_value(graph->start, data));
}

/***************************************************************/
/* translate the current x stop value into actual x axis value */
/***************************************************************/
gdouble graph_xstop_get(gpointer data)
{
struct graph_pak *graph = data;

return(graph_x_value(graph->stop, data));
}

/****************************************************/
/* translate real x axis start value to array index */
/****************************************************/
void graph_xstart_set(gdouble x, gpointer data)
{
struct graph_pak *graph = data;

graph->start = graph_x_index(x, data);
}

/***************************************************/
/* translate real x axis stop value to array index */
/***************************************************/
void graph_xstop_set(gdouble x, gpointer data)
{
struct graph_pak *graph = data;

graph->stop = graph_x_index(x, data);
}

/***************************/
/* set min value on y axis */
/***************************/
void graph_ymin_set(gdouble y, gpointer data)
{
struct graph_pak *graph = data;

graph->ymin = y;
}

/***************************/
/* set max value on y axis */
/***************************/
void graph_ymax_set(gdouble y, gpointer data)
{
struct graph_pak *graph = data;

graph->ymax = y;
}

/**********************************/
/* statistics retrieval primitive */
/**********************************/
gdouble graph_ymean_get(gpointer data)
{
struct graph_pak *graph = data;

return(graph->ymean);
}

/**********************************/
/* statistics retrieval primitive */
/**********************************/
gdouble graph_ysd_get(gpointer data)
{
struct graph_pak *graph = data;

return(graph->ysd);
}

/********************************************/
/* overlay one graph with data from another */
/********************************************/
// adds 1st data set from g2 to g1's display list
gint graph_overlay(gpointer g1, gpointer g2)
{
gint i, j;
gchar *text;
gdouble x, *y_new, *y_overlay;
struct graph_pak *base=g1, *overlay=g2;

/* checks */
if (!base || !overlay)
  return(FALSE);
if (base->size < 1)
  return(FALSE);

/* y data to overlay */
y_overlay = g_slist_nth_data(overlay->set_list, 0);
if (!y_overlay)
  return(FALSE);

/* new y data to add to base model */
y_new = g_malloc(base->size*sizeof(gdouble));

for (i=0 ; i<base->size ; i++)
  {
  x = graph_x_value(i, base);
  j = graph_x_index(x, overlay);

//printf("index: [%d] -> [%d]\n", i, j);

  if (j >= 0 && j < overlay->size)
    y_new[i] = y_overlay[j];
  else
    y_new[i] = 0.0;
  }

/* TODO - recalculate ymin/ymax for BOTH sets */
if (overlay->ymax > base->ymax)
  base->ymax = overlay->ymax;

if (overlay->ymin < base->ymin)
  base->ymin = overlay->ymin;

base->set_list = g_slist_append(base->set_list, y_new);

/* CURRENT - adjust yname to indicate extra data set */
text = g_strdup_printf("%s | %s", base->yname, overlay->yname);
g_free(base->yname);
base->yname = text;

return(TRUE);
}

/*******************/
/* graph exporting */
/*******************/
void graph_write(const gchar *fullpath, gpointer data)
{
gint i;
gdouble x, *y, xstep;
GSList *list;
GString *header, *line;
FILE *fp;
struct graph_pak *graph = data;

/* checks */
g_assert(graph != NULL);
g_assert(fullpath != NULL);

/* init */
fp = fopen(fullpath, "wt");
if (!fp)
  return;

/* header */
header = g_string_new("#");
if (graph->xname)
  g_string_append_printf(header, " %s", graph->xname);
else
  g_string_append_printf(header, " x axis");

/* TODO - cope with multi-column plots */
if (graph->yname)
  g_string_append_printf(header, " | %s", graph->yname);
else
  g_string_append_printf(header, " | y axis");

fprintf(fp, "%s\n", header->str);

g_string_free(header, TRUE);

/* init */
/*
if (graph->size)
  xstep = (graph->xmax - graph->xmin + 1.0) / (gdouble) graph->size;
else
  xstep = 1.0;
for (list=graph->set_list ; list ; list=g_slist_next(list))
  {
  y = (gdouble *) list->data;
  x = graph->xmin;
  for (i=0 ; i<graph->size ; i++)
    {
    fprintf(fp, "%f %f\n", x, y[i]);
    x += xstep;
    }
  }
*/

/* NEW */
line = g_string_new(NULL);

for (i=graph->start ; i<graph->stop ; i++)
  {
  x = graph_x_value(i, graph);

  g_string_printf(line, "%f", x);

  for (list=graph->set_list ; list ; list=g_slist_next(list))
    {
    y = (gdouble *) list->data;
    g_string_append_printf(line, " %f", y[i]);
    }
  fprintf(fp, "%s\n", line->str);
  }

g_string_free(line, TRUE);

fclose(fp);
}

/*****************************************/
/* raw column data import (for plotting) */
/*****************************************/
extern gint reaxmd_parse_properties(FILE *, struct model_pak *, gpointer );
gint graph_import(gpointer import)
{
FILE *fp;

fp = fopen(import_fullpath(import), "rt");
if (fp)
  reaxmd_parse_properties(fp, NULL, import);

return(0);
}

