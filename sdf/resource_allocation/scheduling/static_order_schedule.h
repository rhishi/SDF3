/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_order_schedule.h
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
 * $Id: static_order_schedule.h,v 1.2 2008/03/06 13:59:06 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_SCHEDULING_STATIC_ORDER_SCHEDULE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_SCHEDULING_STATIC_ORDER_SCHEDULE_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * Static-order schedule entry
 * One position in a static-order schedule.
 **/
class StaticOrderScheduleEntry
{
public:
    // Constructor
    StaticOrderScheduleEntry(SDFactor *actor = NULL) 
        : actor(actor) {};
    
    // Destructor
    ~StaticOrderScheduleEntry() {};
    
    // Actor scheduled with this entry
    SDFactor *actor;
};

/**
 * Static-order schedule entry iterator
 * Iterator that provides a reference to an entry in a static-order schedule.
 **/
typedef vector<StaticOrderScheduleEntry>::iterator StaticOrderScheduleEntryIter;

/**
 * Static-order schedule
 * A static-order schedule provides a sequence of actor firings. The schedule
 * may contain a cycle which makes it effectively an infinite schedule.
 **/
class StaticOrderSchedule : public vector<StaticOrderScheduleEntry>
{
public:
    // Constructor
    StaticOrderSchedule() { startPeriodicSchedule = UINT_MAX; };
    
    // Destructor
    ~StaticOrderSchedule() {};
    
    // Append actor to the schedule
    void appendActor(SDFactor *a) {
        push_back(StaticOrderScheduleEntry(a));
    };
    
    // Append schedule entry to the schedule
    void appendScheduleEntry(StaticOrderScheduleEntry &s) {
        push_back(s);
    };

    // Insert actor befere the element at position into the schedule and return
    // iterator to newly inserted element
    StaticOrderScheduleEntryIter insertActor(
            StaticOrderScheduleEntryIter position, SDFactor *a)
    {
        // Update start of periodic schedule when new element is in transient
        if (begin() + startPeriodicSchedule > position)
            startPeriodicSchedule++;
        
        // Insert actor
        return insert(position, StaticOrderScheduleEntry(a));
    };

    // Access schedule entry
    StaticOrderScheduleEntryIter getScheduleEntry(const uint i) {
        if (i >= size())
            return end();
        return (begin() + i);
    };
    
    // Loop-back in schedule
    uint getStartPeriodicSchedule() {
        return startPeriodicSchedule;
    };
    void setStartPeriodicSchedule(const uint i) {
        startPeriodicSchedule = i;
    };

    // Next entry in the schedule
    uint next(uint current) {
        if (isLast(current))
            return getStartPeriodicSchedule();
        
        return ++current;
    };

    // Previous entry in the schedule
    uint previous(uint current)
    {
        if (current == 0)
            return UINT_MAX;
        
        return --current;
    };
    
    // Last entry before the loop-back
    bool isLast(uint current) const
    {
        if (current + 1 == size())
            return true;
        
        return false;
    }
    
    // Compute a minimized version of the schedule
    void minimize();
    
    // Change the associated actors in the schedule
    void changeActorAssociations(SDFgraph *newGraph);
    
    // Convert the schedule to an XML representation
    CNode *convertToXML();
    
private:
    uint startPeriodicSchedule;
};

#endif

