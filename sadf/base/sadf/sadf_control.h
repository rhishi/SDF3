/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_control.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Control
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

#ifndef SADF_CONTROL_H_INCLUDED
#define SADF_CONTROL_H_INCLUDED

// Include type definitions

#include "sadf_channel.h"

// SADF_Control Definition

class SADF_Control : public SADF_Component {

public:
	// Constructor

	SADF_Control(const CId ID, SADF_Channel* C, const CString &V);

	// Destructor

	~SADF_Control() { };

	// Access to instance variables

	void setScenarioIdentity(const CId ID) { ScenarioIdentity = ID; };
	
	SADF_Channel* getChannel() const { return Channel; };
	CString& getValue() { return Value; };
	CId getScenarioIdentity() { return ScenarioIdentity; };
	
private:
	// Instance variables
	
	SADF_Channel* Channel;

	CString Value;
	CId ScenarioIdentity;
};

#endif
