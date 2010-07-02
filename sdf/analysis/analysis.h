/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   analysis.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 3, 2008
 *
 *  Function        :   SDF graph analysis algorithms
 *
 *  History         :
 *      03-03-08    :   Initial version.
 *
 * $Id: analysis.h,v 1.1 2008/03/06 10:49:41 sander Exp $
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

#ifndef SDF_ANALYSIS_ANALYSIS_H_INCLUDED
#define SDF_ANALYSIS_ANALYSIS_H_INCLUDED

// Buffer sizing
#include "buffersizing/buffersizing.h"

// Dependency graph
#include "dependency_graph/dependency_graph.h"

// Latency
#include "latency/latency.h"

// Maximum Cycle Mean
#include "mcm/mcm.h"

// Throughput
#include "throughput/throughput.h"

// Token communication
#include "token_communication/comm_trace.h"

#endif

