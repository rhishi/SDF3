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
 *  Function        :   CSDF channel
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: channel.h,v 1.1.1.1 2007/10/02 10:59:49 sander Exp $
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

#ifndef CSDF_BASE_CHANNEL_H_INCLUDED
#define CSDF_BASE_CHANNEL_H_INCLUDED

#include "actor.h"

// Forward class definition
class CSDFchannel;
class CSDFgraph;

/**
 * CSDFchannel
 * CSDF channel.
 */
class CSDFchannel : public CSDFcomponent
{
public:

    // Constructor
    CSDFchannel(CSDFcomponent &c);
        
    // Destructor
    virtual ~CSDFchannel();
    
    // Construct
    virtual CSDFchannel *create(CSDFcomponent &c) const;
    virtual CSDFchannel *createCopy(CSDFcomponent &c) const;
    virtual CSDFchannel *clone(CSDFcomponent &c) const;
    void construct(const CNodePtr channelNode);
    
    // Connections
    CSDFport *getSrcPort() const { return src; };
    CSDFport *getDstPort() const { return dst; };
    CSDFactor *getSrcActor() const 
        { return (src == NULL ?  NULL : getSrcPort()->getActor()); };
    CSDFactor *getDstActor() const
        { return (dst == NULL ?  NULL : getDstPort()->getActor()); };
    void connectSrc(CSDFport *p);
    void connectDst(CSDFport *p);
    CSDFport *oppositePort(CSDFport *p) const 
        { return (src == p ? dst : src); };

    // Initial tokens
    uint getInitialTokens() const { return initialTokens; };
    void setInitialTokens(const uint t) { initialTokens = t; };
    
    // Properties
    bool isConnected() const;

    // Graph
    CSDFgraph *getGraph() const { return (CSDFgraph*)getParent(); };

    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, CSDFchannel &c)
        { return c.print(out); };
    
private:
    CSDFport *src;
    CSDFport *dst;
    uint    initialTokens;
};

typedef list<CSDFchannel*>            	CSDFchannels;
typedef CSDFchannels::iterator          CSDFchannelsIter;
typedef CSDFchannels::const_iterator    CSDFchannelsCIter;

#endif
