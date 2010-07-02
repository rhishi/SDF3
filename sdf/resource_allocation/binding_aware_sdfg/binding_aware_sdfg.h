/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 27, 2008
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      27-02-08    :   Initial version.
 *
 * $Id: binding_aware_sdfg.h,v 1.3 2008/03/06 13:59:05 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_BINDING_AWARE_SDFG_BINDING_AWARE_SDFG_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_BINDING_AWARE_SDFG_BINDING_AWARE_SDFG_H_INCLUDED

#include "../flow/types.h"
#include "../mpsoc_arch/graph.h"
#include "../../base/timed/graph.h"

/**
 * BindingAwareSDFG ()
 * Binding-aware SDFG.
 */
class BindingAwareSDFG : public TimedSDFgraph
{
public:
    // Constructor
    BindingAwareSDFG(TimedSDFgraph *constrainedSDFG,
            PlatformGraph *platformGraph, SDFflowType flowType);
    
    // Destructor
    ~BindingAwareSDFG() {};

    // Binding of actor to tile
    CId getBindingOfActorToTile(SDFactor *a) {
        if (a->getId() >= actorBinding.size())
            return ACTOR_NOT_BOUND;
        return actorBinding[a->getId()];
    };

    // Binding of channels to connections
    CId getBindingOfChannelToConnection(SDFchannel *c) { 
        if (c->getId() >= channelBinding.size())
            return CHANNEL_NOT_BOUND;
        return channelBinding[c->getId()];
    };

    // Static-order schedule on processor in tile
    StaticOrderSchedule &getScheduleOnTile(CId tileId) { return schedule[tileId]; };
    void setScheduleOnTile(CId tileId, StaticOrderSchedule &s) { schedule[tileId] = s; };
    
    // TDMA scheduler setting on processor in tile
    SDFtime getTDMAsizeOnTile(CId tileId) const { return tdmaSize[tileId]; };
    SDFtime getTDMAsliceOnTile(CId tileId) const { return tdmaSlice[tileId]; };

    // Number of tiles in the platform graph
    uint nrTilesInPlatformGraph() const { return nrTiles; };

private:
    // Extraction of actor and channel related binding and scheduling properties
    void extractActorMapping(TimedSDFgraph *constrainedSDFG,
            PlatformGraph *platformGraph);
    void extractChannelMapping(TimedSDFgraph *constrainedSDFG,
            PlatformGraph *platformGraph);
    
    // Model binding of application to platform in an SDFG
    void constructBindingAwareSDFG(TimedSDFgraph *constrainedSDFG,
            PlatformGraph *platformGraph, SDFflowType flowType);

    // The actual modeling functions for the NSoC flow
    void createMappedActorNSoC(TimedSDFactor *a, Tile *t);   
    void createMappedChannelToTileNSoC(TimedSDFchannel *c, Tile *t);
    void createMappedChannelToConnectionNSoC(TimedSDFchannel *ch, Connection *cn);
    void modelBindingInNSoCFlow(TimedSDFgraph *g, PlatformGraph *ag);
       
    // The actual modeling functions for the MPARM flow
    SDFtime getLatencyAMBAbusSemaphore();
    SDFtime computeLatencyAMBAbus(CSize tokenSize);
    void createMappedActorMPFlow(TimedSDFactor *a, Tile *t);   
    void createMappedChannelToTileMPFlow(TimedSDFchannel *c, Tile *t);
    void createMappedChannelToConnectionMPFlow(TimedSDFchannel *ch, Connection *cn);
    void modelBindingInMPFlow(TimedSDFgraph *g, PlatformGraph *ag);

private:
    // Binding of channels to connections
    vector< CId > channelBinding;

    // Binding of actors to tiles
    vector< CId > actorBinding;

    // Number of tiles
    uint nrTiles;

    // Static-order schedules
    vector< StaticOrderSchedule > schedule;

    // Timewheels and slices
    vector< SDFtime > tdmaSize;
    vector< SDFtime > tdmaSlice;
};

#endif

