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
 * $Id: components.h,v 1.1.1.1 2007/10/02 10:59:49 sander Exp $
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

#ifndef CSDF_BASE_ALGO_COMPONENTS_H_INCLUDED
#define CSDF_BASE_ALGO_COMPONENTS_H_INCLUDED

#include "base/base.h"
#include "../../csdf.h"

/**
 * CSDFgraphComponent(s)
 * CSDF graph strongly connected component(s)
 */
typedef CSDFactors                          CSDFgraphComponent;
typedef list<CSDFgraphComponent>            CSDFgraphComponents;
typedef CSDFgraphComponents::iterator       CSDFgraphComponentsIter;
typedef CSDFgraphComponents::const_iterator CSDFgraphComponentsCIter;

/**
 * stronglyConnectedComponents ()
 * The function determines the strongly connected components in the graph.
 */
CSDFgraphComponents stronglyConnectedComponents(CSDFgraph *g);

/**
 * actorInComponent ()
 * The function check wether an actor is in a component. If so, the
 * function returns true. Else it returns false.
 */
bool actorInComponent(CSDFactor *a, CSDFgraphComponent &component);

/**
 * isStronglyConnectedGraph ()
 * The function checks that the graph is a strongly connnected component.
 */
bool isStronglyConnectedGraph(CSDFgraph *g);

/**
 * componentToSDFgraph ()
 * The function returns an CSDF graph containing all actors and channels inside
 * the component.
 */
CSDFgraph *componentToCSDFgraph(CSDFgraphComponent &component);

#endif
