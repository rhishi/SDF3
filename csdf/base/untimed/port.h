/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   port.h
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
 * $Id: port.h,v 1.3 2008/03/22 14:24:21 sander Exp $
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

#ifndef CSDF_BASE_PORT_H_INCLUDED
#define CSDF_BASE_PORT_H_INCLUDED

#include "component.h"
#include "rate.h"

// Forward class definitions
class CSDFport;
class CSDFactor;
class CSDFchannel;
class CSDFgraph;

/**
 * CSDFport
 * Port on an CSDF actor.
 */
class CSDFport : public CSDFcomponent
{
public:
    enum CSDFportType { In, Out };

    // Constructor
    CSDFport(CSDFcomponent &c);
    
    // Destructor
    virtual ~CSDFport();

    // Construct
    virtual CSDFport *create(CSDFcomponent &c) const;
    virtual CSDFport *createCopy(CSDFcomponent &c) const;
    virtual CSDFport *clone(CSDFcomponent &c) const;
    void construct(const CNodePtr portNode);
    
    // Type
    CSDFportType getType() const { return type; };
    CString getTypeAsString() const { 
        if (type == In)
            return "in";
        else
            return "out";
    };
    void setType(const CSDFportType t) { type = t; };
    void setType(const CString &t) {
        if (t == "in")
            type = In;
        else if (t == "out")
            type = Out;
    };
    
    // Rate
    CSDFrate &getRate() { return rate; };
    void setRate(const CSDFrate &r) { rate = r; };
        
    // Channel
    CSDFchannel *getChannel() const { return channel; };
    void connectToChannel(CSDFchannel *c);
    
    // Actor
    CSDFactor *getActor() const { return (CSDFactor*)getParent(); };
    
    // Properties
    bool isConnected() const;
        
    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, CSDFport &p)
        { return p.print(out); };
    
private:
    CSDFportType type;
    CSDFrate     rate;
    CSDFchannel  *channel;
};

typedef list<CSDFport*>             CSDFports;
typedef CSDFports::iterator         CSDFportsIter;
typedef CSDFports::const_iterator   CSDFportsCIter;

#endif

