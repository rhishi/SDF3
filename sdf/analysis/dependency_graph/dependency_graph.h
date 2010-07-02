/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dependency_graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 20, 2006
 *
 *  Function        :   Find the abstract dependency graph for an SDFG
 *
 *  History         :
 *      20-06-06    :   Initial version.
 *
 * $Id: dependency_graph.h,v 1.1 2008/03/06 10:49:43 sander Exp $
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

#ifndef SDF_ANALYSIS_DEPENDENCY_GRAPH_DEPENDENCY_GRAPH_H_INCLUDED
#define SDF_ANALYSIS_DEPENDENCY_GRAPH_DEPENDENCY_GRAPH_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * stateSpaceAbstractDepGraph ()
 * Compute the throughput and abstract dependency graph of an SDFG for
 * unconstrained buffer sizes and using auto-concurrency using a state-space
 * traversal.
 *
 * The abstract dependency graph is stored in the matrices 'nodes' (2D) and
 * 'edges' (3D). The value stored at 'nodes[a][b]' indicates whether there is at
 * least one edge which goes directlt from node a to node b. The value stored at
 * 'edges[a][b][c]' indicates wether the channel c in the SDFG has a depedency
 * edge in the abstract dependency graph from node a to node b.
 */
double stateSpaceAbstractDepGraph(TimedSDFgraph *gr, bool ***nodes, 
        bool ****edges, unsigned long long stackSz = 1000, 
        unsigned long long hashSz = 10000);

#endif
