/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mcm.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Maximum cycle mean of a HSDF graph
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: mcm.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "mcm.h"
#include "../../base/hsdf/check.h"
#include "../../base/algo/components.h"
#include "../../base/algo/cycle.h"
#include "../../base/algo/graph.h"

#define MIN(a,b)    ((a)<(b) ? (a) : (b))
#define MAX(a,b)    ((a)>(b) ? (a) : (b))

//#define __CALC_MCM_PER_CYCLE__

/**
 * maximumCycleMeanCycle ()
 * The function computes the maximum cycle mean of a cycle.
 */
CFraction maximumCycleMeanCycle(TimedSDFgraph *g, SDFgraphCycle &cycle)
{
    SDFtime t = 0;
    uint tokens = 0;
    SDFactorsIter cycleIter = cycle.begin();
    
    while (cycleIter != cycle.end())
    {
        TimedSDFactor *u = (TimedSDFactor*)(*cycleIter);
        TimedSDFactor *v = (TimedSDFactor*)(*(++cycleIter));
        
        // Add execution time of actor to execution time of the cycle
        t += u->getExecutionTime();
        
        if (cycleIter == cycle.end())
        {
            // Next actor in cycle is the first actor            
            v = (TimedSDFactor*)(*cycle.begin());
        }

        // Find channel with minimum number of tokens to next actor in cycle
        int tokensCh = 0;
        bool firstPort = true;
        for (SDFportsIter iterP = u->portsBegin(); 
                iterP != u->portsEnd(); iterP++)
        {
            SDFport *p = *iterP;
            
            // Is port an input port?
            if (p->getType() == SDFport::In)
                continue;
            
            // Connects port to next actor?
            if (p->getChannel()->getDstActor()->getId() == v->getId())
            {
                if (firstPort)
                    tokensCh = p->getChannel()->getInitialTokens();
                else
                    tokensCh = MIN(tokensCh, 
                                (int)p->getChannel()->getInitialTokens());
                
                firstPort = false;
            }
        }
        
        // Add tokens found between u and v to tokens on cycle
        tokens += tokensCh;
    }
    
    if (tokens == 0)
        return CFraction(INT_MAX, 1);
        
    return CFraction(t, tokens);
}

#ifndef __CALC_MCM_PER_CYCLE__

/**
 * mcmGetAdjacentActors ()
 * The function returns a list with actors directly reachable from
 * actor a.
 */
static
v_uint mcmGetAdjacentActors(uint a, v_uint &nodeId, v_uint &actorId,
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
 * mcmSimpleCycleVisit ()
 * The visitor function of the DFS based algorithm used for detecting 
 * simple cycles and computing the MCM of them.
 */
static
void mcmSimpleCycleVisit(uint a, TimedSDFgraph *g, v_uint &nodeId,
        v_uint &actorId, vector<v_uint> &graph, uint size, v_int &color,
        v_uint &pi, CFraction &mcmCycles)
{
    // color[a] <- gray
    color[a] = 1;

    // for each b in Adj(e)
    v_uint adj = mcmGetAdjacentActors(a, nodeId, actorId, graph, size);
    for (uint i = 0; i < adj.size(); i++)
    {
        uint b = adj[i];
                
        // do if color[b] = white
        if (color[b] == 0)
        {
            pi[b] = a;
            mcmSimpleCycleVisit(b, g, nodeId, actorId, graph, size,
                                            color, pi, mcmCycles);
        }
        else if (color[b] == 1)
        {
            CFraction mcmCycle;
            SDFgraphCycle cycle;
            
            // Create new cycle
            cycle.push_front(g->getActor(a));

            uint u = a;
            while (u != b)
            {
                u = pi[u];
                cycle.push_front(g->getActor(u));
            }

            // MCM of the cycle
            mcmCycle = maximumCycleMeanCycle(g, cycle);

            // Maximum of all cycles
            mcmCycles = MAX(mcmCycles, mcmCycle);
        }
    }
    
    // color[a] <- white
    color[a] = 0;
}

/**
 * mcmSimpleCycles ()
 * The function performs a depth first search on the graph to discover all
 * simple cycles in the graph. The cycle mean of each cycle is computed
 * and the maximum cycle mean is returned.
 */
static
CFraction mcmSimpleCycles(TimedSDFgraph *g, bool transpose)
{
    CFraction mcmGraph = 0;
    v_uint pi(g->nrActors());

    // Split graph in strongly connected components
    SDFgraphComponents components = stronglyConnectedComponents(g);

    if (components.size() == 0)
        return INT_MAX;

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
        v_uint adj = mcmGetAdjacentActors(a, nodeId, actorId, graph,
                                            component.size());
        for (uint i = 0; i < adj.size(); i++)
        {
            uint b = adj[i];
            
            if (color[b] == 0)
            {
                pi[b] = a;
                mcmSimpleCycleVisit(b, g, nodeId, actorId, graph,
                                    component.size(), color, pi, mcmGraph);
            }
            else if (color[b] == 1)
            {
                CFraction mcmCycle;
                SDFgraphCycle cycle;

                // Self-loop
                cycle.push_front(g->getActor(a));

                // MCM of the cycle
                mcmCycle = maximumCycleMeanCycle(g, cycle);

                // Graph is maximum of all cycles
                mcmGraph = MAX(mcmGraph, mcmCycle);
            }
        }
    }

    return mcmGraph;    
}

/**
 * maximumCycleMeanCycles ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * the cycle-based definition of the MCM.
 */
CFraction maximumCycleMeanCycles(TimedSDFgraph *g)
{
    CFraction mcmGraph;
    
    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g))
        throw CException("Graph is not an HSDF graph.");    

    // Check that the graph g is a strongly connected graph
    //if (!isStronglyConnectedGraph(g))
    //    throw CException("Graph is not strongly connected.");

    // Find simple cycles in graph and calculate MCM for each cycle
    mcmGraph = mcmSimpleCycles(g, false);
    
    return mcmGraph;
}

#else // __CALC_MCM_PER_CYCLE__

/**
 * maximumCycleMeanCycles ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * the cycle-based definition of the MCM.
 */
CFraction maximumCycleMeanCycles(TimedSDFgraph *g)
{
    CFraction mcmGraph = 0;
    
    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g))
        throw CException("Graph is not an HSDF graph.");    

    // Check that the graph g is a strongly connected graph
    //if (!isStronglyConnectedGraph(g))
    //    throw CException("Graph is not strongly connected.");

    // Find simple cycles in graph
    SDFgraphCycles cycles = findSimpleCycles(g);
    
    // Calculate MCM for each cycle
    for (SDFgraphCyclesIter iter = cycles.begin(); iter != cycles.end(); iter++)
    {
        CFraction mcmCycle;
        SDFgraphCycle cycle = *iter;
        
        // MCM of the cycle
        mcmCycle = maximumCycleMeanCycle(g, cycle);
        
        // Graph is maximum of all cycles
        mcmGraph = MAX(mcmGraph, mcmCycle);
    }
    
    return mcmGraph;
}

#endif // __CALC_MCM_PER_CYCLE__
