/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   timed_graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 19, 2005
 *
 *  Function        :   Timed CSDF graph
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: graph.h,v 1.2 2008/03/22 14:24:21 sander Exp $
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

#ifndef CSDF_BASE_TIMED_GRAPH_H_INCLUDED
#define CSDF_BASE_TIMED_GRAPH_H_INCLUDED

#include "channel.h"
#include "actor.h"

/**
 * TimedCSDFgraph
 * Container for timed CSDF graph.
 */
class TimedCSDFgraph : public CSDFgraph
{
public:

    // Constructor
    TimedCSDFgraph(CSDFcomponent &c);
    
    // Destructor
    ~TimedCSDFgraph();

    // Construct
    TimedCSDFgraph *create(CSDFcomponent &c) const;
    TimedCSDFgraph *createCopy(CSDFcomponent &c) const;
    TimedCSDFgraph *clone(CSDFcomponent &c) const;
    TimedCSDFgraph *clone() const;
    
    // Actors
    TimedCSDFactor *createActor();
    TimedCSDFactor *createActor(CSDFcomponent &c);
    
    // Channels
    TimedCSDFchannel *createChannel(CSDFcomponent &c);
    TimedCSDFchannel *createChannel(CSDFactor *src, CSDFrate rateSrc, 
            CSDFactor *dst, CSDFrate rateDst);
    
    // Time constraints
    CSDFthroughput getThroughputConstraint() const 
        { return throughputConstraint; };
    void setThroughputConstraint(const CSDFthroughput t) 
        { throughputConstraint = t; };

    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, TimedCSDFgraph &g)
        { return g.print(out); };

private:
    // Throughput
    CSDFthroughput throughputConstraint;
};

/**
 * constructTimedCSDFgraph ()
 * Construct a timed CSDF graph.
 */
TimedCSDFgraph *constructTimedCSDFgraph(const CNodePtr csdfNode,
        const CNodePtr csdfPropertiesNode);

#endif
