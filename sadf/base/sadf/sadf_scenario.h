/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_scenario.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Scenario
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

#ifndef SADF_SCENARIO_H_INCLUDED
#define SADF_SCENARIO_H_INCLUDED

// Include type definitions

#include "sadf_communication.h"
#include "sadf_profile.h"
#include "sadf_control.h"
#include "sadf_markovchain.h"

// SADF_Scenario Definition

class SADF_Scenario : public SADF_Component {

public:
	// Constructor

	SADF_Scenario(const CString &N, const CId ID);

	// Destructor

	virtual ~SADF_Scenario();

	// Access to instance variables
	
	void setConsumptions(vector<SADF_Communication*> C) { Consumptions = C; };
	void setProductions(vector<SADF_Communication*> P) { Productions = P; };
	void setProfiles(vector<SADF_Profile*> P) { Profiles = P; };
	void setControls(vector<SADF_Control*> C) { Controls = C; };
	void setMarkovChain(SADF_MarkovChain* M) { MarkovChain = M; };

	SADF_Communication* getConsumption(const CId ID) { return Consumptions[ID]; };
	SADF_Communication* getProduction(const CId ID) { return Productions[ID]; };
	SADF_Profile* getProfile(const CId ID) { return Profiles[ID]; };
	SADF_Control* getControl(const CId ID) { return Controls[ID]; };
	SADF_MarkovChain* getMarkovChain() { return MarkovChain; };

	CId getNumberOfProductions() const { return Productions.size(); };
	CId getNumberOfConsumptions() const { return Consumptions.size(); };
	CId getNumberOfProfiles() const { return Profiles.size(); };
	CId getNumberOfControls() const { return Controls.size(); };

	CString& getValueReceivedFromChannel(const CId ControlChannelID);
	CId getScenarioIdentityReceivedFromChannel(const CId ControlChannelID);
	CString& getValueProducedToChannel(const CId OutputChannelID);
	CId getScenarioIdentityProducedToChannel(const CId OutputChannelID);

	void deleteAllProfiles();

private:
	// Instance variables
	
	vector<SADF_Communication*> Consumptions;
	vector<SADF_Communication*> Productions;
	vector<SADF_Profile*> Profiles;
	vector<SADF_Control*> Controls;

	SADF_MarkovChain* MarkovChain;
};

#endif
