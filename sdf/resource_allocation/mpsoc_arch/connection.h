/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   connection.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Connection between tiles in an architecture graph
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: connection.h,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_CONNECTION_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_CONNECTION_H_INCLUDED

#include "component.h"
#include "binding.h"

// Forward class definition
class Tile;

/**
 * Connection
 * Container for connection.
 */
class Connection : public ArchComponent
{
public:

    // Constructor
    Connection(ArchComponent &c);
    
    // Destructor
    virtual ~Connection();

    // Latency
    TTime getLatency() const { return latency; };
    void setLatency(const TTime t) { latency = t; };

    // Tiles
    Tile *getSrcTile() const { return srcTile; };
    void setSrcTile(Tile *t) { srcTile = t; };
    Tile *getDstTile() const { return dstTile; };
    void setDstTile(Tile *t) { dstTile = t; };
    
    // Channel binding
    bool bindChannel(TimedSDFchannel *c);
    bool unbindChannel(TimedSDFchannel *c);
    CompBindings *getChannelBindings() { return channelBindings; };
    
private:
    // Latency
    TTime latency;
    
    // Tiles
    Tile *srcTile;
    Tile *dstTile;
    
    // Channel binding
    CompBindings *channelBindings;
};

typedef vector<Connection*>         Connections;
typedef Connections::iterator       ConnectionsIter;
typedef Connections::const_iterator ConnectionsCIter;

#endif
