/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_selftimed_reduced.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Construction of Reduced TPS (Markov Chain) using ASAP / Self-Timed Sceduling
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

#ifndef SADF_ASAP_REDUCED_H_INCLUDED
#define SADF_ASAP_REDUCED_H_INCLUDED

// Include type definitions

#include "../../base/tps/sadf_tps.h"

// Functions to construct the TPS

SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_InterFiringLatency(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId ProcessType, CId ProcessID);
SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_BufferOccupancy(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId SourceProcessType, CId SourceProcessID, CId DestinationProcessType, CId DestinationProcessID);
SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_BufferOccupancy(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId SourceProcessType, CId SourceProcessID);
//SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_BufferSize(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId ChannelType, CId ChannelID, CId SourceProcessType, CId SourceProcessID);

#endif
