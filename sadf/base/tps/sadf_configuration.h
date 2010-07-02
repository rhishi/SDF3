/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_configuration.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Configuration
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

#ifndef SADF_CONFIGURATION_H_INCLUDED
#define SADF_CONFIGURATION_H_INCLUDED

// Include type definitions

#include "../sadf/sadf_graph.h"
#include "sadf_channel_status.h"
#include "sadf_control_status.h"
#include "sadf_transition.h"

// Forward declarations

class SADF_TPS;
class SADF_KernelStatus;
class SADF_DetectorStatus;

// SADF_Configuration Definition

class SADF_Configuration : public SADF_Component {

public:
	// Constructors
	
	SADF_Configuration(SADF_Graph* Graph, SADF_TPS* TPS, CId StepType);
	SADF_Configuration(SADF_Graph* Graph, SADF_Configuration* C, CId StepType, CDouble StepValue);
	
	// Destructor
	
	~SADF_Configuration();

	// Hash key and Step Value
	
	void computeHashKey();
	CDouble getHashKey() const { return HashKey; };

	// Access to current status
	
	SADF_KernelStatus* getKernelStatus(const CId KernelID) const { return KernelStatus[KernelID]; };
	SADF_DetectorStatus* getDetectorStatus(const CId DetectorID) const { return DetectorStatus[DetectorID]; };
	SADF_ChannelStatus* getChannelStatus(const CId DataChannelID) const { return ChannelStatus[DataChannelID]; };
	SADF_ControlStatus* getControlStatus(const CId ControlChannelID) { return ControlStatus[ControlChannelID]; };

	void deleteContent();

	// Transitions - common functions for all configuration types

	list<SADF_Transition*>& getTransitions() { return Transitions; };
	void addTransition(SADF_Configuration* Destination, const CDouble Probability, const CDouble TimeSample);
	SADF_Transition* hasTransitionToConfigurationWithIdentity(const CId ID);
	CDouble getTransitionProbabilityTo(SADF_Configuration* Destination);
	void removeAllTransitions();							// Does not delete transitions, only empties the list of pointers
	void deleteAllTransitions();							// Deletes all transitions
	void deleteTransitionsToIrrelevantConfigurations();

	// Step Value

	CDouble getStepValue() const { return StepValue; };

	// Minimal remaining execution time
	
	void setMinimalRemainingExecutionTime(const CDouble T) { MinimalRemainingExecutionTime = T; };
	CDouble getMinimalRemainingExecutionTime() const { return MinimalRemainingExecutionTime; };

	// Advancing time
	
	SADF_Configuration* time(SADF_Graph* Graph, SADF_TPS* TPS, bool RelevantStep);

	// Marking
	
	void setMarking(const bool B) { Marking = B; };
	bool isMarked() const { return Marking; };

	// Relevance
	
	void setRelevance(const bool R) { Relevant = R; };
	bool isRelevant() const { return Relevant; };

	// Performance results
	
	void initialiseLocalResults(const CId NumberOfResults) { LocalResults.resize(NumberOfResults); };
	void setLocalResult(const CId Index, const CDouble Result) { LocalResults[Index] = Result; };
	CDouble getLocalResult(const CId Index) const { return LocalResults[Index]; };

	// Equality operator
	
	bool equal(SADF_Configuration* C);

	// Print for debug
	
	void print(ostream& out);

private:
	// Instance Variables

	CDouble HashKey;
	CDouble StepValue;
	CDouble MinimalRemainingExecutionTime;

	vector<SADF_KernelStatus*> KernelStatus;
	vector<SADF_DetectorStatus*> DetectorStatus;
	vector<SADF_ChannelStatus*> ChannelStatus;
	vector<SADF_ControlStatus*> ControlStatus;

	list<SADF_Transition*> Transitions;
	
	bool Marking;
	bool Relevant;
	
	vector<CDouble> LocalResults;
};

// Type definitions

typedef list<SADF_Configuration*> SADF_ListOfConfigurations;
typedef list<SADF_ListOfConfigurations> SADF_HashedListOfConfigurations;

#endif
