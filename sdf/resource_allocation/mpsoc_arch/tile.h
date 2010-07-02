/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Tile in the architecture graph.
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: tile.h,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_TILE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_TILE_H_INCLUDED

#include "connection.h"
#include "processor.h"
#include "memory.h"
#include "networkinterface.h"

/**
 * Tile
 * Container for tile.
 */
class Tile : public ArchComponent
{
public:

    // Constructor
    Tile(ArchComponent &c);
    
    // Destructor
    virtual ~Tile();

    // Processor
    Processor *getProcessor() const { return processor; };
    Processor *createProcessor(const CString &name);
    void setProcessor(Processor *p) { processor = p; };
        
    // Memory
    Memory *getMemory() const { return memory; };
    Memory *createMemory(const CString &name);
    void setMemory(Memory *m) { memory = m; };
    
    // Network interface
    NetworkInterface *getNetworkInterface() const { return networkInterface; };
    NetworkInterface *createNetworkInterface(const CString &name);
    void setNetworkInterface(NetworkInterface *n) { networkInterface = n; };
    
    // Connections
    ConnectionsIter inConnectionsBegin() { return inConnections.begin(); };
    ConnectionsIter inConnectionsEnd() { return inConnections.end(); };
    void addInConnection(Connection *c);
    ConnectionsIter outConnectionsBegin() { return outConnections.begin(); };
    ConnectionsIter outConnectionsEnd() { return outConnections.end(); };
    void addOutConnection(Connection *c);
    
private:
    // Processor
    Processor *processor;
    
    // Memory
    Memory *memory;
    
    // Network interface
    NetworkInterface *networkInterface;
    
    // Connections
    Connections inConnections;    
    Connections outConnections;
};

typedef vector<Tile*>           Tiles;
typedef Tiles::iterator         TilesIter;
typedef Tiles::const_iterator   TilesCIter;

#endif
