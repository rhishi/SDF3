/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdftocsdf.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   Convert an SDF graph to a CSDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: sdftocsdf.cc,v 1.2 2008/03/22 14:24:21 sander Exp $
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

#include "sdftocsdf.h"

/**
 * addCSDFactorToGraph ()
 * The function adds an actor to the graph with the same properties as
 * the supplied actor.
 */
static
void addCSDFactorToGraph(TimedCSDFgraph *csdfGraph, TimedSDFactor *a)
{
    TimedCSDFactor *aNew = csdfGraph->createActor();
    
    // Component properties
    aNew->setName(a->getName());
    aNew->setId(a->getId());

    // Base properties
    aNew->setType(a->getType());
    
    // Timed properties
    for (TimedSDFactor::ProcessorsIter iter = a->processorsBegin();
            iter != a->processorsEnd(); iter++)
    {
        TimedSDFactor::Processor *p = *iter;
        TimedCSDFactor::Processor *pNew = new TimedCSDFactor::Processor;
        
        // Processor properties
        pNew->type = p->type;
        pNew->execTime.push_back(p->execTime);
        pNew->stateSize = p->stateSize;

        // Add processor to the list
        aNew->addProcessor(pNew);
    }
    aNew->setDefaultProcessor(a->getDefaultProcessor());    
}

/**
 * addCSDFchannelToGraph ()
 * The function adds a channel to the graph with the same properties as
 * the supplied channel.
 */
static
void addCSDFchannelToGraph(TimedCSDFgraph *csdfGraph, TimedSDFchannel *c)
{
    TimedCSDFchannel *cNew;
    CSDFbufferSize bufferSize;
    
    // Create a new CSDF channel on the graph
    cNew = csdfGraph->createChannel(
                csdfGraph->getActor(c->getSrcActor()->getId()),
                c->getSrcPort()->getRate(),
                csdfGraph->getActor(c->getDstActor()->getId()),
                c->getDstPort()->getRate());

    // Component properties
    cNew->setName(c->getName());
    cNew->setId(c->getId());

    // Base properties
    cNew->setInitialTokens(c->getInitialTokens());
    
    // Timed properties
    bufferSize.sz = c->getBufferSize().sz;
    bufferSize.src = c->getBufferSize().src;
    bufferSize.dst = c->getBufferSize().dst;
    bufferSize.mem = c->getBufferSize().mem;
    cNew->setBufferSize(bufferSize);
    cNew->setTokenType(c->getTokenType());
    cNew->setMinBandwidth(c->getMinBandwidth());
    cNew->setTokenSize(c->getTokenSize());
    cNew->setMinLatency(c->getMinLatency());
}

/**
 * convertSDFGtoCSDFG ()
 * The function converts an SDFG to an equivalent CSDFG.
 */
TimedCSDFgraph *convertSDFGtoCSDFG(TimedSDFgraph *sdfGraph)
{
    TimedCSDFgraph *csdfGraph;
    
    // Create graph
    CSDFcomponent component = CSDFcomponent(NULL, 0, sdfGraph->getName());
    csdfGraph = new TimedCSDFgraph(component);

    // Create a CSDF actor for each actor in the sdf
    for (SDFactorsIter iter = sdfGraph->actorsBegin();
            iter != sdfGraph->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        
        addCSDFactorToGraph(csdfGraph, a);
    }
    
    // Create a CSDF channel for each channel in the sdf
    for (SDFchannelsIter iter = sdfGraph->channelsBegin();
            iter != sdfGraph->channelsEnd(); iter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)(*iter);
        
        addCSDFchannelToGraph(csdfGraph, c);
    }
    
    // Copy the properties of the graph
    csdfGraph->setThroughputConstraint(sdfGraph->getThroughputConstraint());

    return csdfGraph;
}
