/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_scenario.cc
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

#include "sadf_scenario.h"

// Constructor

SADF_Scenario::SADF_Scenario(const CString &N, const CId ID) : SADF_Component(N, ID) {

	MarkovChain = NULL;
}

// Destructor

SADF_Scenario::~SADF_Scenario() {

	CId i;

	for (i = 0; i != Consumptions.size(); i++)
		delete Consumptions[i];

	for (i = 0; i != Productions.size(); i++)
		delete Productions[i];

	for (i = 0; i != Profiles.size(); i++)
		delete Profiles[i];

	for (i = 0; i != Controls.size(); i++)
		delete Controls[i];
		
	if (MarkovChain != NULL)
		delete MarkovChain;
}

// Access to instance variables

CString SADF_EmptyString = "";

CString& SADF_Scenario::getValueReceivedFromChannel(const CId ControlChannelID) {

	for (CId i = 0; i != Controls.size(); i++)
		if (Controls[i]->getChannel()->getIdentity() == ControlChannelID)
			return Controls[i]->getValue();

	return SADF_EmptyString;
}

CString& SADF_Scenario::getValueProducedToChannel(const CId OutputChannelID) {

	for (CId i = 0; i != Productions.size(); i++)
		if (Productions[i]->getChannel()->getType() == SADF_CONTROL_CHANNEL && Productions[i]->getChannel()->getIdentity() == OutputChannelID)
			return Productions[i]->getValue();

	return SADF_EmptyString;
}

CId SADF_Scenario::getScenarioIdentityReceivedFromChannel(const CId ControlChannelID) {

	for (CId i = 0; i != Controls.size(); i++)
		if (Controls[i]->getChannel()->getIdentity() == ControlChannelID)
			return Controls[i]->getScenarioIdentity();

	return SADF_UNDEFINED;
}

CId SADF_Scenario::getScenarioIdentityProducedToChannel(const CId OutputChannelID) {

	for (CId i = 0; i != Productions.size(); i++)
		if (Productions[i]->getChannel()->getType() == SADF_CONTROL_CHANNEL && Productions[i]->getChannel()->getIdentity() == OutputChannelID)
			return Productions[i]->getScenarioIdentity();

	return SADF_UNDEFINED;
}

void SADF_Scenario::deleteAllProfiles() {

	for (CId i = 0; i != Profiles.size(); i++)
		delete Profiles[i];
}
