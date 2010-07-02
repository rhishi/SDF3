/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   knowledge.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Global knowledge NoC scheduling algorithm
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: knowledge.cc,v 1.2 2008/11/01 16:05:49 sander Exp $
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

#include "knowledge.h"

/**
 * knowledge ()
 * Global knowledge NoC scheduling algorithm
 */
bool KnowledgeNoCScheduler::knowledge(const CSize maxDetour,
        const uint maxNrRipups)
{
    bool found;
    LinksReqs linksReqs;
    uint nrRipups = 0;

    // Compute requirements for all messages
    setRequirementsMessages(linksReqs);
    
    #if 0
    // Print requirements for all messages
    cerr << "# Link requirements" << endl;
    for (uint i = 0; i < getInterconnectGraph()->nrLinks(); i++)
    {
        LinkReqs &linkReqs = linksReqs[i];

        cerr << "Link " << i << ": " << endl;

        for (LinkReqsIter iter = linkReqs.begin(); 
                iter != linkReqs.end(); iter++)
        {
            LinkReq *lReq = *iter;


            cerr << "(" << lReq->startTime << ", " << lReq->endTime << "): ";
            cerr << lReq->nrSlotsReq << endl;

        }
        cerr << endl;
    }
    #endif
    
    // Sort all messages using cost function
    sortMessagesOnCost();
    
    // Iterate over sorted messages
    for (MessagesIter iter = messagesBegin(); iter != messagesEnd(); iter++)
    {
        Message *m = *iter;

        // Try to find a scheduling entity for the message
        found = findScheduleEntityForMessageUsingKnowledge(m, 
                    maxDetour, linksReqs);
        
        // Failed to find a scheduling entity?
        if (!found)
        {
            if (nrRipups < maxNrRipups)
            {
                // Ripup some already scheduled message
                ripupScheduleEntity(iter);
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
 * setRequirementsMessages ()
 * The function computes the requirements for all messages and store them in 
 * the array linksReqs which contains the requirements for all links in the
 * architecture graph.
 */
void KnowledgeNoCScheduler::setRequirementsMessages(LinksReqs &linksReqs)
{
    uint nrLinks = getInterconnectGraph()->nrLinks();
    TTime slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    
    // Construct array of link requirements
    linksReqs.resize(nrLinks);
    for (uint i = 0; i < nrLinks; i++)
    {
        LinkReq *lReq = new LinkReq;
        lReq->startTime = 0;
        lReq->endTime = slotTablePeriod-1;
        lReq->nrSlotsReq = 0;
        linksReqs[i].push_back(lReq);
    }
    
    // Iterate over all messages
    for (MessagesIter iter = messagesBegin(); iter != messagesEnd(); iter++)
    {
        Message *m = *iter;
        
        setRequirementsMessage(m, linksReqs);
    }
}

/**
 * updateRequirementsLink ()
 * Add the slot requirements (nrSlotsReq) to the correct time intervals
 * for the link requirements 'lReqs'.
 */
void KnowledgeNoCScheduler::updateRequirementsLink(const TTime startTime, 
    const TTime endTime, const bool loopPeriod, const uint nrSlotsReq, 
    LinkReqs &lReqs)
{
    for (LinkReqsIter iterR = lReqs.begin(); iterR != lReqs.end(); iterR++)
    {
        LinkReq *lReq = *iterR;

        // Link requirement starts after end message
        if (!loopPeriod && lReq->startTime > endTime)
            return;

        // Link requirement within time interval message (without loop)
        if (!loopPeriod && lReq->endTime >= startTime)
        {
            if (lReq->startTime < startTime)
            {
                // Link requirement starts before message
                LinkReq *lnReq = new LinkReq;
                lnReq->startTime = lReq->startTime;
                lnReq->endTime = startTime - 1;
                lnReq->nrSlotsReq = lReq->nrSlotsReq;
                lReq->startTime = startTime;
                lReqs.insert(iterR, lnReq);
                iterR--;
            }
            else if (lReq->endTime <= endTime)
            {
                // Link requirement ends before message ends
                lReq->nrSlotsReq += nrSlotsReq;
            }
            else if (endTime < lReq->endTime)
            {
                // Link requirement ends after message ends
                LinkReq *lnReq = new LinkReq;
                lnReq->startTime = lReq->startTime;
                lnReq->endTime = endTime;
                lnReq->nrSlotsReq = lReq->nrSlotsReq + nrSlotsReq;
                lReq->startTime = endTime+1;
                lReqs.insert(iterR, lnReq);
            }
        }
        
        // Link requirement within time interval message (with loop)
        if (loopPeriod && (lReq->endTime >= startTime 
                        || (loopPeriod && lReq->startTime <= endTime)))
        {
            if (lReq->endTime <= endTime)
            {
                // Link requirement ends before message
                lReq->nrSlotsReq += nrSlotsReq;
            }
            else if (endTime < lReq->endTime 
                            && endTime >= lReq->startTime)
            {
                // Link requirement ends after message ends
                LinkReq *lnReq = new LinkReq;
                lnReq->startTime = lReq->startTime;
                lnReq->endTime = endTime;
                lnReq->nrSlotsReq = lReq->nrSlotsReq + nrSlotsReq;
                lReq->startTime = endTime+1;
                lReqs.insert(iterR, lnReq);
            }

            if (lReq->startTime >= startTime)
            {
                // Link requirement starts after message starts
                lReq->nrSlotsReq += nrSlotsReq;
            }
            else if (lReq->endTime >= startTime)
            {
                // Link requirement starts before message starts
                LinkReq *lnReq = new LinkReq;
                lnReq->startTime = lReq->startTime;
                lnReq->endTime = startTime - 1;
                lReq->nrSlotsReq = lReq->nrSlotsReq + nrSlotsReq;
                lReq->startTime = startTime;
                lReqs.insert(iterR, lnReq);
            }
        }
    }
}

/**
 * setRequirementsMessage ()
 * The function computes the requirements for a message and updates the
 * information in the requirements on the links (i.e. nr of slots needed
 * on links)
 */
void KnowledgeNoCScheduler::setRequirementsMessage(Message *m,
        LinksReqs &linksReqs)
{
    TTime startTime, endTime, slotTablePeriod;
    Node *srcNode, *dstNode;
    bool loopPeriod;
    uint nrSlotsReq;
    Routes routes;

    // No requirements set for any link regarding this message
    vector<bool> setLinkReqs(getInterconnectGraph()->nrLinks(), false);

    // Start and end time of the message
    slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    startTime = m->getStartTime();
    endTime = (m->getStartTime() + m->getDuration()) % slotTablePeriod;
    loopPeriod = false;

    // The scheduling entity loops around the period when its start time
    // is equal or more then its end time. In the former case, the scheduling
    // entity takes a complete period, in the latter case it is less then a
    // period.
    if (startTime >= endTime && m->getDuration() != 1)
        loopPeriod = true;

    // Source and destination node in the architecture graph
    srcNode = getInterconnectGraph()->getNode(m->getSrcNodeId());    
    dstNode = getInterconnectGraph()->getNode(m->getDstNodeId());    
    
    // Number of slots required on shortest route (assuming single packet)
    nrSlotsReq = nrSlotsRequired(m->getDuration(), m->getSize(), 1);

    // Get a list of all shortest routes for the message
    findAllRoutes(srcNode, dstNode, 0, true, routes);

    // Iterate over all routes and set slot requirements on their links
    for (RoutesIter iter = routes.begin(); iter != routes.end(); iter++)
    {
        Route &r = *iter;
        
        for (LinksIter iterL = r.linksBegin(); iterL != r.linksEnd(); iterL++)
        {
            Link *l = *iterL;
            LinkReqs &lReqs = linksReqs[l->getId()];
            
            // Already set requirements for this link?
            if (!setLinkReqs[l->getId()])
            {
                setLinkReqs[l->getId()] = true;
                updateRequirementsLink(startTime, endTime, loopPeriod,
                                                            nrSlotsReq, lReqs);
            }
        }
    }
}

/**
 * costLinkForMessage ()
 * The cost for using a link by a message is the maximum number of slots
 * allocated in the time period within which the message can be sent.
 */
double KnowledgeNoCScheduler::costLinkForMessage(Message *m,
        LinkReqs &linkReqs)
{
    TTime startTime, endTime, slotTablePeriod;
    bool loopPeriod;
    double maxNrSlots = 0;
    
    // Start and end time of the message
    slotTablePeriod = getInterconnectGraph()->getSlotTablePeriod();
    startTime = m->getStartTime();
    endTime = (m->getStartTime() + m->getDuration()) % slotTablePeriod;
    loopPeriod = (startTime > endTime ? true : false);

    for (LinkReqsIter iter = linkReqs.begin(); iter != linkReqs.end(); iter++)
    {
        LinkReq *lReq = *iter;
        
        // Link requirements after end of message
        if (!loopPeriod && lReq->startTime > endTime)
            break;
        
        // Link requirement during time of message
        if (lReq->endTime >= startTime 
                || (loopPeriod && lReq->startTime <= endTime))
        {
            // Nr slots required in this time interval larger then know max?
            if (lReq->nrSlotsReq > maxNrSlots)
                maxNrSlots = lReq->nrSlotsReq;
        }
    }
    
    return maxNrSlots;
}

/**
 * sortRoutesOnMessageRequirements ()
 * Sort a set of routes based on the resource requirements a message
 * has on the links. Routes are sorted from least used route to
 * heaviest used route.
 */
void KnowledgeNoCScheduler::sortRoutesOnMessageRequirements(Message *m, 
    LinksReqs &linksReqs, Routes &routes)
{
    // Set the cost of each route
    for (RoutesIter iter = routes.begin(); iter != routes.end(); iter++)
    {
        Route &r = *iter;
        double cost = 0;
        
        // Cost of a route is the sum of the link requirements (max within time
        // period of message)
        for (LinksIter iterL = r.linksBegin(); iterL != r.linksEnd(); iterL++)
        {
            Link *l = *iterL;
            
            cost += costLinkForMessage(m, linksReqs[l->getId()]);
        }
        
        r.setCost(cost);
    }
    
    // Sort the routes
    routes.sort();
}

/**
 * findScheduleEntityForMessageUsingKnowledge ()
 * The function tries to find a schedule entity for the message. On success it
 * returns true and allocates the entity on the NoC. Else it returns false and
 * no resources are claimed.
 */
bool KnowledgeNoCScheduler:: findScheduleEntityForMessageUsingKnowledge(
    Message *m, const CSize maxDetour, LinksReqs &linksReqs)
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
        sortRoutesOnMessageRequirements(m, linksReqs, routes);
        
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
