/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   noc_scheduler.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Communication scheduling
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: noc_scheduler.cc,v 1.2 2008/11/01 16:06:17 sander Exp $
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

#include "noc_scheduler.h"

/**
 * assignSchedulingEntities ()
 * Load scheduling entities for the scheduling problems from XML.
 */
void NoCScheduler::assignSchedulingEntities(SetOfNoCScheduleProblems &problems,
        CNode *networkMappingNode)
{
    CString name;
    
    // Iterate over all messages nodes in the network node
    for (CNode *messagesNode = CGetChildNode(networkMappingNode, "messages");
            messagesNode != NULL; 
            messagesNode = CNextNode(messagesNode, "messages"))
    {
        // Name of the set of scheduling entities
        name = CGetAttribute(messagesNode, "name");
        
        // Locate the corresponding scheduling problem
        for (NoCScheduleProblemsIter iter = problems.scheduleProblemsBegin();
                iter != problems.scheduleProblemsEnd(); iter++)
        {
            NoCScheduleProblem *p = *iter;

            if (p->getName() == name)
            {
                // Set scheduling problem p as the current problem
                setSchedulingProblem(p);
                
                // Load scheduling entities
                assignSchedulingEntities(messagesNode);
            }
        }
    }
}

/**
 * assignSchedulingEntities ()
 * Load scheduling entities for the current scheduling problems from XML.
 */
void NoCScheduler::assignSchedulingEntities(CNode *messagesNode)
{
    InterconnectGraph *interconnectGraph = getInterconnectGraph();
    SlotReservations slotReservations;
    NoCSchedulingEntity *e;
    CStrings links, slots;
    MessagesIter msgIter;
    CString route, slot;
    CSize slotTableSize;
    Message *m;
    uint msgId;
    Route *r;

    // NoC properties
    slotTableSize = interconnectGraph->getSlotTableSize();
    
    // Iterate over all scheduling entities in the messages node
    for (CNode *schedulingNode = CGetChildNode(messagesNode, "schedulingEntity");
            schedulingNode != NULL; 
            schedulingNode = CNextNode(schedulingNode, "schedulingEntity"))
    {
        // Id of the corresponding message
        msgId = CGetAttribute(schedulingNode, "msg");
        
        // Find the corresponding message in the problem
        m = NULL;
        msgIter = messagesBegin();
        while (m == NULL && msgIter != messagesEnd())
        {
            if ((*msgIter)->getId() == msgId)
                m = *msgIter;
            msgIter++;
        }

        // Found corresponding message?
        if (m == NULL)
            throw CException("[ERROR] message does not exist.");
                        
        // Create a new scheduling entity for the message
        e = new NoCSchedulingEntity(m);
        
        // Start time of the scheduling entity
        e->setStartTime(CGetAttribute(schedulingNode, "startTime"));
        
        // Duration of the scheduling entity
        e->setDuration(CGetAttribute(schedulingNode, "duration"));

        // Route of the scheduling entity
        r = new Route;
        route = CGetAttribute(schedulingNode, "route");
        links = route.split(',');
        while (!links.empty())
        {
            r->appendLink(interconnectGraph->getLink(links.front()));
            links.pop_front();
        }
        e->setRoute(r);

        // Slot reservations
        slotReservations.resize(slotTableSize);
        for (uint i = 0; i < slotTableSize; i++)
            slotReservations[i] = false;
        slot = CGetAttribute(schedulingNode, "slots");
        slots = slot.split(',');
        while (!slots.empty())
        {
            if (uint(slots.front()) >= slotTableSize)
                throw CException("[ERROR] slot outside slot table.");
            slotReservations[slots.front()] = true;
            slots.pop_front();
        }
        e->setSlotReservations(slotReservations);

        // Reserve resources for the scheduling entity
        reserveResources(e);

        // Link the scheduling entity to the message
        m->setSchedulingEntity(e);
    }
}

/**
 * schedule ()
 * The function tries to find a valid schedule for all scheduling problems.
 * On success, the function returns true. Otherwise, it returns false.
 */
bool NoCScheduler::schedule(SetOfNoCScheduleProblems &problems)
{
    bool success = true;

    // Iterate over the list of schedule problems and solve them one by one
    for (NoCScheduleProblemsIter iter = problems.scheduleProblemsBegin();
            iter != problems.scheduleProblemsEnd(); iter++)
    {
        NoCScheduleProblem *p = *iter;
        
        // Set scheduling problem p as the current problem
        setSchedulingProblem(p);
        
        // Mark prefered slots in all links
        markPreferedSlotsOnLinks(problems);
        
        // Try to solve the problem
        p->setSolvedFlag(solve());
        
        // No schedule found?
        if (!p->isProblemSolved())
        {
            logError("Failed solving scheduling problem '"
                        + p->getName() + "'");
            success = false;
            
            // No chance of successfully solving the set of problems.
            break;
        }
    }
    
    return success;
}

/**
 * markPreferedSlotsOnLinks ()
 * The function marks every slot in every link of the interconnect graph
 * in problem as prefered (with maximimal level - UINT_MAX) when this
 * slot is used in any of the scheduling problems solved so far. This scheduler
 * uses this information to minimize the number of newly allocated slots.
 * Leaving as many slots free as possible for other applications.
 */
void NoCScheduler::markPreferedSlotsOnLinks(SetOfNoCScheduleProblems &problems)
{
    // Iterate over the list of schedule problems and solve them one by one
    for (NoCScheduleProblemsIter iter = problems.scheduleProblemsBegin();
            iter != problems.scheduleProblemsEnd(); iter++)
    {
        NoCScheduleProblem *p = *iter;
        
        // Problem p already solved?
        if (p->isProblemSolved())
        {
            getSchedulingProblem()->markPreferedSlots(p->getInterconnectGraph());
        }
    }
}

/**
 * sortMessagesOnCost ()
 * The function sorts all messages using a cost function with cost from high to
 * low. The cost of a communication event is determined by its size and time
 * bounds.
 */
void NoCScheduler::sortMessagesOnCost()
{
    double cost;
    
    // Assign costs to all messages
    for (MessagesIter iter = messagesBegin(); iter != messagesEnd(); iter++)
    {
        Message *m = *iter;
        
        cost = m->getSize() + 1 / (double)(m->getDuration());
        m->setCost(cost);
    }
    
    // Sort the messages
    getSchedulingProblem()->getMessages().sort(MessageLess());
    getSchedulingProblem()->getMessages().reverse();
}

/**
 * findScheduleEntityForMessage ()
 * The function tries to find a schedule entity for the message. On success it
 * returns true and allocates the entity on the NoC. Else it returns false and
 * no resources are claimed.
 */
bool NoCScheduler:: findScheduleEntityForMessage(Message *m, 
    const CSize maxDetour)
{
    NoCSchedulingEntity *e = new NoCSchedulingEntity(m);
    Node *srcNode, *dstNode;
    SlotReservations slotReservations;
    CSize slotTableSize, minLengthRoute, detour = 0;
    TTime maxDuration, minDuration;
    Routes routes;

    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();

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

    // Minimal duration (assuming all slots available)
    slotReservations.resize(slotTableSize);
    for (uint i = 0; i < slotTableSize; i++)
        slotReservations[i] = true;
    minDuration = minimalDuration(m, e->getStartTime(), slotReservations);

    do
    {
        // Cleanup list of routes
        routes.clear();

        // Duration is as long as possible considering length of route
        e->setDuration(maxDuration - detour);
        
        // Find all routes with the increasing detour and sort them on cost
        findAllRoutes(srcNode, dstNode, detour, true, routes);
        sortRoutesOnCost(routes, e->getStartTime(), e->getDuration());
        
        // Try all routes in order of decreasing cost
        for (RoutesIter iter = routes.begin(); iter != routes.end(); iter++)
        {
            Route &r = *iter;
            
            // Set the route of the scheduling entity
            Route *re = new Route(r);
            e->setRoute(re);

            // Duration is as long as possible considering length of route
            e->setDuration(maxDuration - (re->length() - minLengthRoute));

            // Try with decreasing duration to find feasible slot allocation
            while (e->getDuration() >= minDuration)
            {

                // Found a valid slot assignment?
                if (findSlotsOnRoute(e, slotReservations))
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
                
                // Decrease duration and try again
                e->setDuration(e->getDuration()/2);
            }
                        
            // Cleanup
            delete re;
            e->setRoute(NULL);
        }
    
        // Increase detour
        detour++;
    } while (detour <= maxDetour);
    
    // Cleanup
    delete e;
    
    return false;
}

/**
 * sortRoutesOnCost ()
 * The function sorts all routes using a cost function with cost
 * from high to low. The cost is determined by the minimum number of free slots 
 * available overall links within the given time bounds.
 */
void NoCScheduler::sortRoutesOnCost(Routes &routes, const TTime startTime,
    const TTime duration)
{
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    TTime startTimeLink, endTimeLink;
    bool loopPeriod;
    double costLink, cost;
    TTime start, end;
    
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
            costLink = 0;
            
            // Compute start and end time on link
            startTimeLink = (startTime+linkNr)%slotTablePeriod;
            endTimeLink = (startTime+linkNr+duration-1)%slotTablePeriod;
            loopPeriod = false;

            // The scheduling entity loops around the period when its start time
            // is equal or more then its end time. In the former case, the
            // entity takes a complete period, in the latter case it is less
            // then a period.
            if (startTimeLink >= endTimeLink && duration != 1)
                loopPeriod = true;
            
            // Compute number of free slots on l between time bounds (this
            // is the cost of a link)
            for (SlotTablesIter iterS = l->slotTableSeqBegin(); 
                    iterS != l->slotTableSeqEnd(); iterS++)
            {
                SlotTable &s = *iterS;
                
                // Slot table after start time
                if (s.getStartTime() >= startTimeLink
                        || (loopPeriod && s.getStartTime() <= endTimeLink))
                {
                    start = s.getStartTime() > startTimeLink ? 
                                s.getStartTime() : startTimeLink;
                    end = s.getEndTime() < endTimeLink ? 
                                s.getEndTime() : endTimeLink;

                    costLink = costLink + s.getNrFreeSlots() * (end - start);
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
 * findAllRoutes ()
 * The function finds all routes between the given source and destination node 
 * with the specified maximum detour.
 */
void NoCScheduler::findAllRoutes(const Node *src, const Node *dst,
        const CSize maxDetour, bool exact, Routes &routes)
{
    CSize minLength;
    Route route;

    // Compute shortest path between the source and destination
    minLength = getLengthShortestPathBetweenNodes(src,dst);

    // Create routes starting from source in all directions
    if (exact)
    {
        findRoutes(src, dst, minLength+maxDetour, minLength+maxDetour, 
                    route, routes);
    }
    else
    {
        findRoutes(src, dst, minLength, minLength+maxDetour, route, routes);
    }
}

/**
 * findAllRoutes ()
 * The function finds all routes between the given source and destination node 
 * within the specified minimum and maximum length.
 */
void NoCScheduler::findRoutes(const Node *src, const Node *dst,
        const CSize minLength, const CSize maxLength, Route &route, 
        Routes &routes)
{
    Route r;
    Link *l;
    
    // End of recursion if the source node is the destination node.
    if (src == dst)
    {
        if (route.length() >= minLength)
            routes.push_back(route);
        return;
    }

    // maximum length reached, but destination not reached
    if (maxLength == 0) return;

    // Continue with creating new routes along all outgoing links
    for (LinksCIter iter = src->outgoingLinksBegin(); 
            iter != src->outgoingLinksEnd(); iter++)
    {
        l = *iter;
        
        // Is the destination node not seen on this route before?
        if (!route.containsNode(l->getDstNode()))
        {
            // Create a new route with the link l added and continue till
            // destination is reached
            r = route;
            r.appendLink(l);
            findRoutes(l->getDstNode(), dst, minLength, maxLength-1, r, routes);
        }
    }
}

/**
 * getLengthShortestPathBetweenNodes ()
 * Compute the shortes path distance between two nodes.
 */
CSize NoCScheduler::getLengthShortestPathBetweenNodes(const Node *src, 
        const Node *dst)
{
    Nodes Q;
    Node *n, *m;
    Link *l;
    NodesIter iterN;
    
    // Initialization
    v_uint d(getInterconnectGraph()->nrNodes(), UINT_MAX);
    d[src->getId()] = 0;
    Q = getInterconnectGraph()->getNodes();
    
    while (!Q.empty())
    {
        // Extract min from Q
        n = NULL;
        for (NodesIter iter = Q.begin(); iter != Q.end(); iter++)
        {
            m = *iter;
            
            if (n == NULL || d[m->getId()] < d[n->getId()])
            {
                n = m;
                iterN = iter;
            }
        }
        Q.erase(iterN);
        
        // Reached destination?
        if (n == dst)
            return d[n->getId()];
        
        for (LinksCIter iter = n->outgoingLinksBegin();
                iter != n->outgoingLinksEnd(); iter++)
        {
            l = *iter;
            m = l->getDstNode();
            
            if (d[m->getId()] > d[n->getId()] + 1)
                d[m->getId()] = d[n->getId()] + 1;
        }
    }
    
    return UINT_MAX;
}

/**
 * minFreeSlotsOnLink ()
 * The function returns the minimum number of slots available on the link
 * between the given timing constraints.
 */
uint NoCScheduler::minFreeSlotsOnLink(Link *l, const TTime startTime,
        const TTime duration)
{
    uint min = getInterconnectGraph()->getSlotTableSize();
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    TTime endTime = (startTime + duration)%slotTablePeriod;
    bool loopPeriod = false;
    
    // The scheduling entity loops around the period when its start time
    // is equal or more then its end time. In the former case, the scheduling
    // entity takes a complete period, in the latter case it is less then a
    // period.
    if (startTime >= endTime && duration != 1)
        loopPeriod = true;
        
    // Iterate over slot tables to find slot table associated with start time
    for (SlotTablesIter iter = l->slotTableSeqBegin();
            iter != l->slotTableSeqEnd(); iter++)
    {
        SlotTable &s = *iter;
        
        if ((s.getStartTime() >= startTime || (loopPeriod 
                && s.getStartTime() <= endTime)) && s.getNrFreeSlots() < min)
            min = s.getNrFreeSlots();
                
        if (!loopPeriod && s.getEndTime() >= endTime)
            return min;
    }
    
    return min;
}

/**
 * findFreePackets ()
 * The function locates all available packets within the timing constraints and
 * available slots on the route (slotsRoute).
 */
void NoCScheduler::findFreePackets(NoCSchedulingEntity *e,
    SlotReservations &slotsRoute, Packets &packets)
{
    CSize slotTableSize;
    bool startPacket;
    Packet *p;
    
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();
    
    // More then one slot table rotation?
    if (e->getDuration() >= TTime(slotTableSize))
    {
        // Find all packets which can allocated in the slot table
        startPacket = true;
        for (uint i = 0; i < slotTableSize; i++)
        {
            if (slotsRoute[i])
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
    }
    else
    {
        // Slot table rotation?
        if (e->getStartTime()%slotTableSize >
                (e->getStartTime()+e->getDuration())%slotTableSize)
        {
        
            // Find all packets which can allocated in the slot table
            startPacket = true;
            for (uint i = 0; 
                    i < (e->getStartTime()+e->getDuration())%slotTableSize; i++)
            {
                if (slotsRoute[i])
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

            startPacket = true;
            for (uint i = e->getStartTime()%slotTableSize; 
                    i < slotTableSize; i++)
            {
                if (slotsRoute[i])
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
        }
        else
        {
            startPacket = true;
            for (uint i = e->getStartTime()%slotTableSize; 
                    i < (e->getStartTime()+e->getDuration())%slotTableSize; i++)
            {
                if (slotsRoute[i])
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
}

/**
 * findSlotsOnRoute ()
 * The function looks for a set of slots which can be reserved along the route
 * such that flits entering a router leave it in the next slot and there is
 * enough space to transmit both the data and the packatization headers. On
 * success, the slot reservations for the first link of the route are stored in
 * s and the function returns true. On failure, the function returns false.
 * The function tries to minimize the number of packets used.
 */
bool NoCScheduler::findSlotsOnRoute(NoCSchedulingEntity *e, SlotReservations &s)
{
    SlotReservations slotsRoute;
    uint linkSeqNr, posSlotTable;
    CSize slotTableSize;
    bool slotsAllocated;
    int prefLevel;
    Route *r;
    
    // Route
    r = e->getRoute();
    
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();

    // Compute all slots available along the route
    slotsRoute = findFreeSlotsOnRoute(e->getRoute(), e->getStartTime(),
                                        e->getDuration());

    // Compute slot preferences
    linkSeqNr = 0;
    vector<int> slotPreferences(slotTableSize, 0);
    for (LinksIter iterL = r->linksBegin(); iterL != r->linksEnd(); iterL++)
    {
        Link *l = *iterL;
        
        for (uint i = 0; i < slotTableSize; i++)
        {
            // Is slot i on link l used in a scheduling problem or entity?
            if (l->preferredSlots[i] > 0)
            {
                slotPreferences[(slotTableSize + i - linkSeqNr)
                                    % slotTableSize]++;
            }
        }
        
        // Next link in the route
        linkSeqNr++;
    }

    // Select all slots which are at the highest preference level
    SlotReservations slotsForPackets(slotTableSize, false);
    for (uint i = 0; i < slotTableSize; i++)
    {
        if (slotsRoute[i] && slotPreferences[i] == (int)(r->length()))
            slotsForPackets[i] = true;
    }

    // Allocate slots by decreasing preference level
    prefLevel = r->length() - 1;
    posSlotTable = 0;
    slotsAllocated = false;
    while (!slotsAllocated && prefLevel >= 0)    
    {
        bool addSlot = false;

        // Try to allocate slots using the currently selected slots
        slotsAllocated = findSlotsOnRoute(e, slotsForPackets, s);

        // Failed slot allocation?
        if (!slotsAllocated)
        {
            // Add next slot
            while (!addSlot && prefLevel >= 0)
            {
                // Try to find slot at current preference level
                while (!addSlot && posSlotTable < slotTableSize)
                {
                    if (slotsRoute[posSlotTable] 
                            && slotPreferences[posSlotTable] == prefLevel)
                    {
                        slotsForPackets[posSlotTable] = true;
                        addSlot = true;
                    }

                    // Next slot in table
                    posSlotTable++;
                }

                // Failed to find slot at current level?
                if (!addSlot)
                {
                    // Lower preference level and try again
                    prefLevel--;
                    posSlotTable = 0;
                }
            }
        }
    }

    return slotsAllocated;
}

/**
 * findSlotsOnRoute ()
 * The function looks for a set of slots which can be reserved along the route
 * such that flits entering a router leave it in the next slot and there is
 * enough space to transmit both the data and the packatization headers. On
 * success, the slot reservations for the first link of the route are stored in
 * s and the function returns true. On failure, the function returns false.
 * The function tries to minimize the number of packets used. It only uses
 * slots from the slots marked in slotsForPackets. The caller of the function
 * must guarantee that those slots are actually available for sending the data.
 */
bool NoCScheduler::findSlotsOnRoute(NoCSchedulingEntity *e,
        SlotReservations &slotsForPackets, SlotReservations &s)
{
    CSize nrPackets, nrSlots, slotTableSize, nrSlotsAlloc;
    bool slotsAllocated;
    Packets packets;
    Packet *p;

    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();

    // No slots reserved yet
    slotsAllocated = false;
    s.resize(slotTableSize);
    for (uint i = 0; i < slotTableSize; i++)
        s[i] = false;

    // Find all packets which can allocated in the slot table
    findFreePackets(e, slotsForPackets, packets);
        
    // No packets available?
    if (packets.size() == 0)
        return false;
    
    // Sort packets on size
    packets.sort(PacketLess());
    packets.reverse();
        
    // First attempt: all slots in one package
    nrPackets = 1;
    nrSlotsAlloc = 0;
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
            slotsAllocated = true;
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
    return slotsAllocated;

#ifdef __SINGLE_PACKET
    SlotReservations slotsRoute;
    CSize nrPackets = 1;
    CSize nrSlots, nrFreeSlots = 0;
    CSize slotTableSize = getInterconnectGraph()->getSlotTableSize();
    CSize slotCnt = 0;

    // Compute all slots available along the route
    slotsRoute = findFreeSlotsOnRoute(e->getRoute(), e->getStartTime(),
                                        e->getDuration());

    // Count the number of free slots in the route
    for (uint i = 0; i < slotTableSize; i++)
        if (slotsRoute[i])
            nrFreeSlots++;

    // Compute number of slots needed in slot table to sent data
    nrSlots = nrSlotsRequired(e->getDuration(), e->getMessage()->getSize(),
                                nrPackets);
    
    // Enough slots available?
    if (nrFreeSlots < nrSlots)
    {
        s.resize(slotTableSize);
        for (uint i = 0; i < slotTableSize; i++)
            s[i] = false;
        return false;
    }

    // Locate required number of slots in slot table (first sequence of
    // required nr of slots (nrSlots).
    for (uint i = 0; i < slotTableSize; i++)
    {
        // Slot i free?
        if (slotsRoute[i])
            slotCnt++;
        else
            slotCnt = 0;

        // Found sequence containing enough slots?
        if (slotCnt == nrSlots)
        {
            // Mark slots which should be used with true in slot
            // reservation.
            s.resize(slotTableSize);
            for (uint i = 0; i < slotTableSize; i++)
                s[i] = false;
            for (uint j = i-slotCnt+1; j < i+1; j++)
                s[j] = true;

            return true;
        }
    }
        
    // Failed to find required number of slots
    return false;
#endif
}

/**
 * nrSlotsRequired ()
 * The function computes the number of slots required to sent a communication
 * event within the given time bounds along a route. It takes the packatization
 * overhead into account.
 */
uint NoCScheduler::nrSlotsRequired(const TTime duration, const CSize size,
    const uint nrPacketsPerSlotTable)
{
    CSize szTotal, szFlit, szHeader;
    CSize nrSlots, nrPackets, nrSlotsPerSlotTable;
    long slotTableRotations, slotTableSize;
    
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();
    szFlit = getInterconnectGraph()->getFlitSize();
    szHeader = getInterconnectGraph()->getPacketHeaderSize();

    // Less then one slot table rotation possible?
    if (duration < TTime(slotTableSize))
    {
        szTotal = szHeader * nrPacketsPerSlotTable + size;
        nrSlots = (CSize)ceil(szTotal / (double)(szFlit));
        return nrSlots;
    }

    // Number of complete slot table rotations within duration
    slotTableRotations = duration / slotTableSize;

    // At least one rotation should be possible
    if (slotTableRotations == 0)
        throw CException("Number of slot table rotations must be > 0."); 

    // So many slot rotations are possible that an overflow occurs, let's
    // round the number of rotations back to a reasonable amount...
    if (slotTableRotations < 0)
        slotTableRotations = LONG_MAX;
    
    // Compute total size to be sent (= data + headers)
    nrPackets = nrPacketsPerSlotTable * (CSize)slotTableRotations;
    szTotal = szHeader * nrPackets + size;
    
    // Compute number of slots needed per slot table rotation
    nrSlots = (CSize)ceil(szTotal / (double)(szFlit));
    nrSlotsPerSlotTable = (CSize)ceil(nrSlots / (double)(slotTableRotations));

    return nrSlotsPerSlotTable;
}

/**
 * findFreeSlotsOnRoute ()
 * The function computes all slots which are available in consecutive links
 * along a route within the given time bounds. Available slots are marked with
 * true in the slot reservations.
 */
SlotReservations NoCScheduler::findFreeSlotsOnRoute(const Route *r, 
    const TTime startTime, const TTime duration)
{
    CSize slotTableSize = getInterconnectGraph()->getSlotTableSize();
    SlotReservations slotsRoute(slotTableSize, true);
    SlotReservations slotsLink;
    uint linkNr = 0;
    
    // Iterate over all links in the route and combine the available slots
    // in the links
    for (LinksCIter iter = r->linksBegin(); iter != r->linksEnd(); iter++)
    {
        Link *l = *iter;
        
        if (iter == r->linksBegin())
            slotsLink = findFreeSlotsOnFirstLinkRoute(r, startTime, duration);
        else
            slotsLink = findFreeSlotsOnLink(l, startTime + linkNr, duration);

        // Combine available slots of this link with route
        for (uint i = 0; i < slotTableSize; i++)
        {
            slotsRoute[i] = slotsRoute[i] 
                                && slotsLink[(i+linkNr)%slotTableSize];
        }
        
        // Next link
        linkNr++;
    }

    return slotsRoute;
}

/**
 * findFreeSlotsOnLink ()
 * The function computes all slots which are available in a link within the
 * given time bounds. These slots are marked as true within the slot
 * reservations.
 */
SlotReservations NoCScheduler::findFreeSlotsOnLink(const Link *l, 
    const TTime startTime, const TTime duration)
{
    uint slotTableSize = getInterconnectGraph()->getSlotTableSize();
    SlotReservations slotsLink(slotTableSize,true);
    TTime slotTablePeriod, entStartTime, entEndTime;
    SlotReservations slotsSlot;
    bool loopPeriod;
    
    // Compute time frame for scheduling entity
    slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    entStartTime = startTime % slotTablePeriod;
    entEndTime = (startTime + duration - 1) % slotTablePeriod;
    loopPeriod = false;

    // The scheduling entity loops around the period when its start time
    // is equal or more then its end time. In the former case, the scheduling
    // entity takes a complete period, in the latter case it is less then a
    // period.
    if (entStartTime >= entEndTime && duration != 1)
        loopPeriod = true;
    
    // Iterate over all slots in the link    
    for (SlotTablesCIter iter = l->slotTableSeqBegin();
            iter != l->slotTableSeqEnd(); iter++)
    {
        const SlotTable &s = *iter;
        
        if (s.getEndTime() >= entStartTime 
                || (loopPeriod && s.getStartTime() <= entEndTime))
        {
            slotsSlot = s.getSlotReservations(NULL);
            
            // Combine slot reservations of this 
            for (uint i = 0; i < slotTableSize; i++)
                slotsLink[i] = slotsLink[i] && slotsSlot[i];
        }
        
        // Slot table ends after end time?
        if (!loopPeriod && s.getEndTime() >= entEndTime)
            break;
    }
    
    return slotsLink;
}

/**
 * findFreeSlotsOnFirstLinkRoute ()
 * The function computes all slots which are available in the first link of a
 * route within the given time bounds, taking into account the time needed to
 * reconfigure the NI. These slots are marked as true within the slot
 * reservations.
 */
SlotReservations NoCScheduler::findFreeSlotsOnFirstLinkRoute(const Route *r, 
    const TTime startTime, const TTime duration)
{
    TTime entStartTime, entEndTime, reconfTime;
    bool loopPeriod, loopReconf;
    SlotReservations slotsSlot;

    // First link
    const Link *l = *r->linksBegin();

    // NoC properties
    uint slotTableSize = getInterconnectGraph()->getSlotTableSize();
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    SlotReservations slotsLink(slotTableSize,true);

    // Compute time at which communication ends
    entStartTime = startTime % slotTablePeriod;
    entEndTime = (startTime + duration) % slotTablePeriod;
    loopPeriod = false;

    // The scheduling entity loops around the period when its start time
    // is equal or more then its end time. In the former case, the scheduling
    // entity takes a complete period, in the latter case it is less then a
    // period.
    if (entStartTime >= entEndTime && duration != 1)
        loopPeriod = true;

    // Compute time at which reconfiguration starts
    reconfTime = slotTablePeriod + startTime 
                        - getInterconnectGraph()->getReconfigurationTimeNI();
    reconfTime = reconfTime % slotTablePeriod;
    loopReconf = (reconfTime >= entStartTime ? true : false);

    // Iterate over all slot tables
    for (SlotTablesCIter iter = l->slotTableSeqBegin();
            iter != l->slotTableSeqEnd(); iter++)
    {
        const SlotTable &s = *iter;

        if (s.getEndTime() >= reconfTime || 
                ((loopReconf || loopPeriod) && s.getStartTime() <= entEndTime))
        {
            slotsSlot = s.getSlotReservations(NULL);

            // Is slots s in the reconfiguration time and not in actual time of
            // communication?
            if (((s.getEndTime() >= reconfTime && s.getEndTime() < entStartTime)
                        || (loopReconf && s.getStartTime() <= entStartTime))
                    && (!(s.getEndTime() >= entStartTime
                        || (loopPeriod && entEndTime <= s.getStartTime()))))
                    
            {
                // Check wether occupied slots are using the same route
                // as the route currently examined by the function.
                uint i = 0;
                for (NoCSchedulingEntitiesCIter iterS = s.begin();
                        iterS != s.end(); iterS++)
                {
                    NoCSchedulingEntity *e = *iterS;
                    
                    if (!slotsSlot[i] && e != NULL && *e->getRoute() == *r)
                        slotsSlot[i] = true;
                        
                    // Next slot table entry
                    i++;
                }
            }            
            
            // Combine slot reservations of this 
            for (uint i = 0; i < slotTableSize; i++)
                slotsLink[i] = slotsLink[i] && slotsSlot[i];
        }
        
        // Slot table ends after end time?
        if (!loopPeriod && s.getEndTime() >= entEndTime)
            break;
    }

    return slotsLink;
}

/**
 * severityConflictSchedulingEntities ()
 * The function computes the severity of a conflict between a scheduling entity
 * 'e' and a message 'm' which must be scheduled over a route 'r'. The severity
 * is defined as the number of slots used by 'e' on the route 'r'. Only slots
 * within the time bounds of 'm' are counted.
 */
double NoCScheduler::severityConflict(NoCSchedulingEntity *e, Message *m, 
    Route *r)
{
    double severity = 0, severityLink = 0;
    TTime e1startTime, e1endTime, e2startTime, e2endTime;
    bool e1loopPeriod, e2loopPeriod;
    SlotReservations slotReservations;
    Link *l1, *l2;

    // NoC properties
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    CSize slotTableSize = getInterconnectGraph()->getSlotTableSize();

    // Start of communication    
    e1startTime = e->getStartTime() % slotTablePeriod;
    e2startTime = m->getStartTime() % slotTablePeriod;

    // End of communication
    e1endTime = (e->getStartTime() + e->getDuration()
                    + e->getRoute()->length() - 1) % slotTablePeriod;
    e2endTime = (m->getStartTime() + m->getDuration()
                    + r->length() - 1) % slotTablePeriod;

    // Continue in next period?
    e1loopPeriod = false;
    if (e1startTime >= e1endTime && e->getDuration() != 1)
        e1loopPeriod = true;
    e2loopPeriod = false;
    if (e2startTime >= e2endTime && m->getDuration() != 1)
        e2loopPeriod = true;

    // Is there no overlap in time between the scheduling entities?
    if ((!e1loopPeriod && !e2loopPeriod && (e1startTime > e2endTime 
                    || e1endTime < e2startTime)) 
            || (e1loopPeriod && !e2loopPeriod  && (e1startTime > e2endTime 
                    && e1endTime < e2startTime)) 
            || (!e1loopPeriod && e2loopPeriod  && (e2startTime > e1endTime 
                    && e2endTime < e1startTime)))
    {
        return 0;
    }

    // Each link has the same severity if it conflicts (- the number of slots
    // allocated on all links is equal)
    slotReservations = e->getSlotReservations();
    for (uint i = 0; i < slotTableSize; i++)
        if (slotReservations[i] == true)
            severityLink += 1;

    // Iterate over links in route of m.
    for (LinksCIter iterL2 = r->linksBegin(); iterL2 != r->linksEnd(); iterL2++)
    {
        l2 = *iterL2;
        
        // Link l2 also used in route of e?
        for (LinksCIter iterL1 = e->getRoute()->linksBegin(); 
                iterL1 != e->getRoute()->linksEnd(); iterL1++)
        {
            l1 = *iterL1;
    
            if (l1 == l2)
            {
                // Compute severity of the conflict for the links l1.
                // This is the number of slots used by e on link l1.
                severity += severityLink;
            }
        }    
    }
    
    return severity;
};

/**
 * earliestStartTime ()
 * The function returns the earliest start time for the message m. The earliest 
 * start time is always later then the latest end time of messages earlier in
 * the same stream.
 */
TTime NoCScheduler::earliestStartTime(const Message *m) const
{
    Message *prevMsg;
    NoCSchedulingEntity *e;

    // Find last message before m in the stream which is scheduled
    prevMsg = m->getPreviousMessageInStream();
    while (prevMsg != NULL && prevMsg->getSchedulingEntity() == NULL)
        prevMsg = prevMsg->getPreviousMessageInStream();

    // No earlier message in the stream already scheduled?
    if (prevMsg == NULL)
        return m->getStartTime();

    // Scheduling entity of previous message
    e = prevMsg->getSchedulingEntity();

    // Message m starts after complete communication of previous message?
    if (m->getStartTime() > e->getStartTime() + e->getDuration())
        return m->getStartTime();
    
    // Message m must wait till previous message is completed
    return e->getStartTime() + e->getDuration() + 1;
}

/**
 * maximalDuration ()
 * The function returns the maximal duration for the message m. It considers the
 * start time of messages which arrive later in the stream (i.e. higher sequence
 * number) and the start time of this message. 
 */
TTime NoCScheduler::maximalDuration(const Message *m, TTime startTime,
    CSize lengthRoute) const
{
    Message *nextMsg;
    NoCSchedulingEntity *e;
    TTime duration;

    // Find first message after m in the stream which is scheduled
    nextMsg = m->getNextMessageInStream();
    while (nextMsg != NULL && nextMsg->getSchedulingEntity() == NULL)
        nextMsg = nextMsg->getNextMessageInStream();

    // No later message in the stream already scheduled?
    if (nextMsg == NULL)
    {
        duration = m->getDuration() + m->getStartTime() 
                            - startTime - lengthRoute + 1;

        return duration;
    }
    
    // Scheduling entity of next message
    e = nextMsg->getSchedulingEntity();
    
    // Message m must end before next message even starts?
    if (m->getStartTime() + m->getDuration() + TTime(lengthRoute)
                            < e->getStartTime() + e->getRoute()->length())
    {
        duration = m->getDuration() + m->getStartTime() 
                            - startTime - lengthRoute + 1;

        return duration;
    }

    // Message m must finish before next message starts arriving at destination
    duration = (e->getStartTime() + e->getRoute()->length() - startTime 
                    - lengthRoute + 1);

    return duration;
}

/**
 * minimalDuration ()
 * The function returns the minimal duration for the message m. It considers the
 * start time of messages the message given by 'startTime', the length of the
 * route given by 'lengthRoute', and the slot reservations given by
 * 'slotReservations'. The later is used to take the packatization overhead
 * into account.
 */
TTime NoCScheduler::minimalDuration(const Message *m, TTime startTime,
    SlotReservations slotReservations) const
{
    TTime duration;
    long szData, szFlit, szHeader;
    uint curSlot, slotTableSize;
    bool startPacket = true;
    
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();
    szFlit = getInterconnectGraph()->getFlitSize();
    szHeader = getInterconnectGraph()->getPacketHeaderSize();
    
    // Size of the message which must be sent
    szData = m->getSize();
    
    // Synchronize startTime with correct slot entry
    curSlot = startTime % slotTableSize;
    duration = 1;

    while (szData > 0)
    {
        // Sent data in current slot?
        if (slotReservations[curSlot])
        {
            // Start of packet?
            if (startPacket)
            {
                szData = szData - szFlit + szHeader;
                startPacket = false;
            }
            else
            {
                szData = szData - szFlit;
            }
        }
        else
        {
            startPacket = true;
        }
        
        // Next time instance
        duration++;
        curSlot = (curSlot + 1) % slotTableSize;
    }
    
    return duration;
}

/**
 * reserveResources ()
 * Claim resources in the architecture graph for the scheduling entity e.
 */
void NoCScheduler::reserveResources(NoCSchedulingEntity *e)
{
    uint linkNr = 0;
    CSize slotTableSize = getInterconnectGraph()->getSlotTableSize();
    SlotReservations slotReservations = e->getSlotReservations();
    SlotReservations slotsLink(slotTableSize);

    if (e == NULL)
        return;

    // Iterate over all links in the route
    for (LinksIter iter = e->getRoute()->linksBegin();
            iter != e->getRoute()->linksEnd(); iter++)
    {
        Link *l = *iter;

        // Construct cyclically shifted slot reservations for the link
        for (uint i = 0; i < slotTableSize; i++)
            slotsLink[(i+linkNr)%slotTableSize] = slotReservations[i];

        l->reserveSlots(e, slotsLink, linkNr);
        
        // Next link
        linkNr++;
    }
    
    // Reserved resources for scheduling entity
    e->resourcesReserved(true);

    // Raise preference level slots on route
    raisePreferenceLevelSlotsOnRoute(e);
}

/**
 * releaseResources ()
 * Release resources in the architecture graph claimed by the scheduling 
 * entity e.
 */
void NoCScheduler::releaseResources(NoCSchedulingEntity *e)
{
    ulong linkNr = 0;

    if (e == NULL)
        return;

    // Iterate over all links in the route
    for (LinksIter iter = e->getRoute()->linksBegin();
            iter != e->getRoute()->linksEnd(); iter++)
    {
        Link *l = *iter;

        l->releaseSlots(e, linkNr);
        linkNr++;
    }

    // Released resources needed for scheduling entity
    e->resourcesReserved(false);

    // Lower preference level slots on route
    lowerPreferenceLevelSlotsOnRoute(e);
}

/**
 * ripupScheduleEntity ()
 * Remove the schedule entity which has the largest conflict with the given
 * message.
 */
void NoCScheduler::ripupScheduleEntity(MessagesIter iterMsg)
{
    double conflict, maxConflict = 0;
    MessagesIter iterMaxConflict = iterMsg;
    Node *srcNode, *dstNode;
    NoCSchedulingEntity *e;
    Routes routes;
    Message *m;

    // Message
    m = *iterMsg;
    
    // Get src and dst node in the architecture graph
    srcNode = getInterconnectGraph()->getNode(m->getSrcNodeId());
    dstNode = getInterconnectGraph()->getNode(m->getDstNodeId());
    if (srcNode == NULL || dstNode == NULL)
        throw CException("Node used in communication event does not exist in"
                            + CString(" architecture."));

    // Find the best shortest route for the message
    findAllRoutes(srcNode, dstNode, 0, true, routes);
    sortRoutesOnCost(routes, m->getStartTime(), m->getDuration());

    // Find largest conflicting scheduling entity    
    for (MessagesIter iter = messagesBegin(); iter != iterMsg; iter++)
    {
        if ((*iter)->getSchedulingEntity() == NULL)
            continue;
        if (routes.size() == 0)
            continue;
            
        conflict = severityConflict((*iter)->getSchedulingEntity(), m,
                                            &(routes.front()));
        
        if (conflict > maxConflict)
        {
            maxConflict = conflict;
            iterMaxConflict = iter;
        }
    }
    
    // Release resources claimed by largest conflicting scheduling entity
    e = (*iterMaxConflict)->getSchedulingEntity();
    releaseResources(e);
    (*iterMaxConflict)->setSchedulingEntity(NULL);
    delete e;
    
    // Insert removed message after iterMsg in list to have it scheduled again.
    getSchedulingProblem()->getMessages().insert(++iterMsg, (*iterMaxConflict));
    getSchedulingProblem()->getMessages().erase(iterMaxConflict);
}

/**
 * ripupStream ()
 * Remove schedule of all messages which belong to the stream.
 */
void NoCScheduler::ripupStream(MessagesIter iterMsg)
{
    CId streamId = (*iterMsg)->getStreamId();
    NoCSchedulingEntity *e;
    Message *m;
    
    for (MessagesIter iter = messagesBegin(); iter != messagesEnd(); iter++)
    {
        m = *iter;
        
        if (m->getStreamId() == streamId && m->getSchedulingEntity() != NULL)
        {
            // Ripup scheduling entity of the message
            e = m->getSchedulingEntity();
            releaseResources(e);
            m->setSchedulingEntity(NULL);
            delete e;

            // Insert removed message after iterMsg in list 
            // to have it scheduled again.
            getSchedulingProblem()->getMessages().insert(++iterMsg, m);
            getSchedulingProblem()->getMessages().erase(iter);
        }    
    }
}

/**
 * getFirstScheduledMessageInStream ()
 * The function returns a pointer to the scheduling entity of the
 * first message in the stream which is scheduled. Or NULL if no such
 * message exists.
 */
NoCSchedulingEntity *NoCScheduler::getFirstScheduledMessageInStream(Message *m)
{
    Message *x = m;
    
    // Find first message in stream
    while (x->getPreviousMessageInStream() != NULL)
    {
        x = x->getPreviousMessageInStream();
    }

    // Find first message which is scheduled starting from x
    while (x != NULL)
    {
        // Is x scheduled?
        if (x->getSchedulingEntity() != NULL)
            return x->getSchedulingEntity();
    
        // Next message
        x = x->getNextMessageInStream();
    }
    
    return NULL;
}

/**
 * findSlotsAllocatedForStream ()
 * The function locates all slots which are used to sent messages from
 * the same stream starting at the source link.
 */
void NoCScheduler::findSlotsAllocatedForStream(NoCSchedulingEntity *e,
    SlotReservations &slotsAllocated)
{
    CSize slotTableSize;
    CId streamId;
    Link *l;
    
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();
    
    // No slots allocated to stream
    slotsAllocated.resize(slotTableSize);
    for (uint i = 0; i < slotTableSize; i++)
        slotsAllocated[i] = false;
    
    // First link in route and the stream id
    l = *(e->getRoute()->linksBegin());
    streamId = e->getMessage()->getStreamId();
    
    // Iterate over the complete time period to find slots used by 
    // same stream
    for (SlotTablesIter iter = l->slotTableSeqBegin();
            iter != l->slotTableSeqEnd(); iter++)
    {
        SlotTable &s = *iter;
        
        for (uint i = 0; i < slotTableSize; i++)
            if (s[i] != NULL && s[i]->getMessage()->getStreamId() == streamId)
            {
                slotsAllocated[i] = true;
            }
    }
}

/**
 * findFreeSlotsForStream ()
 * The function locates all slots which are not used by any stream at
 * any moment in time along the complete route r.
 */
void NoCScheduler::findFreeSlotsForStream(Route *r,
    SlotReservations &slotsRoute)
{
    CSize slotTableSize;
    uint linkSeqNr;
    Link *l;
     
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();
    
    // All slots assumed available
    slotsRoute.resize(slotTableSize);
    for (uint i = 0; i < slotTableSize; i++)
        slotsRoute[i] = true;
    
    // Iterate over all links in the route
    linkSeqNr = 0;
    for (LinksIter iterL = r->linksBegin(); iterL != r->linksEnd(); iterL++)
    {
        l = *iterL;

        // Iterate over the complete time period to find slots not used
        // by any stream (i.e. free slots)
        for (SlotTablesIter iter = l->slotTableSeqBegin();
                iter != l->slotTableSeqEnd(); iter++)
        {
            SlotTable &s = *iter;

            for (uint i = 0; i < slotTableSize; i++)
            {
                if (!s.isSlotFree(i))
                {
                    slotsRoute[(slotTableSize+i-linkSeqNr)%slotTableSize]
                             = false;
                }
            }
        }
        
        // Next link
        linkSeqNr++;
    }
}

/**
 * raisePreferenceLevelSlotsOnRoute ()
 * The function increases the preference level of the slots used by the
 * scheduling entity on its route. This preference level indicates how
 * often a slot in a link is used in the scheduling problem. When its
 * preference level is equal to infinity (UINT_MAX), the slot is also
 * used in another scheduling problem that is scheduled along with this
 * problem.
 */
void NoCScheduler::raisePreferenceLevelSlotsOnRoute(NoCSchedulingEntity *e)
{
    SlotReservations slotReservations;
    CSize slotTableSize;
    Route *r;
    uint linkSeqNr;
    
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();
    
    // Route
    r = e->getRoute();
    
    // Slot reservations
    slotReservations = e->getSlotReservations();
    
    // Iterate over all links in the route
    linkSeqNr = 0;
    for (LinksIter iterL = r->linksBegin(); iterL != r->linksEnd(); iterL++)
    {
        Link *l = *iterL;
        
        // Raise preference level of the occupied slots
        for (uint i = 0; i < slotTableSize; i++)
        {
            // Compute slot j which corresponds to slot i considering the
            // position of the link l in the route.
            uint j = (i + linkSeqNr + slotTableSize) % slotTableSize;
        
            if (slotReservations[i] && l->preferredSlots[j] != UINT_MAX)
            {
                l->preferredSlots[j]++;
            }
        }
        
        // Next link in route
        linkSeqNr++;
    }
}

/**
 * lowerPreferenceLevelSlotsOnRoute ()
 * The function decreases the preference level of the slots used by the
 * scheduling entity on its route. This preference level indicates how
 * often a slot in a link is used in the scheduling problem. When its
 * preference level is equal to infinity (UINT_MAX), the slot is also
 * used in another scheduling problem that is scheduled along with this
 * problem.
 */
void NoCScheduler::lowerPreferenceLevelSlotsOnRoute(NoCSchedulingEntity *e)
{
    SlotReservations slotReservations;
    CSize slotTableSize;
    Route *r;
    uint linkSeqNr;
    
    // NoC properties
    slotTableSize = getInterconnectGraph()->getSlotTableSize();
    
    // Route
    r = e->getRoute();
    
    // Slot reservations
    slotReservations = e->getSlotReservations();
    
    // Iterate over all links in the route
    linkSeqNr = 0;
    for (LinksIter iterL = r->linksBegin(); iterL != r->linksEnd(); iterL++)
    {
        Link *l = *iterL;
        
        // Raise preference level of the occupied slots
        for (uint i = 0; i < slotTableSize; i++)
        {
            // Compute slot j which corresponds to slot i considering the
            // position of the link l in the route.
            uint j = (i + linkSeqNr + slotTableSize) % slotTableSize;
        
            if (slotReservations[i] && l->preferredSlots[j] != UINT_MAX)
            {
                if (l->preferredSlots[j] == 0)
                    throw CException("Preference level cannot become negative");
                
                l->preferredSlots[j]--;
            }
        }
        
        // Next link in route
        linkSeqNr++;
    }
}

/**
 * print ()
 * Output the status of the scheduler (overview of all link allocations) and the
 * number of messages scheduled to the supplied output stream.
 */
ostream &NoCScheduler::print(ostream &out) const
{
    uint nrMessagesScheduled = 0;
    
    // Output slot tables on all links
    for (LinksCIter iter = getInterconnectGraph()->linksBegin();
            iter != getInterconnectGraph()->linksEnd(); iter++)
    {
        const Link *l = *iter;

        l->print(out);
        out << endl;
    }

    // Count number of scheduled messages
    for (MessagesCIter iter = messagesBegin(); iter != messagesEnd(); iter++)
    {
        const Message *m = *iter;
        
        if (m->getSchedulingEntity() != NULL)
            nrMessagesScheduled++;
    }

    out << "Scheduled " << nrMessagesScheduled << " messages." << endl;

    return out;
}

