/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mcmgraph.cc
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
 * $Id: mcmgraph.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "base/base.h"
#include "mcmgraph.h"
#include "../../base/hsdf/check.h"

/**
 * ~MCMgraph
 * Constructor.
 */
MCMgraph::MCMgraph()
{
}

/**
 * ~MCMgraph
 * Destructor.
 */
MCMgraph::~MCMgraph()
{
    for (MCMnodesIter iter = nodes.begin(); iter != nodes.end(); iter++)
        delete (*iter);

    for (MCMedgesIter iter = edges.begin(); iter != edges.end(); iter++)
        delete (*iter);
}

/**
 * createInitialMCMgraph ()
 * The function converts an HSDF graph to a weighted directed graph
 * by replacing each actor in the HSDF with a node in the graph and each
 * channel with an edge. Each edge is assigned a weight equal to the
 * execution time of the destination actor of the corresponding channel.
 * The initial tokens are preserved during the conversion from channels to
 * edges.
 */
static
void createInitialMCMgraph(TimedSDFgraph *g, MCMgraph *mcmGraph)
{
    for (SDFactorsIter iter = g->actorsBegin(); 
            iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        MCMnode *n = new MCMnode;
        
        n->id = a->getId();
        n->visible = true;
        mcmGraph->nodes.push_back(n);
        
    }
    
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        MCMedge *e = new MCMedge;
        
        e->id = c->getId();
        e->visible = true;
        e->d = c->getInitialTokens();
        e->w = ((TimedSDFactor*)(c->getSrcActor()))->getExecutionTime();
        
        ASSERT(e->w > 0, "Execution time must be > 0.")
        
        e->src = NULL;
        e->dst = NULL;
        for (MCMnodesIter iterN = mcmGraph->nodes.begin();
                iterN != mcmGraph->nodes.end(); iterN++)
        {
            MCMnode *n = *iterN;
            
            if (n->id == c->getSrcActor()->getId())
            {
                e->src = n;
                n->out.push_back(e);
            }
            if (n->id == c->getDstActor()->getId())
            {
                e->dst = n;
                n->in.push_back(e);
            }
            
            if (e->src != NULL && e->dst != NULL)
                break;
        }
        ASSERT(e->dst != NULL, "No dst node found.");
        ASSERT(e->src != NULL, "No src node found.");
        
        mcmGraph->edges.push_back(e);
    }
}

/**
 * splitMCMedgeToSequence ()
 * The function converts an MCM edge with more then one delay
 * into a sequence of edges with one delay (uses recursive call
 * to itself).
 */
static
void splitMCMedgeToSequence(MCMgraph *g, MCMedge *e)
{
    // Create dummy node n;
    MCMnode *n = new MCMnode;
    n->id = g->nodes.size();
    n->visible = true;
    g->nodes.push_back(n);

    // Create a new edge between the src node of e and a new
    // dummy node.
    MCMedge *eN = new MCMedge;
    eN->id = g->edges.size();
    eN->visible = true;
    eN->src = e->src;
    eN->dst = n;
    eN->w = 0;
    eN->d = e->d - 1;
    g->edges.push_back(eN);

    // Remove e from the set of edges its source node is connected to
    // and add eN to this list
    for (MCMedgesIter iter = e->src->out.begin();
            iter != e->src->out.end(); iter++)
    {
        if ((*iter)->id == e->id)
        {
            e->src->out.erase(iter);
            break;
        }
    }
    e->src->out.push_back(eN);
    
    // Connect e to node n
    e->src = n;
    n->out.push_back(e);
    n->in.push_back(eN);
    
    // One delay left on e
    e->d = 1;

    // More then one delay on the new edge?
    if (eN->d > 1)
        splitMCMedgeToSequence(g, eN);
}

/**
 * addLongestDelayEdgeForNode ()
 * The function add an additional edge between the node n and any node
 * reachable over e which only uses edges with no initial delays (except
 * for the initial edge e). The weight of the new edge is the longest
 * path from the node n to the destination node.
 * The function uses an adapted version of Dijkstra's shirtest path
 * algorithm.
 */
static
void addLongestDelayEdgeForNode(MCMgraph *g, MCMnode *n, MCMedge *e,
        int **visit)
{
    int dMax;
    int *d = new int [g->nodes.size()];
    MCMnode **pi = new MCMnode* [g->nodes.size()];
    MCMnodes S;
    MCMnodes Q = g->nodes;

    // Initialize single source
    for (uint v = 0; v < g->nodes.size(); v++)
    {
        d[v] = -1;
        pi[v] = NULL;
    }
    d[n->id] = 0;

    // Initialize the node connected via the edge e to n
    d[e->dst->id] = e->w;
    
    // Remove node n from Q and add it to S (only when edge e is no
    // self-edge)
    if (e->dst->id != n->id)
    {
        for (MCMnodesIter iter = Q.begin(); iter != Q.end(); iter++)
        {
            if ((*iter)->id == n->id)
            {
                // Is edge e
                Q.erase(iter);
                break;
            }
        }
        S.push_back(n);
    }
    
    // Find all longest paths till Q is empty or all reachable paths seen    
    while (!Q.empty())
    {
        MCMnode *u = NULL;
        
        // Find node u in Q with largest distance
        dMax = -1;
        for (MCMnodesIter iter = Q.begin(); iter != Q.end(); iter++)
        {
            if (d[(*iter)->id] > dMax)
            {
                u = *iter;
                dMax = d[u->id];
            }
        }
        
        // Found no node with a non-negative distance ?
        if (dMax < 0)
            break;
        
        // Remove node u from Q and add it to S
        for (MCMnodesIter iter = Q.begin(); iter != Q.end(); iter++)
        {
            if ((*iter)->id == u->id)
            {
                Q.erase(iter);
                break;
            }
        }
        S.push_back(u);

#if __VISITED_NODES__
        // Visited node u before?
        if (visit[u->id] != NULL)
        {
            for (MCMnodesIter iter = g->nodes.begin();
                    iter != g->nodes.end(); iter++)
            {
                MCMnode *v = *iter;
                
                // Node v reachable from u in the past?
                if (visit[u->id][v->id] > visit[u->id][u->id])
                {
                    // Distance to v is distance to v in the past minus
                    // the distance to u in the past plus the distance
                    // to u in the current situation.
                    d[v->id] = visit[u->id][v->id] - visit[u->id][u->id] 
                                    + d[u->id];
                    S.push_back(v);
                }
            }
        }
#endif

        // Relax all nodes v adjacent to u (connected via edges with no tokens)
        for (MCMedgesIter iter = u->out.begin();
                iter != u->out.end(); iter++)
        {
            MCMedge *e = *iter;
            MCMnode *v = e->dst;
            
            if (e->d == 0 && d[v->id] < d[u->id] + e->w)
            {
                if (d[v->id] != -1)
                {
                    // Found a longer path to v, add it to list of
                    // nodes to be checked (again).
                    Q.push_back(v);
                }
                
                d[v->id] = d[u->id] + e->w;
                pi[v->id] = u;
            }
        }
    }

#if __VISITED_NODES__
    // Store the distance for next runs of the longest path function
    // Skip first node on longest path (this node may have outgoing edges
    // not explored in this run - i.e. algo start with specific edge)
    MCMnode *v = S.back();
    while (v != NULL && pi[v->id] != NULL)
    {
        visit[v->id] = d;
        v = pi[v->id];
    }
#endif

    // Add an edge between the node n and any node m reachable from n
    // with a weight equal to the longest path from n to m
    for (MCMnodesIter iter = S.begin(); iter != S.end(); iter++)
    {
        MCMnode *m = *iter;

        // Node m reachable from n and not connected directly to n via e?
        if (d[m->id] > 0 && e->dst->id != m->id)
        {
            // Create an edge between n and m
            MCMedge *eN = new MCMedge;
            eN->id = g->edges.size();
            eN->visible = false;
            eN->src = n;
            eN->dst = m;
            eN->w = d[m->id];
            eN->d = e->d;           // This should always be 1
            n->out.push_back(eN);
            m->in.push_back(eN);
            g->edges.push_back(eN);
        }
    }
}

/**
 * addLongestDelayEdgesToMCMgraph ()
 * The function adds additional edges to the graph which express the
 * longest path between two nodes crossing one edge with a delay. Edges
 * with no delay are removed and edges with more then one delay element
 * are converted into a sequence of edges with one delay element.
 */
static
void addLongestDelayEdgesToMCMgraph(MCMgraph *g)
{
    int **visit;

    // Find longest path between a node n and a node m
    // over all sequences of edges in which only the
    // first edge may contain a delay
    for (MCMedgesIter iter = g->edges.begin();
            iter != g->edges.end(); iter++)
    {
        MCMedge *e = *iter;

        // More then one delay on the edge?
        if (e->d > 1)
            splitMCMedgeToSequence(g, e);
    }

    // Initialize visit array which contains distances of already
    // computed paths
    visit = new int* [g->nodes.size()];
    for (uint i = 0; i < g->nodes.size(); i++)
        visit[i] = NULL;

    // Find longest path between a node n and a node m
    // over all sequences of edges in which only the
    // first edge may contain a delay
    for (MCMedgesIter iter = g->edges.begin();
            iter != g->edges.end(); iter++)
    {
        MCMedge *e = *iter;

        // Initial tokens on edge?
        if (e->d != 0 && e->visible)
        {
            // Find the longest path from n to any node m
            // and add an edge with the path wait to the graph
            addLongestDelayEdgeForNode(g, e->src, e, visit);

            // Seen this edge (set it to invisible)
            e->visible = false;
        }
    }
    
    // Hide all edges which do not contain a delay
    for (MCMedgesIter iter = g->edges.begin();
            iter != g->edges.end(); iter++)
    {
        MCMedge *e = *iter;
        
        // No initial tokens on edge?
        if (e->d == 0)
            e->visible = false;
        else
            e->visible = true;
    }
}

/**
 * transformHSDFtoMCMgraph ()
 * The function converts an HSDF graph to a weighted directed graph
 * used in the MCM algorithm of Karp (and its variants). By default, the
 * a longest path calculation is performed to make the graph suitable
 * as input for an MCM algorithm. Setting the flag 'mcmFormulation' to false
 * result in an MCM graph which is suitable for an MCR (cycle ratio)
 * formulation. This avoids running the longest path algo.
 */
MCMgraph *transformHSDFtoMCMgraph(TimedSDFgraph *g, bool mcmFormulation)
{
    MCMgraph *mcmGraph = new MCMgraph;
    
    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g))
        throw CException("Graph is not an HSDF graph.");    

    // Create an initial directed weighted graph
    createInitialMCMgraph(g, mcmGraph);
    
    // Convert the initial directed weighted graph to the required form
    if (mcmFormulation)
        addLongestDelayEdgesToMCMgraph(mcmGraph);

#if 0    
    cerr << "edge: (u, v, w, d)" << endl;
    for (MCMedgesIter iter = mcmGraph->edges.begin();
            iter != mcmGraph->edges.end(); iter++)
    {
        MCMedge *e = *iter;
        
        if (!e->visible)
            continue;

        cerr << "(" << e->src->id;
        cerr << ", " << e->dst->id;
        cerr << ", " << e->w;
        cerr << ", " << e->d;
        cerr << ")" << endl;
    }
    cerr << endl;
#endif
 
    return mcmGraph;
}

/**
 * getAdjacentNodes ()
 * The function returns a list with nodes directly reachable from
 * node a (in case transpose if false). If transpose is 'true', the
 * graph is transposed and the function returns a list with nodes
 * which are directly reachable from a in the transposed graph.
 */
static
MCMnodes getAdjacentNodes(MCMnode *a, bool transpose)
{
    MCMnodes nodes;
    
    if (!transpose)
    {
        for (MCMedgesIter iter = a->out.begin(); iter != a->out.end(); iter++)
        {
            MCMedge *e = *iter;

            if (e->visible)
                nodes.push_back(e->dst);
        }
    }
    else
    {
        for (MCMedgesIter iter = a->in.begin(); iter != a->in.end(); iter++)
        {
            MCMedge *e = *iter;

            if (e->visible)
                nodes.push_back(e->src);
        }
    }

    return nodes;
}

/**
 * getNextNode ()
 * The function returns the node from the list of nodes with the highest
 * order. Its order is set to -1. If all nodes have order -1, a NULL pointer is
 * returned.
 */
static
MCMnode *getNextNode(MCMnodes &nodes, v_int &order)
{
    MCMnode *a = NULL;
    int orderA = -1;
        
    // Find actor with largest order
    for (MCMnodesIter iter = nodes.begin(); iter != nodes.end(); iter++)
    {
        MCMnode *b = *iter;
        
        if (orderA < order[b->id])
        {
            a = b;
            orderA = order[b->id];
        }
    }
    
    // All actors have order -1?
    if (orderA == -1)
        return NULL;
    
    return a;
}

/**
 * dfsVisit ()
 * The visitor function of the DFS algorithm.
 */
static
void dfsVisit(MCMnode *u, int &time, v_int &color, v_int &d, v_int &f, 
        MCMnode **pi, bool transpose)
{    
    // color[u] <- gray
    color[u->id] = 1;

    // d[u] <- time <- time + 1
    time++;
    d[u->id] = time;

    // for each v in Adj(e)
    MCMnodes adj = getAdjacentNodes(u, transpose);
    for (MCMnodesIter iter = adj.begin(); iter != adj.end(); iter++)
    {
        MCMnode *v = *iter;
        
        // do if color[v] = white
        if (color[v->id] == 0)
        {
            pi[v->id] = u;
            dfsVisit(v, time, color, d, f, pi, transpose);
        }
    }
    
    // color[u] <- black
    color[u->id] = 2;

    // f[u] <- time <- time + 1
    time++;
    f[u->id] = time;
}

/**
 * dfsMCMgraph ()
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
void dfsMCMgraph(MCMgraph *g, v_int &d, v_int &f, MCMnode **pi, bool transpose)
{
    int time;
    
    // for each u in G do order[u] <- f[u]
    v_int order(f);

    // for each u in G do color[u] <- white
    v_int color(g->nrNodes(), 0);

    // time <- 0
    time = 0;
    
    // for each u in G do pi[u] <- NIL
    for (uint u = 0; u < g->nrNodes(); u++)
        pi[u] = NULL;

    // for each u in G (visit in order given by order)
    for (MCMnode *a = getNextNode(g->nodes, order); a != NULL; 
            a = getNextNode(g->nodes, order))
    {
        // Mark node as visited
        order[a->id] = -1;

        // if color[u] = white
        if (color[a->id] == 0)
            dfsVisit(a, time, color, d, f, pi, transpose);
    }
}

/**
 * addNodeToComponent ()
 * The function adds a new copy of a node to the component and it
 * also creates copies for all edges between this node and all nodes
 * already in the component.
 */
static
void addNodeToComponent(MCMnode *n, MCMgraph *comp)
{
    MCMnode *m;
    
    // Create a copy of n and add it to the component
    m = new MCMnode;
    m->id = n->id;
    m->visible = true;
    comp->nodes.push_back(m);
    
    // Check all edges of n for inclusion in the component
    for (MCMedgesIter iter = n->out.begin(); iter != n->out.end(); iter++)
    {
        MCMedge *e = *iter;
        
        if (!e->visible)
            continue;
        
        // Is destination node in the component?
        for (MCMnodesIter iterN = comp->nodes.begin();
                iterN != comp->nodes.end(); iterN++)
        {
            if (e->dst->id == (*iterN)->id)
            {
                // Add a copy of the edge to the component
                MCMedge *eN = new MCMedge;
                eN->id = e->id;
                eN->visible = e->visible;
                eN->d = e->d;
                eN->w = e->w;
                eN->src = m;
                eN->dst = *iterN;
                comp->edges.push_back(eN);
                m->out.push_back(eN);
                (*iterN)->in.push_back(eN);
                break;
            }
        }
    }
    for (MCMedgesIter iter = n->in.begin(); iter != n->in.end(); iter++)
    {
        MCMedge *e = *iter;
        
        // Is source node in the component?
        for (MCMnodesIter iterN = comp->nodes.begin();
                iterN != comp->nodes.end(); iterN++)
        {
            if (e->src->id == (*iterN)->id)
            {
                // Add a copy of the edge to the component
                MCMedge *eN = new MCMedge;
                eN->id = e->id;
                eN->visible = e->visible;
                eN->d = e->d;
                eN->w = e->w;
                eN->src = *iterN;
                eN->dst = m;
                comp->edges.push_back(eN);
                m->in.push_back(eN);
                (*iterN)->out.push_back(eN);
                break;
            }
        }
    }  
}

/**
 * treeVisitChildren ()
 * The function visits all children of the actor 'u'. The parent-child
 * relation is given via the vector 'pi'.
 */
static
bool treeVisitChildren(MCMgraph *g, MCMnode **pi, MCMnode *u, MCMgraph *comp)
{
    bool children = false;
    
    for (uint i = 0; i < g->nodes.size(); i++)
    {
        // Node v points to node u (i.e. v is a child of u)?
        if (pi[i] != NULL && pi[i]->id == u->id)
        {
            MCMnode *v = NULL;
            
            for (MCMnodesIter iter = g->nodes.begin();
                    iter != g->nodes.end(); iter++)
            {
                if ((*iter)->id == i)
                {
                    v = *iter;
                    break;
                }
            }
            ASSERT(v != NULL, "The must always be a node v.");
            
            // Add node v to the component
            v->visible = true;
            addNodeToComponent(v, comp);
            children = true;
            
            // Find all children of v in the tree
            treeVisitChildren(g, pi, v, comp);
        }
    }
    
    return children;
}

/**
 * findComponentsInMCMgraph ()
 * The function determines the strongly connected components in the graph. To do
 * this, it performs depth-first walk on the forest given by 'pi'.
 */
static
void findComponentsInMCMgraph(MCMgraph *g, MCMnode **pi, MCMgraphs &components)
{
    MCMgraph *comp;

    // Set all node as invisible
    for (MCMnodesIter iter = g->nodes.begin(); iter != g->nodes.end(); iter++)
    {
        MCMnode *n = *iter;

        n->visible = false;
    }
    
    // Find the strongly connected component and make all of its nodes visible
    for (MCMnodesIter iter = g->nodes.begin(); iter != g->nodes.end(); iter++)
    {
        MCMnode *n = *iter;
        
        if (pi[n->id] == NULL)
        {
            // Create a new graph for the component
            comp = new MCMgraph;
            
            // Find all children of n in the tree
            if (treeVisitChildren(g, pi, n, comp))
            {
                // Node n has children, so it belongs to the strongly
                // connected component
                n->visible = true;
                addNodeToComponent(n, comp);
            }
            else
            {
                // Node n may have a self-loop making it a strongly
                // connected component
                for (MCMedgesIter iterE = n->in.begin();
                        iterE != n->in.end(); iterE++)
                {
                    // Is edge a self-loop?
                    if ((*iterE)->src->id == n->id)
                    {
                        n->visible = true;
                        addNodeToComponent(n, comp);
                    }
                }
            }
            
            // Found a strongly connected component (at least one edge)?
            if (comp->nrEdges() > 0)
                components.push_back(comp);
            else
                delete comp;
        }
    }
    
    // Make all edges to invisible nodes also invisible
    for (MCMedgesIter iter = g->edges.begin(); iter != g->edges.end(); iter++)
    {
        MCMedge *e = *iter;
        
        if (e->visible)
            if (!e->src->visible || !e->dst->visible)
                e->visible = false;
    }
}

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
void stronglyConnectedMCMgraph(MCMgraph *g, MCMgraphs &components)
{
    // Initialize
    v_int d(g->nrNodes());
    v_int f(g->nrNodes(), 0);
    MCMnode **pi = (MCMnode**)malloc(sizeof(MCMnode*)*g->nrNodes());
    ASSERT (pi != NULL, "Failled malloc");

    // Call dfs(g) ti compute f[u] for each u
    dfsMCMgraph(g, d, f, pi, false);

    // Call dfs(gT), consider the vertices in order of decreasing f[u]
    dfsMCMgraph(g, d, f, pi, true);

    // Component is given by the vertices of the tree in the depth-first forest
    findComponentsInMCMgraph(g, pi, components);

#if 0
    // Print the MCM graph in dot-format
    cerr << "digraph g {" << endl;
    cerr << "    size=\"7,10\";" << endl;
    for (MCMedgesIter iter = g->edges.begin(); iter != g->edges.end(); iter++)
    {
        MCMedge *e = *iter;

        if (e->visible)
        {
            cerr << "    " << e->src->id << " -> " << e->dst->id;
            cerr << " [ label=" << e->w << " ];" << endl;
        }
    }
    cerr << "}" << endl;
#endif
        
    // Cleanup
    free(pi);
}

/**
 * relabelMCMgraph ()
 * The function removes all hidden nodes and edges from the graph. All visible
 * edges are assigned a new id starting in the range [0,nrNodes()).
 */
void relabelMCMgraph(MCMgraph *g)
{
    uint nodeId = 0, edgeId = 0;

    // Relabel nodes
    for (MCMnodesIter iter = g->nodes.begin(); iter != g->nodes.end(); iter++)
    {
        MCMnode *n = *iter;
        
        if (n->visible)
        {
            n->id = nodeId;
            nodeId++;
        }
    }
    
    // Relabel edges
    for (MCMedgesIter iter = g->edges.begin(); iter != g->edges.end(); iter++)
    {
        MCMedge *e = *iter;
        
        if (e->visible)
        {
            e->id = edgeId;
            edgeId++;
        }
    }
    
    // Remove edges from nodes
    for (MCMnodesIter iter = g->nodes.begin(); iter != g->nodes.end(); iter++)
    {
        MCMnode *n = *iter;

        if (!n->visible)
            continue;
        
        for (MCMedgesIter iterE = n->in.begin(); iterE != n->in.end();)
        {
            MCMedgesIter iterEN = iterE;

            // Next iterator
            iterE++;
            
            // Erase current iterator?
            if (!(*iterEN)->visible)
                n->in.erase(iterEN);
        }
        
        for (MCMedgesIter iterE = n->out.begin(); iterE != n->out.end();)
        {
            MCMedgesIter iterEN = iterE;

            // Next iterator
            iterE++;
            
            // Erase current iterator?
            if (!(*iterEN)->visible)
                n->out.erase(iterEN);
        }
    }    

    // Remove nodes from graph
    for (MCMnodesIter iter = g->nodes.begin(); iter != g->nodes.end();)
    {
        MCMnodesIter iterN = iter;
        
        // Next iterator
        iter++;
        
        if (!(*iterN)->visible)
            g->nodes.erase(iterN);
    }
    
    // Remove edges from graph
    for (MCMedgesIter iter = g->edges.begin(); iter != g->edges.end();)
    {
        MCMedgesIter iterE = iter;
        
        // Next iterator
        iter++;
        
        if (!(*iterE)->visible)
            g->edges.erase(iterE);
    }
}
