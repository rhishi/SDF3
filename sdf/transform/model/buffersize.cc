/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffersize.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 19, 2005
 *
 *  Function        :   Model buffer sizes in the graph with explicit channels.
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: buffersize.cc,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

/**
 * modelBufferSizeInSDFgraph ()
 * Create a new timed SDF graph in which channel sizes are modelled
 * through explicit channels (all buffer sizes are set to unbounded).
 */
TimedSDFgraph *modelBufferSizeInSDFgraph(const TimedSDFgraph *graph)
{
    TimedSDFgraph *g;
    SDFactor *srcActor, *dstActor;
    SDFport *srcPort, *dstPort;
    TimedSDFchannel *cN;
    SDFport *srcPortN, *dstPortN;
    TimedSDFchannel::BufferSize unboundedBuffer;
    
    // Create a copy of the original graph
    SDFcomponent comp(graph->getParent(), graph->getId());
    g = graph->clone(comp);
    
    // Unbounded buffer size
    unboundedBuffer.sz = -1;
    unboundedBuffer.mem = -1;
    unboundedBuffer.src = -1;
    unboundedBuffer.dst = -1;
    
    // Iterate over all channels in the graph
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)(*iter);

        // Source and destination actor and port of channel
        srcActor = c->getSrcActor();
        srcPort = c->getSrcPort();
        dstActor = c->getDstActor();
        dstPort = c->getDstPort();
        
        // Is channel bounded?
        if (!c->isUnbounded())
        {
            // Create a backward channel to model buffer size
            comp = SDFcomponent(g, g->nrChannels());
            cN = c->create(comp);
            cN->setName(CString("_") + c->getName() + CString("b"));
            
            // Initial tokens on channel to model buffer size of channel c
            cN->setInitialTokens((c->getBufferSize().sz) 
                    - c->getInitialTokens());
            
            // Set channels to be unbounded
            c->setBufferSize(unboundedBuffer);
            cN->setBufferSize(unboundedBuffer);
            
            // Create new ports on the source and destination actors
            comp = SDFcomponent(dstActor, dstActor->nrPorts());
            srcPortN = dstPort->create(comp);
            srcPortN->setName(CString("_p") + CString(dstActor->nrPorts()+1));
            srcPortN->setType("out");
            srcPortN->setRate(dstPort->getRate());
            dstActor->addPort(srcPortN);
            
            comp = SDFcomponent(srcActor, srcActor->nrPorts());
            dstPortN = srcPort->create(comp);
            dstPortN->setName(CString("_p") + CString(srcActor->nrPorts()+1));
            dstPortN->setType("in");
            dstPortN->setRate(srcPort->getRate());
            srcActor->addPort(dstPortN);

            // Connect ports to channel
            cN->connectSrc(srcPortN);
            cN->connectDst(dstPortN);
            
            // Create relation between buffer channel and channel c
            cN->setStorageSpaceChannel(c);
            
            // Add new channel to the graph
            g->addChannel(cN);   
        }
    }
    
    return g;
}
