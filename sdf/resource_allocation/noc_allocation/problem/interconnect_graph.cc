/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   interconnect_graph.cc
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
 * $Id: interconnect_graph.cc,v 1.1 2008/03/20 16:16:19 sander Exp $
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

#include "interconnect_graph.h"

/**
 * InterconnectGraph ()
 * Constructor.
 */
InterconnectGraph::InterconnectGraph(uint slotTableSize, TTime slotTablePeriod, 
    uint packetHeaderSize, uint flitSize, TTime reconfigurationTimeNI)
        :
    slotTableSize(slotTableSize),
    slotTablePeriod(slotTablePeriod), 
    packetHeaderSize(packetHeaderSize),
    flitSize(flitSize),
    reconfigurationTimeNI(reconfigurationTimeNI)
{
}
    

/**
 * InterconnectGraph ()
 * Constructor.
 */
InterconnectGraph::InterconnectGraph(const CNodePtr archNode,
        TTime slotTablePeriod) : slotTablePeriod(slotTablePeriod)
{
    Node *srcNode, *dstNode;
    CString name, src, dst;
    CNode *networkNode;
    Node *n;
    
    // Network node in architecture
    networkNode = CGetChildNode(archNode, "network");
    if (networkNode == NULL)
        throw CException("[ERROR] Missing 'network' in 'architectureGraph'.");
    
    // Network properties
    slotTableSize = CGetAttribute(networkNode, "slotTableSize");
    packetHeaderSize = CGetAttribute(networkNode, "packetHeaderSize");
    flitSize = CGetAttribute(networkNode, "flitSize");
    reconfigurationTimeNI = CGetAttribute(networkNode, "reconfigurationTimeNI");

    // Create node in the architecture graph for each tile
    for (CNode *tileNode = CGetChildNode(archNode, "tile");
            tileNode != NULL; tileNode = CNextNode(tileNode, "tile"))
    {
        name = CGetAttribute(tileNode, "name");
        n = createNode(name);
        n->setType("tile");
    }
    
    // Create node in the architecture graph for each router
    for (CNode *routerNode = CGetChildNode(networkNode, "router");
            routerNode != NULL; routerNode = CNextNode(routerNode, "router"))
    {
        name = CGetAttribute(routerNode, "name");
        n = createNode(name);
        n->setType("router");
    }
    
    // Create link between the nodes in the architecture graph
    for (CNode *linkNode = CGetChildNode(networkNode, "link");
            linkNode != NULL; linkNode = CNextNode(linkNode, "link"))
    {
        name = CGetAttribute(linkNode, "name");
        src = CGetAttribute(linkNode, "src");
        dst = CGetAttribute(linkNode, "dst");
        
        srcNode = getNode(src);
        dstNode = getNode(dst);
        
        createLink(name, srcNode, dstNode);
    }
}

/**
 * ~InterconnectGraph ()
 * Destructor.
 */
InterconnectGraph::~InterconnectGraph()
{
    // Cleanup nodes
    for (NodesIter iter = nodesBegin(); iter != nodesEnd(); iter++)
    {
        Node *n = *iter;
        
        delete n;
    }
    
    // Cleanup links
    for (LinksIter iter = linksBegin(); iter != linksEnd(); iter++)
    {
        Link *l = *iter;
        
        delete l;
    }
}

/**
 * createNode ()
 * Create a new node and add it to the graph.
 */
Node *InterconnectGraph::createNode(const CString &name)
{
    Node *n;
    
    // Node with the same name does exist?
    if (getNode(name) != NULL)
        throw CException("[ERROR] Duplicate node name.");
    
    // Create a new node and add it to the graph
    n = new Node(name, nrNodes());
    nodes.push_back(n);
    
    return n;
}

/**
 * createLink ()
 * Create a new link and add it to the graph.
 */
Link *InterconnectGraph::createLink(const CString &name, Node *src, Node *dst)
{
    Link *l;
    
    // Link with the same name does exist?
    if (getLink(name) != NULL)
        throw CException("[ERROR] Duplicate link name.");
    
    // Create a new link and add it to the graph
    l = new Link(name, nrLinks(), src, dst, getSlotTableSize(),
                                                        getSlotTablePeriod());
    links.push_back(l);
    src->addOutgoingLink(l);
    dst->addIncomingLink(l);
    
    return l;
}

/**
 * getNode ()
 * The function returns a pointer to the node with the supplied id, or NULL if
 * no such node exists.
 */
Node *InterconnectGraph::getNode(const CId id)
{
    for (NodesIter iter = nodesBegin(); iter != nodesEnd(); iter++)
        if ((*iter)->getId() == id)
            return *iter;
    
    return NULL;
}

/**
 * getNode ()
 * The function returns a pointer to the node with the supplied name, or NULL if
 * no such node exists.
 */
Node *InterconnectGraph::getNode(const CString &name)
{
    for (NodesIter iter = nodesBegin(); iter != nodesEnd(); iter++)
        if ((*iter)->getName() == name)
            return *iter;
    
    return NULL;
}

/**
 * getLink ()
 * The function returns a pointer to the linl with the supplied id, or NULL if
 * no such link exists.
 */
Link *InterconnectGraph::getLink(const CId id)
{
    for (LinksIter iter = linksBegin(); iter != linksEnd(); iter++)
        if ((*iter)->getId() == id)
            return *iter;
            
    return NULL;
}

/**
 * getLink ()
 * The function returns a pointer to the linl with the supplied name, or NULL if
 * no such link exists.
 */
Link *InterconnectGraph::getLink(const CString &name)
{
    for (LinksIter iter = linksBegin(); iter != linksEnd(); iter++)
        if ((*iter)->getName() == name)
            return *iter;
            
    return NULL;
}

/**
 * setUsageInterconnectGraph ()
 * Reserve resource in the interconnect graph.
 */
void InterconnectGraph::setUsage(const CNodePtr usageNode)
{
    SlotReservations usedSlots(getSlotTableSize());
    CString name, occupiedSlots;
    CNode *networkNode;
    CStrings slots;
    Link *l;
    
    // Usage information given?
    if (usageNode == NULL)
        return;
        
    // Network node in architecture
    networkNode = CGetChildNode(usageNode, "network");
    if (networkNode == NULL)
        return;
    
    // Set usage of each link in the network
    for (CNode *linkNode = CGetChildNode(networkNode, "link");
            linkNode != NULL; linkNode = CNextNode(linkNode, "link"))
    {
        // Find link in architecture graph
        name = CGetAttribute(linkNode, "name");
        l = getLink(name);
        if (l == NULL)
            throw CException("[ERROR] Cannot set usage of non existing link.");

        // Mark all slots as not used
        for (uint i = 0; i < getSlotTableSize(); i++)
            usedSlots[i] = false;

        // Split string of occupied slots in a list and mark the individual
        // slots which are occupied as used
        occupiedSlots = CGetAttribute(linkNode, "occupiedSlots");
        slots.clear();
        stringtok(slots, occupiedSlots, ",");
        for (CStringsIter iter = slots.begin(); iter != slots.end(); iter++)
        {
            CString s = *iter;
            uint sl = (uint)(s);
            
            if (sl >= getSlotTableSize())
            {
                throw CException("[ERROR] Cannot mark slot outside "\
                                 "slot table as used.");
            }

            usedSlots[sl] = true;
        }
        
        // Mark all occupied slots 
        l->setUsedSlots(usedSlots, 0, TTIME_MAX);
    }    
}

/**
 * createInterconnectGraphNode ()
 * The function returns a description of the interconnect graph in XML format.
 */
CNode *InterconnectGraph::createInterconnectGraphNode()
{
    CNode *networkNode, *nodeNode, *linkNode;
    
    // Create new node
    networkNode = CNewNode("network");

    // Add properties to the network node
    CAddAttribute(networkNode, "slotTableSize", getSlotTableSize());
    CAddAttribute(networkNode, "packetHeaderSize", getPacketHeaderSize());
    CAddAttribute(networkNode, "flitSize", getFlitSize());
    CAddAttribute(networkNode, "reconfigurationTimeNI",
                                            getReconfigurationTimeNI());
    
    // Create node for every tile and router
    for (NodesIter iter = nodesBegin(); iter != nodesEnd(); iter++)
    {
        Node *n = *iter;
        
        // Create node
        nodeNode = CAddNode(networkNode, n->getType());
        CAddAttribute(nodeNode, "name", n->getName());
    }
    
    // Create node for every link
    for (LinksIter iter = linksBegin(); iter != linksEnd(); iter++)
    {
        Link *l = *iter;
        
        // Create node
        linkNode = CAddNode(networkNode, "link");
        CAddAttribute(linkNode, "name", l->getName());
        CAddAttribute(linkNode, "src", l->getSrcNode()->getName());
        CAddAttribute(linkNode, "dst", l->getDstNode()->getName());
    }
    
    return networkNode;
}

