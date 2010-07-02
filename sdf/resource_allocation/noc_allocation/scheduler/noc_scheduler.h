/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   noc_scheduler.h
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
 * $Id: noc_scheduler.h,v 1.1 2008/03/20 16:16:21 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_NOC_SCHEDULER_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_NOC_SCHEDULER_H_INCLUDED

#include "../problem/problem.h"
#include "../problem/packet.h"

class NoCScheduler
{
public:
    // Constructor
    NoCScheduler() {};
    
    // Destructor
    virtual ~NoCScheduler() {};

    // Schedule function
    bool schedule(SetOfNoCScheduleProblems &problems);

    // Load scheduling entities for the scheduling problems from XML
    void assignSchedulingEntities(SetOfNoCScheduleProblems &problems,
            CNode *networkMappingNode);

protected:
    // Schedule the current scheduling problem
    virtual bool solve() = 0;

    // Load scheduling entities for the current scheduling problems from XML
    void assignSchedulingEntities(CNode *messagesNode);

    // Mark prefered slots in the interconnect graph
    void markPreferedSlotsOnLinks(SetOfNoCScheduleProblems &problems);

    // Scheduling problem that is being solved
    void setSchedulingProblem(NoCScheduleProblem *p) { curScheduleProblem = p; };
    NoCScheduleProblem *getSchedulingProblem() const {
        ASSERT(curScheduleProblem != NULL, "No scheduling problem set.");
        return curScheduleProblem; 
    };

    // Interconnect graph of the current scheduling problem
    InterconnectGraph *getInterconnectGraph() const {
        return getSchedulingProblem()->getInterconnectGraph(); 
    };
    
    // Messages of the current scheduling problem
    MessagesIter messagesBegin() { 
        return getSchedulingProblem()->messagesBegin(); 
    };
    MessagesIter messagesEnd() { 
        return getSchedulingProblem()->messagesEnd(); 
    };
    MessagesCIter messagesBegin() const { 
        return getSchedulingProblem()->messagesBegin(); 
    };
    MessagesCIter messagesEnd() const { 
        return getSchedulingProblem()->messagesEnd(); 
    };
            
    // Scheduling entities of the current scheduling problem
    NoCSchedulingEntitiesIter schedulingEntitiesBegin() { 
        return getSchedulingProblem()->schedulingEntitiesBegin();
    };
    NoCSchedulingEntitiesIter schedulingEntitiesEnd() {
        return getSchedulingProblem()->schedulingEntitiesEnd(); 
    };    
    uint nrSchedulingEntities() const { 
        return getSchedulingProblem()->nrSchedulingEntities(); 
    };
    
    // Routing
    void findAllRoutes(const Node *src, const Node *dst, 
            const CSize maxDetour, bool exact, Routes &routes);
    void findRoutes(const Node *src, const Node *dst, const CSize minLength,
            const CSize maxLength, Route &route, Routes &routes);
    CSize getLengthShortestPathBetweenNodes(const Node *src, const Node *dst);

    // Slots
    uint minFreeSlotsOnLink(Link *l, const TTime startTime,
            const TTime duration);
    bool findSlotsOnRoute(NoCSchedulingEntity *e, SlotReservations &s);
    uint nrSlotsRequired(const TTime time, const CSize size, 
            const uint nrPacketsPerSlotTable);
    void findFreePackets(NoCSchedulingEntity *e, SlotReservations &slotsRoute,
            Packets &packets);
    SlotReservations findFreeSlotsOnRoute(const Route *r, const TTime startTime,
            const TTime duation);
    bool findSlotsOnRoute(NoCSchedulingEntity *e,
            SlotReservations &slotsForPackets, SlotReservations &s);
    SlotReservations findFreeSlotsOnLink(const Link *l, const TTime startTime,
            const TTime duration);
    SlotReservations findFreeSlotsOnFirstLinkRoute(const Route *r, 
            const TTime startTime, const TTime duration);
    void raisePreferenceLevelSlotsOnRoute(NoCSchedulingEntity *e);
    void lowerPreferenceLevelSlotsOnRoute(NoCSchedulingEntity *e);

    // Stream
    void ripupStream(MessagesIter iterMsg);
    void findSlotsAllocatedForStream(NoCSchedulingEntity *e,
            SlotReservations &slotsAllocated);
    void findFreeSlotsForStream(Route *r,
            SlotReservations &slotsRoute);

    // Conflicts
    double severityConflict(NoCSchedulingEntity *e, Message *m, Route *r);
    void ripupScheduleEntity(MessagesIter iterMsg);

    // Cost
    void sortMessagesOnCost();
    void sortRoutesOnCost(Routes &routes, const TTime startTime,
            const TTime duration);
    
    // Scheduling entities
    bool findScheduleEntityForMessage(Message *m, const CSize maxDetour);
    TTime earliestStartTime(const Message *m) const;
    TTime maximalDuration(const Message *m, TTime startTime,
            CSize lengthRoute) const;
    TTime minimalDuration(const Message *m, TTime startTime,
            SlotReservations slotReservations) const;
    NoCSchedulingEntity *getFirstScheduledMessageInStream(Message *m); 

    // Resource management
    void reserveResources(NoCSchedulingEntity *e);
    void releaseResources(NoCSchedulingEntity *e);

    // Routes (hash)
    void createRoutesHash(const CSize maxDetour);
    void destroyRoutesHash();
    
    // Print
    ostream &print(ostream &out) const;

private:
    // Scheduling problem that is being solved
    NoCScheduleProblem *curScheduleProblem;
};

#endif
