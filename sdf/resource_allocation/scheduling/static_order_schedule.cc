/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_order_schedule.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 3, 2008
 *
 *  Function        :   Static-order schedule for SDF graphs
 *
 *  History         :
 *      03-03-08    :   Initial version.
 *
 * $Id: static_order_schedule.cc,v 1.2 2008/03/06 13:59:06 sander Exp $
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

#include "static_order_schedule.h"

/**
 * minimize ()
 * The function returns a new static-order schedule in which the length of
 * the current schedule is reduced as much as possible. This new schedule
 * contains the smallest repeated part in the periodic phase of the schedule. 
 * From the transient phase, all complete repetitions of the periodic phase are
 * removed. The schedule entries in the minimized schedule are assigned new 
 * id's once the minimization is completed.
 */
void StaticOrderSchedule::minimize()
{
    uint periodicStart, periodicEnd, transientEnd, posPattern, posSchedule;
    StaticOrderSchedule newSchedule;
    bool startOfPattern = true;
    
    // Empty schedule?
    if (empty())
        return;

    // Start of the periodic schedule
    periodicStart = getStartPeriodicSchedule();
    ASSERT(periodicStart != UINT_MAX, "No periodic regime in the schedule");

    // Find smallest repeated part in the periodic schedule
    periodicEnd = periodicStart;
    posPattern = periodicStart;
    posSchedule = next(periodicEnd);

    while (posSchedule != periodicStart)
    {
        // Examining position in schedule (so no longer at start of pattern)
        startOfPattern = false;
        
        // Actor at posSchedule not equal to actor at posPattern?
        if (getScheduleEntry(posSchedule)->actor->getId() 
                != getScheduleEntry(posPattern)->actor->getId())
        {
            // Add schedule part up-to and including posSchedule to the
            // pattern and restart the search starting from the next
            // position in the schedule
            periodicEnd = posSchedule;
            posPattern = periodicStart;
        }
        else
        {
            // Actor in schedule matches with actor in pattern, move one
            // position forward in pattern and in schedule (loop on pattern
            // if needed).
            posPattern = next(posPattern);
            if (posPattern == next(periodicEnd))
            {
                posPattern = periodicStart;
                startOfPattern = true;
            }
        }
        
        // Last element reached?
        if (isLast(posSchedule))
            break;
        
        // Next
        posSchedule = next(posSchedule);
    }

    // Is the exact end of the repeated part of the schedule not reached?
    if (!startOfPattern)
    {
        // No exact repetition of pattern
        periodicEnd = posSchedule;
    }

    // Remove actor from non-repeated part of the schedule
    // An part can be removed if (starting from the back), the
    // complete repeated pattern is found in the transient part.
    // If not, no further reduction of the transient part is
    // possible.
    transientEnd = previous(periodicStart);
    posPattern = periodicEnd;
    posSchedule = transientEnd;
    while (getScheduleEntry(posSchedule) != end())
    {
        if (getScheduleEntry(posSchedule)->actor->getId() 
                != getScheduleEntry(posPattern)->actor->getId())
        {
            break;
        }
        else
        {
            posSchedule = previous(posSchedule);
            if (posPattern == periodicStart)
            {
                posPattern = periodicEnd;
                transientEnd = posSchedule;
            }
            else
            {
                posPattern = previous(posPattern);
            }
        }
    }

    // Create a new schedule
    // Step 1: Is there a transient part?
    if (getScheduleEntry(transientEnd) != end())
    {
        // Add transient to the schedule
        newSchedule.insert(newSchedule.end(), begin(), 
                                    getScheduleEntry(++transientEnd));

        // Set the start of the periodic phase
        newSchedule.setStartPeriodicSchedule(transientEnd);
    }
    else
    {
        // Set the start of the periodic phase
        newSchedule.setStartPeriodicSchedule(0);
    }
    
    // Step 2: Add periodic part to the schedule
    newSchedule.insert(newSchedule.end(), getScheduleEntry(periodicStart), 
                                    getScheduleEntry(++periodicEnd));

    // Put new schedule in place of the old schedule
    *this = newSchedule;
}

/**
 * changeActorAssociations ()
 * Change the associated actors in the schedule.
 */
void StaticOrderSchedule::changeActorAssociations(SDFgraph *newGraph)
{
    StaticOrderScheduleEntryIter pos = begin();
    
    while (pos != end())
    {
        if (newGraph->getActor(pos->actor->getName()) != NULL)
        {
            pos->actor = newGraph->getActor(pos->actor->getName());
            
            // Next
            pos++;
        }
        else
        {
            // Entry at pos inside the transient?
            if (pos < begin() + startPeriodicSchedule)
                startPeriodicSchedule--;
            
            pos = erase(pos);
        }
    }
}

/**
 * convertToXML ()
 * Convert the schedule to an XML representation.
 */
CNode *StaticOrderSchedule::convertToXML()
{
    CNode *scheduleNode, *stateNode;
    
    // Create a schedule node
    scheduleNode = CNewNode("schedule");
    
    // Create a state node for each schedule entry
    for (uint i = 0 ; i < size(); i++)
    {
        StaticOrderScheduleEntryIter pos = getScheduleEntry(i);
        stateNode = CAddNode(scheduleNode, "state");
        
        // Check that the state is associated with an actor
        ASSERT(pos->actor != NULL, "State not associated with an actor.");

        // Set attributes on the node        
        CAddAttribute(stateNode, "actor", pos->actor->getName());

        if (i == startPeriodicSchedule)
        {
            CAddAttribute(stateNode, "startOfPeriodicRegime","true");
        }
    }
    
    return scheduleNode;
}

