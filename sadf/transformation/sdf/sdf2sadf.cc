/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf2sadf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert SDF Graph in SADF Graph
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

#include "sdf2sadf.h"

SADF_Graph* SDF2SADF(TimedSDFgraph* SDF) {

	SADF_Graph* SADF = new SADF_Graph(SDF->getName(), SDF->getId());

	// Create Kernels with default scenario

	vector<SADF_Process*> Kernels((CId)(SDF->nrActors()));

	CId KernelID = 0;

	for (SDFactorsIter i = SDF->actorsBegin(); i != SDF->actorsEnd(); i++) {
		TimedSDFactor* Actor = (TimedSDFactor*)(*i);
		SADF_Process* Kernel = new SADF_Process(Actor->getName(), KernelID, SADF_KERNEL);
		vector<SADF_Scenario*> Scenarios(1);
		SADF_Scenario* Scenario = new SADF_Scenario("default", 0);
		vector<SADF_Profile*> Profiles(1);
		SADF_Profile* Profile = new SADF_Profile(0);

		for (TimedSDFactor::ProcessorsIter j = Actor->processorsBegin(); j != Actor->processorsEnd(); j++)
			if (Actor->getDefaultProcessor() == (*j)->type)
				Profile->setExecutionTime((CDouble)((*j)->execTime));

		Profiles[0] = Profile;
		Scenario->setProfiles(Profiles);
		Scenarios[0] = Scenario;
		Kernel->setScenarios(Scenarios);
		Kernels[KernelID] = Kernel;
		KernelID++;
	}
		
	SADF->setKernels(Kernels);

	// Create Data Channels
	
	vector<SADF_Channel*> DataChannels((CId)(SDF->nrChannels()));
	
	CId DataChannelID = 0;
	
	for (SDFchannelsIter i = SDF->channelsBegin(); i != SDF->channelsEnd(); i++) {
		TimedSDFchannel* Channel = (TimedSDFchannel*)(*i);
		SADF_Channel* DataChannel = new SADF_Channel(Channel->getName(), DataChannelID, SADF_DATA_CHANNEL);
		
		if (Channel->isConnected()) {
			DataChannel->setSource(SADF->getKernel(Channel->getSrcActor()->getName()));
			DataChannel->setDestination(SADF->getKernel(Channel->getDstActor()->getName()));
		} else
			throw CException((CString)("Channel '") + Channel->getName() + "' of SDF graph '" + SDF->getName() + "' is not connected.");
		
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

	for (SDFactorsIter i = SDF->actorsBegin(); i != SDF->actorsEnd(); i++) {
		TimedSDFactor* Actor = (TimedSDFactor*)(*i);
		SADF_Process* Kernel = SADF->getKernel(Actor->getName());

		CId NumberOfConsumptions = 0;
		CId NumberOfProductions = 0;

		for (SDFportsIter j = Actor->portsBegin(); j != Actor->portsEnd(); j++) {
			if ((*j)->getType() == SDFport::In)
				NumberOfConsumptions++;
			if ((*j)->getType() == SDFport::Out)
				NumberOfProductions++;
		}
		
		vector<SADF_Communication*> Consumptions(NumberOfConsumptions);
		vector<SADF_Communication*> Productions(NumberOfProductions);
		
		CId ConsumptionID = 0;
		CId ProductionID = 0;
		
		for (SDFportsIter j = Actor->portsBegin(); j != Actor->portsEnd(); j++) {
			
			if ((*j)->getType() == SDFport::In) {
				if ((*j)->isConnected()) {			
					SADF->getKernel(Actor->getName())->addInputChannel(SADF->getChannel((*j)->getChannel()->getName()));
					Consumptions[ConsumptionID] = new SADF_Communication(ConsumptionID, SADF->getChannel((*j)->getChannel()->getName()), (*j)->getRate());
					ConsumptionID++;
				} else
					throw CException((CString)("Actor '") + Actor->getName() + "' of SDF graph '" + SDF->getName() + "' consumes from unknown channel through port '" + (*j)->getName() + "'.");
			}
			
			if ((*j)->getType() == SDFport::Out) {
				if ((*j)->isConnected()) {			
					SADF->getKernel(Actor->getName())->addOutputChannel(SADF->getChannel((*j)->getChannel()->getName()));
					Productions[ProductionID] = new SADF_Communication(ProductionID, SADF->getChannel((*j)->getChannel()->getName()), (*j)->getRate());
					ProductionID++;
				} else
					throw CException((CString)("Actor '") + Actor->getName() + "' of SDF graph '" + SDF->getName() + "' produces onto unknown channel through port '" + (*j)->getName() + "'.");
			}
			
		}
		
		Kernel->getScenario(0)->setConsumptions(Consumptions);
		Kernel->getScenario(0)->setProductions(Productions);
	}

	return SADF;
}
