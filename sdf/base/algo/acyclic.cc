/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   acyclic.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 29, 2005
 *
 *  Function        :   Check that graph is acyclic
 *
 *  History         :
 *      29-07-05    :   Initial version.
 *
 * $Id: acyclic.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "acyclic.h"
#include "components.h"

/**
 * isAcyclic ()
 * The function returns true of the SDF graph is acyclic. Else it returns
 * false.
 */
bool isAcyclic(SDFgraph *g)
{
    // Split graph in strongly connected components
    SDFgraphComponents components = stronglyConnectedComponents(g);

    // Each strongly connected component must contain exactly one actor
    for (SDFgraphComponentsIter iter = components.begin();
            iter != components.end(); iter++)
    {
        SDFgraphComponent &component = *iter;
        
        if (component.size() != 1)
            return false;
    }
    
    return true;
}
