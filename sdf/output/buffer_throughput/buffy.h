/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffy.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   Generate buffy model
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *
 * $Id: buffy.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_OUTPUT_BUFFER_THROUGHPUT_BUFFY_H_INCLUDED
#define SDF_OUTPUT_BUFFER_THROUGHPUT_BUFFY_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * outputSDfasBuffyModel ()
 * Output the SDF graph as a buffy model.
 */
void outputSDFasBuffyModel(TimedSDFgraph *g, ostream &out,
        CString &output_actor, unsigned long long stackSz,
        unsigned long long hashSz, unsigned long long depStackSz);

/**
 * outputSDfasBuffyModelThroughput ()
 * Output the SDF graph as a buffy model to find the maximal throughput
 * for a given distribution size.
 */
void outputSDFasBuffyModelThroughput(TimedSDFgraph *g, ostream &out,
        CString &output_actor, unsigned long long stackSz, 
        unsigned long long hashSz, unsigned long long depStackSz);

/**
 * outputSDFasStateSpaceThroughputModel ()
 * Output the SDF graph as a buffy model to find the maximal
 * throughput for unconstrained buffer sizes and using auto-concurrency.
 */
void outputSDFasStateSpaceThroughputModel(TimedSDFgraph *g, ostream &out,
        unsigned long long stackSz, unsigned long long hashSz);

#endif
