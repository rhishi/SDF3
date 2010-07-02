/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_buffer_occupancy.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of buffer occupancy
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

#ifndef SADF_BUFFER_OCCUPANCY_H_INCLUDED
#define SADF_BUFFER_OCCUPANCY_H_INCLUDED

// Include required verification algorithms

#include "../../verification/simple/sadf_simple.h"
#include "../../verification/simple/sadf_ergodic.h"
#include "../../verification/boundedness/sadf_boundedness.h"

// Include scheduler and type definitions

#include "../schedulers/sadf_asap_reduced.h"

// Functions to analyse buffer occupancy (returns number of stored states)

CSize SADF_Analyse_LongRunBufferOccupancy(SADF_Graph* Graph, CId ChannelType, CId ChannelID, CDouble& Average, CDouble& Variance);
CSize SADF_Analyse_MaximumBufferOccupancy(SADF_Graph* Graph, CId ChannelType, CId ChannelID, CDouble& Maximum);
//CSize SADF_Analyse_BufferSize(SADF_Graph* Graph, CId ChannelType, CId ChannelID, CDouble& Result);

#endif
