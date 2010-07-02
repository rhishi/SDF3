/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 29, 2005
 *
 *  Function        :   Convert SDF graph to basic graph structure
 *
 *  History         :
 *      29-07-05    :   Initial version.
 *
 * $Id: graph.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_ALGO_GRAPH_H_INCLUDED
#define SDF_BASE_ALGO_GRAPH_H_INCLUDED

#include "components.h"
#include "../untimed/graph.h"

/**
 * sdfToGraph ()
 * The function converts a SDF graph G(A,C) to an graph G'(N,E). For each 
 * actor a in A, a node u in N is present in the graph G'. The actor a and
 * node n are related to each other via the arrays nodeId (u = nodeId[a]) and 
 * actorId (a = actorId[u]). For each channel (a,b) in C an edge (u,v) in E 
 * is created in the graph (graph[u][v] = graph[nodeId[a]][nodeId[b]] = 1). If
 * no edge exists between two actors a and b, the corresponding graph[u][v] = 0.
 * Note: that in multiple channels in the SDF graph are translated into a single
 * edge in the graph.
 */
void sdfToGraph(SDFgraph *g, v_uint &nodeId, v_uint &actorId, 
        vector<v_uint> &graph, const bool transpose = false);

/**
 * sdfToGraph ()
 * The function converts a component S of an SDF graph G(A,C) to an graph
 * G'(N,E). For each actor a in S (i.e. also a in A), a node u in N is present
 * in the graph G'. The actor a and node n are related to each other via the 
 * arrays nodeId (u = nodeId[a]) and actorId (a = actorId[u]). For each channel
 * (a,b) in C an edge (u,v) in E is created in the graph (graph[u][v] =
 * graph[nodeId[a]][nodeId[b]] = 1). If no edge exists between two actors a and
 * b, the corresponding graph[u][v] = 0. Note: that in multiple channels in the
 * SDF graph are translated into a single edge in the graph.
 */
void sdfComponentToGraph(SDFgraph *g, SDFgraphComponent &component, 
        v_uint &nodeId, v_uint &actorId, vector<v_uint> &graph,
        const bool transpose = false);

#endif
