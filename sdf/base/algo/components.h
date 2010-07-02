/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   components.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Strongly connected components
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: components.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_ALGO_COMPONENTS_H_INCLUDED
#define SDF_BASE_ALGO_COMPONENTS_H_INCLUDED

#include "base/base.h"
#include "../untimed/graph.h"

/**
 * SDFgraphComponent(s)
 * SDF graph strongly connected component(s)
 */
typedef SDFactors                           SDFgraphComponent;
typedef list<SDFgraphComponent>             SDFgraphComponents;
typedef SDFgraphComponents::iterator        SDFgraphComponentsIter;
typedef SDFgraphComponents::const_iterator  SDFgraphComponentsCIter;

/**
 * stronglyConnectedComponents ()
 * The function determines the strongly connected components in the graph.
 */
SDFgraphComponents stronglyConnectedComponents(SDFgraph *g);

/**
 * actorInComponent ()
 * The function check wether an actor is in a component. If so, the
 * function returns true. Else it returns false.
 */
bool actorInComponent(SDFactor *a, SDFgraphComponent &component);

/**
 * isStronglyConnectedGraph ()
 * The function checks that the graph is a strongly connnected component.
 */
bool isStronglyConnectedGraph(SDFgraph *g);

/**
 * componentToSDFgraph ()
 * The function returns an SDF graph containing all actors and channels inside
 * the component.
 */
SDFgraph *componentToSDFgraph(SDFgraphComponent &component);

#endif
