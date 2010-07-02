/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   port.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF port
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: port.cc,v 1.4 2008/03/22 14:24:21 sander Exp $
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

#include "port.h"

/**
 * CSDFport ()
 * Constructor.
 */
CSDFport::CSDFport(CSDFcomponent &c)
    :
        CSDFcomponent(c),
        rate(1),
        channel(NULL)
{
}

/**
 * ~CSDFport ()
 * Destructor.
 */
CSDFport::~CSDFport()
{
}

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF port object.
 */
CSDFport *CSDFport::create(CSDFcomponent &c) const
{
    return new CSDFport(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF port object.
 * The properties of the port are also cloned. Note: the connection to the
 * channel is lost. Cloning the channel restores this connection.
 */
CSDFport *CSDFport::clone(CSDFcomponent &c) const
{
    CSDFport *p = new CSDFport(c);
    
    p->setName(getName());
    p->setType(getType());
    p->setRate(rate);
    
    return p;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF port object.
 * The properties of the port are also copied.
 */
CSDFport *CSDFport::createCopy(CSDFcomponent &c) const
{
    CSDFport *p = new CSDFport(c);
    
    p->setName(getName());
    p->setType(getType());
    p->setRate(rate);
    
    return p;
}

/**
 * construct ()
 * The function initializes all port properties based on the XML data.
 */
void CSDFport::construct(const CNodePtr portNode)
{
    CSDFport *p = this;
    
    // Name
    if (!CHasAttribute(portNode, "name"))
        throw CException("Invalid SDF graph, missing port name.");
    p->setName(CGetAttribute(portNode, "name"));

    // Type
    if (!CHasAttribute(portNode, "type"))
        throw CException("Invalid SDF graph, missing port type.");
    p->setType(CGetAttribute(portNode, "type"));

    // Rate
    if (!CHasAttribute(portNode, "rate"))
        throw CException("Invalid SDF graph, missing port rate.");
    p->setRate(CGetAttribute(portNode, "rate"));
}

/**
 * connectToChannel ()
 * The function connects the port to a channel. A connection can only be made
 * if the port is not yet connected.
 */
void CSDFport::connectToChannel(CSDFchannel *c)
{
    if (isConnected() && c != NULL)
        throw CException("Port '" + getParent()->getName() 
                        + "." + getName() + "' already connected.");
        
    channel = c;
}

/**
 * isConnected ()
 * The function return true if the port is connected to a channel, else it
 * returns false.
 */
bool CSDFport::isConnected() const
{
    if (getChannel() == NULL)
        return false;
        
    return true;
}

/**
 * print ()
 * Print the port to the supplied output stream.
 */
ostream &CSDFport::print(ostream &out)
{
    out << "Port (" << getName() << ")" << endl;
    out << "id:        " << getId() << endl;
    out << "type:      " << getTypeAsString() << endl;
    out << "rate:      " << getRate() << endl;
    out << "connected: " << (isConnected() ? "true" : "false") << endl;
    out << endl;
        
    return out;
}

