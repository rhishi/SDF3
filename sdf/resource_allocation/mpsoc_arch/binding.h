/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.h
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
 * $Id: binding.h,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_BINDING_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_BINDING_H_INCLUDED

#include "../../base/timed/graph.h"

class ComponentBinding
{
public:
    // Constructor
    ComponentBinding(SDFcomponent *c, uint nrParams)
            : component(c), params(nrParams) {};
    
    // Destructor
    ~ComponentBinding() {};

    // Component
    SDFcomponent *getComponent() const { return component; };

    // Parameters
    double getValue(uint param) const;
    void setValue(uint param, double val) { params[param] = val; };
    uint nrParams() const { return params.size(); };
    
private:
    // Component    
    SDFcomponent *component;
    
    // Parameters
    vector<double> params;
};

typedef list<ComponentBinding*>             ComponentBindings;
typedef ComponentBindings::iterator         ComponentBindingsIter;
typedef ComponentBindings::const_iterator   ComponentBindingsCIter;

/**
 * CompBindings
 * Container for bindings of components to resource
 */
class CompBindings
{
public:
    typedef enum {TypeSum = 0, TypeMax} Type;
    
public:
    // Constructor
    CompBindings(Type t = TypeSum);
    CompBindings(vector<double> &maxValues, Type t = TypeSum);
    
    // Destructor
    ~CompBindings();
    
    // Type of binding function (sum or max)
    Type getType() const { return type; };
    
    // Parameters
    double getMaxVal(const uint param) const;
    double getCurVal(const uint param) const;
    uint nrParams() const { return maxVals.size(); };
    
    // Bind component to resource
    bool bind(ComponentBinding *b);

    // Remove binding from component to resource
    bool unbind(SDFcomponent *c);
    
    // Find binding of component to resource
    ComponentBinding *find(SDFcomponent *c);
    
    // List of all components bound to the resource
    ComponentBindingsIter begin() { return bindings.begin(); };
    ComponentBindingsIter end() { return bindings.end(); };
    ComponentBindingsCIter begin() const { return bindings.begin(); };
    ComponentBindingsCIter end() const { return bindings.end(); };
    
private:
    Type type;
    ComponentBindings bindings;
    vector<double> maxVals;
    vector<double> curVals;
};

#endif
