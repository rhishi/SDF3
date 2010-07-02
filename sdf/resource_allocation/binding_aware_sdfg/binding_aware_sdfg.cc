/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.cc
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
 * $Id: binding_aware_sdfg.cc,v 1.3 2008/03/06 13:59:05 sander Exp $
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

#include "binding_aware_sdfg.h"

/**
 * BindingAwareSDFG ()
 * Constructor.
 */
BindingAwareSDFG::BindingAwareSDFG(TimedSDFgraph *constrainedSDFG,
            PlatformGraph *platformGraph, SDFflowType flowType)
    : TimedSDFgraph()
{
    // No application or architecture given?
    ASSERT(constrainedSDFG != NULL, "No constrained SDFG supplied.");
    ASSERT(platformGraph != NULL, "No platform graph supplied.");

    // Extract actor related binding and scheduling properties
    extractActorMapping(constrainedSDFG, platformGraph);
    
    // Extract channel related binding and scheduling properties
    extractChannelMapping(constrainedSDFG, platformGraph);
    
    // Create a binding-aware SDFG
    constructBindingAwareSDFG(constrainedSDFG, platformGraph, flowType);
}

/**
 * extractActorMapping ()
 * The function extracts the actor binding and scheduling from the platform
 * graph. This information is stored in the object variables 'actorBinding',
 * 'nrTiles', 'schedule', 'tdmaSize', and 'tdmaSlice'.
 */
void BindingAwareSDFG::extractActorMapping(TimedSDFgraph *constrainedSDFG,
        PlatformGraph *platformGraph)
{
    // Number of tiles in platform
    nrTiles = platformGraph->nrTiles();

    // Resize vectors
    actorBinding.resize(constrainedSDFG->nrActors());
    schedule.resize(nrTiles);
    tdmaSize.resize(nrTiles);
    tdmaSlice.resize(nrTiles);
    
    // Initialize actor bindings
    for (uint i = 0; i < constrainedSDFG->nrActors(); i++)
        actorBinding[i] = ACTOR_NOT_BOUND;

    // Extract binding and scheduling from the platform graph
    for (TilesIter iter = platformGraph->tilesBegin();
            iter != platformGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();

        // Tile contains processor?
        if (p != NULL)
        {
            // Set binding of actors to the tile containing this processor
            for (ComponentBindingsIter bIter = p->getActorBindings()->begin();
                    bIter != p->getActorBindings()->end(); bIter++)
            {
                CId actorId = (*bIter)->getComponent()->getId();
                
                // Actor should not be bound already
                ASSERT(actorBinding[actorId] == ACTOR_NOT_BOUND,
                        "Actor bound to multiple tiles.");
                
                // Set the binding of the actor to the tile
                actorBinding[actorId] = t->getId();
            }
            
            // Set the schedule for the actors in the tile
            schedule[t->getId()] = p->getSchedule();
            
            // TDMA wheel size
            tdmaSize[t->getId()] = p->getTimewheelSize();
            
            // TDMA time slice of application
            tdmaSlice[t->getId()] = p->getReservedTimeSlice();
        }
    }
}

/**
 * extractChannelMapping ()
 * The function extracts the actor binding and scheduling from the platform
 * graph. This information is stored in the object variable 'channelBinding'.
 */
void BindingAwareSDFG::extractChannelMapping(TimedSDFgraph *constrainedSDFG,
        PlatformGraph *platformGraph)
{
    // Resize vectors
    channelBinding.resize(constrainedSDFG->nrChannels());
    
    // Initialize binding
    for (uint i = 0; i < constrainedSDFG->nrChannels(); i++)
        channelBinding[i] = CHANNEL_NOT_BOUND;

    // Iterate over the connections to find channel to connection bindings
    for (ConnectionsIter iter = platformGraph->connectionsBegin(); 
            iter != platformGraph->connectionsEnd(); iter++)
    {
        Connection *c = *iter;

        // Iterate over the component bindings
        for (ComponentBindingsIter bIter = c->getChannelBindings()->begin();
                bIter != c->getChannelBindings()->end(); bIter++)
        {
            CId channelId = (*bIter)->getComponent()->getId();
            
            // Channel should not be bound already
            ASSERT(channelBinding[channelId] == CHANNEL_NOT_BOUND,
                        "Channel bound to multiple connections.");
            
            channelBinding[channelId] = c->getId();
        }
    }
}

/**
 * constructBindingAwareSDFG ()
 * Create a binding-aware SDFG for the constrained SDFG taking into account the
 * properties of the platform graph and the mapping as specified in the same
 * graph. Note that this function is just a shell to select the correct flow
 * model. The actual binding-aware SDFG construction is done in a different
 * function.
 */
void BindingAwareSDFG::constructBindingAwareSDFG(TimedSDFgraph *constrainedSDFG,
        PlatformGraph *platformGraph, SDFflowType flowType)
{
    switch (flowType)
    {
        case SDFflowTypeNSoC:
            modelBindingInNSoCFlow(constrainedSDFG, platformGraph);
            break;
        
        case SDFflowTypeMPFlow:
            modelBindingInMPFlow(constrainedSDFG, platformGraph);
            break;
        
        default:
            throw CException("[ERROR] This flow type is not supported.");
            break;
    }
}

/**
 * createMappedActorNSoC ()
 * The function models an actor mapped to a tile.
 */
void BindingAwareSDFG::createMappedActorNSoC(TimedSDFactor *a, Tile *t)
{
    SDFchannel *c;
    TimedSDFactor::Processor wcrt;
    
    // Set the execution time of the actor as its worst-case response time
    wcrt.type = "wcrt";
    wcrt.execTime = a->getExecutionTime(t->getProcessor()->getType());
    wcrt.stateSize = 0;
    a->addProcessor(&wcrt);
    a->setDefaultProcessor("wcrt");

    // Add self-loop with one token
    c = a->getGraph()->createChannel(a, 1, a, 1, 1);
}

/**
 * createMappedChannelToTileNSoC ()
 * The function models a channel mapped to a tile.
 */
void BindingAwareSDFG::createMappedChannelToTileNSoC(TimedSDFchannel *c, 
        Tile *t)
{
    SDFgraph *g;
    TimedSDFchannel *cMem;
        
    // Add channel from dst to src actor to model memory space
    g = c->getGraph();
    cMem = (TimedSDFchannel*) g->createChannel(c->getDstActor(),
                                               c->getDstPort()->getRate(),
                                               c->getSrcActor(),
                                               c->getSrcPort()->getRate(), 0);
    // Initial tokens reflect storage space
    if ((CSize)c->getBufferSize().mem < c->getInitialTokens())
        throw CException("Insufficient memory space to store initial tokens.");
    cMem->setInitialTokens(c->getBufferSize().mem - c->getInitialTokens());
    
    // Channel cMem models storage space of channel c
    cMem->setStorageSpaceChannel(c);
}

/**
 * createMappedChannelToConnectionNSoC ()
 * The function models a channel mapped to a connection.
 */
void BindingAwareSDFG::createMappedChannelToConnectionNSoC(TimedSDFchannel *ch, 
        Connection *cn)
{
    TimedSDFactor::Processor latency, tdma;
    SDFchannel *cConn, *cA, *cB, *cC, *cD, *cE;
    TimedSDFchannel *cSrc, *cDst;
    TimedSDFactor *c, *d, *e;
    Processor *dstP;
    SDFgraph *g;
    double connectionDelay;
    
    // SDF graph
    g = ch->getGraph();

    // Destination processor of connection
    dstP = cn->getDstTile()->getProcessor();
    
    // Create an actor which models the connection
    c = (TimedSDFactor*)g->createActor();
    c->setName(ch->getName() + "_connection");
    connectionDelay = ch->getTokenSize() / ch->getMinBandwidth();
    latency.execTime = cn->getLatency() + (CSize) (ceil(connectionDelay));
    latency.type = "latency";

    latency.stateSize = 0;
    c->addProcessor(&latency);
    c->setDefaultProcessor("latency");

    // Add self-loop to actor c (only 1 token sent at the same time)
    cConn = g->createChannel(c, 1, c, 1, 1);

    // Create channel to model source buffer
    cSrc = (TimedSDFchannel*) g->createChannel(c, 1, ch->getSrcActor(),
                                            ch->getSrcPort()->getRate(), 0);
    if ((CSize)ch->getBufferSize().src < ch->getInitialTokens())
        throw CException("Insufficient memory space for initial tokens.");
    cSrc->setInitialTokens(ch->getBufferSize().src - ch->getInitialTokens());

    // Channel cSrc models storage space of channel ch
    cSrc->setStorageSpaceChannel(ch);

    // Create channel to model destination buffer
    cDst = (TimedSDFchannel*)  g->createChannel(ch->getDstActor(),
                                            ch->getDstPort()->getRate(),c,1,0);
    cDst->setInitialTokens(ch->getBufferSize().dst);

    // Channel cSrc models storage space of channel ch
    cDst->setStorageSpaceChannel(ch);

    // Connect source actor channel ch to actor c
    cA = g->createChannel(ch->getSrcActor(),ch->getSrcPort()->getRate(),c, 1,0);
    cA->setInitialTokens(ch->getInitialTokens());

    // Synchronization between timewheels needed?
    if (dstP->getTimewheelSize() > dstP->getReservedTimeSlice())
    {
        // Create an actor which models the synchronization between TDMA wheels
        d = (TimedSDFactor*)g->createActor();
        d->setName(ch->getName() + "_tdma");
        tdma.type = "tdma";
        tdma.execTime = dstP->getTimewheelSize() - dstP->getReservedTimeSlice();
        tdma.stateSize = 0;
        d->addProcessor(&tdma);
        d->setDefaultProcessor("tdma");

        // The channel ch must be split in 3 channels connected to actor c and d
        cB = g->createChannel(c, 1, d, 1, 0);
        cC = g->createChannel(d, 1, ch->getDstActor(),
                                                ch->getDstPort()->getRate(), 0);
    }
    else
    {
        cC = g->createChannel(c, 1, ch->getDstActor(),
                                                ch->getDstPort()->getRate(), 0);
    }
    
    // Create actor to model minimal latency between production and
    // consumption of tokens on a channel bound to a connection.
    if (ch->getMinLatency() > 0)
    {
        e = (TimedSDFactor*)g->createActor();
        e->setName(ch->getName() + "_latency");
        latency.type = "latency";
        latency.execTime = ch->getMinLatency();
        latency.stateSize = 0;
        e->addProcessor(&latency);
        e->setDefaultProcessor("latency");

        cD = (TimedSDFchannel*) g->createChannel(ch->getSrcActor(),
                                        ch->getSrcPort()->getRate(), e, 1, 0);
        cE = (TimedSDFchannel*) g->createChannel(e, 1, ch->getDstActor(),
                                        ch->getDstPort()->getRate(), 0);
        cD->setInitialTokens(ch->getInitialTokens());
    }
}

/**
 * modelBindingInNSoCFlow ()
 * Model all binding decisions in a timed SDF graph.
 */
void BindingAwareSDFG::modelBindingInNSoCFlow(TimedSDFgraph *g, 
        PlatformGraph *pg)
{
    Tile *t, *srcTile, *dstTile;
    Connections channelBinding;
    Tiles actorBinding;
    Connection *c;
    
    // Copy all actors and channels of the constrained SDFG 'g' into 
    // this binding-aware graph
    // Step 1: properties of the graph
    setId(g->getId());
    setName(g->getName());
    setType(g->getType());

    // Step 2: copy of the actors
    for (SDFactorsCIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFcomponent component = SDFcomponent(this, nrActors());
        SDFactor *a = (*iter)->clone(component);
        addActor(a);
    }
    
    // Step 2: copy of the channels
    for (SDFchannelsCIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        SDFcomponent component = SDFcomponent(this, nrChannels());
        SDFchannel *ch = (*iter)->clone(component);
        addChannel(ch);
    }
    
    // Set the execution time of each actor to the correct processor
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
    
        // Tile
        t = pg->getTile(getBindingOfActorToTile(a));
    
        // Actor not bound to a tile?
        ASSERT(t != NULL, "All actors must be bound to a tile.");

        createMappedActorNSoC((TimedSDFactor*)(getActor(a->getId())), t);
    }
    
    // Insert SDF model for each channel
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        TimedSDFchannel *ch = (TimedSDFchannel*)*iter;

        // Skip the channel if its a self-edge on an actor
        if (ch->getSrcActor()->getId() == ch->getDstActor()->getId())
            continue;
        
        // Tiles
        srcTile = pg->getTile(getBindingOfActorToTile(ch->getSrcActor()));
        dstTile = pg->getTile(getBindingOfActorToTile(ch->getDstActor()));
        
        // Source and destination actor bound to same tile?
        if (srcTile == dstTile)
        {
            // Channel should not be mapped to a connection
            ASSERT(getBindingOfChannelToConnection(ch) == CHANNEL_NOT_BOUND,
                    "Channel must not be bound to connection.");
        
            createMappedChannelToTileNSoC((TimedSDFchannel*)
                                (getChannel(ch->getId())), srcTile);
        }
        else
        {
            // Channel must be bound to a connection
            ASSERT(getBindingOfChannelToConnection(ch) != CHANNEL_NOT_BOUND,
                    "Channel must be bound to connection.");
            
            // Find connection c used for channel ch
            c = pg->getConnection(getBindingOfChannelToConnection(ch));
                         
            // Channel must be bound to a connection from source to destination
            // tile
            if (c->getSrcTile() != srcTile || c->getDstTile() != dstTile)
            {
                throw CException("Channel bound to invalid connection.");
            }
            
            createMappedChannelToConnectionNSoC((TimedSDFchannel*)
                                (getChannel(ch->getId())), c);
        }
    }
}

/**
 * createMappedActorMPFlow ()
 * The function models an actor mapped to a tile.
 */
void BindingAwareSDFG::createMappedActorMPFlow(TimedSDFactor *a, Tile *t)
{
    SDFchannel *c;
    TimedSDFactor::Processor wcrt;
    
    // Set the execution time of the actor as its worst-case response time
    wcrt.type = "wcrt";
    wcrt.execTime = a->getExecutionTime(t->getProcessor()->getType());
    wcrt.stateSize = 0;
    a->addProcessor(&wcrt);
    a->setDefaultProcessor("wcrt");

    // Add self-loop with one token
    c = a->getGraph()->createChannel(a, 1, a, 1, 1);
}

/**
 * createMappedChannelToTileMPFlow ()
 * The function models a channel mapped to a tile.
 */
void BindingAwareSDFG::createMappedChannelToTileMPFlow(TimedSDFchannel *c, 
        Tile *t)
{
    SDFgraph *g;
    TimedSDFchannel *cMem;
        
    // Add channel from dst to src actor to model memory space
    g = c->getGraph();
    cMem = (TimedSDFchannel*) g->createChannel(c->getDstActor(),
                                               c->getDstPort()->getRate(),
                                               c->getSrcActor(),
                                               c->getSrcPort()->getRate(), 0);
    // Initial tokens reflect storage space
    if ((CSize)c->getBufferSize().mem < c->getInitialTokens())
        throw CException("Insufficient memory space to store initial tokens.");
    cMem->setInitialTokens(c->getBufferSize().mem - c->getInitialTokens());
    
    // Channel cMem models storage space of channel c
    cMem->setStorageSpaceChannel(c);
}

/**
 * getLatencyAMBAbusSemaphore ()
 * The function returns the latency for sending a semaphore through an AMBA bus.
 */
SDFtime BindingAwareSDFG::getLatencyAMBAbusSemaphore()
{
    return 11;
}

/**
 * computeLatencyAMBAbus ()
 * The function takes as an argument the size of a token (in bits) and returns
 * the time needed to transfer this token over the AMBA bus.
 */
SDFtime BindingAwareSDFG::computeLatencyAMBAbus(CSize tokenSize)
{
    SDFtime latency;
    
    // Round up to multiple of 32 bits
    if (tokenSize % 32 != 0)
        tokenSize = tokenSize + 32 - tokenSize % 32;
    
    // Equation for latency depends on the token size
    if (tokenSize < 1024)
    {
        latency = (SDFtime) ceil(0.34144 * tokenSize + 110.592);
    }
    else
    {
        latency = (SDFtime) ceil(0.36660 * tokenSize + 90.806);
    }
    
    return latency;
}

/**
 * createMappedChannelToConnectionMPFlow ()
 * The function models a channel mapped to a connection.
 */
void BindingAwareSDFG::createMappedChannelToConnectionMPFlow(
        TimedSDFchannel *ch, Connection *cn)
{
    TimedSDFactor::Processor semaphoreLatency, tdmaSrcSync, tdmaDstSync;
    TimedSDFactor::Processor creditLatency, communicationLatency;
    TimedSDFactor *creditLatencyActor, *communicationLatencyActor;
    TimedSDFactor *srcActor, *dstActor, *semaphoreLatencyActor;
    TimedSDFactor *tdmaSrcSyncActor, *tdmaDstSyncActor;
    TimedSDFchannel *chSrcSemaphoreActor, *chSemaphoreTDMAdstActor;
    TimedSDFchannel *chTDMAdstCommActor, *chCommDstActor;
    TimedSDFchannel *chCommCreditActor, *chCreditTDMAsrcActor;
    TimedSDFchannel *chDstCommActor, *chTDMAsrcSrcActor;
    SDFport *srcPort, *dstPort;
    Processor *srcProc, *dstProc;
    SDFgraph *g;

    // SDF graph
    g = ch->getGraph();

    // Source and destination actor and port
    srcActor = (TimedSDFactor*)ch->getSrcActor();
    dstActor = (TimedSDFactor*)ch->getDstActor();
    srcPort = ch->getSrcPort();
    dstPort = ch->getDstPort();
    
    // Source and destination processor of connection
    dstProc = cn->getDstTile()->getProcessor();
    srcProc = cn->getSrcTile()->getProcessor();

    // Create an actor to model semaphore latency
    semaphoreLatencyActor = (TimedSDFactor*)g->createActor();
    semaphoreLatencyActor->setName(ch->getName() + "_semaphore");
    semaphoreLatency.execTime = getLatencyAMBAbusSemaphore();
    semaphoreLatency.type = "latency";
    semaphoreLatency.stateSize = 0;
    semaphoreLatencyActor->addProcessor(&semaphoreLatency);
    semaphoreLatencyActor->setDefaultProcessor("latency");
    
    // Create an actor to model credit latency
    creditLatencyActor = (TimedSDFactor*)g->createActor();
    creditLatencyActor->setName(ch->getName() + "_credit");
    creditLatency.execTime = getLatencyAMBAbusSemaphore();
    creditLatency.type = "latency";
    creditLatency.stateSize = 0;
    creditLatencyActor->addProcessor(&creditLatency);
    creditLatencyActor->setDefaultProcessor("latency");
    
    // Create an actor to model communication latency
    communicationLatencyActor = (TimedSDFactor*)g->createActor();
    communicationLatencyActor->setName(ch->getName() + "_communication");
    communicationLatency.execTime = computeLatencyAMBAbus(ch->getTokenSize());
    communicationLatency.type = "latency";
    communicationLatency.stateSize = 0;
    communicationLatencyActor->addProcessor(&communicationLatency);
    communicationLatencyActor->setDefaultProcessor("latency");
    
    // Create an actor to model TDMA synchronization on the src processor
    tdmaSrcSyncActor = (TimedSDFactor*)g->createActor();
    tdmaSrcSyncActor->setName(ch->getName() + "_tdma_sync_src");
    tdmaSrcSync.type = "tdma";
    tdmaSrcSync.execTime = srcProc->getTimewheelSize() 
                                            - srcProc->getReservedTimeSlice();
    tdmaSrcSync.stateSize = 0;
    tdmaSrcSyncActor->addProcessor(&tdmaSrcSync);
    tdmaSrcSyncActor->setDefaultProcessor("tdma");
    
    // Create an actor to model TDMA synchronization on the dst processor
    tdmaDstSyncActor = (TimedSDFactor*)g->createActor();
    tdmaDstSyncActor->setName(ch->getName() + "_tdma_sync_dst");
    tdmaDstSync.type = "tdma";
    tdmaDstSync.execTime = dstProc->getTimewheelSize() 
                                            - dstProc->getReservedTimeSlice();
    tdmaDstSync.stateSize = 0;
    tdmaDstSyncActor->addProcessor(&tdmaDstSync);
    tdmaDstSyncActor->setDefaultProcessor("tdma");

    // Create channel from source actor to semaphore latency actor
    chSrcSemaphoreActor = (TimedSDFchannel*) g->createChannel(srcActor, 
                            srcPort->getRate(), semaphoreLatencyActor, 1, 0);
    chSrcSemaphoreActor->setInitialTokens(ch->getInitialTokens());
    
    // Create channel from semaphore latency actor to TDMA sync actor on dst
    chSemaphoreTDMAdstActor = (TimedSDFchannel*) g->createChannel(
                            semaphoreLatencyActor, 1, tdmaDstSyncActor, 1, 0);
    
    // Create channel from TDMA sync actor on dst to communication latency actor
    chTDMAdstCommActor = (TimedSDFchannel*) g->createChannel(
                            tdmaDstSyncActor, 1, communicationLatencyActor,
                            dstPort->getRate(), 0);
    
    // Create channel from communication latency actor to dst actor
    chCommDstActor = (TimedSDFchannel*) g->createChannel(
                            communicationLatencyActor, dstPort->getRate(), 
                            dstActor, dstPort->getRate(), 0);

    //Create channel from dst actor to communication latency actor
    chDstCommActor = (TimedSDFchannel*) g->createChannel(
                            dstActor, dstPort->getRate(), 
                            communicationLatencyActor, dstPort->getRate(), 0);
    chDstCommActor->setInitialTokens(dstPort->getRate());
    
    // Create channel from communication latency actor to credit actor
    chCommCreditActor = (TimedSDFchannel*) g->createChannel(
                            communicationLatencyActor, dstPort->getRate(), 
                            creditLatencyActor, 1, 0);
    
    // Create channel from credit latency actor to TDMA sync actor on src
    chCreditTDMAsrcActor = (TimedSDFchannel*) g->createChannel(
                            creditLatencyActor, 1, 
                            tdmaSrcSyncActor, 1, 0);
    
    // Create channel from TDMA sync actor on src to src actor
    chTDMAsrcSrcActor = (TimedSDFchannel*) g->createChannel(
                            tdmaSrcSyncActor, 1, 
                            srcActor, srcPort->getRate(), 0);
    chTDMAsrcSrcActor->setInitialTokens(ch->getBufferSize().src 
                                            - ch->getInitialTokens());                      
    if ((CSize)ch->getBufferSize().src < ch->getInitialTokens())
        throw CException("Insufficient memory space for initial tokens.");

    // Update the binding of actors to tiles
    actorBinding.resize(g->nrActors(), ACTOR_NOT_BOUND);
    actorBinding[communicationLatencyActor->getId()] = cn->getDstTile()->getId();

    // Extend the schedule on the destination processor with the communication
    // latency actor (this actor must be placed directly in front of the dst
    // actor).
    StaticOrderSchedule &s = schedule[cn->getDstTile()->getId()];
    
    for (StaticOrderScheduleEntryIter entry = s.begin(); 
            entry != s.end(); entry++)
    {
        if (entry->actor->getId() == dstActor->getId())
        {
            // Add schedule entry to the schedule
            entry = s.insertActor(entry, communicationLatencyActor);
            
            // Skip over newly added entry
            entry++;
        }
    }
}

/**
 * modelBindingInMPFlow ()
 * Model all binding decisions in a timed SDF graph.
 */
void BindingAwareSDFG::modelBindingInMPFlow(TimedSDFgraph *g, 
        PlatformGraph *pg)
{
    Tile *t, *srcTile, *dstTile;
    Connections channelBinding;
    Tiles actorBinding;
    Connection *c;
    
    // Copy all actors and channels of the constrained SDFG 'g' into 
    // this binding-aware graph
    // Step 1: properties of the graph
    setId(g->getId());
    setName(g->getName());
    setType(g->getType());

    // Step 2: copy of the actors
    for (SDFactorsCIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFcomponent component = SDFcomponent(this, nrActors());
        SDFactor *a = (*iter)->clone(component);
        addActor(a);
    }
    
    // Step 2: copy of the channels
    for (SDFchannelsCIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        SDFcomponent component = SDFcomponent(this, nrChannels());
        SDFchannel *ch = (*iter)->clone(component);
        addChannel(ch);
    }
    
    // Set the execution time of each actor to the correct processor
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
    
        // Tile
        t = pg->getTile(getBindingOfActorToTile(a));
    
        // Actor not bound to a tile?
        ASSERT(t != NULL, "All actors must be bound to a tile.");

        createMappedActorMPFlow((TimedSDFactor*)(getActor(a->getId())), t);
    }
    
    // Insert SDF model for each channel
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        TimedSDFchannel *ch = (TimedSDFchannel*)*iter;

        // Skip the channel if its a self-edge on an actor
        if (ch->getSrcActor()->getId() == ch->getDstActor()->getId())
            continue;
        
        // Tiles
        srcTile = pg->getTile(getBindingOfActorToTile(ch->getSrcActor()));
        dstTile = pg->getTile(getBindingOfActorToTile(ch->getDstActor()));
        
        // Source and destination actor bound to same tile?
        if (srcTile == dstTile)
        {
            // Channel should not be mapped to a connection
            ASSERT(getBindingOfChannelToConnection(ch) == CHANNEL_NOT_BOUND,
                    "Channel must not be bound to connection.");
        
            createMappedChannelToTileMPFlow((TimedSDFchannel*)
                                (getChannel(ch->getId())), srcTile);
        }
        else
        {
            // Channel must be bound to a connection
            ASSERT(getBindingOfChannelToConnection(ch) != CHANNEL_NOT_BOUND,
                    "Channel must be bound to connection.");
            
            // Find connection c used for channel ch
            c = pg->getConnection(getBindingOfChannelToConnection(ch));
                         
            // Channel must be bound to a connection from source to destination
            // tile
            if (c->getSrcTile() != srcTile || c->getDstTile() != dstTile)
            {
                throw CException("Channel bound to invalid connection.");
            }
            
            createMappedChannelToConnectionMPFlow((TimedSDFchannel*)
                                (getChannel(ch->getId())), c);
        }
    }
}

