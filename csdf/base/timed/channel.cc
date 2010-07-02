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
 *  Function        :   Timed CSDF channel
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: channel.cc,v 1.1.1.1 2007/10/02 10:59:49 sander Exp $
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
 * TimedCSDFchannel ()
 * Constructor.
 */
TimedCSDFchannel::TimedCSDFchannel(CSDFcomponent &c)
    :
        CSDFchannel(c),
        modelStorageSpaceCh(NULL),
        tokenSize(-1)
{
    // Unbounded
    bufferSize.sz = -1;
    bufferSize.src = -1;
    bufferSize.dst = -1;
    bufferSize.mem = -1;
    minBandwidth = 0;
    minLatency = 0;
}

/**
 * ~TimedCSDFchannel ()
 * Destructor.
 */
TimedCSDFchannel::~TimedCSDFchannel()
{
}

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF channel object.
 */
TimedCSDFchannel *TimedCSDFchannel::create(CSDFcomponent &c) const
{
    return new TimedCSDFchannel(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF channel object.
 * The properties of the object are cloned.
 */
TimedCSDFchannel *TimedCSDFchannel::clone(CSDFcomponent &c) const
{
    TimedCSDFchannel *ch = createCopy(c);
    
    // Connection
    if (getSrcActor() != NULL)
    {
        CSDFport *p = ch->getGraph()->getActor(getSrcActor()->getName())
                            ->getPort(getSrcPort()->getName());
        ch->connectSrc(p);
    }
    if (getDstActor() != NULL)
    {
        CSDFport *p = ch->getGraph()->getActor(getDstActor()->getName())
                            ->getPort(getDstPort()->getName());
        ch->connectDst(p);
    }

    // Initial tokens
    ch->setInitialTokens(getInitialTokens());
   
    return ch;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF channel object.
 * The properties of the object are copied.
 */
TimedCSDFchannel *TimedCSDFchannel::createCopy(CSDFcomponent &c) const
{
    TimedCSDFchannel *ch = new TimedCSDFchannel(c);
    
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
 * isUnbounded ()
 * The function returns true if the channel is unbounded. If the size is bounded
 * it returns false.
 */
bool TimedCSDFchannel::isUnbounded() const
{
    if (getBufferSize().sz == -1)
        return true;
    
    return false;
}

/**
 * isControlToken ()
 * The function returns true if the channel is contains control tokens, else it
 * return false. Note, if the token size is unknown, the functions always
 * returns false.
 */
bool TimedCSDFchannel::isControlToken() const
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
bool TimedCSDFchannel::isDataToken() const
{
    if (getTokenSize() > 0)
        return true;
    
    return false;
}

/**
 * print ()
 * Print the channel to the supplied output stream.
 */
ostream &TimedCSDFchannel::print(ostream &out)
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

    CSDFbufferSize bufferSize = getBufferSize();
    if (bufferSize.sz != -1)
        out << "buffer size:    " << bufferSize.sz << endl;
    
    if (getTokenSize() != -1)
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
