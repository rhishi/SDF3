/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   single_processor_random_staticorder.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 3, 2008
 *
 *  Function        :   Find minimal latency using random static-order schedules
 *
 *  History         :
 *      08-03-03    :   Initial version.
 *
 * $Id: single_processor_random_staticorder.cc,v 1.1 2008/03/06 10:49:44 sander Exp $
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

#include "single_processor_random_staticorder.h"
#include "../../base/algo/repetition_vector.h"
#include "../../base/algo/components.h"

// Random number generator
static MTRand mtRand;

/**
 * fireActor ()
 * The function removes the number of tokens required for a firing of the actor
 * from all input ports and it produces tokens on all output ports.
 */
static
void fireActor(SDFactor *a)
{
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        
        if (p->getType() == SDFport::In)
        {
            if (c->getInitialTokens() < p->getRate())
            {
                cerr << "actor: " << a->getId() << endl;
                throw CException("Not enough tokens to fire actor.");
            }

            c->setInitialTokens(c->getInitialTokens() - p->getRate());
        }
        else
        {
            c->setInitialTokens(c->getInitialTokens() + p->getRate());
        }
    }
}

/**
 * isActorReady ()
 * The function checks wether there are enough tokens on the input ports
 * to fire the actor.
 */
static
bool isActorReady(SDFactor *a)
{
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        
        if (p->getType() == SDFport::In)
        {
            SDFchannel *c = p->getChannel();
            
            if (c->getInitialTokens() < p->getRate())
                return false;
        }
    }
    
    return true;
}

/**
 * updateActorReadyList ()
 * The function updates the list of ready actors after the firing of actor a.
 */
static
void updateActorReadyList(SDFactors &readyList, SDFactor *a,
        RepetitionVector &demandList)
{
    // Is actor a no longer ready?
    if (!isActorReady(a) || demandList[a->getId()] == 0)
    {
        // Remove actor a from the ready list
        for (SDFactorsIter iter = readyList.begin();
                iter != readyList.end(); iter++)
        {
            if ((*iter)->getId() == a->getId())
            {
                readyList.erase(iter);
                break;
            }
        }
    }
    
    // Which actors are ready because actor a fired?
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        
        if (p->getType() == SDFport::Out)
        {
            SDFactor *b = p->getChannel()->getDstActor();
            
            if (isActorReady(b) && demandList[b->getId()] > 0)
            {
                bool alreadyListed = false;
                
                // Add actor only when not yet in the list
                for (SDFactorsIter iter = readyList.begin();
                        iter != readyList.end() && !alreadyListed; iter++)
                {
                    if ((*iter)->getId() == b->getId())
                        alreadyListed = true;
                }
                if (!alreadyListed)
                    readyList.push_back(b);
            }
        }
    }
}

/**
 * getRandomActorFromActorReadyList ()
 * The function returns a random actor from the list.
 */
static
SDFactor *getRandomActorFromActorReadyList(SDFactors &readyList)
{
    int n;
    
    // Select a random actor from the list
    n = mtRand.randInt(readyList.size()-1);
    
    for (SDFactorsIter iter = readyList.begin();
            iter != readyList.end(); iter++)
    {
        if (n == 0)
            return *iter;
        n--;
    }
    
    return NULL;
}

/**
 * execSDFgraph ()
 * Fire the actors in the SDF graph. The number of firings is n (or less in case
 * of deadlock).
 */
static
SDFtime execSDFgraph(TimedSDFgraph *g, SDFactor *srcActor, SDFactor *dstActor)
{
    RepetitionVector demandList;
    bool firstSrcFiring = true;
    SDFtime latency = 0;
    SDFactors readyList;
    TimedSDFactor *a;
     
    // Initial demand list
    demandList = computeRepetitionVector(g);
    
    // Block source actor from firing
    demandList[srcActor->getId()] = 0;
    
    // Create initial list of ready actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        a = (TimedSDFactor*)(*iter);
    
        if (isActorReady(a) && demandList[a->getId()] > 0)
        {
            readyList.push_back(a);
        }
    }

    // Fire till deadlock
    while (!readyList.empty())
    {
        // Get a random actor from the list of ready actors
        a = (TimedSDFactor*)getRandomActorFromActorReadyList(readyList);
        ASSERT(a != NULL, "ready list is empty");
        
        // Fire the actor
        fireActor(a);
        
        // Update the list of ready actors
        updateActorReadyList(readyList, a, demandList);
    }

    // Update demand list
    demandList = computeRepetitionVector(g);

    // Create initial list of ready actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        a = (TimedSDFactor*)(*iter);
    
        if (isActorReady(a))
        {
            readyList.push_back(a);
        }
    }
    
    // Firings
    while (!readyList.empty())
    {
        // Get a random actor from the list of ready actors
        a = (TimedSDFactor*)getRandomActorFromActorReadyList(readyList);
        ASSERT(a != NULL, "ready list is empty");
        
        // Fire the actor
        fireActor(a);
        
        // Update demand list
        demandList[a->getId()]--;
        
        // Update the list of ready actors
        updateActorReadyList(readyList, a, demandList);
        
        // Update latency; firing source actor?
        if (a->getId() == srcActor->getId() && firstSrcFiring)
        {
            latency = a->getExecutionTime();
            firstSrcFiring = false;
        }
        else
        {
            latency += a->getExecutionTime();
        }
        
        // All firings destination actor complete?
        if (a->getId() == dstActor->getId() && demandList[a->getId()] == 0)
        {
            return latency;
        }
    }
    
    // Deadlock
    return UINT_MAX;
}

/**
 * latencyAnalysisForRandomStaticOrderSingleProc ()
 * The function computes the latency for a random static-order schedule on
 * a single processor system. The minimal latency for the specified number of
 * attempts is returned.
 */
extern
SDFtime latencyAnalysisForRandomStaticOrderSingleProc(TimedSDFgraph *g,
        SDFactor *srcActor, SDFactor *dstActor, uint nrAttempts)
{
    uint attempt = 0;
    SDFtime latency, minLatency = UINT_MAX;
    
    while (attempt < nrAttempts)
    {
        latency = execSDFgraph(g, srcActor, dstActor);
        
        if (latency < minLatency)
            minLatency = latency;
            
        attempt++;
    }
    
    return minLatency;
}
