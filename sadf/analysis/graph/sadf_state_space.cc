/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_state_space.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of size of state space
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

#include "sadf_state_space.h"

// Functions to analyse size of state space

void SADF_ConstructTPS_ASAP(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source) {

	SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP(Graph, TPS, Source);
	
	while (!NewConfigurations.empty()) {
		SADF_ConstructTPS_ASAP(Graph, TPS, NewConfigurations.front());
		NewConfigurations.pop_front();
	}
}

CSize SADF_Analyse_NumberOfStates(SADF_Graph* Graph) {

	// Check whether graph satisfied required properties

	if (!SADF_Verify_Boundedness(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not bounded.");

	// Construct TPS

	SADF_TPS* TPS = new SADF_TPS(Graph);
	TPS->addConfiguration(TPS->getInitialConfiguration());
	
	SADF_ConstructTPS_ASAP(Graph, TPS, TPS->getInitialConfiguration());

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

	delete TPS;

	return NumberOfConfigurations + 1;  // The additional one is the initial configuration entered without performing any action
}

void SADF_ConstructTPS_ASAP_Resolved(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source) {

	SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP_Resolved(Graph, TPS, Source);
	
	while (!NewConfigurations.empty()) {
		SADF_ConstructTPS_ASAP_Resolved(Graph, TPS, NewConfigurations.front());
		NewConfigurations.pop_front();
	}
}

CSize SADF_Analyse_NumberOfStates_Resolved(SADF_Graph* Graph) {

	// Check whether graph satisfied required properties

	if (!SADF_Verify_Boundedness(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not bounded.");

	// Construct TPS

	SADF_TPS* TPS = new SADF_TPS(Graph);
	TPS->addConfiguration(TPS->getInitialConfiguration());
	SADF_ConstructTPS_ASAP_Resolved(Graph, TPS, TPS->getInitialConfiguration());

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

	delete TPS;

	return NumberOfConfigurations + 1;  // The additional one is the initial configuration entered without performing any action
}
