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
 *  Function        :   CSDF actor
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: actor.cc,v 1.3 2008/03/22 14:24:21 sander Exp $
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
CSDFactor::CSDFactor(CSDFcomponent &c)
    :
        CSDFcomponent(c)
{
}

/**
 * ~CSDFactor ()
 * Destructor.
 */
CSDFactor::~CSDFactor()
{
    // Ports
    for (CSDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        CSDFport *p = *iter;

        delete p;
    }
}

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 */
CSDFactor *CSDFactor::create(CSDFcomponent &c) const
{
    return new CSDFactor(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 * All properties and ports of the actor are cloned.
 */
CSDFactor *CSDFactor::clone(CSDFcomponent &c) const
{
    CSDFactor *a = new CSDFactor(c);
    
    // Properties
    a->setName(getName());
    a->setType(getType());
    
    // Ports
    for (CSDFportsCIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        CSDFport *p = *iter;
        CSDFcomponent component = CSDFcomponent(a, a->nrPorts());
        CSDFport *pA = p->clone(component);
        a->addPort(pA);
    }
    
    return a;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 * All properties of the actor are copied.
 */
CSDFactor *CSDFactor::createCopy(CSDFcomponent &c) const
{
    CSDFactor *a = new CSDFactor(c);
    
    // Properties
    a->setName(getName());
    a->setType(getType());
    
    return a;
}

/**
 * construct ()
 * The function initializes all actor properties based on the XML data.
 */
void CSDFactor::construct(const CNodePtr actorNode)
{
    CSDFactor *a = this;
    
    // Name
    if (!CHasAttribute(actorNode, "name"))
        throw CException("Invalid CSDF graph, missing actor name.");
    a->setName(CGetAttribute(actorNode, "name"));

    // Type
    if (!CHasAttribute(actorNode, "type"))
        throw CException("Invalid CSDF graph, missing actor type.");
    a->setType(CGetAttribute(actorNode, "type"));
}

/**
 * createPort ()
 * Create a new port on the actor.
 */
CSDFport *CSDFactor::createPort(CSDFcomponent &c)
{
    CSDFport *p = new CSDFport(c);
    addPort(p);
    
    return p;
}

/**
 * createPort ()
 * Create a new port on the actor.
 */
CSDFport *CSDFactor::createPort(const CString &type, const CSDFrate rate)
{
    CSDFcomponent c = CSDFcomponent(this, nrPorts());
    CSDFport *p = createPort(c);

    p->setType(type);
    p->setRate(rate);    
    
    return p;
}

/**
 * isConnected ()
 * The function returns true if all ports are connected to a channel, else
 * it returns false.
 */
bool CSDFactor::isConnected() const
{
    for (CSDFportsCIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        const CSDFport *p = *iter;
        
        if (!p->isConnected())
            return false;
    }
    
    return true;
}

/**
 * getPort ()
 * The function returns a reference to a port with the given id.
 */
CSDFport *CSDFactor::getPort(const CId id)
{
    for (CSDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        CSDFport *p = *iter;
        
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
CSDFport *CSDFactor::getPort(const CString &name)
{
    for (CSDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        CSDFport *p = *iter;
        
        if (p->getName() == name)
            return p;
    }
    
    throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
}

/**
 * addPort ()
 * Add a port to an actor.
 */
void CSDFactor::addPort(CSDFport *p)
{
    ports.push_back(p);
}

/**
 * removePort ()
 * Remove a port from an actor and destory its memory space.
 */
void CSDFactor::removePort(const CString &name)
{
    for (CSDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        CSDFport *p = *iter;
        
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
 * removePorts ()
 * Remove all ports from the actor and destroy their memory space.
 */
void CSDFactor::removePorts()
{
    for (CSDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        CSDFport *p = *iter;
        delete p;
    }

    ports.clear();
}

/**
 * print ()
 * Print the actor to the supplied output stream.
 */
ostream &CSDFactor::print(ostream &out)
{
    out << "Actor (" << getName() << ")" << endl;
    out << "id:        " << getId() << endl;
    out << "type:      " << getType() << endl;
    out << endl;

    for (CSDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        CSDFport *p = *iter;
        
        p->print(out);
    }
    
    return out;
}
