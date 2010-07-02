/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_selftimed_reduced.cc
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

#include "sadf_asap_reduced.h"

SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_InterFiringLatency(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId ProcessType, CId ProcessID) {

	// Non-determinism resolved and policy assumed to have no effect on peformance result

	bool ActionPossible = false;

	SADF_ListOfConfigurations NewRelevantConfigurations;
	SADF_ListOfConfigurations NewIrrelevantConfigurations;

	// Perform end action - relevant cases

	if (!ActionPossible && ProcessType == SADF_KERNEL)
		if (Source->getKernelStatus(ProcessID)->isReadyToEnd()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getKernelStatus(ProcessID)->end(Graph, TPS, true);
		}
	
	if (!ActionPossible && ProcessType == SADF_DETECTOR)
		if (Source->getDetectorStatus(ProcessID)->isReadyToEnd()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getDetectorStatus(ProcessID)->end(Graph, TPS, true);
		}				

	// Perform control action - resulting configuration is irrelevant

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->control(Graph, TPS, false);
		}
			
	// Perform detect action - resulting configurations are irrelevant

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->detect(Graph, TPS, false);
		}		

	// Perform start action - resulting configurations are irrelevant	

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->start(Graph, TPS, false);
		}
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->start(Graph, TPS, false);
		}

	// Perform end action - irrelevant cases
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToEnd()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->end(Graph, TPS, false);
		}
		
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToEnd()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->end(Graph, TPS, false);
		}

	// Perform time step - resulting configuration is considered irrelevant
	
	if (!ActionPossible) {

		if (Source->getType() == SADF_TIME_STEP || Source->getMinimalRemainingExecutionTime() == SADF_MAX_DOUBLE)
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has a deadlock.");
			
		NewIrrelevantConfigurations.push_front(Source->time(Graph, TPS, false));
	}
	
	// Recursively progress for all newly created irrelevant configurations until all reachable relevant configurations are found and fix transitions

	for (SADF_ListOfConfigurations::iterator i = NewIrrelevantConfigurations.begin(); i != NewIrrelevantConfigurations.end(); i++) {

		SADF_ListOfConfigurations OtherNewRelevantConfigurations = SADF_ProgressTPS_ASAP_InterFiringLatency(Graph, TPS, *i, ProcessType, ProcessID);

		while (!OtherNewRelevantConfigurations.empty()) {
			NewRelevantConfigurations.push_front(OtherNewRelevantConfigurations.front());
			OtherNewRelevantConfigurations.pop_front();
		}
	}

	// Update transitions and fix time samples

	for (SADF_ListOfConfigurations::iterator i = NewIrrelevantConfigurations.begin(); i != NewIrrelevantConfigurations.end(); i++) {

		if ((*i)->getType() == SADF_TIME_STEP)
			for (list<SADF_Transition*>::iterator j = (*i)->getTransitions().begin(); j != (*i)->getTransitions().end(); j++)
				Source->addTransition((*j)->getDestination(), (*j)->getProbability(), (*i)->getStepValue() + (*j)->getTimeSample());
		else if ((*i)->getType() == SADF_DETECT_STEP || (*i)->getType() == SADF_START_STEP)
			for (list<SADF_Transition*>::iterator j = (*i)->getTransitions().begin(); j != (*i)->getTransitions().end(); j++)
				Source->addTransition((*j)->getDestination(), (*j)->getProbability() * Source->getTransitionProbabilityTo(*i), (*j)->getTimeSample());
		else
			for (list<SADF_Transition*>::iterator j = (*i)->getTransitions().begin(); j != (*i)->getTransitions().end(); j++)
				Source->addTransition((*j)->getDestination(), (*j)->getProbability(), (*j)->getTimeSample());
	}
	
	Source->deleteTransitionsToIrrelevantConfigurations();
	
	// Delete irrelevant configurations
	
	while (!NewIrrelevantConfigurations.empty()) {
		delete (NewIrrelevantConfigurations.front());
		NewIrrelevantConfigurations.pop_front();
	}

	return NewRelevantConfigurations;
}

SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_BufferOccupancy(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId SourceProcessType, CId SourceProcessID, CId DestinationProcessType, CId DestinationProcessID) {

	// Non-determinism resolved and policy assumed to have no effect on peformance result

	bool ActionPossible = false;

	SADF_ListOfConfigurations NewRelevantConfigurations;
	SADF_ListOfConfigurations NewIrrelevantConfigurations;

	// Perform relevant start action
	
	if (!ActionPossible && SourceProcessType == SADF_KERNEL)
		if (Source->getKernelStatus(SourceProcessID)->isReadyToStart()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getKernelStatus(SourceProcessID)->start(Graph, TPS, true);
		}

	if (!ActionPossible && SourceProcessType == SADF_DETECTOR)
		if (Source->getDetectorStatus(SourceProcessID)->isReadyToStart()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getDetectorStatus(SourceProcessID)->start(Graph, TPS, true);
		}
	
	// Perform relevant end action

	if (!ActionPossible && DestinationProcessType == SADF_KERNEL)
		if (Source->getKernelStatus(DestinationProcessID)->isReadyToEnd()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getKernelStatus(DestinationProcessID)->end(Graph, TPS, true);
		}
	
	if (!ActionPossible && DestinationProcessType == SADF_DETECTOR)
		if (Source->getDetectorStatus(DestinationProcessID)->isReadyToEnd()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getDetectorStatus(DestinationProcessID)->end(Graph, TPS, true);
		}				

	// Perform control action - resulting configuration is irrelevant

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->control(Graph, TPS, false);
		}
			
	// Perform detect action - resulting configurations are irrelevant

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->detect(Graph, TPS, false);
		}		

	// Perform start action - irrelevant cases

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->start(Graph, TPS, false);
		}
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->start(Graph, TPS, false);
		}

	// Perform end action - irrelevant cases
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToEnd()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->end(Graph, TPS, false);
		}
		
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToEnd()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->end(Graph, TPS, false);
		}

	// Perform time step - resulting configuration is considered irrelevant
	
	if (!ActionPossible) {

		if (Source->getType() == SADF_TIME_STEP || Source->getMinimalRemainingExecutionTime() == SADF_MAX_DOUBLE)
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has a deadlock.");
			
		NewIrrelevantConfigurations.push_front(Source->time(Graph, TPS, false));
	}

	// Recursively progress for all newly created irrelevant configurations until all reachable relevant configurations are found and fix transitions

	for (SADF_ListOfConfigurations::iterator i = NewIrrelevantConfigurations.begin(); i != NewIrrelevantConfigurations.end(); i++) {

		SADF_ListOfConfigurations OtherNewRelevantConfigurations = SADF_ProgressTPS_ASAP_BufferOccupancy(Graph, TPS, *i, SourceProcessType, SourceProcessID, DestinationProcessType, DestinationProcessID);

		while (!OtherNewRelevantConfigurations.empty()) {
			NewRelevantConfigurations.push_front(OtherNewRelevantConfigurations.front());
			OtherNewRelevantConfigurations.pop_front();
		}
	}

	// Update transitions and fix time samples

	for (SADF_ListOfConfigurations::iterator i = NewIrrelevantConfigurations.begin(); i != NewIrrelevantConfigurations.end(); i++) {

		if ((*i)->getType() == SADF_TIME_STEP)
			for (list<SADF_Transition*>::iterator j = (*i)->getTransitions().begin(); j != (*i)->getTransitions().end(); j++)
				Source->addTransition((*j)->getDestination(), (*j)->getProbability(), (*i)->getStepValue() + (*j)->getTimeSample());
		else if ((*i)->getType() == SADF_DETECT_STEP || (*i)->getType() == SADF_START_STEP)
			for (list<SADF_Transition*>::iterator j = (*i)->getTransitions().begin(); j != (*i)->getTransitions().end(); j++)
				Source->addTransition((*j)->getDestination(), (*j)->getProbability() * Source->getTransitionProbabilityTo(*i), (*j)->getTimeSample());
		else
			for (list<SADF_Transition*>::iterator j = (*i)->getTransitions().begin(); j != (*i)->getTransitions().end(); j++)
				Source->addTransition((*j)->getDestination(), (*j)->getProbability(), (*j)->getTimeSample());
	}
	
	Source->deleteTransitionsToIrrelevantConfigurations();

	// Delete irrelevant configurations
	
	while (!NewIrrelevantConfigurations.empty()) {
		delete (NewIrrelevantConfigurations.front());
		NewIrrelevantConfigurations.pop_front();
	}

	return NewRelevantConfigurations;
}

SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_BufferOccupancy(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId SourceProcessType, CId SourceProcessID) {

	// Non-determinism resolved and policy has effect on peformance result - writes are scheduled before reads

	bool ActionPossible = false;

	SADF_ListOfConfigurations NewRelevantConfigurations;
	SADF_ListOfConfigurations NewIrrelevantConfigurations;

	// Perform relevant start action
	
	if (!ActionPossible && SourceProcessType == SADF_KERNEL)
		if (Source->getKernelStatus(SourceProcessID)->isReadyToStart()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getKernelStatus(SourceProcessID)->start(Graph, TPS, true);
		}

	if (!ActionPossible && SourceProcessType == SADF_DETECTOR)
		if (Source->getDetectorStatus(SourceProcessID)->isReadyToStart()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getDetectorStatus(SourceProcessID)->start(Graph, TPS, true);
		}
	
	// Perform control action - resulting configuration is irrelevant

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->control(Graph, TPS, false);
		}
			
	// Perform detect action - resulting configurations are irrelevant

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->detect(Graph, TPS, false);
		}		

	// Perform start action - irrelevant cases

	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->start(Graph, TPS, false);
		}
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->start(Graph, TPS, false);
		}

	// Perform end action - resulting configurations are irrelevant
	
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToEnd()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getKernelStatus(i)->end(Graph, TPS, false);
		}
		
	for (CId i = 0; !ActionPossible && i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToEnd()) {
			ActionPossible = true;
			NewIrrelevantConfigurations = Source->getDetectorStatus(i)->end(Graph, TPS, false);
		}

	// Perform time step - resulting configuration is considered irrelevant
	
	if (!ActionPossible) {

		if (Source->getType() == SADF_TIME_STEP || Source->getMinimalRemainingExecutionTime() == SADF_MAX_DOUBLE)
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has a deadlock.");
			
		NewIrrelevantConfigurations.push_front(Source->time(Graph, TPS, false));
	}

	// Recursively progress for all newly created irrelevant configurations until all reachable relevant configurations are found and fix transitions

	for (SADF_ListOfConfigurations::iterator i = NewIrrelevantConfigurations.begin(); i != NewIrrelevantConfigurations.end(); i++) {

		SADF_ListOfConfigurations OtherNewRelevantConfigurations = SADF_ProgressTPS_ASAP_BufferOccupancy(Graph, TPS, *i, SourceProcessType, SourceProcessID);

		while (!OtherNewRelevantConfigurations.empty()) {
			NewRelevantConfigurations.push_front(OtherNewRelevantConfigurations.front());
			OtherNewRelevantConfigurations.pop_front();
		}
	}

	// Delete transitions - don't need them
	
	Source->deleteAllTransitions();

	// Delete irrelevant configurations
	
	while (!NewIrrelevantConfigurations.empty()) {
		delete (NewIrrelevantConfigurations.front());
		NewIrrelevantConfigurations.pop_front();
	}

	return NewRelevantConfigurations;
}
/*
SADF_ListOfConfigurations SADF_ProgressTPS_ASAP_BufferSize(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId ChannelType, CId ChannelID, CId SourceProcessType, CId SourceProcessID) {

	// Non-determinism not resolved to find schedule that minimizes buffer occupancy

	bool ActionPossible = false;

	SADF_ListOfConfigurations NewRelevantConfigurations;
	SADF_ListOfConfigurations NewIrrelevantConfigurations;

	// Perform relevant start action
	
	if (SourceProcessType == SADF_KERNEL)
		if (Source->getKernelStatus(SourceProcessID)->isReadyToStart()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getKernelStatus(SourceProcessID)->start(Graph, TPS, false);
		}

	if (SourceProcessType == SADF_DETECTOR)
		if (Source->getDetectorStatus(SourceProcessID)->isReadyToStart()) {
			ActionPossible = true;
			NewRelevantConfigurations = Source->getDetectorStatus(SourceProcessID)->start(Graph, TPS, false);
		}
	
	// Perform control action - resulting configuration is irrelevant

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			
			SADF_ListOfConfigurations Temp = Source->getKernelStatus(i)->control(Graph, TPS, false);
			
			while (!Temp.empty()) {
				NewIrrelevantConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}
			
	// Perform detect action - resulting configurations are irrelevant

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToFire()) {
			ActionPossible = true;
			
			SADF_ListOfConfigurations Temp = Source->getDetectorStatus(i)->detect(Graph, TPS, false);
			
			while (!Temp.empty()) {
				NewIrrelevantConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}

	// Perform start action - irrelevant cases

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		if (Source->getKernelStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			
			SADF_ListOfConfigurations Temp = Source->getKernelStatus(i)->start(Graph, TPS, false);
			
			while (!Temp.empty()) {
				NewIrrelevantConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}
	
	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		if (Source->getDetectorStatus(i)->isReadyToStart()) {
			ActionPossible = true;
			
			SADF_ListOfConfigurations Temp = Source->getDetectorStatus(i)->start(Graph, TPS, false);

			while (!Temp.empty()) {
				NewIrrelevantConfigurations.push_front(Temp.front());
				Temp.pop_front();
			}
		}

	// Perform end action - resulting configurations are irrelevant
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		if (SourceProcessType == SADF_DETECTOR || i != SourceProcessID)
			if (Source->getKernelStatus(i)->isReadyToEnd()) {
				ActionPossible = true;
			
				SADF_ListOfConfigurations Temp = Source->getKernelStatus(i)->end(Graph, TPS, false);
				
				while (!Temp.empty()) {
					NewIrrelevantConfigurations.push_front(Temp.front());
					Temp.pop_front();
				}
			}
		
	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		if (SourceProcessType == SADF_KERNEL || i != SourceProcessID)
			if (Source->getDetectorStatus(i)->isReadyToEnd()) {
				ActionPossible = true;
			
				SADF_ListOfConfigurations Temp = Source->getDetectorStatus(i)->end(Graph, TPS, false);
				
				while (!Temp.empty()) {
					NewIrrelevantConfigurations.push_front(Temp.front());
					Temp.pop_front();
				}
			}

	// Perform time step - resulting configuration is considered irrelevant
	
	if (!ActionPossible) {

		if (Source->getType() == SADF_TIME_STEP || Source->getMinimalRemainingExecutionTime() == SADF_MAX_DOUBLE)
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has a deadlock.");
			
		NewIrrelevantConfigurations.push_front(Source->time(Graph, TPS, false));
	}

	// Delete all transitions - don't need them
	
	Source->deleteTransitionsToIrrelevantConfigurations();

	// Recursively progress for all newly created irrelevant configurations until all reachable relevant configurations are found and fix transitions

	for (SADF_ListOfConfigurations::iterator i = NewIrrelevantConfigurations.begin(); i != NewIrrelevantConfigurations.end(); i++) {

		SADF_ListOfConfigurations OtherNewRelevantConfigurations = SADF_ProgressTPS_ASAP_BufferSize(Graph, TPS, *i, ChannelType, ChannelID, SourceProcessType, SourceProcessID);

		while (!OtherNewRelevantConfigurations.empty()) {
			NewRelevantConfigurations.push_front(OtherNewRelevantConfigurations.front());
			OtherNewRelevantConfigurations.pop_front();
		}
	}

	// Delete irrelevant configurations
	
	while (!NewIrrelevantConfigurations.empty()) {
		delete (NewIrrelevantConfigurations.front());
		NewIrrelevantConfigurations.pop_front();
	}

	// Find next relevant configuration that requires minimum buffer space (how to ensure that all possible behaviours are still included?)

	SADF_Configuration* RelevantConfiguration = NewRelevantConfigurations.front();

	for (SADF_ListOfConfigurations::iterator i = NewRelevantConfigurations.begin(); i != NewRelevantConfigurations.end(); i++) {

		if (ChannelType == SADF_DATA_CHANNEL) {
			if ((*i)->getChannelStatus(ChannelID)->getOccupation() < RelevantConfiguration->getChannelStatus(ChannelID)->getOccupation())
				RelevantConfiguration = *i;
		} else {
			if ((*i)->getControlStatus(ChannelID)->getOccupation() < RelevantConfiguration->getControlStatus(ChannelID)->getOccupation())
				RelevantConfiguration = *i;
		}
	}

	// Check existance of relevant configuration
	
	RelevantConfiguration->computeHashKey();
	
	SADF_Configuration* Test = TPS->inConfigurationSpace(RelevantConfiguration);
				
	if (Test != NULL)
		delete RelevantConfiguration;
	else {
		RelevantConfiguration->setRelevance(true);
		TPS->addConfiguration(RelevantConfiguration);
		NewRelevantConfigurations.push_front(RelevantConfiguration);
	}

	return NewRelevantConfigurations;
}
*/
