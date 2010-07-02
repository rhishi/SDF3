/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Platform graph
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: graph.cc,v 1.2 2008/03/06 10:49:44 sander Exp $
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
 * PlatformGraph ()
 * Constructor.
 */
PlatformGraph::PlatformGraph(ArchComponent &c)
    :
        ArchComponent(c)
{
}

/**
 * ~PlatformGraph ()
 * Destructor.
 */
PlatformGraph::~PlatformGraph()
{
    // Tiles
    for (TilesIter iter = tilesBegin(); iter != tilesEnd(); iter++)
    {
        Tile *t = *iter;
        
        delete t;
    }

    // Connections
    for (ConnectionsIter iter = connectionsBegin();
            iter != connectionsEnd(); iter++)
    {
        Connection *c = *iter;
        
        delete c;
    }
}

/**
 * getTile ()
 * The function returns a pointer to a tile with the given id or NULL if 
 * the tile does not exist.
 */
Tile *PlatformGraph::getTile(const CId id)
{
    for (TilesIter iter = tilesBegin(); iter != tilesEnd(); iter++)
    {
        Tile *t = *iter;
    
        if (t->getId() == id)
            return t;
    }

    return NULL;
}

/**
 * getTile ()
 * The function returns a pointer to a tile with the given name or NULL if 
 * the tile does not exist.
 */
Tile *PlatformGraph::getTile(const CString &name)
{
    for (TilesIter iter = tilesBegin(); iter != tilesEnd(); iter++)
    {
        Tile *t = *iter;
    
        if (t->getName() == name)
            return t;
    }

    return NULL;
}

/**
 * createTile ()
 * The adds a tile to the platform graph and returns a pointer to the
 * newly constructed tile.
 */
Tile *PlatformGraph::createTile(const CString &name)
{
    ArchComponent component(this, tiles.size(), name);
    Tile *t;
    
    // Create new tile
    t = new Tile(component);
    
    // Add tile to graph
    tiles.push_back(t);
    
    return t;
}

/**
 * getConnections ()
 * The function returns a list of all connections from the source to
 * the destination tile.
 */
Connections PlatformGraph::getConnections(Tile *srcTile, Tile *dstTile) const
{
    Connections connections;
    
    for (ConnectionsCIter iter = srcTile->outConnectionsBegin();
            iter != srcTile->outConnectionsEnd(); iter++)
    {
        Connection *c = *iter;
        
        if (c->getDstTile() == dstTile)
            connections.push_back(c);
    }

    return connections;
}

/**
 * getConnection ()
 * The function returns the first connection  it finds from the source to
 * the destination tile.
 */
Connection *PlatformGraph::getConnection(Tile *srcTile, Tile *dstTile) const
{
    for (ConnectionsCIter iter = srcTile->outConnectionsBegin();
            iter != srcTile->outConnectionsEnd(); iter++)
    {
        Connection *c = *iter;
        
        if (c->getDstTile() == dstTile)
            return c;
    }

    return NULL;
}

/**
 * getConnection ()
 * The function returns a pointer to a connection with the given id or NULL if 
 * the connection does not exist.
 */
Connection *PlatformGraph::getConnection(const CId id)
{
    if (id >= connections.size())
        return NULL;
        
    return connections[id];
}

/**
 * getConnection ()
 * The function returns a pointer to a connection with the given name or NULL if
 * the connection does not exist.
 */
Connection *PlatformGraph::getConnection(const CString &name)
{
    for (ConnectionsCIter iter = connectionsBegin();
            iter != connectionsEnd(); iter++)
    {
        Connection *c = *iter;
        
        if (c->getName() == name)
            return c;
    }

    return NULL;
}

/**
 * createConnection ()
 * The adds a connection to the platform graph and returns a pointer to the
 * newly constructed connection.
 */
Connection *PlatformGraph::createConnection(const CString &name)
{
    ArchComponent component(this, connections.size(), name);
    Connection *c;
    
    // Create new connection
    c = new Connection(component);
    
    // Add connection to graph
    connections.push_back(c);
    
    return c;
}

/**
 * constructPlatformGraph ()
 * Construct an platform graph.
 */
PlatformGraph *constructPlatformGraph(const CNodePtr archNode)
{
    CSize nrConnections, inBandwidth, outBandwidth;
    CNode *procNode, *arbitNode, *memNode, *niNode;
    CString name;
    NetworkInterface *ni;
    Connection *c;
    Processor *p;
    PlatformGraph *g;
    Memory *m;
    Tile *t;
    
    // Graph
    if (!CHasAttribute(archNode, "name"))
        throw CException("Architecture graph must have a name");
    name = CGetAttribute(archNode, "name");
    ArchComponent component = ArchComponent(NULL, 0, name);
    g = new PlatformGraph(component);
    
    // Tiles
    for (CNode *tileNode = CGetChildNode(archNode, "tile");
            tileNode != NULL; tileNode = CNextNode(tileNode, "tile"))
    {
        if (!CHasAttribute(tileNode, "name"))
            throw CException("Tile must have a name");
        name = CGetAttribute(tileNode, "name");
        
        // Create tile
        t = g->createTile(name);
        
        // Processor
        procNode = CGetChildNode(tileNode, "processor");
        if (procNode != NULL)
        {
            if (!CHasAttribute(procNode, "name"))
                throw CException("Processor must have a name.");
            name = CGetAttribute(procNode, "name");
            p = t->createProcessor(name);
            if (!CHasAttribute(procNode, "type"))
                throw CException("Processor must have a type.");
            p->setType(CGetAttribute(procNode, "type"));
            if (!CHasChildNode(procNode, "arbitration"))
                throw CException("Processor must have arbitration mechanism.");
            arbitNode = CGetChildNode(procNode, "arbitration");
            if (CGetAttribute(arbitNode, "type") != "TDMA")
                throw CException("Only TDMA arbitration supported.");
            if (!CHasAttribute(arbitNode, "wheelsize"))
                throw CException("TDMA arbitration needs wheelsize.");
            p->setTimewheelSize(CGetAttribute(arbitNode,"wheelsize"));
        }
        
        // Memory
        memNode = CGetChildNode(tileNode, "memory");
        if (memNode != NULL)
        {
            if (!CHasAttribute(memNode, "name"))
                throw CException("Memory must have a name.");
            name = CGetAttribute(memNode, "name");
            m = t->createMemory(name);
            if (!CHasAttribute(memNode, "size"))
                throw CException("Memory must have a size.");
            m->setSize(CGetAttribute(memNode, "size"));
        }
                
        // Network interface
        niNode = CGetChildNode(tileNode, "networkInterface");
        if (niNode != NULL)
        {
            if (!CHasAttribute(niNode, "name"))
                throw CException("Network interface  must have a name.");
            name = CGetAttribute(niNode, "name");
            ni = t->createNetworkInterface(name);
            if (!CHasAttribute(niNode, "nrConnections"))
                throw CException("Network interface must have nrConnections.");
            nrConnections = CGetAttribute(niNode, "nrConnections");
            if (!CHasAttribute(niNode, "inBandwidth"))
                throw CException("Network interface must have inBandwidth.");
            inBandwidth = CGetAttribute(niNode, "inBandwidth");
            if (!CHasAttribute(niNode, "outBandwidth"))
                throw CException("Network interface must have outBandwidth."); 
            outBandwidth = CGetAttribute(niNode, "outBandwidth");
            ni->setConnections(nrConnections, inBandwidth, outBandwidth);
        }
    }
    
    // Connections
    for (CNode *connectionNode = CGetChildNode(archNode, "connection");
            connectionNode != NULL; 
                connectionNode = CNextNode(connectionNode, "connection"))
    {
        // Create connection
        if (!CHasAttribute(connectionNode, "name"))
            throw CException("Connection must have a name");
        name = CGetAttribute(connectionNode, "name");
        c = g->createConnection(name);

        // Set properties
        if (!CHasAttribute(connectionNode, "delay"))
            throw CException("Connection must have a delay");
        c->setLatency(CGetAttribute(connectionNode, "delay"));

        // Connect it to source tile
        if (!CHasAttribute(connectionNode,"srcTile"))
            throw CException("Connection must have a srcTile");
        t = g->getTile(CGetAttribute(connectionNode,"srcTile"));
        if (t == NULL)
            throw CException("srcTile does not exist.");
        c->setSrcTile(t);
        t->addOutConnection(c);
        
        // Connect it to destination tile
        if (!CHasAttribute(connectionNode,"dstTile"))
            throw CException("Connection must have a dstTile");
        t = g->getTile(CGetAttribute(connectionNode,"dstTile"));
        if (t == NULL)
            throw CException("dstTile does not exist.");
        c->setDstTile(t);
        t->addInConnection(c);
    }
    
    return g;
}

/**
 * setUsagePlatformGraph ()
 * Reserve resource in platform graph.
 */
void setUsagePlatformGraph(PlatformGraph *g, const CNodePtr archNode)
{
    CSize nrConnections, inBandwidth, outBandwidth;
    CNode *procNode, *memNode, *niNode;
    CString name;
    NetworkInterface *ni;
    Processor *p;
    Memory *m;
    Tile *t;
    
    // Tiles
    for (CNode *tileNode = CGetChildNode(archNode, "tile");
            tileNode != NULL; tileNode = CNextNode(tileNode, "tile"))
    {
        if (!CHasAttribute(tileNode, "name"))
            throw CException("Tile must have a name");
        name = CGetAttribute(tileNode, "name");
        
        // Find tile in architecture graph
        t = g->getTile(name);
        if (t == NULL)
            throw CException("Cannot set system usage. Tile does not exist.");
        
        // Processor
        if (CHasChildNode(tileNode, "processor"))
        {
            // Processor in graph
            p = t->getProcessor();
            if (p == NULL)
                throw CException("Tile contains no processor.");
            
            // Set used time slice
            procNode = CGetChildNode(tileNode, "processor");
            if (!CHasAttribute(procNode, "timeSlice"))
                throw CException("Processor must specify used timeSlice");
            p->setOccupiedTimeSlice(CGetAttribute(procNode, "timeSlice"));
        }
        
        // Memory
        if (CHasChildNode(tileNode, "memory"))
        {
            // Memory in graph
            m = t->getMemory();
            if (m == NULL)
                throw CException("Tile contains no memory.");
            
            // Set used memory size
            memNode = CGetChildNode(tileNode, "memory");
            if (!CHasAttribute(memNode, "size"))
                throw CException("Memory must specify used size");
            m->reserveMemory(CGetAttribute(memNode, "size"));
        }
        
        // Network interface
        if (CHasChildNode(tileNode, "networkInterface"))
        {
            // Memory in graph
            ni = t->getNetworkInterface();
            if (ni == NULL)
                throw CException("Tile contains no network interface.");
            
            // Set used connections and bandwidth
            niNode = CGetChildNode(tileNode, "networkInterface");
            if (!CHasAttribute(niNode, "nrConnections"))
                throw CException("NI must specify used nrConnections");
            nrConnections = CGetAttribute(niNode, "nrConnections");
            if (!CHasAttribute(niNode, "inBandwidth"))
                throw CException("NI must specify used inBandwidth");
            inBandwidth = CGetAttribute(niNode, "inBandwidth");
            if (!CHasAttribute(niNode, "outBandwidth"))
                throw CException("NI must specify used outBandwidth");
            outBandwidth = CGetAttribute(niNode, "outBandwidth");
            ni->reserveConnection(NULL, nrConnections, inBandwidth,
                    outBandwidth);
        }
    }
}

/**
 * setMappingProcessor ()
 * The function sets the mapping of the application graph to the processor.
 */
void setMappingProcessor(Processor *p, TimedSDFgraph *g,
        const CNodePtr procNode)
{
    StaticOrderSchedule s;

    if (p == NULL)
        throw CException("[ERROR] processor does not exist.");

    // Reserve timeslice on processor
    if (!p->reserveTimeSlice(CGetAttribute(procNode, "timeslice")))
        throw CException("[ERROR] failed allocating timeslice on processor.");

    // Iterate over all actors bound to the processor
    for (CNode *actorNode = CGetChildNode(procNode, "actor");
            actorNode != NULL; 
                actorNode = CNextNode(actorNode, "actor"))
    {
        TimedSDFactor *a = (TimedSDFactor*)g->getActor(CGetAttribute(actorNode,
                                                            "name"));
        
        if (a == NULL)
            throw CException("[ERROR] actor does not exist.");
        
        if (!p->bindActor(a))
        {
            throw CException("[ERROR] failed binding actor to processor.");
        }
    }
    
    // Construct schedule and assign it to the processor
    if (CHasChildNode(procNode, "schedule"))
    {
        CNode *scheduleNode = CGetChildNode(procNode, "schedule");
        
        // Construct states of the schedule
        for (CNode *stateNode = CGetChildNode(scheduleNode, "state");
                stateNode != NULL; stateNode = CNextNode(stateNode, "state"))
        {
            // Add the actor to the schedule
            s.appendActor(g->getActor(CGetAttribute(stateNode, "actor")));
        
            // Is this schedule entry the start of the periodic regime?
            if (CHasAttribute(stateNode, "startOfPeriodicRegime") 
                 && CGetAttribute(stateNode, "startOfPeriodicRegime") == "true")
            {
                s.setStartPeriodicSchedule(s.size() - 1);
            }
        }

        // Link schedule to processor
        p->setSchedule(s);
    }
}

/**
 * setMappingMemory ()
 * The function sets the mapping of the application graph to the memory.
 */
void setMappingMemory(Memory *m, TimedSDFgraph *g, const CNodePtr memNode)
{
    if (m == NULL)
        throw CException("[ERROR] memory does not exist.");

    // Iterate over all channels bound to the memory
    for (CNode *channelNode = CGetChildNode(memNode, "channel");
            channelNode != NULL; 
                channelNode = CNextNode(channelNode, "channel"))
    {
        SDFchannel *ch = g->getChannel(CGetAttribute(channelNode, "name"));
        
        if (ch == NULL)
            throw CException("[ERROR] channel does not exist.");
        
        if (!m->reserveMemory(ch, CGetAttribute(channelNode, "size")))
        {
            throw CException("[ERROR] failed binding channel to memory.");
        }
    }

    // Iterate over all actors bound to the memory
    for (CNode *actorNode = CGetChildNode(memNode, "actor");
            actorNode != NULL; 
                actorNode = CNextNode(actorNode, "actor"))
    {
        SDFactor *a = g->getActor(CGetAttribute(actorNode, "name"));
        
        if (a == NULL)
            throw CException("[ERROR] actor does not exist.");
        
        if (!m->reserveMemory(a, CGetAttribute(actorNode, "size")))
        {
            throw CException("[ERROR] failed binding actor to memory.");
        }
    }
}

/**
 * setMappingNetworkInterface ()
 * The function sets the mapping of the application graph to the NI.
 */
void setMappingNetworkInterface(NetworkInterface *ni, TimedSDFgraph *g, 
        const CNodePtr niNode)
{
    if (ni == NULL)
        throw CException("[ERROR] network interface does not exist.");

    // Iterate over all channels bound to the network interface
    for (CNode *channelNode = CGetChildNode(niNode, "channel");
            channelNode != NULL; 
                channelNode = CNextNode(channelNode, "channel"))
    {
        TimedSDFchannel *ch = (TimedSDFchannel*)g->getChannel(CGetAttribute(
                                                        channelNode, "name"));
        
        if (ch == NULL)
            throw CException("[ERROR] channel does not exist.");
        
        if (!ni->reserveConnection(ch, 
                CGetAttribute(channelNode, "nrConnections"),
                CGetAttribute(channelNode, "inBandwidth"),
                CGetAttribute(channelNode, "outBandwidth")))
        {
            throw CException("[ERROR] failed binding channel to network "
                             "interface.");
        }
    }

}

/**
 * setMappingTile ()
 * The function sets the mapping of the application graph to the tile.
 */
void setMappingTile(Tile *t, TimedSDFgraph *g, const CNodePtr tileNode)
{
    Processor *p;
    Memory *m;
    NetworkInterface *ni;
    
    if (t == NULL)
        throw CException("[ERROR] tile does not exist.");

    // Processor
    if (CHasChildNode(tileNode, "processor"))
    {
        p = t->getProcessor();
        setMappingProcessor(p, g, CGetChildNode(tileNode, "processor"));
    }
    
    // Memory
    if (CHasChildNode(tileNode, "memory"))
    {
        m = t->getMemory();
        setMappingMemory(m, g, CGetChildNode(tileNode, "memory"));
    }
    
    // Network interface
    if (CHasChildNode(tileNode, "networkInterface"))
    {
        ni = t->getNetworkInterface();
        setMappingNetworkInterface(ni, g, 
                                CGetChildNode(tileNode, "networkInterface"));
    }
}

/**
 * setMappingConnection ()
 * The function sets the mapping of the application graph to the connection.
 */
void setMappingConnection(Connection *c, TimedSDFgraph *g,
        const CNodePtr connectionNode)
{
    if (c == NULL)
        throw CException("[ERROR] connection does not exist.");

    // Iterate over all channels bound to the connection
    for (CNode *channelNode = CGetChildNode(connectionNode, "channel");
            channelNode != NULL; 
                channelNode = CNextNode(channelNode, "channel"))
    {
        TimedSDFchannel *ch = (TimedSDFchannel*)g->getChannel(CGetAttribute(
                                                        channelNode, "name"));
        
        if (ch == NULL)
            throw CException("[ERROR] channel does not exist.");
        
        if (!c->bindChannel(ch))
            throw CException("[ERROR] failed binding channel to connection.");
    }
}

/**
 * setMappingPlatformGraph ()
 * The function sets the mapping of the application graph to the platform
 * graph as specified in the mapping node.
 */
void setMappingPlatformGraph(PlatformGraph *ar, TimedSDFgraph *ap,
        const CNodePtr mappingNode)
{
    // Name of application graph does not match?
    if (CGetAttribute(mappingNode, "appGraph") != ap->getName())
        throw CException("[ERROR] Name of application graph does not match.");

    // Name of architecture graph does not match?
    if (CGetAttribute(mappingNode, "archGraph") != ar->getName())
        throw CException("[ERROR] Name of architecture graph does not match.");

    // Iterate over the tiles in the mapping node
    for (CNode *tileNode = CGetChildNode(mappingNode, "tile");
            tileNode != NULL; tileNode = CNextNode(tileNode, "tile"))
    {
        Tile *t = ar->getTile(CGetAttribute(tileNode, "name"));
        setMappingTile(t, ap, tileNode);
    }
    
    // Iterate over the connections in the mapping node
    for (CNode *connectionNode = CGetChildNode(mappingNode, "connection");
            connectionNode != NULL; 
                connectionNode = CNextNode(connectionNode, "connection"))
    {
        Connection *c =ar->getConnection(CGetAttribute(connectionNode, "name"));
        setMappingConnection(c, ap, connectionNode);
    }
   
}
