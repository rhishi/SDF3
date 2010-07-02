/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 18, 2005
 *
 *  Function        :   Timed SDF channel
 *
 *  History         :
 *      18-07-05    :   Initial version.
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

/**
 * TimedSDFchannel ()
 * Constructor.
 */
TimedSDFchannel::TimedSDFchannel(SDFcomponent &c)
    :
        SDFchannel(c),
        modelStorageSpaceCh(NULL),
        tokenSize(SDF_INFINITE_SIZE)
{
    // Unbounded
    bufferSize.sz = SDF_INFINITE_SIZE;
    bufferSize.src = SDF_INFINITE_SIZE;
    bufferSize.dst = SDF_INFINITE_SIZE;
    bufferSize.mem = SDF_INFINITE_SIZE;
    
    // No delay
    minBandwidth = 0;
    minLatency = 0;
}

/**
 * ~TimedSDFchannel ()
 * Destructor.
 */
TimedSDFchannel::~TimedSDFchannel()
{
}

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF channel object.
 */
TimedSDFchannel *TimedSDFchannel::create(SDFcomponent &c) const
{
    return new TimedSDFchannel(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF channel object.
 * The properties of the object are cloned.
 */
TimedSDFchannel *TimedSDFchannel::clone(SDFcomponent &c) const
{
    TimedSDFchannel *ch = createCopy(c);
    
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
 * The properties of the object are copied.
 */
TimedSDFchannel *TimedSDFchannel::createCopy(SDFcomponent &c) const
{
    TimedSDFchannel *ch = new TimedSDFchannel(c);
    
    // Properties
    ch->setName(getName());
    ch->setBufferSize(getBufferSize());
    ch->setTokenSize(getTokenSize());
    ch->setTokenType(getTokenType());
    ch->setMinBandwidth(getMinBandwidth());
    ch->setStorageSpaceChannel(getStorageSpaceChannel());
    ch->setMinLatency(getMinLatency());

    return ch;
}

/**
 * setProperties ()
 * Set the properties of a timed channel.
 */
void TimedSDFchannel::setProperties(const CNodePtr propertiesNode)
{
    // Buffer size
    if (CHasChildNode(propertiesNode, "bufferSize"))
    {
        CNode *bufferSizeNode = CGetChildNode(propertiesNode, "bufferSize");
        BufferSize bufferSize;

        bufferSize.sz = SDF_INFINITE_SIZE;
        bufferSize.src = SDF_INFINITE_SIZE;
        bufferSize.dst = SDF_INFINITE_SIZE;
        bufferSize.mem = SDF_INFINITE_SIZE;
        
        if (CHasAttribute(bufferSizeNode, "sz"))
            bufferSize.sz = CGetAttribute(bufferSizeNode, "sz");
            
        if (CHasAttribute(bufferSizeNode, "src"))
            bufferSize.src = CGetAttribute(bufferSizeNode, "src");

        if (CHasAttribute(bufferSizeNode, "dst"))
            bufferSize.dst = CGetAttribute(bufferSizeNode, "dst");

        if (CHasAttribute(bufferSizeNode, "mem"))
            bufferSize.mem = CGetAttribute(bufferSizeNode, "mem");
        
        setBufferSize(bufferSize);
    }

    // Token size
    if (CHasChildNode(propertiesNode, "tokenSize"))
    {
        CNode *tokenSizeNode = CGetChildNode(propertiesNode, "tokenSize");
        
        if (CHasAttribute(tokenSizeNode, "sz"))
        {
            int sz = CGetAttribute(tokenSizeNode, "sz");
            setTokenSize(sz);
        }
    }

    // Token type
    if (CHasChildNode(propertiesNode, "tokenType"))
    {
        CNode *tokenTypeNode = CGetChildNode(propertiesNode, "tokenType");
        
        if (CHasAttribute(tokenTypeNode, "type"))
        {
            CString type = CGetAttribute(tokenTypeNode, "type");
            setTokenType(type);
        }
    }

    // Bandwidth
    if (CHasChildNode(propertiesNode, "bandwidth"))
    {
        CNode *bwNode = CGetChildNode(propertiesNode, "bandwidth");
        
        if (CHasAttribute(bwNode, "min"))
        {
            double sz = CGetAttribute(bwNode, "min");
            setMinBandwidth(sz);
        }
    }

    // Latency
    if (CHasChildNode(propertiesNode, "latency"))
    {
        CNode *latencyNode = CGetChildNode(propertiesNode, "latency");
        
        if (CHasAttribute(latencyNode, "min"))
        {
            int sz = CGetAttribute(latencyNode, "min");
            setMinLatency(sz);
        }
    }
}

/**
 * isUnbounded ()
 * The function returns true if the channel is unbounded. If the size is bounded
 * it returns false.
 */
bool TimedSDFchannel::isUnbounded() const
{
    if (getBufferSize().sz == SDF_INFINITE_SIZE)
        return true;
    
    return false;
}

/**
 * isControlToken ()
 * The function returns true if the channel is contains control tokens, else it
 * return false. Note, if the token size is unknown, the functions always
 * returns false.
 */
bool TimedSDFchannel::isControlToken() const
{
    if (getTokenSize() == 0)
        return true;
    
    return false;
}

/**
 * isDataToken ()
 * The function returns true if the channel is contains data tokens, else it
 * return false. Note, if the token size is unknown, the functions always
 * returns false.
 */
bool TimedSDFchannel::isDataToken() const
{
    if (getTokenSize() > 0)
        return true;
    
    return false;
}

/**
 * print ()
 * Print the channel to the supplied output stream.
 */
ostream &TimedSDFchannel::print(ostream &out)
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

    TimedSDFchannel::BufferSize bufferSize = getBufferSize();
    if (bufferSize.sz != SDF_INFINITE_SIZE)
        out << "buffer size:    " << bufferSize.sz << endl;
    
    if (getTokenSize() != SDF_INFINITE_SIZE)
        out << "token size:     " << getTokenSize() << endl;

    if (getMinLatency() != 0)
        out << "min latency:    " << getMinLatency() << endl;

    if (getMinBandwidth() != 0)
        out << "min bandwidth:  " << getMinBandwidth() << endl;
    
    if (!getTokenType().empty())
        out << "token type:     " << getTokenType() << endl;
    
    out << endl;
        
    return out;
}
