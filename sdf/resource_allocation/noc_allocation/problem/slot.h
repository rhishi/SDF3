/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   slot.h
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
 * $Id: slot.h,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_SLOT_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_SLOT_H_INCLUDED

#include "../../../basic_types.h"
#include "base/base.h"

/**
 * SlotReservations
 * A sequence of booleans which make wether a slot in the slot table is used or
 * not-used.
 */
typedef vector<bool> SlotReservations;

// Forward class definition
class NoCSchedulingEntity;
typedef vector<NoCSchedulingEntity*>           NoCSchedulingEntities;
typedef NoCSchedulingEntities::iterator        NoCSchedulingEntitiesIter;
typedef NoCSchedulingEntities::const_iterator  NoCSchedulingEntitiesCIter;

/**
 * SlotTable
 * The slot table...
 */
class SlotTable
{
public:
    // Constructor
    SlotTable(uint sz);

    // Destructor
    ~SlotTable();
    
    // Properties
    TTime getStartTime() const { return startTime; };
    void setStartTime(const TTime t) { startTime = t; };
    TTime getEndTime() const { return endTime; };
    void setEndTime(const TTime t) { endTime = t; };
    uint getNrSlots() const { return nrSlots; };
    uint getNrFreeSlots() const { return nrFreeSlots; };
    bool isSlotFree(const uint i) const;
    
    // Slots
    NoCSchedulingEntitiesIter begin() { return tableEntities.begin(); };
    NoCSchedulingEntitiesCIter begin() const { return tableEntities.begin(); };
    NoCSchedulingEntitiesIter end() { return tableEntities.end(); };
    NoCSchedulingEntitiesCIter end() const { return tableEntities.end(); };
    NoCSchedulingEntity *operator[] (const uint i) { return tableEntities[i]; };
    
    // Slot (de)allocation
    void setUsedSlots(SlotReservations &s);
    void reserveSlots(NoCSchedulingEntity *e, SlotReservations &s);
    void releaseSlots(NoCSchedulingEntity *e);
    
    // Slot reservations
    SlotReservations getSlotReservations(NoCSchedulingEntity *e) const;
    SlotReservations getUsedSlotsOfSchedule() const;
    
private:
    // Properties
    TTime startTime;
    TTime endTime;
    uint nrFreeSlots;
    uint nrSlots;
    SlotReservations tableReservations;
    NoCSchedulingEntities tableEntities;
};

typedef list<SlotTable>             SlotTables;
typedef SlotTables::iterator        SlotTablesIter;
typedef SlotTables::const_iterator  SlotTablesCIter;

/**
 * SlotTableSeq
 * A sequence of slot tables
 */
class SlotTableSeq
{
public:
    // Constructor
    SlotTableSeq(uint slotTableSize, TTime slotTablePeriod);

    // Destructor
    ~SlotTableSeq();
    
    // Properties
    uint getSlotTableSize() const { return slotTableSize; };
    TTime getSlotTablePeriod() const { return slotTablePeriod; };
    uint nrFreeSlots(const TTime startTime, const TTime duration);
    
    // Slot tables
    SlotTablesIter seqBegin() { return seq.begin(); };
    SlotTablesCIter seqBegin() const { return seq.begin(); };
    SlotTablesIter seqEnd() { return seq.end(); };
    SlotTablesCIter seqEnd() const { return seq.end(); };
    
    // Modify slot table sequence
    void insertSlotTable(SlotTablesIter pos, SlotTable &s)
            { seq.insert(pos, s); };
    
    // Slot (de)allocation
    void setUsedSlots(SlotReservations &s, TTime startTime, TTime endTime);
    void reserveSlots(NoCSchedulingEntity *e, SlotReservations &s,
            const ulong linkSeqNr);
    void releaseSlots(NoCSchedulingEntity *e, const ulong linkSeqNr);
    
private:
    // Properties
    uint slotTableSize;
    TTime slotTablePeriod;

    // Slot tables
    SlotTables seq;
};

#endif
