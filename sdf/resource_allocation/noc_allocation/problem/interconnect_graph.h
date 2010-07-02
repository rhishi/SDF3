/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   interconnect_graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Interconnect graph
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: interconnect_graph.h,v 1.1 2008/03/20 16:16:19 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_INTERCONNECT_GRAPH_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_INTERCONNECT_GRAPH_H_INCLUDED

#include "link.h"
#include "node.h"

/**
 * InterconnectGraph
 * Interconnect graph.
 */
class InterconnectGraph
{
public:
    // Constructor
    InterconnectGraph(uint slotTableSize, TTime slotTablePeriod,
        uint packetHeaderSize, uint flitSize, TTime reconfigurationTimeNI);
    InterconnectGraph(const CNodePtr archNode, TTime slotTablePeriod);
    
    // Destructor
    ~InterconnectGraph();

    // Nodes
    NodesIter nodesBegin() { return nodes.begin(); };
    NodesIter nodesEnd() { return nodes.end(); };
    uint nrNodes() const { return nodes.size(); };
    Node *createNode(const CString &name);
    Node *getNode(const CId id);
    Node *getNode(const CString &name);
    Nodes getNodes() const { return nodes; };
        
    // Links
    LinksIter linksBegin() { return links.begin(); };
    LinksIter linksEnd() { return links.end(); };
    uint nrLinks() const { return links.size(); };
    Link *createLink(const CString &name, Node *src, Node *dst);
    Link *getLink(const CId id);
    Link *getLink(const CString &name);
    
    // NoC properties (slots, header, flits, NI)
    uint getSlotTableSize() const { return slotTableSize; };
    TTime getSlotTablePeriod() const { return slotTablePeriod; };
    uint getPacketHeaderSize() const { return packetHeaderSize; };
    uint getFlitSize() const { return flitSize; };
    TTime getReconfigurationTimeNI() const { return reconfigurationTimeNI; };

    // Reserve resource in the interconnect graph
    void setUsage(const CNodePtr usageNode);    
    
    // Convert the interconnect graph to XML format
    CNode *createInterconnectGraphNode();
    
private:
    // Nodes
    Nodes nodes;
    
    // Links
    Links links;
    
    // NoC properties (slots, header, flits, NI)
    uint slotTableSize;
    TTime slotTablePeriod;
    uint packetHeaderSize;
    uint flitSize;
    TTime reconfigurationTimeNI;
};

typedef list<InterconnectGraph*>            InterconnectGraphs;
typedef InterconnectGraphs::iterator        InterconnectGraphsIter;
typedef InterconnectGraphs::const_iterator  InterconnectGraphsCIter;

#endif

