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
 *  Function        :   SDF port
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: port.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_UNTIMED_PORT_H_INCLUDED
#define SDF_BASE_UNTIMED_PORT_H_INCLUDED

#include "component.h"

// Rate on ports of SDF graphs
typedef uint SDFrate;

// Forward class definition
class SDFport;
class SDFactor;
class SDFchannel;
class SDFgraph;

/**
 * SDFport
 * Port on an SDF actor.
 */
class SDFport : public SDFcomponent
{
public:
    enum SDFportType { In, Out, Undef };

    // Constructor
    SDFport(SDFcomponent &c);
    
    // Destructor
    virtual ~SDFport();

    // Construct
    virtual SDFport *create(SDFcomponent &c) const;
    virtual SDFport *createCopy(SDFcomponent &c) const;
    virtual SDFport *clone(SDFcomponent &c) const;
    void construct(const CNodePtr portNode);
    
    // Type
    SDFportType getType() const { return type; };
    CString getTypeAsString() const { 
        if (type == In)
            return "in";
        else if (type == Out)
            return "out";
        else
            return "undef";
    };
    void setType(const SDFportType t) { type = t; };
    void setType(const CString &t) {
        if (t == CString("in"))
            type = In;
        else if (t == CString("out"))
            type = Out;
        else
            type = Undef;
    };
    
    // Rate
    SDFrate getRate() const { return rate; };
    void setRate(const SDFrate r) { rate = r; };
        
    // Channel
    SDFchannel *getChannel() const { return channel; };
    void connectToChannel(SDFchannel *c);
    
    // Actor
    SDFactor *getActor() const { return (SDFactor*)getParent(); };
    
    // Properties
    bool isConnected() const;
        
    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, SDFport &p)
        { return p.print(out); };
    
private:
    SDFportType type;
    SDFrate     rate;
    SDFchannel  *channel;
};

typedef list<SDFport*>              SDFports;
typedef SDFports::iterator          SDFportsIter;
typedef SDFports::const_iterator    SDFportsCIter;

#endif
