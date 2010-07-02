/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   generate.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2005
 *
 *  Function        :   Generate SDF graphs
 *
 *  History         :
 *      14-07-05    :   Initial version.
 *
 * $Id: generate.h,v 1.2 2008/09/18 07:38:21 sander Exp $
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

#ifndef CSDF_GENERATE_GENERATE_H_INCLUDED
#define CSDF_GENERATE_GENERATE_H_INCLUDED

#include "sdf/sdf.h"
#include "../csdf.h"

/**
 * generateCSDFgraph ()
 * Generate a random CSDF graph.
 */
TimedCSDFgraph *generateCSDFgraph(const uint period,
        const uint nrActors, const double avgInDegree, 
        const double varInDegree, const double minInDegree,
        const double maxInDegree, const double avgOutDegree,
        const double varOutDegree, const double minOutDegree, 
        const double maxOutDegree, const double avgRate,
        const double varRate, const double minRate, const double maxRate, 
        const bool acyclic, const bool stronglyConnected, 
        const double initialTokenProp, const uint repetitionVectorSum,
        const bool execTime, const uint nrProcTypes, 
        const double mapChance, const double avgExecTime,
        const double varExecTime, const double minExecTime, 
        const double maxExecTime, const bool stateSize,
        const double avgStateSize, const double varStateSize, 
        const double minStateSize, const double maxStateSize,
        const bool tokenSize, const double avgTokenSize, 
        const double varTokenSize, const double minTokenSize, 
        const double maxTokenSize, const bool throughputConstraint,
        const uint autoConcurrencyDegree, const double throughputScaleFactor,
        const bool bandwidthRequirement, const double avgBandwidth,
        const double varBandwidth, const double minBandwidth,
        const double maxBandwidth, const bool bufferSize,
        const bool latencyRequirement, const double avgLatency,
        const double varLatency, const double minLatency,
        const double maxLatency, const bool multigraph, 
        const bool integerMCM);

#endif
