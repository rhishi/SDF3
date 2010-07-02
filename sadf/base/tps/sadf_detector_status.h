/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_detector_status.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Detector Status
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

#ifndef SADF_DETECTOR_STATUS_H_INCLUDED
#define SADF_DETECTOR_STATUS_H_INCLUDED

// Include type definitions

#include "../sadf/sadf_graph.h"
#include "sadf_configuration.h"

// Forward Declarations

class SADF_DetectorState;

// SADF_DetectorTransition Definition

class SADF_DetectorTransition {

public:
	// Constructor

	SADF_DetectorTransition(SADF_DetectorState* D, const CDouble P);
	
	// Destructor

	~SADF_DetectorTransition() { };

	// Access to instance variables

	CDouble getProbability() const { return Probability; };
	SADF_DetectorState* getDestination() const { return Destination; };

private:
	// Instance Variables

	SADF_DetectorState* Destination;
	CDouble	Probability;
};

// SADF_DetectorState Definition

class SADF_DetectorState : public SADF_Component {

public:
	// Constructors
	
	SADF_DetectorState(SADF_Process* D, CId ID, CId ScenarioID, CId SubScenarioID, vector<CId> MCStatus, CId ActionType, CDouble Time);
	
	// Destructor
	
	~SADF_DetectorState();

	// Access to instance variables

	SADF_Process* getDetector() const { return Detector; };
	
	void addTransition(SADF_DetectorState* D, CDouble P);

	CId getScenario() const { return Scenario; };
	CId getSubScenario() const { return SubScenario; };
	CDouble getExecutionTime() const { return ExecutionTime; };
	CId getStateOfMarkovChain(const CId MarkovChainID) const { return MarkovChainStatus[MarkovChainID]; };
	vector<CId>& getMarkovChainStatus() { return MarkovChainStatus; };

	list<SADF_DetectorTransition*>& getTransitions() { return Transitions; };

private:

	// Instance Variables
	
	SADF_Process* Detector;

	CId Scenario;
	CId SubScenario;
	CDouble ExecutionTime;
	vector<CId> MarkovChainStatus;
	
	list<SADF_DetectorTransition*> Transitions;
};

// SADF_DetectorStatus Definition

class SADF_DetectorStatus {

public:
	// Constructor
	
	SADF_DetectorStatus(SADF_Configuration* C, SADF_DetectorState* S, CDouble T);
	SADF_DetectorStatus(SADF_Configuration* C, SADF_DetectorStatus* S);

	// Destructor
	
	~SADF_DetectorStatus() { };

	// Access to instance variables
	
	SADF_Configuration* getConfiguration() const { return Configuration; };
	
	void setState(SADF_DetectorState* S) { State = S; RemainingExecutionTime = State->getExecutionTime(); };
	SADF_DetectorState* getState() const { return State; };
	void setRemainingExecutionTime(CDouble T) { RemainingExecutionTime = T; };
	CDouble getRemainingExecutionTime() const { return RemainingExecutionTime; };

	// Functions to determine current status
	
	bool isReadyToFire();
	bool isReadyToStart();
	bool isReadyToEnd();

	// Functions to change status
	
	SADF_ListOfConfigurations detect(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep);
	SADF_ListOfConfigurations start(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep);
	SADF_ListOfConfigurations end(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep);

	// Equality operator
	
	bool equal(SADF_DetectorStatus* S);
	
private:
	// Instance Variables
	
	SADF_Configuration* Configuration;
	
	SADF_DetectorState* State;
	CDouble RemainingExecutionTime;
};

#endif
