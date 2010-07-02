/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   csdf2sadf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert CSDF Graph in SADF Graph
 *
 *  History         :
 *      13-09-06    :   Initial version.
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

#include "csdf2sadf.h"

SADF_Graph* CSDF2SADF(TimedCSDFgraph* CSDF) {

	SADF_Graph* SADF = new SADF_Graph(CSDF->getName(), CSDF->getId());

    // Compute number of kernels and detectors
    
    CId NumberOfKernels = 0;
    
    for (CSDFactorsIter i = CSDF->actorsBegin(); i != CSDF->actorsEnd(); i++)
        if ((*i)->sequenceLength() == 1)
            NumberOfKernels++;

	// Create Kernels with default scenario

	vector<SADF_Process*> Kernels(NumberOfKernels);

	CId KernelID = 0;

	for (CSDFactorsIter i = CSDF->actorsBegin(); i != CSDF->actorsEnd(); i++)
        if ((*i)->sequenceLength() == 1) {
    		TimedCSDFactor* Actor = (TimedCSDFactor*)(*i);
		    SADF_Process* Kernel = new SADF_Process(Actor->getName(), KernelID, SADF_KERNEL);
	    	vector<SADF_Scenario*> Scenarios(1);
    		SADF_Scenario* Scenario = new SADF_Scenario("default", 0);
		    vector<SADF_Profile*> Profiles(1);
	    	SADF_Profile* Profile = new SADF_Profile(0);

    		for (TimedCSDFactor::ProcessorsIter j = Actor->processorsBegin(); j != Actor->processorsEnd(); j++)
		    	if (Actor->getDefaultProcessor() == (*j)->type)
	    			Profile->setExecutionTime((CDouble)((*j)->execTime[0]));

    		Profiles[0] = Profile;
		    Scenario->setProfiles(Profiles);
	    	Scenarios[0] = Scenario;
    		Kernel->setScenarios(Scenarios);
		    Kernels[KernelID] = Kernel;
	    	KernelID++;
    	}
		
	SADF->setKernels(Kernels);
    
    // Create Detectors with appropriate scenario sequence
    
    vector<SADF_Process*> Detectors((CId)(CSDF->nrActors()) - NumberOfKernels);
    
    CId DetectorID = 0;
    
	for (CSDFactorsIter i = CSDF->actorsBegin(); i != CSDF->actorsEnd(); i++)
        if ((*i)->sequenceLength() > 1) {
    		TimedCSDFactor* Actor = (TimedCSDFactor*)(*i);
		    SADF_Process* Detector = new SADF_Process(Actor->getName(), DetectorID, SADF_DETECTOR);
	    	
            vector<SADF_Scenario*> Scenarios(1);
    		SADF_Scenario* Scenario = new SADF_Scenario("default", 0);
            
            vector<SADF_Scenario*> SubScenarios(Actor->sequenceLength());
        	vector<SADF_MarkovChainState*> StateSpace(Actor->sequenceLength());
            
            for (CId j = 0; j != Actor->sequenceLength(); j++) {
            
                SADF_Scenario* SubScenario = new SADF_Scenario("Phase" + (CString)(j), j);
                vector<SADF_Profile*> Profiles(1);
                SADF_Profile* Profile = new SADF_Profile(0);
                
        		for (TimedCSDFactor::ProcessorsIter k = Actor->processorsBegin(); k != Actor->processorsEnd(); k++)
	    	    	if (Actor->getDefaultProcessor() == (*k)->type)
	        			Profile->setExecutionTime((CDouble)((*k)->execTime[j]));
            
                Profiles[0] = Profile;
                SubScenario->setProfiles(Profiles);            
                SubScenarios[j] = SubScenario;

                SADF_MarkovChainState* State = new SADF_MarkovChainState("Phase" + (CString)(j), j, SubScenario);
                StateSpace[j] = State;
            }

            Detector->setSubScenarios(SubScenarios);
           
            SADF_MarkovChain* MarkovChain = new SADF_MarkovChain();
            MarkovChain->setStateSpace(StateSpace);
            MarkovChain->setInitialState(MarkovChain->getState(Actor->sequenceLength() - 1));

           	vector< vector<CDouble> > TransitionMatrix(Actor->sequenceLength());
	
        	for (CId j = 0; j != Actor->sequenceLength(); j++) {
        		vector<CDouble> Transitions(Actor->sequenceLength(), 0);
                
                if (j == Actor->sequenceLength() - 1)
                    Transitions[0] = 1;
                else
                    Transitions[j + 1] = 1;
                
        		TransitionMatrix[j] = Transitions;
        	}

            MarkovChain->setTransitionMatrix(TransitionMatrix);
            
            Scenario->setMarkovChain(MarkovChain);
            Scenarios[0] = Scenario;

    		Detector->setScenarios(Scenarios);
		    Detectors[DetectorID] = Detector;
	    	DetectorID++;
    	}

    SADF->setDetectors(Detectors);    

	// Create Data Channels
	
	vector<SADF_Channel*> DataChannels((CId)(CSDF->nrChannels()));
	
	CId DataChannelID = 0;
	
	for (CSDFchannelsIter i = CSDF->channelsBegin(); i != CSDF->channelsEnd(); i++) {
		TimedCSDFchannel* Channel = (TimedCSDFchannel*)(*i);
		SADF_Channel* DataChannel = new SADF_Channel(Channel->getName(), DataChannelID, SADF_DATA_CHANNEL);
		
		if (Channel->isConnected()) {

            if (SADF->getKernel(Channel->getSrcActor()->getName()) != NULL)
    			DataChannel->setSource(SADF->getKernel(Channel->getSrcActor()->getName()));
            else
                DataChannel->setSource(SADF->getDetector(Channel->getSrcActor()->getName()));
                
            if (SADF->getKernel(Channel->getDstActor()->getName()) != NULL)
    			DataChannel->setDestination(SADF->getKernel(Channel->getDstActor()->getName()));
            else
    			DataChannel->setDestination(SADF->getDetector(Channel->getDstActor()->getName()));
            
		} else
			throw CException((CString)("Channel '") + Channel->getName() + "' of CSDF graph '" + CSDF->getName() + "' is not connected.");
		
		if (Channel->getInitialTokens() != 0)
			DataChannel->setNumberOfInitialTokens((CId)(Channel->getInitialTokens()));
		if (Channel->getBufferSize().sz != -1)
			DataChannel->setBufferSize((CId)(Channel->getBufferSize().sz));
		if (Channel->getTokenSize() != -1)
			DataChannel->setTokenSize((CId)(Channel->getTokenSize()));
		
		DataChannels[DataChannelID] = DataChannel;
		DataChannelID++;
	}

	SADF->setDataChannels(DataChannels);
	
	// Finalise creation of Kernels

	for (CSDFactorsIter i = CSDF->actorsBegin(); i != CSDF->actorsEnd(); i++)
        if ((*i)->sequenceLength() == 1) {

    		TimedCSDFactor* Actor = (TimedCSDFactor*)(*i);
	    	SADF_Process* Kernel = SADF->getKernel(Actor->getName());

		    CId NumberOfConsumptions = 0;
		    CId NumberOfProductions = 0;

    		for (CSDFportsIter j = Actor->portsBegin(); j != Actor->portsEnd(); j++) {
	    		if ((*j)->getType() == CSDFport::In)
		    		NumberOfConsumptions++;
    			if ((*j)->getType() == CSDFport::Out)
	    			NumberOfProductions++;
		    }
		
    		vector<SADF_Communication*> Consumptions(NumberOfConsumptions);
	    	vector<SADF_Communication*> Productions(NumberOfProductions);
		
    		CId ConsumptionID = 0;
	    	CId ProductionID = 0;
		
    		for (CSDFportsIter j = Actor->portsBegin(); j != Actor->portsEnd(); j++) {
			
	    		if ((*j)->getType() == CSDFport::In) {
		    		if ((*j)->isConnected()) {			
			    		SADF->getKernel(Actor->getName())->addInputChannel(SADF->getChannel((*j)->getChannel()->getName()));
				    	Consumptions[ConsumptionID] = new SADF_Communication(ConsumptionID, SADF->getChannel((*j)->getChannel()->getName()), (*j)->getRate()[0]);
					    ConsumptionID++;
    				} else
	    				throw CException((CString)("Actor '") + Actor->getName() + "' of CSDF graph '" + CSDF->getName() + "' consumes from unknown channel through port '" + (*j)->getName() + "'.");
		    	}
			
			    if ((*j)->getType() == CSDFport::Out) {
				    if ((*j)->isConnected()) {			
    					SADF->getKernel(Actor->getName())->addOutputChannel(SADF->getChannel((*j)->getChannel()->getName()));
	    				Productions[ProductionID] = new SADF_Communication(ProductionID, SADF->getChannel((*j)->getChannel()->getName()), (*j)->getRate()[0]);
		    			ProductionID++;
			    	} else
				    	throw CException((CString)("Actor '") + Actor->getName() + "' of CSDF graph '" + CSDF->getName() + "' produces onto unknown channel through port '" + (*j)->getName() + "'.");
    			}
		
    		}
		
	    	Kernel->getScenario(0)->setConsumptions(Consumptions);
		    Kernel->getScenario(0)->setProductions(Productions);
    	}
        
    // Finalise creation of Detectors

	for (CSDFactorsIter i = CSDF->actorsBegin(); i != CSDF->actorsEnd(); i++)
        if ((*i)->sequenceLength() > 1) {

    		TimedCSDFactor* Actor = (TimedCSDFactor*)(*i);
	    	SADF_Process* Detector = SADF->getDetector(Actor->getName());

            for (CId j = 0; j != Actor->sequenceLength(); j++) {

    		    CId NumberOfConsumptions = 0;
	    	    CId NumberOfProductions = 0;

    	    	for (CSDFportsIter k = Actor->portsBegin(); k != Actor->portsEnd(); k++) {
	    	    	if (((*k)->getType() == CSDFport::In) && ((*k)->getRate()[j] != 0))
		    	    	NumberOfConsumptions++;
        			if (((*k)->getType() == CSDFport::Out) && ((*k)->getRate()[j] != 0))
	        			NumberOfProductions++;
		        }
		
      		    vector<SADF_Communication*> Consumptions(NumberOfConsumptions);
	    	    vector<SADF_Communication*> Productions(NumberOfProductions);
		
    		    CId ConsumptionID = 0;
	    	    CId ProductionID = 0;
		
        		for (CSDFportsIter k = Actor->portsBegin(); k != Actor->portsEnd(); k++) {

	    		    if (((*k)->getType() == CSDFport::In) && ((*k)->getRate()[j] != 0)) {
		        		if ((*k)->isConnected()) {			
		    	    		SADF->getDetector(Actor->getName())->addInputChannel(SADF->getChannel((*k)->getChannel()->getName()));
	    			    	Consumptions[ConsumptionID] = new SADF_Communication(ConsumptionID, SADF->getChannel((*k)->getChannel()->getName()), (*k)->getRate()[j]);
    					    ConsumptionID++;
    				    } else
	    		    		throw CException((CString)("Actor '") + Actor->getName() + "' of CSDF graph '" + CSDF->getName() + "' consumes from unknown channel through port '" + (*k)->getName() + "'.");
		        	}

		    	    if (((*k)->getType() == CSDFport::Out) && ((*k)->getRate()[j] != 0)) {
	    			    if ((*k)->isConnected()) {			
           					SADF->getDetector(Actor->getName())->addOutputChannel(SADF->getChannel((*k)->getChannel()->getName()));
	    				    Productions[ProductionID] = new SADF_Communication(ProductionID, SADF->getChannel((*k)->getChannel()->getName()), (*k)->getRate()[j]);
		    		    	ProductionID++;
			        	} else
			    	    	throw CException((CString)("Actor '") + Actor->getName() + "' of CSDF graph '" + CSDF->getName() + "' produces onto unknown channel through port '" + (*k)->getName() + "'.");
       	    		}
        		}
		
	        	Detector->getSubScenario(j)->setConsumptions(Consumptions);
    		    Detector->getSubScenario(j)->setProductions(Productions);

            }
    	}

	return SADF;
}
