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
 * $Id: actor.h,v 1.3 2008/03/22 14:24:21 sander Exp $
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

#ifndef CSDF_BASE_UNTIMED_ACTOR_H_INCLUDED
#define CSDF_BASE_UNTIMED_ACTOR_H_INCLUDED

#include "port.h"

/**
 * CSDFactor
 * Actor in CSDF graph
 */
class CSDFactor : public CSDFcomponent
{
public:

    // Constructor
    CSDFactor(CSDFcomponent &c);
    
    // Desctructor
    virtual ~CSDFactor();

    // Construct
    virtual CSDFactor *create(CSDFcomponent &c) const;
    virtual CSDFactor *createCopy(CSDFcomponent &c) const;
    virtual CSDFactor *clone(CSDFcomponent &c) const;
    void construct(const CNodePtr actorNode);
        
    // Type
    CString getType() const { return type; };
    void setType(const CString &t) { type = t; };
    
    // Ports
    CSDFport *getPort(const CId id);
    CSDFport *getPort(const CString &name);
    CSDFports &getPorts() { return ports; };
    uint nrPorts() const { return ports.size(); };
    CSDFportsIter portsBegin() { return ports.begin(); };
    CSDFportsIter portsEnd() { return ports.end(); };
    CSDFportsCIter portsBegin() const { return ports.begin(); };
    CSDFportsCIter portsEnd() const { return ports.end(); };
    void addPort(CSDFport *p);
    void removePort(const CString &name);
    void removePorts();
    virtual CSDFport *createPort(CSDFcomponent &c);
    CSDFport *createPort(const CString &type, const CSDFrate rate);
    
    // Sequence
    uint sequenceLength() const {
        if (ports.empty())
            return 0;
        return ports.front()->getRate().size();
    };
    
    // Properties
    bool isConnected() const;
    
    // Graph
    CSDFgraph *getGraph() const { return (CSDFgraph*)getParent(); };

    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, CSDFactor &a)
        { return a.print(out); };
    
private:
    // Type
    CString type;
        
    // Ports
    CSDFports ports;
};

typedef list<CSDFactor*>            CSDFactors;
typedef CSDFactors::iterator        CSDFactorsIter;
typedef CSDFactors::const_iterator  CSDFactorsCIter;

#endif
