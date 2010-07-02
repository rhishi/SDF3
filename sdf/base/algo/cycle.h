/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   cycle.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Find all simple cycles in a graph
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: cycle.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_ALGO_CYCLE_H_INCLUDED
#define SDF_BASE_ALGO_CYCLE_H_INCLUDED

#include "../untimed/graph.h"

/**
 * sdfGraphCmpCycle
 * Compare two cycles of SDF actors to see wether they contain the same cycle.
 */
struct sdfGraphCmpCycle
{
  bool operator()(const SDFactors c1, const SDFactors c2) const
  {
    SDFactorsCIter c1Iter = c1.begin();
    SDFactorsCIter c2Iter = c2.begin();
    SDFactorsCIter c1IterMinId;
    SDFactorsCIter c2IterMinId;
    CId c1MinId = UINT_MAX;
    CId c2MinId = UINT_MAX;

    // Number of actors in c1 less than in c2?
    if (c1.size() < c2.size())
        return true;

    // Number of actors in c1 more than in c2?
    if (c1.size() > c2.size())
        return false;

    // Same number of actors in c1 and c2. Compare actors in cycles, starting
    // from actor which lowest id in c1. Id of the first actor which differs in
    // both cycles determines ordening relation.
    
    // Find actor with smallest id in c1
    while (c1Iter != c1.end())
    {
        if ((*c1Iter)->getId() < c1MinId)
        {
            c1IterMinId = c1Iter;
            c1MinId = (*c1Iter)->getId();
        }
        
        // Next actor in c1
        c1Iter++;
    }

    // Find actor with smallest id in c2
    while (c2Iter != c2.end())
    {
        if ((*c2Iter)->getId() < c2MinId)
        {
            c2IterMinId = c2Iter;
            c2MinId = (*c2Iter)->getId();
        }
        
        // Next actor in c2
        c2Iter++;
    }

    // Actor with smallest id not similar in both cycles?
    if (c1MinId != c2MinId)
    {
        if (c1MinId < c2MinId)
            return true;
        else
            return false;
    }

    // Actor with smallest id is similar, what about other actors
    c1Iter = c1IterMinId;
    c2Iter = c2IterMinId;
    do
    {
        // Next actor in cycle
        c1Iter++;
        c2Iter++;
        if (c1Iter == c1.end())
            c1Iter = c1.begin();
        if (c2Iter == c2.end())
            c2Iter = c2.begin();
        
        // Different id?
        if ((*c1Iter)->getId() < (*c2Iter)->getId())
            return true;
        else if ((*c1Iter)->getId() > (*c2Iter)->getId())
            return false;
    } while ((*c1Iter)->getId() != c1MinId);

    return false;
  }
};

/**
 * SDFgraphComponent(s)
 * SDF graph strongly connected component(s)
 */
typedef SDFactors                               SDFgraphCycle;
typedef set<SDFgraphCycle, sdfGraphCmpCycle>    SDFgraphCycles;
typedef SDFgraphCycles::iterator                SDFgraphCyclesIter;
typedef SDFgraphCycles::const_iterator          SDFgraphCyclesCIter;

/**
 * findSimpleCycles ()
 * The function performs a depth first search on the graph to discover all
 * simple cycles in the graph
 */
SDFgraphCycles findSimpleCycles(SDFgraph *g, bool transpose = false);

/**
 * findSimpleCycle ()
 * The function performs a depth first search on the graph to discover a
 * simple cycles in the graph. The first cycle found is returned.
 */
SDFgraphCycle findSimpleCycle(SDFgraph *g, bool transpose = false);

#endif
