/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 3, 2006
 *
 *  Function        :   Binding of SDF components to architecture components.
 *
 *  History         :
 *      03-04-06    :   Initial version.
 *
 * $Id: binding.cc,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#include "binding.h"

/**
 * getValue ()
 * The function returns the value of a parameter.
 */
double ComponentBinding::getValue(const uint param) const
{
    // No paramter param associated with binding?
    if (param >= nrParams())
        return 0;
    
    return params[param];
}

/**
 * CompBindings ()
 * Constructor.
 */
CompBindings::CompBindings(Type t)
{
    type = t;
}

/**
 * CompBindings ()
 * Constructor.
 */
CompBindings::CompBindings(vector<double> &maxValues, Type t)
{
    maxVals = maxValues;
    curVals = vector<double>(maxValues.size(), 0);
    type = t;
}

/**
 * ~CompBindings ()
 * Destructor.
 */
CompBindings::~CompBindings()
{
    for (ComponentBindingsIter iter = begin();
            iter != end(); iter++)
    {
        delete (*iter);
    }
}

/**
 * getMaxVal ()
 * The function returns the maximum value of a parameter.
 */
double CompBindings::getMaxVal(const uint param) const
{
    // No paramter param associated with binding?
    if (param >= nrParams())
        return 0;
    
    return maxVals[param];
}

/**
 * getCurVal ()
 * The function returns the current value of a parameter.
 */
double CompBindings::getCurVal(const uint param) const
{
    // No paramter param associated with binding?
    if (param >= nrParams())
        return 0;
    
    return curVals[param];
}

/**
 * bind ()
 * Bind component to resource. On success, resources are claimed and
 * true is returned. Else, the function returns false.
 */
bool CompBindings::bind(ComponentBinding *b)
{
    if (getType() == TypeSum)
    {
        // Enough resources available?
        for (uint i = 0; i < nrParams(); i++)
        {
            if (getCurVal(i) + b->getValue(i) > maxVals[i])
                return false;
        }

        // Reserve resources
        for (uint i = 0; i < nrParams(); i++)
            curVals[i] += b->getValue(i);
    }
    else
    {
        // Enough resources available?
        for (uint i = 0; i < nrParams(); i++)
        {
            if (b->getValue(i) > maxVals[i])
                return false;
        }

        // Reserve resources
        for (uint i = 0; i < nrParams(); i++)
        {
            if (getCurVal(i) < b->getValue(i))
                curVals[i] = b->getValue(i);
        }
    }
            
    // Add component to list of bound components
    bindings.push_back(b);
    
    // Done
    return true;
}

/**
 * unbind ()
 * Remove binding from component to resource.
 */
bool CompBindings::unbind(SDFcomponent *c)
{
    bool success = false;
    
    // Find SDF component in list of bound component
    for (ComponentBindingsIter iter = begin();
            iter != end(); iter++)
    {
        ComponentBinding *b = *iter;
        
        if (b->getComponent() == c)
        {
            // Update resource usage
            if (getType() == TypeSum)
            {
                // Add values of parameters to available resources
                for (uint i = 0; i < nrParams(); i++)
                    curVals[i] -= b->getValue(i);
            }
                        
            // Cleanup binding
            delete b;
            
            // Erase binding from list of bound component
            bindings.erase(iter);
            
            // Done
            success = true;
            break;
        }
    }

    // Update resource usage
    if (success && getType() == TypeMax)
    {
        for (uint i = 0; i < nrParams(); i++)
            curVals[i] = 0;

        for (ComponentBindingsIter iter = begin();
                iter != end(); iter++)
        {
            ComponentBinding *b = *iter;
            
            for (uint i = 0; i < nrParams(); i++)
                if (curVals[i] < b->getValue(i))
                    curVals[i] = b->getValue(i);
        }
    }
        
    return success;
}

/**
 * find ()
 * Find binding of component to resource.
 */
ComponentBinding *CompBindings::find(SDFcomponent *c)
{
    // Find SDF component in list of bound component
    for (ComponentBindingsIter iter = begin();
            iter != end(); iter++)
    {
        ComponentBinding *b = *iter;
        
        if (b->getComponent()->getId() == c->getId())
            return b;
    }

    // No SDF component c found
    return NULL;
}
