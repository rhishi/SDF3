/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   loadbalance.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 11, 2006
 *
 *  Function        :   Load-balance algorithm
 *
 *  History         :
 *      11-04-06    :   Initial version.
 *
 * $Id: loadbalance.cc,v 1.4 2008/03/06 10:49:45 sander Exp $
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

#include "base/base.h"
#include "loadbalance.h"
#include "../../base/algo/cycle.h"
#include "../scheduling/scheduling.h"

// Scheduling strategy (default: list scheduling)
//#define _CREATE_STATIC_ORDER_SCHEDULES_WITH_PRIORITIES

/**
 * LoadBalanceBinding ()
 * Constructor.
 */
LoadBalanceBinding::LoadBalanceBinding(SDFflowType flowType) : Binding(flowType)
{
    maxCycleMean = NULL;
    tileLoad = NULL;
    setConstantsTileCostFunction(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
}

/**
 * ~LoadBalanceBinding ()
 * Destructor.
 */
LoadBalanceBinding::~LoadBalanceBinding()
{
    delete [] maxCycleMean;
    delete [] tileLoad;
}

/**
 * setAppGraph ()
 * Set the application graph. This function is overloaded from the base class.
 */
void LoadBalanceBinding::setAppGraph(TimedSDFgraph *g)
{
    // Class function in base class
    appGraph = g;
    
    // Compute repetition vector
    repVec = computeRepetitionVector(appGraph);
    
    // Cleanup existing cycle mean estimate
    delete [] maxCycleMean;
    maxCycleMean = NULL;

    // Initialize vector containing all actor to tile bindings
    actorTileBinding.resize(appGraph->nrActors());
    for (uint i = 0; i < appGraph->nrActors(); i++)
    {
        actorTileBinding[i] = NULL;
    }
}

/**
 * getConstantsTileCostFunction ()
 * The function returns all constants used in the tile cost function.
 */
void LoadBalanceBinding::getConstantsTileCostFunction(double &a, double &b,
        double &c, double &d, double &e, double &f, double &g, double &k, 
        double &l, double &m, double &n, double &o, double &p, double &q)
{
    a = cnst_a;
    b = cnst_b;
    c = cnst_c;
    d = cnst_d;
    e = cnst_e;
    f = cnst_f;
    g = cnst_g;
    k = cnst_k;
    l = cnst_l;
    m = cnst_m;
    n = cnst_n;
    o = cnst_o;
    p = cnst_p;
    q = cnst_q;
}

/**
 * setConstantsTileCostFunction ()
 * The function sets all constants used in the tile cost function.
 */
void LoadBalanceBinding::setConstantsTileCostFunction(double a, double b,
        double c, double d, double e, double f, double g, double k, double l,
        double m, double n, double o, double p, double q)
{
    cnst_a = a;
    cnst_b = b;
    cnst_c = c;
    cnst_d = d;
    cnst_e = e;
    cnst_f = f;
    cnst_g = g;
    cnst_k = k;
    cnst_l = l;
    cnst_m = m;
    cnst_n = n;
    cnst_o = o;
    cnst_p = p;
    cnst_q = q;
}

/**
 * estimateMaxCycleMean ()
 * The function estimates the maximum cycle mean of all actors in the
 * application graph.
 */
void LoadBalanceBinding::estimateMaxCycleMean()
{
    SDFgraphCycles cycles;
    double *actorWeight;
    double **channelWeight;

    // Cleanup existing estimates
    if (maxCycleMean != NULL)
        delete [] maxCycleMean;
    
    // Initialize
    maxCycleMean = new double [appGraph->nrActors()];
    for (uint i = 0; i < appGraph->nrActors(); i++)
        maxCycleMean[i] = 0;
    
    // Actor weight
    actorWeight = new double [appGraph->nrActors()];
    for (SDFactorsIter iter = appGraph->actorsBegin();
            iter != appGraph->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;
        double execTime = 0;
        
        // Average execution time
        for (TimedSDFactor::ProcessorsIter iterP = a->processorsBegin();
                iterP != a->processorsEnd(); iterP++)
        {
            TimedSDFactor::Processor *p = *iterP;
            
            execTime += p->execTime;
        }
        execTime = execTime / a->nrProcessors();
        
        actorWeight[a->getId()] = execTime * repVec[a->getId()];
    }

    // Channel weight
    channelWeight = new double* [appGraph->nrActors()];
    for (uint i = 0; i < appGraph->nrActors(); i++)
    {
        channelWeight[i] = new double [appGraph->nrActors()];

        for (uint j = 0; j < appGraph->nrActors(); j++)
            channelWeight[i][j] = 0;
    }
    for (SDFchannelsIter iter = appGraph->channelsBegin();
            iter != appGraph->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        CId srcActorId = c->getSrcActor()->getId();
        CId dstActorId = c->getDstActor()->getId();
        double weight;
        
        // Channel weight
        weight = c->getInitialTokens() / c->getDstPort()->getRate();
        
        // Weight of this channel larger then of any other channel between
        // same source and destination seen so far
        if (channelWeight[srcActorId][dstActorId] < weight)
            channelWeight[srcActorId][dstActorId] = weight;
    }
    
    // Find all simple cycles in the graph
    cycles = findSimpleCycles(appGraph);

    // Compute the cycle mean for each cycle
    for (SDFgraphCyclesIter iter = cycles.begin(); iter != cycles.end(); iter++)
    {
        SDFactor *srcActor, *dstActor;
        SDFactorsIter actorIter;
        double cycleMean;
        double weightChannels = 0;
        double weightActors = 0;
        SDFactors cycle = *iter;
        
        actorIter = cycle.begin();
        srcActor = *actorIter;
        dstActor = *(++actorIter);
        while (actorIter != cycle.end())
        {
            // Add weight of source actor and channel from source to destination
            weightActors += actorWeight[srcActor->getId()];
            weightChannels+=channelWeight[srcActor->getId()][dstActor->getId()];
            
            // Next channel in the cycle
            srcActor = dstActor;
            dstActor = *(++actorIter);
        }
        
        // srcActor points to last actor in the cycle. Its weight must be added,
        // as well as the weight of the channel to the first actor in the cycle.
        dstActor = cycle.front();
        weightActors += actorWeight[srcActor->getId()];
        weightChannels+=channelWeight[srcActor->getId()][dstActor->getId()];
        
        // Compute cycle mean
        cycleMean = weightActors / weightChannels;
        
        // Update maximum cycle mean for all actors on the cycle
        actorIter = cycle.begin();
        srcActor = *actorIter;
        while (actorIter != cycle.end())
        {
            // Cycle mean of this cycle larger then current maximum
            if (cycleMean > maxCycleMean[srcActor->getId()])
                maxCycleMean[srcActor->getId()] = cycleMean;
                
            // Next channel in the cycle
            srcActor = *(++actorIter);
        }
    }

    // Cleanup
    for (uint i = 0; i < appGraph->nrActors(); i++)
        delete [] channelWeight[i];
    delete [] channelWeight;
    delete [] actorWeight;
}

/**
 * sortActorsOnCriticality ()
 * The function computes for each actor the maximum cycle mean of all simple
 * cycles it is part of. The cycle mean is defined as the sum of the execution
 * times of the actors (in one period) divided by the normalized initial tokens
 * on the edges of the cycle. The function orders the actors according to their
 * maximum cycle mean (high to low).
 */
SDFactors LoadBalanceBinding::sortActorsOnCriticality()
{
    SDFactors sortedActors;

    // Cycle mean not estimated?
    if (maxCycleMean == NULL)
        estimateMaxCycleMean();
    
    // Sort actors based on maximum cycle mean (high to low)
    sortedActors = appGraph->getActors();
    sortOnCost(sortedActors, maxCycleMean);
    sortedActors.reverse();
   
    return sortedActors;
}

/**
 * sortTilesOnLoad ()
 * The function sorts the list of tiles from a low to a high load to which actor
 * a can be mapped.
 *
 * Cost of a tile is determined by:
 *  - the processor load measured in execution time of the mapped actors per
 *    iteration of the graph,
 *  - the memory requirements of the mapped actors, 
 *  - the memory requirements of the mapped communication channels,
 *  - the bandwidth requirements of the mapped communication channels,
 *  - the number of mapped communication channels,
 *  - the number of additional connections required when mapping the actor to
 *    the tile.
 *
 * When computing the cost of a tile, while considering the mapping of an
 * actor to the tile, the actor is considered to be mapped to the tile. It
 * is only checked wether the processor type inside the tile is supported by
 * the actor.
 *
 * The cost of a tile t is defined as:
 *
 *    cost(t) = a*ProcLoad^k + b*(MemAct+MemCh)^l + c*Bw^m 
                + d*NrConn^n + e*NrNewConn^o + g*latencyConn^q
 *
 * The constants a, b, c, d, e are used to scale the various properties
 * determining the cost wrt each other. The constants k, l, m, n, o are used to
 * increase cost for heavily loaded tiles.
 */
void LoadBalanceBinding::sortTilesOnLoad(TimedSDFactor *a, Tiles &tiles,
        double const_a, double const_b, double const_c, double const_d, 
        double const_e, double const_f, double const_g, double const_k, 
        double const_l, double const_m, double const_n, double const_o, 
        double const_p, double const_q)
{
    double procLoad, memLoad, inBwLoad, outBwLoad, bwLoad, connLoad, commLoad;
    double newConnLoad, maxProcLoad, maxConnBinding = 0;
    double *connBinding, *costOfTile;
    
    // Initialize
    connBinding = new double [archGraph->nrTiles()];
    costOfTile = new double [archGraph->nrTiles()];

    // Estimate the maximum, average processing load to scale procLoad
    maxProcLoad = 0;
    for (SDFactorsIter iter = appGraph->actorsBegin(); 
            iter != appGraph->actorsEnd(); iter++)
    {
        TimedSDFactor *b = (TimedSDFactor*) *iter;
        SDFtime maxExecTime = 0;
        
        for (TimedSDFactor::ProcessorsIter iterP = b->processorsBegin();
                iterP != b->processorsEnd(); iterP++)
        {
            if ((*iterP)->execTime > maxExecTime)
                maxExecTime = (*iterP)->execTime;
        }
        
        maxProcLoad = repVec[b->getId()] * maxExecTime;
    }
    maxProcLoad = maxProcLoad / (double) archGraph->nrTiles();

    // Compute resource usage and cost for each tile
    for (TilesIter iter = tiles.begin(); iter != tiles.end(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        Memory *m = t->getMemory();
        NetworkInterface *ni = t->getNetworkInterface();
        bool actorOnTile = false;
        
        // Can actor a be mapped to tile t?
        if (p != NULL && a->getProcessor(p->getType()) != NULL)
            actorOnTile = true;
        
        // Processing load of actors mapped to tile t
        procLoad = 0;
        for (SDFactorsIter iterA = appGraph->actorsBegin();
                iterA != appGraph->actorsEnd(); iterA++)
        {
            TimedSDFactor *b = (TimedSDFactor*) *iterA;
            Tile *tB = actorTileBinding[b->getId()];
            
            // Actor b mapped to tile t?
            if (tB != NULL && tB->getId() == t->getId())
            {
                // Add total execution time for executing actor b on tile t
                // to processing load of tile t
                procLoad += actorLoadOnTile(b, t);
            }
            
            // Actor b is same as actor a which is considered for mapping
            // and actor a can be mapped to tile t?
            if (a->getId() == b->getId() && actorOnTile)
            {
                // Add total execution time for executing actor a on tile t
                // to processing load of tile t
                procLoad += actorLoadOnTile(a, t);
            }
        }
        
        // Add penalty for time wheel occupation
        //procLoad = procLoad * (p->availableTimewheelSize() 
        //                                / (double) p->getTimewheelSize());
        //if (p->availableTimewheelSize() > 0)
        //    procLoad = procLoad * (p->getTimewheelSize() 
        //                            / (double) p->availableTimewheelSize());

        // Scale procLoad        
        procLoad = procLoad / maxProcLoad;
        
        // Memory
        memLoad = m->getSize() - m->availableMemorySize();
        if (actorOnTile)
        {
            // Is the memory reserved for actors not large enough?
            if (m->occupiedMemorySizeByActors() 
                    < a->getProcessor(p->getType())->stateSize)
            {
                // Increase memory needed for actor state
                memLoad = memLoad + a->getProcessor(p->getType())->stateSize
                            - m->occupiedMemorySizeByActors();
            }

            // Add memory needed for newly mapped channels            
            memLoad += memLoadChannelsOnTile(a, t);
        }
        memLoad = memLoad / (double) m->getSize(); 

        // Incoming bandwidth
        inBwLoad = ni->getInBandwidth() - ni->availableInBandwidth();
        if (actorOnTile)
            inBwLoad += bwChannelsMappedToInConnection(a, t);
        inBwLoad = inBwLoad / (double) ni->getInBandwidth();        
        
        // Outgoing bandwidth
        outBwLoad = ni->getOutBandwidth() - ni->availableOutBandwidth();
        if (actorOnTile)
            outBwLoad += bwChannelsMappedToOutConnection(a, t);
        outBwLoad = outBwLoad / (double) ni->getOutBandwidth();
        
        // Bandwidth
        if (inBwLoad > outBwLoad)
            bwLoad = inBwLoad;
        else
            bwLoad = outBwLoad;
            
        // Connections
        connLoad = ni->getNrConnections() - ni->availableNrConnections();
        if (actorOnTile)
            connLoad += nrChannelsMappedToConnection(a, t);
        connLoad = connLoad / (double) ni->getNrConnections();
        
        // New connections
        newConnLoad = 0;
        if (actorOnTile)
            newConnLoad = nrChannelsMappedToConnection(a, t);
        newConnLoad = newConnLoad / (double) a->nrPorts();
        
        // Communication load
        commLoad = (inBwLoad + outBwLoad + connLoad) / 3.0;
        
        // Connection binding
        connBinding[t->getId()] = computeLoadOfChannelToConnectionBinding(a, t);
        if (connBinding[t->getId()] > maxConnBinding)
            maxConnBinding = connBinding[t->getId()];
        
        // Cost of tile t
        costOfTile[t->getId()]  = const_a * pow(procLoad, const_k);
        costOfTile[t->getId()] += const_b * pow(memLoad, const_l);
        costOfTile[t->getId()] += const_c * pow(bwLoad, const_m);
        costOfTile[t->getId()] += const_d * pow(connLoad, const_n);
        costOfTile[t->getId()] += const_e * pow(newConnLoad, const_o);
        costOfTile[t->getId()] += const_f * pow(commLoad, const_p);
    }

    // Add cost of connections (latency) to the cost of every tile
    for (TilesIter iter = tiles.begin(); iter != tiles.end(); iter++)
    {
        Tile *t = *iter;
        
        // Connection binding (normalize wrt to maximal load)
        connBinding[t->getId()] = connBinding[t->getId()] / maxConnBinding;
        
        // Cost of tile t
        costOfTile[t->getId()] += const_g 
                                    * pow(connBinding[t->getId()], const_q);
    }

    // Sort tiles on cost
    sortOnCost(tiles, costOfTile);
    
    // Cleanup
    delete [] costOfTile;
    delete [] connBinding;
}

/**
 * sortTilesOnCommunicationOverhead ()
 * The function takes as input a list of tiles ordered based on the processor
 * load and an actor when can be mapped to all tiles in the list. The functions
 * considers reordering the tiles based on the expected communication overhead
 * (connection delay + timewheel synchronization). When the communication
 * overhead of mapping actor a to tile t1 exceeds the gain of mapping actor a
 * onto a different tile t2, the tiles t1 and t2 are reordered.
 */
void LoadBalanceBinding::sortTilesOnCommunicationOverhead(TimedSDFactor *a,
        Tiles &tiles)
{
    double remapCommFactor;
    double *costOfTile;
    bool *visitedActor;

    // Factor which determines the weight with which the communication overhead
    // is corrected when the overhead can be resolved by remapping an already
    // mapped actor to a different tile (e.g. a valued of 0.5 means count only
    // overhead for 50%).
    remapCommFactor = 0.5;

    // Find all tiles which have a processor supported by the actor
    for (TilesIter iter = tiles.begin(); iter != tiles.end();)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        
        // Processor type of tile t not supported by actor a?
        if (p == NULL || a->getProcessor(p->getType()) == NULL)
            tiles.erase(iter);
        else
            iter++;
    }        
    
    // Initialize cost of tiles
    costOfTile = new double [archGraph->nrTiles()];
    for (TilesIter iter = tiles.begin(); iter != tiles.end(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        
        // Tile contains a processor
        if (p != NULL)
        {
            costOfTile[t->getId()] = tileLoad[t->getId()]
                    + a->getExecutionTime(p->getType()) * repVec[a->getId()];
        }
    }
    
    // Initialize list of visited actors
    visitedActor = new bool [appGraph->nrActors()];
    for (uint i = 0; i < appGraph->nrActors(); i++)
        visitedActor[i] = false;
        
    // Consider all actors connected with a channel to actor a
    for (SDFportsIter iterA = a->portsBegin(); iterA != a->portsEnd(); iterA++)
    {
        SDFport *p = *iterA;
        SDFchannel *c = p->getChannel();
        TimedSDFactor *b = (TimedSDFactor*)c->oppositePort(p)->getActor();
        
        // Seen other channel between actors a and b before?
        if (visitedActor[b->getId()])
            continue;
        
        // Iterate over all tiles to which actor a can be mapped
        for (TilesIter iterT = tiles.begin(); iterT != tiles.end(); iterT++)
        {
            Tile *tileA = *iterT;
            Tile *tileB = actorTileBinding[b->getId()];
            
            // Tile A contains no processor? then no mapping possible
            if (tileA->getProcessor() == NULL)
                continue;
            
            // Is actor b mapped, but not to tile t?
            if (tileB != NULL && tileB != tileA)
            {
                double commOverhead = 0;
                Connection *co;
                
                // Find connection in architecture graph to be used
                if (p->getType() == SDFport::In)
                {
                    co = archGraph->getConnection(tileB, tileA);
                }
                else
                {
                    co = archGraph->getConnection(tileA, tileB);
                }
                
                // Compute communication overhead 
                //  overhead = latency connection + max over all channels of
                // (token size * #tokens per firing * #firings per period) 
                // divided by the bandwidth of the connection.
                
                // Find maximum over all channels which connect actor a and b
                commOverhead = 0;
                for (SDFportsIter iterB = b->portsBegin();
                        iterB != b->portsEnd(); iterB++)
                {
                    SDFport *pB = *iterB;
                    TimedSDFchannel *ch = (TimedSDFchannel*)pB->getChannel();
                    double commOverheadCh;
                    
                    // Does the channel connected to port pB connect to actor a?
                    if (ch->oppositePort(pB)->getActor()->getId() == a->getId())
                    {
                        commOverheadCh = ch->getTokenSize() * pB->getRate();
                        commOverheadCh = commOverheadCh * repVec[b->getId()];
                        commOverheadCh = commOverheadCh / ch->getMinBandwidth();
                        commOverheadCh = ceil(commOverheadCh);
                        
                        if (commOverheadCh > commOverhead)
                            commOverhead = commOverheadCh;
                    }
                }
                
                // Add delay of the connection to the overhead
                commOverhead += co->getLatency();
                
                // Can actor b be mapped to tile tileA?
                if (b->getProcessor(tileA->getProcessor()->getType()) != NULL)
                {
                    // Let's reduce the impact of the communication as it can be
                    // resolved
                    commOverhead = remapCommFactor * commOverhead;
                }
                
                costOfTile[tileA->getId()] += commOverhead;
            }
        }
        
        // Mark actor b (connected to actor a) as visited
        visitedActor[b->getId()] = true;
    }
    
    // Sort tiles on cost
    sortOnCost(tiles, costOfTile, 1, archGraph->nrTiles());
    
    // Cleanup
    delete [] costOfTile;
    delete [] visitedActor;
}

/**
 * computeLoadOfChannelToConnectionBinding ()
 * The function computes the cost of binding actor a to tile t in terms of the
 * latency of the created connections. It computes the sum of the latency of
 * all channels that are bound to a connection by binding actor a to tile t.
 */
double LoadBalanceBinding::computeLoadOfChannelToConnectionBinding(SDFactor *a,
        Tile *t)
{
    PlatformGraph* archGraph = (PlatformGraph*)(t->getParent());
    double connBindingCost = 0;
    Connection *c;
    
    for (SDFportsIter iterP = a->portsBegin();
            iterP != a->portsEnd(); iterP++)
    {
        SDFport *p = *iterP;
        SDFactor *b; 

        // Opposite actor connected to channel c
        b = p->getChannel()->oppositePort(p)->getActor();

        // Actor b mapped to a tile not equal to tile t?
        if (actorTileBinding[b->getId()] != NULL 
                && actorTileBinding[b->getId()]->getId() != t->getId())
        {
            
            if (p->getType() == SDFport::In)
                c = archGraph->getConnection(actorTileBinding[b->getId()], t);
            else
                c = archGraph->getConnection(t, actorTileBinding[b->getId()]);

            connBindingCost += c->getLatency();
        }
    }
        
    return connBindingCost;
}

/**
 * memLoadChannelsOnTile ()
 * The function returns the memory size needed to map all channels connected to
 * actor a when the actor is mapped to tile t. The size of a channel is only
 * considered when the other actor which is connected to the channel is already
 * mapped to a tile.
 */
CSize LoadBalanceBinding::memLoadChannelsOnTile(SDFactor *a, Tile *t)
{
    CSize mem = 0;
    
    // Iterate over all channels connected to actor a
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        TimedSDFchannel *c = (TimedSDFchannel*) p->getChannel();
        SDFactor *b = c->oppositePort(p)->getActor();
        Tile *tB = actorTileBinding[b->getId()];
        
        // Actor b already mapped to a tile?
        if (tB != NULL)
        {
            // Actor b mapped to same tile as actor a?
            if (t->getId() == tB->getId())
            {
                mem += c->getBufferSize().mem * c->getTokenSize();
            }
            else
            {
                // Actor a source of the channel?
                if (p->getType() == SDFport::Out)
                {
                    mem += c->getBufferSize().src * c->getTokenSize();
                }
                else
                {
                    mem += c->getBufferSize().dst * c->getTokenSize();
                }
            }
        }
    }
    
    return mem;
}  

/**
 * bwChannelsMappedToInConnection ()
 * The function returns the bandwidth of channels connected to the actor a which
 * will be mapped to an ingoing connection if the actor is mapped to tile t.
 * Channels of which the other actor is not mapped are ignored.
 */
double LoadBalanceBinding::bwChannelsMappedToInConnection(SDFactor *a, Tile *t)
{
    double bw = 0;
    
    // Iterate over all channels connected to actor a
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        TimedSDFchannel *c = (TimedSDFchannel*) p->getChannel();
        SDFactor *b;
        
        // Port p is an input port?
        if (p->getType() == SDFport::In)
        {
            // Opposite actor connected to channel c
            b = c->oppositePort(p)->getActor();

            // Actor b mapped to a tile not equal to tile t?
            if (actorTileBinding[b->getId()] != NULL 
                    && actorTileBinding[b->getId()]->getId() != t->getId())
            {
                bw += c->getMinBandwidth();
            }
        }
    }
    
    return bw;
}  

/**
 * bwChannelsMappedToOutConnection ()
 * The function returns the bandwidth of channels connected to the actor a which
 * will be mapped to an outgoing connection if the actor is mapped to tile t.
 * Channels of which the other actor is not mapped are ignored.
 */
double LoadBalanceBinding::bwChannelsMappedToOutConnection(SDFactor *a, Tile *t)
{
    double bw = 0;
    
    // Iterate over all channels connected to actor a
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        TimedSDFchannel *c = (TimedSDFchannel*) p->getChannel();
        SDFactor *b;
        
        // Port p is an output port?
        if (p->getType() == SDFport::Out)
        {

            // Opposite actor connected to channel c
            b = c->oppositePort(p)->getActor();

            // Actor b mapped to a tile not equal to tile t?
            if (actorTileBinding[b->getId()] != NULL 
                    && actorTileBinding[b->getId()]->getId() != t->getId())
            {
                bw += c->getMinBandwidth();
            }
        }
    }
    
    return bw;
}  

/**
 * nrChannelsMappedToConnection ()
 * The function returns the number of channels connected to the actor a which
 * will be mapped to a connection if the actor is mapped to tile t. Channels of
 * which the other actor is not mapped are ignored.
 */
int LoadBalanceBinding::nrChannelsMappedToConnection(SDFactor *a, Tile *t)
{
    int nrConnections = 0;
    
    // Iterate over all channels connected to actor a
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        SDFactor *b;
        
        // Opposite actor connected to channel c
        b = c->oppositePort(p)->getActor();
        
        // Actor b mapped to a tile not equal to tile t?
        if (actorTileBinding[b->getId()] != NULL 
                && actorTileBinding[b->getId()]->getId() != t->getId())
        {
            nrConnections++;
        }
    }
    
    return nrConnections;
}  

/**
 * initTileLoad ()
 * Initially all tiles are not loaded.
 */
void LoadBalanceBinding::initTileLoad()
{
    // Cleanup
    if (tileLoad != NULL)
        delete [] tileLoad;
    
    // Initialize
    tileLoad = new double [archGraph->nrTiles()];
    for (uint i = 0; i < archGraph->nrTiles(); i++)
        tileLoad[i] = 0;
}

/**
 * actorLoadOnTile ()
 * The function returns the load of an actor on a tile. The load is defined as
 * the execution time of the actor per firing times the number of firings
 * required per period.
 */
double LoadBalanceBinding::actorLoadOnTile(TimedSDFactor *a, Tile *t)
{
    Processor *p = t->getProcessor();
    
    // Tile contains no processor?
    if (p == NULL)  
        return 0;
    
    return a->getProcessor(p->getType())->execTime * repVec[a->getId()];
}

/**
 * increaseLoadTile ()
 * Add the load of an actor to a tile. The load is defined as the execution
 * time of the actor per firing times the number of firings required per period.
 */
void LoadBalanceBinding::increaseLoadTile(TimedSDFactor *a, Tile *t)
{
    tileLoad[t->getId()] += actorLoadOnTile(a, t);
}

/**
 * decreaseLoadTile ()
 * Remove the load of an actor from a tile. The load is defined as the execution
 * time of the actor per firing times the number of firings required per period.
 */
void LoadBalanceBinding::decreaseLoadTile(TimedSDFactor *a, Tile *t)
{
    tileLoad[t->getId()] -= actorLoadOnTile(a, t);
    
    ASSERT(tileLoad[t->getId()] >= 0, "tileLoad cannot be negative.");
}

/**
 * isActorBound ()
 * The function returns true if an actor is bound to a tile. Otherwise it
 * returns false.
 */
bool LoadBalanceBinding::isActorBound(const SDFactor *a) const
{
    if (actorTileBinding[a->getId()] != NULL)
        return true;
    
    return false;
}

/**
 * isChannelBound ()
 * The function returns true if a channel is both the source and destination
 * actor are bound to the a tile. Otherwise it returns false.
 */
bool LoadBalanceBinding::isChannelBound(const SDFchannel *c) const
{
    Tile *src = actorTileBinding[c->getSrcActor()->getId()];
    Tile *dst = actorTileBinding[c->getDstActor()->getId()];

    if (src != NULL && dst != NULL)
        return true;
    
    return false;
}

/**
 * isChannelBoundToTile ()
 * The function returns true if a channel is both the source and destination
 * actor are bound to the same tile. Otherwise it returns false.
 */
bool LoadBalanceBinding::isChannelBoundToTile(const SDFchannel *c) const
{
    Tile *src = actorTileBinding[c->getSrcActor()->getId()];
    Tile *dst = actorTileBinding[c->getDstActor()->getId()];

    if (src != NULL && dst != NULL && src == dst)
        return true;
    
    return false;
}

/**
 * isChannelBoundToConnection ()
 * The function returns true if a channel is both the source and destination
 * actor are bound to the different tiles. Otherwise it returns false.
 */
bool LoadBalanceBinding::isChannelBoundToConnection(const SDFchannel *c) const
{
    Tile *src = actorTileBinding[c->getSrcActor()->getId()];
    Tile *dst = actorTileBinding[c->getDstActor()->getId()];

    if (src != NULL && dst != NULL && src != dst)
        return true;
    
    return false;
}

/**
 * releaseResources ()
 * The function releases all resources allocated by a application.
 */
void LoadBalanceBinding::releaseResources()
{
    for (SDFactorsIter iter = appGraph->actorsBegin();
            iter != appGraph->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*) (*iter);
        Tile *t = actorTileBinding[a->getId()];

        // Actor bound to tile?
        if (t != NULL)
        {
            // Release resources reserved by actor (including channels)
            releaseResources(a, t);
        }
    }
}

/**
 * releaseConnectionResources ()
 * The function releases all connection resources allocated by a channel.
 */
void LoadBalanceBinding::releaseConnectionResources(TimedSDFchannel *c)
{
    NetworkInterface *srcNI, *dstNI;
    Memory *srcMem, *dstMem;
    Tile *srcTile, *dstTile;
    Connections connections;
        
    // Source and destination tiles
    srcTile = actorTileBinding[c->getSrcActor()->getId()];
    dstTile = actorTileBinding[c->getDstActor()->getId()];

    // Release memory allocations
    srcMem = srcTile->getMemory();
    srcMem->releaseMemory(c);

    dstMem = dstTile->getMemory();
    dstMem->releaseMemory(c);

    // Release network interface allocations
    srcNI = srcTile->getNetworkInterface();
    srcNI->releaseConnection(c);

    dstNI = dstTile->getNetworkInterface();
    dstNI->releaseConnection(c);

    // Remove binding of channel to connection
    connections = archGraph->getConnections(srcTile, dstTile);
    for (ConnectionsIter iter = connections.begin();
        iter != connections.end(); iter++)
    {
        Connection *co = *iter;
        co->unbindChannel(c);
    }            

    #ifdef VERBOSE
    cerr << "Removed binding of channel '" << c->getName() << "' to connection";
    cerr << " from '" << srcTile->getName() << "' to '" << dstTile->getName();
    cerr << "'." << endl;
    #endif
}

/**
 * releaseResources ()
 * The function releases all resources allocated by an actor on a tile t. This
 * includes possible connections to other actors.
 */
void LoadBalanceBinding::releaseResources(TimedSDFactor *a, Tile *t)
{
    Processor *p;
    Memory *m;
    
    // Components inside the tile
    p = t->getProcessor();
    m = t->getMemory();

    // Release memory allocation
    m->releaseMemory(a);
    
    // Release reservations for channels
    for (SDFportsIter portIter = a->portsBegin();
            portIter != a->portsEnd(); portIter++)
    {
        SDFport *po = *portIter;
        TimedSDFchannel *ch = (TimedSDFchannel*)po->getChannel();
        
        if (isChannelBoundToTile(ch))
        {
            // Release memory allocation
            m->releaseMemory(ch);

            #ifdef VERBOSE
            cerr << "Removed binding of channel '" << ch->getName() << "' to ";
            cerr << "tile '" << t->getName() << "'." << endl;
            #endif

        }
        else if (isChannelBoundToConnection(ch))
        {
            // Release connection allocations
            releaseConnectionResources(ch);
        }
    }

    // Release binding of actor to processor
    p->unbindActor(a);

    // Release time slice allocation on processor
    p->releaseTimeSlice();
        
    // Update binding of actor to tile
    actorTileBinding[a->getId()] = NULL;

    // Update load on tile
    decreaseLoadTile(a, t);

    #ifdef VERBOSE
    cerr << "Removed binding of actor '" << a->getName() << "' to tile '";
    cerr << t->getName() << "'." << endl;
    #endif
}

/**
 * allocateConnectionResources ()
 * The function allocates all connection resources needed by a channel.
 */
bool LoadBalanceBinding::allocateConnectionResources(TimedSDFchannel *c)
{
    NetworkInterface *srcNI, *dstNI;
    Memory *srcMem, *dstMem;
    Tile *srcTile, *dstTile;
    Connections connections;
    bool boundChannel;
        
    // Source and destination tiles
    srcTile = actorTileBinding[c->getSrcActor()->getId()];
    dstTile = actorTileBinding[c->getDstActor()->getId()];

    // Bind channel to connection
    boundChannel = false;
    connections = archGraph->getConnections(srcTile, dstTile);
    for (ConnectionsIter iter = connections.begin();
        iter != connections.end() && !boundChannel; iter++)
    {
        Connection *co = *iter;

        if (co->bindChannel(c))
            boundChannel = true;
    }            
    if (!boundChannel)
        return false;

    // Allocate memory
    srcMem = srcTile->getMemory();
    if (!srcMem->reserveMemory(c, c->getBufferSize().src * c->getTokenSize()))
    {
        #ifdef VERBOSE
        cerr << "Not enough memory in source tile." << endl;
        #endif
        return false;
    }

    dstMem = dstTile->getMemory();
    if (!dstMem->reserveMemory(c, c->getBufferSize().dst * c->getTokenSize()))
    {
        #ifdef VERBOSE
        cerr << "Not enough memory in destination tile." << endl;
        #endif
        return false;
    }
        
    // Allocate network interface bandwidth and connection
    srcNI = srcTile->getNetworkInterface();
    if (!srcNI->reserveConnection(c, 1, 0, c->getMinBandwidth()))
    {
        #ifdef VERBOSE
        cerr << "Not enough bandwidth in source tile." << endl;
        #endif
        return false;
    }

    dstNI = dstTile->getNetworkInterface();
    if (!dstNI->reserveConnection(c, 1, c->getMinBandwidth(), 0))
    {
        #ifdef VERBOSE
        cerr << "Not enough bandwidth in destination tile." << endl;
        #endif
        return false;
    }

    #ifdef VERBOSE
    cerr << "Bound channel '" << c->getName() << "' to connection";
    cerr << " from '" << srcTile->getName() << "' to '" << dstTile->getName();
    cerr << "'." << endl;
    #endif
    
    return true;
}

/**
 * allocateResources ()
 * The function attempts to allocate resources for actor a on the tile t.
 * The allocated time slice is equal to the actors execution time.
 */
bool LoadBalanceBinding::allocateResources(TimedSDFactor *a, Tile *t)
{
    NetworkInterface *ni;
    Processor *p;
    Memory *m;
    
    // Components inside the tile
    ni = t->getNetworkInterface();
    p = t->getProcessor();
    m = t->getMemory();
    
    // No processor available?
    if (p == NULL)
        return false;
    
    // No time slice available?
    if (p->availableTimewheelSize() == 0)
        return false;
    
    // Bind actor to tile
    if (!p->bindActor(a))
        return false;

    // Actor is now mapped to tile; update binding of actor to tile
    actorTileBinding[a->getId()] = t;

    // Update load on tile
    increaseLoadTile(a, t);
    
    // Allocate memory for the actor
    if (!m->reserveMemory(a, a->getStateSize()))
    {
        #ifdef VERBOSE
        cerr << "Failed allocating memory for actor '";
        cerr << a->getName() << "' on tile '";
        cerr << t->getName() << "'." << endl;
        #endif

        // Release allocations
        releaseResources(a, t);
        
        return false;
    }
    
    #ifdef VERBOSE
    cerr << "Bound actor '" << a->getName() << "' to tile '";
    cerr << t->getName() << "'." << endl;
    #endif
    
    // Allocate network interface resources (if needed)
    for (SDFportsIter portIter = a->portsBegin();
            portIter != a->portsEnd(); portIter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)((*portIter)->getChannel());
        
        // Is connected actor b already mapped?
        if (isChannelBoundToTile(c))
        {
            // Allocate memory for channel in tile
            if (!m->reserveMemory(c, c->getBufferSize().mem 
                                                        * c->getTokenSize()))
            {
                #ifdef VERBOSE
                cerr << "Failed allocating connection for channel '";
                cerr << c->getName() << "' in tile '";
                cerr << t->getName() << "'." << endl;
                #endif
                
                // Release allocations
                releaseResources(a, t);
                
                return false;
            }
            
            #ifdef VERBOSE
            cerr << "Bound channel '" << c->getName() << "' to tile '";
            cerr << t->getName() << "'." << endl;
            #endif
        }
        else if (isChannelBoundToConnection(c))
        {
            if (!allocateConnectionResources(c))
            {
                #ifdef VERBOSE
                cerr << "Failed allocating connection for channel '";
                cerr << c->getName() << "' from between tiles.";
                cerr << endl;
                #endif
                
                // Release allocations
                releaseResources(a, t);
                
                return false;
            }
        }
    }

    // Done
    return true; 
}

/**
 * moveActorBinding ()
 * The function moves an actor from one processor to another processor. This
 * operation invalidates the schedules of the involved processors.
 */
bool LoadBalanceBinding::moveActorBinding(TimedSDFactor *a,
        bool allowExistingTile)
{
    Tile *oldTile;
    Tiles tiles;
    
    // Remove old binding of actor to tile
    oldTile = actorTileBinding[a->getId()];
	releaseResources(a, oldTile);

    // Sort tiles based on load
    tiles = archGraph->getTiles();
    sortTilesOnLoad(a, tiles, cnst_a, cnst_b, cnst_c, cnst_d, cnst_e, cnst_f,
                            cnst_g, cnst_k, cnst_l, cnst_m, cnst_n, cnst_o,
                            cnst_p, cnst_q);

    // Try resource allocation on tiles
    for (TilesIter tileIter = tiles.begin();
            tileIter != tiles.end(); tileIter++)
    {
        Tile *t = *tileIter;

        // Mapping of actor to old tile not allowed?
        if (!allowExistingTile && oldTile->getId() == t->getId())
            continue;

        // Successfully allocated resources for the actor on the tile?
        if (allocateResources(a, t))
        {
            #ifdef VERBOSE
            cerr << "Moved actor '" << a->getName() << "' from tile '";
            cerr << oldTile->getName() << "' to tile '";
            cerr << t->getName() << "'." << endl;
            #endif

            // Invalidate schedule of involved tiles (if needed)
            if (oldTile->getId() != t->getId())
            {
                oldTile->getProcessor()->getSchedule().clear();
	            t->getProcessor()->getSchedule().clear();
            }
            
            return true;
        }
    }

    // No resources allocated for the actor, keep original binding
    allocateResources(a, oldTile);

    #ifdef VERBOSE
    cerr << "Failed allocating resources for actor '";
    cerr << a->getName();
    cerr << "'. Original binding preserved." << endl;
    #endif

    return false;
}

/**
 * changeBandwidthAllocation ()
 * The function modifies the bandwidth allocation of a channel. On success, it
 * return true. On failure, it returns false and the channel keeps the original
 * bandwidth assignment.
 */
bool LoadBalanceBinding::changeBandwidthAllocation(TimedSDFchannel *c,
    double bw)
{
    NetworkInterface *srcNI, *dstNI;
    ComponentBinding *binding;
    Tile *srcTile, *dstTile;
    double curBw;
    
    // Bandwidth below required minimum?
    if (bw < c->getMinBandwidth())
        return false;

    // Source and destination network interface
    srcTile = actorTileBinding[c->getSrcActor()->getId()];
    dstTile = actorTileBinding[c->getDstActor()->getId()];
    srcNI = srcTile->getNetworkInterface();
    dstNI = dstTile->getNetworkInterface();

    // Current bandwidth allocated to channel binding
    binding = srcNI->getBindings()->find(c);
    curBw = binding->getValue(NetworkInterface::outBw);

    // Enough bandwidth available on source and destination NI
    if (srcNI->availableOutBandwidth() + curBw < bw
            || dstNI->availableInBandwidth() + curBw < bw)
    {
        return false;
    }
        
    // Remove existing binding
    srcNI->releaseConnection(c);
    dstNI->releaseConnection(c);

    // Allocate network interface bandwidth and connection
    srcNI->reserveConnection(c, 1, 0, bw);
    dstNI->reserveConnection(c, 1, bw, 0);

    #ifdef VERBOSE
    cerr << "Changed bandwidth of channel '" << c->getName() << "' from ";
    cerr << curBw << " to " << bw << "." << endl;
    #endif
    
    return true;
}

/**
 * changeSlotAllocation ()
 * The function changes the size of the TDMA slot allocation of the tile to
 * the given size. On failure, the original allocation is maintained and false
 * is returned. On success, the function returns true.
 */
bool LoadBalanceBinding::changeSlotAllocation(Tile *t, CSize sz)
{
    Processor *p = t->getProcessor();
    
    // Tile contains no processor?
    if (p == NULL)
        return false;
        
    // Not enough free space available for new allocation?
    if (p->availableTimewheelSize() + p->getReservedTimeSlice() < sz)
    {
        return false;
    }
    
    // Reserve TDMA time slice
    p->releaseTimeSlice();
    p->reserveTimeSlice(sz);
    
    return true;
}
 
/**
 * bindActorsToTiles ()
 * Bind each actor to a tile. Actors are handled in order of their criticality
 * and tiles are tried in order of their load. The objective is to spread the
 * load evenly over all tiles.
 */
bool LoadBalanceBinding::bindActorsToTiles()
{
    SDFactorsIter actorIter;
    SDFactors actors;
    
    // Sort actors based on criticality
    actors = sortActorsOnCriticality();
    
    // Map actors one-by-one
    actorIter = actors.begin();
    while (actorIter != actors.end())
    {
        TimedSDFactor *a = (TimedSDFactor*)(*actorIter);
        bool mappedActor = false;
        Tiles tiles;

        // Sort tiles based on load
        tiles = archGraph->getTiles();
        sortTilesOnLoad(a, tiles, cnst_a, cnst_b, cnst_c, cnst_d, cnst_e,
                            cnst_f, cnst_g, cnst_k, cnst_l, cnst_m, cnst_n,
                            cnst_o, cnst_p, cnst_q);
        
        // Try resource allocation on tiles
        for (TilesIter tileIter = tiles.begin();
                tileIter != tiles.end(); tileIter++)
        {
            Tile *t = *tileIter;
            
            // Successfully allocated resources for the actor on the tile?
            if (allocateResources(a, t))
            {
                mappedActor = true;
                break;
            }
        }
    
        // No resources allocated for the actor?
        if (!mappedActor)
        {
            #ifdef VERBOSE
            cerr << "Failed allocating resources for actor '";
            cerr << a->getName();
            cerr << "'." << endl;
            #endif
            
            return false;
        }
    
        // Next actor
        actorIter++;
    }

    return true;
}

/**
 * constructStaticOrderSchedules ()
 * The function generates a static-order schedule for each processor in the
 * architecture graph. The schedules order the firing of the actors in the
 * application graph. They respects the buffer requirements and mapping of all 
 * actors.
 */
void LoadBalanceBinding::constructStaticOrderSchedules()
{
    BindingAwareSDFG *bindingAwareSDFG;

#ifdef _CREATE_STATIC_ORDER_SCHEDULES_WITH_PRIORITIES
    SDFstateSpacePriorityListScheduler listScheduler;
#else
    SDFstateSpaceListScheduler listScheduler;
#endif

    // Assume that 50% of available tdma wheel is allocated for graph
    reserveTimeSlices(.5);

    // Create a binding-aware SDFG
    bindingAwareSDFG = new BindingAwareSDFG(appGraph, archGraph, getFlowType());

    // Construct static-order schedules
    listScheduler.schedule(bindingAwareSDFG);

    // Assign schedules to processors in the tiles
    for (uint i = 0; i < bindingAwareSDFG->nrTilesInPlatformGraph(); i++)
    {
        Tile *t = archGraph->getTile(i);
        Processor *p = t->getProcessor();
        
        p->setSchedule(bindingAwareSDFG->getScheduleOnTile(i));
    }

    // Release the allocated time slices
    releaseTimeSlices();

    // Actors used in the schedule of the processors come from
    // 'mappedAppGraph'. These must be mapped to actors from 'appGraph'.
    for (TilesIter iter = archGraph->tilesBegin();
            iter != archGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        
        // Processor inside the tile?
        if (p != NULL)
        {
            p->getSchedule().changeActorAssociations(appGraph);
            
            logMsg("Schedule on '" + t->getName() + "' has "
                   + CString(p->getSchedule().size()) + " states.");
        }
    }

    // Cleanup
    delete bindingAwareSDFG;
}

/**
 * reconstructStaticOrderSchedules ()
 * The function generates a static-order schedule for each processor in the
 * architecture graph. The schedules order the firing of the actors in the
 * application graph. They respects the buffer requirements and mapping of all 
 * actors.
 */
void LoadBalanceBinding::reconstructStaticOrderSchedules()
{
    BindingAwareSDFG *bindingAwareSDFG;

#ifdef _CREATE_STATIC_ORDER_SCHEDULES_WITH_PRIORITIES
    SDFstateSpacePriorityListScheduler listScheduler;
#else
    SDFstateSpaceListScheduler listScheduler;
#endif
    
    // Create a binding-aware SDFG
    bindingAwareSDFG = new BindingAwareSDFG(appGraph, archGraph, getFlowType());

    // Construct static-order schedules
    listScheduler.schedule(bindingAwareSDFG);
    
    // Assign schedules to processors in the tiles
    for (uint i = 0; i < bindingAwareSDFG->nrTilesInPlatformGraph(); i++)
    {
        Tile *t = archGraph->getTile(i);
        Processor *p = t->getProcessor();
        
        p->setSchedule(bindingAwareSDFG->getScheduleOnTile(i));
    }

    // Actors used in the schedule of the processors come from
    // 'mappedAppGraph'. These must be mapped to actors from 'appGraph'.
    for (TilesIter iter = archGraph->tilesBegin();
            iter != archGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        
        // Processor inside the tile?
        if (p != NULL)
        {
            p->getSchedule().changeActorAssociations(appGraph);
            
            logMsg("Schedule on '" + t->getName() + "' has "
                   + CString(p->getSchedule().size()) + " states.");
        }
    }

    // Cleanup
    delete bindingAwareSDFG;
}

/**
 * optimizeActorToTileBindings ()
 * The function moves actors between tiles to achieve an optimal load balance.
 * It considers both computation and communication cost when moving actors.
 */
void LoadBalanceBinding::optimizeActorToTileBindings()
{
    SDFactors actors = appGraph->getActors();
    uint nrActors = appGraph->nrActors();
    bool updateCost = true;
    double *actorCost;
    
    // Allocate memory for storing actorCost
    actorCost = new double [nrActors * 2];
    
    while (updateCost)
    {
        // Compute cost of actors considering the tile bindings
        for (SDFactorsIter iter = actors.begin(); iter != actors.end(); iter++)
        {
            SDFactor *a = *iter;
            Tile *t = actorTileBinding[a->getId()];

            // Actor mapped to a tile?
            if (t != NULL)
            {
                // Dimension 1: tile load
                actorCost[a->getId()] = 1.0 / tileLoad[t->getId()];

                // Dimension 2: actor criticallity
                actorCost[nrActors + a->getId()] = maxCycleMean[a->getId()];
            }
        }

        // Sort actors on cost
        sortOnCost(actors, actorCost, 2, nrActors);

        // Cost are up-to-date
        updateCost = false;

        // Traverse actors in order and try to move them to a new tile
        for (SDFactorsIter iter = actors.begin(); 
                !updateCost && iter != actors.end(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*) *iter;

            // Try to move actor to new tile
            if (moveActorBinding(a, true))
            {
                // Remove all actors already considered from list and update
                // cost of remaining actors
                actors.erase(actors.begin(), ++iter);
                updateCost = true;
            }
        }
    }

    // Cleanup
    delete [] actorCost;
}

/**
 * reserveTimeSlices ()
 * The function reserves a TDMA time slice of fraction of the available wheel
 * size on each tile which contains actors of the application graph.
 */
void LoadBalanceBinding::reserveTimeSlices(double fraction)
{
    CSize slice;
    
    ASSERT(fraction >= 0 && fraction <= 1, "Fraction must be within [0,1]");
    
    // Reserve slice for each tile
    for (TilesIter iter = archGraph->tilesBegin(); 
            iter != archGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        
        // No processor and/or no actor mapped to tile?
        if (p == NULL || tileLoad[t->getId()] == 0)
            continue;

        // Slice is fraction of available time wheel
        slice = CSize(fraction * (double)(p->availableTimewheelSize()));

        // At least 1 slot is needed
        if (slice < 1)
            slice = 1;
        
        // Reserve time slice
        p->reserveTimeSlice(slice);
        
        #ifdef VERBOSE
        double percTile = p->getReservedTimeSlice() 
                                        / (double) p->getTimewheelSize();
        cerr << "Reserved " << 100.0 * percTile << "% of time wheel tile '";
        cerr << t->getName() << "'" << endl;
        #endif
    }
}

/**
 * releaseTimeSlices ()
 * The function releases the complete TDMA time slice it has allocated on
 * each tile.
 */
void LoadBalanceBinding::releaseTimeSlices()
{
    // Release slice for each tile
    for (TilesIter iter = archGraph->tilesBegin(); 
            iter != archGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        
        // Tile contains a processor?
        if (p != NULL)
        {
            // Reserve time slice
            p->releaseTimeSlice();

            #ifdef VERBOSE
            cerr << "Released complete slice of time wheel on tile '";
            cerr << t->getName() << "'" << endl;
            #endif
        }
    }
}

/**
 * minimizeTimeSlices ()
 * The function minimizes the time slice allocation of all used tiles. The
 * objective is to find the minimal time slices with which the throughput
 * constraint is met. The function uses a binary search for this problem.
 */
bool LoadBalanceBinding::minimizeTimeSlices(double step, const double minStep)
{
    vector<CSize> currentSlice(archGraph->nrTiles());
    vector<double> tileUtilization;
    double thrGraph, thrConstraint;
    CSize slice;
    CTimer timer;
    
    // Throughput constraint
    thrConstraint = appGraph->getThroughputConstraint().value();
    
    // Analyze throughput
    startTimer(&timer);
    thrGraph = analyzeThroughput(tileUtilization);
    stopTimer(&timer);
    
    #ifdef VERBOSE
    cerr << "Thr: " << thrGraph << endl;
    cerr << "Analysis time: ";
    printTimer(cerr, &timer);
    cerr << endl;
    #endif

    // No step allowed?
    if (step < minStep)
    {
        // Constraint met?
        if (thrConstraint <= thrGraph)
        {
            #ifdef VERBOSE
            cerr << "Minimum step size reached, constraint met." << endl;
            #endif
            return true;
        }
        else
        {
            #ifdef VERBOSE
            cerr << "Minimum step size reached, constraint not met." << endl;
            #endif
            return false;
        }
    }

    // No optimization needed (throughput graph less then 10% above throughput
    // constraint)?
    if (thrConstraint <= thrGraph && thrConstraint*1.1 >= thrGraph)
    {
        #ifdef VERBOSE
        cerr << "No minimization needed; throughput within 10% of constraint";
        cerr << endl;
        #endif
        
        return true;
    }

    // Throughput constraint not met?
    if (thrConstraint > thrGraph)
    {
        bool increasedSlice = false;
        
        // Increase time slices and try again
        for (TilesIter iter = archGraph->tilesBegin();
                iter != archGraph->tilesEnd(); iter++)
        {
            Tile *t = *iter;
            Processor *p = t->getProcessor();
            
            // No processor and/or no actor mapped to tile?
            if (p == NULL || tileLoad[t->getId()] == 0)
                continue;

            // Compute increase of slice
            slice = CSize(ceil(step * p->availableTimewheelSize()));
            
            // Space available to increase slice?
            if (slice >= 1)
            {
                // Reserve the slice
                p->reserveTimeSlice(p->getReservedTimeSlice() + slice);
                increasedSlice = true;
            
                #ifdef VERBOSE
                double percTile = (double) p->getReservedTimeSlice() 
                            / (double) p->getTimewheelSize();
                cerr << "Increased time wheel reservation of tile '";
                cerr << t->getName() << "' to " << 100.0 * percTile << "%";
                cerr << endl;
                #endif
            }
        }
        
        // No slices increased
        if (increasedSlice == false)
            return false; 
        
        // Recursively search for optimal time slices
        return minimizeTimeSlices(step, minStep);
    }
    else
    {
        bool decreasedSlice = false;
        
        // Constraint met, further reduction may be possible. However, do not
        // attempt if current throughput analysis took more then 5 minutes
        if (timer.time.tv_sec > 5 * 60)
        {
            #ifdef VERBOSE
            cerr << "Abort optimization - timeout" << endl;
            #endif
            return true;
        }
            
        // Decrease time slices and try again
        for (TilesIter iter = archGraph->tilesBegin();
                iter != archGraph->tilesEnd(); iter++)
        {
            Tile *t = *iter;
            Processor *p = t->getProcessor();

            // No processor and/or actor mapped to tile?
            if (p == NULL || tileLoad[t->getId()] == 0)
                continue;

            // Keep size of current slice
            currentSlice[t->getId()] = p->getReservedTimeSlice();
            
            // Compute decrease of slice
            slice = CSize(step * p->availableTimewheelSize());
            
            // Reduction must be less then current slice
            if (slice >= p->getReservedTimeSlice())
                slice = 1;
            
            // Slice reduced by at least 1 slot?
            if (slice >= 1 && p->getReservedTimeSlice() > slice)
            {
                // Reserve the slice
                if (!p->reserveTimeSlice(p->getReservedTimeSlice() - slice))
                    throw CException("[ERROR] Failed decreasing time slice.");
                decreasedSlice = true;
                
                #ifdef VERBOSE
                double percTile = (double) p->getReservedTimeSlice() 
                            / (double) p->getTimewheelSize();
                cerr << "Reduced time wheel reservation of tile '";
                cerr << t->getName() << "' to " << 100.0 * percTile << "%";
                cerr << endl;
                #endif
            }
        }

        // No slices decreased
        if (decreasedSlice == false)
            return true; 
        
        // Recursively search for optimal time slices
        if (!minimizeTimeSlices(step/2, minStep))
        {
            // Decreased time slices cause failure on througput constraint
            // With current slices, the constraint is met. Keep current slices.
            
            for (TilesIter iter = archGraph->tilesBegin();
                    iter != archGraph->tilesEnd(); iter++)
            {
                Tile *t = *iter;
                Processor *p = t->getProcessor();
            
                // Tile contains a processor?
                if (p != NULL)
                {
                    p->reserveTimeSlice(currentSlice[t->getId()]);

                    #ifdef VERBOSE
                    double percTile = (double) p->getReservedTimeSlice() 
                                / (double) p->getTimewheelSize();
                    cerr << "Reset time wheel reservation of tile '";
                    cerr << t->getName() << "' to " << 100.0 * percTile << "%";
                    cerr << endl;
                    #endif
                }
            }
            
            return true;
        }
    }
    
    return true;
}

/**
 * optimizeTimeSlices ()
 * The function performs a binary search to minimize the time slice allocated on
 * each tile. 
 */
void LoadBalanceBinding::optimizeTimeSlices()
{
    vector<CSize> minSlice(archGraph->nrTiles());
    vector<CSize> maxSlice(archGraph->nrTiles());
    double maxTileLoad = 0;
    
    // Find maximum tile load
    for (TilesIter iter = archGraph->tilesBegin();
            iter != archGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        
        if (tileLoad[t->getId()] > maxTileLoad)
            maxTileLoad = tileLoad[t->getId()];
    }
    
    // Set bound on size of all slices
    for (TilesIter iter = archGraph->tilesBegin();
            iter != archGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        
        // Tile contains a processor
        if (p != NULL)
        {
            maxSlice[t->getId()] = p->getReservedTimeSlice();
            minSlice[t->getId()] = (CSize)floor(maxSlice[t->getId()] 
                                * (tileLoad[t->getId()] / maxTileLoad));
        }
        else
        {
            maxSlice[t->getId()] = 0;
            minSlice[t->getId()] = 0;
        }
    }
    
    // Perform the real optimization...
    if (!optimizeTimeSlices(minSlice, maxSlice))
    {
        // Optimization failed, revert to original slices
        for (TilesIter iterT = archGraph->tilesBegin();
                iterT != archGraph->tilesEnd(); iterT++)
        {
            Tile *t = *iterT;
            Processor *p = t->getProcessor();

            // Actor mapped to tile t?
            if (p != NULL && tileLoad[t->getId()] != 0)
            {
                // Reserve the slice
                p->reserveTimeSlice(maxSlice[t->getId()]);                 

                #ifdef VERBOSE
                double percTile = (double) p->getReservedTimeSlice() 
                            / (double) p->getTimewheelSize();
                cerr << "Reset time wheel reservation of tile '";
                cerr << t->getName() << "' to " << 100.0 * percTile << "%";
                cerr << endl;
                #endif
            }
        }
    }
}

/**
 * optimizeTimeSlices ()
 * The function performs a binary search to minimize the time slice allocated on
 * each tile. 
 */
bool LoadBalanceBinding::optimizeTimeSlices(vector<CSize> minSlice, 
        vector<CSize> maxSlice)
{
    vector<CSize> newSlice(archGraph->nrTiles());
    vector<double> tileUtilization;
    double thrGraph, thrConstraint;
    bool resizedSlice = false;
    CTimer timer;

    // Resize slices
    for (TilesIter iter = archGraph->tilesBegin();
            iter != archGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();

        // Tile contains no processor?
        if (p == NULL)
            continue;

        // Can slice be resized?
        if (minSlice[t->getId()] + 1 >= maxSlice[t->getId()])
        {
            newSlice[t->getId()] = maxSlice[t->getId()];
            
            // Size different from current allocation?
            if (newSlice[t->getId()] != p->getReservedTimeSlice())
            {
                p->reserveTimeSlice(newSlice[t->getId()]);

                #ifdef VERBOSE
                double percTile = (double) p->getReservedTimeSlice() 
                            / (double) p->getTimewheelSize();
                cerr << "Set time wheel reservation of tile '";
                cerr << t->getName() << "' to " << 100.0 * percTile << "%";
                cerr << endl;
                #endif
            }
        }
        else
        {        
            // Compute new size of slice
            newSlice[t->getId()] = minSlice[t->getId()] 
                    + (CSize) ceil((maxSlice[t->getId()] 
                                        - minSlice[t->getId()]) / 2.0);

            // Reserve the slice
            p->reserveTimeSlice(newSlice[t->getId()]);
            resizedSlice = true;

            #ifdef VERBOSE
            double percTile = (double) p->getReservedTimeSlice() 
                        / (double) p->getTimewheelSize();
            cerr << "Changed time wheel reservation of tile '";
            cerr << t->getName() << "' to " << 100.0 * percTile << "%";
            cerr << endl;
            #endif
        }
    }    
        
    // Throughput constraint
    thrConstraint = appGraph->getThroughputConstraint().value();
    
    // Analyze throughput
    startTimer(&timer);
    thrGraph = analyzeThroughput(tileUtilization);
    stopTimer(&timer);
    
    #ifdef VERBOSE
    cerr << "Thr: " << thrGraph;
    if (thrGraph >= thrConstraint)
        cerr << " (constraint met)";
    cerr << endl;
    cerr << "Analysis time: ";
    printTimer(cerr, &timer);
    cerr << endl;
    #endif

    // No slice resized?
    if (!resizedSlice)
    {
        #ifdef VERBOSE
        cerr << "No slice resized - stop optimization." << endl;
        #endif
        
        // Done, constraint met?
        if (thrGraph >= thrConstraint)
            return true;
        else
            return false;
    }
    
    // Constraint met?
    if (thrGraph >= thrConstraint)
    {
        // Decrease size of slice (set max to current size and call function)
        // If call returns false, reset the size of the slice to current size
        if (!optimizeTimeSlices(minSlice, newSlice))
        {
            for (TilesIter iterT = archGraph->tilesBegin();
                    iterT != archGraph->tilesEnd(); iterT++)
            {
                Tile *t = *iterT;
                Processor *p = t->getProcessor();
                
                // Actor mapped to tile?
                if (p != NULL && tileLoad[t->getId()] != 0)
                {
                    // Reserve the slice
                    p->reserveTimeSlice(newSlice[t->getId()]);                 

                    #ifdef VERBOSE
                    double percTile = (double) p->getReservedTimeSlice() 
                                / (double) p->getTimewheelSize();
                    cerr << "Reset time wheel reservation of tile '";
                    cerr << t->getName() << "' to " << 100.0 * percTile << "%";
                    cerr << endl;
                    #endif
                }
            }
        }
        
        return true;
    }
    else
    {
        // Increase size of slice (set min to current size and call function)
        return optimizeTimeSlices(newSlice, maxSlice);
    }

    return false;
}

/**
 * updateStorageSpaceAllocation ()
 * The function changes the storage space allocation of every
 * channel marked as storage space in the mappedAppGraph into the
 * storage space indicated by the storage distribution d.
 */
void LoadBalanceBinding::updateStorageSpaceAllocation(
        TimedSDFgraph *mappedAppGraph, StorageDistribution *d)
{
    SDFactor *srcActor, *dstActor;
    TimedSDFchannel *c, *cX;
    TimedSDFchannel::BufferSize b, bPrev;
    bool update = false;
    Memory *m;
    Tile *t;
    
    if (d == NULL)
    {   
        logMsg("No new storage space distribution found.");
        logMsg("Storage space is already minimal.");
        return;
    }
    
    for (SDFchannelsIter iter = mappedAppGraph->channelsBegin();
            iter != mappedAppGraph->channelsEnd(); iter++)
    {
        c = (TimedSDFchannel*)(*iter);
        
        // Channel models storage space?
        if (c->modelsStorageSpace())
        {
            cX = (TimedSDFchannel*) appGraph->getChannel(
                                        c->getStorageSpaceChannel()->getId());
            srcActor = cX->getSrcActor();
            dstActor = cX->getDstActor();
            b = cX->getBufferSize();
            bPrev = cX->getBufferSize();
            
            // Channel cX mapped to tile?
            if (isChannelBoundToTile(cX))
            {
                // Buffer does not have to be resized?
                if ((ulong)b.mem == d->sp[c->getId()])
                    continue;
                    
                // Update buffers in channel
                b.sz = d->sp[c->getId()];
                b.mem = d->sp[c->getId()];
                cX->setBufferSize(b);
                
                // Update resource allocation
                t = actorTileBinding[srcActor->getId()];
                m = t->getMemory();
                m->releaseMemory(cX);
                if (!m->reserveMemory(cX, b.mem * cX->getTokenSize()))
                {
                    throw CException("Failed resizing storage space "
                                     "of channel.");
                }
                
                // Performed update
                update = true;

                logMsg("Changed storage space '"
                        + cX->getName() + "' on tile '" + t->getName()
                        + "' from " + CString(bPrev.mem)
                        + " tokens to " + CString(b.mem) + " tokens.");
            }
            else if (c->getDstActor()->getId() == srcActor->getId())
            {
                // Channel c models source buffer

                // Buffer does not have to be resized?
                if ((ulong)b.src == d->sp[c->getId()])
                    continue;
                
                // Update buffers in channel
                b.src = d->sp[c->getId()];
                cX->setBufferSize(b);
                
                // Update resource allocation
                t = actorTileBinding[srcActor->getId()];
                m = t->getMemory();
                m->releaseMemory(cX);
                if (!m->reserveMemory(cX, b.src * cX->getTokenSize()))
                {
                    throw CException("Failed resizing storage space "
                                     "of channel.");
                }

                // Performed update
                update = true;

                logMsg("Changed storage space '"
                        + cX->getName() + "' on tile '" + t->getName()
                        + "' from " + CString(bPrev.src)
                        + " tokens to " + CString(b.src) + " tokens.");
            }
            else
            {            
                // Channel c models destination buffer

                // Buffer does not have to be resized?
                if ((ulong)b.dst == d->sp[c->getId()])
                    continue;

                // Update buffers in channel
                b.dst = d->sp[c->getId()];
                cX->setBufferSize(b);
                
                // Update resource allocation
                t = actorTileBinding[dstActor->getId()];
                m = t->getMemory();
                m->releaseMemory(cX);
                if (!m->reserveMemory(cX, b.dst * cX->getTokenSize()))
                {
                    throw CException("Failed resizing storage space "
                                     "of channel.");
                }
                
                // Performed update
                update = true;

                logMsg("Changed storage space '"
                        + cX->getName() + "' on tile '" + t->getName()
                        + "' from " + CString(bPrev.dst)
                        + " tokens to " + CString(b.dst) + " tokens.");
            }
        }
    }
    
    if (!update)
        logMsg("Storage space is already minimal.");
}

/**
 * minimizeStorageSpace ()
 * The function computes the minimal buffer allocations needed to meet the
 * throughput constraint. Resource allocations are adjusted to this minimal
 * storage space.
 */
void LoadBalanceBinding::minimizeStorageSpace()
{
    SDFstateSpaceBindingAwareBufferAnalysis bufferAnalysisAlgo;
    StorageDistributionSet *minStorageDeps, *storageDep;
    BindingAwareSDFG *bindingAwareSDFG;
    double thrConstraint;
    bool *bufferChannels;
    
    // Throughput constraint
    thrConstraint = appGraph->getThroughputConstraint().value();
    
    // Create a binding-aware SDFG
    bindingAwareSDFG = new BindingAwareSDFG(appGraph, archGraph, getFlowType());

    // Mark every channels as buffer channel or non-buffer channel
    bufferChannels = new bool [bindingAwareSDFG->nrChannels()];
    for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++)
    {   
        TimedSDFchannel *c = (TimedSDFchannel*)(bindingAwareSDFG->getChannel(i));
        if (c->modelsStorageSpace())
            bufferChannels[i] = true;
        else
            bufferChannels[i] = false;
    }

    // Compute buffer/throughput trade-off space
    minStorageDeps = bufferAnalysisAlgo.analyze(bindingAwareSDFG, 
                                           bufferChannels, true, thrConstraint);

    // Update the storage space allocation with the smallest storage
    // distribution that satisfies the throughput constraint
    storageDep =  minStorageDeps;
    while (storageDep != NULL)
    {
        if (storageDep->thr >= thrConstraint)
        {
            updateStorageSpaceAllocation(bindingAwareSDFG,
                                            storageDep->distributions);
            break;
        }
        
        storageDep = storageDep->next;
    }   
    
    #ifdef VERBOSE
    // There should always be a storage dependency which satisfies the
    // throughput constraint (assuming that the initial allocation satisfies
    // the constraint)
    if (storageDep == NULL)
        cerr << "Storage space could not be optimized further." << endl;
    #endif
    
    // Cleanup
    delete [] bufferChannels;
    delete bindingAwareSDFG;
}

/**
 * bindingCheck ()
 * Run only binding step of the load balance binding algorithm.
 */
bool LoadBalanceBinding::bindingCheck()
{
    #ifdef VERBOSE
    cerr << "[INFO] Load balancing mapping algorithm (binding check)" << endl;
    #endif

    #ifdef VERBOSE
    cerr << "[INFO] Constants tile cost function:" << endl;
    cerr << "    a: " << cnst_a << endl;
    cerr << "    b: " << cnst_b << endl;
    cerr << "    c: " << cnst_c << endl;
    cerr << "    d: " << cnst_d << endl;
    cerr << "    e: " << cnst_e << endl;
    cerr << "    f: " << cnst_f << endl;
    cerr << "    g: " << cnst_g << endl;
    cerr << "    k: " << cnst_k << endl;
    cerr << "    l: " << cnst_l << endl;
    cerr << "    m: " << cnst_m << endl;
    cerr << "    n: " << cnst_n << endl;
    cerr << "    o: " << cnst_o << endl;
    cerr << "    p: " << cnst_p << endl;
    cerr << "    q: " << cnst_q << endl;
    #endif    

    // Initialize vector containing all actor to tile bindings
    actorTileBinding.resize(appGraph->nrActors());
    for (uint i = 0; i < appGraph->nrActors(); i++)
    {
        actorTileBinding[i] = NULL;
    }

    // Initialize the load of the tiles
    initTileLoad();

    #ifdef VERBOSE
    cerr << "[INFO] Binding" << endl;
    #endif
    
    // Bind each actor to a tile
    if (!bindActorsToTiles())
    {
        #ifdef VERBOSE
        cerr << "Failed binding actor to tile." << endl;
        #endif
        return false;
    }
    
    // Reserve complete time wheel on each tile
    reserveTimeSlices(1);
    
    #ifdef VERBOSE
    cerr << "[INFO] Verifying throughput constraint" << endl;
    #endif

    // Check throughput constraint
    return isThroughputConstraintSatisfied();
}

/**
 * bindSDFGtoTiles ()
 *
 */
bool LoadBalanceBinding::bindSDFGtoTiles()
{
    #ifdef VERBOSE
    cerr << "[INFO] Load balancing mapping algorithm" << endl;
    #endif

    #ifdef VERBOSE
    uint nrActorsHSDFG = 0;
    for (uint i = 0; i < appGraph->nrActors(); i++)
        nrActorsHSDFG += repVec[i];

    cerr << "[INFO] Application graph analysis" << endl;
    cerr << "Application thr: " << analyzeThroughputApplication() << endl;
    cerr << "#actors:         " << appGraph->nrActors() << endl;
    cerr << "#channels:       " << appGraph->nrChannels() << endl;
    cerr << "#actors (HSDFG): " << nrActorsHSDFG << endl;
    #endif

    #ifdef VERBOSE
    cerr << "[INFO] Constants tile cost function:" << endl;
    cerr << "    a: " << cnst_a << endl;
    cerr << "    b: " << cnst_b << endl;
    cerr << "    c: " << cnst_c << endl;
    cerr << "    d: " << cnst_d << endl;
    cerr << "    e: " << cnst_e << endl;
    cerr << "    f: " << cnst_f << endl;
    cerr << "    g: " << cnst_g << endl;
    cerr << "    k: " << cnst_k << endl;
    cerr << "    l: " << cnst_l << endl;
    cerr << "    m: " << cnst_m << endl;
    cerr << "    n: " << cnst_n << endl;
    cerr << "    o: " << cnst_o << endl;
    cerr << "    p: " << cnst_p << endl;
    cerr << "    q: " << cnst_q << endl;
    #endif    

    // Initialize the load of the tiles
    initTileLoad();

    #ifdef VERBOSE
    cerr << "[INFO] Binding" << endl;
    #endif
    
    // Bind each actor to a tile
    if (!bindActorsToTiles())
    {
        #ifdef VERBOSE
        cerr << "Failed binding actor to tile." << endl;
        #endif

        // Release allocated resources on failure
        for (SDFactorsIter iter = appGraph->actorsBegin();
                iter != appGraph->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*) (*iter);
            Tile *t = actorTileBinding[a->getId()];

            // Actor bound to tile?
            if (t != NULL)
            {
                // Release resources reserved by actor (including channels)
                releaseResources(a, t);
            }
        }

        return false;
    }

    #ifdef VERBOSE
    double max;
    max = 0;
    for (uint i = 0; i < archGraph->nrTiles(); i++)
        max = (max > tileLoad[i] ? max : tileLoad[i]);

    cerr << "Tile load:" << endl;
    for (uint i = 0; i < archGraph->nrTiles(); i++)
    {
        cerr << "    " << archGraph->getTile(i)->getName() << ": ";
        cerr << (100 * tileLoad[i] / max) << "%" << endl;
    }
    #endif

    #ifdef VERBOSE
    cerr << "[INFO] Optimize binding" << endl;
    #endif

    // Optimize tile bindings
    optimizeActorToTileBindings();

    #ifdef VERBOSE
    max = 0;
    for (uint i = 0; i < archGraph->nrTiles(); i++)
        max = (max > tileLoad[i] ? max : tileLoad[i]);

    cerr << "Tile load:" << endl;
    for (uint i = 0; i < archGraph->nrTiles(); i++)
    {
        cerr << "    " << archGraph->getTile(i)->getName() << ": ";
        cerr << (100 * tileLoad[i] / max) << "%" << endl;
    }
    #endif
    
    return true;
}

/**
 * constructStaticOrderScheduleTiles ()
 *
 */
bool LoadBalanceBinding::constructStaticOrderScheduleTiles()
{
    constructStaticOrderSchedules();

    #ifdef VERBOSE
    cerr << "[INFO] Minimize static-order schedules" << endl;
    #endif

    minimizeStaticOrderSchedules(archGraph);

    return true;
}

/**
 * allocateTDMAtimeSlices ()
 *
 */
bool LoadBalanceBinding::allocateTDMAtimeSlices()
{
    vector<double> tileUtilization;
    int success;
    
    reserveTimeSlices(0.5);

    #ifdef VERBOSE
    cerr << "[INFO] Minimize time slices" << endl;
    #endif

    success = minimizeTimeSlices(0.5, 0.01);

    if (success)
    {
        #ifdef VERBOSE
        cerr << "[INFO] Optimize time slices" << endl;
        #endif

        optimizeTimeSlices();

        #ifdef VERBOSE
        cerr << "[INFO] Tile utilization" << endl;
        
        // Analyze tile utilization
        analyzeThroughput(tileUtilization);

        for (TilesIter iter = archGraph->tilesBegin(); 
                iter != archGraph->tilesEnd(); iter++)
        {
            Tile *t = *iter;

            cerr << "   Tile load (" << t->getName() << "): ";
            cerr << 100.0 * tileUtilization[t->getId()] << "%" << endl;
        }
        #endif
    }
    
    #ifdef VERBOSE
    cerr << "[INFO] Verifying throughput constraint" << endl;
    #endif
    
    // Check throughput constraint
    success = isThroughputConstraintSatisfied();
    
    // Release allocated resources on failure
    if (!success)
    {
        for (SDFactorsIter iter = appGraph->actorsBegin();
                iter != appGraph->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*) (*iter);
            Tile *t = actorTileBinding[a->getId()];
            
            // Actor bound to tile?
            if (t != NULL)
            {
                // Release resources reserved by actor (including channels)
                releaseResources(a, t);
            }
        }
    }
        
    return success;
}

/**
 * optimizeStorageSpaceAllocations ()
 *
 */
bool LoadBalanceBinding::optimizeStorageSpaceAllocations()
{
    minimizeStorageSpace();

    return true;
}

/**
 * bind ()
 * Load balance binding algorithm.
 */
bool LoadBalanceBinding::bind()
{
    if (!bindSDFGtoTiles())
        return false;
    
    if (!constructStaticOrderScheduleTiles())
        return false;

    if (!allocateTDMAtimeSlices())
        return false;

    if (!optimizeStorageSpaceAllocations())
        return false;
    
    return true;
}
