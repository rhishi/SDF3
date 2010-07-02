/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   slot.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Slot tables
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: slot.cc,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#include "slot.h"
#include "schedulingentity.h"

/**
 * SlotTable ()
 * Constructor.
 */
SlotTable::SlotTable(uint sz)
{ 
    nrSlots = sz; 
    nrFreeSlots = sz;
    tableReservations.resize(sz);
    tableEntities.resize(sz);
    for (uint i = 0; i < sz; i++)
    {
        tableReservations[i] = false;
        tableEntities[i] = NULL;
    }
}

/**
 * ~SlotTable ()
 * Destructor.
 */
SlotTable::~SlotTable()
{ 
}

/**
 * setUsedSlots ()
 * The function marks the slots specified in s as occupied between the given
 * start and end time by another schedule.
 */
void SlotTable::setUsedSlots(SlotReservations &s)
{
    for (uint i = 0; i < getNrSlots(); i++)
    {
        // Slot i marked as used?
        if (s[i] == true)
        {
            // Slot still free?
            if (tableReservations[i] == false)
            {
                tableReservations[i] = true;
                nrFreeSlots--;
            }
        }
    }
}

/**
 * reserveSlots ()
 * Reserve a slot in the slot table for a given scheduling entity.
 */
void SlotTable::reserveSlots(NoCSchedulingEntity *e, SlotReservations &s)
{
    for (uint i = 0; i < getNrSlots(); i++)
    {
        // Should slot i be reserved for e?
        if (s[i] == true)
        {
            // Slot already reserved?
            if (tableReservations[i])
            {
                cerr << "Conflict on slot between (";
                cerr << getStartTime() << "," << getEndTime() << ")" << endl;
                cerr << "Trying to reserve slot for message: ";
                e->getMessage()->print(cerr);
                cerr << endl;
                cerr << "    Scheduling entity: ";
                e->print(cerr);
                cerr << endl;
                if (tableEntities[i] != NULL)
                {
                    cerr << "Slot used for message: ";
                    tableEntities[i]->getMessage()->print(cerr);
                    cerr << endl;
                    cerr << "    Scheduling entity: ";
                    tableEntities[i]->print(cerr);
                    cerr << endl;
                }
                else
                {
                    cerr << "Slot used in another schedule." << endl;
                }
                
                throw CException("[ERROR] Slot already reserved.");
            }

            tableReservations[i] = true;
            tableEntities[i] = e;
            nrFreeSlots--;
         }
    }
}

/**
 * releaseSlots ()
 * Release the claim on a slot in the slot table.
 */
void SlotTable::releaseSlots(NoCSchedulingEntity *e)
{
    uint i = 0;
    
    for (NoCSchedulingEntitiesIter iter = begin(); iter != end(); iter++)
    {
        NoCSchedulingEntity *s = *iter;
        
        if (s == e)
        {
            tableReservations[i] = false;
            tableEntities[i] = NULL;
            nrFreeSlots++;
        }
        
        // Next
        i++;
    }
}

/**
 * getSlotReservations ()
 * The function returns a vector containing booleans which mark wether a slot
 * is used (true) or not-used (false) by the given scheduling entity. The empty
 * (non-allocated slots) are found by looking at the slot reservations of the
 * NULL element.
 */
SlotReservations SlotTable::getSlotReservations(NoCSchedulingEntity *e) const
{
    SlotReservations s(getNrSlots(), false);

    for (uint i = 0; i < getNrSlots(); i++)
    {
        if (tableEntities[i] == e) 
        {
            if (e == NULL)
            {
                if (tableReservations[i] == false)
                    s[i] = true;
            }
            else
            {
                s[i] = true;
            }
        }
    }

    return s;
}

bool SlotTable::isSlotFree(const uint i) const
{
    if (i >= tableReservations.size())
        return false;
        
    return !tableReservations[i];
}

/**
 * getUsedSlotsOfSchedule ()
 * The function returns a vector which indicates which slots are used for
 * the current scheduling problem. This excludes reservations of slots which
 * are used for other schedules.
 */
SlotReservations SlotTable::getUsedSlotsOfSchedule() const
{
    SlotReservations s(getNrSlots(), false);

    for (uint i = 0; i < getNrSlots(); i++)
    {
        if (tableEntities[i] != NULL)
        {
            s[i] = true;
        }
    }

    return s;
}

/**
 * SlotTableSeq
 * Constructor
 */
SlotTableSeq::SlotTableSeq(uint slotTableSize, TTime slotTablePeriod)
        :
    slotTableSize(slotTableSize),
    slotTablePeriod(slotTablePeriod)
{
    // Create a new slot table
    SlotTable s = SlotTable(slotTableSize);
    s.setStartTime(0);
    s.setEndTime(slotTablePeriod-1);
    
    // Add slot table to slot table sequence
    seq.push_back(s);
}

/**
 * SlotTableSeq
 * Constructor
 */
SlotTableSeq::~SlotTableSeq()
{
}

/**
 * nrFreeSlots ()
 * The function returns the number of free slots within the time interval given
 * by the start time and duration.
 */
uint SlotTableSeq::nrFreeSlots(const TTime startTime, const TTime duration)
{
    SlotTablesIter slotTableIter = seqBegin();
    SlotTable s = *slotTableIter;
    TTime period = getSlotTablePeriod();
    bool loopPeriod = false;
    uint freeSlots = 0;

    // End of message after period bound
    if (startTime + duration - 1 >= period)
        loopPeriod = true;
    
    // Check for each time-instance within the period wether the
    // associated slot is free
    for (TTime t = 0; t < getSlotTablePeriod(); t++)
    {
        // Move to next slot table?
        if (s.getEndTime() < t)
        {
            slotTableIter++;
            if (slotTableIter == seqEnd())
                break;
            s = *slotTableIter;
        }
        
        // Time t within interval [startTime, startTime+duration)?
        if ((t >= startTime && t <= startTime + duration - 1)
                || (loopPeriod && t <= (startTime + duration - 1) % period))
        {
            // Slot at time t available?
            if (s.isSlotFree(t % getSlotTableSize()))
                freeSlots++;
        }
        
        // Slot table valid till end of communication
        if (s.getEndTime() >= startTime + duration)
            break;
    }

    return freeSlots; 
}

/**
 * setUsedSlots ()
 * The function marks the slots specified in s as occupied between the given
 * start and end time by another schedule.
 */
void SlotTableSeq::setUsedSlots(SlotReservations &s, TTime startTime, 
        TTime endTime)
{
    bool slotUsed = false;

    // Check that at least one slot is used
    for (uint i = 0; i < s.size() && !slotUsed; i++)
    {
        if (s[i])
            slotUsed = true;
    }
    if (!slotUsed)
        return;
    
    // Iterate over the whole sequence
    for (SlotTablesIter iter = seqBegin(); iter != seqEnd(); iter++)
    {
        SlotTable &t = *iter;
        
        // Slot table valid after end of reservations
        if (t.getStartTime() > endTime)
            return;
        
        // Current slot table falls within time frame of reservation
        if (t.getEndTime() >= startTime)
        {
            // Does slot table start before reservations start?
            if (t.getStartTime() < startTime)
            {
                // Situation: t starts before startTime
                // Action: split t into two parts (before and from startTime)
                //         no slot reservation required

                // Create a new slot table sn which is a copy of t
                SlotTable sn = t;

                // Update start and end time of t and sn
                sn.setEndTime(startTime-1);
                t.setStartTime(startTime);

                // Add new slot table sn to sequence (before t)
                insertSlotTable(iter, sn);
                iter--;
            }
            else
            {
                // Slot table starts at startTime or later; check end 
                // of table first
                if (t.getEndTime() <= endTime)
                {
                    // Slot table ends before or exactly at end of reservation
                    // Reserve slots in this table
                    t.setUsedSlots(s);
                }
                else
                {
                    // Situation: endTime before table t ends
                    // Action: split t into two parts (till and after
                    //         endTime). Reserve slots in first part.

                    // Create a new slot table sn which is a copy of t
                    SlotTable sn = t;

                    // Update start and end time of t and sn
                    sn.setEndTime(endTime);
                    t.setStartTime(endTime+1);

                    // Reserve slots in sn
                    sn.setUsedSlots(s);

                    // Add new slot table sn to sequence (before t)
                    insertSlotTable(iter, sn);
                }
            }
        }
    }
}

/**
 * reserveSlots ()
 * Reserve all slots in the slot tables for scheduling entity e. The 'linkSeqNr'  
 * indicates the position of the link this slot table sequence belongs to. The
 * linkSeqNr starts at zero (=first link in route of e).
 */
void SlotTableSeq::reserveSlots(NoCSchedulingEntity *e, SlotReservations &slots,
    const ulong linkSeqNr)
{
    TTime period, entStartTime, entEndTime, startTime, endTime;
    bool loopPeriod;

    // Time frame of scheduling entity
    startTime = e->getStartTime() + linkSeqNr;
    endTime = e->getStartTime() + linkSeqNr + e->getDuration() - 1;
    period = getSlotTablePeriod();
    entStartTime = startTime % period;
    entEndTime = endTime % period;
    loopPeriod = false;

    // The scheduling entity loops around the period when its start time
    // is equal or more then its end time. In the former case, the scheduling
    // entity takes a complete period, in the latter case it is less then a
    // period.
    if (entStartTime >= entEndTime && e->getDuration() != 1)
        loopPeriod = true;

    if (!loopPeriod)
    {
        // Iterate over the current slot table sequence to find slot table
        // with larger end time than the scheduling entity
        for (SlotTablesIter iter = seqBegin(); iter != seqEnd(); iter++)
        {
            SlotTable &s = *iter;

            // Slot table valid after end of scheduling entity
            if (s.getStartTime() > entEndTime)
                return;

            // Current slot table falls within time frame scheduling entity?
            if (s.getEndTime() >= entStartTime)
            {
                // Does s start before e starts?
                if (s.getStartTime() < entStartTime)
                {
                    // Situation: s starts before e starts
                    // Action: split s into two parts (before and from e start)
                    //         no slot reservation required

                    // Create a new slot table sn which is a copy of s
                    SlotTable sn = s;

                    // Update start and end time of s and sn
                    sn.setEndTime(entStartTime-1);
                    s.setStartTime(entStartTime);

                    // Add new slot table sn to sequence (before s)
                    insertSlotTable(iter, sn);
                    iter--;
                }
                else
                {
                    // Slot table s start at start of e or later, do slot
                    // reservations
                    if (s.getEndTime() <= entEndTime)
                    {
                        // Situation: s ends before or exactly when e ends
                        // Action: reserve slots in s and continue
                        s.reserveSlots(e, slots);
                    }
                    else if (entEndTime < s.getEndTime())
                    {
                        // Situation: e ends before s ends
                        // Action: split s into two parts (till and after e end)
                        //         reserve slots in first part

                        // Create a new slot table sn which is a copy of s
                        SlotTable sn = s;

                        // Update start and end time of s and sn
                        sn.setEndTime(entEndTime);
                        s.setStartTime(entEndTime+1);

                        // Reserve slots in s
                        sn.reserveSlots(e, slots);

                        // Add new slot table sn to sequence (before s)
                        insertSlotTable(iter, sn);
                    }
                }
            }
        }
    }
    else
    {
        // Iterate over the current slot table sequence to find slot table
        // with larger end time than the scheduling entity
        for (SlotTablesIter iter = seqBegin(); iter != seqEnd(); iter++)
        {
            SlotTable &s = *iter;

            // Current slot table falls within time frame scheduling entity?
            if (s.getEndTime() >= entStartTime
                    || s.getStartTime() <= entEndTime)
            {
                // Slot table s start at start of e or later, do slot
                // reservations
                if (s.getEndTime() <= entEndTime)
                {
                    // Situation: s ends before or exactly when e ends
                    // Action: reserve slots in s and continue
                    s.reserveSlots(e, slots);
                }
                else if (entEndTime < s.getEndTime()
                            && entEndTime >= s.getStartTime())
                {
                    // Situation: e ends before s ends
                    // Action: split s into two parts (till and after e end)
                    //         reserve slots in first part

                    // Create a new slot table sn which is a copy of s
                    SlotTable sn = s;

                    // Update start and end time of s and sn
                    sn.setEndTime(entEndTime);
                    s.setStartTime(entEndTime+1);

                    ASSERT(sn.getStartTime() <= sn.getEndTime(),
                            "Incorrect ordering slot table.");
                    ASSERT(s.getStartTime() <= s.getEndTime(),
                            "Incorrect ordering slot table.");

                    // Reserve slots in s
                    sn.reserveSlots(e, slots);

                    // Add new slot table sn to sequence (before s)
                    insertSlotTable(iter, sn);
                }
                
                // Slot table s starts at or before e starts, do slot
                // reservations
                if (s.getStartTime() >= entStartTime)
                {
                    // Situation: s starts after or at start of e
                    // Action: reserve slots in s and continue
                    s.reserveSlots(e, slots);
                }
                else if (s.getEndTime() >= entStartTime)
                {
                    // Slot table s starts before e, but ends at or after
                    // e starts.
                    // Action: split s into two parts (before and from start e)
                    //         no slots reservation needed in second part

                    // Create a new slot table sn which is a copy of s
                    SlotTable sn = s;

                    // Update start and end time of s and sn
                    sn.setEndTime(entStartTime-1);
                    s.setStartTime(entStartTime);

                    ASSERT(sn.getStartTime() <= sn.getEndTime(),
                            "Incorrect ordering slot table.");
                    ASSERT(s.getStartTime() <= s.getEndTime(),
                            "Incorrect ordering slot table.");

                    // Reserve slots in s
                    s.reserveSlots(e, slots);

                    // Add new slot table sn to sequence (before s)
                    insertSlotTable(iter, sn);
                }
            }
        }
    }
}

/**
 * releaseSlots ()
 * Release the claim on the slot in the slot tables made by scheduling entity e.  
 * The 'linkSeqNr' indicates the position of the link this slot table sequence
 * belongs to. The linkSeqNr starts at zero (=first link in route of e).
 */
void SlotTableSeq::releaseSlots(NoCSchedulingEntity *e, const ulong linkSeqNr)
{ 
    TTime period, entStartTime, entEndTime;
    bool loopPeriod;

    // Time frame of scheduling entity
    period = getSlotTablePeriod();
    entStartTime = (e->getStartTime() + linkSeqNr) % period;
    entEndTime = (e->getStartTime() + linkSeqNr + e->getDuration()) % period;
    loopPeriod = false;

    // The scheduling entity loops around the period when its start time
    // is equal or more then its end time. In the former case, the scheduling
    // entity takes a complete period, in the latter case it is less then a
    // period.
    if (entStartTime >= entEndTime && e->getDuration() != 1)
        loopPeriod = true;
    
    // Iterate over all slot tables in the sequence and release the reserved
    // slots for all slot tables within time bounds.
    for (SlotTablesIter iter = seqBegin(); iter != seqEnd(); iter++)
    {
        SlotTable &s = *iter;

        // Current slot table starts after scheduling entity?
        if (!loopPeriod && s.getStartTime() > entEndTime)
            return;

        // Current slot table falls within time frame scheduling entity?
        if (s.getStartTime() >= entStartTime 
                || (loopPeriod && entEndTime >= s.getEndTime()))
        {
            // Update the slot table
            s.releaseSlots(e);
        }
    }
}
