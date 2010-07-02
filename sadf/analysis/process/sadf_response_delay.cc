/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_response_delay.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of response delay
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

#include "sadf_response_delay.h"

// Functions to analyse response delay

CSize SADF_Analyse_ResponseDelay(SADF_Graph* Graph, CId ProcessType, CId ProcessID, CDouble& Expected, CDouble& Minimum, CDouble& Maximum) {

	// Check whether graph satisfied required properties

	if (!SADF_Verify_SingleComponent(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not consist of a single component.");

	if (!SADF_Verify_Timed(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not timed.");
	
	if (!SADF_Verify_Boundedness(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not bounded.");

	// Construct TPS

	SADF_TPS* TPS = new SADF_TPS(Graph);
	SADF_Configuration* InitialConfiguration = TPS->getInitialConfiguration();

	SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP_InterFiringLatency(Graph, TPS, InitialConfiguration, ProcessType, ProcessID);

	while (!NewConfigurations.empty())
		NewConfigurations.pop_front();

	// Compute Results

	Expected = 0;
	Minimum = SADF_MAX_DOUBLE;
	Maximum = 0;

	for (list<SADF_Transition*>::iterator i = InitialConfiguration->getTransitions().begin(); i != InitialConfiguration->getTransitions().end(); i++) {
		
		Expected += (*i)->getProbability() * (*i)->getTimeSample();
		
		if ((*i)->getTimeSample() < Minimum)
			Minimum = (*i)->getTimeSample();

		if ((*i)->getTimeSample() > Maximum)
			Maximum = (*i)->getTimeSample();
	}

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

	delete TPS;
	
	return NumberOfConfigurations;
}
