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
 * $Id: throughput.h,v 1.2 2008/09/18 07:35:13 sander Exp $
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

#ifndef SDF_ANALYSIS_THROUGHPUT_THROUGHPUT_H_INCLUDED
#define SDF_ANALYSIS_THROUGHPUT_THROUGHPUT_H_INCLUDED

/**
 * Throughput analysis
 * Compute the throughput of an SDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal.
 */
#include "selftimed_throughput.h"

/**
 * Binding-aware throughput analysis
 * Computes the throughput of an SDFG mapped to an architecture platform. It
 * is assumed that the platform uses a TDMA resource arbitration mechanism on
 * the processors. An Actor is scheduled on a processor if its input tokens are
 * available and the static-order schedule indicates that the actor is allowed
 * to fire. Actors which are not mapped to a processor, only wait for their
 * input tokens.
 */
#include "tdma_schedule.h"

/**
 * Deadlock analysis
 * Check that an SDFG is deadlock free (i.e. executing each actor as often as
 * indicated by the repetition vector returns the graph to its initial state).
 */
#include "deadlock.h"

/**
 * Throughput analysis with a static-periodic schedule
 * Comput the throughput of the graph under the given storage constraints and 
 * using the supplied static-periodic schedule. The graph execution follows the 
 * operational semantics of Ning and Gao.
 */
#include "static_periodic_ning_gao.h"

#endif

