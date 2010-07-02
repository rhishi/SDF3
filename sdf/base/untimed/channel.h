/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.h
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
 * $Id: channel.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_UNTIMED_CHANNEL_H_INCLUDED
#define SDF_BASE_UNTIMED_CHANNEL_H_INCLUDED

#include "actor.h"

// Forward class definition
class SDFchannel;
class SDFgraph;

/**
 * SDFchannel
 * SDF channel.
 */
class SDFchannel : public SDFcomponent
{
public:

    // Constructor
    SDFchannel(SDFcomponent &c);
        
    // Destructor
    virtual ~SDFchannel();
    
    // Construct
    virtual SDFchannel *create(SDFcomponent &c) const;
    virtual SDFchannel *createCopy(SDFcomponent &c) const;
    virtual SDFchannel *clone(SDFcomponent &c) const;
    void construct(const CNodePtr channelNode);
    
    // Connections
    SDFport *getSrcPort() const { return src; };
    SDFport *getDstPort() const { return dst; };
    SDFactor *getSrcActor() const 
        { return (src == NULL ?  NULL : getSrcPort()->getActor()); };
    SDFactor *getDstActor() const
        { return (dst == NULL ?  NULL : getDstPort()->getActor()); };
    void connectSrc(SDFport *p);
    void connectDst(SDFport *p);
    SDFport *oppositePort(SDFport *p) const { return (src == p ? dst : src); };

    // Initial tokens
    uint getInitialTokens() const { return initialTokens; };
    void setInitialTokens(const uint t) { initialTokens = t; };
    
    // Properties
    bool isConnected() const;

    // Graph
    SDFgraph *getGraph() const { return (SDFgraph*)getParent(); };

    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, SDFchannel &c)
        { return c.print(out); };
    
private:
    SDFport *src;
    SDFport *dst;
    uint    initialTokens;
};

typedef list<SDFchannel*>            	SDFchannels;
typedef SDFchannels::iterator           SDFchannelsIter;
typedef SDFchannels::const_iterator     SDFchannelsCIter;

#endif
