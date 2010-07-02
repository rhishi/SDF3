/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 3, 2006
 *
 *  Function        :   Output mapping and system usage in XML format.
 *
 *  History         :
 *      03-04-06    :   Initial version.
 *
 * $Id: xml.cc,v 1.2 2008/03/06 10:49:45 sander Exp $
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

#include "xml.h"

/**
 * createMappingNode ()
 * Create an XML node which describes the mapping of an application graph
 * onto an platform graph.
 */
CNode *createMappingNode(PlatformGraph *g, SDFgraph *appGraph)
{
    CNode *mappingNode, *tileNode, *procNode, *memNode, *niNode;
    CNode *actorNode, *channelNode, *connectionNode;
    NetworkInterface *ni;
    CString strValue;
    Processor *p;
    Memory *m;
    
    // Mapping node
    mappingNode = CNewNode("mapping");
    CAddAttribute(mappingNode, "appGraph", appGraph->getName());
    CAddAttribute(mappingNode, "archGraph", g->getName());
    
    // Tiles
    for (TilesIter iter = g->tilesBegin(); iter != g->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        
        // Tile
        tileNode = CAddNode(mappingNode, "tile");
        CAddAttribute(tileNode, "name", t->getName());

        // Processor
        p = t->getProcessor();
        if (p != NULL)
        {
            procNode = CAddNode(tileNode, "processor");
            CAddAttribute(procNode, "name", p->getName());
            CAddAttribute(procNode, "timeslice", p->getReservedTimeSlice());

            // Bindings present?
            if (p->getActorBindings() != NULL)
            {
                // Actors mapped to processor
                for (ComponentBindingsIter iterB = 
                        p->getActorBindings()->begin();
                            iterB != p->getActorBindings()->end(); iterB++)
                {
                    ComponentBinding *b = *iterB;

                    actorNode = CAddNode(procNode, "actor");
                    CAddAttribute(actorNode, "name",
                                                b->getComponent()->getName());
                }
            }
            
            // Schedule
            CAddNode(procNode, p->getSchedule().convertToXML());
        }
            
        // Memory
        m = t->getMemory();
        if (m != NULL)
        {
            memNode = CAddNode(tileNode, "memory");
            CAddAttribute(memNode, "name", m->getName());

            // Actor bindings present?
            if (m->getActorBindings() != NULL)
            {

                // Actors mapped to memory
                for (ComponentBindingsIter
                        iterB = m->getActorBindings()->begin();
                            iterB != m->getActorBindings()->end(); iterB++)
                {
                    ComponentBinding *b = *iterB;
                    SDFcomponent *c = b->getComponent();
                    
                    if(c == NULL)
                         continue;

                    strValue = CString(b->getValue(0));
                    actorNode = CAddNode(memNode, "actor");
                    CAddAttribute(actorNode, "name", c->getName());
                    CAddAttribute(actorNode, "size", strValue);                                  }
            }
            
            // Channel bindings present?
            if (m->getChannelBindings() != NULL)
            {
                // Channels mapped to memory
                for (ComponentBindingsIter
                        iterB = m->getChannelBindings()->begin();
                            iterB != m->getChannelBindings()->end(); iterB++)
                {
                    ComponentBinding *b = *iterB;
                    SDFcomponent *c = b->getComponent();
                    
                    if(c == NULL)
                         continue;

                    strValue = CString(b->getValue(0));
                    channelNode = CAddNode(memNode, "channel");
                    CAddAttribute(channelNode, "name", c->getName());
                    CAddAttribute(channelNode, "size", strValue);
                }
            }
        }

        // Network interface
        ni = t->getNetworkInterface();
        if (ni != NULL)
        {
            niNode = CAddNode(tileNode, "networkInterface");
            CAddAttribute(niNode, "name", ni->getName());
            
            // Bindings present?
            if (ni->getBindings() != NULL)
            {
                // Channels mapped to network interface
                for (ComponentBindingsIter iterB = ni->getBindings()->begin();
                        iterB != ni->getBindings()->end(); iterB++)
                {
                    ComponentBinding *b = *iterB;
                    SDFcomponent *c = b->getComponent();

                    if (c == NULL)
                        continue;

                    channelNode = CAddNode(niNode, "channel");
                    CAddAttribute(channelNode, "name", c->getName());
                    strValue = CString(b->getValue(0));
                    CAddAttribute(channelNode, "nrConnections", strValue);
                    strValue = CString(b->getValue(1));
                    CAddAttribute(channelNode, "inBandwidth", strValue);
                    strValue = CString(b->getValue(2));
                    CAddAttribute(channelNode, "outBandwidth", strValue);
                }
            }
        }
    }
    
    // Connections
    for (ConnectionsIter iter = g->connectionsBegin(); 
            iter != g->connectionsEnd(); iter++)
    {
        Connection *c = *iter;
        
        // Connection
        connectionNode = CAddNode(mappingNode, "connection");
        CAddAttribute(connectionNode, "name", c->getName());

        // Bindings present?
        if (c->getChannelBindings() != NULL)
        {
            // Channels mapped to connection
            for (ComponentBindingsIter iterB = c->getChannelBindings()->begin();
                    iterB != c->getChannelBindings()->end(); iterB++)
            {
                ComponentBinding *b = *iterB;

                channelNode = CAddNode(connectionNode, "channel");
                CAddAttribute(channelNode, "name",
                                                b->getComponent()->getName());
            }
        }
    }

    return mappingNode;
}

/**
 * outputBindingAsXML ()
 * Output the binding of an SDFG to an platform graph in XML
 * format.
 */
void outputBindingAsXML(PlatformGraph *g, SDFgraph *appGraph, ostream &out)
{
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");
    
    // Mapping node
    CAddNode(sdf3Node, createMappingNode(g, appGraph));
    
    // Create document and save it
    CDoc *doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);
}

/**
 * createTileUsageNode ()
 * Create an XML node which describes the resource usage of a tile in the
 * platform graph.
 */
CNode *createTileUsageNode(Tile *t)
{
    CNode *tileNode, *procNode, *memNode, *niNode;
    NetworkInterface *ni;
    Processor *p;
    Memory *m;
    
    // Tile node
    tileNode = CNewNode("tile");
    CAddAttribute(tileNode, "name", t->getName());
    
    // Processor
    p = t->getProcessor();
    if (p != NULL)
    {
        procNode = CAddNode(tileNode, "processor");
        CAddAttribute(procNode, "name", p->getName());
        CAddAttribute(procNode, "timeSlice", 
                p->getTimewheelSize() - p->availableTimewheelSize());
    }
        
    // Memory
    m = t->getMemory();
    if (m != NULL)
    {
        memNode = CAddNode(tileNode, "memory");
        CAddAttribute(memNode, "name", m->getName());
        CAddAttribute(memNode, "size", m->getSize() - m->availableMemorySize());
    }
    
    // Network interface
    ni = t->getNetworkInterface();
    if (ni != NULL)
    {
        niNode = CAddNode(tileNode, "networkInterface");
        CAddAttribute(niNode, "name", ni->getName());
        CAddAttribute(niNode, "nrConnections", 
                ni->getNrConnections() - ni->availableNrConnections());
        CAddAttribute(niNode, "inBandwidth", 
                CString(ni->getInBandwidth() - ni->availableInBandwidth()));
        CAddAttribute(niNode, "outBandwidth", 
                CString(ni->getOutBandwidth() - ni->availableOutBandwidth()));
    }
    
    return tileNode;    
}

/**
 * createSystemUsageNode ()
 * The function constructs a system usage node in XML format.
 */
CNode *createSystemUsageNode(PlatformGraph *g)
{
    CNode *usageNode = CNewNode("systemUsage");
    CAddAttribute(usageNode, "archGraph", g->getName());
    
    for (TilesIter iter = g->tilesBegin(); iter != g->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        
        CAddNode(usageNode, createTileUsageNode(t));
    }
    
    return usageNode;
}


/**
 * outputSystemUsageAsXML ()
 * Output the usage of an platform graph in XML format.
 */
void outputSystemUsageAsXML(PlatformGraph *g, ostream &out)
{
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");
    
    // System usage node
    CAddNode(sdf3Node, createSystemUsageNode(g));

    // Create document and save it
    CDoc *doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);    
}

/**
 * createTileNode ()
 * Create an XML node which describes the tile t in an platform graph.
 */
CNode *createTileNode(Tile *t)
{
    CNode *tileNode, *procNode, *memNode, *niNode, *arbitNode;
    NetworkInterface *ni;
    Processor *p;
    Memory *m;
    
    // Tile node
    tileNode = CNewNode("tile");
    CAddAttribute(tileNode, "name", t->getName());

    // Processor
    p = t->getProcessor();
    if (p != NULL)
    {
        procNode = CAddNode(tileNode, "processor");
        CAddAttribute(procNode, "name", p->getName());
        CAddAttribute(procNode, "type", p->getType());
        arbitNode = CAddNode(procNode, "arbitration");
        CAddAttribute(arbitNode, "type", "TDMA");
        CAddAttribute(arbitNode, "wheelsize", p->getTimewheelSize());
    }
    
    // Memory    
    m = t->getMemory();
    if (m != NULL)
    {
        memNode = CAddNode(tileNode, "memory");
        CAddAttribute(memNode, "name", m->getName());
        CAddAttribute(memNode, "size", m->getSize());
    }
    
    // Network interface   
    ni = t->getNetworkInterface();
    if (ni != NULL)
    {
        niNode = CAddNode(tileNode, "networkInterface");
        CAddAttribute(niNode, "name", ni->getName());
        CAddAttribute(niNode, "nrConnections", ni->getNrConnections());
        CAddAttribute(niNode, "inBandwidth", CString(ni->getInBandwidth()));
        CAddAttribute(niNode, "outBandwidth", CString(ni->getOutBandwidth()));
    }
    
    return tileNode;
}

/**
 * createConnectionNode ()
 * Create an XML node which describes the connection c in an platform graph.
 */
CNode *createConnectionNode(Connection *c)
{
    CNode *connNode;

    // Connection node
    connNode = CNewNode("connection");
    CAddAttribute(connNode, "name", c->getName());
    CAddAttribute(connNode, "srcTile", c->getSrcTile()->getName());
    CAddAttribute(connNode, "dstTile", c->getDstTile()->getName());
    CAddAttribute(connNode, "delay", c->getLatency());
    
    return connNode;
}

/**
 * createPlatformGraphNode ()
 * Create an XML node which describes the platform graph.
 */
CNode *createPlatformGraphNode(PlatformGraph *g)
{
    CNode *archNode;
    
    // Architecture graph node
    archNode = CNewNode("architectureGraph");
    CAddAttribute(archNode, "name", g->getName());
    
    // Tiles
    for (TilesIter iter = g->tilesBegin(); iter != g->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        
        CAddNode(archNode, createTileNode(t));
    }
    
    // Connections
    for (ConnectionsIter iter = g->connectionsBegin();
            iter != g-> connectionsEnd(); iter++)
    {
        Connection *c = *iter;
        
        CAddNode(archNode, createConnectionNode(c));
    }
    
    return archNode;
}

/**
 * outputPlatformGraphAsXML ()
 * Output the platform graph in XML format.
 */
void outputPlatformGraphAsXML(PlatformGraph *g, ostream &out)
{
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");
    
    // Architecture graph node
    CAddNode(sdf3Node, createPlatformGraphNode(g));

    // Create document and save it
    CDoc *doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);    
}
