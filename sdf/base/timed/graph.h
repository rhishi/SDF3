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
 *  Function        :   Timed SDF graph
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: graph.h,v 1.2 2008/03/17 14:07:37 sander Exp $
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

#ifndef SDF_BASE_TIMED_GRAPH_H_INCLUDED
#define SDF_BASE_TIMED_GRAPH_H_INCLUDED

#include "channel.h"
#include "actor.h"

/**
 * TimedSDFgraph
 * Container for timed SDF graph.
 */
class TimedSDFgraph : public SDFgraph
{
public:

    // Constructor
    TimedSDFgraph(SDFcomponent &c);
    TimedSDFgraph();
    
    // Destructor
    ~TimedSDFgraph();

    // Construct
    TimedSDFgraph *create(SDFcomponent &c) const;
    TimedSDFgraph *createCopy(SDFcomponent &c) const;
    TimedSDFgraph *clone(SDFcomponent &c) const;
    TimedSDFgraph *clone() const;
    void construct(const CNodePtr sdfNode, const CNodePtr sdfPropertiesNode);

    // Properties
    void setProperties(const CNodePtr propertiesNode);

    // Actors
    TimedSDFactor *createActor();
    TimedSDFactor *createActor(SDFcomponent &c);
    
    // Channels
    TimedSDFchannel *createChannel(SDFcomponent &c);
    TimedSDFchannel *createChannel(SDFactor *src, SDFrate rateSrc, 
            SDFactor *dst, SDFrate rateDst, uint initialTokens);
    
    // Time constraints
    SDFthroughput getThroughputConstraint() const 
        { return throughputConstraint; };
    void setThroughputConstraint(const SDFthroughput t) 
        { throughputConstraint = t; };
    
    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, TimedSDFgraph &g)
        { return g.print(out); };

private:
    // Throughput
    SDFthroughput throughputConstraint;
};

#endif
