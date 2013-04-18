/*

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

/*
 * If you have any questions or comments about DL_POLY file support in GDIS,
 * send it to GDIS mailing list
 * http://lists.sourceforge.net/lists/listinfo/gdis-users
 * (you can also CC to Marcin Wojdyr, search the web for e-mail)
 * 
 * Details about 
 *   The DL_POLY Molecular Simulation Package
 * can be found at http://www.cse.clrc.ac.uk/msi/software/DL_POLY/
 * Following DL_POLY configuration files are (partially) supported:
 *
 *     CONFIG/REVCON                    read-write
 *     HISTORY formatted                read
 *     HISTORY unformatted (binary)     not implemented 
 * HISTORY file provides trajectory and can be very large.
 *
 * To see what features are not supported yet read comments and the code.  
 *
 * When using shell model, there is no way to recognize which "atom"
 * represents core and which shell using only CONFIG/HISTORY file.
 * Convention is used: name of shell "atom" is created with
 * one of following postfixes: -shell _shell -shel _shel -shl _shl -sh _sh
 * eg. Zn-shl or Zn_shel
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "gdis.h"
#include "coords.h"
#include "model.h"
#include "file.h"
#include "parse.h"
#include "scan.h"
#include "matrix.h"
#include "interface.h"
#include "animate.h"

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/* TODO - use the tokenize() and/or get_tokens() function, which pretty much do this stuff */

/* Replaces all blanks in str with '\0' and put pointers to tokens into tokens.
 * Returns number of tokens (n <= max_split). It doesn't allocates any memory.
 * tokens[] should have length >= max_split. 
 */
int split_string(char *str, int max_split, char **tokens)
{
    char *ptr;
    int counter=0;
    int was_space=1;
    for (ptr=str; *ptr; ++ptr) {
        if (isspace(*ptr)) {
            *ptr=0;
            was_space=1;
        }
        else if (was_space) {
            was_space=0;
            tokens[counter] = ptr;
            counter++;
            if (counter >= max_split)
                break;
        }
    }
    return counter;
}

int get_splitted_line(FILE *fp, int max_split, char **tokens)
{
    gchar line[1024];
    if (fgets(line, 1024, fp))
        return split_string(line, max_split, tokens);
    else
        return 0;
}

/**************** file reading ****************/

/*****************/
/* read PBC info */
/*****************/
/* imcon -- periodic boundary key: 
                imcon       meaning
                0   no periodic boundaries
                1   cubic boundary conditions
                2   orthorhombic boundary conditions
                3   parallelepiped boundary conditions
                4   truncated octahedral boundary conditions
                5   rhombic dodecahedral boundary conditions
                6   x-y parallelogram boundary conditions with
                    no periodicity in the z direction
                7   hexagonal prism boundary conditions
*/
gint read_dlpoly_pbc(gpointer scan, gint imcon, struct model_pak *model)
{
gint num_tokens, i, j;
gchar **buff;

switch (imcon)
  {
  case 0:
    model->periodic = 0;
    break;

/* dl_poly just dumps lattice vectors I think? */
  default:
    model->periodic = 3;
    for (i=0 ; i<3 ; ++i)
      {
      buff = scan_get_tokens(scan, &num_tokens);
      if (num_tokens > 2)
        {
        for (j=0; j<3; ++j)
          model->latmat[3*i+j] = str_to_float(buff[j]);

        model->construct_pbc = TRUE;
        matrix_lattice_init(model); 
        }
      g_strfreev(buff);
      }
  }

return(0);
}

/* checks if "atom" is a shell in core-shell model */
static gint read_dlpoly_is_shell(gchar *name)
{
    /* check for postfixes: -shell _shell -shel _shel -shl _shl -sh _sh
     * and return postfix length */
    int len = strlen(name);
    if (len > 6 && (g_ascii_strcasecmp(name+len-6, "-shell") == 0
                    || g_ascii_strcasecmp(name+len-6, "_shell") == 0))
        return 6;
    else if (len > 5 && (g_ascii_strcasecmp(name+len-5, "-shel") == 0
                    || g_ascii_strcasecmp(name+len-5, "_shel") == 0))
        return 5;
    else if (len > 4 && (g_ascii_strcasecmp(name+len-4, "-shl") == 0
                    || g_ascii_strcasecmp(name+len-4, "_shl") == 0))
        return 4;
    else if (len > 3 && (g_ascii_strcasecmp(name+len-3, "-sh") == 0
                    || g_ascii_strcasecmp(name+len-3, "_sh") == 0))
        return 3;
    else
        return 0;
}

/* OH -> O, etc */
static void dlpoly_cut_elem_postfix(gchar *elem)
{
    /* don't cut Cl, Ca, Cs, Cr, Co, Cu, Cd, Ce, Os, Hf */
    gchar c = elem[0];
    gchar x = 0;
    if (strlen(elem) <= 1)
        return;
    x = elem[1];
    if (((c=='C' || c=='c') && (x!='l' && x!='L' && x!='a' && x!='A' &&
                                x!='s' && x!='S' && x!='r' && x!='R' &&
                                x!='o' && x!='O' && x!='u' && x!='U' &&
                                x!='d' && x!='D' && x!='e' && x!='E'))
        || ((c=='O' || c=='o') && (x!='s' && x!='S')) 
        || ((c=='H' || c=='h') && (x!='f' && x!='F')))
        elem[1] = '\0';
}

/*****************************************/
/* read in atoms and assoc data (if any) */
/*****************************************/
/* NB:levcfg: 1 - only coordinates in file
              2 - coordinates and velocities
              3 - coordinates, velocities and forces 
*/
gint read_dlpoly_atoms(gpointer scan, gint levcfg, struct model_pak *model)
{
gchar **buff;
gint rec=0, num_tokens, sl=0;
gdouble *x=0, *v=0;
struct core_pak *core=NULL;
struct shel_pak *shell=NULL;
GSList *core_list, *shell_list;

model->cores = g_slist_reverse(model->cores);
model->shels = g_slist_reverse(model->shels);
core_list = model->cores;
shell_list = model->shels;

while (!scan_complete(scan))
  {
  buff = scan_get_tokens(scan, &num_tokens);
  if (!buff)
    break;
  if (g_ascii_strncasecmp(buff[0], "timestep", 8) == 0)
    break;

  switch (rec)
    {
    case 0:
      if (num_tokens < 1)
        {
        gui_text_show(ERROR, "Unexpected syntax in DL_POLY file (in record i).\n");
        break;
        }
      sl = read_dlpoly_is_shell(buff[0]);
      if (sl)
        {
//printf("shell: %s\n", buff[0]);
/*
        g_strlcpy(elem, tokens[0], 9);
        elem[strlen(elem) - sl] = '\0';
        dlpoly_cut_elem_postfix(elem);
*/

        if (shell_list)
          {
          shell = shell_list->data;
          shell_list = g_slist_next(shell_list);
          }
        else
          {
          shell = shell_new(buff[0], buff[0], model);
          model->shels = g_slist_prepend(model->shels, shell);
          }
        x = shell->x;
        v = shell->v;
        }
      else
        {
//printf("core: %s\n", buff[0]);
/*
        g_strlcpy(elem, tokens[0], 9);
        dlpoly_cut_elem_postfix(elem);
*/

        if (core_list)
          {
          core = core_list->data;
          core_list = g_slist_next(core_list);
          }
        else
          {
          core = core_new(buff[0], buff[0], model);
          model->cores = g_slist_prepend(model->cores, core);
          }
        x = core->x;
        v = core->v;
        }
      break;

    case 1:
      if (num_tokens < 3)
        {
        gui_text_show(ERROR, "Unexpected syntax in DL_POLY file (in record ii).\n");
        return 1;
        }
      VEC4SET(x, str_to_float(buff[0]), str_to_float(buff[1]), str_to_float(buff[2]), 1.0);
      break;

    case 2:
      if (num_tokens < 3)
        {
        gui_text_show(ERROR, "Unexpected syntax in DL_POLY file (in record iii).\n");
        return 1;
        }
      VEC3SET(v, str_to_float(buff[0]), str_to_float(buff[1]), str_to_float(buff[2]));
      break;

    case 3:
      if (num_tokens < 3)
        {
        gui_text_show(ERROR, "Unexpected syntax in DL_POLY file (in record iv).\n");
        return 1;
        }
/* no force info in GDIS */
      break;

    default:
      printf("ERROR\n");
    }

  g_strfreev(buff);

  rec = (rec+1) % (levcfg+2); /* rec = 0, 1, ..., levcfg+1, 0, 1, ... */
  }

model->cores = g_slist_reverse(model->cores);
model->shels = g_slist_reverse(model->shels);
//printf("# cores: %i; # shells: %i\n", g_slist_length(model->cores), g_slist_length(model->shels));

return(0);
}

/********************************/
/* dl_poly file format (revcon) */
/********************************/
/* FIXME - assumed tokenize will work - but dl_poly uses stupid formated output */
/* which could result in no spaces in between (eg) coords and therefore no tokens */
gint read_dlpoly(gchar *filename, struct model_pak *model)
{
gint levcfg=0, imcon=0;
gint num_tokens, history=FALSE;
gchar *line, **buff;
gpointer scan;

scan = scan_new(filename);
if (!scan)
  return(1);

/* 1st line is the title */
line = scan_get_line(scan);
model->title = parse_strip_newline(line);

/* 2nd line - type and symmetry */
buff = scan_get_tokens(scan, &num_tokens);
if (num_tokens > 1) 
  {
  levcfg = str_to_float(buff[0]);
  imcon = str_to_float(buff[1]);
  }
g_strfreev(buff);

/* 3rd line - could be history? */
line = scan_get_line(scan);
if (g_ascii_strncasecmp(line, "timestep", 8) == 0)
  history = TRUE;

/* FIXME - I don't understand this format */
if (history) 
  {
  printf("Bizzare dl_poly format (history) not understood\n");
  }
else 
  {
/* not history - put back */
  scan_put_line(scan);
/* parse pbc */
  if (imcon)
    read_dlpoly_pbc(scan, imcon, model);
/* parse coords */
  read_dlpoly_atoms(scan, levcfg, model);
  }

/* model setup */
g_free(model->filename);
model->filename = g_strdup(filename);

g_free(model->basename);
model->basename = parse_strip(filename);

model->fractional = FALSE;
model_prep(model);

scan_free(scan);

return 0;
}

/********************************/
/* write dl_poly (revcon?) file */
/********************************/
gint write_dlpoly(gchar *filename, struct model_pak *model)
{
GSList *list;
struct core_pak *core;
struct shel_pak *shell;
FILE *fp;
gint levcfg=0, imcon=0;
gdouble x[3];
gint i, count;

/* checks */
g_return_val_if_fail(model != NULL, 1);
g_return_val_if_fail(filename != NULL, 2);

/* open the file */
fp = fopen(filename,"wt");
if (!fp)
  return(3);
    
/* first line is a title */
if (model->title)
  fprintf(fp, "%s\n", model->title);
else
  gdis_blurb(fp);

/* FIXME - I'm going to assume dl_poly dump lattice vectors here */
/* FIXME - are they in rows or columns??? */
switch (model->periodic)
  {
  case 3:
// default to parallel piped
    imcon = 3;
    if (model->latmat[0*3+1] == 0.  && model->latmat[0*3+2] == 0.
            && model->latmat[1*3+0] == 0. && model->latmat[1*3+2] == 0.
            && model->latmat[2*3+0] == 0. && model->latmat[2*3+1] == 0.)
      {
      if (model->latmat[0*3+0] == model->latmat[1*3+1] && model->latmat[0*3+0] == model->latmat[2*3+2])
        imcon = 1; //cubic
      else
        imcon = 2; //orthorhombic
      }
    break;

  default:
    imcon = 0; //no PBC
  }

/* TODO when levcfg should be 1? */
fprintf(fp, "%10i%10i\n", levcfg, imcon);

/* TODO write current nstep and time-step */
/* output lattice vectors */
if (imcon > 0)
  {
  for (i=0; i<3; ++i)
     fprintf(fp, "%20.10f%20.10f%20.10f\n", model->latmat[3*i], model->latmat[3*i+1], model->latmat[3*i+2]);
  }

/* output atoms */
count=1;
for (list=model->cores ; list ; list=g_slist_next(list))
  {
  core = list->data;
  if (core->status & DELETED)
    continue;

  fprintf(fp, "%-8s%-10d\n", core->atom_label, count++); 
 
  ARR3SET(x, core->x);
  vecmat(model->latmat, x);
  fprintf(fp, "%20.8f%20.8f%20.8f\n", x[0], x[1], x[2]);
  if (levcfg > 0)
    fprintf(fp,"%20.8f%20.8f%20.8f\n",core->v[0],core->v[1],core->v[2]);
  }

/* TODO order of atoms (shells and cores) should be preserved,
 * now if there are shells, they go at the end */
for (list=model->shels ; list ; list=g_slist_next(list))
  {
  shell = list->data;
  if (shell->status & DELETED)
      continue;

  fprintf(fp, "%-8s\n", shell->shell_label); 
    
  ARR3SET(x, shell->x);
  vecmat(model->latmat, x);
  fprintf(fp, "%20.8f%20.8f%20.8f\n", x[0], x[1], x[2]);
  if (levcfg > 0)
      fprintf(fp, "%20.8f%20.8f%20.8f\n", shell->v[0], shell->v[1], shell->v[2]);
  }

fclose(fp);

return(0);
}

