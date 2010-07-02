/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_deadline_miss.cc
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

#include "sadf_deadline_miss.h"

// Functions to analyse periodic deadline miss probability

void SADF_ConstructTPS_PeriodicDeadlineMissProbability(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId ProcessType, CId ProcessID, CDouble& Deadline) {

	SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP_InterFiringLatency(Graph, TPS, Source, ProcessType, ProcessID);

	Source->initialiseLocalResults(1);
	CDouble DeadlineMissProbability = 0;

	for (list<SADF_Transition*>::iterator i = Source->getTransitions().begin(); i != Source->getTransitions().end(); i++)
		if ((*i)->getTimeSample() > Deadline)
			DeadlineMissProbability += (*i)->getProbability();

	Source->setLocalResult(0, DeadlineMissProbability);

	// Fix transition probabilities (individual time samples lost)
	
	list<SADF_Transition*> OriginalTransitions(Source->getTransitions());

	Source->removeAllTransitions();	

	for (list<SADF_Transition*>::iterator i = OriginalTransitions.begin(); i != OriginalTransitions.end(); i++) {

		SADF_Transition* ExistingTransition = Source->hasTransitionToConfigurationWithIdentity((*i)->getDestination()->getIdentity());

		if (ExistingTransition == NULL)
			Source->addTransition((*i)->getDestination(), (*i)->getProbability(), 0);
		else
			ExistingTransition->setProbability(ExistingTransition->getProbability() + (*i)->getProbability());
			
		delete *i;
	}

	// Proceed constructing TPS from new configurations

	while (!NewConfigurations.empty()) {
		SADF_ConstructTPS_PeriodicDeadlineMissProbability(Graph, TPS, NewConfigurations.front(), ProcessType, ProcessID, Deadline);
		NewConfigurations.pop_front();
	}
}

CSize SADF_Analyse_PeriodicDeadlineMissProbability(SADF_Graph* Graph, CId ProcessType, CId ProcessID, CDouble& Deadline, CDouble& Result) {

	// Check whether graph satisfied required properties

	if (!SADF_Verify_SingleComponent(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not consist of a single component.");

	if (!SADF_Verify_Timed(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not timed.");
	
	if (!SADF_Verify_Boundedness(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not bounded.");

	bool Ergodic = SADF_Verify_SimpleErgodicity(Graph);	// Only in case Ergodic is true, the SADF graph is ergodic for sure. Otherwise, ergodicity test needed after generating TPS

	// Construct TPS

	SADF_TPS* TPS = new SADF_TPS(Graph);
	SADF_ConstructTPS_PeriodicDeadlineMissProbability(Graph, TPS, TPS->getInitialConfiguration(), ProcessType, ProcessID, Deadline);
	TPS->deleteContentOfConfigurations();
	TPS->removeTransientConfigurations();

	if (!Ergodic)
		if (!TPS->isSingleStronglyConnectedComponent())
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not imply an ergodic Markov chain.");

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

    vector<CDouble> EquilibriumDistribution = TPS->computeEquilibriumDistribution();
    
	// Compute Results

	Result = 0;

	for (SADF_HashedListOfConfigurations::iterator i = TPS->getConfigurationSpace().begin(); i != TPS->getConfigurationSpace().end(); i++)
		for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++)
			Result += EquilibriumDistribution[(*j)->getIdentity()] * (*j)->getLocalResult(0);

	delete TPS;

	return NumberOfConfigurations;
}

// Functions to analyse response deadline miss probability

CSize SADF_Analyse_ResponseDeadlineMissProbability(SADF_Graph* Graph, CId ProcessType, CId ProcessID, CDouble& Deadline, CDouble& Result) {

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

	Result = 0;

	for (list<SADF_Transition*>::iterator i = InitialConfiguration->getTransitions().begin(); i != InitialConfiguration->getTransitions().end(); i++)
		if ((*i)->getTimeSample() > Deadline)
			Result += (*i)->getProbability();

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

	delete TPS;
	
	return NumberOfConfigurations;
}
