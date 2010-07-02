/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mcmhoward.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 8, 2005
 *
 *  Function        :   Compute the MCM for an HSDF graph using Dasdan-Gupta's
 *                      algorithm.
 *
 *  History         :
 *      08-11-05    :   Initial version.
 *
 * $Id: mcmdg.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "mcmgraph.h"
#include "../../base/hsdf/check.h"
#include "../../base/algo/components.h"

#define MIN(a,b)    ((a)<(b) ? (a) : (b))
#define MAX(a,b)    ((a)>(b) ? (a) : (b))

/**
 * mcmDG ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * Dasdan-Gupta's algorithm.
 */
static
double mcmDG(MCMgraph *mcmGraph)
{
    int k, n;
    int *level;
    int **pi, **d;
    double l, ld;
    MCMnode *u;
    list<int> Q_k;
    list<MCMnode*> Q_u;

    // Allocate memory
    n = mcmGraph->nrNodes();
    level = new int [n];
    pi = new int* [n+1];
    d = new int* [n+1];
    for (int i = 0; i < n+1; i++)
    {
        pi[i] = new int [n];
        d[i] = new int [n];
    }

    // Initialize
    for (int i = 0; i < n; i++)
        level[i] = -1;
    d[0][0] = 0;
    pi[0][0] = -1;
    level[0] = 0;
    Q_k.push_back(0);
    Q_u.push_back(mcmGraph->nodes.front());

    // Compute the distances
    k = Q_k.front(); Q_k.pop_front();
    u = Q_u.front(); Q_u.pop_front();
    do
    {
        for (MCMedgesIter iter = u->out.begin(); iter != u->out.end(); iter++)
        {
            MCMedge *e = *iter;
            MCMnode *v = e->dst;

            if (level[v->id] < k+1)
            {
                Q_k.push_back(k+1);
                Q_u.push_back(v);
                pi[k+1][v->id] = level[v->id];
                level[v->id] = k + 1;
                d[k+1][v->id] = -INT_MAX;
            }
            d[k+1][v->id] = MAX(d[k+1][v->id],d[k][u->id]+e->w);
        }
        k = Q_k.front(); Q_k.pop_front();
        u = Q_u.front(); Q_u.pop_front();
    } while (k < n);

    // Compute lamda using Karp's theorem
    l = -INT_MAX;
    for (MCMnodesIter iter = mcmGraph->nodes.begin();
            iter != mcmGraph->nodes.end(); iter++)
    {
        u = *iter;

        if (level[u->id] == n)
        {
            ld = INT_MAX;
            k = pi[n][u->id];
            while (k > -1)
            {
                ld = MIN(ld, (double)(d[n][u->id]-d[k][u->id]) / (double)(n-k));
                k = pi[k][u->id];
            }
            l = MAX(l, ld);
        }
    }

    // Cleanup
    delete [] level;  
    for (int i = 0; i < n+1; i++)
    {
        delete [] pi[i];
        delete [] d[i];
    }
    delete [] pi;
    delete [] d;
    
    return l;
}

/**
 * mcmDasdanGupta ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * Dasdan-Gupta's algorithm.
 */
static
CFraction mcmDasdanGupta(TimedSDFgraph *g)
{
    MCMgraphs components;
    MCMgraph *mcmGraph;
    double mcm = 0, mcmComp;

    // Transform the HSDF to a weighted directed graph
    mcmGraph = transformHSDFtoMCMgraph(g);

    // Extract the strongly connected component from the graph
    // According to the Max-Plus book there is exactly one strongly
    // connected component in the graph when starting from a strongly
    // connected (H)SDF graph.
    stronglyConnectedMCMgraph(mcmGraph, components);

    // Iterate over all components to find maximum MCM
    for (MCMgraphsIter iter = components.begin();
            iter != components.end(); iter++)
    {
        MCMgraph *comp = *iter;

        // Remove all hidden edges and nodes from the graph and assign new id's
        relabelMCMgraph(comp);

        // Compute MCM of the component
        mcmComp = mcmDG(comp);

        if (mcmComp > mcm)
            mcm = mcmComp;
    }

    // Cleanup
    delete mcmGraph;
    
    return CFraction(mcm);
}

/**
 * maximumCycleMeanDasdanGupta ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * Dasan-Gupta's algorithm.
 */
CFraction maximumCycleMeanDasdanGupta(TimedSDFgraph *g)
{
    CFraction mcmGraph;
    
    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g))
        throw CException("Graph is not an HSDF graph.");    

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(g))
        throw CException("Graph is not strongly connected.");
        
    mcmGraph = mcmDasdanGupta(g);
    
    return mcmGraph;
}

