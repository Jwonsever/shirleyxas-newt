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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gdis.h"
#include "coords.h"
#include "error.h"
#include "file.h"
#include "parse.h"
#include "matrix.h"
#include "zmatrix.h"
#include "model.h"
#include "interface.h"
#include "animate.h"

#define DEBUG_READ_PW_BLOCK 0

gint set_true_false();
gint set_energy_units();
gint set_temperature_units();
gint set_length_units();
gint set_force_units();
gint set_pressure_units();
gint set_time_units();
gint set_mominert_units();

gint print_file_energy_units();
gint print_file_mass_units();
gint print_file_length_units();
gint print_file_time_units();
gint print_file_temperature_units();
gint print_file_pressure_units();
gint print_file_force_units();
gint print_file_mominert_units();

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

enum {PW_TYPE,  PW_LATTICE_VECTORS,  PW_COORDS};
/****************/
/* file writing */
/****************/
gint write_pwi(gchar *filename, struct model_pak *model)
    {
    gdouble x[3],dm;
    GSList *list,*species_list;
    struct core_pak *core;
    struct species_pak *species_data;
   // struct elem_pak *elem;
    FILE *fp;

    /* checks */
    g_return_val_if_fail(model != NULL, 1);
    g_return_val_if_fail(filename != NULL, 2);
    species_list = fdf_species_build(model);

    /* open the file */
    fp = fopen(filename,"wt");
    if (!fp)
        return(3);
   // dm=abs(model->latmat[0]);
    dm=1;
    /* print header */
    fprintf(fp,"    &CONTROL \ncalculation = 'scf'\nprefix='%s'\npseudo_dir = './'\noutdir='./'\ntstress=.true.\ntprnfor = .true.\nforc_conv_thr=1.0d-4\nnstep=200\n/\n",model->basename);
    fprintf(fp,"    &SYSTEM \nibrav=0\ncelldm(1)=%f\nnat=%d\nntyp=%d \necutwfc=36\n/ \n",dm/AU2ANG,g_slist_length(model->cores),g_slist_length(species_list));
    fprintf(fp,"    &ELECTRONS\nmixing_beta=0.7\nconv_thr =  1.0d-8\nelectron_maxstep=200\n/\n");
    fprintf(fp,"    &IONS\ntrust_radius_max=0.2\n/\n");
    fprintf(fp,"    &CELL\ncell_dynamics='bfgs'\n/\n");
    fprintf(fp,"ATOMIC_SPECIES\n");

     for (list=species_list ; list ; list=g_slist_next(list))
        {
    species_data = list->data;
   fprintf(fp, "  %2s  %7.4f  %2s.UPF\n",  species_data->label,elements[species_data->number].weight,species_data->label);
         }

    
    fprintf(fp,"\nCELL_PARAMETERS (alat) \n");
    fprintf(fp,"%15.10f %15.10f %15.10f\n",
        model->latmat[0]/dm, model->latmat[3]/dm, model->latmat[6]/dm);
    fprintf(fp,"%15.10f %15.10f %15.10f\n",
        model->latmat[1]/dm, model->latmat[4]/dm, model->latmat[7]/dm);
    fprintf(fp,"%15.10f %15.10f %15.10f\n\n",
        model->latmat[2]/dm, model->latmat[5]/dm, model->latmat[8]/dm);

    fprintf(fp,"ATOMIC_POSITIONS (crystal)\n");

    for (list=model->cores ; list ; list=g_slist_next(list))
        {
        core = list->data;
        if (core->status & DELETED)
            continue;

        ARR3SET(x, core->x);
        // vecmat(model->latmat, x);
        fprintf(fp,"%2s    %14.9f  %14.9f  %14.9f\n",
            elements[core->atom_code].symbol, x[0], x[1], x[2]);
        }
    fprintf(fp,"K_POINTS {automatic}\n 4 4 4  1 1 1 \n");

    fclose(fp);
    return(0);
    }

/*********************************/
/* process a lattice vector line */
/*********************************/
void pw_parse_lattice(const gchar *line, gint n, gdouble scale,  struct model_pak *model)
    {
    gint num_tokens;
    gchar **buff;

    g_assert(line != NULL);

    buff = tokenize(line, &num_tokens);
    if (num_tokens > 2 && str_is_float(*(buff+0)))
        {
        model->latmat[0+n] = str_to_float(*(buff+0))*scale;
        model->latmat[3+n] = str_to_float(*(buff+1))*scale;
        model->latmat[6+n] = str_to_float(*(buff+2))*scale;
        }
    else
        {
        model->latmat[0+n] = str_to_float(*(buff+3))*scale;
        model->latmat[3+n] = str_to_float(*(buff+4))*scale;
        model->latmat[6+n] = str_to_float(*(buff+5))*scale;
        }

    model->construct_pbc = TRUE;
    model->periodic = 3;
#if DEBUG_READ_PW_BLOCK
    printf("lattice: %f %f %f\n", model->latmat[0+n],model->latmat[3+n],model->latmat[6+n]);
#endif

    g_strfreev(buff);
    }

/******************************/
/* main pwo data block reader */
/******************************/
gint read_pwo_block(FILE *fp, struct model_pak *model)
    {
    gint count,  num_tokens,  block_type=333;
    gchar **buff, *line;
    struct core_pak *core;
    gdouble scale = AU2ANG;
    GSList *clist=NULL;
    g_assert(fp != NULL);
    g_assert(model != NULL);

    clist = model->cores;

    for (;;)
        {
        line = file_read_line(fp);
        if (!line)
            return (1);
        buff = tokenize(line, &num_tokens);

        /* get next line if blank */
        if (!buff)
            continue;
        // one keyword per line
        if (g_strrstr(line, "number of atoms") != NULL)
            {
            model->num_atoms =(gint) str_to_float(*(buff+4));
            continue;
            }
        /* lattice constant scaling */
        if (g_strrstr(line, "celldm(1)") != NULL) {
            buff=g_strsplit (line, "=", -1);
            if (str_to_float(*(buff+1))!= 0.0) {
                model->siesta.eden_scale=AU2ANG*str_to_float(*(buff+1));
                }
            else 
                model->siesta.eden_scale=AU2ANG;
            continue;
            }
        /* initial atomic positions */
        if (g_strrstr(line, "CELL_PARAMETERS") != NULL||g_strrstr(line, "cart. coord. in units of a_0") != NULL) {
            block_type =  PW_LATTICE_VECTORS;
            }

/* GDIS will get confused if you set both model->fractional true and false while reading the same file */
        //if (g_strrstr(line, "ATOMIC_POSITIONS") != NULL||g_strrstr(line, "positions (a_0 units)") != NULL) {

        if (g_strrstr(line, "ATOMIC_POSITIONS"))
            {
            if (g_strrstr(line, "crystal"))
              {
              model->fractional = TRUE;
              scale=1.0;
              } 
            else
              {
              model->fractional = FALSE;
              if (g_strrstr(line,"angstrom") != NULL)
                scale=1.0;
              else if (g_strrstr(line,"alat") != NULL||g_strrstr(line, "positions (a_0 units)") != NULL)
                {
                scale=model->siesta.eden_scale;
                }
              }
            block_type =  PW_COORDS;
            }

#if DEBUG_READ_PW_BLOC//K
        printf("processing block [type %d]\n", block_type);
        printf("line:  %s", line);
#endif
        if (block_type==1 ||block_type==2)
            {
            count = 0; 
            for (;;)
                {
                line = g_strjoinv(" ", g_strsplit(file_read_line(fp), "(", -1));
                if (!line )
                    return (2);
                buff = tokenize(line, &num_tokens);

                /* get next line if blank */
                if (!buff)
                    continue;

                switch (block_type)
                    {
                    case  PW_TYPE:
                        break;

                    case  PW_LATTICE_VECTORS:
                        { 
                        pw_parse_lattice(line, count, model->siesta.eden_scale,  model);
                        if (count==2) { 
                            block_type = 333;
                            goto  pw_done_line;
                            }
                        }
                        break;

                    case  PW_COORDS:
                        {
                        core=NULL;
                        if (clist)
                            {
#if DEBUG_READ_PW_BLOCK
                            printf("=======  clist =======");
#endif
                            core = clist->data;
                            clist = g_slist_next(clist);
                            g_free(core->atom_label);
                            if (str_is_float(*(buff+1))&&str_is_float(*(buff+2))&&str_is_float(*(buff+3)))
                                {
                                g_strcanon (*(buff+0), G_CSET_A_2_Z G_CSET_a_2_z,' ');
                                core->atom_label = g_strdup(*buff);
                                elem_init(core, model);
                                core->x[0] = scale*str_to_float(*(buff+1));
                                core->x[1] = scale*str_to_float(*(buff+2));
                                core->x[2] = scale*str_to_float(*(buff+3));
                                }
                            else  if (str_is_float(*(buff+6))&&str_is_float(*(buff+7))&&str_is_float(*(buff+5)))
                                {
                                g_strcanon (*(buff+1), G_CSET_A_2_Z G_CSET_a_2_z,' ');
                                core->atom_label = g_strdup(*buff+1);
                                elem_init(core, model);
                                core->x[0] = scale*str_to_float(*(buff+5));
                                core->x[1] = scale*str_to_float(*(buff+6));
                                core->x[2] = scale*str_to_float(*(buff+7));
                                }
                            }
                        else
                            {
#if DEBUG_READ_PW_BLOCK
                            printf("+++++ no clist ++++++");
#endif
                            if (str_is_float(*(buff+1))&& str_is_float(*(buff+2))&& str_is_float(*(buff+3)))
                                {
                                g_strcanon (*(buff+0), G_CSET_A_2_Z G_CSET_a_2_z,' ');
                                core = new_core(*(buff+0), model);
                                model->cores = g_slist_append(model->cores, core);
                                core->x[0] = scale*str_to_float(*(buff+1));
                                core->x[1] = scale*str_to_float(*(buff+2));
                                core->x[2] = scale*str_to_float(*(buff+3));
                                }
                            else if (str_is_float(*(buff+6))&& str_is_float(*(buff+7))&& str_is_float(*(buff+5)))
                                {
                                g_strcanon (*(buff+1), G_CSET_A_2_Z G_CSET_a_2_z,' ');
                                core = new_core(*(buff+1), model);
                                model->cores = g_slist_append(model->cores, core);
                                core->x[0] = scale*str_to_float(*(buff+5));
                                core->x[1] = scale*str_to_float(*(buff+6));
                                core->x[2] = scale*str_to_float(*(buff+7));
                                }
                            }
#if DEBUG_READ_PW_BLOCK
                        printf("processing coord:%s\n",line);
                        //print_core_cart(core);
                        //print_core(core);
#endif
                        g_free(line); 
                        g_strfreev(buff);
                        if (count == model->num_atoms-1)
                            {
                            return (0);
                            }
                        }
                        break;

                    default:
                        goto pw_done_line;
                        break;
                    } 
                count++;
                } 
            }
pw_done_line:;
        /* loop cleanup */
        g_strfreev(buff);
        g_free(line);
        } 
    return (0);
    }


/**************************/
/*reading pwo  files*/
/**************************/
gint read_pwo(gchar *filename, struct model_pak *model)
    {
    FILE *fp;

    /* checks */
    g_return_val_if_fail(model != NULL, 1);
    g_return_val_if_fail(filename != NULL, 2);

    fp = fopen(filename, "rt");
    if (!fp)
        return(3);

    // loop while there's data
    while (read_pwo_block(fp, model) == 0)
      animate_frame_store(model);

//    printf("Only ibrav=0, celldm(1), nat, CELL_PARAMETERS and ATOMIC_POSITIONS keywords are implemented in pwscf.out files\n");

    /* model setup */
    g_free(model->filename);
    model->filename = g_strdup(filename);
    g_free(model->basename);
    model->basename = g_strdup("model");

    model_prep(model);

    return(0);
    }


/**************************/
/*reading pwi  files*/
/**************************/
gint read_pwi(gchar *filename, struct model_pak *model)
    {
    gchar **buff, *line;
    gdouble scale = AU2ANG;
    gint i=0,num_tokens;
    FILE *fp;
    struct core_pak *core;

    /* checks */
    g_return_val_if_fail(model != NULL, 1);
    g_return_val_if_fail(filename != NULL, 2);

    fp = fopen(filename, "rt");
    if (!fp)
        return(3);

    for(;;)
        {
        line = file_read_line(fp);
        if (!line)
            break;
        buff = tokenize(line, &num_tokens);
        /* get next line if blank */
        if (!buff)
            continue;
        // one keyword per line
        if (g_strrstr(line, "nat") != NULL)
            {
            buff=g_strsplit (line, "=", -1);
            model->num_atoms =(gint) str_to_float(*(buff+1)); 
            continue;
            }
        /* lattice constant scaling */
        if (g_strrstr(line, "celldm(1)") != NULL)
            {
            buff=g_strsplit (line, "=", -1);
            if (str_to_float(*(buff+1))!= 0.0) {
                scale=AU2ANG*str_to_float(*(buff+1));
                }
            continue;
            }

        /* default cell dimensions */
        if (g_strrstr(line, "CELL_PARAMETERS") != NULL)
            {
            for (i=0;i<3;)
                {
                line=file_read_line(fp);
                buff = tokenize(line, &num_tokens);

                if (!buff)
                    continue;
                if (num_tokens == 3)
                    {
                    model->latmat[0+i] = scale*str_to_float(*(buff+0));
                    model->latmat[3+i] = scale*str_to_float(*(buff+1));
                    model->latmat[6+i] = scale*str_to_float(*(buff+2));
#if DEBUG_READ_PW_BLOCK
                    printf("reading cell_param:  %s\n", line);
                    printf("scale=  %f\n", scale);
                    printf("nat: %i\n", model->num_atoms);
                    printf("lattice: %15.10f %15.10f %15.10f\n", model->latmat[0+i],model->latmat[3+i],model->latmat[6+i]);
#endif
                    i++;
                    }
                }
            g_strfreev(buff);
            model->construct_pbc = TRUE;
            model->periodic = 3;
            continue;
            }   //  End CELL_PARAMETERS 
        if (g_strrstr(line, "ATOMIC_POSITIONS") != NULL)  {
            if (g_strrstr(line, "crystal") != NULL) {
                model->fractional = TRUE;
                scale=1.0;
                }
            else {
                model->fractional = FALSE;
                if (g_strrstr(line,"angstrom") != NULL) {
                    scale=1.0;
                    }
                else if (g_strrstr(line,"bohr") != NULL) {
                    scale=AU2ANG;
                    }
                }
            for ( i=0 ; i < model->num_atoms; i++) {
                line=file_read_line(fp);
                buff = tokenize(line, &num_tokens);
                /* get next line if blank */
                if (!buff)
                    continue;

                core = new_core(*(buff+0), model);
                model->cores = g_slist_append(model->cores, core);
                core->x[0] = scale*str_to_float(*(buff+1));
                core->x[1] = scale*str_to_float(*(buff+2));
                core->x[2] = scale*str_to_float(*(buff+3));
#if DEBUG_READ_PW_BLOCK
                printf("reading atomic pos:  %s\n", line);
                printf("scale for at pos:  %f\n", scale);
                printf("pos: %15.10f %15.10f %15.10f\n", core->x[0],core->x[1],core->x[2]);
#endif
                }
            break;
            } //End ATOMIC_POSITIONS
        }


//    printf("One keyword per line, please.\nOnly nat, celldm(1)=alat(Bohr), CELL_PARAMETERS and ATOMIC_POSITIONS keywords are readable in pwscf.in files \n");

    /* model setup */
    g_free(model->filename);
    model->filename = g_strdup(filename);
    g_free(model->basename);
    model->basename = g_strdup("model");

    model_prep(model);

    return(0);
    }
/*********************/
/* pwo  frame read   */
/*********************/
gint read_pwo_frame(FILE *fp, struct model_pak *model)
    {
    read_pwo_block(fp, model);
    return(0);
    }

