/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_selftimed.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Construction of TPS (Markov Decision Process or Markov Chain) using ASAP / Self-Timed Sceduling
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

#ifndef SADF_ASAP_H_INCLUDED
#define SADF_ASAP_H_INCLUDED

// Include type definitions

#include "../../base/tps/sadf_tps.h"

// Functions to construct the TPS

SADF_ListOfConfigurations SADF_ProgressTPS_ASAP(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source);
SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_Resolved(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source);

#endif
