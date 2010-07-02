/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_process.h
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

#ifndef SADF_PROCESS_H_INCLUDED
#define SADF_PROCESS_H_INCLUDED

// Include type definitions

#include "sadf_channel.h"
#include "sadf_scenario.h"

// Forward declarations

class SADF_Graph;

// SADF_Process Definition

class SADF_Process : public SADF_Component {

public:
	// Constructor
	SADF_Process(const CString &N, const CId ID, const CId T);
	
	// Destructor
	virtual ~SADF_Process();

	// Access to instance variables
	
	void addInputChannel(SADF_Channel* Channel) { InputChannels.push_front(Channel); };
	void addOutputChannel(SADF_Channel* Channel) { OutputChannels.push_front(Channel); };
	void addControlChannel(SADF_Channel* Channel) { ControlChannels.push_front(Channel); };
	void setScenarios(vector<SADF_Scenario*> S);						// Resets scenario activity status of kernel
	void setSubScenarios(vector<SADF_Scenario*> S) { SubScenarios = S; };
	
	SADF_Channel* getInputChannel(const CString &ChannelName);
	SADF_Channel* getOutputChannel(const CString &ChannelName);
	SADF_Channel* getControlChannel(const CString &ChannelName);
	SADF_Scenario* getScenario(const CString &ScenarioName);
	SADF_Scenario* getSubScenario(const CString &SubScenarioName);

	SADF_Scenario* getScenario(const CId ID) { return Scenarios[ID]; };
	SADF_Scenario* getSubScenario(const CId ID) { return SubScenarios[ID]; };

	bool hasControlChannels() const { return !ControlChannels.empty(); };
	bool hasControls() const { return ControlChannels.size() > 1; };

	list<SADF_Channel*>& getInputChannels() { return InputChannels; };
	list<SADF_Channel*>& getOutputChannels() { return OutputChannels; };
	list<SADF_Channel*>& getControlChannels() { return ControlChannels; };
	CId getNumberOfScenarios() const { return Scenarios.size(); };
	CId getNumberOfSubScenarios() const { return SubScenarios.size(); };

	CId getConsumptionRate(const CId ChannelID, const CId ScenarioID);
	CId getProductionRate(const CId ChannelID, const CId ChannelType, const CId ScenarioID);

	// Functions for kernels

	void deactivate(const CId ScenarioID) { ActiveInScenario[ScenarioID] = false; };
	bool isActive(const CId ScenarioID) const { return ActiveInScenario[ScenarioID]; };
	bool isAlwaysActive();

private:
	// Instance variables

	list<SADF_Channel*> InputChannels;
	list<SADF_Channel*> OutputChannels;
	list<SADF_Channel*> ControlChannels;

	vector<SADF_Scenario*> Scenarios;
	vector<SADF_Scenario*> SubScenarios;
	
	vector<bool> ActiveInScenario;
};

#endif
