/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dfs.cc
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
 * $Id: dfs.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "dfs.h"

/**
 * getAdjacentActors ()
 * The function returns a list with actors directly reachable from
 * actor a (in case transpose if false). If transpose is 'true', the
 * graph is transposed and the function returns a list with actors
 * which are directly reachable from a in the transposed graph.
 */
static
SDFactors getAdjacentActors(SDFactor *a, bool transpose)
{
    SDFactors actors;
    
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        
        if (!transpose)
        {
            if (p->getType() == SDFport::Out)
                actors.push_back(p->getChannel()->getDstActor());
        }
        else
        {
            if (p->getType() == SDFport::In)
                actors.push_back(p->getChannel()->getSrcActor());
        }
    }
    
    return actors;
}

/**
 * getNextActor ()
 * The function returns the actor from the list of actors with the highest
 * order. Its order is set to -1. If all actors have order -1, a NULL pointer is
 * returned.
 */
static
SDFactor *getNextActor(SDFactors &actors, v_int &order)
{
    SDFactor *a = NULL;
    int orderA = -1;
        
    // Find actor with largest order
    for (SDFactorsIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        SDFactor *b = *iter;
        
        if (orderA < order[b->getId()])
        {
            a = b;
            orderA = order[b->getId()];
        }
    }
    
    // All actors have order -1
    if (orderA == -1)
        return NULL;
    
    return a;
}

/**
 * dfsVisit ()
 * The visitor function of the DFS algorithm.
 */
static
void dfsVisit(SDFactor *u, int &time, v_int &color, v_int &d, v_int &f, 
        SDFactor **pi, bool transpose)
{    
    // color[u] <- gray
    color[u->getId()] = 1;
    
    // d[u] <- time <- time + 1
    time++;
    d[u->getId()] = time;
    
    // for each v in Adj(e)
    SDFactors adj = getAdjacentActors(u, transpose);
    for (SDFactorsIter iter = adj.begin(); iter != adj.end(); iter++)
    {
        SDFactor *v = *iter;
        
        // do if color[v] = white
        if (color[v->getId()] == 0)
        {
            pi[v->getId()] = u;
            dfsVisit(v, time, color, d, f, pi, transpose);
        }
    }
    
    // color[u] <- black
    color[u->getId()] = 2;
    
    // f[u] <- time <- time + 1
    time++;
    f[u->getId()] = time;
}

/**
 * dfs ()
 * The function performs a depth first search on the graph. The discover
 * time (d), finish time (f) and predecessor tree (pi) are passed back.
 * The graph is transposed if the argument 't' is 'true'. Vertices are
 * considered in decreasing order of f[u].
 *
 * The function derives an 'order' vector from the vector 'f'. Actors are
 * considered in decreasing order according to this vector. When an actor
 * is visited, its order is set to -1. At the end, all actors must have 
 * order -1 (i.e. all actors are visited).
 */
void dfs(SDFgraph *g, v_int &d, v_int &f, SDFactor **pi, bool transpose)
{
    int time;
    
    // for each u in G do order[u] <- f[u]
    v_int order(f);

    // for each u in G do color[u] <- white
    v_int color(g->nrActors(), 0);

    // time <- 0
    time = 0;
    
    // for each u in G do pi[u] <- NIL
    for (uint u = 0; u < g->nrActors(); u++)
        pi[u] = NULL;

    // for each u in G (visit in order given by order)
    for (SDFactor *a = getNextActor(g->getActors(), order); a != NULL; 
            a = getNextActor(g->getActors(), order))
    {
        // Mark actor as visited
        order[a->getId()] = -1;

        // if color[u] = white
        if (color[a->getId()] == 0)
            dfsVisit(a, time, color, d, f, pi, transpose);
    }
}
