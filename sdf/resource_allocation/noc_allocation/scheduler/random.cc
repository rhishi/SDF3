/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   random.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Random-based NoC scheduling algorithm
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: random.cc,v 1.1 2008/03/20 16:16:21 sander Exp $
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

#include "random.h"

// Random number generator
static MTRand mtRand;

/**
 * random ()
 * Random-based NoC scheduling algorithm
 */
bool RandomNoCScheduler::random(const CSize maxDetour, const uint maxNrRipups, 
        const uint maxNrTries)
{
    uint nrRipups = 0, nrTries = 0;
    bool found, solved;
    
    while (nrTries < maxNrTries)
    {
        // Assume problem solved till proven otherwise
        solved = true;
        
        // Put messages in a random order
        putMessagesInRandomOrder();

        // Iterate over sorted messages
        for (MessagesIter iter = messagesBegin(); iter != messagesEnd(); iter++)
        {
            Message *m = *iter;

            // Try to find a scheduling entity for the message
            found = findScheduleEntityForMessageUsingRandom(m, maxDetour);

            // Failed to find a scheduling entity?
            if (!found && nrRipups < maxNrRipups && iter != messagesBegin())
            {
                // Ripup some already scheduled message
                ripupScheduleEntity(iter);
                nrRipups++;

                // Re-try current message
                iter--;
            } 
            else if (!found)
            {
                // Failed to schedule all messages
                solved = false;
                break;
            }
        }
        
        // All messages scheduled?
        if (solved)
            return true;
        
        // Next attempt
        nrTries++;

        // Unschedule already scheduled messages
        for (MessagesIter iter = messagesBegin(); iter != iter; iter++)
        {
            Message *m = *iter;
            releaseResources(m->getSchedulingEntity());
        }

    }

    // Output slot tables on all links
    //print(cerr);
    cerr << "Failed finding scheduling entity for all message. " << endl;
    
    // Failed to schedule all messages
    return false;
}

/**
 * putMessagesInRandomOrder ()
 * The function puts all messages in a random order by first assigning a
 * random cost to each messages and then sorting them using this cost.
 */
void RandomNoCScheduler::putMessagesInRandomOrder()
{
    double cost;
    
    // Assign costs to all messages
    for (MessagesIter iter = messagesBegin(); iter != messagesEnd(); iter++)
    {
        Message *m = *iter;
        
        cost = mtRand.rand();
        m->setCost(cost);
    }
    
    // Sort the messages
    getSchedulingProblem()->getMessages().sort(MessageLess());
}

/**
 * putRoutesInRandomOrder ()
 * The function puts all routes in a random order by first assigning a
 * random cost to each route and then sorting them using this cost.
 */
void RandomNoCScheduler::putRoutesInRandomOrder(Routes &routes)
{
    double cost;
    
    // Assign costs to all routes
    for (RoutesIter iter = routes.begin(); iter != routes.end(); iter++)
    {
        Route &r = *iter;
        
        cost = mtRand.rand();
        r.setCost(cost);
    }
    
    // Sort the routes
    routes.sort();
}

/**
 * findScheduleEntityForMessageUsingRandom ()
 * The function tries to find a schedule entity for the message. For this it
 * uses random choices on the start time, duration and route. On success it
 * returns true and allocates the entity on the NoC. Else it returns false and
 * no resources are claimed.
 */
bool RandomNoCScheduler:: findScheduleEntityForMessageUsingRandom(Message *m, 
    const CSize maxDetour)
{
    NoCSchedulingEntity *e = new NoCSchedulingEntity(m);
    Node *srcNode, *dstNode;
    SlotReservations slotReservations;
    CSize minLengthRoute, detour = 0;
    TTime maxDuration, minDuration, startTime, slotTablePeriod, duration;
    TTime slotTableSize;
    Routes routes;

    // Get src and dst node in the architecture graph
    srcNode = getInterconnectGraph()->getNode(m->getSrcNodeId());
    dstNode = getInterconnectGraph()->getNode(m->getDstNodeId());
    
    if (srcNode == NULL || dstNode == NULL)
        throw CException("Node used in communication event does not exist in"
                            + CString(" architecture."));
    
    // Slot table properties
    slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    slotTableSize = getInterconnectGraph()->getSlotTableSize();

    // Compute minimal length between src and dst node
    minLengthRoute = getLengthShortestPathBetweenNodes(srcNode, dstNode);

    // Minimal and maximal duration when using shortest route
    startTime = earliestStartTime(m);
    slotReservations.resize(slotTableSize);
    for (uint i = 0; i < slotTableSize; i++)
        slotReservations[i] = true;
    minDuration = slotTableSize + minimalDuration(m, 0, slotReservations); 
    maxDuration = maximalDuration(m, startTime, minLengthRoute);
    if (minDuration > maxDuration)
        return false;
            
    // Choice a random start time between the bounds
    //startTime += mtRand.randInt(maxDuration - minDuration);
    e->setStartTime(startTime%slotTablePeriod);

    // Find all routes with at most the specified detour and put them in a
    // random order
    findAllRoutes(srcNode, dstNode, detour, false, routes);
    putRoutesInRandomOrder(routes);

    // Try all routes
    for (RoutesIter iter = routes.begin(); iter != routes.end(); iter++)
    {
        Route &r = *iter;

        // Set the route of the scheduling entity
        Route *re = new Route(r);
        e->setRoute(re);

        // Choice a random duration between the bounds
        detour = re->length() - minLengthRoute;
        if ((long)(maxDuration) - (long)(detour) <= (long)(minDuration)) 
            continue;
        duration = mtRand.randInt(maxDuration - detour - minDuration);
        e->setDuration(duration + minDuration);

        // Found a valid slot assignment?
        if (findSlotsOnRoute(e, slotReservations))
        {
            e->setSlotReservations(slotReservations);

            // Enough time for actual communication?
            if (e->getDuration() < minimalDuration(m, 
                        e->getStartTime(), slotReservations))
                continue;

            // Minimize the duration
            e->setDuration(minimalDuration(m, e->getStartTime(),
                                slotReservations));

            // Reserve the resource in the NoC
            reserveResources(e);

            // Link scheduling entity to communication event
            m->setSchedulingEntity(e);

            // Done
            return true;
        }

        // Cleanup
        delete re;
        e->setRoute(NULL);
    }

    // Cleanup
    delete e;

    return false;
}
