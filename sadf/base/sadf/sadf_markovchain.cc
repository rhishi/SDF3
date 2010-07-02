/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_markovchain.cc
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

#include "sadf_markovchain.h"

// Constructors

SADF_MarkovChainState::SADF_MarkovChainState(const CString &N, const CId ID, SADF_Scenario* S) : SADF_Component(N, ID) { 

	SubScenario = S;
}

SADF_MarkovChain::SADF_MarkovChain() { }

// Destructor

SADF_MarkovChain::~SADF_MarkovChain() {

	for (CId i = 0; i != StateSpace.size(); i++)
		delete StateSpace[i];
}

// Access to instance variables

SADF_MarkovChainState* SADF_MarkovChain::getState(const CString &StateName) {

	for (CId i = 0; i != StateSpace.size(); i++)
		if (StateSpace[i]->getName() == StateName)
			return StateSpace[i];

	return NULL;
}

// Functions to determine whether Markov chain consists of single component

void SADF_MarkovChain::colorConnectedStates(CId StateID, vector<CId> &Color) {

	Color[StateID] = 1;
	
	for (CId i = 0; i != TransitionMatrix.size(); i++)
		if (TransitionMatrix[StateID][i] > 0 && Color[i] == 0)
			colorConnectedStates(i, Color);
}
	
bool SADF_MarkovChain::isSingleComponent() {

	vector<CId> Color(StateSpace.size(), 0);
	
	colorConnectedStates(InitialState->getIdentity(), Color);
	
	for (CId i = 0; i != StateSpace.size(); i++)
		if (Color[i] == 0)
			return false;

	return true;
}

// Functions to determine whether Markov chain is strongly connected

void SADF_MarkovChain::colorReverselyConnectedStates(CId StateID, vector<CId> &Color) {

	Color[StateID] = 1;
	
	for (CId i = 0; i != TransitionMatrix.size(); i++)
		if (TransitionMatrix[i][StateID] > 0 && Color[i] == 0)
			colorReverselyConnectedStates(i, Color);
}

bool SADF_MarkovChain::isSingleStronglyConnectedComponent() {

	// Markov Chain is assumed to be a single component as checked by the parser
	
	vector<CId> Color(StateSpace.size(), 0);

	colorReverselyConnectedStates(InitialState->getIdentity(), Color);
	
	for (CId i = 0; i != StateSpace.size(); i++)
		if (Color[i] == 0)
			return false;

	return true;
}

// Functions to determine whether Markov chain is single simple cyclic strongly connected component

bool SADF_MarkovChain::isDeterministicCycle() {

	bool Deterministic = isSingleStronglyConnectedComponent();

	for (CId i = 0; Deterministic && i != TransitionMatrix.size(); i++)
		for (CId j = 0; j != TransitionMatrix.size(); j++)
			if (TransitionMatrix[i][j] > 0)
				if (TransitionMatrix[i][j] < 1)
					return false;

	return true;
}

// Function to compute equilibrium distribution

vector<CDouble> SADF_MarkovChain::computeEquilibriumDistribution() {

	// Initialise augmented matrix

    SparseMatrix* Matrix = new SparseMatrix(StateSpace.size());

    for (CId i = 0; i != StateSpace.size(); i++)
        for (CId j = 0; j != StateSpace.size() - 1; j++)
            if (TransitionMatrix[i][j] > 0)
                Matrix->set(j, i, TransitionMatrix[i][j]);

	for (CId i = 0; i != StateSpace.size() + 1; i++) {
		
		Matrix->set(StateSpace.size() - 1, i, 1);
		
		if (i < StateSpace.size() - 1)
			Matrix->set(i, i, Matrix->get(i, i) - 1);
	}

    vector<CDouble> EquilibriumDistribution = Matrix->computeEigenVector();

    delete Matrix;

    return EquilibriumDistribution;
}
