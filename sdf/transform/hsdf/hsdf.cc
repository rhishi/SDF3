/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   hsdf.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 20, 2005
 *
 *  Function        :   HSDF graph
 *
 *  History         :
 *      20-07-05    :   Initial version.
 *
 * $Id: hsdf.cc,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#include "hsdf.h"
#include "../../base/algo/repetition_vector.h"

/**
 * transformSDFtoHSDF ()
 * The function transform a SDF graph into a HSDF graph.
 */
SDFgraph *transformSDFtoHSDF(SDFgraph *g)
{
    SDFgraph *h;
    
    // Construct a new (H)SDF graph
    SDFcomponent component = SDFcomponent(NULL, 0);
    h = g->createCopy(component);
    
    // Calculate repetition vector for the graph
    RepetitionVector repetitionVector = computeRepetitionVector(g);
    
    // Add actors to graph (number of actors depends on repetition vector)
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *gA = *iter;
        
        for (int i = 0; i < repetitionVector[gA->getId()]; i++)
        {
            // Create new actor
            component = SDFcomponent(h, h->nrActors());
            SDFactor *a = gA->createCopy(component);
            a->setName(gA->getName() + CString("_") + CString(i));
            
            // Add actor to graph
            h->addActor(a);
        }
    }
    
    // Channels and ports
    for (SDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *gC = *iter;
        
        // Source and destination port and actor in SDF graph
        SDFport *gSrcP = gC->getSrcPort();
        SDFport *gDstP = gC->getDstPort();
        SDFactor *gSrcA = gSrcP->getActor();
        SDFactor *gDstA = gDstP->getActor();

        uint nA = gSrcP->getRate();
        uint nB = gDstP->getRate();
        uint qA = repetitionVector[gSrcA->getId()];
        uint qB = repetitionVector[gDstA->getId()];
        uint d = gC->getInitialTokens();
        for (uint i = 1; i <= qA; i++)
        {
            // Get pointer to source actor
            SDFactor *hSrcA = h->getActor(gSrcA->getName() 
                                + CString("_") + CString(i-1));
            
            for (uint k = 1; k <= nA; k++)
            {
                uint l = 1 + (d + (i-1)*nA + k - 1) % (nB*qB);
                uint j = 1 + (uint)floor((double)((d + (i-1)*nA + k - 1) %
                                            (nB*qB)) / (double)(nB));
                
                // Create port on source node
                component = SDFcomponent(hSrcA, hSrcA->nrPorts());
                SDFport *hSrcP = gSrcP->createCopy(component);
                hSrcP->setName(gSrcP->getName()
                                    + CString("_") + CString(k-1));
                hSrcP->setRate(1);
                hSrcA->addPort(hSrcP);
                
                // Create port on destination node
                SDFactor *hDstA = h->getActor(gDstA->getName()
                                    + CString("_") + CString(j-1));
                component = SDFcomponent(hDstA, hDstA->nrPorts());
                SDFport *hDstP = gDstP->createCopy(component);
                hDstP->setName(gDstP->getName()
                                        + CString("_") + CString(l-1));
                hDstP->setRate(1);
                hDstA->addPort(hDstP);
                
                // Create channel between actors
                component = SDFcomponent(h, h->nrChannels());
                SDFchannel *c = gC->createCopy(component);
                c->setName(gC->getName() + CString("_") 
                                + CString((i-1)*nA+(k-1)));
                c->connectSrc(hSrcP);
                c->connectDst(hDstP);
                
                // Initial tokens
                uint t = (uint) floor((double)(d + (i-1)*nA + k-1) /
                                        (double)(nB*qB));
                c->setInitialTokens(t);
                
                // Add channel to graph
                h->addChannel(c);
            }
        }
    }
    
    return h;
}

