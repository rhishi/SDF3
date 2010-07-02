/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_selftimed.cc
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

#include "sadf_asap.h"

SADF_ListOfConfigurations SADF_ProgressTPS_ASAP(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source) {

	SADF_ListOfConfigurations NewConfigurations;
	
	bool ActionPossible = false;
	
	// Perform all possible control actions
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToFire()) {
			
			ActionPossible = true;
				
			SADF_ListOfConfigurations Temp = Source->getKernelStatus(i)->control(Graph, TPS, true);
				
			while (!Temp.empty()) {
				NewConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}
	
	// Perform all possible detect actions

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToFire()) {

			ActionPossible = true;

			SADF_ListOfConfigurations Temp = Source->getDetectorStatus(i)->detect(Graph, TPS, true);
			
			while (!Temp.empty()) {
				NewConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}		
	
	// Perform all possible start actions

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToStart()) {

			ActionPossible = true;

			SADF_ListOfConfigurations Temp = Source->getKernelStatus(i)->start(Graph, TPS, true);
			
			while (!Temp.empty()) {
				NewConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}
	
	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToStart()) {

			ActionPossible = true;

			SADF_ListOfConfigurations Temp = Source->getDetectorStatus(i)->start(Graph, TPS, true);
			
			while (!Temp.empty()) {
				NewConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}

	// Perform all possible end actions
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToEnd()) {

			ActionPossible = true;

			SADF_ListOfConfigurations Temp = Source->getKernelStatus(i)->end(Graph, TPS, true);
			
			while (!Temp.empty()) {
				NewConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}
		
	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToEnd()) {

			ActionPossible = true;

			SADF_ListOfConfigurations Temp = Source->getDetectorStatus(i)->end(Graph, TPS, true);
			
			while (!Temp.empty()) {
				NewConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}

	// Check whether a time step should be performed istead of any actions
	
	if (!ActionPossible) {

		if (Source->getType() == SADF_TIME_STEP || Source->getMinimalRemainingExecutionTime() == SADF_MAX_DOUBLE)
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has a deadlock.");

		SADF_Configuration* NewConfiguration = Source->time(Graph, TPS, true);
			
		if (NewConfiguration != NULL)
			NewConfigurations.push_front(NewConfiguration);
	}

	return NewConfigurations;
}

SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_Resolved(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source) {

	SADF_ListOfConfigurations NewConfigurations;
	
	bool ActionPossible = false;
	
	// Perform a control action
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Graph->getKernel(i)->hasControlChannels())
			if (Source->getKernelStatus(i)->isReadyToFire()) {
				ActionPossible = true;
				NewConfigurations = Source->getKernelStatus(i)->control(Graph, TPS, true);
			}
	
	// Perform a detect action

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			NewConfigurations = Source->getDetectorStatus(i)->detect(Graph, TPS, true);
		}		
	
	// Perform a start action

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			NewConfigurations = Source->getKernelStatus(i)->start(Graph, TPS, true);
		}
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			NewConfigurations = Source->getDetectorStatus(i)->start(Graph, TPS, true);
		}

	// Perform an end action
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToEnd()) {
			ActionPossible = true;
			NewConfigurations = Source->getKernelStatus(i)->end(Graph, TPS, true);
		}
		
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToEnd()) {
			ActionPossible = true;
			NewConfigurations = Source->getDetectorStatus(i)->end(Graph, TPS, true);
		}

	// Check whether a time step should be performed istead of an action
	
	if (!ActionPossible) {

		if (Source->getType() == SADF_TIME_STEP || Source->getMinimalRemainingExecutionTime() == SADF_MAX_DOUBLE)
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has a deadlock.");

		SADF_Configuration* NewConfiguration = Source->time(Graph, TPS, true);
			
		if (NewConfiguration != NULL)
			NewConfigurations.push_front(NewConfiguration);
	}

	return NewConfigurations;
}
