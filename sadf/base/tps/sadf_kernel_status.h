/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_kernel_status.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Kernel Status
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

#ifndef SADF_KERNEL_STATUS_H_INCLUDED
#define SADF_KERNEL_STATUS_H_INCLUDED

// Include type definitions

#include "../sadf/sadf_graph.h"
#include "sadf_configuration.h"

// Forward Declarations

class SADF_KernelState;

// SADF_KernelTransition Definition

class SADF_KernelTransition {

public:
	// Constructor

	SADF_KernelTransition(SADF_KernelState* D, const CDouble P);
	
	// Destructor

	~SADF_KernelTransition() { };

	// Access to instance variables

	CDouble getProbability() const { return Probability; };
	SADF_KernelState* getDestination() const { return Destination; };

private:
	// Instance Variables

	SADF_KernelState* Destination;
	CDouble	Probability;
};

// SADF_KernelState Definition

class SADF_KernelState : public SADF_Component {

public:
	// Constructors
	
	SADF_KernelState(SADF_Process* K, CId ID, CId ScenarioID, CId ActionType, CDouble Time);
	
	// Destructor
	
	~SADF_KernelState();

	// Access to instance variables
	
	SADF_Process* getKernel() const { return Kernel; };
	
	void addTransition(SADF_KernelState* D, CDouble P);

	CId getScenario() const { return Scenario; };
	CDouble getExecutionTime() const { return ExecutionTime; };

	list<SADF_KernelTransition*>& getTransitions() { return Transitions; };
	
private:

	// Instance Variables

	SADF_Process* Kernel;

	CId Scenario;
	CDouble ExecutionTime;
	
	list<SADF_KernelTransition*> Transitions;
};

// SADF_KernelStatus Definition

class SADF_KernelStatus {

public:
	// Constructor
	
	SADF_KernelStatus(SADF_Configuration* C, SADF_KernelState* S, CDouble T);
	SADF_KernelStatus(SADF_Configuration* C, SADF_KernelStatus* S);

	// Destructor
	
	~SADF_KernelStatus() { };

	// Access to instance variables
	
	SADF_Configuration* getConfiguration() const { return Configuration; };
	
	void setState(SADF_KernelState* S) { State = S; RemainingExecutionTime = State->getExecutionTime(); };
	SADF_KernelState* getState() const { return State; };
	void setRemainingExecutionTime(CDouble T) { RemainingExecutionTime = T; };
	CDouble getRemainingExecutionTime() const { return RemainingExecutionTime; };

	// Functions to determine current status
	
	bool isReadyToFire();
	bool isReadyToStart();
	bool isReadyToEnd();

	// Functions to change status
	
	SADF_ListOfConfigurations control(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep);
	SADF_ListOfConfigurations start(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep);
	SADF_ListOfConfigurations end(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep);

	// Equality operator
	
	bool equal(SADF_KernelStatus* S);
	
private:
	// Instance Variables
	
	SADF_Configuration* Configuration;
	
	SADF_KernelState* State;
	CDouble RemainingExecutionTime;
};

#endif
