/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffer_capacity_constrained.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 10, 2007
 *
 *  Function        :   Model buffer sizes in the graph with explicit channels
 *                      according to the model of Ning and Gao.
 *
 *  History         :
 *      10-08-07    :   Initial version.
 *
 * $Id: buffer_capacity_constrained.cc,v 1.2 2008/09/18 07:38:21 sander Exp $
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

#include "buffersize.h"
#include "autoconc.h"
#include "../../base/hsdf/check.h"
#include "../../analysis/mcm/mcm.h"

/**
 * createCapacityConstrainedModel ()
 * Create a new timed HSDF graph in which every actor 'a' of the original graph 
 * 'g' belongs to a cycle consisting of the actor 'a' and a new actor 'aMCM'.
 * This cycle contains one initial token and its cycle mean is equal to the MCM
 * of the graph g when g is executed without auto-concurrency.
 */
TimedSDFgraph *createCapacityConstrainedModel(TimedSDFgraph *g)
{
    TimedSDFgraph *gNoConcurrency, *gConstrained;
    CFraction mcm;
    
    // Is input graph an HSDFG?
    if (!isHSDFgraph(g))
        throw CException("Graph is not an HSDF graph.");    

    // Exclude auto-concurrency
    gNoConcurrency = (TimedSDFgraph*)modelAutoConcurrencyInSDFgraph(g, 1);

    // Compute maximal throughput and MCM
    mcm = maximumCycleMeanCycles(gNoConcurrency);
    mcm = mcm.lowestTerm();

    // MCM must be an integer value
    if (mcm.denominator() != 1)
        throw CException("MCM must have an integer value.");
    
    // Create a new graph
    gConstrained = g->clone();

    // Add for every actor 'a' in the graph a new actor 'aMCM' to the graph. The
    // two actors are placed on a cycle with one initial token. The cycle mean
    // is equal to the MCM of the original graph.
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        TimedSDFactor::Processor *p;
        TimedSDFactor *aMCM, *aConstrained;
        
        // Find corresponding actor of 'a' in the constrained graph
        aConstrained = (TimedSDFactor*)gConstrained->getActor(a->getName());
        
        // Create a new actor on gConstrained
        aMCM = gConstrained->createActor();
        
        // Copy properties
        aMCM->setName(a->getName() + "MCM");
        aMCM->setType("MCM");
        
        // Add a processor with execution time zero
        p = aMCM->addProcessor("default");
        p->execTime = mcm.numerator() - a->getExecutionTime();
        p->stateSize = 0;

        // Make the new processor the default processor
        aMCM->setDefaultProcessor("default");
        
        // Create a channel from 'aMCM' to 'aConstrained' with one initial token
        gConstrained->createChannel(aMCM, 1, aConstrained, 1, 1);
        
        // Create a channel from 'aConstrained' to 'aMCM' with zero initial tokens
        gConstrained->createChannel(aConstrained, 1, aMCM, 1, 0);
    }

    // Cleanup
    delete gNoConcurrency;
    
    return gConstrained;
}

/**
 * modelCapacityConstrainedBuffer ()
 * Create a new timed HSDF graph in which channel sizes are modelled
 * through explicit channels (all buffer sizes are set to unbounded).
 * The function 'getStorageSpaceChannel()' on each channel gives a
 * pointer to the channel in input graph g for which the storage space
 * is determined.
 */
TimedSDFgraph *modelCapacityConstrainedBuffer(TimedSDFgraph *g, const uint mcm)
{
    TimedSDFgraph *gNew;
    
    // Is input graph an HSDFG?
    if (!isHSDFgraph(g))
        throw CException("Graph is not an HSDF graph.");    
    
    // Create a new graph
    SDFcomponent comp(g->getParent(), g->getId());
    gNew = new TimedSDFgraph(comp);

    // Each actor in 'g' has a corresponding actor in 'gNew' with execution time
    // zero.
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        TimedSDFactor::Processor *p;
        TimedSDFactor *aNew;
        
        // Create a new actor on gNew
        aNew = gNew->createActor();
        
        // Copy properties
        aNew->setName(a->getName());
        aNew->setType(a->getType());
        
        // Add a processor with execution time zero
        p = aNew->addProcessor("default");
        p->execTime = 0;
        p->stateSize = 0;

        // Make the new processor the default processor
        aNew->setDefaultProcessor("default");
    }
    
    // Add the Ning and Gao channel model for each channel in the graph 'g' to
    // the graph gNew.
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)(*iter);
        TimedSDFchannel *cNewBuf, *cNew;
        TimedSDFactor *aNewA, *aNewB, *srcNew, *dstNew, *srcA;
        TimedSDFactor::Processor *p;
        
        // Source actor of the channel c
        srcA = (TimedSDFactor*)c->getSrcActor();
        
        // Find corresponding source and destination actor of c in gNew
        srcNew = (TimedSDFactor*)gNew->getActor(c->getSrcActor()->getName());
        dstNew = (TimedSDFactor*)gNew->getActor(c->getDstActor()->getName());

        // Execution time of source actor should be equal or larger then mcm
        if (mcm < srcA->getExecutionTime())
            throw CException("[ERROR] execution time of actor lower then MCM.");

        // Create a new actor on gNew which models the mcm of the graph
        aNewA = gNew->createActor();
        aNewA->setType("mcmGraph");
        p = aNewA->addProcessor("default");
        p->execTime = mcm - srcA->getExecutionTime();
        p->stateSize = 0;
        aNewA->setDefaultProcessor("default");

        // Create a new actor on gNew which models execution time of 
        // the source actor.
        aNewA->setType("execTime");
        aNewB = gNew->createActor();
        p = aNewB->addProcessor("default");
        p->execTime = srcA->getExecutionTime();
        p->stateSize = 0;
        aNewB->setDefaultProcessor("default");
        
        // Create all channels in the Ning and Gao (capacity constrained) model
        gNew->createChannel(srcNew, 1, aNewB, 1, 0);
        gNew->createChannel(aNewB, 1, aNewA, 1, 0);
        gNew->createChannel(aNewA, 1, srcNew, 1, 1);
        cNew = gNew->createChannel(aNewB, 1, dstNew, 1, c->getInitialTokens());
        
        if ( c->getBufferSize().sz >= (int)(c->getInitialTokens()))
        {
            cNewBuf = gNew->createChannel(dstNew, 1, aNewB, 1,
                                c->getBufferSize().sz - c->getInitialTokens());
        }
        else
        {
            cNewBuf = gNew->createChannel(dstNew, 1, aNewB, 1, 0);
        }
        
        cNewBuf->setStorageSpaceChannel(c);
    }
    
    return gNew;
}
