/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
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
 * $Id: memory.cc,v 1.5 2008/03/06 13:59:05 sander Exp $
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

// Search the complete storage-space throughput trade-off space in the 
// computeStorageDistributions function. This makes it possible to quickly
// select a distribution in the next step of the flow (i.e. faster iterations).
// The alternative is to search the space only when a new Pareto point is
// needed.
//#define _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE

/**
 * modelNonLocalMemoryAccesses ()
 * Model non-local memory accesses in the application graph.
 */
void SDF3Flow::modelNonLocalMemoryAccesses()
{
    cerr << "[WARNING] model non-local memory access not implemented." << endl;
    
    // Advance to next state in the flow
    setNextStateOfFlow(FlowComputeStorageDist);    
}

/**
 * computeStorageDistributions ()
 * Compute trade-off space between storage space allocated to channels and
 * maximal throughput of the application graph.
 */
void SDF3Flow::computeStorageDistributions()
{
    StorageDistributionSet *distributionSet = NULL;
    StorageDistribution *distr = NULL;
    TimedSDFgraph *g;
    
    // Output current state of the flow
    logInfo("Compute storage-space / throughput trade-offs.");

    // Assume absense of auto-concurrency (i.e. add self-edge to every actor)
    g = getAppGraph()->clone();
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        g->createChannel(a, 1, a, 1, 1);
    }
    
    // Set processor with lowest execution time as the default processor
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        TimedSDFactor::Processor *defaultProc = NULL;
        
        for (TimedSDFactor::ProcessorsIter iterP = a->processorsBegin();
                iterP != a->processorsEnd(); iterP++)
        {
            TimedSDFactor::Processor *p = *iterP;
            
            if (defaultProc == NULL || defaultProc->execTime > p->execTime)
                defaultProc = p;
        }
        
        a->setDefaultProcessor(defaultProc->type);
    }    

#ifdef _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE    
    // Compute trade-off space
    minStorageDistributions = bufferAnalysisAlgo.analyze(g);

    // Update storage-space size to ignore added self-edges. These channels have
    // space for 2 tokens.
    distributionSet = minStorageDistributions;
    while (distributionSet != NULL)
    {
        // Only update size when it is larger then zero
        if (distributionSet->sz > 0)
        {
            // Update size of the set
            distributionSet->sz = distributionSet->sz - 2 * g->nrActors();
        
            // Update all distributions in the set
            distr = distributionSet->distributions;
            while (distr != NULL)
            {
                // Update size of the distribution
                distr->sz = distributionSet->sz;

                // Next distribution
                distr = distr->next;
            }
        }
                
        // Next set
        distributionSet = distributionSet->next;
    }

    // No storage distribution selected so far
    selectedStorageDistributionSet = NULL;
    selectedStorageDistribution = NULL;

#else // _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE

    // Initialize the exploration algorithm
    bufferAnalysisAlgo.initSearch(g);

    // No storage distribution found and selected so far
    minStorageDistributions = NULL;
    selectedStorageDistributionSet = NULL;
    selectedStorageDistribution = NULL;

    // Search the storage-space / throughput trade-off space for a Pareto point
    // with a throughput equal or larger then the constraint
    do
    {
        // Find next pareto point in the space
        selectedStorageDistributionSet 
                        = bufferAnalysisAlgo.findNextStorageDistributionSet();

        // No new pareto point discovered?
        if (selectedStorageDistributionSet == NULL)
        {
            logError("No storage distribution meets throughput constraint.");
            setNextStateOfFlow(FlowFailed);
            return;
        }

        // Update storage-space size to ignore added self-edges. These channels
        // have space for 2 tokens. Note that only  distributions with a size
        // larger then zero must be updated.
        distributionSet = selectedStorageDistributionSet;
        if (distributionSet->sz > 0)
        {
            // Update size of the set
            distributionSet->sz = distributionSet->sz - 2 * g->nrActors();

            // Update all distributions in the set
            distr = distributionSet->distributions;
            while (distr != NULL)
            {
                // Update size of the distribution
                distr->sz = distributionSet->sz;

                // Next distribution
                distr = distr->next;
            }
        }

        // Is this the first point ever to be found?
        if (minStorageDistributions == NULL)
            minStorageDistributions = selectedStorageDistributionSet;
    } while (selectedStorageDistributionSet->thr 
                            < getAppGraph()->getThroughputConstraint().value());


#endif
    
    // Advance to next state in the flow
    setNextStateOfFlow(FlowSelectStorageDist);    
}

/**
 * selectStorageDistribution ()
 * Select storage distribution from the trade-off space.
 */
void SDF3Flow::selectStorageDistribution()
{
    SDFthroughput thrConstraint = getAppGraph()->getThroughputConstraint();

    // Output current state of the flow
    logInfo("Select storage distribution.");

#ifdef _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE

    // Currently no storage distribution set selected?
    if (selectedStorageDistributionSet == NULL)
    {
        // Find first set with throughput equal or larger
        // then constraint
        selectedStorageDistributionSet = minStorageDistributions;
        while (selectedStorageDistributionSet != NULL
                 && selectedStorageDistributionSet->thr < thrConstraint.value())
        {
            selectedStorageDistributionSet 
                                        = selectedStorageDistributionSet->next;
        }
        
        // No storage distribution set found which has sufficiently large
        // throughput?
        if (selectedStorageDistributionSet == NULL)
        {
            logError("No storage distribution meets throughput constraint.");
            setNextStateOfFlow(FlowFailed);
            return;
        }
    }

    // Select next storage distribution within the set (advance to next set when
    // current set is exhausted)
    do
    {
        if (selectedStorageDistribution == NULL)
        {
            selectedStorageDistribution =
                                  selectedStorageDistributionSet->distributions;
        }
        else
        {
            selectedStorageDistribution = selectedStorageDistribution->next;
        }
        
        // All storage distribution within current set (size) tried?
        if (selectedStorageDistribution == NULL)
        {
            selectedStorageDistributionSet 
                                        = selectedStorageDistributionSet->next;
        }
        
        // New set of larger storage distributions does not exist?
        if (selectedStorageDistributionSet == NULL)
        {
            logError("No storage distribution left.");
            setNextStateOfFlow(FlowFailed);
            return;
        }
    }
    while (selectedStorageDistribution == NULL);  

#else // _SEARCH_THROUGHPUT_BUFFER_SPACE_AT_ONCE
    StorageDistributionSet *distributionSet = NULL;
    StorageDistribution *distr = NULL;

    // Select next storage distribution within the set (advance to next set when
    // current set is exhausted)
    do
    {
        if (selectedStorageDistribution == NULL)
        {
            selectedStorageDistribution =
                                  selectedStorageDistributionSet->distributions;
        }
        else
        {
            selectedStorageDistribution = selectedStorageDistribution->next;
        }
        
        // All storage distribution within current set (size) tried?
        if (selectedStorageDistribution == NULL)
        {
            selectedStorageDistributionSet 
                          = bufferAnalysisAlgo.findNextStorageDistributionSet();

            // New set of larger storage distributions does not exist?
            if (selectedStorageDistributionSet == NULL)
            {
                logError("No storage distribution left.");
                setNextStateOfFlow(FlowFailed);
                return;
            }

            // Update storage-space size to ignore added self-edges. These
            // channels have space for 2 tokens. Note that only  distributions
            // with a size larger then zero must be updated.
            distributionSet = selectedStorageDistributionSet;
            if (distributionSet->sz > 0)
            {
                // Update size of the set
                distributionSet->sz = distributionSet->sz 
                                            - 2 * getAppGraph()->nrActors();

                // Update all distributions in the set
                distr = distributionSet->distributions;
                while (distr != NULL)
                {
                    // Update size of the distribution
                    distr->sz = distributionSet->sz;

                    // Next distribution
                    distr = distr->next;
                }
            }
        }
    }
    while (selectedStorageDistribution == NULL);  

#endif

    logMsg(CString("Selected distribution of size ")
            + CString(selectedStorageDistribution->sz) + " with throughput " 
            + CString(selectedStorageDistribution->thr));

    // Advance to next state in the flow
    setNextStateOfFlow(FlowEstimateStorageDist);    
}

/**
 * estimateStorageConstraints ()
 * Set the storage space constraints of the channels based on the
 * selected storage distribution.
 */
void SDF3Flow::estimateStorageConstraints()
{
    TimedSDFchannel::BufferSize buf;
    StorageDistributionSet *minDeadlockFreeDistributions;
    
    // Output current state of the flow
    logInfo("Estimate storage distribution per connection.");

    for (SDFchannelsIter iter = getAppGraph()->channelsBegin();
            iter != getAppGraph()->channelsEnd(); iter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)*iter;
        
        // Storage size for the channel
        buf.sz = selectedStorageDistribution->sp[c->getId()];
        
        // Compute source and destination buffer sizes
        if (getFlowType() == SDFflowTypeMPFlow)
        {
            buf.src = buf.sz;
            buf.dst = c->getDstPort()->getRate();
        }
        else
        {
            buf.src = buf.sz / 2 + (buf.sz % 2 > 0 ? 1 : 0);
            buf.dst = buf.sz / 2 + (buf.sz % 2 > 0 ? 1 : 0);
        }
        
        // Correct buffer sizes for constraints
        if (buf.src < (int)c->getSrcPort()->getRate())
            buf.src = c->getSrcPort()->getRate();
        if (buf.src < (int)c->getInitialTokens())
            buf.src = c->getInitialTokens();
        if (buf.dst < (int)c->getDstPort()->getRate())
            buf.dst = c->getDstPort()->getRate();

        // Select minimal deadlock free buffersize for channels mapped
        // to memory (connected actors are fired sequentially anyhow)
        minDeadlockFreeDistributions = minStorageDistributions;
        while (minDeadlockFreeDistributions != NULL
                    && minDeadlockFreeDistributions->thr == 0)
        {
            minDeadlockFreeDistributions = minDeadlockFreeDistributions->next;
        }
        buf.mem = minDeadlockFreeDistributions->distributions->sp[c->getId()];

        c->setBufferSize(buf);
        
        logMsg("Storage space channel '" + c->getName() + "' (mem/src/dst): "
                    + CString(buf.mem) + " / " + CString(buf.src) + " / " 
                    + CString(buf.dst) + " tokens");
    }

    // Advance to next state in the flow
    setNextStateOfFlow(FlowEstimateLatencyConstraint);    
}

