/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   apg.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 2, 2005
 *
 *  Function        :   Acyclic precedence graph
 *
 *  History         :
 *      02-08-05    :   Initial version.
 *
 * $Id: apg.h,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#ifndef SDF_TRANSFORM_TO_APG_APG_H_INCLUDED
#define SDF_TRANSFORM_TO_APG_APG_H_INCLUDED

#include "base/base.h"

// Forward class definitions
class APGnode;
class APGedge;
class APGgraph;

typedef list<APGnode*>      APGnodes;
typedef APGnodes::iterator  APGnodesIter;

typedef list<APGedge*>      APGedges;
typedef APGedges::iterator  APGedgesIter;

typedef vector<APGgraph*>   APGgraphs;
typedef APGgraphs::iterator APGgraphsIter;

/**
 * APGnode
 * Node in acyclic precedence graph.
 */
class APGnode
{
public:
    // Constructor
    APGnode(const CId id) { this->id = id; weight = 0;};
    
    // Desctructor
    ~APGnode() {};
    
    // Id
    CId getId() const { return id; };

    // Weight
    int getWeight() const { return weight; };
    void setWeight(int w) { weight = w; };
    
    // Edges (management)
    void newEdge(APGedge *e);
    void delEdge(APGedge *e);
    void delAllEdges();
    
    // Edges (access)
    APGnodesIter adjNodesBegin() { return adjNodes.begin(); };
    APGnodesIter adjNodesEnd() { return adjNodes.end(); };
    APGedgesIter inEdgesBegin() { return inEdges.begin(); };
    APGedgesIter inEdgesEnd() { return inEdges.end(); };
    APGedgesIter outEdgesBegin() { return outEdges.begin(); };
    APGedgesIter outEdgesEnd() { return outEdges.end(); };
    APGedgesIter edgesBegin() { return edges.begin(); };
    APGedgesIter edgesEnd() { return edges.end(); };

    // Nodes
    APGnode *opposite(APGedge *e);
    
    // Degree
    uint inDegree() const { return inEdges.size(); };
    uint outDegree() const { return outEdges.size(); };
    uint degree() const { return edges.size(); };

private:
    CId         id;
    int         weight;
    APGnodes    adjNodes;
    APGedges    inEdges;
    APGedges    outEdges;
    APGedges    edges; 
};

/**
 * APGedge
 * Edge in acyclic precedence graph.
 */
class APGedge
{
public:
    // Constructor
    APGedge(const CId id, APGnode *src = NULL, APGnode *dst = NULL)
        { this->id = id; weight = 0; this->src = src; this->dst = dst; };
    
    // Destructor
    ~APGedge() {};
    
    // Id
    CId getId() const { return id; };

    // Weight
    int getWeight() const { return weight; };
    void setWeight(int w) { weight = w; };
    
    // Nodes
    APGnode *getSrc() const { return src; };
    APGnode *getDst() const { return dst; };
    void changeSrc(APGnode *n) { src = n; };
    void changeDst(APGnode *n) { dst = n; };
    APGnode *opposite(APGnode *n) 
        { return (n->getId() == src->getId() ? dst : src); };
    
private:
    CId     id;
    int     weight;
    APGnode *src;
    APGnode *dst;
};

/**
 * APGgraph
 * Acyclic precedence graph.
 */
class APGgraph
{
public:
    // Constructor
    APGgraph(const CId id = 0);
    
    // Destructor
    ~APGgraph();

    // Nodes (management)    
    uint nrNodes() const { return nodes.size(); };
    APGnode *newNode(const CId id);
    void delNode(APGnode *n);
    void delAllNodes();

    // Nodes (access)
    APGnode *getNode(CId id);
    APGnodesIter nodesBegin() { return nodes.begin(); };
    APGnodesIter nodesEnd() { return nodes.end(); };
    
    // Edges (management)
    uint nrEdges() const { return edges.size(); };
    APGedge *newEdge(const CId id, APGnode *src, APGnode *dst);
    void delEdge(APGedge *e);
    void delAllEdges();
    
    // Edges (access)
    APGedge *getEdge(CId id);
    APGedgesIter edgesBegin() { return edges.begin(); };
    APGedgesIter edgesEnd() { return edges.end(); };
        
private:
    CId         id;
    APGnodes    nodes;
    APGedges    edges;
};

#endif
