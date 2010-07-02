/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_buffer_occupancy.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   17 October 2006
 *
 *  Function        :   Analysis of buffer occupancy
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

#include "sadf_buffer_occupancy.h"

// Function to construct TPS for analysing buffer occupancy metrics

void SADF_ConstructTPS_BufferOccupancy(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId ChannelType, CId ChannelID, CId SourceProcessType, CId SourceProcessID, CId DestinationProcessType, CId DestinationProcessID) {

	SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP_BufferOccupancy(Graph, TPS, Source, SourceProcessType, SourceProcessID, DestinationProcessType, DestinationProcessID);

	// Determine local results

	Source->initialiseLocalResults(2);

	CDouble AverageTime = 0;

	for (list<SADF_Transition*>::iterator i = Source->getTransitions().begin(); i != Source->getTransitions().end(); i++)
		AverageTime += (*i)->getProbability() * (*i)->getTimeSample();

	Source->setLocalResult(0, AverageTime);
	
	if (ChannelType == SADF_DATA_CHANNEL)
		Source->setLocalResult(1, Source->getChannelStatus(ChannelID)->getOccupation());
	else
		Source->setLocalResult(1, Source->getControlStatus(ChannelID)->getOccupation());

	// Fix transition probabilities (individual time samples lost)
	
	list<SADF_Transition*> OriginalTransitions(Source->getTransitions());

	Source->removeAllTransitions();	

	for (list<SADF_Transition*>::iterator i = OriginalTransitions.begin(); i != OriginalTransitions.end(); i++) {

		SADF_Transition* ExistingTransition = Source->hasTransitionToConfigurationWithIdentity((*i)->getDestination()->getIdentity());

		if (ExistingTransition == NULL)
			Source->addTransition((*i)->getDestination(), (*i)->getProbability(), 0);
		else
			ExistingTransition->setProbability(ExistingTransition->getProbability() + (*i)->getProbability());
			
		delete *i;
	}

	// Proceed constructing TPS from new configurations

	while (!NewConfigurations.empty()) {
		SADF_ConstructTPS_BufferOccupancy(Graph, TPS, NewConfigurations.front(), ChannelType, ChannelID, SourceProcessType, SourceProcessID, DestinationProcessType, DestinationProcessID);
		NewConfigurations.pop_front();
	}
}

void SADF_ConstructTPS_BufferOccupancy(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId SourceProcessType, CId SourceProcessID) {

	SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP_BufferOccupancy(Graph, TPS, Source, SourceProcessType, SourceProcessID);

	// Remove transitions
	
	Source->removeAllTransitions();

	// Proceed constructing TPS from new configurations

	while (!NewConfigurations.empty()) {
		SADF_ConstructTPS_BufferOccupancy(Graph, TPS, NewConfigurations.front(), SourceProcessType, SourceProcessID);
		NewConfigurations.pop_front();
	}
}

// Function to analyse long-run buffer occupancy

CSize SADF_Analyse_LongRunBufferOccupancy(SADF_Graph* Graph, CId ChannelType, CId ChannelID, CDouble& Average, CDouble& Variance) {

	// Check whether graph satisfied required properties

	if (!SADF_Verify_SingleComponent(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not consist of a single component.");

	if (!SADF_Verify_Timed(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not timed.");
	
	if (!SADF_Verify_Boundedness(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not bounded.");

	bool Ergodic = SADF_Verify_SimpleErgodicity(Graph);	// Only in case Ergodic is true, the SADF graph is ergodic for sure. Otherwise, ergodicity test needed after generating TPS

	// Construct TPS

	CId SourceProcessType;
	CId SourceProcessID;
	CId DestinationProcessType;
	CId DestinationProcessID;
	
	if (ChannelType == SADF_DATA_CHANNEL) {
		SourceProcessType = Graph->getDataChannel(ChannelID)->getSource()->getType();
		SourceProcessID = Graph->getDataChannel(ChannelID)->getSource()->getIdentity();
		DestinationProcessType = Graph->getDataChannel(ChannelID)->getDestination()->getType();
		DestinationProcessID = Graph->getDataChannel(ChannelID)->getDestination()->getIdentity();
	} else {
		SourceProcessType = Graph->getControlChannel(ChannelID)->getSource()->getType();
		SourceProcessID = Graph->getControlChannel(ChannelID)->getSource()->getIdentity();
		DestinationProcessType = Graph->getControlChannel(ChannelID)->getDestination()->getType();
		DestinationProcessID = Graph->getControlChannel(ChannelID)->getDestination()->getIdentity();
	}

	SADF_TPS* TPS = new SADF_TPS(Graph);

	SADF_ConstructTPS_BufferOccupancy(Graph, TPS, TPS->getInitialConfiguration(), ChannelType, ChannelID, SourceProcessType, SourceProcessID, DestinationProcessType, DestinationProcessID);

	TPS->deleteContentOfConfigurations();
	TPS->removeTransientConfigurations();

	if (!Ergodic)
		if (!TPS->isSingleStronglyConnectedComponent())
			throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not imply an ergodic Markov chain.");

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

    vector<CDouble> EquilibriumDistribution = TPS->computeEquilibriumDistribution();

	// Compute results

	CDouble AverageTime = 0;
	CDouble AverageBufferOccupancy = 0;
	CDouble AverageSquaredBufferOccupancy = 0;

	for (SADF_HashedListOfConfigurations::iterator i = TPS->getConfigurationSpace().begin(); i != TPS->getConfigurationSpace().end(); i++)
		for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++) {
	
			CDouble Temp = EquilibriumDistribution[(*j)->getIdentity()] * (*j)->getLocalResult(0);

			AverageTime += Temp;
			AverageBufferOccupancy += Temp * (*j)->getLocalResult(1);
			AverageSquaredBufferOccupancy += Temp * (*j)->getLocalResult(1) * (*j)->getLocalResult(1);
		}
	
	delete TPS;
	
	if (AverageTime == 0)
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not perform any scenarios in the long-run that imply progress in time.");

	Average = AverageBufferOccupancy / AverageTime;
	Variance = (AverageSquaredBufferOccupancy / AverageTime) - ((AverageBufferOccupancy * AverageBufferOccupancy) / (AverageTime * AverageTime));

	if (Variance < 0)
		Variance = 0;					// Circumvent rounding errors;

	return NumberOfConfigurations;
}

// Function to analyse maximum buffer occupancy

CSize SADF_Analyse_MaximumBufferOccupancy(SADF_Graph* Graph, CId ChannelType, CId ChannelID, CDouble& Maximum) {

	// Check whether graph satisfied required properties

	if (!SADF_Verify_SingleComponent(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not consist of a single component.");
	
	if (!SADF_Verify_Boundedness(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not bounded.");

	// Construct TPS

	CId SourceProcessType;
	CId SourceProcessID;
	
	if (ChannelType == SADF_DATA_CHANNEL) {
		SourceProcessType = Graph->getDataChannel(ChannelID)->getSource()->getType();
		SourceProcessID = Graph->getDataChannel(ChannelID)->getSource()->getIdentity();
	} else {
		SourceProcessType = Graph->getControlChannel(ChannelID)->getSource()->getType();
		SourceProcessID = Graph->getControlChannel(ChannelID)->getSource()->getIdentity();
	}

	SADF_TPS* TPS = new SADF_TPS(Graph);
	TPS->addConfiguration(TPS->getInitialConfiguration());
	SADF_ConstructTPS_BufferOccupancy(Graph, TPS, TPS->getInitialConfiguration(), SourceProcessType, SourceProcessID);

	// Compute Results

	Maximum = 0;

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

	for (SADF_HashedListOfConfigurations::iterator i = TPS->getConfigurationSpace().begin(); i != TPS->getConfigurationSpace().end(); i++)
		for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++)
			if (ChannelType == SADF_DATA_CHANNEL) {
				if ((*j)->getChannelStatus(ChannelID)->getOccupation() > Maximum)
					Maximum = (*j)->getChannelStatus(ChannelID)->getOccupation();
			} else {
				if ((*j)->getControlStatus(ChannelID)->getOccupation() > Maximum)
					Maximum = (*j)->getControlStatus(ChannelID)->getOccupation();
			}

	delete TPS;
	
	return NumberOfConfigurations;
}

// Functions to analyse buffer size
/*
void SADF_ConstructTPS_BufferSize(SADF_Graph* Graph, SADF_TPS* TPS, SADF_Configuration* Source, CId ChannelType, CId ChannelID, CId SourceProcessType, CId SourceProcessID) {

	SADF_ListOfConfigurations NewConfigurations = SADF_ProgressTPS_ASAP_BufferSize(Graph, TPS, Source, ChannelType, ChannelID, SourceProcessType, SourceProcessID);
	
	// Proceed constructing TPS from new configurations

	while (!NewConfigurations.empty()) {
		SADF_ConstructTPS_BufferSize(Graph, TPS, NewConfigurations.front(), ChannelType, ChannelID, SourceProcessType, SourceProcessID);
		NewConfigurations.pop_front();
	}
}

CSize SADF_Analyse_BufferSize(SADF_Graph* Graph, CId ChannelType, CId ChannelID, CDouble& Result) {

	// Check whether graph satisfied required properties

	if (!SADF_Verify_SingleComponent(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not consist of a single component.");
	
	if (!SADF_Verify_Boundedness(Graph))
		throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' is not bounded.");

	// Construct TPS

	CId SourceProcessType;
	CId SourceProcessID;
	
	if (ChannelType == SADF_DATA_CHANNEL) {
		SourceProcessType = Graph->getDataChannel(ChannelID)->getSource()->getType();
		SourceProcessID = Graph->getDataChannel(ChannelID)->getSource()->getIdentity();
	} else {
		SourceProcessType = Graph->getControlChannel(ChannelID)->getSource()->getType();
		SourceProcessID = Graph->getControlChannel(ChannelID)->getSource()->getIdentity();
	}

	SADF_TPS* TPS = new SADF_TPS(Graph);
	TPS->addConfiguration(TPS->getInitialConfiguration());
	SADF_ConstructTPS_BufferSize(Graph, TPS, TPS->getInitialConfiguration(), ChannelType, ChannelID, SourceProcessType, SourceProcessID);

	// Compute Results

	Result = 0;

	CSize NumberOfConfigurations = TPS->getNumberOfConfigurations();

	for (SADF_HashedListOfConfigurations::iterator i = TPS->getConfigurationSpace().begin(); i != TPS->getConfigurationSpace().end(); i++)
		for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++)
			if (ChannelType == SADF_DATA_CHANNEL) {
				if ((*j)->getChannelStatus(ChannelID)->getOccupation() > Result)
					Result = (*j)->getChannelStatus(ChannelID)->getOccupation();
			} else {
				if ((*j)->getControlStatus(ChannelID)->getOccupation() > Result)
					Result = (*j)->getControlStatus(ChannelID)->getOccupation();
			}

	delete TPS;
	
	return NumberOfConfigurations;
}
*/
