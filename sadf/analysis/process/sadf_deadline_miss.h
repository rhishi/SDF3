/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_deadline_miss.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of deadline miss probability
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#ifndef SADF_DEADLINE_MISS_H_INCLUDED
#define SADF_DEADLINE_MISS_H_INCLUDED

// Include required verification algorithms

#include "../../verification/simple/sadf_simple.h"
#include "../../verification/simple/sadf_ergodic.h"
#include "../../verification/boundedness/sadf_boundedness.h"

// Include scheduler and type definitions

#include "../schedulers/sadf_asap_reduced.h"

// Functions to analyse the deadline miss probability (returns number of stored states)

CSize SADF_Analyse_PeriodicDeadlineMissProbability(SADF_Graph* Graph, CId ProcessType, CId ProcessID, CDouble& Deadline, CDouble& Result);
CSize SADF_Analyse_ResponseDeadlineMissProbability(SADF_Graph* Graph, CId ProcessType, CId ProcessID, CDouble& Deadline, CDouble& Result);

#endif
