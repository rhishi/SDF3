/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   statespace.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 5, 2006
 *
 *  Function        :   State-space based analysis techniques
 *
 *  History         :
 *      05-04-06    :   Initial version.
 *
 * $Id: statespace.h,v 1.1.1.1 2007/10/02 10:59:49 sander Exp $
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

#ifndef CSDF_ANALYSIS_STATESPACE_H_INCLUDED
#define CSDF_ANALYSIS_STATESPACE_H_INCLUDED

#include "../../csdf.h"

typedef unsigned long TBufSize;
typedef double TDtime;

namespace CSDF
{
    /**
     * Distribution
     * A storage distribution.
     */
    typedef struct _Distribution
    {
        TBufSize sz;
        TBufSize *sp;
        TDtime thr;
        bool *dep;
        struct _Distribution *prev;
        struct _Distribution *next;
    } Distribution;

    /**
     * DistributionSet
     * A container for a linked-list of storage distributions with the same
     * size. The container can also be used to build a linked-list of
     * distributions of different size.
     */
    typedef struct _DistributionSet
    {
        TDtime thr;
        TBufSize sz;
        Distribution *distributions;
        struct _DistributionSet *prev;
        struct _DistributionSet *next;
    } DistributionSet;
}

/**
 * stateSpaceThroughputAnalysis ()
 * Compute the throughput of an CSDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal.
 */
double stateSpaceThroughputAnalysis(TimedCSDFgraph *gr);

/**
 * stateSpaceBufferAnalysis ()
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as the throughput bound (thrBound)
 * is reached. To find the complete pareto-space, the throughput bound should
 * be set to DOUBLE_MAX.
 */
CSDF::DistributionSet *stateSpaceBufferAnalysis(TimedCSDFgraph *gr,
        const double thrBound = DBL_MAX, unsigned long long stackSz = 1000, 
        unsigned long long hashSz = 10000);

#endif
