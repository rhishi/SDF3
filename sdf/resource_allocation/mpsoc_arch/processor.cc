/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Processor.
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: processor.cc,v 1.2 2008/03/06 10:49:45 sander Exp $
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

#include "processor.h"

/**
 * Processor ()
 * Constructor.
 */
Processor::Processor(ArchComponent &c) : ArchComponent(c)
{ 
    // TDMA 
    wheelSize = 0;
    occupiedTimeSlice = 0;
    reservedTimeSlice = 0;
    
    // Actor bindings
    actorBindings = new CompBindings();
}

/**
 * getTimewheelSize ()
 * The function returns the size of the TDMA timewheel.
 */
CSize Processor::getTimewheelSize() const
{
    return wheelSize;
}

/**
 * setTimewheelSize ()
 * The function sets the size of the TDMA timewheel. The reserved slice is lost.
 */
void Processor::setTimewheelSize(const CSize sz)
{
    // Empty timewheel
    wheelSize = sz;
    reservedTimeSlice = 0;

    if (occupiedTimeSlice > wheelSize)
        throw CException("Occupied size on timewheel exceeds timewheel size.");
}

/**
 * getOccupiedTimeSlice ()
 * The function returns the size of the time slice occupied by other
 * reservations (i.e. this part of the wheel is not available).
 */
CSize Processor::getOccupiedTimeSlice() const
{
    return occupiedTimeSlice;
}

/**
 * getOccupiedTimeSlice ()
 * The function sets the size of the time slice occupied by other
 * reservations (i.e. this part of the wheel is not available).
 */
void Processor::setOccupiedTimeSlice(const CSize sz)
{
    occupiedTimeSlice = sz;
    
    if (occupiedTimeSlice > wheelSize)
        throw CException("Occupied size on timewheel exceeds timewheel size.");
}

/**
 * getReservedTimeSlice ()
 * The function returns the size of the time slice reserved for the SDF graph.
 */
CSize Processor::getReservedTimeSlice() const
{
    return reservedTimeSlice;
}

/**
 * reserveTimeSlice ()
 * The function tries to reserve a timeslice on the timewheel.
 */
bool Processor::reserveTimeSlice(CSize sz)
{
    // Not enough space on timewheel?
    if (getReservedTimeSlice() + availableTimewheelSize() < sz)
        return false;

    // Reserve the slice
    reservedTimeSlice = sz;    
    
    return true;
}

/**
 * releaseTimeSlice ()
 * The function removes the time slice reservation.
 */
bool Processor::releaseTimeSlice()
{
    reservedTimeSlice = 0;
    
    return true;
}

/**
 * availableTimewheelSize ()
 * The function returns the available timewheel size.
 */
CSize Processor::availableTimewheelSize() const
{
    return wheelSize - occupiedTimeSlice - reservedTimeSlice;
}

/**
 * bindActor ()
 * The function tries to bind an actor to the processor. On success, it
 * returns true and adss a binding of the actor to the resource. On failure,
 * it returns false and it does not change the bindings to the processor.
 */
bool Processor::bindActor(TimedSDFactor *a)
{
    ComponentBinding *b;

    // Can actor not run on this processor type?
    if (a->getProcessor(getType()) == NULL)
        return false;
    
    // Create component binding
    b = new ComponentBinding(a, 0);
    return actorBindings->bind(b);
}

/**
 * unbindActor ()
 * The function removes the binding of the actor to the processor.
 */
bool Processor::unbindActor(TimedSDFactor *a)
{
    return actorBindings->unbind(a);
}

