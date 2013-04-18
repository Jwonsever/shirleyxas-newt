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

enum {
IMAGE_ANIMATE,
IMAGE_ARROW,
IMAGE_AXES,
IMAGE_CABINET,
IMAGE_CANVAS_SINGLE,
IMAGE_CANVAS_CREATE, // 5
IMAGE_CANVAS_DELETE,
IMAGE_CAMERA,
IMAGE_COMPASS,
IMAGE_CROSS,
IMAGE_DIFFRACTION, // 10
IMAGE_DISK,
IMAGE_ELEMENT,
IMAGE_FOLDER,
IMAGE_INFO,
IMAGE_ISOSURFACE, // 15
IMAGE_MEASURE,  
IMAGE_PERIODIC,
IMAGE_PALETTE,
IMAGE_PLUS,
IMAGE_SURFACE, // 20
IMAGE_TOOLS,
IMAGE_USER,
IMAGE_PLOT,
IMAGE_DEFECT,
IMAGE_DYNAMICS, // 25
IMAGE_OVERLAY,
IMAGE_ZMATRIX,
IMAGE_COG,
IMAGE_ARCS,
IMAGE_HOME,     // 30
IMAGE_DOWNLOAD,
IMAGE_CONNECT,
IMAGE_LAST
};

void image_table_init(void);
void image_table_free(void);
gpointer image_table_lookup(gint);

