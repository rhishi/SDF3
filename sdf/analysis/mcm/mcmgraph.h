/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mcmgraph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 7, 2005
 *
 *  Function        :   Convert HSDF graph to weighted directed graph for MCM
 *                      calculation.
 *
 *  History         :
 *      07-11-05    :   Initial version.
 *
 * $Id: mcmgraph.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_ANALYSIS_MCM_MCMGRAPH_H_INCLUDED
#define SDF_ANALYSIS_MCM_MCMGRAPH_H_INCLUDED

#include "../../base/timed/graph.h"

struct _MCMnode;

typedef struct _MCMedge
{
    uint id;
    bool visible;
    struct _MCMnode *src;
    struct _MCMnode *dst;
    int w;
    uint d;
} MCMedge;

typedef list<MCMedge*>      MCMedges;
typedef MCMedges::iterator  MCMedgesIter;

typedef struct _MCMnode
{
    uint id;
    bool visible;
    MCMedges in;
    MCMedges out;
} MCMnode;

typedef list<MCMnode*>      MCMnodes;
typedef MCMnodes::iterator  MCMnodesIter;

class MCMgraph
{
public:
    // Constructor
    MCMgraph();
    
    // Destructor
    ~MCMgraph();
    
    // Nodes
    MCMnodes nodes;
    uint nrNodes() {
        uint nrNodes = 0;
        for (MCMnodesIter iter = nodes.begin(); iter != nodes.end(); iter++)
            if ((*iter)->visible) nrNodes++;
        return nrNodes;
    };
    
    // Edges
    MCMedges edges;
    uint nrEdges() {
        uint nrEdges = 0;
        for (MCMedgesIter iter = edges.begin(); iter != edges.end(); iter++)
            if ((*iter)->visible) nrEdges++;
        return nrEdges;
    };
};

typedef list<MCMgraph*>      MCMgraphs;
typedef MCMgraphs::iterator  MCMgraphsIter;

/**
 * transformHSDFtoMCMgraph ()
 * The function converts an HSDF graph to a weighted directed graph
 * used in the MCM algorithm of Karp (and its variants). By default, the
 * a longest path calculation is performed to make the graph suitable
 * as input for an MCM algorithm. Setting the flag 'mcmFormulation' to false
 * result in an MCM graph which is suitable for an MCR (cycle ratio)
 * formulation. This avoids running the longest path algo.
 */
MCMgraph *transformHSDFtoMCMgraph(TimedSDFgraph *g, bool mcmFormulation = true);

/**
 * Extract the strongly connected components from the graph. These components
 * are returned as a set of MCM graphs. All nodes which belong to at least
 * one of the strongly connected components are set to visible in the graph g,
 * all other nodes are made invisible. Also edges between two nodes in (possibly
 * different) strongly connected components are made visible and all others
 * invisible. The graph g consists in the end of only nodes which are part of
 * a strongly connnected component and all the edges between these nodes. Some
 * MCM algorithms work also on this graph (which reduces the execution time
 * needed in some of the conversion algorithms).
 */
void stronglyConnectedMCMgraph(MCMgraph *g, MCMgraphs &components);

/**
 * relabelMCMgraph ()
 * The function removes all hidden nodes and edges from the graph. All visible
 * edges are assigned a new id starting in the range [0,nrNodes()).
 */
void relabelMCMgraph(MCMgraph *g);

#endif
