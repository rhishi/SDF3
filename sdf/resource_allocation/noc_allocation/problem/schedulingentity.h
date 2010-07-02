/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   commevent.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Scheduling entity
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: schedulingentity.h,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_SCHEDULINGENTITY_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_SCHEDULINGENTITY_H_INCLUDED

#include "message.h"
#include "route.h"

/**
 * NoCSchedulingEntity
 * SchedulingEntity class used for NoC scheduling
 */
class NoCSchedulingEntity
{
public:
    // Constructor
    NoCSchedulingEntity(Message *m) : reserved(false), message(m) {route = NULL;};
    
    // Destrcutor
    ~NoCSchedulingEntity() { delete route; };

    // Message
    Message *getMessage() const { return message; };
    
    // Properties
    TTime getStartTime() const { return startTime; };
    void setStartTime(const TTime t) { startTime = t; };
    TTime getDuration() const { return duration; };
    void setDuration(const TTime d) { duration = d; };
    Route *getRoute() const { return route; };
    void setRoute(Route *r) { route = r; };
    SlotReservations getSlotReservations() const { return slotReservations; }; 
    void setSlotReservations(const SlotReservations &s)
            { slotReservations = s; };
    
    // Flag to signal resource reservations
    bool resourcesReserved() const { return reserved; };
    void resourcesReserved(const bool flag) { reserved = flag; };

    // Cost
    void setCost(double c) { cost = c; };
    double getCost() const { return cost; };
    bool operator<(const NoCSchedulingEntity &s)
            { return getCost() < s.getCost() ? true : false; };
    
    // Print
    ostream &print(ostream &out) const;
    
private:
    // Properties
    TTime startTime;
    TTime duration;
    Route *route;
    SlotReservations slotReservations;
    bool reserved;
    
    // Message
    Message *message;

    // Cost
    double cost;
};

#endif
