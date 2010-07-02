/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   connected.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 29, 2005
 *
 *  Function        :   Check that SDF graph is connected
 *
 *  History         :
 *      29-07-05    :   Initial version.
 *
 * $Id: connected.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "connected.h"

/**
 * getAdjacentActors ()
 * The function returns a list with actors directly reachable from
 * actor a. The graph is considered to be undirected.
 */
static
SDFactors getAdjacentActors(SDFactor *a)
{
    SDFactors actors;
    
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        
        if (p->getType() == SDFport::Out)
            actors.push_back(p->getChannel()->getDstActor());
        if (p->getType() == SDFport::In)
            actors.push_back(p->getChannel()->getSrcActor());
    }
    
    return actors;
}

/**
 * connectedVisit ()
 * The visitor function of the DFS algorithm.
 */
static
void connectedVisit(SDFactor *u, v_int &color)
{    
    // color[u] <- gray
    color[u->getId()] = 1;
    
    // for each v in Adj(e)
    SDFactors adj = getAdjacentActors(u);
    for (SDFactorsIter iter = adj.begin(); iter != adj.end(); iter++)
    {
        SDFactor *v = *iter;
        
        // do if color[v] = white
        if (color[v->getId()] == 0)
        {
            connectedVisit(v, color);
        }
    }
}

/**
 * isConnected ()
 * The function returns true if the SDF graph is connected, else it
 * returns false.
 *
 * A graph is connected if a DFS on the undirected graph from an
 * arbitrary node visits all nodes in the graph.
 */
bool isConnected(SDFgraph *g)
{
    // for each u in G do color[u] <- white
    v_int color(g->nrActors(), 0);

    // Start DFS from first actor
    connectedVisit(*(g->actorsBegin()), color);

    // All actors must be visited in connected graph
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        // if color[u] == white
        if (color[a->getId()] == 0)
            return false;
    }
    
    return true;
}
