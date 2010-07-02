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
 * $Id: problem.h,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_PROBLEM_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_PROBLEM_H_INCLUDED

#include "schedulingentity.h"

// Forward class definition
class NoCScheduleProblem;

/**
 * ScheduleSwitchConstraint
 * This data structure describes the time-relation between two scheduling
 * problems when switching from one to another one. The overlap is defined
 * as the amount of time start of the second schedule (to) overlaps with the
 * first schedule (from). When the overlap is equal or larger then the schedule
 * period of the 'from' or 'to' schedule, the two schedules overlap completly.
 */
typedef struct _NoCScheduleSwitchConstraint
{
    NoCScheduleProblem *from;
    NoCScheduleProblem *to;
    TTime overlap;
} NoCScheduleSwitchConstraint;

typedef list<NoCScheduleSwitchConstraint>            NoCScheduleSwitchConstraints;
typedef NoCScheduleSwitchConstraints::iterator       NoCScheduleSwitchConstraintsIter;
typedef NoCScheduleSwitchConstraints::const_iterator NoCScheduleSwitchConstraintsCIter;

/**
 * NoCScheduleProblem
 * A NoC scheduling problem (single set of messages)
 */
class NoCScheduleProblem
{
public:
    // Constructor
    NoCScheduleProblem(CString name, CNode *messagesNode, CNode *archGraphNode,
        CNode *systemUsageNode);
    
    // Destructor
    ~NoCScheduleProblem();

    // Name
    CString getName() const { return scheduleName; };
    
    // Interconnect graph
    InterconnectGraph *getInterconnectGraph() const { 
        return interconnectGraph; 
    };

    // Schedule switche constraint
    void addScheduleSwitchConstraint(NoCScheduleSwitchConstraint &s) {
        scheduleSwitchConstraints.push_back(s);
    };

    // Messages
    MessagesIter messagesBegin() { return messages.begin(); };
    MessagesIter messagesEnd() { return messages.end(); };
    MessagesCIter messagesBegin() const { return messages.begin(); };
    MessagesCIter messagesEnd() const { return messages.end(); };
    Messages &getMessages() { return messages; };
    
    // Scheduling entities
    NoCSchedulingEntitiesIter schedulingEntitiesBegin()
            { return schedulingEntities.begin(); };
    NoCSchedulingEntitiesIter schedulingEntitiesEnd()
            { return schedulingEntities.end(); };    
    uint nrSchedulingEntities() const { return schedulingEntities.size(); };

    // Problem solved?
    bool isProblemSolved() const { return solvedFlag; };
    bool setSolvedFlag(bool flag) { solvedFlag = flag; return solvedFlag; };

    // Schedule constraints (this adds all constraints which are given by the
    // this schedule on schedule p to the schedule p).
    void constrainOtherSchedulingProblem(NoCScheduleProblem *p);

    // Scheduling constraints
    NoCScheduleSwitchConstraintsIter scheduleSwitchConstraintsBegin() {
        return scheduleSwitchConstraints.begin();
    };
    NoCScheduleSwitchConstraintsIter scheduleSwitchConstraintsEnd() {
        return scheduleSwitchConstraints.end();
    };

    // Slot preferences
    void markPreferedSlots(InterconnectGraph *g);
    
private:
    // Construct messages
    void constructMessages(CNode *messagesNode, InterconnectGraph *archGraph);
    
private:
    // Name of the schedule
    CString scheduleName;
    
    // Period after which schedule repeats
    TTime schedulePeriod;    
    
    // Interconnect graph
    InterconnectGraph *interconnectGraph;

    // Set of messages which must be scheduled
    Messages messages;

    // Schedule switches
    NoCScheduleSwitchConstraints scheduleSwitchConstraints;
    
    // Scheduling entities
    NoCSchedulingEntities schedulingEntities;
    
    // Solved flag
    bool solvedFlag;
};

typedef list<NoCScheduleProblem*>              NoCScheduleProblems;
typedef NoCScheduleProblems::iterator          NoCScheduleProblemsIter;
typedef NoCScheduleProblems::const_iterator    NoCScheduleProblemsCIter;

/**
 * SetOfNoCScheduleProblems
 * A set of NoC scheduling problems
 */
class SetOfNoCScheduleProblems
{
public:
    // Constructor
    SetOfNoCScheduleProblems(CNode *messagesSetNode, CNode *archGraphNode,
        CNode *systemUsageNode);
    
    // Destructor
    ~SetOfNoCScheduleProblems() {};

    // Convert to XML
    CNode *createNetworkMappingNode();
    CNode *createNetworkUsageNode();
    
    // Schedule problems
    NoCScheduleProblemsIter scheduleProblemsBegin() { 
        return scheduleProblems.begin(); 
    };
    NoCScheduleProblemsIter scheduleProblemsEnd() { 
        return scheduleProblems.end(); 
    };
    uint nrScheduleProblems() const { return scheduleProblems.size(); };

    // Clear the set of scheduling problems
    void clear() { scheduleProblems.clear(); };

private:
    // Schedule problems
    NoCScheduleProblems scheduleProblems;
};

#endif

