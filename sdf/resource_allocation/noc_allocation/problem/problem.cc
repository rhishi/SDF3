/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   problem.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   October 4, 2006
 *
 *  Function        :   Schedule problem
 *
 *  History         :
 *      04-10-06    :   Initial version.
 *
 * $Id: problem.cc,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#include "problem.h"

/**
 * NoCScheduleProblem ()
 * Constructor.
 */
NoCScheduleProblem::NoCScheduleProblem(CString name, CNode *messagesNode,
        CNode *archGraphNode, CNode *systemUsageNode)
{
    // Name of schedule problem
    scheduleName = name;
    
    // Schedule period
    schedulePeriod = CGetAttribute(messagesNode, "period");

    // Construct interconnect graph
    interconnectGraph = new InterconnectGraph(archGraphNode, schedulePeriod);
    
    // Construct set of messages which must be scheduled
    constructMessages(messagesNode, interconnectGraph);
    
    // Set occupation of resource in the architecture
    interconnectGraph->setUsage(systemUsageNode);
    
    // The problem is not solved yet
    solvedFlag = false;
}

/**
 * ~ScheduleProblem ()
 * Destructor.
 */
NoCScheduleProblem::~NoCScheduleProblem()
{
    // Cleanup interconnect graph
    delete interconnectGraph;
    
    // Cleanup messages
    for (MessagesIter iter = messages.begin(); iter != messages.end(); iter++)
        delete (*iter);
}

/**
 * constrainOtherSchedulingProblem ()
 * The function marks slots on links of the interconnect graph in the
 * schedule problem p which it uses in its own schedule.
 *
 * It is possible that the overlap between the schedules contains multiple
 * periods of the source and/or destination schedule.
 *
 */
void NoCScheduleProblem::constrainOtherSchedulingProblem(NoCScheduleProblem *p)
{
    bool isSrcSchedule = false, isDstSchedule = false;
    TTime timeOverlap, timeInSrc, timeInDst, periodSrc, periodDst, duration;    
    SlotTable slotsSrc = SlotTable(0), slotsDst = SlotTable(0);
    TTime overlap = 0;
    SlotReservations slots;
    Link *lSrc, *lDst;
    
    // Is this schedule problem not solved yet?
    if (!isProblemSolved())
        return;

    // Is this problem an empty problem (i.e. it takes no time)?
    if (schedulePeriod == 0)
        return;

    // Check wether this schedule is the source or destination in a switch with
    // the schedule problem p. Find also the maximal overlap which needs to be
    // considered between the two schedules.
    for (NoCScheduleSwitchConstraintsIter iter = scheduleSwitchConstraints.begin();
            iter != scheduleSwitchConstraints.end(); iter++)
    {
        NoCScheduleSwitchConstraint sw = *iter;
        
        // Is this schedule problem the source schedule in the switch?
        if (sw.to->getName() == p->getName() && sw.from->getName() == getName())
        {
            isSrcSchedule = true;
            overlap = overlap > sw.overlap ? overlap : sw.overlap;
        }
        
        // Is this schedule problem the destination schedule in the switch?
        if (sw.from->getName() == p->getName() && sw.to->getName() == getName())
        {
            isDstSchedule = true;
            overlap = overlap > sw.overlap ? overlap : sw.overlap;
        }
    }

    // Schedule period of src and dst schedule
    if (isSrcSchedule)
    {
        periodSrc = schedulePeriod;
        periodDst = p->getInterconnectGraph()->getSlotTablePeriod();
    }
    else
    {
        periodSrc = p->getInterconnectGraph()->getSlotTablePeriod();
        periodDst = schedulePeriod;
    }

    // Iterate over all links in the interconnect
    for (LinksIter iter = interconnectGraph->linksBegin();
            iter != interconnectGraph->linksEnd(); iter++)
    {
        // Link l in src and dst scheduling problem
        if (isSrcSchedule)
        {
            lSrc = *iter;
            lDst = p->getInterconnectGraph()->getLink(lSrc->getName());
        }
        else
        {
            lDst = *iter;
            lSrc = p->getInterconnectGraph()->getLink(lDst->getName());
        }

        timeOverlap = 0;
        timeInSrc = periodSrc - (overlap % periodSrc);
        timeInDst = 0;
        while (timeOverlap < overlap)
        {
            // Find slot table in dst schedule which includes
            // timeInDst
            for (SlotTablesIter sIter = lDst->slotTableSeqBegin();
                sIter != lDst->slotTableSeqEnd(); sIter++)
            {
                slotsDst = *sIter;

                // timeInDst within this slot table?
                if (slotsDst.getStartTime() <= timeInDst 
                        && timeInDst <= slotsDst.getEndTime())
                {
                    break;
                }
            }

            // Find slot table in src schedule which includes
            // timeInSrc
            for (SlotTablesIter sIter = lSrc->slotTableSeqBegin();
                sIter != lSrc->slotTableSeqEnd(); sIter++)
            {
                slotsSrc = *sIter;

                // timeInSrc within this slot table?
                if (slotsSrc.getStartTime() <= timeInSrc 
                        && timeInSrc <= slotsSrc.getEndTime())
                {
                    break;
                }
            }

            // For how long is the table valid? Answer: Minimum of the
            // remaining length slot table of src and dst schedule
            if (slotsDst.getEndTime() - timeInDst 
                    < slotsSrc.getEndTime() - timeInSrc)
            {
                duration = slotsDst.getEndTime() - timeInDst;
            }
            else
            {
                duration = slotsSrc.getEndTime() - timeInSrc;
            }

            // Set slot occupation in src or dst
            if (isSrcSchedule)
            {
                slots = slotsSrc.getUsedSlotsOfSchedule();
                lDst->setUsedSlots(slots, timeInDst, timeInDst + duration);
            }
            else
            {
                slots = slotsDst.getUsedSlotsOfSchedule();
                lSrc->setUsedSlots(slots, timeInSrc, timeInSrc + duration);
            }

            // Next moment in time
            timeOverlap = timeOverlap + duration + 1;
            timeInSrc = (timeInSrc + duration + 1) % periodSrc;
            timeInDst = (timeInDst + duration + 1) % periodDst;
        }
    }
}

/**
 * markPreferedSlots ()
 * The function marks all slots in all links of the interconnect graph
 * as prefered slots (with the maximal preference level - UINT_MAX) when
 * this slot is used in the corresponding link of the interconnect graph g.
 */
void NoCScheduleProblem::markPreferedSlots(InterconnectGraph *g)
{
    Link *l;
    CSize slotTableSize;
    
    // NoC properties
    slotTableSize = interconnectGraph->getSlotTableSize();
    
    // Iterate over all links in the interconnect graph
    for (LinksIter iter = g->linksBegin(); iter != g->linksEnd(); iter++)
    {
        Link *lg = *iter;
        
        // Find matching link in interconnect graph of this problem
        l = interconnectGraph->getLink(lg->getId());
        
        // Mark any slot used in lg as a prefered slot in l
        for (uint i = 0; i < slotTableSize; i++)
        {
            if (lg->preferredSlots[i] > 0)
                l->preferredSlots[i] = UINT_MAX;
        }
    }
}

/**
 * constructMessages ()
 * The function constructs a list of all messages from an XML document.
 */
void NoCScheduleProblem::constructMessages(CNode *messagesNode,
        InterconnectGraph *archGraph)
{
    Message *m, *prevMsg = NULL;
    Node *srcNode, *dstNode;
    CStrings channels;
    CString channel;
    CId streamId;
    CId nr;
    
    // Iterate over the list of messages
    for (CNode *messageNode = CGetChildNode(messagesNode, "message");
            messageNode != NULL; 
            messageNode = CNextNode(messageNode, "message"))
    {
        // Get id of message
        nr = CGetAttribute(messageNode, "nr");

        // Allocate memory for message
        m = new Message(nr);

        // Find source node in graph
        srcNode = archGraph->getNode(CGetAttribute(messageNode, "src"));
        if (srcNode == NULL)
            throw CException("[ERROR] Source node not found in architecture " \
                             "graph.");
        m->setSrcNodeId(srcNode->getId());

        // Find destination node in graph
        dstNode = archGraph->getNode(CGetAttribute(messageNode, "dst"));
        if (dstNode == NULL)
            throw CException("[ERROR] Destination node not found in " \
                             "architecture graph.");
        m->setDstNodeId(dstNode->getId());
        
        // Set stream id based on channel name
        channel = CGetAttribute(messageNode, "channel");
        streamId = 0;
        for (CStringsIter iter = channels.begin();
                iter != channels.end(); iter++)
        {
            if ((*iter) == channel)
                break;
            streamId++;
        }
        // No channel found with this name
        if (streamId == channels.size())
            channels.push_back(channel);
        m->setStreamId(streamId);
                
        // Set properties of message
        m->setSeqNr(CGetAttribute(messageNode, "seqNr"));
        m->setStartTime(CGetAttribute(messageNode, "startTime"));
        m->setDuration(CGetAttribute(messageNode, "duration"));
        m->setSize(CGetAttribute(messageNode, "size"));
        
        // Add message to the set of messages
        messages.push_back(m);
        
        // Create link between messages in stream
        if (prevMsg != NULL && prevMsg->getStreamId() == m->getStreamId())
        {
            // Check sequence order
            if (prevMsg->getSeqNr() >= m->getSeqNr())
                throw CException("Sequence order incorrect.");

            prevMsg->setNextMessageInStream(m);
            m->setPreviousMessageInStream(prevMsg);
        }
        prevMsg = m;
    }
}

/**
 * SetOfNoCScheduleProblems ()
 * Constructor.
 */
SetOfNoCScheduleProblems::SetOfNoCScheduleProblems(CNode *messagesSetNode, 
        CNode *archGraphNode, CNode *systemUsageNode)
{
    NoCScheduleSwitchConstraint scheduleSwitchConstraint;
    NoCScheduleProblem *problem;
    CString name, from, to;
    uint overlap;

    // Create a schedule problem for each 'messages' node
    for (CNode *messagesNode = CGetChildNode(messagesSetNode, "messages");
            messagesNode != NULL; 
                messagesNode = CNextNode(messagesNode, "messages"))
    {
        name = CGetAttribute(messagesNode, "name");
            
        // Create new scheduling problem
        problem = new NoCScheduleProblem(name, messagesNode, archGraphNode,
                                        systemUsageNode);
        
        // Add scheduling problem to the set of problems
        scheduleProblems.push_back(problem);
    }

    // Set all switching options between the scheduling problems
    for (CNode *switchNode = CGetChildNode(messagesSetNode, "switch");
            switchNode != NULL; switchNode = CNextNode(switchNode, "switch"))
    {
        from = CGetAttribute(switchNode, "from");
        to = CGetAttribute(switchNode, "to");
        overlap = CGetAttribute(switchNode, "overlap");

        // Initialize schedule switch
        scheduleSwitchConstraint.from = NULL; 
        scheduleSwitchConstraint.to = NULL;
        scheduleSwitchConstraint.overlap = overlap;
        
        // Find problems mentioned in switch
        for (NoCScheduleProblemsIter iter = scheduleProblemsBegin();
                iter != scheduleProblemsEnd(); iter++)
        {
            problem = *iter;
            
            if (problem->getName() == from)
                scheduleSwitchConstraint.from = problem;
            if (problem->getName() == to)
                scheduleSwitchConstraint.to = problem;
        }
        
        if (scheduleSwitchConstraint.from == NULL 
                || scheduleSwitchConstraint.to == NULL)
        {
            throw CException("[ERROR] to/from messages not found.");
        }
        
        // Link switch to scheduling problems
        scheduleSwitchConstraint.from->addScheduleSwitchConstraint(
                                                    scheduleSwitchConstraint);
        scheduleSwitchConstraint.to->addScheduleSwitchConstraint(
                                                    scheduleSwitchConstraint);
    }
}

/**
 * createNetworkMappingNode ()
 * Create an XML node which describes the mapping of messages send over
 * the interconnect to the resources in the interconnect.
 */
CNode *SetOfNoCScheduleProblems::createNetworkMappingNode()
{
    CNode *networkNode, *messagesNode, *entityNode;
    NoCScheduleProblem *p;
    
    // Network node
    networkNode = CNewNode("network");
    
    // Create node for each set of messages (i.e. each schedule problem)
    for (NoCScheduleProblemsIter iter = scheduleProblemsBegin();
            iter != scheduleProblemsEnd(); iter++)
    {
        p = *iter;
        
        // Create messages node for a schedule
        messagesNode = CAddNode(networkNode, "messages");
        CAddAttribute(messagesNode, "name", p->getName());
        
        // Add schedule to messages node
        for (MessagesIter iterMsg = p->messagesBegin();
                iterMsg != p->messagesEnd(); iterMsg++)
        {
            Message *m = *iterMsg;
            NoCSchedulingEntity *e = m->getSchedulingEntity();
            Route *r = e->getRoute();
            SlotReservations sr = e->getSlotReservations();
            CString route, slots;
            bool first;
            
            // Scheduling entity
            entityNode = CAddNode(messagesNode, "schedulingEntity");
            CAddAttribute(entityNode, "msg", m->getId());
            CAddAttribute(entityNode, "startTime", e->getStartTime());
            CAddAttribute(entityNode, "duration", e->getDuration());
            
            // Route of entity
            first = true;
            for (LinksIter iterL = r->linksBegin();
                    iterL != r->linksEnd(); iterL++)
            {
                Link *l = *iterL;
                
                if (!first)
                    route += ",";
                route += l->getName();
                first = false;
            }
            CAddAttribute(entityNode, "route", route);
            
            // Slot reservations of entity
            first = true;
            for (uint i = 0; i < sr.size(); i++)
            {
                if (sr[i])
                {
                    if (!first)
                        slots += ",";
                    slots += i;
                    first = false;
                }
            }
            CAddAttribute(entityNode, "slots", slots);
        }
    }
    
    return networkNode;
}

/**
 * createNetworkUsageNode ()
 * Create an XML node which describes the usage of the links in the
 * interconnect.
 */
CNode *SetOfNoCScheduleProblems::createNetworkUsageNode()
{
    SlotReservations *slotReservations;
    CNode *networkNode, *linkNode;
    InterconnectGraph *interconnectGraph;
    NoCScheduleProblem *p;
    uint slotTableSize;
    
    // Network node
    networkNode = CNewNode("network");

    // No problems available?
    if (scheduleProblems.size() == 0)
        return networkNode;

    // Initially all slots in all links assumed free
    interconnectGraph = scheduleProblems.front()->getInterconnectGraph();    
    slotTableSize = interconnectGraph->getSlotTableSize();
    slotReservations = new SlotReservations [interconnectGraph->nrLinks()];
    for (uint i = 0; i < interconnectGraph->nrLinks(); i++)
    {
        slotReservations[i].resize(slotTableSize);
        for (uint j = 0; j < slotTableSize; j++)
            slotReservations[i][j] = false;
    }
    
    // Find for each link which slots are occupied at least one moment in time
    // in at least one architecture graph.
    for (NoCScheduleProblemsIter iter = scheduleProblemsBegin();
            iter != scheduleProblemsEnd(); iter++)
    {
        p = *iter;
        interconnectGraph = p->getInterconnectGraph();
    
        for (LinksIter iterL = interconnectGraph->linksBegin();
                iterL != interconnectGraph->linksEnd(); iterL++)
        {
            Link *l = *iterL;
            
            for (SlotTablesIter iterS = l->slotTableSeqBegin();
                    iterS != l->slotTableSeqEnd(); iterS++)
            {
                SlotTable &s = *iterS;
                
                for (uint i = 0; i < slotTableSize; i++)
                {
                    if (!s.isSlotFree(i))
                        slotReservations[l->getId()][i] = true;
                }
            }
        }
    }    
    
    // Create a node for each link which indicates the occupied slots
    for (LinksIter iterL = interconnectGraph->linksBegin();
            iterL != interconnectGraph->linksEnd(); iterL++)
    {
        Link *l = *iterL;
        bool first = true;
        CString slots;

        // Slots used from the link
        for (uint i = 0; i < slotTableSize; i++)
        {
            if (slotReservations[l->getId()][i])
            {
                if (!first)
                    slots += ",";
                slots += i;
                first = false;
            }
        }

        // Using a slot from the link?
        if (!slots.empty())
        {
            // Add a link node to the network
            linkNode = CAddNode(networkNode, "link");
            CAddAttribute(linkNode, "name", l->getName());
            CAddAttribute(linkNode, "occupiedSlots", slots);
        }
    }
    
    return networkNode;
}

