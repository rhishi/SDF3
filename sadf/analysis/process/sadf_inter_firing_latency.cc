/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_inter_firing_latency.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of inter firing time
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

#include "sadf_inter_firing_latency.h"

// Function to construct TPS for analysing inter-firing latency metrics

void SADF_ConstructTPS_InterFiringLatency(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId ProcessType, CId ProcessID) {

	SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP_InterFiringLatency(Graph, TPS, Source, ProcessType, ProcessID);

	// Determine local results

	Source->initialiseLocalResults(4);

	CDouble AverageInterFiringTime = 0;
	CDouble AverageSquaredInterFiringTime = 0;

	CDouble Minimum = SADF_MAX_DOUBLE;
	CDouble Maximum = 0;

	for (list<SADF_Transition*>::iterator i = Source->getTransitions().begin(); i != Source->getTransitions().end(); i++) {

		CDouble Temp = (*i)->getProbability() * (*i)->getTimeSample();

		AverageInterFiringTime += Temp;
		AverageSquaredInterFiringTime += Temp * (*i)->getTimeSample();
		
		if ((*i)->getTimeSample() > Maximum)
			Maximum = (*i)->getTimeSample();
		
		if ((*i)->getTimeSample() < Minimum)
			Minimum = (*i)->getTimeSample();
	}
	
	Source->setLocalResult(0, AverageInterFiringTime);
	Source->setLocalResult(1, AverageSquaredInterFiringTime);

	Source->setLocalResult(2, Minimum);
	Source->setLocalResult(3, Maximum);

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
		SADF_ConstructTPS_InterFiringLatency(Graph, TPS, NewConfigurations.front(), ProcessType, ProcessID);
		NewConfigurations.pop_front();
	}
}

// Function to analyse inter-firing latency

CSize SADF_Analyse_LongRunInterFiringLatency(SADF_Graph* Graph, CId ProcessType, CId ProcessID, CDouble& Average, CDouble& Variance) {

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
	SADF_ConstructTPS_InterFiringLatency(Graph, TPS, TPS->getInitialConfiguration(), ProcessType, ProcessID);

	TPS->deleteContentOfConfigurations();
	TPS->removeTransientConfigurations();

	if (!Ergodic)
		if (!TPS->isSingleStronglyConnectedComponent())
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not imply an ergodic Markov chain.");

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

    vector<CDouble> EquilibriumDistribution = TPS->computeEquilibriumDistribution();

	// Compute Results

	Average = 0;
	CDouble AverageSquared = 0;

	for (SADF_HashedListOfConfigurations::iterator i = TPS->getConfigurationSpace().begin(); i != TPS->getConfigurationSpace().end(); i++)
		for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++) {
			Average += EquilibriumDistribution[(*j)->getIdentity()] * (*j)->getLocalResult(0);
			AverageSquared += EquilibriumDistribution[(*j)->getIdentity()] * (*j)->getLocalResult(1);
		}

	delete TPS;
	
	Variance = AverageSquared - (Average * Average);

	if (Variance < 0)
		Variance = 0;					// Circumvent rounding errors;

	return NumberOfConfigurations;
}

// Function to analyse extreem inter-firing latency

CSize SADF_Analyse_ExtreemInterFiringLatency(SADF_Graph* Graph, CId ProcessType, CId ProcessID, CDouble& Minimum, CDouble& Maximum) {

	// Check whether graph satisfied required properties

	if (!SADF_Verify_SingleComponent(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not consist of a single component.");

	if (!SADF_Verify_Timed(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not timed.");
	
	if (!SADF_Verify_Boundedness(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not bounded.");

	// Construct TPS

	SADF_TPS* TPS = new SADF_TPS(Graph);
	SADF_ConstructTPS_InterFiringLatency(Graph, TPS, TPS->getInitialConfiguration(), ProcessType, ProcessID);

	// Compute Results

	Minimum = SADF_MAX_DOUBLE;
	Maximum = 0;

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

	for (SADF_HashedListOfConfigurations::iterator i = TPS->getConfigurationSpace().begin(); i != TPS->getConfigurationSpace().end(); i++)
		for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++) {

			if ((*j)->getLocalResult(2) < Minimum)
				Minimum = (*j)->getLocalResult(2);

			if ((*j)->getLocalResult(3) > Maximum)
				Maximum = (*j)->getLocalResult(3);
		}

	delete TPS;
	
	return NumberOfConfigurations;
}
