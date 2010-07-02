/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_configuration.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Configuration
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#include "sadf_configuration.h"
#include "sadf_tps.h"
#include "sadf_kernel_status.h"
#include "sadf_detector_status.h"

// Constructors

SADF_Configuration::SADF_Configuration(SADF_Graph* Graph, SADF_TPS* TPS, CId StepType) : SADF_Component(SADF_UNDEFINED) {

	KernelStatus.resize(Graph->getNumberOfKernels());
	DetectorStatus.resize(Graph->getNumberOfDetectors());
	ChannelStatus.resize(Graph->getNumberOfDataChannels());
	ControlStatus.resize(Graph->getNumberOfControlChannels());

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		KernelStatus[i] = new SADF_KernelStatus(this, TPS->getInitialKernelState(i), SADF_MAX_DOUBLE);
	
	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		DetectorStatus[i] = new SADF_DetectorStatus(this, TPS->getInitialDetectorState(i), SADF_MAX_DOUBLE);
		
	for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
		ChannelStatus[i] = new SADF_ChannelStatus(Graph->getDataChannel(i));
		
	for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
		ControlStatus[i] = new SADF_ControlStatus(Graph->getControlChannel(i));

	MinimalRemainingExecutionTime = SADF_MAX_DOUBLE;

	setType(StepType);

	computeHashKey();

	Relevant = false;
	Marking = false;
}

SADF_Configuration::SADF_Configuration(SADF_Graph* Graph, SADF_Configuration* C, CId StepType, CDouble Value) : SADF_Component(SADF_UNDEFINED) {
	
	KernelStatus.resize(Graph->getNumberOfKernels());
	DetectorStatus.resize(Graph->getNumberOfDetectors());
	ChannelStatus.resize(Graph->getNumberOfDataChannels());
	ControlStatus.resize(Graph->getNumberOfControlChannels());

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		KernelStatus[i] = new SADF_KernelStatus(this, C->getKernelStatus(i));

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		DetectorStatus[i] = new SADF_DetectorStatus(this, C->getDetectorStatus(i));

	for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
 		ChannelStatus[i] = new SADF_ChannelStatus(C->getChannelStatus(i));
	
	for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
		ControlStatus[i] = new SADF_ControlStatus(C->getControlStatus(i));

	MinimalRemainingExecutionTime = C->getMinimalRemainingExecutionTime();

	setType(StepType);
	StepValue = Value;

	Relevant = false;
	Marking = false;
}

// Destructors

SADF_Configuration::~SADF_Configuration() {

	for (CId i = 0; i != KernelStatus.size(); i++)
		delete KernelStatus[i];
	
	for (CId i = 0; i != DetectorStatus.size(); i++)
		delete DetectorStatus[i];
	
	for (CId i = 0; i != ChannelStatus.size(); i++)
		delete ChannelStatus[i];
	
	for (CId i = 0; i != ControlStatus.size(); i++)
		delete ControlStatus[i];
	
	for (list<SADF_Transition*>::iterator i = Transitions.begin(); i != Transitions.end(); i++)
		delete *i;
}

void SADF_Configuration::deleteContent() {

	for (CId i = 0; i != KernelStatus.size(); i++)
		delete KernelStatus[i];
	
	for (CId i = 0; i != DetectorStatus.size(); i++)
		delete DetectorStatus[i];
	
	for (CId i = 0; i != ChannelStatus.size(); i++)
		delete ChannelStatus[i];
	
	for (CId i = 0; i != ControlStatus.size(); i++)
		delete ControlStatus[i];
	
	KernelStatus.resize(0);
	DetectorStatus.resize(0);
	ChannelStatus.resize(0);
	ControlStatus.resize(0);
}

// Computation of hash key

void SADF_Configuration::computeHashKey() {

	HashKey = getType() * 0.6180339887;
	
	for (CId i = 0; i != KernelStatus.size(); i++) {
		HashKey = HashKey * 39164205.20662217 + KernelStatus[i]->getState()->getScenario() * 0.6180339887;
		
		if (KernelStatus[i]->getRemainingExecutionTime() != SADF_MAX_DOUBLE)
			HashKey = HashKey * 39164205.20662217 + KernelStatus[i]->getRemainingExecutionTime() * 0.6180339887;
	}

	for (CId i = 0; i != DetectorStatus.size(); i++) {
		HashKey = HashKey * 39164205.20662217 + DetectorStatus[i]->getState()->getScenario() * 0.6180339887;

		if (DetectorStatus[i]->getRemainingExecutionTime() != SADF_MAX_DOUBLE)
			HashKey = HashKey * 39164205.20662217 + DetectorStatus[i]->getRemainingExecutionTime() * 0.6180339887;
		
		for (CId j = 0; j != DetectorStatus[i]->getState()->getMarkovChainStatus().size(); j++)
			HashKey = HashKey * 39164205.20662217 + DetectorStatus[i]->getState()->getStateOfMarkovChain(j) * 0.6180339887;
	}
	
	for (CId i = 0; i != ChannelStatus.size(); i++) {
		HashKey = HashKey * 39164205.20662217 + ChannelStatus[i]->getAvailableTokens() * 0.6180339887;
	}

	for (CId i = 0; i != ControlStatus.size(); i++) {
		HashKey = HashKey * 39164205.20662217 + ControlStatus[i]->getAvailableTokens() * 0.6180339887;
	}
}

// Functions to access transitions

void SADF_Configuration::addTransition(SADF_Configuration* Destination, CDouble Probability, CDouble TimeSample) {

	Transitions.push_front(new SADF_Transition(Destination, Probability, TimeSample));
}

SADF_Transition* SADF_Configuration::hasTransitionToConfigurationWithIdentity(const CId ID) {

	for (list<SADF_Transition*>::iterator i = Transitions.begin(); i != Transitions.end(); i++)
		if ((*i)->getDestination()->getIdentity() == ID)
			return *i;
	
	return NULL;
}

CDouble SADF_Configuration::getTransitionProbabilityTo(SADF_Configuration* Destination) {

	for (list<SADF_Transition*>::iterator i = Transitions.begin(); i != Transitions.end(); i++)
		if ((*i)->getDestination()->equal(Destination))
			return (*i)->getProbability();

	return 0;
}

void SADF_Configuration::removeAllTransitions() {

	while (!Transitions.empty())
		Transitions.pop_front();
}

void SADF_Configuration::deleteAllTransitions() {

	while (!Transitions.empty()) {
		SADF_Transition* Transition = Transitions.front();
		Transitions.pop_front();
		delete Transition;
	}
}

void SADF_Configuration::deleteTransitionsToIrrelevantConfigurations() {

	for (list<SADF_Transition*>::iterator i = Transitions.begin(); i != Transitions.end();) {

		list<SADF_Transition*>::iterator j = i;
		i++;

		if (!(*j)->getDestination()->isRelevant()) {
			delete *j;
			Transitions.erase(j);
		}
	}
}

// Functions to change status

SADF_Configuration* SADF_Configuration::time(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep) {

	SADF_Configuration* NewConfiguration = new SADF_Configuration(Graph, this, SADF_TIME_STEP, MinimalRemainingExecutionTime);
	NewConfiguration->setMinimalRemainingExecutionTime(SADF_MAX_DOUBLE);
		
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		if (KernelStatus[i]->getState()->getType() == SADF_START_STEP) {
			
			CDouble RemainingExecutionTime = NewConfiguration->getKernelStatus(i)->getRemainingExecutionTime() - MinimalRemainingExecutionTime;

			if (RemainingExecutionTime > 0) {
				NewConfiguration->getKernelStatus(i)->setRemainingExecutionTime(RemainingExecutionTime);
				if (RemainingExecutionTime < NewConfiguration->getMinimalRemainingExecutionTime())
					NewConfiguration->setMinimalRemainingExecutionTime(RemainingExecutionTime);
			} else
				NewConfiguration->getKernelStatus(i)->setRemainingExecutionTime(0);
		}

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		if (DetectorStatus[i]->getState()->getType() == SADF_START_STEP) {
			
			CDouble RemainingExecutionTime = NewConfiguration->getDetectorStatus(i)->getRemainingExecutionTime() - MinimalRemainingExecutionTime;
				
			if (RemainingExecutionTime > 0) {
				NewConfiguration->getDetectorStatus(i)->setRemainingExecutionTime(RemainingExecutionTime);
				if (RemainingExecutionTime < NewConfiguration->getMinimalRemainingExecutionTime())
					NewConfiguration->setMinimalRemainingExecutionTime(RemainingExecutionTime);
			} else 
				NewConfiguration->getDetectorStatus(i)->setRemainingExecutionTime(0);
		}

	if (RelevantStep) {
	
		NewConfiguration->computeHashKey();
		SADF_Configuration* Test = TPS->inConfigurationSpace(NewConfiguration);
				
		if (Test != NULL) {
			addTransition(Test, 1, 0);
			delete NewConfiguration;
			return NULL;
		} else {
			NewConfiguration->setRelevance(true);
			addTransition(NewConfiguration, 1, 0);
			TPS->addConfiguration(NewConfiguration);
			return NewConfiguration;
		}
	
	} else {
	
		addTransition(NewConfiguration, 1, 0);
		return NewConfiguration;
	}
}

// Equality operators

bool SADF_Configuration::equal(SADF_Configuration* C) {

	bool Equal = getType() == C->getType() && MinimalRemainingExecutionTime == C->getMinimalRemainingExecutionTime();

	for (CId i = 0; Equal && i != DetectorStatus.size(); i++)
		Equal = DetectorStatus[i]->equal(C->getDetectorStatus(i));
	
	for (CId i = 0; Equal && i != ControlStatus.size(); i++)
		Equal = ControlStatus[i]->equal(C->getControlStatus(i));

	for (CId i = 0; Equal && i != KernelStatus.size(); i++)
		Equal = KernelStatus[i]->equal(C->getKernelStatus(i));
		
	for (CId i = 0; Equal && i != ChannelStatus.size(); i++)
		Equal = ChannelStatus[i]->equal(C->getChannelStatus(i));
	
	return Equal;
}

// Print for debug

void SADF_Configuration::print(ostream& out) {

	out << " ----- Configuration ";
	
	if (getIdentity() == SADF_UNDEFINED)
		out << "Unknown -----" << endl << endl;
	else
		out << getIdentity() << " -----" << endl << endl;
	
	out << "Minimal Remaining Execution Time: " << MinimalRemainingExecutionTime << endl << endl;
	
	for (CId i = 0; i != KernelStatus.size(); i++) {
		out << "*** Kernel " << i << " ***" << endl;
		out << " State Type: " << KernelStatus[i]->getState()->getType() << endl;
		out << " Execution Time: " << KernelStatus[i]->getState()->getExecutionTime() << endl;
		out << " Scenario: " << KernelStatus[i]->getState()->getScenario() << endl;
		out << " RemainingExecutionTime: " << KernelStatus[i]->getRemainingExecutionTime() << endl << endl;
	}

	for (CId i = 0; i != DetectorStatus.size(); i++) {
		out << "*** Detector " << i << " ***" << endl;
		out << " State Type: " << DetectorStatus[i]->getState()->getType() << endl;
		out << " Execution Time: " << DetectorStatus[i]->getState()->getExecutionTime() << endl;
		out << " Scenario: " << DetectorStatus[i]->getState()->getScenario() << endl;
		out << " SubScenario: " << DetectorStatus[i]->getState()->getSubScenario() << endl;
		out << " RemainingExecutionTime: " << DetectorStatus[i]->getRemainingExecutionTime() << endl << endl;
	}
	
	for (CId i = 0; i != ChannelStatus.size(); i++) {
		out << "*** Data Channel " << i << " ***" << endl;
		out << " Number of Available Tokens: " << ChannelStatus[i]->getAvailableTokens() << endl;
		out << " Reserved Locations: " << ChannelStatus[i]->getReservedLocations() << endl << endl;
	}
	
	for (CId i = 0; i != ControlStatus.size(); i++) {
		out << "*** Control Channel " << i << " ***" << endl;
		out << " Number of Available Tokens: " << ControlStatus[i]->getAvailableTokens() << endl;
		out << " Reserved Locations: " << ControlStatus[i]->getReservedLocations() << endl << endl;
	}
	
	for (list<SADF_Transition*>::iterator i = Transitions.begin(); i != Transitions.end(); i++)
		out << "Transition to " << (*i)->getDestination()->getIdentity() << " with probability " << (*i)->getProbability() << endl;
}
