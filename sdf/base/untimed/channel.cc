/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF channel
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: channel.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "channel.h"
#include "graph.h"

/**
 * SDFchannel ()
 * Constructor.
 */
SDFchannel::SDFchannel(SDFcomponent &c)
    :
        SDFcomponent(c),
        src(NULL),
        dst(NULL),
        initialTokens(0)
{
}

/**
 * ~SDFchannel ()
 * Destructor.
 */
SDFchannel::~SDFchannel()
{
}

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF channel object.
 */
SDFchannel *SDFchannel::create(SDFcomponent &c) const
{
    return new SDFchannel(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF channel object.
 * All properties and the connection of the channel are cloned.
 */
SDFchannel *SDFchannel::clone(SDFcomponent &c) const
{
    SDFchannel *ch = new SDFchannel(c);
    
    // Properties
    ch->setName(getName());
    
    // Connection
    if (getSrcActor() != NULL)
    {
        SDFport *p = ch->getGraph()->getActor(getSrcActor()->getName())
                            ->getPort(getSrcPort()->getName());
        ch->connectSrc(p);
    }
    if (getDstActor() != NULL)
    {
        SDFport *p = ch->getGraph()->getActor(getDstActor()->getName())
                            ->getPort(getDstPort()->getName());
        ch->connectDst(p);
    }
    
    // Initial tokens
    ch->setInitialTokens(getInitialTokens());

    return ch;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF channel object.
 * All properties of the channel are copied.
 */
SDFchannel *SDFchannel::createCopy(SDFcomponent &c) const
{
    SDFchannel *ch = new SDFchannel(c);
    
    // Properties
    ch->setName(getName());

    return ch;
}

/**
 * construct ()
 * The function initializes all channel properties based on the XML data.
 */
void SDFchannel::construct(const CNodePtr channelNode)
{
    CString strSrcActor, strSrcPort, strDstActor, strDstPort;
    SDFport *srcPort, *dstPort;
    SDFgraph *g;
    SDFchannel *c = this;
    
    // Graph
    g = c->getGraph();
    
    // Name
    if (!CHasAttribute(channelNode, "name"))
        throw CException("Invalid SDF graph, missing channel name.");
    c->setName(CGetAttribute(channelNode, "name"));

    // Initial tokens
    if (CHasAttribute(channelNode, "initialTokens"))
        c->setInitialTokens(CGetAttribute(channelNode, "initialTokens"));

    // Source and destination actor ports
    if (!CHasAttribute(channelNode, "srcActor"))
        throw CException("Invalid SDF graph, missing channel srcActor.");
    strSrcActor = CGetAttribute(channelNode, "srcActor");

    if (!CHasAttribute(channelNode, "srcPort"))
        throw CException("Invalid SDF graph, missing channel srcPort.");
    strSrcPort = CGetAttribute(channelNode, "srcPort");

    if (!CHasAttribute(channelNode, "dstActor"))
        throw CException("Invalid SDF graph, missing channel dstActor.");
    strDstActor = CGetAttribute(channelNode, "dstActor");

    if (!CHasAttribute(channelNode, "dstPort"))
        throw CException("Invalid SDF graph, missing channel dstPort.");
    strDstPort = CGetAttribute(channelNode, "dstPort");
    
    // Find actors and port
    srcPort = g->getActor(strSrcActor)->getPort(strSrcPort);
    dstPort = g->getActor(strDstActor)->getPort(strDstPort);

    // Connect channel to source and destination ports
    c->connectSrc(srcPort);
    c->connectDst(dstPort);
}

/**
 * connectSrc ()
 * The function connects the channel to a source port and port to channel.
 */
void SDFchannel::connectSrc(SDFport *p)
{
    if (p == NULL)
    {
        src->connectToChannel(NULL);
        src = NULL;
        return;
    }

    if (getSrcPort() != NULL)
        throw CException("Channel '" + getName() + "' already connected.");
    
    if (p->getType() != SDFport::Out)
    {
    cerr << "type: " << p->getTypeAsString() << endl;
            throw CException("Cannot connect port '" + p->getName() + "' to "
                         "channel '" + getName() + "'; conflicting port type.");
    }
    
    src = p;
    src->connectToChannel(this);
}

/**
 * connectDst ()
 * The function connects the channel to a destination port and port to channel.
 */
void SDFchannel::connectDst(SDFport *p)
{
    if (p == NULL)
    {
        dst->connectToChannel(NULL);
        dst = NULL;
        return;
    }
    
    if (getDstPort() != NULL)
        throw CException("Channel '" + getName() + "' already connected.");
    
    if (p->getType() != SDFport::In)
    {
    cerr << "type: " << p->getTypeAsString() << endl;
        throw CException("Cannot connect port '" + p->getName() + "' to "
                         "channel '" + getName() + "'; conflicting port type.");
    }
    
    dst = p;
    dst->connectToChannel(this);
}

/**
 * isConnected ()
 * The function returns true if the channel is connected to a source and
 * destination port.
 */
bool SDFchannel::isConnected() const
{
    if (getSrcPort() == NULL || getDstPort() == NULL)
        return false;
    
    return true;
}

/**
 * print ()
 * Print the channel to the supplied output stream.
 */
ostream &SDFchannel::print(ostream &out)
{
    out << "Channel (" << getName() << ")" << endl;
    out << "id:             " << getId() << endl;
    out << "initial tokens: " << getInitialTokens() << endl;
    out << "connected:      " << (isConnected() ? "true" : "false") << endl;
    
    if (isConnected())
    {
        out << "source:         " << getSrcActor()->getName() << ".";
        out << getSrcPort()->getName() << endl;
        out << "destination:    " << getDstActor()->getName() << ".";
        out << getDstPort()->getName() << endl;
    }
    
    out << endl;
        
    return out;
}
