/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dfs.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Depth-first search
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: dfs.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#ifndef SDF_BASE_ALGO_DFS_H_INCLUDED
#define SDF_BASE_ALGO_DFS_H_INCLUDED

#include "base/base.h"
#include "../untimed/graph.h"

/**
 * dfs ()
 * The function performs a depth first search on the graph. The discover
 * time (d), finish time (f) and predecessor tree (pi) are passed back.
 * The graph is transposed if the argument 'transpose' is 'true'. Vertices are
 * considered in decreasing order of f[u].
 *
 * Note: enough memory space must be allocated for the d, f, and pi vectors
 * by the calling function.
 */
void dfs(SDFgraph *g, v_int &d, v_int &f, SDFactor **pi,
            bool transpose = false);

#endif
