/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_parser.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   XML Parser for SADF Graphs
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

#include "sadf_parser.h"

// Functions to check inter-process matching of scenarios and cross-fix scenario identities

void SADF_FixScenarioIdentitiesToControlKernel(CString& GraphName, SADF_Process* Kernel) {

	// Match scenarios if kernel is controlled by single detector
	
	if (!Kernel->hasControls()) {

		CId ChannelID = (*Kernel->getControlChannels().begin())->getIdentity();
		SADF_Process* Detector = (*Kernel->getControlChannels().begin())->getSource();

		// Check existance of kernel scenarios as produced values for detector

		for (CId i = 0; i != Kernel->getNumberOfScenarios(); i++) {

			bool Match = false;
			
			for (CId j = 0; !Match && j != Detector->getNumberOfSubScenarios(); j++)
				if (Detector->getSubScenario(j)->getValueProducedToChannel(ChannelID) == Kernel->getScenario(i)->getName())
					Match = true;
			
			if (!Match)
				throw CException((CString)("Error: Kernel '") + Kernel->getName() + "' of SADF graph '" + GraphName
					+ "' can operate in a scenario '" + Kernel->getScenario(i)->getName() + "', while detector '" + Detector->getName()
					+ "' controlling it does have a subscenario in which that scenario is invoked.");
		}

		// Fix properties of kernel for scenarios produced by detector and fix scenario identities
		
		for (CId i = 0; i != Detector->getNumberOfSubScenarios(); i++) {

			if (Kernel->getScenario(Detector->getSubScenario(i)->getValueProducedToChannel(ChannelID)) == NULL) {

				vector<SADF_Scenario*> Scenarios(Kernel->getNumberOfScenarios() + 1);
				vector<bool> ActiveInScenario(Kernel->getNumberOfScenarios() + 1);
				
				for (CId j = 0; j != Kernel->getNumberOfScenarios(); j++) {
					Scenarios[j] = Kernel->getScenario(j);
					ActiveInScenario[j] = Kernel->isActive(j);
				}

					
				SADF_Scenario* Scenario = new SADF_Scenario(Detector->getSubScenario(i)->getValueProducedToChannel(ChannelID), Kernel->getNumberOfScenarios());
				vector<SADF_Profile*> Profiles(1);
				Profiles[0] = new SADF_Profile(0);
				Scenario->setProfiles(Profiles);
				Scenarios[Kernel->getNumberOfScenarios()] = Scenario;
				Kernel->setScenarios(Scenarios);					// setSceranios resizes and resets scenario activity status of kernel

				ActiveInScenario[Kernel->getNumberOfScenarios()] = false;
				
				for (CId j = 0; j != Kernel->getNumberOfScenarios(); j++)
					if (!Kernel->isActive(j))
						Kernel->deactivate(j);
			}
				
			// Fix the scenario identity for the production of the detector
				
			for (CId j = 0; j != Detector->getSubScenario(i)->getNumberOfProductions(); j++)
				if (Detector->getSubScenario(i)->getProduction(j)->getChannel()->getIdentity() == ChannelID)
					Detector->getSubScenario(i)->getProduction(j)->setScenarioIdentity(Kernel->getScenario(Detector->getSubScenario(i)->getValueProducedToChannel(ChannelID))->getIdentity());
		}
	}

	// Match scenarios if kernel is controlled by multiple detectors
	
	if (Kernel->hasControls()) {
	
		for (list<SADF_Channel*>::iterator i = Kernel->getControlChannels().begin(); i != Kernel->getControlChannels().end(); i++) {
	
			CId ChannelID = (*i)->getIdentity();
			SADF_Process* Detector = (*i)->getSource();
		
			// Check existance of kernel control values as produced values for detector

			for (CId j = 0; j != Kernel->getNumberOfScenarios(); j++)
				for (CId k = 0; k != Kernel->getScenario(j)->getNumberOfControls(); k++) {
			
					bool Match = false;
				
					for (CId m = 0; !Match && m != Detector->getNumberOfScenarios(); m++)
						if (Detector->getSubScenario(m)->getValueProducedToChannel(ChannelID) == Kernel->getScenario(j)->getControl(k)->getValue())
							Match = true;
						
					if (!Match)
						throw CException((CString)("Error: Kernel '") + Kernel->getName() + "' of SADF graph '" + GraphName
							+ "' can operate in a scenario '" + Kernel->getScenario(j)->getName() + "' by receiving control value '" + Kernel->getScenario(j)->getControl(k)->getValue()
							+ "', while detector '" + Detector->getName() + "' controlling it does have a subscenario in which control tokens valued '"
							+ Kernel->getScenario(j)->getControl(k)->getValue() + "' are produced.");
				}
			
			// Fix properties of kernel for scenarios produced by detector and fix scenario identities
		
			for (CId j = 0; j != Detector->getNumberOfSubScenarios(); j++)
				for (CId k = 0; k != Kernel->getNumberOfScenarios(); k++)
					for (CId m = 0; m != Kernel->getScenario(k)->getNumberOfControls(); m++)
						if (Kernel->getScenario(k)->getControl(m)->getChannel()->getIdentity() == ChannelID
							&& Detector->getSubScenario(j)->getValueProducedToChannel(ChannelID) == Kernel->getScenario(k)->getControl(m)->getValue())
								Kernel->getScenario(k)->getControl(m)->setScenarioIdentity(Detector->getSubScenario(j)->getScenarioIdentityProducedToChannel(ChannelID));
		}
		
		// Ensure that any inactive scenarios exist
		
		cout << "Warning: Automatic construction of scenarios for which kernels with multiple control inputs are inactive is not yet implemented." << endl;
	}
}

void SADF_FixScenarioIdentitiesToControlDetector(CString& GraphName, SADF_Process* Detector) {

	// Match scenarios if destination detector is controlled by single source detector
	
	if (!Detector->hasControls()) {

		CId ChannelID = (*Detector->getControlChannels().begin())->getIdentity();
		SADF_Process* SourceDetector = (*Detector->getControlChannels().begin())->getSource();

		// Check existance of destination detector scenarios as produced values for source detector
		
		for (CId i = 0; i != Detector->getNumberOfScenarios(); i++) {
		
			bool Match = false;
			
			for (CId j = 0; !Match && j != SourceDetector->getNumberOfSubScenarios(); j++)
				if (SourceDetector->getSubScenario(j)->getValueProducedToChannel(ChannelID) == Detector->getScenario(i)->getName())
					Match = true;
			
			if (!Match)
				throw CException((CString)("Error: Detector '") + Detector->getName() + "' of SADF graph '" + GraphName
					+ "' can operate in a scenario '" + Detector->getScenario(i)->getName() + "', while detector '" + SourceDetector->getName()
					+ "' controlling it does have a subscenario in which that scenario is invoked.");
		}

		// Fix properties of destination detector for scenarios produced by source detector and fix scenario identities
		
		for (CId i = 0; i != SourceDetector->getNumberOfSubScenarios(); i++) {

			if (Detector->getScenario(SourceDetector->getSubScenario(i)->getValueProducedToChannel(ChannelID)) == NULL)
				throw CException((CString)("Error: Detector '") + Detector->getName() + "' of SADF graph '" + GraphName
					+ "' does not have a scenario '" + SourceDetector->getSubScenario(i)->getValueProducedToChannel(ChannelID) + "', while detector '" + SourceDetector->getName()
					+ "' controlling it does have a subscenario in which that scenario is invoked.");
				
			// Fix the scenario identity for the production of the source detector
				
			for (CId j = 0; j != SourceDetector->getSubScenario(i)->getNumberOfProductions(); j++)
				if (SourceDetector->getSubScenario(i)->getProduction(j)->getChannel()->getIdentity() == ChannelID)
					SourceDetector->getSubScenario(i)->getProduction(j)->setScenarioIdentity(Detector->getScenario(SourceDetector->getSubScenario(i)->getValueProducedToChannel(ChannelID))->getIdentity());
		}
	}
	
	// Match scenarios if destination detector is controlled by multiple source detectors

	if (Detector->hasControls()) {

		for (list<SADF_Channel*>::iterator i = Detector->getControlChannels().begin(); i != Detector->getControlChannels().end(); i++) {
	
			CId ChannelID = (*i)->getIdentity();
			SADF_Process* SourceDetector = (*i)->getSource();
		
			// Check existance of destination detector control values as produced values for source detector

			for (CId j = 0; j != Detector->getNumberOfScenarios(); j++)
				for (CId k = 0; k != Detector->getScenario(j)->getNumberOfControls(); k++) {
			
					bool Match = false;
				
					for (CId m = 0; !Match && m != SourceDetector->getNumberOfScenarios(); m++)
						if (SourceDetector->getSubScenario(m)->getValueProducedToChannel(ChannelID) == Detector->getScenario(j)->getControl(k)->getValue())
							Match = true;
						
					if (!Match)
						throw CException((CString)("Error: Detector '") + Detector->getName() + "' of SADF graph '" + GraphName
							+ "' can operate in a scenario '" + Detector->getScenario(j)->getName() + "' by receiving control value '" + Detector->getScenario(j)->getControl(k)->getValue()
							+ "', while detector '" + SourceDetector->getName() + "' controlling it does have a subscenario in which control tokens valued '"
							+ Detector->getScenario(j)->getControl(k)->getValue() + "' are produced.");
				}
			
			// Fix properties of destination detector for scenarios produced by source detector and fix scenario identities
		
			for (CId j = 0; j != SourceDetector->getNumberOfSubScenarios(); j++)
				for (CId k = 0; k != Detector->getNumberOfScenarios(); k++)
					for (CId m = 0; m != Detector->getScenario(k)->getNumberOfControls(); m++)
						if (Detector->getScenario(k)->getControl(m)->getChannel()->getIdentity() == ChannelID
							&& SourceDetector->getSubScenario(j)->getValueProducedToChannel(ChannelID) == Detector->getScenario(k)->getControl(m)->getValue())
								Detector->getScenario(k)->getControl(m)->setScenarioIdentity(SourceDetector->getSubScenario(j)->getScenarioIdentityProducedToChannel(ChannelID));
		}
	
		// Ensure that a scenario exists for all possible combinations of control inputs
	
		cout << "Warning: Cheking existance of scenarios for all control value combinations for detector wirh multiple control inputs is not yet implemented." << endl;
	}
}

// Construct SADF profile from XML

void SADF_ConstructProfile(SADF_Profile* Profile, const CNodePtr ProfileNode) {

	if (CHasAttribute(ProfileNode, "execution_time")) {
		if ((CDouble)(CGetAttribute(ProfileNode, "execution_time")) < 0)
			throw  CException("Error: Negative execution time sepecified.");
		else
			Profile->setExecutionTime(CGetAttribute(ProfileNode, "execution_time"));
	}

	if (CHasAttribute(ProfileNode, "weight"))
		Profile->setWeight(CGetAttribute(ProfileNode, "weight"));
}

// Construct SADF channel from XML

void SADF_ConstructChannel(CString& GraphName, SADF_Channel* Channel, const CNodePtr ChannelPropertyNode) {

	if (CHasAttribute(ChannelPropertyNode, "buffer_size"))
		Channel->setBufferSize(CGetAttribute(ChannelPropertyNode, "buffer_size"));

	if (Channel->getType() == SADF_DATA_CHANNEL)
		if (CHasAttribute(ChannelPropertyNode, "number_of_initial_tokens"))
			Channel->setNumberOfInitialTokens(CGetAttribute(ChannelPropertyNode, "number_of_initial_tokens"));
	
	if (CHasAttribute(ChannelPropertyNode, "token_size"))
		Channel->setTokenSize(CGetAttribute(ChannelPropertyNode, "token_size"));
		
	if (Channel->getType() == SADF_CONTROL_CHANNEL)
		if (CHasAttribute(ChannelPropertyNode, "initial_tokens")) {
		
			CString TokenSequence = CGetAttribute(ChannelPropertyNode, "initial_tokens").trim();
		
			// Parse sequence of initial control tokens
			
			while (TokenSequence.size() != 0) {
			
				char c;
				
				CString Element = "";
				
				do {
					c = TokenSequence[0];
					TokenSequence = TokenSequence.substr(1);
				
					if (c == ',')
						break;
				
					Element += c;
				
				} while (TokenSequence.size() != 0);

				bool NumberFound = false;

				CString Tokens = "";
				
				do {
					c = Element[0];
					Element = Element.substr(1);
					
					if (c == '*') {
						NumberFound = true;
						break;
					}
					
					Tokens += c;
					
				} while (Element.size() != 0);
				
				if (NumberFound & (Element.trim().size() == 0))
					throw CException((CString)("Error: Initial token sequence for channel '") + Channel->getName() + "' of SADF graph '" + GraphName + "' is invalid.");
					
				CString ScenarioName = "";
				CId NumberOfTokens = 1;
				
				if (NumberFound) {
					NumberOfTokens = strtol(Tokens, NULL, 10);
					ScenarioName = Element;
				} else					
					ScenarioName = Tokens;
				
				ScenarioName.trim();
					
				if (!Channel->getDestination()->hasControls()) {
					if (Channel->getDestination()->getScenario(ScenarioName) == NULL)
						throw CException((CString)("Error: Initial token valued '") + ScenarioName + "' as specified for channel '" + Channel->getName()
							+ "' of SADF graph '" + GraphName + "' is not a valid scenario for " + ((Channel->getDestination()->getType() == SADF_KERNEL) ? "kernel '" : "detector '")
							+ Channel->getDestination()->getName() + "'.");
					
					Channel->addInitialTokens(NumberOfTokens, Channel->getDestination()->getScenario(ScenarioName)->getIdentity());
				} else {
				
					bool ScenarioNotFound = true;
				
					for (CId i = 0; ScenarioNotFound && i != Channel->getDestination()->getNumberOfScenarios(); i++)
						if (Channel->getDestination()->getScenario(i)->getValueReceivedFromChannel(Channel->getIdentity()) == ScenarioName) {
							ScenarioNotFound = false;
							Channel->addInitialTokens(NumberOfTokens, Channel->getDestination()->getScenario(i)->getScenarioIdentityReceivedFromChannel(Channel->getIdentity()));
						}
					
					if (ScenarioNotFound)
						throw CException((CString)("Error: Initial token valued '") + ScenarioName + "' as specified for channel '" + Channel->getName()
							+ "' of SADF graph '" + GraphName + "' is not a valid control value for " + ((Channel->getDestination()->getType() == SADF_KERNEL) ? "kernel '" : "detector '")
							+ Channel->getDestination()->getName() + "'.");
				}
			}
		}
}

// Construct SADF Scenario from XML

void SADF_ConstructScenario(CString& GraphName, SADF_Process* Process, SADF_Scenario* Scenario, const CNodePtr ScenarioNode) {

	// Parse consumptions

	CId ConsumptionID = 0;
	
	vector<SADF_Communication*> Consumptions(CGetNumberOfChildNodes(ScenarioNode, "consume"));

	for (CNodePtr ConsumptionNode = CGetChildNode(ScenarioNode, "consume"); ConsumptionNode != NULL; ConsumptionNode = CNextNode(ConsumptionNode, "consume")) {
	
		if (!CHasAttribute(ConsumptionNode, "channel"))
			throw CException((CString)("Error: No channel specified for consumption in ") + ((Process->getType() == SADF_KERNEL) ? "scenario for kernel '" : "subscenario for detector '")
				+ Process->getName() + "' of SADF graph '" + GraphName + "'.");

		if (Process->getInputChannel(CGetAttribute(ConsumptionNode, "channel").trim()) == NULL)
			throw CException((CString)("Error: Channel '") + CGetAttribute(ConsumptionNode, "channel").trim() + "' for which consumption is specified in "
				+ ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + Scenario->getName() + "' is not an input channel for "
				+ ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

		for (CNodePtr ConsumptionNode2 = CNextNode(ConsumptionNode, "consume"); ConsumptionNode2 != NULL; ConsumptionNode2 = CNextNode(ConsumptionNode2, "consume"))
			if (CHasAttribute(ConsumptionNode2, "channel"))
				if (CGetAttribute(ConsumptionNode, "channel").trim() == CGetAttribute(ConsumptionNode2, "channel").trim())
					throw CException((CString)("Error: Multiple consumptions specified to channel '") + CGetAttribute(ConsumptionNode, "channel").trim() + "' in "
						+ ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + Scenario->getName() + "' for "
						+ ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

		if (!CHasAttribute(ConsumptionNode, "tokens"))
			throw CException((CString)("Error: No rate specified for consumption from channel '") + CGetAttribute(ConsumptionNode, "channel").trim() + "' in "
				+ ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + Scenario->getName() + "' for "
				+ ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

		Consumptions[ConsumptionID] = new SADF_Communication(ConsumptionID, Process->getInputChannel(CGetAttribute(ConsumptionNode, "channel").trim()), CGetAttribute(ConsumptionNode, "tokens"));
		ConsumptionID++;
	}

	Scenario->setConsumptions(Consumptions);

	// Parse productions

	CId ProductionID = 0;

	vector<SADF_Communication*> Productions(CGetNumberOfChildNodes(ScenarioNode, "produce"));

	for (CNodePtr ProductionNode = CGetChildNode(ScenarioNode, "produce"); ProductionNode != NULL; ProductionNode = CNextNode(ProductionNode, "produce")) {
	
		if (!CHasAttribute(ProductionNode, "channel"))
			throw CException((CString)("Error: No channel specified for production in ") + ((Process->getType() == SADF_KERNEL) ? "scenario for kernel '" : "subscenario for detector '")
				+ Process->getName() + "' of SADF graph '" + GraphName + "'.");

		if (Process->getOutputChannel(CGetAttribute(ProductionNode, "channel").trim()) == NULL)
			throw CException((CString)("Error: Channel '") + CGetAttribute(ProductionNode, "channel").trim() + "' for which production is specified in "
				+ ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + Scenario->getName() + "' is not an output channel for "
				+ ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + " of SADF graph '" + GraphName + "'.");

		for (CNodePtr ProductionNode2 = CNextNode(ProductionNode, "produce"); ProductionNode2 != NULL; ProductionNode2 = CNextNode(ProductionNode2, "produce"))
			if (CHasAttribute(ProductionNode2, "channel"))
				if (CGetAttribute(ProductionNode, "channel").trim() == CGetAttribute(ProductionNode2, "channel").trim())
					throw CException((CString)("Error: Multiple productions specified to channel '") + CGetAttribute(ProductionNode, "channel").trim() + "' in "
						+ ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + Scenario->getName() + "' for "
						+ ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

		if (!CHasAttribute(ProductionNode, "tokens"))
			throw CException((CString)("Error: No rate specified for production to channel '") + CGetAttribute(ProductionNode, "channel").trim() + "' in "
				+ ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + Scenario->getName() + "' for "
				+ ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

		SADF_Communication* Production = new SADF_Communication(ProductionID, Process->getOutputChannel(CGetAttribute(ProductionNode, "channel").trim()), CGetAttribute(ProductionNode, "tokens"));

		if ((Process->getType() == SADF_DETECTOR) && (Production->getChannel()->getType() == SADF_CONTROL_CHANNEL)) {
			if (CHasAttribute(ProductionNode, "value")) {
	
				Production->setValue(CGetAttribute(ProductionNode, "value").trim());
	
				if (CGetAttribute(ProductionNode, "value").trim() == Scenario->getName())
					Production->setScenarioIdentity(Scenario->getIdentity());

			} else {

				Production->setValue(Scenario->getName());
				Production->setScenarioIdentity(Scenario->getIdentity());
			}
		}

		Productions[ProductionID] = Production;
		ProductionID++;
	}

	Scenario->setProductions(Productions);

	// Parse profiles

	if (!CHasChildNode(ScenarioNode, "profile")) {

		vector<SADF_Profile*> Profiles(1);
		Profiles[0] = new SADF_Profile(0);
		Scenario->setProfiles(Profiles);
	
	} else {

		CId ProfileID = 0;
		CDouble Total = 0;

		vector<SADF_Profile*> Profiles(CGetNumberOfChildNodes(ScenarioNode, "profile"));

		for (CNodePtr ProfileNode = CGetChildNode(ScenarioNode, "profile"); ProfileNode != NULL; ProfileNode = CNextNode(ProfileNode, "profile")) {

			SADF_Profile* Profile = new SADF_Profile(ProfileID);
			SADF_ConstructProfile(Profile, ProfileNode);		
			Profiles[ProfileID] = Profile;
			Total += Profile->getWeight();
			ProfileID++;
		}
	
		for (CId i = 0; i != Profiles.size(); i++)
			Profiles[i]->setWeight(Profiles[i]->getWeight() / Total);

		Scenario->setProfiles(Profiles);
	}
}

// Construct SADF Markov Chain from XML

void SADF_ConstructMarkovChain(CString& GraphName, SADF_Process* Detector, SADF_MarkovChain* MarkovChain, const CNodePtr MarkovChainNode) {

	// Determine state space

	if (!CHasChildNode(MarkovChainNode, "state"))
		throw CException((CString)("Error: No states specified for Markov chain")
			+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
			+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

	CId StateID = 0;

	vector<SADF_MarkovChainState*> StateSpace(CGetNumberOfChildNodes(MarkovChainNode, "state"));

	for (CNodePtr StateNode = CGetChildNode(MarkovChainNode, "state"); StateNode != NULL; StateNode = CNextNode(StateNode, "state")) {

		if (!CHasAttribute(StateNode, "name"))
			throw CException((CString)("Error: No name specified for state of Markov chain")
				+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
				+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

		for (CNodePtr StateNode2 = CNextNode(StateNode, "state"); StateNode2 != NULL; StateNode2 = CNextNode(StateNode2, "state"))
			if (CHasAttribute(StateNode2, "name"))
				if (CGetAttribute(StateNode, "name").trim() == CGetAttribute(StateNode2, "name").trim())
					throw CException((CString)("Error: Multiple states named '") + CGetAttribute(StateNode, "name").trim() + "' specified for Markov chain"
						+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
						+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

		if (!CHasAttribute(StateNode, "subscenario"))
			throw CException((CString)("Error: No subscenario specified for state '") + CGetAttribute(StateNode, "name").trim() + "' of Markov chain"
				+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
			 	+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

		if (Detector->getSubScenario(CGetAttribute(StateNode, "subscenario").trim()) == NULL)
			throw CException((CString)("Error: Subscenario specified for state '") + CGetAttribute(StateNode, "name").trim() + "' of Markov chain"
				+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
			 	+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "' does not exist.");

		StateSpace[StateID] = new SADF_MarkovChainState(CGetAttribute(StateNode, "name").trim(), StateID, Detector->getSubScenario(CGetAttribute(StateNode, "subscenario").trim()));
		StateID++;
	}
	
	MarkovChain->setStateSpace(StateSpace);
	
	// Determinde initial state
	
	SADF_MarkovChainState* InitialState = MarkovChain->getState(CGetAttribute(MarkovChainNode, "initial_state").trim());
	
	if (InitialState != NULL)
		MarkovChain->setInitialState(InitialState);
	else
		throw CException((CString)("Error: Initial state '") + CGetAttribute(MarkovChainNode, "initial_state").trim() + "' of Markov chain"
			+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
			+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "' does not exist.");
	
	// Determine transition matrix
	
	vector< vector<CDouble> > TransitionMatrix(StateSpace.size());
	
	for (CId i = 0; i != StateSpace.size(); i++) {
		vector<CDouble> Transitions(StateSpace.size(), 0);
		TransitionMatrix[i] = Transitions;
	}
	
	for (CNodePtr StateNode = CGetChildNode(MarkovChainNode, "state"); StateNode != NULL; StateNode = CNextNode(StateNode, "state")) {
	
		SADF_MarkovChainState* State = MarkovChain->getState(CGetAttribute(StateNode, "name").trim());
	
		if (!CHasChildNode(StateNode, "transition"))
			throw CException((CString)("Error: No transitions specified from state '") + State->getName() + "' of Markov chain"
				+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
				+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
	
		CDouble Total = 0;

		for (CNodePtr TransitionNode = CGetChildNode(StateNode, "transition"); TransitionNode != NULL; TransitionNode = CNextNode(TransitionNode, "transition")) {

			if (!CHasAttribute(TransitionNode, "destination"))
				throw CException((CString)("Error: No destination specified for transition from state '") + State->getName() + "' of Markov chain"
					+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
					+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

			if (MarkovChain->getState(CGetAttribute(TransitionNode, "destination").trim()) == NULL)
				throw CException((CString)("Error: Destination state '") + CGetAttribute(TransitionNode, "destination").trim()
					+ "' does not exist for transition of Markov chain"
					+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
					+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

			for (CNodePtr TransitionNode2 = CNextNode(TransitionNode, "transition"); TransitionNode2 != NULL; TransitionNode2 = CNextNode(TransitionNode2, "transition"))
				if (CHasAttribute(TransitionNode2, "destination"))
					if (CGetAttribute(TransitionNode, "destination").trim() == CGetAttribute(TransitionNode2, "destination").trim())
						throw CException((CString)("Error: Multiple transitions to destination '") + CGetAttribute(TransitionNode, "destination").trim()
							+ "' specified from state '" + State->getName() + "' of Markov chain"
							+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
							+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
			
			CDouble Probability = 1; 

			if (CHasAttribute(TransitionNode, "weight"))
				Probability = CGetAttribute(TransitionNode, "weight"); 
	
			TransitionMatrix[State->getIdentity()][MarkovChain->getState(CGetAttribute(TransitionNode, "destination").trim())->getIdentity()] = Probability;
			Total = Total + Probability;
		}

		for (CId i = 0; i != StateSpace.size(); i++)
			TransitionMatrix[State->getIdentity()][i] = TransitionMatrix[State->getIdentity()][i] / Total;
	}
	
	MarkovChain->setTransitionMatrix(TransitionMatrix);
	
	if (!MarkovChain->isSingleComponent())
		throw CException((CString)("Error: Markov chain") + ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
			+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "' does not consist of a single component.");
}

// Construct SADF Kernel from XML

void SADF_ConstructKernel(CString& GraphName, SADF_Process* Kernel, const CNodePtr KernelPropertyNode) {

	// Construct scenarios

	if (!CHasChildNode(KernelPropertyNode, "scenario"))
		throw CException((CString)("Error: No scenario(s) speficied for kernel '") + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

	if (!Kernel->hasControlChannels() && (CGetNumberOfChildNodes(KernelPropertyNode, "scenario") > 1))
		throw CException((CString)("Error: Multiple scenarios defined for kernel '") + Kernel->getName() + "' without control inputs for SADF graph '" + GraphName + "'.");

	CId ScenarioID = 0;
	
	vector<SADF_Scenario*> Scenarios(CGetNumberOfChildNodes(KernelPropertyNode, "scenario"));

	for (CNodePtr ScenarioNode = CGetChildNode(KernelPropertyNode, "scenario"); ScenarioNode != NULL; ScenarioNode = CNextNode(ScenarioNode, "scenario")) {
		
		if (!CHasAttribute(ScenarioNode, "name"))
			throw CException((CString)("Error: No name specified for scenario of ") + "kernel '" + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");
		
		for (CNodePtr ScenarioNode2 = CNextNode(ScenarioNode, "scenario"); ScenarioNode2 != NULL; ScenarioNode2 = CNextNode(ScenarioNode2, "scenario"))
			if (CHasAttribute(ScenarioNode2, "name"))
				if (CGetAttribute(ScenarioNode, "name").trim() == CGetAttribute(ScenarioNode2, "name").trim())
					throw CException((CString)("Error: Multiple scenarios '") + CGetAttribute(ScenarioNode, "name").trim() + "' specified for kernel "
						+ Kernel->getName() + "' of SADF graph '" + GraphName + "'.");
		
		SADF_Scenario* Scenario = new SADF_Scenario(CGetAttribute(ScenarioNode, "name").trim(), ScenarioID);

		SADF_ConstructScenario(GraphName, Kernel, Scenario, ScenarioNode);

		// Construct scenario definition in case of multiple control channels

		if (Kernel->hasControls()) {

			if (!CHasChildNode(ScenarioNode, "control"))		
				throw CException((CString)("Error: Kernel '") + Kernel->getName() + "' requires specification of controls that define scenario '"
					+ CGetAttribute(ScenarioNode, "name").trim() + "' for SADF graph '" + GraphName + "'.");

			CId ControlID = 0;
			
			vector<SADF_Control*> Controls(CGetNumberOfChildNodes(ScenarioNode, "control"));
			
			for (CNodePtr ControlNode = CGetChildNode(ScenarioNode, "control"); ControlNode != NULL; ControlNode = CNextNode(ControlNode, "control")) {

				if (!CHasAttribute(ControlNode, "channel"))
					throw CException((CString)("Error: No channel specified for control that defines scenario '") + CGetAttribute(ScenarioNode, "name").trim() + "' for kernel '"
						+ Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

				if (Kernel->getControlChannel(CGetAttribute(ControlNode, "channel").trim()) == NULL )
					throw CException((CString)("Error: Channel '") + CGetAttribute(ControlNode, "channel").trim() + "' for which control is specified in scenario '"
						+ CGetAttribute(ScenarioNode, "name").trim() + "' is not a control channel to kernel '" + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

				for (CNodePtr ControlNode2 = CNextNode(ControlNode, "control"); ControlNode2 != NULL; ControlNode2 = CNextNode(ControlNode2, "control"))
					if (CHasAttribute(ControlNode2, "channel"))
						if (CGetAttribute(ControlNode, "channel").trim() == CGetAttribute(ControlNode2, "channel").trim())
							throw CException((CString)("Error: Multiple controls specified for channel '") + CGetAttribute(ControlNode, "channel").trim() + "' that define scneario '"
								+ CGetAttribute(ScenarioNode, "name").trim() + "' for kernel '" + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

				if (!CHasAttribute(ControlNode, "value"))
					throw CException((CString)("Error: No value specified for control from channel '") + CGetAttribute(ControlNode, "channel").trim() + "' in scenario '"
						+ CGetAttribute(ScenarioNode, "name").trim() + "' for kernel '" + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

				Controls[ControlID] = new SADF_Control(ControlID, Kernel->getControlChannel(CGetAttribute(ControlNode, "channel").trim()), CGetAttribute(ControlNode, "value").trim());
				ControlID++;
			}
			
			Scenario->setControls(Controls);
		}

		Scenarios[ScenarioID] = Scenario;
		ScenarioID++;
	}

	Kernel->setScenarios(Scenarios);

	// Check availability of control values for all control channels
	
	if (Kernel->hasControls())		
		for (list<SADF_Channel*>::iterator i = Kernel->getControlChannels().begin(); i != Kernel->getControlChannels().end(); i++)
			for (CId j = 0; j != Kernel->getNumberOfScenarios(); j++)
				if (Kernel->getScenario(j)->getValueReceivedFromChannel((*i)->getIdentity()) == "")
					throw CException((CString)("Error: No control value specified for control channel '") + (*i)->getName() + "' to kernel '"
						+ Kernel->getName() + "' of SADF graph '" + GraphName + "' for scenario '" + Kernel->getScenario(j)->getName() + "'.");
		
	// Check production/consumption on each of the channels in any scenario
	
	for (list<SADF_Channel*>::iterator i = Kernel->getInputChannels().begin(); i != Kernel->getInputChannels().end(); i++) {

		bool ConsumptionFound = false;
	
		for (CId j = 0; !ConsumptionFound && j != Kernel->getNumberOfScenarios(); j++)
			for (CId m = 0; m != Kernel->getScenario(j)->getNumberOfConsumptions(); m++)
				if (Kernel->getScenario(j)->getConsumption(m)->getChannel()->getIdentity() == (*i)->getIdentity())
					ConsumptionFound = true;
		
		if (!ConsumptionFound)
			throw CException((CString)("Error: No consumption from channel '") + (*i)->getName() + "' in any scenario of kernel '"
				+ Kernel->getName() + "' of SADF graph '" + GraphName + "'.");
	}
	
	for (list<SADF_Channel*>::iterator i = Kernel->getOutputChannels().begin(); i != Kernel->getOutputChannels().end(); i++) {

		bool ProductionFound = false;
	
		for (CId j = 0; !ProductionFound && j != Kernel->getNumberOfScenarios(); j++)
			for (CId m = 0; m != Kernel->getScenario(j)->getNumberOfProductions(); m++)
				if (Kernel->getScenario(j)->getProduction(m)->getChannel()->getIdentity() == (*i)->getIdentity())
					ProductionFound = true;
		
		if (!ProductionFound)
			throw CException((CString)("Error: No production on channel '") + (*i)->getName() + "' in any scenario of kernel '"
				+ Kernel->getName() + "' of SADF graph '" + GraphName + "'.");
	}
}

void SADF_ConstructDetector(CString& GraphName, SADF_Process* Detector, const CNodePtr DetectorPropertyNode) {

	// Construct subscenarios

	if (!CHasChildNode(DetectorPropertyNode, "subscenario"))
		throw CException((CString)("Error: No subscenario(s) speficied for ") + "detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

	CId SubScenarioID = 0;

	vector<SADF_Scenario*> SubScenarios(CGetNumberOfChildNodes(DetectorPropertyNode, "subscenario"));

	for (CNodePtr SubScenarioNode = CGetChildNode(DetectorPropertyNode, "subscenario"); SubScenarioNode != NULL; SubScenarioNode = CNextNode(SubScenarioNode, "subscenario")) {
		
		if (!CHasAttribute(SubScenarioNode, "name"))
			throw CException((CString)("Error: No name specified for subscenario of ") + "detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
		
		for (CNodePtr SubScenarioNode2 = CNextNode(SubScenarioNode, "subscenario"); SubScenarioNode2 != NULL; SubScenarioNode2 = CNextNode(SubScenarioNode2, "subscenario"))
			if (CHasAttribute(SubScenarioNode2, "name"))
				if (CGetAttribute(SubScenarioNode, "name").trim() == CGetAttribute(SubScenarioNode2, "name").trim())
					throw CException((CString)("Error: Multiple subscenarios '") + CGetAttribute(SubScenarioNode, "name").trim() + "' specified for detector "
						+ Detector->getName() + "' of SADF graph '" + GraphName + "'.");
		
		SADF_Scenario* SubScenario = new SADF_Scenario(CGetAttribute(SubScenarioNode, "name").trim(), SubScenarioID);
		SADF_ConstructScenario(GraphName, Detector, SubScenario, SubScenarioNode);
		
		// Check productions to all control channels
	
		for (list<SADF_Channel*>::iterator i = Detector->getOutputChannels().begin(); i != Detector->getOutputChannels().end(); i++)
			if ((*i)->getType() == SADF_CONTROL_CHANNEL) {
				
				bool Match = false;
				
				for (CId j = 0; (j != SubScenario->getNumberOfProductions() && !Match); j++)
					if (SubScenario->getProduction(j)->getChannel()->getIdentity() == (*i)->getIdentity())
						Match = true;
				
				if (!Match)
					throw CException((CString)("Error: No production specified to output channel '") + (*i)->getName() + "' of detector '"
						+ Detector->getName() + "' of SADF graph '" + GraphName + "' for subscenario '" + SubScenario->getName() + "'.");
			}
		
		SubScenarios[SubScenarioID] = SubScenario;
		SubScenarioID++;
	}

	Detector->setSubScenarios(SubScenarios);

	// Check naming uniqueness of valued productions to control channels and fix subscenario identities
	
	CId ScenarioIdentity = SubScenarios.size();
	
	for (CId i = 0; i != SubScenarios.size(); i++)
		for (CId j = 0; j != SubScenarios[i]->getNumberOfProductions(); j++)
			if (SubScenarios[i]->getProduction(j)->getScenarioIdentity() == SADF_UNDEFINED) {

				for (CId k = 0; k != SubScenarios.size(); k++)
					if (SubScenarios[i]->getProduction(j)->getValue() == SubScenarios[k]->getName())
						throw CException((CString)("Error: Production to output channel '") + SubScenarios[i]->getProduction(j)->getChannel()->getName() + "' of detector '"
						+ Detector->getName() + "' of SADF graph '" + GraphName + "' for subscenario '" + SubScenarios[i]->getName()
						+ "' is equally valued as the name of subscenario '" + SubScenarios[k]->getName() + "' for that detector.");
				
				SubScenarios[i]->getProduction(j)->setScenarioIdentity(ScenarioIdentity);
				ScenarioIdentity++;
			}

	// Construct scenario definitions in case of control channels

	if (Detector->hasControls()) {

		if (!CHasChildNode(DetectorPropertyNode, "scenario"))		
			throw CException((CString)("Error: Detector '") + Detector->getName() + "' for SADF graph '" + GraphName
				 + "' requires specification of scenarios for combinations of input values on control channels.");

		vector<SADF_Scenario*> Scenarios(CGetNumberOfChildNodes(DetectorPropertyNode, "scenario"));

		CId ScenarioID = 0;

		for (CNodePtr ScenarioNode = CGetChildNode(DetectorPropertyNode, "scenario"); ScenarioNode != NULL; ScenarioNode = CNextNode(ScenarioNode, "scenario")) {
			
			if (!CHasAttribute(ScenarioNode, "name"))
				throw CException((CString)("Error: No name specified for scenario of ") + "detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
			
			for (CNodePtr ScenarioNode2 = CNextNode(ScenarioNode, "scenario"); ScenarioNode2 != NULL; ScenarioNode2 = CNextNode(ScenarioNode2, "scenario"))
				if (CHasAttribute(ScenarioNode2, "name"))
					if (CGetAttribute(ScenarioNode, "name").trim() == CGetAttribute(ScenarioNode2, "name").trim())
						throw CException((CString)("Error: Multiple scenarios '") + CGetAttribute(ScenarioNode, "name").trim() + "' specified for detector "
							+ Detector->getName() + "' of SADF graph '" + GraphName + "'.");

			SADF_Scenario* Scenario = new SADF_Scenario(CGetAttribute(ScenarioNode, "name").trim(), ScenarioID);
			
			if (!CHasChildNode(ScenarioNode, "control"))		
				throw CException((CString)("Error: Detector '") + Detector->getName() + "' requires specification of controls that define scenario '"
					+ Scenario->getName() + "' for SADF graph '" + GraphName + "'.");

			CId ControlID = 0;
			
			vector<SADF_Control*> Controls(CGetNumberOfChildNodes(ScenarioNode, "control"));
			
			for (CNodePtr ControlNode = CGetChildNode(ScenarioNode, "control"); ControlNode != NULL; ControlNode = CNextNode(ControlNode, "control")) {

				if (!CHasAttribute(ControlNode, "channel"))
					throw CException((CString)("Error: No channel specified for control that defines scenario '") + Scenario->getName() + "' for detector '"
						+ Detector->getName() + "' of SADF graph '" + GraphName + "'.");

				if (Detector->getControlChannel(CGetAttribute(ControlNode, "channel").trim()) == NULL )
					throw CException((CString)("Error: Channel '") + CGetAttribute(ControlNode, "channel").trim() + "' for which control is specified in scenario '"
						+ Scenario->getName() + "' is not a control channel to detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

				for (CNodePtr ControlNode2 = CNextNode(ControlNode, "control"); ControlNode2 != NULL; ControlNode2 = CNextNode(ControlNode2, "control"))
					if (CHasAttribute(ControlNode2, "channel"))
						if (CGetAttribute(ControlNode, "channel").trim() == CGetAttribute(ControlNode2, "channel").trim())
							throw CException((CString)("Error: Multiple controls specified for channel '") + CGetAttribute(ControlNode, "channel").trim() + "' that define scneario '"
								+ Scenario->getName() + "' for detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

				if (!CHasAttribute(ControlNode, "value"))
					throw CException((CString)("Error: No value specified for control from channel '") + CGetAttribute(ControlNode, "channel").trim() + "' in scenario '"
						+ Scenario->getName() + "' for detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

				Controls[ControlID] = new SADF_Control(ControlID, Detector->getControlChannel(CGetAttribute(ControlNode, "channel").trim()), CGetAttribute(ControlNode, "value").trim());
				ControlID++;
			}

			Scenario->setControls(Controls);
			
			// Check availability of control values for all control channels
	
			for (list<SADF_Channel*>::iterator i = Detector->getControlChannels().begin(); i != Detector->getControlChannels().end(); i++)
				if (Scenario->getValueReceivedFromChannel((*i)->getIdentity()) == "")
					throw CException((CString)("Error: No control value specified for control channel '") + (*i)->getName() + "' to detector '"
						+ Detector->getName() + "' of SADF graph '" + GraphName + "' for scenario '" + Scenario->getName() + "'.");
			
			Scenarios[ScenarioID] = Scenario;
			ScenarioID++;
		}

		Detector->setScenarios(Scenarios);
	}
	
	// Construct Markov chain(s)

	if (!CHasChildNode(DetectorPropertyNode, "markov_chain"))
		throw CException((CString)("Error: No Markov chain(s) specified for detector '") + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

	if (!Detector->hasControlChannels() && (CGetNumberOfChildNodes(DetectorPropertyNode, "markov_chain") > 1))
		throw CException((CString)("Error: Multiple Markov chains specified for detector '") + Detector->getName() + "' of SADF graph '" + GraphName + "' while it requires only one.");

	for (CNodePtr MarkovChainNode = CGetChildNode(DetectorPropertyNode, "markov_chain"); MarkovChainNode != NULL; MarkovChainNode = CNextNode(MarkovChainNode, "markov_chain")) {

		if (Detector->hasControlChannels() && !CHasAttribute(MarkovChainNode, "scenario"))
			throw CException((CString)("Error: No scenario specified for Markov chain of detector '") + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

		if (Detector->hasControlChannels()) {

			if (!Detector->hasControls()) {

				vector<SADF_Scenario*> Scenarios(Detector->getNumberOfScenarios() + 1);
				
				for (CId i = 0; i != Detector->getNumberOfScenarios(); i++)
					Scenarios[i] = Detector->getScenario(i);
				
				SADF_Scenario* Scenario = new SADF_Scenario(CGetAttribute(MarkovChainNode, "scenario").trim(), Detector->getNumberOfScenarios());
				Scenarios[Detector->getNumberOfScenarios()] = Scenario;
				Detector->setScenarios(Scenarios);
			}

			if (Detector->hasControls() && Detector->getScenario(CGetAttribute(MarkovChainNode, "scenario").trim()) == NULL)
				throw CException((CString)("Error: Markov chain for non-existing scenario '") + CGetAttribute(MarkovChainNode, "scenario").trim() + " specified for detector '"
					+ Detector->getName() + "' of SADF graph '" + GraphName + "'.");
		}

		if (!CHasAttribute(MarkovChainNode, "initial_state"))
			throw CException((CString)("Error: No initial state specified for Markov chain")
				+ ((Detector->hasControlChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
					+ "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

		SADF_MarkovChain* MarkovChain = new SADF_MarkovChain();
		
		SADF_ConstructMarkovChain(GraphName, Detector, MarkovChain, MarkovChainNode);
		
		if (!Detector->hasControlChannels()) {

			vector<SADF_Scenario*> Scenarios(1);
			SADF_Scenario* Scenario = new SADF_Scenario("default", 0);
			Scenario->setMarkovChain(MarkovChain);
			Scenarios[0] = Scenario;
			Detector->setScenarios(Scenarios);
			
		} else
			Detector->getScenario(CGetAttribute(MarkovChainNode, "scenario").trim())->setMarkovChain(MarkovChain);
	}

	// Check existance of Markov chains for all scenarios (in case detector has control channels)

	if (Detector->hasControlChannels())
		for (CId i = 0; i != Detector->getNumberOfScenarios(); i++)
			if (Detector->getScenario(i)->getMarkovChain() == NULL)
				throw CException((CString)("Error: No Markov chain corresponding to scenario '") + Detector->getScenario(i)->getName() + "' specified for detector '"
					+ Detector->getName() + "' of SADF graph '" + GraphName + "'.");


	// Check production/consumption on each of the channels in any scenario
	
	for (list<SADF_Channel*>::iterator i = Detector->getInputChannels().begin(); i != Detector->getInputChannels().end(); i++) {

		bool ConsumptionFound = false;
	
		for (CId j = 0; !ConsumptionFound && j != Detector->getNumberOfSubScenarios(); j++)
			for (CId m = 0; m != Detector->getSubScenario(j)->getNumberOfConsumptions(); m++)
				if (Detector->getSubScenario(j)->getConsumption(m)->getChannel()->getIdentity() == (*i)->getIdentity())
					ConsumptionFound = true;
		
		if (!ConsumptionFound)
			throw CException((CString)("Error: No consumption from channel '") + (*i)->getName() + "' in any subscenario of detector '"
				+ Detector->getName() + "' of SADF graph '" + GraphName + "'.");
	}
	
	for (list<SADF_Channel*>::iterator i = Detector->getOutputChannels().begin(); i != Detector->getOutputChannels().end(); i++)
		if ((*i)->getType() == SADF_DATA_CHANNEL) {

			bool ProductionFound = false;
	
			for (CId j = 0; !ProductionFound && j != Detector->getNumberOfSubScenarios(); j++)
				for (CId m = 0; m != Detector->getSubScenario(j)->getNumberOfProductions(); m++)
					if (Detector->getSubScenario(j)->getProduction(m)->getChannel()->getIdentity() == (*i)->getIdentity())
						ProductionFound = true;
		
			if (!ProductionFound)
				throw CException((CString)("Error: No production on channel '") + (*i)->getName() + "' in any subscenario of detector '"
					+ Detector->getName() + "' of SADF graph '" + GraphName + "'.");
		}
}

// Construct SADF graph from XML

SADF_Graph* SADF_ConstructGraph(const CNodePtr ApplicationNode, const CId ApplicationNumber) {

	if (!CHasAttribute(ApplicationNode, "name"))
		throw CException("Error: No name specified for SADF graph.");
		
	SADF_Graph* Graph = new SADF_Graph(CGetAttribute(ApplicationNode, "name").trim(), ApplicationNumber);

	// Create graph structure

	if (!CHasChildNode(ApplicationNode, "structure"))
		throw CException((CString)("Error: No structure speficied for SADF graph ") + Graph->getName() + ".");

	CNodePtr Structure = CGetChildNode(ApplicationNode, "structure");

	// Create kernels

	CId KernelID = 0;
	
	vector<SADF_Process*> Kernels(CGetNumberOfChildNodes(Structure, "kernel"));

	for (CNodePtr KernelNode = CGetChildNode(Structure, "kernel"); KernelNode != NULL; KernelNode = CNextNode(KernelNode, "kernel")) {
		
		if (!CHasAttribute(KernelNode, "name"))
			throw CException((CString)("Error: No name specified for kernel of SADF graph '") + Graph->getName() + "'.");

		for (CNodePtr KernelNode2 = CNextNode(KernelNode, "kernel"); KernelNode2 != NULL; KernelNode2 = CNextNode(KernelNode2, "kernel"))
			if (CHasAttribute(KernelNode2, "name"))
				if (CGetAttribute(KernelNode, "name").trim() == CGetAttribute(KernelNode2, "name").trim())
					throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' contains multiple kernels named '"
						+ CGetAttribute(KernelNode, "name").trim() + "'.");

		Kernels[KernelID] = new SADF_Process(CGetAttribute(KernelNode, "name").trim(), KernelID, SADF_KERNEL);
		KernelID++;
	}

	Graph->setKernels(Kernels);

	// Create detectors

	CId DetectorID = 0;

	vector<SADF_Process*> Detectors(CGetNumberOfChildNodes(Structure, "detector"));

	for (CNodePtr DetectorNode = CGetChildNode(Structure, "detector"); DetectorNode != NULL; DetectorNode = CNextNode(DetectorNode, "detector")) {
		
		if (!CHasAttribute(DetectorNode, "name"))
			throw CException((CString)("Error: No name specified for detector of SADF graph '") + Graph->getName() + "'.");
		
		for (CNodePtr DetectorNode2 = CNextNode(DetectorNode, "detector"); DetectorNode2 != NULL; DetectorNode2 = CNextNode(DetectorNode2, "detector"))
			if (CHasAttribute(DetectorNode2, "name"))
				if (CGetAttribute(DetectorNode, "name").trim() == CGetAttribute(DetectorNode2, "name").trim())
					throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' contains multiple detectors named '"
						+ CGetAttribute(DetectorNode, "name").trim() + "'.");

		Detectors[DetectorID] = new SADF_Process(CGetAttribute(DetectorNode, "name").trim(), DetectorID, SADF_DETECTOR);
		DetectorID++;
	}

	Graph->setDetectors(Detectors);
	
	// Validate naming uniqueness of kernels versus detectors
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		for (CId j = 0; j != Graph->getNumberOfDetectors(); j++)
			if (Graph->getKernel(i)->getName() == Graph->getDetector(j)->getName())
				throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' contains kernel and detector with the same name '" + Graph->getKernel(i)->getName() + "'.");

	// Create Channels

	CId DataChannelID = 0;
	CId ControlChannelID = 0;
	
	for (CNodePtr ChannelNode = CGetChildNode(Structure, "channel"); ChannelNode != NULL; ChannelNode = CNextNode(ChannelNode, "channel"))
		if (CHasAttribute(ChannelNode, "type")) {
			if (CGetAttribute(ChannelNode, "type").trim() == "data")
				DataChannelID++;
			else if (CGetAttribute(ChannelNode, "type").trim() == "control")
				ControlChannelID++;
		}
	
	vector<SADF_Channel*> DataChannels(DataChannelID);
	vector<SADF_Channel*> ControlChannels(ControlChannelID);

	DataChannelID = 0;
	ControlChannelID = 0;
	
	for (CNodePtr ChannelNode = CGetChildNode(Structure, "channel"); ChannelNode != NULL; ChannelNode = CNextNode(ChannelNode, "channel")) {

		if (!CHasAttribute(ChannelNode, "name"))
			throw CException((CString)("Error: No name specified for channel of SADF graph '") + Graph->getName() + "'.");

		if (!CHasAttribute(ChannelNode, "source"))
			throw CException((CString)("Error: No source specified for channel '") + CGetAttribute(ChannelNode, "name").trim() + "' of SADF graph '" + Graph->getName() + "'.");

		if (!CHasAttribute(ChannelNode, "destination"))
			throw CException((CString)("Error: No destination specified for channel '") + CGetAttribute(ChannelNode, "name").trim() + "' of SADF graph '" + Graph->getName() + "'.");
		
		if (!CHasAttribute(ChannelNode, "type"))
			throw CException((CString)("Error: No type specified for channel '") + CGetAttribute(ChannelNode, "name").trim() + "' of SADF graph '" + Graph->getName() + "'.");

		for (CNodePtr ChannelNode2 = CNextNode(ChannelNode, "channel"); ChannelNode2 != NULL; ChannelNode2 = CNextNode(ChannelNode2, "channel"))
			if (CHasAttribute(ChannelNode2, "name"))
				if (CGetAttribute(ChannelNode, "name").trim() == CGetAttribute(ChannelNode2, "name").trim())
					throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' contains multiple channels named '"
						+ CGetAttribute(ChannelNode, "name").trim() + "'.");

		SADF_Channel* Channel = NULL;

		if (CGetAttribute(ChannelNode, "type").trim() == "data") {

			Channel = new SADF_Channel(CGetAttribute(ChannelNode, "name").trim(), DataChannelID, SADF_DATA_CHANNEL);
			DataChannels[DataChannelID] = Channel;
			DataChannelID++;

		} else if (CGetAttribute(ChannelNode, "type").trim() == "control") {
		
			Channel = new SADF_Channel(CGetAttribute(ChannelNode, "name").trim(), ControlChannelID, SADF_CONTROL_CHANNEL);
			ControlChannels[ControlChannelID] = Channel;
			ControlChannelID++;

		} else 
			throw CException((CString)("Error: Invalid type specified for channel '") + CGetAttribute(ChannelNode, "name").trim() + "' of SADF graph '" + Graph->getName() + "'.");

		// Validate connectivity to existing source and destination

		SADF_Process* Kernel = Graph->getKernel(CGetAttribute(ChannelNode, "source").trim());
		SADF_Process* Detector;
			
		if (Kernel != NULL) {
			
			if (Channel->getType() == SADF_CONTROL_CHANNEL)
				throw CException((CString)("Error: Source of control channel '") + Channel->getName() + "' is kernel '" + Kernel->getName() + "' in SADF graph '" + Graph->getName() + "'.");
			
			Channel->setSource(Kernel);
			
		} else {
			Detector = Graph->getDetector(CGetAttribute(ChannelNode, "source").trim());
				
			if (Detector != NULL)
				Channel->setSource(Detector);
			else
				throw CException((CString)("Error: No matching kernel or detector for source '") + CGetAttribute(ChannelNode, "source").trim() + "' of channel '"
					+ Channel->getName() + "' of SADF graph '" + Graph->getName() + "'.");
		}

		Kernel = Graph->getKernel(CGetAttribute(ChannelNode, "destination").trim());
			
		if (Kernel != NULL)
			Channel->setDestination(Kernel);
		else {
			Detector = Graph->getDetector(CGetAttribute(ChannelNode, "destination").trim());
			
			if (Detector != NULL)
				Channel->setDestination(Detector);
			else
				throw CException((CString)("Error: No matching kernel or detector for destination '") + CGetAttribute(ChannelNode, "destination").trim() + "' of channel '"
					+ Channel->getName() + "' of SADF graph '" + Graph->getName() + "'.");
		}
	}
	
	Graph->setDataChannels(DataChannels);
	Graph->setControlChannels(ControlChannels);

	// Validate connectivity of processes to existing channels
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
	
		for (CId j = 0; j != Graph->getNumberOfDataChannels(); j++) {

			if (Kernels[i]->getName() == DataChannels[j]->getSource()->getName())
				Kernels[i]->addOutputChannel(DataChannels[j]);

			if (Kernels[i]->getName() == DataChannels[j]->getDestination()->getName())
				Kernels[i]->addInputChannel(DataChannels[j]);
		}

		for (CId j = 0; j != Graph->getNumberOfControlChannels(); j++) {

			if (Kernels[i]->getName() == ControlChannels[j]->getSource()->getName())
				throw CException((CString)("Error: Source of control channel '") + ControlChannels[j]->getName() + "' refers to kernel '"
					+ Kernels[i]->getName() + "' instead of a detector in SADF graph '" + Graph->getName() + "'.");

			if (Kernels[i]->getName() == ControlChannels[j]->getDestination()->getName())
				Kernels[i]->addControlChannel(ControlChannels[j]);
		}
	}

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
	
		for (CId j = 0; j != Graph->getNumberOfDataChannels(); j++) {

			if (Detectors[i]->getName() == DataChannels[j]->getSource()->getName())
				Detectors[i]->addOutputChannel(DataChannels[j]);
			
			if (Detectors[i]->getName() == (DataChannels[j])->getDestination()->getName())
				Detectors[i]->addInputChannel(DataChannels[j]);
		}
		
		for (CId j = 0; j != Graph->getNumberOfControlChannels(); j++) {
		
			if (Detectors[i]->getName() == ControlChannels[j]->getSource()->getName())
				Detectors[i]->addOutputChannel(ControlChannels[j]);

			if (Detectors[i]->getName() == ControlChannels[j]->getDestination()->getName())
				Detectors[i]->addControlChannel(ControlChannels[j]);
		}
	}

	// Annotate processes with their properties

	if (!CHasChildNode(ApplicationNode, "properties"))
		throw CException((CString)("Error: No properties specified for SADF graph '") + Graph->getName() + "'.");

	CNodePtr Properties = CGetChildNode(ApplicationNode, "properties");
	
	for (CNodePtr KernelPropertyNode = CGetChildNode(Properties, "kernel_properties"); KernelPropertyNode != NULL; KernelPropertyNode = CNextNode(KernelPropertyNode, "kernel_properties")) {
	
		if (!CHasAttribute(KernelPropertyNode, "kernel"))
			throw CException((CString)("Error: No kernel name specified for kernel properties in SADF graph '") + Graph->getName() + "'.");
		
		SADF_Process* Kernel = Graph->getKernel(CGetAttribute(KernelPropertyNode, "kernel").trim());
		
		if (Kernel != NULL)
			SADF_ConstructKernel(Graph->getName(), Kernel, KernelPropertyNode);
		else
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has no kernel named '" + CGetAttribute(KernelPropertyNode, "kernel").trim() + "'.");
	}

	for (CNodePtr DetectorPropertyNode = CGetChildNode(Properties, "detector_properties"); DetectorPropertyNode != NULL; DetectorPropertyNode = CNextNode(DetectorPropertyNode, "detector_properties")) {
	
		if (!CHasAttribute(DetectorPropertyNode, "detector"))
			throw CException((CString)("Error: No detector name specified for detector properties in SADF graph '") + Graph->getName() + "'.");
		
		SADF_Process* Detector = Graph->getDetector(CGetAttribute(DetectorPropertyNode, "detector").trim());

		if (Detector != NULL)
			SADF_ConstructDetector(Graph->getName(), Detector, DetectorPropertyNode);
		else
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has no detector named '" + CGetAttribute(DetectorPropertyNode, "detector").trim() + "'.");
	}

	// Check inter-process matching of scenarios and fix scenario identities

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		if (Kernels[i]->hasControlChannels())
			SADF_FixScenarioIdentitiesToControlKernel(Graph->getName(), Kernels[i]);

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		if (Detectors[i]->hasControlChannels())
			SADF_FixScenarioIdentitiesToControlDetector(Graph->getName(), Detectors[i]);

	// Deactive inactive scenarios of kernels
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
			if (Graph->getKernel(i)->getScenario(j)->getNumberOfConsumptions() + Graph->getKernel(i)->getScenario(j)->getNumberOfProductions() == 0)
				Graph->getKernel(i)->deactivate(j);

	// Annotate channels with their properties

	for (CNodePtr ChannelPropertyNode = CGetChildNode(Properties, "channel_properties"); ChannelPropertyNode != NULL; ChannelPropertyNode = CNextNode(ChannelPropertyNode, "channel_properties")) {
	
		if (!CHasAttribute(ChannelPropertyNode, "channel"))
			throw CException((CString)("Error: No channel name specified for channel properties in SADF graph '") + Graph->getName() + "'.");
		
		SADF_Channel* Channel = Graph->getChannel(CGetAttribute(ChannelPropertyNode, "channel").trim());
		
		if (Channel != NULL)
			SADF_ConstructChannel(Graph->getName(), Channel, ChannelPropertyNode);
		else
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has no channel named '" + CGetAttribute(ChannelPropertyNode, "channel").trim() + "'.");
	}

	// Return the constructed SADF graph

	return Graph;
}
