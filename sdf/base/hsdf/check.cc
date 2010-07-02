/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   check.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 22, 2005
 *
 *  Function        :   Check that SDF graph is a HSDF graph
 *
 *  History         :
 *      22-07-05    :   Initial version.
 *
 * $Id: check.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "check.h"

/**
 * isHSDFgraph ()
 * The function checks wether a SDF graph is a HSDF graph.
 */
bool isHSDFgraph(SDFgraph *g)
{
    // Check all actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        // Check all ports
        for (SDFportsIter iterPorts = a->portsBegin(); 
                iterPorts != a->portsEnd(); iterPorts++)
        {
            SDFport *p = *iterPorts;
            
            if (p->getRate() != 1)
                return false;    
        }
    }
    
    return true;
}
