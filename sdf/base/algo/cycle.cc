/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   cycle.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Find all simple cycles in a graph
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: cycle.cc,v 1.2 2008/09/18 07:38:21 sander Exp $
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

#include "cycle.h"
#include "components.h"

/*
 *  findSimpleCycles(G):
 *  1 Split graph G(A,C) in a set of strongly connected components S
 *  2 For each s(A',C') in S
 *  3   For each actor a in A'
 *  4     color[a] <- white
 *  5     pi[a] <- NIL
 *  4   For an actor a in A'
 *  5     color[a] <- gray
 *  6     For each actor b in Adj(a, s)
 *  7       pi[b] <- a
 *  8       simpleCycleVisit(a)
 *
 *  simpleCycleVisit(a):
 *  1 color[a] <- gray
 *  2 For each b in Adj(a, s)
 *  3   Do if color[a] = white
 *  4     pi[b] <- a
 *  5     simpleCycleVisit(b)
 *  6   Else if color[a] = gray
 *  7     Create new cycle c
 *  8 color[a] <- white
 */
 
//#define __FIND_CYCLE_USING_SDFGRAPH__
#ifdef __FIND_CYCLE_USING_SDFGRAPH__
/**
 * getAdjacentActors ()
 * The function returns a list with actors directly reachable from
 * actor a (in case transpose if false). If transpose is 'true', the
 * graph is transposed and the function returns a list with actors
 * which are directly reachable from a in the transposed graph.
 */
static
SDFactors getAdjacentActors(SDFactor *a, SDFgraphComponent &component,
        bool transpose)
{
    SDFactors actors;
    
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        
        if (!transpose)
        {
            if (p->getType() == SDFport::Out)
            {
                SDFactor *b = p->getChannel()->getDstActor();
                
                if (actorInComponent(b, component))
                    actors.push_back(b);
            }
        }
        else
        {
            if (p->getType() == SDFport::In)
            {
                SDFactor *b = p->getChannel()->getSrcActor();

                if (actorInComponent(b, component))
                    actors.push_back(b);
            }
        }
    }

    return actors;
}

/**
 * simpleCycleVisit ()
 * The visitor function of the DFS based algorithm used for detecting 
 * simple cycles.
 */
static
void simpleCycleVisit(SDFactor *a, v_int &color, SDFactor **pi,
        bool transpose, SDFgraphComponent &component, SDFgraphCycles &cycles)
{
    // color[a] <- gray
    color[a->getId()] = 1;

    // for each b in Adj(e)
    SDFactors adj = getAdjacentActors(a, component, transpose);
    for (SDFactorsIter iter = adj.begin(); iter != adj.end(); iter++)
    {
        SDFactor *b = *iter;
        
        // do if color[b] = white
        if (color[b->getId()] == 0)
        {
            pi[b->getId()] = a;
            simpleCycleVisit(b, color, pi, transpose, component, cycles);
        }
        else if (color[b->getId()] == 1)
        {
            SDFgraphCycle cycle;
            
            // Create new cycle
            cycle.push_front(a);

            SDFactor *u = a;
            while (u->getId() != b->getId())
            {
                u = pi[u->getId()];
                if (u != NULL)
                    cycle.push_front(u);
            }

            // Add cycle to list of cycles
            cycles.insert(cycle);
        }
    }
    
    // color[a] <- white
    color[a->getId()] = 0;
}

/**
 * findSimpleCycles ()
 * The function performs a depth first search on the graph to discover all
 * simple cycles in the graph
 */
SDFgraphCycles findSimpleCycles(SDFgraph *g, bool transpose)
{
    SDFgraphCycles cycles;
    SDFactor **pi = new SDFactor* [g->nrActors()];

    // Split graph in strongly connected components
    SDFgraphComponents components = stronglyConnectedComponents(g);

    // Iterate over all components
    for (SDFgraphComponentsIter iter = components.begin();
            iter != components.end(); iter++)
    {
        SDFgraphComponent &component = *iter;
       
        // For each a in A' do color[a] <- white
        v_int color(g->nrActors(), 0);

        // For each a in A' do pi[a] <- NIL
        for (uint a = 0; a < g->nrActors(); a++)
            pi[a] = NULL;
    
        // For an actor a in A' (take the first actor)
        SDFactor *a = component.front();
        
        // color[a] <- gray
        color[a->getId()] = 1;
        
        // For each actor b in Adj(a, s)
        SDFactors adj = getAdjacentActors(a, component, transpose);
        for (SDFactorsIter iterA = adj.begin(); iterA != adj.end(); iterA++)
        {
            SDFactor *b = *iterA;
            
            if (color[b->getId()] == 0)
            {
                pi[b->getId()] = a;
                simpleCycleVisit(b, color, pi, transpose, component, cycles);
            }
            else if (color[b->getId()] == 1)
            {
                // Self-loop
                SDFgraphCycle cycle;

                cycle.push_front(a);
                cycles.insert(cycle);                
            }
        }
    }

    // Cleanup
    delete [] pi;

    return cycles;    
}

#else // __FIND_CYCLE_USING_SDFGRAPH__

#include "graph.h"

/**
 * getAdjacentActors ()
 * The function returns a list with actors directly reachable from
 * actor a.
 */
static
v_uint getAdjacentActors(uint a, v_uint &nodeId, v_uint &actorId,
        vector<v_uint> &graph, uint nrNodes)
{
    v_uint actors;
    
    uint u = nodeId[a];
    for (uint v = 0; v < nrNodes; v++)
    {
        if (graph[u][v] == 1)
            actors.push_back(actorId[v]);
    }

    return actors;
}

/**
 * simpleCycleVisit ()
 * The visitor function of the DFS based algorithm used for detecting 
 * simple cycles.
 */
static
void simpleCycleVisit(uint a, SDFgraph *g, v_uint &nodeId, v_uint &actorId, 
        vector<v_uint> &graph, uint size, v_int &color, v_uint &pi,
        SDFgraphCycles &cycles)
{
    // color[a] <- gray
    color[a] = 1;

    // for each b in Adj(e)
    v_uint adj = getAdjacentActors(a, nodeId, actorId, graph, size);
    for (uint i = 0; i < adj.size(); i++)
    {
        uint b = adj[i];
        
        // do if color[b] = white
        if (color[b] == 0)
        {
            pi[b] = a;
            simpleCycleVisit(b, g, nodeId, actorId, graph, size, color, pi,
                                cycles);
        }
        else if (color[b] == 1)
        {
            SDFgraphCycle cycle;
            
            // Create new cycle
            cycle.push_front(g->getActor(a));

            uint u = a;
            while (u != b)
            {
                u = pi[u];
                cycle.push_front(g->getActor(u));
            }

            // Add cycle to list of cycles
            cycles.insert(cycle);
        }
    }
    
    // color[a] <- white
    color[a] = 0;
}

/**
 * findSimpleCycles ()
 * The function performs a depth first search on the graph to discover all
 * simple cycles in the graph
 */
SDFgraphCycles findSimpleCycles(SDFgraph *g, bool transpose)
{
    SDFgraphCycles cycles;
    v_uint pi(g->nrActors());

    // Split graph in strongly connected components
    SDFgraphComponents components = stronglyConnectedComponents(g);

    // Iterate over all components
    for (SDFgraphComponentsIter iter = components.begin();
            iter != components.end(); iter++)
    {
        v_uint actorId, nodeId;
        vector<v_uint> graph;
        SDFgraphComponent &component = *iter;
       
        // Convert component to graph structure
        sdfComponentToGraph(g, component, nodeId, actorId, graph, transpose);
        
        // For each a in A' do color[a] <- white
        v_int color(g->nrActors(), 0);

        // For each a in A' do pi[a] <- NIL
        for (uint a = 0; a < g->nrActors(); a++)
            pi[a] = 0;

        // For an actor a in A' (take the first actor)
        uint a = actorId[0];
    
        // color[a] <- gray
        color[a] = 1;
        
        // For each actor b in Adj(a, s)
        v_uint adj = getAdjacentActors(a, nodeId, actorId, graph,
                                            component.size());
        for (uint i = 0; i < adj.size(); i++)
        {
            uint b = adj[i];
            
            if (color[b] == 0)
            {
                pi[b] = a;
                simpleCycleVisit(b, g, nodeId, actorId, graph, component.size(),
                                    color, pi, cycles);
            }
            else if (color[b] == 1)
            {
                // Self-loop
                SDFgraphCycle cycle;

                cycle.push_front(g->getActor(a));
                cycles.insert(cycle);                
            }
        }
    }

    return cycles;    
}

/**
 * simpleCycleVisit ()
 * The visitor function of the DFS based algorithm used for detecting 
 * simple cycles.
 */
static
void simpleCycleVisit(uint a, SDFgraph *g, v_uint &nodeId, v_uint &actorId, 
        vector<v_uint> &graph, uint size, v_int &color, v_uint &pi,
        SDFgraphCycle &cycle)
{
    // color[a] <- gray
    color[a] = 1;

    // for each b in Adj(e)
    v_uint adj = getAdjacentActors(a, nodeId, actorId, graph, size);
    for (uint i = 0; i < adj.size(); i++)
    {
        uint b = adj[i];
        
        // do if color[b] = white
        if (color[b] == 0)
        {
            pi[b] = a;
            simpleCycleVisit(b, g, nodeId, actorId, graph, size, color, pi,
                                cycle);
        }
        else if (color[b] == 1)
        {
            // Create new cycle
            cycle.push_front(g->getActor(a));

            uint u = a;
            while (u != b)
            {
                u = pi[u];
                cycle.push_front(g->getActor(u));
            }
        }

        // Found a cycle?
        if (cycle.size() != 0)
            break;
    }
    
    // color[a] <- white
    color[a] = 0;
}

/**
 * findSimpleCycle ()
 * The function performs a depth first search on the graph to discover a
 * simple cycles in the graph. The first cycle found is returned.
 */
SDFgraphCycle findSimpleCycle(SDFgraph *g, bool transpose)
{
    SDFgraphCycle cycle;
    v_uint pi(g->nrActors());

    // Split graph in strongly connected components
    SDFgraphComponents components = stronglyConnectedComponents(g);

    // Iterate over all components
    for (SDFgraphComponentsIter iter = components.begin();
            iter != components.end(); iter++)
    {
        v_uint actorId, nodeId;
        vector<v_uint> graph;
        SDFgraphComponent &component = *iter;
       
        // Convert component to graph structure
        sdfComponentToGraph(g, component, nodeId, actorId, graph, transpose);

        // For each a in A' do color[a] <- white
        v_int color(g->nrActors(), 0);

        // For each a in A' do pi[a] <- NIL
        for (uint a = 0; a < g->nrActors(); a++)
            pi[a] = 0;

        // For an actor a in A' (take the first actor)
        uint a = actorId[0];
    
        // color[a] <- gray
        color[a] = 1;
        
        // For each actor b in Adj(a, s)
        v_uint adj = getAdjacentActors(a, nodeId, actorId, graph,
                                            component.size());
        for (uint i = 0; i < adj.size(); i++)
        {
            uint b = adj[i];
            
            if (color[b] == 0)
            {
                pi[b] = a;
                simpleCycleVisit(b, g, nodeId, actorId, graph, component.size(),
                                    color, pi, cycle);
            }
            else if (color[b] == 1)
            {
                // Self-loop
                cycle.push_front(g->getActor(a));
            }
            
            // Found a cycle?
            if (cycle.size() != 0)
                return cycle;
        }
    }

    return cycle;    
}

#endif // __FIND_CYCLE_USING_SDFGRAPH__
