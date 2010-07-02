/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mcm.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Maximum cycle mean of a HSDF graph
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: mcm.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_ANALYSIS_MCM_MCM_H_INCLUDED
#define SDF_ANALYSIS_MCM_MCM_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * maximumCycleMeanCycles ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * the cycle-based definition of the MCM.
 */
CFraction maximumCycleMeanCycles(TimedSDFgraph *g);

/**
 * maximumCycleMeanKarp ()
 * The function computes the maximum cycle mean of a HSDF graph using Karp's
 * algorithm.
 */
CFraction maximumCycleMeanKarp(TimedSDFgraph *g);

/**
 * maximumCycleMeanHoward ()
 * The function computes the maximum cycle mean of a HSDF graph using Howard's
 * algorithm.
 */
CFraction maximumCycleMeanHoward(TimedSDFgraph *g);

/**
 * maximumCycleMeanDasdanGupta ()
 * The function computes the maximum cycle mean of a HSDF graph using
 * Dasan-Gupta's algorithm.
 */
CFraction maximumCycleMeanDasdanGupta(TimedSDFgraph *g);

/**
 * maximumCycleMeanYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of a HSDF graph using 
 * Young-Tarjan-Orlin's algorithm.
 */
CFraction maximumCycleYoungTarjanOrlin(TimedSDFgraph *g, bool mcmFormulation);

#endif
