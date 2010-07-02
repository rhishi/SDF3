/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
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
 * $Id: graph.cc,v 1.2 2008/09/18 07:38:21 sander Exp $
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

#include "graph.h"

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
        vector<v_uint> &graph, const bool transpose)
{
    // Init vectors
    nodeId.resize(g->nrActors());
    actorId.resize(g->nrActors());
    graph.resize(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++)
        graph[i].resize(g->nrActors());
    
    // Initialize id's
    uint nId = 0;
    for (SDFactorsIter iter = g->actorsBegin(); 
            iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        // Mappings
        nodeId[a->getId()] = nId;
        actorId[nId] = a->getId();
        
        // Next
        nId++;
    }
    
    // Initialize graph
    for (uint u = 0; u < g->nrActors(); u++)    
        for (uint v = 0; v < g->nrActors(); v++)
            graph[u][v] = 0;
    
    // graph[u][v] <- 1 for each channel (u,v) in C for which u,v in S
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *ch = *iter;
        
        uint u = nodeId[ch->getSrcActor()->getId()];
        uint v = nodeId[ch->getDstActor()->getId()];

        if (!transpose)
            graph[u][v] = 1;
        else
            graph[v][u] = 1;
    }
}

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
        const bool transpose)
{
    // Init vectors
    nodeId.resize(g->nrActors());
    actorId.resize(component.size());
    graph.resize(component.size());
    for (uint i = 0; i < component.size(); i++)
        graph[i].resize(component.size());
    
    // Initialize id's
    uint nId = 0;
    for (SDFactorsIter iter = component.begin(); 
            iter != component.end(); iter++)
    {
        SDFactor *a = *iter;
        
        // Mappings
        nodeId[a->getId()] = nId;
        actorId[nId] = a->getId();
        
        // Next
        nId++;
    }
    
    // Initialize graph
    for (uint u = 0; u < component.size(); u++)    
        for (uint v = 0; v < component.size(); v++)
            graph[u][v] = 0;
    
    // graph[u][v] <- 1 for each channel (u,v) in C for which u,v in S
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *ch = *iter;
        
        if (actorInComponent(ch->getSrcActor(), component)
                && actorInComponent(ch->getDstActor(), component))
        {
            uint u = nodeId[ch->getSrcActor()->getId()];
            uint v = nodeId[ch->getDstActor()->getId()];
            
            if (!transpose)
                graph[u][v] = 1;
            else
                graph[v][u] = 1;
        }
    }
}
