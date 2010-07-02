/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   random.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Random NoC scheduling algorithm
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: random.h,v 1.1 2008/03/20 16:16:21 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_RANDOM_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_SCHEDULER_RANDOM_H_INCLUDED

#include "noc_scheduler.h"

/**
 * RandomNoCScheduler ()
 * Random NoC scheduling algorithm
 */
class RandomNoCScheduler : public NoCScheduler
{
public:
    // Constructor
    RandomNoCScheduler(const CSize maxDetour, const uint maxNrRipups,
        const uint maxNrTries) : maxDetour(maxDetour), maxNrRipups(maxNrRipups),
        maxNrTries(maxNrTries) {};
    
    // Destructor
    ~RandomNoCScheduler() {};
    
    // Schedule function
    bool solve() { 
        bool success = random(maxDetour, maxNrRipups, maxNrTries);  
        return getSchedulingProblem()->setSolvedFlag(success);
    };

private:
    // Random schedule function
    bool random(const CSize maxDetour, const uint maxNrRipups, 
            const uint maxNrTries);

    // Random ordering
    void putMessagesInRandomOrder();
    void putRoutesInRandomOrder(Routes &routes);
    
    // Schedule single message
    bool findScheduleEntityForMessageUsingRandom(Message *m, 
            const CSize maxDetour);

private:
    CSize maxDetour;
    uint maxNrRipups;
    uint maxNrTries;
}; 
 
#endif
