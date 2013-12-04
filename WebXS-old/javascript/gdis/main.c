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

/* irix */
#define _BSD_SIGNALS 1

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#ifndef __WIN32
#include <sys/wait.h>
#endif

#include "gdis.h"
#include "command.h"
#include "file.h"
#include "parse.h"
#include "task.h"
#include "host.h"
#include "grid.h"
#include "render.h"
#include "matrix.h"
#include "model.h"
#include "project.h"
#include "numeric.h"
#include "module.h"
#include "library.h"
#include "interface.h"
#include "gui_image.h"

#ifdef WITH_GRISU
//#include "jni_grisu.h"
#endif

/* main data structures */
struct sysenv_pak sysenv;
struct elem_pak elements[MAX_ELEMENTS];

/********************************/
/* sfc hash table value cleanup */
/********************************/
void free_sfc(gpointer sfc_list)
{
free_slist((GSList *) sfc_list);
}

/******************/
/* system cleanup */
/******************/
void sys_free(void)
{
/* NEW - cleanup */
g_free(sysenv.render.animate_file);
g_free(sysenv.render.morph_finish);

g_free(sysenv.babel_exe);
g_free(sysenv.povray_exe);
g_free(sysenv.convert_exe);
g_free(sysenv.viewer_exe);
g_free(sysenv.gulp_exe);
g_free(sysenv.gamess_exe);
g_free(sysenv.siesta_exe);
g_free(sysenv.monty_exe);
g_free(sysenv.ffmpeg_exe);
g_free(sysenv.reaxmd_exe);

g_free(sysenv.babel_path);
g_free(sysenv.povray_path);
g_free(sysenv.convert_path);
g_free(sysenv.viewer_path);
g_free(sysenv.gulp_path);
g_free(sysenv.gamess_path);
g_free(sysenv.siesta_path);
g_free(sysenv.monty_path);
g_free(sysenv.ffmpeg_path);
g_free(sysenv.reaxmd_path);

g_free(sysenv.init_file);
g_free(sysenv.elem_file);
g_free(sysenv.gdis_path);
g_free(sysenv.cwd);

g_free(sysenv.default_idp);
g_free(sysenv.default_user);
g_free(sysenv.default_vo);
g_free(sysenv.default_ws);

free_slist(sysenv.render.light_list);

if (sysenv.sfc_table)
  g_hash_table_destroy(sysenv.sfc_table);

if (sysenv.library)
  g_hash_table_destroy(sysenv.library);

/*
if (sysenv.manual)
  g_hash_table_destroy(sysenv.manual);
*/
g_static_mutex_free(&sysenv.grid_auth_mutex);

task_queue_free();

image_table_free();

file_free();

project_browse_free();

grid_free();

#ifdef WITH_GRISU
//jni_free();
#endif

#ifndef _WIN32
host_free_all();
#endif
}

/**********************************/
/* read the init file if possible */
/**********************************/
gint read_gdisrc(void)
{
gint i, element_flag, num_tokens;
gchar *line, **buff;
gdouble version;
GSList *versions;
FILE *fp;

/* attempt to open */
fp = fopen(sysenv.init_file, "r");

/* check for an old/bad gdisrc */
if (fp)
  {
/* scan the 1st line */
  buff = get_tokenized_line(fp, &num_tokens);
  if (g_ascii_strncasecmp(*buff,"gdis",4) == 0)
    {
/* test for right version */
    if (num_tokens < 2)
      return(1);
    version = str_to_float(*(buff+1));
    sscanf(*(buff+1),"%lf",&version);
/* 0.75 was when the new element scheme was implemented */
    if (version < 0.75)
      return(1);
    }
  else
    return(1);
  g_strfreev(buff);
  }
else
  return(1);

/* read until EOF, or (failsafe) reached elements[] array allocation */
element_flag=0;
for (;;)
  {
/* NB: we need line in some cases, so can't auto tokenize everything */
  line = file_read_line(fp);
  if (!line)
     break;

  buff = tokenize(line, &num_tokens);
  if (!buff)
    break;

/* decide what to read */
  if (g_ascii_strncasecmp("size",*buff,4) == 0)
    {
    if (num_tokens > 2)
      {
      sysenv.width = str_to_float(*(buff+1));
      sysenv.height = str_to_float(*(buff+2));
      if (sysenv.width < MIN_WIDTH)
        sysenv.width = MIN_WIDTH;
      if (sysenv.height < MIN_HEIGHT)
        sysenv.height = MIN_HEIGHT;
      }
    } 

/* model pane width */
  if (g_ascii_strncasecmp("pane",*buff,4) == 0)
    {
    if (num_tokens > 2)
      {
      sysenv.tree_width = str_to_float(*(buff+1));
      sysenv.tray_height = str_to_float(*(buff+2));
      }
    }

/* model tree/property divider */
  if (g_ascii_strncasecmp("divide",*buff,6) == 0)
    {
    if (num_tokens > 1)
      sysenv.tree_divider = (gint) str_to_float(*(buff+1));
    }

/* povray executable */
  if (g_ascii_strncasecmp("povray_p", *buff, 8) == 0)
    sysenv.povray_path = parse_strip_newline(line+12);

  if (g_ascii_strncasecmp("povray_e", *buff, 8) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(sysenv.povray_exe);
      sysenv.povray_exe = g_strdup(*(buff+1));
      }
    }

/* animation creation tool */
  if (g_ascii_strncasecmp("ffmpeg_p", *buff, 9) == 0)
    sysenv.ffmpeg_path = parse_strip_newline(line+13);

  if (g_ascii_strncasecmp("ffmpeg_e", *buff, 9) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(sysenv.ffmpeg_exe);
      sysenv.ffmpeg_exe = g_strdup(*(buff+1));
      }
    }

/* animation creation tool */
  if (g_ascii_strncasecmp("convert_p", *buff, 9) == 0)
    sysenv.convert_path = parse_strip_newline(line+13);

  if (g_ascii_strncasecmp("convert_e", *buff, 9) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(sysenv.convert_exe);
      sysenv.convert_exe = g_strdup(*(buff+1));
      }
    }

/* image viewing */
  if (g_ascii_strncasecmp("viewer_p", *buff, 8) == 0)
    sysenv.viewer_path = parse_strip_newline(line+12);

  if (g_ascii_strncasecmp("viewer_e", *buff, 8) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(sysenv.viewer_exe);
      sysenv.viewer_exe = g_strdup(*(buff+1));
      }
    }

/* GAMESS */
  if (g_ascii_strncasecmp("gamess_p", *buff, 8) == 0)
    sysenv.gamess_path = parse_strip_newline(line+12);

  if (g_ascii_strncasecmp("gamess_e", *buff, 8) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(sysenv.gamess_exe);
      sysenv.gamess_exe = g_strdup(*(buff+1));
      }
    }

/* GULP */
  if (g_ascii_strncasecmp("gulp_p", *buff, 6) == 0)
    sysenv.gulp_path = parse_strip_newline(&line[10]);

  if (g_ascii_strncasecmp("gulp_e", *buff, 6) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(sysenv.gulp_exe);
      sysenv.gulp_exe = g_strdup(*(buff+1));
      }
    }

/* SIESTA */
  if (g_ascii_strncasecmp("siesta_p", *buff, 8) == 0)
    sysenv.siesta_path = parse_strip_newline(&line[12]);

  if (g_ascii_strncasecmp("siesta_e", *buff, 8) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(sysenv.siesta_exe);
      sysenv.siesta_exe = g_strdup(*(buff+1));
      }
    }

/* SIESTA */
  if (g_ascii_strncasecmp("reaxmd_p", *buff, 8) == 0)
    sysenv.reaxmd_path = parse_strip_newline(&line[12]);

  if (g_ascii_strncasecmp("reaxmd_e", *buff, 8) == 0)
    {
    if (num_tokens > 1)
      {
      g_free(sysenv.reaxmd_exe);
      sysenv.reaxmd_exe = g_strdup(*(buff+1));
      }
    }

/* OpenGL drawing font */
  if (g_ascii_strncasecmp("gl_font",*buff,7) == 0)
    if (num_tokens > 1)
      strcpy(sysenv.gl_fontname, g_strstrip(&line[8]));

/* model tree box */
  if (g_ascii_strncasecmp("mtb",*buff,3) == 0)
    if (num_tokens > 1)
      sysenv.mtb_on = (gint) str_to_float(*(buff+1));

/* model properties box */
  if (g_ascii_strncasecmp("mpb",*buff,3) == 0)
    if (num_tokens > 1)
      sysenv.mpb_on = (gint) str_to_float(*(buff+1));

/* model symmetry box */
  if (g_ascii_strncasecmp("msb",*buff,3) == 0)
    if (num_tokens > 1)
      sysenv.msb_on = (gint) str_to_float(*(buff+1));

/* atom properties box */
  if (g_ascii_strncasecmp("apb",*buff,3) == 0)
    if (num_tokens > 1)
      sysenv.apb_on = (gint) str_to_float(*(buff+1));

/* halo type */
  if (g_ascii_strncasecmp("halo",*buff,4) == 0)
    if (num_tokens > 1)
      sysenv.render.halos = (gint) str_to_float(*(buff+1));

/* low quality rotation */
  if (g_ascii_strncasecmp("fast",*buff,4) == 0)
    if (num_tokens > 1)
      sysenv.render.fast_rotation = (gint) str_to_float(*(buff+1));

  if (g_ascii_strncasecmp(*buff, "colour_bg", 9) == 0)
    {
    if (num_tokens > 3)
      {
      sysenv.render.bg_colour[0] = str_to_float(*(buff+1));
      sysenv.render.bg_colour[1] = str_to_float(*(buff+2));
      sysenv.render.bg_colour[2] = str_to_float(*(buff+3));
      }
    }
  if (g_ascii_strncasecmp(*buff, "colour_morph", 11) == 0)
    {
    if (num_tokens > 3)
      {
      sysenv.render.morph_colour[0] = str_to_float(*(buff+1));
      sysenv.render.morph_colour[1] = str_to_float(*(buff+2));
      sysenv.render.morph_colour[2] = str_to_float(*(buff+3));
      }
    }
  if (g_ascii_strncasecmp("colour_rsurf", *buff, 12) == 0)
    {
    if (num_tokens > 3)
      {
      sysenv.render.rsurf_colour[0] = str_to_float(*(buff+1));
      sysenv.render.rsurf_colour[1] = str_to_float(*(buff+2));
      sysenv.render.rsurf_colour[2] = str_to_float(*(buff+3));
      }
    }

/* parse cached grid id info */
  if (g_ascii_strncasecmp("default_idp", *buff, 11) == 0)
    {
    if (num_tokens > 1)
      sysenv.default_idp = g_strdup(*(buff+1));
    }
  if (g_ascii_strncasecmp("default_user", *buff, 12) == 0)
    {
    if (num_tokens > 1)
      sysenv.default_user = g_strdup(*(buff+1));
    }
  if (g_ascii_strncasecmp("default_vo", *buff, 10) == 0)
    {
    if (num_tokens > 1)
      sysenv.default_vo = g_strdup(*(buff+1));
    }
  if (g_ascii_strncasecmp("default_ws", *buff, 10) == 0)
    {
    if (num_tokens > 1)
      sysenv.default_ws = g_strdup(*(buff+1));
    }

/* parse cached grid applications */
  if (g_strrstr(line, "block grid_app"))
    {
    g_free(line);
    line = file_read_line(fp);
    while (line)
      {
      if (g_strrstr(line, "endblock"))
        break;

/* comma separated - as some things (versions) have spaces in them */
      g_strfreev(buff);
      buff = g_strsplit(line,",",-1);
      num_tokens = g_strv_length(buff);

      versions = NULL;
      if (num_tokens > 2)
        {
        for (i=2 ; i<num_tokens ; i++)
          versions = g_slist_append(versions, g_strstrip(*(buff+i)));
//printf("+[%s]", *(buff+2));
        }
      if (num_tokens > 1)
        {
        grid_table_append(*buff, *(buff+1), versions);
//printf("adding: %s %s\n", *buff, *(buff+1));
        }
      g_slist_free(versions);

      g_free(line);
      line = file_read_line(fp);
      }
    }

/* cleanup */
  g_strfreev(buff);
  g_free(line);
  }

/* parse for elements */
rewind(fp);
read_elem_data(fp, MODIFIED);

return(0);
}

/*********************************************/
/* write setup & elements to the gdisrc file */
/*********************************************/
gint write_gdisrc(void)
{
const gchar *name, *queue;
GSList *item, *list, *item2, *versions;
FILE *fp;


fp = fopen(sysenv.init_file,"w");
if (!fp)
  {
  printf("Error: unable to create %s\n", sysenv.init_file);
  return(1);
  }

/* save final dimensions */
if (sysenv.mpane)
  if (GTK_IS_WIDGET(sysenv.mpane))
    sysenv.tree_width = GTK_WIDGET(sysenv.mpane)->allocation.width;
if (sysenv.tpane)
  if (GTK_IS_WIDGET(sysenv.tpane))
    sysenv.tray_height = GTK_WIDGET(sysenv.tpane)->allocation.height;

fprintf(fp,"gdis %f\n", VERSION);
fprintf(fp,"canvas %d\n", sysenv.canvas);
fprintf(fp,"size %d %d\n", sysenv.width,sysenv.height);
fprintf(fp,"pane %d %d\n", sysenv.tree_width, sysenv.tray_height);
fprintf(fp,"divider %d\n", sysenv.tree_divider);
fprintf(fp,"gl_font %s\n", sysenv.gl_fontname);
fprintf(fp,"mtb %d\n", sysenv.mtb_on);
fprintf(fp,"mpb %d\n", sysenv.mpb_on);
fprintf(fp,"msb %d\n", sysenv.msb_on);
fprintf(fp,"apb %d\n", sysenv.apb_on);
fprintf(fp,"halos %d\n", sysenv.render.halos);
fprintf(fp,"fast %d\n", sysenv.render.fast_rotation);

fprintf(fp,"colour_bg %f %f %f\n", sysenv.render.bg_colour[0],
                                   sysenv.render.bg_colour[1],
                                   sysenv.render.bg_colour[2]);
fprintf(fp,"colour_morph %f %f %f\n", sysenv.render.morph_colour[0],
                                      sysenv.render.morph_colour[1],
                                      sysenv.render.morph_colour[2]);
fprintf(fp,"colour_rsurf %f %f %f\n", sysenv.render.rsurf_colour[0],
                                      sysenv.render.rsurf_colour[1],
                                      sysenv.render.rsurf_colour[2]);

if (sysenv.babel_path)
  fprintf(fp,"babel_path %s\n", sysenv.babel_path);
if (sysenv.convert_path)
  fprintf(fp,"convert_path %s\n", sysenv.convert_path);
if (sysenv.ffmpeg_path)
  fprintf(fp,"ffmpeg_path %s\n", sysenv.ffmpeg_path);
if (sysenv.gulp_path)
  fprintf(fp,"gulp_path %s\n", sysenv.gulp_path);
if (sysenv.gamess_path)
  fprintf(fp,"gamess_path %s\n", sysenv.gamess_path);
if (sysenv.reaxmd_path)
  fprintf(fp,"reaxmd_path %s\n", sysenv.reaxmd_path);
if (sysenv.siesta_path)
  fprintf(fp,"siesta_path %s\n", sysenv.siesta_path);
if (sysenv.povray_path)
  fprintf(fp,"povray_path %s\n", sysenv.povray_path);
if (sysenv.viewer_path)
  fprintf(fp,"viewer_path %s\n", sysenv.viewer_path);

fprintf(fp,"babel_exe %s\n", sysenv.babel_exe);
fprintf(fp,"convert_exe %s\n", sysenv.convert_exe);
fprintf(fp,"ffmpeg_exe %s\n", sysenv.ffmpeg_exe);
fprintf(fp,"gulp_exe %s\n", sysenv.gulp_exe);
fprintf(fp,"gamess_exe %s\n", sysenv.gamess_exe);
fprintf(fp,"siesta_exe %s\n", sysenv.siesta_exe);
fprintf(fp,"povray_exe %s\n", sysenv.povray_exe);
fprintf(fp,"viewer_exe %s\n", sysenv.viewer_exe);

/* GRID */
if (sysenv.default_idp)
  fprintf(fp,"default_idp %s\n", sysenv.default_idp);
if (sysenv.default_user)
  fprintf(fp,"default_user %s\n", sysenv.default_user);
if (sysenv.default_vo)
  fprintf(fp,"default_vo %s\n", sysenv.default_vo);
if (sysenv.default_ws)
  fprintf(fp,"default_ws %s\n", sysenv.default_ws);

/* save the registered grid applications/queues */
/* NB: some versions have spaces, so let's try comma separated values */
list = grid_table_list_get();
if (list)
  {
  fprintf(fp, "%%block grid_application\n");
  for (item=list ; item ; item=g_slist_next(item))
    {
    name = grid_table_application_get(item->data);
    queue = grid_table_queue_get(item->data);
    versions = grid_table_versions_get(item->data);

    fprintf(fp, "%s,%s", name, queue);
/* NB: some versions have spaces, so let's try comma separated values */
    for (item2=versions ; item2 ; item2=g_slist_next(item2))
      fprintf(fp, ",%s", (gchar *) item2->data);
    fprintf(fp, "\n");
    }
  fprintf(fp, "%%endblock\n");
  }

/* write the non-default element data */
write_elem_data(fp);

fclose(fp);
return(0);
}

/**************/
/* main setup */
/**************/
#define DEBUG_SYS_INIT 0
void sys_init(gint argc, gchar *argv[])
{
gchar *temp, *logfile;
const gchar *ctemp;
struct light_pak *light;
FILE *fp;

/* top level structure initialization */
sysenv.max_threads = 1;
sysenv.task_list = NULL;
sysenv.host_list = NULL;
sysenv.dialog_list = NULL;
sysenv.glconfig = NULL;
sysenv.stereo = FALSE;
sysenv.stereo_windowed = FALSE;
sysenv.stereo_fullscreen = FALSE;
sysenv.canvas_list = NULL;
sysenv.canvas_rows = 1;
sysenv.canvas_cols = 1;
sysenv.width = START_WIDTH;
sysenv.height = START_HEIGHT;
sysenv.snapshot = FALSE;
sysenv.snapshot_filename = NULL;
sysenv.tree_width = START_WIDTH/4;
sysenv.tree_divider = -1;
sysenv.tray_height = 60;
sysenv.mpane = NULL;
sysenv.tpane = NULL;
sysenv.fps = 40;
sysenv.moving = FALSE;
sysenv.roving = FALSE;
sysenv.refresh_canvas = 0;
sysenv.refresh_dialog = FALSE;
sysenv.refresh_tree = FALSE;
sysenv.refresh_properties = FALSE;
sysenv.refresh_text = FALSE;
sysenv.mtb_on = TRUE;
sysenv.mpb_on = TRUE;
sysenv.msb_on = TRUE;
sysenv.apb_on = TRUE;
sysenv.lmb_on = FALSE;
sysenv.pib_on = FALSE;
/* default to single model display */
sysenv.mal = NULL;
sysenv.displayed[0] = -1;
sysenv.canvas = TRUE;
sysenv.select_mode = CORE;
sysenv.calc_pbonds = TRUE;

/* default masks */
sysenv.file_type = DATA;
sysenv.babel_type = AUTO;
sysenv.num_elements = sizeof(elements) / sizeof(struct elem_pak);
sysenv.elements = NULL;

/* rendering setup */
sysenv.render.width = 600;
sysenv.render.height = 600;
sysenv.render.vp_dist = 5.0;
sysenv.render.show_energy = FALSE;

/* stereo defaults */
sysenv.render.stereo_quadbuffer = FALSE;
sysenv.render.stereo_use_frustum = TRUE;
sysenv.render.stereo_eye_offset = 1.0;
sysenv.render.stereo_parallax = 1.0;
sysenv.render.stereo_left = TRUE;
sysenv.render.stereo_right = TRUE;

/* default light */
light = g_malloc(sizeof(struct light_pak));
light->type = DIRECTIONAL;
VEC3SET(light->x, -1.0, 1.0, -1.0);
VEC3SET(light->colour, 1.0, 1.0, 1.0);
light->ambient = 0.2;
light->specular = 0.7;
light->diffuse = 0.5;
sysenv.render.light_list = NULL;
sysenv.render.light_list = g_slist_append(sysenv.render.light_list, light);
sysenv.render.num_lights = 1;

sysenv.render.connect_redo = FALSE;
sysenv.render.perspective = FALSE;
sysenv.render.antialias = TRUE;
sysenv.render.fog = FALSE;
sysenv.render.wire_model = FALSE;
sysenv.render.wire_surface = FALSE;
sysenv.render.shadowless = FALSE;
sysenv.render.animate = FALSE;
sysenv.render.no_povray_exec = FALSE;
sysenv.render.no_keep_tempfiles = TRUE;
sysenv.render.animate_type = ANIM_GIF;
sysenv.render.animate_file = g_strdup("movie.mpg");
sysenv.render.atype = FALSE;
sysenv.render.axes = FALSE;
sysenv.render.morph_finish = g_strdup("F_Glass4");
sysenv.render.ref_index = 2.5;
sysenv.render.delay = 20.0;
sysenv.render.mpeg_quality = 95.0;
sysenv.render.sphere_quality = 3.0;
sysenv.render.cylinder_quality = 9.0;
sysenv.render.ribbon_quality = 10.0;
sysenv.render.ms_grid_size = 0.5;
sysenv.render.auto_quality = FALSE;
sysenv.render.fast_rotation = TRUE;
sysenv.render.halos = FALSE;
sysenv.render.ambience = 0.2;
sysenv.render.diffuse = 0.9;
sysenv.render.specular = 0.9;
sysenv.render.transmit = 0.7;
sysenv.render.ghost_opacity = 0.5;
sysenv.render.ball_rad = 0.3;
sysenv.render.stick_rad = 0.1;
sysenv.render.stick_thickness = GTKGL_LINE_WIDTH;
sysenv.render.line_thickness = GTKGL_LINE_WIDTH;
sysenv.render.frame_thickness = GTKGL_LINE_WIDTH;
sysenv.render.geom_line_width = 3.0;
sysenv.render.cpk_scale = 0.35;  // 1.0 is too large, usually
sysenv.render.fog_density = 0.35;
sysenv.render.fog_start = 0.5;
sysenv.render.ribbon_curvature = 0.5;
sysenv.render.ribbon_thickness = 1.0;
sysenv.render.phonon_scaling = 2.0;
sysenv.render.phonon_resolution = 10.0;
sysenv.render.ahl_strength = 0.7;
sysenv.render.ahl_size = 20;
sysenv.render.shl_strength = 0.8;
sysenv.render.shl_size = 20;
VEC3SET(sysenv.render.fg_colour, 1.0, 1.0, 1.0);
VEC3SET(sysenv.render.bg_colour, 0.0, 0.0, 0.0);
VEC3SET(sysenv.render.morph_colour, 0.1, 0.1, 0.8);
VEC3SET(sysenv.render.rsurf_colour, 0.0, 0.3, 0.6);
VEC3SET(sysenv.render.label_colour, 1.0, 1.0, 0.0);
VEC3SET(sysenv.render.title_colour, 0.0, 1.0, 1.0);
VEC3SET(sysenv.render.ribbon_colour, 0.0, 0.0, 1.0);
VEC3SET(sysenv.render.ms_colour, 0.0, 0.0, 1.0);
sysenv.default_idp = NULL;
sysenv.default_user = NULL;
sysenv.default_vo = NULL;
sysenv.default_ws = NULL;

/* setup directory and file pointers */
{
const gchar *envdir = g_getenv("GDIS_START_DIR");
if (envdir)
  sysenv.cwd = g_strdup(envdir);
else
  sysenv.cwd = g_get_current_dir();
}

#if DEBUG_SYS_INIT
printf("cwd: [%s]\n", sysenv.cwd);
#endif

/* generate element file full pathname */
/* sometimes this returns the program name, and sometimes it doesn't */
temp = g_find_program_in_path(argv[0]);
/* remove program name (if attached) */
if (g_file_test(temp, G_FILE_TEST_IS_DIR))
  sysenv.gdis_path = temp;
else
  {
  sysenv.gdis_path = g_path_get_dirname(temp);
  g_free(temp);
  }

#if DEBUG_SYS_INIT
printf("%s path: [%s]\n", argv[0], sysenv.gdis_path);
#endif

if (sysenv.gdis_path)
  sysenv.elem_file = g_build_filename(sysenv.gdis_path, ELEM_FILE, NULL);
else
  {
  printf("WARNING: gdis directory not found.\n");
  sysenv.elem_file = g_build_filename(sysenv.cwd, ELEM_FILE, NULL);
  }

/* generate gdisrc full pathname */
sysenv.init_file=NULL;

ctemp = g_get_home_dir();
if (ctemp)
  sysenv.init_file = g_build_filename(ctemp, ".gdis", INIT_FILE, NULL);
else
  {
  printf("WARNING: home directory not found.\n");
  if (sysenv.gdis_path)
    sysenv.init_file = g_build_filename(sysenv.gdis_path, INIT_FILE, NULL);
  else
    {
    if (sysenv.cwd)
      sysenv.init_file = g_build_filename(sysenv.cwd, INIT_FILE, NULL);
    else
      {
      printf("FATAL: current directory not found.\n");
      exit(-1);
      }
    }
  }

/* NEW - if .gdis/ dir doesn't exist - create */
temp = g_path_get_dirname(sysenv.init_file);
file_mkdir(temp);

/* NEW - if downloads path doesn't exist - create */
sysenv.download_path = g_build_filename(temp, "downloads", NULL);
file_mkdir(sysenv.download_path);

//printf("Downloads: %s\n", sysenv.download_path);

/* redirect stderr */
logfile = g_build_filename(temp, "gdis.log", NULL);
if (freopen(logfile, "a", stderr))
  fprintf(stderr, "Starting GDIS...\n");
else
  printf("Warning: failed to redirect STDERR to log file.\n");

g_free(logfile);

g_free(temp);

/* defaults */
sysenv.babel_exe = g_strdup("babel");
sysenv.povray_exe = g_strdup("povray");
sysenv.ffmpeg_exe = g_strdup("ffmpeg");
sysenv.convert_exe = g_strdup("convert");
sysenv.viewer_exe = g_strdup("display");
sysenv.gulp_exe = g_strdup("gulp");
sysenv.gamess_exe = g_strdup("gamess-us");
sysenv.monty_exe = g_strdup("monty");
sysenv.reaxmd_exe = g_strdup("reaxmd");
sysenv.siesta_exe = g_strdup("siesta");

strcpy(sysenv.gl_fontname, GL_FONT);

/* atomic scattering factor coefficients */
sysenv.sfc_table = g_hash_table_new_full(g_str_hash, hash_strcmp, g_free, free_sfc);

/* IMPORTANT this must be done before gdisrc is parsed as */
/* setup the element data before any possible modifications */
/* can be read in from the gdisrc file */
printf("scanning: %-50s ", sysenv.elem_file);
fp = fopen(sysenv.elem_file, "rt");
if (fp)
  {
  read_elem_data(fp, DEFAULT);
  fclose(fp);
  printf("[ok]\n");
  }
else
  {
/* missing default element info is fatal */
  printf("[not found]\n");
  exit(1);
  }

/* if failed to read gdisrc - write a new one */
/* TODO - if overwrite old version, save/rename first? */
sysenv.babel_path = NULL;
sysenv.convert_path = NULL;
sysenv.gamess_path = NULL;
sysenv.gulp_path = NULL;
sysenv.monty_path = NULL;
sysenv.reaxmd_path = NULL;
sysenv.siesta_path = NULL;
sysenv.povray_path = NULL;
sysenv.viewer_path = NULL;

if (read_gdisrc())
  {
  printf("creating: %-50s ", sysenv.init_file);
  if (write_gdisrc())
    printf("[error]\n");
  else
    printf("[ok]\n");
  }
else
  fprintf(stderr, "scanning: %-50s [ok]\n", sysenv.init_file);

file_path_find();

/* tree init */
sysenv.num_trees = 0;
sysenv.select_source = NULL;
sysenv.module_list = NULL;
sysenv.projects = NULL;
sysenv.manual = NULL;
sysenv.surfaces = NULL;
sysenv.workspace_changed = FALSE;
sysenv.workspace = project_new("default", sysenv.cwd);

// NEW
g_static_mutex_init(&sysenv.grid_auth_mutex);

file_init();
#ifdef WITH_GUI
help_init();
#endif
init_math();
library_init();
grid_init();
}

/********/
/* MAIN */
/********/
gint main(int argc, char *argv[])
{
/* NBL read this 1st as it affects canvas type, but command */
/* line arguments should be able to overide */
sys_init(argc, argv);
//module_setup();
sysenv.write_gdisrc = FALSE;

/* init thread pool and lock for GUI creation */
task_queue_init();
gdk_threads_enter();

/* command line? */
if (argc > 1)
  {
  if (g_ascii_strncasecmp(argv[1],"-c",2) == 0)
    sysenv.canvas = FALSE;
  }

#ifdef WITH_GRISU
//jni_init();
#endif

#ifdef WITH_GUI
/* set up main window and event handlers (or start non-gui mode) */
if (sysenv.canvas)
  {
  gint i;

/* main interface */
  gui_init(argc, argv);

/* CURRENT - gui updates done through a timer */
/* this was done so that threads can safely schedule a gui update */

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
  gdk_threads_add_timeout(333, (GSourceFunc) &gui_widget_handler, NULL);
  gdk_threads_add_timeout(33, (GSourceFunc) &gui_canvas_handler, NULL);
#else
  g_timeout_add(333, (GSourceFunc) &gui_widget_handler, NULL);
  g_timeout_add(33, (GSourceFunc) &gui_canvas_handler, NULL);
#endif

  if (argc > 1)
    {
/* import all files */
    for (i=1 ; i<argc ; i++)
      gui_import_new(argv[i]);
    }

  gtk_main();
  gdk_threads_leave();
  }
else
#else
sysenv.canvas = FALSE;
#endif
  command_main_loop(argc, argv);

return(0);
}

/* CURRENT */
/* routines that are not cleanly seperable when we build with no GUI */
#ifndef WITH_GUI
gpointer tree_project_get(void)
{
return(NULL);
}
gpointer tree_model_get(void)
{
return(NULL);
}
gpointer gui_tree_parent_object(gpointer child)
{
return(NULL);
}
gint gui_tree_append(gpointer parent_data, gint child_id, gpointer child_data)
{
return(0);
}
void gui_tree_import_refresh(gpointer import)
{
}


void gui_text_show(gint type, gchar *msg)
{
printf("%s", msg);
}

void gui_refresh(gint dummy)
{
}

void tree_select_delete(void)
{
}
void tree_select_active(void)
{
}
void tree_select_model(struct model_pak *m)
{
}
void tree_model_add(struct model_pak *m)
{
}
void tree_model_refresh(struct model_pak *m)
{
}

void dialog_destroy_type(gint dummy)
{
}

void meas_graft_model(struct model_pak *m)
{
}

void meas_prune_model(struct model_pak *m)
{
}

void image_table_free(void)
{
}

/* FIXME - this should actually be replaced by gui_refresh() */
void redraw_canvas(gint dummy)
{
}
#endif
