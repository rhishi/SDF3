/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_control_status.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Control Status
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

#include "sadf_control_status.h"

// Constructors

SADF_ControlStatus::SADF_ControlStatus(SADF_Channel* C) { 

	ReservedLocations = 0;
	AvailableTokens = C->getNumberOfInitialTokens();
	
	NumbersQueue = C->getNumbersQueue();
	ContentQueue = C->getContentQueue();
}

SADF_ControlStatus::SADF_ControlStatus(SADF_ControlStatus* S) {
	
	ReservedLocations = S->getReservedLocations();
	AvailableTokens = S->getAvailableTokens();

	NumbersQueue = S->getNumbersQueue();
	ContentQueue = S->getContentQueue();
}

// Functions to change status

void SADF_ControlStatus::reserve(const CId NumberOfTokens) {

	ReservedLocations += NumberOfTokens;
}

void SADF_ControlStatus::write(const CId NumberOfTokens, const CId ScenarioID) {

	ReservedLocations -= NumberOfTokens;
	AvailableTokens += NumberOfTokens;
	
	if (!ContentQueue.empty()) {
		if (ContentQueue.back() == ScenarioID)
			NumbersQueue.back() += NumberOfTokens;
		else {
			NumbersQueue.push(NumberOfTokens);
			ContentQueue.push(ScenarioID);
		}
	} else {
		NumbersQueue.push(NumberOfTokens);
		ContentQueue.push(ScenarioID);
	}
}

CId SADF_ControlStatus::inspect() {

	return ContentQueue.front();
}

void SADF_ControlStatus::remove() {

	AvailableTokens--;
	NumbersQueue.front()--;

	if (NumbersQueue.front() == 0) {
		NumbersQueue.pop();
		ContentQueue.pop();
	}
}

// Equality operator

bool SADF_ControlStatus::equal(SADF_ControlStatus* S) {

	return AvailableTokens == S->getAvailableTokens() && NumbersQueue == S->getNumbersQueue() && ContentQueue == S->getContentQueue();

	// Checking equality of available tokens is to speed up the check
	// Equality of reserved locations follows from equality of status of writing process
}
