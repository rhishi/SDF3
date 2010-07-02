/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   link.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Link
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: link.h,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_LINK_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_LINK_H_INCLUDED

#include "slot.h"

// Forward class definition
class Node;

/**
 * Link
 * A physical connection between two nodes
 */
class Link
{
public:
    // Constructor
    Link(CString name, CId id, Node *src, Node *dst, uint slotTableSize, 
            TTime slotTablePeriod);
    
    // Destrcutor
    ~Link();

    // Id
    CId getId() const { return id; };
    
    // Name
    CString getName() const { return name; };
    
    // Nodes
    Node *getSrcNode() const { return srcNode; };
    Node *getDstNode() const { return dstNode; };
    
    // Slot
    SlotTablesIter slotTableSeqBegin() { return slotTableSeq->seqBegin(); };
    SlotTablesCIter slotTableSeqBegin() const 
        { return slotTableSeq->seqBegin(); };
    SlotTablesIter slotTableSeqEnd() { return slotTableSeq->seqEnd(); };
    SlotTablesCIter slotTableSeqEnd() const { return slotTableSeq->seqEnd(); };
    SlotTableSeq *getSlotTableSeq() const { return slotTableSeq; };
    SlotReservations getUsedSlotsInSchedule();

    // Slot (de)allocation
    void setUsedSlots(SlotReservations &s, TTime startTime, TTime endTime);
    void reserveSlots(NoCSchedulingEntity *e, SlotReservations &s, ulong seqNr)
            { slotTableSeq->reserveSlots(e, s, seqNr); };
    void releaseSlots(NoCSchedulingEntity *e, ulong seqNr)
            { slotTableSeq->releaseSlots(e, seqNr); };

    // Preferred slots (value indicates preference level)
    vector<uint> preferredSlots;

    // Print
    ostream &print(ostream &out) const;
    
private:
    // Name
    CString name;
    
    // Id
    CId id;
    
    // Nodes
    Node *srcNode;
    Node *dstNode;
    
    // Slot
    SlotTableSeq *slotTableSeq;
};

typedef list<Link*>             Links;
typedef Links::iterator         LinksIter;
typedef Links::const_iterator   LinksCIter;

#endif
