/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF actor
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: actor.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "actor.h"

/**
 * SDFactor ()
 * Constructor.
 */
SDFactor::SDFactor(SDFcomponent &c)
    :
        SDFcomponent(c)
{
}

/**
 * ~SDFactor ()
 * Destructor.
 */
SDFactor::~SDFactor()
{
    // Ports
    for (SDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        SDFport *p = *iter;

        delete p;
    }
}

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF actor object.
 */
SDFactor *SDFactor::create(SDFcomponent &c) const
{
    return new SDFactor(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF actor object.
 * All properties and ports of the actor are cloned.
 */
SDFactor *SDFactor::clone(SDFcomponent &c) const
{
    SDFactor *a = new SDFactor(c);
    
    // Properties
    a->setName(getName());
    a->setType(getType());
    
    // Ports
    for (SDFportsCIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        SDFport *p = *iter;
        SDFcomponent component = SDFcomponent(a, a->nrPorts());
        SDFport *pA = p->clone(component);
        a->addPort(pA);
    }
    
    return a;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF actor object.
 * All properties of the actor are copied.
 */
SDFactor *SDFactor::createCopy(SDFcomponent &c) const
{
    SDFactor *a = new SDFactor(c);
    
    // Properties
    a->setName(getName());
    a->setType(getType());
    
    return a;
}

/**
 * construct ()
 * The function initializes all actor properties based on the XML data.
 */
void SDFactor::construct(const CNodePtr actorNode)
{
    SDFactor *a = this;
    
    // Name
    if (!CHasAttribute(actorNode, "name"))
        throw CException("Invalid SDF graph, missing actor name.");
    a->setName(CGetAttribute(actorNode, "name"));

    // Type
    if (!CHasAttribute(actorNode, "type"))
        throw CException("Invalid SDF graph, missing actor type.");
    a->setType(CGetAttribute(actorNode, "type"));
}

/**
 * createPort ()
 * Create a new port on the actor.
 */
SDFport *SDFactor::createPort(SDFcomponent &c)
{
    SDFport *p = new SDFport(c);
    addPort(p);
    
    return p;
}

/**
 * createPort ()
 * Create a new port on the actor.
 */
SDFport *SDFactor::createPort(const SDFport::SDFportType type,
        const SDFrate rate)
{
    SDFcomponent c = SDFcomponent(this, nrPorts());
    SDFport *p = createPort(c);

    p->setType(type);
    p->setRate(rate);    
    
    return p;
}

/**
 * isConnected ()
 * The function returns true if all ports are connected to a channel, else
 * it returns false.
 */
bool SDFactor::isConnected() const
{
    for (SDFportsCIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        const SDFport *p = *iter;
        
        if (!p->isConnected())
            return false;
    }
    
    return true;
}

/**
 * getPort ()
 * The function returns a reference to a port with the given id.
 */
SDFport *SDFactor::getPort(const CId id)
{
    for (SDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        SDFport *p = *iter;
        
        if (p->getId() == id)
            return p;
    }
    
    throw CException("Actor '" + getName() 
                + "' has no port with id '" + CString(id) + "'.");
}

/**
 * getPort ()
 * The function returns a reference to a port with the given name.
 */
SDFport *SDFactor::getPort(const CString &name)
{
    for (SDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        SDFport *p = *iter;
        
        if (p->getName() == name)
            return p;
    }
    
    throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
}

/**
 * addPort ()
 * Add a port to an actor.
 */
void SDFactor::addPort(SDFport *p)
{
    ports.push_back(p);
}

/**
 * removePort ()
 * Remove a port from an actor and destory its memory space.
 */
void SDFactor::removePort(const CString &name)
{
    for (SDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        SDFport *p = *iter;
        
        if (p->getName() == name)
        {
            delete p;
            ports.erase(iter);
            return;
        }
    }

    throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
}

/**
 * print ()
 * Print the actor to the supplied output stream.
 */
ostream &SDFactor::print(ostream &out)
{
    out << "Actor (" << getName() << ")" << endl;
    out << "id:        " << getId() << endl;
    out << "type:      " << getType() << endl;
    out << endl;

    for (SDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        SDFport *p = *iter;
        
        p->print(out);
    }
    
    return out;
}
