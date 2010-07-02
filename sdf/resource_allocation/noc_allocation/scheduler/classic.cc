/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   classic.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Classic NoC scheduling algorithm
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: classic.cc,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#include "classic.h"
#include "../problem/packet.h"

/**
 * classic ()
 * Classic NoC scheduling algorithm
 */
bool ClassicNoCScheduler::classic(const CSize maxDetour,
        const uint maxNrRipups)
{
    bool found;
    uint nrRipups = 0;
    
    // Sort all messages using cost function
    sortMessagesOnCost();
    
    // Iterate over sorted messages
    for (MessagesIter iter = messagesBegin(); iter != messagesEnd(); iter++)
    {
        Message *m = *iter;

        // Try to find a scheduling entity for the message
        found = findScheduleEntityForMessageUsingClassic(m, maxDetour);
        
        // Failed to find a scheduling entity?
        if (!found)
        {
            if (nrRipups < maxNrRipups)
            {
                // Ripup some already scheduled stream of message
                ripupStream(iter);
                nrRipups++;
                
                // Re-try current message
                iter--;
            }
            else
            {
                // Output slot tables on all links
                //print(cerr);
                cerr << "Failed finding scheduling entity for message: ";
                m->print(cerr);
                cerr << endl;

                return false;
            }
        }
    }

    // All messages scheduled.
    return true;
}

/**
 * findScheduleEntityForMessageUsingClassic ()
 * The function tries to find a schedule entity for the message. On success it
 * returns true and allocates the entity on the NoC. Else it returns false and
 * no resources are claimed.
 */
bool ClassicNoCScheduler::findScheduleEntityForMessageUsingClassic(Message *m, 
    const CSize maxDetour)
{
    NoCSchedulingEntity *eStream, *e = new NoCSchedulingEntity(m);
    Node *srcNode, *dstNode;
    SlotReservations slotReservations;
    CSize minLengthRoute, detour = 0;
    TTime maxDuration;
    Routes routes;

    // Get src and dst node in the architecture graph
    srcNode = getInterconnectGraph()->getNode(m->getSrcNodeId());
    dstNode = getInterconnectGraph()->getNode(m->getDstNodeId());
    if (srcNode == NULL || dstNode == NULL)
        throw CException("Node used in communication event does not exist in"
                            + CString(" architecture."));

    // Compute minimal length between src and dst node
    minLengthRoute = getLengthShortestPathBetweenNodes(srcNode, dstNode);

    // Scheduling entity starts as soon as possible
    e->setStartTime(earliestStartTime(m));

    // Maximal duration when using shortest route
    maxDuration = maximalDuration(m, e->getStartTime(), minLengthRoute);

    // Already scheduled other message of same stream
    eStream = getFirstScheduledMessageInStream(m);

    // No message scheduled of this stream?
    if (eStream == NULL)
    {
        // Select a route for the message
        do
        {
            // Cleanup list of routes
            routes.clear();

            // Duration is as long as possible considering length of route
            e->setDuration(maxDuration - detour);

            // Find all routes with the increasing detour and sort them on cost
            findAllRoutes(srcNode, dstNode, detour, true, routes);
            sortRoutesOnCostUsingClassic(routes, 
                                        e->getStartTime(), e->getDuration());

            // Try all routes in order of decreasing cost
            for (RoutesIter iter = routes.begin(); iter != routes.end(); iter++)
            {
                Route &r = *iter;

                // Set the route of the scheduling entity
                Route *re = new Route(r);
                e->setRoute(re);

                // Duration is as long as possible considering length of route
                e->setDuration(maxDuration - (re->length() - minLengthRoute));

                // Found a valid slot assignment?
                if (findSlotsOnRouteUsingClassic(e, slotReservations))
                {
                    e->setSlotReservations(slotReservations);

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

            // Increase detour
            detour++;
        } while (detour <= maxDetour);
    }
    else
    {
        // All messages of same stream must use same route
        Route *re = new Route(*eStream->getRoute());
        e->setRoute(re);

        // Duration is as long as possible considering length of route
        e->setDuration(maxDuration - (re->length() - minLengthRoute));

        // Found a valid slot assignment?
        if (findSlotsOnRouteUsingClassic(e, slotReservations))
        {
            e->setSlotReservations(slotReservations);

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
    }
    
    // Cleanup
    delete e;
    
    return false;
}

/**
 * sortRoutesOnCostUsingClassic ()
 * The function sorts all routes using a cost function with cost
 * from high to low. The cost is determined by the minimum number of free slots 
 * available overall links within the given time bounds.
 */
void ClassicNoCScheduler::sortRoutesOnCostUsingClassic(Routes &routes, 
    const TTime startTime, const TTime duration)
{
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    TTime startTimeLink, endTimeLink;
    bool loopPeriod;
    double costLink, cost;
    
    // Assign costs to all routes
    for (RoutesIter iter = routes.begin(); iter != routes.end(); iter++)
    {
        Route &r = *iter;
        cost = getInterconnectGraph()->getSlotTableSize();
        ulong linkNr = 0;
        
        // Iterate over all links in the route
        for (LinksIter iterL = r.linksBegin(); iterL != r.linksEnd(); iterL++)
        {
            Link *l = *iterL;

            costLink = getInterconnectGraph()->getSlotTableSize();
            
            // Compute start and end time on link
            startTimeLink = (startTime+linkNr)%slotTablePeriod;
            endTimeLink = (startTime+linkNr+duration-1)%slotTablePeriod;
            loopPeriod = false;
            
            // The scheduling entity loops around the period when its start time
            // is equal or more then its end time. In the former case, the
            // entity takes a complete period, in the latter case it is less
            // then a period.
            if (duration != 1 && startTimeLink >= endTimeLink)
                loopPeriod = true;
            
            // Find minimum number of free slots on l between time bounds (this
            // is the cost of a link)
            for (SlotTablesIter iterS = l->slotTableSeqBegin(); 
                    iterS != l->slotTableSeqEnd(); iterS++)
            {
                SlotTable &s = *iterS;
                
                // Slot table after start time
                if (s.getStartTime() >= startTimeLink
                        || (loopPeriod && s.getStartTime() <= endTimeLink))
                {
                    costLink = costLink < s.getNrFreeSlots() 
                                                ? costLink : s.getNrFreeSlots();
                }
                
                // Slot table ends after end time
                if (!loopPeriod && s.getEndTime() >= endTimeLink)
                    break;
            }

            // Cost of route is minimum of cost of links
            cost = cost < costLink ? cost : costLink;
            
            // Next link
            linkNr++;            
        }
        
        r.setCost(cost);
    }
    
    // Sort the routes
    routes.sort();
    routes.reverse();
}

/**
 * findSlotsOnRouteUsingClassic ()
 * The function looks for a set of slots which can be reserved along the route
 * such that flits entering a router leave it in the next slot and there is
 * enough space to transmit both the data and the packatization headers. On
 * success, the slot reservations for the first link of the route are stored in
 * s and the function returns true. On failure, the function returns false.
 * The function tries to minimize the number of packets used. The classic
 * reservation technique assumes that reconfiguration is not possible and slots
 * cannot be shared amongst streams.
 */
bool ClassicNoCScheduler::findSlotsOnRouteUsingClassic(NoCSchedulingEntity *e,
    SlotReservations &s)
{
    CSize nrPackets, nrSlots, slotTableSize, nrSlotsAlloc;
    bool startPacket, slotAlloc, foundAllSlots = false;
    SlotReservations slotsRoute;
    Packets packets;
    Packet *p;
    
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();

    // No slots reserved (yet)
    s.resize(slotTableSize);
    for (uint i = 0; i < slotTableSize; i++)
        s[i] = false;

    // Get slots reserved for messages in the same stream following
    // this route
    findSlotsAllocatedForStream(e, s);

    // Count number of slots already allocated
    nrSlotsAlloc = 0;
    for (uint i = 0; i < slotTableSize; i++)
        if (s[i])
            nrSlotsAlloc++;    

    // Find all packets already allocated in the slot table
    startPacket = true;
    for (uint i = 0; i < slotTableSize; i++)
    {
        // Slot must be used by same stream and within timing constraints
        slotAlloc = s[i];
        if (s[i] && e->getDuration() < TTime(slotTableSize))
        {
            slotAlloc = false;
            
            if (e->getStartTime() % slotTableSize 
                    > (e->getStartTime() + e->getDuration()) % slotTableSize)
            {
                // loop around
                if (i <= e->getStartTime() % slotTableSize
                        || i >= (e->getStartTime() + e->getDuration()) 
                                    % slotTableSize)
                {
                    slotAlloc = true;
                }
            }
            else
            {
                // no loop around
                if (i >= e->getStartTime() % slotTableSize 
                        && i <= (e->getStartTime() + e->getDuration()) 
                                    % slotTableSize)
                {
                    slotAlloc = true;
                }
            }
        }
        
        if (slotAlloc)
        {
            if (startPacket)
            {
                startPacket = false;
                
                p = new Packet();
                p->nrSlots = 1;
                p->startTime = i;
                p->endTime = i;
                
                packets.push_back(p);
            }
            else
            {
                packets.back()->nrSlots++;
                packets.back()->endTime = i;
            }
        }
        else
        {
            startPacket = true;
        }
    }
    
    // Last and first package connect in time (slot table rotation)
    // and currently seen as two packages?
    if (packets.size() > 1 && packets.front()->startTime == 0 
            && packets.back()->endTime == (TTime)(slotTableSize - 1))
    {
        // Merge last and first package
        packets.front()->nrSlots += packets.back()->nrSlots;
        packets.front()->startTime = packets.back()->startTime;
        packets.front()->loop = true;
        packets.pop_back();
    }

    // Try to extend existing packets
    if (packets.size() > 0)
    {
        nrSlots = nrSlotsRequired(e->getDuration(), e->getMessage()->getSize(), 
                                        packets.size());   

        // Enough slots allocated to sent message?
        if (nrSlots >= nrSlotsAlloc)
            return true;
    }
    else
    {
        nrSlots = nrSlotsRequired(e->getDuration(), 
                                    e->getMessage()->getSize(), 1);   
    }
    
    // Get slots not used for any stream (yet)
    findFreeSlotsForStream(e->getRoute(), slotsRoute);

    // Try to extend existing packets
    while (packets.size() > 0)
    {
        uint i;

        // Locate slot which can be added to a package
        for (i = 0; i < slotTableSize; i++)
        {
            // Slot i free?
            if (slotsRoute[i])
            {
                // Connects to package?
                if ((i != 0 && s[i-1]) 
                        || (i == 0 && s[slotTableSize-1]) 
                        || (i == slotTableSize-1 && s[0]) 
                        || (i != slotTableSize-1 && s[i+1]))
                {
                    break;
                }
            }
        }

        // No slot found?
        if (i == slotTableSize)
            break;

        // Add slot to package
        slotsRoute[i] = false;
        s[i] = true;
        nrSlotsAlloc++;

        // Enough slots to sent message?
        if (nrSlots >= nrSlotsAlloc)
            return true;
    }
        
    // Allocate extra packets
    nrPackets = packets.size() + 1;
    packets.clear();

    // Find all packets which can allocated in the slot table
    findFreePackets(e, slotsRoute, packets);

    // No packets available?
    if (packets.size() == 0)
        return false;
    
    // Sort packets on size
    packets.sort(PacketLess());
    packets.reverse();
        
    // First attempt: all slots in one extra package
    nrSlots = nrSlotsRequired(e->getDuration(), e->getMessage()->getSize(), 
                                        nrPackets);

    // Allocate slots in as few packages as possible
    for (PacketsIter iter = packets.begin(); iter != packets.end(); iter++)
    {
        p = *iter;
        
        // All slots needed?
        if (p->nrSlots <= nrSlots)
        {
            if (!p->loop)
            {
                for (uint i = p->startTime; i <= p->endTime; i++)
                    s[i] = true;
            }
            else
            {
                for (uint i = p->startTime; i < slotTableSize; i++)
                    s[i] = true;
                for (uint i = 0; i <= p->endTime; i++)
                    s[i] = true;
            }

            nrSlotsAlloc += p->nrSlots;    
        }
        else
        {
            // Not all slots are needed, allocate what is needed
            if (!p->loop)
            {
                for (uint i = p->startTime; i <= p->endTime; i++)
                {
                    s[i] = true;
                    nrSlotsAlloc++;

                    if (nrSlotsAlloc == nrSlots)
                        break;
                }
            }
            else
            {
                for (uint i = p->startTime; i < slotTableSize
                        && nrSlotsAlloc != nrSlots; i++)
                {
                    s[i] = true;
                    nrSlotsAlloc++;
                }
                for (uint i = 0; i <= p->endTime 
                            && nrSlotsAlloc != nrSlots; i++)
                {
                    s[i] = true;
                    nrSlotsAlloc++;
                }
            }
        } 
        
        // Enough slots allocated?
        if (nrSlotsAlloc == nrSlots)
        {
            foundAllSlots = true;
            break;
        }
        
        // Continue with slot allocation in next package
        // Check wether extra slots are needed for headers
        nrPackets++;
        nrSlots = nrSlotsRequired(e->getDuration(), e->getMessage()->getSize(), 
                                        nrPackets);
    }
    
    // Cleanup
    for (PacketsIter iter = packets.begin(); iter != packets.end(); iter++)
        delete *iter;

    // Done
    return foundAllSlots;
}

