/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_control_status.h
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

#ifndef SADF_CONTROL_STATUS_H_INCLUDED
#define SADF_CONTROL_STATUS_H_INCLUDED

// Include type definitions

#include "../sadf/sadf_graph.h"

// SADF_ControlStatus Definition

class SADF_ControlStatus {

public:
	// Contructor
	
	SADF_ControlStatus(SADF_Channel* C);
	SADF_ControlStatus(SADF_ControlStatus* S);

	// Destructor

	~SADF_ControlStatus() { };

	// Access to current status

	CId getAvailableTokens() const { return AvailableTokens; };
	CId getReservedLocations() const { return ReservedLocations; };
	CId getOccupation() const { return AvailableTokens + ReservedLocations; };

	CQueue& getContentQueue() { return ContentQueue; };
	CQueue& getNumbersQueue() { return NumbersQueue; };

	// Functions to change status

	void reserve(const CId NumberOfTokens);
	void write(const CId NumberOfTokens, const CId ScenarioID);
	CId inspect();
	void remove();

	// Equality operator
	
	bool equal(SADF_ControlStatus* S);

protected:
	// instance variables

	CId AvailableTokens;
	CId ReservedLocations;
	
	CQueue ContentQueue;
	CQueue NumbersQueue;

};

#endif
