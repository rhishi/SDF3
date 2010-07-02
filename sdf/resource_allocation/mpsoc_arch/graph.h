/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
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
 * $Id: graph.h,v 1.2 2008/02/29 15:24:49 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_GRAPH_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_GRAPH_H_INCLUDED

#include "tile.h"
#include "connection.h"

#define ACTOR_NOT_BOUND     CID_MAX
#define CHANNEL_NOT_BOUND   CID_MAX

/**
 * PlatformGraph
 * Container for platform graph.
 */
class PlatformGraph : public ArchComponent
{
public:

    // Constructor
    PlatformGraph(ArchComponent &c);
    
    // Destructor
    virtual ~PlatformGraph();

    // Tiles
    TilesIter tilesBegin() { return tiles.begin(); };
    TilesCIter tilesBegin() const { return tiles.begin(); };
    TilesIter tilesEnd() { return tiles.end(); };
    TilesCIter tilesEnd() const { return tiles.end(); };
    CSize nrTiles() const { return tiles.size(); };
    Tiles getTiles() const { return tiles; };
    Tile *getTile(const CId id);
    Tile *getTile(const CString &name);
    Tile *createTile(const CString &name);
    
    // Connections
    ConnectionsIter connectionsBegin() { return connections.begin(); };
    ConnectionsCIter connectionsBegin() const { return connections.begin(); };
    ConnectionsIter connectionsEnd() { return connections.end(); };
    ConnectionsCIter connectionsEnd() const { return connections.end(); };
    Connections getConnections(Tile *srcTile, Tile *dstTile) const;
    Connection *getConnection(Tile *srcTile, Tile *dstTile) const;
    Connection *getConnection(const CId id);
    Connection *getConnection(const CString &name);
    Connection *createConnection(const CString &name);

private:
    // Tiles
    Tiles tiles;
    
    // Connections
    Connections connections;
};

typedef list<PlatformGraph*>            PlatformGraphs;
typedef PlatformGraphs::iterator        PlatformGraphsIter;
typedef PlatformGraphs::const_iterator  PlatformGraphsCIter;

/**
 * constructPlatformGraph ()
 * Construct an platform graph.
 */
PlatformGraph *constructPlatformGraph(const CNodePtr archNode);

/**
 * setUsagePlatformGraph ()
 * Reserve resource in platform graph.
 */
void setUsagePlatformGraph(PlatformGraph *g, const CNodePtr usageNode);

/**
 * setMappingPlatformGraph ()
 * The function sets the mapping of the application graph to the platform
 * graph as specified in the mapping node.
 */
void setMappingPlatformGraph(PlatformGraph *ar, TimedSDFgraph *ap,
        const CNodePtr mappingNode);

#endif
