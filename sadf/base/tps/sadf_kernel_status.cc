/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_kernel_status.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Kernel Status
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

#include "sadf_kernel_status.h"
#include "sadf_tps.h"

// Constructors

SADF_KernelTransition::SADF_KernelTransition(SADF_KernelState* D, const CDouble P) {

	Destination = D;
	Probability = P;
}

SADF_KernelState::SADF_KernelState(SADF_Process* K, CId ID, CId ScenarioID, CId ActionType, CDouble Time) : SADF_Component(ID) {

	Kernel = K;

	setType(ActionType);
	Scenario = ScenarioID;
	ExecutionTime = Time;
}

SADF_KernelStatus::SADF_KernelStatus(SADF_Configuration* C, SADF_KernelState* S, CDouble T) {
	
	Configuration = C;

	State = S;
	RemainingExecutionTime = T;
}

SADF_KernelStatus::SADF_KernelStatus(SADF_Configuration* C, SADF_KernelStatus* S) {

	Configuration = C;

	State = S->getState();
	RemainingExecutionTime = S->getRemainingExecutionTime();
}

// Destructor

SADF_KernelState::~SADF_KernelState() {

	for (list<SADF_KernelTransition*>::iterator i = Transitions.begin(); i != Transitions.end(); i++)
		delete *i;
}

// Access to instance variables

void SADF_KernelState::addTransition(SADF_KernelState* K, CDouble P) {

	Transitions.push_front(new SADF_KernelTransition(K, P));
}

// Functions to determine current status

bool SADF_KernelStatus::isReadyToFire() {

    // Only check for controlled kernels in which case the scenario is not specified after end step

	bool ReadyToFire = State->getType() == SADF_END_STEP && State->getScenario() == SADF_UNDEFINED;

	for (list<SADF_Channel*>::iterator i = State->getKernel()->getControlChannels().begin(); ReadyToFire && i != State->getKernel()->getControlChannels().end(); i++)
		if (Configuration->getControlStatus((*i)->getIdentity())->getAvailableTokens() == 0)
			ReadyToFire = false;
	
	return ReadyToFire;
}

bool SADF_KernelStatus::isReadyToStart() {

	// Determine Scenario

	CId Scenario = State->getScenario();

	bool ReadyToStart = State->getType() == SADF_CONTROL_STEP || (State->getType() == SADF_END_STEP && Scenario != SADF_UNDEFINED);

	// Check availability of tokens and space

	if (ReadyToStart && State->getKernel()->isActive(Scenario)) {
	
		for (list<SADF_Channel*>::iterator i = State->getKernel()->getInputChannels().begin(); ReadyToStart && i != State->getKernel()->getInputChannels().end(); i++)
			if (Configuration->getChannelStatus((*i)->getIdentity())->getAvailableTokens() < State->getKernel()->getConsumptionRate((*i)->getIdentity(), Scenario))
				ReadyToStart = false;
				
		for (list<SADF_Channel*>::iterator i = State->getKernel()->getOutputChannels().begin(); ReadyToStart && i != State->getKernel()->getOutputChannels().end(); i++)
			if ((*i)->getBufferSize() != SADF_UNBOUNDED)
				if (Configuration->getChannelStatus((*i)->getIdentity())->getAvailableTokens() + State->getKernel()->getProductionRate((*i)->getIdentity(), SADF_DATA_CHANNEL, Scenario) > (*i)->getBufferSize())
					ReadyToStart = false;
	}

	return ReadyToStart;
}

bool SADF_KernelStatus::isReadyToEnd() {

	return State->getType() == SADF_START_STEP && RemainingExecutionTime <= 0;	// <= is used to circumvent rounding errors
}

// Functions to change status

SADF_ListOfConfigurations SADF_KernelStatus::control(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep) {

	SADF_ListOfConfigurations NewConfigurations;
	
	SADF_Configuration* NewConfiguration = new SADF_Configuration(Graph, Configuration, SADF_CONTROL_STEP, 0);
	
	// Update status of kernel in new configuration
	
	SADF_Process* Kernel = State->getKernel();

	bool ScenarioNotFound = true;
				
	for (list<SADF_KernelTransition*>::iterator i = State->getTransitions().begin(); ScenarioNotFound && i != State->getTransitions().end(); i++) {
				
		CId Scenario = (*i)->getDestination()->getScenario();

		if (Kernel->hasControls()) {
						
			bool MatchingControls = true;
						
			for (list<SADF_Channel*>::iterator j = Kernel->getControlChannels().begin(); MatchingControls && j != Kernel->getControlChannels().end(); j++)
				if (Configuration->getControlStatus((*j)->getIdentity())->inspect() != Kernel->getScenario(Scenario)->getScenarioIdentityReceivedFromChannel((*j)->getIdentity()))
					MatchingControls = false;
						
			if (MatchingControls) {
				NewConfiguration->getKernelStatus(Kernel->getIdentity())->setState((*i)->getDestination());
				ScenarioNotFound = false;
			}

		} else
			if (Configuration->getControlStatus(Kernel->getControlChannels().front()->getIdentity())->inspect() == Scenario) {
				NewConfiguration->getKernelStatus(Kernel->getIdentity())->setState((*i)->getDestination());
				ScenarioNotFound = false;
			}
	}
	
	// Check existance of new configuration if relevant step
	
	if (RelevantStep) {
	
		NewConfiguration->computeHashKey();
		SADF_Configuration* Test = TPS->inConfigurationSpace(NewConfiguration);
				
		if (Test != NULL) {
			Configuration->addTransition(Test, 1, 0);
			delete NewConfiguration;
		} else {
			NewConfiguration->setRelevance(true);
			Configuration->addTransition(NewConfiguration, 1, 0);
			TPS->addConfiguration(NewConfiguration);
			NewConfigurations.push_front(NewConfiguration);
		}
	
	} else {
	
		Configuration->addTransition(NewConfiguration, 1, 0);
		NewConfigurations.push_front(NewConfiguration);
	}
	
	return NewConfigurations;
}

SADF_ListOfConfigurations SADF_KernelStatus::start(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep) {

	SADF_ListOfConfigurations NewConfigurations;

	SADF_Process* Kernel = State->getKernel();
	
	for (list<SADF_KernelTransition*>::iterator i = State->getTransitions().begin(); i != State->getTransitions().end(); i++) {
				
		SADF_Configuration* NewConfiguration = new SADF_Configuration(Graph, Configuration, SADF_START_STEP, 0);
		
		// Update status of kernel in new configuration
		
		if ((State->getType() == SADF_END_STEP || State->getType() == SADF_UNDEFINED) && Kernel->isActive(0))
			for (list<SADF_Channel*>::iterator j = Kernel->getOutputChannels().begin(); j != Kernel->getOutputChannels().end(); j++)
				NewConfiguration->getChannelStatus((*j)->getIdentity())->reserve(Kernel->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, 0));

		if (State->getType() == SADF_CONTROL_STEP && Kernel->isActive(State->getScenario()))
			for (list<SADF_Channel*>::iterator j = Kernel->getOutputChannels().begin(); j != Kernel->getOutputChannels().end(); j++)
				NewConfiguration->getChannelStatus((*j)->getIdentity())->reserve(Kernel->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, State->getScenario()));
		
		NewConfiguration->getKernelStatus(Kernel->getIdentity())->setState((*i)->getDestination());

		CDouble ExecutionTime = NewConfiguration->getKernelStatus(Kernel->getIdentity())->getRemainingExecutionTime();
					
		if (ExecutionTime > 0 && ExecutionTime < Configuration->getMinimalRemainingExecutionTime())
			NewConfiguration->setMinimalRemainingExecutionTime(ExecutionTime);

		// Check existance of new configuration if relevant step

		if (RelevantStep) {

			NewConfiguration->computeHashKey();
			SADF_Configuration* Test = TPS->inConfigurationSpace(NewConfiguration);
				
			if (Test != NULL) {
				Configuration->addTransition(Test, (*i)->getProbability(), 0);
				delete NewConfiguration;
			} else {
				NewConfiguration->setRelevance(true);
				Configuration->addTransition(NewConfiguration, (*i)->getProbability(), 0);
				TPS->addConfiguration(NewConfiguration);
				NewConfigurations.push_front(NewConfiguration);
			}

		} else {

			Configuration->addTransition(NewConfiguration, (*i)->getProbability(), 0);
			NewConfigurations.push_front(NewConfiguration);
		}
	}
	
	return NewConfigurations;
}

SADF_ListOfConfigurations SADF_KernelStatus::end(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep) {

	SADF_ListOfConfigurations NewConfigurations;

	SADF_Configuration* NewConfiguration = new SADF_Configuration(Graph, Configuration, SADF_END_STEP, 0);
	
	// Update status of kernel in new configuration
	
	SADF_Process* Kernel = State->getKernel();

	for (list<SADF_Channel*>::iterator i = Kernel->getControlChannels().begin(); i != Kernel->getControlChannels().end(); i++)
		NewConfiguration->getControlStatus((*i)->getIdentity())->remove();
	
	if (Kernel->isActive(State->getScenario())) {
	
		for (list<SADF_Channel*>::iterator i = Kernel->getInputChannels().begin(); i != Kernel->getInputChannels().end(); i++)
			NewConfiguration->getChannelStatus((*i)->getIdentity())->remove(Kernel->getConsumptionRate((*i)->getIdentity(), State->getScenario()));
		
		for (list<SADF_Channel*>::iterator i = Kernel->getOutputChannels().begin(); i != Kernel->getOutputChannels().end(); i++)
			NewConfiguration->getChannelStatus((*i)->getIdentity())->write(Kernel->getProductionRate((*i)->getIdentity(), SADF_DATA_CHANNEL, State->getScenario()));
	}

	NewConfiguration->getKernelStatus(Kernel->getIdentity())->setState(State->getTransitions().front()->getDestination());

	// Check existance of new configuration if relevant step
	
	if (RelevantStep) {
	
		NewConfiguration->computeHashKey();
		SADF_Configuration* Test = TPS->inConfigurationSpace(NewConfiguration);
		
		if (Test != NULL) {
			Configuration->addTransition(Test, 1, 0);
			delete NewConfiguration;
		} else {
			NewConfiguration->setRelevance(true);
			Configuration->addTransition(NewConfiguration, 1, 0);
			TPS->addConfiguration(NewConfiguration);
			NewConfigurations.push_front(NewConfiguration);
		}
		
	} else {
	
		Configuration->addTransition(NewConfiguration, 1, 0);
		NewConfigurations.push_front(NewConfiguration);
	}
	
	return NewConfigurations;
}

// Equality operators
	
bool SADF_KernelStatus::equal(SADF_KernelStatus* S) {
	
	return State->getIdentity() == S->getState()->getIdentity() && RemainingExecutionTime == S->getRemainingExecutionTime();
}
