/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Tile memory.
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: memory.cc,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#include "memory.h"

/**
 * getSize ()
 * Get the size of the memory.
 */
CSize Memory::getSize() const
{
    double sz;
    
    if (memActor == NULL)
        return 0;
    
    sz = memActor->getMaxVal(0);
    
    return CSize(sz);
}

/**
 * setSize ()
 * Set the size of the memory. The function removes all existing component
 * bindings to the memory.
 */
void Memory::setSize(const CSize sz)
{
    vector<double> maxVals(1);
    maxVals[0] = sz;

    // Cleanup existing component bindings
    delete memActor;
    delete memChannel;

    // Create new component bindings component for memory
    memActor = new CompBindings(maxVals, CompBindings::TypeMax);
    memChannel = new CompBindings(maxVals, CompBindings::TypeSum);
}

/**
 * availableMemorySize ()
 * The function returns the amount of memory which is free.
 */
CSize Memory::availableMemorySize() const
{
    double sz = memActor->getMaxVal(0) - memActor->getCurVal(0)
                    - memChannel->getCurVal(0);
    
    return (CSize)(sz);
}

/**
 * occupiedMemorySizeByActors ()
 * The function returns the amount of memory used by the bound actors.
 */
CSize Memory::occupiedMemorySizeByActors() const
{
    return (CSize)(memActor->getCurVal(0));
}


/**
 * reserveMemory ()
 * The function tries to reserve memory space with size sz for the SDF actor a.
 * On success, it returns true and it creates a binding between the actor
 * and the memory. Else, it returns false and no binding is created.
 */
bool Memory::reserveMemory(SDFactor *a, CSize sz)
{
    ComponentBinding *b;

    // Memory requirement of actor more then current usage?
    if (occupiedMemorySizeByActors() < sz)
    {
        // Not enough space available?
        if (availableMemorySize() < sz - occupiedMemorySizeByActors())
            return false;
    }

    // Create component binding
    b = new ComponentBinding(a, 1);
    b->setValue(0, sz);
    
    return memActor->bind(b);
}

/**
 * releaseMemory ()
 * The function releases the memory reserved by SDF actor a and removes its
 * binding to the memory.
 */
bool Memory::releaseMemory(SDFactor *a)
{
    return memActor->unbind(a);
}

/**
 * reserveMemory ()
 * The function tries to reserve memory space with size sz for the SDF channl c.
 * On success, it returns true and it creates a binding between the channel
 * and the memory. Else, it returns false and no binding is created.
 */
bool Memory::reserveMemory(SDFchannel *c, CSize sz)
{
    ComponentBinding *b;

    // Enough space available?
    if (availableMemorySize() < sz)
        return false;
    
    // Create component binding
    b = new ComponentBinding(c, 1);
    b->setValue(0, sz);
    
    return memChannel->bind(b);
}

/**
 * releaseMemory ()
 * The function releases the memory reserved by SDF channl c and removes its
 * binding to the memory.
 */
bool Memory::releaseMemory(SDFchannel *c)
{
    return memChannel->unbind(c);
}

/**
 * reserveMemory ()
 * The function tries to reserve memory space with size sz. On success, it
 * returns true. Else, it returns false and no binding is created.
 */
bool Memory::reserveMemory(CSize sz)
{
    ComponentBinding *b;

    // Enough space available?
    if (availableMemorySize() < sz)
        return false;
    
    // Create component binding
    b = new ComponentBinding(NULL, 1);
    b->setValue(0, sz);
    
    return memChannel->bind(b);
}

/**
 * releaseMemory ()
 * The function releases the memory.
 */
bool Memory::releaseMemory()
{
    return memChannel->unbind(NULL);
}
