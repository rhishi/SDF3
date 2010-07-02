/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   repetition_vector.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 26, 2007
 *
 *  Function        :   Repetition vector
 *
 *  History         :
 *      26-07-07    :   Initial version.
 *
 * $Id: repetition_vector.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_ALGO_REPETITION_VECTOR_H_INCLUDED
#define SDF_BASE_ALGO_REPETITION_VECTOR_H_INCLUDED

#include "base/base.h"
#include "../untimed/graph.h"

/**
 * Repetition vector
 */
typedef std::vector<int>   RepetitionVector;

/**
 * computeRepetitionVector ()
 * The function computes the repetition vector of an SDFG.
 */
RepetitionVector computeRepetitionVector(SDFgraph *g);

/**
 * isSDFgraphConsistent ()
 * The function checks the consistency of the SDFG and returns true when
 * the graph is consistent, else it returns false.
 */
bool isSDFgraphConsistent(SDFgraph *g);

#endif
