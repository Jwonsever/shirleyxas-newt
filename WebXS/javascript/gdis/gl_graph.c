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
#include "opengl.h"
#include "interface.h"
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/* externals */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];
extern gint gl_fontsize;

/****************/
/* draw a graph */
/****************/
#define DEBUG_GRAPH_1D 0
void graph_draw_1d(struct canvas_pak *canvas, struct graph_pak *graph)
{
gint i, n, x, y, oldx, oldy, ox, oy, sx, sy;
gint flag, yname_offset;
gchar *text, **buff;
gdouble *ptr;
gdouble xf, yf, dx, dy;
gdouble xmin, xmax, ysum, ysumsq, ysize;
GSList *list;

#if DEBUG_GRAPH_1D
printf("x range (entire bin interval): [%f - %f]\n", graph->xmin, graph->xmax);
printf("y range (entire bin interval): [%f - %f]\n", graph->ymin, graph->ymax);
printf("actual plotting interval bins: [%d - %d]\n", graph->start, graph->stop);
#endif

/* compute origin */
ox = canvas->x + 4*gl_fontsize;
if (graph->ylabel)
  ox += 4*gl_fontsize;

oy = canvas->y + canvas->height - 2*gl_fontsize;
if (graph->xlabel)
  oy -= 2*gl_fontsize;

/* increments for screen drawing */
dy = (canvas->height-8*gl_fontsize);
dx = (canvas->width-2*ox);

/* axes label colour */
glColor3f(sysenv.render.fg_colour[0],
          sysenv.render.fg_colour[1],
          sysenv.render.fg_colour[2]);
glLineWidth(2.0);

/* NEW - compute desired graph limits */
xmin = graph_x_value(graph->start, graph);
xmax = graph_x_value(graph->stop, graph);

/* NEW - x label */
if (graph->xname)
  gl_print_window(graph->xname, ox+0.5*dx, oy+3*gl_fontsize, canvas);

/* x tick marks */
oldx = ox;
for (i=0 ; i<graph->xticks ; i++)
  {
/* get real index */
  xf = (gdouble) i / (gdouble) (graph->xticks-1);

  x = ox + xf*dx;
  xf *= (xmax-xmin);
  xf += xmin;

  if (graph->xlabel)
    {
    text = g_strdup_printf("%.2f", xf);
    gl_print_window(text, x-2*gl_fontsize, oy+2*gl_fontsize, canvas);
    g_free(text);
    }

/* axis segment + tick */
  glBegin(GL_LINE_STRIP);
  gl_vertex_window(oldx, oy, canvas);
  gl_vertex_window(x, oy, canvas);
  gl_vertex_window(x, oy+5, canvas);
  glEnd();

  oldx = x;
  }

/* NEW - y label */
if (graph->yname)
  {
  buff = g_strsplit(graph->yname, "|", -1);

  if (buff)
    {
    n=0;
    yname_offset=0;
    while (buff[n])
      {
      glColor3f(elements[n+3].colour[0], elements[n+3].colour[1], elements[n+3].colour[2]);

// FIXME - only works properly for fixed width fonts
      gl_print_window(buff[n], ox+yname_offset*gl_fontsize, oy - dy - gl_fontsize, canvas);
      yname_offset += strlen(buff[n]);
      n++;
      }
    g_strfreev(buff);
    }
  }

glColor3f(sysenv.render.fg_colour[0],
          sysenv.render.fg_colour[1],
          sysenv.render.fg_colour[2]);

/* y tick marks */
oldy = oy;
for (i=0 ; i<graph->yticks ; i++)
  {
/* get screen position */
  yf = (gdouble) i / (gdouble) (graph->yticks-1);
  y = -yf*dy;
  y += oy;

/* get real value */
  yf *= (graph->ymax - graph->ymin);
  yf += graph->ymin;

/* label */
  if (graph->ylabel)
    {
    if (graph->ymax > 999.999999)
      text = g_strdup_printf("%.2e", yf);
    else
      text = g_strdup_printf("%7.2f", yf);
    gl_print_window(text, 0, y-1, canvas);
    g_free(text);
    }

/* axis segment + tick */
  glBegin(GL_LINE_STRIP);
  gl_vertex_window(ox, oldy, canvas);
  gl_vertex_window(ox, y-1, canvas);
  gl_vertex_window(ox-5, y-1, canvas);
  glEnd();

  oldy = y;
  }

/* TODO - different colour for each item in the set list */
/* data drawing colour */
glColor3f(sysenv.render.title_colour[0],
          sysenv.render.title_colour[1],
          sysenv.render.title_colour[2]);
glLineWidth(1.0);

flag = FALSE;
sx = sy = 0;

/* graph the data sets */
n=0;
for (list=graph->set_list ; list ; list=g_slist_next(list))
  {
  ptr = (gdouble *) list->data;

/* NEW - stats */
  ysum = 0.0;
  ysumsq = 0.0;
  ysize = graph->stop - graph->start;

/* NEW - different colour for each set */
/* TODO - proper colour order, rather than fake it with 2nd row elem data */
glColor3f(elements[n+3].colour[0],
          elements[n+3].colour[1],
          elements[n+3].colour[2]);

/* draw */
  glBegin(GL_LINE_STRIP);
  for (i=graph->start ; i<graph->stop ; i++)
    {
/* compute fractional offset */
    xf = (gdouble) (i-graph->start) / (gdouble) (graph->stop - graph->start - 1);
    x = ox + xf*dx;

/* get real value */
    yf = ptr[i];

/* compute statistics */
    ysum += yf;
    ysumsq += yf*yf;

/* compute plotting coordinate */
    yf -= graph->ymin;
    yf /= (graph->ymax - graph->ymin);
    yf *= dy;

    y = (gint) yf;
    y *= -1;
    y += oy;

if (i == graph->select)
  {
  sx = x;
  sy = y-1;
  flag = TRUE;
  }

/* lift y axis 1 pixel up so y=0 won't overwrite the x axis */
    gl_vertex_window(x, y-1, canvas);
    }
  glEnd();

/* FIXME - this assumes only 1 data set */
  graph->ymean = ysum / ysize;

/* TODO - see towards the bottom of this link (rapid calculation methods) for */
/* minimizing numerical errors: http://en.wikipedia.org/wiki/Standard_deviation */
/* particularly when numbers in this calc become very large */
graph->ysd = ysumsq/ysize - graph->ymean*graph->ymean;
graph->ysd = sqrt(graph->ysd);

#if DEBUG_GRAPH_1D
printf("y mean = %f\n", graph->ymean);
printf("y sd = %f\n", graph->ysd);
#endif

/* draw peak selector (if any) */
/* TODO - turn off is click outside range? */
  if (flag)
    {
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x0303);
    glColor3f(0.9, 0.7, 0.4);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    gl_vertex_window(sx, sy-10, canvas);
    gl_vertex_window(sx, 3*gl_fontsize, canvas);
    glEnd();

    if (graph->select_label)
      {
      gint xoff;

      xoff = strlen(graph->select_label);
      xoff *= gl_fontsize;
      xoff /= 4;
      gl_print_window(graph->select_label, sx-xoff, 2*gl_fontsize, canvas);
      }
    glDisable(GL_LINE_STIPPLE);
    }
  n++;
  }
}

/*********************************/
/* init for OpenGL graph drawing */
/*********************************/
void graph_draw(struct canvas_pak *canvas, gpointer data)
{
struct graph_pak *graph = data;

/* checks */
g_assert(canvas != NULL);
g_assert(graph != NULL);

/* init drawing model */
glDisable(GL_LIGHTING);
glDisable(GL_LINE_STIPPLE);
glDisable(GL_DEPTH_TEST);

glEnable(GL_BLEND);
glEnable(GL_LINE_SMOOTH);
glEnable(GL_POINT_SMOOTH);

glDisable(GL_COLOR_LOGIC_OP);

/* draw the appropriate type */
graph_draw_1d(canvas, graph);
}

