/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   link.cc
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
 * $Id: link.cc,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#include "link.h"
#include "node.h"
#include "schedulingentity.h"

/**
 * Link
 * Constructor.
 */
Link::Link(CString name, CId id, Node *src, Node *dst, uint slotTableSize, 
        TTime slotTablePeriod)
    :
        name(name),
        id(id),
        srcNode(src),
        dstNode(dst)
{
    slotTableSeq = new SlotTableSeq(slotTableSize, slotTablePeriod);
    preferredSlots.resize(slotTableSize);
    for (uint i = 0; i < slotTableSize; i++)
        preferredSlots[i] = 0;    
}

/**
 * Link
 * Destructor.
 */
Link::~Link()
{
    delete slotTableSeq;
}

/**
 * getUsedSlotsInSchedule ()
 * The function returns a table of slot reservations for the link. A slot
 * is considered to be reserved if it is at some point in time used for the
 * problem that is scheduled on the link.
 */
SlotReservations Link::getUsedSlotsInSchedule()
{
    SlotReservations slotReservations, reservationsInSlot;
    uint slotTableSize;
    
    // Slot table size
    slotTableSize = slotTableSeq->getSlotTableSize();
    
    // Initialize slot reservations
    slotReservations.resize(slotTableSize);
    for (uint i = 0; i < slotTableSize; i++)
        slotReservations[i] = false;
    
    // Iterate over the slot table sequence
    for (SlotTablesIter iter = slotTableSeq->seqBegin();
            iter != slotTableSeq->seqEnd(); iter++)
    {
        SlotTable &s = *iter;
        
        reservationsInSlot = s.getUsedSlotsOfSchedule();
        
        for (uint i = 0; i < slotTableSize; i++)
            slotReservations[i] = slotReservations[i] || reservationsInSlot[i];
    }
    
    return slotReservations;
}

/**
 * setUsedSlots ()
 * The function marks the slots specified in s as occupied between the given
 * start and end time by another schedule.
 */
void Link::setUsedSlots(SlotReservations &s, TTime startTime, TTime endTime)
{
    slotTableSeq->setUsedSlots(s, startTime, endTime);
}

/**
 * print ()
 * Output all slot tables of a link
 */
ostream &Link::print(ostream &out) const
{
    TTime slotTablePeriod = slotTableSeq->getSlotTablePeriod();
    double width = log10((double)(slotTablePeriod));
    width = ceil(width);
    
    out << "link " << getName() << ": " << getSrcNode()->getName();
    out << " -> " << getDstNode()->getName() << endl;

    for (SlotTablesCIter iter = slotTableSeqBegin();
            iter != slotTableSeqEnd(); iter++)
    {
        const SlotTable &s = *iter;
        bool first = true;
        
        out << "(";
        out.width(int(width));    
        out << s.getStartTime();
        out << ", ";
        out.width(int(width));    
        out << s.getEndTime();
        out << "): ";

        uint i = 0;
        for (NoCSchedulingEntitiesCIter iterE = s.begin();
                iterE != s.end(); iterE++, i++)
        {
            const NoCSchedulingEntity *e = *iterE;
            
            if (!first)
                out << ", ";
            else
                first = false;
            
            if (!s.isSlotFree(i) && e == NULL)
                out << "r";
            else if (e == NULL)
                out << "x";
            else
                out << e->getMessage()->getId();
        }
        
        out << endl;
    }
    
    return out;
}
