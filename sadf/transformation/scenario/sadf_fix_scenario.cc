/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_fix_scenario.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   Transformations of SADF Graphs related to (Sub)Scenarios
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

#include "sadf_fix_scenario.h"

SADF_Graph* SADF_FixScenarios(SADF_Graph* Graph, vector<CId> SubScenarios) {

    SADF_Graph* NewGraph = new SADF_Graph(CString(Graph->getName()), Graph->getIdentity());

    // Identify scenarios for kernels
    
    vector<CId> Scenarios(Graph->getNumberOfKernels(), 0);

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        if (Graph->getKernel(i)->hasControlChannels()) {
            if (Graph->getKernel(i)->hasControls()) {
            
    			bool ScenarioNotFound = true;

	    		for (CId j = 0; ScenarioNotFound && j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
			
    				bool MatchingControls = true;
			
	    			for (list<SADF_Channel*>::iterator k = Graph->getKernel(i)->getControlChannels().begin(); MatchingControls && k != Graph->getKernel(i)->getControlChannels().end(); k++)
                        if (SubScenarios[(*k)->getSource()->getIdentity()] != Graph->getKernel(i)->getScenario(j)->getScenarioIdentityReceivedFromChannel((*k)->getIdentity()))
                            MatchingControls = false;

    				if (MatchingControls) {
	    				ScenarioNotFound = false;
		    			Scenarios[i] = j;
				    }
			    }
                
                if (ScenarioNotFound)
                    throw CException((CString)("Error: Requested combination of subscenarios does not yield valid behaviour for kernel '") + Graph->getKernel(i)->getName() + "'.");

            } else {

                SADF_Channel* ControlChannel = Graph->getKernel(i)->getControlChannels().front();
                SADF_Process* Detector = ControlChannel->getSource();

                Scenarios[i] = Detector->getSubScenario(SubScenarios[Detector->getIdentity()])->getScenarioIdentityProducedToChannel(ControlChannel->getIdentity());
            }
	}

    // Create kernels
    
    vector<SADF_Process*> Kernels(Graph->getNumberOfKernels() + Graph->getNumberOfDetectors());
    
    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        Kernels[i] = new SADF_Process(CString(Graph->getKernel(i)->getName()), i, SADF_KERNEL);
    
    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        Kernels[Graph->getNumberOfKernels() + i] = new SADF_Process(CString(Graph->getDetector(i)->getName()), Graph->getNumberOfKernels() + i, SADF_KERNEL);

    NewGraph->setKernels(Kernels);

    // Create channels

    CId NumberOfActiveDataChannels = 0;
    vector<bool> ActivityOnDataChannel(Graph->getNumberOfDataChannels(), true);
      
    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {

        SADF_Process* Source = Graph->getDataChannel(i)->getSource();
        SADF_Process* Destination = Graph->getDataChannel(i)->getDestination();

        CId ConsumptionRate = 0;
        CId ProductionRate = 0;
        
        if (Source->getType() == SADF_KERNEL)
            ProductionRate = Source->getProductionRate(i, SADF_DATA_CHANNEL, Scenarios[Source->getIdentity()]);
        else
            ProductionRate = Source->getProductionRate(i, SADF_DATA_CHANNEL, SubScenarios[Source->getIdentity()]);
        
        if (Destination->getType() == SADF_KERNEL)
            ConsumptionRate = Destination->getConsumptionRate(i, Scenarios[Destination->getIdentity()]);
        else
            ConsumptionRate = Destination->getConsumptionRate(i, SubScenarios[Destination->getIdentity()]);
            
        if (ProductionRate != 0 && ConsumptionRate != 0)
            NumberOfActiveDataChannels++;
        else
            ActivityOnDataChannel[i] = false;
    }

    vector<SADF_Channel*> Channels(NumberOfActiveDataChannels + Graph->getNumberOfControlChannels());

    CId NewDataChannelID = 0;

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
        if (ActivityOnDataChannel[i]) {

            SADF_Channel* Channel = new SADF_Channel(CString(Graph->getDataChannel(i)->getName()), NewDataChannelID, SADF_DATA_CHANNEL);
        
            SADF_Process* Source = NewGraph->getKernel(Graph->getDataChannel(i)->getSource()->getName());
            SADF_Process* Destination = NewGraph->getKernel(Graph->getDataChannel(i)->getDestination()->getName());
        
            Channel->setSource(Source);
            Channel->setDestination(Destination);
            Source->addOutputChannel(Channel);
            Destination->addInputChannel(Channel);

            Channel->setBufferSize(Graph->getDataChannel(i)->getBufferSize());
            Channel->setNumberOfInitialTokens(Graph->getDataChannel(i)->getNumberOfInitialTokens());
            Channel->setTokenSize(Graph->getDataChannel(i)->getTokenSize());

            Channels[NewDataChannelID] = Channel;
            NewDataChannelID++;
        }
        
    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {

        SADF_Channel* Channel = new SADF_Channel(CString(Graph->getControlChannel(i)->getName()), NumberOfActiveDataChannels + i, SADF_DATA_CHANNEL);

        SADF_Process* Source = NewGraph->getKernel(Graph->getControlChannel(i)->getSource()->getName());
        SADF_Process* Destination = NewGraph->getKernel(Graph->getControlChannel(i)->getDestination()->getName());

        Channel->setSource(Source);
        Channel->setDestination(Destination);
        Source->addOutputChannel(Channel);
        Destination->addInputChannel(Channel);

        Channel->setBufferSize(Graph->getControlChannel(i)->getBufferSize());
        Channel->setNumberOfInitialTokens(Graph->getControlChannel(i)->getNumberOfInitialTokens());
        Channel->setTokenSize(Graph->getControlChannel(i)->getTokenSize());

        Channels[NumberOfActiveDataChannels + i] = Channel;
    }

    NewGraph->setDataChannels(Channels);

    // Construct new kernel properties for original kernels
    
    for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
    
        SADF_Scenario* Default = new SADF_Scenario(CString(Graph->getKernel(i)->getScenario(Scenarios[i])->getName()), 0);

        // Construct consumptions

        CId NumberOfValidConsumptions = 0;

        for (CId j = 0; j != Graph->getKernel(i)->getScenario(Scenarios[i])->getNumberOfConsumptions(); j++)
            if (ActivityOnDataChannel[Graph->getKernel(i)->getScenario(Scenarios[i])->getConsumption(j)->getChannel()->getIdentity()])
                NumberOfValidConsumptions++;
        
        vector<SADF_Communication*> Consumptions(NumberOfValidConsumptions + Graph->getKernel(i)->getControlChannels().size());

        CId ConsumptionID = 0;

        for (CId j = 0; j != Graph->getKernel(i)->getScenario(Scenarios[i])->getNumberOfConsumptions(); j++)
            if (ActivityOnDataChannel[Graph->getKernel(i)->getScenario(Scenarios[i])->getConsumption(j)->getChannel()->getIdentity()]) {
                Consumptions[ConsumptionID] = new SADF_Communication(ConsumptionID, NewGraph->getChannel(Graph->getKernel(i)->getScenario(Scenarios[i])->getConsumption(j)->getChannel()->getName()), Graph->getKernel(i)->getScenario(Scenarios[i])->getConsumption(j)->getRate());
                ConsumptionID++;
            }

        for (list<SADF_Channel*>::iterator j = Graph->getKernel(i)->getControlChannels().begin(); j != Graph->getKernel(i)->getControlChannels().end(); j++) {
            Consumptions[ConsumptionID] = new SADF_Communication(ConsumptionID, NewGraph->getChannel((*j)->getName()), 1);
            ConsumptionID++;
        }
            
        Default->setConsumptions(Consumptions);

        // Construct productions
        
        CId NumberOfValidProductions = 0;
        
        for (CId j = 0; j != Graph->getKernel(i)->getScenario(Scenarios[i])->getNumberOfProductions(); j++)
            if (ActivityOnDataChannel[Graph->getKernel(i)->getScenario(Scenarios[i])->getProduction(j)->getChannel()->getIdentity()])
                NumberOfValidProductions++;

        vector<SADF_Communication*> Productions(NumberOfValidProductions);

        CId ProductionID = 0;

        for (CId j = 0; j != Graph->getKernel(i)->getScenario(Scenarios[i])->getNumberOfProductions(); j++)
            if (ActivityOnDataChannel[Graph->getKernel(i)->getScenario(Scenarios[i])->getProduction(j)->getChannel()->getIdentity()]) {
                Productions[ProductionID] = new SADF_Communication(ProductionID, NewGraph->getChannel(Graph->getKernel(i)->getScenario(Scenarios[i])->getProduction(j)->getChannel()->getName()), Graph->getKernel(i)->getScenario(Scenarios[i])->getProduction(j)->getRate());
                ProductionID++;
            }
            
        Default->setProductions(Productions);
        
        // Copy profiles

        vector<SADF_Profile*> Profiles(Graph->getKernel(i)->getScenario(Scenarios[i])->getNumberOfProfiles());
        
        for (CId j = 0; j != Graph->getKernel(i)->getScenario(Scenarios[i])->getNumberOfProfiles(); j++) {
            Profiles[j] = new SADF_Profile(j);
            Profiles[j]->setExecutionTime(Graph->getKernel(i)->getScenario(Scenarios[i])->getProfile(j)->getExecutionTime());
            Profiles[j]->setWeight(Graph->getKernel(i)->getScenario(Scenarios[i])->getProfile(j)->getWeight());
        }
        
        Default->setProfiles(Profiles);
        
        // Store constructed default scenario
        
        vector<SADF_Scenario*> KernelScenarios(1);
        KernelScenarios[0] = Default;
        Kernels[i]->setScenarios(KernelScenarios);
    }

    // Construct new kernel properties for original detectors

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
        
        SADF_Scenario* SubScenario = Graph->getDetector(i)->getSubScenario(SubScenarios[i]);
        
        SADF_Scenario* Default = new SADF_Scenario(CString(SubScenario->getName()), 0);

        // Construct consumptions

        CId NumberOfValidConsumptions = 0;
        
        for (CId j = 0; j != SubScenario->getNumberOfConsumptions(); j++)
            if (ActivityOnDataChannel[SubScenario->getConsumption(j)->getChannel()->getIdentity()])
                NumberOfValidConsumptions++;

        vector<SADF_Communication*> Consumptions(NumberOfValidConsumptions + Graph->getDetector(i)->getControlChannels().size());

        CId ConsumptionID = 0;

        for (CId j = 0; j != SubScenario->getNumberOfConsumptions(); j++)
            if (ActivityOnDataChannel[SubScenario->getConsumption(j)->getChannel()->getIdentity()]) {
                Consumptions[ConsumptionID] = new SADF_Communication(ConsumptionID, NewGraph->getChannel(SubScenario->getConsumption(j)->getChannel()->getName()), SubScenario->getConsumption(j)->getRate());
                ConsumptionID++;
            }

        for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getControlChannels().begin(); j != Graph->getDetector(i)->getControlChannels().end(); j++) {
            Consumptions[ConsumptionID] = new SADF_Communication(ConsumptionID, NewGraph->getChannel((*j)->getName()), 1);
            ConsumptionID++;
        }
            
        Default->setConsumptions(Consumptions);
        
        // Construct productions
        
        CId NumberOfValidProductions = 0;

        for (CId j = 0; j != SubScenario->getNumberOfProductions(); j++)
            if (SubScenario->getProduction(j)->getChannel()->getType() == SADF_DATA_CHANNEL) {
                if (ActivityOnDataChannel[SubScenario->getProduction(j)->getChannel()->getIdentity()])
                    NumberOfValidProductions++;
            } else
                NumberOfValidProductions++;

        vector<SADF_Communication*> Productions(NumberOfValidProductions);

        CId ProductionID = 0;

        for (CId j = 0; j != SubScenario->getNumberOfProductions(); j++)
            if (SubScenario->getProduction(j)->getChannel()->getType() == SADF_DATA_CHANNEL) {
                if (ActivityOnDataChannel[SubScenario->getProduction(j)->getChannel()->getIdentity()]) {
                    Productions[ProductionID] = new SADF_Communication(ProductionID, NewGraph->getChannel(SubScenario->getProduction(j)->getChannel()->getName()), SubScenario->getProduction(j)->getRate());
                    ProductionID++;
                }
            } else {
                Productions[ProductionID] = new SADF_Communication(ProductionID, NewGraph->getChannel(SubScenario->getProduction(j)->getChannel()->getName()), SubScenario->getProduction(j)->getRate());
                ProductionID++;
            }
            
        Default->setProductions(Productions);
        
        // Copy profiles
        
        vector<SADF_Profile*> Profiles(SubScenario->getNumberOfProfiles());
        
        for (CId j = 0; j != SubScenario->getNumberOfProfiles(); j++) {
            Profiles[j] = new SADF_Profile(j);
            Profiles[j]->setExecutionTime(SubScenario->getProfile(j)->getExecutionTime());
            Profiles[j]->setWeight(SubScenario->getProfile(j)->getWeight());
        }
        
        Default->setProfiles(Profiles);
        
        // Store constructed default scenario
        
        vector<SADF_Scenario*> DetectorScenarios(1);
        DetectorScenarios[0] = Default;
        Kernels[Graph->getNumberOfKernels() + i]->setScenarios(DetectorScenarios);
    }

    return NewGraph;
}
