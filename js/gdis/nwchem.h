/*
Copyright (C) 2004 by Andrew Lloyd Rohl and Sean David Fleming

andrew@ivec.org

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

/* definitions */

enum {
// control
NWCHEM_DUMMY, NWCHEM_START, NWCHEM_GEOMETRY,
NWCHEM_SYSTEM, NWCHEM_CRYSTAL, NWCHEM_SURFACE, NWCHEM_POLYMER,
NWCHEM_BASIS, NWCHEM_CHARGE, NWCHEM_TASK, NWCHEM_END,
// theory
NWCHEM_TASK_SCF, NWCHEM_TASK_DFT, NWCHEM_TASK_SODFT, NWCHEM_TASK_MP2, NWCHEM_TASK_DIRECT_MP2,
NWCHEM_TASK_RIMP2, NWCHEM_TASK_CCSD, NWCHEM_TASK_CCSD3, NWCHEM_TASK_CCSD4, NWCHEM_TASK_MCSCF,
NWCHEM_TASK_SELCI, NWCHEM_TASK_MD, NWCHEM_TASK_PSPW, NWCHEM_TASK_BAND, NWCHEM_TASK_TCE,
// operation
NWCHEM_TASK_ENERGY, NWCHEM_TASK_GRADIENT, NWCHEM_TASK_OPTIMIZE, NWCHEM_TASK_SADDLE, NWCHEM_TASK_HESSIAN,
NWCHEM_TASK_FREQUENCIES, NWCHEM_TASK_PROPERTY, NWCHEM_TASK_DYNAMICS, NWCHEM_TASK_THERMODYNAMICS,
NWCHEM_LAST};

extern gchar *nwchem_symbol_table[];

/* structures */
struct nwchem_pak
{
gchar *start;
gchar *charge;
gchar *task_theory;
gchar *task_operation;
GHashTable *library;
};

/* prototypes */
