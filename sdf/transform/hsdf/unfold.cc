/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   unfold.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 21, 2005
 *
 *  Function        :   Unfold an HSDF graph
 *
 *  History         :
 *      21-07-05    :   Initial version.
 *
 * $Id: unfold.cc,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#include "unfold.h"
#include "../../base/hsdf/check.h"

/**
 * unfoldHSDF ()
 * The function unfolds an HSDF graph for N times.
 */
SDFgraph *unfoldHSDF(SDFgraph *g, const uint N)
{
    SDFgraph *h;
    
    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g))
        throw CException("Graph is not an HSDF graph.");    
    
    // Construct a new HSDF graph
    SDFcomponent component = SDFcomponent(NULL, 0);
    h = g->createCopy(component);
    
    // Actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *gA = *iter;
        
        // Create N copies of the actor A in G
        for (uint i = 0; i < N; i++)
        {
            // Create actor a
            component = SDFcomponent(h, h->nrActors());
            SDFactor *a = gA->createCopy(component);
            a->setName(gA->getName() + CString("_") + CString(i));
            
            // Create ports on actor a
            for (SDFportsIter iter = gA->portsBegin(); 
                    iter != gA->portsEnd(); iter++)
            {
                SDFport *gP = *iter;
            
                // Create port
                component = SDFcomponent(a, a->nrPorts());
                SDFport *p = gP->createCopy(component);
                
                // Add port to actor
                a->addPort(p);
            }
            
            // Add actor to graph
            h->addActor(a);
        }
    }

    // Channels
    for (SDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *gC = *iter;
        
        uint d = gC->getInitialTokens();
        
        for (uint l = 0; l < d % N; l++)
        {
            uint k = N + l - d % N;
            
            // Source and destination port
            SDFport *hSrcP = h->getActor(gC->getSrcActor()->getName() 
                                            + CString("_") + CString(k)
                                        )->getPort(gC->getSrcPort()->getName());
            SDFport *hDstP = h->getActor(gC->getDstActor()->getName() 
                                            + CString("_") + CString(l)
                                        )->getPort(gC->getDstPort()->getName());
            
            // Create channel c
            component = SDFcomponent(h, h->nrChannels());
            SDFchannel *c = gC->createCopy(component);
            c->setName(gC->getName() + CString("_") + CString(l));
            c->connectSrc(hSrcP);
            c->connectDst(hDstP);
            
            // Initial tokens
            uint t = (uint)floor((double)(d)/(double)(N))+1;
            c->setInitialTokens(t);
            
            // Add channel to graph
            h->addChannel(c);
        }
        
        for (uint l = d % N; l < N; l++)
        {
            uint k = l - d % N;

            // Source and destination port
            SDFport *hSrcP = h->getActor(gC->getSrcActor()->getName() 
                                            + CString("_") + CString(k)
                                        )->getPort(gC->getSrcPort()->getName());
            SDFport *hDstP = h->getActor(gC->getDstActor()->getName() 
                                            + CString("_") + CString(l)
                                        )->getPort(gC->getDstPort()->getName());
            
            // Create channel c
            component = SDFcomponent(h, h->nrChannels());
            SDFchannel *c = gC->createCopy(component);
            c->setName(gC->getName() + CString("_") + CString(l));
            c->connectSrc(hSrcP);
            c->connectDst(hDstP);
            
            // Initial tokens
            uint t = (uint)floor((double)(d)/(double)(N));
            c->setInitialTokens(t);
            
            // Add channel to graph
            h->addChannel(c);
        }    
    }
    
    return h;
}

