/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_process.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Process
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

#include "sadf_process.h"

// Constructor

SADF_Process::SADF_Process(const CString &N, const CId ID, const CId T) : SADF_Component(N, ID, T) { }

// Destructor

SADF_Process::~SADF_Process() {

	for (CId i = 0; i != Scenarios.size(); i++)
		delete Scenarios[i];
		
	for (CId i = 0; i != SubScenarios.size(); i++)
		delete SubScenarios[i];
}

// Access to instance variables

void SADF_Process::setScenarios(vector<SADF_Scenario*> S) {

	Scenarios = S;
	
	ActiveInScenario.resize(Scenarios.size());
	
	for (CId i = 0; i != Scenarios.size(); i++)
		ActiveInScenario[i] = true;
}

SADF_Channel* SADF_Process::getInputChannel(const CString &ChannelName) {

	for (list<SADF_Channel*>::iterator i = InputChannels.begin(); i != InputChannels.end(); i++)
		if ((*i)->getName() == ChannelName)
			return *i;
	
	return NULL;
}

SADF_Channel* SADF_Process::getOutputChannel(const CString &ChannelName) {

	for (list<SADF_Channel*>::iterator i = OutputChannels.begin() ; i != OutputChannels.end(); i++)
		if ((*i)->getName() == ChannelName)
			return *i;
	
	return NULL;
}

SADF_Channel* SADF_Process::getControlChannel(const CString &ChannelName) {

	for (list<SADF_Channel*>::iterator i = ControlChannels.begin(); i != ControlChannels.end(); i++)
		if ((*i)->getName() == ChannelName)
			return *i;
	
	return NULL;
}

SADF_Scenario* SADF_Process::getScenario(const CString &ScenarioName) {

	for (CId i = 0; i != Scenarios.size(); i++)
		if (Scenarios[i]->getName() == ScenarioName)
			return Scenarios[i];

	return NULL;
}

SADF_Scenario* SADF_Process::getSubScenario(const CString &SubScenarioName) {

	for (CId i = 0; i != SubScenarios.size(); i++)
		if (SubScenarios[i]->getName() == SubScenarioName)
			return SubScenarios[i];

	return NULL;
}

CId SADF_Process::getConsumptionRate(const CId ChannelID, const CId ScenarioID) {

	if (getType() == SADF_KERNEL) {

		for (CId i = 0; i != Scenarios[ScenarioID]->getNumberOfConsumptions(); i++)
			if (Scenarios[ScenarioID]->getConsumption(i)->getChannel()->getIdentity() == ChannelID)
				return Scenarios[ScenarioID]->getConsumption(i)->getRate();

	} else {

		for (CId i = 0; i != SubScenarios[ScenarioID]->getNumberOfConsumptions(); i++)
			if (SubScenarios[ScenarioID]->getConsumption(i)->getChannel()->getIdentity() == ChannelID)
				return SubScenarios[ScenarioID]->getConsumption(i)->getRate();
	}
	
	return 0;
}

CId SADF_Process::getProductionRate(const CId ChannelID, const CId ChannelType, const CId ScenarioID) {

	if (getType() == SADF_KERNEL) {
	
		for (CId i = 0; i != Scenarios[ScenarioID]->getNumberOfProductions(); i++)
			if (Scenarios[ScenarioID]->getProduction(i)->getChannel()->getIdentity() == ChannelID)
				return Scenarios[ScenarioID]->getProduction(i)->getRate();

	} else {
	
		for (CId i = 0; i != SubScenarios[ScenarioID]->getNumberOfProductions(); i++)
			if (SubScenarios[ScenarioID]->getProduction(i)->getChannel()->getIdentity() == ChannelID && SubScenarios[ScenarioID]->getProduction(i)->getChannel()->getType() == ChannelType)
				return SubScenarios[ScenarioID]->getProduction(i)->getRate();
	}
	
	return 0;
}

// Basic properties

bool SADF_Process::isAlwaysActive() {

	if (getType() == SADF_KERNEL) {

		bool Active = true;
	
		for (CId i = 0; i != ActiveInScenario.size(); i++)
			Active = Active && ActiveInScenario[i];
	
		return Active;
	} else
		return true;
}
