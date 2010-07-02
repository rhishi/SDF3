/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_markovchain.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Markov Chain
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

#ifndef SADF_MARKOVCHAIN_H_INCLUDED
#define SADF_MARKOVCHAIN_H_INCLUDED

// Include type definitions

#include "sadf_component.h"

// Forward definitions

class SADF_Scenario;

// SADF_MarkovChainState Definition

class SADF_MarkovChainState : public SADF_Component {

public:
	// Constructors
	
	SADF_MarkovChainState(const CString &N, const CId ID, SADF_Scenario* S);
	
	// Destructor
	
	~SADF_MarkovChainState() { };

	// Access to instance variables
	
	SADF_Scenario* getSubScenario() { return SubScenario; };

private:
	// Instance Variables

	SADF_Scenario* SubScenario;
};

// SADF_MarkovChain Definition

class SADF_MarkovChain {

public:
	// Constructors
	
	SADF_MarkovChain();

	// Destructor
	
	virtual ~SADF_MarkovChain();

	// Access to instance variables
	
	void setInitialState(SADF_MarkovChainState* State) { InitialState = State; };
	void setStateSpace(vector<SADF_MarkovChainState*> S) { StateSpace = S; };
	void setTransitionMatrix(vector< vector<CDouble> > M) { TransitionMatrix = M; };

	SADF_MarkovChainState* getInitialState() const { return InitialState; };
	SADF_MarkovChainState* getState(const CId StateID) { return StateSpace[StateID]; };
	SADF_MarkovChainState* getState(const CString &StateName);
	CId getNumberOfStates() const { return StateSpace.size(); };
	CDouble getTransitionProbability(const CId SourceID, const CId DestinationID) const { return TransitionMatrix[SourceID][DestinationID]; };

    vector<CDouble> computeEquilibriumDistribution();

	// Functions to determine properties
	
	bool isSingleComponent();			        // Used by parser
	bool isSingleStronglyConnectedComponent();	// Used by function isDeterministicCycle() and verification of ergodicity
	bool isDeterministicCycle();			    // Used for determining whether SADF Graph is actually a CSDF Graph

private:
	// Instance variables
	
	SADF_MarkovChainState* InitialState;
	vector<SADF_MarkovChainState*> StateSpace;
	vector< vector<CDouble> > TransitionMatrix;
	
	// Functions
	
	void colorConnectedStates(CId StateID, vector<CId> &Color);
	void colorReverselyConnectedStates(CId StateID, vector<CId> &Color);
};

#endif
