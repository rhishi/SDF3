/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   constraint.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 7, 2007
 *
 *  Function        :   SDFG mapping to MP-SoC
 *
 *  History         :
 *      07-02-07    :   Initial version.
 *
 * $Id: constraint.cc,v 1.3 2008/03/06 10:49:44 sander Exp $
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

#include "flow.h"
#include "../../analysis/analysis.h"
#include "../../base/algo/cycle.h"
#include "../../transform/model/buffersize.h"

/**
 * estimateLatencyConstraints ()
 * Estimate the minimal latency between the production and consumption of
 * a token on a channel.
 */
void SDF3Flow::estimateLatencyConstraints()
{
    double throughputDist = selectedStorageDistribution->thr;
    TimedSDFgraph *storageAppGraph;
    RepetitionVector repVec;
    SDFgraphCycles cycles;
    double **channelWeight;
    double *actorWeight;
    int latency = 0;

    // Output current state of the flow
    logInfo("Estimate latency constraint.");

    // Initialize latency of all channels to infinity
    for (SDFchannelsIter iter = getAppGraph()->channelsBegin();
            iter != getAppGraph()->channelsEnd(); iter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)*iter;
        
        c->setMinLatency(UINT_MAX);
    }

    // Model storage constraints in the graph
    storageAppGraph = modelBufferSizeInSDFgraph(getAppGraph());

    // Compute repetition vector
    repVec = computeRepetitionVector(storageAppGraph);

#if 0
    // Actor weight (average execution time)
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
        execTime = execTime / (double)(a->nrProcessors());
        
        actorWeight[a->getId()] = execTime * repVec[a->getId()];
    }
#endif

    // Actor weight (minimum execution time)
    actorWeight = new double [appGraph->nrActors()];
    for (SDFactorsIter iter = appGraph->actorsBegin();
            iter != appGraph->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;
        double execTime = UINT_MAX;
        
        // Minimum execution time
        for (TimedSDFactor::ProcessorsIter iterP = a->processorsBegin();
                iterP != a->processorsEnd(); iterP++)
        {
            TimedSDFactor::Processor *p = *iterP;
            
            if (execTime > p->execTime)
                execTime = p->execTime;
        }
        
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
    for (SDFchannelsIter iter = storageAppGraph->channelsBegin();
            iter != storageAppGraph->channelsEnd(); iter++)
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
    cycles = findSimpleCycles(storageAppGraph);

    // Compute the cycle mean for each cycle and the latency for all channels on
    // the cycle
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
        
        // Cycle mean
        cycleMean = weightActors / weightChannels;

        // Latency for every channel on the cycle
        latency = (int) ceil((((1.0/cycleMean) - throughputDist) 
                    / throughputDist) * (weightActors / cycle.size()));
        
        // Latency should always be a non-negative value
        if (latency < 0)
            latency = 0;
        if (latency > 100000)
            latency = 100000;
        
        // Update latency for all channels on the cycle
        actorIter = cycle.begin();
        srcActor = *actorIter;
        while (actorIter != cycle.end())
        {
            // Destination actor
            actorIter++;
            if (actorIter == cycle.end())
                dstActor = cycle.front();
            else
                dstActor = *actorIter;
            
            // Find all channels from the source to the destination actor
            for (SDFchannelsIter iter = getAppGraph()->channelsBegin();
                    iter != getAppGraph()->channelsEnd(); iter++)
            {
                TimedSDFchannel *c = (TimedSDFchannel*)*iter;

                if (c->getSrcActor()->getId() == srcActor->getId()
                        && c->getDstActor()->getId() == dstActor->getId())
                {
                    if (c->getMinLatency() > (uint)(latency))
                        c->setMinLatency(latency);
                }
            }
                
            // Destination becomes next source actor
            srcActor = dstActor;
        }
    }

    if (logHasLevel())
    {
        // Output all latencies as messages to terminal
        for (SDFchannelsIter iter = getAppGraph()->channelsBegin();
                iter != getAppGraph()->channelsEnd(); iter++)
        {
            TimedSDFchannel *c = (TimedSDFchannel*)*iter;

            logMsg("Latency channel '" + c->getName() + "': "
                        + CString(c->getMinLatency()) + " time-units");
        }
    }
    
    // Cleanup
    for (uint i = 0; i < appGraph->nrActors(); i++)
        delete [] channelWeight[i];
    delete [] channelWeight;
    delete [] actorWeight;
    delete storageAppGraph;
    
    // Advance to next state in the flow
    setNextStateOfFlow(FlowEstimateBandwidthConstraint);  
}

/**
 * estimateBandwidthConstraints ()
 * Estimate the required minimal bandwidth when binding a channel to a
 * connection.
 */
void SDF3Flow::estimateBandwidthConstraints()
{
    double bandwidth;
    RepetitionVector repVec = computeRepetitionVector(getAppGraph());
    
    // Output current state of the flow
    logInfo("Compute bandwidth constraint.");

    for (SDFchannelsIter iter = getAppGraph()->channelsBegin();
            iter != getAppGraph()->channelsEnd(); iter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)*iter;
        
        bandwidth = c->getSrcPort()->getRate() 
                        * repVec[c->getSrcActor()->getId()] 
                        * c->getTokenSize() 
                        * selectedStorageDistribution->thr;

        c->setMinBandwidth(bandwidth);

        logMsg("Bandwidth channel '" + c->getName() + "': "
                    + CString(c->getMinBandwidth()) + " bits/time-units");
    }

    // Advance to next state in the flow
    setNextStateOfFlow(FlowBindSDFGtoTile);    
}
